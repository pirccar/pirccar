#include "speedMeter.h"
#include <wiringPi.h>

volatile int SpeedMeter::nInterrupts = 0;

SpeedMeter::SpeedMeter(void)
: Thread()
{
	isAlive = true;
	lastHigh = false;
}

float SpeedMeter::getSpeed()
{
	return speed;
}

bool SpeedMeter::getAlive()
{
	return isAlive;
}

float SpeedMeter::getDT()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	long seconds, useconds;
	
	seconds = now.tv_sec - previousTime.tv_sec;
	useconds = now.tv_usec - previousTime.tv_usec;
	
	previousTime = now;
	return seconds + useconds / 1000000.0;
}

void SpeedMeter::interrupt()
{
	nInterrupts++;
}

void SpeedMeter::mainLoop(void)
{
	/*
	bool high = bcm2835_gpio_lev(SPEEDIN) == HIGH ? true : false; //get current value of pin
	if(high && !lastHigh) //this will show a rising edge
	{
		printf("HIGH \n");
		float dt = getDT();
		speed = 2.85f / dt;
		printf("DT: %f speed: %f\n", dt, speed);
	}
	else if(!high && lastHigh) //this will show a falling edge
	{
		printf("low\n");
	}
	
	lastHigh = high;
	*/
	counter += getDT(); //add DT to counter
	float sampleTime = 0.5f; //set the sampleTime
	if(counter >= sampleTime)
	{
		speed = (1.425f * nInterrupts) / sampleTime; //cm\s
		printf("NI: %d speed: %f\n", nInterrupts, speed);
		nInterrupts = 0; //reset interrupts counter
		counter -= sampleTime; //and remove smapleTime from time counter
	}
	usleep(5000); //sleep for a while
}

void SpeedMeter::init(void)
{
	//bcm2835_gpio_fsel(SPEEDIN, BCM2835_GPIO_FSEL_INPT);
	
	wiringPiSetup() ; //init
	pinMode(SPEEDINW, INPUT); //input
	wiringPiISR(SPEEDINW, INT_EDGE_FALLING, &SpeedMeter::interrupt); //interrupt setup
	
	isAlive = true;
	getDT();
	nInterrupts = 0;
}

void SpeedMeter::cleanup(void)
{
	//bcm2835_gpio_clr_ren(SPEEDIN);
	isAlive = false;
}
