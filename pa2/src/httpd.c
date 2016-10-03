

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>

#define RESPONSE_SIZE  3072
#define HTML_SIZE 	   2048
#define HEADER_SIZE    1024
#define POST_BODY_SIZE 1024
#define QUERY_BUFFER   1024

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

void generateHTML(char* html, struct sockaddr_in cli,  char *postContent, char *url)
{
	/* Get the host information from the client */
	char host[1024];
	char service[20];
	getnameinfo(&cli, sizeof cli, host, sizeof host, service, sizeof service, 0);

	gboolean isColorRequest = g_str_has_prefix(url, "/color");

	if (isColorRequest)
	{
		gchar** query = NULL;
		query = g_strsplit(url, "?", 10);

		gchar ** splittedQuery = NULL;
		splittedQuery = g_strsplit(query[1], "=", 10);

		gchar ** color = NULL;
		color = g_strsplit(splittedQuery[1], "&", 10);

		strcat(html, "<!DOCTYPE html>\r\n");
		strcat(html, "<html>\r\n");
		strcat(html, "\t<body style=\"background-color:");
		strcat(html, color[0]);
		strcat(html, "\">\r\n\t</body>\r\n");
		strcat(html, "</html>\r\n");

		g_strfreev(query);
		g_strfreev(splittedQuery);
		g_strfreev(color);

		return;
	}

	/* Get the port of the client */
	char s_port[10] = "";
	sprintf(s_port, "%d", ntohs(cli.sin_port));

	strcat(html, "<!DOCTYPE html>\r\n");
	strcat(html, "<html>\r\n");
	strcat(html, "\t<body>\r\n");
	strcat(html, "\t\t<p> ");
	strcat(html, host);
	strcat(html, url);
	strcat(html, " ");
	strcat(html, inet_ntoa(cli.sin_addr));
	strcat(html, ":");
	strcat(html, s_port);
	strcat(html, " </p>\r\n");

	if (postContent != NULL)
	{
		strcat(html, "\t\t<p> ");
		strcat(html, postContent);
		strcat(html, " </p>\r\n");
	}

	/* Generate HTML from the URI's query */
	gchar* res = g_strrstr(url, "?\0");

	if (res != NULL)
	{
		gchar** query = NULL;
		query = g_strsplit(url, "?", 10);

		gchar** queries = NULL;
		queries = g_strsplit(query[1], "&", 10);

		int i = 0;
		gchar* curr = queries[i];
		while (curr != NULL)
		{
			strcat(html, "\t\t<p> ");
			strcat(html, curr);
			strcat(html, " </p>\r\n");
			curr = queries[i+1];
			i++;
		}

		g_strfreev(query);
		g_strfreev(queries);
	}

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

				gchar** splittedBySpace = NULL;
				splittedBySpace = g_strsplit(splittedMessage[0], " ", 10);

				/* Check if GET request */
				if (g_str_has_prefix(splittedMessage[0], "GET"))
				{
					generateHTML(html, client, NULL, splittedBySpace[1]);
					generateHeader(header, strlen(html));

					strcat(response, header);
					strcat(response, html);
					printf("GET request\n");
				}
				/* Check if HEAD request */
				else if (g_str_has_prefix(splittedMessage[0], "HEAD"))
				{
					/* Generate the HTML to get the content length, which should be the
					    size of the body that would have been sent had the request been
							GET (HTTP 1.1 RFC) */
					generateHTML(html, client, NULL, splittedBySpace[1]);
					generateHeader(header, strlen(html));

					strcat(response, header);
					printf("HEAD request\n");
				}
				/* Check if POST request */
				else if (g_str_has_prefix(splittedMessage[0], "POST"))
				{
					memset(&splittedMessage[0], 0, sizeof(splittedMessage)); /* Clear the array */
					splittedMessage = g_strsplit(message, "\r\n\r\n", 10); /* Access the POST data */
					generateHTML(html, client, splittedMessage[1], splittedBySpace[1]);
					generateHeader(header, strlen(html));

					strcat(response, header);
					strcat(response, html);
					printf("POST request\n");
				}

				g_strfreev(splittedMessage);
				g_strfreev(splittedBySpace);

        /* Send the message back. */
        send(connfd, response, (size_t) sizeof(response), 0);

        /* Close the connection. */
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }
}
