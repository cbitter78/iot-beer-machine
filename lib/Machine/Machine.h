#ifndef MACHINE_H
#define MACHINE_H

#include <VendSlot.h>

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
// Yellow, Red, Black
struct powerSensorData {
    float Amps;
    float Watts;
};

class Machine {

public:
  Machine(LcdDisplay *lcd_display);
  void init();
  internalSensorData read_internal();
  externalSensorData read_external();  
  powerSensorData    read_power_usage();
  void update_all_slot_status();
  String name();

  static const int SLOT_COUNT = 6;
  VendSlot* slots[SLOT_COUNT];
  

protected:
  bool _BMEStatus;
  String _machine_name;
  LcdDisplay *_display;
};
#endif /* MACHINE_H */ 
