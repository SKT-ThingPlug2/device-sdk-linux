/**
 * @file Configuration.h
 *
 * @brief Configuration header for The Samples
 *
 * Copyright (C) 2017. SK Telecom, All Rights Reserved.
 * Written 2017, by SK Telecom
 */

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#define MQTT_HOST                           "(TBD)"
#define MQTT_SECURE_HOST                    "(TBD)"
#define MQTT_PORT                           1883
#define MQTT_SECURE_PORT                    8883
#define MQTT_KEEP_ALIVE                     120
#define MQTT_ENABLE_SERVER_CERT_AUTH        0

#define JSON_FORMAT
// #define CSV_FORMAT
#define SIMPLE_DEVICE_TOKEN                 "(TBD)" // device token(Check with ThingPlug Portal)
#define SIMPLE_SERVICE_NAME                 "(TBD)" // service name(Check with ThingPlug Portal)
#define SIMPLE_DEVICE_NAME                  "(TBD)" // device name(Check with ThingPlug Portal)

#endif // _CONFIGURATION_H_
