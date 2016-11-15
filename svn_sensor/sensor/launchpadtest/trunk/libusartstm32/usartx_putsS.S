/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_putsS.S
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : Fast inner loop for adding chars to a buffer
*******************************************************************************/ 


/*******************************************************************************
* char* usartx_putsS(char* p, char** pnow,  char* pend);
* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: == 0 for buffer did not overflow; otherwise, return pointing to char not stored
*******************************************************************************/
//	while (*p != 0) 		// Store until zero terminator is reached
//	{
//
//	 	*USARTcb2.ptx_now++ = *p++;	// Store a char
//		if (USARTcb2.ptx_now >= (USARTcb2.ptx_begin_m + USART2_BUFFER_TX_LINE_SIZE) ) 
//		{
//			return p; 	// Here we hit the end of the buffer!
//		}
//	}
//	return 0;	
// Do above by substituting all of the above with the following (which calls the assembly routine)--
//	 return USARTx_putsS(p,&USARTcb2.ptx_now,&(USARTcb2.ptx_begin_m + USART2_BUFFER_TX_LINE_SIZE));
//
	.syntax unified
	.thumb
	.thumb_func
	.text
	.global	usartx_putsS
// r0, r1, r2 are set up by the calling program	
// r0 = p (input pointer)
// r1 = pnow (address of char output pointer)
// r2 = pend (end of output buffer pointer)
// r3 = temporary char
// r4 = char output pointer
usartx_putsS:
	push {r3,r4}		// Save registers that will be changed
	ldr	r4,[r1]		// r4 now points to output buffer	
loop:
	ldrb.w	r3,[r0],#1	// *p++
	cbz	r3,endloop	// Br if string terminator
	strb.w	r3,[r4],#1	// *USARTcb2.ptx_now++
	cmp	r4,r2		// At end of buffer?
	blo	loop		// Br no
	tst	r4,#0
endloop:
	it	ne		// Skip next, if PSR Z is not set
	// Skipping the following returns the input pointer pointing to next char not stored
	movne	r0,#0		// Return: Show we did not hit end of buffer
	str	r4,[r1]		// Update pointer USARTcb2.ptx_now
	pop {r3,r4}
	bx	lr
	
