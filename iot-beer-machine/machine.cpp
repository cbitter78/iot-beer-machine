/*
 * This file holds values that are unique to a beer machine wireing.
 * This abstraction is needed to support a test harness which may
 * be wired differently than the actual beer machine.
 */

#define MACHINE
#ifdef MACHINE

#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "machine.h"
#include "yhdc.h"
#include "logging.h"

#define SLOT_1_DIO_PIN 9
#define SLOT_2_DIO_PIN 10
#define SLOT_3_DIO_PIN 11
#define SLOT_4_DIO_PIN 12
#define SLOT_5_DIO_PIN 5
#define SLOT_6_DIO_PIN 6

#define SLOT_1_VEND_ADC 1 
#define SLOT_2_VEND_ADC 1 
#define SLOT_3_VEND_ADC 1 
#define SLOT_4_VEND_ADC 1 
#define SLOT_5_VEND_ADC 0 
#define SLOT_6_VEND_ADC 0 

#define SLOT_1_VEND_ADC_PIN 0
#define SLOT_2_VEND_ADC_PIN 1
#define SLOT_3_VEND_ADC_PIN 2
#define SLOT_4_VEND_ADC_PIN 3
#define SLOT_5_VEND_ADC_PIN 0
#define SLOT_6_VEND_ADC_PIN 1

#define SLOT_1_EMPTY_ADC 0 
#define SLOT_2_EMPTY_ADC 0 
#define SLOT_3_EMPTY_ADC 2 
#define SLOT_4_EMPTY_ADC 2 
#define SLOT_5_EMPTY_ADC 2 
#define SLOT_6_EMPTY_ADC 2 

#define SLOT_1_EMPTY_ADC_PIN 2
#define SLOT_2_EMPTY_ADC_PIN 3
#define SLOT_3_EMPTY_ADC_PIN 0
#define SLOT_4_EMPTY_ADC_PIN 1
#define SLOT_5_EMPTY_ADC_PIN 2
#define SLOT_6_EMPTY_ADC_PIN 3


/* Sensors */
#define AMP_MEETER_ANALOG_PIN    A0

OneWire oneWire(15);
DallasTemperature internalTempSensor(&oneWire);
Adafruit_BME280 bme;

VendSlot  slot1;
VendSlot  slot2;
VendSlot  slot3;
VendSlot  slot4;
VendSlot  slot5;
VendSlot  slot6;

#define ADC_COUNT 3
Adafruit_ADS1115* adcs[ADC_COUNT];
Adafruit_ADS1115  adc0(0x48); 
Adafruit_ADS1115  adc1(0x49); 
Adafruit_ADS1115  adc2(0x4A); 



Machine::Machine(LcdDisplay *lcd_display){
    _display = lcd_display;
    _internalTempSensor = &internalTempSensor;
    _externalBMESensor = &bme;
    _BMEStatus = false; 
    _machine_name = String("Beer Machine");
}

void Machine::init(){
    DEBUG_PRINTLN("Machine::init");

    adcs[0] = &adc0;
    adcs[1] = &adc1;
    adcs[2] = &adc2;
    for (int i = 0; i < ADC_COUNT; i++){
      (*adcs[i]).begin();
    }

    slot1.setup(1, SLOT_1_DIO_PIN, adcs[SLOT_1_VEND_ADC], SLOT_1_VEND_ADC_PIN, adcs[SLOT_1_EMPTY_ADC], SLOT_1_EMPTY_ADC_PIN, _display);
    slot2.setup(2, SLOT_2_DIO_PIN, adcs[SLOT_2_VEND_ADC], SLOT_2_VEND_ADC_PIN, adcs[SLOT_2_EMPTY_ADC], SLOT_2_EMPTY_ADC_PIN, _display);
    slot3.setup(3, SLOT_3_DIO_PIN, adcs[SLOT_3_VEND_ADC], SLOT_3_VEND_ADC_PIN, adcs[SLOT_3_EMPTY_ADC], SLOT_3_EMPTY_ADC_PIN, _display);
    slot4.setup(4, SLOT_4_DIO_PIN, adcs[SLOT_4_VEND_ADC], SLOT_4_VEND_ADC_PIN, adcs[SLOT_4_EMPTY_ADC], SLOT_4_EMPTY_ADC_PIN, _display);
    slot5.setup(5, SLOT_5_DIO_PIN, adcs[SLOT_5_VEND_ADC], SLOT_5_VEND_ADC_PIN, adcs[SLOT_5_EMPTY_ADC], SLOT_5_EMPTY_ADC_PIN, _display);
    slot6.setup(6, SLOT_6_DIO_PIN, adcs[SLOT_6_VEND_ADC], SLOT_6_VEND_ADC_PIN, adcs[SLOT_6_EMPTY_ADC], SLOT_6_EMPTY_ADC_PIN, _display);
  
    slots[0] = &slot1;
    slots[1] = &slot2;
    slots[2] = &slot3;
    slots[3] = &slot4;
    slots[4] = &slot5;
    slots[5] = &slot6;
    
    DallasTemperature dt  = *_internalTempSensor;
    Adafruit_BME280   bme = *_externalBMESensor;
    dt.begin(); 
    if (!bme.begin()) {
        ERROR_PRINTLN(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
        _BMEStatus = false;
    }else{
        _BMEStatus = true;
    } 
}

internalSensorData Machine::read_internal(){
    DEBUG_PRINTLN(F("Machine::read_internal"));
    DallasTemperature s = *_internalTempSensor;
    float c = s.getTempCByIndex(0);
    float f = (c*1.8)+32.0f;

    INFO_PRINT(F("Machine::read_internal Temp in Celcius:    "));
    INFO_PRINTLN(c, 4);
    INFO_PRINT(F("Machine::read_internal Temp in Fahrenheit: "));
    INFO_PRINTLN(f, 4);
    return {f, c};
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
    DEBUG_PRINTLN(F("Before de-referance"));
    Adafruit_BME280 s = *_externalBMESensor;
    
    float c    = s.readTemperature();
    float f    = (c*1.8)+32.0f;
    DEBUG_PRINTLN(F("after de-referance"));
    float hPa  = s.readPressure() / 100.0F;
    float inHg = (hPa / 3386.0F) * 100.0F;
    float h    = s.readHumidity();
  
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
