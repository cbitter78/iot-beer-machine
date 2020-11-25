#include "lcd_display.h"

/*  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9
0   W     S  :  _     S  :  _     External T
1   A     S  :  _     S  :  _     External H
2         S  :  _     S  :  _     External P
3   B  E  E  R  :  Internal Temp                          Watts
*/


LcdDisplay::LcdDisplay(){
    _lcd = NULL;
    _adafruit_status = false;
    _internal_temp = 0.0F;
    _external_temp = 0.0F;
    _external_inHg = 0.0F;
    _external_humidity = 0.0F;
    _amps = 0.0F;
    _watts = 0.0F;
    //_slot_status = {0,0,0,0,0,0};
}

void LcdDisplay::init(LiquidCrystal_I2C *lcd){
    _lcd = lcd;
    LiquidCrystal_I2C l = *_lcd;  

    // See https://maxpromer.github.io/LCD-Character-Creator/ to make custom LCD characters
    byte wait0[]                = {B00100,B10101,B01110,B00100,B00000,B00000,B00000,B00000};
    byte wait1[]                = {B00100,B00100,B00100,B10101,B01110,B00100,B00000,B00000};
    byte wait2[]                = {B00100,B00100,B00100,B00100,B00100,B10101,B01110,B00100};
    byte smile[]                = {B00000,B01010,B00000,B00100,B10001,B01110,B00000,B00000};
    byte frown[]                = {B00000,B01010,B00000,B00100,B00000,B01110,B10001,B00000};
    byte skull[]                = {B00000,B01110,B10101,B11011,B01110,B01110,B00000,B00000};
    byte wifiConnected[]        = {B00001,B00001,B00011,B00111,B00111,B01111,B01111,B11111};
    byte wifiConnected1[]       = {B11111,B11111,B01110,B00100,B00100,B00100,B00100,B00100};
    byte adaFruitConnected[]    = {B00000,B00000,B01010,B11111,B11111,B01110,B00100,B00000};
    byte adaFruitNotConnected[] = {B00000,B00000,B01010,B10101,B10001,B01010,B00100,B00000};
    l.createChar(LCD_WAIT1, wait0);
    l.createChar(LCD_WAIT2, wait1);
    l.createChar(LCD_WAIT3, wait2);
    l.createChar(LCD_WIFI_CONNECTED, wifiConnected1);
    l.createChar(LCD_AIO_CONNECTED, adaFruitConnected);
    l.createChar(LCD_AIO_DISCONNECTED, adaFruitNotConnected);
    l.createChar(LCD_OK, smile);
    l.createChar(LCD_ERROR, frown);

    printAt("initializing...", 0,0);
    for (int i = 0; i < 8; i++){
      writeAt(i,i,2);
      delay(500);
    }
    clear();
}

void LcdDisplay::repaint(){
    DEBUG_PRINT(F("LcdDisplay::repaint: Re-Painting the LCD Screen..."));
    set_wifi_status();
    set_adafruit_status(_adafruit_status);
}


void LcdDisplay::set_wifi_status(){
    int col = 0;
    int row = 0;
    if (WiFi.status() == WL_CONNECTED){
        DEBUG_PRINT(F("LcdDisplay::set_wifi_status: Connected"));
        writeAt(LCD_WIFI_CONNECTED, col, row);
    }else{
        DEBUG_PRINT(F("LcdDisplay::set_wifi_status: NOT! Connected"));
        writeAt(LCD_ERROR, col, row);
    }
}


void LcdDisplay::set_adafruit_status(bool s){
    _adafruit_status = s;
    int col = 0;
    int row = 1;
    if (s){
        DEBUG_PRINT(F("LcdDisplay::set_adafruit_status: Connected"));
        writeAt(LCD_AIO_CONNECTED, col, row);
    }else{
        DEBUG_PRINT(F("LcdDisplay::set_adafruit_status: NOT! Connected"));
        writeAt(LCD_AIO_DISCONNECTED, col, row);
    }
}

void LcdDisplay::backlight_on(){
    (*_lcd).backlight();
}

void LcdDisplay::backlight_off(){
    (*_lcd).noBacklight();
}

void LcdDisplay::writeAt(uint8_t v, uint8_t col, uint8_t row){
    DEBUG_PRINT("LcdDisplay::writeAt(v:");
    DEBUG_PRINT(v);
    DEBUG_PRINT(", col:");
    DEBUG_PRINT(col);
    DEBUG_PRINT(", row:");
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(")");
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.write(v);
}

void LcdDisplay::printAt(const char c[], uint8_t col, uint8_t row){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.printstr(c);
}

void LcdDisplay::clear(){
    (*_lcd).clear();
}
