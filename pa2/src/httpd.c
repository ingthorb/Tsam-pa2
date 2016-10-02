/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>

char webpage[]=
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html> <head><title>Test</title>\r\n"
"<body><center><h1>Hardcoded shit</h1><br>\r\n"
"</center></body></html>\r\n";
int main(int argc, char *argv[])
{
	int sockfd;
    struct sockaddr_in server, client;

    char message[2048];

		/* sockfd = fd_ server  Create and bind a UDP socket */
	  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	  memset(&server, 0, sizeof(server));
	  server.sin_family = AF_INET;

    /* Network functions need arguments in network byte order instead of
       host byte order. The macros htonl, htons convert the values. */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(18484);

		//Gefur -1 í error Viljum við fara eftir þvi?
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

		//integerinn er number of connections simeltins
		listen(sockfd, 1);
		printf("Why this not work");
    /* Before the server can accept messages, it has to listen to the
       welcome port. A backlog of one connection is allowed. */
			 //Create the string after you get the ip and port
		for(;;)
		{
			socklen_t len = (socklen_t) sizeof(client);
			  int connfd = accept(sockfd, (struct sockaddr *) &client, &len);
				/* Receive from connfd, not sockfd. */
				ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
				//could be -1 should we check?
				//printf("We got the client connection");
 				send(connfd, webpage, sizeof(webpage), 0);
				shutdown(connfd, SHUT_RDWR);
				printf("Why won't you close");
				close(connfd);
		/*		if(!fork())
				{
					/*child proccess*/
					/*close(sockfd);
					read(fd_client, message, 2047);

					printf("%sn", message);
					write(fd_client, webpage, sizeof(webpage) - 1);
				}
				printf("Closing");
				close(fd_client);*/
		}
		printf("We never close");
}
