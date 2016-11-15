/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cangate.c
* Hacker	     : deh
* Date First Issued  : 09/12/2013
* Board              : PC
* Description        : CAN Bus via 'gateway' (stm32)
*******************************************************************************/
/*
 Hack of winchPC.c which is--
 Hack of canonical.c & gateway.c
 See p. 277  _Linux Application Development_ Johnson, Troan, Addison-Wesley, 1998

10/28/2012
  Hack of dateselect.c to provide selection of launch times stored int POD.

09/12/2013
  Hack of 'launchselect.c'

11/27/2013
  Hack of 'canldr.c'


compile/link--
./mm

compile and execute--
./mm && sudo ./cangate /dev/ttyUSB0
  OR
./mm && sudo ./cangate /dev/ttyUSB0 ../test/testmsg1.txt

*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "common_can.h"

#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "commands.h"
#include "cmd_s.h"
#include "sockclient.h"

/* Subroutine prototypes */
int CANsendmsg(struct CANRCVBUF* pin);

/* Buffer for PC msg */
#define PCMSGBUFSIZE	256
u8 PCmsg[PCMSGBUFSIZE];

/* Initial test msg */
char *ptestfox = "THE FOX JUMPED OVER A LAZY DOG'S BACK 0123456789\n";
#define TESTBINSIZE	11
unsigned char testbin[TESTBINSIZE] = {CAN_PC_ESCAPE,CAN_PC_ESCAPE,0x01,0x02,0x03,0x04,0xAA,0x55,CAN_PC_FRAMEBOUNDARY,CAN_PC_ESCAPE,CAN_PC_FRAMEBOUNDARY};
unsigned char *ptestbin = &testbin[0];


static struct PCTOGATEWAY pctogateway; // Receives de-stuffed incoming msgs from PC.
static struct CANRCVBUF canrcvbuf;
//static struct PCTOGATECOMPRESSED pctogatecompressed;



/* For input file */
#define LINESIZE	256
char listbuf[LINESIZE];

/* Keyboard key to break loop and exit */
#define CONTROLC 0x03	/* Control C input char */

#define NOSERIALCHARS	10000	/* usec to wait for chars to be received on serial port */

/* baudrate settings are defined in <asm/termbits.h>, which is
  included by <termios.h> */
//#define BAUDRATE B115200
#define BAUDRATE B230400
//#define BAUDRATE B460800
//#define BAUDRATE B921600
/* Serial port definition */

#define SERIALPORT "/dev/ttyUSB0"	/*  */
// #define _POSIX_SOURCE 1 /* POSIX compliant source */
char * serialport_default = SERIALPORT;
char * serialport;

#define SIZESERIALIN		8192	/* Size of buffer for serial input */
#define SIZESERIALOUTBUFF 	4096	/* Size of buffer for serial chars that go out */

#define LINEINSIZE		4096	/* Size of input line from stdin (keyboard) */

struct LINEIN
{
	char b[LINEINSIZE];
	int	idx;
	int	size;
}lineinK,lineinS;

#define BUFSIZE	4096
char buf[BUFSIZE];

/* For start stop */
struct SS
{
	time_t t;	// Linux time
	struct tm tm;	// Year, month, etc.
	char c[14];	// ascii yymmddhhmmss (e.g. 111229235900'\0' for Dec 12, 2011 23:59:00)
}s_start,s_stop;
char * pc;

/* Start|stop linux times */
//time_t t_start_pod;
//time_t t_stop_pod;
//unsigned long long ll_start,ll_stop;

struct timeval tv;	/* time value for select() */

//fd_set fdsetRd,fdsetRdx;	/* fd_set struc */
//fd_set fdsetWr,fdsetWrx;
//int iSelRet;	/* select() return value */

/* The following needs to have file scope so that we can use them in signal handlers */
struct termios pts;	/* termios settings on port */
struct termios sts;	/* termios settings on stdout/in */
struct termios pots;	/* saved termios settings */
struct termios sots;	/* saved termios settings */
struct sigaction sact;	/* used to initialize the signal handler */
fd_set	ready;		/* used for select */

//int fds;	/* file descriptor, serial */
int fdp;	/* port file descriptor */

int done;	/* Prorgram termination flag */
int filedone;	/* File closed flag */
int opensw;	/* 0 = file has not been opened; 1 = file is open */
int nofilesw;	/* 0 = file name specified; 1 = no file name on command line */
int goodfilesw;	/* 0 = no file was opened and written; 1 = yes we did it */

int	linect;

char vv[512];	/* General purpose char buffer */

FILE *fpOut;
FILE *fpList;
int fpListsw = 0; // 0 = no test msg file list

/* Sequence number checking */
u8 sequence_prev;


/* Debug */
u32 err_seq;
u32 err_ascii;

/* =========================== Assemble an input line =================================== */
int linecompleted(struct LINEIN *p, char c)
{ /* Assemble a full line */
	p->b[p->idx++] = c;	/* Store the char received */
	if (p->idx >= (LINEINSIZE-1)) p->idx--; /* Don't run off end of buffer */
	if ( c == '\n')	/* NL completes the line */
	{
		p->b[p->idx] = '\0';	/* Place null at end of line */
		p->size = p->idx;	/* Save length of string */
		p->idx = 0;		/* reset the index */
		return 1;		/* show we have a full line */
	}
	return 0;

}
/* ====================================================================================== */

/* Restore original terminal settings on exit */
void cleanup_termios(int signal)
{
	tcsetattr(fdp, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf("\nwinchPC exit due to Control C\n");
	exit(0);
}
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char *argv[])
{
/* Misc */
	int	i,j;		/* The for loop variable of choice */
	int	select_timeout;
	int	xret;	// Subroutine return value
	int	port;	// Socket port number
	char xbuf[256];

	fpListsw = 0; // 0 = no test msg file list opened


	printf ("CANGATGE has started\n");

printf ("argc %u\n",argc);

	if (argc > 4)
	{
			printf ("Command line error: too many arguments %i args, for example--\n./cangate /dev/ttyUSB0\nOR,\n./cangate /dev/ttyUSB0 ../test/testmsg1.txt\n",argc); return -1;
	}

	if (argc < 2)
	{
		printf("Command line error: no arguments, for example--\n./cangate /dev/ttyUSB0\nOR,\n./cangate /dev/ttyUSB0 ../test/testmsg1.txt\n"); return -1;
	}
	
	/* Determine if it is a socket connection or uart */
	if (strncmp(argv[1], "/dev/tty", 8) == 0) // uart?
	{ // Here, yes.
		serialport = argv[1];
		if (argc == 3)
		{ // Here there were two arguments on the command line and 2nd is presumed to be the file with the test msgs.
			if ( (fpList = fopen (argv[2],"r")) == NULL)
			{
				printf("Test msg file given on command line did not open: %s\n",argv[2]); return -1;
			}
			else
			{
				printf("Test msg file opened: %s\n",argv[2]);
				fpListsw = 1; // test msg file list
				while ( (fgets(xbuf,256, fpList)) != NULL)	// Get a line
				{
					printf("%s",&xbuf[0]);
				}
				
			}
		}		
	/* =============== Serial port setup ===================================================================== */
//         fd = open(serialport, O_RDWR | O_NOCTTY | O_NONBLOCK );
         fdp = open(serialport, O_RDWR);

	 if (fdp < 0) {perror(serialport); printf ("Error: So sorry, the serial port did not open\n"); return -1; }

        tcgetattr(fdp,&pots); 		/* Get the current port configuration for saving */

	/* Modify the serial port settings */
	pts.c_lflag &= ~ICANON;	/* Turn off canonicalization, i.e. set raw mode (no chars are special) */
	pts.c_lflag &=~(ECHO | ECHOCTL | ECHONL); /* No local echo */
	pts.c_cc[VMIN] = 1; /* See page 307 */
	pts.c_cc[VTIME] = 0;
	/* Without the following two lines the remote system would see Return twice for each press */
	pts.c_oflag &= ~ONLCR;
	pts.c_iflag &= ~ICRNL;

	pts.c_cflag &= ~(CSIZE | PARENB);
	pts.c_cflag |= CS8;	// 8 bits
	pts.c_cflag |= CREAD;	// Enable reading
	pts.c_cflag |= CLOCAL;	// Disable waiting for modem lines

	/* No flow control */
	pts.c_cflag &= ~CRTSCTS;
	pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	/* Speed */
	cfsetospeed(&pts, BAUDRATE);
	cfsetispeed(&pts, BAUDRATE);
 

	/* Modify the local terminal settings */
	tcgetattr(STDIN_FILENO,&sts);	/* Current settings */
	sots = sts;			/* Save current settings */
	sts.c_iflag &= ~BRKINT;		/* Turn off break.  Only useful when serial port can send break */
	sts.c_iflag |= IGNBRK;		/* Ignore breaks */
	sts.c_lflag &= ~ISIG;		/* This keeps driver from sending SIGINTR, SIGQUIT, SIGTSTP when
					INTR, QUIT, or SUSP is received. */
	sts.c_cc[VMIN] = 1;
	sts.c_cc[VTIME] = 0;
	sts.c_lflag &= ~ICANON;
	sts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL); /* No local echo */

	/* Set the signal handler to restore the old termios handler */
	/* Setup the signal handlers */
	sact.sa_handler = cleanup_termios;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL); /* User sent Control C */
	sigaction(SIGPIPE, &sact, NULL);
	sigaction(SIGTERM, &sact, NULL);

	/* With signal handlers setup we can now set the modifed termios settings */
	tcsetattr(fdp, TCSANOW, &pts);	/* TCSANOW = take effect immediately */
	tcsetattr(STDIN_FILENO, TCSANOW, &sts);
	/* =============== End Serial port setup ================================================================ */
	}
	else
	{ // Here, not a uart.  See if it looks like an ip address

		char* parg = argv[1];
		j = 0;
		for (i = 0; i < strlen(argv[1]); i++)
		{
			if (*parg++ == '.') j += 1;
		}
		if ( j != 3) {printf("1st arg doesn't look like /dev/tty, or ip address xxx.xxx.xxx.xxx -- %s %i\n", argv[1],j); return -1;}

		/* Here, the arguments should be: <ip> <port> <optional test file> */
		if (argc < 3){printf ("1st arg looks like an IP, but not enough arguments\n"); return -1;}

		/* Here, next argument should be the port number */
		sscanf(argv[2],"%i",&port);

		fdp = sockclient_connect(argv[1], port);
		if (fdp < 0){ printf("socket did not open: ip %s, port: %i\n",argv[1],port); return -1;}
		
		printf ("Socket opened OK.  IP: %s PORT: %i\n",argv[1], port);

		if (argc == 4)
		{ // Here there were three arguments on the command line. the 3rd is presumed to be the file with the test msgs.
			if ( (fpList = fopen (argv[3],"r")) == NULL)
			{
				printf("Test msg file given on command line did not open: %s\n",argv[3]); return -1;
			}
			else
			{
				printf("Test msg file opened!: %s\n",argv[3]);
				fpListsw = 1; // test msg file list
			}		
		}
	}
	
	

	/* Message for the hapless op (maybe he or she would rather have Morse code?) */	
	printf ("Control C to break & exit\n");


	lineinK.idx = 0;lineinS.idx=0;	/* jic */

	/* Timer for detecting response failure from gateway/CAN BUS */
	select_timeout = 1000;	// 1000 usec (1 ms) timeout
	struct timeval TMDETECT = {0,select_timeout}; //  timeout
	struct timeval tmdetect;

	PC_msg_initg(&pctogateway);		// Initialize struct for a msg from gateway to PC
	pctogateway.mode_link = MODE_LINK;	// Set modes for routines that receive and send CAN msgs
	do_printmenu();				// Print an initial keyboard command menu
/* ************************************************************************************************** */
	/* The following is endless until broken by ctl C */
/* ************************************************************************************************** */
	do
	{
		FD_ZERO(&ready);		/* Clear file descriptors */
		FD_SET(STDIN_FILENO, &ready); 	/* Add stdin to set */
		FD_SET(fdp, &ready);		/* Add serial port to set */
		tmdetect = TMDETECT;		/* Refresh timeout timer */
	
		select (fdp+1, &ready, NULL, NULL, &tmdetect);	/* Wait for something to happen */

		/* Send again if we timed out waiting for an expected response */
		if ( !( (FD_ISSET(fdp, &ready)) || (FD_ISSET(STDIN_FILENO, &ready)) ) )
		{ // When no file descriptors are responsible, then it must have been the timeout expiring
		/* Sending test msgs to CAN if we opened a file with the list. */

		}

		/* Incoming bytes from CAN gateway, arriving via serial port (file descriptor: 'fdp'). */
		if (FD_ISSET(fdp, &ready))	/* Was the serial port in the select's return? */
		{ /* fdp has characters for us */
			xret = USB_PC_get_msg_mode(fdp, &pctogateway, &canrcvbuf); // Build msg from incoming bytes
			if ( xret != 0 ) // Did the above result in frame, or line, from the gateway?
			{ // Here yes.  We have a msg, but it might not be valid.
				if (xret == 1)
				{ // Here the msg had not errors.
					do_canbus_msg(&canrcvbuf);	// Do something appropriate to the command in effect (commands.c)

					/* Poll for outputting test msgs, timed/throttled by counting incoming CAN msgs. */
					if (fpListsw > 0)
						cmd_s_do_msg(fpList, fdp);
				}
				else
				{ // Something wrong with the msg
					if (xret < -4) // Did msg complete, but compression returned an error?
					{ // Here, yes.
						printf("CANuncompress error (with and without -4 offset): %i %i\n", xret, xret + 4);
					}
					else
					{ // Here, msg did not complete correctly: xret = (-4 <= (return code) < 0)
						printf("\nPC_msg_get_msg_mode error: %i\n",xret);
						err_ascii += 1;					
					}
					/* Check for sequence error (even though msg might have had errors). */
					sequence_prev += 1;
					if (pctogateway.seq != sequence_prev)
					{
//						printf("Seq err-- new %u old %u\n",pctogateway.seq,sequence_prev);
						sequence_prev = pctogateway.seq;
						err_seq += 1;
					}
				}
				PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC
			}
		}

		/* Incoming data arriving from keyboard (STDIN). */
		if (FD_ISSET(STDIN_FILENO, &ready))		// Was the keyboard in the select's return ?
		{ /* standard input has characters for us */
			i = read (STDIN_FILENO, buf, BUFSIZE);	// Read one or more chars from keyboard
			if (i >= 1)				// If no chars then assume it is a EOF
			{
				for (j = 0; j < i; j++)	// Look at all the chars available
				{
					write(STDOUT_FILENO,&buf[j],1); // Echo input back to the Hapless Op

					if (buf[j] == CONTROLC) done = 1;		// Ctl C causes loop to break
					if (linecompleted(&lineinK,buf[j]) == 1)	// Did we get a new line?
					{ // Receiving a '\n' line completes a line.
						do_command_keybrd(&lineinK.b[0]);	
					}
				}
			}
			else
			{
				done = 1;
			}
		}


	} while (done == 0); // End "do" when someone sets the 'done' switch to non-zero

	/* Restore original terminal settings and exit */
	tcsetattr(fdp, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf ("\ncangate Done\n");

	if (goodfilesw == 0)
	{ // Here a file was not opened (and written)
		
		exit(-1);	// Let a script calling this know to exit
	}
	else
	{ // What looks like a valid file was opened and then closed
		exit(0);	// Let a script calling this know to proceed
	}
}
/* **************************************************************************************
 * int CANsendmsg(struct CANRCVBUF* pin);
 * @brief	: Send CAN msg to gateway.
 * @param	: pin = pointer to struct that is ready to go
 * @return	: 0 = OK;
 * ************************************************************************************** */

//extern int fdp;		/* port file descriptor (cangate.c) */
int CANsendmsg(struct CANRCVBUF* pin)
{	
	int size;
	struct PCTOGATECOMPRESSED pctogate;
	u8 c[128];	// Framed and byte stuffed chars that go to the serial port

	/* Compress the msg if possible */
	if (CANcompress(&pctogate, pin) == 0)
	{ // Here, compressed.
		size = PC_msg_prep(&c[0], 128, &pctogate.cm[0], (int)pctogate.ct);
	}
	else
	{ // Here, extended address format
		size = PC_msg_prep(&c[0], 128, (u8*)pin, (int)sizeof(struct CANRCVBUF));
	}
	write(fdp,&c[0],size);	// Send it out the serial port.
int i; printf("CT:%d %d ",(int)pctogate.ct,size); for (i=0;i<size;i++) printf("%02x ",c[i]); printf("\n");

	return 0;
}



