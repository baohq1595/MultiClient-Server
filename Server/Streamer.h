/*
 * Streamer.cpp
 *
 * Created on: Mar 3, 2017
 * Author: TrucNDT
*/

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
#include "CameraController.h"
#include "Mediator/utils.h"

class CameraController;

class Streamer
{
public:
    Streamer(string listenPort, CameraController *cameraController);

    void startStreaming();
    void Initialize();
    void connectionHdl();

private:
    CameraController* mCamController;
    fd_set readFDs;
    //fd_set writeFDs;
    string mListenPort;
    int mListenSock;
    int maxFD;
    boost::mutex mtxFDSet;

    static const int MAX_BACKLOG = 3;

    int newConnectionHdl();
    void *getInAddr(struct sockaddr *sockAddr);
    void sendStreamingData();
};

#endif // STREAMER_H
