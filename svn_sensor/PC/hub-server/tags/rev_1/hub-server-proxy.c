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
       fprintf(stderr,"usage: $ export HUB_SERVER_PROXY_HS_HOST=<hostname>\n");
       fprintf(stderr,"       $ export HUB_SERVER_PROXY_HS_PORT=<portnum>\n");
       fprintf(stderr,"       $ export HUB_SERVER_PROXY_TT_HOST=<hostname>\n");
       fprintf(stderr,"       $ export HUB_SERVER_PROXY_TT_PORT=<portnum>\n");
       fprintf(stderr,"       $ export HUB_SERVER_PROXY_BINARY={\"Y\" | \"N\"}\n");
       fprintf(stderr,"       $ hub-server-proxy\n");
       exit(EXIT_FAILURE);
}/* Local variables */
typedef struct thread_stuff_t
{
	sem_t sem;					/* Our startup sync semaphore */
	char *hostname;				/* Server host name (or IP addr.) */
	char *serviceport;			/* Server host port number */
	int is_tty;					/* Used in binary mode */
	int sockfd;					/* Socket file descriptor */
	int run_flag;				/* Thread running flag */
	pthread_t tid;				/* Thread ID */
	struct thread_stuff_t *other;/* Pointer to other thread stuff */
} thread_stuff_t;

static thread_stuff_t hs_stuff;	/* Hub-server stuff */
static thread_stuff_t tt_stuff;	/* tty-server stuff */

static int tty_is_binary;		/* HS side is always ASCII */

/* Local functions */
static void error(char *msg);
static int binary2ASCII(char *send_buf, char *recv_buf, int n);
static int ASCII2binary(char *send_buf, char *recv_buf, int n);

/* main starts here
 */
int main(int argc) {
	char *p, ch;
	void *thread(void *arg);

    /* Check command line arguments */
    if (argc != 1) usage();

    /* Cross-link thread stuff blocks */
    hs_stuff.other = &tt_stuff;
	tt_stuff.other = &hs_stuff;
	
	/* Identify the tty port.  Binary mode assumes tty side is binary and
	 * hub-server side is ASCII.
	 */
	hs_stuff.is_tty = (1==0);
	tt_stuff.is_tty = (1==1);

	/* Get environment variables or set default string */
	p = getenv("HUB_SERVER_PROXY_HS_HOST");
	hs_stuff.hostname = p ? p : "localhost";	/* Default HS host */
	p = getenv("HUB_SERVER_PROXY_TT_HOST");
	tt_stuff.hostname = p ? p : "localhost";	/* Default TT host */

	p = getenv("HUB_SERVER_PROXY_HS_PORT");
	hs_stuff.serviceport = p ? p : "32123";		/* Default HS port */
	p = getenv("HUB_SERVER_PROXY_TT_PORT");
	tt_stuff.serviceport = p ? p : "32124";		/* Default TT port */

	p = getenv("HUB_SERVER_PROXY_BINARY");
	if(!p) p = "N";
	ch = toupper(*p);
	if(ch == 'N')
		tty_is_binary = (1==0);
	else if(ch == 'Y')
		tty_is_binary = (1==1);
	else
		usage();

	/* Initialize startup semaphores */
	sem_init(&hs_stuff.sem, (1==0), 0);
	sem_init(&tt_stuff.sem, (1==0), 0);

	/* Create two copies of the same thread */
    if(pthread_create(&hs_stuff.tid, NULL, thread, (void *)&hs_stuff) != 0) 
    	error("Can't start hs thread");
    if(pthread_create(&tt_stuff.tid, NULL, thread, (void *)&tt_stuff) != 0) 
    	error("Can't start tt thread");

	/* Nothing more for the main thread to do but we keep it around */
	do sleep(100); while(1==1);
	
	return 0;
}

#define	MY_BUFSIZE	256

void *thread(void *arg) {
	thread_stuff_t *s = (thread_stuff_t *)arg;
    struct sockaddr_in serveraddr;
    struct hostent *server;

    /* socket: create the socket */
    s->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(s->hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", s->hostname);
        exit(EXIT_FAILURE);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(atoi(s->serviceport));

    /* connect: create a connection with the server */
    if (connect(s->sockfd,
    		(const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

	/* So we know when to stop */
	s->run_flag = (1==1);

    /* Synchronize with the other thread */
    sem_post(&s->sem);			/* Post on my semaphore */
    sem_wait(&s->other->sem);	/* Wait on his semaphore */

	/* Read/write loop */
	while(s->run_flag)
	{
		ssize_t n, m;
    	char recv_buf[MY_BUFSIZE], send_buf[3 * MY_BUFSIZE], *b;

		/* Read a single batch of between 1 and MY_BUFSIZE characters */
		n = recv(s->sockfd, recv_buf, MY_BUFSIZE, 0);
		if(n <= 0) 
		{
			fprintf(stderr, "recv(%d, ...) returns %d\n", s->sockfd, n);

			if(n == 0) break;	/* n == 0 --> EOF */
			continue;
		}

		/* Convert to binary (if enabled) */
		if(tty_is_binary)
		{
			if(s->is_tty)
				n = binary2ASCII(send_buf, recv_buf, n);
			else
				n = ASCII2binary(send_buf, recv_buf, n);
				
			b = send_buf;
		}
		else
			b = recv_buf;			

		/* Write out n characters */
		for( ; n > 0; b += m, n -= m)
		{
			m = send(s->other->sockfd, b, n, 0);
			if(m == 0) continue;
			if(m < 0) 
			{
				fprintf(stderr, "recv(%d, ...) returns %d\n",
													s->other->sockfd, n);
				break;
			}
		}
	}
	
	return NULL;
}

/* Binary(tty) to ASCII(hub-server) converter.
 */
static int binary2ASCII(char *send_buf, char *recv_buf, int count)
{
	error("Binary(tty) to ASCII(hub-server) not yet implemented");
}

/* ASCII(hub-server) to binary(tty) converter.
 */
static int ASCII2binary(char *send_buf, char *recv_buf, int count)
{
	error("ASCII(hub-server) to binary(tty) not yet implemented");
}

/* error - wrapper for perror
 */
static void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
