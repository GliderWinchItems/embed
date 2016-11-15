/******************************************************************************/
/* hub-server-util.c -- Implementation -- Utilities for hub-server.
 */
#include <stdio.h>
#include <stdlib.h>
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

	printf("assert failed: ");
	if(msg0 && *msg0) printf("%s -- ", msg0);
	if(msg1 && *msg1) printf("%s", msg1);
	printf("\n");
	exit(EXIT_FAILURE);
}

void hs_fatal(char *msg0, char *msg1)
{
	printf("hs_fatal: ");
    if(msg0 && *msg0) printf("%s -- ", msg0);
    if(msg1 && *msg1) printf("%s", msg1);
    if(msg0 || msg1) printf("\n");
    exit(EXIT_FAILURE);
}

void hs_send(int sock, char *buf, int n)
{
	while(n > 0)
	{
		int m = send(sock, buf, n, 0);
		if(m < 0)
			hs_fatal("hs_send", "error");
		buf += m;
		n -= m;
	}
}

int hs_recv(int sock, char* buf, int n)
{
	return recv(sock, buf, n, 0);
}

/******************************************************************************/
