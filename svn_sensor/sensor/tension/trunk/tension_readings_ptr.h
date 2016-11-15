/******************************************************************************
* File Name          : tension_readings_ptr.h
* Date First Issued  : 06/07/2016
* Board              :
* Description        : Pointer to reading versus code
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TENSION_READINGS_PTR
#define __TENSION_READINGS_PTR

struct READINGSPTR
{
	void* ptr;
	int32_t code;
};

extern const struct READINGSPTR tension_readings_ptr[];

#endif

