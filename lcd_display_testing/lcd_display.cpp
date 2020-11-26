#include "lcd_display.h"

/*  
For Character Map see https://www.sparkfun.com/datasheets/LCD/HD44780.pdf page 16.  Page 17 does not apply.

*/


LcdDisplay::LcdDisplay(){
    _lcd = NULL;
    _adafruit_status   = false;
    _internal_temp     = 33.45774F;
    _external_temp     = 72.12558F;
    _external_inHg     = 32.81447F;
    _external_humidity = 95.17544F;
    _amps              =  4.25218F;
    _watts             =467.73908F;
    for (int i = 0; i < 7; i++){
        _slot_status[i] = -1;
    }

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
    byte adaFruitConnected[]    = {B00000,B00000,B01010,B11111,B11111,B01110,B00100,B00000};
    byte adaFruitNotConnected[] = {B00000,B00000,B01010,B10101,B10001,B01010,B00100,B00000};
    l.createChar(LCD_CUSTOM_CHAR_WAIT1, wait0);
    l.createChar(LCD_CUSTOM_CHAR_WAIT2, wait1);
    l.createChar(LCD_CUSTOM_CHAR_WAIT3, wait2);
    l.createChar(LCD_CUSTOM_CHAR_VERRY_BAD, skull);
    l.createChar(LCD_CUSTOM_CHAR_AIO_CONNECTED, adaFruitConnected);
    l.createChar(LCD_CUSTOM_CHAR_AIO_DISCONNECTED, adaFruitNotConnected);
    l.createChar(LCD_CUSTOM_CHAR_OK, smile);
    l.createChar(LCD_CUSTOM_CHAR_ERROR, frown);
}

void LcdDisplay::repaint(){
    DEBUG_PRINTLN(F("LcdDisplay::repaint: Re-Painting the LCD Screen..."));
    clear();
    set_wifi_status();
    set_adafruit_status(_adafruit_status);
    set_internal_temp(_internal_temp);
    set_external_temp(_external_temp);
    set_external_humidity(_external_humidity);
    set_external_inHg(_external_inHg);
    set_amps(_amps);
    set_watts(_watts);
    set_watts(_watts);
    for (int i = 0; i < 6; i++){
       set_slot_status(i, _slot_status[i]);
    }
}



void LcdDisplay::set_slot_status(int slot, int slot_status){
    if (slot < 0 || slot > 5) { 
      DEBUG_PRINT(F("SLOT OUT OF RANGE!  slot: "));
      DEBUG_PRINTLN(slot);
      return; 
    } 
    
    _slot_status[slot] = slot_status;
    int col     = SLOT_COLUMNS[slot];
    int row     = SLOT_ROWS[slot];
    char c_slot = SLOT_NAMES[slot];
    DEBUG_PRINT(F("LcdDisplay::set_slot_status: (slot: "));
    DEBUG_PRINT(slot);
    DEBUG_PRINT(F(", slot_status: "));
    DEBUG_PRINT(slot_status);
    DEBUG_PRINT(F(", col: "));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row: "));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    
    printAt(c_slot, col, row);
    print(':');
    if (slot_status == 0){
        write(LCD_CUSTOM_CHAR_OK);
    }else
    {
        write(LCD_CUSTOM_CHAR_VERRY_BAD);
    }
}


void LcdDisplay::set_watts(float watts){
    _watts = watts;
    DEBUG_PRINT(F("LcdDisplay::set_watts: "));
    DEBUG_PRINTLN(_watts, 1);
    printAt(_watts, 1, 6, 3);
    print('w');
}

void LcdDisplay::set_amps(float amps){
    _amps = amps;
    DEBUG_PRINT(F("LcdDisplay::set_amps: "));
    DEBUG_PRINTLN(_amps, 1);
    printAt(_amps, 1, 0, 3);
    print('a');
}

void LcdDisplay::set_internal_temp(float temp){
    _internal_temp = temp;
    DEBUG_PRINT(F("LcdDisplay::set_internal_temp: "));
    DEBUG_PRINTLN(_internal_temp, 1);
    printAt(_internal_temp, 2, 14, 0);
    print(LCD_CHAR_DEGREE);
}


void LcdDisplay::set_external_temp(float temp){
    _external_temp = temp;
    DEBUG_PRINT(F("LcdDisplay::set_external_temp: "));
    DEBUG_PRINTLN(_external_temp, 1);
    printAt(_external_temp, 2, 14, 1);
    print(LCD_CHAR_DEGREE);
}

void LcdDisplay::set_external_humidity(float humidity){
    _external_humidity = humidity;
    DEBUG_PRINT(F("LcdDisplay::set_external_humidity: "));
    DEBUG_PRINTLN(_external_humidity, 1);
    printAt(_external_humidity, 2, 14, 2);
    print('%');
}

void LcdDisplay::set_external_inHg(float inHg){
    _external_inHg = inHg;
    DEBUG_PRINT(F("LcdDisplay::set_external_inHg: "));
    DEBUG_PRINTLN(_external_inHg, 1);
    printAt(_external_inHg, 1, 14, 3);
    print("Hg");
}

void LcdDisplay::set_wifi_status(){
    int col = 0;
    int row = 0;
    if (WiFi.status() == WL_CONNECTED){
        DEBUG_PRINT(F("LcdDisplay::set_wifi_status: Connected"));
        printAt(LCD_CHAR_WIFI_CONNECTED, col, row);
    }else{
        DEBUG_PRINT(F("LcdDisplay::set_wifi_status: NOT! Connected"));
        printAt(LCD_CHAR_WIFI_NOT_CONNECTED, col, row);
    }
}

void LcdDisplay::set_adafruit_status(bool s){
    _adafruit_status = s;
    int col = 0;
    int row = 1;
    if (s){
        DEBUG_PRINT(F("LcdDisplay::set_adafruit_status: Connected"));
        writeAt(LCD_CUSTOM_CHAR_AIO_CONNECTED, col, row);
    }else{
        DEBUG_PRINT(F("LcdDisplay::set_adafruit_status: NOT! Connected"));
        writeAt(LCD_CUSTOM_CHAR_AIO_DISCONNECTED, col, row);
    }
}

void LcdDisplay::backlight_on(){
    DEBUG_PRINTLN("LcdDisplay::backlight_on()");
    LiquidCrystal_I2C l = *_lcd;
    l.backlight();
}

void LcdDisplay::backlight_off(){
    DEBUG_PRINTLN("LcdDisplay::backlight_off()");
    LiquidCrystal_I2C l = *_lcd;
    l.noBacklight();
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

void LcdDisplay::printAt(float f, int accuracy, uint8_t col, uint8_t row){
    DEBUG_PRINT("LcdDisplay::printAt(f:");
    DEBUG_PRINT(f, accuracy);
    DEBUG_PRINT(", col:");
    DEBUG_PRINT(col);
    DEBUG_PRINT(", row:");
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(")");
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.print(f, accuracy);
}

void LcdDisplay::printAt(const char c[], uint8_t col, uint8_t row){
    DEBUG_PRINT("LcdDisplay::printAt(c:");
    DEBUG_PRINT(c);
    DEBUG_PRINT(", col:");
    DEBUG_PRINT(col);
    DEBUG_PRINT(", row:");
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(")");
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.printstr(c);
}


void LcdDisplay::printAt(char c, uint8_t col, uint8_t row){
    DEBUG_PRINT("LcdDisplay::printAt(c:");
    DEBUG_PRINT(c);
    DEBUG_PRINT(", col:");
    DEBUG_PRINT(col);
    DEBUG_PRINT(", row:");
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(")");
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(col, row);
    l.print(c);
}

void LcdDisplay::write(uint8_t value){
    LiquidCrystal_I2C l = *_lcd;
    l.write(value);
}

void LcdDisplay::print(char c){
    LiquidCrystal_I2C l = *_lcd;
    l.print(c);
}

void LcdDisplay::print(const char c[]){
    LiquidCrystal_I2C l = *_lcd;
    l.print(c);
}

void LcdDisplay::clear(){
    DEBUG_PRINTLN("LcdDisplay::clear()");
    LiquidCrystal_I2C l = *_lcd;
    l.clear();
}
