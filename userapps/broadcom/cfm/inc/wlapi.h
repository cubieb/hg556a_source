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
//  Filename:       wlapi.h
//  Author:         Peter T. Tran
//  Creation Date:  10/17/02
//
******************************************************************************
//  Description:
//      Define the Wireless Configuration Api functions.
//
*****************************************************************************/
/* 2008/01/28 Jiajun Weng : Modified for wireless update */

#ifndef __WL_API_H__
#define __WL_API_H__

/********************** Include Files ***************************************/

#include "wldefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmWl_Init(void);
extern void BcmWl_Init_All(void);
extern void BcmWl_Uninit(void);
extern void BcmWl_Setup(WL_SETUP_TYPE type);
extern void BcmWl_Store(void);
extern void BcmWl_Retrieve(int isDefault);
extern void BcmWl_GetVar(char *varName, char *varValue);
extern void BcmWl_GetVarEx(int argc, char **argv, char *varValue);
extern void BcmWl_SetVar(char *varName, char *varValue);
extern void *BcmWl_getFilterMac(void *pVoid, char *mac, char *ssid, char* ifcName);
extern WL_STATUS BcmWl_addFilterMac(char *mac, char *ssid, char *ifcName);
extern WL_STATUS BcmWl_removeFilterMac(char *mac);
extern void *BcmWl_IsWdsMacConfigured(char *mac);
extern void BcmWl_ScanWdsMacStart(void);
extern void BcmWl_ScanWdsMacResult(void);
extern void *BcmWl_getScanWdsMac(void *pVoid, char *mac);
extern void *BcmWl_getScanWdsMacSSID(void *pVoid, char *mac, char *ssid);
extern WL_STATUS BcmWl_addWdsMac(char *mac);
extern WL_STATUS BcmWl_removeAllWdsMac(void);
extern void BcmWl_startService(void);
extern void BcmWl_startService_All(void);
extern void BcmWl_stopService(void);
extern void BcmWl_aquireStationList(void);
extern int BcmWl_getNumStations(void);
extern void BcmWl_getStation(int i, char *macAddress, char *associated,  char *authorized, char *ssid, char *ifcName);
extern void BcmWl_Monitor(void);
extern void BcmWl_MonitorByIndex(int index);
extern void BcmWl_ReloadVar(bool reqReload);
extern void BcmWl_getAllVars(WIRELESS_PSI_VARS *vars);
extern WL_STATUS BcmWl_enumInterfaces(char *devname);
extern int  BcmWl_GetMaxMbss(void);
extern int  BcmWl_GetWlIndexInUse(void);
extern void  BcmWl_Switch_instance(int index);
extern void  BcmWl_incAdaptor();
extern void ChangeWirelessStatus(void);
extern int BcmWl_getNumAdaptor();
void  BcmWl_StartWpsPbc(void);
/* exported by libwlctl */
extern void wlctl_cmd(char *cmd);
/*start-add by 00112761 for VDF_C02*/
extern char *  BcmWl_getEncypKeyStr(char * AutheMode);
/*end-add by 00112761 for VDF_C02*/
extern int BcmWl_nvram_set(const char *name, const char *value);

#ifdef  SUPPORT_WLAN_PRNTCTL
void BcmWl_createAutoCfgSwTsk(void);
#endif

#if defined(__cplusplus)
}
#endif

#ifdef DSLCPE_SHLIB
#define BCMWL_WLCTL_CMD(x) wlctl_cmd(x)
#define BCMWL_STR_CONVERT(in, out) strcpy(out, in)
#else
#define BCMWL_WLCTL_CMD(x) bcmSystem(x)
#define BCMWL_STR_CONVERT(in, out) bcmConvertStrToShellStr(in, out)
#endif

#endif
