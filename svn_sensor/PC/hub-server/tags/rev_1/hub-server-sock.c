/******************************************************************************/
/* hub-server-sock.c -- Implementation -- Socket stuff for hub-server.
 */

/* Dependencies 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include "hub-server-util.h"
#include "hub-server-sock.h"

/******************************************************************************/
/* #defines */

#define	HS_MAX_CONNECT	10		/* Maximum number of client connections */

#define IN_LINE_SIZE	300		/* Size of input buffer */
#define	ASCII_NL 		012		/* ADCII -- Line terminator */
#define	BINARY_DLE		0x7D	/* Binary -- Data link escape */
#define	BINARY_EOB		0x7E	/* Binary -- End of block */

/* Local structures */

typedef struct client_t
{
	int socket;					/* Socket fd */
	char *ptibs;				/* Pointer to input buffer start */
	char *ptibe;				/* Pointer to input buffer end */
	char *ptibfs;				/* Pointer to input buffer free space */
} client_t;

/* Local variables */

static pthread_t client_io_tid;
static long select_seqn = 0;

static client_t ccb[HS_MAX_CONNECT];	/* Client connection blocks */

#define	LOOP(p) for(p=ccb+0; p<ccb+HS_MAX_CONNECT; p++)
#define	SKIP(p) if(p->socket < 0) continue

/* Local function declarations */

static void *client_io_thread(void *);

/* Global functions */

/* Initialize the single client I/O thread.
 */
void hs_sock_init(void)			/* Called from the other thread */
{
	client_t *p;
	
	LOOP(p)
	{
		p->socket = -1;
		p->ptibs = hs_malloc(IN_LINE_SIZE);
		p->ptibe = p->ptibs + IN_LINE_SIZE;
	}

	if(pthread_create(&client_io_tid, NULL, client_io_thread, NULL) != 0) 
		hs_fatal("hs_sock_init", "Failed to create socket thread");
}

/* Create and populate a new client control block.  Link it onto the front of the
 * list of client control blocks.
 */
void hs_sock_new_client(int newsock)
{
	client_t *p;
	
	printf("hs_sock_new_client(%d) called\n", newsock);

	LOOP(p)
		if(p->socket < 0)
		{
			p->ptibfs = p->ptibs;
			p->socket = newsock;
			break;
		}

	if(p == ccb + HS_MAX_CONNECT)
	{
		printf("hs_sock_new_client(%d) -- too many clients\n", newsock);
		close(newsock);
	}
}

/* Do all client I/O as follows:
 * - Search the array of clients and build the fd structure for select(...).
 * - Do a select(...).
 * - Do a recv(...) for each client flagged by select for input.
 * - Find the last ASCII line terminator (if any) in the buffer.
 * - Send(...) up to the last ASCII line terminator to all other clients.
 * - Loop forever.
 */
static void *client_io_thread(void *arg)		/* 1 of these for all clients */
{
	int client_fd_max;
	int n, tmp;
	fd_set recv_fd;
	struct timeval timeout;
	client_t *p, *q;
	char *s;

	/* Loop forever
	 */
	while(1==1)
	{
		/* Build fd set for recv(...)
		 */
		FD_ZERO(&recv_fd);
		client_fd_max = 0;
		LOOP(p)
		{
			SKIP(p);

			if(client_fd_max < p->socket)
				client_fd_max = p->socket;
			FD_SET(p->socket, &recv_fd);
		}

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		/* Wait for input or timeout
		 */
		tmp = select(client_fd_max+1, &recv_fd, NULL, NULL, &timeout);
		select_seqn += 1;

		if(tmp == 0)
			printf("%ld> select(%d, ...) == 0\n", 
										select_seqn, client_fd_max+1);
		
		/* Loop through all active clients looking for work to do.
		 */
		LOOP(p)
		{
			SKIP(p);

			/* If input is ready for this client...
			 */
			if(FD_ISSET(p->socket, &recv_fd))
			{
				/* Append new input to end of existing input.
				 */
				n = hs_recv(p->socket, p->ptibfs, p->ptibe - p->ptibfs);
				if(n <= 0)
				{
					printf("hs_recv err, close(%d)\n", p->socket);
					
					close(p->socket);		/* Close client connection */
					p->socket = -1;			/* Show ccb as closed */
				}
				else
				{
					p->ptibfs += n;
					
					/* Search backwards for last ASCII line terminator
					 */
					for(s = p->ptibfs; s > p->ptibs; )
						if(*--s == ASCII_NL)
							break;
					
					if(*s++ == ASCII_NL)		/* Full line(s)? */
					{
						/* Send line(s) to all client(s) except ourselves
						 */
						LOOP(q)
						{
							SKIP(q);

							if(p != q)
								hs_send(q->socket, p->ptibs, s - p->ptibs);
						}
	
						/* Copy unsent bytes at end down to start of buffer
						 */
						n = p->ptibfs - s;
						if(n > 0)
							memmove(p->ptibs, s, n);
						p->ptibfs = p->ptibs + n;
					}
					else if(p->ptibfs == p->ptibe)	/* Long line error */
					{
						printf("long line err, close(%d)\n", p->socket);
						
						close(p->socket);		/* Close client connection */
						p->socket = -1;			/* Show ccb as closed */
					}
				}
			}
		}
	}
}
