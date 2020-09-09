#define USE_WINC1500

#include "secrets.h"
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);


// Set up feeds.
AdafruitIO_Feed *command   = io.feed("beer-machine.command");          // Used to recieve commands
AdafruitIO_Feed *c_return  = io.feed("beer-machine.return");           // Used to reply to commands
AdafruitIO_Feed *msg       = io.feed("beer-machine.msg");              // Used to send general string messages
AdafruitIO_Feed *status    = io.feed("beer-machine.machine-status");   // Overall Machine Status (Ready, Vending, Open, Error)
AdafruitIO_Feed *vend_rate = io.feed("beer-machine.vend_rate");        // Reported per can vending 
AdafruitIO_Feed *in_temp   = io.feed("beer-machine.inner-temp");       // The temp inside the machine (next to the cans)
AdafruitIO_Feed *out_temp  = io.feed("beer-machine.outside-temp");     // The temp outside the machine 
AdafruitIO_Feed *out_quh   = io.feed("beer-machine.outside-quh");      // The Barometric Pressure outside the machine 
AdafruitIO_Feed *out_rh    = io.feed("beer-machine.outside-rh");       // Relative Humidity outside the machine
AdafruitIO_Feed *ac_amps   = io.feed("beer-machine.outside-ac-amps");  // Amps used by the AC Compressor

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  command->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  command->get();

}

void loop() {
  io.run();
}


void iotCommand(AdafruitIO_Data *data){
  Serial.print("received <- ");
  Serial.println(data->value());

  int v = data->toInt();
  if (v == 256){
    status->save("Vending")
    msg->save("About to vend a beer!")
    delay(3000);
    c_return->save(0);
    msg->save("Beer is ready for you to pick up.  Cheers!")
    status->save("Ready")
  }
}
