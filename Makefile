include Makefile.inc

TARGET = parkrpi

OBJ= main.o system_server.o web_server.o input.o gui.o

all: ${OBJ}
	${CC} -o ${TARGET} ${OBJ}

system_server.o: ${SYSTEM}/system_server.h ${SYSTEM}/system_server.c
	${CC} -g ${CFLAGS} -c ${SYSTEM}/system_server.c

gui.o: ${UI}/gui.h ${UI}/gui.c
	${CC} -g ${CFLAGS} -c ${UI}/gui.c

input.o: ${UI}/input.h ${UI}/input.c
	${CC} -g ${CFLAGS} -c ${UI}/input.c

web_server.o: ${WEB_SERVER}/web_server.h ${WEB_SERVER}/web_server.c
	${CC} -g ${CFLAGS} -c ${WEB_SERVER}/web_server.c

clean:
	rm -rf *.o
	rm -rf *.o parkrpi
