#pragma once
#include <math.h>
#include <string>
#include <string.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sstream>
#include "thread.h"
#include "lcdscreen.h"

class LcdThread : public Thread
{

public:
	LcdThread(void);
	void setChannel(int channel, double value);
	void setConnected(bool connected);
	void setGotConfig(bool gotConfig);
	
private:
	LcdScreen* lcdscreen;
	bool showIp;
	bool connected;
	bool gotConfig;
	double channel0;
	double channel2;
	std::string localAdresses[10];
	int localAdressCount;
	

	virtual void mainLoop(void);
	virtual void init(void);
	virtual void cleanup(void);
	void getLocalAdresses();
	
};