#include "Server.h"

int main(int argc, char *argv[])
{
	Server *server = new Server(argv[1]);

	server->Initialize();
	server->startStreaming();
	while(1)
	{
		sleep(1000);
	}
	return 0;
}
