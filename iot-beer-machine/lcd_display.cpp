#include "lcd_display.h"
#include "logging.h"

#include <WiFi101.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*  
  Class to manage the display for the iot beer machine.
  For Character Map see https://www.sparkfun.com/datasheets/LCD/HD44780.pdf page 16.  Page 17 does not apply.
*/


LiquidCrystal_I2C l(0x27,20,4);

LcdDisplay::LcdDisplay(){
    _adafruit_status     = false;
    _slot_animation_col  = 0;
    _slot_animation_skip = 0;
    _internal_temp       = 0.0F;
    _external_temp       = 0.0F;;
    _external_inHg       = 0.0F;;
    _external_humidity   = 0.0F;;
    _amps                = 0.0F;;
    _watts               = 0.0F;;
    for (int i = 0; i < 7; i++){
        _slot_status[i] = -1;
    }
}


void LcdDisplay::init(){
    l.init();
    l.backlight();

    // See https://maxpromer.github.io/LCD-Character-Creator/ to make custom LCD characters
    byte pack_man_open[]        = {B00000,B00111,B01110,B01100,B01110,B00111,B00000,B00000};
    byte pack_man_closed[]      = {B00000,B01110,B11111,B11000,B11111,B01110,B00000,B00000};
    byte pack_man_goast[]       = {B00000,B01110,B11111,B10101,B11111,B11111,B10101,B00000};
    byte smile[]                = {B00000,B01010,B00000,B00100,B10001,B01110,B00000,B00000};
    byte frown[]                = {B00000,B01010,B00000,B00100,B00000,B01110,B10001,B00000};
    byte skull[]                = {B00000,B01110,B10101,B11011,B01110,B01110,B00000,B00000};
    byte adaFruitConnected[]    = {B00000,B00000,B01010,B11111,B11111,B01110,B00100,B00000};
    byte adaFruitNotConnected[] = {B00000,B00000,B01010,B10101,B10001,B01010,B00100,B00000};
    l.createChar(LCD_CUSTOM_CHAR_PACK_MAN_OPEN,     pack_man_open);
    l.createChar(LCD_CUSTOM_CHAR_PACK_MAN_CLOSED,   pack_man_closed);
    l.createChar(LCD_CUSTOM_CHAR_PACK_MAN_GOAST,    pack_man_goast);
    l.createChar(LCD_CUSTOM_CHAR_VERRY_BAD,         skull);
    l.createChar(LCD_CUSTOM_CHAR_AIO_CONNECTED,     adaFruitConnected);
    l.createChar(LCD_CUSTOM_CHAR_AIO_NOT_CONNECTED, adaFruitNotConnected);
    l.createChar(LCD_CUSTOM_CHAR_OK,                smile);
    l.createChar(LCD_CUSTOM_CHAR_ERROR,             frown);
}


void LcdDisplay::display_default_status(){
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
    for (int i = 0; i < 6; i++){
       set_slot_status(i, _slot_status[i]);
    }
}


void LcdDisplay::display_network_info(int delay_then_display_default){
  DEBUG_PRINTLN(F("LcdDisplay::display_network_info:"));
  clear();

  if (WiFi.status() != WL_CONNECTED){
    printAt("WIFI Disconnected", 0, 0);
    printAt("Status:", 0, 2);
    printAt(_wifi_status_string(WiFi.status()), 1,3); 
    print(F(":"));
    print(WiFi.status());   
    delay(delay_then_display_default);
    display_default_status();
    return;
  }
  
  printAt("dBm: ", 11, 1);
  print((int)WiFi.RSSI());

  _abvPrintIpAt(0,0,  "IP: " , WiFi.localIP());
  _abvPrintIpAt(0,1,  "NM: " , WiFi.subnetMask());
  _abvPrintIpAt(11,0, "GW:  ", WiFi.gatewayIP());
  

  byte mac[6];
  WiFi.macAddress(mac);
  _printMacAt(0, 2, "L: ", mac); /* Local Mac Address */
  WiFi.BSSID(mac);
  _printMacAt(0, 3, "R: ", mac); /* Remote (Gateway) Mac Address */
  
  delay(delay_then_display_default);
  display_default_status();
}


void LcdDisplay::start_vend(int slot, const char beer[]){
  DEBUG_PRINT(F("LcdDisplay::start_vend:("));
  DEBUG_PRINT(F("slot: "));
  DEBUG_PRINT(slot);
  DEBUG_PRINT(F(", beer: "));
  DEBUG_PRINT(beer);
  DEBUG_PRINTLN(F(")"));

  l.clear();
  l.home();
  String slot_name = String("Slot #") + String(SLOT_NAMES[slot]);
  l.print(center(slot_name));
  l.setCursor(0, 1);
  l.print(center("Is Vending a Can Of"));
  l.setCursor(0, 2);
  l.print(center(beer));
  l.setCursor(0, 3);
  _slot_animation_col = 0;
}

void LcdDisplay::finish_vend(const char beer[], const char drinker[], int delay_then_display_default){
  DEBUG_PRINT(F("LcdDisplay::finish_vend:("));
  DEBUG_PRINT(F("beer: "));
  DEBUG_PRINT(beer);
  DEBUG_PRINT(F(", drinker: "));
  DEBUG_PRINT(drinker);
  DEBUG_PRINTLN(F(")"));

  l.clear();
  l.home();
  l.print(center(drinker));
  l.setCursor(0, 1);
  l.print(center("Enjoy your"));
  l.setCursor(0, 2);
  l.print(center(beer));
  l.setCursor(0, 3);
  l.print(center(String("Beer Temp: ") + String(_internal_temp, 2) + String(LCD_CHAR_DEGREE)));
  delay(delay_then_display_default);
  display_default_status();
}

void LcdDisplay::delay_with_animation(int delay_time, int animation_ratio){
  /* Only animate every x times.  This allows for thread 
  *  yealding without super fast animation 
  */
  if (_slot_animation_skip < animation_ratio){  
    delay(delay_time);
    _slot_animation_skip++;
    return;  
  }

  /* Reset the animation column to zero once we have printed
   * all the pack man and goeast.
   */
  if (_slot_animation_col > 22){ _slot_animation_col = 0; }

  /* If we are at the begining of the LCD for the first
   * time or again we load the row with packman food.
   */
  if(_slot_animation_col == 0){
      l.setCursor(0, 3);
      for(int i = 0; i < 20; i++){
      l.print(LCD_CHAR_PACK_MAN_FOOD);
    }
  }

  /* Switch between packman chars Even = Open, Odd = Closed  */
  int pack_man_car = ((_slot_animation_col % 2) == 0) ? LCD_CUSTOM_CHAR_PACK_MAN_OPEN : LCD_CUSTOM_CHAR_PACK_MAN_CLOSED;

  /* Print a space to remove the old pack man char then print
   * the pack man char.   We want to stop printing 
   * as we get closer to the end of the LCD.
   */
  if (_slot_animation_col < 20){
    l.setCursor(_slot_animation_col, 3);
    l.print(" ");
  }
  if (_slot_animation_col < 19){
    l.write(pack_man_car);
  }

  /* Print a pace to remvoe the old goast, then print the new goast
   * we want to stop as we reach the endof the LCD.
   */
  if (_slot_animation_col > 1 && _slot_animation_col < 23){
    l.setCursor(_slot_animation_col - 3, 3);
    l.print(" ");
  }
  if (_slot_animation_col > 1 && _slot_animation_col < 22){
    l.write(LCD_CUSTOM_CHAR_PACK_MAN_GOAST);
  }

  _slot_animation_col++;
  _slot_animation_skip = 0;
  delay(delay_time);
}



void LcdDisplay::disply_msg(const char msg[], int delay_then_display_default){
  scroll_msg(msg, 0, delay_then_display_default);
}

void LcdDisplay::disply_msg(String msg, int delay_then_display_default){
  scroll_msg(msg, 0, delay_then_display_default);
}

void LcdDisplay::scroll_msg(const char msg[], int scroll_delay, int delay_then_display_default){
  return scroll_msg(String(msg), scroll_delay, delay_then_display_default);
}

void LcdDisplay::scroll_msg(String msg, int scroll_delay, int delay_then_display_default){
  DEBUG_PRINT(F("LcdDisplay::scrool_msg:("));
  DEBUG_PRINT(F("msg: "));
  DEBUG_PRINT(msg);
  DEBUG_PRINT(F(", scroll_delay: "));
  DEBUG_PRINT(scroll_delay);
  DEBUG_PRINTLN(F(")"));

  l.clear();
  l.home();

  int str_len = (msg.length() > 80) ? 80 : msg.length();
  for (int i = 0; i < str_len; i++) {
    switch(i) {
     case 20 :
        l.setCursor(0, 1);
        break; 
     case 40:
        l.setCursor(0, 2);
        break; 
     case 60:
        l.setCursor(0, 3);
        break;
    }
    l.print(msg[i]);
    if (scroll_delay > 0) { delay(scroll_delay); }
  }

  if (delay_then_display_default >= 0) {
    delay(delay_then_display_default);
    display_default_status();
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

    switch (slot_status) {
     case -1:    /* SLOT_STATUS_NOT_SET_UP */
       print(" ");
       break;
     case 0:     /* SLOT_STATUS_OK */
       write(LCD_CUSTOM_CHAR_OK);
       break;
     case 10:     /* SLOT_STATUS_RUNNING_OUT */
       write(LCD_CUSTOM_CHAR_ERROR);
       break;
   default:
       write(LCD_CUSTOM_CHAR_VERRY_BAD);
       break;
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
        writeAt(LCD_CUSTOM_CHAR_AIO_NOT_CONNECTED, col, row);
    }
}

void LcdDisplay::backlight_on(){
    DEBUG_PRINTLN(F("LcdDisplay::backlight_on()"));
    l.backlight();
}

void LcdDisplay::backlight_off(){
    DEBUG_PRINTLN(F("LcdDisplay::backlight_off()"));
    l.noBacklight();
}

void LcdDisplay::writeAt(uint8_t v, uint8_t col, uint8_t row){
    DEBUG_PRINT(F("LcdDisplay::writeAt(v:"));
    DEBUG_PRINT(v);
    DEBUG_PRINT(F(", col:"));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row:"));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    l.setCursor(col, row);
    l.write(v);
}

void LcdDisplay::printAt(float f, int accuracy, uint8_t col, uint8_t row){
    DEBUG_PRINT(F("LcdDisplay::printAt(f:"));
    DEBUG_PRINT(f, accuracy);
    DEBUG_PRINT(F(", col:"));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row:"));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    l.setCursor(col, row);
    l.print(f, accuracy);
}

void LcdDisplay::printAt(const char c[], uint8_t col, uint8_t row){
    DEBUG_PRINT(F("LcdDisplay::printAt(c:"));
    DEBUG_PRINT(c);
    DEBUG_PRINT(F(", col:"));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row:"));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    l.setCursor(col, row);
    l.printstr(c);
}

void LcdDisplay::printAt(String s, uint8_t col, uint8_t row){
    DEBUG_PRINT(F("LcdDisplay::printAt(s:"));
    DEBUG_PRINT(s);
    DEBUG_PRINT(F(", col:"));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row:"));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    l.setCursor(col, row);
    l.print(s);
}


void LcdDisplay::printAt(char c, uint8_t col, uint8_t row){
    DEBUG_PRINT(F("LcdDisplay::printAt(c:"));
    DEBUG_PRINT(c);
    DEBUG_PRINT(F(", col:"));
    DEBUG_PRINT(col);
    DEBUG_PRINT(F(", row:"));
    DEBUG_PRINT(row);
    DEBUG_PRINTLN(F(")"));
    l.setCursor(col, row);
    l.print(c);
}

void LcdDisplay::write(uint8_t value){
    l.write(value);
}

void LcdDisplay::print(char c){
    l.print(c);
}

void LcdDisplay::print(const char c[]){
    l.print(c);
}

void LcdDisplay::print(int i){
    l.print(i);
}

void LcdDisplay::print(String s){
    l.print(s);
}

void LcdDisplay::clear(){
    DEBUG_PRINTLN(F("LcdDisplay::clear()"));
    l.clear();
}

String LcdDisplay::center(const char c[]){
  return center(String(c));
}

String LcdDisplay::center(String s){
  int s_len = s.length();
  if (s_len > 20)  { return s.substring(0, 20); } /* Truncate to fit the screen */
  if (s_len == 19 || s_len == 20)   { return s; } /* Not much you can do with a 19 char string but return it. */
  int prepend_len = (20 - s_len) / 2;             /* Subtract the string len from 20 then devid by 2 to get the number of spaces to prepend the string with to center it. */
  String spaces = String("                  ");   /* 18 spaces that we can use to chop up. */
  String prefix = spaces.substring(0, prepend_len);
  prefix.concat(s);
  return prefix;
}



void LcdDisplay::_abvPrintIpAt(int col, int row, const char prefix[], IPAddress ip){
  INFO_PRINT(F("LcdDisplay::_abvPrintIpAt:"));
  INFO_PRINT(prefix);
  INFO_PRINT(F(" "));
  INFO_PRINTLN(ip);
  
  l.setCursor(col, row);
  l.print(prefix); 
  l.print(ip[2]);
  l.print(".");
  l.print(ip[3]);
}

void LcdDisplay::_printMacAt(int col, int row, const char prefix[], byte mac[]){
  l.setCursor(col, row);
  l.print(prefix);
  
  INFO_PRINT(F("LcdDisplay::_printMacAt: MAC Address: "));
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      l.print("0");
      INFO_PRINT(F("0"));
    }
    l.print(mac[i], HEX);
    INFO_PRINT(mac[i], HEX);
    if (i > 0) {
      l.print(":");
      INFO_PRINT(F(":"));
    } 
  }
  INFO_PRINTLN();
}

String LcdDisplay::_wifi_status_string(uint8_t wifi_status){
  switch(wifi_status) {
     case 255 :
        return F("NO SHIELD");
     case 0 :
        return F("IDLE STATUS");
     case 1 :
        return F("NO SSID AVAIL");
     case 2 :
        return F("SCAN COMPLETED");
     case 3 :
        return F("CONNECTED");
     case 4 :
        return F("CONNECT FAILED");
     case 5 :
        return F("CONNECTION LOST");
     case 6 :
        return F("AP LISTENING");
     case 7 :
        return F("AP CONNECTED");
     case 8 :
        return F("AP FAILED");
     case 9 :
        return F("PROVISIONING");
     case 10 :
        return F("PROVISIONING FAILED");
     default:
        return F("UNKNOWN STATUS");
    }
}
