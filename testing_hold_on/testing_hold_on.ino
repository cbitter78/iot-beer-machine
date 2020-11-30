#include <Arduino.h>
#include "hold_on.h"


void hold_on_one();
void hold_on_two();

HoldOn h1(10000, hold_on_one);
HoldOn h2(1000,  hold_on_two);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(5000);
}

void hold_on_one(){
    Serial.println("Hold on function called (Every 10 Seconds)");
}

void hold_on_two(){
    Serial.println("Hold on function called (Every 1 Second)");
}

void loop() {
  h1.ReadyYet();
  h2.ReadyYet();
  flash();
}


void flash(){
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(250);                     
  digitalWrite(LED_BUILTIN, LOW);  
  delay(250);                       
}
