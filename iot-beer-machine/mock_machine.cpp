#define MOCK_MACHINE

#ifdef MOCK_MACHINE

#include <Adafruit_ADS1015.h>
#include "machine.h"
#include "logging.h"

/*
 * This class sets up the VendSlots and performs machine 
 * Specific operations (Test Rig).  We don't fully abstract logic here because
 * the aim was just a abstract enough to allow for a test rig.
 */

VendSlot  slot1;
VendSlot  slot2;
VendSlot  slot3;
VendSlot  slot4;
VendSlot  slot5;
VendSlot  slot6;

Adafruit_ADS1115  adc(0x48); 

Machine::Machine(LcdDisplay *lcd_display){
    _display = lcd_display;
    _machine_name = "Mock test rig";
    _BMEStatus = false; 
}

void Machine::init(){
    DEBUG_PRINTLN("MockMachine::init");
    DEBUG_PRINTLN("Machine::init");

    adc.begin();

    slot1.setup(1, 12, &adc, 0, &adc, 2, _display);
    slot2.setup(2, 11, &adc, 1, &adc, 3, _display);
    slot3.setup(3, 12, &adc, 0, &adc, 2, _display);
    slot4.setup(4, 11, &adc, 1, &adc, 3, _display);
    slot5.setup(5, 12, &adc, 0, &adc, 2, _display);
    slot6.setup(6, 11, &adc, 1, &adc, 3, _display);
  
    slots[0] = &slot1;
    slots[1] = &slot2;
    slots[2] = &slot3;
    slots[3] = &slot4;
    slots[4] = &slot5;
    slots[5] = &slot6;
    update_all_slot_status();
}

void Machine::update_all_slot_status(){
    LcdDisplay lcd = *_display;
    lcd.set_slot_status(0, slot1.slot_status());
    lcd.set_slot_status(1, slot2.slot_status());
    lcd.set_slot_status(2, slot3.slot_status());
    lcd.set_slot_status(3, slot4.slot_status());
    lcd.set_slot_status(4, slot5.slot_status());
    lcd.set_slot_status(5, slot6.slot_status());
}


internalSensorData Machine::read_internal(){
    DEBUG_PRINTLN(F("MockMachine::read_internal"));
    return {
        0.856F,        /* Celcius         */
        33.45774F,     /* Fahrenhite      */
    };
}
externalSensorData Machine::read_external(){
    DEBUG_PRINTLN(F("MockMachine::read_external"));
    return {
        22.54944F,      /* Celcius         */
        72.589F,        /* Fahrenhite      */
        81.7372050F,    /* Presure in hPa  */
        32.81447F,      /* Presure in inHG */
        95.17544F       /* Humidity        */
    };

}  
powerSensorData Machine::read_power_usage(){
    DEBUG_PRINTLN(F("MockMachine::read_power_usage"));
    return {
        4.25218F,       /* Amps            */
        467.73908F,     /* Watts           */
    };
}
String Machine::name()
{
    DEBUG_PRINT(F("MockMachine::name:"));
    DEBUG_PRINTLN(_machine_name);
    return _machine_name;  
}

#endif
