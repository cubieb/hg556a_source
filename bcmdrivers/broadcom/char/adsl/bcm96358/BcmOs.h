/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
//**************************************************************************
// File Name  : BcmOs.c
//
// Description: This is the header file for Linux OS calls
//
//**************************************************************************
#if !defined(_BCMOS_H_)
#define _BCMOS_H_

#if defined(_WIN32_WCE)
typedef	unsigned char	Bool;
typedef ULONG (*FN_ISR) (ULONG);

#ifdef DEBUG
    #define DBG_MSG		1
#else
    #define DBG_MSG		0
#endif

#elif defined(_CFE_)
#include "lib_types.h"
#include "cfe_timer.h"
#else
#include <asm/param.h>
#ifndef _SYS_TYPES_H
#include <linux/types.h>
#define	_SYS_TYPES_H
#endif

#endif /* _WIN32_WCE || CFE */

#define LOCAL static 

typedef unsigned long   OS_SEMID;       /* Linux semaphone ID type */
typedef unsigned long   OS_TIME;        /* Linux time type */
typedef unsigned long   OS_TICKS;       /* Linux tick type */
typedef unsigned long   OS_TASKID;      /* Linux task ID type */
typedef unsigned long   OS_TASKARG;     /* Linux task start argument */
typedef unsigned long   OS_STATUS;      /* Linux return status type */

#define OS_STATUS_OK  1
#define OS_STATUS_FAIL  0
#define OS_STATUS_TIMEOUT -1

extern int	gConsoleOutputEnable;

#ifdef _WIN32_WCE
#define HZ					1000
#define BCMOS_EVENT_LOG(x)	NKDbgPrintfW x
#define	KERN_CRIT
#elif defined(_CFE_)
#define HZ					CFE_HZ
#define	TEXT(__str__)		__str__
#define BCMOS_EVENT_LOG(x)	printf x
#define	KERN_CRIT
#define printk              printf
#else   /* !_WIN32_WCE */
#define	TEXT(__str__)		__str__
#define BCMOS_EVENT_LOG(x)	if(gConsoleOutputEnable) printk x
#endif /* _WIN32_WCE */

#define BCMOS_MSEC_PER_TICK (1000/HZ)
#define OS_WAIT_FOREVER  (OS_TIME) -1

OS_STATUS BcmOsInitialize( void );
OS_STATUS BcmOsUninitialize( void );
OS_STATUS bcmOsTaskCreate( char* name, int stackSize, int priority,
                       void* taskEntry, OS_TASKARG argument, OS_TASKID *taskId );
OS_STATUS bcmOsTaskDelete( OS_TASKID taskId );
OS_STATUS bcmOsSemCreate(char *semName, OS_SEMID *semId);
OS_STATUS bcmOsSemDelete( OS_SEMID semId );
OS_STATUS bcmOsSemTake(OS_SEMID semId, OS_TIME timeout);
OS_STATUS bcmOsSemGive( OS_SEMID semId );
OS_STATUS bcmOsGetTime(OS_TICKS *osTime);
OS_STATUS bcmOsSleep( OS_TIME timeout );

void * bcmOsDpcCreate(void* dpcEntry, void * arg);
void bcmOsDpcEnqueue(void* dpcHandle);
void * bcmOsTimerCreate(void* timerEntry, void * arg);
void bcmOsTimerStart(void* timerHandle, int timeout);
void bcmOsDelay(unsigned long timeMs);

#ifdef _WIN32_WCE
void BcmHalInterruptEnable (int intrId);
void BcmHalInterruptDisable (int intrId);
void BcmHalMapInterrupt(FN_ISR pIntrHandler, int param, int intrId);
#else
void *calloc( unsigned long num, unsigned long size );
void free( void *memblock );
#endif


#endif // _BCMOS_H_
