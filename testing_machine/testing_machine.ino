#include <Adafruit_BME280.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "logging.h"
#include "machine.h"   

/* Sensors */
OneWire oneWire(15);
DallasTemperature internalTempSensor(&oneWire);
Adafruit_BME280 bme;

Machine m(&internalTempSensor, &bme);

void setup(void){
  Serial.begin(115200);
  while(! Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Setting up");
  m.init();
}

void loop(void){
    Serial.println("Going to read the value");
    flash_built_in_led();

    INFO_PRINT(F("Reading Sensor Data From: "));
    INFO_PRINTLN(m.name());
    internalSensorData id = m.read_internal();
    externalSensorData ed = m.read_external();
    powerSensorData    pd = m.read_power_usage();

    INFO_PRINTLN(F("-----> Internal Data <-----"));
    INFO_PRINT("   Celcius Temp: ");
    INFO_PRINTLN(id.CelciusTemp, 2);
    INFO_PRINT("   Fahrenheit Temp: ");
    INFO_PRINTLN(id.FahrenheitTemp, 2);
    INFO_PRINTLN(F("----------------------------\n\n"));

    INFO_PRINTLN(F("-----> External Data <-----"));
    INFO_PRINT("   Celcius Temp: ");
    INFO_PRINTLN(ed.CelciusTemp, 2);
    INFO_PRINT("   Fahrenheit Temp: ");
    INFO_PRINTLN(ed.FahrenheitTemp, 2);
    INFO_PRINT("   Pressure hPa: ");
    INFO_PRINTLN(ed.Pressure_hPa, 2);
    INFO_PRINT("   Pressure inHG: ");
    INFO_PRINTLN(ed.Pressure_inHG, 2);
    INFO_PRINT("   Humidity: ");
    INFO_PRINTLN(ed.Humidity, 2);
    INFO_PRINTLN(F("----------------------------\n\n"));

    INFO_PRINTLN(F("-----> Power Data <-----"));
    INFO_PRINT("   Amps: ");
    INFO_PRINTLN(pd.Amps , 2);
    INFO_PRINT("   Watts: ");
    INFO_PRINTLN(pd.Watts, 2);
    INFO_PRINTLN(F("----------------------------\n\n"));
}

void flash_built_in_led(){
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);                     
  digitalWrite(LED_BUILTIN, LOW);  
  delay(500);    
}