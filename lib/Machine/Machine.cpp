#ifndef MOCK_MACHINE

#include <Machine.h>
#include <yhdc.h>
#include <Logging.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <OneWire.h>

/* Sensors */
#define AMP_MEETER_ANALOG_PIN    A0
#define INTERNAL_TEMP_SENSOR_PIN A1

OneWire oneWire(INTERNAL_TEMP_SENSOR_PIN);
DallasTemperature internalTempSensor(&oneWire);
Adafruit_BME280 bme;

VendSlot  slot1;
VendSlot  slot2;
VendSlot  slot3;
VendSlot  slot4;
VendSlot  slot5;
VendSlot  slot6;

Adafruit_ADS1115  adc0(0x48); 
Adafruit_ADS1115  adc1(0x49); 
Adafruit_ADS1115  adc2(0x4A); 


Machine::Machine(LcdDisplay *lcd_display){
    _display = lcd_display;
    _BMEStatus = false; 
    _machine_name = String("Beer Machine");
}

void Machine::init(){
    DEBUG_PRINTLN("Machine::init");

    adc0.begin();
    adc1.begin();
    adc2.begin();

    slot1.setup(1, 9,  &adc1, 0, &adc0, 2, _display);
    slot2.setup(2, 10, &adc1, 1, &adc0, 3, _display);
    slot3.setup(3, 11, &adc1, 2, &adc2, 0, _display);
    slot4.setup(4, 12, &adc1, 3, &adc2, 1, _display);
    slot5.setup(5, 5,  &adc0, 0, &adc2, 2, _display);
    slot6.setup(6, 6,  &adc0, 1, &adc2, 3, _display);
  
    slots[0] = &slot1;
    slots[1] = &slot2;
    slots[2] = &slot3;
    slots[3] = &slot4;
    slots[4] = &slot5;
    slots[5] = &slot6;
    
    internalTempSensor.begin(); 
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

}

internalSensorData Machine::read_internal(){
    DEBUG_PRINTLN(F("Machine::read_internal"));
    internalTempSensor.requestTemperaturesByIndex(0);
    float c = internalTempSensor.getTempCByIndex(0);
    float f = (c*1.8)+32.0f;

    INFO_PRINT(F("Machine::read_internal Temp in Celcius:    "));
    INFO_PRINTLN(c, 4);
    INFO_PRINT(F("Machine::read_internal Temp in Fahrenheit: "));
    INFO_PRINTLN(f, 4);
    return {c, f};
}

externalSensorData Machine::read_external(){
    DEBUG_PRINT(F("Machine::read_external("));
    DEBUG_PRINT(F("_BMEStatus: "));
    DEBUG_PRINT(_BMEStatus);
    DEBUG_PRINTLN(F(")"));
    if (_BMEStatus == false){
        WARN_PRINTLN(F("Machine::read_external: No Sensor fake data will be returned"));
        delay(1000);
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
    float a = readACCurrentValue(AMP_MEETER_ANALOG_PIN);
    float w = a * 110;
    INFO_PRINT(F("Machine::read_power_usage: Amps: "));
    INFO_PRINT(a, 2);
    INFO_PRINT(F(" Watts: "));
    INFO_PRINTLN(w, 1);
    return {a, w};
}

String Machine::name()
{
    DEBUG_PRINT(F("Machine::name:"));
    DEBUG_PRINTLN(_machine_name)
    return _machine_name;  
}

#endif
