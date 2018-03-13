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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <termios.h>
#include <signal.h>

#include "SMA.h"

#define SMA_STRCMP(x,y) (strncmp(x,y,strlen(y)) == 0)&&(strlen(x) == strlen(y))

int DS18B20_GetData(char *data, int *len);
int HTU21D_GetData(char *data, int *len);
int BH1750_GetData(char *data,int *len);

void SMAGetData(char *sensorType, char** output, int *len)
{
    if( SMA_STRCMP(sensorType,"batterystate") ) {
        *output = strdup("1");
        *len = strlen(*output);
    } else if( SMA_STRCMP(sensorType, "temp1") ) {
        char data[64];
        DS18B20_GetData(data, len);
        *output = strdup(data);
    } else if( SMA_STRCMP(sensorType, "humi1") ) {
        char data[64];
        HTU21D_GetData(data, len);
        *output = strdup(data);
    } else if( SMA_STRCMP(sensorType, "light1") ) {
        char data[64];
        BH1750_GetData(data, len);
        *output = strdup(data);
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


// >>>>>>>>>>>>>>>>>>>>>>>>  connection type : 1-wire  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/*
 **************************************** 
 * DS18B20 : temperature driver
 **************************************** 
 */
#define MASTER_FILENAME  "cat /sys/bus/w1/devices/w1_bus_master1/w1_master_slaves"
#define MASTER_FILENAME  "cat /sys/bus/w1/devices/w1_bus_master1/w1_master_slaves"
#ifdef DUMMY_DATA
static char gDummyTemp[10][20] =
{"20.1","20.2","20.3","20.4","20.5","20.1","20.3","20.0","20.1","20.1"};
static int gDummyTempIndex = 0;
#endif
char slavePath[256];

int getTemperatureValue(char *data)
{
    int value = 0;
    if(data == NULL) {
        return 0;
    }
    char dump[128] = "";
    memcpy(dump, data, strlen(data));
    char *ptr;
    //Extract temperature value
    // fprintf(stderr, ">>>>>>>>>>>>>> temp raw data : %s\n", dump);
    ptr = strtok(dump,"=");
    if( ptr == NULL ) goto error_ret;
    ptr = strtok(NULL,"=");
    if( ptr == NULL ) goto error_ret;
    ptr = strtok(NULL,"=");
    if( ptr == NULL ) goto error_ret;

    value = atoi(ptr);
    return value;

error_ret:
    value = 999999;
    return value;
}

int DS18B20_Create(void)
{
#ifdef DUMMY_DATA
    return 0;
#else
    FILE *gW1MasterFd;
    char slaveName[256];
    int ret = 0;
    gW1MasterFd = popen(MASTER_FILENAME, "r");
    if(gW1MasterFd == NULL) { 	
        return -1;
    }
    (void)memset(slaveName,0,256);
    fgets(slaveName,255,gW1MasterFd);
    slaveName[strlen(slaveName)-1] = '\0';
    sprintf(slavePath,"/sys/bus/w1/devices/");
    strcat(slavePath,slaveName);
    strcat(slavePath,"/w1_slave");
    pclose(gW1MasterFd);
    return ret;
#endif
}

int DS18B20_GetData(char *data, int *len)
{
#ifdef DUMMY_DATA  
    strcpy(data,gDummyTemp[gDummyTempIndex]);
    *len = strlen(data);
    gDummyTempIndex++;
    gDummyTempIndex %= 10;
    return 0;
#else
    static int init_flag = 0;
    float value;
    int  readByte = 0;
    char buf[128] = "";
    FILE *fp;

    if(init_flag == 0) {
        DS18B20_Create();
        init_flag = 1;
    }

    fp = fopen(slavePath, "r");
    if(fp == NULL) {
        sprintf(data, "N/A:OPEN");
        *len = strlen(data);
        return 0;
    }

    while((readByte = fread(buf, 1, sizeof buf, fp)) > 0) {
        ;;
    }

    fclose(fp);

    value = (float)getTemperatureValue(buf);
    if( value > 999990) {
        sprintf(data,"N/A:DATAERROR");
        *len = strlen(data);
        return 0;
    }

    value = value / 1000;

    sprintf(data,"%.3f",value);
    if( data[0] == '0' && data[1] == '.' && data[2] =='0' && data[3] == '0') {
        sprintf(data,"N/A:ERROR");
    }

    *len = strlen(data);

    return 0;
#endif
}

// >>>>>>>>>>>>>>>>>>>>>>>>  connection type : I2C  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/*
 **************************************** 
 * HTU21D : humidity driver
 **************************************** 
 */

#define HUMIDITY_ADDRESS 0x40
#define ILLUMINANCE_ADDRESS 0x23
#define DEVICE_REG_READ 0xE7
#define DEVICE_REG_HUMD 0xE5
#define ADAPTER_NUMBER 1
#define DEV_I2C_NUMBER "/dev/i2c-%d"


#ifdef DUMMY_DATA
static char gDummyHumi[10][20] =
{"77.8","60.6","60.6","50.5","50.5","77.7","60.6","50.0","70.7","70.7"};
static int gDummyHumiIndex = 0;
#else
static int gHTU21DFd;
#endif //DUMMY_DATA

int HTU21D_GetData(char *data, int *len)
{
#ifdef DUMMY_DATA  
	strcpy(data,gDummyHumi[gDummyHumiIndex]);
	*len = strlen(data);
	gDummyHumiIndex++;
	gDummyHumiIndex %= 10;
	return 0;
#else
	char fileName[20];
	char  buf[3];
	int   sensorData[3];
	int   readByte  = 0;
	float value     = 0;
	int   writeByte = 0;

	memset(sensorData, 0, 3);
	memset(buf,0,3);
	memset(fileName, 0, 20);

	snprintf(fileName, 19, DEV_I2C_NUMBER, ADAPTER_NUMBER);
	gHTU21DFd = open(fileName, O_RDWR);
	if(gHTU21DFd < 0) {
		sprintf(data,"N/A:OPEN");
		*len = strlen(data);
		return 0;
	}

	if(ioctl(gHTU21DFd, I2C_SLAVE, HUMIDITY_ADDRESS) < 0) {
		sprintf(data,"N/A:IOCTL");
		*len = strlen(data);
		close(gHTU21DFd);
       	return 0;
	}

	memset(buf,0,3);
	buf[0] = DEVICE_REG_READ;
	writeByte = write(gHTU21DFd,buf,1);
	if(writeByte < 0) {
		sprintf(data,"N/A:WRITE");
		*len = strlen(data);
		close(gHTU21DFd);
		return 0;
	}

	buf[0] = DEVICE_REG_HUMD;
	writeByte = write(gHTU21DFd,buf,1);
	if(writeByte < 0) {
		sprintf(data,"N/A:WRITE");
		*len = strlen(data);
		close(gHTU21DFd);
		return 0;
	}

	readByte = read(gHTU21DFd, buf, 3);
	if(readByte <= 0) {
		sprintf(data,"N/A:READ");
		*len = strlen(data);
		close(gHTU21DFd);
		return 0;
	}

	lseek(gHTU21DFd,0,SEEK_SET);

	sensorData[0] = (int)buf[0];
	sensorData[1] = (int)buf[1];

	value = (((125.0*(((256*sensorData[0]) + sensorData[1])&0xFFFC))/65536) - 6);	
	sprintf(data,"%.1f",value);
	*len = strlen(data);

	if(gHTU21DFd < 0) {
		sprintf(data,"N/A:CLOSE");
		*len = strlen(data);
		return 0;
	}
	close(gHTU21DFd);
	return 0;
#endif //DUMMY_DATA
}

/*
 **************************************** 
 * BH1750 : light driver
 **************************************** 
 */

#ifdef DUMMY_DATA
static char gDummyLight[10][20] =
{"556","556","556","556","556","656","656","656","656","656"};
static int gDummyLightIndex = 0;
#else
static int gBH1750Fd;
#endif //DUMMY_DATA

int BH1750_GetData(char *data,int *len)
{
#ifdef DUMMY_DATA  
	strcpy(data,gDummyLight[gDummyLightIndex]);
	*len = strlen(data);
	gDummyLightIndex++;
	gDummyLightIndex %= 10;
	return 0;
#else
	char fileName[20];
	char buf[2];
	char sensorData[2];
	int   writeByte = 0;
	int   readByte = 0;
	float value;

	memset(fileName, 0, 20);
	memset(buf, 0, 2);
	snprintf(fileName, 19, DEV_I2C_NUMBER, ADAPTER_NUMBER);

	gBH1750Fd = open(fileName, O_RDWR);
	if(gBH1750Fd < 0) {
		sprintf(data,"N/A:OPEN");
		*len = strlen(data);
		return 0;
	}

	if(ioctl(gBH1750Fd, I2C_SLAVE, ILLUMINANCE_ADDRESS) < 0) {
		sprintf(data,"N/A:IOCTL");
		*len = strlen(data);
		close(gBH1750Fd);
        return 0;
    }

	(void)memset(buf,0,2); 
	(void)memset(sensorData,0,2);
	buf[0] = 0x11;
	writeByte = write(gBH1750Fd, buf, 1);
	if(writeByte < 0) {
		sprintf(data,"N/A:WRITE");
		*len = strlen(data);
		close(gBH1750Fd);
		return 0;
	}

	(void)memset(buf,0,2); 
	readByte = read(gBH1750Fd, buf, 2); 
	if(readByte <= 0) {
		sprintf(data,"N/A:READ");
		*len = strlen(data);
		close(gBH1750Fd);
		return 0;
	}

	lseek(gBH1750Fd,0,SEEK_SET);

	(void)memset(buf,0,2);
	readByte = read(gBH1750Fd, buf, 2); 
	if(readByte <= 0) {
		sprintf(data,"N/A:READ");
		*len = strlen(data);
		close(gBH1750Fd);
		return 0;
	}

	sensorData[0] = (int)buf[0];
	sensorData[1] = (int)buf[1];

	value = (((256*sensorData[0]) + sensorData[1]) / 1.2);
	sprintf(data,"%.0f",value);
	*len = strlen(data);

	if(gBH1750Fd < 0) {
		sprintf(data,"N/A:CLOSE");
		*len = strlen(data);
		return 0;
	}
	close(gBH1750Fd);
	return 0;
#endif //DUMMY_DATA
}

