#include "servocontrol.h"
#include "pca9685.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "ini.h"

using namespace std;

ServoControl::ServoControl()
{
}

//Public
void ServoControl::start(void)
{
	init();
}

void ServoControl::stop(void)
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

char ServoControl::saveDefaultValues(char*)
{
	printf("Opening file for reading...");
	string values;
	ifstream infile;
	infile.open ("carserver.ini");
	
	printf("OK!\n");
	printf("Reading");
	while(!infile.eof())				// Read file until EOF.
	{
		printf("");
		getline(infile,values); // Saves the line from file to string
		cout<<values; // And prints it out.
	}
	printf("\n");
	infile.close();
	
	char result = 0;
	
	return result;
}

//Private
void ServoControl::init(void)
{
	printf("Initializing PWM-module\n");
	PCA9685_init();
	PCA9685_reset();
	setPWMFreq(60);
	
	if(loadDefaultValues() == TRUE) {
		printf("Default PWM-values loaded successfully!");
	}
	else {
		printf("Damn! something went wrong... I quit..");
		//Shut down program here...
	}
}

char ServoControl::loadDefaultValues(void)
{
	char result = FALSE;
	int servoposition;
	
    INIReader reader("carserver.ini");		//Open file

    if (reader.ParseError() < 0) {			//File opened OK?
        std::cout << "Can't load 'carserver.ini'\n";
		return 1;
    }
	else {
		std::cout << "Config loaded from 'carserver.ini': app name=" << reader.Get("info", "appname", "UNKNOWN") << ",\nversion=" << reader.GetInteger("info", "version", -1) << , "\n\n";
		for(i=1;i<17;i++) {
			servoposition = reader.GetInteger("servo"+itoa(i), "default_value", -1);		//Read out servo value from ini-file....
			setServoPosition(i,servoposition);												//...and write it to PWM module
			printf("Servo %d set to %d\n", i, servoposition);
		}
        result = TRUE;
	}
	
 return result;
}