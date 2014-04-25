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
// File Name  : Adsl.c
//
// Description: This file contains API for ADSL PHY
//
//**************************************************************************

#ifdef _WIN32_WCE
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <memory.h>
#include <linklist.h>
#include <nkintr.h>
#include <hwcomapi.h>
#include <devload.h>
#include <pm.h>
#elif defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "boardparms.h"
#elif defined(TARG_OS_RTEMS)
#include <alloc.h>
#include <xapi.h>
#include "types.h"
#include "bspcfg.h"
#define ulong unsigned long
#else
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include "boardparms.h"
#endif

#ifndef _WIN32_WCE
#include "bcmtypes.h"
#endif

#include "softdsl/AdslCoreDefs.h"

#include "BcmOs.h"
#include "board.h"
#if defined(CONFIG_BCM96338)
#include "6338_map.h"
#endif
#if defined(CONFIG_BCM96358)
#include "6358_map.h"
#endif
#if defined(CONFIG_BCM96348)
#include "6348_map.h"
#endif
#if !defined(TARG_OS_RTEMS)
#include "bcm_intr.h"
#endif
#include "bcmadsl.h"
#include "BcmAdslCore.h"

#if !defined(__KERNEL__)
#define KERNEL_VERSION(a,b,c) 0x7fffffff
#endif

extern void BcmAdslCoreDiagWriteStatusString(char *fmt, ...);

#if !defined(TARG_OS_RTEMS)
LOCAL void		*g_TimerHandle;

LOCAL void BcmAdsl_Status(void);
LOCAL void BcmAdsl_Timer(void * arg);
#else
#define calloc(L,X)     xmalloc(L)
#define free(P)         xfree(P)
extern void AdslLinkReset(void);
LOCAL OS_SEMID  g_StatusSemId;
LOCAL OS_TASKID g_StatusTid;

LOCAL void StatusTask(void);
#endif


#ifdef VDF_SPAIN
int g_ulAdslCuts = 0;
int g_ulAdslOnLineStart = 0;
#endif



LOCAL void IdleNotifyCallback (ADSL_LINK_STATE AdslLinkState, UINT32 ulParm)
{
}

#define ADSL_RJ11_INNER_PAIR	0
#define ADSL_RJ11_OUTER_PAIR	1

LOCAL void SetRj11Pair( UINT16 usPairToEnable, UINT16 usPairToDisable );

LOCAL ADSL_FN_NOTIFY_CB g_pFnNotifyCallback = IdleNotifyCallback;
LOCAL UINT32 g_ulNotifyCallbackParm;
ADSL_LINK_STATE g_LinkState;
LOCAL unsigned short g_GpioInnerPair = 0xffff;
LOCAL unsigned short g_GpioOuterPair = 0xffff;
LOCAL unsigned short g_BoardType;
LOCAL int			 g_RJ11Pair = ADSL_RJ11_INNER_PAIR;
#ifndef DYING_GASP_API
#if defined(__KERNEL__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
LOCAL irqreturn_t BcmDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs);
#else
LOCAL unsigned int BcmDyingGaspIsr( void );
#endif
#endif
int g_nAdslExit = 0;
int g_nAdslInitialized = 0;
//LGD_FOR_TR098
unsigned long g_ShowtimeStartTicks = 0;
#ifdef _WIN32_WCE

BCMADSL_STATUS BcmAdsl_Initialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, adslCfgProfile *pAdslCfg)
{
    DEBUGMSG (DBG_MSG, (TEXT("BcmAdsl_Initialize=0x%08X, &g_nAdslExit=0x%08X\n"), (int)BcmAdsl_Initialize, (int) &g_nAdslExit));

    BcmOsInitialize ();

    g_pFnNotifyCallback = (pFnNotifyCb != NULL) ? pFnNotifyCb : IdleNotifyCallback;
    g_ulNotifyCallbackParm = ulParm;

    //BcmAdslCoreSetSDRAMBaseAddr((void *) (((ulong) kerSysGetSdramSize() - 0x40000) | 0xA0000000));
    BcmAdslCoreSetSDRAMBaseAddr((void *) ((0x800000 - 0x40000) | 0xA0000000));
	BcmAdslCoreConfigure(pAdslCfg);

    BcmAdslCoreInit();
    g_LinkState = BCM_ADSL_LINK_DOWN;
    g_nAdslExit = 0;
	g_nAdslInitialized = 1;
	g_TimerHandle = bcmOsTimerCreate(BcmAdsl_Timer, NULL);
	if (NULL != g_TimerHandle)
		bcmOsTimerStart(g_TimerHandle, 1000);

	return BCMADSL_STATUS_SUCCESS;
}

#elif !defined(TARG_OS_RTEMS)

//**************************************************************************
// Function Name: BcmAdsl_Initialize
// Description  : Initializes ADSL PHY.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_Initialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, adslCfgProfile *pAdslCfg)
{

    printk("BcmAdsl_Initialize=0x%08X, g_pFnNotifyCallback=0x%08X\n", (int)BcmAdsl_Initialize, (int) &g_pFnNotifyCallback);

    if (g_nAdslInitialized != 0) {
		BcmAdslCoreConfigure(pAdslCfg);
		return BCMADSL_STATUS_SUCCESS;
	}

    BcmOsInitialize ();
#ifndef DYING_GASP_API
	{
	unsigned long	ulIntr;
#if defined(CONFIG_BCM963x8)
#if defined(__KERNEL__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) 
#endif
	{
		BcmHalMapInterrupt((void *)BcmDyingGaspIsr, 0, INTERRUPT_ID_DG);
		BcmHalInterruptEnable( INTERRUPT_ID_DG );
    }
#endif
	}
#endif
    if( BpGetRj11InnerOuterPairGpios(&g_GpioInnerPair, &g_GpioOuterPair) == BP_SUCCESS ) {
        g_GpioInnerPair = GPIO_NUM_TO_MASK(g_GpioInnerPair);
        g_GpioOuterPair = GPIO_NUM_TO_MASK(g_GpioOuterPair);
    }
    else {
        g_GpioInnerPair = 0xffff;
        g_GpioOuterPair = 0xffff;
    }
    g_BoardType = 0;
    g_pFnNotifyCallback = (pFnNotifyCb != NULL) ? pFnNotifyCb : IdleNotifyCallback;
    g_ulNotifyCallbackParm = ulParm;

    BcmAdslCoreSetSDRAMBaseAddr((void *) (((unsigned long) kerSysGetSdramSize() - 0x40000) | 0xA0000000));
	BcmAdslCoreConfigure(pAdslCfg);

    BcmAdslCoreInit();
    g_LinkState = BCM_ADSL_LINK_DOWN;
    g_nAdslExit = 0;
	g_nAdslInitialized = 1;
	g_TimerHandle = bcmOsTimerCreate(BcmAdsl_Timer, NULL);
	if (NULL != g_TimerHandle)
		bcmOsTimerStart(g_TimerHandle, 1000);

    
	return BCMADSL_STATUS_SUCCESS;
}
#else /* defined(TARG_OS_RTEMS) */
BCMADSL_STATUS BcmAdsl_Initialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, adslCfgProfile *pAdslCfg)
{
    typedef void (*FN_HDLR) (unsigned long);
    BcmOsInitialize ();

#if 0
/* The interrupt handling of the Dying gasp is controlled external */
    BcmHalMapInterrupt((FN_HDLR)BcmDyingGaspIsr, 0, INTERRUPT_ID_DYING_GASP);
    BcmHalInterruptEnable( INTERRUPT_ID_DYING_GASP );
#if defined(BOARD_bcm96348) || defined(BOARD_bcm96338)
    BcmHalMapInterrupt((FN_HDLR)BcmDyingGaspIsr, 0, INTERRUPT_ID_DG);
    BcmHalInterruptEnable( INTERRUPT_ID_DG );
#endif
#endif

    g_pFnNotifyCallback = (pFnNotifyCb != NULL) ? pFnNotifyCb : IdleNotifyCallback;
    g_BoardType = 0;
    g_ulNotifyCallbackParm = ulParm;
    bcmOsSemCreate(NULL, &g_StatusSemId);

    /* kerSysGetSdramSize subtracts the size reserved for the ADSL MIPS */
    BcmAdslCoreSetSDRAMBaseAddr((void *)
        ((unsigned long) kerSysGetSdramSize() | 0xA0000000));
    BcmAdslCoreConfigure(pAdslCfg);

    BcmAdslCoreInit();
    g_LinkState = BCM_ADSL_LINK_DOWN;
    g_nAdslExit = 0;
	g_nAdslInitialized = 1;
    bcmOsTaskCreate("ADSL", 20*1024, 50, StatusTask, 0, &g_StatusTid);
    
    return BCMADSL_STATUS_SUCCESS;
}
#endif

//**************************************************************************
// Function Name: BcmAdsl_Check
// Description  : Checks presense of ADSL phy which is always present on
//                the BCM63xx.
// Returns      : STS_SUCCESS if ADSL board found
//**************************************************************************
BCMADSL_STATUS BcmAdsl_Check(void)
{
    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_Uninitialize
// Description  : Uninitializes ADSL PHY.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_Uninitialize(void)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;

    g_nAdslExit = 1;
    BcmAdsl_Notify();
    BcmAdslCoreUninit();
	BcmOsUninitialize();
	g_nAdslInitialized = 0;
    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_ConnectionStart
// Description  : Start ADSL connections.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_ConnectionStart(void)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreConnectionStart();
    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_ConnectionStop
// Description  : Stop ADSL connections
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_ConnectionStop(void)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreConnectionStop();
    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_GetPhyAddresses
// Description  : Return addresses of Utopia ports for ADSL PHY
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_GetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr)
{
    pChannelAddr->usFastChannelAddr = 1;
    pChannelAddr->usInterleavedChannelAddr = 2;

    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SetPhyAddresses
// Description  : Configure addresses of Utopia ports
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr)
{
    return BCMADSL_STATUS_ERROR;
}

//**************************************************************************
// Function Name: BcmAdsl_GetConnectionInfo
// Description  : Return ADSL connection info
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_GetConnectionInfo(PADSL_CONNECTION_INFO pConnectionInfo)
{
    BcmAdslCoreGetConnectionInfo(pConnectionInfo);

    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_DiagCommand
// Description  : Process ADSL diagnostic command
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_DiagCommand(PADSL_DIAG pAdslDiag)
{
    BcmAdslCoreDiagCmd(pAdslDiag);
    return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SetObjectValue
// Description  : Set ADS MIB object
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
int BcmAdsl_SetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	return BcmAdslCoreSetObjectValue (objId, objIdLen, dataBuf, dataBufLen);
}
//**************************************************************************
// Function Name: BcmAdsl_GetObjectValue
// Description  : Get ADS MIB object
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
int BcmAdsl_GetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	return BcmAdslCoreGetObjectValue (objId, objIdLen, dataBuf, dataBufLen);
}

//**************************************************************************
// Function Name: BcmAdsl_StartBERT
// Description  : Start BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_StartBERT(unsigned long totalBits)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreStartBERT(totalBits);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_StopBERT
// Description  : Stops BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_StopBERT(void)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreStopBERT();
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_BertStartEx
// Description  : Start BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_BertStartEx(unsigned long bertSec)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreBertStartEx(bertSec);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_BertStopEx
// Description  : Stops BERT test
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_BertStopEx(void)
{
    if (0 == g_nAdslInitialized)
		return BCMADSL_STATUS_ERROR;
	BcmAdslCoreBertStopEx();
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SendDyingGasp
// Description  : Sends DyingGasp EOC message
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SendDyingGasp(int powerCtl)
{
	BcmAdslCoreSendDyingGasp(powerCtl);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_Configure
// Description  : Changes ADSL current configuration
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_Configure(adslCfgProfile *pAdslCfg)
{
	BcmAdslCoreConfigure(pAdslCfg);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_GetVersion
// Description  : Changes ADSL version information
// Returns      : STS_SUCCESS 
//**************************************************************************
BCMADSL_STATUS BcmAdsl_GetVersion(adslVersionInfo *pAdslVer)
{
	BcmAdslCoreGetVersion(pAdslVer);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_Get/Set OEM Parameter
// Description  : Gets or sets ADSL OEM parameter
// Returns      : # of bytes used
//**************************************************************************
int BcmAdsl_GetOemParameter (int paramId, void *buf, int len)
{
	return BcmAdslCoreGetOemParameter (paramId, buf, len);
}

int BcmAdsl_SetOemParameter (int paramId, void *buf, int len)
{
	return BcmAdslCoreSetOemParameter (paramId, buf, len);
}

int BcmAdsl_SetXmtGain(int gain)
{
	return BcmAdslCoreSetXmtGain(gain);
}

BCMADSL_STATUS BcmAdsl_SetSDRAMBaseAddr(void *pAddr)
{
	BcmAdslCoreSetSDRAMBaseAddr(pAddr);
	return BCMADSL_STATUS_SUCCESS;
}

BCMADSL_STATUS BcmAdsl_ResetStatCounters(void)
{
	BcmAdslCoreResetStatCounters();
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SetGfc2VcMapping
// Description  : Enables/disables GFC to VC mapping for ATM transmitter
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetGfc2VcMapping(int bOn)
{
	return BcmAdslCoreSetGfc2VcMapping(bOn) ? BCMADSL_STATUS_SUCCESS : BCMADSL_STATUS_ERROR;
}

//**************************************************************************
// Function Name: BcmAdsl_SetVcEntry
// Description  : Maps a port/vpi/vci to a GFC index.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetVcEntry (int gfc, int port, int vpi, int vci)
{
	return BcmAdslCoreSetVcEntry (gfc, port, vpi, vci, 0) ?
		BCMADSL_STATUS_SUCCESS : BCMADSL_STATUS_ERROR;
}

//**************************************************************************
// Function Name: BcmAdsl_SetVcEntryEx
// Description  : Maps a port/vpi/vci to a GFC index.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci, int pti_clp)
{
	return BcmAdslCoreSetVcEntry (gfc, port, vpi, vci, pti_clp) ?
		BCMADSL_STATUS_SUCCESS : BCMADSL_STATUS_ERROR;
}

//**************************************************************************
// Function Name: BcmAdsl_SetAtmLoopbackMode
// Description  : Sets ADSL to loopback ATM cells
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetAtmLoopbackMode()
{
	return BcmAdslCoreSetAtmLoopbackMode() ? 
		BCMADSL_STATUS_SUCCESS : BCMADSL_STATUS_ERROR;
}

//**************************************************************************
// Function Name: BcmAdsl_SetTestMode
// Description  : Sets ADSL special test mode
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetTestMode(ADSL_TEST_MODE testMode)
{
	BcmAdslCoreSetTestMode(testMode);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SelectTones
// Description  : Test function to set tones used by the ADSL modem
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap)
{
	BcmAdslCoreSelectTones(xmtStartTone,xmtNumTones, rcvStartTone,rcvNumTones, xmtToneMap,rcvToneMap);
	return BCMADSL_STATUS_SUCCESS;
}

//**************************************************************************
// Function Name: BcmAdsl_SetDiagMode
// Description  : Test function to set tones used by the ADSL modem
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMADSL_STATUS BcmAdsl_SetDiagMode(int diagMode)
{
	BcmAdslCoreSetAdslDiagMode(diagMode);
	return BCMADSL_STATUS_SUCCESS;
}

/***************************************************************************
 * Function Name: BcmAdsl_GetConstellationPoints
 * Description  : Gets constellation point for selected tone
 * Returns      : number of points rettrieved 
 ***************************************************************************/
int BcmAdsl_GetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints)
{
	return BcmAdslCoreGetConstellationPoints (toneId, pointBuf, numPoints);
}



BCMADSL_STATUS BcmAdsl_G997SendData(void *buf, int len)
{
    return BcmAdslCoreG997SendData(buf, len) ? 
        BCMADSL_STATUS_SUCCESS : BCMADSL_STATUS_ERROR;
}

void *BcmAdsl_G997FrameGet(int *pLen)
{
    return BcmAdslCoreG997FrameGet(pLen);
}

void *BcmAdsl_G997FrameGetNext(int *pLen)
{
    return BcmAdslCoreG997FrameGetNext(pLen);
}

void  BcmAdsl_G997FrameFinished(void)
{
    BcmAdslCoreG997FrameFinished();
}

//**************************************************************************
// 
//		ATM EOP workaround functions
// 
//**************************************************************************

void BcmAdsl_AtmSetPortId(int path, int portId)
{
	BcmAdslCoreAtmSetPortId(path,portId);
}

void BcmAdsl_AtmClearVcTable(void)
{
	BcmAdslCoreAtmClearVcTable();
}

void BcmAdsl_AtmAddVc(int vpi, int vci)
{
	BcmAdslCoreAtmAddVc(vpi, vci);
}

void BcmAdsl_AtmDeleteVc(vpi, vci)
{
	BcmAdslCoreAtmDeleteVc(vpi, vci);
}

void BcmAdsl_AtmSetMaxSdu(unsigned short maxsdu)
{
	BcmAdslCoreAtmSetMaxSdu(maxsdu);
}

//**************************************************************************
// Function Name: BcmAdsl_Status
// Description  : Runs in a separate thread of execution. Wakes up every
//                second to check status of DSL PHY
// Returns      : None.
//**************************************************************************


void BcmAdsl_G997GetAndSendFrameData(void)
{
	int len=0;
	char* frameReadPtr;
	char* frameBuffer;
	int adv;
	int bufLen;
	
	if(NULL == (frameReadPtr=(char*)BcmAdsl_G997FrameGet(&bufLen)))
		return;
	
	while(frameReadPtr!=NULL) {
		len=len+bufLen;
		frameReadPtr=(char*)BcmAdsl_G997FrameGetNext(&bufLen);
	}
	
	BcmAdslCoreDiagWriteStatusString("G997: Got Frame totalLen=%d",len);	
	frameBuffer=(char*)calloc(len,1);
	
	if(frameBuffer==NULL) {
		BcmAdslCoreDiagWriteStatusString("G997: Memory Allocation failed");
		return;
	}
	
	adv=0;
	frameReadPtr=(char*)BcmAdsl_G997FrameGet(&bufLen);
	while(frameReadPtr!=NULL) {
		memcpy((frameBuffer+adv),frameReadPtr,bufLen);
		adv+=bufLen;
		frameReadPtr=(char*)BcmAdsl_G997FrameGetNext(&bufLen);
	}
	
	BcmAdsl_G997FrameFinished();
	
	if( BCMADSL_STATUS_SUCCESS != BcmAdsl_G997SendData(frameBuffer,len)) 
		free(frameBuffer);
	
	return;
}

int ClearEOCLoopBackEnabled=0;
void BcmAdsl_Status()
{
    ADSL_CONNECTION_INFO ConnectionInfo;
    ADSL_LINK_STATE      adslEvent;

    static char* frameBuffer;
  

    BcmAdslCoreGetConnectionInfo(&ConnectionInfo);

    if (ConnectionInfo.LinkState != g_LinkState) {

#ifdef VDF_SPAIN
	 if (g_LinkState == BCM_ADSL_LINK_UP)
 	 {
 	 	g_ulAdslCuts++;
 	 }
#endif
		
        switch (ConnectionInfo.LinkState) {
			case BCM_ADSL_LINK_UP:

#ifdef VDF_SPAIN
				g_ulAdslOnLineStart = jiffies;
#endif

				//LGD_FOR_TR098
				bcmOsGetTime(&g_ShowtimeStartTicks);
				if( ConnectionInfo.ulInterleavedUpStreamRate )
				{
					BCMOS_EVENT_LOG((KERN_CRIT \
						TEXT("ADSL link up, interleaved, us=%lu, ds=%lu\n"), \
						ConnectionInfo.ulInterleavedUpStreamRate / 1000, \
						ConnectionInfo.ulInterleavedDnStreamRate / 1000));
				}
				else
				{
					BCMOS_EVENT_LOG((KERN_CRIT \
						TEXT("ADSL link up, fast, us=%lu, ds=%lu\n"), \
						ConnectionInfo.ulFastUpStreamRate / 1000, \
						ConnectionInfo.ulFastDnStreamRate / 1000));
				}
			break;

			case BCM_ADSL_TRAINING_G994:
				BCMOS_EVENT_LOG((KERN_CRIT TEXT("ADSL G.994 training\n")));
				break;

			case BCM_ADSL_TRAINING_G992_STARTED:
				BCMOS_EVENT_LOG((KERN_CRIT TEXT("ADSL G.992 started\n")));
				break;

			case BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS:
				BCMOS_EVENT_LOG((KERN_CRIT TEXT("ADSL G.992 channel analysis\n")));
				break;

			case BCM_ADSL_TRAINING_G992_EXCHANGE:
				BCMOS_EVENT_LOG((KERN_CRIT TEXT("ADSL G.992 message exchange\n")));
				break;

			case BCM_ADSL_LINK_DOWN:
				BcmAdslCoreBertStopEx();
				BCMOS_EVENT_LOG((KERN_CRIT TEXT("ADSL link down\n")));
				break;

			default:
				break;
		}

        g_LinkState = ConnectionInfo.LinkState;
        (*g_pFnNotifyCallback)(g_LinkState, g_ulNotifyCallbackParm);
    }

    /* call extended info to see if the callback needs to be called (TBD) */

    while ((adslEvent = BcmAdslCoreGetEvent()) != -1) {
		if (ADSL_SWITCH_RJ11_PAIR == adslEvent) {
			g_RJ11Pair ^= 1;
#if !defined(BOARD_bcm96345) || defined(BOARDVAR_bant_a)
	 		if ( (g_GpioInnerPair != 0xffff) &&
				 (g_GpioOuterPair != 0xffff) ) {
				if (ADSL_RJ11_INNER_PAIR == g_RJ11Pair)
					SetRj11Pair( g_GpioInnerPair, g_GpioOuterPair );
				else 
					SetRj11Pair( g_GpioOuterPair, g_GpioInnerPair );
			}
#else
			if (ADSL_RJ11_INNER_PAIR == g_RJ11Pair) 
			{
				// Switch to inner (GPIO is low)
				GPIO->GPIODir |= GPIO_AUTOSENSE_CTRL; // define as output
				GPIO->GPIOio &= ~GPIO_AUTOSENSE_CTRL;
			}
			else
			{
				// Switch to outer (GPIO is high)
				GPIO->GPIODir |= GPIO_AUTOSENSE_CTRL; // define as output
				GPIO->GPIOio |= GPIO_AUTOSENSE_CTRL;
			}
#endif
		}
		else {
			if ((BCM_ADSL_LINK_UP != adslEvent) && (BCM_ADSL_LINK_DOWN != adslEvent)) {
				  if(BCM_ADSL_G997_FRAME_RECEIVED == adslEvent && ClearEOCLoopBackEnabled ){
				  	BcmAdslCoreDiagWriteStatusString("G997: Frame Received event");
					BcmAdsl_G997GetAndSendFrameData();
				  }
				  else if(BCM_ADSL_G997_FRAME_SENT == adslEvent && ClearEOCLoopBackEnabled)
				  	BcmAdslCoreDiagWriteStatusString("G997: Frame Sent Event");
				  else
				  	(*g_pFnNotifyCallback)(adslEvent, g_ulNotifyCallbackParm);
			}
		}
    }
}

void BcmAdsl_Notify(void)
{
#if !defined(TARG_OS_RTEMS)
    if( g_nAdslExit == 0 )
		BcmAdsl_Status();
#else
    bcmOsSemGive (g_StatusSemId);
#endif
}

#if !defined(TARG_OS_RTEMS)
LOCAL void BcmAdsl_Timer(void * arg)
{
	if( g_nAdslExit == 1 )
		return;

	BcmAdsl_Status();
	bcmOsTimerStart(g_TimerHandle, 1000);
}
#else
LOCAL void StatusTask()
{
    while (TRUE) {
        /* Sleep 1 second */
        bcmOsSemTake(g_StatusSemId, 1000 / BCMOS_MSEC_PER_TICK);

        if( g_nAdslExit == 1 )
            break;

		BcmAdsl_Status();
	}
}
#endif

//**************************************************************************
// Function Name: SetRj11Pair
// Description  : Sets the RJ11 wires to use either the inner pair or outer
//                pair for ADSL.
// Returns      : None.
//**************************************************************************
LOCAL void SetRj11Pair( UINT16 usPairToEnable, UINT16 usPairToDisable )
{
    int i;
    GPIO->GPIODir |= usPairToEnable | usPairToDisable;

    /* Put the "other" pair into a disabled state */
    GPIO->GPIOio &= ~usPairToDisable;

    /* Enable the pair two times just to make sure. */
    for( i = 0; i < 2; i++ )
    {
        GPIO->GPIOio |= usPairToEnable;
		bcmOsDelay(10);
        GPIO->GPIOio &= ~usPairToEnable;
		bcmOsDelay(10);
    }
} /* SetRj11Pair */

//**************************************************************************
// Function Name: BcmAdsl_ConfigureRj11Pair
// Description  : Configures RJ11 pair setting according to ADSL profile
//                pair for ADSL.
// Returns      : None.
//**************************************************************************
#if !defined(BOARD_bcm96345) || defined(BOARDVAR_bant_a)
void BcmAdsl_ConfigureRj11Pair(long pair)
{
	if ( (g_GpioInnerPair == 0xffff) || (g_GpioOuterPair == 0xffff) )
		return;

	if (kAdslCfgLineInnerPair == (pair & kAdslCfgLinePairMask)) {
		g_RJ11Pair = ADSL_RJ11_INNER_PAIR;
		SetRj11Pair( g_GpioInnerPair, g_GpioOuterPair );
	}
	else {
		g_RJ11Pair = ADSL_RJ11_OUTER_PAIR;
		SetRj11Pair( g_GpioOuterPair, g_GpioInnerPair );
	}
} /* BcmAdsl_ConfigureRj11Pair */
#else
void BcmAdsl_ConfigureRj11Pair(long pair)
{
	if (kAdslCfgLineInnerPair == (pair & kAdslCfgLinePairMask)) {
		g_RJ11Pair = ADSL_RJ11_INNER_PAIR;
		// Switch to inner (GPIO is low)
		GPIO->GPIODir |= GPIO_AUTOSENSE_CTRL; // define as output
		GPIO->GPIOio &= ~GPIO_AUTOSENSE_CTRL;
	}
	else {
		g_RJ11Pair = ADSL_RJ11_OUTER_PAIR;
		// Swith to outer (GPIO is high)
		GPIO->GPIODir |= GPIO_AUTOSENSE_CTRL; // define as output
		GPIO->GPIOio |= GPIO_AUTOSENSE_CTRL;
	}
} /* BcmAdsl_ConfigureRj11Pair */
#endif

#ifndef DYING_GASP_API

//**************************************************************************
// Function Name: BcmDyingGaspIsr
// Description  : Handles power off to the board.
// Returns      : 1
//**************************************************************************
#if defined(__KERNEL__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
LOCAL irqreturn_t BcmDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs)
{
	if (BcmAdslCoreCheckPowerLoss())
		BcmAdslCoreSendDyingGasp(1);
    return( IRQ_HANDLED );
} /* BcmDyingGaspIsr */
#else
LOCAL unsigned int BcmDyingGaspIsr( void )
{
	if (BcmAdslCoreCheckPowerLoss())
		BcmAdslCoreSendDyingGasp(1);
    return( 1 );
} /* BcmDyingGaspIsr */
#endif

#else /* DYING_GASP_API */

//**************************************************************************
// Function Name: BcmAdsl_DyingGaspHandler
// Description  : Handles power off to the board.
// Returns      : none
//**************************************************************************
void BcmAdsl_DyingGaspHandler(void *context)
{
	BcmAdslCoreSendDyingGasp(1);	
	
}

#endif
