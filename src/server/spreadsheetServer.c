#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "server.h"
#include "functions.h"


int createServerMessage(struct ServerMessage* msg, enum Code code, int* version, struct Sheet* sheet)
{
	int result = 0;

	switch(code)
	{
		case OK:
		{
			msg->header.code = code;
			msg->header.sheetVersion = (*version)++;
			msg->sheet = *sheet;

			result = 1;
		}break;

		case ACKNOWLEDGED:
		{
			msg->header.code = code;
			msg->header.sheetVersion = *version;
			result = 1;
		}break;

		default:
		{
			
		}break;
	}

	return result;
}


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

	// NOTE(afb) :: Setting up spreadsheet
	
	if(server.state == SERVER_ACTIVE)
	{
		printf("[SERVER] Active...\n");

		server.spreadsheet.size = 9;
		getBlankSheet(&server.spreadsheet);

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


		struct ClientMessage cliMsg = {0};
		struct ServerMessage serverMsg = {0};
		char* packet = 0;
		
		while(!shouldClose(server))
		{
			pthread_mutex_lock(&(server.serverDataLock));
			int success = getNextMessage(server.messages, &cliMsg);
			pthread_mutex_unlock(&(server.serverDataLock));

			if(success)
			{
				printf("[SERVER] Messages to process\n");
				enum Code code;
				parseCommand(cliMsg.command,
							 &code,
							 &(server.spreadsheet));
				switch(code)
				{
					case OK:
					{
						if(createServerMessage(&serverMsg,
											   OK,
											   &server.sheetVersion,
											   &server.spreadsheet))
						{
							pthread_mutex_lock(&(server.serverDataLock));

							int packetLen = serializeServerMsg(serverMsg,
															   &packet);
					
							for(int i = 0, len = server.connectedClientsCount; i < len; i++)
							{
								int soc = server.connectedClientSockets[i];
								send(soc, packet, packetLen,0);
							}

							pthread_mutex_unlock(&(server.serverDataLock));
						}

					}break;

					default:
					{
						// TODO(afb) :: log input not supported
					}break;
				}

				free(cliMsg.command);
			}
			else
			{
				// NOTE(afb) :: No messages
				// printf("[SERVER] No messages to process\n");
			}

			// printf("[SERVER] Connected clients: %d\n", server.connectedClientsCount);
			
			fflush(stdout);

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
