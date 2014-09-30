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
#include <jpeglib.h>

#include "pca9685.h"
#include "rs232.h"

//#include <wiringPi.h>
#include "PCD8544.h"

#define MAIN_TEXTURE_WIDTH 160
#define MAIN_TEXTURE_HEIGHT 120
#define IMAGE_QUALITY 100
#define BUFFER_SIZE (MAIN_TEXTURE_WIDTH * MAIN_TEXTURE_HEIGHT)*3
#define BLOCK_SIZE 2048
#define IRCAMERA RPI_V2_GPIO_P1_18

bool gotConfig = false;
bool ready = false;
bool obstructed = false;
bool globalConnected = false;
char *frameIP;

int imageWidth;
int imageHeight;
int imageQuality;

int throttleFChannel = 3;
int throttleBChannel = 4;
int gearChannel = -1;

sig_atomic_t alarm_counter;
int recTime = 10;
bool stabilized = false;
bool udpSend = true;
int sendFailCounter = 0;

double chann0Double;
double chann1Double;
double chann2Double;
double chann3Double;
std::string chann0String;
std::string chann1String;
std::string chann2String;
std::string chann3String;


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
		if(size < 16)
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
		throttleFChannel = atoi(id);
		printf("Setting throttle to channel %d \n", throttleFChannel);
		
		id[0] = buf[3];
		id[1] = buf[4];
		id[2] = '\0';
		throttleBChannel = atoi(id);
		printf("Setting throttle to channel %d \n", throttleBChannel);
		
		//Gear
		id[0] = buf[5];
		id[1] = buf[6];
		id[2] = '\0';
		gearChannel = atoi(id);
		printf("Setting gearChannel to channel %d \n", gearChannel);
		
		format[0] = buf[7];
		format[1] = buf[8];
		format[2] = buf[9];
		format[3] = buf[10];
		format[4] = '\0';
		
		imageWidth = atoi(format);
		
		format[0] = buf[11];
		format[1] = buf[12];
		format[2] = buf[13];
		format[3] = buf[14];
		format[4] = '\0';
		
		imageHeight = atoi(format);
		printf("Setting Image format to %dx%d \n", imageWidth, imageHeight);
		
		quality[0] = buf[15];
		quality[1] = buf[16];
		quality[2] = buf[17];
		quality[3] = '\0';
		
		imageQuality = atoi(quality);
		printf("Setting image quality to %d \n", imageQuality);
		
		if(buf[18] == '1')
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
		gotConfig = true;
	}
	else if(ready){ //is command S010400 = set servo 1 to 400 off
		//printf("Got servo data \n");
		
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
				if(servo != throttleFChannel || servo != throttleBChannel)
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
					//printf("Setting pwm\n");
					//setPWM(servo, value);
					//printf("pwm set done\n");
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
	SendThread* sendThread = new SendThread();
	AdcThread* adc = new AdcThread(lcd, sendThread);
	ServoControlThread* servo = new ServoControlThread();
	
	if(!bcm2835_init()){
		printf("BCM error \n");
		return 1;
	}
	
	bcm2835_gpio_fsel(IRCAMERA, BCM2835_GPIO_FSEL_INPT);
	
//	PCA9685_init();
//	printf("Resetting PCA9685\n");
//	PCA9685_reset();
//	setPWMFreq(50);
	
	servo->start();
	adc->start();
	lcd->start();
	
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
		clientLen = sizeof(clientAddr);
		
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
		
		while(connected)
		{
			char buffer[256];
			bzero(buffer, 256);
			int recMsgSize;
			alarm(recTime);
			//printf("Looping \n");
			
			//printf("Reading message \n");
			if( (recMsgSize = recv(clientSocket, buffer, 255, 0)) < 0){
				printf("Recv failed \n"); //Indicates that it took more than 1 second to get a message from the client, bad connection
				stopPWM();

				alarm(0);
				recTime = 10; //Increase recv timer to 10 seconds
				stabilized = false;
				if(!senderStarted)
					sendThread->setStabilized(false);
				recvCounter++;
				printf("End of recv failed\n");
				//break;
			}
			else if(recMsgSize == 0)
			{
				printf("Client disconnected \n");
				connected = false;
				globalConnected = false;
				lcd->setConnected(globalConnected);
				sendThread->setHalted(true);
				senderStarted = true;
				//clientSocket = -1;
				stopPWM();
				alarm(0);
				break;
			}
			else{
				//printf("parsing message \n");
				alarm(0);
				parseMessage(buffer, recMsgSize);
				//printf("After parsing \n");
				sendThread->setImageQuality(imageQuality);
				recvCounter = 0;
				if(stabilized)
				{
					recTime = 1;
				}
				else
				{
					recTime = 10;
				}
				
				if(gotConfig)
				{
					if(!senderStarted)
					{
						printf("Starting sendThread\n");
						//pthread_create(&t1, NULL, sendThreadUDP, NULL);
						std::string s(frameIP);
						sendThread->setUdp(udpSend);
						sendThread->setTargetIP(s);
						sendThread->setWidth(imageWidth);
						sendThread->setHeight(imageHeight);
						sendThread->setImageQuality(imageQuality);
						
						sendThread->start();
						
						senderStarted = true;
					}
					else
					{
						printf("initializing sendThread");
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
			
			//printf("External Sendfail: %d \n", sendFailCounter);
			if(sendFailCounter >= 150 || recvCounter >= 15)
			{
				printf("Send failed triggered!\n");
				connected = false;
				globalConnected = false;
				senderStarted = false;
				sendThread->stop();
				clientSocket = -1;
				stopPWM();
			}
			
			uint8_t val = bcm2835_gpio_lev(IRCAMERA);
			val = 1; //disable
			if(val == 0 && ready) //Detected something, stop throttle 
			{
				obstructed = true;
				setPWM(throttleFChannel, 160); //stopping throttle channel
				setPWM(throttleBChannel, 160); //stopping throttle channel
				
			}
			else
			{
				obstructed = false;
			}
			//printf("value: %d \n", val);
		}
		printf("Leaving connected loop\n");
		ready = false;
	}
	stopPWM();
	//pthread_join(t1, NULL);
	lcd->stop();
	adc->stop();
	sendThread->stop();
	//pthread_join(lcd_thread, NULL);
	printf("Shutting down \n");
	delete[] frameIP;
	delete lcd;
	delete adc;
	delete sendThread;
	bcm2835_close();
		
	return 0;
}