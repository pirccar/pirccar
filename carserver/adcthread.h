#pragma once
#include <unistd.h>
#include <string.h>
#include <ostream>
#include "thread.h"
#include "lcdthread.h"
#include "sendthread.h"
#include "bcm2835.h"

class AdcThread : public Thread
{
public:
	AdcThread(void);
	AdcThread(LcdThread* lcdThread, SendThread* sendThread);
	void setSendThread(SendThread* sendThread);
	double getChannel(int channel);
	
private:
	double channel0;
	double channel1;
	double channel2;
	double channel3;
	std::string channel0S;
	std::string channel1S;
	std::string channel2S;
	std::string channel3S;
	
	LcdThread* lcdThread;
	SendThread* sendThread;
	
	double readADCValue(int channel);
	std::string getADCString(int channel);
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};