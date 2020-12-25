#ifndef VEND_SLOT_H
#define VEND_SLOT_H

#include <Adafruit_ADS1015.h>
#include <LCDDisplay.h>


#define SLOT_STATUS_NOT_SET_UP  -1
#define SLOT_STATUS_OK          0
#define SLOT_STATUS_VENDING     1
#define SLOT_STATUS_RUNNING_OUT 10
#define SLOT_STATUS_STUCK       50
#define SLOT_STATUS_ERROR       255

class VendSlot{

public:
  VendSlot();
  void setup(int slot_number, int relay_pin, 
             Adafruit_ADS1115 *vend_adc, int vend_adc_pin, 
             Adafruit_ADS1115 *slot_low_adc, int slot_low_adc_pin, 
             LcdDisplay *lcd_display);

  int slot_status();
  void vend();
  void reset();

protected:
    
  uint16_t  _read_adc(Adafruit_ADS1015 *adc, int pin);
  bool _is_vending_done();
  void _moter_on();
  void _moter_off();

  

  Adafruit_ADS1015  *_slot_low_adc;
  Adafruit_ADS1015  *_vend_adc;
  LcdDisplay        *_display;
  
  int _slot_number;
  int _relay_pin;
  int _vend_adc_pin;
  int _slot_low_adc_pin;
  int _moter_status;
};

#endif // VEND_SLOT_H
