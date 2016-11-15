/* File: hub-server-queue.h -- Queue/dequeue utilities.
 */
#ifndef	__HUB_SERVER_QUEUE_H
#define	__HUB_SERVER_QUEUE_H

#include <semaphore.h>
#include <sys/uio.h>

typedef struct
{
	sem_t sem;						/* Local queue manipulation semaphore */
	int size;						/* Size of queue buffer */
	int num;						/* Number of data chars in queue */
	char *low, *high;				/* Queue buffer low/high addresses */
	char *front, *rear;				/* Data front/rear(+1) addresses */
} hsq_t;							/* Hub server queue stuff */

void hsq_new(hsq_t *q, int size);	/* Create a queue structure */
void hsq_del(hsq_t *q);				/* Delete a queue structure */

int hsq_enqueue_chars(hsq_t *q, char *buf, int count);
int hsq_write_lines_from_queue(hsq_t *q, int fd);

int hsq_get_used_bytes(hsq_t *q);	/* Get "number of used bytes in queue" */
int hsq_get_free_bytes(hsq_t *q);	/* Get "number of free bytes in queue" */

#endif
