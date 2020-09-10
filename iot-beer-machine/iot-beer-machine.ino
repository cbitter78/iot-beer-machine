#define USE_WINC1500

#include "secrets.h"
#include "util.h"
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up feeds.
AdafruitIO_Feed *command   = io.feed("iot-beer-command");          // Used to recieve commands
AdafruitIO_Feed *msg       = io.feed("iot-beer-msg");              // Used to send general string messages
AdafruitIO_Feed *m_status  = io.feed("iot-beer-status");           // Overall Machine Status (Ready, Vending, Open, Error)
AdafruitIO_Feed *vend_rate = io.feed("iot-beer-vend_rate");        // Reported per can vending 
AdafruitIO_Feed *in_temp   = io.feed("iot-beer-inner-temp");       // The temp inside the machine (next to the cans)
AdafruitIO_Feed *out_temp  = io.feed("iot-beer-outside-temp");     // The temp outside the machine 
AdafruitIO_Feed *out_quh   = io.feed("iot-beer-outside-quh");      // The Barometric Pressure outside the machine 
AdafruitIO_Feed *out_rh    = io.feed("iot-beer-outside-rh");       // Relative Humidity outside the machine
AdafruitIO_Feed *ac_amps   = io.feed("iot-beer-outside-ac-amps");  // Amps used by the AC Compressor


void setup() {
  Serial.begin(115200);
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println(io.statusText());

  command->onMessage(iotCommand);
  //command->get();
  
  m_status->save("Ready");
  m("Beer Machine Ready...Lets have a beer!");
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

  int v = data->toInt();
  if (v > 0){
    switch(v) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
         vend_slot(v);
         command->save("R:" + String(v) + ":Success");
         break;
      default :
         Serial.print("Unkinown command id: ");
         Serial.println(v);

         command->save("R:" + String(v) + ":Not Found"); 
         msg->save("I did not understand the command you sent.");    
    }
  }
}



void vend_slot(int slot){
  m_status->save("Vending");
  m("Vending slot:" + String(slot));
  delay(10000);
  m("Beer from " + String(slot) + " is ready for you to pick up.  Cheers!");
  m_status->save("Ready");
}
  
void m(String s){
  Serial.println(s);
  msg->save(s);
}
