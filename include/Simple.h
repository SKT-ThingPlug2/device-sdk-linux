/**
 * @file Simple.h
 *
 * @brief The header file for Simple API
 *
 * Copyright (C) 2017. SK Telecom, All Rights Reserved.
 * Written 2017, by SK Telecom
 */
#ifndef _SIMPLE_H_
#define _SIMPLE_H_

#include "Define.h"
#include "cJSON.h"

/*
 ****************************************
 * Enumerations
 ****************************************
 */
typedef enum data_type {
    JSON_TYPE_STRING = 0,     // char pointer
    JSON_TYPE_RAW,            // deal with words numerically
    JSON_TYPE_LONG,           // int, long type
    JSON_TYPE_LONGLONG,       // long long type
    JSON_TYPE_DOUBLE,         // double type
    JSON_TYPE_BOOLEAN         // int 0(false) or other(true)
} DATA_TYPE;

typedef enum data_format {
    FORMAT_JSON = 0,          // json format
    FORMAT_CSV,               // csv format
    FORMAT_OFFSET             // offset format
} DATA_FORMAT;

/*
 ****************************************
 * Structure Definition
 ****************************************
 */
 typedef struct
 {
     /** data type **/
     DATA_TYPE type;
     /** JSon name **/
     char* name;
     /** JSon value **/
     void* value;
 } Element;
 
 typedef struct
 {
     /** Element real count **/
     int total;
     /** Element capacity **/
     int capacity;
     /** Element list **/
     Element* element;
 } ArrayElement;
 
 typedef struct
 {
     /** command(string) **/
     char* cmd;
     /** command ID(int) **/
     int cmdId;
     /** JSON RPC version(string) **/
     char* jsonrpc;
     /** request ID from server(int) **/
     int id;
     /** control result(flag) **/
     int result;
     /** result body(ArrayElement) **/
     ArrayElement* resultArray;
 } RPCResponse;
 
 typedef struct
 {
     /** command **/
     char* cmd;
     /** sensor node ID(optional) **/
     char* sensorNodeId;
     /** all sensor node flag **/
     int isTargetAll;
     /** attribute array **/
     const char **attribute;
     int attribute_size;
     /** telemetry array **/
     const char **telemetry;
     int telemetry_size;
     /** command ID **/
     unsigned int cmdId;
 } DeviceSubscribe;
 
/*
 ****************************************
 * Major Function
 ****************************************
 */
int tpSimpleAddData(char* data, unsigned char length);
 
int tpSimpleInitialize(char* serviceID, char* deviceID);

int tpSimpleTelemetry(ArrayElement* telemetry, unsigned char useAddedData);

int tpSimpleAttribute(ArrayElement* attribute);

int tpSimpleResult(RPCResponse* response);

int tpSimpleSubscribe(DeviceSubscribe* subscribe);

int tpSimpleRawTelemetry(char* telemetry, DATA_FORMAT format);

int tpSimpleRawAttribute(char* attribute, DATA_FORMAT format);

int tpSimpleRawResult(char* result);
#endif
