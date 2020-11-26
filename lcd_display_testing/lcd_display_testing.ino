#include "lcd_display.h"
#include "secrets.h"
#include "logging.h"
#include <WiFi101.h>

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2  

LiquidCrystal_I2C lcd(0x27,20,4); 
LcdDisplay l_display;

void setup(void)
{
  Serial.begin(115200);
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  lcd.init();
  lcd.backlight();
  l_display.init(&lcd);

  //l_display.repaint();
  wifi_connect();
  l_display.set_slot_status(1, 0);
  l_display.set_slot_status(2, 0);
  l_display.set_slot_status(3, 0);
  l_display.set_slot_status(5, 0);
  l_display.set_slot_status(4, 0);
  l_display.disply_msg("I love to display messages!! Becuse its lots and lots of fun!", 5000);
  l_display.scrool_msg("The quick brown fox jumpped over the lazy log...The quick brown fox jumpped over the lazy log...The quick brown fox jumpped over the lazy log...", 50, 5000);
}



void loop(){
 
  flash(13);
  wifi_connect();
  delay(5000);
  DEBUG_PRINTLN("Not doing much...");
  l_display.scrool_msg("The quick brown fox jumpped over the lazy log...The quick brown fox jumpped over the lazy log...The quick brown fox jumpped over the lazy log...", 50, 5000);


}

void flash(int pin){
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
}


void wifi_connect(){
  while (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINT(F("Attempting to connect to SSID: "));
    DEBUG_PRINTLN(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  
    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }
  l_display.set_wifi_status();
  l_display.set_adafruit_status(true);
}
