#pragma once
#include <pthread.h>

class Thread
{
public:
	Thread(void);
	void start(void);
	void stop(void);
	
private:
	pthread_t thread;
	bool isRunning;
	void loop(void);
	virtual void init(void);
	virtual void cleanup(void);
	virtual void mainLoop(void) = 0;
	static void* threadMain(void*);
};