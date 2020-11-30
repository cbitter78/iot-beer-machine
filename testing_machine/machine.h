#ifndef MACHINE_H
#define MACHINE_H


#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include "lcd_display.h"

struct internalSensorData {
    float cTemp;
    float fTemp;
};


class Machine {

public:
  Machine(DallasTemperature *internalTempSensor, Adafruit_BME280 *bme);
  float read_internal();
  float read_power_usage();
  float read_external();  


protected:

  DallasTemperature *_internalTempSensor;  
  Adafruit_BME280 *_externalBMESensor;  
  
  /* Methods */

};


#endif /* MACHINE_H */ 