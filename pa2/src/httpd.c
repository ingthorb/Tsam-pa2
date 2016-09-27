/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int sockfd;
    struct sockaddr_in server, client;
    char message[512];

		/* Create and bind a UDP socket */
	  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	  memset(&server, 0, sizeof(server));
	  server.sin_family = AF_INET;

    /* Network functions need arguments in network byte order instead of
       host byte order. The macros htonl, htons convert the values. */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(32000);

    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    /* Before the server can accept messages, it has to listen to the
       welcome port. A backlog of one connection is allowed. */

    for (;;) {
        /* We first have to accept a TCP connection, connfd is a fresh
           handle dedicated to this connection. */
        socklen_t len = (socklen_t) sizeof(client);
				recvfrom(sockfd, message, sizeof(message) - 1,
				              0, (struct sockaddr *) &client, &len);

          }
}
