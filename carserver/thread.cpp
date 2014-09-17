#include "thread.h"
#include <cstdio>

Thread::Thread(void) 
: isRunning(true)
{
}

void* Thread::threadMain(void* self)
{
	((Thread*)self)->loop();
}

void Thread::start(void)
{
	isRunning = true;
	pthread_create(&thread, NULL, threadMain, this);
}

void Thread::stop(void)
{
	isRunning = false;
}

void Thread::init(void)
{
}

void Thread::loop(void)
{
	init();
	while(isRunning)
	{
		mainLoop();
	}
	cleanup();
}

void Thread::cleanup(void)
{
}