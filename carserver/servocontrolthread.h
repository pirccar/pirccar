#pragma once
#include "thread.h"
#include "servocontrol.h"

class ServoControlThread : public Thread
{
public:
	ServoControlThread(void);
	
private:
	ServoControl* servocontrol;

	virtual void mainLoop(void);
	virtual void init(void);
	virtual void cleanup(void);
};