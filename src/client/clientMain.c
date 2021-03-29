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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Globals to be used by signals
int shouldWait = 1;
struct ServerMessage sdata;
char *smsg;
int pipefd[2];

void waitForSheet(pid_t ppid)
{
    int const PORT   = 10000;
    const char *HOST = "127.0.0.1";
    struct sockaddr_in addr;
    int first = 1;

    //child handles connection to server and sheet buffering

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
            //kill(ppid, SIGUSR1);
            //exit(1);
        }

        //testing inter-process communication

        while (1)
        {
            sleep(2);
            sdata.header.code         = OK;
            sdata.header.senderId     = 4;
            sdata.header.sheetVersion = ++i;
            sdata.message             = NULL;
            sdata.sheet.lineLength    = 112;
            sdata.sheet.rowCount      = 20;
            sdata.sheet.size          = 9;

            if (sdata.sheet.grid == NULL)
            {
                sdata.sheet.grid = calloc(sdata.sheet.rowCount, sizeof *sdata.sheet.grid);
                for (int i = 0; i < sdata.sheet.rowCount; i++)
                {
                    sdata.sheet.grid[i] = calloc(sdata.sheet.lineLength, sizeof *(sdata.sheet.grid[i]));
                }
            }
            sdata.sheet.grid[0]  = "        A           B           C           D           E           F           G           H           I      ";
            sdata.sheet.grid[1]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[2]  = "1 | Balance   |           |           |           |           |           | 2.415e+07 |           | Cumula... |";
            sdata.sheet.grid[3]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[4]  = "2 |           |    789.00 |           |           |           |           |           |           |           |";
            sdata.sheet.grid[5]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[6]  = "3 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[7]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[8]  = "4 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[9]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[10] = "5 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[11] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[12] = "6 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[13] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[14] = "7 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[15] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[16] = "8 |           |           |           |           |           |           |           |           |           |";
            sdata.sheet.grid[17] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
            sdata.sheet.grid[18] = "9 |           |           |           |           |  25789.00 |           |           |           |           |";
            sdata.sheet.grid[19] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";

            smsg = serializeServerMsg(sdata);

            for (int i = 0; i < sdata.sheet.rowCount; i++)
            {
                //free(sdata.sheet.grid[i]);
            }
           // free(sdata.sheet.grid);

            if (first)
            {
                first = 0;
                kill(ppid, SIGUSR2);
            }
        }
    }
    else
    {
        //kill(ppid, SIGUSR1);
        //exit(1);
    }
}

void exitOnFailure(int sig, siginfo_t *info, void *ucontext)
{
    printErrorMsg("Program failed to start", NULL);
    _exit(1);
}

void stopWait(int sig, siginfo_t *info, void *ucontext)
{
    shouldWait = 0;
}

void writeSheetUpdate(int sig, siginfo_t *info, void *ucontext)
{
    write(pipefd[1], smsg, HEADER_SIZE);
    write(pipefd[1], smsg, strlen(smsg));
}

int main(int argc, char const *argv[])
{
    struct sigaction onFailure;
    struct sigaction onRequestUpdate;
    struct sigaction onSuccess;
    pthread_t thread;

    onFailure.sa_flags     = SA_SIGINFO;
    onFailure.sa_sigaction = &exitOnFailure;

    sigaction(SIGUSR1, &onFailure, NULL);

    onSuccess.sa_flags     = SA_SIGINFO;
    onSuccess.sa_sigaction = &stopWait;

    sigaction(SIGUSR2, &onSuccess, NULL);

    if (pipe(pipefd) == -1)
    {
        printErrorMsg("Establishing process communication failed", NULL);
        exit(1);
    }

    pid_t ppid = getpid();
    pid_t cpid = fork();
    if (cpid == -1)
    {
        printErrorMsg("Failed to start program", NULL);
        exit(1);
    }
    if (cpid == 0)
    {
        //child process allows for asynchronus handling of connection to server and sheet buffering
        close(pipefd[0]);
        onRequestUpdate.sa_flags     = SA_SIGINFO;
        onRequestUpdate.sa_sigaction = &writeSheetUpdate;
        sigaction(SIGUSR2, &onRequestUpdate, NULL);
        waitForSheet(ppid);
    }
    else
    {
        close(pipefd[1]);

        printInfoMsg("\nConnecting to server...\n");
        while (shouldWait)
            ;
        printSuccessMsg("Connected");
        while (1)
        {
            sleep(6);
            kill(cpid, SIGUSR2);

            char *header = malloc(HEADER_SIZE + 1);
            read(pipefd[0], header, HEADER_SIZE);

            int length = getPayloadLength(header) + HEADER_SIZE;
            smsg       = malloc(length);
            read(pipefd[0], smsg, length);

            sdata = parseServerMsg(smsg);

            printf("\nLatest sheet version: %d\n", sdata.header.sheetVersion);
        }
    }
    return 0;
}
