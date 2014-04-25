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
//  Filename:       dbobject.h
//  Author:         Peter T. Tran
//  Creation Date:  02/18/05
//
******************************************************************************
//  Description:
//      Define the BcmDb_obj* functions.
//
*****************************************************************************/

#ifndef __DB_OBJECT_H__
#define __DB_OBJECT_H__

/********************** Include Files ***************************************/

#include <stdio.h>
#include <stdarg.h>
#include "bcmtypes.h"
#include "bcmatmapi.h"
#include "objectdefs.h"

typedef enum _objStatus {
   OBJ_STS_OK = 0,
   OBJ_STS_NEED_REBOOT,
   OBJ_STS_ERR_GENERAL,
   OJB_STS_ERR_MEMORY,
   OBJ_STS_ERR_OBJECT_NOT_FOUND,
   OBJ_STS_ERR_OBJECT_EXIST,
   OBJ_STS_ERR_FUNCTION_NOT_FOUND,
   OBJ_STS_ERR_INVALID_ARGUMENT,
   OBJ_STS_ERR_VCC_IN_USED,
   OBJ_STS_ERR_VCC_NOT_AVAILABLE,
   OBJ_STS_ERR_WAN_NOT_AVAILABLE,
   OJB_STS_ERR_EXECUTION
} OBJ_STATUS;

typedef OBJ_STATUS (*OBJ_HDL_FNC) (va_list ap);

typedef struct {
   char *objName;
   short rebootState;
   OBJ_HDL_FNC fncGet;
   OBJ_HDL_FNC fncSet;   
   OBJ_HDL_FNC fncDelete;
} DB_OBJECT_ITEM, *PDB_OBJECT_ITEM;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
    OBJ_STATUS BcmDb_objGet(char *objName, ...);
    OBJ_STATUS BcmDb_objSet(char *objName, ...);
    OBJ_STATUS BcmDb_objDelete(char *objName, ...);
    OBJ_STATUS BcmObj_getSystem(va_list ap);
    OBJ_STATUS BcmObj_doSystem(va_list ap);
    OBJ_STATUS BcmObj_getSwVersion(va_list ap);
    OBJ_STATUS BcmObj_getHwVersion(va_list ap);
    OBJ_STATUS BcmObj_getSysLogData(va_list ap);
    OBJ_STATUS BcmObj_getSysUserName(va_list ap);
    OBJ_STATUS BcmObj_setSysUserName(va_list ap);
    OBJ_STATUS BcmObj_getSptUserName(va_list ap);
    OBJ_STATUS BcmObj_getUsrUserName(va_list ap);
    OBJ_STATUS BcmObj_getSysPassword(va_list ap);
    OBJ_STATUS BcmObj_setSysPassword(va_list ap);
    OBJ_STATUS BcmObj_getSptPassword(va_list ap);
    OBJ_STATUS BcmObj_setSptPassword(va_list ap);
    OBJ_STATUS BcmObj_getUsrPassword(va_list ap);
    OBJ_STATUS BcmObj_setUsrPassword(va_list ap);
    OBJ_STATUS BcmObj_getDnsInfo(va_list ap);
    OBJ_STATUS BcmObj_setDnsInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteDnsInfo(va_list ap);
    OBJ_STATUS BcmObj_getDefaultGatewayInfo(va_list ap);
    OBJ_STATUS BcmObj_setDefaultGatewayInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteDefaultGatewayInfo(va_list ap);
    OBJ_STATUS BcmObj_getSysLogInfo(va_list ap);
    OBJ_STATUS BcmObj_setSysLogInfo(va_list ap);
    OBJ_STATUS BcmObj_getSysFlagInfo(va_list ap);
    OBJ_STATUS BcmObj_setSysFlagInfo(va_list ap);
    OBJ_STATUS BcmObj_getLanInfo(va_list ap);
    OBJ_STATUS BcmObj_setLanInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteLanInfo(va_list ap);
    OBJ_STATUS BcmObj_getAdslInfo(va_list ap);
    OBJ_STATUS BcmObj_setAdslInfo(va_list ap);
    OBJ_STATUS BcmObj_getAtmInitCfg(va_list ap);
    OBJ_STATUS BcmObj_setAtmInitCfg(va_list ap);
    OBJ_STATUS BcmObj_getAtmTdInfo(va_list ap);
    OBJ_STATUS BcmObj_setAtmTdInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteAtmTdInfo(va_list ap);
    OBJ_STATUS BcmObj_getAtmVccInfo(va_list ap);
    OBJ_STATUS BcmObj_setAtmVccInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteAtmVccInfo(va_list ap);
    OBJ_STATUS BcmObj_getWanInfo(va_list ap);
    OBJ_STATUS BcmObj_setWanInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteWanInfo(va_list ap);
    OBJ_STATUS BcmObj_getPppSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_setPppSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_deletePppSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_getIpSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_setIpSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteIpSrvInfo(va_list ap);
    OBJ_STATUS BcmObj_getRipGlobal(va_list ap);
    OBJ_STATUS BcmObj_setRipGlobal(va_list ap);
    OBJ_STATUS BcmObj_getRipIfc(va_list ap);
    OBJ_STATUS BcmObj_setRipIfc(va_list ap);
    OBJ_STATUS BcmObj_deleteRipIfc(va_list ap);
    OBJ_STATUS BcmObj_getRouteCfg(va_list ap);
    OBJ_STATUS BcmObj_setRouteCfg(va_list ap);
    OBJ_STATUS BcmObj_deleteRouteCfg(va_list ap);
    OBJ_STATUS BcmObj_getDmzHost(va_list ap);
    OBJ_STATUS BcmObj_setDmzHost(va_list ap);
    OBJ_STATUS BcmObj_deleteDmzHost(va_list ap);
    OBJ_STATUS BcmObj_getSrvCntrInfo(va_list ap);
    OBJ_STATUS BcmObj_setSrvCntrInfo(va_list ap);
    OBJ_STATUS BcmObj_getFilterOut(va_list ap);
    OBJ_STATUS BcmObj_setFilterOut(va_list ap);
    OBJ_STATUS BcmObj_deleteFilterOut(va_list ap);
    OBJ_STATUS BcmObj_getFilterIn(va_list ap);
    OBJ_STATUS BcmObj_setFilterIn(va_list ap);
    OBJ_STATUS BcmObj_deleteFilterIn(va_list ap);
    OBJ_STATUS BcmObj_getFilterMac(va_list ap);
    OBJ_STATUS BcmObj_setFilterMac(va_list ap);
    OBJ_STATUS BcmObj_deleteFilterMac(va_list ap);
    OBJ_STATUS BcmObj_getVirtualServer(va_list ap);
    OBJ_STATUS BcmObj_setVirtualServer(va_list ap);
    OBJ_STATUS BcmObj_deleteVirtualServer(va_list ap);
    OBJ_STATUS BcmObj_getPortTrigger(va_list ap);
    OBJ_STATUS BcmObj_setPortTrigger(va_list ap);
    OBJ_STATUS BcmObj_deletePortTrigger(va_list ap);
    OBJ_STATUS BcmObj_getAccessControl(va_list ap);
    OBJ_STATUS BcmObj_setAccessControl(va_list ap);
    OBJ_STATUS BcmObj_deleteAccessControl(va_list ap);
    OBJ_STATUS BcmObj_getAccessControlMode(va_list ap);
    OBJ_STATUS BcmObj_setAccessControlMode(va_list ap);
    OBJ_STATUS BcmObj_getQosClass(va_list ap);
    OBJ_STATUS BcmObj_setQosClass(va_list ap);
    OBJ_STATUS BcmObj_deleteQosClass(va_list ap);
#ifdef ETH_CFG
    OBJ_STATUS BcmObj_getEthInfo(va_list ap);
    OBJ_STATUS BcmObj_setEthInfo(va_list ap);
#endif
#ifdef SUPPORT_PORT_MAP
    OBJ_STATUS BcmObj_getPMapCfg(va_list ap);
    OBJ_STATUS BcmObj_setPMapCfg(va_list ap);
    OBJ_STATUS BcmObj_deletePMapCfg(va_list ap);
    OBJ_STATUS BcmObj_getPMapIfcCfgSts(va_list ap);
    OBJ_STATUS BcmObj_setPMapIfcCfgSts(va_list ap);
#endif   // SUPPORT_PORT_MAP
#ifdef SUPPORT_SNMP
    OBJ_STATUS BcmObj_getSnmpInfo(va_list ap);
    OBJ_STATUS BcmObj_setSnmpInfo(va_list ap);
#endif
#ifdef SUPPORT_DDNSD
    OBJ_STATUS BcmObj_getDDnsInfo(va_list ap);
    OBJ_STATUS BcmObj_setDDnsInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteDDnsInfo(va_list ap);
#endif   // SUPPORT_DDNSD
#ifdef SUPPORT_SNTP
    OBJ_STATUS BcmObj_getSntpInfo(va_list ap);
    OBJ_STATUS BcmObj_setSntpInfo(va_list ap);
#endif   // SUPPORT_SNTP
#ifdef SUPPORT_IPP
    OBJ_STATUS BcmObj_getIppInfo(va_list ap);
    OBJ_STATUS BcmObj_setIppInfo(va_list ap);
#endif   // SUPPORT_IPP
#ifdef SUPPORT_TOD
    OBJ_STATUS BcmObj_getTodInfo(va_list ap);
    OBJ_STATUS BcmObj_setTodInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteTodInfo(va_list ap);
#endif   // SUPPORT_TOD

#ifdef SUPPORT_MAC //add byl66195 for VDF
    OBJ_STATUS BcmObj_getMacInfo(va_list ap);
    OBJ_STATUS BcmObj_setMacInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteMacInfo(va_list ap);
#endif   // SUPPORT_MAC

#ifdef SUPPORT_MACMATCHIP 
    OBJ_STATUS BcmObj_getMacMatchInfo(va_list ap);
    OBJ_STATUS BcmObj_setMacMatchInfo(va_list ap);
    OBJ_STATUS BcmObj_deleteMacMatchInfo(va_list ap);
#endif   

#ifdef VOXXXLOAD
    OBJ_STATUS BcmObj_getVoice(va_list ap);
    OBJ_STATUS BcmObj_setVoice(va_list ap);
#endif
#ifdef WIRELESS
    OBJ_STATUS BcmObj_getWireless(va_list ap);
    OBJ_STATUS BcmObj_setWireless(va_list ap);
#endif
#ifdef PORT_MIRRORING
   OBJ_STATUS BcmObj_getPortMirrorStatus(va_list ap);
   OBJ_STATUS BcmObj_setPortMirrorStatus(va_list ap);
   OBJ_STATUS BcmObj_getPortMirrorCfg(va_list ap);
   OBJ_STATUS BcmObj_setPortMirrorCfg(va_list ap);
   OBJ_STATUS BcmObj_deletePortMirrorCfg(va_list ap);
#endif
#if defined(__cplusplus)
}
#endif

#endif
