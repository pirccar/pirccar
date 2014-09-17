#include "adcthread.h"


AdcThread::AdcThread(void)
: Thread()
{
}

AdcThread::AdcThread(LcdThread* lcdThread, SendThread* sendThread)
: Thread()
{
	this->lcdThread = lcdThread;
	this->sendThread = sendThread;
}

void AdcThread::mainLoop(void)
{
	channel0 = readADCValue(0);
	channel1 = readADCValue(1);
	channel2 = readADCValue(2);
	channel3 = readADCValue(3);
	
	channel0S = getADCString(0);
	channel1S = getADCString(1);
	channel2S = getADCString(2);
	channel3S = getADCString(3);
	
	lcdThread->setChannel(0, channel0);
	lcdThread->setChannel(2, channel2);
	
	sendThread->setChannel(0, channel0S);
	sendThread->setChannel(1, channel1S);
	sendThread->setChannel(2, channel2S);
	sendThread->setChannel(3, channel3S);
	
	sleep(3);
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
		return channel0;
	else if(channel == 1)
		return channel1;
	else if(channel == 2)
		return channel2;
	else if(channel == 3)
		return channel3;
	
	return 0.0;
}

double AdcThread::readADCValue(int channel)
{
	if (channel > 3 ||channel < 0)
	{
		return 0.0;
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

std::string AdcThread::getADCString(int channel)
{
	double val = readADCValue(channel);
	std::ostringstream os;
	os << channel; //channel
	os << val; //value
	std::string marker = "V";
	std::string str = marker + os.str();
	
	return str;
}
