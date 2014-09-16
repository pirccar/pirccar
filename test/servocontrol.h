#pragma once
#include <string>
#include <stdint.h>


class ServoControl
{
public:
	ServoControl(void);
	
	void start(void);
	void stop(void);
	void setServoPosition(char servo = 0, int position = 0);
	int readServoPosition(char servo = 0);
	
private:
	void init(void);
	void setPwmFrequency(int);
	void loadDefaultValues(void);
};