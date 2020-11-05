#include <Adafruit_ADS1015.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MQTT.h>







#define MAX_MILLIS 4294967295
#define MQTT_PING_INTERVAL 30000
unsigned long MQTT_NEXT_PING_TIME;


void lcd_display_msg(String msg, LiquidCrystal_I2C *lcd_20_x_4, int scroll_delay, bool clear_after, int clear_after_delay){
  LiquidCrystal_I2C l = *lcd_20_x_4;
  l.backlight();

  int msg_len = (msg.length() > 80) ? 80 : msg.length();
  l.setCursor(0, 0); /* column, row */
  for (int i = 0; i < msg_len; i++) {

    switch(i) {
     case 20 :
        l.setCursor(0, 1);
        break; 
     case 40:
        l.setCursor(0, 2);
        break; 
    
     case 60:
        l.setCursor(0, 3);
        break;
    }
    l.print(msg[i]);
    delay(scroll_delay);
  }

  if (clear_after){
    delay(clear_after_delay);
    l.clear();
  }
  l.noBacklight();
}


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




/*  
 *  Finds the size of a char* array. 
 *  Works by looking at the memory address of the array
 *  then advancing until we find the array the end \0
 *  This function returns is the number of elements (starts at 1) 
 */
int char_array_size(char *ptr)
{
    int offset = 0;
    int count = 0;

    while (*(ptr + offset) != '\0') /* Until we get to the end of the array */ 
    {
        ++count;
        ++offset;
    }
    return count;
}
