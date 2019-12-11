// reference AD-013%20Command%20Operation%20Guide_v1.2.pdf
// reference Fingerprint_sensor_module_User_Manual_v1.0_2019-1-22.pdf

#include "fingerprintModule.h"

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

int minEnrolled, maxEnrolled, totalPrints;

int main()
{ 
	// fingerprint buffer
	int state = 0, i = 0, ret = 0, image = 0, alarm = 0, alarmFile, usersFile, deleteThisPrint;
	char userBuffer[3000], delName[30], testing[1];
	
	FILE* file;

	minEnrolled = 0, totalPrints = 0;

	file = fopen("/dev/ttyS0", "r+"); 
	if(file == -1)
	{
		printf("failed to open serial port\n");
		syslog(LOG_ERR, "uart port failed to open");
		return -1;
	}

	alarmFile = open("/bin/buzzerAlarm", O_WRONLY);
	if(alarmFile == -1)
	{
		printf("buzzer file not opened\n");
		syslog(LOG_ERR, "buzzer file was not found or opened");
		return -1;
	}
	
	usersFile = open("/bin/fingerprintUsers", O_RDWR | O_CREAT | O_APPEND, 0777);
	if(usersFile == -1)
	{
		printf("user file not opened\n");
		syslog(LOG_ERR, "users file was not found or opened");
		return -1;
	}
	
	// wiring init failed
	if(wiringPiSetup() == -1)
	{
		printf("wiringPi.h failed!\n");
		return 1; 
	}
	
	while(1) // may want to change to stop the fingerprint module TODO
	{
		switch(state)
		{
			// user selection
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

				clearBuffer();

				fwrite (GetImage, GetImageLength, 1, file);

				for (i = 0; i < GetImageLength; i++) 
				{
					fingerprintBuffer[i] = getc(file);
					if (i == 0 && fingerprintBuffer[i] != start) 
					{
						fingerprintBuffer[i] = getc(file);
						printf("getImg resp: %x\n", fingerprintBuffer[i]);
					}
				}
				
				break;
			}
/*
			if (fp1_rd[9]== 0)

				IOWR(LEDS_BASE,0,0x3ff);

				for(int i = 0; i < 50; i++) { //clear read buffers
					fp1_rd[i] = 0;
					fp2_rd[i] = 0;
				}

				len = sizeof(genChar)/sizeof(genChar[0]);
				genChar[len-1] = checksum(genChar,len);
				genChar[10] = 1;
				genChar[len-1] = checksum(genChar,len);
				fwrite (genChar,len, 1, fp);

				for (int i = 0; i < 12; i++) {
					fp1_rd[i] = getc(fp);
					if (i == 0 && fp1_rd[i] != 0xef) {
						fp1_rd[i] = getc(fp);
					}
				}
				for(int i = 0; i < 50; i++) { //clear read buffers
					fp1_rd[i] = 0;
					fp2_rd[i] = 0;
				}

				len = sizeof(search)/sizeof(search[0]);
				search[len-3] = nEnrolled;
				search[len-1] = checksum(search,len);

				fwrite (search,len, 1, fp);

				for (int i = 0; i < 16; i++) {
					fp1_rd[i] = getc(fp);
					if (i == 0 && fp1_rd[i] != 0xef) {
						fp1_rd[i] = getc(fp);
					}
				}

				if (fp1_rd[13] >= 70) {
					IOWR(PWM_CONTROL_BASE,0,5);
					IOWR(HEX_5_BASE, 0, 0x8C);	// P
					IOWR(HEX_4_BASE, 0, 0x88);	// A
					IOWR(HEX_3_BASE, 0, 0x92);	// S
					IOWR(HEX_2_BASE, 0, 0x92);	// S
					IOWR(HEX_1_BASE, 0, 0xFF);	// ''
					IOWR(HEX_0_BASE, 0, 0xFF);	// ''
					usleep(2000000);
					IOWR(HEX_5_BASE, 0, 0xFF);	// F
					IOWR(HEX_4_BASE, 0, 0xFF);	// A
					IOWR(HEX_3_BASE, 0, 0xFF);	// I
					IOWR(HEX_2_BASE, 0, 0xFF);	// L
					IOWR(HEX_1_BASE, 0, 0xFF);	// ''
					IOWR(HEX_0_BASE, 0, 0xFF);	// ''
					IOWR(PWM_CONTROL_BASE,0,10);

				}
				else {		//failed
					IOWR(PWM_CONTROL_BASE,0,10);
					IOWR(HEX_5_BASE, 0, 0x8E);	// F
					IOWR(HEX_4_BASE, 0, 0x88);	// A
					IOWR(HEX_3_BASE, 0, 0xCF);	// I
					IOWR(HEX_2_BASE, 0, 0xC7);	// L
					IOWR(HEX_1_BASE, 0, 0xFF);	// ''
					IOWR(HEX_0_BASE, 0, 0xFF);	// ''
					usleep(2000000);
					IOWR(HEX_5_BASE, 0, 0xFF);	// F
					IOWR(HEX_4_BASE, 0, 0xFF);	// A
					IOWR(HEX_3_BASE, 0, 0xFF);	// I
					IOWR(HEX_2_BASE, 0, 0xFF);	// L
					IOWR(HEX_1_BASE, 0, 0xFF);	// ''
					IOWR(HEX_0_BASE, 0, 0xFF);	// ''
				}
				state = 0;
				break;
			}
			else {
				state = 0;
				break;
			}
		}
		//REGISTER PRINT
		case 0x02:
		{
			len = sizeof(genChar)/sizeof(genChar[0]);
			IOWR(LEDS_BASE,0, 0x2aa);
			for (int i = 0; i < 15; i++){
				for(int i = 0; i < 50; i++) { //clear read buffers
					fp1_rd[i] = 0;
					fp2_rd[i] = 0;
				}

				fp1_rd[9] = 0x1;
				while (fp1_rd[9] != 0) {
					fwrite (getImage, 12, 1, fp);
					for (int i = 0; i < 12; i++) {
						fp1_rd[i] = getc(fp);
						if (i == 0 && fp1_rd[i] != 0xef) {
							fp1_rd[i] = getc(fp);
						}
					}
				}

				genChar[10] = i+1;
				genChar[len-1] = checksum(genChar,len);
				fwrite (genChar,len, 1, fp);

				for (int i = 0; i < 12; i++) {
					fp1_rd[i] = getc(fp);
					if (i == 0 && fp1_rd[i] != 0xef) {
						fp1_rd[i] = getc(fp);
					}
				}

			}
			uint8_t len = sizeof(regModel)/sizeof(regModel[0]);
			//	regModel[len-1] = checksum(regModel,len);
			for(int i = 0; i < 50; i++) { //clear read buffers
				fp1_rd[i] = 0;
				fp2_rd[i] = 0;
			}

			fwrite(regModel,len,1,fp);

			for (int i = 0; i < 12; i++) {
				fp1_rd[i] = getc(fp);
				if (i == 0 && fp1_rd[i] != 0xef) {
					fp1_rd[i] = getc(fp);
				}
			}

			len = sizeof(storeChar)/sizeof(storeChar[0]);
			storeChar[len-3] = nEnrolled;
			storeChar[len-1] = checksum(storeChar,len);


			fwrite(storeChar,len,1,fp);

			for (int i = 0; i < 12; i++) {
				fp1_rd[i] = getc(fp);
				if (i == 0 && fp1_rd[i] != 0xef) {
					fp1_rd[i] = getc(fp);
				}
			}

			nEnrolled++;
			state = 0;

		}
*/
	}

}

}
