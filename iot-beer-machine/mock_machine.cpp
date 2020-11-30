

#ifndef MACHINE

#include "machine.h"




/*
 * This file holds values that are unique to a beer machine wireing.
 * This abstraction is needed to support a test harness which may
 * be wired differently than the actual beer machine.
 */
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>


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

#endif