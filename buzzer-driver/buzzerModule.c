void buzzer(int passedFile)
{
	char *noAlarm = "N";
	int i = 0;
	
	// set GPIO header 9, pin 12 to output = pin 60 (32*1 + 28)
	int file = open("/sys/class/gpio/gpio60/value", O_WRONLY);
	if(file == -1)
	{
		printf("buzzer file didnt open\n");
		syslog(LOG_INFO, "buzzer file didn't open");
	}
	else
		printf("buzzer file opened\n");	
	
	// turn on buzzer for three seconds and turn off for 1 second... repeat
	for(i = 0; i < 3; i++)
	{
		write(file, "1", 1);
		usleep(3000000);
		write(file, "0", 1);
		usleep(1000000);
	}
	
	// reset file
	int ret = write(passedFile, noAlarm, strlen(noAlarm));
	if(ret == -1)
		syslog(LOG_ERROR, "file was not reset to no");
	
	close(file);
}

int main()
{
	char *buffer = "0";
	char *yesAlarm = "Y";
	int ret = 3, i = 0, buzzerFile;
	
	// setup file
	if((buzzerFile = open("/dev/buzzerAlarm", O_RDWR | O_CREAT, 0666)) == -1)
	{
		syslog(LOG_ERROR, "file was not found or opened");
		return -1;
	}
	
	while(1)
	{
		// check file for alarm
		int ret = read(buzzerFile, buffer, strlen(yesAlarm));
		if(ret == 0)
			syslog(LOG_ERROR, "nothing in file to read");

		ret = strcmp(buffer, yesAlarm);
		// file contains "Y"
		if(ret == 0)
			buzzer(buzzerFile);
	}	

	close(buzzerFile);
}
