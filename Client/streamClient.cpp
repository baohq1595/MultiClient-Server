#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	int sockfd, bytesRead, imgSize;
	struct addrinfo hints, *servInfo, *p;
	char recvBuffer[1024];
	char sendBuffer[1024];
	unsigned int buffLeng = sizeof recvBuffer;

	/*
	 * Connect to server
	 */
	if (argc < 3)
	{
		cout << "Server address and port is missing.\n" << "./streamClient IP_ADDR PORT" << endl;
		return -1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(argv[1], argv[2], &hints, &servInfo) != 0)
	{
		cout << "Could not resolve IP addr.\n";
		return -1;
	}

	for (p = servInfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = (socket(p->ai_family, p->ai_socktype, p->ai_protocol))) == -1)
		{
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		cout << "Connect failed.\n";
		return -1;
	}

	freeaddrinfo(servInfo);		//Done network stuffs

	///////////////////////////////////////////////////////
	while(1)
	{
		cout << "Enter message to send: ";
		cin >> sendBuffer;
		send(sockfd, sendBuffer(), buffLeng, 0);
		
		bytesRead = 0;
		if ((bytesRead = recv(sockfd, buffer, buffLeng, 0)) == -1)
		{
			cout << "Server closed.\n";
			return;
		}
		
		cout << "Message received from Server: " << string(recvBuffer, bytesRead) << endl;
	}
	return 0;
}
