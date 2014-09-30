#pragma once
#include <string>
#include <stdint.h>

using namespace std;

class ServoControl
{
public:
	ServoControl(void);
	
	void start(void);											//Starts PWM-module, sets frequency etc.
	void stop(void);											//Stops PWM-module, sets PWM=0, frequency=0
	void setServoPosition(char servo, int position);			//Writes new position value to PWM-module
	int readServoPosition(char servo);							//Returns value of servo from PWM-module
	char saveDefaultValues(char*);								//Saves default values to file, returns 0 if successful
	
private:
	bool loadDefaultValues(void);								//Loads default values from file, returns 0 if successful
	void init(void);											//Internal function, initializes PWM-module 
//	static int ServoControl::handler(void* user, const char* section, const char* name, const char* value)
};