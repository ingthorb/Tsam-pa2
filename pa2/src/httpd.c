/* A TCP echo server.
 *  *
 *   * Receive a message on port 32000, turn it into upper case and return
 *    * it to the sender.
 *     *
 *      * Copyright (c) 2016, Marcel Kyas
 *       * All rights reserved.
 *        *
 *         * Redistribution and use in source and binary forms, with or without
 *          * modification, are permitted provided that the following conditions are met:
 *           *     * Redistributions of source code must retain the above copyright
 *            *       notice, this list of conditions and the following disclaimer.
 *             *     * Redistributions in binary form must reproduce the above copyright
 *              *       notice, this list of conditions and the following disclaimer in the
 *               *       documentation and/or other materials provided with the distribution.
 *                *     * Neither the name of Reykjavik University nor the
 *                 *       names of its contributors may be used to endorse or promote products
 *                  *       derived from this software without specific prior written permission.
 *                   *
 *                    * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *                     * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *                      * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *                       * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MARCEL
 *                        * KYAS NOR REYKJAVIK UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT,
 *                         * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *                          * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *                           * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *                            * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *                             * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *                              * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *                               * OF THE POSSIBILITY OF SUCH DAMAGE.
 *                                */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#define RESPONSE_SIZE 3072
#define HTML_SIZE 	  2048
#define HEADER_SIZE   1024

void generateHeader(char *header, int contentLength)
{
	strcat(header, "HTTP/1.1 200 OK\r\n");

	strcat(header, "Date: ");
	GTimeVal now;
	g_get_current_time(&now);
	strcat(header, g_time_val_to_iso8601(&now));
	strcat(header, "\r\n");

	strcat(header, "Content-Type: text/html\r\n");

	strcat(header, "Content-Length: ");
	char s_contentLength[5] = "";
	sprintf(s_contentLength, "%d", contentLength);
	strcat(header, s_contentLength);

	strcat(header, "\r\nServer: Donut server 2.0\r\n\r\n");
}

void generateHTML(char* html, struct sockaddr_in cli)
{
	/* Get the host information from the client */
	char host[1024];
	char service[20];
	getnameinfo(&cli, sizeof cli, host, sizeof host, service, sizeof service, 0);

	strcat(html, "<!DOCTYPE html>\r\n");
	strcat(html, "<html>\r\n");
	strcat(html, "\t<body>\r\n");
	strcat(html, "\t\t<p> ");
	strcat(html, host);
	strcat(html, " </p>\r\n");
	strcat(html, "\t</body>\r\n");
	strcat(html, "</html>\r\n");
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server, client;
    char message[1024];

    /* Create and bind a TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Network functions need arguments in network byte order instead of
 *        host byte order. The macros htonl, htons convert the values. */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(atoi(argv[1]));
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    /* Before the server can accept messages, it has to listen to the
 *        welcome port. A backlog of one connection is allowed. */
    listen(sockfd, 1);

    for (;;) {
        /* We first have to accept a TCP connection, connfd is a fresh
 *            handle dedicated to this connection. */
        socklen_t len = (socklen_t) sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr *) &client, &len);

        /* Receive from connfd, not sockfd. */
        ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);

        message[n] = '\0';
        fprintf(stdout, "Received:\n%s\n", message);
				printf("------------------\n");

				char header[HEADER_SIZE] = "";
				char html[HTML_SIZE] = "";
				char response[RESPONSE_SIZE] = "";

				/* Check the request type*/
				gchar** splittedMessage = NULL;
				splittedMessage = g_strsplit(message, "\n", 10);

				/* Check if GET request */
				if (g_str_has_prefix(splittedMessage[0], "GET"))
				{
					generateHTML(html, client);
					generateHeader(header, strlen(html));

					strcat(response, header);
					strcat(response, html);
					printf("Yessss\n");
				}

        /* Send the message back. */
        send(connfd, response, (size_t) sizeof(response), 0);

        /* Close the connection. */
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }
}

