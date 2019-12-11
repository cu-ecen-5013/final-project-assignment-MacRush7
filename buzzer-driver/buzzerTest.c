#include <stdio.h>
#include <stdint.h>

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
	lseek(buzzerFile, 0, SEEK_SET);
	write(buzzerFile, 1, 1);

	close(alarmFile);
}
