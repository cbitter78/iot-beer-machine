#include "lcd_display.h"

LcdDisplay::LcdDisplay(){
    _lcd = NULL;
}

void LcdDisplay::init(bool back_light){

    LiquidCrystal_I2C lcd(0x27,20,4); 
    lcd.init();
    if (back_light){
        lcd.backlight();
    }
    _lcd = &lcd;
    _back_light_on = back_light;




    byte wait0[]                = {B00100,B10101,B01110,B00100,B00000,B00000,B00000,B00000};
    byte wait1[]                = {B00100,B00100,B00100,B10101,B01110,B00100,B00000,B00000};
    byte wait2[]                = {B00100,B00100,B00100,B00100,B00100,B10101,B01110,B00100};
    byte smile[]                = {B00000,B01010,B00000,B00100,B10001,B01110,B00000,B00000};
    byte frown[]                = {B00000,B01010,B00000,B00100,B00000,B01110,B10001,B00000};
    byte skull[]                = {B00000,B01110,B10101,B11011,B01110,B01110,B00000,B00000};
    byte wifiConnected[]        = {B11111,B11111,B01110,B00100,B00100,B00100,B00100,B00100};
    byte adaFruitConnected[]    = {B00000,B00000,B01010,B11111,B11111,B01110,B00100,B00000};
    byte adaFruitNotConnected[] = {B00000,B00000,B01010,B10101,B10001,B01010,B00100,B00000};

    lcd.createChar(0, wait0);
    lcd.createChar(1, wait1);
    lcd.createChar(2, wait2);
    lcd.createChar(C_WIFI_CONNECTED, wifiConnected);
    lcd.createChar(4, adaFruitConnected);
    lcd.createChar(5, adaFruitNotConnected);
    lcd.createChar(6, smile);
    lcd.createChar(C_ERROR, frown);

    printAt("initializing...", 0,0);
    for (int i = 0; i < 8; i++){
      writeAt(i,i,2);
      delay(500);
    }
    delay(2000);
    reset();
//    printAt("testing", 0,3);
//    writeAt(3, 0,2);


}

void LcdDisplay::set_wifi_status(uint8_t wifi_status){
    if (wifi_status == WL_CONNECTED){
        Serial.println("Dsiplay: Wifi Connected");
        writeAt(C_WIFI_CONNECTED,2,2);
    }else{
        Serial.println("Dsiplay: Wifi NOT! Connected");
        writeAt(C_ERROR,2,2);
    }
}

void LcdDisplay::backlight_on(){
    (*_lcd).backlight();
}

void LcdDisplay::backlight_off(){
    (*_lcd).noBacklight();
}

void LcdDisplay::writeAt(uint8_t value, uint8_t col, uint8_t row){
    Serial.print("LcdDisplay::writeAt(value:");
    Serial.print(value);
    Serial.print(", col:");
    Serial.print(col);
    Serial.print(", row:");
    Serial.print(row);
    Serial.println(")");
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.write(value);
}

void LcdDisplay::printAt(const char c[], uint8_t col, uint8_t row){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.printstr(c);
}

void LcdDisplay::clear(){
    (*_lcd).clear();
}

void LcdDisplay::reset(){
    LiquidCrystal_I2C l = *_lcd;
    l.init();
    if (_back_light_on){
        l.backlight();
    }
}
