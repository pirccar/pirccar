#pragma once
#include <stdio.h>
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
	
	void resolveHostname(std::string to);
	
	public:
	Socket(void); //default constructor
	Socket(bool isUdp, int port); //choose if this socket should use udp and specify a port
	~Socket(void); //destructor 
	
	void initialize(); //initialize local sockets
	int connect(std::string to); //connect to a remote socket(tcp), udp uses this to initialize the remote socket
	void waitForConnection(int port); //wait for a connection from a remote socket (tcp)
	void disconnect(); //disconnect the socket
	
	int write(const char* msg); //Sends a string and return number of chars sent
	char[] read(); //reads a string from a remote socket
};
