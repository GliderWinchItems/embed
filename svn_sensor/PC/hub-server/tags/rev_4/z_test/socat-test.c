/* Hack hub-server-proxy.c to make a socat server bandwidth test */
/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <semaphore.h>

void usage(void)
{
       fprintf(stderr,"usage: $ socat-test <size_of_first_write>\n");
       exit(EXIT_FAILURE);
}

#define MY_BUFSIZE 256

int first_write;
long write_acc = 0;
long read_acc = 0;

/* Local functions */
static void error(char *msg);
static int binary2ASCII(char *send_buf, char *recv_buf, int n);
static int ASCII2binary(char *send_buf, char *recv_buf, int n);

/* main starts here
 */
int main(int argc, char **argv) {
	int sockfd;					/* Socket file descriptor */
	char *p, ch;
	void *thread(void *arg);
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buf[MY_BUFSIZE], *b;

    /* Check command line arguments */
    if (argc != 2) usage();
    first_write = atoi(argv[1]);
    if(first_write <= 0 || first_write > MY_BUFSIZE) usage();

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", "localhost");
        exit(EXIT_FAILURE);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(32124);

    /* connect: create a connection with the server */
    if (connect(sockfd,
    		(const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

	/* Read/write loop */
	send(sockfd, buf, first_write, 0);
	
	while(1==1)
	{
		ssize_t n, m;

		/* Read a single batch of between 1 and MY_BUFSIZE characters */
		n = recv(sockfd, buf, MY_BUFSIZE, 0);
		if(n < 0) 
		{
			fprintf(stderr, "recv(%d, ...) returns %d\n", sockfd, n);
			continue;
		}
		else
			read_acc += n;

		/* Write out n characters */
		for( ; n > 0; b += m, n -= m)
		{
			m = send(sockfd, b, n, 0);
			if(m == 0) continue;
			if(m < 0) 
			{
				fprintf(stderr, "recv(%d, ...) returns %d\n",
													sockfd, n);
				break;
			}
		}

		if(read_acc > write_acc)
		{
			printf("%ld chars\n", read_acc);
			write_acc += 10000;
		}
	}
	
	return 0;
}

/* error - wrapper for perror
 */
static void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
