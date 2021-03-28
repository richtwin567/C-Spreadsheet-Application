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
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void waitForSheet(struct ServerMessage *data)
{
    int const PORT   = 10000;
    const char *HOST = "127.0.0.1";
    struct sockaddr_in addr;

    printf("\nTHREAD!!!\n");
    //child handles connection to server and sheet buffering

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("%d sock", sock);

    printf("\nThread: %p\n", data);

    int i = 0;
    if (sock > 0)
    {
        addr.sin_addr.s_addr = inet_addr(HOST);
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(PORT);

        int success = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (success < 0)
        {
            //raise(SIGUSR1);
        }

        while (1)
        {
            sleep(2);
            data->header.code         = OK;
            data->header.senderId     = 4;
            data->header.sheetVersion = ++i;
            data->message             = NULL;
            data->sheet.grid          = NULL;
            data->sheet.lineLength    = 0;
            data->sheet.rowCount      = 0;
            data->sheet.size          = 0;

            printf("\nTHREAD: %d", data->header.sheetVersion);
        }
    }
    else
    {
        //raise(SIGUSR1);
    }
}

void exitOnFailure(int sig, siginfo_t *info, void *ucontext)
{
    printf("\nfailed to start");
    _exit(1);
}

int main(int argc, char const *argv[])
{
    struct ServerMessage s_msg;
    struct sigaction action;
    pthread_t thread;

    action.sa_flags     = SA_SIGINFO;
    action.sa_sigaction = &exitOnFailure;

    sigaction(SIGUSR1, &action, NULL);

    //thread handles connection to server and sheet buffering
    pthread_create(&thread, NULL, waitForSheet, &s_msg);

    printf("\nMain: %p\n", &s_msg);

    while (1){
        sleep(6);
        printf("\n%d", s_msg.header.sheetVersion);
    }

    return 0;
}
