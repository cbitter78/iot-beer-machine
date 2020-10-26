#ifndef VEND_SLOT_H
#define VEND_SLOT_H

#include <Adafruit_ADS1015.h>
#include <LiquidCrystal_I2C.h>


class VendSlot{

public:
  
  VendSlot();
  void setup(int slot_number, int relay_pin, Adafruit_ADS1115 *adc, int adc_pin, LiquidCrystal_I2C *lcd, int lcd_column, int lcd_row);

  int slot_status();
  int vend();

  static const int STATUS_NOT_SET_UP   = -1;
  static const int STATUS_READY   = 0;
  static const int STATUS_VENDING = 1;
  static const int STATUS_STUCK   = 50;
  static const int STATUS_ERROR   = 255;

protected:
  
  bool _is_vending_done();
  void _moter_on();
  void _moter_off();
  void _set_status(int s);
  void _clear_dispaly();
  void _lcd_display(char* msg);
  
  Adafruit_ADS1015  *_adc;
  LiquidCrystal_I2C *_lcd;
  int _slot_number;
  int _relay_pin;
  int _adc_pin;
  int _lcd_column;
  int _lcd_row;
  int _slot_status;

};

#endif // VEND_SLOT_H
