#include "VendSlot.h"

VendSlot::VendSlot(){
     _slot_number = 0;
    _relay_pin    = 0;
    _adc          = NULL;
    _adc_pin      = 0;
    _lcd          = NULL;
    _lcd_column   = 0;
    _lcd_row      = 0;
    _slot_status  = -1;
}


void VendSlot::setup(int slot_number, int relay_pin, Adafruit_ADS1115 *adc, int adc_pin, LiquidCrystal_I2C *lcd, int lcd_column, int lcd_row){

    _slot_number = slot_number;
    _relay_pin   = relay_pin;
    _adc         = adc;
    _adc_pin     = adc_pin;
    _lcd         = lcd;
    _lcd_column  = lcd_column;
    _lcd_row     = lcd_row;
    _slot_status = 0;

    pinMode(_relay_pin, OUTPUT);
    digitalWrite(_relay_pin, HIGH);
    _clear_dispaly();
    reset();
    _set_status(VendSlot::STATUS_READY);
}

void VendSlot::reset(){
  if (_is_vending_done() == false){
    vend();
  }
}


int VendSlot::vend(){
     _set_status(VendSlot::STATUS_VENDING);
     _moter_on();
     delay(2000);
     int t = 0;
     while (t < 65) {   // This is about a 15 second timeout.   If timeout then the slot is stuck.
       if (_is_vending_done()){
         delay(100);
         if (_is_vending_done()){
           _moter_off();
           _set_status(VendSlot::STATUS_READY);
           return VendSlot::STATUS_READY;
         }
       }
       t++;
       delay(200);
     }
     _moter_off();
     _set_status(VendSlot::STATUS_STUCK);
     return VendSlot::STATUS_STUCK;
}


 void VendSlot::_moter_on(){
   digitalWrite(_relay_pin, LOW);
 }

 void VendSlot::_moter_off(){
   digitalWrite(_relay_pin, HIGH);
 }

 void VendSlot::_set_status(int s){
   _slot_status = s;
   switch (_slot_status) {
     case VendSlot::STATUS_READY:    
       _lcd_display("Ready             ");
       break;
     case VendSlot::STATUS_VENDING:     
       _lcd_display("Vending ");
       break;
    case VendSlot::STATUS_STUCK:     
       _lcd_display("Stuck   ");
       break;
    case VendSlot::STATUS_ERROR:     
       _lcd_display("Error   ");
       break;
   default:
       _slot_status = VendSlot::STATUS_ERROR;
       _lcd_display("Error   ");
       break;
   }
 }
 
 bool VendSlot::_is_vending_done(){
   int v = (*_adc).readADC_SingleEnded(_adc_pin);
   Serial.print("ADC Value from slot " + String(_slot_number) + " reads: ");
   Serial.println(v);
//   LiquidCrystal_I2C l = *_lcd;
//   l.setCursor(9, _lcd_column);
//   l.print(v);
   if (v > 2000 and v < 65000){
     return false;  // This means there is the circut is closed (has power) and vending is still taking place.
   }
   else{
     return true;   // When there is no voltagge read by the ADC then vending is done.  
   }
 }

 void VendSlot::_lcd_display(char* msg){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(_lcd_row, _lcd_column);
    l.print(String(_slot_number) + ": ");
    l.print(msg);
 }
 
 void VendSlot::_clear_dispaly(){
    LiquidCrystal_I2C l = *_lcd;
    l.setCursor(_lcd_row, _lcd_column);
    l.print("Setting up Slot " + String(_slot_number));
    delay(1000);
    l.setCursor(_lcd_row, _lcd_column);
    l.print("                    "); 
 }
