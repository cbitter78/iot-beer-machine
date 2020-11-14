#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <WiFi101.h>
#include "lcd_art.h"

class Display{

public:
  Display(uint8_t lcd_Addr, bool back_light_on);
  void backlight_on();
  void backlight_off();
  void writeAt(uint8_t value, uint8_t col, uint8_t row);
  void printAt(const char c[], uint8_t col, uint8_t row);
  void clear();
  void reset();
  void set_wifi_status(uint8_t wifi_status);

protected:
  LiquidCrystal_I2C _lcd;
  bool _back_light_on;
}

#endif // DISPLAY_H