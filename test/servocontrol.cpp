#include "servocontrol.h"
#include "pca9685.h"
#include <cstdio>

ServoControl::ServoControl()
{
}

void ServoControl::start(void)
{
	init();
}

void ServoControl::stop(void)
{
}

void ServoControl::loadDefaultValues(void)
{
}

void ServoControl::setServoPosition(char servo, int position)
{
}

int ServoControl::readServoPosition(char servo)
{
	int servoPosition = 0;
	return servoPosition;
}

void ServoControl::init(void)
{
}
