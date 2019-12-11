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
	char *test = "1";

	alarmFile = open("/bin/buzzerAlarm", O_WRONLY);
	if(alarmFile == -1)
	{
		printf("buzzer file not opened\n");
		syslog(LOG_ERR, "buzzer file was not found or opened");
		return -1;
	}

	// send 1 to file
	lseek(alarmFile, 0, SEEK_SET);
	int ret = write(alarmFile, test, 1);
	if(ret == -1 || ret == 0)
		printf("write failed\n");
	else
		printf("wrote 1 to file\n");

	close(alarmFile);
}
