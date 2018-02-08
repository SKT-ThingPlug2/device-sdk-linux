/**
 * @file SMA.c
 *
 * @brief SensorManagementAgent Process
 *
 * Copyright (C) 2016. SKT, All Rights Reserved.
 * Written 2016,by SKT
 */

 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include "SMA.h"
 
 #define SMA_STRCMP(x,y) (strncmp(x,y,strlen(y)) == 0)&&(strlen(x) == strlen(y))
 
 void SMAGetData(char *sensorType, char** output, int *len)
 {
     srand(time(NULL));
     int random_num = rand();
 
     if( SMA_STRCMP(sensorType,"batterystate") ) {
         *output = strdup("1");
         *len = strlen(*output);
     } else if( SMA_STRCMP(sensorType, "temp1") ) {
     *output = strdup("XX.XX");
     sprintf(*output, "%.2f", (random_num%5000+2000)/100.0);
             *len = strlen(*output);
     } else if( SMA_STRCMP(sensorType, "humi1") ) {
     *output = strdup("XX");
     sprintf(*output, "%2d", (random_num%10+40));
             *len = strlen(*output);
     } else if( SMA_STRCMP(sensorType, "light1") ) {
     *output = strdup("XXX");
     sprintf(*output, "%3d", (random_num%50+200));
             *len = strlen(*output);
         } else {
         *output = NULL;
     }
 }
 
 static int ledStatus = 0;
 
 int RGB_LEDControl(int color)
 {
     // r=1,g=2,b=3,c=5,m=4,y=6,w=7,off=0
     if( color == 1 ) {
         ledStatus = 1;
     } else if( color == 2 ) {
         ledStatus = 2;
     } else if( color == 3 ) {
         ledStatus = 3;
     } else if( color == 4 ) {
         ledStatus = 4;
     } else if( color == 5 ) {
         ledStatus = 5;
     } else if( color == 6 ) {
         ledStatus = 6;
     } else if( color == 7 ) {
         ledStatus = 7;
     } else if( color == 0 ) {
         ledStatus = 0;
     } else {
         return -1;
     }
     return 0;
 }
 
 
 int RGB_LEDStatus() 
 {
     return ledStatus;
 }