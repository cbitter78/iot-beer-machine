#include <Adafruit_MQTT.h>



#define MAX_MILLIS 4294967295
#define MQTT_PING_INTERVAL 30000
unsigned long MQTT_NEXT_PING_TIME;



void mqtt_ping(Adafruit_MQTT_Client *mqtt){
  unsigned long t = millis();
  if (MQTT_NEXT_PING_TIME < t){
    if (t >= MAX_MILLIS - MQTT_PING_INTERVAL){
      /* 
       *  This means mills is about to roll over to 0
       *  To deal with this we will just wait until it does
       *  and then wait the MQTT_PING_INTERVAL
       *  This might double the ping interval but thats ok
       */ 
      MQTT_NEXT_PING_TIME = MQTT_PING_INTERVAL;
    }else{
      MQTT_NEXT_PING_TIME = t + MQTT_PING_INTERVAL;
    }
    (*mqtt).ping();     
  }
}




void flash(int pin){
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
}





