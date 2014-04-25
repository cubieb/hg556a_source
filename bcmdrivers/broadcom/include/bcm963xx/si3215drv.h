/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : ac97drv.h
 *
 * Description: 
 *
 ***************************************************************************/

#if !defined(SI3215DRV_H)
#define SI3215DRV_H

#if defined(__cplusplus)
extern "C" {
#endif


typedef enum SI3215IOCTL_INDEX
{
   SI3215IO_INIT_INDEX = 0,
   SI3215IO_MODE_CTRL_INDEX,
   SI3215IO_IS_OFFHOOK_INDEX,
   SI3215IO_MAX_INDEX
} SI3215IOCTL_INDEX;


/* Defines. */
#define SI3215DRV_MAJOR            225 /* arbitrary unused value */

#define SI3215IOCTL_INIT \
    _IOWR(SI3215DRV_MAJOR, SI3215IO_INIT_INDEX, SI3215DRV_INIT_PARAM)

#define SI3215IOCTL_IS_OFFHOOK \
    _IOWR(SI3215DRV_MAJOR, SI3215IO_IS_OFFHOOK_INDEX, SI3215DRV_IS_OFFHOOK_PARAM)

#define SI3215IOCTL_MODE_CTRL \
    _IOWR(SI3215DRV_MAJOR, SI3215IO_MODE_CTRL_INDEX, SI3215DRV_MODE_CTRL_PARAM)


#define MAX_SI3215DRV_IOCTL_COMMANDS   SI3215IO_MAX_INDEX

#define SI3215_MODE_OPEN	      0x00	/* TIP & RING tri-stated, (Disconnect state) */
#define SI3215_MODE_FWD_ACTIVE	0x01	/* Forward Active mode */
#define SI3215_MODE_FWD_OHT	   0x02	/* Onhook transmission, forward */
#define SI3215_MODE_TIP_OPEN	   0x03	/* TIP tri-stated, (Ground start state) */
#define SI3215_MODE_RINGING	   0x04	/* Ringing mode */
#define SI3215_MODE_REV_ACTIVE	0x05	/* Reverse Active Mode */
#define SI3215_MODE_REV_OHT	   0x06	/* Onhook Transmission, reverse */
#define SI3215_MODE_RING_OPEN	   0x07	/* RING tri-stated, (Scan state) */


typedef struct
{
   unsigned int      size;    /* Size of the structure (including the size field) */
   unsigned short    devNum;  /* device number to be initialized */
   unsigned long     data;
} SI3215DRV_INIT_PARAM, *PSI3215DRV_INIT_PARAM;


typedef struct
{
   unsigned int      size;    /* Size of the structure (including the size field) */
   unsigned short    devNum;  /* device number to be checked for hook status */
   unsigned long     data;
} SI3215DRV_IS_OFFHOOK_PARAM, *PSI3215DRV_IS_OFFHOOK_PARAM;

typedef struct
{
   unsigned int      size;       /* Size of the structure (including the size field) */
   unsigned short    devNum;     /* device number whose mode is to be changed */
   unsigned int      data;
} SI3215DRV_MODE_CTRL_PARAM, *PSI3215DRV_MODE_CTRL_PARAM;


#if defined(__cplusplus)
}
#endif

#endif // SI3215DRV_H
