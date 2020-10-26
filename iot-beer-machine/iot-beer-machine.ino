#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include "VendSlot.h"

#include "util.h"

//#include <ArduinoJson.h>  //https://arduinojson.org/v6/assistant/

Adafruit_ADS1115 a2d(0x48);      
LiquidCrystal_I2C lcd(0x27,20,4); 
VendSlot* slots[2];
VendSlot slot1;
VendSlot slot2;

void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  
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
    if (analogRead(A0) > 100){
      (*slots[0]).vend();
    }

    if (analogRead(A1) > 100){
      (*slots[1]).vend();
    }
  
    print_adc(a2d, lcd);
    flash();
    delay(250);
}
