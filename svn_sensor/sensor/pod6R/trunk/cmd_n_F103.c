/******************************************************************************
* File Name          : cmd_n_F103.c
* Date First Issued  : 06/07/2015
* Board              : F103
* Description        : Once per sec CAN bus id summary (cangate cmd_n equivalent)
*******************************************************************************/

#include <malloc.h>
#include <stdio.h>
#include "can_driver.h"
#include "DTW_counter.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"

#define CANIDTABLESIZEDEFAULT	32

struct CANIN
{
	unsigned int	canid;
	unsigned int	ct;
};
static struct CANIN* pbase;	// Pointer to beginning of table
static struct CANIN* pend;	// Pointer to end of table + 1
static unsigned int tik = 0;
static unsigned int tblsize = 0;
static unsigned int flag_ov = 0;

/* **************************************************************************************
 * int32_t cmd_n_init_F103(unsigned int count);
 * @brief	: Purloin some memory for this mess
 * @param	: count = size of CAN id table
 * @return	: 0 = OK, -1 = fail
 * ************************************************************************************** */
int32_t cmd_n_init_F103(unsigned int count)
{

	pbase = (struct CANIN*)calloc(count,sizeof(struct CANIN));
	if (pbase == NULL) return -1;
	tblsize = count;	// Save size locally
	DTW_counter_init();	// Init DTW counter for timing
	pend  = (pbase + count);
	return 0;
}
/* **************************************************************************************
 * void cmd_n_F103(struct CANRCVBUF* p);
 * @brief	: make list of different CAN ids and counts
 * @param	: p = Pointer to CAN msg
 * ************************************************************************************** */
void cmd_n_F103(struct CANRCVBUF* p)
{
	struct CANIN* ptmp;
	int total;

	if (tblsize == 0)
	{ 
		tblsize = CANIDTABLESIZEDEFAULT;
		cmd_n_init_F103(CANIDTABLESIZEDEFAULT);
	}
/* NOTE: The last entry in the table is followed by a zero CAN id. */
	/* Check if this ID is in the list. */
	for (ptmp = pbase; ptmp->canid != 0; ptmp++)
	{
		if (ptmp->canid == p->id)
		{ // Here, we found it
			ptmp->ct += 1;	// Tally this CAN id

//if (((int)(DTWTIME - t_cmdn)) > 0) // Has the time expired?
			if (ptmp->canid == 0x00400000)	// Time tick msg?
			{ // Here, 1/64th sec tick from GPS
//t_cmdn = DTWTIME + 168000000; 
				if ((tik & 63) == 0) // 64 time ticks per second
				{ // Here, print one second summary line
					total = 0; // Total for this interval
					/* Print list        */
					for (ptmp = pbase; ptmp->canid != 0; ptmp++)
					{
						if ((ptmp->canid & ~0xffe00000) == 0)
						{ // Compress 11 b CAN id 
							printf("%03x %d|",(ptmp->canid >> 20),ptmp->ct);
						}
						else
						{ // Full CAN id
							printf("%08x %d|", ptmp->canid,ptmp->ct);
						}
						total += ptmp->ct;	// Sum for total
						ptmp->ct = 0;	// Reset count for next interval
					}
					printf (" %d\n\r",total);
					if ((tik & 4095) == 0) // Reset list
					{
						pbase->canid = 0;
						printf("\tRESET\n\r");
					}
					USART1_txint_send();
				}
				tik += 1;
				flag_ov = 0;
			}
			return;
		}
	}
	/* Here, the search failed. Add ID to list. */
	if (ptmp >= (pend - 2))
	{ // Count errors if table is full
		flag_ov += 1;
		return;
	}
	ptmp->canid = p->id;
	ptmp->ct = 1;
	ptmp++;
	ptmp->canid = 0;
	return;
}

