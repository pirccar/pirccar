#pragma once
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
//#include <bcm2835.h>

#include "thread.h"

#define SPEEDIN 	RPI_BPLUS_GPIO_J8_36
#define SPEEDINW	27

class SpeedMeter : public Thread
{
public:
	SpeedMeter(void);
	
	bool getAlive();
	float getSpeed();
private:
	float speed;
	struct timeval previousTime;
	bool isAlive;
	bool lastHigh;
	static volatile int nInterrupts;
	float counter;
	
	static void interrupt(void);
	float getDT();
	
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};