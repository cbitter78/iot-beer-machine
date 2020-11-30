#include <Adafruit_ADS1015.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoJson.h>                    // https://arduinojson.org/v6/assistant/
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFi101.h>
#include <Wire.h>
#include <SPI.h>
#include "VendSlot.h"
#include "yhdc.h"
#include "util.h"
#include "secrets.h"
#include "logging.h"
#include "lcd_display.h"

Adafruit_BME280 bme;


#include "mock_machine.h"  /* Only used when testing. */
//#include "machine.h"      /* Used for beer machine. */

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
LcdDisplay l_display;

WiFiClient client;

Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/cmd-rx");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");


OneWire oneWire(15);
DallasTemperature internalTempSensor(&oneWire);

void setup(void)
{
  Serial.begin(115200);
  
  internalTempSensor.begin();
  unsigned status = bme.begin();  
  if (!status) {
      ERROR_PRINTLN(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
  }
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle);
   
  lcd.init();
  lcd.backlight();
  l_display.init(&lcd);

                                                                     
  DEBUG_PRINT(F("Adafruit:SUBSCRIPTIONDATALEN: "));
  DEBUG_PRINTLN(SUBSCRIPTIONDATALEN);
  DEBUG_PRINT(F("Adafruit::MAXBUFFERSIZE: "));
  DEBUG_PRINTLN(MAXBUFFERSIZE);

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
    print_amps();
    print_internal_temp();
    print_external_temp();
}

void print_external_temp(){
  float c = bme.readTemperature();
  float f = (c*1.8)+32.0f;
  float hPa = bme.readPressure() / 100.0F;
  float inHg = (hPa / 3386.0F) * 100.0F;
  float h = bme.readHumidity();
  
  l_display.set_external_temp(f);
  l_display.set_external_humidity(h);
  l_display.set_external_inHg(inHg);
  
  INFO_PRINT(F("Exterinal Temp: "));
  INFO_PRINT(c,2);
  INFO_PRINT(F(" *C  "));
  INFO_PRINT(f,2);
  INFO_PRINTLN(F(" *F"));
  
  INFO_PRINT(F("Exterinal Pressure: "));
  INFO_PRINT(hPa, 4);
  INFO_PRINT(F(" hPa "));
  INFO_PRINT(inHg, 4);
  INFO_PRINTLN(F(" inHg"));
  
  INFO_PRINT(F("Exterinal Humidity: "));
  INFO_PRINT(h, 4);
  INFO_PRINTLN(F("%"));
}


void print_internal_temp(){
  internalTempSensor.requestTemperatures(); 
  float c = internalTempSensor.getTempCByIndex(0);
  float f = internalTempSensor.toFahrenheit(c);
  l_display.set_internal_temp(f);
  INFO_PRINT(F("Internal Temp in Celcius: "));
  INFO_PRINTLN(c, 4);
  INFO_PRINT(F("Internal Temp in Fahrenheit: "));
  INFO_PRINTLN(f, 4);
  
}

void print_amps(){
  float a = readACCurrentValue(A0);
  float w = a * 110
  l_display.set_amps(a);
  l_display.set_watts(w);
  INFO_PRINT(F("Amps: "));
  INFO_PRINT(a, 2);
  INFO_PRINT(F(" Watts: "));
  INFO_PRINTLN(w, 1);
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



void wifi_connect(){
  while (WiFi.status() != WL_CONNECTED) {
    INFO_PRINT(F("Attempting to connect to SSID: "));
    INFO_PRINTLN(WIFI_SSID);
    l_display.clear();
    l_display.printAt(l_display.center("Connecting To:"), 0, 0);
    l_display.printAt(l_display.center(WIFI_SSID), 0, 1);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  
    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      for (int i = 0; i < 4; i++){
        l_display.delay_with_animation(250, 1);
      }
    }
    
    l_display.scroll_msg("Wifi Connected", 100, -1);
    delay(1000);
    l_display.display_network_info(3000);
  }
}



// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  wifi_connect();
  
  // Stop if already connected.
  if (mqtt.connected()) {
    l_display.set_adafruit_status(true);
    return;
  }

  /* We have lost connection.  Connecting logic */
  l_display.set_adafruit_status(false);
  l_display.clear();
  l_display.printAt(l_display.center("Connecting To MQTT"), 0, 0);
  INFO_PRINTLN(F("Connecting to MQTT... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
     mqtt.disconnect();
     l_display.printAt(l_display.center("MQTT Status:"), 0, 1);
     l_display.printAt(l_display.center(mqtt.connectErrorString(ret)), 0, 2);
     
     INFO_PRINT(F("MQTT Error: "));
     INFO_PRINTLN(mqtt.connectErrorString(ret));
     INFO_PRINTLN(F("Retrying MQTT connection in 5 seconds..."));
     for (int i = 0; i < 21; i++){  /* 5 Second Delay  with animation */
        l_display.delay_with_animation(250, 1);
     }
  }
  INFO_PRINTLN(F("MQTT Connected!"));
  l_display.set_adafruit_status(true);
  l_display.display_default_status();
}
