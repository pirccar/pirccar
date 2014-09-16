#include "lcdthread.h"
#include <cstdio>

LcdThread::LcdThread(void)
: Thread()							//Inherits from Thread
{
}

void LcdThread::mainLoop(void)
{
}

void LcdThread::init(void)
{
	printf("Starting LCD thread!\n");
	lcdscreen = new LcdScreen();	//Creates new instance
	
	lcdscreen->init();				//Initialize LCD
	lcdscreen->showLogo();			//Display logo on LCD
}

void LcdThread::cleanup(void)
{
	printf("Shutting down LCD thread! ");
	lcdscreen->clear();				//Clear LCD screen
	delete lcdscreen;				//Deletes instance
	lcdscreen = 0;					//Clean sweep
	printf("Done!\n");
}
