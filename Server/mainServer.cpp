#include "Server.h"

using namespace std;

map<string, string> commandlineArgument;
void printHelp();
int processCommandlineArgument(int argc, char *argv[]);

int main(int argc, char *argv[])
{

	processCommandlineArgument(argc, argv);
	Server server(commandlineArgument["server_port"], commandlineArgument["heartbeat_port"]);
	server.startServer();
	while(1)
		sleep(1000);

	return 0;
}

int processCommandlineArgument(int argc, char *argv[])
{
	int goRetVal;
	static struct option long_options[] =
	{
			{"server_port", required_argument, 0, 's'},
			{"heartbeat_port", required_argument, 0, 's'},
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
	cout << "\t-s" << endl;
	cout << "\t--server_port SERVER_TCP_PORT" << endl;
	cout << "\tDefault SERVER_TCP_PORT=12345" << endl << endl;
	cout << "\t-u" << endl;
	cout << "\t--heartbeat_port HEARTBEAT_UDP_PORT" << endl;
	cout << "\tDefault HEARTBEAT_UDP_PORT=54321" << endl << endl;
	cout << "\t-h" << endl;
	cout << "\t--help" << endl << endl;
}
