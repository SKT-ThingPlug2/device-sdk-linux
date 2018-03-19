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

int tempPin = 0;
int LightPin = 1;
int LedGPIO = 123;

int led_init(int pin)
{
    char cmd[64];
    sprintf(cmd,"echo %d > /sys/class/gpio/export", pin);
    system(cmd);
    sprintf(cmd,"echo out > /sys/class/gpio/gpio%d/direction", pin);
    system(cmd);
    return 0;
}

int led_switch(int status, int pin)
{
    char cmd[64];
    sprintf(cmd,"echo %d > /sys/class/gpio/gpio%d/value", status, pin);
    system(cmd);
    return 0;
}

int analogRead(int pin) {
    FILE * fd;
    char fName[64];
    char val[8];

    // open value file
    sprintf(fName,"/sys/devices/126c0000.adc/iio:device0/in_voltage%d_raw",pin);
    if((fd = fopen(fName, "r")) == NULL) {
	printf("Error: can't open analog voltage value\n");
	return 0;
    }
    fgets(val, 8, fd);
    fclose(fd);
    return atoi(val);
}

int TempRead(char *data, int *len)
{
    int sensorVal = analogRead(tempPin);
    float temp = sensorVal * 5.0;
    temp = temp / 1024.0;
    temp = (temp - 0.5) * 100;
    temp = temp * 9 / 5 + 32.0;
    temp = temp / 100;
    printf("temp is %.2f\n", temp);

    sprintf(data,"%.3f",temp);
    *len = strlen(data);

    return 0;
}

int LightRead(char *data,int *len)
{
    int light = analogRead(LightPin);
    printf("light is %d\n", light);

    sprintf(data,"%d",light);
    *len = strlen(data);

    return 0;
}


void SMAGetData(char *sensorType, char** output, int *len)
{
    srand(time(NULL));
    int random_num = rand();

    if( SMA_STRCMP(sensorType,"batterystate") ) {
	*output = strdup("1");
	*len = strlen(*output);
    } else if( SMA_STRCMP(sensorType, "temp1") ) {
        char str_val[8];
        TempRead(str_val, len);
	*output = strdup(str_val);
    } else if( SMA_STRCMP(sensorType, "humi1") ) {
	*output = strdup("XX");
	sprintf(*output, "%2d", (random_num%10+40));
	*len = strlen(*output);
    } else if( SMA_STRCMP(sensorType, "light1") ) {
        char str_val[8];
        LightRead(str_val, len);
	*output = strdup(str_val);
    } else {
	*output = NULL;
    }
}

static int gLedStatus = 0;
static int led_init_flag = 0;
int RGB_LEDControl(int color)
{
    if(led_init_flag == 0) {
        led_init(LedGPIO);
        led_init_flag = 1;
    }
    int rc = 0;
    // if( strncmp("red", color, sizeof("red")) == 0 ) {r=1,g=2,b=3,c=5,m=4,y=6,w=7,off=0
    if( color == 0 ) {
	rc = led_switch(0, LedGPIO);
    } else {
	rc = led_switch(1, LedGPIO);
    }
    return rc;
} 

int RGB_LEDStatus() 
{
    return gLedStatus;
}

