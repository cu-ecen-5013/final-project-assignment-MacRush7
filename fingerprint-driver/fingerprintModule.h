#include <wiringPi.h>
#include <stdio.h>

#define LedPin    	0		// TODO
#define ButtonPin 	1		// TODO

#define BUF_SIZE	100
#define NUM_IMAGES	6

int checkButton();
int checksum(uint8_t cmd[], int length);
void clearBuffer();
void fingerprint();
