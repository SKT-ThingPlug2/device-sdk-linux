OUTPUT = ./lib
SDK_DIR = ./src

DEBUG = -DDEBUG_ENABLE#

FEATURE += $(DEBUG)

#CROSS_COMPILE = arm-linux-gnueabihf-
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
RANLIB = $(CROSS_COMPILE)ranlib

#c files
SIMPLE_SDK_OBJS = \
	$(SDK_DIR)/SKTtpDebug.o \
	$(SDK_DIR)/net/MQTTClient.o \
	$(SDK_DIR)/ThingPlug.o \
	$(SDK_DIR)/simple/Simple.o \
	$(SDK_DIR)/simple/cJSON.o \

INC = -I./include 
LIBS = 
CFLAGS = -g -c -Wall $(FEATURE)
COBJS = $(SIMPLE_SDK_OBJS)
TARGET = libtplinuxsdk.a

all: $(TARGET)

$(TARGET): $(COBJS)
	$(AR) crv $(TARGET) $(COBJS)
	$(RANLIB) $(TARGET)
	mkdir -p $(OUTPUT)
	mv $(TARGET) $(OUTPUT)
	
.c.o:
	$(CC) $(CFLAGS) $(INC) $< -o $@
	
clean :
	rm -rf $(SIMPLE_SDK_OBJS)
	rm -rf $(OUTPUT)/$(TARGET)
	
