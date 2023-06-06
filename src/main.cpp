#include "Arduino.h"
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>  
#include <WiFi101.h>  
#include <WiFiUdp.h> 
#include <EasyNTPClient.h> 
#include <TimeLib.h>
#include <TimeAlarms.h>          
#include <ArduinoJson.h>         /* https://arduinojson.org/v6/assistant */

#include "secrets.h"
#include <Logging.h>
#include <LCDDisplay.h>
#include <VendSlot.h>
#include <Machine.h>             /* Switch from machine and mock machine in platformio.ini */

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2 
#define UDP_NTP_PORT 2390 

LcdDisplay l_display;
Machine machine(&l_display);

/* All things MQTT */
#ifndef MQTT_SSL
WiFiClient client;
Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);  
#else
WiFiSSLClient client;
Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 8883, AIO_USERNAME, AIO_KEY); 
#endif

Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/cmd-rx");
Adafruit_MQTT_Publish   aio_vend_count = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/vend-count");
Adafruit_MQTT_Publish   aio_int_temp   = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/int-tmp");
Adafruit_MQTT_Publish   aio_ext_temp   = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/ext-tmp");
Adafruit_MQTT_Publish   aio_watts      = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/feeds/watts");

WiFiUDP Udp;
EasyNTPClient ntpClient(Udp, "time.nist.gov"); 

void wifi_connect();
void MQTT_connect();
void MQTTProcessMessages(int timeout);
void iot_command(char *data);
String vend(DynamicJsonDocument doc);
String reset_display(DynamicJsonDocument doc);
void update_sensor_data();
void every_30_seconds();
void every_1_minute();
void post_telemetry();
time_t getTime();

void setup(void){
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  l_display.init();

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  wifi_connect();

  // Set up NTP
  l_display.clear();
  l_display.printAt(l_display.center("Syncing NTP Time"), 0, 0);
  for (int i = 0; i < 8; i++){ l_display.delay_with_animation(250, 1); }  /* Give the UDP port time to set up */
  time_t t = getTime();
  while (t == 0){
    for (int i = 0; i < 4; i++){ l_display.delay_with_animation(250, 1); }
    Serial.print(F("Waiting for NTP Sync: "));
    Serial.println((unsigned int)now());
    t = getTime();
  }
  setTime(t);
  setSyncInterval(1800);     /* NTP sync every 30 Minutes */
  setSyncProvider(getTime);
  l_display.clear();

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
  MQTT_connect();
  post_telemetry();
}

void loop(void){
  digitalWrite(LED_BUILTIN, HIGH); 
  MQTTProcessMessages(2000);
  digitalWrite(LED_BUILTIN, LOW);  
  Alarm.delay(250);
}



void MQTTProcessMessages(int timeout){
  MQTT_connect();
  Adafruit_MQTT_Subscribe *sub;
  while ((sub = mqtt.readSubscription(timeout))) {
    if (sub == &aio_command) {
      char * cmd_data = (char *)aio_command.lastread;
      DEBUG_PRINT(F("MQTTProcessMessages: cmd_data: "));
      DEBUG_PRINTLN(cmd_data);
      iot_command(cmd_data);
      
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
  l_display.display_network_info(2000);
  post_telemetry();
}

void iot_command(char *data){
  DEBUG_PRINT(F("iot_command: "));
  DEBUG_PRINTLN(data);
  
  // Parse the JSON document 
  // Example:
  //          {"name":"vend","id":"1","args":[2, "Charles", "Beer"]}
  //
  DynamicJsonDocument doc(200);
  DeserializationError err = deserializeJson(doc, data);
  if (err) {
    WARN_PRINT(F("iot_command:WARN:Parsing JSON command failed: "));
    WARN_PRINTLN(err.c_str());
    return;
  }

  const char* cmd_name   = doc["cmd"]; 
  const char* cmd_id     = doc["id"]; 
  String status_str = "ERROR,NO_SUCH_COMMAND";

  if (String(cmd_name) == String("vend")){
    status_str = vend(doc);
  }

  if (String(cmd_name) == String("reset_display")){
    status_str = reset_display(doc);
  }

  String rx = String(cmd_id) + "::" + status_str;
  char buff[rx.length() + 1];
  rx.toCharArray(buff, rx.length() +1);
  if (aio_command_rx.publish(buff)) {
    INFO_PRINT(F("iot_command:MQTT:Publish:cmd_rx: "));
    INFO_PRINTLN(buff);
  }else{
    ERROR_PRINT(F("iot_command:ERROR:MQTT:Publish:cmd_rx: Failed to post to cmd_rx. post_data: "));
    ERROR_PRINTLN(buff);
  }
}

String reset_display(DynamicJsonDocument doc){
  l_display.reset();
  return "OK";
}

String vend(DynamicJsonDocument doc){

  String vend_status_str = "OK";
  int cmd_slot           = doc["args"][0];
  const char* drinker    = doc["args"][1]; 
  const char* beer       = doc["args"][2];

  if (cmd_slot >= Machine::SLOT_COUNT && cmd_slot < 0 ){
    WARN_PRINTLN("vend: slot: " + String(cmd_slot) + " does not exists");
    return "ERROR,NO_SUCH_SLOT";
  }

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
    vend_status_str = "OK";
    l_display.finish_vend(beer, drinker, 4000);
  }
  l_display.display_default_status();
  
  if (aio_vend_count.publish((uint32_t)1)) {
    INFO_PRINTLN(F("vend:MQTT:Publish:vend-count: 1"));
  }else{
    WARN_PRINTLN(F("vend:WARN:MQTT:Publish:vend-count: 1"));
  }
  return vend_status_str;
}

void post_telemetry(){
    externalSensorData ed = machine.read_external();
    internalSensorData id = machine.read_internal();
    powerSensorData    pd = machine.read_power_usage();
  
    double e_temp = ed.FahrenheitTemp;
    if (aio_ext_temp.publish(e_temp, 4)) {
      INFO_PRINT(F("post_telemetry:MQTT:Publish:ext-tmp: "));
      INFO_PRINTLN(e_temp, 4);
    }else{
      WARN_PRINT(F("post_telemetry:WARN:MQTT:Publish:ext-tmp: "));
      WARN_PRINTLN(e_temp, 4);
    }

    double i_temp = id.FahrenheitTemp;
    if (aio_int_temp.publish(i_temp, 4)) {
      INFO_PRINT(F("post_telemetry:MQTT:Publish:int-tmp: "));
      INFO_PRINTLN(i_temp, 4);
    }else{
      WARN_PRINT(F("post_telemetry:WARN:MQTT:Publish:int-tmp: "));
      WARN_PRINTLN(i_temp, 4);
    }

    double watts  = pd.Watts;
    if (aio_watts.publish(watts, 4)) {
      INFO_PRINT(F("post_telemetry:MQTT:Publish:watts: "));
      INFO_PRINTLN(watts, 4);
    }else{
      WARN_PRINT(F("post_telemetry:WARN:MQTT:Publish:watts: "));
      WARN_PRINTLN(watts, 4);
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
  INFO_PRINT(F("Syncing Time from NTP: "));
  time_t t = ntpClient.getUnixTime();
  INFO_PRINTLN((unsigned long)t);
  return t;
}

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
     
     WARN_PRINT(F("MQTT Error: "));
     WARN_PRINTLN(mqtt.connectErrorString(ret));
     WARN_PRINTLN(F("Retrying MQTT connection in 5 seconds..."));
     for (int i = 0; i < 21; i++){  /* 5 Second Delay  with animation */
        l_display.delay_with_animation(250, 1);
     }
  }
  INFO_PRINTLN(F("MQTT Connected!"));
  l_display.set_adafruit_status(true);
  l_display.display_default_status();
  delay(2000);  /* Give the adafruit server time to respond */
}
