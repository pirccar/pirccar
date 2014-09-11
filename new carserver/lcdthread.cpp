#include "lcdthread.h"
#include <cstdio>

LcdThread::LcdThread(void)
: Thread()
{
}

void LcdThread::mainLoop(void)
{
}

void LcdThread::init(void)
{
	lcdscreen = new LcdScreen();
	
	lcdscreen->init();
	lcdscreen->showLogo();
}

void LcdThread::cleanup(void)
{
//	printf("hepp!\n");
	lcdscreen->clear();
	delete lcdscreen;
	lcdscreen = 0;
}
