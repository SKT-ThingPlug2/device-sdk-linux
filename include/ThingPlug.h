/**
 * @file ThingPlug.h
 *
 * @brief ThingPlug API header
 *
 * Copyright (C) 2016. SK Telecom, All Rights Reserved.
 * Written 2016, by SK Telecom
 */

#ifndef _THINGPLUG_H_
#define _THINGPLUG_H_

/*
 ****************************************
 * Enumerations
 ****************************************
 */


/*
 ****************************************
 * Structure Definition
 ****************************************
 */


/*
 ****************************************
 * Type Definition
 ****************************************
 */
typedef void tpMQTTConnectedCallback(int result);

typedef void tpMQTTSubscribedCallback(int result);

typedef void tpMQTTDisconnectedCallback(int result);

typedef void tpMQTTConnectionLostCallback(char* cause);

typedef void tpMQTTMessageDeliveredCallback(int token);

typedef void tpMQTTMessageArrivedCallback(char* topic, char* payload, int payloadLen);

/*
 ****************************************
 * Major Function
 ****************************************
 */
int tpMQTTSetCallbacks(tpMQTTConnectedCallback* cc, tpMQTTSubscribedCallback* sc, tpMQTTDisconnectedCallback* dc, 
    tpMQTTConnectionLostCallback* clc, tpMQTTMessageDeliveredCallback* mdc, tpMQTTMessageArrivedCallback* mac);

int tpSDKCreate(char* host, int port, int keepalive, char* userName, char* password, int enableServerCertAuth, 
    char* subscribeTopic[], int subscribeTopicSize, char* publishTopic, char* clientID);

int tpMQTTIsConnected();

int tpMQTTDisconnect();

void tpSDKDestroy();

#endif //_THINGPLUG_H_

