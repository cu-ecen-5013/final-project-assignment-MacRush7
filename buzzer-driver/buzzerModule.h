#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wiringPi.h>
#include <pthread.h>

#define buzzerPin	2

pthread_t ThreadIdx[3];
pthread_mutex_t alarmLock;

void buzzer(int passedFile);
