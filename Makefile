include Makefile.inc

TARGET = parkrpi

OBJ = main.o system_server.o shared_memory.o web_server.o input.o gui.o
CXXOBJ = camera_HAL.o ControlThread.o

all: ${OBJ} ${CXXOBJ}
	${CXX} -o ${TARGET} ${OBJ} ${CXXOBJ} ${CXXLIBS} -lpthread

system_server.o: ${SYSTEM}/system_server.h ${SYSTEM}/system_server.c
	${CC} -g ${CFLAGS} -c ${SYSTEM}/system_server.c

shared_memory.o: ${SYSTEM}/shared_memory.h ${SYSTEM}/shared_memory.c
	${CC} -g ${CFLAGS} -c ${SYSTEM}/shared_memory.c

gui.o: ${UI}/gui.h ${UI}/gui.c
	${CC} -g ${CFLAGS} -c ${UI}/gui.c

input.o: ${UI}/input.h ${UI}/input.c
	${CC} -g ${CFLAGS} -c ${UI}/input.c

web_server.o: ${WEB_SERVER}/web_server.h ${WEB_SERVER}/web_server.c
	${CC} -g ${CFLAGS} -c ${WEB_SERVER}/web_server.c

camera_HAL.o: ${HAL}/camera_HAL.cpp
	${CXX} -g ${CXXFLAGS} -c ${HAL}/camera_HAL.cpp

ControlThread.o: ${HAL}/ControlThread.cpp
	${CXX} -g ${CXXFLAGS} -c ${HAL}/ControlThread.cpp

clean:
	rm -rf *.o parkrpi
