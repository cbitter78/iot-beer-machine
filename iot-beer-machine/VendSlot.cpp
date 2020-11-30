#include "VendSlot.h"

VendSlot::VendSlot(){
    _slot_number      = 0;
    _relay_pin        = 0;
    _vend_adc         = NULL;
    _vend_adc_pin     = 0;
    _slot_low_adc     = NULL;
    _slot_low_adc_pin = 0;
    _lcd              = NULL;
    _lcd_row          = 0;
    _lcd_column       = 0;
    _vend_status      = VendSlot::VEND_STATUS_NOT_SET_UP;
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
    _lcd_display      = lcd_display
    pinMode(_relay_pin, INPUT_PULLUP);
    reset();
    slot_status();
}

void VendSlot::reset(){
  if (_is_vending_done() == false){
    vend();
  }
  _set_vend_status(VendSlot::VEND_STATUS_READY);
}


int VendSlot::slot_status(){
  int v = _read_adc(_slot_low_adc, _slot_low_adc_pin);  
  if (v > 200){
    WARN_PRINTLN("Slot " + String(_slot_number) + " is running out of beer");
    return VendSlot::SLOT_STATUS_RUNNING_OUT;
  }else
  {
    return VendSlot::SLOT_STATUS_OK;
  }
}

    

int VendSlot::vend(){
    INFO_PRINT("Vending Slot: ");
    INFO_PRINT(_slot_number);
     _moter_on();
     for(int i = 0; i < 15; i++){
        _delay_with_animation(100);
     }

     int t = 0;
     while (t < 200) {   // This is about a 20 second timeout.   If timeout then the slot is stuck.
       if (_is_vending_done()){
        delay(25);
         if (_is_vending_done()){
            delay(25);
             if (_is_vending_done()){
               _moter_off();
               _set_vend_status(VendSlot::VEND_STATUS_READY);
               return VendSlot::VEND_STATUS_READY;
             }
         }
       }
       t++;
       _delay_with_animation(100);
     }
     _moter_off();
     slot_status();
     _set_vend_status(VendSlot::VEND_STATUS_STUCK);  //TODO: We need an icon for this
     slot_status();
     return VendSlot::VEND_STATUS_STUCK;
}







 void VendSlot::_set_vend_status(int s){
   _vend_status = s;
   switch (_vend_status) {
     case VendSlot::VEND_STATUS_READY:    
       _lcd_display(6);
       break;
     case VendSlot::VEND_STATUS_VENDING:     
       _lcd_display(1);
       break;
    case VendSlot::VEND_STATUS_STUCK:     
       _lcd_display(7);
       break;
    case VendSlot::VEND_STATUS_ERROR:     
       _lcd_display(7);
       break;
   default:
       _vend_status = VendSlot::VEND_STATUS_ERROR;
       _lcd_display(7);
       break;
   }
 }

void VendSlot::_moter_on(){
   INFO_PRINTLN(F("Turning moter on"));
   digitalWrite(_relay_pin, LOW);
}

void VendSlot::_moter_off(){
   INFO_PRINTLN(F("Turning moter off"));
   digitalWrite(_relay_pin, HIGH);
}

 bool VendSlot::_is_vending_done(){
   uint16_t v = _read_adc(_vend_adc, _vend_adc_pin);
   DEBUG_PRINT("VENDING ADC READ: ");
   DEBUG_PRINTLN(v);
   if (v > 2000){
     return false;  // This means there is the circut is closed (has power) and vending is still taking place.
   }
   else{
     return true;   // When there is no voltagge (circut open) then vending is done.  
   }
}

 uint16_t VendSlot::_read_adc(Adafruit_ADS1015 *adc, int pin){
    uint16_t v = (*adc).readADC_SingleEnded(pin);
    if (v == 65535){ v = 0; }
    return v;
}
