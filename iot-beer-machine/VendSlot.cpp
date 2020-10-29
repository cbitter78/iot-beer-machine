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
    _vend_status      = VendSlot::VEND_STATUS_NOT_SET_UP;
}


void VendSlot::setup(int slot_number, int relay_pin, 
                    Adafruit_ADS1115 *vend_adc, int vend_adc_pin, 
                    Adafruit_ADS1115 *slot_low_adc, int slot_low_adc_pin, 
                    LiquidCrystal_I2C *lcd, int lcd_row){

    _slot_number      = slot_number;
    _relay_pin        = relay_pin;
    _vend_adc         = vend_adc;
    _vend_adc_pin     = vend_adc_pin;
    _slot_low_adc     = slot_low_adc;
    _slot_low_adc_pin = slot_low_adc_pin;
    _lcd              = lcd;
    _lcd_row          = lcd_row;

    _clear_dispaly();
    pinMode(_relay_pin, OUTPUT);
    digitalWrite(_relay_pin, HIGH);

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
    Serial.println("Slot " + String(_slot_number) + " is running out of beer");
    return VendSlot::SLOT_STATUS_RUNNING_OUT;
  }else
  {
    return VendSlot::SLOT_STATUS_OK;
  }
}

    
int VendSlot::vend_status(){ return _vend_status; }



int VendSlot::vend(){
     _set_vend_status(VendSlot::VEND_STATUS_VENDING);
     _moter_on();
     delay(2000);
     int t = 0;
     while (t < 65) {   // This is about a 15 second timeout.   If timeout then the slot is stuck.
       if (_is_vending_done()){
         delay(100);
         if (_is_vending_done()){
           _moter_off();
           _set_vend_status(VendSlot::VEND_STATUS_READY);
           slot_status();
           return VendSlot::VEND_STATUS_READY;
         }
       }
       t++;
       delay(200);
     }
     _moter_off();
     slot_status();
     _set_vend_status(VendSlot::VEND_STATUS_STUCK);
     slot_status();
     return VendSlot::VEND_STATUS_STUCK;
}


 void VendSlot::_moter_on(){
   digitalWrite(_relay_pin, LOW);
 }

 void VendSlot::_moter_off(){
   digitalWrite(_relay_pin, HIGH);
 }

 void VendSlot::_set_vend_status(int s){
   _vend_status = s;
   switch (_vend_status) {
     case VendSlot::VEND_STATUS_READY:    
       _lcd_display("Ready    ");
       break;
     case VendSlot::VEND_STATUS_VENDING:     
       _lcd_display("Vending ");
       break;
    case VendSlot::VEND_STATUS_STUCK:     
       _lcd_display("Stuck   ");
       break;
    case VendSlot::VEND_STATUS_ERROR:     
       _lcd_display("Error   ");
       break;
   default:
       _vend_status = VendSlot::VEND_STATUS_ERROR;
       _lcd_display("Error   ");
       break;
   }
 }
 
 bool VendSlot::_is_vending_done(){
   uint16_t v = _read_adc(_vend_adc, _vend_adc_pin);ß
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

 void VendSlot::_lcd_display(char* msg){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(_lcd_row, 0);
    l.print(String(_slot_number) + ": ");
    l.print(msg);
 }
 
 void VendSlot::_clear_dispaly(){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(_lcd_row, 0);
    l.print("Setting up Slot " + String(_slot_number));
    delay(1000);
    l.setCursor(_lcd_row, 0);
    l.print("                    "); 
 }
