#ifndef MACHINE_H
#define MACHINE_H

#include <Adafruit_BME280.h>
#include <DallasTemperature.h>


struct internalSensorData {
    float CelciusTemp;
    float FahrenheitTemp;
};

struct externalSensorData {
    float CelciusTemp;
    float FahrenheitTemp;
    float Pressure_hPa;
    float Pressure_inHG;
    float Humidity;
};

struct powerSensorData {
    float Amps;
    float Watts;
};

class Machine {

public:
  Machine(DallasTemperature *internalTempSensor, Adafruit_BME280 *bme);
  void init();
  internalSensorData read_internal();
  externalSensorData read_external();  
  powerSensorData read_power_usage();
  String name();

protected:
  DallasTemperature *_internalTempSensor;  
  Adafruit_BME280 *_externalBMESensor;  
  String _machine_name;
};
#endif /* MACHINE_H */ 
