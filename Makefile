CC = gcc
CFLAGS = -c -g -Wall
LDFLAGS = 
OBJECTS = rpjtag_stateMachine.o rpjtag.o rpjtag_bit_reader.o rpjtag_io.o

all: RpiJtag

rpjtag.o: rpjtag.c rpjtag.h rpjtag_io.h rpjtag_stateMachine.h rpjtag_bit_reader.h
	${CC} ${CFLAGS} rpjtag.c -o rpjtag.o

rpjtag_io.o: rpjtag_io.c rpjtag_io.h
	${CC} ${CFLAGS} rpjtag_io.c -o rpjtag_io.o

rpjtag_stateMachine.o: rpjtag_stateMachine.c rpjtag_stateMachine.h rpjtag_io.h
	${CC} ${CFLAGS} rpjtag_stateMachine.c -o rpjtag_stateMachine.o

rpjtag_bit_reader.o: rpjtag_bit_reader.c rpjtag_bit_reader.h rpjtag_stateMachine.h rpjtag_io.h rpjtag.h
	${CC} ${CFLAGS} rpjtag_bit_reader.c -o rpjtag_bit_reader.o

RpiJtag: rpjtag.o rpjtag_stateMachine.o rpjtag_bit_reader.o rpjtag_io.o
	${CC} ${LDFLAGS} ${OBJECTS} -o RpiJtag

clean:
	rm -f ${OBJECTS} RpiJtag

.PHONY: clean
