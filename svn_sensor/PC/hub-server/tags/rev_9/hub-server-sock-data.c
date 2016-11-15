/******************************************************************************/
/* hub-server-sock-data.c -- Data handling stuff for hub-server-sock.
 */
#include <syslog.h>
#include "hub-server-sock.h"
#include "hub-server-queue.h"

/******************************************************************************/
/* Local variables.
 */
static connect_t *ccb_base;				/* Save address of ccb[0] here */
static int ccb_max;						/* Save number of ccbs here */

static volatile connect_t *ccb_tmp;		/* Kludge to supress warnings */
static volatile int int_tmp;			/* Kludge to supress warnings */

int hsd_init(connect_t *first_ccb, int num_ccb)
{
	ccb_base = first_ccb;
	ccb_max = num_ccb;
	return 0;
}

int hsd_open(connect_t *ccb)
{
	ccb_tmp = ccb;
	return 0;
}

int hsd_new_in_data(connect_t *in, int size)
{
	ccb_tmp = in;
	int_tmp = size;
	return 0;
}

int hsd_new_in_out_pair(connect_t *in, connect_t *out, int size)
{
	int n = hsq_enqueue_chars(&out->oq, in->ptibs, size);
	
	if(n != size)
		syslog(LOG_INFO, "[%d/%d,%d/%d]hsd_new_in_out_pair -- enqueue err, %d/%d\n",
					in-ccb_base, in->socket, out-ccb_base, out->socket, n, size);
	return n;
}

int hsd_close(connect_t *ccb)
{
	ccb_tmp = ccb;
	return 0;
}
