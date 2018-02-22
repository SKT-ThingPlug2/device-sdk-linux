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
// #define SIZE_TOPIC                          128
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

typedef struct
{
    /** device IP address **/
    char deviceIpAddress[30];
    /** gateway IP address **/
    char gatewayIpAddress[30];
} NetworkInfo;

static char mTopicControlDown[SIZE_TOPIC] = "";
static char mClientID[SIZE_CLIENT_ID] = "";

static void attribute();
static void telemetry();
static char* make_response(RPCResponse *rsp, char* resultBody);

void MQTTConnected(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTConnected result : %d", result);
    // if connection failed
    if(result) {
        mConnectionStatus = DISCONNECTED;
    } else {
        mConnectionStatus = CONNECTED;
    }
    SKTDebugPrint(LOG_LEVEL_INFO, "CONNECTION_STATUS : %d", mConnectionStatus);
}

void MQTTSubscribed(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTSubscribed result : %d", result);
    attribute();
}

void MQTTDisconnected(int result) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTDisconnected result : %d", result);
}

void MQTTConnectionLost(char* cause) {
    SKTDebugPrint(LOG_LEVEL_INFO, "MQTTConnectionLost result : %s", cause);
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

        } else if(strncmp(method, RPC_CLOCK_SYNC, strlen(RPC_CLOCK_SYNC)) == 0) {
            // TODO CLOCK SYNC
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_CLOCK_SYNC");

        } else if(strncmp(method, RPC_SIGNAL_STATUS_REPORT, strlen(RPC_SIGNAL_STATUS_REPORT)) == 0) {
            // TODO SIGNAL STASTUS REPORT
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_SIGNAL_STATUS_REPORT");

        } 
        
        if(strncmp(method, RPC_USER, strlen(RPC_USER)) == 0 ) {
            SKTDebugPrint(LOG_LEVEL_INFO, "RPC_USER");
            if(!paramsObject) return;
            cJSON* paramObject = cJSON_GetArrayItem(paramsObject, 0);
            controlObject = cJSON_GetObjectItemCaseSensitive(paramObject, "act7colorLed");
            if(!controlObject) return;
            control = controlObject->valueint;
            SKTDebugPrint(LOG_LEVEL_INFO, "\nrpc : %s,\nid : %d,\ncontrol : %d", rpc, id, control);
            rc = RGB_LEDControl(control);
            // control success
            if(rc == 0) {
                char body[128] = "";
                snprintf(body, sizeof(body), "{\"%s\":%d}", controlObject->string, control);
                char* rpcRsp = make_response(&rsp, body);
                tpSimpleRawResult(rpcRsp);
                free(rpcRsp);
            }
            // control fail
            else {
                rsp.result = 0;
                ArrayElement* resultArray = calloc(1, sizeof(ArrayElement));
                resultArray->capacity = 2;
                resultArray->element = calloc(1, sizeof(Element) * resultArray->capacity);
                int errorCode = 106;

                Element* item = resultArray->element + resultArray->total;
                item->type = JSON_TYPE_LONG;
                item->name = "code";
                item->value = &errorCode;
                resultArray->total++;

                item = resultArray->element + resultArray->total;
                item->type = JSON_TYPE_STRING;
                item->name = "message";
                item->value = "FAIL";
                resultArray->total++;

                rsp.resultArray = resultArray;

                tpSimpleResult(&rsp);
                free(resultArray->element);
                free(resultArray);
            }
        } else {
            ArrayElement* resultArray = calloc(1, sizeof(ArrayElement));
            resultArray->capacity = 1;
            resultArray->element = calloc(1, sizeof(Element) * resultArray->capacity);
            Element* item = resultArray->element + resultArray->total;
            item->type = JSON_TYPE_STRING;
            item->name = "status";
            item->value = "SUCCESS";
            resultArray->total++;
            rsp.resultArray = resultArray;
            tpSimpleResult(&rsp);
        }
                
    } else {
        cJSON* cmdObject = cJSON_GetObjectItemCaseSensitive(root, "cmd");
        cJSON* cmdIdObject = cJSON_GetObjectItemCaseSensitive(root, "cmdId");
        if(!cmdObject || !cmdIdObject) return;
        char* cmd = cmdObject->valuestring;
        int cmdId = cmdIdObject->valueint;
        if(!cmd) return;
        // if attribute control
        if(strncmp(cmd, "setAttribute", strlen("setAttribute")) == 0) {
            cJSON* attribute = cJSON_GetObjectItemCaseSensitive(root, "attribute");
            if(!attribute) return;
            cJSON* act7colorLedObject = cJSON_GetObjectItemCaseSensitive(attribute, "act7colorLed");
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
            tpSimpleAttribute(arrayElement);
            free(arrayElement->element);
            free(arrayElement);
#endif
#ifdef CSV_FORMAT
            char csvAttr[256] = "";
            snprintf(csvAttr, sizeof(csvAttr), ",,,,,,,,,,%d", act7colorLed);
            tpSimpleRawAttribute(csvAttr, FORMAT_CSV);
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

char *sensor_list[] = { "temp1", "humi1", "light1" };

static void telemetry() {
    mStep = PROCESS_TELEMETRY;
#ifdef JSON_FORMAT
    char *temp, *humi, *light;
    int len;
    ArrayElement* arrayElement = calloc(1, sizeof(ArrayElement));
    
    arrayElement->capacity = 4;
    arrayElement->element = calloc(1, sizeof(Element) * arrayElement->capacity);
    Element *item;

    SMAGetData(sensor_list[arrayElement->total], &temp, &len);
    temp = SRAConvertRawData(temp);
    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_RAW;
    item->name = sensor_list[arrayElement->total];
    item->value = temp;
    arrayElement->total++;

    SMAGetData(sensor_list[arrayElement->total], &humi, &len);
    humi = SRAConvertRawData(humi);
    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_RAW;
    item->name = sensor_list[arrayElement->total];
    item->value = humi;
    arrayElement->total++;
    
    SMAGetData(sensor_list[arrayElement->total], &light, &len);
    light = SRAConvertRawData(light);
    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_RAW;
    item->name = sensor_list[arrayElement->total];
    item->value = light;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_LONGLONG;
    item->name = TIMESTAMP;
    long long time = current_timestamp();
    item->value = (void *)&time;
    arrayElement->total++;

    tpSimpleTelemetry(arrayElement, 0);
    free(arrayElement->element);
    free(arrayElement);
    free(temp);
    free(humi);
    free(light);
#endif
#ifdef CSV_FORMAT
    char *temp, *humi, *light,time[16];
    int len,total_len;

    long long ctime = current_timestamp();
    snprintf(time, 16, "%lld", ctime);
    total_len = (strlen(time) + 1);
    SMAGetData(sensor_list[0], &temp, &len);
    total_len += (strlen(temp) + 1);
    SMAGetData(sensor_list[1], &humi, &len);
    total_len += (strlen(humi) + 1);
    SMAGetData(sensor_list[2], &light, &len);
    total_len += (strlen(light) + 1);

    char* csv_data = (char *) calloc(total_len + 1, sizeof(char));
    SRAConvertCSVData( csv_data, time);
    SRAConvertCSVData( csv_data, temp);
    SRAConvertCSVData( csv_data, humi);
    SRAConvertCSVData( csv_data, light);
    tpSimpleRawTelemetry(csv_data, FORMAT_CSV);
    free(csv_data);
    free(temp);
    free(humi);
    free(light);
#endif
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

static void attribute() {

#ifdef JSON_FORMAT
    ArrayElement* arrayElement = calloc(1, sizeof(ArrayElement));
    
    arrayElement->capacity = 15;
    arrayElement->element = calloc(1, sizeof(Element) * arrayElement->capacity);
    
    Element* item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_LONG;
    item->name = "sysAvailableMemory";
    unsigned long availableMemory = getAvailableMemory();
    item->value = &availableMemory;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysFirmwareVersion";
    item->value = "2.0.0";
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysHardwareVersion";
    item->value = "1.0";
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysSerialNumber";
    item->value = "710DJC5I10000290";
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_LONG;
    item->name = "sysErrorCode";
    int errorCode = 0;
    item->value = &errorCode;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysNetworkType";
    item->value = "ethernet";
    arrayElement->total++;

    NetworkInfo info;
    memset(&info, 0, sizeof(NetworkInfo));
    getNetworkInfo(&info, "eth0");
    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysDeviceIpAddress";
    item->value = info.deviceIpAddress;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "sysThingPlugIpAddress";
    item->value = MQTT_HOST;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_RAW;
    item->name = "sysLocationLatitude";    
    item->value = "37.380257";
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_RAW;
    item->name = "sysLocationLongitude";
    item->value = "127.115479";
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_LONG;
    item->name = "act7colorLed";
    int act7colorLed = 0;
    item->value = &act7colorLed;
    arrayElement->total++;

    tpSimpleAttribute(arrayElement);
    free(arrayElement->element);
    free(arrayElement);

    mStep = PROCESS_TELEMETRY;
#endif
#ifdef CSV_FORMAT
    char csv_attr[256] = "";
    unsigned long availableMemory = getAvailableMemory();
    char tmp[64];
    snprintf( tmp, 64, "%lu", availableMemory );
    // Memory 
    SRAConvertCSVData( csv_attr, tmp);
    // SW Version
    SRAConvertCSVData( csv_attr, "2.0.0");
    // HW Version
    SRAConvertCSVData( csv_attr, "1.0");
    //Serial
    SRAConvertCSVData( csv_attr, "710DJC5I10000290");
    int errorCode = 0;
    snprintf( tmp, 64, "%d", errorCode);
    //Error code
    SRAConvertCSVData( csv_attr, tmp);
    //NetworkType
    SRAConvertCSVData( csv_attr, "ethernet");
    NetworkInfo info;
    memset(&info, 0, sizeof(NetworkInfo));
    getNetworkInfo(&info, "eth0");
    //IPAddr
    SRAConvertCSVData( csv_attr, info.deviceIpAddress);
    //ServerIPAddr
    SRAConvertCSVData( csv_attr, MQTT_HOST); 
    //Latitude
    SRAConvertCSVData( csv_attr, "37.380257"); 
    //Longitude
    SRAConvertCSVData( csv_attr, "127.115479"); 
    //Led
    int act7colorLed = 0;
    snprintf( tmp, 64, "%d", act7colorLed );
    SRAConvertCSVData( csv_attr, tmp);
    tpSimpleRawAttribute(csv_attr, FORMAT_CSV);
    
    mStep = PROCESS_TELEMETRY;
#endif
}

static char* make_response(RPCResponse *rsp, char* resultBody) {
    char* jsonData;
    cJSON* jsonObject = cJSON_CreateObject();
    cJSON* rpcRspObject = cJSON_CreateObject();
    cJSON* resultObject;

    cJSON_AddStringToObject(jsonObject, CMD, rsp->cmd);
    cJSON_AddNumberToObject(jsonObject, CMD_ID, rsp->cmdId);
    if(rsp->result) {
        cJSON_AddStringToObject(jsonObject, RESULT, "success");
    } else {
        cJSON_AddStringToObject(jsonObject, RESULT, "fail");
    }

    cJSON_AddStringToObject(rpcRspObject, JSONRPC, rsp->jsonrpc);
    cJSON_AddNumberToObject(rpcRspObject, ID, rsp->id);
    if(resultBody!=NULL) {
        resultObject = cJSON_CreateRaw(resultBody);
        if(rsp->result) {
            cJSON_AddItemToObject(rpcRspObject, RESULT, resultObject);
        } else {
            cJSON_AddItemToObject(rpcRspObject, ERROR, resultObject);
        }
    }
    cJSON_AddItemToObject(jsonObject, RPC_RSP, rpcRspObject);
    jsonData = cJSON_PrintUnformatted(jsonObject);
    cJSON_Delete(jsonObject);
    return jsonData;
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
    start();

    while (mStep < PROCESS_END) {
		if(tpMQTTIsConnected() && mStep == PROCESS_TELEMETRY) {
            telemetry();
        } 
        // reconnect when disconnected 
        else if(mConnectionStatus == DISCONNECTED) {
            tpSDKDestroy();
            start();
        }

        #if defined(WIN32) || defined(WIN64)
            Sleep(100);
        #else
            usleep(10000000L);
        #endif
    }
    tpSDKDestroy();
    return 0;
}
