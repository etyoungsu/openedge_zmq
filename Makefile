
# Makefile for OpenEdge Software Framework for Application Gateway
# Author : Byunghun Hwang <bh.hwang@iae.re.kr>
# Usage : make ARCH=armhf
# Note : You should make with GCC/G++ version 8

# Makefile

OS := $(shell uname)

#Set Architecutre
ARCH := armhf

#Compilers
ifeq ($(ARCH),armhf)
	CC := /usr/bin/arm-linux-gnueabihf-g++-8
	GCC := /usr/bin/arm-linux-gnueabihf-gcc-8
	LD_LIBRARY_PATH += -L./lib/armhf
	OUTDIR		= ./bin/armhf/
	BUILDDIR		= ./bin/armhf/
	INCLUDE_DIR = -I./ -I./include/ -I./include/3rdparty/
	LD_LIBRARY_PATH += -L/usr/local/lib -L./lib/armhf
	LD_RUN_PATH += -L/usr/local/lib -L./lib/armhf
else
	CC := g++
	GCC := gcc
	LD_LIBRARY_PATH += -L./lib/x86_64
	OUTDIR		= ./bin/x86_64/
	BUILDDIR		= ./bin/x86_64/
	INCLUDE_DIR = -I./ -I./include/ -I./include/3rdparty/
	LD_LIBRARY_PATH += -L/usr/local/lib -L./lib/x86_64
endif

# OS
ifeq ($(OS),Linux) #for Linux
	LDFLAGS = -Wl,--export-dynamic 
	LDLIBS = -pthread -lrt -ldl -lm
	GTEST_LDLIBS = -lgtest
endif

$(shell mkdir -p $(OUTDIR))
$(shell mkdir -p $(BUILDDIR))

#if release(-O3), debug(-O0)
CXXFLAGS = -O3 -fPIC -Wall -std=c++17 -D__cplusplus=201703L

#custom definitions
CXXFLAGS += -D__MAJOR__=0 -D__MINOR__=0 -D__REV__=5
RM	= rm -rf

#directories

INCLUDE_FILES = ./include/
SOURCE_FILES = ./
APP_SOURCE_FILES = ./apps/
EXAMPLE_SOURCE_FILES = ./examples/
TASK_SOURCE_FILES = ./tasks/
SERVICE_SOURCE_FILES = ./services/
SUPPORT_SOURCE_FILES = ./support/
INSTALL_DIR = /usr/local/bin/

# Make
openedge:	$(BUILDDIR)openedge.o
			$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -o $(BUILDDIR)$@ $^ $(LDLIBS)

oeware_test:	$(BUILDDIR)oeware_test.o
				$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -o $(BUILDDIR)$@ $^ $(LDLIBS) $(GTEST_LDLIBS)

# edge service engine
edge:	$(BUILDDIR)edge.o \
		$(BUILDDIR)edge_instance.o \
		$(BUILDDIR)task_manager.o \
		$(BUILDDIR)driver.o \
		$(BUILDDIR)profile.o \
		$(BUILDDIR)rt_timer.o
		$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq

#
# edge service engine
#
$(BUILDDIR)edge.o: $(APP_SOURCE_FILES)edge/edge.cc
				$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)task_manager.o: $(APP_SOURCE_FILES)edge/task_manager.cc
				$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)edge_instance.o: $(APP_SOURCE_FILES)edge/instance.cc
				$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@


#openedge base
$(BUILDDIR)rt_trigger.o: $(INCLUDE_FILES)openedge/core/rt_trigger.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

# include
$(BUILDDIR)rt_timer.o: $(INCLUDE_FILES)openedge/core/rt_timer.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

############################ Openedge Cores
$(BUILDDIR)driver.o:	$(INCLUDE_FILES)openedge/core/driver.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)profile.o:	$(INCLUDE_FILES)openedge/core/profile.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIRf) -c $^ -o $@
$(BUILDDIR)uuid.o:	$(INCLUDE_FILES)openedge/util/uuid.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)general.o:	$(INCLUDE_FILES)openedge/device/general.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

############################ Openedge Sys
$(BUILDDIR)info.o:	$(INCLUDE_FILES)openedge/sys/info.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)cpuload.o:	$(INCLUDE_FILES)openedge/sys/cpuload.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)netload.o:	$(INCLUDE_FILES)openedge/sys/netload.cc
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

############################ Tasks
simple.task: $(BUILDDIR)simple.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS)
$(BUILDDIR)simple.task.o: $(TASK_SOURCE_FILES)simple.task/simple.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

simple2.task: $(BUILDDIR)simple2.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS)
$(BUILDDIR)simple2.task.o: $(TASK_SOURCE_FILES)simple2.task/simple2.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

aop10t.pilot.task: $(BUILDDIR)aop10t.pilot.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS)
$(BUILDDIR)aop10t.pilot.task.o: $(TASK_SOURCE_FILES)aop10t.pilot.task/aop10t.pilot.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

#sys.mdns.manage.task
sys.mdns.manage.task: $(BUILDDIR)sys.mdns.manage.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -ltinyxml2
$(BUILDDIR)sys.mdns.manage.task.o: $(TASK_SOURCE_FILES)sys.mdns.manage.task/sys.mdns.manage.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

qual.dmr.task: $(BUILDDIR)qual.dmr.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS)
$(BUILDDIR)qual.dmr.task.o: $(TASK_SOURCE_FILES)qual.dmr.task/qual.dmr.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@


# i2c sensor logging task for injection modeling prj
logger.task: $(BUILDDIR)logger.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -li2c
$(BUILDDIR)logger.task.o: $(TASK_SOURCE_FILES)logger.task/logger.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

uvlc.ag.control.task: $(BUILDDIR)uvlc.ag.control.task.o \
					$(BUILDDIR)dkm_dx3000.o 
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmodbus
$(BUILDDIR)uvlc.ag.control.task.o: $(TASK_SOURCE_FILES)uvlc.ag.control.task/uvlc.ag.control.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)dkm_dx3000.o: $(SUPPORT_SOURCE_FILES)device/dkm_dx3000.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@


uvlc.control.task: $(BUILDDIR)uvlc.control.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmosquittopp -lmosquitto
$(BUILDDIR)uvlc.control.task.o: $(TASK_SOURCE_FILES)uvlc.control.task/uvlc.control.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

agsys.manage.task: $(BUILDDIR)agsys.manage.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmodbus
$(BUILDDIR)agsys.manage.task.o: $(TASK_SOURCE_FILES)agsys.manage.task/agsys.manage.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

modbusRTU.task: $(BUILDDIR)modbusRTU.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq
$(BUILDDIR)modbusRTU.task.o: $(TASK_SOURCE_FILES)modbusRTU.task/modbusRTU.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

# sysmon task
sysmon.task: $(BUILDDIR)sysmon.o \
			 $(BUILDDIR)info.o \
			 $(BUILDDIR)cpuload.o \
			 $(BUILDDIR)netload.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq
$(BUILDDIR)sysmon.o: $(TASK_SOURCE_FILES)sysmon.task/sysmon.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

procmanage.task: $(BUILDDIR)procmanage.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq
$(BUILDDIR)procmanage.o: $(TASK_SOURCE_FILES)procmanage.task/procmanage.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

pcan.mqtt.task: $(BUILDDIR)pcan.mqtt.task.o \
				$(BUILDDIR)dkm_dx3000.o 
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmosquittopp -lmosquitto
$(BUILDDIR)pcan.mqtt.task.o: $(TASK_SOURCE_FILES)pcan.mqtt.task/pcan.mqtt.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

dx3000.control.task: $(BUILDDIR)dx3000.control.task.o \
				$(BUILDDIR)dkm_dx3000.o 
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmosquittopp -lmosquitto -lmodbus
$(BUILDDIR)dx3000.control.task.o: $(TASK_SOURCE_FILES)dx3000.control.task/dx3000.control.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)dkm_dx3000.o: $(SUPPORT_SOURCE_FILES)device/dkm_dx3000.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

A.task: $(BUILDDIR)A.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH)~ -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmosquittopp -lmosquitto -lmodbus
$(BUILDDIR)A.task.o: $(TASK_SOURCE_FILES)A/A.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

B.task: $(BUILDDIR)B.task.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH)~ -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lczmq -lzmq -lmosquittopp -lmosquitto -lmodbus
$(BUILDDIR)B.task.o: $(TASK_SOURCE_FILES)B/B.task.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

############################ Services

lsis.fenet.connector.service: $(BUILDDIR)lsis.fenet.connector.service.o \
							$(BUILDDIR)xgt.protocol.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) ./lib/$(ARCH)/libsockpp.a
$(BUILDDIR)lsis.fenet.connector.service.o: $(SERVICE_SOURCE_FILES)lsis.fenet.connector.service/lsis.fenet.connector.service.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)xgt.protocol.o: $(SERVICE_SOURCE_FILES)lsis.fenet.connector.service/xgt.protocol.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

mongodb.connector.service: $(BUILDDIR)mongodb.connector.service.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lbson-1.0 -lmongoc-1.0
$(BUILDDIR)mongodb.connector.service.o: $(SERVICE_SOURCE_FILES)mongodb.connector.service/mongodb.connector.service.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

mqtt.publisher.service: $(BUILDDIR)mqtt.publisher.service.o \
						$(BUILDDIR)mqtt.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lmosquittopp
$(BUILDDIR)mqtt.publisher.service.o: $(SERVICE_SOURCE_FILES)mqtt.publisher.service/mqtt.publisher.service.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@
$(BUILDDIR)mqtt.o: $(SERVICE_SOURCE_FILES)mqtt.publisher.service/mqtt.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

modbus.rtu.service: $(BUILDDIR)modbus.rtu.service.o
	$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -shared -o $(BUILDDIR)$@ $^ $(LDLIBS) -lmodbus
$(BUILDDIR)modbus.rtu.service.o: $(SERVICE_SOURCE_FILES)modbus.rtu.service/modbus.rtu.service.cc
	$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

all : edge tasks services

test : oeware_test
tasks : simple.task simple2.task aop10t.pilot.task sys.mdns.manage.task qual.dmr.task sysmon.task procmanage.task uvlc.ag.control.task pcan.mqtt.task agsys.manage.task uvlc.contro.task
services : lsis.fenet.connector.service mongodb.connector.service mqtt.publisher.service modbus.rtu.service
deploy : FORCE
	cp $(BUILDDIR)*.task $(BUILDDIR)edge $(BINDIR)
clean : FORCE 
		$(RM) $(BUILDDIR)*.o $(BUILDDIR)openedge $(BUILDDIR)edge $(BUILDDIR)*.task $(BUILDDIR)*.service
FORCE : 