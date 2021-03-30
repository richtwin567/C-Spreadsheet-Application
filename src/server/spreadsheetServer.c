#include <stdlib.h>
#include "server.h"


int main(int argc, char** argv)
{
	int portNo = atoi(argv[1]);
	const unsigned int clientCap = 5;

	// Client connection thread
	pthread_t connectionThread = {0};
	int threadError = 0;
		
	Server server = startServer(portNo, clientCap);
	
	if(server.state == SERVER_ACTIVE)
	{
		pthread_mutex_init(&server.messageQueueLock, NULL);

		if(!(threadError = pthread_create(&connectionThread, NULL,
										 acceptClientsAsync, &server)))
		{
			// TODO(afb) :: log success
		}
		else
		{
			// TODO(afb) :: handle error
		}

		
		while(!shouldClose(server))
		{
			
			struct Command* msg = getNextMessage(server.messages);
			
			// CommandOutput result = executeCommand(command);
		}
	}
	else
	{
		// TODO(afb) :: log error
	}
}
