#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

#include "../spreadsheet/spreadsheetData.h"
#include "../interface/message.h"

#define MESSAGE_CAPACITY 32



enum ServerState
{
	SERVER_ACTIVE,
	SERVER_INVALID
};

typedef struct _MessageQueue
{
	struct Command* messages[MESSAGE_CAPACITY];

	int first;
	int count;
}MessageQueue;


// TODO(afb) :: Resizeable client capacity
typedef struct _Server
{
	int          socketNumber;

	int*         connectedClientSockets;
	unsigned int connectedClientsCount;    
	unsigned int maxClientCapacity;

	pthread_mutex_t serverDataLock; // Used when modifying values in the server
	
	MessageQueue* messages;
	pthread_mutex_t messageQueueLock;
	
	// TODO(afb) :: Determine if this just needs to be a 'bool'
	// to decide if the sever should close or not. Consider
	// the normal way the server gets notified to close. What
	// should happen if everyone leaves etc.
	enum ServerState state;
}Server;


#define MAX_MESSAGE_LENGTH 255
typedef struct _ClientMessageThread
{
	int socketNumber;
	pthread_mutex_t* lock;
	MessageQueue* messageQueue;

	char procBuffer[MAX_MESSAGE_LENGTH];
}ClientMessageThread;


struct Command* getNextMessage(MessageQueue* messages)
{
	struct Command* result = NULL;

	if(messages->first != messages->count)
	{
		result = messages->messages[messages->first++];
		if(messages->first >= MESSAGE_CAPACITY)
			messages->first = 0;
	}

	return result;
}


// NOTE(afb) :: Will eat the first message if the buffer is full
// shouldn't cause any problems though since the messages shouldn't
// take long to process and pile up
void addMessage(MessageQueue* messages, struct Command* command)
{
	messages->messages[messages->count++] = command;
	if(messages->count >= MESSAGE_CAPACITY)
		messages->count = 0;
}


void* handleClientMessages(void* args)
{
	
	// TODO(afb) :: Complete funcion
	ClientMessageThread* data = (ClientMessageThread*)args;

	int quit = 0;
	
	// TODO(afb) :: Consider if its better to use read or rcv.
	while(!quit)
	{
		int error = read(data->socketNumber,
						 data->procBuffer,
						 MAX_MESSAGE_LENGTH);

		if(error < 0)
		{
			// TODO(afb) :: log error
		}
		else
		{
			
			
			struct ClientMessage result = {0};
			parseClientMsg(data->procBuffer, &result);
			// TODO(afb) :: Process messagge to see if valid
			
			pthread_mutex_lock(data->lock);
			addMessage(data->messageQueue, result.command);
			pthread_mutex_unlock(data->lock);
		}
	}
	
}


void acceptClientsAsync(void* args)
{
	Server* server = (Server*)args;
	
	ClientMessageThread* threadData = (ClientMessageThread*)calloc(server->maxClientCapacity, sizeof(ClientMessageThread));

	// Client message handler threads
	pthread_t* clientMessageHandler = (pthread_t*)calloc(server->maxClientCapacity, sizeof(pthread_t));

	struct sockaddr_in newClientAddress = {0};
	socklen_t newClientAddressSize = sizeof(newClientAddress);

	// Run until server is no longer active
	while(server->state == SERVER_ACTIVE)
	{
		printf("[SERVER] Started accepting clients...\n");
		
		int newClient = accept(server->socketNumber,
							   (struct sockaddr*)&newClientAddress,
							   &newClientAddressSize);

		ClientMessageThread* data = &(threadData[server->connectedClientsCount]);
		data->socketNumber = newClient;
		data->lock = &server->messageQueueLock;
		data->messageQueue = server->messages;
			
		if(newClient < 0)
		{
			// TODO(afb) :: handle error
			// Maybe quit. For now go to next loop
		}
		else
		{
			pthread_mutex_lock(&server->serverDataLock);
			pthread_t* th = &(clientMessageHandler[server->connectedClientsCount++]);
			pthread_mutex_unlock(&server->serverDataLock);
			
			// NOTE(afb) :: Create a new thread to process client
			// messages.
			int threadError = 0;
			if(!(threadError = pthread_create(th, NULL,
											  handleClientMessages,
											  data)))
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
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port   = htons(portNumber);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if((result.socketNumber = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// TODO(afb) :: log error
		result.state = SERVER_INVALID;
	}
	else
	{
		// NOTE(afb) :: Releases port after program ends.
		int true = 1;
		setsockopt(result.socketNumber, SOL_SOCKET, SO_REUSEADDR, &true,
				   sizeof(int));;
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
		result.connectedClientSockets = (int*)calloc(maxClients,
													 sizeof(int));
		
		result.messages = (MessageQueue*)malloc(sizeof(MessageQueue));
		memset(result.messages, 0, sizeof(MessageQueue));
	}

	return result;
}


#endif
