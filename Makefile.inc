DIR = ..

SYSTEM = ./system
UI = ./ui
WEB_SERVER = ./web_server
HAL = ./hal

DIRS = ${SYSTEM} ${UI} ${WEB_SERVER}

# -I./ is to add mq_message.h
INC = -I${SYSTEM} -I${UI} -I${WEB_SERVER} -I${HAL} -I./

CC = gcc
CFLAGS = -Wall -Wextra -Werror ${INC}
CXX = g++
CXXLIBS = -lpthread -lm -lrt -Wl,--no-as-needed -ldl -lseccomp
CXXFLAGS = ${INC} -g -O0 -std=c++14
