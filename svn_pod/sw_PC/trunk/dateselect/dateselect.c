/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : dateselect.c
* Hacker	     : deh
* Date First Issued  : 12/20/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Send data/time to POD and store download to file
*******************************************************************************/
/*
 Hack of winchPC.c which is--
 Hack of canonical.c & gateway.c
 See p. 277  _Linux Application Development_ Johnson, Troan, Addison-Wesley, 1998

compile/link--
gcc dateselect.c -o dateselect

execute--
sudo ./dateselect /dev/ttyUSB2 12-30-11 00:00:00 12-29-11 23:59:59
Or, execute with default serial port devices ('/dev/ttyUSB0')
sudo ./dateselect 12-30-11 00:00:00 12-29-11 23:59:59

Or, execute for manually stopping readout (only specify start)
sudo ./dateselect /dev/ttyUSB0 12-30-11 00:00:00
Or, with default serial device--
sudo ./dateselect 12-30-11 00:00:00

Or, example for compile and execute--
gcc dateselect.c -o dateselect && sudo ./dateselect /dev/ttyUSB0 12-30-11 00:00:00 12-29-11 23:59:59

01-05-2011 example--
sudo ./dateselect /dev/ttyUSB1 01-06-12 02:30:22 01-06-12 02:29:00

01-06-2011 example of data selection followed by reformatting--
sudo ./dateselect /dev/ttyUSB0 01-07-12 00:05:00 01-06-12 19:08:42 && cd ../read* && sudo ./reformat ../dateselect/120107.000500 && cd ../da*


Fields--
/dev/ttyUSB0 -- serial port
start month-day-year
start hour:minute:second
stop month-day-year
stop hour:minute:second

Hours: 0-23
Minutes: 0-59
Seconds: 0 - 59

NOTES:
The POD sends a '<' as the first char of a line at the end of the menu display.  This shows
this program that the POD is ready to receive a command.  When 'd' is pressed on the keyboard
the d plus the start|stop date/times is sent as a command to the POD.

The POD then searches for the start time and begins sending the packet readouts.  When the stop time is
reached the POD sends a '>' that signals this program that the transfer has been completed.



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

/* Subroutine prototypes */
static struct SS convertargtotime_t(char * p, char * r);
static struct SS convertdotnametotime_t(char * p);
static void converttime_ttochar(struct SS *s);

/* This definition shifts the date/time of the Linux format time */
/* (see 'svn_pod/sw_stm32/trunk/lib/libsupport/gps_time.h' */
#define PODTIMEEPOCH	(1318478400)	// Offset from Linux epoch to save bits

#define FALSE 0
#define TRUE 1
#define CONTROLC 0x03	/* Control C input char */

#define NOSERIALCHARS	10000	/* usec to wait for chars to be received on serial port */
#define PRINTF	TRUE

/* baudrate settings are defined in <asm/termbits.h>, which is
  included by <termios.h> */
#define BAUDRATE B115200
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
int Readysw;		// 1 = POD just send '>' on 1st char of line

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


int uiKB1;	/* sscan'ed keyboard input var 1 */

long	linect;

char vv[512];	/* General purpose char buffer */
char vvf[512];	/* Output file name */

/* Time zone buried in the bowels of the system */
extern long timezone;

FILE *fpOut;


/* =========================== Assemble an input line =================================== */
int linecompleted(struct LINEIN *p, char c)
{ /* Assemble a full line */
	p->b[p->idx++] = c;	/* Store the char received */
	if (p->idx >= (LINEINSIZE-1)) p->idx--; /* Don't run off end of buffer */
	if ( c == '\n')	/* NL completes the line */
	{
		p->b[p->idx] = '\0'; /* Place null at end of line */
		p->size = p->idx;	/* Save length of string */
		p->idx = 0;	/* reset the index */
		return 1;	/* show we have a full line */
	}
	return 0;

}

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
	int	i,j,k,l;		/* The for loop variable of choice */
	char *pa;	
	int	nX;
	int	nZ;

	printf ("'dateselect' has started\n");
//	printf ("arg count %u\n",argc);

printf ("argc %u\n",argc);

struct timeval tmeval;
tmeval.tv_sec = time(NULL);
printf ("%9u %s",(unsigned int)tmeval.tv_sec,ctime(&tmeval.tv_sec));
printf("timezone %lu\n",timezone);



/* Pass in args for tweaking xmit enable timing */
	serialport = serialport_default;	// Compiled-in serial port device
	serialport = argv[1];

		if (argc == 4)
		{
			serialport = argv[1];
printf ("serialport %s\n",serialport);
			
			/* Check if 4th argument is a time or a number */
			pa = argv[3]; nX = 0;
			while (*pa != 0)
			{
				if ((*pa < '0') || (*pa > '9'))
				{
					nX = 1; break;
				}
				pa++;
			}
			if (nX == 0)
			{ // Here it looks like it is number.  So we expect the 'yymmdd.hhmmss ssss' format.
				serialport = argv[1];
				nZ = atoi(argv[3]);	// Convert seconds duration
printf("argv[2]: %s %u\n",argv[2], nZ);
				s_stop = convertdotnametotime_t(argv[2]); // Get start-of-event time (low end of time)
				if (s_stop.t == 0)	// Was there a gross error in the argument?
				{ // Here, yes.  Bomb out.
					printf ("start time didn't past the checks %u\n",(unsigned int)s_stop.t); return -1;
				}
				s_start.t = s_stop.t + nZ;	// Compute end-of-event time (which is start of readout backwards)
				/* Convert time_t to a 'tm' to char string */
				converttime_ttochar(&s_start);
printf ("c array: %s\n",&s_start.c[0]);
			}
		}
		else
		{
			if (argc != 2)
			{
				printf ("Command line error.  There are two forms:\n./dateselect /dev/ttyUSB0 yymmdd.hhmmss duration\n./dateselect  /dev/ttyUSB0\n");
				return -1;
			}
			nofilesw = 1;	// No file name, so don't try to write to the file output.
		}

	if (nofilesw == 0)
	{
		printf("   Search date/time will be: %s",ctime (&s_start.t));
		printf("   Ending date/time will be: %s",ctime (&s_stop.t));
		printf("  The duration (seconds) is: %d\n",nZ);

		/* Convert linux time (secs since epoch) to POD time.  These are the times sent to the POD as 'd' command arguments */
		printf ("epoch shift %9u\n",PODTIMEEPOCH);
		s_start.t -= PODTIMEEPOCH;	// Shift epoch time
		printf ("start time epoch shifted: %9u\n",(unsigned int)s_start.t);
		s_start.t  = s_start.t << 11;	// Scale for 2048 sub sec ticks

		s_stop.t -= PODTIMEEPOCH;
		printf ("stop  time epoch shifted: %9u\n",(unsigned int)s_stop.t);
		s_stop.t = s_stop.t << 11;

		/* Output file */
		/* First, create a file name in the format yymmdd.hhmmss (year month day . hour minute second) */
		l = 0; k = 0;
		while (k < 6) vvf[l++] = s_stop.c[k++]; 
		vvf[l++] = '.';
		while (k < 12) vvf[l++] = s_stop.c[k++]; 
		vvf[l++] = 0;

		printf ("Data is transfered this readout will be saved as file named: %s\n",vvf);
	}
	


	/* Message for the hapless op (he or she would rather have Morse code) */	
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

	/* The following is endless until broken by something (such as ctl C) */
	do
	{
		FD_ZERO(&ready);	/* Clear file descriptors */
		FD_SET(STDIN_FILENO, &ready); /* Add stdin to set */
		FD_SET(pf, &ready);	/* Add serial port to set */

		select (pf+1, &ready, NULL, NULL, NULL);	/* Wait for something to happen */
		if (FD_ISSET(pf, &ready))	/* Was the serial port in the select's return? */
		{ /* pf has characters for us */
			i = read(pf, buf, BUFSIZE);	/* Read one or more chars */
			if ( i >= 1 )		/* If no chars, then assume it is a EOF condition */
			{
				for (j = 0; j < i; j++)
				{
					if (buf[j] == CONTROLC) done = 1;
					if (linecompleted(&lineinS,buf[j]) == 1)
					{ // Here a completed line has been assembled
						write(STDOUT_FILENO,lineinS.b,lineinS.size); /* Output assembled line on the console terminal */

						/* Sync to POD so that we only send a command when it is ready to receive one */
						if (lineinS.b[1] == '>')	// Check 1st char on a line from POD
						{ // "> Ready for next command\n\r" was received from POD
							Readysw = 1;	// Switch that shows that the POD is ready to receive a command
						}
						else
						{ // Something else intervened, so turn off the switch.
							Readysw = 0;
						}

						/* Select only lines that are packet (hex) data */
						//First get rid of 0x0d at beginning of line if present
						lineinS.b[lineinS.size] = 0; // Place '0' to be sure we have a string 
						if  (lineinS.b[0] == 0x0d)
							strcpy (&lineinS.b[0],&lineinS.b[1]); 
						/* Eliminate lines that do not start with a non-hex char */
						if ( ((lineinS.b[0] >= '0') && (lineinS.b[0] <= '9')) || ((lineinS.b[0] >= 'A') && (lineinS.b[0] <= 'F')) )
						{
							if (nofilesw == 0)
							{ // Here a file name was specified on the command line argument
								if (filedone == 0)
								{ // Don't come here if the file has been closed
									if (opensw == 0)
									{ // Here, the file has not been opened
										if ( (fpOut = fopen (vvf,"w")) == NULL)
										{
											printf ("\nOutput file did not open %s\n",vvf); return 1;
										}
										opensw = 1;
										goodfilesw = 1;
									}
									fputs(&lineinS.b[0],fpOut);	// Write the line to the file
									linect += 1;	// Simple line counter
								}
							}
						}

						/* Is this a unique line from the POD that signals end of file. */
						if (lineinS.b[0] == '<')
						{
							if (filedone == 0)
							{
								if (opensw != 0)
									fclose (fpOut);
								printf ("(PC) file closed, line ct: %d  file name: %s\n",linect,vvf);
								filedone = 1;
							}
						}
						
					}
				}
//				write(STDOUT_FILENO, buf, i); /* Output the raw serial chars on the console terminal */
			}
			else
			{
				done = 1;	// Huh!  What is this for?
			}

		}
		if (FD_ISSET(STDIN_FILENO, &ready))	/* Was the keyboard in the select's return ? */
		{ /* standard input has characters for us */
			i = read (STDIN_FILENO, buf, BUFSIZE);	/* Read one or more chars from keyboard */
			if (i >= 1)		/* If no chars then assume it is a EOF */
			{
				for (j = 0; j < i; j++)
				{
					if (buf[j] == CONTROLC) done = 1;	// Ctl C causes loop to break
					if (linecompleted(&lineinK,buf[j]) == 1)	// Did we get a new line?
					{ // Receiving a '\n' line completes a line.
						vv[0] = lineinK.b[0]; // This takes care of sending single char commands
						if ((vv[0] == 'd') && (Readysw == 1))	// 
						{ // 'd' command is a special case that sends the start|stop times to the POD
							if (filedone == 0)
							{
								Readysw = 0; l = 1;
								for (k = 0; k < 12; k++) vv[l++] = s_start.c[k];
								for (k = 0; k < 12; k++) vv[l++] = s_stop.c[k];
								vv[l++] = 0x0d; vv[l++] = 0;
printf("(PC) len: %u send cmd %s\n",l,vv);
								write(pf,vv,l);
							}
							else
							{
								printf(" #### Readout file has already been written and closed. Ctl-C and restart this program if a re-readout is desired\n");
							}
						}
						else
						{ // All other commands just send the single char command 
							vv[1] = 0x0d; vv[2] = 0;
							write(pf,vv,2);
//printf("vv: %x %x\n",vv[0],vv[1]);
						}
					}
				}
//				write(pf,buf,i);
			}
			else
			{
				done = 1;
			}
		}
	} while (done == 0);

	/* Restore original terminal settings and exit */
	tcsetattr(pf, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf ("\ndateselect Done\n");

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
static int convert_n(char * p, int n)
{
	int i;
	int y;
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
/*************************************************************************************************************************************
 * static struct SS convertargtotime_t(char * p, char * r);
 * @brief	: convert the input argument to linux time_t (int) format, whilst checking for gross errors
 * @param	: p: pointer to argument input string with yy-mm-dd
 * @param	: r: pointer to argument input string with hh:mmm:ss
 * @return	: t_time: pointer to linux format time derived input argument; 0 = error
*************************************************************************************************************************************/
static struct SS convertargtotime_t(char * p, char * r)
{
	struct SS s;

	s.t = 0;	// Default return for indicating an error

	/* Pointer into char array built from input command line argument */
	pc = &s.c[0];	// Use global var to skip messy return

	/* Take each field and convert to binary, whilst checking for bogus chars */
	s.tm.tm_year = convert_n( (p+6),2);	// Convert year (e.g. 2011)
//printf ("%s yr %u\n",p,s.tm.tm_year);
	if (s.tm.tm_year < 0) return s;	// Return of bogus char encountered
	if (s.tm.tm_year == 20) return s; 	// Hapless op typed '2011' rather than '11'
 	s.tm.tm_year += (2000 - 1900) ;

	s.tm.tm_mon = convert_n( (p+0),2);	// Convert month (01 - 12)
//printf ("%s mo %u\n",p,s.tm.tm_mon);
	s.tm.tm_mon -= 1;
	if (s.tm.tm_mon < 0) return s;
	if (s.tm.tm_mon > 11) return s;

	s.tm.tm_mday = convert_n( (p+3),2);	// Convert day of month (01-31)
//printf ("%s da %u\n",p,s.tm.tm_mday);
	if (s.tm.tm_mday < 1) return s;	
	if (s.tm.tm_mday >31) return s;

	s.tm.tm_hour = convert_n( (r+0),2);	// Convert hour (00-23)
//printf ("%s hr %u\n",p,s.tm.tm_hour);
	if (s.tm.tm_hour < 0) return s;
	if (s.tm.tm_hour > 23) return s;

	s.tm.tm_min = convert_n( (r+3),2);	// Convert minute (00-59)
//printf ("%s mn %u\n",p,s.tm.tm_min);
	if (s.tm.tm_min < 0) return s;
	if (s.tm.tm_min >59) return s;

	s.tm.tm_sec = convert_n( (r+6),2);	// Convert minute (00-59)
//printf ("%s sc %u\n",p,s.tm.tm_sec);
	if (s.tm.tm_sec < 0) return s;
	if (s.tm.tm_sec >59) return s;

	s.tm.tm_isdst = 0;			// Daylight saving time switch off

	/* Here the fields all passed the check.  Now convert to linux time */
	
	s.t = mktime (&s.tm);	// Convert to time_t format;
	return s;
}
/*************************************************************************************************************************************
 * static time_t convertdotnametotime_t(char * p);
 * @brief	: convert the input argument to linux time_t (int) format, whilst checking for gross errors
 * @param	: p: pointer to argument input string.  Format expected: yymmdd.hhmmss or  yymmdd_hhmmss
 * @return	: t_time: pointer to linux format time derived input argument; 0 = error
*************************************************************************************************************************************/
static struct SS convertdotnametotime_t(char * p)
{
	//	convertdotnametotime_t(argv[2])
	struct SS s;

	s.t = 0;	// Default return for indicating an error

	/* Pointer into char array built from input command line argument */
	pc = &s.c[0];	// Use global var to skip messy return
	
	/* Take each field and convert to binary, whilst checking for bogus chars */
	s.tm.tm_year = convert_n( (p+0),2);	// Convert year (e.g. 2011)
//printf ("%s yr %u\n",p,s.tm.tm_year);
	if (s.tm.tm_year < 0) return s;	// Return of bogus char encountered
	if (s.tm.tm_year == 20) return s; 	// Hapless op typed '2011' rather than '11'
 	s.tm.tm_year += (2000 - 1900) ;

	s.tm.tm_mon = convert_n( (p+2),2);	// Convert month (01 - 12)
//printf ("%s mo %u\n",p,s.tm.tm_mon);
	s.tm.tm_mon -= 1;
	if (s.tm.tm_mon < 0) return s;
	if (s.tm.tm_mon > 11) return s;

	s.tm.tm_mday = convert_n( (p+4),2);	// Convert day of month (01-31)
//printf ("%s da %u\n",p,s.tm.tm_mday);
	if (s.tm.tm_mday < 1) return s;	
	if (s.tm.tm_mday >31) return s;

	if (!((*(p+6) == '.') || (*(p+6) == '_'))) return s;	

	s.tm.tm_hour = convert_n( (p+7),2);	// Convert hour (00-23)
//printf ("%s hr %u\n",p,s.tm.tm_hour);
	if (s.tm.tm_hour < 0) return s;
	if (s.tm.tm_hour > 23) return s;

	s.tm.tm_min = convert_n( (p+9),2);	// Convert minute (00-59)
//printf ("%s mn %u\n",p,s.tm.tm_min);
	if (s.tm.tm_min < 0) return s;
	if (s.tm.tm_min >59) return s;

	s.tm.tm_sec = convert_n( (p+11),2);	// Convert minute (00-59)
//printf ("%s sc %u\n",p,s.tm.tm_sec);
	if (s.tm.tm_sec < 0) return s;
	if (s.tm.tm_sec >59) return s;

	s.tm.tm_isdst = -1;			// Daylight saving info not available

	/* Here the fields all passed the check.  Now convert to linux time */
	
	s.t = mktime (&s.tm);	// Convert to time_t format;
	return s;	
}


/*************************************************************************************************************************************
 * static void converttime_ttochar(struct SS *s);
 * @brief	: convert time_t to 'tm' to char array
 * @param	: p: pointer to struct SS
 * @return	: t_time: pointer to linux format time derived input argument; 0 = error
*************************************************************************************************************************************/
static void converttime_ttochar(struct SS *s)
{
	char *p = &s->c[0];

	localtime_r(&s->t,&s->tm);
	
	sprintf ((p+ 0),"%02u",s->tm.tm_year-100);
	sprintf ((p+ 2),"%02u",s->tm.tm_mon+1);
	sprintf ((p+ 4),"%02u",s->tm.tm_mday);
	sprintf ((p+ 6),"%02u",s->tm.tm_hour);
	sprintf ((p+ 8),"%02u",s->tm.tm_min);
	sprintf ((p+10),"%02u",s->tm.tm_sec);
	s->tm.tm_isdst = 0;	// Don't deal with daylight time.
	return;
}

 
