/******************************************************************************/
/* File: sdlog.h -- Interface for sdcard data logger.
 */

/*
10-08-2011 -- Changes to "sdlog.[ch]".

0. Changed sdlog_write(...) buffer pointer type to "void *".

1. Changed sdlog_read(...) buffer pointer type to "void *".

2. Changed the name of "sdlog_read_seek(...)" to "sdlog_seek(...)".  It doesn't
   make sense to have separate read and write seeks.  Only read seek makes 
   sense.

3. Implemented "sdlog_seek(~0ULL)" to reset "sdlog_read_backwards(...)" to 
   start reading from most recent place where "sdlog_write(...)" is at.  Also
   note that the above call returns SDLOG_OK.  Any other value of the argument 
   returns SDLOG_ERR and does nothing.

07-31-2013 -- Changes to "sdlog.[ch]".

4. Update for corrupted-block recovery.

*/

#ifndef __SDLOG_H_
#define	__SDLOG_H_

#include "sdcard.h"

/* The sdlog package supports logging (writing) and reading back of data packets.
 * A "packet" is a block of data of the length written.  So, you read back the 
 * same packets that you wrote.  Some general notes:
 *
 *	0.	All functions (except sdlog_tell()) return status.  If status is less
 *		than 0, then there's some sort of error.  Equal to 0 is no error.
 *		Greater than zero is no error with some sort of status.  .GT. 0 is
 *		used for returning packet sizes on read.
 *
 *	1.	sdlog_init(), sdlog_seek(...), sdlog_tell() and sdlog_shutdown()
 *		are synchronous and don't return until the operation (if any) is
 *		complete.
 *
 *	2.	sdlog_write(...), sdlog_read(...), and sdlog_read_backwards(...) are
 *		also synchronous; but due to internal buffering, almost always return
 *		immediatly.  In all cases, they return when the data has been
 *		transferred.
 *
 *	3.	The count for sdlog_write(...) must be greater than 0 and less than
 *		SDLOG_WRITE_MAX, which isn't very big (for now, 250).
 *
 *	4.	There is no reason for a read buffer used for sdlog_read(...) or
 *		sdlog_read_backwards(...) to be any bigger than SDLOG_WRITE_MAX.
 *
 *	5.	The sdlog package never looks at the data in the packet.  Packet
 *		type bytes, timestamps, etc. are all private matters of the calling
 *		client.
 */
 
/* Constants
 */
#define	SDLOG_WRITE_MAX		250

#define	SDLOG_SEEK_SET		  0
#define	SDLOG_SEEK_CUR		  1
#define	SDLOG_SEEK_END		  2

#define	SDLOG_OK			  0		/* Read or write OK */
#define	SDLOG_EOF			 -1		/* Read end-of-file */
#define	SDLOG_ERR			 -2		/* Read or write sdcard error */
#define	SDLOG_CORRUPTED		 -3		/* Read corrupted block */

/* sdlog_init() is called once and gets things going.  Do this before any other
 * call.
 */
int sdlog_init(void);

/* sdlog_write(...) writes a "count" character packet located at "buf" to the 
 * log.  Subsequent sdlog_write(...) calls will append the next packet to the
 * end of the stream of log packets.  Count may not be less than 1 or greater
 * than SDLOG_WRITE_MAX.  Subsequent calls to sdlog_read(...) will read 
 * packets starting at the first packet of the stream of log packets.
 * sdlog_write(...) returns SDLOG_OK if all is ok, or SDLOG_ERR if the lower
 * level sdcard routines returned an error.
 */
int sdlog_write(void *buf, int count);

/* sdlog_read_backwards(...) reads the next packet from the input stream and 
 * put the data into "buf".  If "count" is less than SDLOG_WRITE_MAX and the 
 * next packet size is greater than "count", then the "count" bytes of data 
 * are copied to "buf" and the remainder are discarded.  No error is signalled.
 * The size of the packet read is returned, or negative for error.  An
 * attempt to read backwards past the first packet is signalled by returning 
 * a SDLOG_EOF.  An attempt to read a corrupted block returns SDLOG_CORRUPTED.
 * attempting a subsequent sdlog_read_backwards(...) after SDLOG_EOF or
 * SDLOG_CORRUPTED will read the "next block" and attempt to continue.  
 * Whether this results in anything useful is not guaranteed, YMMV.
 * Note that the first read after restart reads the last packet written.
 */
int sdlog_read_backwards(void *buf, int buf_size);

/* sdlog_seek and the two sdlog_tells use a representation of the packet ID
 * (pid) called sdlog_seek_t.  Note that it's a "seek type", which is to remind
 * the programmer that the only thing to do with what's returned by "tell" is to
 * "seek."
 */
typedef unsigned long long sdlog_seek_t;
 
/* sdlog_seek(...) positions the next sdlog_read_backwards(...) to the 
 * packet sequence number specified by "offset".  if "offset" would position
 * the file past the last packet, the  position is quietly set to the last 
 * packet.  If "offset" would position the file before the first packet of 
 * the file, then the position is quietly set to the first packet.
 */
int sdlog_seek(sdlog_seek_t pid);

/* sdlog_read_tell(...) returns the packet ID of the last packet returned
 * by sdlog_read_backwards(...).  sdlog_write_tell(...) returns the packet
 * ID of the last packet written with sdlog_write(...).
 */
sdlog_seek_t sdlog_read_tell(void);
sdlog_seek_t sdlog_write_tell(void);

/* sdlog_flush() is called to flush unwritten data before an intentional
 * system shutdown or whenever the client wants to make sure no buffered data
 * can be lost.  All other state of the sdlog system is preserved.  This
 * means that the system can continue after a call to sdlog_flush().
 */
int sdlog_flush(void);

/******************************************************************************/
/* "Extra Blocks" interface.  The read and write entries read and write fixed
 * length blocks from/to fixed SD-card block addresses.  If blocknum is out of 
 * range (see note below), the called function does an internal ASSERT.  If the 
 * operation completes ok, the called function returns SDLOG_OK, else SDLOG_ERR.
 *
 * Note: 0 <= blocknum < SDLOG_EXTRA_BLOCKS;
 */
#define	SDLOG_EXTRA_BLOCKS	2048

int sdlog_read_extra_block(int blocknum, char buf[SDCARD_BLOCKSIZE]);
int sdlog_write_extra_block(int blocknum, char buf[SDCARD_BLOCKSIZE]);

/******************************************************************************/
/* Debugging
 */
int sdlog_debug_get_card_size_in_blocks(void);
void sdlog_debug_set_card_size_in_blocks(int);

#endif
