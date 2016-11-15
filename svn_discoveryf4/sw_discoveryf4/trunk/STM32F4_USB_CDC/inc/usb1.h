/******************************************************************************
* File Name          : usb1.h
* Date               : 10/11/2013
* Board              : F4-Discovery
* Description        : USB PC<->CAN gateway: initialzation, etc.
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB1_H
#define __USB1_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

void usb1_init(void);

#endif

