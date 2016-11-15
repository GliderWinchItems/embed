/******************************************************************************/
/* sdcard_ll.h -- Interface for SD card low level stuff
 */
#ifndef _SDCARD_LL_H
#define	_SDCARD_LL_H

/* External interface -- All functions are synchronous.
 *
 * Note: Function names contain the strings "read" and "write".  These terms
 * are host-relative.  "Read" is a transfer from SDCard to host and "write" 
 * is a transfer from host to SDCard.
 *
 * Start by calling sdcard_ll_init().
 *
 * Commands return -1 for timeout, or they return the response bytes (up to
 * 3 of 'em in an int, which is == 0 for no-error or > 0 for error.  See the
 * error defines below.
 *
 *
 * Note that sdc_get_last_response(...) returns the 
 * response in an int.  For response type R1, the response occupies the low-
 * order 8 bits of the int.  For response type R2, the extra 8 bits of the
 * response occupies the next 8 bits (for a total of 16).  Certain other
 * commands return status in the next 8 bits (for a total of 24).  The rest of
 * the bits are 0.
 *
 * Note that sdc_get_last_response(...) gets the response from the last
 * command only.  All previous responses are lost.
 *
 * Note that for all read and write operations, the user buffer is fixed size.
 * The client should use the #defines below.
 */

#ifndef NULL
	#define	NULL	((void*)0)
#endif

/* User buffer sizes.
 */
#define	SDC_COND_SIZE		4
#define	SDC_CSD_SIZE		16
#define	SDC_CID_SIZE		16
#define	SDC_STATUS_SIZE		64
#define	SDC_DATA_SIZE		512
#define	SDC_OCR_SIZE		4

/* Error codes.
 */
#define SDCARD_LL_OK		 0
#define	SDCARD_LL_TIMEOUT1	((~0xf) | 1)
#define	SDCARD_LL_TIMEOUT2	((~0xf) | 2)
#define	SDCARD_LL_TIMEOUT3	((~0xf) | 3)
#define	SDCARD_LL_TIMEOUT4	((~0xf) | 4)
#define	SDCARD_LL_TIMEOUT5	((~0xf) | 5)
#define	SDCARD_LL_TIMEOUT6	((~0xf) | 6)
#define	SDCARD_LL_TIMEOUT7	((~0xf) | 7)
#define	SDCARD_LL_TIMEOUT8	((~0xf) | 8)
#define	SDCARD_LL_CHECKSUM	((~0xf) | 9)

/* Supported commands
 */
#define	ACMD				0x40		/* ACMDn is (ACMD+n) */

#define	CMD0				 0			/* GO_IDLE_STATE */
#define	CMD1				 1			/* SEND_OP_COND */
#define	CMD8				 8			/* SEND_IF_COND */
#define	CMD9				 9			/* SEND_CSD */
#define	CMD10				10			/* SEND CID */
#define	CMD13				13			/* SEND_STATUS */
#define	CMD16				16			/* SET_BLOCKLEN */
#define	CMD17				17			/* READ_SINGLE_BLOCK */
#define	CMD24				24			/* WRITE_BLOCK */
#define	CMD28				28			/* SET_WRITE_PROT (test use only) */
#define	CMD29				29			/* CLR_WRITE_PROT */
#define	CMD32				32			/* ERASE_WR_BLK_START_ADDR */
#define	CMD33				33			/* ERASE_WR_BLK_END_ADDR */
#define	CMD38				38			/* ERASE */
#define	CMD58				58			/* READ_OCR */
#define	CMD59				59			/* CRC_ON_OFF */
#define	ACMD13				(ACMD+13)	/* SD_STATUS */
#define	ACMD41				(ACMD+41)	/* SD_SEND_OP_COND */

/* r1/r2/r1x errors.
 */
#define	R1_R2_R1X_OK		0x000000

#define	R1_MASK				0x0000FF
#define	R1_IN_IDLE_STATE	0x000001
#define	R1_ERASE_RESET		0x000002
#define	R1_ILLEGAL_CMD		0x000004
#define	R1_COM_CRC_ERR		0x000008
#define	R1_ERASE_SEQ_ERR	0x000010
#define	R1_ADDRESS_ERR		0x000020
#define	R1_PARAM_ERR		0x000040
#define	R1_ALWAYS_0			0x000080

#define	R2_MASK				0x00FF00
#define	R2_CARD_LOCKED		0x000100
#define	R2_WP_ERASE_SKIP	0x000200
#define	R2_ERROR			0x000400
#define	R2_CARD_CNTL_ERR	0x000800
#define	R2_CARD_ECC_FAIL	0x001000
#define	R2_WP_VIOLATON		0x002000
#define	R2_ERASE_PARAM		0x004000
#define	R2_OOR_O_CSDOW		0x008000

#define	R1X_MASK			0xFF0000
#define	R1X_IN_IDLE_STATE	0x010000
#define	R1X_ERASE_RESET		0x020000
#define	R1X_ILLEGAL_CMD		0x040000
#define	R1X_COM_CRC_ERR		0x080000
#define	R1X_ERASE_SEQ_ERR	0x100000
#define	R1X_ADDRESS_ERR		0x200000
#define	R1X_PARAM_ERR		0x400000
#define	R1X_ALWAYS_0		0x800000

/* Initialization.  Call this once.
 * Returns == SDCARD_LL_OK for no error, != SDCARD_LL_OK for errors.
 */
int sdcard_ll_init(void);

/* Sunchronous command dispatcher.  If buf and num are not needed, they must
 * be NULL & 0 respectively.  Note that a read command will always read num
 * bytes.
 *
 * cmd -- Command number {+ a_flag} -- a_flag == 64 forces cmd55 prefix.
 * arg -- 4-byte command argument.
 * buf -- Pointer to I/O buffer (buffer size is implied by the command).
 *
 * Returns the value of (r1) or ((r2<<8)|r1) as appropriate.
 */
int sdcard_ll_cmd(int cmd, unsigned long arg, void *buf);

/* Utility stuff
 */
int  sdcard_ll_get_resp(void);		/* Return r1/r2/r1 from last command */
void sdcard_ll_set_ccsf(int flag);	/* Set card-capacitiy support flag */

/* Debug stuff
 */
void sdc_debug(int data);			/* Put data in the debug queue */
int sdc_get_crc(void);				/* Get last crc from/to sdcard */

#endif
