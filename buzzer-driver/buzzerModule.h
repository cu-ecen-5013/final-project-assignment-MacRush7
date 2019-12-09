/*#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wiringPi.h>
#include <pthread.h>
*/
#include <linux/mutex.h>

#define buzzerPin		2

pthread_mutex_t alarmLock;

void buzzer(int passedFile);
