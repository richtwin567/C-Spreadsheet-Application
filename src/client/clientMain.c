/**
 * @file clientMain.c
 * @author 
 * @brief The driver for the client
 * @version 0.1
 * @date 2021-03-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "dialog.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// GLOBALS TO BE USED IN BOTH THREADS AND IN SIGNALS //

int shouldMainWait  = 1;  // whether the main thread should wait until an event occurs in the child thread
int shouldChildWait = 0;  // whether the child thread should wait until an event occurs in the main thread
int bufferIsDirty   = 0;  // whether there is a pending message for the user
int sock            = -1; // the socket file descriptor for connecting to the server
pthread_t thread;         // a thread that will handle incoming server messages
pthread_t mainThread;     // the main thread which handles user input
int CID;                  // The client's ID assigned by the server

// END GLOBALS //

// FUNCTIONS TO HANDLE PROGRAM EXIT //

/**
 * @brief Send an exit signal to main thread    
 * 
 * @param msg An error message to print before exit
 */
void sendExitSignal(char *msg, int sig)
{
    printErrorMsg(msg, NULL);
    pthread_kill(mainThread, sig);
    // wait for signal to be handled
    while (1)
        ;
}

/**
 * @brief Exits a thread leaving the cleanup to the main thread
 * 
 * @param sig 
 * @param info 
 * @param ucontext 
 */
void exitThread(int sig, siginfo_t *info, void *ucontext)
{
    if (pthread_self() == thread)
    {
        pthread_exit(NULL);
    }
}

/**
 * @brief Exit program after killing the thread and closing the socket
 *
 * @param code exit code 
 */
void exitProgram(int code)
{
    if (pthread_self() == mainThread)
    {
        //send disconnect message
        struct ClientMessage disconnect;
        disconnect.command             = NULL;
        disconnect.header.code         = DISCONNECTED;
        disconnect.header.senderId     = CID;
        disconnect.header.sheetVersion = -9999;
        char *packet                   = malloc(1);
        int length                     = serializeClientMsg(disconnect, &packet);
        send(sock, packet, length, 0);

        //exit
        pthread_kill(thread, SIGUSR2);
        close(sock);
        pthread_join(thread, NULL);
        exit(code);
    }
}

/**
 * @brief Exit program on signal
 * 
 * @param sig 
 * @param info 
 * @param ucontext 
 */
void exitOnSignal(int sig, siginfo_t *info, void *ucontext)
{
    if (sig == SIGUSR1)
    {
        printErrorMsg("Failed to start", NULL);
    }
    exitProgram(sig);
}

// END OF EXIT FUNCTIONS //

// FUNCTIONS TO HANDLE RECEIVING SERVER MESSAGES //

/**
 * @brief Receives a message through the socket
 * 
 * @param data the ServerMessage struct to store the message in 
 * @param sock the socket
 * @param msgPart stores a part of the message as needed
 * @param msg stores the whole message
 */
void receiveMsg(struct ServerMessage *data, char **msgPart, char **msg)
{
    int success;

    *msgPart = realloc(*msgPart, HEADER_SIZE);

    // receive header
    success = recv(sock, *msgPart, HEADER_SIZE, MSG_WAITALL);
    if (success <= 0)
    {
        sendExitSignal("Connection to server lost. Shutting down...", SIGTERM);
    }

    int payloadLength = getPayloadLength(*msgPart);

    // ensure there is enough memory to store the received message
    *msg = realloc(*msg, HEADER_SIZE + payloadLength);

    strncat(*msg, *msgPart, HEADER_SIZE);

    memset(*msgPart, 0, HEADER_SIZE);

    // resize msgPart to receive the payload
    *msgPart = realloc(*msgPart, payloadLength);

    memset(*msgPart, 0, payloadLength);

    // receive payload
    success = recv(sock, *msgPart, payloadLength, MSG_WAITALL);
    if (success <= 0)
    {
        sendExitSignal("Connection to server lost. Shutting down...", SIGTERM);
    }

    strncat(*msg, *msgPart, payloadLength);

    // don't write the new message yet if the main thread is using the current data
    // prevents the data from being changed as it is being accessed
    while (shouldChildWait || bufferIsDirty)
        ;

    // parse message
    parseServerMsg(*msg, data);

    if (data->header.code == DISCONNECTED)
    {
        sendExitSignal("Connection to server lost. Shutting down...", SIGTERM);
    }

    if (data->message != NULL)
    {
        bufferIsDirty = 1;
    }

    // clear memory in preparation for the next message
    memset(*msg, 0, HEADER_SIZE + payloadLength);
    memset(*msgPart, 0, payloadLength);
}

/**
 * @brief Connects to the server and waits for spreadsheet updates
 * 
 * @param data the struct to store the data in
 */
void *waitForSheet(struct ServerMessage *data)
{
    // thread handles connection to server and sheet buffering
    const int PORT   = 10000;
    const char *HOST = "127.0.0.1";
    char *msgPart    = malloc(1);
    char *msg        = malloc(1);
    struct sockaddr_in addr;

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock > 0)
    {
        // set address
        addr.sin_port   = htons(PORT);
        addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
        {
            sendExitSignal("Invalid server address", SIGUSR1);
        }

        printInfoMsg("> Connecting to server...");
        int success = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (success < 0)
        {
            sendExitSignal("Failed to connect to server", SIGUSR1);
        }

        // get acknowledgement
        fd_set fds;
        FD_SET(sock, &fds);
        struct timeval timeout;
        timeout.tv_sec  = 10;
        timeout.tv_usec = 0;

        int ready = select(sock + 1, &fds, NULL, NULL, &timeout);
        if (ready)
        {
            receiveMsg(data, &msgPart, &msg);
        }
        else
        {
            sendExitSignal("Failed to connect to server", SIGUSR1);
        }

        if (data->header.code != ACKNOWLEDGED || data->message == NULL)
        {
            printMsgFromCode(*data);
            sendExitSignal("Failed to connect to server", SIGUSR1);
        }

        printSuccessMsg(">> Connected");
        shouldMainWait = 0;
        while (1)
        {
            //TODO test receive messages
            receiveMsg(data, &msgPart, &msg);
            shouldMainWait = 0;
        }
    }
    else
    {
        sendExitSignal("Failed to create socket", SIGUSR1);
    }

    pthread_exit(NULL);
}

// END MESSAGE RECEIVING FUNCTONS //

int main(int argc, char const *argv[])
{
    struct ServerMessage serverMsg; // stores messages sent by the server, i.e. the server message buffer
    struct ClientMessage clientReq; // stores request to be sent to the server
    struct ClientMessage saveReq;   // stores a special save request
    struct sigaction onExit;        // an action to be taken when the client fails to start or the user requests to exit
    struct sigaction onThreadExit;

    char *packet       = NULL; // the packet to be sent to the server
    int currentVersion = 0;    // stores the version of the spreadsheet that the user is working on.
    // useful since the serverMsg buffer may be updated before the next request is sent by the client.
    int packetLength = 0; // the length of the packet to be sent

    // initialize message structs
    initServerMessage(&serverMsg);
    initClientMessage(&clientReq);
    initClientMessage(&saveReq);

    // allocate memory for the command
    clientReq.command = malloc(sizeof *clientReq.command);

    //save the main thread ID
    mainThread = pthread_self();

    // set a custom signal handler to be triggered on exit
    onExit.sa_flags     = SA_SIGINFO;
    onExit.sa_sigaction = &exitOnSignal;

    onThreadExit.sa_flags     = SA_SIGINFO;
    onThreadExit.sa_sigaction = &exitThread;

    // register exit handlers //
    //
    // custom exit to raise when server connection fails
    sigaction(SIGUSR1, &onExit, NULL);
    // crashes or user interrupts
    sigaction(SIGINT, &onExit, NULL);
    sigaction(SIGSEGV, &onExit, NULL);
    sigaction(SIGTERM, &onExit, NULL);
    // exit thread before clean up
    sigaction(SIGUSR2, &onThreadExit, NULL);

    printInfoMsg("Starting up...");

    //thread handles connection to server and spreadsheet buffer
    int success = pthread_create(&thread, NULL, (void *(*)(void *))waitForSheet, &serverMsg);

    if (success != 0)
    {
        pthread_kill(mainThread, SIGUSR1);
    }

    // waits until client connects to server and receives a message
    while (shouldMainWait)
        ;

    // get unique client ID assigned by server
    CID = atoi(serverMsg.message);
    bufferIsDirty   = 0;


    // intialize the saveReq since it does not change except for the sheetVersion
    saveReq.header.code        = SAVE;
    saveReq.header.senderId    = CID;
    saveReq.header.clientCount = -1;

    printSuccessMsg("Started");
    // TODO @richtwin567 test client
    int choice       = promptMenu();
    int returnToMenu = 0;
    int refresh = 0;
    while (1)
    {
        if (returnToMenu)
        {
            choice = promptMenu();
        }
        switch (choice)
        {
            case 0:
                // kill the thread before exiting
                exitProgram(0);
                break;

            case 1:
                // stop child thread from changing the data temporarily
                shouldChildWait = 1;
                currentVersion  = serverMsg.header.sheetVersion;
                printSheet(serverMsg.sheet);
                printMsgFromCode(serverMsg);
                printf("There %s %d client%s connected\n", serverMsg.header.clientCount > 1 || serverMsg.header.clientCount == 0 ? "are" : "is", serverMsg.header.clientCount, serverMsg.header.clientCount > 1 || serverMsg.header.clientCount == 0 ? "s" : " \b");
                // resume child
                bufferIsDirty   = 0;
                shouldChildWait = 0;

                // get command from user
                clientReq.command->coords = promptForCell(&returnToMenu,&refresh);
                if (returnToMenu || refresh)
                {
                    continue;
                }
                clientReq.command->input = promptForData();
            

                // set header fields
                clientReq.header.code         = REQUEST;
                clientReq.header.sheetVersion = currentVersion;
                clientReq.header.senderId     = CID;
                clientReq.header.clientCount  = -1;
                // send command
                packetLength = serializeClientMsg(clientReq, &packet);
                if (send(sock, packet, packetLength, 0) != packetLength)
                {
                    perror("Send failed");
                }
                shouldMainWait = 1;
                // wait until the message has been received
                while (shouldMainWait)
                    ;
                break;
            case 2:
                saveReq.header.sheetVersion = serverMsg.header.sheetVersion;

                packetLength = serializeClientMsg(saveReq, &packet);
                if (send(sock, packet, packetLength, 0) != packetLength)
                {
                    perror("Send failed");
                }
                shouldMainWait = 1;
                // wait until the message has been received
                while (shouldMainWait)
                    ;
                // pause child so that the message is not overwritten
                shouldChildWait = 1;
                printMsgFromCode(serverMsg);
                serverMsg.header.code=0;
                // resume child
                bufferIsDirty   = 0;
                shouldChildWait = 0;
                returnToMenu=1;
                break;
            default:
                break;
        }
    }

    return 0;
}
