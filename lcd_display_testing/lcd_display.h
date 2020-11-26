#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi101.h>
#include "logging.h"

#define LCD_CUSTOM_CHAR_WAIT1 0
#define LCD_CUSTOM_CHAR_WAIT2 1
#define LCD_CUSTOM_CHAR_WAIT3 2
#define LCD_CUSTOM_CHAR_VERRY_BAD 3
#define LCD_CUSTOM_CHAR_AIO_CONNECTED 4
#define LCD_CUSTOM_CHAR_AIO_DISCONNECTED 5
#define LCD_CUSTOM_CHAR_OK 6
#define LCD_CUSTOM_CHAR_ERROR 7
#define LCD_CHAR_WIFI_CONNECTED (char)B10110111
#define LCD_CHAR_WIFI_NOT_CONNECTED (char)B10110010
#define LCD_CHAR_DEGREE (char)B11011111
#define LCD_CHAR_OHMS (char)B11110100

const char SLOT_NAMES[]   = { '1', '2', '3', '4', '5', '6' };
const int  SLOT_COLUMNS[] = {  2,   6,  10,   2,   6,   10 };
const int  SLOT_ROWS[]    = {  0,   0,   0,   1,   1,   1 };


class LcdDisplay {

public:
  LcdDisplay();
  void init(LiquidCrystal_I2C *lcd);
  void backlight_on();
  void backlight_off();
  void writeAt(uint8_t value, uint8_t col, uint8_t row);
  void write(uint8_t value);
  void printAt(const char c[], uint8_t col, uint8_t row);
  void printAt(char c, uint8_t col, uint8_t row);
  void printAt(float f, int accuracy, uint8_t col, uint8_t row);
  void print(char c);
  void print(const char c[]);
  void clear();
  void reset();
  void set_wifi_status();
  void set_adafruit_status(bool s);
  void set_internal_temp(float temp);
  void set_external_temp(float temp);
  void set_external_inHg(float inHg);
  void set_external_humidity(float humidity);
  void set_amps(float amps);
  void set_watts(float watts);
  void set_slot_status(int slot, int slot_status); 
  void disply_msg(const char msg[]);
  void delay_with_animation(int mills, int slot); 
  void repaint();


protected:
  LiquidCrystal_I2C *_lcd;
  bool _adafruit_status;
  float _internal_temp;
  float _external_temp;
  float _external_inHg;
  float _external_humidity;
  float _amps;
  float _watts;
  int _slot_status[6];
};

#endif // LCD_DISPLAY_H
