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

all: buzzerModule fingerprintModule buzzerTest

buzzerModule: buzzer-driver/buzzerModule.c
	$(CC) $(CCFLAGS) -o buzzerModule buzzer-driver/buzzerModule.c $(LDFLAGS)

fingerprintModule: fingerprint-driver/fingerprintModule.c
	$(CC) $(CCFLAGS) -o fingerprintModule fingerprint-driver/fingerprintModule.c $(LDFLAGS)

buzzerTest: buzzer-driver/buzzerTest.c
	$(CC) $(CCFLAGS) -o buzzerTest buzzer-driver/buzzerTest.c $(LDFLAGS)

clean:
	-rm -f *.o *.d
	-rm -f buzzerModule
	-rm -f fingerprintModule
	-rm -f buzzerTest
