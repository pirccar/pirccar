
class Network
{
	private:
	void compressAndSend(unsigned char buffer[],int frameServerSocket, struct sockaddr_in frameServerAddr);
	void compressImage();
	void run();
	
	bool execute;
	int frameServerSocket;
	struct sockaddr_in frameServerAddr;
	unsigned short frameServerPort;
	
	public:
	Network(void);
	~Network(void);
	
	void start();
	void stop();
};