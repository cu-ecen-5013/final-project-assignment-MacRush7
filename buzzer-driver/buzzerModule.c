#include "buzzerModule.h"

void buzzer(int passedFile)
{
	char *noAlarm = "0";
	int i = 0, ret = 0;
	char buffer[1];

	// turn on buzzer for three seconds and turn off for 1 second... repeat
	for(i = 0; i < 3; i++)
	{
		digitalWrite(buzzerPin, HIGH);
		sleep(2);
		digitalWrite(buzzerPin, LOW);
		sleep(1);
	}
	
	// reset file
//	pthread_mutex_lock(&alarmLock);
	lseek(passedFile, 0, SEEK_SET);
	ret = write(passedFile, noAlarm, 1);
//	pthread_mutex_lock(&alarmLock);

	if(ret == -1)
		syslog(LOG_ERR, "file was not reset to 0");
}

int main(int argc, char *argv[])
{
	char buffer[1];
	char *fileStarter = "0";
	int ret = 3, buzzerFile;

	// daemon variables
	pid_t processId = 0;
	pid_t sid = 0;

	// check for daemon
	if(argc == 2)
	{
		// create child
		if((processId = fork()) == -1)
		{
			syslog(LOG_ERR, "fork failed for daemon");
			return -1;
		}
		
		// kill parent
		if(processId != 0)
		{
			exit(0);
		}

		// new session id
		if((sid = setsid()) == -1)
		{
			syslog(LOG_ERR, "setsid failed for daemon");
			return -1;
		}
			
		// change directory to root
		if((chdir("/")) == -1)
		{
			syslog(LOG_ERR, "chdir failed for daemon");
			return -1;
		}
			
		umask(0);
			
		// close files
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// redirect files to /dev/null
		if((open("/dev/null", O_RDWR)) == -1)
		{
			syslog(LOG_ERR, "first open failed for daemon");
			return -1;
		}

		if((open("/dev/null", O_RDWR)) == -1)
		{
			syslog(LOG_ERR, "second open failed for daemon");
			return -1;
		}

		if((open("/dev/null", O_RDWR)) == -1)
		{
			syslog(LOG_ERR, "third open failed for daemon");
			return -1;
		}			
	}

	if(wiringPiSetup() == -1)
	{
		syslog(LOG_ERR, "wiringPi failed");
		return -1;
	}
	
	pinMode(buzzerPin, OUTPUT);
	digitalWrite(buzzerPin, LOW);
	
	// setup file
	if((buzzerFile = open("/bin/buzzerAlarm", O_RDWR | O_CREAT, 0777)) == -1)
	{
		syslog(LOG_ERR, "buzzer file was not found or opened");
		return -1;
	}
	
	// initialize file
	lseek(buzzerFile, 0, SEEK_SET);
	write(buzzerFile, fileStarter, 1);
	
	while(1)
	{
		// check file for alarm
//		pthread_mutex_unlock(&alarmLock);
		lseek(buzzerFile, 0, SEEK_SET);
		ret = read(buzzerFile, buffer, 1);
//		pthread_mutex_unlock(&alarmLock);
		
		if(ret == 0)
			syslog(LOG_ERR, "nothing in buzzer file to read");
		if(ret == -1)
			syslog(LOG_ERR, "buzzer file read failed");

		ret = atoi(buffer);
		// file contains "1"
		if(ret == 1)
			buzzer(buzzerFile);
	}	

	close(buzzerFile);
}
