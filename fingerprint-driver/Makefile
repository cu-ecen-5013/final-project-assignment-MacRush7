# normal simple makefile
ifdef CROSS_COMPILE
	CC= arm-unknown-linux-gnueabi-gcc
else
	CC= gcc -Wall -Werror
endif

INCLUDE_DIR= -I/usr/local/include -L/usr/local/lib -lwiringPi
CFLAGS= -g $(INCLUDE_DIRS)
LIBS= 
OBJS= fingerprintModule.o

all= fingerprintModule

fingerprintModule: $(OBJS)
	$(CC) -o fingerprintModule $(OBJS) $(INCLUDE_DIR)

fingerprintModule.o: fingerprintModule.c
	$(CC) -c fingerprintModule.c $(INCLUDE_DIR)

clean:
	-rm -f *.o *.d
	-rm -f fingerprintModule
