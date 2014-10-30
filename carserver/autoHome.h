#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <vector>
#include <pthread.h>
#include "pca9685.h"

struct AutoHomeData
{
	int* servos;
	float distance;
};

class AutoHome
{
public:
	AutoHome();
	bool goingHome();
	void setSpeed(float speed);
	void goHome();
	void update();
	void setChannels(int steeringFChannel, int steeringBChannel, int throttleChannel, int gearChannel);
	int getServoValue(int channel);
	void setServoValue(int channel, int value);
	int getCurrentIndex();
	AutoHomeData getCheckpoint(int index);
	void reset();
	
private:
	void saveState();
	void internalGoHome();
	float getDT();
	void calculateSpeed(float speed);
	void setServos();
	static void* threadMain(void* ptr);
	void homeLoop();
	
	std::vector<AutoHomeData> goHomeData;
	bool going;
	int goHomeIndex;
	int prevGearValue;
	bool stop;
	int* lastChannelIndex;
	int* servos;
	
	float speed;
	int throttleChannel;
	int steeringFChannel;
	int steeringBChannel;
	int gearChannel;
	struct timeval previousTime;
	
	pthread_t thread;
};