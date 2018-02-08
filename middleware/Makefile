OUTPUT = ./output

DEBUG = -DDEBUG_ENABLE#

FEATURE += $(DEBUG)

#CROSS_COMPILE = arm-linux-gnueabihf-
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
RANLIB = $(CROSS_COMPILE)ranlib

#c files
SIMPLE_SDK_OBJS = \
	./SRA/SRA.o \
	./SMA/SMA.o \
	./MA/MA.o \
	./ThingPlug_Simple_SDK.o

INC = -I../include -I./ -I./SRA -I./SMA -I./MA 
LIBS = -L../lib -L/usr/local/lib
CFLAGS = -g -c -Wall $(FEATURE)
CXXFLAGS = -c -Wall -D__STDC_LIMIT_MACROS
COBJS = $(SIMPLE_SDK_OBJS)
LDFLAGS = -ltplinuxsdk -lpaho-mqtt3as -lssl -lcrypto -lpthread -ltplinuxsdk -lpaho-mqtt3as -lssl -lcrypto -lpthread
TARGET = ThingPlug_Simple_SDK

all: $(TARGET)

$(TARGET): $(CXXOBJS) $(COBJS)
	$(CC) $(COBJS) $(INC) $(LIBS) $(LDFLAGS) -o $@
	mkdir -p $(OUTPUT)
	mv $(TARGET) $(OUTPUT)
	
.c.o:
	$(CC) $(CFLAGS) $(INC) $(LIBS)  $< -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC) $(LIBS)  $< -o $@

clean :
	rm -rf $(SIMPLE_SDK_OBJS)
	rm -rf $(OUTPUT)
