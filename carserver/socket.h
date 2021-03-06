#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

class Socket
{
	private:
	
	int localSocket; //socket for receiving data from a remote socket
	int remoteSocket; //socket for sending data to a remote socket
	struct sockaddr_in localAddr; //this sockets address
	struct sockaddr_in remoteAddr; //remote address
	unsigned short port; //the port
	bool isUdp; //is this socket using udp?
	bool outgoing; //is this socket sending?
	char* readBuffer;
	
	void resolveHostname(std::string to);
	
	public:
	Socket(void); //default constructor
	Socket(bool isUdp, bool outgoing, int port); //choose if this socket should use udp and specify a port
	~Socket(void); //destructor 
	
	void initialize(); //initialize local sockets
	int connect(std::string to); //connect to a remote socket(tcp), udp uses this to initialize the remote socket
	void waitForConnection(); //wait for a connection from a remote socket (tcp)
	void disconnect(); //disconnect the socket
	
	int write(const char* msg); //Sends a string and return number of chars sent
	int write(unsigned char* msg, int size);
	char* read(); //reads a string from a remote socket
};
