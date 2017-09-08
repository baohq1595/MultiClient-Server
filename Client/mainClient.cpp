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
#include <getopt.h>
#include <thread>

#include <arpa/inet.h>

using namespace cv;
using namespace std;

map<string, string> commandlineArgument;

int processCommandlineArgument(int argc, char *argv[]);
void connectToTCPServer();
void connectToHeartBeatServer();
void printHelp();
void start();

int main(int argc, char** argv)
{
	processCommandlineArgument(argc, argv);
	start();
	while(1)
	{
		sleep(1000);
	}
	// int sockfd, bytesRead, imgSize;
	// struct addrinfo hints, *servInfo, *p;
	// char recvBuffer[1024];
	// char sendBuffer[1024];
	// unsigned int buffLeng = sizeof recvBuffer;
	
	return 0;
}

int processCommandlineArgument(int argc, char *argv[])
{
	int goRetVal;
	static struct option long_options[] =
	{
		{"server_ip", required_argument, 0, 'i'},
		{"server_port", required_argument, 0, 's'},
		{"heartbeat_port", required_argument, 0, 'u'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	while(1)
	{
		int option_index;
		goRetVal = getopt_long(argc, argv, "s:h:", long_options, &option_index);

		if (goRetVal == -1)
		{
			break;
		}

		switch(goRetVal)
		{
			case 'i':
				commandlineArgument["server_ip"] = optarg;
				break;
			case 's':
				commandlineArgument["server_port"] = optarg;
				break;
			case 'u':
				commandlineArgument["heartbeat_port"] = optarg;
				break;
			case 'h':
				printHelp();
				exit(1);
			default:
				printHelp();
				break;
		}
	}

	 /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

	//Default value
	if (commandlineArgument.find("server_ip") == commandlineArgument.end())
	{
		commandlineArgument["server_ip"] = "127.0.0.1";
	}

	if (commandlineArgument.find("server_port") == commandlineArgument.end())
	{
		commandlineArgument["server_port"] = "12345";
	}

	if (commandlineArgument.find("heartbeat_port") == commandlineArgument.end())
	{
		commandlineArgument["heartbeat_port"] = "54321";
	}

	return 0;
}

void printHelp()
{
	cout << "Usage:" << endl;
	cout << "\t-i" << endl;
	cout << "\t--server_port SERVER_IP" << endl;
	cout << "\tDefault SERVER_IP=127.0.0.1" << endl << endl;
	cout << "\t-s" << endl;
	cout << "\t--server_port SERVER_TCP_PORT" << endl;
	cout << "\tDefault SERVER_TCP_PORT=12345" << endl << endl;
	cout << "\t-u" << endl;
	cout << "\t--heartbeat_port HEARTBEAT_UDP_PORT" << endl;
	cout << "\tDefault HEARTBEAT_UDP_PORT=54321" << endl << endl;
	cout << "\t-h" << endl;
	cout << "\t--help" << endl << endl;
}

void connectToTCPServer()
{
	int sockfd;
	struct addrinfo hints, *servInfo, *p;
	char recvBuffer[1024];
	char sendBuffer[1024];
	unsigned int buffLeng = sizeof recvBuffer;

	/*
	 * Connect to server
	 */

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(commandlineArgument["server_ip"].c_str(), commandlineArgument["server_port"].c_str(), &hints, &servInfo) != 0)
	{
		cout << "Could not resolve IP addr.\n";
		return;
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

	freeaddrinfo(servInfo);

	if (p == NULL)
	{
		cout << "Connect failed.\n";
		return;
	}	//Done network stuffs

	if (sockfd == -1)
	{
		cout << "Connect to TCP server failed.\n";
		return;
	}
	///////////////////////////////////////////////////////
	while(1)
	{
		memset(sendBuffer, 0, buffLeng);
		memset(recvBuffer, 0, buffLeng);
		cout << "Enter message to send: ";
		cin >> sendBuffer;
		cout << "Input..." << sendBuffer << endl;
		send(sockfd, sendBuffer, buffLeng, 0);
		
		int bytesRead = 0;
		if ((bytesRead = recv(sockfd, recvBuffer, buffLeng, 0)) == -1)
		{
			cout << "Server closed.\n";
			return;
		}
		
		cout << "Message received from Server: " << string(recvBuffer, bytesRead) << endl;
	}
}

void connectToHeartBeatServer()
{
	char toSend[] = "hey";
	int sockfd;
	struct addrinfo hints, *servInfo, *p;

	/*
	 * Connect to server
	 */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(commandlineArgument["server_ip"].c_str(), commandlineArgument["heartbeat_port"].c_str(), &hints, &servInfo) != 0)
	{
		cout << "Could not resolve IP addr.\n";
		return;
	}

	for (p = servInfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = (socket(p->ai_family, p->ai_socktype, p->ai_protocol))) == -1)
		{
			cout << "Error create socket to Server.\n";
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		cout << "Connect failed.\n";
		return;
	}

	freeaddrinfo(servInfo);

	while(1)
	{
		if (sendto(sockfd, toSend, sizeof toSend, 0, p->ai_addr, p->ai_addrlen) == -1)
		{
			cout << "Send to HBServer failed.\n";
		}
		cout << "HeartBeat Sent.\n";
		sleep(1);
	}
}

void start()
{
	thread heartbeatThread(&connectToHeartBeatServer);
	thread tcpServerThread(&connectToTCPServer);
}