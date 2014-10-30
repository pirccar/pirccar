#include "autoHome.h"

AutoHome::AutoHome()
{
	this->speed = 0;
	this->going = false;
	this->goHomeIndex = 0;
	this->stop = false;
	this->lastChannelIndex = new int[3]; //channel 0 = steering front, channel 1 = steering back, channel 2 = gear
	this->servos = new int[4];//channel 0 = steering front, channel 1 = steering back, channel 2 = gear, channel 4 = throttle
	this->servos[0] = 320;
	this->servos[1] = 320;
	this->servos[2] = 130;
	this->servos[3] = 180;
}

bool AutoHome::goingHome()
{
	return going;
}

void AutoHome::setSpeed(float speed)
{
	this->speed = speed;
}

void AutoHome::goHome()
{
	if(!going)
	{
		pthread_create(&thread, NULL, threadMain, this);
	}
	going = true;
	printf("Going home \n");
}

void AutoHome::update()
{
	if(!going)
	{
		saveState();
	}
	else
	{
		//internalGoHome();
	}
}

void AutoHome::setChannels(int steeringFChannel, int steeringBChannel, int throttleChannel, int gearChannel)
{
	this->steeringFChannel = steeringFChannel;
	this->steeringBChannel = steeringBChannel;
	this->throttleChannel = throttleChannel;
	this->gearChannel = gearChannel;
}

int AutoHome::getServoValue(int channel)
{
	if(channel < 0 || channel > 3)
		return servos[channel];
}

void AutoHome::setServoValue(int channel, int value)
{
	if(channel > 2 || channel < 0)
		return;
	if(value < 0 || value > 4096)
		return;
	servos[channel] = value;
}

int AutoHome::getCurrentIndex()
{
	return goHomeIndex;
}

AutoHomeData AutoHome::getCheckpoint(int index)
{
	return goHomeData[index];
}

void AutoHome::reset()
{
	goHomeData.clear();
	goHomeIndex = -1;
	this->servos[0] = 320;
	this->servos[1] = 320;
	this->servos[2] = 130;
	this->servos[3] = 180;
	this->speed = 0;
}

//Privates
void AutoHome::saveState()
{
	float dt = getDT();

	if(goHomeIndex == -1)
	{
		AutoHomeData current;
		current.servos = new int[3];
		current.servos[0] =  servos[0];
		current.servos[1] =  servos[1];
		current.servos[2] =  servos[2];
		current.distance = 0;
		goHomeData.push_back(current);
		goHomeIndex++;
		lastChannelIndex[0] = 0;
		lastChannelIndex[1] = 0;
		lastChannelIndex[2] = 0;
	}
	else
	{
		bool diff = false;
		AutoHomeData current;
		current.servos = new int[3];
		current.servos[0] =  -1;
		current.servos[1] =  -1;
		current.servos[2] =  -1;
		if(goHomeData[lastChannelIndex[0]].servos[0] != servos[0]) //steering front
		{
			diff = true;
			current.servos[0] = servos[0];
			lastChannelIndex[0] = goHomeIndex;
		}
		
		if(goHomeData[lastChannelIndex[1]].servos[1] != servos[1]) //steering back
		{
			diff = true;
			current.servos[1] = servos[1];
			lastChannelIndex[1] = goHomeIndex;
		}
		
		if(goHomeData[lastChannelIndex[2]].servos[2] != servos[2]) //gear
		{
			diff = true;
			current.servos[2] = servos[2];
			lastChannelIndex[2]= goHomeIndex;
		}
		
		goHomeData[goHomeIndex].distance += speed * dt;
		
		if(speed * dt > 0.0f)
			printf("Distance: %f \n", goHomeData[goHomeIndex].distance);
			
		if(diff)
		{
			current.distance = 0;
			if(goHomeData[goHomeIndex].distance > 0.0f) 
			{
				goHomeIndex++;
			}
			else
			{
				goHomeData.pop_back();
			}
			goHomeData.push_back(current);
		}
	}
}

void AutoHome::internalGoHome()
{
	float dt = getDT();
	if(goHomeIndex == -1) //stop the car!
	{
		servos[3] = 180;
		speed = 0; //TODO REMOVE
		if(speed == 0)
		{
			going = false;
			goHomeIndex = -1;
			goHomeData.clear();
		}
		return;
	}
	
	for(int i = goHomeIndex; i >= 0; i--)
	{
		if(goHomeData[i].servos[2] != -1)
		{
			servos[2] = goHomeData[i].servos[2] == 130 ? 470 : 130;
			break;
		}
	}
	
	if(servos[2] != prevGearValue)
	{
		stop = true;
		speed = 0; //TODO REMOVE
	}
	
	if(stop)
	{
		if(speed == 0)
			stop = false;
		servos[3] = 180;
	}
	else if(goHomeData[goHomeIndex].distance > 0)
	{
		speed = 2; // TODO REMOVE
		calculateSpeed(goHomeData[goHomeIndex].distance);
		goHomeData[goHomeIndex].distance -= speed * dt;
		
		//printf("Distance: %f\n", goHomeData[goHomeIndex].distance);
	}
	else
	{
		goHomeIndex--;
		for(int i = goHomeIndex; i >= 0; i--)
		{
			if(goHomeData[i].servos[0] != -1)
			{
				servos[0] = goHomeData[i].servos[0];
				break;
			}
		}
		for(int i = goHomeIndex; i >= 0; i--)
		{
			if(goHomeData[i].servos[1] != -1)
			{
				servos[1] = goHomeData[i].servos[1];
				break;
			}
		}
		printf("At checkpoint %d \n", goHomeIndex);
	}
	prevGearValue = servos[2];
}

float AutoHome::getDT()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	long seconds, useconds;
	
	seconds = now.tv_sec - previousTime.tv_sec;
	useconds = now.tv_usec - previousTime.tv_usec;
	
	previousTime = now;
	return seconds + useconds / 1000000.0;
}

void AutoHome::calculateSpeed(float distance)
{
	if(distance <= 0)
	{
		servos[3] = 180;
		return;
	}
	float calcSpeed = speed;
	if(calcSpeed == 0)
		calcSpeed = 1;
	
	servos[3] = (distance * 4.35f) / calcSpeed;
	servos[3] = servos[3] < 1 ? 1: (servos[3] > 15 ? 15 : servos[3]);
	servos[3] += 180;
}

void AutoHome::setServos()
{

	setPWM(steeringFChannel, servos[0]);
	setPWM(steeringBChannel, servos[1]);
	setPWM(gearChannel, servos[2]);
	setPWM(throttleChannel, servos[3]);
}

void* AutoHome::threadMain(void* self)
{
	((AutoHome*)self)->homeLoop();
}

void AutoHome::homeLoop()
{
	printf("At checkpoint %d \n", goHomeIndex);
	while(going)
	{
		internalGoHome();
		setServos();
	}
	printf("Honey i'm home\n");
}