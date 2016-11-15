/******************************************************************************
* File Name          : packet_extract.h
* Date First Issued  : 06/14/2013
* Board              : any
* Description        : Retrieve packet from a sd block
*******************************************************************************/

#include "packet_extract.h"

/*******************************************************************************
 * int packet_extract_first(struct PKTP *pktp, unsigned char* pblk);
 * @brief 	: Extract PID, find last packet in block
 * @param	: pointer to struct with packet count, pointer, and block pid
 * @param	: pblk--pointer to control block
 * @return	:  0 = OK
 *		: -1 = Packet size too big or too small
 * 		: -2 = All zero
 * 		: pktp->p Points to beginning of packet
*******************************************************************************/

int packet_extract_first(struct PKTP *pktp, unsigned char* pblk)
{
	int i = 0;
	unsigned char* px = pblk + SDLOG_DATASIZE - 1;
	union BLOCK *pb = (union BLOCK *)pblk;

	/* Extract pid */
	pktp->pid = (pb->str.pid);

	/* Search back to first non-zero byte */	
	while ((*px == 0) && (i++ < SDLOG_DATASIZE)) px--;

	if (i >= SDLOG_DATASIZE) 
	{
		pktp->ct = i; return -2;
	}

	pktp->ct = *px;
	if ((*px > 250) || (*px < 1))
	{ // Here, error, ct is too big.
		  return -1;
	}

	pktp->p = px - *px;	// Point to beginning of packet
	pktp->plast = pktp->p;	// Save pointer to last packet
	
	return 0;
}
/*******************************************************************************
 * int packet_extract_next(struct PKTP *pktp);
 * @param	: pointer to struct with packet count, pointer, and block pid
 * @return	: 0 = OK
*******************************************************************************/

int packet_extract_next(struct PKTP *pktp)
{
	unsigned char* px = pktp->p;

	px--;		// Step to next packet count

	pktp->ct = *px;
	if ((*px > 250) || (*px < 1))
	{ // Here, error, ct is too big.
		pktp->p = 0; pktp->ct = *px; return -1;
	}

	pktp->p = px - *px;	// Point to beginning of packet

	return 0;
}


/*******************************************************************************
 * int sdblk_allzero(unsigned char * p);
 * @param	: pointer to a sd block
 * @return	: 0 = all zero; plus = count of non-zero bytes
*******************************************************************************/
int sdblk_allzero(unsigned char * p)
{
	int i;
	int j = 0;

	for (i = 0; i < 512; i++)
	{
		if (*p++ != 0) j += 1;
	}

	return j;
}
/*******************************************************************************
 * void packet_convert(struct CANRCVTIMBUF* pout, struct PKTP *pp);
 * @brief 	: convert packet to CAN type struct
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_convert(struct CANRCVTIMBUF* pout, struct PKTP *pp)
{
	pout->U.ull 	= *(unsigned long long*)pp->p;
	pout->R.id     	= *(unsigned int*)(pp->p + 8);
	pout->R.dlc    	= *(unsigned int*)(pp->p + 8 + 4);
	pout->R.cd.ull  = *(unsigned long long*)(pp->p + 8 + 4 + 4);
	return;	
}
/******************************************************************************
 * int getatime(int blk, struct PKTP *pktp);
 * @brief	: Read block and extract any time msg if present
 * @param	: blk: block number 
 * @return	:  1 = msg time < search time;
 *		:  0 = msg time >= search time;
 *		: -1 = large number examined and no time msg found
 ******************************************************************************/
int getatime(int blk, struct PKTP *pktp)
{
	struct CANRCVTIMBUF can;
	packet_convert(&can, pktp);
	return 0;
}


