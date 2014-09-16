#include "adcthread.h"

AdcThread::AdcThread(void)
: Thread()
{
}

void AdcThread::mainLoop(void)
{
	chann0Double = readADCValue(0);
	chann1Double = readADCValue(1);
	chann2Double = readADCValue(2);
	chann3Double = readADCValue(3);
}

void AdcThread::init(void)
{
	bcm2835_init();
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
}

void AdcThread::cleanup(void)
{
	bcm2835_spi_end();
}

double AdcThread::getChannel(int channel)
{
	if(channel == 0)
		return chann0Double;
	else if(channel == 1)
		return chann1Double;
	else if(channel == 2)
		return chann2Double;
	else if(channel == 3)
		return chann3Double;
	
	return 0.0;
}

double AdcThread::readADCValue(int channel)
{
	if (channel > 3 ||channel < 0)
	{
		printf("Invalid channel\n");
		return 0;
	}
	char* input = new char[3];
	char* output = new char[3];
	input[0] = 0x06; //00000110 start + single
	
	if(channel == 0)
		input[1] = 0x00; //00000000
	else if(channel == 1)
		input[1] = 0x40; //01000000
	else if(channel == 2)
		input[1] = 0x80; //10000000
	else if(channel ==3)
		input[1] = 0xC0; //11000000
	bcm2835_spi_transfernb(input,output,3);
	unsigned int val = ((output[1] & 0xF) << 8 ) + output[2];
	double ret = (double)val/4096.0*3.3;
	
	delete[] input;
	delete[] output;
	
	return ret;
}

