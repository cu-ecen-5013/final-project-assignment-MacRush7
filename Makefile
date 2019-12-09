# normal simple makefile

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CCFLAGS),)
	CCFLAGS= -g -Wall
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -lwiringPi -lwiringPiDev -lpthread -lm -lcrypt -lrt
endif

all: buzzerModule fingerprintModule

buzzerModule: buzzer-driver/buzzerModule.c
	$(CC) $(CCFLAGS) -o buzzerModule buzzer-driver/buzzerModule.c $(LDFLAGS)

fingerprintModule: fingerprint-driver/fingerprintModule.c
	$(CC) $(CCFLAGS) -o fingerprintModule fingerprint-driver/fingerprintModule.c $(LDFLAGS)

clean:
	-rm -f *.o *.d
	-rm -f buzzerModule
	-rm -f fingerprintModule
