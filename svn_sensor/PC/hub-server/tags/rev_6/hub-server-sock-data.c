/******************************************************************************/
/* hub-server-sock-data.c -- Data handling stuff for hub-server-sock.
 */
#include <syslog.h>
#include "hub-server-sock.h"
#include "hub-server-queue.h"

/******************************************************************************/

int hsd_new_in_data(connect_t *in, int size)
{
	return in->is_server ? size : size;		/* Avoid warnings */
}

int hsd_new_in_out_pair(connect_t *in, connect_t *out, int size)
{
	int n = hsq_enqueue_chars(&out->oq, in->ptibs, size);
	
	if(n != size)
		syslog(LOG_INFO, "hsd_new_in_out_pair -- enqueue err, %d/%d\n", n, size);

	return n;
}
