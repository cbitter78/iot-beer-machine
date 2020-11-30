#ifndef HOLD_ON_H
#define HOLD_ON_H

#include <delay.h>
#include <Arduino.h>

#define HOLD_ON_MAX_MILLIS 4294967295

typedef void (*HoldOnCallBack)(); 

class HoldOn {

public:
  HoldOn(unsigned long wait_interval, HoldOnCallBack cb);
  void ReadyYet();  

protected:
  HoldOnCallBack _call_back;
  unsigned long _wait_interval;
  unsigned long _ready;
};

#endif /* HOLD_ON_H */ 
