/******************************************************************************/
/* sdcard_ll.c -- Implementation of SD card low level stuff
 */
#include "sdcard_ll.h"
#include "../devices/spi2sdcard.h"
#include "../devices/PODpinconfig.h"

/* Local variables & macros:
 */
#define	R1B_TIMEOUT	50000
#define	WRITE_TIMEOUT  150000				/* 10,000 -> 88,000 usec. */
#define	GET_R1_TIMEOUT	50000
#define	GET_FE_TIMEOUT	50000

static char init_flag = (1==0);				/* Initialization flag */
static char ccs_flag = (1==0);				/* SDHC or SDXC flag */
static unsigned char cmd_buf[6];			/* Command buffer */
static unsigned char res_buf[4];			/* Response buffer */
static unsigned char crc_buf[2];			/* CRC buffer */
static unsigned char tmp_buf[10];			/* temporary 10 char buffer */

/* Local functions:
 */
static void	send_cmd(int cmd, long arg);
static int get_r1(unsigned char *buf);
static int get_fe(void);
static int clr_cs(int arg);
static void my_spi_read (unsigned char *buf, int size);
static void my_spi_write(unsigned char *buf, int size);
static unsigned char crc7(unsigned char crc, unsigned char *buf, int len);
static unsigned short crc_ccitt(unsigned short crc, unsigned char *ptr, unsigned int ct);

/* Debug stuff
 */
static volatile int ll_HANG_flag = (1==1);
int caw_sdcard_ll_HANG(int arg) { do {} while(ll_HANG_flag); return arg;}
#define assert(bool) do if(!(bool)) caw_sdcard_ll_HANG(__LINE__); while(1==0)

/******************************************************************************/
int sdcard_ll_init(void)
{
	init_flag = (1==1);
	ccs_flag = (1==0);

    spi2sdcard_init();          // Initialize hardware (spi2sdcard.c)

//spi2_poi();                 // Power On Insertion (reset)
//SDCARD_CS_hi;				/* Set chip select false */
//return SDCARD_LL_OK;

	SDCARD_CS_hi;				/* Set chip select false */
	my_spi_read(tmp_buf, 10);	/* Exactly 80 clocks (for unknown reasons) */		
	SDCARD_CS_low;				/* Set chip select true */
	my_spi_read(tmp_buf, 10);	/* Another 80 clocks (for unknown reasons) */
	SDCARD_CS_hi;				/* Set chip select false */

	return SDCARD_LL_OK;
}

/******************************************************************************/
int sdcard_ll_cmd(int cmd, unsigned long arg, void *buf)
{
	int i;						/* Scratch loop counter */
	static unsigned char fe[1] = {0xfe}; 
	unsigned short crc;

	assert(init_flag);
	
	res_buf[0] = res_buf[1] = res_buf[2] = res_buf[3] = 0;
	crc_buf[0] = crc_buf[1] = 0;

	SDCARD_CS_low;				/* Set chip select true */
	my_spi_read(tmp_buf, 1);	/* This delay is required!! */

	switch(cmd)
	{
		/* r1 expected, no data.
		 */
		case CMD32: 	/* ERASE_WR_BLK_START_ADDR (r1 + 0d) */
		case CMD33: 	/* ERASE_WR_BLK_END_ADDR   (r1 + 0d) */
			if(!ccs_flag) arg <<= 9;
		case CMD0: 		/* GO_IDLE_STATE           (r1 + 0d) */
		case CMD1: 		/* SEND_OP_COND            (r1 + 0d) */
		case CMD16: 	/* SET_BLOCK_LEN           (r1 + 0d) */
		case CMD59:		/* CRC_ON_OFF			   (r1 + 0d) */
		case ACMD41:	/* SD_SEND_OP_COND 		   (r1 + 0d) ?? */
			assert(buf == NULL);
			send_cmd(cmd, arg);
			return clr_cs(get_r1(res_buf+0) ? SDCARD_LL_TIMEOUT1
											: sdcard_ll_get_resp());

		/* r2 expected, no data.
		 */
		case CMD13: 	/* SEND_STATUS             (r2 + 0d) */
		case ACMD13:	/* SD_STATUE               (r2 + 0d) */
			send_cmd(cmd, arg);
			if(get_r1(res_buf+0))			/* Get the r1 byte */
				return clr_cs(SDCARD_LL_TIMEOUT1);
			my_spi_read(res_buf+1, 1);		/* Get the r2 byte */
			return clr_cs(sdcard_ll_get_resp());

		/* r1 expected followed by 4 bytes of data.
		 */
		case CMD8: 		/* SEND_IF_COND 	(r7 == (r1 + 4d)) */
		case CMD58:		/* READ_OCR     	(r3 == (r1 + 4d)) */
			assert(buf != NULL);
			send_cmd(cmd, arg);
			if(get_r1(res_buf))					/* Get the r1 byte */
				return clr_cs(SDCARD_LL_TIMEOUT1);
			my_spi_read(buf, SDC_COND_SIZE);	/* Get the data */
			return clr_cs(sdcard_ll_get_resp());
		
		/* r1 expected followed by 16 bytes of data.
		 */
		case CMD9: 		/* SEND_CSD 	(cmd/r1/0xff/0xfe/16d/crc) */
		case CMD10: 	/* SEND_CID 	(cmd/r1/0xff/0xfe/16d/crc) */
			assert(buf != NULL);
			send_cmd(cmd, arg);
			if(get_r1(res_buf))
				return clr_cs(SDCARD_LL_TIMEOUT1);
			if((*res_buf & R1_ILLEGAL_CMD) != 0)
				return clr_cs(sdcard_ll_get_resp());
			if(get_fe())
				return clr_cs(SDCARD_LL_TIMEOUT2);
			my_spi_read(buf, SDC_CSD_SIZE);	/* Get the data */
			my_spi_read(crc_buf, sizeof(crc_buf));
			return clr_cs(sdcard_ll_get_resp());

		/* Read data block.
		 */
		case CMD17:		/* READ_SINGLE_BLOCK (cmd/r1/0xff/0xfe/512d/crc) */
			assert(buf != NULL);
			if(!ccs_flag) arg <<= 9;
			send_cmd(cmd, arg);
			if(get_r1(res_buf+0))
				return clr_cs(SDCARD_LL_TIMEOUT1);
			if(*(res_buf+0) != 0)
				return clr_cs(sdcard_ll_get_resp());
			if(get_fe())
				return clr_cs(SDCARD_LL_TIMEOUT2);
			my_spi_read(buf, SDC_DATA_SIZE);
			my_spi_read(crc_buf, sizeof(crc_buf));

			crc = (crc_buf[0] << 8) | crc_buf[1];
			if(crc != crc_ccitt(0x0000, buf, SDC_DATA_SIZE))
				return SDCARD_LL_CHECKSUM;

			return clr_cs(sdcard_ll_get_resp());

		/* Write data block.
		 */
		case CMD24:		/* WRITE_BLOCK (cmd + r1 + tok/512d/2crc?? + tok/busy) */
			assert(buf != NULL);
			if(!ccs_flag) arg <<= 9;
			crc = crc_ccitt(0x0000, buf, SDC_DATA_SIZE);
			crc_buf[0] = (unsigned char)((crc >> 8) & 0xff);
			crc_buf[1] = (unsigned char)((crc >> 0) & 0xff);
			send_cmd(cmd, arg);
			if(get_r1(res_buf+0))
				return clr_cs(SDCARD_LL_TIMEOUT1);
			my_spi_write(fe, 1);
			my_spi_write(buf, SDC_DATA_SIZE);
			my_spi_write(crc_buf, sizeof(crc_buf));

			for(i=0; i<R1B_TIMEOUT; i+=1)	/* Get data response token */
			{
				my_spi_read(tmp_buf, 1);
				if((*tmp_buf & 0x11) == 0x01) break;
			}
			if(i == R1B_TIMEOUT)
				return clr_cs(SDCARD_LL_TIMEOUT2);

			*tmp_buf &= 0x0f;				/* Low 4 bits of data token */
			if(*tmp_buf != ((0x2<<1)|1))	/* '010' - Data accepted */
			{
				*(res_buf+3) = *tmp_buf; 	/* crc or write error */
				return SDCARD_LL_CHECKSUM;
			}

			for(i=0; i<WRITE_TIMEOUT; i+=1)	/* Spin ignoring busy bytes */
			{
				my_spi_read(tmp_buf, 1);
				if(*tmp_buf != 0) break;	/* == 0 -> busy, != 0 -> notbusy */
			}
			if(i == WRITE_TIMEOUT)
				return clr_cs(SDCARD_LL_TIMEOUT3);
			
			return clr_cs(sdcard_ll_get_resp());
			
		/* No data followed by r1b.
		 */
		case CMD28:						/* SET_WRITE_PROT (cmd + r1b) */
		case CMD29:						/* CLR_WRITE_PROT (cmd + r1b) */
			if(ccs_flag)				/* SDHC or SDXC? */
				return SDCARD_LL_OK;	/* Yes.  Quietly return OK. */
			arg <<= 9;					/* Conv. blockno to byte offset */
		case CMD38:						/* ERASE          (cmd + r1b) */
			assert(buf == NULL);
			send_cmd(cmd, arg);
			if(get_r1(res_buf+0))
				return clr_cs(SDCARD_LL_TIMEOUT1);

			for(i=0; i<R1B_TIMEOUT; i+=1)	/* Spin ignoring busy bytes */
			{
				my_spi_read(tmp_buf, 1);
				if(*tmp_buf != 0) break;	/* == 0 -> busy, != 0 -> notbusy */
			}
			if(i == R1B_TIMEOUT)
				return clr_cs(SDCARD_LL_TIMEOUT2);

			return clr_cs(sdcard_ll_get_resp());
			
		default:
			assert(1==0);
	}
	
	return SDCARD_LL_OK;					/* Dummy to supress warning msg */
}

/******************************************************************************/
int sdcard_ll_get_resp(void)
{
	return (res_buf[0] <<  0) |			/* r1 for command */
		   (res_buf[1] <<  8) |			/* r2 for commands that need it */
		   (res_buf[2] << 16) |			/* r1 for write data (aka "r1d") */
		   (res_buf[3] << 24    );		/* Data response token from write */
}

void sdcard_ll_set_ccsf(int flag)
{
	ccs_flag = flag;
}

/******************************************************************************/
/* Local utilities
 */
static unsigned char cmd55[6] = {0x40+55, 0, 0, 0, 0, 0x65};

static void	send_cmd(int cmd, long arg)
{
 	cmd_buf[0] = ((cmd) & 0x3f) | 0x40;	/* Note ACMD == 0x40 */
	cmd_buf[1] = (((arg) >> 24) & 0xff);
	cmd_buf[2] = (((arg) >> 16) & 0xff);
	cmd_buf[3] = (((arg) >>  8) & 0xff);
	cmd_buf[4] = (((arg) >>  0) & 0xff);
	cmd_buf[5] = (crc7(0x00, cmd_buf, 5) << 1) | 1;

	if((cmd & ACMD) != 0)				/* ACMD prefixes CMD55 */
	{
		my_spi_write(cmd55, 6);			/* Write the prefix command */
		get_r1(res_buf+2);				/* Scan r1 into res_buf[2] */
		my_spi_read(tmp_buf, 4);		/* Delay */
	}

	my_spi_write(cmd_buf, 6);			/* Write the base command */
}

static int get_r1 (unsigned char *buf)
{
	int i, imax = GET_R1_TIMEOUT;		/* Usually no more than 1 loop */
	
	for(i=0; i<imax; i+=1)
	{
		my_spi_read(buf, 1);
		if((*buf & R1_ALWAYS_0) == 0)
			return SDCARD_LL_OK;
	}

	return SDCARD_LL_TIMEOUT8;
}

static int get_fe (void)
{
	int i, imax = GET_FE_TIMEOUT;		/* Usually no more than 1 loop */

	for(i=0; i<imax; i+=1)
	{
		my_spi_read(tmp_buf, 1);
		if(*tmp_buf == 0xfe)
			return SDCARD_LL_OK;
	}

	return SDCARD_LL_TIMEOUT7;
}

static int clr_cs(int arg)				/* Assert CS false & pass arg through */
{
	SDCARD_CS_hi;
	return arg;
}

/******************************************************************************/
/* Local SPI read/write interface.
 */
extern volatile char xmit2_busy_flag;		/* For direct calls to spi2xxx() */

static void my_spi_read (unsigned char *buf, int size)
{
	assert(size > 0);
	spi2_read ((char *)buf, size, (char)0xff, NULL);
	do {} while(xmit2_busy_flag); 
}

static void my_spi_write(unsigned char *buf, int size)
{
	assert(size > 0);
	spi2_write((char *)buf, size, NULL);
	do {} while(xmit2_busy_flag); 
}

/******************************************************************************/
/* Kinda low level debugging stuff
 */
int sdc_get_crc(void)
{
	return (crc_buf[0] << 8) | crc_buf[1];
}

/******************************************************************************/
/* Very low level debugging stuff
 */
typedef struct { int count; int data; } sdc_debug_t;
#define	SDC_DEBUG_SIZE	100
sdc_debug_t *sdc_pt_queue = 0;
sdc_debug_t sdc_queue[SDC_DEBUG_SIZE] = {{0, 0}};

void sdc_debug(int data)
{
	if(!sdc_pt_queue)
	{
		sdc_pt_queue = sdc_queue;
		sdc_pt_queue->count = 1;
		sdc_pt_queue->data = data;
		return;
	}
	
	if(sdc_pt_queue->data == data)
	{
		sdc_pt_queue->count += 1;
		return;
	}
	
	sdc_pt_queue += 1;
	if(sdc_pt_queue == sdc_queue + SDC_DEBUG_SIZE)
		sdc_pt_queue = sdc_queue;

	sdc_pt_queue->count = 1;
	sdc_pt_queue->data = data;
}
/******************************************************************************/
/*
 *      crc7.c
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
 */

/* Table for CRC-7 (polynomial x^7 + x^3 + 1) */
const unsigned char crc7_syndrome_table[256] = {
	0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
	0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
	0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
	0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
	0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
	0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
	0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
	0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
	0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
	0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
	0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
	0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
	0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
	0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
	0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
	0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
	0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
	0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
	0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
	0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
	0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
	0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
	0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
	0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
	0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
	0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
	0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
	0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
	0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
	0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
	0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
	0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

static inline unsigned char crc7_byte(unsigned char crc, unsigned char data)
{
	return crc7_syndrome_table[(crc << 1) ^ data];
}

/**
 * crc7 - update the CRC7 for the data buffer
 * @crc:     previous CRC7 value
 * @buffer:  data pointer
 * @len:     number of bytes in the buffer
 * Context: any
 *
 * Returns the updated CRC7 value.
 */
static unsigned char crc7(unsigned char crc, unsigned char *buf, int len)
{
	while (len--)
		crc = crc7_byte(crc, *buf++);
	return crc;
}

/**************************************************************************/
static unsigned short ccitt_tbl[256] = {
  0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
  0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
  0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
  0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
  0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
  0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
  0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
  0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
  0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
  0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
  0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
  0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
  0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
  0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
  0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
  0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
  0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
  0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
  0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
  0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
  0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
  0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
  0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
  0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
  0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
  0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
  0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
  0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
  0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
  0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
  0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
  0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0

};

static unsigned short crc_ccitt(unsigned short crc, unsigned char *ptr, unsigned int ct)
{
	int i;

    for(i = 0; i < (int)ct; i++) 
        crc = (unsigned int)((crc << 8) ^ ccitt_tbl[((0xff & (crc >> 8)) ^ (0xff & *(ptr+i)))]);

    return crc;
}
