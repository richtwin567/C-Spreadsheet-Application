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
#include <time.h>
#include <unistd.h>

// GLOBALS TO BE USED IN SIGNALS //

int shouldWait = 1;

// END GLOBALS //

void *waitForSheet(struct ServerMessage *data)
{
    //thread handles connection to server and sheet buffering
    int const PORT   = 10000;
    const char *HOST = "127.0.0.1";
    struct sockaddr_in addr;

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

        //test for memory usage levels
        char *t = NULL;
        while (1)
        {
            data->header.code         = OK;
            data->header.senderId     = 4;
            data->header.sheetVersion = ++i % 100000000;
            data->sheet.lineLength    = 112;
            data->sheet.rowCount      = 20;
            data->sheet.size          = 9;

            if (data->sheet.grid == NULL)
            {
                data->sheet.grid = calloc(data->sheet.rowCount, sizeof *data->sheet.grid);
                for (int i = 0; i < data->sheet.rowCount; i++)
                {
                    data->sheet.grid[i] = calloc(data->sheet.lineLength + 1, sizeof *(data->sheet.grid[i]));
                }
            }
            else
            {
                data->sheet.grid = realloc(data->sheet.grid, data->sheet.rowCount * (sizeof *data->sheet.grid));
                for (int i = 0; i < data->sheet.rowCount; i++)
                {
                    data->sheet.grid[i] = realloc(data->sheet.grid[i], data->sheet.lineLength + 1 * (sizeof *(data->sheet.grid[i])));
                }
            }

            strcpy(data->sheet.grid[0], "        A           B           C           D           E           F           G           H           I      ");

            strcpy(data->sheet.grid[1], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[2], "1 | Balance   |           |           |           |           |           | 2.415e+07 |           | Cumula... |");
            strcpy(data->sheet.grid[3], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[4], "2 |           |    789.00 |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[5], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[6], "3 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[7], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[8], "4 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[9], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[10], "5 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[11], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[12], "6 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[13], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[14], "7 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[15], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[16], "8 |           |           |           |           |           |           |           |           |           |");
            strcpy(data->sheet.grid[17], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");
            strcpy(data->sheet.grid[18], "9 |           |           |           |           |  25789.00 |           |           |           |           |");
            strcpy(data->sheet.grid[19], "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+");

            serializeServerMsg(*data, &t);

            parseServerMsg(t, data);

            shouldWait=0;

            //printf("THREAD: %d\n", data->header.sheetVersion);
        }
    }
    else
    {
        //raise(SIGUSR1);
    }

    return NULL;
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

    initServerMessage(&s_msg);

    action.sa_flags     = SA_SIGINFO;
    action.sa_sigaction = &exitOnFailure;

    sigaction(SIGUSR1, &action, NULL);

    //thread handles connection to server and sheet buffering
    pthread_create(&thread, NULL, (void *(*)(void *))waitForSheet, &s_msg);

    printf("\nMain: %p\n", &s_msg);

    while (1)
    {
        while (shouldWait)
            ;
        sleep(4);
        printf("%d\n", s_msg.header.sheetVersion);
    }

    return 0;
}
