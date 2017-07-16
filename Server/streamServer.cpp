/*
 * streamServer.cpp
 *
 *  Created on: Mar 24, 2017
 *      Author: baohq
 */

#include "Streamer.h"

int main(int argc, char *argv[])
{
	CameraController *cam = new CameraController();
	Streamer *streamer = new Streamer(argv[1], cam);

	cam->enableCamera();
	cam->startCamera();

	streamer->Initialize();
	streamer->startStreaming();
	namedWindow("Test", 1);
	Mat frame;
	while(1)
	{
		sleep(100);
	}
	return 0;
}
