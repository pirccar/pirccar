#include "servocontrol.h"
#include "pca9685.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
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
	infile.open ("servo_values.txt");
	
	printf("OK!\n");
	printf("Reading");
	while(!infile.eof())				// Read file until EOF.
	{
		printf(".");
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
	char temp;
	
	PCA9685_init();
	printf("Initializing PWM-module\n");
	PCA9685_reset();
	printf("Initializing PWM-module\n");
	setPWMFreq(60);
	printf("Initializing PWM-module\n");
	temp = loadDefaultValues();
	
	if(temp == 0)
	{
		printf("Default PWM-values loaded successfully!");
	}
	else
	{
		printf("Damn! something went wrong when opening the file...");
	}
}

char ServoControl::loadDefaultValues(void)
{
	char result = 1;
	
	
	return result;
}