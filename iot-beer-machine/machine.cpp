//#define MACHINE

#ifdef MACHINE

#include "machine.h"
#include "yhdc.h"
#include "logging.h"

/*
 * This file holds values that are unique to a beer machine wireing.
 * This abstraction is needed to support a test harness which may
 * be wired differently than the actual beer machine.
 */


#define ADC_0_ADDRESS 0x48
#define ADC_1_ADDRESS 0x49
#define ADC_2_ADDRESS 0x4A

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



Machine::Machine(DallasTemperature *internalTempSensor, Adafruit_BME280 *bme){
    _machine_name = String("Machine");
}

void Machine::init(){
    DEBUG_PRINTLN("Machine::init");
    //   internalTempSensor.begin(); 
    //   if (!bme.begin()) {
    //       l_display.disply_msg("Cloud not find BME280 sensor!", 3000);
    //       ERROR_PRINTLN(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
    //   }
}

internalSensorData Machine::read_internal(){
    DEBUG_PRINTLN(F("Machine::read_internal"));
    return {
        0.856F,        /* Celcius         */
        33.45774F,     /* Fahrenhite      */
    }
}
externalSensorData Machine::read_external(){
    DEBUG_PRINTLN(F("Machine::read_external"));
    return {
        22.54944F,      /* Celcius         */
        72.589F,        /* Fahrenhite      */
        81.7372050F,    /* Presure in hPa  */
        32.81447F,      /* Presure in inHG */
        95.17544F       /* Humidity        */
    }

}  
powerSensorData Machine::read_power_usage(){
    DEBUG_PRINTLN(F("Machine::read_power_usage"));
    return {
        4.25218F,       /* Amps            */
        467.73908F,     /* Watts           */
    }
}
String Machine::name()
{
    DEBUG_PRINT(F("Machine::name:"));
    DEBUG_PRINTLN(_machine_name)
    return _machine_name;  
}

#endif


// void print_external_temp(){
//   float c = bme.readTemperature();
//   float f = (c*1.8)+32.0f;
//   float hPa = bme.readPressure() / 100.0F;
//   float inHg = (hPa / 3386.0F) * 100.0F;
//   float h = bme.readHumidity();
  
//   l_display.set_external_temp(f);
//   l_display.set_external_humidity(h);
//   l_display.set_external_inHg(inHg);
  
//   INFO_PRINT(F("Exterinal Temp: "));
//   INFO_PRINT(c,2);
//   INFO_PRINT(F(" *C  "));
//   INFO_PRINT(f,2);
//   INFO_PRINTLN(F(" *F"));
  
//   INFO_PRINT(F("Exterinal Pressure: "));
//   INFO_PRINT(hPa, 4);
//   INFO_PRINT(F(" hPa "));
//   INFO_PRINT(inHg, 4);
//   INFO_PRINTLN(F(" inHg"));
  
//   INFO_PRINT(F("Exterinal Humidity: "));
//   INFO_PRINT(h, 4);
//   INFO_PRINTLN(F("%"));
// }


// void print_internal_temp(){
//   internalTempSensor.requestTemperatures(); 
//   float c = internalTempSensor.getTempCByIndex(0);
//   float f = internalTempSensor.toFahrenheit(c);
//   l_display.set_internal_temp(f);
//   INFO_PRINT(F("Internal Temp in Celcius: "));
//   INFO_PRINTLN(c, 4);
//   INFO_PRINT(F("Internal Temp in Fahrenheit: "));
//   INFO_PRINTLN(f, 4);
  
// }

// void print_amps(){
//   float a = readACCurrentValue(A0);
//   float w = a * 110
//   l_display.set_amps(a);
//   l_display.set_watts(w);
//   INFO_PRINT(F("Amps: "));
//   INFO_PRINT(a, 2);
//   INFO_PRINT(F(" Watts: "));
//   INFO_PRINTLN(w, 1);
// }
