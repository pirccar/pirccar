#include "servocontrol.h"
#include "pca9685.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
//#include "INIReader.h"
#include "inifile.h"

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
	for(int i=1;i<17;i++) {
		setPWM(i, 0);
		setPWMFreq(0);
	}
}


void ServoControl::setServoPosition(char servo, int position)
{
		setPWM(servo, position);
}

int ServoControl::readServoPosition(char servo)
{
	int servoPosition = 0;
	return servoPosition;
}

bool ServoControl::saveDefaultValues(char section, int* newvalue)
{
	// Open ini file for writing
    if (reader.Load("carserver.ini") != true) {						//File opened OK?
        std::cout << "Can't load 'carserver.ini'\n";
		return 1;
    }
	else {
		switch(section) {
			case 'S' :				//New value for servo
			{
				
			}
			break;
			case network :
			statement....
			break
			default :
			
		}
		CIniSection* pSection = reader.GetSection("info");			//Look for section "info" in ini-file
		if( pSection ) {											//Section exists ?
			std::cout << "'carserver.ini' opened for writing\n"\n"; //Prints out some stuff from the info section
		}
		else {
			std::cout << "Section 'info' does not exists\n";
		}
		
		for(int i=1;i<17;i++) {									//Loops through all the servo sections in ini-file

			string text;          								// string which will contain the result
			ostringstream convert;   							// stream used for the conversion
			convert << "servo";      							// insert the textual representation of 'Servo' in the characters in the stream
			convert << i;      									// insert the textual representation of 'i' in the characters in the stream
			text = convert.str(); 								// set 'text' to the contents of the stream

			pSection = reader.GetSection(text.c_str());			//Look for section servo x (x=1 to 16)
			if( pSection ) {									// Section exists ?
					CIniKey* pKey = pSection->GetKey("value");
						if( pKey ) {										//Key exists ?
								servoposition = std::atoi(pKey->GetValue().c_str());
								
								if(servoposition != -1) {
									std::cout << pSection->GetKeyValue("function") << " : ";
									setServoPosition(i,servoposition);												//...and write it to PWM module
									std::cout << servoposition << "\n";
								}
								else {
								}
						}
						else {
							std::cout << "No value defined for " << text.c_str() << " : Using default value : ";
							pSection = reader.GetSection("servo0");
							CIniKey* pKey = pSection->GetKey("value");
							servoposition = std::atoi(pKey->GetValue().c_str());
							std::cout << servoposition << "\n";
						}
			}
			else {
				std::cout << "No entry in ini-file for " << text.c_str() << " : Using default value\n";
			}
		}
        result = true;
	}
	
	char result = 0;
	
	return result;
}

//Private
void ServoControl::init(void)
{
	std:cout << "Initializing PWM-module\n";
	PCA9685_init();
	std::cout << "Resetting PCA9685\n";
	PCA9685_reset();
	setPWMFreq(50);
	
	if(loadDefaultValues() == true) {
		std::cout << "Default PWM-values loaded successfully!\n";
	}
	else {
		std::cout << "Damn! something went wrong... I quit..\n";
		//Shut down program here...
	}
}

bool ServoControl::loadDefaultValues(void)
{
	bool result = false;
	int servoposition;
	
    CIniFile reader;
	
	// Loads ini file for reading
    if (reader.Load("carserver.ini") != true) {						//File opened OK?
        std::cout << "Can't load 'carserver.ini'\n";
		return 1;
    }
	else {
		CIniSection* pSection = reader.GetSection("info");			//Look for section "info" in ini-file
		if( pSection ) {											//Section exists ?
			std::cout << "'carserver.ini' opened for reading\napp name=" << reader.GetKeyValue("info","appname") << "\nversion=" << reader.GetKeyValue("info","version") << "\n"; //Prints out some stuff from the info section
		}
		else {
			std::cout << "Section 'info' does not exists\n";
		}
		
		for(int i=1;i<17;i++) {									//Loops through all the servo sections in ini-file

			string text;          								// string which will contain the result
			ostringstream convert;   							// stream used for the conversion
			convert << "servo";      							// insert the textual representation of 'Servo' in the characters in the stream
			convert << i;      									// insert the textual representation of 'i' in the characters in the stream
			text = convert.str(); 								// set 'text' to the contents of the stream

			pSection = reader.GetSection(text.c_str());			//Look for section servo x (x=1 to 16)
			if( pSection ) {									// Section exists ?
					CIniKey* pKey = pSection->GetKey("value");
						if( pKey ) {										//Key exists ?
								servoposition = std::atoi(pKey->GetValue().c_str());
								
								if(servoposition != -1) {
									std::cout << pSection->GetKeyValue("function") << " : ";
									setServoPosition(i,servoposition);												//...and write it to PWM module
									std::cout << servoposition << "\n";
								}
								else {
								}
						}
						else {
							std::cout << "No value defined for " << text.c_str() << " : Using default value : ";
							pSection = reader.GetSection("servo0");
							CIniKey* pKey = pSection->GetKey("value");
							servoposition = std::atoi(pKey->GetValue().c_str());
							std::cout << servoposition << "\n";
						}
			}
			else {
				std::cout << "No entry in ini-file for " << text.c_str() << " : Using default value\n";
			}
		}
        result = true;
	}
	
	return result;
}