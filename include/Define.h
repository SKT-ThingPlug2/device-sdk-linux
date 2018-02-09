/**
 * @file Simple.h
 *
 * @brief The header file for Defines
 *
 * Copyright (C) 2017. SK Telecom, All Rights Reserved.
 * Written 2017, by SK Telecom
 */
#ifndef _DEFINE_H_
#define _DEFINE_H_

/*
 ****************************************
 * RPC Procedure Definition
 ****************************************
 */
#define RPC_RESET                   "tp_reset"
#define RPC_REBOOT                  "tp_reboot"
#define RPC_UPLOAD                  "tp_upload"
#define RPC_DOWNLOAD                "tp_download"
#define RPC_SOFTWARE_INSTALL        "tp_install"
#define RPC_SOFTWARE_REINSTALL      "tp_reinstall"
#define RPC_SOFTWARE_UNINSTALL      "tp_uninstall"
#define RPC_SOFTWARE_UPDATE         "tp_update"
#define RPC_FIRMWARE_UPGRADE        "tp_fwupgrade"
#define RPC_CLOCK_SYNC              "tp_clocksync"
#define RPC_SIGNAL_STATUS_REPORT    "tp_sigstatusreport"
#define RPC_USER                    "tp_user"
#define RPC_REMOTE                  "tp_remote"

/*
 ****************************************
 * Definition
 ****************************************
 */
/** subscribe **/
#define SUBSCRIBE                   "subscribe"
/** unsubscribe **/
#define UNSUBSCRIBE                 "unsubscribe"
/** device **/
#define DEVICE                      "device"
/** service name **/
#define SERVICE_NAME                "serviceName"
/** device name **/
#define DEVICE_NAME                 "deviceName"

/** devicesOnline(unsigned char) **/
#define DEVICES_ONLINE              "devicesOnline"
/** attributesUploaded(unsigned char) **/
#define ATTRIBUTES_UPLOADED         "attributesUploaded"
/** telemetryUploaded(unsigned char) **/
#define TELEMETRY_UPLOADED          "telemetryUploaded"
/** timestamp(unsigned long) **/
#define TIMESTAMP                   "ts"

/** device descriptor(string) **/
#define DEVICE_DESCRIPTOR           "deviceDescriptor"

/** command(string) **/
#define CMD                         "cmd"
/** command ID(int) **/
#define CMD_ID                      "cmdId"
/** device ID(string) **/
#define DEVICE_ID                   "deviceId"
/** telemetry(string array) **/
#define TELEMETRY                   "telemetry"
/** attribute(string array) **/
#define ATTRIBUTE                   "attribute"

/** sensor node ID(string) **/
#define SENSOR_NODE_ID              "sensorNodeId"
/** is target all(boolean) **/
#define IS_TARGET_ALL               "isTargetAll"

/** RPC response(JSON) **/
#define RPC_RSP                     "rpcRsp"
/** JSON RPC version(string) **/
#define JSONRPC                     "jsonrpc"
/** Identifier(int) **/
#define ID                          "id"
/** method(string) **/
#define METHOD                      "method"
/** control result **/
#define RESULT                      "result"
/** error(JSON) **/
#define ERROR                       "error"
/** error code(int) **/
#define CODE                        "code"
/** error message(string) **/
#define MESSAGE                     "message"
/** result status(string) **/
#define STATUS                      "status"
/** result success(string) **/
#define SUCCESS                     "success"
/** result fail(string) **/
#define FAIL                        "fail"


#define TOPIC_TELEMETRY             "v1/dev/%s/%s/telemetry"
#define TOPIC_TELEMETRY_CSV         "v1/dev/%s/%s/telemetry/csv"
#define TOPIC_TELEMETRY_OFFSET      "v1/dev/%s/%s/telemetry/offset"

#define TOPIC_ATTRIBUTE             "v1/dev/%s/%s/attribute"
#define TOPIC_ATTRIBUTE_CSV         "v1/dev/%s/%s/attribute/csv"
#define TOPIC_ATTRIBUTE_OFFSET      "v1/dev/%s/%s/attribute/offset"

#define TOPIC_UP                    "v1/dev/%s/%s/up"

/* CONNACK : 0 Connection Accepted */
#define CONNECTION_ACCEPTED 0
/* CONNACK : 1 Connection Refused, unacceptable protocol version */
#define CONNECTION_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION 1
/* CONNACK : 2 Connection Refused, identifier rejected */
#define CONNECTION_REFUSED_IDENTIFIER_REJECTED 2
/* CONNACK : 3 Connection Refused, Server unavailable */
#define CONNECTION_REFUSED_SERVER_UNAVAILABLE 3
/* CONNACK : 4 Connection Refused, bad user name or password */
#define CONNECTION_REFUSED_BAD_USERNAME_OR_PASSWORD 4
/* CONNACK : 5 Connection Refused, not authorized */
#define CONNECTION_REFUSED_NOT_AUTHORIZED 5

/* Return code: No error. Indicates successful completion */
#define TP_SDK_SUCCESS 0
/* Return code: A generic error code indicating the failure */
#define TP_SDK_FAILURE -1
/* error code -2 is MQTTAsync_PERSISTENCE_ERROR */
#define TP_SDK_MQTT_PERSISTENCE_ERROR -2
/* Return code: The client is disconnected. */
#define TP_SDK_MQTT_DISCONNECTED -3
/* Return code: The maximum number of messages allowed to be simultaneously in-flight has been reached. */
#define TP_SDK_MQTT_MAX_MESSAGES_INFLIGHT -4
/* Return code: An invalid UTF-8 string has been detected. */
#define TP_SDK_MQTT_BAD_UTF8_STRING -5
/* Return code: A NULL parameter has been supplied when this is invalid. */
#define TP_SDK_MQTT_NULL_PARAMETER -6
/**
 * Return code: The topic has been truncated (the topic string includes
 * embedded NULL characters). String functions will not access the full topic.
 * Use the topic length value to access the full topic.
 */
#define TP_SDK_MQTT_TOPICNAME_TRUNCATED -7
/* Return code: A structure parameter does not have the correct eyecatcher and version number. */
#define TP_SDK_MQTT_BAD_STRUCTURE -8
/* Return code: A qos parameter is not 0, 1 or 2 */
#define TP_SDK_MQTT_BAD_QOS -9
/* Return code: All 65535 MQTT msgids are being used */
#define TP_SDK_MQTT_NO_MORE_MSGIDS -10
/* Return code: the request is being discarded when not complete */
#define TP_SDK_MQTT_OPERATION_INCOMPLETE -11
/* Return code: no more messages can be buffered */
#define TP_SDK_MQTT_MAX_BUFFERED_MESSAGES -12
/* Return code: Resource type or Operation is not supported */
#define TP_SDK_NOT_SUPPORTED -13
/* Return code: Parameter is invalid */
#define TP_SDK_INVALID_PARAMETER -14
 
/* topic size */
#define SIZE_TOPIC              128

#endif
