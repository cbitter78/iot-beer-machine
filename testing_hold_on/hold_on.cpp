#include "hold_on.h"

HoldOn::HoldOn(unsigned long wait_interval, HoldOnCallBack cb){
    unsigned long t = millis();
    _wait_interval = wait_interval;
    _ready = t + _wait_interval;
    _call_back = cb;
}

void HoldOn::ReadyYet(){
    /*
     * Check to see if time is greater then _ready.   Depending on what else
     * is going on in your program the timing may be off.   This class does 
     * not try to fix that.  It just runs the _call_back() method if _ready
     * is some time in the past.  For many things this is good enough.
     */
    unsigned long t = millis();
    if (_ready <= t){ 
      _call_back();  
  
     /* 
      * Here we need to reset the _ready value.  To do this
      * we need to ensure we do not exceed the upper boundry 
      * of mills() which is a unsigned long.   When mills() hits
      * the max of unsinged long it will reset to zero.  Therefore
      * if max subtract our next wait time is less then mills() then 
      * we know it will roll over soon.  So we just assume mills()
      * will reset to zero soon and just set _ready = _wait_interval
      * 
      * This could double the time the process waits in this case.
      * but this will happen only every 50 days.  ¯\_(ツ)_/¯ 
      * for what this will be used with, I don't much care.
      */ 
      t = millis();
      _ready = (t > (HOLD_ON_MAX_MILLIS - _wait_interval) ) ? _wait_interval : (t + _wait_interval);    
    }
}
