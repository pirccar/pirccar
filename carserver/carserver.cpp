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

#include "camera.h"
#include "lcdscreen.h"
#include "adcthread.h"
#include "lcdthread.h"
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


// pin setup
int _din = 4;
int _sclk = 5;
int _dc = 3;
int _rst = 0;
int _cs = 2;

//Tritech bitmap
const uint8_t image_data_tritech[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 
0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x00, 
0x00, 0xFF, 0xFF, 0xFF, 0x83, 0x83, 0x83, 0x83, 0x83, 0xC3, 0xE7, 0x7E, 0x1C, 0x00, 0x00, 0xC6, 0xEF, 0xEF, 0xEF, 0xC6, 
0x00, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 
0x03, 0x03, 0x00, 0xF0, 0xF8, 0xFE, 0x07, 0x03, 0x03, 0x01, 0x01, 0x01, 0x03, 0x07, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 
0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x03, 0x07, 0x1F, 0x3C, 0xF0, 0xC0, 0x00, 0x00, 0x30, 
0x79, 0x79, 0x79, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 
0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x03, 0x0F, 0x1F, 0x38, 0x70, 0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x70, 0x00, 
0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00 };

//Compresses a raw RBGA image to jpeg
void compressImage()
{
	unsigned char rgbBuffer[(MAIN_TEXTURE_WIDTH*MAIN_TEXTURE_HEIGHT)*3];
	int count = 0;
	
	//convert from RGBA to RGB
	for(int i = 0; i < BUFFER_SIZE; i+=4)
	{
		rgbBuffer[count] = tmpBuffer[i];
		rgbBuffer[count+1] = tmpBuffer[i+1];
		rgbBuffer[count+2] = tmpBuffer[i+2];
			
		count+=3;
	}

	unsigned char* mem = NULL;
	unsigned long mem_size = 0;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &mem, &mem_size);
	
	//jpeg_stdio_dest(&cinfo, outfile);
	
	cinfo.image_width = MAIN_TEXTURE_WIDTH;  
    cinfo.image_height = MAIN_TEXTURE_HEIGHT;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
	
    jpeg_set_defaults( &cinfo );
    jpeg_set_quality(&cinfo, IMAGE_QUALITY, TRUE);
	
    jpeg_start_compress( &cinfo, TRUE );
    
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &rgbBuffer[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
	
    jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
   
	//printf("Compressed size: %d \n", mem_size);
	memcpy(tmpBuffer, mem, mem_size);
	outputSize = (int)mem_size;
	
	delete[] mem;
}

//Compresses and image and sends it to a client
void compressAndSend(unsigned char buffer[],int frameServerSocket, struct sockaddr_in frameServerAddr)
{
	unsigned char rgbBuffer[(imageWidth*imageHeight)*3];
	int count = 0;
	int bufferSize = imageWidth*imageHeight*4;
	//convert from RGBA to RGB
	/*for(int i = 0; i < bufferSize; i+=4)
	{
		rgbBuffer[count] = buffer[i];
		rgbBuffer[count+1] = buffer[i+1];
		rgbBuffer[count+2] = buffer[i+2];
			
		count+=3;
	}*/
	
	unsigned char* mem = NULL;
	unsigned long mem_size = 0;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &mem, &mem_size);
	
	//jpeg_stdio_dest(&cinfo, outfile);
	
	cinfo.image_width = imageWidth;  
    cinfo.image_height = imageHeight;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
	
    jpeg_set_defaults( &cinfo );
    jpeg_set_quality(&cinfo, imageQuality, TRUE);
	
    jpeg_start_compress( &cinfo, TRUE );
    
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &buffer[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
	
    jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
   
	
	int nSent = 0;
	
	if((nSent = sendto(frameServerSocket, mem, mem_size, 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr))) != mem_size)
	{
		printf("Can't send\n");
		printf("Outputsize: %ld Sent: %d\n", mem_size, nSent);
		sendFailCounter++;
	}
	else
	{
		//printf("Sent to: %s \n", inet_ntoa(frameServerAddr.sin_addr));
		sendFailCounter = 0;
	}
	
	delete[] mem;
}

//Send msg to client
void sendToClient(int clientSocket, const char* msg){
	int msgLen = strlen(msg);
	char buffer[32];
	int n;
	sprintf(buffer, "%d\n", 1);
	
	printf("Msglen: %d \n", msgLen);
	if( (n = write(clientSocket, msg, msgLen)) < 0)
		printf("Error sending message \n");
}

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

//Creates a string containing the value of the ADC channel, used for sending data to client
/*std::string getADCString(int channel)
{
	double val = readADCValue(channel);
	std::ostringstream os;
	os << channel; //channel
	os << val; //value
	std::string marker = "V";
	std::string str = marker + os.str();
	
	return str;
}*/

//Sendthread UDP
void *sendThreadUDP(void *ptr)
{
	//For rendering camra on PI
	/*InitGraphics();
	GfxTexture texture;
	texture.CreateRGBA(MAIN_TEXTURE_WIDTH, MAIN_TEXTURE_HEIGHT);
	texture.GenerateFrameBuffer();
	*/
	
	//The camera
	//StopCamera();
	Camera cam;
	
	cam.setWidthHeight(160, 120);
	cam.initialize();
	
	std::string serial_command_buf;
	int serial_command_len = 0;
	
	//UDP Socket
	int frameServerSocket;
	struct sockaddr_in frameServerAddr;
	unsigned short frameServerPort;
	//char frameIP[] = "172.26.101.205";
	
	frameServerPort = 8002;
	
	uint8_t run = 1;
	uint8_t connected = 0;
	
	if((frameServerSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		printf("Socket fail \n");
	
	
	memset(&frameServerAddr, 0, sizeof(frameServerAddr));
	frameServerAddr.sin_family = AF_INET;
	frameServerAddr.sin_addr.s_addr = inet_addr(frameIP);
	frameServerAddr.sin_port = htons(frameServerPort);
	
	//SPI for battery values
	printf("Starting SPI \n");
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	printf("SPI Init done \n");
	stabilized = false;
	long int start_time;
	struct timespec now;
	bool prev_stab_state = false;
	
	int batCount = 0;
	int stableCount = 0;
	int gearCount = 0;
	
	printf("Init comport \n");
	if(RS232_OpenComport(gpsComport, gpsBaud)) //GPS 16 usb 4800, 22 uart 9600
	{
		printf("Couldn't open comport \n");
	}
	printf("Comport init done! \n");
	clock_gettime(CLOCK_REALTIME, &now);
	start_time = now.tv_sec;
	
	while(globalConnected)
	{
		//Send texture			
		int n = 0;
		unsigned char* imageBuffer = NULL;
		//printf("Trying to read camera \n");
		imageBuffer = cam.getBuffer();
		if(imageBuffer != NULL) //read an image from the camera
		{
			compressAndSend(imageBuffer, frameServerSocket, frameServerAddr);

			batCount++;
			stableCount++;
			gearCount++;
		}
		else
		{
			usleep(50000);
			continue;
		}
		
		//Unstable / stable connection
		if(prev_stab_state && !stabilized)
		{
			clock_gettime(CLOCK_REALTIME, &now);
			start_time = now.tv_sec;
			
			printf("connection unstable \n");
		}
		
		if(!stabilized)
		{
			clock_gettime(CLOCK_REALTIME, &now);
			if(now.tv_sec - start_time > 5)
			{
				stabilized = true;
				
				printf("connection stablilized \n");
			}
		}
		
		
		//let the client know about the connection status
		if(stableCount >= 24)
		{
			if(stabilized)
			{
				if(sendto(frameServerSocket, "ready", 5, 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != 5)
				{
					printf("Failed to send stabilized message \n");
				}
			}
			else
			{
				if(sendto(frameServerSocket, "unready", 7, 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != 7)
				{
					printf("Failed to send unstable message \n");
				}
			}
			stableCount = 0;
		}
		
		
		//Send battery info
		if(batCount >= 12)
		{
			
			std::string sendStr = chann0String+ ":" + chann1String + ":" + chann2String + ":" + chann3String;;
			//sendStr += 
			const char* send = sendStr.c_str();
			//printf("Sending: %s \n", send);
			if(sendto(frameServerSocket, send, sendStr.length(), 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != sendStr.length())
			{
				connected = false;
				printf("UDP disconnected\n");
			}
			
			batCount = 0;
		}
		else if(gearCount >= 32) //send gear info
		{
			int gearValue = getPWMOff(gearChannel);
			if(gearValue < 470)
			{
				if(sendto(frameServerSocket, "G0", 2, 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != 2)
				{
					printf("Failed to send gear message message \n");
				}
			}
			else
			{
				if(sendto(frameServerSocket, "G1", 2, 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != 2)
				{
					printf("Failed to send gear message message \n");
				}
			}
			gearCount = 0;
		}
		prev_stab_state = stabilized;
		unsigned char serial_buf[256];
		
		//read serialport (GPS)
		//printf("GPS time \n");
		int ser_read = RS232_PollComport(gpsComport, serial_buf, 256);
		//printf("GPS Read done \n");
		if(ser_read > 0) //found data
		{
			//printf("GPS found data \n");
			serial_buf[ser_read] = 0;
			std::string sub_command((char*)serial_buf);
			std::string comp("\n");
			std::size_t found = sub_command.find(comp);
			if(found != std::string::npos) //if current buf contains \n (end of GPS command)
			{
				char save_buf[256];
				int save_len = 0;
				
				//check if \n is the last char
				if(found != sub_command.length()-1)
				{
					//if \n wasn't the last char read then there is data from a new command after the \n command, save those chars and parse later
					std::size_t length = sub_command.copy(save_buf, sub_command.length() - found -1, found+1);
					save_buf[length] = '\0';
					sub_command[found+1] = '\0';
					save_len = length;
				}
				
			
				if(serial_command_len == 0) //whole command found in one read
				{
					serial_command_buf = sub_command.substr(0, found+1);
				}
				else //Command found, composed of old read
				{
					serial_command_buf += sub_command.substr(0, found+1);
				}
				
				//of the command is of type RMC, then send it to the client 
				comp = "GPRMC";
				found = serial_command_buf.find(comp);
				if(found != std::string::npos)
				{
					//send to client
					char const* send = serial_command_buf.c_str();
					if(sendto(frameServerSocket, send, serial_command_buf.length(), 0, (struct sockaddr*)&frameServerAddr, sizeof(frameServerAddr)) != serial_command_buf.length())
					{
						printf("UDP disconnected\n");
					}
				}
				serial_command_len = save_len;
				serial_command_buf = save_buf;
			}
			else // no \n save untill \n is found
			{
				if(serial_command_len == 0)
				{
					serial_command_buf = sub_command;
				}
				else
				{
					serial_command_buf += sub_command;
				}
				serial_command_len += ser_read;
			}
		}
	}
	
	printf("Closing com \n");
	RS232_CloseComport(gpsComport);
	
	printf("stopping camera\n");
	//StopCamera();
	
	printf("closing socket\n");
	close(frameServerSocket);
	
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
	
	pthread_t t1;
	
	LcdThread* lcd = new LcdThread();
	AdcThread* adc = new AdcThread(lcd);
	
	if(!bcm2835_init()){
		printf("BCM error \n");
		return 1;
	}
	
	bcm2835_gpio_fsel(IRCAMERA, BCM2835_GPIO_FSEL_INPT);
	
	PCA9685_init();
	
	printf("Resetting PCA9685\n");
	PCA9685_reset();
	
	setPWMFreq(50);
	
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
	lcd->stop();
	adc->stop();
	//pthread_join(lcd_thread, NULL);
	printf("Shutting down \n");
	delete[] frameIP;
	delete lcd;
	delete adc;
	bcm2835_close();
		
	return 0;
}