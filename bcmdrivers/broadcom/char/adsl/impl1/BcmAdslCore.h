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
****************************************************************************
** File Name  : BcmAdslCore.h
**
** Description: This file contains the definitions, structures and function
**              prototypes for Bcm Core ADSL PHY interface
**
***************************************************************************/
#if !defined(_BcmAdslCore_H)
#define _BcmAdslCore_H

#include "AdslMibDef.h"

/* 
**	Internal ADSL driver events handled by the ADSL driver
**  defined not to intersect with ADSL_LINK_STATE events (in vcmadsl.h)
*/

#define	ADSL_SWITCH_RJ11_PAIR	0x80000001

/***************************************************************************
** Function Prototypes
***************************************************************************/

void BcmAdslCoreInit(void);
void BcmAdslCoreUninit(void);
Bool BcmAdslCoreCheckBoard(void);
void BcmAdslCoreConnectionStart(void);
void BcmAdslCoreConnectionStop(void);
void BcmAdslCoreConnectionReset(void);

void BcmAdslCoreGetConnectionInfo(PADSL_CONNECTION_INFO pConnectionInfo);
void BcmAdslCoreDiagCmd(PADSL_DIAG pAdslDiag);
void BcmAdslCoreDiagCmdCommon(int diagCmd, int len, void *pCmdData);
char * BcmAdslCoreDiagScrambleString(char *s);
int  BcmAdslCoreSetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
int  BcmAdslCoreGetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
void BcmAdslCoreStartBERT(unsigned long totalBits);
void BcmAdslCoreStopBERT(void);
void BcmAdslCoreBertStartEx(unsigned long bertSec);
void BcmAdslCoreBertStopEx(void);
#ifndef DYING_GASP_API
Bool BcmAdslCoreCheckPowerLoss(void);
#endif
void BcmAdslCoreSendDyingGasp(int powerCtl);
void BcmAdslCoreConfigure(adslCfgProfile *pAdslCfg);
void BcmAdslCoreGetVersion(adslVersionInfo *pAdslVer);
void BcmAdslCoreSetTestMode(int testMode);
void BcmAdslCoreSetTestExecutionDelay(int testMode, unsigned long value);
void BcmAdslCoreSelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap);
void BcmAdslCoreDiagSelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap);
void BcmAdslCoreSetAdslDiagMode(int diagMode);
int BcmAdslCoreGetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints);
int BcmAdslCoreGetOemParameter (int paramId, void *buf, int len);
int BcmAdslCoreSetOemParameter (int paramId, void *buf, int len);
int BcmAdslCoreSetXmtGain(int gain);
int  BcmAdslCoreGetSelfTestMode(void);
void BcmAdslCoreSetSelfTestMode(int stMode);
int  BcmAdslCoreGetSelfTestResults(void);
void BcmAdslCoreAfeTestMsg(void *pMsg);
void BcmAdslCoreDebugCmd(void *pMsg);
void BcmAdslCoreGdbCmd(void *pCmd, int cmdLen);


ADSL_LINK_STATE BcmAdslCoreGetEvent (void);
Bool BcmAdslCoreSetSDRAMBaseAddr(void *pAddr);
Bool BcmAdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp);
Bool BcmAdslCoreSetGfc2VcMapping(Bool bOn);
Bool BcmAdslCoreSetAtmLoopbackMode(void);
void BcmAdslCoreResetStatCounters(void);

Bool BcmAdslCoreG997SendData(void *buf, int len);

void *BcmAdslCoreG997FrameGet(int *pLen);
void *BcmAdslCoreG997FrameGetNext(int *pLen);
void BcmAdslCoreG997FrameFinished(void);

void BcmAdslCoreAtmSetPortId(int path, int portId);
void BcmAdslCoreAtmClearVcTable(void);
void BcmAdslCoreAtmAddVc(int vpi, int vci);
void BcmAdslCoreAtmDeleteVc(int vpi, int vci);
void BcmAdslCoreAtmSetMaxSdu(unsigned short maxsdu);

void BcmAdsl_Notify(void);
void BcmAdsl_ConfigureRj11Pair(long pair);
void BcmDiagDataLogNotify(int set);

/* synchronization with ADSL PHY DPC/bottom half */

#ifdef __KERNEL__
#define	BcmCoreDpcSyncEnter()		local_bh_disable()
#define	BcmCoreDpcSyncExit()		local_bh_enable()
#elif defined(VXWORKS)
#define	BcmCoreDpcSyncEnter()		taskLock()
#define	BcmCoreDpcSyncExit()		taskUnlock()
#else
#define	BcmCoreDpcSyncEnter()
#define	BcmCoreDpcSyncExit()
#endif

#define	BcmCoreCommandHandlerSync(cmd)	do {	\
	BcmCoreDpcSyncEnter();						\
	AdslCoreCommandHandler(cmd);				\
	BcmCoreDpcSyncExit();						\
} while (0)

#endif /* _BcmAdslCore_H */

