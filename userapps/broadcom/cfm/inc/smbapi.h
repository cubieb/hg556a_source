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
//  Filename:       smbapi.h
//  Author:         ....
//  Creation Date:  01/22/08
//
******************************************************************************
//  Description:
//      
//
*****************************************************************************/
#ifndef __SMB_API_H
#define __SMB_API_H
#include "sambadefs.h"

#if defined(__cplusplus)
extern "C" {
#endif
   extern void BcmSmb_init(void);
   extern void BcmSmb_unInit(void);
   extern void *BcmSmb_getSmbAccnt(void *pVoid, PSAMBA_ACCNT_ENTRY pObject);
   extern SMB_STATUS BcmSmb_addSmbAccnt(PSAMBA_ACCNT_ENTRY pObject);
   extern SMB_STATUS BcmSmb_removeSmbAccntByInst(uint32 clsInst);
   extern void BcmSmb_getSmbCfgInfo(PSAMBA_MGMT_CFG pSmbMgmtCfg);
   extern void BcmSmb_setSmbCfgInfo(PSAMBA_MGMT_CFG pSmbMgmtCfg);
   extern void BcmSmb_startSambaServer();
   extern void BcmSmb_stopSambaServer();
   /* HUAWEI HGW w69233 2008-04-08 Add begin */
   /* Fix AU8D00388: Change of samba server can take effect immediately and not required reset HGW */
   extern void BcmSmb_restartSambaServer();
   /* HUAWEI HGW w69233 2008-04-08 Add end */

#if defined(__cplusplus)
}
#endif
#endif