/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : packet_compression.c
* Hackeroos          : deh
* Date First Issued  : 12/16/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for compressing sensor packets
*******************************************************************************/
/*
This routine adds a reading to buffer.  When the buffer is filled the 
pointer is advanced to the next buffer, circularly, so that readings
can be added under interrupt and the mainline polling can remove completed
packets.

The readings are compressed.  
See the following for detailed layout--
../svn_sensor/docs/trunk/proposal/packet_compression_scheme.txt

'packet_mgr.c' is used to manage the addition of packets so that
each new CAN msg ID encountered will have a set of packet buffers
setup and initialized.

Usage (without using 'packet_mgr.c')--

// Sensor packet buffers: shaft, speed
#define PKTBUFSIZE	4
struct PKT_PACKED pkt_buf_shaft[PKTBUFSIZE];
struct PKT_PACKED pkt_buf_speed[PKTBUFSIZE];

// Control blocks
struct PKT_CTL_BLOCK pkt_ctl_shaft;
struct PKT_CTL_BLOCK pkt_ctl_speed;

 main();
... initialization...

// Set packet buffer pointer
pkt_ctl_shaft.pp = &pkt_buf_shaft[0];
pkt_ctl_speed.pp = &pkt_buf_speed[0];
// Set packet size
pkt_ctl_shaft.buf_size = PKTBUFSIZE;
pkt_ctl_speed.buf_size = PKTBUFSIZE;
// Set packet id_pkt
pkt_ctl_shaft.pp = SENSOR1_SHAFT;
pkt_ctl_speed.pp = SENSOR1_SPEED;

// Reset parameters in first packet 
if (packpkt_init(&pkt_ctl_shaft) != 0) screwed();
if (packpkt_init(&pkt_ctl_speed) != 0) screwed();
...
...
...polling loop removing packets
see: 
../svn_pod/sw_pod/trunk/pod_v1/p1_logging_handler.c
...
...interrupt service: adding data...
packpkt_add(&pkt_ctl_shaft, reading_shaft);
...
packpkt_add(&pkt_ctl_speed, reading_speed);
...
*/

#include "packet_compression.h"
#include "packet_mgr.h"

/* Subroutine prototypes */
static void packpkt_store_entry(struct PKT_CTL_BLOCK * pb);
static void packpkt_store_in_bitfield(struct PKT_CTL_BLOCK * pb, int nbits, int number);
static struct NSB packpkt_num_sig_bits(int n);
static void packpkt_new_packet(struct PKT_CTL_BLOCK * pb);
static void packpkt_buf_idxi_adv(struct PKT_CTL_BLOCK * pb);

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}


/*******************************************************************************
Inline assembly.  Hacked from code at the following link--
http://balau82.wordpress.com/2011/05/17/inline-assembly-instructions-in-gcc/
*******************************************************************************/
static inline __attribute__((always_inline)) unsigned int arm_clz(unsigned int v) 
{
  unsigned int d;
  asm ("CLZ %[Rd], %[Rm]" : [Rd] "=r" (d) : [Rm] "r" (v));
  return d;
}

/*******************************************************************************
 * static void packpkt_buf_idxi_adv(struct PKT_CTL_BLOCK * pb);
 * @brief 	: Advance to next packet buffer (circular)
 * @param	: pb--pointer to control block
 * @return	: Info is in control block	
*******************************************************************************/
static void packpkt_buf_idxi_adv(struct PKT_CTL_BLOCK * pb)
{
	pb->idx_bufi = adv_index(pb->idx_bufi, NUMPACKETBUF);
	return;
}

/*******************************************************************************
 * static void packpkt_new_packet(struct PKT_CTL_BLOCK * pb);
 * @brief 	: Initialize for a new packet
 * @param	: pb--pointer to control block
 * @return	: Info is in control block	
*******************************************************************************/
static void zero(unsigned long ul[])
{
	int i;
	for ( i = 0; i < PKT_PACKEDSIZE; i++)
		ul[i] = 0;
	return;
}

static void packpkt_new_packet(struct PKT_CTL_BLOCK * pb)
{
	unsigned char * p;
	unsigned char * pu;

	/* Set point 'pb->pp' to the next packet in the circular buffer */
	packpkt_buf_idxi_adv(pb);	// Advance to next packet buffer

	/* Zero out the packet buffer--make use of store double instruction.  Cut loop overhead with subroutine. */	
	zero(&pb->pkt[pb->idx_bufi].ul[0]);

	/* Set id_pkt and time header in packet */
	p = &pb->pkt[pb->idx_bufi].uc[0];

	/* Translate raw 'id_can' into limited 'id_pkt' for logging */
	*p++ = (pb->id_pkt >> CAN_PKTID_SHIFT);	// Copy 8 bits of raw CAN ID

	/* Copy 5 byte POD-like time: linux time, epoch shifted, 64 ticks/sec */
	pu = &pb->U.uc[4];
	*p++ = *pu--;	// Copy time 4 
	*p++ = *pu--;	// Copy time 3
	*p++ = *pu--;	// Copy time 2
	*p++ = *pu--;	// Copy time 1
	*p++ = *pu--;	// Copy time 0
	*p = 0;		// Entry count

	/* Zero some control block items */
	pb->repct = 0;		// Repetition count

// The following is lifted from 'commonbitband.h' in usart library.  (Known to work.)
//#define BITBAND(addr,bitnum) ((addr & 0xf0000000)+0x02000000+((addr & 0xfffff)<<5)+(bitnum<<2))
//#define MEM_ADDR(addr) *((volatile unsigned long *) (addr))

	/* Set 'bitfield' to the bit banding address for the first bit in the bit field of new packet buffer */
	// 'bitfield' is defined as a long, and not a pointer.
	pb->bitfield = 	0x22000000 + ( ( (unsigned long)(&pb->pkt[pb->idx_bufi].uc[7]) & 0xfffff) << 5) ;	// Bit zero of 1st bit of bit field

	/* Set 'bitfield_end' to the bit banding address for the last bit in the bit field of new packet buffer */
	pb->bitfield_end = pb->bitfield + ((PKT_PACKEDSIZE * 4) << 2) - 4;	// Number bytes << 2
	
	return;
}
/*******************************************************************************
 * void packpkt_init(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan);
 * @brief 	: Initialize a packet
 * @param	: pb--pointer to control block
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: Info is in control block	
*******************************************************************************/
void packpkt_init(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan)
{
	/* One-time-only initialization for new set of buffers */
	pb->idx_bufi = PKT_PACKEDSIZE + 1;	// Force index wraparound
	pb->cur_val = 0;
	pb->idx_bufm = 0;

	/* Initialize as if we advanced from a completed packet to the next */
	packpkt_new_packet(pb);		// Initialize the packet
	
	/* Add the reading to the buffer */	
	packpkt_add(pb, pcan);

	return;
}
/*******************************************************************************
 * void packpkt_add(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan);
 * @brief 	: Build a packed packet
 * @param	: pb--pointer to control block
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: Info is in control block	
*******************************************************************************/
void packpkt_add(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan)
{
	pb->reading = pcan->R.cd.si[1];	// Save locally (so don't have to pass it to a lot of subroutines)
	pb->nDelta = (pb->reading - pb->cur_val);	// +- difference from previous reading
	pb->U.ull = pcan->U.ull; // 'time' -This will be used if new reading doesn't fit and a new packet started.
	
	
	/* See if there was a change */
	if (pb->nDelta == 0)
	{ // Here, we have a repetition of the previous current value
		pb->repct += 1;		// Count instances of same reading
		if ( pb->repct >= (1 << PACKPKTREPCTSZ) )// Hit the limit?
		{ // Here, yes. complete the entry and start a new one
			packpkt_store_entry(pb); 	// Store entry in bit field
			pb->repct = 0;	// This might be redudant if 'store_entry does it.
		}
		return;	// <A cogent comment goes here, e.g. "duh.">
	}

	/* Reading is not the same, so start a new *entry* */
	packpkt_store_entry(pb); // Store entry in bit field
// (redundant)	pb->cur_val = reading;	// Update current value

	return;	// <A place for platitudes, e.g. "return from whence we came.">
}
/*******************************************************************************
 * static struct NSB packpkt_num_sig_bits(int n);
 * @brief 	: Determine the number of significant bits
 * @param	: Number of interest
 * @return	: struct NSB with 'd' = left justified bit field, 'z' = field width-1
*******************************************************************************/
/*
"The Definitive Guide..." p 378--
CLZ (count leading zeros) instruction.  Counts the number of leading zeros
in the value in Rm and returns the result in Rd.  The result value is 32 if
no bits are set in the source register and 0 if bit[31] is set.
*/
struct NSB	// Use this for returning two values
{
	unsigned int d;		// "n": one's compliment if "n" was negative, otherwise "n"
	unsigned int z;		// Bit field width for "d"
};

static struct NSB packpkt_num_sig_bits(int n)
{
	struct NSB nsb = {0,0};

	/* Determine number of bits  */
	if (n < 0)
	{ // Here "n" is negative (bit[31] on), complement "n"
		nsb.d = ~n;			// One's complement the number
		nsb.z = arm_clz(nsb.d);		// Count leading zeros
		nsb.d = (nsb.d << (nsb.z - 1)) | 0x80000000; // Left justify with compliment bit ON
	}
	else
	{ // Here bit[31] of "n" is off, so the number is positive.
		nsb.z = arm_clz(nsb.d);		// Count leading zeros
		nsb.d = (nsb.d << (nsb.z - 1)); // Left justify with compliment bit OFF
	}
/* Since after the complement test bit[31] will never be a one, the leading zero
   count will be a minimum of 1 (bit[30] on) and maximum of 32 (no bits on).  The
   return is then converted to be the number of bits of "n" to be stored in the bit
   field.
*/
	nsb.z = (32 - nsb.z);	// Return field width; e.g. 0 = 1 bit width, 31 = 32 bit width
	return nsb;	// Return bit field size and the bits to be stored
}
/*******************************************************************************
 * static void packpkt_store_in_bitfield(struct PKT_CTL_BLOCK * pb, unsigned int nbits, unsigned int number);
 * @brief 	: Store 'number' in the bit field using 'nbits' of 'number'
 * @param	: pb = pointer to control block
 * @param	: nbits = number of bits in 'number' to store less justified
 * @param	: number = the number (duh)
 * @param	: 32 bit value which will be converted
 * @return	: info is in control block	
*******************************************************************************/
/* 
struct PKT_CTL_BLOCK supplies the current *bit field address* into the bit field.

Little Endian'ed.  
*/
/*
Presumes:
1) pb->bitfield was initialized to the bit banding address of first bit in the bit field
2) the bit field was initialized to zero when a new packet was started.

// The following is lifted from 'commonbitband.h' in usart library.  (Known to work.)
#define BITBAND(addr,bitnum) ((addr & 0xf0000000)+0x02000000+((addr & 0xfffff)<<5)+(bitnum<<2))
#define MEM_ADDR(addr) *((volatile unsigned long *) (addr))

*/
static void packpkt_store_in_bitfield(struct PKT_CTL_BLOCK *pb, int nbits, int number)
{
	int i;
	for (i = 0; i < nbits; i++)
	{
		if (number < 0)
		{ // Here, high order bit set.  Set bit in bitfield
			*((volatile unsigned long *)pb->bitfield) = 0x1;
		}
		pb->bitfield += 4;	// Next bit banding alias'ed address
		number = number << 1;	// Next input bit
	}
	return;
}
/*******************************************************************************
 * static void packpkt_store_entry(struct PKT_CTL_BLOCK * pb);
 * @brief 	: Store the data for the current entry
 * @param	: pb--pointer to control block
 * @return	: Info is in control block	
*******************************************************************************/
static void packpkt_store_entry(struct PKT_CTL_BLOCK * pb)
{
/* At ths point we have--
   nDelta: signed difference of the new versus old reading
   repct: number of times the previous reading was the same
   bitfield: current bit banding address into the bit field
*/
	int nTot;
	unsigned int uiX;
	struct NSB nsb;

	/* Close out repetition count */
	if (pb->repct > 0)
	{ // Here there was a repeat of the current value
		uiX = ((pb->repct-1)  | (1 << PACKPKTREPCTSZ) );	// Format bit & Repetition count (offset by 1)
		packpkt_store_in_bitfield(pb, (PACKPKTREPCTSZ + 1), (uiX << (31 -(PACKPKTREPCTSZ+1))) );// Store repetition count for the last entry
	}
	else
	{ // Here no repetition count used.  bit[0] = 0.
		pb->bitfield += 4;	// Skip. Next bit banding alias'ed address
	}

	/* Determine size of delta field, and the delta field (left justified) */
	nsb = packpkt_num_sig_bits(pb->nDelta);

	/* Compute number of bits in this entry */
	nTot = (nsb.z + 6 + PACKPKTREPCTSZ);	// Delta field width size + field width ct + repct bit
	
	/* Will this entry fit in the space remaining? */
	if ((pb->bitfield + (nTot << 2)) > pb->bitfield_end) // Compare bit banding addresses
	{ // No. Here we need to start a new packet

		/* Start a new packet, i.e. next in circular buffer */		
		packpkt_new_packet(pb);	// Initialize the packet
		pb->nDelta = pb->reading;	// 1st delta of a packet = (reading - 0);
	}

	/* Store delta for new reading compared to current value from previous */
	nsb = packpkt_num_sig_bits(pb->nDelta);
	packpkt_store_in_bitfield(pb, 5, nsb.z);	// Store field width of delta for new entry
	packpkt_store_in_bitfield(pb, nsb.z, nsb.d);	// Store delta for new entry

	/* Advance the entry count */
	pb->pkt[pb->idx_bufi].uc[ENTRYCT] += 1;
	
	/* Update current value for next comparison */
	pb->cur_val = pb->reading;	// 	

	return;
}
/******************************************************************************
 * struct PKT_PTR packpkt_get(struct PKT_CTL_BLOCK * pb);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR packpkt_get(struct PKT_CTL_BLOCK * pb)
{
	struct PKT_PTR pkp = {0,0};		// This holds the return values

	if (pb->idx_bufi == pb->idx_bufm) return pkp;	// Return showing no new data
	pkp.ptr  = (char *)&pb->pkt[pb->idx_bufm];	// Set pointer
	pkp.ct   = sizeof (union PKT_PACKET);	// Set count
	pb->idx_bufm = adv_index(pb->idx_bufm, NUMPACKETBUF);
	return pkp;				// Return with pointer and count

	
}

