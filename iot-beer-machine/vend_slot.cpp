#include "vend_slot.h"
#include "logging.h"

VendSlot::VendSlot(){
    _slot_number      = 0;
    _relay_pin        = 0;
    _vend_adc         = NULL;
    _vend_adc_pin     = 0;
    _slot_low_adc     = NULL;
    _slot_low_adc_pin = 0;
    _display          = NULL;
    _moter_status     = SLOT_STATUS_OK;
}


void VendSlot::setup(int slot_number, int relay_pin, 
                    Adafruit_ADS1115 *vend_adc, int vend_adc_pin, 
                    Adafruit_ADS1115 *slot_low_adc, int slot_low_adc_pin, 
                    LcdDisplay *lcd_display){

    _slot_number      = slot_number;
    _relay_pin        = relay_pin;
    _vend_adc         = vend_adc;
    _vend_adc_pin     = vend_adc_pin;
    _slot_low_adc     = slot_low_adc;
    _slot_low_adc_pin = slot_low_adc_pin;
    _display          = lcd_display;
    digitalWrite(_relay_pin, HIGH); 
    pinMode(_relay_pin, OUTPUT);
    reset();
}

void VendSlot::reset(){
  if (_is_vending_done() == false)
  { 
    vend();   /* This should reset the moter */
  }
}


int VendSlot::slot_status(){
  DEBUG_PRINT(F("VendSlot::slot_status(_slot_number: "));
  DEBUG_PRINT(_slot_number);
  DEBUG_PRINTLN(F(")"));
  
  if (_moter_status != SLOT_STATUS_OK) { 
    WARN_PRINT(F("VendSlot::slot_status(_slot_number: "));
    WARN_PRINT(_slot_number);
    WARN_PRINT(F(", _moter_status: "));
    WARN_PRINT(_moter_status);
    WARN_PRINTLN(F("): WARNING Moter Stuck!"));
    return _moter_status; 
  }  

  int v = _read_adc(_slot_low_adc, _slot_low_adc_pin);  
  if (v > 200){
    WARN_PRINT(F("VendSlot::slot_status(_slot_number: "));
    WARN_PRINT(_slot_number);
    WARN_PRINTLN(F("): WARNING Running out of beer!"));
    return SLOT_STATUS_RUNNING_OUT;
  }
  
  return SLOT_STATUS_OK;
}

    

void VendSlot::vend(){
  INFO_PRINT(F("VendSlot::vend(_slot_number: "));
  INFO_PRINT(_slot_number);
  INFO_PRINTLN(F(")"));

  LcdDisplay lcd = *_display;

  _moter_on();
  for(int i = 0; i < 30; i++){
    /* 
      * We need to wait 1.5 seconds to allow the moter advance
      * the the wheel to ensure the paddle switch is engaged
      */ 
    lcd.delay_with_animation(50, 5);  
  }

  for (int i = 0; i < 300; i++){  /* 15 second timeout.   If timeout then the slot is stuck. */
    if (_is_vending_done()){
        lcd.delay_with_animation(25, 5);
          if (_is_vending_done()){
            lcd.delay_with_animation(25, 5);
              if (_is_vending_done()){
                _moter_off();
                return;
              }
          }
    }
    lcd.delay_with_animation(50, 5);
  }

  // If we get here then the moter is stuck
  _moter_off();
  _moter_status = SLOT_STATUS_STUCK;
}

void VendSlot::_moter_on(){
   INFO_PRINTLN(F("VendSlot::_moter_on(): Turning moter on"));
   digitalWrite(_relay_pin, LOW);
}

void VendSlot::_moter_off(){
   INFO_PRINTLN(F("VendSlot::_moter_off(): Turning moter off"));
   digitalWrite(_relay_pin, HIGH);
}

bool VendSlot::_is_vending_done(){
    DEBUG_PRINT(F("VendSlot::_is_vending_done():"));
    uint16_t v = _read_adc(_vend_adc, _vend_adc_pin);
    if (v > 2000){
      return false;  // This means there is the circut is closed (has power) and vending is still taking place.
    }
    else{
      return true;   // When there is no voltagge (circut open) then vending is done.  
    }
}

 uint16_t VendSlot::_read_adc(Adafruit_ADS1015 *adc, int pin){
    DEBUG_PRINT(F("VendSlot::_read_adc(pin: "))
    DEBUG_PRINT(pin);
    DEBUG_PRINT(F(", value: "));

    uint16_t v = (*adc).readADC_SingleEnded(pin);
    if (v == 65535){ v = 0; }
    
    DEBUG_PRINT(v);
    DEBUG_PRINTLN(F(")"));
    return v;
}
