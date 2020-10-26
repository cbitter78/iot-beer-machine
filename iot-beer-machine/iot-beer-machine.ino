#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include "VendSlot.h"

//#include <ArduinoJson.h>  //https://arduinojson.org/v6/assistant/

Adafruit_ADS1115 a2d(0x48);      
LiquidCrystal_I2C lcd(0x27,20,4); 
 
#define BTN_1 A0
#define BTN_2 A1 

VendSlot slot1;
VendSlot slot2;





 
void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  
  a2d.begin();  
  lcd.init();
  lcd.backlight();

  Serial.println("About to set up Slots");
  slot1.setup(1, 12, &a2d, 0, &lcd, 0, 0);
  slot2.setup(2, 11, &a2d, 1, &lcd, 1, 0);

  slot1.vend();
  slot2.vend();

}

void loop(void)
{
    print_adc(a2d, lcd);
    flash();
    delay(250);
}


void print_adc(Adafruit_ADS1115 b, LiquidCrystal_I2C lcd){
  for (int i = 0; i <= 1; i++) {
    int v = b.readADC_SingleEnded(i);
    lcd.setCursor(0,i+2);
    lcd.print(i);
    lcd.print(": ");
    lcd.print(v);
    Serial.print(i);
    Serial.print(": ");
    Serial.println(v);
  }

}

void flash(){
  digitalWrite(13, HIGH);
  delay(250);
  digitalWrite(13, LOW);
}
