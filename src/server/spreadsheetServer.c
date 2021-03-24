#include <stdlib.gh>
#include "server.h"
#include "../interface/message.h"


int main(int argc, char** argv)
{
	int portNo = atoi(argv[1]);
	const unsigned int clientCap = 5;

	// Client connection thread
	pthread_t connectionThread = {0};
	int threadError = 0
		
	Server server = serverStart(portNo, clientCap);

	if(server.state == SERVER_ACTIVE)
	{
		if(!(threadError = pthread_create(&connectionThread, NULL,
										 acceptClientsAsync, &server)))
		{
			// TODO(afb) :: log success
		}
		else
		{
			// TODO(afb) :: handle error
		}

		
		while(!shouldClose())
		{
			updateServerState(server);
			
			struct ClientMessage msg = getNextMessage(server);
			
			CommandOutput result = executeCommand(msg.command);
		}
	}
	else
	{
		// TODO(afb) :: log error
	}
}
