/*****************************************************
 @copyright   Copyright (c) 2010 DFRobot Co.Ltd (https://www.dfrobot.com)
 @licence     The MIT License (MIT)
 @get from https://wiki.dfrobot.com/Gravity_Analog_AC_Current_Sensor__SKU_SEN0211_#target_3

 Created 2016-3-10
 By berinie Chen <bernie.chen@dfrobot.com>

 Revised 2019-8-6
 By Henry Zhao<henry.zhao@dfrobot.com>

 Used and adapted by:
 Charles Bitter (https://github.com/cbitter78/)

 Connection and Diagram can be found here http://bit.ly/3jwthjb
****************************************************/

#include "yhdc.h" 
#include "Arduino.h"

const float Vref = 3300;
const int ACTectionRange = 20; 

  float readACCurrentValue(int ACPin){
  float ACCurrtntValue = 0;
  unsigned int peakVoltage = 0;
  unsigned int voltageVirtualValue = 0; //Vrms
  for (int i = 0; i < 5; i++){
   peakVoltage += analogRead(ACPin); //read peak voltage
   delay(1);
  }
  peakVoltage = peakVoltage / 5;
  voltageVirtualValue = peakVoltage * 0.707; //change the peak voltage to the Virtual Value of voltage
  
  // The circuit is amplified by 2 times, so it is divided by 2.
  // https://openenergymonitor.org/forum-archive/node/1186.html
  voltageVirtualValue = (voltageVirtualValue * Vref / 1024) / 2;
  
  ACCurrtntValue = voltageVirtualValue * ACTectionRange;
  
  return ACCurrtntValue/1000;
} 
