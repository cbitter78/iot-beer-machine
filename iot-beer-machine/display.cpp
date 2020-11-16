//#include "display.h"
//
//Display::Display(uint8_t lcd_Addr, bool back_light_on){
//    _back_light_on = back_light_on
//    LiquidCrystal_I2C _lcd(lcd_Addr,20,4);
//
//    byte* lcd_custom_chars[8] = {charWait0, charWait1, charWait2, charWifi, charAdaFruitNotConnected, charAdaFruitConnected, charSmile, charSkull};
//    for (int i = 0; i < 8; i++){
//        lcd.createChar(i, lcd_custom_chars[i]);
//    }
//
//    reset();
//}
//
//void Display::set_wifi_status(uint8_t wifi_status){
//    if (wifi_status == WL_CONNECTED){
//        writeAt(3,0,0)
//    }else{
//        writeAt(7,0,0)
//    }
//}
//
//void Display::backlight_on(){
//    _lcd.backlight();
//}
//
//void Display::backlight_off(){
//    _lcd.noBacklight();
//}
//
//void Display::writeAt(uint8_t value, uint8_t col, uint8_t row){
//    _lcd.setCursor(col, row);
//    _lcd.write(value)
//}
//
//void Display::printAt(const char c[], uint8_t col, uint8_t row){
//    _lcd.setCursor(col, row);
//    _lcd.printstr(value)
//}
//
//void Display::clear(){
//    _lcd.clear();
//}
//
//void Display::reset(){
//    _lcd.init();
//    if (_back_light_on){
//        _lcd.backlight();
//    }
//}
