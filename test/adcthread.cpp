#include "AdcThread.h"

AdcThread::AdcThread(void)
: Thread()
{
}

void AdcThread::mainLoop(void)
{
}

void AdcThread::init(void)
{
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
