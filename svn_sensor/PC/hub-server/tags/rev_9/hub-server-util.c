/******************************************************************************/
/* hub-server-util.c -- Implementation -- Utilities for hub-server.
 */
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "hub-server-util.h"

/******************************************************************************/

/* Implementation
 */

/* malloc with error handling
 */
void *hs_malloc(int size)
{
	char *me = "hs_malloc";
    void *p;

    hs_assert(size > 0, me, "bad size");
    p = malloc(size);
    hs_assert(p != NULL, me, "can't get memory");
    return p;
}

void hs_free(void *p)
{
	char *me = "hs_free";

    hs_assert(p != NULL, me, "NULL arg ptr");
	free(p);
}

void hs_assert(int assertion, char *msg0, char *msg1)
{
    if(assertion)
    	return;

	syslog(LOG_ERR, "assert failed: ");
    if(msg0 && *msg0) syslog(LOG_ERR, "%s", msg0);
    if(msg0 && *msg0  && msg1 && *msg1) syslog(LOG_ERR, " -- ");
    if(msg1 && *msg1) syslog(LOG_ERR, "%s", msg1);
    if(msg0 || msg1) syslog(LOG_ERR, "\n");
	hs_exit(10);
}

void hs_fatal(char *msg0, char *msg1)
{
	syslog(LOG_ERR, "hs_fatal: ");
    if(msg0 && *msg0) syslog(LOG_ERR, "%s", msg0);
    if(msg0 && *msg0  && msg1 && *msg1) syslog(LOG_ERR, " -- ");
    if(msg1 && *msg1) syslog(LOG_ERR, "%s", msg1);
    if(msg0 || msg1) syslog(LOG_ERR, "\n");
    hs_exit(11);
}

int hs_send(int sock, char *buf, int n)
{
	while(n > 0)
	{
		int m = send(sock, buf, n, 0);
		if(m < 0)
			return -1;
		buf += m;
		n -= m;
	}
	
	return 0;
}

int hs_recv(int sock, char* buf, int n)
{
	return recv(sock, buf, n, 0);
}

void hs_exit(int code)
{
	syslog(LOG_ERR, "my_exit is calling exit(%d);\n", code);
	exit(code);
}


/******************************************************************************/
