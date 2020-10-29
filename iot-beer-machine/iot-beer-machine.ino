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

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2  

LiquidCrystal_I2C lcd(0x27,20,4); 
Adafruit_ADS1115  a2d(0x48);      
WiFiClient        client;

Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/iot-beer-command-rx");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/iot-beer-command");
Adafruit_MQTT_Subscribe timefeed       = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

int status = WL_IDLE_STATUS;

VendSlot* slots[2];
VendSlot slot1;
VendSlot slot2;

void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  aio_command.setCallback(vend_callback);
  mqtt.subscribe(&aio_command);
  
  timefeed.setCallback(timecallback);
  mqtt.subscribe(&timefeed);
  
  a2d.begin();  
  lcd.init();
  lcd.backlight();


  slot1.setup(1, 12, &a2d, 0, &a2d, 2, &lcd, 0, 0);
  slot2.setup(2, 11, &a2d, 1, &a2d, 3, &lcd, 1, 0);

  slots[0] = &slot1;
  slots[1] = &slot2;

//  int i = 0;
//  for (i = 0; i < 2; i++){
//    (*slots[i]).vend();
//  }

}


void vend_callback(char *data, uint16_t len) {
  Serial.print("received <- ");
  Serial.println(data);

  DynamicJsonDocument doc(256);

  // Parse the JSON document from the HTTP response
  DeserializationError err = deserializeJson(doc, data);
  if (err) {
    Serial.print(F("Parsing command failed: "));
    Serial.println(err.c_str());
    return;
  }

  const char* device = doc["device"]; // "BEER1"
  JsonObject cmd = doc["cmd"];
  const char* cmd_name = cmd["name"]; // "vend"
  const char* cmd_id = cmd["id"]; // "55e254fd-a4d6-41d6-9817-f2e280132d56"
  int cmd_slot = cmd["slot"]; // 1
  const char* cmd_msg = cmd["msg"];

  Serial.println("Device:   " + String(device));
  Serial.println("cmd:name  " + String(cmd_name));
  Serial.println("cmd:id:   " + String(cmd_id));
  Serial.println("cmd.slot: " + String(cmd_slot));
  Serial.println("cmd.msg:  " + String(cmd_msg));

//  if (doc["device"] != DEVICE_ID){
//    Serial.println("Command for differnt device");
//    return;
//  }
//
//  if(doc["cmd"]["name"] == "vend"){
//    vend_slot(doc["cmd"]["args"][0], doc["cmd"]["id"]); 
//    return;
//  }
//
//  report_error("Unknown Command", 404, doc["cmd"]["id"]);
}


void loop(void)
{
    MQTT_connect();
    mqtt.processPackets(10000);
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
