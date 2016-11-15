/******************************************************************************/
/* hub-server-sock.c -- Implementation -- Socket stuff for hub-server.
 */

/* Dependencies 
 */
#include <syslog.h>
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
/* Externals */

extern int proxy_mode;					/* Proxy mode flag */

/* Local variables */

static pthread_t connect_in_tid;
static long select_seqn = 0;

static connect_t ccb[HS_MAX_CONNECT];	/* Connection control blocks */

#define	LOOP(p) for(p=ccb+0; p<ccb+HS_MAX_CONNECT; p++)
#define	SKIP(p) if(p->socket < 0) continue

/* Local function declarations */

static void *in_thread(void *);
static void *out_threads(void *);
static void close_socket(connect_t *c, char *msg);

/* Global functions */

/* Initialize the single connect I/O thread.
 */
void hs_sock_init(void)			/* Called from the other thread */
{
	connect_t *p;
	
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

	if(pthread_create(&connect_in_tid, NULL, in_thread, NULL) != 0) 
		hs_fatal("hs_sock_init", "Failed to create socket in thread");
}

/* Create and populate a new connect control block.  Link it onto the front of the
 * list of connect control blocks.
 */
void hs_sock_new_connect(int newsock, int is_server)
{
	connect_t *p;
	
	syslog(LOG_INFO, "hs_sock_new_connect(%d) called\n", newsock);

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
		syslog(LOG_INFO, "hs_sock_new_connect(%d) -- too many connects\n", newsock);
		close(newsock);
	}
}

/* Do all connect I/O as follows:
 * - Search the array of connects and build the fd structure for select(...).
 * - Do a select(...).
 * - Do a recv(...) for each connect flagged by select for input.
 * - Find the last ASCII line terminator (if any) in the buffer.
 * - Send(...) up to the last ASCII line terminator to all other connects.
 * - Loop forever.
 */
static void *in_thread(void *arg)			/* 1 of these for all connects */
{
	int connect_fd_max;
	int n, tmp;
	fd_set recv_fd;
	struct timeval timeout;
	connect_t *in, *out;
	char *s;

	/* Loop forever
	 */
	while(1==1)
	{
		/* Build fd set for recv(...)
		 */
		FD_ZERO(&recv_fd);
		connect_fd_max = 0;
		LOOP(in)
		{
			SKIP(in);

			if(connect_fd_max < in->socket)
				connect_fd_max = in->socket;
			FD_SET(in->socket, &recv_fd);
		}

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		/* Wait for input or timeout
		 */
		tmp = select(connect_fd_max+1, &recv_fd, NULL, NULL, &timeout);
		select_seqn += 1;

if(0)
		if(tmp == 0)
			syslog(LOG_INFO, "%ld> select(%d, ...) == 0\n", 
										select_seqn, connect_fd_max+1);

		/* Loop through all active connects looking for work to do.
		 */
		LOOP(in)							/* Visit all connects */
		{
			SKIP(in);						/* Skip inactive connect */

			/* If input is ready for this connect...
			 */
			if(FD_ISSET(in->socket, &recv_fd))
			{
				/* Append new input to end of existing input.
				 */
				n = hs_recv(in->socket, in->ptibfs, in->ptibe - in->ptibfs);
				if(n <= 0)
					close_socket(in, "connect disconnect or hs_recv err");
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
						int size = s - in->ptibs;
						
						/* Send line(s) to all connect(s) except ourselves.
						 * In proxy mode, clients <==> servers.
						 */
						hsd_new_in_data(in, size);

						LOOP(out)				/* Visit all connects */
						{
							SKIP(out);

							if(proxy_mode)		/* clients <==> servers */
							{
								if( in->is_server &&  out->is_server) continue;
								if(!in->is_server && !out->is_server) continue;
							}
							else				/* Don't talk to ourselves */
							{
								if(in == out) continue;
							}
							
							n = hsd_new_in_out_pair(in, out, size);

							out->q_ok_count += n;
							out->q_ng_count += size - n;

							sem_post(&out->ot_sem);
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
	
	return NULL;						/* Supress warning message */
}

static void close_socket(connect_t *c, char *msg)
{
	close(c->socket);		/* Close connection */
	
	syslog(LOG_INFO, "close(%d) -- in: %lu, outok: %lu, outng: %lu",
					c->socket, c->in_count, c->q_ok_count, c->q_ng_count);
	if(msg && *msg)
		syslog(LOG_INFO, "%s", msg);
	
	c->socket = -1;			/* Show ccb as closed */
}

static void *out_threads(void *p)		/* 1 of these for each connect */
{
	connect_t *out = (connect_t *)p;	/* Point to connect structure */

	while(1)							/* Go forever */
	{
		int i, j;
		
		sem_wait(&out->ot_sem);			/* Wait for work to do */
		
		i = hsq_get_used_bytes(&out->oq);/* Get number of bytes to do */
		
		if(i > 0)						/* Work to do? */
		{	
			j = hsq_write_lines_from_queue(&out->oq, out->socket);
			
			if(j < 0)
			{
				close_socket(out, "Output error, closing socket");
				continue;
			}
			
			if(j < i)
				syslog(LOG_INFO, "hsq_write_lines_from_queue %d/%d\n", j, i);
		}
	}
	
	return NULL;						/* Supress warning message */
}
