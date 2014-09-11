#include <raspicam/raspicam.h>

class Camera
{
	private:
	unsigned char* buffer; //internal buffer for image
	int width, height; //size of image
	raspicam::RaspiCam cam; //the raspicam camera
	raspicam::RASPICAM_FORMAT format; //image format, RGB etc
	
	void allocateBuffer(); //allocates memory for the buffer
	
	public:
	Camera(void);
	~Camera(void);
	
	void initialize(); //intializes the camera with size and format, also opens the camera and makes it ready to capture
	void setFormat(raspicam::RASPICAM_FORMAT format); //sets the format, should be called before initialize
	void setWidthHeight(int width, int height); //sets width and height, should be called before initialize. TODO: split into separate functions
	
	unsigned int getWidth(); //get the width of the image
	unsigned int getHeight(); //get the height of the image
	
	size_t getImageBufferSize(); //get the size of the image buffer
	
	unsigned char* getBuffer(); //get the buffer, this will force the camera to get a new frame, Warning not thread safe!
};