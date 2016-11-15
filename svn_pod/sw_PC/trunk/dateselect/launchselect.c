/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : launchselect.c
* Hacker	     : deh
* Date First Issued  : 10/28/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Readout data from POD using launch times stored in POD
*******************************************************************************/
/*
 Hack of winchPC.c which is--
 Hack of canonical.c & gateway.c
 See p. 277  _Linux Application Development_ Johnson, Troan, Addison-Wesley, 1998

10/28/2012
  Hack of dateselect.c to provide selection of launch times stored int POD.

10/31/2012
revision 689 - 'k ','ks', 'kd' (appears to) work.  Grouping downloads into one directory
  to be added.


compile/link--
gcc launchselect.c -o launchselect -Wall

execute--
sudo ./launchselect /dev/ttyUSB2 12-30-11 00:00:00 12-29-11 23:59:59
Or, execute with default serial port devices ('/dev/ttyUSB0')
sudo ./launchselect 12-30-11 00:00:00 12-29-11 23:59:59

Or, execute for manually stopping readout (only specify start)
sudo ./launchselect /dev/ttyUSB0 12-30-11 00:00:00
Or, with default serial device--
sudo ./launchselect 12-30-11 00:00:00

Or, example for compile and execute--
gcc launchselect.c -o launchselect -Wall &&  ./launchselect /dev/ttyUSB0 12-30-11 00:00:00 12-29-11 23:59:59

01-05-2011 example--
sudo ./launchselect /dev/ttyUSB1 01-06-12 02:30:22 01-06-12 02:29:00

01-06-2011 example of data selection followed by reformatting--
sudo ./launchselect /dev/ttyUSB0 01-07-12 00:05:00 01-06-12 19:08:42 && cd ../read* && sudo ./reformat ../launchselect/120107.000500 && cd ../da*


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
static void converttime_ttochar(struct SS *s, int sw);
static void printkcmd(void);
static int edit_key_command(char w, char * p);
static void edit_pod_command(char w, char c, int x);
static void consolewriteedit(void);
static int edit_ks_command(char *p);
static int k_list_selections(void);
static int q_list_selections(void);
static void kd_next(void);
static void setup_path_and_file(char w, char * p);
static void kcmd_keyboard(void);
static void qcmd_keyboard(void);
static void qd_next(void);
static void printqcmd(void);

/* Long name and I got tired of putting static in front of everything...*/
int convert_linux_datetime_to_yymmddhhmmss(char *pfile, char* ptime);

static char raw_cmd[16];	// raw command line to send to POD (e.g. 'k 1 yyyy')
static int cmd_flag = 0;		// 1 = expecting a response, 0 = not expecting
static int cmd_file_sw = 0;	// 'kd' download switch
static int kq_sw = 'k';		// Current command

/* ---------------- LAUNCHTIME ------------------------------------------------ */
#define	LAUNCHTIMESPERGROUP	20	// Number of launchtimes listed in a bunch
#define MAXLAUNCHTIMES		320	// Max number of launchtimes

#define LTTABLESIZE	20	// Number of entries 
struct LTTABLE
{
	int	entry;		// Launchtime entry number
	char	time[64];	// Date/time (ASCII) for this launchtime
	char	file[16];	// Date/time in file format: "yymmdd.hhmmss"
}lttable[MAXLAUNCHTIMES];	// Array for selection
static int lttable_idx = 0;	// Index into array

static int lt_ctr;
static int lt_ctr_end;
static int console_edit;	// state for how incoming lines are handled	
static int ltnumber[MAXLAUNCHTIMES];
static int ltidx = 0;		// Extracted 'yyyy' of 'k' command
static int kd_next_state = 0;
static int console_edit;
static int lt_ctr;
static int lt_ctr_end;

/* ---------------- PUSHBUTTONTIME ------------------------------------------------ */
#define	PUSHBUTTONTIMESPERGROUP	20	// Number of launchtimes listed in a bunch
#define MAXPUSHBUTTONTIMES		320	// Max number of launchtimes

#define PTTABLESIZE	20	// Number of entries 
struct PTTABLE
{
	int	entry;		// Launchtime entry number
	char	time[64];	// Date/time (ASCII) for this launchtime
	char	file[16];	// Date/time in file format: "yymmdd.hhmmss"
}pttable[MAXPUSHBUTTONTIMES];	// Array for selection
static int pttable_idx = 0;	// Index into array

static int pt_ctr;
static int pt_ctr_end;
static int console_edit;	// state for how incoming lines are handled	
static int ptnumber[MAXPUSHBUTTONTIMES];
static int ptidx = 0;		// Extracted 'yyyy' of 'q' command
static int qd_next_state = 0;
static int console_edit;



const char *file_prefix = "../../../../winch/download";			// Directory where downloads will go
char file_yymmdd[6];		// Under main directory, this one groups downloads by year, month, day
char file_total[128];		// String with total file to open, e.g. '~/winch/download/121023/121023.193554'

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

int	linect;

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

	printf ("'launchselect' has started\n");
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
				converttime_ttochar(&s_start,0);
printf ("c array: %s\n",&s_start.c[0]);
			}
		}
		else
		{
			if (argc != 2)
			{
				printf ("Command line error.  There are two forms:\n./launchselect /dev/ttyUSB0 yymmdd.hhmmss duration\n./launchselect  /dev/ttyUSB0\n");
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

	/* Timer for detecting response failure from POD */
	struct timeval TMDETECT = {0,50000}; // 100,000 usec (100 ms) timeout
	struct timeval tmdetect;

	/* The following is endless until broken by something (such as ctl C) */
	do
	{
		FD_ZERO(&ready);		/* Clear file descriptors */
		FD_SET(STDIN_FILENO, &ready); 	/* Add stdin to set */
		FD_SET(pf, &ready);		/* Add serial port to set */
		tmdetect = TMDETECT;		/* Refresh timeout timer */
	
		select (pf+1, &ready, NULL, NULL, &tmdetect);	/* Wait for something to happen */

		/* Send a command to the pod again if we timed out waiting for an expected response */
		if ( !( (FD_ISSET(pf, &ready)) || (FD_ISSET(STDIN_FILENO, &ready)) ) )
		{ // When neither file descriptors are responsible, then it must have been the timeout expiring
			if ( (cmd_flag != 0) || (cmd_flag != 0) )	// Are we expecting a response?
			{ // Here, yes. 
				printf(" stall\n");	// Debugging
				write(pf,raw_cmd,strlen(raw_cmd));	// Send the last command to the serial port again
			}
		}

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
					   if (lineinS.size > 2) // Ignore lines too short to mean anything
                                           {
						/* This is important for 'k' and 'q' commands */
						consolewriteedit();	// Further edit and output assembled lines

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
						// First get rid of 0x0d at beginning of line if present
						lineinS.b[lineinS.size] = 0; // JIC: place '\0' to be sure we have a string 
						if  (lineinS.b[0] == 0x0d)
						{
//							strcpy (&lineinS.b[0],&lineinS.b[1]); 
							memmove(&lineinS.b[0],&lineinS.b[1],lineinS.size); lineinS.size -= 1;
						}

						/* Eliminate lines that do not start with a non-hex char */
						if ( ((lineinS.b[0] >= '0') && (lineinS.b[0] <= '9')) || ((lineinS.b[0] >= 'A') && (lineinS.b[0] <= 'F')) )
						{
							if ((nofilesw == 0) || (cmd_file_sw == 1))
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
//if (lineinS.b[0] == '1')
// printf("wwwwwwwwwwwwwwwww\n");
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
								switch (cmd_file_sw)
								{
								case 0:
									printf ("(PC) file closed, line ct: %d  file name: %s\n",linect,vvf); // 'd' command
									break;
								case 1:
						printf ("(PC) file closed, LAUNCHTIME line ct: %d  file name: %s\n",linect,file_total); // 'kd' cmd
									filedone = 1; cmd_file_sw = 0; kd_next_state = 5;
									break;
								case 2:
						printf ("(PC) file closed, PUSHBUTTON line ct: %d  file name: %s\n",linect,file_total); // 'qd' cmd
									filedone = 1; cmd_file_sw = 0; qd_next_state = 5; 
									break;
								}
							}
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
						vv[0] = lineinK.b[0]; 			// This takes care of sending single char commands
					   switch(vv[0])
					   {
					   case 'd': // 'd' command is a special case command that sends the start|stop times to the POD
						if (Readysw == 1)	// 'd' command requested, but check if pod is ready for command
						{ // Here, ready to go!
							if (filedone == 0) // End-of-ready signalled?
							{ // Here, no.  Setup info message for the helpless Op.
								Readysw = 0; l = 1;	// Reset pod-ready-for-command sw; set index
								for (k = 0; k < 12; k++) vv[l++] = s_start.c[k];	// Copy for output
								for (k = 0; k < 12; k++) vv[l++] = s_stop.c[k]; 	// Copy for output
								vv[l++] = 0x0d; vv[l++] = 0;				// Fix end of line
		printf("(PC) len: %u send cmd %s\n",l,vv); // Debug: (but it is kinda useful...)			// Progress message
								write(pf,vv,l);						// Output to hapless Op
							}
							else
							{ // Here we tried to send a 'd' command, but the pod has signaled it is ready
		printf(" #### Readout file has already been written and closed. Ctl-C and restart this program if a re-readout is desired\n");
							}
						}
						break;

					  case 'k':  // Command to readout from launch time table in POD
						if ((Readysw == 0) && (kq_sw == 'q'))
						{
							printf("Previous command was a 'q': do 'x' to reset first \n");
							break;
						}
						kq_sw = 'k';
						kcmd_keyboard();	// Handle details of the 'k' command from the keyboard
						qd_next_state = 0;
						break;

					  case 'q':  // Command to readout from launch time table in POD
						if ((Readysw == 0) && (kq_sw == 'k'))
						{
							printf("Previous command was a 'k': do 'x' to reset first \n");
							break;
						}
						kq_sw = 'q';
						qcmd_keyboard();	// Handle details of the 'q' command from the keyboard
						kd_next_state = 0;	
						break;
					  case 'x': // Cancel out switches for any previous command (e,g, 'k' command)
						console_edit = 0; qd_next_state = 0; kd_next_state = 0;						
						   // Drop through and send 'x' command
					  default: // All other inputs/command are just send as a single char command 
						vv[1] = 0x0d; vv[2] = 0;  
						write(pf,vv,2);		// Only 2 chars ever get sent out of the line
						break;

                                           } // End of 'switch (vv[0])'

						

					} // End of 'if (linecompleted(&lineinK,buf[j]) == 1)'
				}
//				write(pf,buf,i);
			} // End: 'for (j = 0; j < i; j++)// Look at all the KEYBOARD chars available'
			else
			{
				done = 1;
			}
		} // End: select();
		/* Poll sequence to download selected launch times */
		kd_next(); qd_next();

	} while (done == 0); // End: "do"

	/* Restore original terminal settings and exit */
	tcsetattr(pf, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	printf ("\nlaunchselect Done\n");

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
 * static void converttime_ttochar(struct SS *s, int sw);
 * @brief	: convert time_t to 'tm' to char array
 * @param	: p: pointer to struct SS
 * @param	: sw: 0 = use 'localtime_r', not zero = use 'gmtime_r'
 * @return	: t_time: pointer to linux format time derived input argument; 0 = error
*************************************************************************************************************************************/
static void converttime_ttochar(struct SS *s, int sw)
{
	char *p = &s->c[0];

	s->tm.tm_isdst = 1;	// Don't deal with daylight time.
	if (sw == 0)
		localtime_r(&s->t,&s->tm);
	else
		gmtime_r(&s->t,&s->tm);

	sprintf ((p+ 0),"%02u",s->tm.tm_year-100);
	sprintf ((p+ 2),"%02u",s->tm.tm_mon+1);
	sprintf ((p+ 4),"%02u",s->tm.tm_mday);
	sprintf ((p+ 6),"%02u",s->tm.tm_hour);
	sprintf ((p+ 8),"%02u",s->tm.tm_min);
	sprintf ((p+10),"%02u",s->tm.tm_sec);
	*(p+12) = 0;
	return;
}
/*************************************************************************************************************************************
 * int convert_linux_datetime_to_yymmddhhmmss(char *pfile, char* ptime);
 * @brief	: Take the ascii field from the date/time from the POD and fix it up for a file name in the format "yymmdd.hhmmss"
 * @param	: pfile = pointer to char output that is receives file name
 * @param	: ptime = pointer to char input that has linux time_t in ascii
*************************************************************************************************************************************/
int convert_linux_datetime_to_yymmddhhmmss(char *pfile, char* ptime)
{
	int i = 0;
	int j = strlen(ptime);
	time_t linuxtime;
	struct SS s;

//char *pdebug1 = pfile;
//char *pdebug2 = ptime;

	/* Locate linux time on '^' line from POD */
	while ((*ptime != ':') && (++i < j)) ptime++;	// Spin forward to ':'
	if (i >= j ) return -1;
	
	ptime -= 25;	// Back up to point to linux time field

	/* Convert linux time to struct tm format */
	sscanf(ptime,"%u",(unsigned int*)&linuxtime);

	/* Pick off items in tm format and place in char array */
	s.t = linuxtime;
 	converttime_ttochar(&s,1);

	/* Turn date/time into a file name string in format "yymmdd.hhmmss" */
	j = 0;
	for (i = 0; i < 6; i++)	*pfile++ = s.c[j++];
	*pfile++ = '.';
	for (i = 0; i < 6; i++)	*pfile++ = s.c[j++];
	*pfile++ = 0;	// String terminator

//printf("CONVERT: %s %s \n",pdebug1, pdebug2); // debugging

	return 0;
}

/*************************************************************************************************************************************
 * static void printkcmd(void);
 * @brief	: Print menu for k command
*************************************************************************************************************************************/
static void printkcmd(void)
{
	printf("COMMAND k\nkk - First group of launch times\nkm - Next group of launch times\nks - select launch times, for example,\n     ks 0,1,2...n or ks 0-5)\n");
	printf("k  - raw/manual line, for example,\n     k 1 0005 to shows launch time '5'\n     k 2 0005 to readout launch time '5'\n");
	return;
}
/*************************************************************************************************************************************
 * static void printqcmd(void);
 * @brief	: Print menu for q command
*************************************************************************************************************************************/
static void printqcmd(void)
{
	printf("COMMAND q\nqq - First group of pushbutton times\nqm - Next group of pushbutton times\nqs - select pushbutton times, for example,\n     qs 0,1,2...n or qs 0-5)\n");
	printf("q  - raw/manual line, for example,\n     q 1 0005 to shows pushbutton time '5'\n     q 2 0005 to readout pushbutton time '5'\n");
	return;
}
/*************************************************************************************************************************************
 * static void edit_pod_command(char w, char c, int x);
 * @brief	: Setup line in 'raw_cmd' to send to POD
 * @param	: w = char of command ('k' or 'q')
 * @param	: c = POD subcommand (1,2)
 * @param	: x = offset
 * @return	: 0 = OK, not zero for badness
*************************************************************************************************************************************/
static void edit_pod_command(char w, char c, int x)
{
	raw_cmd[0] = w; raw_cmd[1] = ' '; raw_cmd[2] = c; raw_cmd[3] = ' ';
	sprintf(&raw_cmd[4],"%04u\n\r",x);
//printf("EDIT_POD_CMD:%s\n",raw_cmd);
	return;
}
/*************************************************************************************************************************************
 * static int edit_key_command(char w, char * p);
 * @brief	: Check raw k command for errors
 * @param	: w = char of command ('k' or 'q')
 * @param	: p = pointer to input line
 * @return	: 0 = OK, not zero for badness
*************************************************************************************************************************************/
static int edit_key_command(char w, char * p)
{
/* Format for k command: 'k 1 yyyy' or 'k 2 yyyy'  where yyyy = offset POD uses to access an entry in the launchtime table */ 
	int x;
	if (*(p+1) != ' ') return -1;
	if (*(p+3) != ' ') return -2;
	if ( !((*(p+2) == '1') || (*(p+2) == '2')) ) return -3;
	sscanf((p+3),"%u",&x);
	if (x < 0) return -4;
	if (x >= 320) return -5;
	edit_pod_command(w, *(p+2), x);
	return x;	// Success
}
/*************************************************************************************************************************************
 * static int k_list_selections(void);
 * @brief	: List launch time selections
 * @return	: 0 = none, + = Number of selections
*************************************************************************************************************************************/
static int k_list_selections(void)
{
	int i;
	if (ltidx == 0) return 0;
	for (i = 0; i < ltidx; i++)
		printf ("%4d, ",ltnumber[i]);
	printf("\n");
	return ltidx;
} 
/*************************************************************************************************************************************
 * static int q_list_selections(void);
 * @brief	: List launch time selections
 * @return	: 0 = none, + = Number of selections
*************************************************************************************************************************************/
static int q_list_selections(void)
{
	int i;
	if (ptidx == 0) return 0;
	for (i = 0; i < ptidx; i++)
		printf ("%4d, ",ptnumber[i]);
	printf("\n");
	return ptidx;
} 
/*************************************************************************************************************************************
 * static int edit_ks_command(char *p);
 * @brief	: Edit input line for selecting launchtimes to readout
 * @return	: 0 = OK, not zero for badness
*************************************************************************************************************************************/

static int edit_ks_command(char *p)
{
	char *px = p+2;
	int sw1 = 0;
	int swTo = 0;
	int x,y,z;
printf("%s\n",px);
	
	ltidx = 0;
	
	while (*px != 0)
	{
		while ( ( (*px == ' ') || (*px == '0') ) && (*px != 0) ) px++; // Spin forward to leading numeric

		if ( (*px == 0) && (sw1 == 0) ) return -1;
		x = 0;
		while ( (*px >= '0') && (*px <= '9') )
		{
			if (*px == 0) break;
			x = x * 10 + (*px - '0');
			px++;
		}
		switch (*px)
		{
		case ' ':
			while (*px == ' ') px++;
			break;
		case ',':
			ltnumber[ltidx++] = x;			// Save number extracted px++; continue; }
			break;

		
		case '-': // Here this should be the "to" between two numbers, e.g. 2-5
			swTo = 1; ltnumber[ltidx++] = x;	// Save number extracted
			break;

		default:
			if (swTo != 0) 
			{ // Here, we have the lower number saved in the array, now fill in the implied numbers
				z = ltnumber[ltidx-1] + 1; y = x - z + 1;
//printf("To %d %d %d %d\n",y,x,ltidx,ltnumber[ltidx-1]);
				while ( (ltidx < MAXLAUNCHTIMES) && (ltidx <= y) )
					ltnumber[ltidx++] = z++;
				swTo = 0;
				break;
			}

			ltnumber[ltidx++] = x;			// Save number extracted

			if (ltidx >= MAXLAUNCHTIMES) return -2;	// Something ran amok
			break;
		}
		px++;
	}	
	if (k_list_selections() == 0) printf("No selections in table\n");

	return 0;	// Success
convertargtotime_t(p,p);
}
/*************************************************************************************************************************************
 * static int edit_qs_command(char *p);
 * @brief	: Edit input line for selecting pushbutton times to readout
 * @return	: 0 = OK, not zero for badness
*************************************************************************************************************************************/

static int edit_qs_command(char *p)
{
	char *px = p+2;
	int sw1 = 0;
	int swTo = 0;
	int x,y,z;
printf("%s\n",px);
	
	ptidx = 0;
	
	while (*px != 0)
	{
		while ( ( (*px == ' ') || (*px == '0') ) && (*px != 0) ) px++; // Spin forward to leading numeric

		if ( (*px == 0) && (sw1 == 0) ) return -1;
		x = 0;
		while ( (*px >= '0') && (*px <= '9') )
		{
			if (*px == 0) break;
			x = x * 10 + (*px - '0');
			px++;
		}
		switch (*px)
		{
		case ' ':
			while (*px == ' ') px++;
			break;
		case ',':
			ptnumber[ptidx++] = x;			// Save number extracted px++; continue; }
			break;

		
		case '-': // Here this should be the "to" between two numbers, e.g. 2-5
			swTo = 1; ptnumber[ptidx++] = x;	// Save number extracted
			break;

		default:
			if (swTo != 0) 
			{ // Here, we have the lower number saved in the array, now fill in the implied numbers
				z = ptnumber[ptidx-1] + 1; y = x - z + 1;
//printf("To %d %d %d %d\n",y,x,ptidx,ptnumber[ptidx-1]);
				while ( (ptidx < MAXLAUNCHTIMES) && (ptidx <= y) )
					ptnumber[ptidx++] = z++;
				swTo = 0;
				break;
			}

			ptnumber[ptidx++] = x;			// Save number extracted

			if (ptidx >= MAXLAUNCHTIMES) return -2;	// Something ran amok
			break;
		}
		px++;
	}	
	if (q_list_selections() == 0) printf("No selections in table\n");

	return 0;	// Success
}
/*************************************************************************************************************************************
 * static void setup_path_and_file(char w, char * p);
 * @brief	: Set up directory and path so that downloads go into a "month & day" directory
 * @param	: w = 'k' or 'q'
 * @param	: p = pointer to file name-- "yymmdd.hhmmss"--for next download
 * @return	: 'file_total' loaded with path and file name
*************************************************************************************************************************************/
static void setup_path_and_file(char w, char *p)
{
	char c[64];
	char *pc = &c[0];
	int i;
	int j = 0;
	char *mkdir = "mkdir  ";
	char *ptot = &file_total[0];

	/* Make up a string with the bash command to make the directory */
	strcpy (pc, mkdir); j += 6;
	strcpy ((pc+j),file_prefix);	// '~/winch/download'
	j += strlen(file_prefix);
	*(pc+j++) = '/';

	for (i = 0; i < 6; i++) *(pc+j++) = *(p+i);	// Copy yymmdd
	*(pc+j++) = 0;	// String terminator
	
//printf("MKDIR %s\n",pc); // Check that this line is set up correctly
	system (pc);	// This will fail if directory exists, but we don't care

	/* Make up a string with the path/file for the download */
	j = 0;
	strcpy ((ptot+j),file_prefix);			// '~/winch/download'
	j += strlen(file_prefix);
	*(ptot+j++) = '/';				// '~/winch/download/'
	for (i = 0; i < 6; i++) *(ptot+j++) = *(p+i);	// '~/winch/download/yymmdd'
	*(ptot+j++) = '/';				// '~/winch/download/yymmdd/'
	
	/* For 'q' command (pushubtton) insert "PB" ahead of file name */
	if (w == 'q')
	{
		*(ptot+j++) = 'P';				// '~/winch/download/yymmdd/P'
		*(ptot+j++) = 'B';				// '~/winch/download/yymmdd/PB'
	}
		
	/* Copy date.time for the file name */
	for (i = 0; i < 14; i++) *(ptot+j++) = *(p+i);	// '~/winch/download/yymmdd/yymmdd.hhmmss' or,
	*(ptot+j++) = 0;	// jic			// '~/winch/download/yymmdd/PByymmdd.hhmmss'
	
printf("FILE_TOTAL: %s\n",ptot);	
	
	return;
}
/*************************************************************************************************************************************
 * static void consolewriteedit(void);
 * @brief	: Output assembled lines after editing
*************************************************************************************************************************************/
static void consolewriteedit(void)
{
int sw = -1;	// set 'sw' to not a '^' line

	switch (console_edit)
	{
	case 10: // 'k<space> ' and other print-all commands
		write(STDOUT_FILENO,lineinS.b,lineinS.size); /* Output assembled line on the console terminal */
		break;

	case 11: // 'kd' command
		
		if (lineinS.b[0] == '^') sw = 0; // Allow for line to be offset by 1
		if (lineinS.b[1] == '^') sw = 1;
		if (sw >= 0)	// Skip non-'^' lines
		{
			cmd_flag = 0;			// Show that we have a response to the last command sent to the POD
			lttable[lttable_idx].entry = lt_ctr;				// Save launch time entry number
			sprintf((char*)&lttable[lttable_idx].time,"%s",&lineinS.b[sw]);	// Save launchtime line from POD

			// Convert linux time on POD line into a file name string: yymmdd.hhmmss 
			if (convert_linux_datetime_to_yymmddhhmmss((char*)&lttable[lttable_idx].file,(char*)&lttable[lttable_idx].time) < 0)
			{ printf("\nfile name error\n"); kd_next_state = 0;console_edit= 0; break;} // Abort 'kd'

			// selection, launchtime entry number, file name, date/time (output for the hapless Op).
			printf("%3u %4u %s %s",lttable_idx,lt_ctr,(char*)&lttable[lttable_idx].file,(char*)&lttable[lttable_idx].time);

			lttable_idx += 1;	// Advance to next selection
			if (lttable_idx < ltidx) // Are we done?
			{ // Here, no.
				lt_ctr = ltnumber[lttable_idx];	// Get entry number
				edit_pod_command('k','1', lt_ctr);	// Set up command line for requesting a launchtime entry
				write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
				cmd_flag = 1;			// Show we are expect response, so a timeout will retry
			}
			else
			{ // Here, entry versus date/time table is complete for all the selections
				lttable_idx = 0;	// Reset index to go through table again for downloads
				kd_next_state = 3;	// Continue download
				console_edit = 10;	// Direct all additional output to the console
			}
		}
		break;

	case 12: // 'kk' and 'km' commands 
		
		if (lineinS.b[0] == '^') sw = 0; // Allow for line to be offset by 1
		if (lineinS.b[1] == '^') sw = 1;
		if (sw >= 0)	// Skip non-'^' lines
		{
			cmd_flag = 0;	// Show that we have a response to the last command sent to the POD
			printf("%s",&lineinS.b[sw]);	// Does the same thing as the above line
			edit_pod_command('k', '1', lt_ctr);	// Set up command line for requesting a launchtime entry
			
			if (lt_ctr < lt_ctr_end)	// Stop after a bunch of lines have been dislayed
			{
				write(pf,raw_cmd,strlen(raw_cmd)); // Send raw line to POD
				lt_ctr += 1; if (lt_ctr >= MAXLAUNCHTIMES) {lt_ctr = 0; lt_ctr_end = LAUNCHTIMESPERGROUP;}
				cmd_flag = 1;		// Show we are expect response, so a timeout will retry
			}
		}
		break;

	case 110: // 'qd' command
		
		if (lineinS.b[0] == '^') sw = 0; // Allow for line to be offset by 1
		if (lineinS.b[1] == '^') sw = 1;
		if (sw >= 0)	// Skip non-'^' lines
		{
			cmd_flag = 0;			// Show that we have a response to the last command sent to the POD
			pttable[pttable_idx].entry = pt_ctr;				// Save launch time entry number
			sprintf((char*)&pttable[pttable_idx].time,"%s",&lineinS.b[sw]);	// Save launchtime line from POD

			// Convert linux time on POD line into a file name string: yymmdd.hhmmss 
			if (convert_linux_datetime_to_yymmddhhmmss((char*)&pttable[pttable_idx].file,(char*)&pttable[pttable_idx].time) < 0)
			{ printf("\nfile name error\n"); qd_next_state = 0;console_edit= 0; break;} // Abort 'qd'

			// selection, launchtime entry number, file name, date/time (output for the hapless Op).
			printf("%3u %4u %s %s",pttable_idx,pt_ctr,(char*)&pttable[pttable_idx].file,(char*)&pttable[pttable_idx].time);

			pttable_idx += 1;	// Advance to next selection
			if (pttable_idx < ptidx) // Are we done?
			{ // Here, no.
				pt_ctr = ptnumber[pttable_idx];	// Get entry number
				edit_pod_command('q', '1', pt_ctr);	// Set up command line for requesting a launchtime entry
				write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
				cmd_flag = 1;			// Show we are expect response, so a timeout will retry
			}
			else
			{ // Here, entry versus date/time table is complete for all the selections
				pttable_idx = 0;	// Reset index to go through table again for downloads
				qd_next_state = 3;	// Continue download
				console_edit = 10;	// Direct all additional output to the console
			}
		}
		break;

	case 120: // 'qq' and 'qm' commands 
		
		if (lineinS.b[0] == '^') sw = 0; // Allow for line to be offset by 1
		if (lineinS.b[1] == '^') sw = 1;
		if (sw >= 0)	// Skip non-'^' lines
		{
			cmd_flag = 0;	// Show that we have a response to the last command sent to the POD
			printf("%s",&lineinS.b[sw]);	// Does the same thing as the above line
			edit_pod_command('q', '1', pt_ctr);	// Set up command line for requesting a launchtime entry
			
			if (pt_ctr < pt_ctr_end)	// Stop after a bunch of lines have been dislayed
			{
				write(pf,raw_cmd,strlen(raw_cmd)); // Send raw line to POD
				pt_ctr += 1; if (pt_ctr >= MAXLAUNCHTIMES) {pt_ctr = 0; pt_ctr_end = LAUNCHTIMESPERGROUP;}
				cmd_flag = 1;		// Show we are expect response, so a timeout will retry
			}
		}
		break;

	default: // Catch all (same as case 10, for now)
		write(STDOUT_FILENO,lineinS.b,lineinS.size); /* Output assembled line on the console terminal */
		break;
	}
	return;
}

/*************************************************************************************************************************************
 * static void kd_next(void);
 * @brief	: Get download started
 * @return	: 0 = none, + = Number of selections
*************************************************************************************************************************************/
static void kd_next(void)
{
/* Note-- This routine is entered from the keyboard "if" for the 'kd' command and
   also entered at the end of the 'select()' routine.  This latter entry occurs after
   the 'kd' command, so CASE 1 below sets the next entry to CASE 2 which does nothing
   until the 'edit_key_command' routine has completed the building of the entry versus
   date/time array for all the selections
*/
	switch (kd_next_state)
	{
	case 0: // Idle loop through
		break;

	case 1: // Make a list of selection versus launch entry times (Enter from keyboard 'kd' command) 
		console_edit = 11;		// Handling of POD response
		lttable_idx = 0;		// Index into array
		lt_ctr = ltnumber[lttable_idx];	// Get entry number
		edit_pod_command('k', '1', lt_ctr);	// Set up command line for requesting a launchtime entry ('1')
		write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
		kd_next_state = 2;
		break;

	case 2: // Check for end of building table 'consolewriteedit()' changes 'kd_next_state' to "3"
		break; // (Obvously redundant, since 'case 0' could be used just as well.)

	case 3:
		// Make a directory if needed and setup the path/file string
		setup_path_and_file('k',lttable[lttable_idx].file);
		// launch time table of entry versus date/time is complete.  Now open a file for the download
		if ( (fpOut = fopen (file_total,"w")) == NULL)
		{
			printf ("\nOutput file did not open %s\n",file_total); kd_next_state = 0; break;
		}
printf("DOWNLOADING #%u, entry %u\n",lttable_idx,ltnumber[lttable_idx]);
		cmd_file_sw = 1;
		goodfilesw = 1;			// This causes an exit code to a calling script to show a good file was written
		filedone = 0; 			// Handle file writing as if we had specified the times on the command line
		opensw = 1; 			// Show that we have opened the file
		linect = 0;			// Simple download line counter
		lt_ctr = ltnumber[lttable_idx];	// Get entry number
		edit_pod_command('k', '2', lt_ctr);	// Set up command line for requesting a launchtime download ('2')
		write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
		Readysw = 0;			// We will need to wait for the POD to be ready for a command after the download completes
		kd_next_state = 4;
		break;

	case 4: // The check for the line starting with '<' from the POD signalling end of download changes 'kd_next_state' to "3"
		break; // (Obvously redundant, since 'case 0' could be used just as well.)

	case 5: // Download file closed and that brings us here.
		if (Readysw == 0) return;	// Wait until POD signals it is ready for a command
		
		lttable_idx += 1;	// Advance to next selection
		if (lttable_idx < ltidx) // Are we done?
		{ // Here, no.  Do next download on the list.
printf("\nNEXT DOWNLOAD %u\n",lttable_idx);
			kd_next_state = 3;	
			break;		
		}
		printf (" ALL %u DOWNLOADS COMPLETE\n\n",ltidx);
		kd_next_state = 0;
		break;
		
	/* Make a directory for downloads (~/winch/download/yymmdd) using entry */
	}
	return;
}
/*************************************************************************************************************************************
 * static void qd_next(void);
 * @brief	: Get download started
 * @return	: 0 = none, + = Number of selections
*************************************************************************************************************************************/
static void qd_next(void)
{
/* Note-- This routine is entered from the keyboard "if" for the 'qd' command and
   also entered at the end of the 'select()' routine.  This latter entry occurs after
   the 'qd' command, so CASE 1 below sets the next entry to CASE 2 which does nothing
   until the 'edit_key_command' routine has completed the building of the entry versus
   date/time array for all the selections
*/

	switch (qd_next_state)
	{
	case 0: // Idle loop through
		break;

	case 1: // Make a list of selection versus launch entry times (Enter from keyboard 'kd' command) 
		console_edit = 110;		// Handling of POD response
		pttable_idx = 0;		// Index into array
		pt_ctr = ptnumber[pttable_idx];	// Get entry number
		edit_pod_command('q', '1', pt_ctr);	// Set up command line for requesting a launchtime entry ('1')
		write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
		qd_next_state = 2;
		break;

	case 2: // Check for end of building table 'consolewriteedit()' changes 'qd_next_state' to "3"
		break; // (Obvously redundant, since 'case 0' could be used just as well.)

	case 3:
		/* Make a directory if needed and setup the path/file string */
		setup_path_and_file('q',pttable[pttable_idx].file);
		// launch time table of entry versus date/time is complete.  Now open a file for the download
		if ( (fpOut = fopen (file_total,"w")) == NULL)
		{
			printf ("\nOutput file did not open %s\n",file_total); qd_next_state = 0; break;
		}
printf("DOWNLOADING #%u, entry %u\n",pttable_idx,ptnumber[pttable_idx]);
		cmd_file_sw = 1;
		goodfilesw = 1;			// This causes an exit code to a calling script to show a good file was written
		filedone = 0; 			// Handle file writing as if we had specified the times on the command line
		opensw = 1; 			// Show that we have opened the file
		linect = 0;			// Simple download line counter
		pt_ctr = ptnumber[pttable_idx];	// Get entry number
		edit_pod_command('q', '2', pt_ctr);	// Set up command line for requesting a launchtime download ('2')
		write(pf,raw_cmd,strlen(raw_cmd)); 	// Send raw line to POD
		Readysw = 0;			// We will need to wait for the POD to be ready for a command after the download completes
		qd_next_state = 4;
		break;

	case 4: // The check for the line starting with '<' from the POD signalling end of download changes 'qd_next_state' to "3"
		break; // (Obvously redundant, since 'case 0' could be used just as well.)

	case 5: // Download file closed and that brings us here.
		if (Readysw == 0) return;	// Wait until POD signals it is ready for a command
		
		pttable_idx += 1;	// Advance to next selection
		if (pttable_idx < ptidx) // Are we done?
		{ // Here, no.  Do next download on the list.
printf("\nNEXT DOWNLOAD %u\n",pttable_idx);
			qd_next_state = 3;	
			break;		
		}
		printf (" ALL %u DOWNLOADS COMPLETE\n\n",ptidx);
		qd_next_state = 0;
		break;
		
	/* Make a directory for downloads (~/winch/download/yymmdd) using entry */
	}
	return;
}
/*************************************************************************************************************************************
 * static void kcmd_keyboard(void);
 * @brief	: Handle details of the 'k' command from the keyboard
*************************************************************************************************************************************/
static void kcmd_keyboard(void)
{
	int	nE;

	switch (lineinK.b[1])	// Sub 'k' commands (all but the "raw" one don't go to the POD)
	{
		case 'k': // 'kk' = list launchtimes: the first number (#LTPERPAGE)
			printf ("\n launch   end time    time/date (GMT) end\n");
			console_edit = 12;
			lt_ctr = 0;	lt_ctr_end = LAUNCHTIMESPERGROUP;
			edit_pod_command('k', '1', lt_ctr++);
			write(pf,raw_cmd,strlen(raw_cmd));
			cmd_flag = 1;	// Show we are waiting for a response
			ltidx = 0;
			break;

		case 'm': // 'km' - list launch times: the next (#LTPERPAGE)
			console_edit = 12;
			lt_ctr_end += LAUNCHTIMESPERGROUP;
			edit_pod_command('k', '1', lt_ctr++);
			write(pf,raw_cmd,strlen(raw_cmd));
			cmd_flag = 1;	// Show we are waiting for a response
			break;
							
		case 's': // 'ks n0, n1,...,nX' or 'ks nN - nM; = "select" launch times to readout
			console_edit = 10;
			edit_ks_command(&lineinK.b[0]);
			break;

		case 'd': // 'kd' download all launches that have been selected
			if (ltidx <= 0)
			{
				printf( "kd - download selections.  NO SELECTIONS in table\n");
				break;
			}
			else
			{
				kd_next_state = 1;	// Begin sequence to download
				kd_next();
			}
				break;
		case ' ': // 'k ' Send a raw "manual" line to the pod
			if ((nE = edit_key_command('k', &lineinK.b[0])) >= 0)	
			{ // Here, line passed the edit checks 
				write(pf,raw_cmd,strlen(raw_cmd)); // Send raw line to POD
				console_edit = 14;
			}
			else
			     { printf("k command error: %d %s",nE,&lineinK.b[0]);  printkcmd(); }
			console_edit = 10;
			ltidx = 0;
			break;

		default:  // Sub command not on list
			console_edit = 0;								
			break;
	}
	return;
}
/*************************************************************************************************************************************
 * static void qcmd_keyboard(void);
 * @brief	: Handle details of the 'q' command from the keyboard
*************************************************************************************************************************************/
static void qcmd_keyboard(void)
{
	int	nE;

	switch (lineinK.b[1])	// Sub 'q' commands (all but the "raw" one don't go to the POD)
	{
		case 'q': // 'qq' = list pushbutton times: the first number (#LTPERPAGE)
			printf ("\n launch   end time    time/date (GMT) end\n");
			console_edit = 120;
			pt_ctr = 0;	pt_ctr_end = PUSHBUTTONTIMESPERGROUP;
			edit_pod_command('q', '1', pt_ctr++);
			write(pf,raw_cmd,strlen(raw_cmd));
			cmd_flag = 1;	// Show we are waiting for a response
			ptidx = 0;
			break;

		case 'm': // 'qm' - list pushbutton times: the next (#LTPERPAGE)
			console_edit = 120;
			pt_ctr_end += PUSHBUTTONTIMESPERGROUP;
			edit_pod_command('q', '1', pt_ctr++);
			write(pf,raw_cmd,strlen(raw_cmd));
			cmd_flag = 1;	// Show we are waiting for a response
			break;
							
		case 's': // 'qs n0, n1,...,nX' or 'ks nN - nM; = "select" pushbutton times to readout
			console_edit = 10;
			edit_qs_command(&lineinK.b[0]);
			break;

		case 'd': // 'qd' download all pushbutton times that have been selected
			if (ptidx <= 0)
			{
				printf( "qd - download selections.  NO SELECTIONS in table\n");
				break;
			}
			else
			{
				qd_next_state = 1;	// Begin sequence to download
				qd_next();
			}
				break;
		case ' ': // 'q ' Send a raw "manual" line to the pod
			if ((nE = edit_key_command('q', &lineinK.b[0])) >= 0)	
			{ // Here, line passed the edit checks 
				write(pf,raw_cmd,strlen(raw_cmd)); // Send raw line to POD
				console_edit = 10;
			}
			else
			     { printf("q command error: %d %s",nE,&lineinK.b[0]);  printqcmd(); }
			console_edit = 10;
			ptidx = 0;
			break;

		default:  // Sub command not on list
			console_edit = 0;								
			break;
	}
	return;
}


