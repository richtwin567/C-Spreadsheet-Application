
// temp
#include <unistd.h>
#include <stdio.h>

#include <stdlib.h>
#include "server.h"


int main(int argc, char** argv)
{
	int portNo = 10000;
	const unsigned int clientCap = 10;

	if(argc == 2)
	{
		portNo = atoi(argv[1]);
	}

	// Client connection thread
	pthread_t connectionThread = {0};
	int threadError = 0;
		
	Server server = startServer(portNo, clientCap);


	if(server.state == SERVER_ACTIVE)
	{
		printf("[SERVER] Active...\n");
		
		pthread_mutex_init(&server.serverDataLock, NULL);
		pthread_mutex_init(&server.messageQueueLock, NULL);

		if((threadError = pthread_create(&connectionThread, NULL,
										 (void*)acceptClientsAsync,
										 &server)) == 0)
		{
			// TODO(afb) :: log success
			printf("[SERVER] Accepting clients...\n");
		}
		else
		{
			// TODO(afb) :: handle error
			printf("[SERVER] Failed to create thread: %d\n", threadError);
			return -1;
		}


		while(!shouldClose(server))
		{

			struct Command* msg = getNextMessage(server.messages);

			if(msg)
			{
				printf("%s\n", msg->input);
				// CommandOutput result = executeCommand(command);
			}
			else
			{
				printf("[SERVER] No messages to process\n");
			}

			printf("[SERVER] Connected clients: %d\n",
				   server.connectedClientsCount);
			
			
			fflush(stdout);

			// if(server.connectedClientsCount > 2)
			//  	break;
		}
	}
	else
	{
		// TODO(afb) :: log error
		printf("[SERVER] Failed to start.\n");
	}


	// TODO(afb) :: Cleanup resources
	printf("[SERVER] Closing...\n");

	close(server.socketNumber);
	return 0;
}
