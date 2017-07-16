/*
 * Test_streamer.cpp
 *
 *  Created on: Mar 18, 2017
 *      Author: baohq
 */

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
	int sockfd, bytesRead, bytesReadTotal, imgSize;
	struct addrinfo hints, *servInfo, *p;
	Mat frame;
	frame = Mat::zeros(480, 640, CV_8UC3);
	imgSize = frame.total() * frame.elemSize();

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
	namedWindow("stream", 1);
	while(1)
	{
		bytesReadTotal = 0;
		bytesRead = 0;
		do
		{
			if ((bytesRead = recv(sockfd, frame.data + bytesReadTotal, imgSize - bytesReadTotal, 0)) == -1)
			{
				cout << "Server closed.\n";
			}

			bytesReadTotal += bytesRead;
		}
		while(bytesReadTotal < imgSize);

		if (!frame.isContinuous())
		{
			frame = frame.clone();
		}

		imshow("stream", frame);

		if (waitKey(10) == 'q')
		{
			break;
		}
	}
	return 0;
}
