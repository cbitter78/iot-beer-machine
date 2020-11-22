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

#ifndef _YHDC_H
#define _YHDC_H

/* Vref:
 *     The referance voltage.
 */
extern const float Vref;


/* ACPin:
 *     THe Current Sensor Amp Rating.  (20A,30A,50A,100A)
 */
extern const int ACTectionRange; 


/* readACCurrentValue()
 *    Pass in the analog pin number where the AC sensor interfaces with the microcontroler
 */
float readACCurrentValue(int ACPin);

#endif // _BASE64_H
