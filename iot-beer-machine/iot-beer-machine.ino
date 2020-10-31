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
#include "secrets.h"

#define MQTT_DEBUG

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2  

#define SLOT_COUNT 2
VendSlot* slots[SLOT_COUNT];
VendSlot  slot1;
VendSlot  slot2;

LiquidCrystal_I2C lcd(0x27,20,4); 
Adafruit_ADS1115  a2d(0x48);      
WiFiClient        client;

Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/iot-beer-command-rx",  /* QOS: Once */ 1);
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/iot-beer-command",     /* QOS: Once */ 1);
//Adafruit_MQTT_Subscribe timefeed       = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");

int status = WL_IDLE_STATUS;

void on_aio_error(char *data, uint16_t len) {
  Serial.print("AIO_ERROR: on_aio_error:received <- ");
  Serial.println(data);
}

void on_throttle(char *data, uint16_t len) {
  Serial.print("AIO_WARNING: on_throttle:received <- ");
  Serial.println(data);
}


void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle);
  
  //timefeed.setCallback(timecallback);
  //mqtt.subscribe(&timefeed);
  
  a2d.begin();  
  lcd.init();
  lcd.backlight();

  Serial.print("Adafruit Max data size: ");
  Serial.println(SUBSCRIPTIONDATALEN);

  // Because our slots array only holds referances we need to keep 
  // a copy of the slot by using a global variable.   If not the slot
  // object will be destroyed and the pointer will point to the wrong
  // memory. 
  slot1.setup(1, 12, &a2d, 0, &a2d, 2, &lcd, 0);
  slot2.setup(2, 11, &a2d, 1, &a2d, 3, &lcd, 1);

//  slot3.setup(3, 10, &a2d_2, 0, &a2d, 2, &lcd_2, 0);
//  slot4.setup(4,  9, &a2d_2, 1, &a2d, 3, &lcd_2, 1);
//  slot5.setup(5,  6, &a2d_2, 0, &a2d, 2, &lcd_3, 0);
//  slot6.setup(6,  5, &a2d_2, 1, &a2d, 3, &lcd_3, 1);

  slots[0] = &slot1;
  slots[1] = &slot2;
}


void vend(char *data) {
  Serial.print("vend_callback:received <- ");
  Serial.println(data);
 
  // Parse the JSON document 
  // Example:
  //          {"name":"vend","id":"1","slot":2,"args":["Charles"]}
  //
  DynamicJsonDocument doc(200);
  DeserializationError err = deserializeJson(doc, data);
  if (err) {
    Serial.print(F("Parsing command failed: "));
    Serial.println(err.c_str());
    return;
  }

  const char* cmd_name = doc["name"]; 
  const char* cmd_id = doc["id"]; 
  int         cmd_slot = doc["slot"]; 
  const char* recipient = doc["args"][0]; 
   
  Serial.println("cmd:name  " + String(cmd_name));
  Serial.println("cmd:id:   " + String(cmd_id));
  Serial.println("cmd.slot: " + String(cmd_slot));
  Serial.println("cmd.msg:  " + String(recipient));
  if (cmd_slot <= SLOT_COUNT && cmd_slot > 0){
    (*slots[cmd_slot - 1]).vend();
  }
  else{
    Serial.println("Slot " + String(cmd_slot) + " does not exists and cant give you beer :(");
  }


  //TODO:  Set up response and find a way to handle dropped packdets.   perhaps use get?
}


void loop(void)
{
    MQTT_connect();
    //mqtt.processPackets(5000);
    
    
    Adafruit_MQTT_Subscribe *sub;
    while ((sub = mqtt.readSubscription(10000))) {
      if (sub == &aio_command) {
        vend((char *)aio_command.lastread);
      } else if(sub == &aio_errors) {
        Serial.print(F("ERROR: "));
        Serial.println((char *)aio_errors.lastread);
      } else if(sub == &aio_throttle){
        Serial.print(F("THROTTLE ERROR: "));
        Serial.println((char *)aio_throttle.lastread);
      }
      
        
    }
        
    mqtt.ping();
      
    if (analogRead(A0) > 100){
      (*slots[0]).vend();
    }

    if (analogRead(A1) > 100){
      (*slots[1]).vend();
    }
  
    //print_adc(a2d, lcd);
    flash();
    delay(250);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);

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

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}









int timeZone = -4; // UTC - 4 eastern daylight time (nyc)
int interval = 4; // trigger every X hours
int last_min = -1;

void timecallback(uint32_t current) {
  // adjust to local time zone
  current += (timeZone * 60 * 60);
  int curr_hour = (current / 60 / 60) % 24;
  int curr_min  = (current / 60 ) % 60;
  int curr_sec  = (current) % 60;

  Serial.print("Time: "); 
  Serial.print(curr_hour); Serial.print(':'); 
  Serial.print(curr_min); Serial.print(':'); 
  Serial.println(curr_sec);
  
  // only trigger on minute change
  if(curr_min != last_min) {
    last_min = curr_min;
    
    Serial.println("This will print out every minute!");
  }
}
