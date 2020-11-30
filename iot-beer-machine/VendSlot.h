#ifndef VEND_SLOT_H
#define VEND_SLOT_H

#include <Adafruit_ADS1015.h>
#include "lcd_display.h"
#include "logging.h"


class VendSlot{

public:
  VendSlot();
  void setup(int slot_number, int relay_pin, 
             Adafruit_ADS1115 *vend_adc, int vend_adc_pin, 
             Adafruit_ADS1115 *slot_low_adc, int slot_low_adc_pin, 
             LcdDisplay *lcd_display);

  int slot_status();
  int vend();
  void reset();

  static const int SLOT_STATUS_NOT_SET_UP  = -1;
  static const int SLOT_STATUS_READY       = 0;
  static const int SLOT_STATUS_VENDING     = 1;
  static const int SLOT_STATUS_RUNNING_OUT = 10;
  static const int SLOT_STATUS_STUCK       = 50;
  static const int SLOT_STATUS_ERROR       = 255;


protected:
  
  bool      _is_vending_done();
  uint16_t  _read_adc(Adafruit_ADS1015 *adc, int pin);

  void _moter_on();
  void _moter_off();
  void _set_vend_status(int s);



  Adafruit_ADS1015  *_slot_low_adc;
  Adafruit_ADS1015  *_vend_adc;
  LcdDisplay        *_display;
  
  int _slot_number;
  int _relay_pin;
  int _vend_adc_pin;
  int _slot_low_adc_pin;
  int _vend_status;

};

#endif // VEND_SLOT_H
