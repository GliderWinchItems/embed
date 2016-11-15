/******************************************************************************/
/* hub-server-util.h -- Interface -- Utilities for hub-server.
 */
#ifndef __HUB_SERVER_UTIL_H_
#define __HUB_SERVER_UTIL_H_
#ifdef _CPLUSPLUS
  extern "C" {
#endif

/* Dependencies 
 */
#include <pthread.h>
#include <semaphore.h>

/******************************************************************************/

/* Interface
 */

/* Utility stuff
 */
void *hs_malloc(int size);
/* Allocate size bytes of memory (size > 0).  If (size <= 0) or the requested
 * memory isn't available, then hs_fatal(...) is called.
 */
 
void hs_free(void *p);
/* Free up memory allocated with hs_malloc(...).
 */

void hs_assert(int assertion, char *msg0, char *msg1);
/* Test the assertion.  If true, return.  Otherwise call someting like:
 * hs_fatal("hs_assert(...) -- assertion failed --" msg);
 */

void hs_fatal(char *msg0, char *msg1);
/* Log up to 2 messages and exit(EXIT_FAILURE).  Either msg0 or msg1 or both
 * (although this seems unlikely) can be NULL.
 */

void hs_send(int sock, char *buf, int n);
/* Loop until all n chars are read into buf.
 */
 
int hs_recv(int sock, char* buf, int n);
/* Try to receive one time and rethrn how many characters read.
 */

#ifdef _CPLUSPLUS
    }
#endif
#endif

/******************************************************************************/

