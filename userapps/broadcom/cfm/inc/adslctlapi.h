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
//  Filename:       adslctlapi.h
//  Author:         Peter T. Tran
//  Creation Date:  04/02/03
//
******************************************************************************
//  Description:
//      Define the ADSL CTL api functions.
//
******************************************************************************/

#ifndef __ADSL_CTL_API_H__
#define __ADSL_CTL_API_H__

/********************** Include Files ***************************************/
#include "bcmtypes.h"
#include "bcmadsl.h"
#include "AdslMibDef.h"

#define ADSL_BERT_STATE_STOP 0
#define ADSL_BERT_STATE_RUN 1

// modulation mode -- up to last 6 bytes
#define ANNEX_A_MODE_GDMT        0x00000001
#define ANNEX_A_MODE_GLITE       0x00000002
#define ANNEX_A_MODE_T1413       0x00000004
#define ANNEX_A_MODE_ADSL2       0x00000008
#define ANNEX_A_MODE_ANNEXL      0x00000010
#define ANNEX_A_MODE_ADSL2PLUS   0x00000020
#define ANNEX_A_MODE_ANNEXM      0x00000040        // default ANNEXM is DISABLED. (bit clear)

#define ANNEX_A_MODE_ALL_MOD     (ANNEX_A_MODE_GDMT | ANNEX_A_MODE_GLITE | ANNEX_A_MODE_T1413 | \
                                 ANNEX_A_MODE_ADSL2 | ANNEX_A_MODE_ANNEXL)
// 6348/6358 has the adsl-plus
#define ANNEX_A_MODE_ALL_MOD_48  (ANNEX_A_MODE_ALL_MOD | ANNEX_A_MODE_ADSL2PLUS)

// line pair define  -- start on 6th byte
#define ANNEX_A_LINE_PAIR_INNER     0x00000000           // default -- INNER PAIR 
#define ANNEX_A_LINE_PAIR_OUTER     0x01000000   
// bitswap bit
#define ANNEX_A_BITSWAP_ENABLE      0x00000000           // default -- enabled 
#define ANNEX_A_BITSWAP_DISENABLE   0x02000000
// SRA bit
#define ANNEX_A_SRA_DISENABLE       0x00000000           // default -- disenabled 
#define ANNEX_A_SRA_ENABLE          0x08000000           // enable

// ANNEX_C mode bit
#define ANNEX_C_BITMAP_DBM          0x00000000           // default
#define ANNEX_C_BITMAP_FBM          0x04000000           // 

#define ADSL_APPID                  "ADSL"
#define ADSL_CONFIG_FLAG            1                    // 32 bit field flag 


/********************** Global APIs Definitions *****************************/
#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmAdslCtl_GetStatistics(adslPhysEntry *pAdslPhysRcv,
                                     adslFullPhysEntry *pAdslPhysXmt,
                                     adslConnectionInfo *pAdslConnInfo,
                                     adsl2ConnectionInfo *pAdsl2ConnInfo,
                                     adslConnectionStat *pAdslConnStat,
                                     adslPerfDataEntry *pAdslRxPerfData,
                                     adslPerfCounters  *pAdslTxPerf,
                                     atmConnectionStat *pAtmConnStat,
                                     char *errMsg,
                                     int linkDown);
extern int  BcmAdslCtl_GetBertState(void);
//extern void BcmAdslCtl_GetBertResults(adslBertResults *pAdslBertRes);
extern cnt64 BcmAdslCtl_GetBertTotalBits(void);
extern cnt64 BcmAdslCtl_GetBertErrBits(void);
extern void BcmAdslCtl_GetPhyVersion(char *version, int len);
extern BCMADSL_STATUS BcmAdslCtl_GetConnectionInfo(PADSL_CONNECTION_INFO pConnInfo);
extern void parseAdslInfo(char *info, char *var, char *val, int len);
extern int  BcmAdslCtl_GetDiagsSocket(void);
extern int  BcmAdslCtl_ProcessDiagsFrames(void);
#if defined(__cplusplus)
}
#endif


/********************** Local APIs Definitions *****************************/

#endif
