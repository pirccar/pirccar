#pragma once
#include <string>
#include <stdint.h>


class ServoControl
{
public:
	ServoControl(void);
	
	void start(void);											//Starts PWM-module, sets frequency etc.
	void stop(void);											//Stops PWM-module, sets PWM=0, frequency=0
	void setServoPosition(char servo = 0, int position = 0);	//Writes new position value to PWM-module
	int readServoPosition(char servo = 0);						//Returns value from PWM-module register
	char saveDefaultValues(char*);								//Saves default values to file, returns 0 if successful
	
private:
	void init(void);											//Internal function, initializes PWM-module 
	char loadDefaultValues(void);								//Loads default values from file, returns 0 if successful
	static int ServoControl::handler(void* user, const char* section, const char* name, const char* value)
};