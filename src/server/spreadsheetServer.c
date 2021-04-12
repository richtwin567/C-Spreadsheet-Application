#include "functions.h"
#include "server.h"

#include <ctype.h>
#include <stdlib.h>

int createServerMessage(struct ServerMessage *msg, enum Code code, int *version, struct Sheet *sheet, int clientCount)
{
    int result = 0;

    switch (code)
    {
        case CONFLICT:
        case OK:
        {
            msg->header.code         = code;
            msg->header.clientCount  = clientCount;
            msg->header.sheetVersion = ++(*version);
            msg->sheet               = *sheet;
            msg->header.senderId     = 0;
            msg->message             = NULL;

            result = 1;
        }
        break;

        case COORD_NOT_FOUND:
        case IMPOSSIBLE:
        case BAD_SYNTAX:
        case NO_FUNCTION:
        case DISCONNECTED:
        case FORBIDDEN:
        case ACKNOWLEDGED:
        case SERVER_ERROR:
        {
            msg->header.code         = code;
            msg->header.clientCount  = clientCount;
            msg->header.sheetVersion = *version;
            msg->header.senderId     = 0;
            msg->message             = NULL;
            msg->sheet               = *sheet;
            result                   = 1;
        }
        break;

        default:
        {
            msg->header.code         = code;
            msg->header.clientCount  = clientCount;
            msg->header.sheetVersion = *version;
            msg->header.senderId     = 0;
            msg->message             = NULL;
            msg->sheet               = *sheet;
            result                   = 1;
        }
        break;
    }

    return result;
}

int main(int argc, char **argv)
{
    int portNo                   = 10000;
    const unsigned int clientCap = 10000;
	
    if (argc == 2)
    {
        portNo = atoi(argv[1]);
    }

    // Client connection thread
    pthread_t connectionThread = {0};
    int threadError            = 0;

    Server server = startServer(portNo, clientCap);

    if (server.state == SERVER_ACTIVE)
    {
        printf("[SERVER] Active...\n");
        // NOTE(afb) :: Setting up spreadsheet
        server.spreadsheet.size = 9;
        getBlankSheet(&server.spreadsheet);

        pthread_mutex_init(&server.serverDataLock, NULL);
        pthread_mutex_init(&server.messageQueueLock, NULL);

        if (!(threadError = pthread_create(&connectionThread, NULL,
                                          (void *)acceptClientsAsync,
                                          &server)))
        {
            printf("[SERVER] Accepting clients...\n");
			pthread_detach(connectionThread);
        }
        else
        {
            printf("[SERVER] Failed to create thread: %d\n", threadError);
            return -1;
        }

        struct ClientMessage cliMsg    = {0};
        struct ServerMessage serverMsg = {0};
        char *packet                   = 0;

        while (!shouldClose(server))
        {
            pthread_mutex_lock(&(server.serverDataLock));
            int success = getNextMessage(server.messages, &cliMsg);
            

            if (success)
            {
                enum Code code;
                parseCommand(cliMsg.command,
                             &code,
                             &server.spreadsheet);

                if (code == OK && cliMsg.header.sheetVersion < server.sheetVersion)
                {
                    code = CONFLICT;
                }

                if (createServerMessage(&serverMsg,
                                        code,
                                        &server.sheetVersion,
                                        &server.spreadsheet, server.connectedClientsCount))
                {
                    int packetLen = serializeServerMsg(serverMsg,
                                                       &packet);

                    for (int i = 0, len = server.connectedClientsCount; i < len; i++)
                    {
                        int soc = server.connectedClientSockets[i];
                        if (cliMsg.header.senderId != soc)
                        {
                            serverMsg.header.code = UPDATE;
                        }
                        else
                        {
                            serverMsg.header.code = code;
                        }

                        int packetLen = serializeServerMsg(serverMsg,
                                                           &packet);
                        send(soc, packet, packetLen, 0);
                    }
                }

                free(cliMsg.command);
            }
            else
            {
                // NOTE(afb) :: No messages
                // printf("[SERVER] No messages to process\n");
            }

            // printf("[SERVER] Connected clients: %d\n", server.connectedClientsCount);

			pthread_mutex_unlock(&(server.serverDataLock));
			
            fflush(stdout);
        }
    }
    else
    {
        printf("[SERVER] Failed to start.\n");
    }

    // TODO(afb) :: Cleanup resources
    printf("[SERVER] Closing...\n");

    close(server.socketNumber);

	printf("[SERVER] Closed\n");
    return 0;
}
