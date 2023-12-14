DIR = ..

SYSTEM = ./system
UI = ./ui
WEB_SERVER = ./web_server
HAL = ./hal

DIRS = ${SYSTEM} ${UI} ${WEB_SERVER}

INC = -I${SYSTEM} -I${UI} -I${WEB_SERVER} -I${HAL}

CC = gcc
CFLAGS = -Wall -Wextra -Werror ${INC}
CXX = g++
CXXLIBS = -lpthread -lm -lrt
CXXFLAGS = ${INC} -g -O0 -std=c++14
