//#define MOCK_MACHINE

#ifdef MOCK_MACHINE

#include "machine.h"
#include "logging.h"

/*
This file holds values that are unique to a beer machine wireing.
This abstraction is needed to support a test harness which may
be wired differently than the actual beer machine.

This is the test harness definition do not include this header
file when deploying to the actual beer machine.

*/
#define ADC_0_ADDRESS 0x48
#define ADC_1_ADDRESS 0x48
#define ADC_2_ADDRESS 0x48

#define SLOT_1_DIO_PIN 12
#define SLOT_2_DIO_PIN 11
#define SLOT_3_DIO_PIN 12
#define SLOT_4_DIO_PIN 11
#define SLOT_5_DIO_PIN 12
#define SLOT_6_DIO_PIN 11

#define SLOT_1_VEND_ADC 1 
#define SLOT_2_VEND_ADC 1 
#define SLOT_3_VEND_ADC 1 
#define SLOT_4_VEND_ADC 1 
#define SLOT_5_VEND_ADC 0 
#define SLOT_6_VEND_ADC 0 

#define SLOT_1_VEND_ADC_PIN 0
#define SLOT_2_VEND_ADC_PIN 1
#define SLOT_3_VEND_ADC_PIN 0
#define SLOT_4_VEND_ADC_PIN 1
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
#define SLOT_3_EMPTY_ADC_PIN 2
#define SLOT_4_EMPTY_ADC_PIN 3
#define SLOT_5_EMPTY_ADC_PIN 2
#define SLOT_6_EMPTY_ADC_PIN 3


Machine::Machine(){
    _machine_name = "Mock test rig";
    _BMEStatus = false; 
}

void Machine::init(){
    DEBUG_PRINTLN("MockMachine::init");
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
