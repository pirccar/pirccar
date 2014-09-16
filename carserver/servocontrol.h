#pragma once
#include <string>
#include <stdint.h>


class ServoControl
{
public:
	ServoControl(void);
	
	void init(void);
	void setServoPosition(char, int);
	void readServoPosition(char);
	
private:
	void setPwmFrequency(int)
};