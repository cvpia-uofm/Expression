#ifndef SERVER_H
#define SERVER_H
#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <vector>
typedef unsigned char uchar;

//#pragma comment(lib,"ws2_32")

class Server
{ 
private:
	SOCKET server;
	WSADATA wsaData;
	sockaddr_in local;
	SOCKET client;
	int port;
	bool serverTerminationRequested;
	std::string strayData;
	float sensorValue;

public:
	// Creates a new server
	Server(int portAddress);
	// Attempts to create a non-blocking socket
	bool initialize();
	// Waits for client request. If succeed
	// returns zero. Otherwise, it returns
	// a status indicating in which step it
	// failed. See code for status codes.
	bool waitForConnection();
	// Sends a string of arbitrary length
	bool sendAny(std::string anyText);
	// Receives a short (256 char max) string
	bool receiveShortStr(std::string &receivedData);
	// Receives a "newline" terminated string.
	// Returns false if there is no newline character
	bool receiveAnyLine(std::string &receivedData);
	inline bool isClosingRequested(){return serverTerminationRequested;};
	// Closes a server
	void closeServer();
	float getSensorValue();
};

#endif