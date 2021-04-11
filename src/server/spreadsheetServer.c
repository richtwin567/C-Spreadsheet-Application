#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "server.h"
#include "functions.h"
/*
typedef struct _CommandOutput
{
	int x, y;
	char* value;

	int valid;
}CommandOutput;


typedef struct _ExcutionData
{
	char* operation;
	char* param1;
	char* param2;
}ExcutionData;

int isAlphaNumeric(char c)
{
	int result = ((c >= 'A' && c <= 'Z') ||
				  (c >= 'a' && c <= 'z') ||
				  (c >= '0' && c <= '9'));

	return result;
}

void eatWhitespace(char** str)
{
	while((str[0] != 0) &&
		  ((str[0][0] == ' ')  ||
		   (str[0][0] == '\n') ||
		   (str[0][0] == 'r'))) 
	{
		str[0]++;
	}
}

char* getNextToken(char** str)
{
	char* result = 0;
	eatWhitespace(str);
	if(str[0] != 0)
	{
		char* start = str[0];
		char* end = str[0];
		while(str[0] != 0 && isAlphaNumeric(str[0][0]))
		{
			end = str[0]++;
		}

		int size = (end-start) + 1;
		
		if(end == start)
		{
			str[0]++;

		}
	   
		result = malloc(size+1);		
		result = malloc(size+1);
		memcpy(result, start, size);
		result[size] = 0;
	}
	return result;
}

int strCompareCaseInsensitive(char* arg1, char* arg2)
{
	int result = 1;
	int len = strlen(arg1);

	if(len == strlen(arg2))
	{
		for(int i = 0; i < len; i++)
		{
			if(tolower(arg1[i]) != tolower(arg2[i]))
			{
				result = 0;
				break;
			}
		}
	}

	return result;
}

int commandIsFunction(struct Command* command, ExcutionData* excData)
{
	int result = 0;
	char* it = command->input;

	char* tk;
	if((tk = getNextToken(&it))[0] == '=')
	{
		free(tk);
		char* op = getNextToken(&it);
		
		if((tk = getNextToken(&it))[0] == '(')
		{
			free(tk);
			char* par1 = getNextToken(&it);
			tk = getNextToken(&it);
			char* par2 = getNextToken(&it);
			
			free(tk);
			if((tk = getNextToken(&it))[0] == ')')
			{
				free(tk);
				excData->param1 = par1;
				excData->param2 = par2;
				excData->operation = op;
				result = 1;
			}
		}
		else
		{
			free(tk);
		}
	}
	else
	{
		free(tk);
	}

	return result;
}

CommandOutput executeCommand(struct Command* command)
{
	CommandOutput result = {0};

	ExcutionData excData = {0};
	
	if(commandIsFunction(command, &excData))
	{
		char* op = excData.operation;

		if(strCompareCaseInsensitive(op, "average"))
		{
			printf("AVERAGE\n");
		}
		else if(strCompareCaseInsensitive(op, "sum"))
		{
			
		}
		else if(strCompareCaseInsensitive(op, "range"))
		{
			
		}
		else
		{
			// TODO(afb) :: log op not supported
			printf("NO SUPPORT\n");
		}

		free(op);
	}
	else
	{
		// TODO(adb) :: Handle value placement
		// TODO(afb) :: log invalid input
	}

	return result;
}

*/



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
	server.spreadsheet.size = 9;
	getBlankSheet(&server.spreadsheet);

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
				enum Code code;
				parseCommand(cliMsg.command,
							 &code,
							 &server.spreadsheet);

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
								write(soc, packet, packetLen);
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
