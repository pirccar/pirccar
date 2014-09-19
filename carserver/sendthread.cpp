#include "sendthread.h"

SendThread::SendThread(void)
: Thread()
{
}

SendThread::SendThread(std::string ip)
: Thread()
{
	this->ip = ip;
	
}

void SendThread::setUdp(bool udp)
{
	this->isUdp = udp;
}

void SendThread::setTargetIP(std::string ip)
{
	this->ip = ip;
}

void SendThread::setWidth(int width)
{
	this->imageWidth = width;
}

void SendThread::setHeight(int height)
{
	this->imageHeight = height;
}

void SendThread::setImageQuality(int imageQuality)
{
	this->imageQuality = imageQuality;
}

void SendThread::setStabilized(bool stabilized)
{
	this->stabilized = stabilized;
}

bool SendThread::getStabilized()
{
	return stabilized;
}

int SendThread::getSendfailcounter()
{
	return sendfailcounter;
}

void SendThread::setHalted(bool halted)
{
	bool temp = this->halted;
	this->halted = halted;
	
	if(temp && !halted)
	{
		printf("cleanup \n");
		cleanup();
		printf("init \n");
		init();
	}
}

void SendThread::setChannel(int channel, std::string value)
{
	if(channel == 0)
		chann0String = value;
	else if(channel == 1)
		chann1String = value;
	else if(channel == 2)
		chann2String = value;
	else if(channel == 3)
		chann3String = value;
}

void SendThread::readGPS()
{
	unsigned char serial_buf[256];
			
	//read serialport (GPS)
	int ser_read = RS232_PollComport(gpsComport, serial_buf, 256);
	if(ser_read > 0) //found data
	{
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
				if(socket.write(send) != serial_command_buf.length())
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

void SendThread::compressAndSend(unsigned char imageBuffer[])
{
	unsigned char rgbBuffer[(imageWidth*imageHeight)*3];
	int count = 0;
	int bufferSize = imageWidth*imageHeight*4;
	//convert from RGBA to RGB
	for(int i = 0; i < bufferSize; i+=4)
	{
		rgbBuffer[count] = imageBuffer[i];
		rgbBuffer[count+1] = imageBuffer[i+1];
		rgbBuffer[count+2] = imageBuffer[i+2];
			
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
	
	cinfo.image_width = imageWidth;  
    cinfo.image_height = imageHeight;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
	
    jpeg_set_defaults( &cinfo );
    jpeg_set_quality(&cinfo, imageQuality, TRUE);
	
    jpeg_start_compress( &cinfo, TRUE );
    
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &rgbBuffer[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
	
    jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
   
	
	int nSent = 0;
	
	//printf("Send size: %lu \n", mem_size); 
	if((nSent = socket.write(mem, mem_size)) != mem_size)
	{
		printf("Can't send\n");
		printf("Outputsize: %ld Sent: %d\n", mem_size, nSent);
		sendfailcounter++;
	}
	else
	{
		//printf("Sent: %d \n", nSent);
		//printf("Sent to: %s \n", inet_ntoa(frameServerAddr.sin_addr));
		sendfailcounter = 0;
	}
	
	delete[] mem;
}

void SendThread::init(void)
{
	socket = Socket(isUdp, true, 8002);
	socket.initialize();
	socket.connect(ip);
	
	
	cam = CameraWrapper();
	cam.setWidthHeight(imageWidth, imageHeight);
	cam.initialize();
	
	gpsComport = 22;
	gpsBaud = 9600;
	
	serial_command_len = 0;
	stabilized = prev_stab_state = false;
	
	if(RS232_OpenComport(gpsComport, gpsBaud)) //GPS 16 usb 4800, 22 uart 9600
	{
		printf("Couldn't open comport \n");
	}
	
	sendfailcounter = 0;
	clock_gettime(CLOCK_REALTIME, &now);
	start_time = now.tv_sec;
	printf("t3\n");
}

void SendThread::mainLoop(void)
{
	if(!halted)
	{
		//Send texture			
		unsigned char* imageBuffer = NULL;
		imageBuffer = cam.getBuffer();
		if(imageBuffer != NULL) //read an image from the camera
		{
			compressAndSend(imageBuffer);
			batCount++;
			stableCount++;
			gearCount++;
		}
		else
		{
			usleep(50000);
			return;
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
				if(socket.write("ready") != 5)
				{
					printf("Failed to send stabilized message \n");
				}
			}
			else
			{
				if(socket.write("unready") != 7)
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
			if(socket.write(send) != sendStr.length())
			{
				printf("UDP disconnected\n");
			}
			
			batCount = 0;
		}
		else if(gearCount >= 32) //send gear info
		{
			int gearValue = 470;//getPWMOff(gearChannel);
			if(gearValue < 470)
			{
				if(socket.write("G0") != 2)
				{
					printf("Failed to send gear message message \n");
				}
			}
			else
			{
				if(socket.write("G1") != 2)
				{
					printf("Failed to send gear message message \n");
				}
			}
			gearCount = 0;
		}
		prev_stab_state = stabilized;
	}
	else
	{
		sleep(3);
	}
}

void SendThread::cleanup(void)
{
	socket.disconnect();
	RS232_CloseComport(gpsComport);
	cam.close();
}