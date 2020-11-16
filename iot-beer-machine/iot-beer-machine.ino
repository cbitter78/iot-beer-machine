#include <Adafruit_ADS1015.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoJson.h>                    // https://arduinojson.org/v6/assistant/
#include <LiquidCrystal_I2C.h>
#include <WiFi101.h>
#include <Wire.h>
#include <SPI.h>
#include "VendSlot.h"
#include "util.h"
#include "lcd_art.h"
#include "secrets.h"
#include "logging.h"

//#include "mock_machine.h"  /* Only used when testing. */
#include "machine.h"      /* Used for beer machine. */

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2  

/*
 A slot will need to be defined for each slot in the beer machine.
 */
#define SLOT_COUNT 6
VendSlot* slots[SLOT_COUNT];
VendSlot  slot1;
VendSlot  slot2;
VendSlot  slot3;
VendSlot  slot4;
VendSlot  slot5;
VendSlot  slot6;

#define ADC_COUNT 3
Adafruit_ADS1115* adcs[ADC_COUNT];
Adafruit_ADS1115  adc0(ADC_0_ADDRESS); 
Adafruit_ADS1115  adc1(ADC_1_ADDRESS); 
Adafruit_ADS1115  adc2(ADC_2_ADDRESS); 

LiquidCrystal_I2C lcd(0x27,20,4); 
LiquidCrystal_I2C lcd_msg(0x25,20,4);    
WiFiClient        client;

Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/cmd-rx");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");


void setup(void)
{
  Serial.begin(115200);
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle);
   
   
  lcd.init();
  lcd.backlight();
  lcd_msg.init();

  byte* lcd_custom_chars[8]     = {charWait0, charWait1, charWait2, charWifi, charAdaFruitNotConnected, charAdaFruitConnected, charSmile, charSkull};
  byte* lcd_msg_custom_chars[8] = {charWifi, charAdaFruitNotConnected, charAdaFruitConnected, charSmile, charFrown, charSkull};
  
  for (int i = 0; i < 8; i++){
    lcd.createChar(i, lcd_custom_chars[i]);
  }
  for (int i = 0; i < 6; i++){
    lcd_msg.createChar(i, lcd_msg_custom_chars[i]);
  }


                                                                     
  Serial.print(F("Adafruit:SUBSCRIPTIONDATALEN: "));
  Serial.println(SUBSCRIPTIONDATALEN);
  Serial.print(F("Adafruit::MAXBUFFERSIZE: "));
  Serial.println(MAXBUFFERSIZE);

  /*
  Set up an array for ADC's so that whenwe set up the slots
  below we can use an ADC number to get the correct object.
  */
  adcs[0] = &adc0;
  adcs[1] = &adc1;
  adcs[2] = &adc2;
  for (int i = 0; i < ADC_COUNT; i++){
    (*adcs[i]).begin();
  }

  /* 
  Because our slots array only holds referances we need to keep 
  a copy of the slot by using a global variable.   If not the slot
  object will be destroyed and the pointer will point to the wrong
  memory.
  */ 
  slot1.setup(1, SLOT_1_DIO_PIN, adcs[SLOT_1_VEND_ADC], SLOT_1_VEND_ADC_PIN, adcs[SLOT_1_EMPTY_ADC], SLOT_1_EMPTY_ADC_PIN, &lcd, 0, 7);
  slot2.setup(2, SLOT_2_DIO_PIN, adcs[SLOT_2_VEND_ADC], SLOT_2_VEND_ADC_PIN, adcs[SLOT_2_EMPTY_ADC], SLOT_2_EMPTY_ADC_PIN, &lcd, 0, 12);
  slot3.setup(3, SLOT_3_DIO_PIN, adcs[SLOT_3_VEND_ADC], SLOT_3_VEND_ADC_PIN, adcs[SLOT_3_EMPTY_ADC], SLOT_3_EMPTY_ADC_PIN, &lcd, 0, 17);
  slot4.setup(4, SLOT_4_DIO_PIN, adcs[SLOT_4_VEND_ADC], SLOT_4_VEND_ADC_PIN, adcs[SLOT_4_EMPTY_ADC], SLOT_4_EMPTY_ADC_PIN, &lcd, 1, 7);
  slot5.setup(5, SLOT_5_DIO_PIN, adcs[SLOT_5_VEND_ADC], SLOT_5_VEND_ADC_PIN, adcs[SLOT_5_EMPTY_ADC], SLOT_5_EMPTY_ADC_PIN, &lcd, 1, 12);
  slot6.setup(6, SLOT_6_DIO_PIN, adcs[SLOT_6_VEND_ADC], SLOT_6_VEND_ADC_PIN, adcs[SLOT_6_EMPTY_ADC], SLOT_6_EMPTY_ADC_PIN, &lcd, 1, 17);;

  slots[0] = &slot1;
  slots[1] = &slot2;
  slots[2] = &slot3;
  slots[3] = &slot4;
  slots[4] = &slot5;
  slots[5] = &slot6;

//  for (int i = 0; i < SLOT_COUNT; i++){
//    if ((*slots[i]).slot_status() == VendSlot::SLOT_STATUS_RUNNING_OUT){
//      String msg = "WARNING!!!  Slot " + String(i +1) + "  is running OUT of   BEER!!  :(";
//      lcd_display_msg(msg, &lcd_msg, 100, true, 3000);
//    }
//  }
}


void loop(void)
{
    MQTT_connect();
      
    Adafruit_MQTT_Subscribe *sub;
    while ((sub = mqtt.readSubscription(10000))) {
      if (sub == &aio_command) {
        vend((char *)aio_command.lastread);
      } else if(sub == &aio_errors) {
        ERROR_PRINT(F("ERROR: "));
        ERROR_PRINTLN((char *)aio_errors.lastread);
      } else if(sub == &aio_throttle){
        ERROR_PRINT(F("THROTTLE ERROR: "));
        ERROR_PRINTLN((char *)aio_throttle.lastread);
      }  
    }

    mqtt_ping(&mqtt);
       
    flash(13);
}


void vend(char *data) {
  DEBUG_PRINT(F("vend:received <- "));
  DEBUG_PRINTLN(data);
 
  // Parse the JSON document 
  // Example:
  //          {"name":"vend","id":"1","slot":2,"args":["Charles"]}
  //
  DynamicJsonDocument doc(200);
  DeserializationError err = deserializeJson(doc, data);
  if (err) {
    WARN_PRINT(F("Parsing command failed: "));
    WARN_PRINTLN(err.c_str());
    return;
  }

  const char* cmd_name  = doc["cmd"]; 
  const char* cmd_id    = doc["id"]; 
  const char* recipient = doc["args"][0]; 
  const char* beer      = doc["args"][1];
  int         cmd_slot  = doc["slot"];
  
  String msg = String(recipient) + String("!  Enjoy drinking your ") + String(beer) + String(" :) "); 
  String vend_status_str = "Unknown";
  if (cmd_slot <= SLOT_COUNT && cmd_slot > 0){
    int vend_status = (*slots[cmd_slot - 1]).vend();

    switch (vend_status) { 
      case VendSlot::VEND_STATUS_READY :
        vend_status_str = "OK";
        break;
        
      case VendSlot::VEND_STATUS_STUCK :
        vend_status_str = "STUCK";
        break;

      case VendSlot::VEND_STATUS_ERROR :
        vend_status_str = "ERROR";
        break;      
    }

    //lcd_display_msg(msg, &lcd_msg, 100, true, 4000);

  }
  else{
    //TODO: Post an error back
    vend_status_str = "NO_SUCH_SLOT";
    ERROR_PRINTLN("Slot " + String(cmd_slot) + " does not exists and cant give you beer :(");
  }

  String rx = String(cmd_id) + "::" + vend_status_str;
  char buff[rx.length() + 1];
  rx.toCharArray(buff, rx.length() +1);
  if (! aio_command_rx.publish(buff)) {
    ERROR_PRINT(F("Failed to post to cmd_rx"));
    ERROR_PRINTLN(buff);
  } 

}




// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Connecting to MQTT... "));

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 5 seconds..."));
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println(F("MQTT Connected!"));
  lcd.setCursor(0,3);
  lcd.print(F("MQTT Connected!"));
}
