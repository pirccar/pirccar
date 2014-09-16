#pragma once
#include <unistd.h>
#include "thread.h"
#include "lcdthread.h"
#include "bcm2835.h"

class AdcThread : public Thread
{
public:
	AdcThread(void);
	AdcThread(LcdThread* lcdThread);
	double getChannel(int channel);
	
private:
	double channel0;
	double channel1;
	double channel2;
	double channel3;
	LcdThread* lcdThread;
	
	double readADCValue(int channel);
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};