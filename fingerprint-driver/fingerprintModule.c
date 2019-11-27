// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf

#include "fingerprintModule.h"

int checkButton()
{
	int buttonFlag = 0;
	
	if(is_high(header, ButtonPin))
	{ 
		//indicate that button has pressed down
		syslog(LOG_INFO, "button pressed");
		buttonFlag = 1;
		pin_high(header, LedPin);   //led on
	}
	else
	{
		syslog(LOG_INFO, "button not pressed");
		buttonFlag = 0;
	}
	
	return buttonFlag;
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

int main()
{
	syslog(LOG_INFO, "start");

	// fingerprint buffer
	int state = 0, button = 0, i = 0, minEnrolled = 1, maxEnrolled = 1;
	int start = 0xEF;
	char name;
	
	// initialize io library
	iolib_init();

	// set pins
	iolib_setdir(header, LedPin, DIR_OUT); 
	iolib_setdir(header, ButtonPin, DIR_IN);
	pin_low(header, LedPin);

	// check button
	button = checkButton();

	// device file
<<<<<<< HEAD
	int file;
	file = open("/dev/fingerprint", "a+");
	if(file == -1)
	{
		syslog(LOG_ERROR, "file didn't open");
=======
	FILE* file;
	file = fopen("/dev/fp_control", O_RDWR | O_CREAT);
	if(file == NULL)
	{
		printf("file didn't open\n");			// failing TODO
>>>>>>> c80ab9fa69fe285b326e94c1c170c25c91262ed7
		return -1;
	}
	
	while(1) // may want to change to stop the fingerprint module TODO
	{
		switch(state)
		{
			// check fingerprint
			case 0:
			{
				// check button for state
				button = checkButton();
				if(is_high(header, ButtonPin))
				{
					// add fingerprint
					state = 1;
					break;
				}
			
				// TODO!! add a check for fingerprint removal request
				
				
				clearBuffer();
			
				// write to device
				write(file, GetImage, GetImgageLength);
				
				// write cmd to buffer for checking
				for(i = 0; i < GetImgageLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						read(file, fingerprintBuffer[i], sizeof(char));
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
							read(file, fingerprintBuffer[i], sizeof(char));
					}
					
					clearBuffer();
					
					// last four bytes in search are min numEnrolled, max numEnrolled, 2 bytes for check sum
					Search[SearchLength-3] = minEnrolled;
					Search[SearchLength-2] = maxEnrolled;
					Search[SearchLength-1] = checksum(Search, SearchLength);
					write(file, Search, SearchLength);
					
					// write cmd to buffer for checking
					for(i = 0; i < SearchLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							read(file, fingerprintBuffer[i], sizeof(char));	
					}
				}
				else
					break;
			}
			
			// add fingerprint
			case 1:
			{				
				// repeat 6 times for adding fingerprint image
				for(i = 0; i < NUM_IMAGES; i++)
				{
					clearBuffer();

					// step 1: get image
					// write to device
					write(file, GetImage, GetImgageLength);
					
					// write cmd to buffer for checking
					for(i = 0; i < GetImgageLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							read(file, fingerprintBuffer[i], sizeof(char));
					}
					
					clearBuffer();

					// step 2: generate char
					GenChar[10] = maxEnrolled+1;
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					write(file, GenChar, GenCharLength);
					
					// write cmd to buffer for checking
					for(i = 0; i < GenCharLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							read(file, fingerprintBuffer[i], sizeof(char));
					}
				}
				
				// register fingerprint
				clearBuffer();
				
				write(file, RegModel, RegModelLength);
				
				// write cmd to buffer for checking
				for(i = 0; i < RegModelLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						read(file, fingerprintBuffer[i], sizeof(char));
				}
	
<<<<<<< HEAD
				syslog(LOG_INFO, "Unique fingerprints temporarily stored"); 
=======
				printf("Unique fingerprints temporarily stored\n"); 		// do we want to do this? TODO
>>>>>>> c80ab9fa69fe285b326e94c1c170c25c91262ed7
				printf("Enter user name\n");
				scanf("%c", &name);
				
				// store fingerprint
				StoreChar[StoreCharLength-3] = maxEnrolled;
				StoreChar[StoreCharLength-1] = checksum(StoreChar, StoreCharLength);
				write(file, StoreChar, StoreCharLength);
				
				// write cmd to buffer for checking
				for(i = 0; i < StoreCharLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						read(file, fingerprintBuffer[i], sizeof(char));
				}
				
				syslog(LOG_INFO, "Unique fingerprints added");
				
				state = 0;
				break;
			}
						
			// remove fingerprint			will need to add another button for this TODO
			case 2:
			{
				syslog(LOG_INFO, "Type user name to delete data");
				scanf("%c", &name);
				// TODO!! check file for name and fingerprint number then remove
			}
		}
	}
	close(file);
	iolib_free();

	return 0;
}
