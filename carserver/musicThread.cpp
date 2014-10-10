#include "musicThread.h"


MusicThread::MusicThread(void)
: Thread()
{
	
}

void MusicThread::mainLoop(void)
{
	if(playTrigger)
	{
		playSong();
		playTrigger = false;
	}
	
	struct timeval start, end;
	long mtime, seconds, useconds;
	bcm2835_gpio_write(TRIGGER, LOW);
	sleep(2);
	bcm2835_gpio_write(TRIGGER, HIGH);
	bcm2835_delayMicroseconds(10);
	bcm2835_gpio_write(TRIGGER, LOW);
	
	while(bcm2835_gpio_lev(ECH) == LOW);
	
	gettimeofday(&start, NULL);
	while(bcm2835_gpio_lev(ECH) == HIGH);
	gettimeofday(&end, NULL);
	
	seconds = end.tv_sec - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	
	printf("Seconds: %ld \n", seconds);
	printf("Micros: %ld \n", useconds);
	
	mtime = ((seconds) * 1000 + useconds) +0.5;
	
	double totSec = seconds + useconds / 1000000.0;
	
	printf("totsec: %d \n", totSec);
	float distance = (totSec * 17150);
	
	printf("Distance: %f cm\n", distance);
}

void MusicThread::init(void)
{
	bcm2835_gpio_fsel(SPEAKER, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(TRIGGER, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(ECH, BCM2835_GPIO_FSEL_INPT);
	//bcm2835_gpio_set_pud(ECH, BCM2835_GPIO_PUD_UP);
	playTrigger = false;
}

void MusicThread::cleanup(void)
{
	playTrigger = false;
}

void MusicThread::play()
{
	playTrigger = true;
}
	
void MusicThread::playTone(int tone, int duration)
{
	bcm2835_gpio_fsel(SPEAKER, BCM2835_GPIO_FSEL_OUTP);
	long elapsed_time = 0;
	int rest_count = 1;
	if (tone > 0) 
	{ // if this isn't a Rest beat, while the tone has
		//  played less long than 'duration', pulse speaker HIGH and LOW
		while (elapsed_time < duration) 
		{
			bcm2835_gpio_write(SPEAKER,HIGH);
			usleep(tone / 2);

			// DOWN
			bcm2835_gpio_write(SPEAKER, LOW);
			usleep(tone / 2);

			// Keep track of how long we pulsed
			elapsed_time += (tone);
		}
	}
	else 
	{
		for (int j = 0; j < rest_count; j++) 
		{
			usleep(duration);  
		}                                
	}                    
}

void MusicThread::playSong()
{

	//uint8_t speakerChannel = 8;
	int  c   =  3830;    // 261 Hz
	int  d   =  3400;    // 294 Hz
	int  e   =  3038;    // 329 Hz
	int  f   =  2864;    // 349 Hz
	int  g   =  2550;    // 392 Hz
	int  a   =  2272;    // 440 Hz
	int  b   =  2028;    // 493 Hz
	int  C   =  1912;    // 523 Hz 
	int  R   =  0;
	
	int glassN[] = {  f,  a,  f,  c,  R,  f,  a,  f,  c,  R,  c,  a,  a,  g,  g,  f};
	int glassB[] = { 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 50, 25, 50, 25, 50};
	int whenTheSaintsN[] = { c,  e,  f,  g,  R,  c,  e,  f,  g,  R,  c,  e,  f,  g,  e,  c,  e,  d,  R,  e,  d,  c,  R,  c,  e, g, g, g,  f,  R,  c,  e,  f,  g,  e,  c,  d,  c};
	int whenTheSaintsB[] = {25, 25, 25, 25, 50, 25, 25, 25, 25, 50, 25, 25, 25, 25, 50, 50, 50, 50, 50, 25, 25, 25, 25, 25, 25, 14, 14, 14, 25, 50, 25, 25, 25, 25, 25, 50, 50, 50};
	int songLength = sizeof(glassN) / sizeof(*glassN);
	
	for(int i = 0; i < songLength; i++)
	{
		int duration = glassB[i] * 10000;
		printf("%d : %d, D: %d \n", i, glassB[i], duration);
		playTone(glassN[i], duration);
		bcm2835_delay(10);
	}
}
