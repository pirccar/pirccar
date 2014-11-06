#include <errno.h>
#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <vector>
#include <math.h>
#include <sys/time.h>

//#define _GNU_SOURCE
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "camerawrapper.h"
#include "lcdscreen.h"
#include "adcthread.h"
#include "lcdthread.h"
#include "sendthread.h"
#include "servocontrol.h"
#include "servocontrolthread.h"
#include "musicThread.h"
#include "autoHome.h"
#include <jpeglib.h>
#include "speedMeter.h"

#include "pca9685.h"
#include "rs232.h"

//#include <wiringPi.h>
#include "PCD8544.h"

#define MAIN_TEXTURE_WIDTH 160
#define MAIN_TEXTURE_HEIGHT 120
#define IMAGE_QUALITY 100
#define BUFFER_SIZE (MAIN_TEXTURE_WIDTH * MAIN_TEXTURE_HEIGHT)*3
#define BLOCK_SIZE 2048
#define IRCAMERA RPI_V2_GPIO_P1_07		// Camera input

bool gotConfig = false;
bool ready = false;
bool obstructed = false;
bool globalConnected = false;
char *frameIP;
MusicThread* music;

int imageWidth;
int imageHeight;
int imageQuality;

int throttleChannel = 3;
int gearChannel = -1;
int steeringFChannel = -1;
int steeringBChannel = -1;

sig_atomic_t alarm_counter;
int recTime = 3;
bool stabilized = false;
bool udpSend = true;
int sendFailCounter = 0;
bool firstRun = true;
struct timeval lastMsgTime;

double chann0Double;
double chann1Double;
double chann2Double;
double chann3Double;
std::string chann0String;
std::string chann1String;
std::string chann2String;
std::string chann3String;

AutoHome autoHome;
int lastAutoHomeIndex;


// pin setup
int _din = 4;
int _sclk = 5;
int _dc = 3;
int _rst = 0;
int _cs = 2;

//Parse incoming messages
int parseMessage(char buf[], int size){
	uint8_t servo;
	int value;
	int i = 0;
	
	if(size <= 0){
		printf("No data to parse \n");
		return 0;
	}
	//printf("Msg size: %d \n", size);
	//printf("%s \n", buf);
	if(buf[0] == 'P'){ // is ping
		//printf("Got a ping\n");
		return 1;
	}
	else if(buf[0] == 'C') //Is config
	{
		if(size < 19)
		{
			printf("Incomplete config \n");
			printf("Got: %s \n", buf);
			return 0;
		}
		
		printf("Config: %s \n", buf);
		
		//Read out all config info and save locally 
		char id[3];
		char format[5];
		char quality[4];
		//Throttle
		id[0] = buf[1];
		id[1] = buf[2];
		id[2] = '\0';
		throttleChannel = atoi(id);
		printf("Setting throttle to channel %d \n", throttleChannel);
		
		//Gear
		id[0] = buf[3];
		id[1] = buf[4];
		id[2] = '\0';
		gearChannel = atoi(id);
		printf("Setting gearChannel to channel %d \n", gearChannel);
		
		id[0] = buf[5];
		id[1] = buf[6];
		id[2] = '\0';
		steeringFChannel = atoi(id);
		printf("Setting steeringFChannel to channel %d \n", steeringFChannel);
		
		id[0] = buf[7];
		id[1] = buf[8];
		id[2] = '\0';
		steeringBChannel = atoi(id);
		printf("Setting steeringBChannel to channel %d \n", steeringBChannel);
		
		format[0] = buf[9];
		format[1] = buf[10];
		format[2] = buf[11];
		format[3] = buf[12];
		format[4] = '\0';
		
		imageWidth = atoi(format);
		
		format[0] = buf[13];
		format[1] = buf[14];
		format[2] = buf[15];
		format[3] = buf[16];
		format[4] = '\0';
		
		imageHeight = atoi(format);
		printf("Setting Image format to %dx%d \n", imageWidth, imageHeight);
		
		quality[0] = buf[17];
		quality[1] = buf[18];
		quality[2] = buf[19];
		quality[3] = '\0';
		
		imageQuality = atoi(quality);
		printf("Setting image quality to %d \n", imageQuality);
		
		if(buf[20] == '1')
		{
			udpSend = true;
			printf("Setting to UDP \n");
		}
		else
		{
			udpSend = false;
			printf("Setting to TCP \n");
		}
		
		printf("Config was received \n");
		
		autoHome.setChannels(steeringFChannel, steeringBChannel, throttleChannel, gearChannel);
		gotConfig = true;
	}
	else if(buf[0] == 'M')
	{
		music->play();
	}
	else if(buf[0] == 'G' && buf[1] == 'O' && buf[2] == 'H')
	{
		autoHome.goHome();
	}
	else if(ready){ //is command S010400 = set servo 1 to 400 off
		//printf("Got servo data \n");
		
		if(autoHome.goingHome())
			return 1;
		
		int i = 0;
		while(i + 7 <= size)
		{
			if(buf[i] == 'S') // servo
			{
				char sId[3];
				char val[5];
				bool allow = true;
				sId[0] = buf[i+1];
				sId[1] = buf[i+2];
				sId[2] = '\0';
				servo = atoi((const char*)&sId);
				
				//channel BVA
				if(servo < 0 || servo > 16)
				{
					printf("Servo index error! index: %u \n", servo);
					return 0;
				}
				
				val[0] = buf[i+3];
				val[1] = buf[i+4];
				val[2] = buf[i+5];
				val[3] = buf[i+6];
				val[4] = '\0';
				
				value = atoi((const char*)&val);
				if(servo != throttleChannel)
				{
					//Steering etc BVA
					if(value < 130)
					{
						printf("Servo %i below min (%i), setting to min \n", servo, value);
						value = 130;
					}
					else if(value > 470)
					{
						printf("Value abow max!, setting to max \n");
						value = 470;
					}
				}
				else if(obstructed) //if obstructed (IR camera detected something) don't let the car move forward
				{
					if(getPWMOff(gearChannel) < 470)
					{
						allow = false;
						printf("Nope \n");
					}
				}
				//printf("S: %i, V: %i \n", servo, value);
				if(allow)
				{
					//printf("Servo: %d Value: %d\n", servo, value);
					setPWM(servo, value);
					//printf("pwm set done\n");
					
					if(servo == throttleChannel)
						autoHome.setSpeed(value - 180);
					else if(servo == steeringFChannel)
						autoHome.setServoValue(0, value);
					else if(servo == steeringBChannel)
						autoHome.setServoValue(1, value);
					else if(servo == gearChannel)
						autoHome.setServoValue(2, value);
				}
				
			}
			i+=7;
		}
	}
		
	return 1;
}

//Bad connection handler
void alarm_handler(int signal) {
    alarm_counter++;
}

//Signal setup to dected slow connection
void setup_alarm_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    //sa.flags = 0;
    if (sigaction(SIGALRM, &sa, 0) < 0)
        printf("Can't establish signal handler");
}

//Stop all PWM channels
void stopPWM()
{
	ServoControl kalle;			//Instantiate ServoControl
	
	kalle.start();				//Run start function

//--- Old stuff
	/*	for(int i = 1; i < 17; i++)
	{
		if(i == throttleFChannel)
		{
			setPWM(throttleFChannel, 160);
			printf("Stopping (160) channel %d, ThrottleF\n", i);
		}
		else if(i == throttleBChannel)
		{
			setPWM(throttleBChannel, 160);
			printf("Stopping (160) channel %d, ThrottleB\n", i);
		}
		else if(i == gearChannel)
		{
			setPWM(gearChannel, 140);
			printf("Stopping (140) channel %d, Gear\n", i);
		}
		else
		{
			setPWM(i, 320);
			printf("Stopping (320) channel %d, Unknown\n", i);
		}
	}	
*/
//------------
}

//Main!
int main()
{
	int serverSocket;
	int clientSocket;
	
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	unsigned short serverPort;
	unsigned int clientLen;
	
	bool run = true;
	bool connected = false;
	bool senderStarted = false;
	
	//pthread_t t1;
	
	LcdThread* lcd = new LcdThread();
	SendThread* sendThread; // = new SendThread(); we no longer create the thread here, we will then pass null to adc thread
	AdcThread* adc = new AdcThread(lcd, NULL);
	ServoControlThread* servo = new ServoControlThread();
	SpeedMeter* speedMeter = new SpeedMeter();
	music = new MusicThread();
	
	if(!bcm2835_init()){
		printf("BCM error \n");
		return 1;
	}
	
	bcm2835_gpio_fsel(IRCAMERA, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(IRCAMERA,  BCM2835_GPIO_PUD_UP);
	
//	PCA9685_init();
//	printf("Resetting PCA9685\n");
//	PCA9685_reset();
//	setPWMFreq(50);
	
	//servo->start();
	//adc->start();
	//lcd->start();
	//music->start();
	
	speedMeter->start();
	float lastSpeed = 0;
	while(speedMeter->getAlive())
	{
		float speed = speedMeter->getSpeed();
		if(speed != lastSpeed)
			//printf("Speed: %f \n", speed);
		lastSpeed = speed;
	}
	
	serverPort = 8001;
	
	setup_alarm_handler();
	
	
	//IPPROTO_TCP
	if((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		printf("Socket fail \n");
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(serverPort);
	char yes='1';
	
	//Fixes bind error problem
	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		printf("socket opt error \n");
	}
	
	if(bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
		printf("bind error \n");
	
	if(listen(serverSocket, 1) < 0)
		printf("listen error \n");
	
	frameIP = new char[100];
	
	while(run)
	{
//		stopPWM();
		lastAutoHomeIndex = 0;
		autoHome.reset();
		clientLen = sizeof(clientAddr);
		sendThread = new SendThread(); //create sendthread here instead, it will then recreate itself on every new connect
		printf("Waiting for connection \n");
		if((clientSocket = accept(serverSocket, (sockaddr*) &clientAddr, &clientLen)) < 0)
			printf("Accept failed \n");
			
		printf("Got connection from: %s \n", inet_ntoa(clientAddr.sin_addr));
		frameIP = inet_ntoa(clientAddr.sin_addr);
		printf("Setting frameIP to: %s \n", frameIP);
		printf("Waiting for config file \n");
		connected = true;
		globalConnected = true;
		lcd->setConnected(globalConnected);
		sendFailCounter = 0;
		int recvCounter = 0;
		adc->setSendThread(sendThread); //here we tell the adcthread about the sendhtread
		
		while(connected)
		{
			char buffer[256];
			bzero(buffer, 256);
			int recMsgSize;
			alarm(recTime);
			
			if( (recMsgSize = recv(clientSocket, buffer, 255, 0)) < 0){
				printf("Recv failed \n"); //Indicates that it took more than recTime second to get a message from the client, bad connection
				//stopPWM();

				alarm(0);
				recTime = 1; //Increase recv timer to 3 seconds
				stabilized = false;
				sendThread->setStabilized(false);
				recvCounter++;
				//break;
			}
			else if(recMsgSize == 0)
			{
				printf("Client disconnected \n");
				connected = false;
				globalConnected = false;
				lcd->setConnected(globalConnected);
				sendThread->setHalted(true); //we should not need to halt anymore as this instance will be removed after this iteration
				senderStarted = true;
				//clientSocket = -1;
				stopPWM();
				alarm(0);
				break;
			}
			else{
				alarm(0);
				parseMessage(buffer, recMsgSize);
				sendThread->setImageQuality(imageQuality);
				recvCounter = 0;
				
				if(firstRun)
				{
					gettimeofday(&lastMsgTime, NULL);
					firstRun = false;
				}
				else
				{
					struct timeval now;
					long seconds, useconds;
					gettimeofday(&now, NULL);
					
					seconds = now.tv_sec - lastMsgTime.tv_sec;
					useconds = now.tv_usec - lastMsgTime.tv_usec;
					
					double totSec = seconds + useconds / 1000000.0;
					//printf("%f \n", totSec);
					if(totSec > 1.0)
						stabilized = false;
					else
						stabilized = true;
					
					sendThread->setStabilized(stabilized);
					
					lastMsgTime = now;
				}
				
				if(stabilized)
				{
					recTime = 1;
				}
				else
				{
					recTime = 3;
				}
				
				if(gotConfig)
				{
					if(!senderStarted)
					{
						printf("Starting sendThread\n");
						std::string s(frameIP);
						sendThread->setUdp(udpSend);
						sendThread->setTargetIP(s);
						sendThread->setWidth(imageWidth);
						sendThread->setHeight(imageHeight);
						sendThread->setImageQuality(imageQuality);
						
						sendThread->start();
						
						senderStarted = true;
					}
					else //this is probalby deprecated now
					{
						printf("initializing sendThread\n");
						std::string s(frameIP);
						sendThread->setUdp(udpSend);
						sendThread->setTargetIP(s);
						sendThread->setWidth(imageWidth);
						sendThread->setHeight(imageHeight);
						sendThread->setImageQuality(imageQuality);
						sendThread->setHalted(false);
					}
					gotConfig = false;
					lcd->setGotConfig(true);
					ready = true;
				}
			}
			
			sendFailCounter = sendThread->getSendfailcounter();
			
			if(sendFailCounter >= 150 || recvCounter >= 150)
			{
				printf("Send failed triggered! s: %d r: %d\n", sendFailCounter, recvCounter);
				connected = false;
				globalConnected = false;
				senderStarted = false;
				//sendThread->stop();
				clientSocket = -1;
				stopPWM();
			}
			
			uint8_t val = bcm2835_gpio_lev(IRCAMERA);
			//val = 1; //disable
			//printf("%d\n", val);
			if(val == 0 && ready) //Detected something, stop throttle 
			{
				obstructed = true;
				setPWM(throttleChannel, 160); //stopping throttle channel
			}
			else
			{
				obstructed = false;
			}
			
			autoHome.update(); //update autohome algorithm
			int currentHomeIndex = autoHome.getCurrentIndex(); //check if we should send new state to client
			
			if(currentHomeIndex != lastAutoHomeIndex && !autoHome.goingHome() && currentHomeIndex != 0)
			{
				printf("Index: %d, prev: %d\n", currentHomeIndex, lastAutoHomeIndex);
				if(autoHome.getCheckpoint(currentHomeIndex-1).distance > 0) //only send this state if the distance is greater than 0, it should be now..
				{
					sendThread->addState(autoHome.getCheckpoint(currentHomeIndex-1));
				}
				lastAutoHomeIndex = currentHomeIndex;
			}
			else if(autoHome.goingHome() && currentHomeIndex != lastAutoHomeIndex) //the car is now moving home, send index to get a reference in the client window
			{
				sendThread->currentCheckpoint(currentHomeIndex);
				lastAutoHomeIndex = currentHomeIndex;
			}
		}
		printf("Leaving connected loop\n");
		ready = false;
		sendThread->stop(); //stop the sendthread on disconnect
		//delete sendThread; //this will create a memory corruption, everything is freed when stop() is called
		senderStarted = false; // so that the sendthread will start on new instance
	}
	stopPWM();
	lcd->stop();
	adc->stop();
	sendThread->stop();
	music->stop();
	printf("Shutting down \n");
	delete[] frameIP;
	delete lcd;
	delete adc;
	bcm2835_close();
		
	return 0;
}