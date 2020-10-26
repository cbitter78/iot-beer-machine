#define USE_WINC1500
#define AIO_DEBUG
#define AIO_ERROR


#include "secrets.h"
//#include "util.h"
#include "AdafruitIO_WiFi.h"
#include <ArduinoJson.h>

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up feeds.
AdafruitIO_Feed *command    = io.feed("iot-beer-command");          // Used to recieve commands
AdafruitIO_Feed *command_rx = io.feed("iot-beer-command-rx");       // Used to to publish commands results
AdafruitIO_Feed *msg        = io.feed("iot-beer-msg");              // Used to send general string messages
AdafruitIO_Feed *m_status   = io.feed("iot-beer-status");           // Overall Machine Status (Ready, Vending, Open, Error)
AdafruitIO_Feed *vend_rate  = io.feed("iot-beer-vend_rate");        // Reported per can vending 
AdafruitIO_Feed *in_temp    = io.feed("iot-beer-inner-temp");       // The temp inside the machine (next to the cans)
AdafruitIO_Feed *out_temp   = io.feed("iot-beer-outside-temp");     // The temp outside the machine 
AdafruitIO_Feed *out_quh    = io.feed("iot-beer-outside-quh");      // The Barometric Pressure outside the machine 
AdafruitIO_Feed *out_rh     = io.feed("iot-beer-outside-rh");       // Relative Humidity outside the machine
AdafruitIO_Feed *ac_amps    = io.feed("iot-beer-outside-ac-amps");  // Amps used by the AC Compressor

void setup() {
  Serial.begin(115200);
  while(! Serial);
  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();
  printWifiStatus();
  //while(io.status() < AIO_CONNECTED) {
    Serial.println(io.statusText());

    Serial.print("networkStatus(): ");
    Serial.println(io.networkStatus());
    Serial.print("connectionType(): ");
    Serial.println(io.connectionType());
    
  //  delay(500);
  //}
  
  Serial.println();
  Serial.println(io.statusText());

  command->onMessage(iotCommand);
  command->get();
  
  m_status->save("Ready");
  m("Beer Machine Ready...Lets have a beer!");
  report_success(3001);
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void loop() {
  io.run();
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}


void iotCommand(AdafruitIO_Data *data){
  Serial.print("received <- ");
  Serial.println(data->value());

  DynamicJsonDocument doc(2048);



  // Parse the JSON document from the HTTP response
  DeserializationError err = deserializeJson(doc, data->value());
  if (err) {
    Serial.print(F("Parsing command failed: "));
    Serial.println(err.c_str());
    return;
  }

  if (doc["device"] != DEVICE_ID){
    Serial.println("Command for differnt device");
    return;
  }

  if(doc["cmd"]["name"] == "vend"){
    vend_slot(doc["cmd"]["args"][0], doc["cmd"]["id"]); 
    return;
  }

  report_error("Unknown Command", 404, doc["cmd"]["id"]);
}



void report_success(int command_id){
  //  Serial.println("Success!!!");
  //  DynamicJsonDocument doc(125);
  //  JsonObject cmd = doc.createNestedObject("command");
  //  doc["device"] = DEVICE_ID;
  //  cmd["id"]     = command_id;
  //  cmd["code"]   = 0;
  //  
  //  String json;
  //  serializeJson(doc, json);
  //  command_rx->save("this is a test " + String(command_id));
  //  command_rx->save(String(json));
  //  Serial.print("report_success: ");
  //  Serial.println(json);

  // Return the data in CSV format.  Device, Command ID, Return Code, Error.   
  // This is because for some reson Adafruit.io is ignoring the json I send.
  
  
  command_rx->save(String(DEVICE_ID) + "," + String(command_id) + ",0");
}

void report_error(String error, int error_code, int command_id){
  //  DynamicJsonDocument doc(250);
  //  JsonObject cmd = doc.createNestedObject("command");
  //  doc["device"]  = DEVICE_ID;
  //  cmd["id"]      = command_id;
  //  cmd["code"]    = error_code;
  //  cmd["error"]   = error;
  //  
  //  String json;
  //  serializeJson(doc, json);
  //  command_rx->save(json);
  //  Serial.print("report_error: ");
  //  Serial.println(json);
  //  Serial.println(json);

  // Return the data in CSV format.  Device, Command ID, Return Code, Error.   
  // This is because for some reson Adafruit.io is ignoring the json I send.
 
  
  command_rx->save(String(DEVICE_ID) + "," + String(command_id) + "," + String(error_code) + "," + error);
}




void vend_slot(int slot, int cmd_id){
  m_status->save("Vending");
  m("Vending slot:" + String(slot));
  delay(10000);
  m("Beer from " + String(slot) + " is ready for you to pick up.  Cheers!");
  m_status->save("Ready");
  Serial.println("ok we are done vending the beer");
  report_success(cmd_id);
}
  
void m(String s){
  Serial.println(s);
}
