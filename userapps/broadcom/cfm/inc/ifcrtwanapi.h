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
//  Filename:       ifcrtwanapi.h
*****************************************************************************/

#ifndef __IFC_RT_WAN_API_H__
#define __IFC_RT_WAN_API_H__

#ifdef CPE_DSL_MIB
    void BcmNtwk_startWan(void);
    void BcmNtwk_unInitWan(PWAN_CON_ID wanIdLeft);
    void BcmPppoe_unInit(PWAN_CON_ID pWanId);
    void BcmMer_unInit(PWAN_CON_ID pWanId);
    void BcmIpoa_unInit(PWAN_CON_ID pWanId);
    void BcmBridge_unInit(PWAN_CON_ID pWanId);
    void BcmIPoWan_unInit(PWAN_CON_ID pWanId);
    void BcmWan_DeleteDataDirectory(PWAN_CON_ID pWanId);
    void BcmRfc2684_unInit(PWAN_CON_ID pWanId);
    void BcmApps_unInit(void);
    void BcmNtwk_unInitDns(void);
    void BcmNtwk_unInitSysLog(void);
    void BcmNtwk_unInitWan(WAN_CON_ID* wanIdLeft);
    void BcmNtwk_stopDefDhcpSrv(void);
    void BcmNtwk_stopArpSpoofing(void);
    void BcmNtwk_stopIgmp(void);
    void BcmNtwk_unInitRt(void);
#endif

#endif
