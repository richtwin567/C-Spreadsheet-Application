#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum ServerState
{
	SERVER_ACTIVE,
	SERVER_INVLAID,
};


// TODO(afb) :: Resizeable client capacity

typedef struct _Server
{
	int          socketNumber;

	int*         connectedClientSockets;
	unsigned int connectedClientsCount;    
	unsigned int maxClientCapacity;
	
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

Server startServer(int portNumber, unsigned int maxClients);

int shouldClose(Server server);


void* handleClientMessages(void* args)
{
	// TODO(afb) :: Complete funcion
}

void acceptClientsAsync(void* args)
{
	Server* server = (Server*)args;
	
	// Lock for using clients to use message queue
	pthread_mutex_t clientMessageLock;
	pthread_mutex_init(&clientMessageLock, NULL);

	ClientMessageThread* threadData =
		(ClientMessageThread*)calloc(server->maxClientCapacity, sizeof(ClientMessageThread));

	// Client message handler threads
	pthread_t* clientMessageHandler =
		(pthread_t*)calloc(server->maxClientCapacity, sizeof(pthread_t));

	// Run until server is no longer active
	while(server->state == SERVER_ACTIVE)
	{
		int newClient = accept(server->socketNumber,
							   (struct sockaddr*)&newClientAddress,
							   sizeof(newClientAddress));

		ClientMessageThread* thData = &(threadData[server->connectedClientsCount]);
		thData->socketNumber = newClient;
		thData->lock = &clientMessageLock;
		
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
			if(!(threadError = pthread_create(th, NULL,
											  handleClientMessages,
											  thData)))
			{
				// TODO(afb) :: log sucess
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


Server startServer(int portNumber, unsigned int maxClients);
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
