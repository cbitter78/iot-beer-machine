#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi101.h>

#define C_WIFI_CONNECTED 3
#define C_ERROR 7



class LcdDisplay {

public:
  LcdDisplay();
  void init(bool back_light);
  void backlight_on();
  void backlight_off();
  void writeAt(uint8_t value, uint8_t col, uint8_t row);
  void printAt(const char c[], uint8_t col, uint8_t row);
  void clear();
  void reset();
  void set_wifi_status(uint8_t wifi_status);
  void set_adafruit_status(bool status);
  void set_internal_temp(double temp);
  void set_external_temp(double temp);
  void set_external_inHg(double inHg);
  void set_external_humidity(double humidity);
  void set_amps(double amps);
  void set_watts(double watts);
  void set_slot_status(int slot, int slot_status); 
  void disply_msg(const char msg[]);
  void delay_with_animation(int mills, int slot); 
 
protected:
  LiquidCrystal_I2C *_lcd;
  bool _back_light_on;
  
};

#endif // LCD_DISPLAY_H
