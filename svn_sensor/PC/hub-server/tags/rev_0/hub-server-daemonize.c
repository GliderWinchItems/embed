/******************************************************************************/
/* This file does the usual stuff to "daemonize" a server.  The technique is
 * pretty much directly out of Comer's book (vol. 3, POSIX version).
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <asm/ioctls.h>

static char *dtpd_dir = "";
static char *lock_file = "./dtpd.lock";
static int lock_fd;
static int file_bits = 0664;

int daemonize(void)
{
	/* Our process ID and Session ID
     */
	pid_t pid, sid;
    int fd, fd_max;
    char pid_buf[32];
	
	/* Fork off the parent process
     */
	pid = fork();
	if(pid < 0) 
		exit(EXIT_FAILURE);         /* Fork error exit */
	else if(pid > 0) 
		exit(EXIT_SUCCESS);         /* Parent normal exit */

    /* Create a new SID for the child process
     */
	sid = setsid();
	if(sid < 0)
		exit(EXIT_FAILURE);

    /* Close inherited descriptors (including stderr and friends).
     */
    for(fd=0,fd_max=getdtablesize(); fd<fd_max; fd+=1)
        close(fd);

    /* Detach from controlling tty
     */
    fd = open("/dev/tty", O_RDWR);
    ioctl(fd, TIOCNOTTY, 0);
    close(fd);

    /* Change the current working directory
     */
	if(*dtpd_dir && chdir(dtpd_dir) < 0)
		exit(EXIT_FAILURE);

	/* Change the file mode mask
     */
	umask(~file_bits & 0777);
	
    /* Set private process group
     */
    setpgid(0, getpid());

    /* Open stdin, stdout & stderr and point 'em all to the NULL device,
     * just in case the C library needs it.
     */
    fd = open("/dev/null", O_RDWR); /* Open stdin */
    dup(fd);                        /* Open stdout */
    dup(fd);                        /* Open stderr */

    /* Open a lock file and write our process ID to it.
     */
    lock_fd = open(lock_file, O_RDWR|O_CREAT, 0666);
    if(lock_fd < 0)
        exit(EXIT_FAILURE);         /* Can't create lock file */
    if(flock(lock_fd, LOCK_EX|LOCK_NB))
        exit(EXIT_SUCCESS);         /* Can't acquire lock */

    sprintf(pid_buf, "%6d\n", getpid());
    write(lock_fd, pid_buf, strlen(pid_buf));

    /* If we haven't exited before this, all is well and we're the only
     * dtp server running.
     */
    return 0;                       /* Return to parent */
}

/* Cleanup the lock file
 */
int daemonize_cleanup(void)
{
    close(lock_fd);
    unlink(lock_file);
}

