/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : canldr.c
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


compile/link--
//gcc canldr.c -o canldr -Wall -I../../../../common_all/trunk -I../../../../sw_f103/trunk/lib/libsensormisc -I../../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32

execute--
//gcc canldr.c -o canldr -Wall -I../../../../common_all/trunk -I../../../../sw_f103/trunk/lib/libsensormisc -I../../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32 && sudo ./canldr /dev/ttyUSB1 ~/svn_sensor/PC/sensor/canldr/test/CSA.progs

./mm && sudo ./canldr /dev/ttyUSB1 ~/svn_sensor/PC/sensor/canldr/test/CSA.progs
make && sudo ./canldr /dev/ttyUSB1 ~/svn_sensor/PC/sensor/canldr/test/CSA.progs

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
#include "commands.h"

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
#define BAUDRATE B460800
/* Serial port definition */
#define SERIALPORT "/dev/ttyUSB0"	/*  */
// #define _POSIX_SOURCE 1 /* POSIX compliant source */
char * serialport_default = SERIALPORT;
char * serialport;

#define SIZESERIALIN	8192	/* Size of buffer for serial input */
#define SIZESERIALOUTBUFF 4096	/* Size of buffer for serial chars that go out */

#define LINEINSIZE	4096	/* Size of input line from stdin (keyboard) */

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
time_t t_start_pod;
time_t t_stop_pod;
unsigned long long ll_start,ll_stop;

struct timeval tv;	/* time value for select() */

fd_set fdsetRd,fdsetRdx;	/* fd_set struc */
fd_set fdsetWr,fdsetWrx;
int iSelRet;	/* select() return value */

/* The following needs to have file scope so that we can use them in signal handlers */
struct termios pts;	/* termios settings on port */
struct termios sts;	/* termios settings on stdout/in */
struct termios pots;	/* saved termios settings */
struct termios sots;	/* saved termios settings */
char *portname;
struct sigaction sact;	/* used to initialize the signal handler */
fd_set	ready;		/* used for select */

int fds;	/* file descriptor, serial */
int pf;		/* port file descriptor */

int done;	/* Prorgram termination flag */
int filedone;	/* File closed flag */
int opensw;	/* 0 = file has not been opened; 1 = file is open */
int nofilesw;	/* 0 = file name specified; 1 = no file name on command line */
int goodfilesw;	/* 0 = no file was opened and written; 1 = yes we did it */



int	linect;

char vv[512];	/* General purpose char buffer */


FILE *fpOut;
FILE *fpList;


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
	tcsetattr(pf, TCSANOW, &pots);
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
//	int 	cmd_flag = 0;
	int	select_timeout;

	printf ("'canldr' has started\n");
//	printf ("arg count %u\n",argc);


printf ("argc %u\n",argc);

struct timeval tmeval;
tmeval.tv_sec = time(NULL);
printf ("%9u %s",(unsigned int)tmeval.tv_sec,ctime(&tmeval.tv_sec));
printf("timezone %lu\n",timezone);


	/* Strange code comes from hacking */
	serialport = serialport_default;	// Compiled-in serial port device
	serialport = argv[1];

	if (argc != 3)
	{
			printf ("Command line error: %i args.  Two args: 'device'  'sensor progs', for example--\n./canldr /dev/ttyUSB2 ~/svn_sensor/PC/sensor/canldr/release/CSA.progs\n",argc);
			return -1;
	}
	serialport = argv[1];

	/* Get the list of program paths, names going. */
	printf ("Program list file is -- %s\n",argv[2]);
	if ( (fpList = fopen (argv[2],"r")) == NULL)
	{
		printf("File with sensor program list failed to open\n");
//		return -1;
	}
//	while ( (fgets (&listbuf[0],LINESIZE,fpList)) != NULL)
//	{
//		printf("%s",listbuf);
//	}
//	rewind(fpList);


	/* Message for the hapless op (he or she would rather have Morse code?) */	
	printf ("Control C to break & exit\n");

	/* Serial port setup */
//         fds = open(serialport, O_RDWR | O_NOCTTY | O_NONBLOCK );
         pf = open(serialport, O_RDWR);

	 if (pf <0) {perror(serialport); printf ("Error: serial port did not open\n"); return -1; }

        tcgetattr(pf,&pots); 		/* Get the current port configuration for saving */

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
	tcsetattr(pf, TCSANOW, &pts);	/* TCSANOW = take effect immediately */
	tcsetattr(STDIN_FILENO, TCSANOW, &sts);

	lineinK.idx = 0;lineinS.idx=0;	/* jic */

	/* Timer for detecting response failure from gateway/CAN BUS */
	select_timeout = 1000;	// 1000 usec (1 ms) timeout
	struct timeval TMDETECT = {0,select_timeout}; //  timeout
	struct timeval tmdetect;

	PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC
	do_printmenu();	// Keyboard command menu
/* ************************************************************************************************** */
	/* The following is endless until broken by something (such as ctl C) */
/* ************************************************************************************************** */
	do
	{
		FD_ZERO(&ready);		/* Clear file descriptors */
		FD_SET(STDIN_FILENO, &ready); 	/* Add stdin to set */
		FD_SET(pf, &ready);		/* Add serial port to set */
		tmdetect = TMDETECT;		/* Refresh timeout timer */
	
		select (pf+1, &ready, NULL, NULL, &tmdetect);	/* Wait for something to happen */

		/* Send again if we timed out waiting for an expected response */
		if ( !( (FD_ISSET(pf, &ready)) || (FD_ISSET(STDIN_FILENO, &ready)) ) )
		{ // When neither file descriptors are responsible, then it must have been the timeout expiring
			do_command_timeout();
			
//			if ( (cmd_flag != 0) || (cmd_flag != 0) )	// Are we expecting a response?
//			{ // Here, yes. 
//				printf(" stall\n");	// Debugging
//				write(pf,raw_cmd,strlen(raw_cmd));	// Send the last command to the serial port again
//			}
		}

		if (FD_ISSET(pf, &ready))	/* Was the serial port in the select's return? */
		{ /* pf has characters for us */
			i = read(pf, buf, BUFSIZE);	/* Read one or more chars */
			if ( i >= 1 )		/* If no chars, then assume it is a EOF condition...(disregard) */
			{
				for (j = 0; j < i; j++)
				{
//printf("%02x ",(unsigned char)buf[j]);
//printf("%c",buf[j]);
					if ( (PC_msg_get(&pctogateway, (u8)buf[j]) ) != 0)	// Do we have a completed msg?
					{ // Here, yes.
						/* Get msg setup in an uncompressed format. */
						if (pctogateway.ct > sizeof (struct CANRCVBUF))
						{ // Here, this is probably some ASCII when the gateway unit intializes after boot up
								*pctogateway.p = 0;
								printf("%s",&pctogateway.c[0]);
						}
						else	
						{ // Here, we expect to have bonafide CAN msgs and non of this ASCII crap.
							if (pctogateway.ct == sizeof (struct CANRCVBUF))
							{ // Here it was not compressed.
								CANcopyuncompressed(&canrcvbuf, &pctogateway);	// Copy uncompressed form from input byte buffer to local struct
							}
							else
							{ // Here, it is in compressed form.
								CANcopycompressed(&canrcvbuf, &pctogateway);	// Copy compressed form, uncompressing into 'canrcvbuf'	
							}										
							/* Separate GATEWAY msg from CAN BUS msg using unitid number. */
							if ((canrcvbuf.id & CAN_UNITID_MASK) == CAN_UNITID_GATE)
							{ // Here, this message has the gateway address)
								do_pc_to_gateway(&canrcvbuf);	// Do something wonderful--same as others for now.
							}
							else
							{ // Here, we have a msg from the CAN BUS
								do_canbus_msg(&canrcvbuf);	// Do something appropriate to the command in effect
							}
						}
						PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC
					}
				}
			}
			else
			{ // Here, it looks like some sort of EOF on the serial port...how could that happen!?
//				done = 1;	// This lets the hapless Op exit the program with Ctl C
			}
		}
		if (FD_ISSET(STDIN_FILENO, &ready))		// Was the keyboard in the select's return ?
		{ /* standard input has characters for us */
			i = read (STDIN_FILENO, buf, BUFSIZE);	// Read one or more chars from keyboard
			if (i >= 1)				// If no chars then assume it is a EOF
			{
				for (j = 0; j < i; j++)	// Look at all the chars available
				{
					write(STDOUT_FILENO,&buf[j],1); // Echo input back to the hapless Op

					if (buf[j] == CONTROLC) done = 1;		// Ctl C causes loop to break
					if (linecompleted(&lineinK,buf[j]) == 1)	// Did we get a new line?
					{ // Receiving a '\n' line completes a line.
						do_command_keybrd(&lineinK.b[0]);	
					}
				}
//				write(pf,buf,i);
			} // End: 'for (j = 0; j < i; j++)// Look at all the KEYBOARD chars available'
			else
			{
				done = 1;
			}
		} // End: select();
	} while (done == 0); // End "do" when someone sets the 'done' switch to non-zero

	/* Restore original terminal settings and exit */
	tcsetattr(pf, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf ("\ncanldr Done\n");

	if (goodfilesw == 0)
	{ // Here a file was not opened (and written)
		
		exit(-1);	// Let a script calling this know to exit
	}
	else
	{ // What looks like a valid file was opened and then closed
		exit(0);	// Let a script calling this know to proceed
	}
}
/*************************************************************************************************************************************
 * static int convert_n(char * p, int n);
 * @brief	: convert "n" ascii chars to binary; check for bogus chars
 * @param	: p: pointer to argument input string
 * @param	: n: number of chars to convert
 * @return	: converted number; negative = error
*************************************************************************************************************************************/
int convert_n(char * p, int n)
{
	int i;
	int x = 0;
	for (i = 0; i < n; i++)
	{
		*pc++ = *p;	// Copy relevent chars to char array 
		if (*p > '9') return -1;
		if (*p < '0') return -1;
		x = x*10 + (*p++ - '0');
	}		
	return x;
}

/* **************************************************************************************
 * int CANsendmsg(struct CANRCVBUF* pin);
 * @brief	: Send CAN msg to gateway.
 * @param	: pin = pointer to struct that is ready to go
 * @return	: 0 = OK;
 * ************************************************************************************** */

//extern int pf;		/* port file descriptor (canldr.c) */
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
	write(pf,&c[0],size);	// Send it out the serial port.
int i; printf("CT:%d %d ",(int)pctogate.ct,size); for (i=0;i<size;i++) printf("%02x ",c[i]); printf("\n");

	return 0;
}



