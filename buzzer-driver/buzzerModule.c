void buzzer()
{
	char *noAlarm = "N";

	// set GPIO header 9, pin 12 to output
	iolib_setdir(header, BuzzerPin, DIR_OUT);
	
	// turn on buzzer for three seconds and turn off for 1 second
	for(int i = 0; i < 5; i++)
	{
		pin_high(header, BuzzerPin);
		sleep(3);
		pin_low(header, BuzzerPin);
		sleep(1);
	}

	// reset file
	int ret = write(buzzerFile, noAlarm, sizeof(char));
	if(ret == -1)
		syslog(LOG_ERROR, "file was not reset to no");
}

int main()
{
	char *buffer;
	char *yesAlarm = "Y";

	// initialize io library
	iolib_init();

	// setup file
	if((int buzzerFile = open("/dev/buzzerAlarm", "w+")) == -1)
	{
		syslog(LOG_ERROR, "file was not found or opened");
		return -1;
	}
	
	while(1)
	{
		// check file for alarm
		int ret = read(buzzerFile, &buffer[0], sizeof(char));
		if(ret == 0)
			syslog(LOG_ERROR, "nothing in file to read");

		ret = strcmp(buffer, yesAlarm);
		// file contains "Y"
		if(ret == 0)
			buzzer();
	}	

	close(buzzerFile);
	iolib_free();
}
