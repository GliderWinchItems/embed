README.txt
01-27-2014

sdpktrdr.c 
  PC program that reads packets from SD card and displays (rudimentary)
  SD card device is hard coded, '/dev/sdf'
  Command line to compile and execute: see comments near top of .c file.
  
packet_print.c
  Formats and prints.
  Edit "if" in 'void packet_print(struct PKTP *pp)' to select
    a single CAN id.

packet_extract.c
  Extracts a packet from a SD card block

Upon execution--enter block number to start and number of blocks.
  When all the blocks have be displayed it repeats.


