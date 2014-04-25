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
// File Name  : bcmvdsl.h
//
// Description: This file contains the definitions, structures and function
//              prototypes for VDSL 6510 interface
//
//**************************************************************************
#if !defined(_BCMXDSL_H_)
#define _BCMXDSL_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Incldes. */
#include "VdslInfoDef.h"

//**************************************************************************
// Type Definitions
//**************************************************************************

// Return status values
typedef enum BcmXdslStatus
{
    BCMXDSL_STATUS_SUCCESS = 0,
    BCMXDSL_STATUS_ERROR
} BCMXDSL_STATUS;

// Return status values
typedef enum XdslLinkState
{
    BCM_XDSL_LINK_UP = 0,
    BCM_XDSL_LINK_DOWN,
	BCM_XDSL_TRAINING_G992_EXCHANGE,
	BCM_XDSL_TRAINING_G992_CHANNEL_ANALYSIS,
	BCM_XDSL_TRAINING_G992_STARTED,
	BCM_XDSL_TRAINING_G994,
	BCM_XDSL_G994_NONSTDINFO_RECEIVED,
    BCM_XDSL_BERT_COMPLETE,
	BCM_XDSL_ATM_IDLE,
    BCM_XDSL_EVENT,
    BCM_XDSL_G997_FRAME_RECEIVED,
    BCM_XDSL_G997_FRAME_SENT
} XDSL_LINK_STATE;

#ifndef DISABLE_VDSL_OLD_DEF
#define	XDSL_LINK_UP		BCM_XDSL_LINK_UP
#define	XDSL_LINK_DOWN		BCM_XDSL_LINK_DOWN
#endif

typedef enum XdslTrafficType
{
    BCM_XDSL_TRAFFIC_INACTIVE = 0,
    BCM_XDSL_TRAFFIC_ATM,
    BCM_XDSL_TRAFFIC_PTM   
} XDSL_TRAFFIC_TYPE;

/* VDSL test modes */
typedef enum VdslTestMode
{
    VDSL_TEST_NORMAL = 0,
    VDSL_TEST_REVERB,
    VDSL_TEST_MEDLEY,
    VDSL_TEST_SELECT_TONES,
    VDSL_TEST_NO_AUTO_RETRAIN,
	VDSL_TEST_MARGIN_TWEAK,
	VDSL_TEST_ESTIMATE_PLL_PHASE,
	VDSL_TEST_REPORT_PLL_PHASE_STATUS,
	VDSL_TEST_AFELOOPBACK,
    VDSL_TEST_L3,
    VDSL_TEST_DIAGMODE,
    VDSL_TEST_L0
} VDSL_TEST_MODE;

// XDSL_CHANNEL_ADDR Contains XDSL Utopia PHY addresses
typedef struct XdslChannelAddr
{
    UINT16 usFastChannelAddr;
    UINT16 usInterleavedChannelAddr;
} XDSL_CHANNEL_ADDR, *PXDSL_CHANNEL_ADDR;

// For ADSL B0 maps to interleaved and B1 to fast 
# define ILV  B0 
# define FAST B1 

// XDSL_CONNECTION_INFO Contains XDSL Connection Info
typedef struct XdslConnectionInfo
{
	XDSL_LINK_STATE LinkState; 
	XDSL_TRAFFIC_TYPE TrafficType[2]; /* B0 and B1 */
    UINT32 ulUpStreamRate[2];         /* B0 and B1 */
    UINT32 ulDnStreamRate[2];         /* B0 and B1 */
} XDSL_CONNECTION_INFO, *PXDSL_CONNECTION_INFO;

/* OEM parameter definition */
#define XDSL_OEM_G994_VENDOR_ID       1    /* Vendor ID used during G.994 handshake */
#define XDSL_OEM_G994_XMT_NS_INFO     2    /* G.994 non-standard info field to send */
#define XDSL_OEM_G994_RCV_NS_INFO     3    /* G.994 received non-standard */
#define XDSL_OEM_EOC_VENDOR_ID        4    /* EOC reg. 0 */
#define XDSL_OEM_EOC_VERSION          5    /* EOC reg. 1 */
#define XDSL_OEM_EOC_SERIAL_NUMBER    6    /* EOC reg. 2 */
#define XDSL_OEM_T1413_VENDOR_ID      7    /* Vendor ID used during T1.413 handshake */
#define XDSL_OEM_T1413_EOC_VENDOR_ID  8    /* EOC reg. 0 (vendor ID) in T1.413 mode */

/* XMT gain definitions */
#define XDSL_XMT_GAIN_AUTO			  0x80000000

typedef struct
{
    int				diagCmd;
    int				diagMap;
    int				logTime;
	int				srvIpAddr;
	int				gwIpAddr;
} VDSL_DIAG, *PVDSL_DIAG;

typedef struct
{
    short			x;
    short			y;
} XDSL_CONSTELLATION_POINT, *PXDSL_CONSTELLATION_POINT;

#define XDSL_CONSTEL_DATA_ID		0
#define XDSL_CONSTEL_PILOT_ID		1

#define VDSL_INFO	vdslInfo
typedef	VDSL_INFO   *PVDSL_INFO;

typedef void (*XDSL_FN_NOTIFY_CB) (XDSL_LINK_STATE XdslLinkState, UINT32 ulParm); 

//**************************************************************************
// Function Prototypes
//**************************************************************************

BCMXDSL_STATUS BcmVdsl_Check(void);
BCMXDSL_STATUS BcmVdsl_Initialize(XDSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, vdslCfgProfile *pVdslCfg);
#ifdef LINUX
BCMXDSL_STATUS BcmVdsl_MapAtmPortIDs(UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId);
#endif
BCMXDSL_STATUS BcmVdsl_Uninitialize(void);
BCMXDSL_STATUS BcmVdsl_ConnectionStart(void);
BCMXDSL_STATUS BcmVdsl_ConnectionStop(void);
BCMXDSL_STATUS BcmVdsl_GetCounterInfo( vdslInfo *pVdslInfo );
BCMXDSL_STATUS BcmVdsl_GetPhyAddresses(PXDSL_CHANNEL_ADDR pChannelAddr);
BCMXDSL_STATUS BcmVdsl_SetPhyAddresses(PXDSL_CHANNEL_ADDR pChannelAddr);
BCMXDSL_STATUS BcmVdsl_GetConnectionInfo(PXDSL_CONNECTION_INFO pConnectionInfo);
BCMXDSL_STATUS BcmVdsl_DiagCommand(PVDSL_DIAG pVdslDiag);
int BcmVdsl_GetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
BCMXDSL_STATUS BcmVdsl_StartBERT(unsigned long totalBits);
BCMXDSL_STATUS BcmVdsl_StopBERT(void);
BCMXDSL_STATUS BcmVdsl_BertStartEx(unsigned long bertSec);
BCMXDSL_STATUS BcmVdsl_BertStopEx(void);
BCMXDSL_STATUS BcmVdsl_CheckPowerLoss(void);
BCMXDSL_STATUS BcmVdsl_SendDyingGasp(int powerCtl);
BCMXDSL_STATUS BcmVdsl_Configure(vdslCfgProfile *pVdslCfg);
BCMXDSL_STATUS BcmVdsl_GetVersion(vdslVersionInfo *pVdslVer);
BCMXDSL_STATUS BcmVdsl_SetSDRAMBaseAddr(void *pAddr);
BCMXDSL_STATUS BcmVdsl_SetVcEntry (int gfc, int port, int vpi, int vci);
BCMXDSL_STATUS BcmVdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci, int pti_clp);

BCMXDSL_STATUS BcmVdsl_ResetStatCounters(void);
BCMXDSL_STATUS BcmVdsl_SetAtmLoopbackMode(void);
BCMXDSL_STATUS BcmVdsl_SetTestMode(VDSL_TEST_MODE testMode);
BCMXDSL_STATUS BcmVdsl_SelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap
	);
BCMXDSL_STATUS BcmVdsl_SetDiagMode(int diagMode);

int BcmVdsl_GetConstellationPoints (int toneId, XDSL_CONSTELLATION_POINT *pointBuf, int numPoints);

int BcmVdsl_GetOemParameter (int paramId, void *buf, int len);
int BcmVdsl_SetOemParameter (int paramId, void *buf, int len);
int BcmVdsl_SetXmtGain(int gain);

UINT32	BcmVdsl_GetSelfTestMode(void);
void	BcmVdsl_SetSelfTestMode(UINT32 stMode);
UINT32	BcmVdsl_GetSelfTestResults(void);

void *BcmVdsl_G997FrameGet(int *pLen);
void *BcmVdsl_G997FrameGetNext(int *pLen);
void  BcmVdsl_G997FrameFinished(void);

/* HUAWEI HGW s48571 2006年8月11日 firmware挂死时重新加载 add begin:*/
void BcmVdsl_getReloadTime(void); 
/* HUAWEI HGW s48571 2006年8月11日 firmware挂死时重新加载 add end.*/

#if defined(__cplusplus)
}
#endif

#endif // _BCMXDSL_H_

