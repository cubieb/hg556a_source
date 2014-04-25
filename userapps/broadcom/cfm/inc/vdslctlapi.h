/*****************************************************************************
//
//  Copyright (c) 2000-2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       vdslctlapi.h
//  Author:         Ben Bradshaw
//  Creation Date:  26th April 2005
//
******************************************************************************
//  Description:
//      Define the VDSL CTL api functions.
//
******************************************************************************/

#ifndef __VDSL_CTL_API_H__
#define __VDSL_CTL_API_H__

/********************** Include Files ***************************************/
#include "bcmtypes.h"
#include "bcmxdsl.h"

#define VDSL_BERT_STATE_STOP 0
#define VDSL_BERT_STATE_RUN 1

/********************** Global APIs Definitions *****************************/
#if defined(__cplusplus)
extern "C" {
#endif

extern void BcmVdslCtl_GetPhyVersion(char *version, int len);
extern void BcmVdslCtl_Initialize( void ) ;
extern BCMXDSL_STATUS BcmVdslCtl_GetConnectionInfo(PXDSL_CONNECTION_INFO pConnInfo,
                                                   char *errMsg);

extern void parseVdslInfo(char *info, char *var, char *val, int len);

extern void BcmVdslCtl_GetStatistics(vdslPerfDataEntry *pVdslRxPerfData,
                                     char *errMsg,
                                     int linkDown); 

#if 0
extern void BcmVdslCtl_GetStatistics(adslPhysEntry *pAdslPhysRcv,
                                     adslFullPhysEntry *pAdslPhysXmt,
                                     adslConnectionInfo *pAdslConnInfo,
                                     adsl2ConnectionInfo *pAdsl2ConnInfo,
                                     adslConnectionStat *pAdslConnStat,
                                     adslPerfDataEntry *pAdslRxPerfData,
                                     adslPerfCounters  *pAdslTxPerf,
                                     atmConnectionStat *pAtmConnStat,
                                     char *errMsg,
                                     int linkDown);
extern int  BcmVdslCtl_GetBertState(void);
extern void BcmVdslCtl_GetBertResults(adslBertResults *pAdslBertRes);
extern void parseVdslInfo(char *info, char *var, char *val, int len);
#endif
#if defined(__cplusplus)
}
#endif


/********************** Local APIs Definitions *****************************/

#endif
