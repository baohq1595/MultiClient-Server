#ifndef SERVER_H
#define SERVER_H

#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <functional>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <map>
#include <iostream>

class Server
{
public:
	Server();
	Server(std::string tcpPort, std::string udpPort);

    void startServer();

private:
    fd_set readFDs;
    std::string tcpListenPort;
	std::string udpListenPort;
    int tcpListenSock;
	int udpListenSock;
    int maxFD;
    std::mutex mtxFDSet;
	std::map<std::string, int> clientList;	//string is for PC's ID (e.g IP address), int is for alive time

    static const int MAX_BACKLOG = 3;	//maximum accepted connections
	static const int HB_MESG_LENG = 3;	//heartbeat packet length 'H','E','Y'
	static const int ALIVE_TIMEOUT = 5;

	void createTCPSocket();
	void createUDPSocket();
    int newConnectionHdl();
    void *getInAddr(struct sockaddr *sockAddr);
	std::string getIPAddrStr(struct sockaddr_storage remoteAddr);
    void tcpConnectionHdl();
	void heartBeatHdl();
	void heartBeatChecker();
	void printClientList();
};

#endif // SERVER_H
