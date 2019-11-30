// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf

#include "fingerprintModule.h"

int checkButton()
{
	int buttonFlag = 0;
	
	if(digitalRead(ButtonPin) == 0)
	{ 
		//indicate that button has pressed down
		printf("button pressed\n");
		// turn on LED
		digitalWrite(LedPin, HIGH);
				
		buttonFlag = 1;
	}
	else
	{
//		printf("button not pressed\n");
		buttonFlag = 0;
	}
	
	return buttonFlag;
}

int checksum(uint32_t cmd[], uint32_t length) 
{
	int checksum = 0;
	// check sum is last two bytes... reference 2
	// start at 6 to not sum header
	for (int i = 6; i < length-2; i++)
		checksum += cmd[i];
	return checksum;
}

void clearBuffer()
{
	// clear buffer
	for(int i = 0; i < BUF_SIZE-1; i++)
	{
		fingerprintBuffer[i] = 0;
	}
}

int minEnrolled, maxEnrolled;

int main()
{
	// fingerprint buffer
	int state = 0, i = 0, image = 0; //, button = 0;
	char name;
	
	minEnrolled = 0;
	
	int file = serialOpen ("/dev/ttyS0", 57600);
	if(file == -1)
		printf("failed to open serial port\n");
	
	// wiring init failed
	if(wiringPiSetup() == -1)
	{
		printf("wiringPi.h failed!\n");
		return 1; 
	}
	
	// set pins
	pinMode(LedPin, OUTPUT); 
	pinMode(ButtonPin, INPUT);
	pullUpDnControl(ButtonPin, PUD_UP); 
	digitalWrite(LedPin, LOW);
	
	while(1) // may want to change to stop the fingerprint module TODO
	{
		switch(state)
		{
			// user selection
			case 0:
			{			
				// check button for state
/*				button = checkButton();
				if(button == 1)
				{
					// add fingerprint
					state = 1;
					break;
				}
*/			
				// TODO!! add a check for fingerprint removal request
				
				printf("What would you like to do?\n");
				printf("Type 1 to enroll new print\n");
				printf("Type 2 to check database\n");
				printf("Type 3 to erase print\n");
				printf("Type 4 to erase database\n");
				printf("Type 5 to test the sensor\n");

				scanf("%d", &state);
				
				break;
			}
			
			// check database
			case 2:
			{	
				printf("place finger on scanner\n");
				sleep(3);
							
				serialFlush(file);
							
				// 10th position will be 0 if finger sensed
				for(i = 0; i < GetImageLength; i++)
					serialPutchar(file, GetImage[i]);

				// write cmd to buffer for checking
				for(i = 0; i < 11; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);	
				}
				
				if(fingerprintBuffer[9] != 0)
				{
					printf("no finger sensed\n");
					break;
				}
				else			
					printf("User detected...\n");
					
				clearBuffer();

				// write to device
				for(i = 0; i < GetImageLength; i++)
					serialPutchar(file, GetImage[i]);

				// write cmd to buffer for checking
				for(i = 0; i < GetImageLength; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);	
						
					printf("getImg resp: %x\n", fingerprintBuffer[i]);														
				}
				
				clearBuffer();
					
				// generate the fingerprint then search	
				GenChar[10] = minEnrolled;
				// generate check sum for last two bytes
				GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
				for(i = 0; i < GenCharLength; i++)
					serialPutchar(file, GenChar[i]);
								
				// write cmd to buffer for checking
				for(i = 0; i < GenCharLength; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);							
				}
				
				clearBuffer();
					
				// last four bytes in search are min numEnrolled, max numEnrolled, 2 bytes for check sum
				Search[SearchLength-5] = minEnrolled;
				Search[SearchLength-3] = maxEnrolled;
//				Search[SearchLength-1] = checksum(Search, SearchLength);
				for(i = 0; i < SearchLength; i++)
				{
					serialPutchar(file, Search[i]);
					printf("search: %x\n", Search[i]);
				}
				
				// write cmd to buffer for checking
				for(i = 0; i < SearchLength-1; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);	
						
					printf("search resp: %x\n", fingerprintBuffer[i]);
				}
							
				printf("fingerprint score: %d\n", fingerprintBuffer[13]);
				
				// check fingerprint score, if > 50 (80% match)
				if(fingerprintBuffer[13] >= 50)
				{
					printf("----------------------\n");
					printf("fingerprint passed\n");
					printf("----------------------\n");					
				}
				else
					printf("fingerprint failed\n");

				state = 0;
				break;
			}
			
			// add fingerprint
			case 1:
			{		
				printf("place finger on scanner\n");
				sleep(3);

				serialFlush(file);

				// 10th position will be 0 if finger sensed
				for(i = 0; i < GetImageLength; i++)
					serialPutchar(file, GetImage[i]);

				// write cmd to buffer for checking
				for(i = 0; i < 11; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);								
				}
				
				if(fingerprintBuffer[9] != 0)
				{
					printf("no finger sensed\n");
					break;
				}
				
				printf("adding fingerprint to database\n");
								
				// repeat 6 times for adding fingerprint image
				for(image = 0; image < NUM_IMAGES; image++)
				{
					clearBuffer();
	
					// step 1: get image
					// write to device
					for(i = 0; i < GetImageLength; i++)
						serialPutchar(file, GetImage[i]);

					if(image == 0)
					{
						// write cmd to buffer for checking
						for(i = 0; i < GetImageLength; i++)
						{
							fingerprintBuffer[i] = serialGetchar(file);
							// wait for the start of the cmd
							if((i == 0) && (fingerprintBuffer[i] != start))
								fingerprintBuffer[i] = serialGetchar(file);	
								
							printf("getImg resp: %x\n", fingerprintBuffer[i]);																		
						}	
					
						clearBuffer();
					}
					
					maxEnrolled = image+1;
					
					// step 2: generate char
					GenChar[10] = maxEnrolled;
					printf("maxEnrolled: %x\n", maxEnrolled);
					
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					printf("genchar checksum: %x\n", GenChar[GenCharLength-1]);
					for(i = 0; i < GenCharLength; i++)
						serialPutchar(file, GenChar[i]);
	
					if(image == 0)
					{
						// write cmd to buffer for checking
						for(i = 0; i < GenCharLength; i++)
						{
							fingerprintBuffer[i] = serialGetchar(file);
							// wait for the start of the cmd
							if((i == 0) && (fingerprintBuffer[i] != start))
								fingerprintBuffer[i] = serialGetchar(file);						
						}
					}
					
					printf("character #%d of %d generated\n", image, NUM_IMAGES); 
				}
				
				// register fingerprint
				clearBuffer();
				
				for(i = 0; i < RegModelLength; i++)
					serialPutchar(file, RegModel[i]);
		
				// write cmd to buffer for checking
				for(i = 0; i < RegModelLength; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);	
						
					printf("reg resp: %x\n", fingerprintBuffer[i]);				
				}
				
				printf("Unique fingerprints temporarily stored\n"); 		// do we want to do this? TODO
//				printf("Enter user name\n");
//				scanf("%c", &name);
				
				// store fingerprint
				StoreChar[StoreCharLength-3] = maxEnrolled;
				StoreChar[StoreCharLength-1] = checksum(StoreChar, StoreCharLength);

				printf("storechar checksum: %x\n", StoreChar[StoreCharLength-1]);

				for(i = 0; i < StoreCharLength; i++)
					serialPutchar(file, StoreChar[i]);
				
				// write cmd to buffer for checking
				for(i = 0; i < StoreCharLength-3; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);	
						
					printf("store resp: %x\n", fingerprintBuffer[i]);					
				}
				
				printf("Unique fingerprints added\n");
			
				state = 0;
				break;
			}
					
			// remove fingerprint	will need to add another button for this TODO
			case 3:
			{
				printf("Type user name to delete data\n");
				scanf("%c", &name);
				// TODO!! check file for name and fingerprint number then remove
				
				state = 0;
				break;
			}
			
			// remove all prints
			case 4:
			{
				for(i = 0; i < EmptyLength; i++)
					serialPutchar(file, Empty[i]);

				// write cmd to buffer for checking
				for(i = 0; i < EmptyLength; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);					
				}
				
				printf("removed all prints from database\n");
				state = 0;
				break;
			}
			
			// test sensor
			case 5:
			{
				sleep(3);
				
				for(i = 0; i < GetImageLength; i++)
					serialPutchar(file, GetImage[i]);

				// write cmd to buffer for checking
				for(i = 0; i < GetImageLength; i++)
				{
					fingerprintBuffer[i] = serialGetchar(file);
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = serialGetchar(file);								
				}
				
				for(i = 0; i < GetImageLength; i++)
					printf("fingerprintbuffer: %d\n", fingerprintBuffer[i]);
					
				printf("buffer position 9: %d\n", fingerprintBuffer[9]);
				
				state = 0;
				break;
			}	
		}
	}
	serialClose(file);
	
	return 0;
}
