#include "camerawrapper.h"

CameraWrapper::CameraWrapper(void)
{
	this->width = 320;
	this->height = 240;
	this->internalFailCounter = 0;
	
	buffer = NULL;
}
CameraWrapper::~CameraWrapper(void)
{
	delete[] buffer;
	buffer = NULL;
}

void CameraWrapper::allocateBuffer()
{
	//if the buffer points to something, delete the old data and allocate new memory
	if(buffer != NULL)
		delete[] buffer;
		
	buffer = new unsigned char[width*height*4];
}

void CameraWrapper::initialize()
{
	StopCamera();
	sleep(1);
	printf("Starting camera \n");
	cam = StartCamera(width, height, 40, 1, true);
	
	allocateBuffer();
}

void CameraWrapper::close()
{
	StopCamera();
	delete[] buffer;
	buffer = NULL;
}

void CameraWrapper::setWidthHeight(int width, int height)
{
	this->width = width;
	this->height = height;
	allocateBuffer(); //reallocate the buffer just in case the size changed
}

unsigned char* CameraWrapper::getBuffer()
{
	int n = cam->ReadFrame(0,buffer,sizeof(unsigned char)*width*height*4);
	//printf("ReadFrame ret: %d\n", n);
	if(n > 0)
	{
		internalFailCounter = 0;
		return buffer;
	}
	else
	{
		internalFailCounter++;
		
		if(internalFailCounter >= 15000)
		{
			printf("Triggering camera restart \n");
			//initialize();
			internalFailCounter = 0;
		}
		return NULL;
	}
}

unsigned int CameraWrapper::getWidth()
{
	return width;
}

unsigned int CameraWrapper::getHeight()
{
	return height;
}