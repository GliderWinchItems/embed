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
#include <netinet/tcp.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#include "hub-server-util.h"
#include "hub-server-sock.h"
#include "hub-server-queue.h"

/******************************************************************************/
/* Externals */
extern char eol;					/* End-of-line character */
extern char dle;					/* Data-link-escape character */
extern int dle_flag;				/* Data-link-excape char set flag */
extern int cs_mode;					/* Client/server mode flag */
extern int nodelay_flag;			/* Get rid of Nagle delay */

/* Local variables */

static pthread_t connect_in_tid;
static long select_seqn = 0;

static connect_t ccb[HS_MAX_CONNECT];	/* Connection control blocks */

#define	LOOP(p) for(p=ccb+0; p<ccb+HS_MAX_CONNECT; p++)
#define	IS_FREE(p) (p->socket < 0)
#define	IS_CLOSING(p) (p->is_closing)

/* Local function declarations */

static void *in_thread(void *);
static void *out_threads(void *);
static void close_socket(connect_t *c, char *msg);

/* Global functions */

/* Initialize 1 input thread and 1 output thread per ccb.
 */
void hs_sock_init(void)			/* Called from the other thread */
{
	connect_t *p;
	
	signal(SIGPIPE, SIG_IGN);	/* Avoid exit(141) during TCP socket close */

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
	
	hsd_init(ccb+0, HS_MAX_CONNECT);

	if(pthread_create(&connect_in_tid, NULL, in_thread, NULL) != 0) 
		hs_fatal("hs_sock_init", "Failed to create socket in thread");
}

/* Create and populate a new connect control block.  Link it onto the front of the
 * list of connect control blocks.
 */
void hs_sock_new_connect(int newsock, int is_server)
{
	connect_t *p;
	int state = 1;
	char *nodelay_msg = "TCP_NODELAY is off";
	
	LOOP(p)
		if(IS_FREE(p))
		{
			p->socket = newsock;
			p->is_server = is_server;
			p->is_closing = (1==0);
			p->ptibfs = p->ptibs;
			p->in_count = 0;
			p->q_ok_count = 0;
			p->q_ng_count = 0;
			p->user0 = NULL;
			p->user1 = NULL;
			
			if(nodelay_flag)
			{
				if(0 != setsockopt(newsock, IPPROTO_TCP, TCP_NODELAY,
												&state, sizeof(state)))
					syslog(LOG_INFO, "[%d/%d]Can't set TCP_NODELAY, errno is %d\n", 
											p-ccb, newsock, errno);
				else
					nodelay_msg = "TCP_NODELAY is ON";
			}

			hsd_open(p);			/* Tell the data interface */
			
			syslog(LOG_INFO, "[%d/%d]hs_sock_new_connect, %s\n", 
													p-ccb, newsock, nodelay_msg);
			break;
		}

	if(p == ccb + HS_MAX_CONNECT)
	{
		syslog(LOG_INFO, "[-/%d]hs_sock_new_connect -- too many connects\n", newsock);
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
	char *ptch;								/* Ptr to a char for finding eof */
	char *base;								/* Low address of new data */

	/* Loop forever
	 */
	while(1==1)
	{
		int ret;
		
		/* Build fd set for recv(...)
		 */
		FD_ZERO(&recv_fd);
		connect_fd_max = 0;
		LOOP(in)
		{
			if(IS_FREE(in) || IS_CLOSING(in)) continue;

			if(connect_fd_max < in->socket)
				connect_fd_max = in->socket;
			FD_SET(in->socket, &recv_fd);
		}

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		/* Wait for input or timeout
		 */
		ret = select(connect_fd_max+1, &recv_fd, NULL, NULL, &timeout);
		if (ret < 0) continue;
		select_seqn += 1;

		if(1==0)		
			if(ret == 0  ||  (select_seqn % 2000) == 0)
				syslog(LOG_DEBUG, "Idle5 %ld\n", select_seqn);

		/* Loop through all active connects looking for work to do.
		 */
		LOOP(in)							/* Visit all connects */
		{
			if(IS_FREE(in) || IS_CLOSING(in)) continue;

			/* If input is ready for this connect...
			 */
			if(FD_ISSET(in->socket, &recv_fd))
			{
				/* Append new input to end of existing input.
				 */
				base = in->ptibfs;
				tmp = in->ptibe - base;
				n = hs_recv(in->socket, base, tmp);
				if(n > tmp) 
					syslog(LOG_DEBUG, "[%d/%d]Huh(0)? %d/%d\n", 
							in-ccb, in->socket, n, tmp);
				if(n <= 0)
					close_socket(in, "disconnect or hs_recv err");
				else
				{
					in->in_count += n;		/* Accumulate stats */
					in->ptibfs += n;		/* Adjust free space ptr */

					/* Search backwards for last end-of-line char.
					 */
					for(ptch = in->ptibfs; ptch > base; )
					{
						if(*--ptch == eol)
						{
							if(!dle_flag) break;

							if(ptch > in->ptibs  &&  *(ptch-1) != dle) break;
						}
					}

					if(*ptch++ == eol)		/* Full line(s)? */
					{
						int size = ptch - in->ptibs;
						
						if(size < 0 || size > IN_LINE_SIZE)
							syslog(LOG_DEBUG, "[%d/%d]Huh(5)? %d\n", 
									in-ccb, in->socket, n);

						/* Send line(s) to all connect(s) except ourselves.
						 * In client/server mode, clients <==> servers.
						 */
						hsd_new_in_data(in, size);

						LOOP(out)				/* Visit all connects */
						{
							if(IS_FREE(out) || IS_CLOSING(out)) continue;

							if(cs_mode)			/* clients <==> servers */
							{
								if( in->is_server &&  out->is_server) continue;
								if(!in->is_server && !out->is_server) continue;
							}
							else				/* Don't talk to ourselves */
							{
								if(in == out) continue;
							}
			
							n = hsd_new_in_out_pair(in, out, size);
							if(n < 0 || n > size)
								syslog(LOG_DEBUG, "[%d/%d/%d/%d]Huh(1)? %d/%d\n", 
									in-ccb, in->socket, out-ccb, out->socket, 
									n, size);

							out->q_ok_count += n;
							out->q_ng_count += size - n;

							sem_post(&out->ot_sem);
						}

						/* Copy unsent bytes at end down to start of buffer
						 */
						n = in->ptibfs - ptch;
						if(n < 0 || n > IN_LINE_SIZE)
							syslog(LOG_DEBUG, "[%d/%d]Huh(2)? %d\n", 
									in-ccb, in->socket, n);
						if(n > 0)
							memmove(in->ptibs, ptch, n);
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
	if(c->socket < 0 || c->socket > (HS_MAX_CONNECT+3))
		syslog(LOG_DEBUG, "[%d/%d]Huh(3)?\n", c-ccb, c->socket);
	
	if(msg && *msg)
		syslog(LOG_INFO, "[%d/%d]close_socket: %s", c-ccb, c->socket, msg);

	c->is_closing = (1==1);	/* Set closing flag */
	sem_post(&c->ot_sem);
}

static void *out_threads(void *p)		/* 1 of these for each connect */
{
	connect_t *out = (connect_t *)p;	/* Point to connect structure */

	while(1)							/* Go forever */
	{
		int i, j;
		
		sem_wait(&out->ot_sem);			/* Wait for work to do */
		
		if(IS_FREE(out)) continue;
		
		if(IS_CLOSING(out))
		{
			hsd_close(out);				/* Tell the data interface */
			
			close(out->socket);			/* Close connection */
			
			syslog(LOG_INFO, "[%d/%d]out_threads close: in: %.1e, outok: %.1e, outng: %.1e",
							out-ccb, out->socket, 
							out->in_count, out->q_ok_count, out->q_ng_count);
		
			out->socket = -1;			/* Show ccb as closed */
			out->is_closing = (1==0);	/* Reset closing flag */
			hsq_flush(&out->oq);		/* Toss any queued data */
			continue;					/* Go wait for work to do */
		}

		i = hsq_get_used_bytes(&out->oq);/* Get number of bytes to do */

		if(i > 0)						/* Work to do? */
		{	
			j = hsq_write_lines_from_queue(&out->oq, out->socket);
			
			if(j < 0)
			{
				syslog(LOG_DEBUG, "[%d/%d]Huh(4)? %d/%d\n", 
						out-ccb, out->socket, j, i);

				close_socket(out, "Output error, closing socket");
				continue;
			}
			
			if(j < i)
				syslog(LOG_INFO, "[%d/%d]hsq_write_lines_from_queue: %d/%d\n",
							out-ccb, out->socket, j, i);
		}
	}
	
	return NULL;						/* Supress warning message */
}
