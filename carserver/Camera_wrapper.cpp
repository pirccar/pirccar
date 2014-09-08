#include "Camera_wrapper.h"

Camera_wrapper::Camera_wrapper(void)
{
	width = 0;
	height = 0;
	imageQuality = 0;
	execute = false;
	alive = true;
	
	pthread_create(&t1, NULL, run, NULL);
}

Camera_wrapper::Camera_wrapper(int width, int height, int quality)
{
	this.width = width;
	this.height = height;
	this.imageQuality = quality;
	execute = false;
	alive = true;
}

Camera_wrapper::~Camera_wrapper(void)
{
	StopCamera();
	alive = false;
}