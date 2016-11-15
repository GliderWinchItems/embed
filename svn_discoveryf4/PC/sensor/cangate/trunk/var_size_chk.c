/******************************************************************************
* File Name          : var_size_chk.c
* Date First Issued  : 01/07/2015
* Board              : 
* Description        : Check sizes of variables
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>

/******************************************************************************
 * int var_size_chk(void);
 * @brief 	: Check that sizes of variables are what we expect
 * @return	: 0 = OK, not zero = failed.
*******************************************************************************/
int var_size_chk(void)
{
	/* Test that ints and floats are 4 bytes. */
	if (	(sizeof(int)               != 4)||
		(sizeof(unsigned int)      != 4)||
		(sizeof(long long)         != 8)||
		(sizeof(unsigned long long)!= 8)||
		(sizeof(float)             != 4)||
		(sizeof(double)            != 8)||
		(sizeof(long double)       != 16)||
		(sizeof(long)              != 8)||
		(sizeof(int32_t) 	   != 4)||
		(sizeof(int64_t)           != 8)||
		(sizeof(uint32_t)          != 4)||
		(sizeof(uint64_t)          != 8))
	{
		printf("SERIOUS ERROR! The size of int, unsigned int, and float are expected "
			"to be 4 bytes.  They were found to be:\n");
		printf("sizeof(int)                = %u\n",(int)sizeof(int));
		printf("sizeof(unsigned int)       = %u\n",(int)sizeof(unsigned int));
		printf("sizeof(float)              = %u\n",(int)sizeof(float));
		printf("sizeof(long long)          = %u\n",(int)sizeof(long long));
		printf("sizeof(unsigned long long) = %u\n",(int)sizeof(unsigned long long));
		printf("sizeof(double)             = %u\n",(int)sizeof(double));
		printf("sizeof(long double)        = %u\n",(int)sizeof(long double));
		printf("sizeof(long)               = %u\n",(int)sizeof(long double));
		printf("sizeof(int32_t)            = %u\n",(int)sizeof(int32_t));
		printf("sizeof(uint32_t)           = %u\n",(int)sizeof(uint32_t));
		printf("sizeof(int64_t)            = %u\n",(int)sizeof(int64_t));
		printf("sizeof(uint64_t)           = %u\n",(int)sizeof(uint64_t));
		printf("This may require modifying the .txt file for commands p and q,\n" 
			"   so that structs and parameters, calibrations, CAN IDs ('//i' lines) align properly.\n");
//		exit (-1);
		return -1;
	}
	return 0;
}
