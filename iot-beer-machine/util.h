//
//#include "AdafruitIO_Feed.h"
//
//void init_feeds(AdafruitIO_Feed feeds)
//{
//  
//
//  int n;
//  for ( n=0 ; n<10 ; ++n )
//  {
//    AdafruitIO_Feed f = feeds[n];
//    if (f.exists() == false){
//      Serial.print("Creating Feed:");
//      Serial.println(f.name);
//      f.create();
//    }
//    Serial.print("Setting Location on Feed:");
//    Serial.println(f.name);
//    Serial.println(f.owner);
//    f.setLocation(LOCATION_LAT, LOCATION_LONG, LOCATION_ALT);
//  }
//
//}
