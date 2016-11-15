/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txcir_putsS.S
* Hackor             : deh
* Date First Issued  : 10/14/2010 deh
* Description        : Fast inner loop for adding chars to a buffer
*******************************************************************************/ 


/*******************************************************************************
* void usartx_txcir_putsS(char* p, char** pnow,  char* pbegin, char* pend);
* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @param	: Address of pointer to where chars are being stored (now)
* @param	: Pointer to beginning if circular buffer
* @param	: Pointer to end of circular buffer
* @return	: none
*******************************************************************************/
//	while (*p != 0)
//	{
//		*pUSARTcbtx->ptx_now_m++ = *p++;
//		if (pUSARTcbtx->ptx_now_m == pUSARTcbtx->ptx_end)
//			pUSARTcbtx->ptx_now_m = pUSARTcbtx->ptx_begin;
//	}
	
// Do above by substituting all of the above with the following (which calls the assembly routine)--
//	USARTx_txcir_putsS(p,&USARTcb2.ptx_now,&(USARTcb2.ptx_begin_m + USART2_BUFFER_TX_LINE_SIZE));
//	return;
//
	.syntax unified
	.thumb
	.thumb_func
	.text
	.global	usartx_txcir_putsS
// r0, r1, r2, r3 are set up by the calling program	
// r0 = p (input pointer)
// r1 = &pnow   (address of char output pointer)
// r2 - pbegin (address of beginning of circular buffer)
// r3 = pend   (address of end of circular buffer)
// r4 = pnow   (address where chars are being added in circular buffer)
// r5 = char   (temp)
usartx_txcir_putsS:
	push {r4,r5}		// Save registers that will be changed
	ldr	r4,[r1]		// r4 points to output buffer 'now_m'
loop:
	ldrb.w	r5,[r0],#1	// *p++  get char to be added
	cbz	r5,endloop	// Br if string terminator
	strb.w	r5,[r4],#1	// *USARTcb2.ptx_now++ store char in buffer
	cmp	r4,r3		// At end of buffer?
	blo	loop		// Br no
	mov	r4,r2		// Reset to beginning
	b	loop		// Continue loop
endloop:
	str	r4,[r1]		// Update pointer USARTcb2.ptx_now
	pop {r4,r5}		// Restore saved registers
	bx	lr		// Return
	
