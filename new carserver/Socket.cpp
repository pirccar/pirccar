#include "socket.h"

Socket::Socket(void)
{
	isUdp = false;
	port = 8000;
	localSocket=0;
	remoteSocket=0;
}

Socket::Socket(bool isUdp, int port)
{
	this->isUdp = isUdp;
	this->port = port;
	localSocket=0;
	remoteSocket=0;
}

Socket::~Socket(void)
 {
	close(localSocket);
	close(remoteSocket);
 }
 
 void Socket::resolveHostname(std::string to)
 {
	struct hostent *he;
	struct in_addr **addr_list;
	 
	//resolve the hostname, its not an ip address
	if ( (he = gethostbyname( to.c_str() ) ) == NULL)
	{
		//gethostbyname failed
		herror("gethostbyname");
		printf("Failed to resolve hostname\n");
		return;
	}
	 
	//Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
	addr_list = (struct in_addr **) he->h_addr_list;

	for(int i = 0; addr_list[i] != NULL; i++)
	{
		remoteAddr.sin_addr = *addr_list[i];
		break;
	}
 }
 
 void Socket::initialize()
 {
	if(isUdp)
	{
		if((localSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			printf("localSocket fail \n");
			
		memset(&localAddr, 0, sizeof(localAddr));
		localAddr.sin_family = AF_INET;
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		localAddr.sin_port = htons(port);
		char yes = '1';
		
		if(bind(localSocket, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0)
			printf("bind error \n");
	}
	else
	{
		if((localSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			printf("localSocket fail \n");
			
		memset(&localAddr, 0, sizeof(localAddr));
		localAddr.sin_family = AF_INET;
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		localAddr.sin_port = htons(port);
		char yes = '1';
		
		//fixes binding error
		if(setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			printf("localSocket opt error \n");
		}
		
		if(bind(localSocket, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0)
			printf("bind error \n");
	
		if(listen(localSocket, 1) < 0)
			printf("listen error \n");
	}
 }
	
int Socket::connect(std::string to)
{	
	int ret = 0;
	if(isUdp)
	{
		remoteSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if(remoteSocket != 0)
		{
			printf("failed to create socket %d\n", remoteSocket);
			return remoteSocket;
		}
		memset(&remoteAddr, 0, sizeof(remoteAddr));
		remoteAddr.sin_family = AF_INET;
		remoteAddr.sin_addr.s_addr = inet_addr(to.c_str());
		remoteAddr.sin_port = htons(port);
	}
	else
	{
		remoteSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(remoteSocket != 0)
		{
			printf("Socket fail %d\n", remoteSocket);
			return remoteSocket;
		}
			
		if(inet_addr(to.c_str()) == -1)
		{
			resolveHostname(to);
		}
		else//plain ip address
		{
			remoteAddr.sin_addr.s_addr = inet_addr( to.c_str() );
		}
		 
		remoteAddr.sin_family = AF_INET;
		remoteAddr.sin_port = htons( port );
		 
		//Connect to remote server
		ret = ::connect(remoteSocket , (struct sockaddr *)&remoteAddr , sizeof(remoteAddr));
		if (ret != 0)
		{
			printf("connect failed. Error: %d", ret);
			return ret;
		}
	}
	
	return ret;
}

void Socket::waitForConnection()
{
	if(!isUdp)
	{
		unsigned int clientLen = sizeof(remoteAddr);
		if((remoteSocket = accept(localSocket, (sockaddr*) &remoteAddr, &clientLen)) < 0)
			printf("Accept failed \n");
	}
}
void Socket::disconnect()
 {
	close(remoteSocket);
 }

int Socket::write(const char* msg)
{
	int n = 0;
	int msgLen = strlen(msg);
	if(isUdp)
	{
		if((n = sendto(remoteSocket, msg, msgLen, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr))) != msgLen)
			printf("Error sending UDP\n");
	}
	else
	{
		printf("Msglen: %d \n", msgLen);
		if( (n = send(remoteSocket, msg, msgLen, 0)) < 0)
			printf("Error sending TCP\n");
	}
	return n;
}
char* Socket::read()
{
	char buffer[1024];
	if(isUdp)
	{
		if(recvfrom(remoteSocket, buffer, sizeof(buffer), 0, NULL, NULL) < 0)
			printf("recvfrom failed\n");
	}
	else
	{
		if(recv(remoteSocket, buffer, sizeof(buffer), 0) < 0)
			printf("recv failed\n");
	}
	return buffer;
}