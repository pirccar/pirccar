#include "lcdthread.h"
#include "servocontrolthread.h"
#include <iostream>

int main()
{
	LcdThread* temp = new LcdThread();
	
	ServoControlThread* temp2 = new ServoControlThread();
	
	temp->start();
	temp2->start();
	
	sleep(2);
	
	temp->stop();
	temp2->stop();
	
	sleep(2);
	delete temp, temp2;
	
	return 0;
}