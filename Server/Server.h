#ifndef STREAMER_H
#define STREAMER_H

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

class Server
{
public:
    Server(std::string listenPort);

    void startStreaming();
    void Initialize();
    void connectionHdl();

private:
    fd_set readFDs;
    std::string mListenPort;
    int mListenSock;
    int maxFD;
    boost::mutex mtxFDSet;

    static const int MAX_BACKLOG = 3;

    int newConnectionHdl();
    void *getInAddr(struct sockaddr *sockAddr);
	std::string getIPAddrStr(struct sockaddr_storage remoteAddr);
    void sendStreamingData();
};

#endif // STREAMER_H
