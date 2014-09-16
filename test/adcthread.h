#pragma once
#include "thread.h"
#include "bcm2835.h"

class AdcThread : public Thread
{
public:
	AdcThread(void);
	double getChannel(int channel);
	
private:
	double chann0Double;
	double chann1Double;
	double chann2Double;
	double chann3Double;
	
	virtual void mainLoop(void);
	virtual void init(void);
	virtual void cleanup(void);
};