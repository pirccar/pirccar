#include <errno.h>
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
#include <iostream>
#include <fstream>
#include <jpeglib.h>
//#include <owcapi.h>
//#include <wiringPi.h>
//#define _GNU_SOURCE
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "camera.h"
#include "graphics.h"
#include "jpge.h"
#include "lodepng.h"
#include "rs232.h"

#define MAIN_TEXTURE_WIDTH 128
#define MAIN_TEXTURE_HEIGHT 128
#define IMAGE_QUALITY 100
#define BUFFER_SIZE (MAIN_TEXTURE_WIDTH * MAIN_TEXTURE_HEIGHT)*4
#define BLOCK_SIZE 2048
#define IRCAMERA RPI_V2_GPIO_P1_18

unsigned char tmpBuffer[(MAIN_TEXTURE_WIDTH*MAIN_TEXTURE_HEIGHT)*4];
bool gotConfig = false;
bool ready = false;
bool obstructed = false;
bool globalConnected = false;
bool lcdOn = true;
bool lcdConfig = false;
char *frameIP;

int imageWidth;
int imageHeight;
int imageQuality;

int throttleFChannel = 3;
int throttleBChannel = 4;
int gearChannel = -1;

int framesPerSecond = 30;
int outputSize = 0;
sig_atomic_t alarm_counter;
clock_t mes_start;
bool mes_ready = true;
int recTime = 10;
bool stabilized = false;
int sendFailCounter = 0;
std::string localAdresses[10];
int localAdressCount;


double chann0Double;
double chann1Double;
double chann2Double;
double chann3Double;
std::string chann0String;
std::string chann1String;
std::string chann2String;
std::string chann3String;


int gpsComport = 22; //usb = 16 uart = 22
int gpsBaud = 9600; //usb = 4800, uart = 9600
double lastCarbat = 0.0;
double lastPibat = 0.0;

//Deprecated function
bool sendCamera(int clientSocket, int length){
	//printf("Msglen: %d \n", length);
	int n = 0;
	if( (n = write(clientSocket, tmpBuffer, length)) < 0)
	{
		printf("Error sending message \n");
		return false;
	}
	
	return true;
}

//Parse incoming messages
int parseMessage(char buf[], int size){
	uint8_t servo;
	int value;
	int i = 0;
	
	if(size <= 0){
		printf("No data to parse \n");
		return 0;
	}
		
		
	if(buf[0] == 'P'){ // is ping
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
		
		
		printf("Config was received \n");
		gotConfig = true;
	}
	else if(ready){ //is command S010400= set servo 1 to 400 off
		
		int i = 0;
		while(i + 6 < size)
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
					setPWM(servo, value);
				
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
	for(int i = 1; i < 17; i++)
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

	if(!bcm2835_init()){
		printf("BCM error \n");
		return 1;
	}
	
	bcm2835_gpio_fsel(IRCAMERA, BCM2835_GPIO_FSEL_INPT);
	
	Carserver_network random; //Call class
	
	pthread_t t1;
	pthread_t lcd_thread;
	pthread_t adc_thread;
	pthread_create(&adc_thread, NULL, adcReadThread, NULL); //Start ADC-thread
	pthread_create(&lcd_thread, NULL, lcdThread, NULL);	//Start LCD-thread
	
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
		stopPWM();
		clientLen = sizeof(clientAddr);
		
		if((clientSocket = accept(serverSocket, (sockaddr*) &clientAddr, &clientLen)) < 0)
			printf("Accept failed \n");
			
		printf("Got connection from: %s \n", inet_ntoa(clientAddr.sin_addr));
		frameIP = inet_ntoa(clientAddr.sin_addr);
		printf("Setting frameIP to: %s \n", frameIP);
		printf("Waiting for config file \n");
		connected = true;
		globalConnected = true;
		sendFailCounter = 0;
		int recvCounter = 0;
		
		while(connected)
		{
			char buffer[256];
			bzero(buffer, 256);
			int recMsgSize;
			alarm(recTime);
			if( (recMsgSize = recv(clientSocket, buffer, 255, 0)) < 0){
				printf("Recv failed \n"); //Indicates that it took more than 1 second to get a message from the client, bad connection
				//connected = false;
				//globalConnected = false;
				//senderStarted = false;
				//clientSocket = -1;
				stopPWM();
				alarm(0);
				recTime = 10; //Increase recv timer to 10 seconds
				stabilized = false;
				recvCounter++;
				//break;
			}
			else if(recMsgSize == 0)
			{
				printf("Client disconnected \n");
				connected = false;
				globalConnected = false;
				senderStarted = false;
				//clientSocket = -1;
				lcdConfig = false;
				stopPWM();
				alarm(0);
				break;
			}
			else{
				alarm(0);
				if(parseMessage(buffer, recMsgSize)){
				
					
				}
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
						printf("Starting UDP thread\n");
						pthread_create(&t1, NULL, sendThreadUDP, NULL);
						senderStarted = true;
					}
					gotConfig = false;
					lcdConfig = true;
					ready = true;
				}
			}
			
			
			if(sendFailCounter >= 150 || recvCounter >= 15)
			{
				connected = false;
				globalConnected = false;
				senderStarted = false;
				lcdConfig = false;
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
	pthread_join(t1, NULL);
	lcdOn = false;
	pthread_join(lcd_thread, NULL);
	printf("Shutting down \n");
	delete[] frameIP;
	bcm2835_close();
		
	return 0;
}