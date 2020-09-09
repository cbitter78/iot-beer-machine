#define USE_WINC1500

#include "secrets.h"
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);


// Set up feeds.
AdafruitIO_Feed *command   = io.feed("iot-beer.command");          // Used to recieve commands
AdafruitIO_Feed *c_return  = io.feed("iot-beer.return");           // Used to reply to commands
AdafruitIO_Feed *msg       = io.feed("iot-beer.msg");              // Used to send general string messages
AdafruitIO_Feed *m_status  = io.feed("iot-beer.machine-status");   // Overall Machine Status (Ready, Vending, Open, Error)
AdafruitIO_Feed *vend_rate = io.feed("iot-beer.vend_rate");        // Reported per can vending 
AdafruitIO_Feed *in_temp   = io.feed("iot-beer.inner-temp");       // The temp inside the machine (next to the cans)
AdafruitIO_Feed *out_temp  = io.feed("iot-beer.outside-temp");     // The temp outside the machine 
AdafruitIO_Feed *out_quh   = io.feed("iot-beer.outside-quh");      // The Barometric Pressure outside the machine 
AdafruitIO_Feed *out_rh    = io.feed("iot-beer.outside-rh");       // Relative Humidity outside the machine
AdafruitIO_Feed *ac_amps   = io.feed("iot-beer.outside-ac-amps");  // Amps used by the AC Compressor



void init_feeds()
{
  AdafruitIO_Feed feeds [10] = {*command, *c_return, *msg, *m_status, *vend_rate, *in_temp, *out_temp, *out_quh, *out_rh, *ac_amps};

  int n;
  for ( n=0 ; n<10 ; ++n )
  {
    AdafruitIO_Feed f = feeds[n];
    if (f.exists() == false){
      Serial.print("Creating Feed:");
      Serial.println(f.name);
      f.create();
    }
    Serial.print("Setting Location on Feed:");
    Serial.println(f.name);
    f.setLocation(LOCATION_LAT, LOCATION_LONG, LOCATION_ALT);
  }

}



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
  command->onMessage(iotCommand);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  init_feeds();
  command->get();
  m_status->save("Ready");
  msg->save("Lets have a beer!");

}

void loop() {
  io.run();
}


void iotCommand(AdafruitIO_Data *data){
  Serial.print("received <- ");
  Serial.println(data->value());

  int v = data->toInt();
  if (v == 256){
    m_status->save("Vending");
    msg->save("About to vend a beer!");
    delay(3000);
    c_return->save(0);
    msg->save("Beer is ready for you to pick up.  Cheers!");
    m_status->save("Ready");
  }
}
