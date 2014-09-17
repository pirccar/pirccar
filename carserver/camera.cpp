#include "camera.h"


	Camera::Camera(void)
	{
		this->width = 320;
		this->height = 240;
		this->format = raspicam::RASPICAM_FORMAT_RGB;
		
		buffer = NULL;
	}
	Camera::~Camera(void)
	{
		delete[] buffer;
		buffer = NULL;
	}
	
	void Camera::allocateBuffer()
	{
		//if the buffer points to something, delete the old data and allocate new memory
		if(buffer != NULL)
			delete[] buffer;
			
		buffer = new unsigned char[cam.getImageBufferSize()];
	}
	
	void Camera::initialize()
	{
		cam.setFormat(this->format);
		cam.setWidth(this->width);
		cam.setHeight(this->height);
		cam.setAWB(raspicam::RASPICAM_AWB_OFF);
		if(!cam.open()) //opens the camera, aka initializes all internal settings and HW, making the camera ready to capture
		{
			printf("Error opening camera \n");
		}
		
		allocateBuffer();
	}
	
	void Camera::close()
	{
		cam.release();
		delete[] buffer;
		buffer = NULL;
	}
	
	void Camera::setFormat(raspicam::RASPICAM_FORMAT format)
	{
		this->format = format;
		cam.setFormat(this->format);
		allocateBuffer(); //reallocate the buffer just in case the size changed
	}
	void Camera::setWidthHeight(int width, int height)
	{
		this->width = width;
		this->height = height;
		cam.setWidth(this->width);
		cam.setHeight(this->height);
		allocateBuffer(); //reallocate the buffer just in case the size changed
	}
	
	unsigned char* Camera::getBuffer()
	{
		cam.grab(); //grabs a frame
		cam.retrieve(buffer); //get the frame and place the data in "buffer"
		
		return buffer;
	}
	
	unsigned int Camera::getWidth()
	{
		return width;
	}
	
	unsigned int Camera::getHeight()
	{
		return height;
	}
	
	size_t Camera::getImageBufferSize()
	{
		return cam.getImageBufferSize();
	}