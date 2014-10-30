#pragma once
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <jpeglib.h>


#include "camerawrapper.h"
#include "thread.h"
#include "socket.h"
#include "rs232.h"
#include "autoHome.h"

class SendThread : public Thread
{
public:
	SendThread(void);
	SendThread(std::string ip);
	void setUdp(bool udp);
	void setTargetIP(std::string ip);
	void setWidth(int width);
	void setHeight(int height);
	void setImageQuality(int imageQuality);
	void setStabilized(bool stabilized);
	bool getStabilized();
	int getSendfailcounter();
	void setHalted(bool halted);
	void setChannel(int channel, std::string value);
	void addState(AutoHomeData state);
	void currentCheckpoint(int checkpoint);
	
private:
	Socket socket;
	CameraWrapper cam;
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
	bool isUdp;
	long int start_time;
	struct timespec now;
	bool prev_stab_state;
	int batCount;
	int stableCount;
	int gearCount;
	
	std::vector<AutoHomeData> states;
	bool sendNewState;
	bool sendNewCheckpoint;
	int checkpoint;
	
	void readBattery();
	bool readCamera();
	void readGear();
	void readGPS();
	void readStabilized();
	void compressAndSend(unsigned char imageBuffer[]);
	void sendState();
	void sendCheckpoint();
	
	virtual void init(void);
	virtual void mainLoop(void);
	virtual void cleanup(void);
};