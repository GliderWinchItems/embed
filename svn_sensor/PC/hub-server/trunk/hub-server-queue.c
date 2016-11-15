/* File: hub-server-queue.c -- Queue/dequeue utilities.
 */
#include <errno.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include "hub-server-queue.h"
#include "hub-server-util.h"

#define	WAIT(sem) do {do {errno = 0; sem_wait(&sem);} while(errno == EAGAIN); } while(1==0);
#define	POST(sem) sem_post(&sem)

void hsq_new(hsq_t *q, int size)
{
	char *me = "hsq_new";
	
	hs_assert(q != NULL && size > 0, me, "bad arg");
	hs_assert(q->low == NULL, me, "already inited");
	
	memset(q, 0, sizeof(hsq_t));
	
	sem_init(&q->sem, 0, 1);
	q->size = size;
	q->low = hs_malloc(size);
	q->high = q->low + size;
	
	q->num = 0;
	q->front = q->low;
	q->rear = q->high;
}

void hsq_del(hsq_t *q)
{
	char *me = "hsq_del";
	
	hs_assert(q != NULL, me, "bad arg");
	hs_assert(q->low != NULL, me, "not inited");
	
	sem_destroy(&q->sem);
	hs_free(q->low);
	memset(q, 0, sizeof(hsq_t));
}

void hsq_flush(hsq_t *q)
{
	char *me = "hsq_flush";
	
	hs_assert(q != NULL, me, "bad arg");
	hs_assert(q->low != NULL, me, "not inited");

	WAIT(q->sem);

	q->num = 0;
	q->front = q->low;
	q->rear = q->high;

	POST(q->sem);	
}

int hsq_enqueue_chars(hsq_t *q, char *buf, int count)
{
	char *me = "hsq_enq_chars";
	int num;
	char *p0, *p1;
	
	hs_assert(q != NULL && buf != NULL && count >= 0, me, "bad arg");
	
	if(count == 0)
		return 0;
	
	WAIT(q->sem);

	if(count > (q->size - q->num))
		count = (q->size - q->num);

	p0 = q->rear;
	p1 = q->high;
	
	for(num = 0; num < count; num += 1)
	{
		if(p0 >= p1)
			p0 = q->low;
		*p0++ = *buf++;
	}
	
	q->rear = p0;
	q->num += num;

	POST(q->sem);
	return num;
}

/* Assertion: (low <= front < high) && (low < rear <= high).
 * Assertion: (0 <= num <= size).
 * Given the above, all cases collapse into 3 cases (go draw some pictures):
 *	0:					0 == num; 			# 0-chunk (empty).
 *	1: front <  rear;	1 <= num <= size;	# 1-chunk (including full).
 *	2: front >= rear;	1 <= num <= size;	# 2-chunks (including full).
 */
int hsq_write_lines_from_queue(hsq_t *q, int fd)
{
	int n;
	struct iovec iov[2];
		
	if(q->num == 0)					/* Case 0: 0-chunk, empty */
		return 0;

	WAIT(q->sem);

	if(q->front < q->rear)			/* Case 1: 1-chunk, partial & full */
		n = write(fd, q->front, q->num);
	else							/* Case 2: 2-chunks, partial & full */
	{
		iov[0].iov_base = q->front;
		iov[0].iov_len = q->high - q->front;		
		iov[1].iov_base = q->low;
		iov[1].iov_len = q->rear - q->low;

		n = writev(fd, iov, 2);		/* Note "gather write" */
	}

	if(n > 0)						/* Update "num" & "front" */
	{
		q->num -= n;
		q->front += n;
		if(q->front >= q->high)
			q->front -= q->size;
	}

	POST(q->sem);
	return n;
}

int hsq_get_used_bytes(hsq_t *q)	/* Get number of "used" bytes in queue */
{
	return q->num;
}

int hsq_get_free_bytes(hsq_t *q)	/* Get number of "free" bytes in queue */
{
	return q->size - q->num;
}
