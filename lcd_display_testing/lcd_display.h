#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi101.h>
#include "logging.h"

#define LCD_WAIT1 0
#define LCD_WAIT2 1
#define LCD_WAIT3 2
#define LCD_WIFI_CONNECTED 3
#define LCD_AIO_CONNECTED 4
#define LCD_AIO_DISCONNECTED 5
#define LCD_OK 6
#define LCD_ERROR 7

class LcdDisplay {

public:
  LcdDisplay();
  void init(LiquidCrystal_I2C *lcd);
  void backlight_on();
  void backlight_off();
  void writeAt(uint8_t value, uint8_t col, uint8_t row);
  void printAt(const char c[], uint8_t col, uint8_t row);
  void clear();
  void reset();
  void set_wifi_status();
  void set_adafruit_status(bool s);
  void set_internal_temp(double temp);
  void set_external_temp(double temp);
  void set_external_inHg(double inHg);
  void set_external_humidity(double humidity);
  void set_amps(double amps);
  void set_watts(double watts);
  void set_slot_status(int slot, int slot_status); 
  void disply_msg(const char msg[]);
  void delay_with_animation(int mills, int slot); 
  void repaint();
 
protected:
  LiquidCrystal_I2C *_lcd;
  bool _adafruit_status;
  double _internal_temp;
  double _external_temp;
  double _external_inHg;
  double _external_humidity;
  double _amps;
  double _watts;
  int _slot_status[6];
};

#endif // LCD_DISPLAY_H
