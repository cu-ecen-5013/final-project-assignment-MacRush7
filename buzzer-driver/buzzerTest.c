#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main ()
{
	int alarmFile;

	alarmFile = open("/bin/buzzerAlarm", O_WRONLY);
	if(alarmFile == -1)
	{
		printf("buzzer file not opened\n");
		syslog(LOG_ERR, "buzzer file was not found or opened");
		return -1;
	}

	// initialize file
	lseek(alarmFile, 0, SEEK_SET);
	write(alarmFile, 1, 1);

	close(alarmFile);
}
