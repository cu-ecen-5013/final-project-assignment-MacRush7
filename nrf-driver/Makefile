# normal simple makefile
ifdef CROSS_COMPILE
	CC= arm-unknown-linux-gnueabi-gcc
else
	CC= gcc -Wall -Werror
endif

INCLUDE_DIR= -pthread -lwiringPi -lm -lrt -lcrypt
CFLAGS= -g $(INCLUDE_DIRS)
LIBS= 
OBJS= nrf.o

all= nrf

nrf: $(OBJS)
	$(CC) -o nrf $(OBJS) $(INCLUDE_DIR)

nrf.o: nrf.c
	$(CC) -c nrf.c $(INCLUDE_DIR) 

clean:
	-rm -f *.o *.d
	-rm -f nrf

