Linux (+TLS)
===

지원 사양
---
1. 테스트 환경
	+ Raspberry PI 2/3, BeagleBone-Black, Samsung ARTIK

2. 최소 동작 환경
	+ CPU : ARM architecture / 100MHz
	+ RAM : 5MB
	+ Flash memory : 5MB

Library
---
다음 오픈소스 라이브러리들을 사용합니다.

라이브러리 | 용도 | 홈페이지
------------ | ------------- | -------------
__cJSON__ | JSON parser | [cJSON Homepage](https://github.com/DaveGamble/cJSON)
__paho__ | MQTT | [paho Homepage](https://github.com/eclipse/paho.mqtt.c/tree/v1.2.0)
__openssl__ | OpenSSL | [openssl Homepage](https://github.com/openssl/openssl/tree/OpenSSL_1_0_2-stable)

Sample build
===

Configuration 설정(samples/Configuration.h)
---
MQTT broker 와의 연결을 위한 정보 및 디바이스 정보를 설정해야 합니다.
```c
#define MQTT_HOST                           ""
#define MQTT_SECURE_HOST                    ""
#define MQTT_PORT                           1883
#define MQTT_SECURE_PORT                    8883						
#define MQTT_KEEP_ALIVE                     120
#define MQTT_ENABLE_SERVER_CERT_AUTH        1
#define SIMPLE_DEVICE_TOKEN                 ""
#define SIMPLE_SERVICE_NAME                 ""
#define SIMPLE_DEVICE_NAME                  ""
```

변수 | 값 | 용도 
------------ | ------------- | -------------
__SIMPLE_DEVICE_TOKEN__ | ThingPlug 포털을 통해 디바이스 등록 후 발급받은 디바이스 토큰 | MQTT 로그인 사용자명으로 사용
__SIMPLE_SERVICE_NAME__ | ThingPlug 포털을 통해 등록한 서비스명 | MQTT Topic 에 사용
__SIMPLE_DEVICE_NAME__ | ThingPlug 포털을 통해 등록한 디바이스명 | MQTT Topic 에 사용


디바이스 디스크립터와 Attribute, Telemetry
---
각 디바이스의 고유의 특성을 전달하는 Attribute 변경 통지와 센서를 통해 측정된 값을 전달하는 Telemetry 전송 데이터는 ThingPlug 포털에 등록한 디바이스 디스크립터의 내용과 1:1 매칭되어야 합니다.
다음은 포털에 등록된 디바이스 디스크립터와 매칭된 소스코드 예시입니다.

```json
"Airconditioner": {
     "telemetries": [{"name":"temperature","type":"number"}, {"name":"humidity","type":"int"}],
     "attribute": [{"name":"control","type":"string"}]
 }
```

```c
void telemetry() {
    double temp = 27.05;
    int humi = 75;

    ArrayElement* arrayElement = calloc(1, sizeof(ArrayElement));    
    arrayElement->capacity = 2;
    arrayElement->element = calloc(1, sizeof(Element) * arrayElement->capacity);

    Element* item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_DOUBLE;
    item->name = "temperature";	
    item->value = &temp;
    arrayElement->total++;

    item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_LONG;
    item->name = "humidity";
    item->value = &humi;
    arrayElement->total++;
    
    tpSimpleTelemetry(arrayElement, 0);
    free(arrayElement->element);
    free(arrayElement);
}

void attribute() {
    char *status = "stopped";
	
    ArrayElement* arrayElement = calloc(1, sizeof(ArrayElement));    
    arrayElement->capacity = 1;
    arrayElement->element = calloc(1, sizeof(Element) * arrayElement->capacity);
    
    Element* item = arrayElement->element + arrayElement->total;
    item->type = JSON_TYPE_STRING;
    item->name = "control";
    item->value = status;
    arrayElement->total++;

    tpSimpleAttribute(arrayElement);
    free(arrayElement->element);
    free(arrayElement);
}

```

ThingPlug_Simple_SDK 빌드(samples/ThingPlug_Simple_SDK.c)
---
1. 빌드

	```
	# make
	```
	
2. 빌드 클리어

	```
	# make clean
	```
	
3. 실행

	```
	# output/ThingPlug_Simple_SDK
	```
	
ThingPlug_Simple_SDK 실행
---
1. 실행 로그 확인
---
[LOG_linux.txt](./LOG_linux.txt)
---
2. Thingplug SensorData
---
![MbedTP.png](images/linuxTP.png)

Copyright (c) 2018 SK telecom Co., Ltd. All Rights Reserved.
Distributed under Apache License Version 2.0. See LICENSE for details.
