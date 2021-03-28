/**
 * @file clientMain.c
 * @author 
 * @brief 
 * @version 0.1
 * @date 2021-03-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char const *argv[])
{

    int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    printf("%d sock",sock);
    return 0;
}
