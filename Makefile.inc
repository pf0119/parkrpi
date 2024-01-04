DIR = ..

SYSTEM = ./system
UI = ./ui
WEB_SERVER = ./web_server
HAL = ./hal

DIRS = ${SYSTEM} ${UI} ${WEB_SERVER}

# -I./ is to add mq_message.h
INC = -I${SYSTEM} -I${UI} -I${WEB_SERVER} -I${HAL} -I./

#CC = gcc
CC = ~/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-gcc
#CFLAGS = -Wall -Wextra -Wall ${INC}
CFLAGS = -Wall ${INC}
#CXX = g++
CXX = ~/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-g++
CXXLIBS = -lpthread -lm -lrt -Wl,--no-as-needed -ldl -lseccomp
CXXFLAGS = ${INC} -g -O0 -std=c++14
