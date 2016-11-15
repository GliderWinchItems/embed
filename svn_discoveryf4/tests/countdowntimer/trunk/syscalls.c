#include <sys/stat.h>
#include <string.h>
#include "bsp_uart.h"

/* Subroutine prototypes for USB */
int VCP_get_char_test(void);
int VCP_get_char(u8 *buf);
void VCP_send_buffer(u8* buf, int len);

#define FD_UMAX		8	// Number of USART/UARTs allowed
#define FD_OPNMAX	(FD_UMAX+4) // Number of fd's max
int i_cur = 0;		// Next available index
struct FD_OPN
{
	void* pv;	// Control block ptr of various types
	int	dev;	// Device code
};
/* Lookup array with all possible fd's that we can assign */
struct FD_OPN fd_opn[FD_OPNMAX]; 


int __errno;

int _close(int file) {
	return 0;
}

int _fstat(int file, struct stat *st) {
	return 0;
}

int _isatty(int file) {
	return 1;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}



int _open(const char *name, int flags, int mode) {
	void* pret;
	int i;

	if (i_cur >= FD_OPNMAX) return -1;// No free slots check

	i = i_cur;

	/* Check if there is someone out there with this name */
	pret = (struct CB_UART*)bsp_uart_open(name); // Check USART/UARTs
	if (pret != 0) 
	{ // Here, bsp_uart found the name and returned a control blk ptr
		fd_opn[i].pv = pret;
		fd_opn[i].dev = 0;	// USART/UART group
		i_cur += 1;
		return (i);
	}
	// Place calls for others here, e.g. usb (device code 0)
	return -1;		// Nobody recognizes this name
}

int _read(int fd, char *ptr, int len) {
	char* p = ptr;
//	if (fd > FD_OPNMAX) return -1;
	if (fd != 0) return -1;

	if (fd_opn[fd].dev == 0) // Is it a USART/UART?
		return bsp_uart_getn_ptr((struct CB_UART*)fd_opn[fd].pv, ptr, len);
//	if (fd_opn[fd].dev == 1) // Is it a USB?
//	{	
		/* Assumes the USB is on STDIN, STDOUT */
//		if (!VCP_get_char_test()) {
//			return 0;
//		}
	
//		while(VCP_get_char_test()  &&  len > 0) 
//		{VCP_get_char((u8*)p++); len -= 1;}
//	}
	
	return p - ptr;
}

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

caddr_t _sbrk_r (struct _reent *r, int incr) {
	extern char   end asm ("end"); /* Defined by the linker.  */
	static char * heap_end;
	char *        prev_heap_end;

	if (heap_end == NULL)
		heap_end = & end;

	prev_heap_end = heap_end;

	if (heap_end + incr > stack_ptr) {
		//errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

int _write(int fd, char *ptr, int len) 
{
	if ((fd == 1) || (fd == 2))
	{
/* Map either 1 or 2 into 0, since that holds the control block pointer. */
		if (fd_opn[0].dev == 0) // Is it a USART/UART?
		{return bsp_uart_putn_ptr((struct CB_UART*)fd_opn[0].pv, ptr, len);}
	}	
//		// USB equivalent goes here-----
//	VCP_send_buffer((u8*)ptr, len); 
	return len;
}
