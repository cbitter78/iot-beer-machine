/*
  NOTE: I used the "Azure IoT Hub WiFi" to get started.




  Install these libraries:

    1. MKRNB
    2. ArduinoBearSSL
    3. ArduinoECCX08
    4. ArduinoMqttClient
    5. Arduino Cloud Provider Examples (You will need this to generate a private key.  It is not required for this source to run.)
*/

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <utility/ECCX08SelfSignedCert.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> 


///Enter your sensitive data in secrets.h
#include "secrets.h"
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
String     deviceId      = SECRET_DEVICE_ID;




WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);

  setupPrivateCertEncryption();

  // Set the client id used for MQTT as the device id
  mqttClient.setId(deviceId);

  // Set the username to "<broker>/<device id>/api-version=2018-06-30" and empty password
  String username;
  username += broker;
  username += "/";
  username += deviceId;
  username += "/api-version=2018-06-30";

  mqttClient.setUsernamePassword(username, "");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
}




void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();

    publishMessage();
  }
}







void publishMessage() {
  Serial.println("Publishing message");

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("devices/" + deviceId + "/messages/events/");
  mqttClient.print("hello ");
  mqttClient.print(millis());
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");


  String messageTopic = (String)mqttClient.messageTopic();
  messageTopic.toUpperCase();
  if (messageTopic.startsWith("$IOTHUB/METHODS/POST/")) { 
    onCommandReceived(messageTopic, messageSize);
  }

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();

}



void onCommandReceived(String messageTopic, int messageSize) {
  Serial.print("Command called "); 
  Serial.println(messageTopic);
  
  String msgId = messageTopic.substring(messageTopic.indexOf("$RID=") + 5);
  Serial.print("Message ID: ");
  Serial.println(msgId);

  // send message, the Print interface can be used to set the message contents
  String reply = "$iothub/methods/res/200/?$rid=" + msgId;

  Serial.print("Replying with : ");
  Serial.println(reply);
  mqttClient.beginMessage(reply);
  mqttClient.print("{ \"status\": 200, \"message\": \"ok\" }");
  mqttClient.endMessage();

}






/*
  Method to set up private key encryption and auth in the BearSSLClient.
  This is used to encrypt the wire and auth the device to Azure iot hub.

  This requires that the a self singed certificate is stored in slot 0,8
  in the ATECC508A or ATECC608A chip on board.  

  You can use this [sketch](https://github.com/arduino-libraries/ArduinoECCX08/blob/master/examples/Tools/ECCX08CSR/ECCX08CSR.ino)
  to create and store the private key.   Keep in mind this is PERMANENT!  Keep track of this key in a very secure 
  place if you need it later.

  Also see: https://create.arduino.cc/projecthub/Arduino_Genuino/securely-connecting-an-arduino-nb-1500-to-azure-iot-hub-af6470 
*/
void setupPrivateCertEncryption(){

  // Only configure encryption if ECCX08 exists.
  if (ECCX08.begin()) {

    // Reconstruct the self signed cert.  It is stored in Slot 0,8
    ECCX08SelfSignedCert.beginReconstruction(0, 8);
    ECCX08SelfSignedCert.setCommonName(ECCX08.serialNumber());
    ECCX08SelfSignedCert.endReconstruction();

    // Set a callback to get the current time used to validate the servers certificate
    ArduinoBearSSL.onGetTime(getTime);

    // Set the ECCX08 slot to use for the private key and the accompanying public certificate for it
    sslClient.setEccSlot(0, ECCX08SelfSignedCert.bytes(), ECCX08SelfSignedCert.length());
  }
}


/*
  Method to get the current epoc time.  This method defers
  the work to the wifi module.   If you have a real time clock (RTC)
  on board you could change this.
*/
unsigned long getTime() {
  return WiFi.getTime();  // get the current time from the WiFi module
}


/*
  Method to connect to Wifi.  
  
  Connects to the local wifi network using the SSID and PW set in secure.h 
  It will try every 2 seconds to connect then once connected pauses for 5 seconds 
  to allow for DNS, and other stuff to normalize.
*/
void connectWiFi() {
  Serial.println("Attempting to connect to : " + (String)ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.println("Connection Error: " + (String)WiFi.status() + "  Retrying in two seconds.");
    delay(2000);
  }
  Serial.println("Connected to the Wifi network.\nPausing for 5 seconds to allow for WiFi to normalize.\n\n");
  delay(5000);
}


/*
  Method to connect to Azure MQTT broker. 

  Tries to connect every 2 seconds.  Once connected it calls mqttSubscribe()
  to set up topic subscriptions.  
*/
void connectMQTT() {
  Serial.println("Attempting connection MQTT broker: " + (String)broker);
  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.println("Connection Error: " + (String)mqttClient.connectError() + "  Retrying in two seconds.");
    delay(2000);
  }
  Serial.println("Connected to the MQTT broker\n\n");
  mqttSubscribe();
}


/*
  Method to set up topic subscriptions.  This has to be done every 
  time the connection is made.  Add any oher topics to this method.
*/
void mqttSubscribe(){
  // subscribe to a topic
  mqttClient.subscribe("devices/" + deviceId + "/messages/devicebound/#");
  mqttClient.subscribe("$iothub/methods/POST/#");  // All methods come in on this topic.
}