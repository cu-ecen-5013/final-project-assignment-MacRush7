void buzzer(int passedFile)
{
	char *noAlarm = "0";
	int i = 0, gpioFile;
	
	// set GPIO header 9, pin 12 to output = pin 60 (32*1 + 28)
	gpioFile = open("/sys/class/gpio/gpio60/value", O_WRONLY);
	if(gpioFile == -1)
		syslog(LOG_ERR, "buzzer file didn't open");
	
	// turn on buzzer for three seconds and turn off for 1 second... repeat
	for(i = 0; i < 3; i++)
	{
		lseek(gpioFile, 0, SEEK_SET);
		write(gpioFile, "1", 1);
		sleep(2);
		lseek(gpioFile, 0, SEEK_SET);
		write(gpioFile, "0", 1);
		sleep(1);
	}
	
	// reset file
	lseek(passedFile, 0, SEEK_SET);
	int ret = write(passedFile, noAlarm, 1);
	if(ret == -1)
		syslog(LOG_ERR, "file was not reset to 0");
	
	close(gpioFile);
}

int main()
{
	char *buffer;
	char *fileStarter = "0";
	int ret = 3, i = 0, buzzerFile;
	
	// setup file
	if((buzzerFile = open("/dev/buzzerAlarm", O_RDWR | O_CREAT, 0666)) == -1)
	{
		syslog(LOG_ERR, " buzzer file was not found or opened");
		return -1;
	}
	
	lseek(buzzerFile, 0, SEEK_SET);
	write(buzzerFile, fileStarter, 1);
	
	while(1)
	{
		// check file for alarm
		lseek(buzzerFile, 0, SEEK_SET);
		ret = read(buzzerFile, buffer, 1);
		if(ret == 0)
			syslog(LOG_ERR, "nothing in buzzer file to read");

		ret = atoi(buffer);
		// file contains "1"
		if(ret == 1)
			buzzer(buzzerFile);
	}	

	close(buzzerFile);
}
