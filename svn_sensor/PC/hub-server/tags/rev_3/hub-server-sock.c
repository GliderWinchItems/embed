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
#include "hub-server-queue.h"

/******************************************************************************/
/* #defines */

#define	HS_MAX_CONNECT	20		/* Maximum number of client connections */

#define IN_LINE_SIZE	300		/* Size of input buffer */
#define	OUT_QUEUE_SIZE 1000		/* Size of output queue */
#define	ASCII_LF 		012		/* ADCII -- Line terminator */

/* Local structures */

typedef struct client_t
{
	int socket;					/* Socket fd */
	char *ptibs;				/* Pointer to input buffer start */
	char *ptibe;				/* Pointer to input buffer end */
	char *ptibfs;				/* Pointer to input buffer free space */
	pthread_t ot_tid;			/* Output thread tid */
	sem_t ot_sem;				/* Output thread semaphore */
	hsq_t oq;					/* Output queue structure */
	/* Debug members follow */
	unsigned long in_count;		/* Num of bytes read */
	unsigned long q_ok_count;	/* Num of bytes accepted by hsq_enqueue */
	unsigned long q_ng_count;	/* Num of bytes not accepted by hsq_enqueue */
} client_t;

/* Local variables */

static pthread_t client_in_tid;
static long select_seqn = 0;

static client_t ccb[HS_MAX_CONNECT];	/* Client connection blocks */

#define	LOOP(p) for(p=ccb+0; p<ccb+HS_MAX_CONNECT; p++)
#define	SKIP(p) if(p->socket < 0) continue

/* Local function declarations */

static void *in_thread(void *);
static void *out_threads(void *);
static void close_socket(client_t *c, char *msg);

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
		
		sem_init(&p->ot_sem, 0, 0);
		hsq_new(&p->oq, OUT_QUEUE_SIZE);

		if(pthread_create(&p->ot_tid, NULL, out_threads, (void *)p) != 0) 
			hs_fatal("hs_sock_init", "Failed to create socket out thread");
	}

	if(pthread_create(&client_in_tid, NULL, in_thread, NULL) != 0) 
		hs_fatal("hs_sock_init", "Failed to create socket in thread");
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
			p->socket = newsock;
			p->ptibfs = p->ptibs;
			p->in_count = 0;
			p->q_ok_count = 0;
			p->q_ng_count = 0;
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
static void *in_thread(void *arg)			/* 1 of these for all clients */
{
	int client_fd_max;
	int n, tmp;
	fd_set recv_fd;
	struct timeval timeout;
	client_t *in, *out;
	char *s;

	/* Loop forever
	 */
	while(1==1)
	{
		/* Build fd set for recv(...)
		 */
		FD_ZERO(&recv_fd);
		client_fd_max = 0;
		LOOP(in)
		{
			SKIP(in);

			if(client_fd_max < in->socket)
				client_fd_max = in->socket;
			FD_SET(in->socket, &recv_fd);
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
		LOOP(in)							/* Visit all clients */
		{
			SKIP(in);						/* Skip inactive client */

			/* If input is ready for this client...
			 */
			if(FD_ISSET(in->socket, &recv_fd))
			{
				/* Append new input to end of existing input.
				 */
				n = hs_recv(in->socket, in->ptibfs, in->ptibe - in->ptibfs);
				if(n <= 0)
					close_socket(in, "client disconnect or hs_recv err");
				else
				{
					in->in_count += n;		/* Accumulate stats */
					in->ptibfs += n;		/* Adjust free space ptr */
					
					/* Search backwards for last ASCII line terminator
					 */
					for(s = in->ptibfs; s > in->ptibs; )
						if(*--s == ASCII_LF)
							break;

					if(*s++ == ASCII_LF)		/* Full line(s)? */
					{
						/* Send line(s) to all client(s) except ourselves
						 */
						LOOP(out)				/* Visit all clients */
						{
							SKIP(out);

							if(in != out)		/* Skip ourselves */
							{
								int i, j;
								
								i = s - in->ptibs;
								j = hsq_enqueue_chars(&out->oq, in->ptibs, i);

								out->q_ok_count += j;
								out->q_ng_count += i - j;

								if(i != j)
									printf("enqueue err, %d/%d\n", j, i);

								sem_post(&out->ot_sem);
							}
						}

						/* Copy unsent bytes at end down to start of buffer
						 */
						n = in->ptibfs - s;
						if(n > 0)
							memmove(in->ptibs, s, n);
						in->ptibfs = in->ptibs + n;
					}
					else if(in->ptibfs == in->ptibe)	/* Long line error */
						close_socket(in, "long line err");						
				}
			}
		}
	}
}

static void close_socket(client_t *c, char *msg)
{
	close(c->socket);		/* Close client connection */
	
	printf("close(%d) -- in: %lu, outok: %lu, outng: %lu",
					c->socket, c->in_count, c->q_ok_count, c->q_ng_count);
	if(msg && *msg)
		printf(" -- %s", msg);
	printf("\n");
	
	c->socket = -1;			/* Show ccb as closed */
}

static void *out_threads(void *p)		/* 1 of these for each client */
{
	client_t *out = (client_t *)p;		/* Point to client structure */

	while(1)							/* Go forever */
	{
		int i, j;
		
		sem_wait(&out->ot_sem);			/* Wait for work to do */
		
		i = hsq_get_used_bytes(&out->oq);/* Get number of bytes to do */
		
		if(i > 0)						/* Work to do? */
		{	
			j = hsq_write_lines_from_queue(&out->oq, out->socket);
			if(j < i)
				printf("hsq_write_lines_from_queue %d/%d\n", j, i);
		}
	}
}
