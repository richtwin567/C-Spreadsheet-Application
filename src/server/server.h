#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

enum ServerState
{
	SERVER_ACTIVE,
	SERVER_INVALID,
};


// TODO(afb) :: Resizeable client capacity

typedef struct _Server
{
	int          socketNumber;

	int*         connectedClientSockets;
	unsigned int connectedClientsCount;    
	unsigned int maxClientCapacity;

	pthread_mutex_t messageQueueLock;
	
	// TODO(afb) :: Determine if this just needs to be a 'bool'
	// to decide if the sever should close or not. Consider
	// the normal way the server gets notified to close. What
	// should happen if everyone leaves etc.
	enum ServerState state;
}Server;


typedef struct _ClientMessageThread
{
	int socketNumber;
	pthread_mutex_t* lock;

	// TODO(afb) :: Add message queue
}ClientMessageThread;


void* handleClientMessages(void* args)
{
	// TODO(afb) :: Complete funcion

	// TODO(afb) :: Wait for messages

	// TODO(afb) :: Process messagge

	// TODO(afb) :: Add message to message queue
}

void acceptClientsAsync(void* args)
{
	Server* server = (Server*)args;
	
	// TODO(afb) :: Maybe need to move mutex to main function
	// so that it can access the message queue
	
	// Lock for using clients to use message queue


	ClientMessageThread* threadData = (ClientMessageThread*)calloc(server->maxClientCapacity, sizeof(ClientMessageThread));

	// Client message handler threads
	pthread_t* clientMessageHandler = (pthread_t*)calloc(server->maxClientCapacity, sizeof(pthread_t));

	struct sockaddr_in newClientAddress = {0};
	socklen_t newClientAddressSize = sizeof(newClientAddress);

	// Run until server is no longer active
	while(server->state == SERVER_ACTIVE)
	{
		int newClient = accept(server->socketNumber,
							   (struct sockaddr*)&newClientAddress,
							   &newClientAddressSize);

		ClientMessageThread* thData = &(threadData[server->connectedClientsCount]);
		thData->socketNumber = newClient;
		thData->lock = &server->messageQueueLock;
		
		if(newClient < 0)
		{
			// TODO(afb) :: handle error
			// Maybe quit. For now go to next loop
		}
		else
		{
			
			pthread_t* th = &(clientMessageHandler[server->connectedClientsCount++]);

			// TODO(afb) :: Send the additional needed data to handle
			// client
			// NOTE(afb) :: Create a new thread to process client
			// messages.
			int threadError = 0;
			if(!(threadError = pthread_create(th, NULL,
											  handleClientMessages,
											  thData)))
			{
				// TODO(afb) :: log sucess
				pthread_detach(*th);
			}
			else
			{
				// TODO(afb) :: log error
			}
		}
	
	}

	pthread_exit(NULL);
}



int shouldClose(Server server)
{
	// TODO(afb) :: Test to see if first client has left
	return server.state == SERVER_INVALID;
}


void closeServer(Server server)
{
	// TODO(afb) :: Dispatch message to clients for them to
	// close before closing
	shutdown(server.socketNumber, SHUT_RDWR);
	//close(server.socketNumber);
}


Server startServer(int portNumber, unsigned int maxClients)
{
	Server result = {0};
	result.maxClientCapacity = maxClients;
	result.state             = SERVER_ACTIVE;

	struct sockaddr_in serverAddress = {0};
	server.sin_family = AF_INET;
	server.sin_port   = htons(portNo);
	server.sin_addr.s_addr = INADDR_ANY;

	if((result.socketNumber = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// TODO(afb) :: log error
		result.state = SERVER_INVALID;
	}

	if(bind(result.socketNumber, (struct sockaddr*)&serverAddress,
			sizeof(serverAddress)) < 0)
	{
		// TODO(afb) :: log error
		result.state = SERVER_INVALID;
	}

	if(listen(result.socketNumber, maxClients) < 0)
	{
		// TODO(afb) :: log error
		result.state = SERVER_INVALID;
	}

	if(result.state == SERVER_ACTIVE)
	{
			result.connectedClients = (int*)calloc(maxClients,
												   sizeof(int));
	}

	return result;
}


#endif
