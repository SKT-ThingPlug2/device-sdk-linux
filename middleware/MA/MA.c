/**
 * @file MA.c
 *
 * @brief MangementAgent
 *
 * Copyright (C) 2017. SK Telecom, All Rights Reserved.
 * Written 2017, by SK Telecom
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "MA.h"
#include "SRA.h"
#include "SMA.h"

#include "ThingPlug.h"
#include "Simple.h"

#include "Configuration.h"
#include "SKTtpDebug.h"

#define MQTT_CLIENT_ID                      "%s_%s"
#define MQTT_TOPIC_CONTROL_DOWN             "v1/dev/%s/%s/down"

#define TOPIC_SUBSCRIBE_SIZE                1

#define SIZE_RESPONSE_CODE                  10
#define SIZE_RESPONSE_MESSAGE               128
#define SIZE_PAYLOAD                        2048
#define SIZE_CLIENT_ID                      24

static enum PROCESS_STEP
{
    PROCESS_START = 0,
    PROCESS_ATTRIBUTE,
    PROCESS_TELEMETRY,
    PROCESS_END
} mStep;

static enum CONNECTION_STATUS
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED
} mConnectionStatus;

typedef enum thingplug_data_type
{
    TELEMETRY_TYPE,
    ATTRIBUTE_TYPE
} THINGPLUG_DATA_TYPE;

typedef struct
{
    /** device IP address **/
    char deviceIpAddress[30];
    /** gateway IP address **/
    char gatewayIpAddress[30];
} NetworkInfo;

static char mTopicControlDown[SIZE_TOPIC] = "";
static char mClientID[SIZE_CLIENT_ID] = "";

static int mErrorCode = 0;

static void make_attribute(char *data);
static void make_telemetry(char *data);
static char* make_response(RPCResponse *rsp, char* resultBody);
static void send_data(THINGPLUG_DATA_TYPE tpDataType, DATA_FORMAT format, char *data);
static char* get_error();
static void set_error(int errorCode);

void MQTTConnected(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTConnected result : %d", result);
    // if connection failed
    if(result) {
        mConnectionStatus = DISCONNECTED;
    } else {
        mConnectionStatus = CONNECTED;
    }
    SKTDebugPrint(LOG_LEVEL_INFO, "CONNECTION_STATUS : %d", mConnectionStatus);
    set_error(result);
    SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCON=%d", result);
}

void MQTTSubscribed(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTSubscribed result : %d", result);

    if(result == 0) {
        char data[SIZE_PAYLOAD] = "";
        make_attribute(data);
#ifdef JSON_FORMAT
        SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPDAT=1,attribute,0,%s", data);
        send_data(ATTRIBUTE_TYPE, FORMAT_JSON, data);
#elif defined(CSV_FORMAT)
        SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPDAT=1,attribute,1,%s", data);
        send_data(ATTRIBUTE_TYPE, FORMAT_CSV, data);
#endif
        mStep = PROCESS_TELEMETRY;
    }
}

void MQTTDisconnected(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTDisconnected result : %d", result);
    if(result == 0) {
        set_error(-1);
        SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCON=-1");
    } else {
        set_error(0);
        SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCON=0");
    }
}

void MQTTConnectionLost(char* cause) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTConnectionLost result : %s", cause);
    SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCON=-1");
    mConnectionStatus = DISCONNECTED;
}

void MQTTMessageDelivered(int token) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTMessageDelivered token : %d, step : %d", token, mStep);
}

void MQTTMessageArrived(char* topic, char* msg, int msgLen) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTMessageArrived topic : %s, step : %d", topic, mStep);

	if(msg == NULL || msgLen < 1) {
		return;
    }
    char payload[SIZE_PAYLOAD] = "";
    memcpy(payload, msg, msgLen);
    SKTDebugPrint(LOG_LEVEL_INFO, "payload : %s", payload);
    
    cJSON* root = cJSON_Parse(payload);
    if(!root) return;

    cJSON* rpcReqObject = cJSON_GetObjectItemCaseSensitive(root, "rpcReq");
    cJSON* errorObject = cJSON_GetObjectItemCaseSensitive(root, "errorCode");
    // error
    if(errorObject) {
        int errorCode = errorObject->valueint;
        set_error(errorCode);

        SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPERR=1");
        char* ret = get_error();
        SKTDebugPrint(LOG_LEVEL_ATCOM, "result : %s", ret);
        free(ret);
    } 
    // if RPC control

    if(rpcReqObject) {
        int control, rc = -1;
        cJSON* cmdObject = cJSON_GetObjectItemCaseSensitive(root, "cmd");
        cJSON* rpcObject = cJSON_GetObjectItemCaseSensitive(rpcReqObject, "jsonrpc");
        cJSON* idObject = cJSON_GetObjectItemCaseSensitive(rpcReqObject, "id");
        cJSON* paramsObject = cJSON_GetObjectItemCaseSensitive(rpcReqObject, "params");
        cJSON* methodObject = cJSON_GetObjectItemCaseSensitive(rpcReqObject, "method");        
        cJSON* controlObject;
        if(!cmdObject || !idObject || !methodObject) return;        
        char* cmd = cmdObject->valuestring;
        char* rpc = rpcObject->valuestring;
        int id = idObject->valueint;
        char* method = methodObject->valuestring;

        if(!cmd || !method) return;

        char* params = NULL;
        if( paramsObject ) {
            params = cJSON_PrintUnformatted(paramsObject);
            if(params) {
                SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCMD=%s,%d,1,%s", method, id, params);
                free(params);
            }
        } else {
            SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCMD=%s,%d,1,", method, id);
        }

        RPCResponse rsp;
        memset(&rsp, 0, sizeof(RPCResponse));
        rsp.result = 1;
        rsp.cmd = cmd;
        rsp.cmdId = 1;
        rsp.jsonrpc = rpc;
        rsp.id = id;

        // Reserved Procedure for ThingPlug
        if(strncmp(method, RPC_RESET, strlen(RPC_RESET)) == 0) {
            // TODO RESET
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_RESET");

        } else if(strncmp(method, RPC_REBOOT, strlen(RPC_REBOOT)) == 0) {
            // TODO REBOOT
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_REBOOT");
            
        } else if(strncmp(method, RPC_UPLOAD, strlen(RPC_UPLOAD)) == 0) {
            // TODO UPLOAD
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_UPLOAD");
            
        } else if(strncmp(method, RPC_DOWNLOAD, strlen(RPC_DOWNLOAD)) == 0) {
            // TODO DOWNLOAD
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_DOWNLOAD");
            
        } else if(strncmp(method, RPC_SOFTWARE_INSTALL, strlen(RPC_SOFTWARE_INSTALL)) == 0) {
            // TODO SOFTWARE INSTALL
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SOFTWARE_INSTALL");
            
        } else if(strncmp(method, RPC_SOFTWARE_REINSTALL, strlen(RPC_SOFTWARE_REINSTALL)) == 0) {
            // TODO SOFTWARE REINSTALL
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SOFTWARE_REINSTALL");
            
        } else if(strncmp(method, RPC_SOFTWARE_UNINSTALL, strlen(RPC_SOFTWARE_UNINSTALL)) == 0) {
            // TODO SOFTWARE UNINSTALL
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SOFTWARE_UNINSTALL");
            
        } else if(strncmp(method, RPC_SOFTWARE_UPDATE, strlen(RPC_SOFTWARE_UPDATE)) == 0) {
            // TODO SOFTWARE UPDATE
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SOFTWARE_UPDATE");
            
        } else if(strncmp(method, RPC_FIRMWARE_UPGRADE, strlen(RPC_FIRMWARE_UPGRADE)) == 0) {
            // TODO FIRMWARE UPGRADE
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_FIRMWARE_UPGRADE");
            // ATCOM INITIATED
            // DO FIRMWARE UPGRADE here...
        } else if(strncmp(method, RPC_CLOCK_SYNC, strlen(RPC_CLOCK_SYNC)) == 0) {
            // TODO CLOCK SYNC
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_CLOCK_SYNC");

        } else if(strncmp(method, RPC_SIGNAL_STATUS_REPORT, strlen(RPC_SIGNAL_STATUS_REPORT)) == 0) {
            // TODO SIGNAL STASTUS REPORT
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SIGNAL_STATUS_REPORT");

        } 
        
        if(strncmp(method, RPC_USER, strlen(RPC_USER)) == 0) {
            SKTDebugPrint(LOG_LEVEL_INFO, method);
            // ATCOM INITIATED
            if(!paramsObject) return;
            cJSON* paramObject = cJSON_GetArrayItem(paramsObject, 0);
            controlObject = cJSON_GetObjectItemCaseSensitive(paramObject, "act7colorLed");
            if(!controlObject) return;
            control = controlObject->valueint;
            SKTDebugPrint(LOG_LEVEL_INFO, "\nrpc : %s,\nid : %d,\ncontrol: %d", rpc, id, control);
            rc = RGB_LEDControl(control);
            // control success
            if(rc == 0) {            
                char at_res[32] = "";
                snprintf(at_res, 32, "{\"%s\":%d}", controlObject->string ,control);
                SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPRES=1,%s,%d,0,%s", method, id, at_res);
                char* rawData = make_response(&rsp, at_res);
                int error = tpSimpleRawResult(rawData);
                set_error(error);
            }
            // control fail
            else {
                char at_res[64] = "";
                snprintf(at_res, 64, "{%s}",  "\"code\" : -14, \"message\" : \"Invalid Parameters\"");
                SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPRES=1,%s,%d,1,%s", method, id, at_res);
                rsp.result = 0;
                char* rawData = make_response(&rsp, at_res);
                int error = tpSimpleRawResult(rawData);
                set_error(error);
            }

        } else if(strncmp(method, RPC_REMOTE, strlen(RPC_REMOTE)) == 0 ) {
            char at_res[32] = "";
            snprintf(at_res, 32, "{%s}",  "\"status\" : \"SUCCESS\"");
            char* rawData = make_response(&rsp, at_res);
            int error = tpSimpleRawResult(rawData);
            set_error(error);
        } else {
            char at_res[32] = "";
            snprintf(at_res, 32, "{%s}",  "\"status\" : \"SUCCESS\"");
            SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPRES=1,%s,%d,0,%s", method,id, at_res);
            char* rawData = make_response(&rsp, at_res);
            int error = tpSimpleRawResult(rawData);
            set_error(error);
        }
        SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCMD=%s,%d,3", method, id);
        SKTDebugPrint(LOG_LEVEL_ATCOM, "OK");
    } else {
        cJSON* cmdObject = cJSON_GetObjectItemCaseSensitive(root, "cmd");
        cJSON* cmdIdObject = cJSON_GetObjectItemCaseSensitive(root, "cmdId");
        if(!cmdObject || !cmdIdObject) return;
        char* cmd = cmdObject->valuestring;
        int cmdId = cmdIdObject->valueint;
        if(!cmd) return;
        // if attribute control
        if(strncmp(cmd, "setAttribute", strlen("setAttribute")) == 0) {
            cJSON* attributeObject = cJSON_GetObjectItemCaseSensitive(root, "attribute");
            if(!attributeObject) return;
            char* attribute = cJSON_PrintUnformatted(attributeObject);
            // ATCOM INITIATED
            SKTDebugPrint(LOG_LEVEL_ATCOM, "+SKTPCMD=set_attr,%d,3,%s", cmdId, attribute);
            free(attribute);

            cJSON* act7colorLedObject = cJSON_GetObjectItemCaseSensitive(attributeObject, "act7colorLed");
            if(!act7colorLedObject) return;
            int act7colorLed = act7colorLedObject->valueint;
            SKTDebugPrint(LOG_LEVEL_INFO, "act7colorLed : %d, %d", act7colorLed, cmdId);
            int rc = RGB_LEDControl(act7colorLed);
            if(rc != 0) {
                act7colorLed = RGB_LEDStatus();
            }
#ifdef JSON_FORMAT
            ArrayElement* arrayElement = calloc(1, sizeof(ArrayElement));
            arrayElement->capacity = 1;
            arrayElement->element = calloc(1, sizeof(Element) * arrayElement->capacity);
            Element* item = arrayElement->element + arrayElement->total;
            item->type = JSON_TYPE_LONG;
            item->name = "act7colorLed";
            item->value = &act7colorLed;
            arrayElement->total++;
            int error = tpSimpleAttribute(arrayElement);
            set_error(error);
            free(arrayElement->element);
            free(arrayElement);
#elif defined(CSV_FORMAT)
            char csvAttr[256] = "";
            snprintf(csvAttr, sizeof(csvAttr), ",,,,,,,,,,%d", act7colorLed);
            int error = tpSimpleRawAttribute(csvAttr, FORMAT_CSV);
            set_error(error);
#endif
        }
    }
    cJSON_Delete(root);
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    // long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // return milliseconds;
    return te.tv_sec;
}

static char* make_response(RPCResponse *rsp, char* resultBody) {
    char* jsonData;
    cJSON* jsonObject = cJSON_CreateObject();
    cJSON* rpcRspObject = cJSON_CreateObject();
    cJSON* resultObject;

    cJSON_AddStringToObject(jsonObject, CMD, rsp->cmd);
    cJSON_AddNumberToObject(jsonObject, CMD_ID, rsp->cmdId);
    if( rsp->result ) {
        cJSON_AddStringToObject(jsonObject, RESULT, "success");
    } else {
        cJSON_AddStringToObject(jsonObject, RESULT, "fail");
    }

    cJSON_AddStringToObject(rpcRspObject, JSONRPC, rsp->jsonrpc);
    cJSON_AddNumberToObject(rpcRspObject, ID, rsp->id);
    if(resultBody != NULL) {
        resultObject = cJSON_CreateRaw(resultBody);
        if(rsp->result) {
            cJSON_AddItemToObject(rpcRspObject, RESULT, resultObject);
        } else {
            cJSON_AddItemToObject(rpcRspObject, ERROR, resultObject);
        }
    }
    cJSON_AddItemToObject(jsonObject, RPC_RSP, rpcRspObject);
    jsonData = cJSON_Print(jsonObject);
    cJSON_Delete(jsonObject);
    return jsonData;
}

char *sensor_list[] = { "temp1", "humi1", "light1" };

static void make_telemetry(char *data) {
    char *temp, *humi, *light, time[16] = "";
    int len;
    long long ctime = current_timestamp();
    snprintf(time, 16, "%lld", ctime);
    SMAGetData(sensor_list[0], &temp, &len);
    SMAGetData(sensor_list[1], &humi, &len);
    SMAGetData(sensor_list[2], &light, &len);
#ifdef JSON_FORMAT
    char sensorInfo[64] = "";
    strncat(data, "{", strlen("{"));
    snprintf(sensorInfo, sizeof(sensorInfo), "\"%s\":%s,", TIMESTAMP, time);
    strncat(data, sensorInfo, strlen(sensorInfo));
    memset(sensorInfo, 0, sizeof(sensorInfo));
    snprintf(sensorInfo, sizeof(sensorInfo), "\"%s\":%s,", sensor_list[0], temp);
    strncat(data, sensorInfo, strlen(sensorInfo));
    memset(sensorInfo, 0, sizeof(sensorInfo));
    snprintf(sensorInfo, sizeof(sensorInfo), "\"%s\":%s,", sensor_list[1], humi);
    strncat(data, sensorInfo, strlen(sensorInfo));
    memset(sensorInfo, 0, sizeof(sensorInfo));
    snprintf(sensorInfo, sizeof(sensorInfo), "\"%s\":%s", sensor_list[2], light);
    strncat(data, sensorInfo, strlen(sensorInfo));
    strncat(data, "}", strlen("}"));
#elif defined(CSV_FORMAT)
    SRAConvertCSVData( data, time);
    SRAConvertCSVData( data, temp);
    SRAConvertCSVData( data, humi);
    SRAConvertCSVData( data, light);
#endif
    if(temp) free(temp);
    if(humi) free(humi);
    if(light) free(light);
}

static void send_data(THINGPLUG_DATA_TYPE tpDataType, DATA_FORMAT format, char *data) {
    int rc = -1;
    switch(tpDataType) {
        case TELEMETRY_TYPE:
            rc = tpSimpleRawTelemetry(data, format);
            break;
        case ATTRIBUTE_TYPE:
            rc = tpSimpleRawAttribute(data, format);
            break;
        default:
            break;
    }
    set_error(rc);
    SKTDebugPrint(LOG_LEVEL_ATCOM, "OK");
}

static unsigned long getAvailableMemory() {
    unsigned long ps = sysconf(_SC_PAGESIZE);
    unsigned long pn = sysconf(_SC_AVPHYS_PAGES);
    unsigned long availMem = ps * pn;
    return availMem;
}

static int getNetworkInfo(NetworkInfo* info, char* interface) {

    char* deviceIpAddress;
    char* gatewayIpAddress = NULL;
    char cmd [1000] = "";
    char line[256]= "";
    int sock;
    struct ifreq ifr;

    // device IP address
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return -1;
    }

    strcpy(ifr.ifr_name, interface);
    if (ioctl(sock, SIOCGIFADDR, &ifr)< 0)
    {
        close(sock);
        return -1;
    }
    deviceIpAddress = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    close(sock);
    memcpy(info->deviceIpAddress, deviceIpAddress, strlen(deviceIpAddress));
    
    // gateway IP address
    sprintf(cmd,"route -n | grep %s  | grep 'UG[ \t]' | awk '{print $2}'", interface);
    FILE* fp = popen(cmd, "r");
    if(fgets(line, sizeof(line), fp) != NULL) {
        gatewayIpAddress = &line[0];
    }
    pclose(fp);
    memcpy(info->gatewayIpAddress, gatewayIpAddress, strlen(gatewayIpAddress) - 1);

    return 0;
}

static void make_attribute(char* data) {
    char availableMemory[64] = "";
    snprintf(availableMemory, sizeof(availableMemory), "%lu", getAvailableMemory());
    NetworkInfo info;
    memset(&info, 0, sizeof(NetworkInfo));
    getNetworkInfo(&info, "eth0");
#ifdef JSON_FORMAT
    char attrInfo[64] = "";
    strncat(data, "{", strlen("{"));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysAvailableMemory\":%s,", availableMemory);
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysFirmwareVersion\":\"%s\",", "2.0.0");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysHardwareVersion\":\"%s\",", "1.0");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysSerialNumber\":\"%s\",", "710DJC5I10000290");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysErrorCode\":%s,", "0");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysNetworkType\":\"%s\",", "ethernet");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysDeviceIpAddress\":\"%s\",", info.deviceIpAddress);
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysThingPlugIpAddress\":\"%s\",", MQTT_HOST);
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysLocationLatitude\":%s,", "37.380257");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"sysLocationLongitude\":%s,", "127.115479");
    strncat(data, attrInfo, strlen(attrInfo));
    memset(attrInfo, 0, sizeof(attrInfo));
    snprintf(attrInfo, sizeof(attrInfo), "\"act7colorLed\":%s", "0");
    strncat(data, attrInfo, strlen(attrInfo));
    strncat(data, "}", strlen("}"));
#elif defined(CSV_FORMAT)
    // Memory
    SRAConvertCSVData(data, availableMemory);
    // SW Version
    SRAConvertCSVData(data, "2.0.0");
    // HW Version
    SRAConvertCSVData(data, "1.0");
    // Serial
    SRAConvertCSVData(data, "710DJC5I10000290");
    // Error code
    SRAConvertCSVData(data, "0");
    // NetworkType
    SRAConvertCSVData(data, "ethernet");
    // IPAddr
    SRAConvertCSVData(data, info.deviceIpAddress);
    // ServerIPAddr
    SRAConvertCSVData(data, MQTT_HOST); 
    // Latitude
    SRAConvertCSVData(data, "37.380257"); 
    // Longitude
    SRAConvertCSVData(data, "127.115479"); 
    // Led    
    SRAConvertCSVData(data, "0");
#endif
    mStep = PROCESS_TELEMETRY;
}

static char* get_error() {
    char error[128] = "";
    snprintf(error, sizeof(error), "+SKTPE:%d", mErrorCode);
    return strdup(error);
}

static void set_error(int errorCode) {
    mErrorCode = errorCode;
}


/**
 * @brief get Device MAC Address without Colon.
 * @return mac address
 */
 char* GetMacAddressWithoutColon() {
    int i, sock;
    struct ifreq ifr;
    char mac_adr[18] = "";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return NULL;
    }

    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        return NULL;
    }

    for (i = 0; i < 6; i++) {
        sprintf(&mac_adr[i*2],"%02X",((unsigned char*)ifr.ifr_hwaddr.sa_data)[i]);
    }
    close(sock);

    return strdup(mac_adr);
}

void start() {
    tpSDKDestroy();
    int rc;

    mConnectionStatus = CONNECTING;

    RGB_LEDControl(0);

    // set callbacks
    rc = tpMQTTSetCallbacks(MQTTConnected, MQTTSubscribed, MQTTDisconnected, MQTTConnectionLost, MQTTMessageDelivered, MQTTMessageArrived);
    SKTDebugPrint(LOG_LEVEL_INFO, "tpMQTTSetCallbacks result : %d", rc);
    // Simple SDK initialize
    rc = tpSimpleInitialize(SIMPLE_SERVICE_NAME, SIMPLE_DEVICE_NAME);
    SKTDebugPrint(LOG_LEVEL_INFO, "tpSimpleInitialize : %d", rc);
    // create clientID - MAC address
    char* macAddress = GetMacAddressWithoutColon();
    snprintf(mClientID, sizeof(mClientID), MQTT_CLIENT_ID, SIMPLE_DEVICE_NAME, macAddress);
    free(macAddress);
    SKTDebugPrint(LOG_LEVEL_INFO, "client id : %s", mClientID);
    // create Topics
    snprintf(mTopicControlDown, SIZE_TOPIC, MQTT_TOPIC_CONTROL_DOWN, SIMPLE_SERVICE_NAME, SIMPLE_DEVICE_NAME);

    char* subscribeTopics[] = { mTopicControlDown };

#if(1)
	char host[] = MQTT_SECURE_HOST;
	int port = MQTT_SECURE_PORT;
#else
	char host[] = MQTT_HOST;
	int port = MQTT_PORT;
#endif
    rc = tpSDKCreate(host, port, MQTT_KEEP_ALIVE, SIMPLE_DEVICE_TOKEN, NULL, 
        MQTT_ENABLE_SERVER_CERT_AUTH, subscribeTopics, TOPIC_SUBSCRIBE_SIZE, NULL, mClientID, MQTT_CLEAN_SESSION);
    SKTDebugPrint(LOG_LEVEL_INFO, "tpSDKCreate result : %d", rc);
}

int MARun() {
    SKTDebugInit(1, LOG_LEVEL_INFO, NULL);
	SKTDebugPrint(LOG_LEVEL_INFO, "ThingPlug_Simple_SDK");
    SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPCON=1,MQTTS,%s,%d,%d,%d,simple_v1,%s,%s,%s", 
        MQTT_SECURE_HOST, 
        MQTT_SECURE_PORT,
        MQTT_KEEP_ALIVE,
        MQTT_CLEAN_SESSION,
        SIMPLE_DEVICE_TOKEN,
        SIMPLE_SERVICE_NAME,
        SIMPLE_DEVICE_NAME
    );
    start();
    SKTDebugPrint(LOG_LEVEL_ATCOM, "OK");
    int sendTelemetryCount = 0;
    while (mStep < PROCESS_END && sendTelemetryCount <= 10) {
		if(tpMQTTIsConnected() && mStep == PROCESS_TELEMETRY) {
            char data[SIZE_PAYLOAD] = "";
            make_telemetry(data);
#ifdef JSON_FORMAT
            SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPDAT=1,telemetry,0,%s", data);
            send_data(TELEMETRY_TYPE, FORMAT_JSON, data);
#elif defined(CSV_FORMAT)
            SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPDAT=1,telemetry,1,%s", data);
            send_data(TELEMETRY_TYPE, FORMAT_CSV, data);
#endif
            SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPERR=1");
            char* ret = get_error();
            SKTDebugPrint(LOG_LEVEL_ATCOM, "result : %s", ret);
            free(ret);
            sendTelemetryCount++;
        }
        // reconnect when disconnected
        // else if(mConnectionStatus == DISCONNECTED) {
        //     start();
        // }
        #if defined(WIN32) || defined(WIN64)
            Sleep(100);
        #else
            usleep(10000000L);
        #endif
    }
    SKTDebugPrint(LOG_LEVEL_ATCOM, "AT+SKTPCON=0");
    tpSDKDestroy();
    SKTDebugPrint(LOG_LEVEL_ATCOM, "OK");
    return 0;
}
