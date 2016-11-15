/******************************************************************************
* File Name          : timer_thread.c
* Date First Issued  : 12/03/2014
* Board              : PC
* Description        : timer
*******************************************************************************/

/* Dependencies 
 */
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/select.h>

/******************************************************************************/
/* Global */
unsigned int timer_thread_ctr;	// Count select timeouts


/* Local variables */
static pthread_t timer_thread;	// thread in which user timer functions execute
static void *timerthread(void *_);


static	struct timeval TMDETECT; //  timeout struct
static	struct timeval tmdetect;
static	int	select_timeout;
static	fd_set	ready;		/* used for select */
static	void 	(*funcptr)(void) = 0;	// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority)

/* Initialize timer */
static int timer_oto = 0;	// Prevent multiple init's
/******************************************************************************
 * int timer_thread_init(void* func(void), int rate);
 * @brief 	: Start new thread if not started, set select timeout, run global var counter
 * @param	: func(void) = Pointer to function to call upon select timeout, 
 *		:   or NULL if callback not needed.
 * @param	: rate = timeout count (microsecs)
*******************************************************************************/
int timer_thread_init(void* func, int rate) 
{
	funcptr = func;
	select_timeout = rate;
	TMDETECT.tv_sec = 0;
	TMDETECT.tv_usec = select_timeout; //  timeout (usec)

	/* One time set up */
	if (timer_oto != 0) return 0 ;
	timer_oto = 1;

	int ret = pthread_create(&timer_thread, (pthread_attr_t*)0, timerthread, (void*)0);
	if (ret != 0) // Mostly for debugging
	{
		printf("TIMER THREAD DID NOT CREATE: return: %d, errno: %d\n\tTXT: ",ret, errno);
		{
			if (errno == EAGAIN) printf("Insufficient resources to create another thread, or a system-imposed limit on  the  number  ofthreads  was  encountered\n");
			if (errno == EINVAL) printf("Invalid settings in attr.\n");
			if (errno == EPERM ) printf("No permission to set the scheduling policy and parameters specified in attr.\n");
			return -1;
		}
	}
	return 0;
}
/******************************************************************************
 * void timer_thread_shutdown(void);
 * @brief 	: Shutdown timer and cancel thread
*******************************************************************************/
/* Shut timer down */
void timer_thread_shutdown(void) 
{
	if (timer_oto == 0) return;
	timer_oto = 0;
	pthread_cancel(timer_thread);
	return;
}

/* Thread starts to run here. */
static void *timerthread(void *_) 
{
	int ret;
	while(1==1)
	{
		FD_ZERO(&ready);		/* Clear file descriptors */
		tmdetect = TMDETECT;		/* Refresh timeout timer */
	
		ret = select (0, &ready, NULL, NULL, &tmdetect);	/* Wait for something to happen */

		timer_thread_ctr += 1;	// Running count of timeouts
		
		if (ret < 0)	// Mostly for debugging
		{
			printf("SELECT ERROR: return: %d, errno: %d\n\tTXT: ",ret, errno);
			if (errno == EBADF) printf("An  invalid  file  descriptor  was  given  in  one of the sets.\n\t (Perhaps a file descriptor that was already closed, or one on which an error has occurred.)\n");
			if (errno == EINTR)  printf("A signal was caught; see signal(7).\n");
			if (errno == EINVAL) printf("nfds is negative or the value contained within timeout is invalid.\n");
			if (errno == ENOMEM) printf("unable to allocate memory for internal tables.\n");
		}

		if (funcptr != 0)	// Having no address for the following is bad.
			(*funcptr)();	// Go do something
	}
	return NULL;
}

