// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf
// reference ExploringBeagleBone.pdf

#include "fingerprintModule.h"

void checkGPIO(int file)
{
	write(file, "1", 1);
	sleep(1);
	write(file, "0", 1);
	sleep(1);
}

int checksum(uint32_t cmd[], uint32_t length) 
{
	int checksum = 0;
	// check sum is last two bytes... reference 2
	for (int i = 6; i < length-2; i++)
		checksum += cmd[i];
	return checksum;
}

void clearBuffer()
{
	// clear buffer
	for(int i = 0; i < BUF_SIZE; i++)
	{
		fingerprintBuffer[i] = 0;
	}
}

int minEnrolled, maxEnrolled, totalPrints;

int main()
{
	syslog(LOG_INFO, "start");

	// fingerprint buffer
	int state = 0, i = 0, image = 0, ret, ledFile, file;
	char name;

	minEnrolled = 0, totalPrints = 0;

	// initialize and check gpio LED pin (GPIO1_18 = 1*32 + 18 = 50)
	ledFile = open("/sys/class/gpio/gpio50/value", O_WRONLY);
	if(ledFile == -1)
		syslog(LOG_INFO, "UART file didn't open");
	
	checkGPIO(ledFile);
	
	// device file
	file = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY);
	if(file == -1)
	{
		printf("UART file didnt open\n");
		syslog(LOG_INFO, "UART file didn't open");
		return -1;
	}
	else
		printf("UART file opened\n");
	
	// set file parameters
	struct termios options;
	tcgetattr(file, &options);
	
	// set control modes... baud 57600, 8 bit, enable receiver, local
	options.c_cflag = B57600 | CS8 | CREAD | CLOCAL;
	// set input modes... ignore parity, look for newline
	options.c_iflag = IGNPAR | ICRNL;
	// flush all info in file
	tcflush(file, TCIFLUSH);
	// no change delay
	tcsetattr(file, TCSANOW, &options);
	
	while(1) // may want to change to stop the fingerprint module TODO
	{
		switch(state)
		{
			// check fingerprint
			case 0:
			{
				printf("What would you like to do?\n");
				printf("Type 1 to enroll new print\n");
				printf("Type 2 to check database\n");
				printf("Type 3 to erase print\n");
				printf("Type 4 to erase database\n");
				printf("Type 5 to test the sensor\n");

				scanf("%d", &state);
				
				break;
			}
			
			// add fingerprint
			case 1:
			{			
				printf("place finger on scanner\n");
				sleep(3);

				tcflush(file, TCIFLUSH);
				clearBuffer();
				
				// get fingerprint image
				ret = write(file, &GetImage, GetImageLength);
				syslog(LOG_INFO, "get image wrote %d bytes", ret);
				
				for(i = 0; i < GetImageLength; i++)
					printf("get image: %d\n", GetImage[i]);
				
				read(file, (void *)fingerprintBuffer, GetImageLength);
				
				// 10th position will be 0 if finger sensed
				if(fingerprintBuffer[9] != 0)
				{
					printf("no finger sensed\n");
					state = 0;
					break;
				}
				
				printf("adding fingerprint to database\n");
				
				// repeat 6 times for adding fingerprint image
				for(image = 0; image < NUM_IMAGES; image++)
				{
					clearBuffer();
					tcflush(file, TCIFLUSH);

					// step 1: get image
					ret = write(file, &GetImage, GetImageLength);
					syslog(LOG_INFO, "get image wrote %d bytes", ret);

					if(image == 0)
					{
						// get response
						read(file, (void *)fingerprintBuffer, GetImageLength);				
						clearBuffer();
					}
				
					maxEnrolled = image+1;
					
					// step 2: generate char
					GenChar[10] = maxEnrolled;
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					ret = write(file, &GenChar, GenCharLength);
					syslog(LOG_INFO, "genchar wrote %d bytes", ret);					
					
					if(image == 0)
					{
						// get response
						read(file, (void *)fingerprintBuffer, 12);
					}

					syslog(LOG_INFO, "character #%d of %d generated", image, NUM_IMAGES); 
				}
				
				// register fingerprint
				clearBuffer();
				tcflush(file, TCIFLUSH);

				ret = write(file, &RegModel, RegModelLength);
				syslog(LOG_INFO, "reg wrote %d bytes", ret);
				
				// get response
				read(file, (void *)fingerprintBuffer, RegModelLength);	
				
				if(fingerprintBuffer[11] == 0xa)
					printf("Unique fingerprints temporarily stored\n");
				else
					printf("Adding fingerprint failed\n");
				
				// store fingerprint
				StoreChar[StoreCharLength-3] = totalPrints;
				StoreChar[StoreCharLength-1] = checksum(StoreChar, StoreCharLength);
				ret = write(file, &StoreChar, StoreCharLength);
				syslog(LOG_INFO, "store char wrote %d bytes", ret);
				
				// get response
				read(file, (void *)fingerprintBuffer, 12);	
				
				printf("Unique fingerprints added\n");
				
				state = 0;
				break;
			}
			// search for match
			case 2:
			{
				printf("place finger on scanner\n");
				sleep(3);		
				
				tcflush(file, TCIFLUSH);
				clearBuffer();
			
				// write to device
				ret = write(file, &GetImage, GetImageLength);
				syslog(LOG_INFO, "get image wrote %d bytes", ret);
				
				// get response
				read(file, (void *)fingerprintBuffer, GetImageLength);
				
				// 10th position will be 0 if success (finger sensed)
				if(fingerprintBuffer[9] != 0)
				{
					printf("no finger sensed\n");
					state = 0;
					break;
				}
				else
					printf("User detected...\n");
				
				clearBuffer();
				tcflush(file, TCIFLUSH)
						
				// generate characters to index 1
				GenChar[10] = minEnrolled;
				// generate check sum for last two bytes
				GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
				ret = write(file, GenChar, GenCharLength);
				syslog(LOG_INFO, "genchar wrote %d bytes", ret);
					
				// get response
				read(file, (void *)fingerprintBuffer, 12);
					
				clearBuffer();
					
				// last four bytes in search are min numEnrolled, max numEnrolled, 2 bytes for check sum
				Search[SearchLength-3] = maxEnrolled;
				Search[SearchLength-1] = checksum(Search, SearchLength);
				ret = write(file, &Search, SearchLength);
				syslog(LOG_INFO, "search wrote %d bytes", ret);
					
				// get response
				read(file, (void *)fingerprintBuffer, 16);
				
				if(fingerpirintBuffer[13] >= 0x50)
					printf("fingerprint passed\n");
				else
					printf("fingerprint failed\n");
				
				state = 0;
				break;
			}
			
			// remove all fingerprint
			case 3:
			{
				ret = write(file, &Empty, EmptyLength);
				syslog(LOG_INFO, "empty wrote %d bytes", ret);
				
				// get response
				read(file, (void *)fingerprintBuffer, EmptyLength);
				
				printf("removed all fingerprints from the database\n");
				
				state = 0;
				break;
			}
		}
	}
	close(file);
	close(ledFile);

	return 0;
}
