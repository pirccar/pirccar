#include "camerawrapper.h"

CameraWrapper::CameraWrapper(void)
{
	this->width = 320;
	this->height = 240;
	
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
	cam = StartCamera(width, height, 30, 1, true);
	
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
	if(n >0)
		return buffer;
	else
		return NULL;
}

unsigned int CameraWrapper::getWidth()
{
	return width;
}

unsigned int CameraWrapper::getHeight()
{
	return height;
}