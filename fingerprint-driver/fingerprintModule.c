// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf
// reference ExploringBeagleBone.pdf

#include "fingerprintModule.h"

void checkGPIO(int file)
{
	printf("checking if LED is flashing\n");
	write(file, "1", 1);
	usleep(100000);
	write(file, "0", 1);
	usleep(100000);
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
	int state = 0, i = 0, image = 0;
	char name;

	minEnrolled = 0, totalPrints = 0;

	// initialize and check gpio LED pin (GPIO1_18 = 1*32 + 18 = 50)
	int ledFile = open("/sys/class/gpio/gpio50/value", O_WRONLY);
	if(ledFile == -1)
	{
		printf("LED file didnt open\n");
		syslog(LOG_INFO, "UART file didn't open");
	}
	else
		printf("LED file opened\n");
	
	checkGPIO(ledFile);
	
	// device file
	int file;
	file = open("/dev/ttyO0", O_RDWR | O_NOCTTY | O_NDELAY);
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

				printf("before flush\n");

				tcflush(file, TCIFLUSH);
				clearBuffer();
				printf("after clear buffer\n");
				
				// get fingerprint image
				printf("before first write\n");
				
				write(file, &GetImage, GetImageLength);
				
				for(i = 0; i < GetImageLength; i++)
					printf("get image: %d\n", GetImage[i]);
				
				read(file, (void *)fingerprintBuffer, sizeof(char));
				while(fingerprintBuffer != start)
				{
					clearBuffer();
					read(file, (void *)fingerprintBuffer, sizeof(char));
				}
				read(file, (void *)fingerprintBuffer, 11);
				
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

					// step 1: get image
					write(file, &GetImage, GetImageLength);
					
					if(image == 0)
					{
						// get response
						read(file, (void *)fingerprintBuffer, sizeof(char));
						while(fingerprintBuffer != start)
						{
							clearBuffer();
							read(file, (void *)fingerprintBuffer, sizeof(char));
						}
						read(file, (void *)fingerprintBuffer, 11);
						
						printf("getImg resp: %x\n", fingerprintBuffer[i]);																		

						clearBuffer();
					}
				
					maxEnrolled = image+1;
					
					// step 2: generate char
					GenChar[10] = maxEnrolled;
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					write(file, &GenChar, GenCharLength);
					
					if(image == 0)
					{
						// get response
						read(file, (void *)fingerprintBuffer, sizeof(char));
						while(fingerprintBuffer != start)
						{
							clearBuffer();
							read(file, (void *)fingerprintBuffer, sizeof(char));
						}
						read(file, (void *)fingerprintBuffer, GenCharLength);
					}

					printf("character #%d of %d generated\n", image, NUM_IMAGES); 
				}
				
				// register fingerprint
				clearBuffer();
				
				write(file, &RegModel, RegModelLength);
				
				// get response
				read(file, (void *)fingerprintBuffer, sizeof(char));
				while(fingerprintBuffer != start)
					read(file, (void *)fingerprintBuffer, sizeof(char));

				read(file, (void *)fingerprintBuffer, RegModelLength);	
				
				if(fingerprintBuffer[11] == 0xa)
					printf("Unique fingerprints temporarily stored\n");
				else
					printf("Adding fingerprint failed\n");
				
				// store fingerprint
				StoreChar[StoreCharLength-3] = totalPrints;
				StoreChar[StoreCharLength-1] = checksum(StoreChar, StoreCharLength);
				write(file, &StoreChar, StoreCharLength);
				
				// get response
				read(file, (void *)fingerprintBuffer, sizeof(char));
				while(fingerprintBuffer != start)
					read(file, (void *)fingerprintBuffer, sizeof(char));

				read(file, (void *)fingerprintBuffer, StoreCharLength);	
				
				printf("Unique fingerprints added\n");
				syslog(LOG_INFO, "Unique fingerprints added");
				
				state = 0;
				break;
			}
		/*			
			// search for match
			case 2:
			{
				printf("place finger on scanner\n");
				sleep(3);		
				
				tcflush(file, TCIFLUSH);
				clearBuffer();
			
				// write to device
				write(file, &GetImage, GetImageLength);
				
				// write cmd to buffer for checking
				for(i = 0; i < GetImageLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						read(file, &fingerprintBuffer[i], sizeof(char));
				}
				
				// 10th position will be 0 if success (finger sensed)
				// generate the fingerprint then search
				if(fingerprintBuffer[9] == 0)
				{
					syslog(LOG_INFO, "User detected...");
					clearBuffer();
					
					// generate characters to index 1
					GenChar[10] = minEnrolled;
					// generate check sum for last two bytes
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					write(file, GenChar, GenCharLength);
					
					// write cmd to buffer for checking
					for(i = 0; i < GenCharLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							read(file, &fingerprintBuffer[i], sizeof(char));
					}
					
					clearBuffer();
					
					// last four bytes in search are min numEnrolled, max numEnrolled, 2 bytes for check sum
					Search[SearchLength-3] = minEnrolled;
					Search[SearchLength-2] = maxEnrolled;
					Search[SearchLength-1] = checksum(Search, SearchLength);
					write(file, &Search, SearchLength);
					
					// write cmd to buffer for checking
//					for(i = 0; i < SearchLength; i++)
//					{
						// wait for the start of the cmd
//						if((i == 0) && (fingerprintBuffer[i] != start))
							read(file, (void *)fingerprintBuffer, SearchLength); //sizeof(char));	
//					}
				}
				else
					break;
			}
			
			// remove fingerprint
			case 3:
			{
				syslog(LOG_INFO, "Type user name to delete data");
				scanf("%c", &name);
				// TODO!! check file for name and fingerprint number then remove
			}
		*/}
	}
	close(file);
	close(ledFile);

	return 0;
}
