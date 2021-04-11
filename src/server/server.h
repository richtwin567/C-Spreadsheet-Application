#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

#include "../spreadsheet/spreadsheetData.h"
#include "../interface/message.h"
#include <unistd.h>

#define MESSAGE_CAPACITY 32



enum ServerState
{
	SERVER_ACTIVE,
	SERVER_INVALID
};

typedef struct _MessageQueue
{
	struct ClientMessage messages[MESSAGE_CAPACITY];

	int first;
	int count;
}MessageQueue;


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
	struct Sheet spreadsheet;
	int sheetVersion;
	
	enum ServerState state;
}Server;

#define HEADER_SIZE 20
#define MAX_MESSAGE_LENGTH 255

typedef struct _ClientMessageThread
{
	int socketNumber;
	pthread_mutex_t* lock;
	MessageQueue* messageQueue;

	Server* server;
	char messageHeader[HEADER_SIZE];
}ClientMessageThread;


void addClientMessage(MessageQueue* messages, struct ClientMessage msg)
{
	messages->messages[messages->count++] = msg;
	if(messages->count >= MESSAGE_CAPACITY)
		messages->count = 0;
}

int getNextMessage(MessageQueue* messages, struct ClientMessage* msg)
{
	int result = 0;

	if(messages->first != messages->count)
	{
		*msg = messages->messages[messages->first++];
		result = 1;
		if(messages->first >= MESSAGE_CAPACITY)
			messages->first = 0;
	}

	return result;
}


// NOTE(afb) :: Will eat the first message if the buffer is full
// shouldn't cause any problems though since the messages shouldn't
// take long to process and pile up

void closeServer(Server* server)
{
	// TODO(afb) :: Dispatch message to clients for them to
	// close before closing
	struct ServerMessage msg = {0};
	msg.header.code = DISCONNECTED;
	//msg.header.sheetVersion = server->sheetVersion;
	msg.header.sheetVersion = server->sheetVersion;
	//msg.sheet = server->spreadsheet;

	char* packet = 0;
	int msgLen = serializeServerMsg(msg, &packet);

	pthread_mutex_lock(&(server->serverDataLock));
	
	for(int i = 0, len = server->connectedClientsCount; i < len; i++)
	{
		int cli = server->connectedClientSockets[i];
		send(cli, packet, msgLen,0);
		server->connectedClientsCount--;
	}

	pthread_mutex_unlock(&(server->serverDataLock));
	shutdown(server->socketNumber, SHUT_RDWR);

	server->state = SERVER_INVALID;
	// TODO(afb) :: save spreadsheet
	// TODO(afb) :: cleanup resources
	//close(server.socketNumber);
}


void disconnectClient(Server* server, int clientSocket)
{
	for(int i = 0, len = server->connectedClientsCount; i < len; i++)
	{
		if(clientSocket == server->connectedClientSockets[i])
		{
			if(i == 0)
			{
				closeServer(server);
				return;
			}

			struct ServerMessage msg = {0};
			msg.header.code = DISCONNECTED;
			msg.header.sheetVersion = server->sheetVersion;
			msg.sheet = server->spreadsheet;
			char* packet = 0;
			
			int msgLen = serializeServerMsg(msg, &packet);
			send(clientSocket, packet, msgLen,0);
			
			server->connectedClientSockets[i] =
				server->connectedClientSockets[server->connectedClientsCount-1];

			server->connectedClientsCount--;
		}
	}
}

// TODO(afb) :: Remove. May not be necessary
void sendMessage(int clientSocket, struct ServerMessage msg, char** packet)
{
	int msgLen = serializeServerMsg(msg, packet);
	send(clientSocket, *packet, msgLen,0);
}

void* handleClientMessages(void* args)
{
	// TODO(afb) :: Complete funcion
	// Closing client handler
	
	ClientMessageThread* data = (ClientMessageThread*)args;
	Server* server = data->server;
	
	int quit = 0;

	printf("[SERVER] New client. (%d)\n", data->socketNumber);

	// NOTE(afb) :: sending acknowledgement
	struct ServerMessage ackMsg;

	ackMsg.header.code         = ACKNOWLEDGED;
	ackMsg.header.sheetVersion = server->sheetVersion;
	ackMsg.sheet               = server->spreadsheet;

	ackMsg.message = (char*)malloc(countDigits(data->socketNumber)+1);
	sprintf(ackMsg.message, "%d", data->socketNumber);

	char* packet = malloc(1);
	int msgLen = serializeServerMsg(ackMsg, &packet);
	send(data->socketNumber, packet, msgLen,0);
	free(packet);

	
	struct ClientMessage message = {0};
	char* msg = malloc(1);
	char* completeMsg = malloc(1);
	
	// TODO(afb) :: Consider if its better to use read or recv.
	while(!quit)
	{
		// recieve header
		int error = read(data->socketNumber,
						 data->messageHeader,
						 HEADER_SIZE);
		if(error <= 0)
		{
			// NOTE(afb) :: Client broken connection
			pthread_mutex_lock(&(server->serverDataLock));

			disconnectClient(server, data->socketNumber);
			
			pthread_mutex_unlock(&(server->serverDataLock));

			quit = 1;
		}
		else
		{
			int msgSize = getPayloadLength(data->messageHeader);
			msg  = realloc(msg, msgSize);
			memset(msg, 0, msgSize);

			// recieve payload
			error = read(data->socketNumber,
						 msg,
						 msgSize);

			if(error <= 0)
			{
				// TODO(afb) :: log error
				pthread_mutex_lock(&(server->serverDataLock));

				disconnectClient(server, data->socketNumber);
			
				pthread_mutex_unlock(&(server->serverDataLock));

				quit = 1;
			}
			else
			{			  
				completeMsg = realloc(completeMsg, HEADER_SIZE + msgSize);
				memset(completeMsg, 0, HEADER_SIZE + msgSize);
				
				completeMsg = strcat(completeMsg,
									  data->messageHeader);
				
				completeMsg = strcat(completeMsg, msg);
				
				parseClientMsg(completeMsg, &message);

				switch(message.header.code)
				{
					case REQUEST:
					{
						struct ClientMessage storedMsg;
						storedMsg.header = message.header;
						
						storedMsg.command = (struct Command*)malloc(sizeof(struct Command));
						memset(storedMsg.command, 0, sizeof(struct Command));
						
						storedMsg.command->coords = message.command->coords;
						storedMsg.command->input = (char*)malloc(strlen(message.command->input)+1);

						strcpy(storedMsg.command->input,
							   message.command->input);

						pthread_mutex_lock(&(server->serverDataLock));
						addClientMessage(data->messageQueue, storedMsg);
						pthread_mutex_unlock(&(server->serverDataLock));
					}break;

					case SAVE:
					{
						// TODO(afb) :: Save spreadsheet
					}break;

					case DISCONNECTED:
					{
						disconnectClient(server, data->socketNumber);
						quit = 1;
					}break;
					
					default:
					{
						// NOTE(afb) :: Unrecognised message.
					}break;
				}
				
			} // payload error checking

		} // header error checking

	} // while loop

	return NULL;
}


void* acceptClientsAsync(void* args)
{
	Server* server = (Server*)args;
	
	ClientMessageThread* threadData = (ClientMessageThread*)calloc(server->maxClientCapacity, sizeof(ClientMessageThread));

	
	// Client message handler threads
	pthread_t* clientMessageHandler = (pthread_t*)calloc(server->maxClientCapacity,
														 sizeof(pthread_t));

	struct sockaddr_in newClientAddress = {0};
	socklen_t newClientAddressSize = sizeof(newClientAddress);

	// Run until server is no longer active
	printf("[SERVER] Started accepting clients...\n");
	while(server->state == SERVER_ACTIVE)
	{

		if(server->connectedClientsCount >= server->maxClientCapacity)
		{
			server->maxClientCapacity *= 2;
			void* resultBuffer = realloc(server->connectedClientSockets, server->maxClientCapacity * sizeof(int));

			pthread_t* buff2 = realloc(clientMessageHandler,server->maxClientCapacity*sizeof(pthread_t));
			
			ClientMessageThread* buff3 = realloc(threadData, server->maxClientCapacity*sizeof(buff3));

			if(resultBuffer == NULL)
			{
				// TODO(afb) :: log error.
				server->state = SERVER_INVALID;
				break;
			}
			else
			{
				server->connectedClientSockets = (int*)resultBuffer;
			}

			if(buff2!=NULL)
			{
				clientMessageHandler = buff2;
			}

			if(buff3!=NULL)
			{
				threadData = buff3;
			}
		}
		
		int newClient = accept(server->socketNumber,
							   (struct sockaddr*)&newClientAddress,
							   &newClientAddressSize);
		
		

		ClientMessageThread* data = &(threadData[server->connectedClientsCount]);
		data->socketNumber = newClient;
		data->lock = &(server->messageQueueLock);
		data->messageQueue = server->messages;
		data->server = server;

		if(newClient < 0)
		{
			// TODO(afb) :: handle error
			// Maybe quit. For now go to next loop
		}
		else
		{
			server->connectedClientSockets[server->connectedClientsCount] = newClient;
			pthread_mutex_lock(&server->serverDataLock);
			pthread_t* clientThread = &(clientMessageHandler[server->connectedClientsCount++]);
			pthread_mutex_unlock(&server->serverDataLock);
			
			// NOTE(afb) :: Create a new thread to process client
			// messages.
			int threadError = 0;
			if(!(threadError = pthread_create(clientThread, NULL,
											  handleClientMessages,
											  data)))
			{
				// TODO(afb) :: log sucess
				printf("[SERVER] New client thread created.\n");
				pthread_detach(*clientThread);
			}
			else
			{
				// TODO(afb) :: log error
				printf("[SERVER] Failed to create new client thread.\n");
			}
		}

		memset(&newClientAddress, 0, sizeof(newClientAddress));
	}
	
	pthread_exit(NULL);
}



int shouldClose(Server server)
{
	// TODO(afb) :: Test to see if first client has left
	return server.state == SERVER_INVALID;
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
