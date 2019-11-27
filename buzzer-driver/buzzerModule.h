#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <iolib.c>

int buzzerFile;

#define BuzzerPin    	12	
#define header		9

void buzzer();
