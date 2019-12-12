#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>

int main()
{
	int ret = 0, SPIfile;
	char data[10];
	
	if(wiringPiSetup() == -1)
	{
		syslog(LOG_ERR, "wiringPi failed");
		return -1;
	}
	
	// channel 0 or 1, speed 500,000 through 32,000,000 Hz
	SPIfile = wiringPiSPISetup(0, 1000000);
	if(SPIfile == -1)
	{
		syslog(LOG_ERR, "cannot open spi");
		return -1;
	}
	
	// simultaneous write/read over SPI
	// channel 0 or 1, unsigned char*data, int len
	ret = wiringPiSPIDataRW(0, data, sizeof(data));
	if(ret == -1)
		syslog(LOG_ERR, "SPI send/receive failure");
		
	close(SPIfile);
	
	return 0;
}
