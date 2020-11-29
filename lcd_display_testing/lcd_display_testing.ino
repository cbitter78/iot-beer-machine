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

  wifi_connect();


  
  int d = 200;
  l_display.display_default_status();
  l_display.set_wifi_status();
  delay(d);
  l_display.set_adafruit_status(true);
  delay(d);
  l_display.set_internal_temp(33.45774F);
  delay(d);
  l_display.set_external_temp(72.12558F);
  delay(d);
  l_display.set_external_humidity(95.17544F);
  delay(d);
  l_display.set_external_inHg(32.81447F);
  delay(d);
  l_display.set_amps(4.25218F);
  delay(d);
  l_display.set_watts(467.73908F);
  delay(d);
  for (int i = 0; i < 6; i++){
     l_display.set_slot_status(i, 0);
     delay(d);
  }
  l_display.set_slot_status(4, 10);
  l_display.set_slot_status(2, 50);
}



void loop(){
  flash(13);
  wifi_connect();
  delay(500);
  if (analogRead(0) > 100){
    vend(0);
  }

  if (analogRead(1) > 100){
    l_display.display_network_info(3000);
  }
}

void vend(int slot){
  INFO_PRINT(F("Vending Slot "));
  INFO_PRINTLN(slot);
  l_display.start_vend(slot, "Ballast Point Sculpin IPA");
  for(int i = 0; i < 250; i++){
    l_display.vend_animation(50);  
  }
  l_display.finish_vend("Ballast Point Sculpin IPA", "Charles", 4000);
  l_display.display_default_status();
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
    l_display.scroll_msg(String(" Connecting to SSID    ") + String(WIFI_SSID), 100, -1);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  
    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
      flash(13);
      l_display.print('.');
    }
    l_display.scroll_msg("Wifi Connected", 100, -1);
    delay(1000);
    l_display.display_network_info(3000);
  }
  l_display.set_wifi_status();
  l_display.set_adafruit_status(true);
}
