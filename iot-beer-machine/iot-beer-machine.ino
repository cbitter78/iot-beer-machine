#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>  
#include <WiFi101.h>  
#include <WiFiUdp.h> 
#include <EasyNTPClient.h> 
#include <TimeLib.h>
#include <TimeAlarms.h>          
#include <ArduinoJson.h>         /* https://arduinojson.org/v6/assistant */

#include "secrets.h"
#include "logging.h"
#include "lcd_display.h"
#include "vend_slot.h"
#include "machine.h"             /* Switch from machine and mock machine in the respeictive .cpp files */

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2 
#define UDP_NTP_PORT 2390 

LcdDisplay l_display;
Machine machine(&l_display);

/* All things MQTT */
WiFiSSLClient client;
Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 8883, AIO_USERNAME, AIO_KEY);  // We should be using port 8883
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/cmd-rx");

WiFiUDP Udp;
EasyNTPClient ntpClient(Udp, "time.nist.gov"); 

void setup(void){
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  

  l_display.init();

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  wifi_connect();

  // Set up NTP
  delay(2000);             /* Give the UDP port time to set up */
  time_t t = getTime();
  while (t == 0){
    delay(1000);
    Serial.print(F("Waiting for NTP Sync: "));
    Serial.println((unsigned int)now());
    t = getTime();
  }
  setTime(t);
  setSyncInterval(1800);     /* NTP sync every 30 Minutes */
  setSyncProvider(getTime);

  Alarm.timerRepeat(60, every_1_minute);  
  Alarm.timerRepeat(30, every_30_seconds);
  Alarm.timerRepeat(5,  update_sensor_data);

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle);                                                         
  DEBUG_PRINT(F("Adafruit:SUBSCRIPTIONDATALEN: "));
  DEBUG_PRINTLN(SUBSCRIPTIONDATALEN);
  DEBUG_PRINT(F("Adafruit::MAXBUFFERSIZE: "));
  DEBUG_PRINTLN(MAXBUFFERSIZE);

  machine.init();
  update_sensor_data();
}


void loop(void){
  Alarm.delay(250);  
  MQTTProcessMessages(2000);
  flash_built_in_led();
}



void MQTTProcessMessages(int timeout){
  MQTT_connect();
  Adafruit_MQTT_Subscribe *sub;
  while ((sub = mqtt.readSubscription(timeout))) {
    if (sub == &aio_command) {
      char * cmd_data = (char *)aio_command.lastread;
      DEBUG_PRINT(F("MQTTProcessMessages: cmd_data: "));
      DEBUG_PRINTLN(cmd_data);
      vend(cmd_data);
      
    } else if(sub == &aio_errors) {
      String mqtt_error = String((char *)aio_errors.lastread);
      ERROR_PRINT(F("MQTT ERROR: "));
      ERROR_PRINTLN(mqtt_error);
      l_display.scroll_msg(String(F("  ..!MQTT ERROR!..  ")) + mqtt_error, 80, 10000);
  
    } else if(sub == &aio_throttle){
      String mqtt_error = String((char *)aio_errors.lastread);
      ERROR_PRINT(F("MQTT THROTTLE ERROR: "));
      ERROR_PRINTLN(mqtt_error);
      l_display.scroll_msg(String(F("  !MQTT THROTTLE!   ")) + mqtt_error, 80, 10000);
    }  
  }   
}

void update_sensor_data(){
  externalSensorData ed = machine.read_external();
  internalSensorData id = machine.read_internal();
  powerSensorData    pd = machine.read_power_usage();

  l_display.set_external_temp(ed.FahrenheitTemp);
  l_display.set_external_inHg(ed.Pressure_inHG);
  l_display.set_external_humidity(ed.Humidity);
  l_display.set_internal_temp(id.FahrenheitTemp);
  l_display.set_amps(pd.Amps);
  l_display.set_watts(pd.Watts);
 
  machine.update_all_slot_status();
}


void every_30_seconds(){
  DEBUG_PRINTLN("every_30_seconds:");
  MQTT_connect();
  mqtt.ping();
}

void every_1_minute(){
  DEBUG_PRINTLN("every_1_minute:");
  l_display.display_network_info(5000);
}


void vend(char *data) {
  DEBUG_PRINT(F("vend:data: "));
  DEBUG_PRINTLN(data);
 
  // Parse the JSON document 
  // Example:
  //          {"name":"vend","id":"1","slot":2,"args":["Charles", "Beer"]}
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
 
  if (cmd_slot < Machine::SLOT_COUNT && cmd_slot >= 0 ){
    VendSlot v = *machine.slots[cmd_slot];
    l_display.start_vend(cmd_slot, beer);
    v.vend();
    
    int slot_status = v.slot_status();
    if (slot_status > SLOT_STATUS_RUNNING_OUT){
      ERROR_PRINT(F("vend:ERROR: slot_status: "));
      ERROR_PRINT(slot_status);
      vend_status_str = String(F("ERROR,")) + String(slot_status);
      l_display.scroll_msg(String(F("  !!VENDING ERROR!!       ")) + vend_status_str, 80, 4000);
    }else if(slot_status == SLOT_STATUS_RUNNING_OUT){
      vend_status_str = String(F("OK,RUNNING_OUT"));
    }else{
      l_display.finish_vend(beer, drinker, 4000);
    }
    l_display.display_default_status();
  }
  else{
    vend_status_str = "ERROR,NO_SUCH_SLOT";
    WARN_PRINTLN("vend: slot: " + String(cmd_slot) + " does not exists");
  }

  String rx = String(cmd_id) + "::" + vend_status_str;
  char buff[rx.length() + 1];
  rx.toCharArray(buff, rx.length() +1);
  if (! aio_command_rx.publish(buff)) {
    ERROR_PRINT(F("vend: ERROR:MQTT: Failed to post to cmd_rx. post_data: "));
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
    Udp.begin(UDP_NTP_PORT);  /* Set up UDP port to listen and get the NTP responce */
    delay(1000);
    l_display.display_network_info(3000);
  }
}


time_t getTime(){
  wifi_connect();
  Serial.print(F("Syncing Time from NTP: "));
  time_t t = ntpClient.getUnixTime();
  Serial.println((unsigned long)t);
  return t;
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
  delay(200);                     
  digitalWrite(LED_BUILTIN, LOW);  
  delay(200);                       
}
