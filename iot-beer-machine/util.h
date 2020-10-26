#include <Adafruit_ADS1015.h>

void print_adc(Adafruit_ADS1115 b, LiquidCrystal_I2C lcd){
  int x = 3;
  int ports[x + 1];
  
  for (int i = 0; i <= x; i++) {
    int v = b.readADC_SingleEnded(i);
    if (v == 65535){ v = 0; }
    ports[i] = v;
  }

  lcd.setCursor(0,2);
  lcd.print("                   ");
  lcd.setCursor(0,3);
  lcd.print("                   ");
  
  lcd.setCursor(0,2);
  lcd.print("0:" + String(ports[0]));
  Serial.println("ADC:0:" + String(ports[0]));
  
  lcd.setCursor(10,2);
  lcd.print("1:" + String(ports[1]));
  Serial.println("ADC:1:" + String(ports[1]));

  lcd.setCursor(0,3);
  lcd.print("2:" + String(ports[2]));
  Serial.println("ADC:2:" + String(ports[2]));

  lcd.setCursor(10,3);
  lcd.print("3:" + String(ports[3]));
  Serial.println("ADC:" + String(ports[3]));
}

void flash(){
  digitalWrite(13, HIGH);
  delay(250);
  digitalWrite(13, LOW);
}
