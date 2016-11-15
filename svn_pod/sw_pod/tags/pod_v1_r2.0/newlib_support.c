/*
 * newlib_stubs.c
 *
 *  Created on: 2 Nov 2010
 *      Author: nanoage.co.uk
https://sites.google.com/site/stm32discovery/open-source-development-with-the-stm32-discovery
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

/* The following are in 'svn_pod/sw_stm32/trunk/lib/libusartstm32' */
#include "libusartstm32/usartallproto.h"	// For USART1, UART4


#undef errno
extern int errno;

/*
 environ
 A pointer to a list of environment variables and their values.
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit(int status) {
	status = status;
    _write(1, "exit\n", 4);
    while (1) {;}
}

int _close(int file) {
	file=file;
    return -1;
}
/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
int _execve(char *name, char **argv, char **env) {
	name=name; argv=argv; env=env;
    errno = ENOMEM;
    return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

int _fork(void) {
    errno = EAGAIN;
    return -1;
}
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat(int file, struct stat *st) {
	file=file;
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

int _getpid(void) {
    return 1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty(int file) {
    switch (file){
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;
    default:
        //errno = ENOTTY;
        errno = EBADF;
        return 0;
    }
}


/*
 kill
 Send a signal. Minimal implementation:
 */
int _kill(int pid, int sig) {
	pid=pid; sig=sig;
    errno = EINVAL;
    return (-1);
}

/*
 link
 Establish a new name for an existing file. Minimal implementation:
 */

int _link(char *old, char *new) {
	old=old; new=new;
    errno = EMLINK;
    return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek(int file, int ptr, int dir) {
	file=file; ptr=ptr; dir=dir;
    return 0;
}

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
static char *heap_end;
extern char _ebss; 	// Defined by .ld
extern char * __MSP;	// Defined in .ld

caddr_t _sbrk(int incr) 
{
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

 //    if ( (heap_end + incr) >  __MSP)
//     {
//         _write (STDERR_FILENO, "Heap and stack collision\n", 25);
//         errno = ENOMEM;
//         return  (caddr_t) -1;
//         //abort ();
//     }

    heap_end += incr;
    return (caddr_t) prev_heap_end;

}

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */


int _read(int file, char *ptr, int len) {
    int num = 0;
 ptr=ptr; len=len;
    switch (file) {
    case STDIN_FILENO:
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return num;
}

/*
 stat
 Status of a file (by name). Minimal implementation:
 int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */

int _stat(const char *filepath, struct stat *st) {
	filepath=filepath;
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 times
 Timing information for current process. Minimal implementation:
 */

clock_t _times(struct tms *buf) {
	buf=buf;
    return -1;
}

/*
 unlink
 Remove a file's directory entry. Minimal implementation:
 */
int _unlink(char *name) {
	name=name;
    errno = ENOENT;
    return -1;
}

/*
 wait
 Wait for a child process. Minimal implementation:
 */
int _wait(int *status) {
	status=status;
    errno = ECHILD;
    return -1;
}

/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
int _write(int file, char *ptr, int len) {
    int n;
    switch (file) {

    case STDOUT_FILENO: /*stdout*/
        for (n = 0; n < len; n++) 
	{
		USART1_txint_putc(*ptr);
		if (*ptr++ == '\n') 	
		{			
			USART1_txint_putc('\r');
			USART1_txint_send();
		}
      }
        break;

    case STDERR_FILENO: /* stderr */
        for (n = 0; n < len; n++) 
	{
		USART1_txint_putc(*ptr);
		if (*ptr++ == '\n') 	
		{			
			USART1_txint_putc('\r');
			USART1_txint_send();
		}
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return len;
}

