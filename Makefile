# normal simple makefile

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CCFLAGS),)
	CCFLAGS= -g -Wall
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -lwiringPi -;wiringPiDev -pthread -lm -lcrypt -lrt
endif

all: buzzerModule fingerprintModule

buzzerModule: buzzer-driver/buzzerModule.c
	$(CC) $(CCFLAGS) -o buzzerModule buzzer-driver/buzzerModule.c buzzer-driver/buzzerModule.h $(LDFLAGS)

fingerprintModule: fingerprint-driver/fingerprintModule.c
	$(CC) $(CCFLAGS) -o fingerprintModule fingerprint-driver/fingerprintModule.c fingerprint-driver/fingerprintModule.h $(LDFLAGS)

clean:
	-rm -f *.o *.d
	-rm -f buzzerModule
	-rm -f fingerprintModule
