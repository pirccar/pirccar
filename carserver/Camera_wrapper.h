
class Camera_wrapper
{
	private:
	void compressImage();
	void run();
	int width;
	int height;
	int imageQuality;
	unsigned char* imageBuffer;
	CCamera* camera;
	pthread_t t1;
	
	bool execute;
	bool alive;
	
	public:
	Camera_wrapper(void);
	Camera_wrapper(int width, int height, int quality);
	~Camera_wrapper(void);
	
	void start();
	void stop();
	void dispose();
	
	void set_width(int width);
	void set_height(int height);
	void set_quality(int quality);
};