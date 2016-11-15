/******************************************************************************/

#ifndef __sdcardio_h_
#define __sdcardio_h_

#include <stdint.h>

#define	SD_BLOCKSIZE	512

int32_t sdcard_open(char *filename);
int sdcard_read(int32_t blockno, char *buf);
int sdcard_write(int32_t blockno, char *buf);
int sdcard_close(void);

#endif
