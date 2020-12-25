#ifdef MOCK_MACHINE

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_BME280.h>
#include <Machine.h>
#include <Logging.h>

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
Adafruit_BME280 bme;

float randomFloat(float minf, float maxf);

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
        
    if (!bme.begin()) {
        ERROR_PRINTLN(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
        _BMEStatus = false;
    }else{
        _BMEStatus = true;
        DEBUG_PRINTLN("Machine::init:  BMP280 started");
    } 
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
    float c = randomFloat(0.56, 3.89);
    float f = (c*1.8)+32.0f;
    
    INFO_PRINT(F("Machine::read_internal Temp in Celcius:    "));
    INFO_PRINTLN(c, 4);
    INFO_PRINT(F("Machine::read_internal Temp in Fahrenheit: "));
    INFO_PRINTLN(f, 4);
    
    return { c, f };
}

externalSensorData Machine::read_external(){
    DEBUG_PRINT(F("Machine::read_external("));
    DEBUG_PRINT(F("_BMEStatus: "));
    DEBUG_PRINT(_BMEStatus);
    DEBUG_PRINTLN(F(")"));
    if (_BMEStatus == false){
        WARN_PRINTLN(F("Machine::read_external: No Sensor fake data will be returned"));
        return {0.0F,0.0F,0.0F,0.0F,0.0F};
    }

    float c    = bme.readTemperature();
    float f    = (c*1.8)+32.0f;
    float hPa  = bme.readPressure() / 100.0F;
    float inHg = (hPa / 3386.0F) * 100.0F;
    float h    = bme.readHumidity();
  
    INFO_PRINT(F("Machine::read_external Temp:     "));
    INFO_PRINT(c,2);
    INFO_PRINT(F(" *C  "));
    INFO_PRINT(f,2);
    INFO_PRINTLN(F(" *F"));
  
    INFO_PRINT(F("Machine::read_external Pressure: "));
    INFO_PRINT(hPa, 4);
    INFO_PRINT(F(" hPa "));
    INFO_PRINT(inHg, 4);
    INFO_PRINTLN(F(" inHg"));
  
    INFO_PRINT(F("Machine::read_external Humidity: "));
    INFO_PRINT(h, 4);
    INFO_PRINTLN(F("%"));
    return { c, f, hPa, inHg, h };
} 

powerSensorData Machine::read_power_usage(){
    DEBUG_PRINTLN(F("Machine::read_power_usage"));
    float a = randomFloat(4.0, 5.0);
    float w = a * 110;
    INFO_PRINT(F("Machine::read_power_usage: Amps: "));
    INFO_PRINT(a, 2);
    INFO_PRINT(F(" Watts: "));
    INFO_PRINTLN(w, 1);
    return {a, w};
}

String Machine::name(){
    DEBUG_PRINT(F("MockMachine::name:"));
    DEBUG_PRINTLN(_machine_name);
    return _machine_name;  
}

float randomFloat(float minf, float maxf)
{
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

#endif
