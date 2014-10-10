#pragma once
#include <bcm2835.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "thread.h"

#define SPEAKER RPI_GPIO_P1_26
#define TRIGGER RPI_BPLUS_GPIO_J8_37
#define ECH 	RPI_BPLUS_GPIO_J8_38

class MusicThread : public Thread
{

public:
	MusicThread(void);
	void play();
	
private:
	void playTone(int tone, int duration);
	void playSong();
	bool playTrigger;
	
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};