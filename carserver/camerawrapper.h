#pragma once
#include <stdio.h>
#include "camera.h"

class CameraWrapper
{
	private:
	unsigned char* buffer; //internal buffer for image
	int width, height; //size of image
	int internalFailCounter;
	CCamera* cam;
	
	void allocateBuffer(); //allocates memory for the buffer
	
	public:
	CameraWrapper(void);
	~CameraWrapper(void);
	
	void initialize(); //intializes the camera with size and format, also opens the camera and makes it ready to capture
	void close(); //stops the camera and release all resources
	void setWidthHeight(int width, int height); //sets width and height, should be called before initialize. TODO: split into separate functions
	
	unsigned int getWidth(); //get the width of the image
	unsigned int getHeight(); //get the height of the image	
	unsigned char* getBuffer(); //get the buffer, this will force the camera to get a new frame, Warning not thread safe!
};