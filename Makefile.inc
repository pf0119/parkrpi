DIR = ..

SYSTEM = ./system
UI = ./ui
WEB_SERVER = ./web_server

DIRS = ${SYSTEM} ${UI} ${WEB_SERVER}

INC = -I${SYSTEM} -I${UI} -I${WEB_SERVER}

CC = gcc
CFLAGS = -Wall -Wextra -Werror ${INC}
