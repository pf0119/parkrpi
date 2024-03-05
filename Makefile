include Makefile.inc

TARGET = parkrpi

OBJ = main.o system_server.o web_server.o input.o gui.o shared_memory.o dump_state.o hardware.o
#CXXOBJ = camera_HAL.o ControlThread.o
CSSOBJ = 
LIBS = libcamera.oem.so libcamera.toy.so

all: $(OBJ) $(CXXOBJ) $(LIBS)
	$(CXX) -o $(TARGET) $(OBJ) $(CXXOBJ) $(CXXLIBS) -lpthread

system_server.o: $(SYSTEM)/system_server.h $(SYSTEM)/system_server.c
	$(CC) -g $(CFLAGS) -c $(SYSTEM)/system_server.c

dump_state.o: $(SYSTEM)/dump_state.h $(SYSTEM)/dump_state.c
	$(CC) -g $(CFLAGS) -c $(SYSTEM)/dump_state.c

shared_memory.o: $(SYSTEM)/shared_memory.h $(SYSTEM)/shared_memory.c
	$(CC) -g $(CFLAGS) -c $(SYSTEM)/shared_memory.c

gui.o: $(UI)/gui.h $(UI)/gui.c
	$(CC) -g $(CFLAGS) -c $(UI)/gui.c

input.o: $(UI)/input.h $(UI)/input.c
	$(CC) -g $(CFLAGS) -c $(UI)/input.c

web_server.o: $(WEB_SERVER)/web_server.h $(WEB_SERVER)/web_server.c
	$(CC) -g $(CFLAGS) -c $(WEB_SERVER)/web_server.c

camera_HAL.o: $(HAL)/camera_HAL.cpp
	$(CXX) -g $(CXXFLAGS) -c $(HAL)/camera_HAL.cpp

ControlThread.o: $(HAL)/ControlThread.cpp
	$(CXX) -g $(CXXFLAGS) -c $(HAL)/ControlThread.cpp

hardware.o: $(HAL)/hardware.c
	$(CC) -g $(CFLAGS) -c $(HAL)/hardware.c

.PHONY: libcamera.oem.so
libcamera.oem.so:
	$(CC) -g -shared -fPIC -o libcamera.oem.so $(INCLUDES) $(CXXFLAGS) $(HAL)/oem/camera_HAL_oem.cpp $(HAL)/oem/ControlThread.cpp

.PHONY: libcamera.toy.so
libcamera.toy.so:
	$(CC) -g -shared -fPIC -o libcamera.toy.so $(INCLUDES) $(CXXFLAGS) $(HAL)/toy/camera_HAL_toy.cpp $(HAL)/toy/ControlThread.cpp

# libcamera.toy.so:
# 	$(CC) -g -shared -fPIC -o libfoo2.so foo2.c

# libfoo3.so:
# 	cc -g -Wl,-Bsymbolic -Wl,-allow-shlib-undefined \
# 		-shared -fPIC -o libfoo3.so foo3.

# camera_HAL.o: $(HAL)/camera_HAL.cpp
# 	$(CXX) -g $(INCLUDES) $(CXXFLAGS) -c  $(HAL)/camera_HAL.cpp

# ControlThread.o: $(HAL)/ControlThread.cpp
# 	$(CXX) -g $(INCLUDES) $(CXXFLAGS) -c  $(HAL)/ControlThread.cpp

GO = GOGC=off go
# go module
# MODULE = $(shell env GO111MODULE=on $(GO) list -m)
#

LDFLAGS += -s -w

## build: Build
.PHONY: fe
fe: | ; $(info $(M) building fe…)
	cd api && npm ci && cd ../toy-fe && npm ci && npm run build && cp -a dist/* ../be/frontend/dist/

.PHONY: be
be: | ; $(info $(M) building be…)
	cd be && env GOARCH=arm64 CGO_ENABLED=1 GOOS=linux GOGC=off GO111MODULE=on  $(GO) build -ldflags '$(LDFLAGS)' -o be

.PHONY: modules
modules:
	cd drivers/sensor && make

.PHONY: clean
clean:
	rm -rf *.o parkrpi
