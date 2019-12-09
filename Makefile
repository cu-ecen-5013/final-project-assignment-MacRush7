# kernel makefile
ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CCFLAGS),)
	CCFLAGS= -g -Wall
endif

ifeq ($(LDFLAGS),)
	LDFLAGS= -pthread -lrt
endif

all: fingerprintModule buzzerModule

fingerprintModule: fingerprint-driver/fingerprintModule.c
	$(CC) $(CCFLAGS) -o fingerprintModule fingerprint-driver/fingerprintModule.c fingerprint-driver/fingerprintModule.h $(LDFLAGS)

buzzerModule: buzzer-driver/buzzerModule.c
	$(CC) $(CCFLAGS) -o buzzerModule buzzer-driver/buzzerModule.c buzzer-driver/buzzerModule.h $(LDFLAGS)

clean:
	-rm -f *.o *.d
	-rm -f fingerprintModule
	-rm -f buzzerModule
