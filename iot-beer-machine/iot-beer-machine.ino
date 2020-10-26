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
Adafruit_ADS1115 a2d(0x48);      
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish   aio_command_rx = Adafruit_MQTT_Publish   (&mqtt, AIO_USERNAME "/feeds/iot-beer-command-rx");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe (&mqtt, AIO_USERNAME "/feeds/iot-beer-command");

int status = WL_IDLE_STATUS;

VendSlot* slots[2];
VendSlot slot1;
VendSlot slot2;

void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  mqtt.subscribe(&aio_command);
  
  a2d.begin();  
  lcd.init();
  lcd.backlight();


  slot1.setup(1, 12, &a2d, 0, &lcd, 0, 0);
  slot2.setup(2, 11, &a2d, 1, &lcd, 1, 0);

  slots[0] = &slot1;
  slots[1] = &slot2;

//  int i = 0;
//  for (i = 0; i < 2; i++){
//    (*slots[i]).vend();
//  }

}

void loop(void)
{
    MQTT_connect();
      
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
