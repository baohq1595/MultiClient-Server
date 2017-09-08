#include "Server.h"

using namespace std;


/*
 * Constrcutor
 */
Server::Server(string tcpPort, string udpPort)
{
	tcpListenPort = tcpPort;
	udpListenPort = udpPort;
	FD_ZERO(&readFDs);
	maxFD = -1;
	tcpListenSock = -1;
	udpListenSock  = -1;
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
void Server::createTCPSocket()
{
	struct addrinfo hints, *ai, *p;
	int rValue;

	//get socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rValue = getaddrinfo(NULL, tcpListenPort.c_str(), &hints, &ai)) != 0)
	{
		cout << "getaddrinfo failed: " << gai_strerror(rValue) << "in " << __FILE__ << __LINE__ << endl;
		return;
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		tcpListenSock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (tcpListenSock < 0)
		{
			continue;
		}
		setsockopt(tcpListenSock, SOL_SOCKET, SO_REUSEADDR, &rValue, sizeof rValue);

		if (bind(tcpListenSock, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(tcpListenSock);
			continue;
		}
		break;
	}

	freeaddrinfo(ai);

	if (p == NULL)
	{
		cout << "bind socket failed: " << __FILE__ << __LINE__ << endl;
		return;
	}

	if (listen(tcpListenSock, MAX_BACKLOG) == -1)
	{
		cout << "listen on socket failed: " << __FILE__ << __LINE__ << endl;
		return;
	}

	FD_SET(tcpListenSock, &readFDs);
}

void Server::createUDPSocket()
{
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    cout << "Creating UDP socket...\n";

    if ((rv = getaddrinfo(NULL, udpListenPort.c_str(), &hints, &servinfo)) != 0)
    {
        cout << "createUDPSocket: getaddrinfo.\n";
        return;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((udpListenSock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            cout << "createUDPSocket: socket.\n";
            continue;
        }

        if (bind(udpListenSock, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(udpListenSock);
            cout << "createUDPSocket: bind.\n";
            continue;
        }

        break;
    }

	freeaddrinfo(servinfo);

    if (p == NULL)
    {
        cout << "createUDPSocket: failed to bind socket\n";
        return;
    }
}

/*
 * Return socket value when a new client connects to server
 */
int Server::newConnectionHdl()
{
	int newFD;
	struct sockaddr_storage remoteAddr;
	unsigned int addrLen = sizeof remoteAddr;

	if ((newFD = accept(tcpListenSock, (struct sockaddr *)&remoteAddr, &addrLen)) == -1)
	{
		cout << "newConnectionHdl failed: " << __FILE__ << __LINE__ << endl;
		return -1;
	}

	// mtxFDSet.lock();
	FD_SET(newFD, &readFDs);

	if (maxFD < newFD)
	{
		maxFD = newFD;
	}
	
	// mtxFDSet.unlock();
	cout << "new connection from " << getIPAddrStr(remoteAddr) << endl;

	return newFD;
}

/*
 * Traverse through connected socket set to check incoming data and send back data
 * Run in a separate thread
 */
void Server::tcpConnectionHdl()
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
		// mtxFDSet.lock();
		tmpMaxFD = maxFD;
		tmpFDs = readFDs;
		// mtxFDSet.unlock();

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
				if (i == tcpListenSock)
				{
					newConnectionHdl();
				}

				else
				{
					memset(buffer, 0, buffLeng);
					bytesRead = recv(i, buffer, buffLeng, 0);

					if (bytesRead <= 0)
					{
						cout << "client closed connection on socket " << i << endl;
						close(i);

						// mtxFDSet.lock();
						FD_CLR(i, &readFDs);
						// mtxFDSet.unlock();
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
 * Receive UDP packets, update clientList information
 */
 void Server::heartBeatHdl()
 {
	int bytesRead = 0, pcID;
	char buffer[HB_MESG_LENG];
	struct sockaddr_storage remote_addr;
	socklen_t addr_len;
	std::map<string, int>::iterator it;

	createUDPSocket();


	cout << "Waiting for HeartBeatClients...\n";

	while(1)
	{
		memset(buffer, 0, HB_MESG_LENG);
		if ((bytesRead = recvfrom(udpListenSock, buffer, HB_MESG_LENG, 0, (struct sockaddr *)&remote_addr, &addr_len)) < 0)
		{
			cout << "hbHandler: recvfrom.\n";
		}

		string ipString = getIPAddrStr(remote_addr);

		//Handle UDP packets
		cout << "UDP packet recv from " << ipString << endl;

		//Update clientList
		if ((it = clientList.find(ipString)) != clientList.end())
		{
			it->second = ALIVE_TIMEOUT;
		}
		else
		{
			int aliveTime = ALIVE_TIMEOUT;
			clientList.insert(pair<string, int>(ipString, aliveTime));
		}
	}
 }

 void Server::heartBeatChecker()
 {
	std::map<string, int>::iterator it;
	cout << "Start HearBeat checker\n";

	while(1)
	{
		bool isTimeout = false;

		//Check alive timeout of client
		for (it = clientList.begin(); it != clientList.end(); ++it)
		{
			//Remove disconnected client
			if(it->second <= 0)
			{
				clientList.erase(it->first);
			}

			//isTimeout is true if there is at least client disconnects
			isTimeout |= (it->second <= 0);

		}

		//Update - print out updated client list
		if(isTimeout)
		{
			// cout << "Update client list:\n";
			printClientList();
		}

		//sleep this thread for 1 sec
		sleep(1);
	}
 }

 void Server::printClientList()
 {
	struct tm *ptm;
	struct timeval tv;
	char time[40];

	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);
	strftime(time, sizeof time, "%Y-%m-%d %H:%M:%S", ptm);

	cout << "Current time: " << time << endl;
	cout << "Online clients:\n";
	std::map<string, int>::iterator it;

	for(it = clientList.begin(); it != clientList.end(); ++it)
	{
		cout << it->first << endl;
	}
 }

 void Server::startServer()
 {
	thread tcpConnectionHdlThread(&Server::tcpConnectionHdl, this);
	thread heartBeatHdlThread(&Server::heartBeatHdl, this);
	thread heartBeatCheckerThread(&Server::heartBeatChecker, this);
 }
