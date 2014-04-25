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
/***************************************************************************
 * File Name  : AtmOsLinux.c
 *
 * Description: This file contains Linux operation system function calls.
 *              All operating system specific functions are isolated to a
 *              source file named AtmOs"xxx" where "xxx" is the operating
 *              system name.
 *
 * Updates    : 08/24/2001  lat.  Created from AtmOsVxWorks.c.
 ***************************************************************************/


/* Includes. */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/semaphore.h>
#include <asm/io.h>
#include <bcmtypes.h>
#include <atmosservices.h>
#include <bcm_map.h>
#include <bcm_intr.h>
#include <board.h>
#include <boardparms.h>


/* typedefs. */
typedef struct
{
    UINT32 ulInUse;
    UINT32 ulTimeout;
    struct tasklet_struct Task;
    struct timer_list TimerList;
} DEFERRED_INFO, *PDEFERRED_INFO;

typedef struct
{
    UINT32 ulBase;
    UINT32 ulOrder;
} PAGE_ALLOC_INFO, *PPAGE_ALLOC_INFO;

/* Defines. */
#define MAX_SEMS            50
#define MAX_PAGE_ALLOCS     50
#define MAX_INT_DATA_TIMERS 4

#define INTR_ATM            (1 << (INTERRUPT_ID_ATM - INTERNAL_ISR_TABLE_OFFSET))

/* Globals. */
static struct InternalData
{
    struct semaphore Sems[MAX_SEMS];
    DEFERRED_INFO DeferredInfo;
    PAGE_ALLOC_INFO PageAllocInfo[MAX_PAGE_ALLOCS];
    int nDisabledBh;
    int nRequestCount;
    char *pTopMemAddr;
    struct timer_list IntDataTimer[MAX_INT_DATA_TIMERS];
} *g_pData = NULL;


/***************************************************************************
 * Function Name: AtmOsInitialize
 * Description  : Operating system specific function initialization.
 * Returns      : RTN_SUCCESS if successful or error status.
 ***************************************************************************/
UINT32 AtmOsInitialize( PATM_OS_FUNCS pOsFuncs )
{
    g_pData = (struct InternalData *)
        kmalloc( sizeof(struct InternalData), GFP_KERNEL );

    if( g_pData )
    {
        unsigned short usValue;

        memset( g_pData, 0x00, sizeof(struct InternalData) );

        /* Initialize semaphore structures to all bits on in order to determine
         * a not in use state.
         */
        memset( g_pData->Sems, 0xff, sizeof(g_pData->Sems) );

        memset( &g_pData->DeferredInfo, 0x00, sizeof(g_pData->DeferredInfo) );
        memset( g_pData->PageAllocInfo, 0x00, sizeof(g_pData->PageAllocInfo) );
        g_pData->nDisabledBh = 0;
        g_pData->nRequestCount = 0;

        g_pData->pTopMemAddr = (char *) (DRAM_BASE + kerSysGetSdramSize());

        pOsFuncs->pfnAlloc           = (FN_GENERIC) AtmOsAlloc;
        pOsFuncs->pfnFree            = (FN_GENERIC) AtmOsFree;
        pOsFuncs->pfnDelay           = (FN_GENERIC) AtmOsDelay;
        pOsFuncs->pfnCreateSem       = (FN_GENERIC) AtmOsCreateSem;
        pOsFuncs->pfnRequestSem      = (FN_GENERIC) AtmOsRequestSem;
        pOsFuncs->pfnReleaseSem      = (FN_GENERIC) AtmOsReleaseSem;
        pOsFuncs->pfnDeleteSem       = (FN_GENERIC) AtmOsDeleteSem;
        pOsFuncs->pfnDisableInts     = (FN_GENERIC) AtmOsDisableInts;
        pOsFuncs->pfnEnableInts      = (FN_GENERIC) AtmOsEnableInts;
        pOsFuncs->pfnInvalidateCache = (FN_GENERIC) AtmOsInvalidateCache;
        pOsFuncs->pfnFlushCache      = (FN_GENERIC) AtmOsFlushCache;
        pOsFuncs->pfnGetTopMemAddr   = (FN_GENERIC) AtmOsTopMemAddr;
        pOsFuncs->pfnGetSystemTick   = (FN_GENERIC) AtmOsTickGet;
        pOsFuncs->pfnStartTimer      = (FN_GENERIC) AtmOsStartTimer;
        pOsFuncs->pfnPrintf          = (FN_GENERIC) AtmOsPrintf;
        if( BpGetWanDataLedGpio( &usValue ) == BP_SUCCESS )
            pOsFuncs->pfnBlinkLed    = (FN_GENERIC) AtmOsBlinkLed;
        else
            pOsFuncs->pfnBlinkLed    = (FN_GENERIC) NULL;
    }

    return( (g_pData) ? RTN_SUCCESS : RTN_ERROR );
} /* AtmOsInitialize */


/***************************************************************************
 * Function Name: AtmOsAlloc
 * Description  : Allocates kernel memory.
 * Returns      : Address of allocated memory of NULL.
 ***************************************************************************/
char *AtmOsAlloc( UINT32 ulSize )
{
    char *pRet = NULL;

    /* The largest block that kmalloc can allocate is 128K.  If the size is
     * greater than 128K, allocate memory pages.
     */
    if( ulSize < (1024 * 128)  )
    {
        if( in_softirq() || in_irq() )
            pRet = (char *) kmalloc(ulSize, GFP_ATOMIC);
        else
            pRet = (char *) kmalloc(ulSize, GFP_KERNEL);
    }
    else
    {
        /* Memory pages need to be allocated.  The number of pages must be an
         * exponent of 2
         */
        PPAGE_ALLOC_INFO pPai;
        UINT32 i;
        for(i=0, pPai=g_pData->PageAllocInfo; i < MAX_PAGE_ALLOCS; i++, pPai++)
        {
            if( pPai->ulBase == 0 )
            {
                pPai->ulOrder = 0;
                while( ulSize > (PAGE_SIZE * (1 << pPai->ulOrder)))
                    pPai->ulOrder++;
                pPai->ulBase = __get_free_pages(GFP_KERNEL, pPai->ulOrder);
                pRet = (char *) pPai->ulBase;
                break;
            }
        }
    }

    return( pRet );
} /* AtmOsAlloc */


/***************************************************************************
 * Function Name: AtmOsFree
 * Description  : Frees memory.
 * Returns      : None.
 ***************************************************************************/
void AtmOsFree( char *pBuf )
{
    PPAGE_ALLOC_INFO pPai;
    UINT32 i;

    if( pBuf )
    {
        for(i=0, pPai=g_pData->PageAllocInfo; i < MAX_PAGE_ALLOCS; i++, pPai++)
        {
            if( pPai->ulBase == (UINT32) pBuf )
            {
                free_pages( pPai->ulBase, pPai->ulOrder );
                pPai->ulBase = pPai->ulOrder = 0;
                break;
            }
        }

        if( i == MAX_PAGE_ALLOCS )
            kfree( pBuf );
    }
}


/***************************************************************************
 * Function Name: AtmOsCreateThread
 * Description  : Calls kernel_thread to create a new thread of execution.
 *                Parameters ulPriority, ulStackSize and pulThreadId are
 *                not used.
 * Returns      : RTN_SUCCESS if successful or error status.
 ***************************************************************************/
UINT32 AtmOsCreateThread( char *pszName, void *pFnEntry, UINT32 ulFnParm,
    UINT32 ulPriority, UINT32 ulStackSize, UINT32 *pulThreadId )
{
    typedef int (*FN_GENERIC_INT) (void *);
    int nRet = kernel_thread( (FN_GENERIC_INT) pFnEntry, (void *) ulFnParm, 0 );
    return( (nRet < 0) ? RTN_ERROR : RTN_SUCCESS );
} /* AtmOsCreateThread */


/***************************************************************************
 * Function Name: AtmOsCreateSem
 * Description  : Finds an unused semaphore and initializes it.
 * Returns      : Semaphore handle if successful or NULL.
 ***************************************************************************/
UINT32 AtmOsCreateSem( UINT32 ulInitialState )
{
    UINT32 i;
    struct semaphore *pSem = NULL;
    struct semaphore *pSemRet = NULL;

    /* Find an unused semaphore structure. */
    for( i = 0, pSem = g_pData->Sems; i < MAX_SEMS; i++, pSem++ )
    {
        if( (pSem->count.counter) == ~0 )
        {
            /* An unused semaphore is found.  Initialize it. */
            memset( pSem, 0x00, sizeof(struct semaphore) );
            sema_init( pSem, (int) ulInitialState );
            pSemRet = pSem;
            break;
        }
    }

    return( (UINT32) pSemRet );
} /* AtmOsCreateSem */


/***************************************************************************
 * Function Name: AtmOsDeleteSem
 * Description  : Makes semaphore available.
 * Returns      : None.
 ***************************************************************************/
void AtmOsDeleteSem( UINT32 ulSem )
{
    /* Mark semaphore unused. */
    memset( (char *) ulSem, 0xff, sizeof(struct semaphore) );
} /* AtmOsDeleteSem */


/***************************************************************************
 * Function Name: AtmOsRequestSemTo
 * Description  : Timeout handler for AtmOsRequestSem
 * Returns      : 0 if successful.
 ***************************************************************************/
void AtmOsRequestSemTo( UINT32 ulSem )
{
    up( (struct semaphore *) ulSem );
} /* AtmOsRequestSemTo */


/***************************************************************************
 * Function Name: AtmOsRequestSem
 * Description  : Requests ownership of the semaphore. ulTimeout is in ms.
 * Returns      : 0 if successful.
 ***************************************************************************/
UINT32 AtmOsRequestSem( UINT32 ulSem, UINT32 ulTimeout )
{
    int nRet = 0;

    /* A bottom half should run without any task context switches and also
     * should not block.  Therefore, just return if currently executing in a bh.
     */
    g_pData->nRequestCount++;
    if( !in_softirq() )
    {
        /* If the timeout is big, no need to start timer. */
        if( ulTimeout > 0x80000000 )
        {
            down_interruptible( (struct semaphore *) ulSem );

            /* At this point, the current thread is protected from other tasks
             * but not from a bottom half.  Disable bottom halves.  Doing this
             * assumes that the semaphore is being used as a mutex.  A future
             * change may be necessary to have both critical section functions
             * and synchronization functions that use semaphores.
             */
            if( g_pData->nDisabledBh == 0 )
            {
                g_pData->nDisabledBh = 1;
                local_bh_disable();
            }
        }
        else
        {
            struct timer_list Timer;
            int nTimerDeleted;

            /* Convert ms to jiffies.  If the timeout is less than the
             * granularity of the system clock, wait one jiffy.
             */
            if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
                ulTimeout = 1;

            init_timer (&Timer);
            Timer.expires = jiffies + ulTimeout;
            Timer.function = AtmOsRequestSemTo;
            Timer.data = ulSem;

            add_timer (&Timer);
            nRet = down_interruptible( (struct semaphore *) ulSem );
            nTimerDeleted = del_timer( &Timer );

            /* If the timer is still active and the semaphore was not
             * interrupted, then there was a timeout.
             */
            if( nTimerDeleted == 0 && !nRet )
                nRet = -1; /* Timed out. */
        }

    }

    return( nRet );
} /* AtmOsRequestSem */


/***************************************************************************
 * Function Name: AtmOsReleaseSem
 * Description  : Releases ownership of the semaphore.
 * Returns      : None.
 ***************************************************************************/
void AtmOsReleaseSem( UINT32 ulSem )
{
    if( --g_pData->nRequestCount <= 0 )
    {
        g_pData->nRequestCount = 0;

        /* If AtmOsRequestSem had disabled bottom havles, reenable here. */
        if( g_pData->nDisabledBh == 1 )
        {
            g_pData->nDisabledBh = 0;
            local_bh_enable();
        }
    }

    if( !in_softirq() )
        up( (struct semaphore *) ulSem );
} /* AtmOsReleaseSem */


/***************************************************************************
 * Function Name: AtmOsRequestSemCli
 * Description  : AtmOsRequestSem prototype but disables interrupts.
 * Returns      : 0 if successful.
 ***************************************************************************/
UINT32 g_ulSemDepth = 0;
UINT32 g_ulSemStack[10] = {0};
UINT32 AtmOsRequestSemCli( UINT32 ulSem, UINT32 ulTimeout )
{
    g_ulSemStack[g_ulSemDepth++] = (PERF->IrqMask & INTR_ATM);
    BcmHalInterruptDisable( INTERRUPT_ID_ATM );
    return(0);
} /* AtmOsRequestSem */


/***************************************************************************
 * Function Name: AtmOsReleaseSemSti
 * Description  : AtmosReleaseSem prototype but restores interrupts.
 * Returns      : None.
 ***************************************************************************/
void AtmOsReleaseSemSti( UINT32 ulSem )
{
    UINT32 ulMask = INTR_ATM;
    if( g_ulSemDepth > 0 )
        ulMask = g_ulSemStack[--g_ulSemDepth];
    else
        printk("AtmOsReleaseSem: **** g_ulSemDepth is 0 ****\n");
    if( ulMask == INTR_ATM )
        BcmHalInterruptEnable( INTERRUPT_ID_ATM );
} /* AtmOsReleaseSem */


/***************************************************************************
 * Function Name: AtmOsDisableInts
 * Description  : Disables interrupts.
 * Returns      : 1 for compatibility
 ***************************************************************************/
UINT32 AtmOsDisableInts( void )
{
    UINT32 flags;

    local_save_flags(flags);
    local_irq_disable();
    return( flags );
} /* AtmOsDisableInts */


/***************************************************************************
 * Function Name: AtmOsEnableInts
 * Description  : Enables interrupts.
 * Returns      : None.
 ***************************************************************************/
void AtmOsEnableInts( UINT32 flags )
{
    local_irq_restore(flags);
} /* AtmOsEnableInts */


/***************************************************************************
 * Function Name: AtmOsDelay
 * Description  : Delays a specified number of milliseconds.
 * Returns      : None.
 ***************************************************************************/
void AtmOsDelay( UINT32 ulTimeout )
{
    if( !in_softirq() && !in_irq() )
    {
        wait_queue_head_t wait;

        /* Convert ms to jiffies.  If the timeout is less than the granularity of
         * the system clock, wait one jiffy.
         */
        if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
            ulTimeout = 1;

        init_waitqueue_head(&wait);
        interruptible_sleep_on_timeout(&wait, (int) ulTimeout );
    }
} /* AtmOsDelay */


/***************************************************************************
 * Function Name: AtmOsTickGet
 * Description  : Returns the current number of milliseconds since the board
 *                was booted.
 * Returns      : Current number of milliesconds.
 ***************************************************************************/
UINT32 AtmOsTickGet( void )
{
    return( jiffies * (1000 / HZ) );
} /* AtmOsTickGet */


/***************************************************************************
 * Function Name: AtmOsTickCheck
 * Description  : Calculates if the number of milliseconds has expired.
 * Returns      : 1 if the number of milliseconds has expired, 0 if not.
 ***************************************************************************/
UINT32 AtmOsTickCheck( UINT32 ulWaitBase, UINT32 ulMsToWait )
{
    return( time_before(jiffies,ulWaitBase + ((HZ * ulMsToWait) / 1000))
        ? 0 : 1 );
} /* AtmOsTickCheck */


/***************************************************************************
 * Function Name: AtmOsInvalidateCache
 * Description  : Flushes the data cache for the specified buffer and length.
 * Returns      : None.
 ***************************************************************************/
void AtmOsFlushCache( void *pBuf, UINT32 ulLength )
{
    dma_cache_wback_inv((unsigned long)pBuf, ulLength);
} /* AtmOsFlushCache */


/***************************************************************************
 * Function Name: AtmOsInvalidateCache
 * Description  : Invalidates the data cache for the specified buffer and
 *                length.
 * Returns      : None.
 ***************************************************************************/
void AtmOsInvalidateCache( void *pBuf, UINT32 ulLength )
{
    dma_cache_inv((unsigned long)pBuf, ulLength);
} /* AtmOsInvalidateCache */


/***************************************************************************
 * Function Name: AtmOsTopMemAddr
 * Description  : Returns top of memory address.
 * Returns      : Top of memory address.
 ***************************************************************************/
char *AtmOsTopMemAddr( void )
{
    return( g_pData->pTopMemAddr );
} /* AtmOsTopMemAddr */


/***************************************************************************
 * Function Name: AtmOsBlinkLed
 * Description  : Calls function to blink the LED.
 * Returns      : None.
 ***************************************************************************/
void AtmOsBlinkLed( void )
{
    kerSysLedCtrl( kLedWanData, kLedStateBlinkOnce );
} /* AtmOsBlinkLed */


/***************************************************************************
 * Function Name: AtmOsDeferredTo
 * Description  : Timeout handler for AtmOsInitDeferredHandler.
 * Returns      : 0 if successful.
 ***************************************************************************/
void AtmOsDeferredTo( UINT32 ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;
    if( pDeferredInfo->ulInUse == 1 )
    {
        /* Schedule the bottom half. */
        tasklet_schedule(&pDeferredInfo->Task);

        /* Restart the timer. */
        pDeferredInfo->TimerList.expires = jiffies + pDeferredInfo->ulTimeout;
        pDeferredInfo->TimerList.function = AtmOsDeferredTo;
        pDeferredInfo->TimerList.data = (UINT32) pDeferredInfo;
        add_timer( &pDeferredInfo->TimerList );
    }
} /* AtmOsDeferredTo */


/***************************************************************************
 * Function Name: AtmOsInitDeferredHandler
 * Description  : Sets up a function for post ISR processing.
 * Returns      : handle that is used in subsequent calls or 0.
 ***************************************************************************/
UINT32 AtmOsInitDeferredHandler( void *pFnEntry, UINT32 ulFnParm,
    UINT32 ulTimeout )
{
    UINT32 ulHandle = 0;

    if( g_pData->DeferredInfo.ulInUse == 0 )
    {
        g_pData->DeferredInfo.ulInUse = 1;

        /* Initialize a tasklet. */
        tasklet_init(&(g_pData->DeferredInfo.Task), (void *) pFnEntry, (unsigned long) ulFnParm);

        /* Start a timer.  Convert ms to jiffies.  If the timeout is less than
         * the granularity of the system clock, wait one jiffy.
         */
        if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
            g_pData->DeferredInfo.ulTimeout = 1;
        else
            g_pData->DeferredInfo.ulTimeout = ulTimeout;

        init_timer( &g_pData->DeferredInfo.TimerList );
        g_pData->DeferredInfo.TimerList.expires = jiffies + g_pData->DeferredInfo.ulTimeout;
        g_pData->DeferredInfo.TimerList.function = AtmOsDeferredTo;
        g_pData->DeferredInfo.TimerList.data = (UINT32) &g_pData->DeferredInfo;
        add_timer( &g_pData->DeferredInfo.TimerList );

        ulHandle = (UINT32) &g_pData->DeferredInfo;
    }
    return( ulHandle );
} /* AtmOsInitDeferredHandler */


/***************************************************************************
 * Function Name: AtmOsScheduleDeferred
 * Description  : Schedules the deferred processing function to run.
 * Returns      : None.
 ***************************************************************************/
void AtmOsScheduleDeferred( UINT32 ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;

    if( pDeferredInfo->ulInUse == 1 )
    {
        tasklet_schedule(&pDeferredInfo->Task);
    }
} /* AtmOsScheduleDeferred */


/***************************************************************************
 * Function Name: AtmOsUninitDeferredHandler
 * Description  : Uninitializes the deferred handler resources.
 * Returns      : handle that is used in subsequent calls or 0.
 ***************************************************************************/
void AtmOsUninitDeferredHandler( UINT32 ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;

    if( pDeferredInfo->ulInUse == 1 )
    {
        pDeferredInfo->ulInUse = 2;
        del_timer( &pDeferredInfo->TimerList );
        pDeferredInfo->ulInUse = 0;
    }
} /* AtmOsUninitDeferredHandler */


/***************************************************************************
 * Function Name: AtmOsStartTimer
 * Description  : Starts a timer.
 * Returns      : 0 if successful, -1 if not
 ***************************************************************************/
UINT32 AtmOsStartTimer( void *pFnEntry, UINT32 ulFnParm, UINT32 ulTimeout )
{
    UINT32 ulRet = (UINT32) -1;
    struct timer_list *pTimer;
    int i;

    for( i = 0, pTimer = &g_pData->IntDataTimer[0]; i < MAX_INT_DATA_TIMERS;
        i++, pTimer++ )
    {
        if( !timer_pending( pTimer ) )
        {
            /* Start a timer.  Convert ms to jiffies.  If the timeout is less than
             * the granularity of the system clock, wait one jiffy.
             */
            if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
                ulTimeout = 1;

            init_timer( pTimer );
            pTimer->expires = jiffies + ulTimeout;
            pTimer->function = pFnEntry;
            pTimer->data = ulFnParm;
            add_timer( pTimer );
            ulRet = 0;
            break;
        }
    }

    return( ulRet );
} /* AtmOsStartTimer */


/***************************************************************************
 * Function Name: AtmOsPrintf
 * Description  : Outputs text to the console.
 * Returns      : None.
 ***************************************************************************/
void AtmOsPrintf( char *pFmt, ... )
{
    va_list args;
    char buf[256];

    va_start(args, pFmt);
    vsnprintf(buf, sizeof(buf), pFmt, args);
    va_end(args);

    printk(buf);
} /* AtmOsPrintf */

