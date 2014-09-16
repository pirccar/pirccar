#pragma once
#include "thread.h"
#include "lcdscreen.h"

class LcdThread : public Thread
{
public:
	LcdThread(void);
	
private:
	LcdScreen* lcdscreen;

	virtual void mainLoop(void);
	virtual void init(void);
	virtual void cleanup(void);
};