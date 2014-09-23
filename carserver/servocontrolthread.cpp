#include "servocontrolthread.h"
#include <cstdio>

ServoControlThread::ServoControlThread(void)
: Thread()										//Inherits from Thread
{
}

void ServoControlThread::mainLoop(void)
{
}

void ServoControlThread::init(void)
{
	printf("Starting servo control thread\n");
	servocontrol = new ServoControl();		//Create new instance
	
	servocontrol->start();					//Starting servo control
}

void ServoControlThread::cleanup(void)
{
	printf("Shutting down servo control thread.. ");
	servocontrol->stop();
	delete servocontrol;
	servocontrol = 0;
	printf("Done!\n");
}
