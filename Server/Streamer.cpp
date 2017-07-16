/*
 * Streamer.cpp
 *
 * Created on: Mar 3, 2017
 * Author: TrucNDT
*/

#include "Streamer.h"

using namespace std;


Streamer::Streamer(string listenPort, CameraController *cameraController)
{
	mListenPort = listenPort;
	mCamController = cameraController;
//	mCamController->enableCamera();
//	mCamController->startCamera();
	FD_ZERO(&readFDs);
//	FD_ZERO(&writeFDs);
}

void *Streamer::getInAddr(struct sockaddr *sockAddr)
{
	if (sockAddr->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sockAddr)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sockAddr)->sin6_addr);
}

void Streamer::Initialize()
{
	struct addrinfo hints, *ai, *p;
	int rValue;

	//get socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rValue = getaddrinfo(NULL, mListenPort.c_str(), &hints, &ai)) != 0)
	{
		cout << "getaddrinfo failed: " << gai_strerror(rValue) << " in " << __FILE__ << __LINE__ << endl;
		return;
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		mListenSock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (mListenSock < 0)
		{
			continue;
		}
		setsockopt(mListenSock, SOL_SOCKET, SO_REUSEADDR, &rValue, sizeof rValue);

		if (bind(mListenSock, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(mListenSock);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		cout << "bind socket failed: " << __FILE__ << __LINE__ << endl;
		return;
	}

	freeaddrinfo(ai);

	if (listen(mListenSock, MAX_BACKLOG) == -1)
	{
		cout << "listen on socket failed: " << __FILE__ << __LINE__ << endl;
		return;
	}

	FD_SET(mListenSock, &readFDs);
}

int Streamer::newConnectionHdl()
{
	int newFD;
	char remoteIP[INET6_ADDRSTRLEN];
	struct sockaddr_storage remoteAddr;
	unsigned int addrLen = sizeof remoteAddr;

	if ((newFD = accept(mListenSock, (struct sockaddr *)&remoteAddr, &addrLen)) == -1)
	{
		cout << "newConnectionHdl failed: " << __FILE__ << __LINE__ << endl;
		return -1;
	}

	mtxFDSet.lock();
	FD_SET(newFD, &readFDs);
	//FD_SET(newFD, &writeFDs);

	if (maxFD < newFD)
	{
		maxFD = newFD;
	}
	mtxFDSet.unlock();
	cout << "new connection from " << inet_ntop(remoteAddr.ss_family, getInAddr((struct sockaddr *)&remoteAddr), remoteIP, newFD) << endl;

	return newFD;

}

void Streamer::sendStreamingData()
{
	//prepare frame to send
	fd_set tmpFDs;
	Mat frame;
	uchar *fPtr;
	struct timeval tv;
	int tmpMaxFD, frameSize, bytesSent, totalFrames;

	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	frameSize = frame.total() * frame.elemSize();
	fPtr = frame.data;
	while(1)
	{
		mtxFDSet.lock();
		tmpMaxFD = maxFD;
		tmpFDs = readFDs;
		mtxFDSet.unlock();

		if (select(tmpMaxFD + 1, NULL, &tmpFDs, NULL, &tv) == -1)
		{
			cout << "select fd set failed: " << __FILE__ << __LINE__ << endl;
			return;
		}

		mCamController->getNewFrame().copyTo(frame);
		frameSize = frame.total() * frame.elemSize();
		totalFrames++;
		cout << "Frame nth: " << totalFrames << endl;
		for (int i = 0; i <= tmpMaxFD; i++)
		{
			if (FD_ISSET(i, &tmpFDs))
			{
				if (i == mListenSock)
				{
					continue;
				}

				if ((bytesSent = send(i, frame.data, frameSize, 0)) <= 0)
				{
					continue;
				}
			}
		}
	}
}

void Streamer::connectionHdl()
{
	fd_set tmpFDs;
	unsigned char buffer[1024];
	int bytesRead;
	int newFD;
	maxFD = mListenSock;

	while(true)
	{
		tmpFDs = readFDs;

		if (select(maxFD + 1, &tmpFDs, NULL, NULL, NULL) == -1)
		{
			cout << "select fd set failed: " << __FILE__ << __LINE__ << endl;
			return;
		}

		//loop through socket to check ready socket
		for (int i = 0; i <= maxFD; i++)
		{
			if (FD_ISSET(i, &tmpFDs))
			{
				if (i == mListenSock)
				{
					newConnectionHdl();
				}

				else
				{
					if ((bytesRead = recv(i, buffer, sizeof buffer, 0)) <= 0)
					{
						cout << "client closed connection on socket " << i << endl;
						close(i);

						mtxFDSet.lock();
						FD_CLR(i, &readFDs);
						//FD_CLR(i, &writeFDs);
						mtxFDSet.unlock();
					}
				}
			}
		}
	}
}

void Streamer::startStreaming()
{
	boost::thread tConnHdl(&Streamer::connectionHdl, this);
	boost::thread tStreamer(&Streamer::sendStreamingData, this);
}
