#include "Server.h"

using namespace std;


Server::Server(string listenPort)
{
	mListenPort = listenPort;
	FD_ZERO(&readFDs);
	maxFD = -1;
	mListenSock = -1;
}

/*
 * Return socket info base on address family
 */
void *Server::getInAddr(struct sockaddr *sockAddr)
{
	if (sockAddr->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sockAddr)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sockAddr)->sin6_addr);
}

/*
 * Return socket info: readable ip address
 */
string Server::getIPAddrStr(struct sockaddr_storage remoteAddr)
{
	char remoteIP[INET6_ADDRSTRLEN];
	inet_ntop(remoteAddr.ss_family, getInAddr((struct sockaddr *)&remoteAddr), remoteIP, INET6_ADDRSTRLEN);
	string str(remoteIP);
	
	return str;
}

/*
 * Create server socket to listen to new connection
 */
void Server::Initialize()
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
		cout << "getaddrinfo failed: " << gai_strerror(rValue) << "in " << __FILE__ << __LINE__ << endl;
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

/*
 * Return socket value when a new client connects to server
 */
int Server::newConnectionHdl()
{
	int newFD;
	struct sockaddr_storage remoteAddr;
	unsigned int addrLen = sizeof remoteAddr;

	if ((newFD = accept(mListenSock, (struct sockaddr *)&remoteAddr, &addrLen)) == -1)
	{
		cout << "newConnectionHdl failed: " << __FILE__ << __LINE__ << endl;
		return -1;
	}

	mtxFDSet.lock();
	FD_SET(newFD, &readFDs);

	if (maxFD < newFD)
	{
		maxFD = newFD;
	}
	
	mtxFDSet.unlock();
	cout << "new connection from " << getIPAddrStr(remoteAddr) << endl;

	return newFD;
}

/*
 * Traverse through connected socket set to check incoming data and send back data
 * Run in a separate thread
 */
void Server::sendStreamingData()
{
	//prepare frame to send
	string gSignal = "Greeting from Server.\n";
	unsigned int strLeng = gSignal.length();
	fd_set tmpFDs;
	struct timeval tv;
	int tmpMaxFD, frameSize, bytesSent, totalFrames, bytesRead;
	char buffer[1024];
	unsigned int buffLeng = sizeof buffer;
	socklen_t sockLeng;
	struct sockaddr_storage remoteAddr;

	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	
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
		
		//loop through socket to check ready socket
		for (int i = 0; i <= maxFD; i++)
		{
			if (FD_ISSET(i, &tmpFDs))
			{
				if (i == mListenSock)
				{
					;
				}

				else
				{
					memset(buffer, 0, buffLeng);
					bytesRead = recv(i, buffer, buffLeng, 0);

					if (bytesRead <= 0)
					{
						cout << "client closed connection on socket " << i << endl;
						close(i);

						mtxFDSet.lock();
						FD_CLR(i, &readFDs);
						mtxFDSet.unlock();
					}
					else
					{
						string recvMesg(buffer, bytesRead);
						getpeername(i, (struct sockaddr *)&remoteAddr, &sockLeng);
						cout << "Message from " << getIPAddrStr(remoteAddr) << ": " << recvMesg << "\n";
						if ((bytesSent = send(i, gSignal.c_str(), strLeng, 0)) > 0)
						{
							cout << "Sent greeting to client.\n";
						}
					}
				}
			}
		}
	}
}

/*
 * Check if there is new connection
 * Run in a separate thread
 */
void Server::connectionHdl()
{
	fd_set tmpFDs;
	unsigned char buffer[1024];
	int bytesRead, newFD, rValue;
	maxFD = mListenSock;

	while(true)
	{
		tmpFDs = readFDs;
		rValue = select(maxFD + 1, &tmpFDs, NULL, NULL, NULL);
		
		if (rValue == -1)
		{
			cout << "select fd set failed: " << __FILE__ << __LINE__ << endl;
			return;
		}

		else
		{
			if (FD_ISSET(mListenSock, &tmpFDs))
			{
				newConnectionHdl();
			}
		}
	}
}


void Server::startStreaming()
{
	boost::thread tConnHdl(&Server::connectionHdl, this);
	boost::thread tServer(&Server::sendStreamingData, this);
}
