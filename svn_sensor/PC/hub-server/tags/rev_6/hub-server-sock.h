/******************************************************************************/
/* hub-server-sock.h -- Interface -- Socket stuff for hub-server.
 */
#ifndef __HUB_SERVER_SOCK_H_
#define __HUB_SERVER_SOCK_H_
#ifdef _CPLUSPLUS
  extern "C" {
#endif

#include "hub-server-queue.h"

/******************************************************************************/

/* #defines */

#define	HS_MAX_CONNECT	20			/* Maximum number of connects */

#define IN_LINE_SIZE	512			/* Size of input buffer */
#define	OUT_QUEUE_SIZE (256*1024)	/* Size of output queue */
#define	ASCII_LF 		012			/* ADCII -- Line terminator */

/* Local structures */

typedef struct connect_t
{
	int socket;					/* Socket fd */
	int is_server;				/* == 0 -> client, != 0 -> server */
	char *ptibs;				/* Pointer to input buffer start */
	char *ptibe;				/* Pointer to input buffer end */
	char *ptibfs;				/* Pointer to input buffer free space */
	pthread_t ot_tid;			/* Output thread tid */
	sem_t ot_sem;				/* Output thread semaphore */
	hsq_t oq;					/* Output queue structure */
	/* Spare slots for <hub-server-sock-data.c> */
	void *user0;
	void *user1;
	/* Debug members follow */
	unsigned long in_count;		/* Num of bytes read */
	unsigned long q_ok_count;	/* Num of bytes accepted by hsq_enqueue */
	unsigned long q_ng_count;	/* Num of bytes not accepted by hsq_enqueue */
} connect_t;


/* Socket interface
 */
void hs_sock_init(void);

void hs_sock_new_connect(int newsock, int is_server);

/* Data interface
 */
int hsd_new_in_data(connect_t *in, int size);

int hsd_new_in_out_pair(connect_t *in, connect_t *out, int size);

#endif
