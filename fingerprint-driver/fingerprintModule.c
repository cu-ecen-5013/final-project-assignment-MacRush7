// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf

#include <wiringPi.h>
#include <stdio.h>

#define LedPin    0		// TODO
#define ButtonPin 1		// TODO

#define BUF_SIZE		100
#define NUM_IMAGES	6

int checkButton()
{
	int buttonFlag = 0;
	
	if(digitalRead(ButtonPin) == 0)
	{ 
		//indicate that button has pressed down
		buttonFlag = 1;
		digitalWrite(LedPin, LOW);   //led on
	}
	else
		buttonFlag = 0;
	
	return buttonFlag;
}

int checksum(uint8_t cmd[], int length) 
{
	int checksum = 0;
	// check sum is last two bytes... reference 2
	for (int i = 6; i < length-2; i++)
		checksum += cmd[i];
	return sum;
}

void clearBuffer()
{
	// clear buffer
	for(i = 0; i < BUF_SIZE; i++)
	{
		fingerprintBuffer[i] = 0;
	}
}

fingerprint()
{
	// fingerprint buffer
	uint32_t fingerprintBuffer[BUF_SIZE];
	uint32_t length = 0;
	int state = 0, button = 0, i = 0, minEnrolled = 1, max Enrolled = 1;
	int start = 0xEF;

	// wiring init failed
	if(wiringPiSetup() == -1)
	{
		printf("wiringPi.h failed!");
		return 1; 
	}
	
	// set pins
	pinMode(LedPin, OUTPUT); 
	pinMode(ButtonPin, INPUT);
	pullUpDnControl(ButtonPin, PUD_UP); 
	digitalWrite(LedPin, HIGH);

	// fingerprint commands... see reference
	uint32_t GetImage[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
	uint32_t GenChar[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x00};
	uint32_t RegModel[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09};
	uint32_t StoreChar[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, 0x01, 0x00, 0x01, 0x00, 0x0F};
	uint32_t Search[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint32_t DeleteChar[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, 0x00, 0x05, 0x00, 0x0A, 0x00, 0x23};
	uint32_t Empty[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0D, 0x00, 0x11};
	
	// cmd lengths
	GetImgageLength = sizeof(GetImage)/sizeof(GetImage[0]);
	GenCharLength = sizeof(GenCharLength)/sizeof(GenCharLength[0]);
	RegModelLength = sizeof(RegModelLength)/sizeof(RegModelLength[0]);
	StoreCharLength = sizeof(StoreCharLength)/sizeof(StoreCharLength);
	SearchLength = sizeof(SearchLength)/sizeof(SearchLength[0]);
	DeleteCharLength = sizeof(DeleteCharLength)/sizeof(DeleteCharLength);
	EmptyLength = sizeof(EmptyLength)/sizeof(EmptyLength[0]);
	
	// device file
	FILE* file;
	file = fopen("/dev/fp1_control", "r+");
	if(!file)
		return -1;
	
	while(1) // may want to change to stop the fingerprint module TODO
	{
		switch(state)
		{
			// check fingerprint
			case 0:
			{
				// check button for state
				button = checkButton();
				if(button == 1)
				{
					// add fingerprint
					state = 1;
					break;
				}
			
				clearBuffer();
			
				// write to device
				fwrite(GetImage, GetImgageLength, 1, file);
				
				// write cmd to buffer for checking
				for(i = 0; i < GetImgageLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = getc(file);
					// copy remaining
					fingerprintBuffer[i] = getc(file);					
				}
				
				// 10th position will be 0 if success (finger sensed)
				// generate the fingerprint then search
				if(fingerprintBuffer[9] == 0)
				{
					clearBuffer();
					
					// generate characters to index 1
					GenChar[10] = minEnrolled;
					// generate check sum for last two bytes
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					fwrite(GenChar, GenCharLength, 1, file);
					
					// write cmd to buffer for checking
					for(i = 0; i < GenCharLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							fingerprintBuffer[i] = getc(file);
						// copy remaining
						fingerprintBuffer[i] = getc(file);					
					}
					
					clearBuffer();
					
					// last four bytes in search are min numEnrolled, max numEnrolled, 2 bytes for check sum
					Search[SearchLength-3] = minEnrolled;
					Search[SearchLength-2] = maxEnrolled;
					Search[SearchLength-1] = checksum(Search, SearchLength);
					fwrite(Search, SearchLength, 1 file);
					
					// write cmd to buffer for checking
					for(i = 0; i < SearchLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							fingerprintBuffer[i] = getc(file);
						// copy remaining
						fingerprintBuffer[i] = getc(file);					
					}
				}
				else
					break;
			}
			
			// add fingerprint
			case 1:
			{				
				// repeat 6 times for adding fingerprint image
				for(i = 0, i < NUM_IMAGES; i++)
				{
					clearBuffer();

					// step 1: get image
					// write to device
					fwrite(GetImage, GetImgageLength, 1, file);
					
					// write cmd to buffer for checking
					for(i = 0; i < GetImgageLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							fingerprintBuffer[i] = getc(file);
						// copy remaining
						fingerprintBuffer[i] = getc(file);					
					}
					
					// step 2: generate char
					GenChar[10] = maxEnrolled+1;
					GenChar[GenCharLength-1] = checksum(GenChar, GenCharLength);
					fwrite(GenChar, GenCharLength, 1, file);
					
					// write cmd to buffer for checking
					for(i = 0; i < GenCharLength; i++)
					{
						// wait for the start of the cmd
						if((i == 0) && (fingerprintBuffer[i] != start))
							fingerprintBuffer[i] = getc(file);
						// copy remaining
						fingerprintBuffer[i] = getc(file);					
					}
				}
				
				// register fingerprint
				clearBuffer();
				
				fwrite(RegModel, RegModelLength, 1, file);
				
				// write cmd to buffer for checking
				for(i = 0; i < RegModelLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = getc(file);
					// copy remaining
					fingerprintBuffer[i] = getc(file);					
				}
				
				// store fingerprint
				StoreChar[StoreCharLength-3] = maxEnrolled;
				StoreChar[StoreCharLength-1] = checksum(StoreChar, StoreCharLength);
				fwrite(StoreChar, StoreCharLength, 1, file);
				
				// write cmd to buffer for checking
				for(i = 0; i < StoreCharLength; i++)
				{
					// wait for the start of the cmd
					if((i == 0) && (fingerprintBuffer[i] != start))
						fingerprintBuffer[i] = getc(file);
					// copy remaining
					fingerprintBuffer[i] = getc(file);					
				}
				
				state = 0;
				break;
			}
		}
}
