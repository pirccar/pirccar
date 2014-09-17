#pragma once
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <jpeglib.h>


#include "camera.h"
#include "thread.h"
#include "socket.h"
#include "rs232.h"

class SendThread : public Thread
{
public:
	SendThread(void);
	SendThread(std::string ip);
	void setTargetIP(std::string ip);
	void setWidth(int width);
	void setHeight(int height);
	void setImageQuality(int imageQuality);
	void setStabilized(bool stabilized);
	bool getStabilized();
	int getSendfailcounter();
	void setHalted(bool halted);
	void setChannel(int channel, std::string value);
	
private:
	Socket socket;
	Camera cam;
	std::string ip;
	std::string serial_command_buf;
	std::string chann0String;
	std::string chann1String;
	std::string chann2String;
	std::string chann3String;
	int imageWidth, imageHeight, imageQuality;
	int sendfailcounter;
	int serial_command_len;
	int gpsComport; //usb = 16 uart = 22
	int gpsBaud; //usb = 4800, uart = 9600
	bool stabilized;
	bool halted;
	long int start_time;
	struct timespec now;
	bool prev_stab_state;
	int batCount;
	int stableCount;
	int gearCount;
	
	void readGPS();
	void compressAndSend(unsigned char imageBuffer[]);
	
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};