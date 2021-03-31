#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char** argv)
{
	int sockfd;
	struct sockaddr_in serverAddress = {0};

	struct hostent* server = 0;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Failed: socket");
		return -1;
	}

	if((server = gethostbyname("localhost")) == NULL)
	{
		printf("Failed: gethostbyname");
		return -1;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port   = htons(10000);
	bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
	
	if(connect(sockfd, (struct sockaddr*)&serverAddress,
			   sizeof(struct sockaddr)) < 0)
	{
		printf("Failed: connect\n");
		return -1;
	}

	sleep(10);
	close(sockfd);

	return 0;
}
