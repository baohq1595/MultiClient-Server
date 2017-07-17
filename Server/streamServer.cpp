#include "Streamer.h"

int main(int argc, char *argv[])
{
	Streamer *streamer = new Streamer(argv[1]);

	streamer->Initialize();
	streamer->startStreaming();
	namedWindow("Test", 1);
	Mat frame;
	while(1)
	{
		sleep(1000);
	}
	return 0;
}
