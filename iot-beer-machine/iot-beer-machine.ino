#include <Adafruit_ADS1015.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>                
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>         /* https://arduinojson.org/v6/assistant */
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <WiFi101.h>
#include <Wire.h>
#include <SPI.h>

#include "VendSlot.h"
#include "yhdc.h"
#include "secrets.h"
#include "logging.h"
#include "lcd_display.h"
#include "hold_on.h"

#include "machine.h"      /* Switch from machine and mock machine in the respeictive .cpp files */

/* Set up HoldOn timers */
void every_30_seconds();
void every_1_minute();
void every_15_minutes();
void every_60_minutes();

HoldOn every30s((30 * 1000),      every_30_seconds);
HoldOn every1m ((60 * 1000),      every_1_minute);
HoldOn every15m((15 * 60 * 1000), every_15_minutes);
HoldOn every60m((60 * 60 * 1000), every_15_minutes);

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2  
WiFiClient client;

LiquidCrystal_I2C lcd(0x27,20,4); 
LcdDisplay l_display;

Machine machine(&l_display);

/* All things MQTT */
Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/cmd-rx");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");

void setup(void){
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  l_display.init(&lcd);

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  wifi_connect();

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle);                                                         
  DEBUG_PRINT(F("Adafruit:SUBSCRIPTIONDATALEN: "));
  DEBUG_PRINTLN(SUBSCRIPTIONDATALEN);
  DEBUG_PRINT(F("Adafruit::MAXBUFFERSIZE: "));
  DEBUG_PRINTLN(MAXBUFFERSIZE);

  machine.init();
}


void loop(void){
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
    flash_built_in_led();
    print_sensor_data();

    every30s.ReadyYet();
    every1m.ReadyYet();
    every15m.ReadyYet();
    every60m.ReadyYet();
}

void print_sensor_data(){
  externalSensorData ed = machine.read_external();
  internalSensorData id = machine.read_internal();
  powerSensorData    pd = machine.read_power_usage();

  l_display.set_external_temp(ed.FahrenheitTemp);
  l_display.set_external_inHg(ed.Pressure_inHG);
  l_display.set_external_humidity(ed.Humidity);
  l_display.set_internal_temp(id.FahrenheitTemp);
  l_display.set_amps(pd.Amps);
  l_display.set_watts(pd.Watts);
 
  for (int i = 0; i <  Machine::SLOT_COUNT; i++){
    VendSlot v = *machine.slots[i];
    l_display.set_slot_status(i, v.slot_status());
  }
}

void every_30_seconds(){
  DEBUG_PRINTLN("every_30_seconds:");
  mqtt.ping();
}

void every_1_minute(){
  DEBUG_PRINTLN("every_1_minute:");
}

void every_15_minutes(){
  DEBUG_PRINTLN("every_15_minutes:");
}

void every_60_minutes(){
  DEBUG_PRINTLN("every_15_minutes:");
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

  String vend_status_str = "OK";
  const char* cmd_name   = doc["cmd"]; 
  const char* cmd_id     = doc["id"]; 
  const char* drinker    = doc["args"][0]; 
  const char* beer       = doc["args"][1];
  int         cmd_slot   = doc["slot"];
  
  if (cmd_slot <= Machine::SLOT_COUNT && cmd_slot > 0){
    VendSlot v = *machine.slots[cmd_slot];
    l_display.start_vend(cmd_slot, beer);
    v.vend();
    if (v.slot_status() != SLOT_STATUS_OK){
      vend_status_str = String(v.slot_status());
    }
    l_display.finish_vend(beer, drinker, 4000);
    l_display.display_default_status();
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



/*
 * Function to connect and reconnect as necessary to the MQTT server.
 * Should be called in the loop function and it will take care if connecting.
 */ 
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


void flash_built_in_led(){
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(250);                     
  digitalWrite(LED_BUILTIN, LOW);  
  delay(250);                       
}
