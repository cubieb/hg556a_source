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
//  Filename:       dbobject.cpp
//  Author:         Peter T. Tran
//  Creation Date:  02/18/05
//
//  Description:
//      Implement the BcmDb_obj* functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sysinfo.h>

#include "dbobject.h"
#include "dbapi.h"
#include "dbvalid.h"
#include "psixml.h"
#include "adslcfgmngr.h"
#include "atmprtcfg.h"
#include "atminitcfg.h"
#include "atmtrffdscrtbl.h"
#include "atmvcccfgtbl.h"
#include "syscall.h"
#include "secapi.h"
#include "routeapi.h"
#include "fwsyscall.h"
#ifdef SUPPORT_DDNSD
#include "ddnsapi.h"
#endif
#ifdef SUPPORT_SNTP
#include "sntpapi.h"
#endif
#ifdef SUPPORT_TOD
#include "todapi.h"
#endif
#ifdef SUPPORT_MAC  // add by l66195 for VDF
#include "macapi.h"
#endif
#ifdef SUPPORT_MACMATCHIP  
#include "macmatchapi.h"
#endif
#ifdef SUPPORT_PORT_MAP
#include "pmapapi.h"
#endif
#ifdef SUPPORT_IPP
#include "ippapi.h"
#endif
#ifdef VOXXXLOAD
#include <vodslapi.h>
#endif
#ifdef WIRELESS
#include "wlapi.h"
#endif
#ifdef PORT_MIRRORING
#include "dbgcfgapi.h"
#endif

static DB_OBJECT_ITEM DbObjectTable[] = {
   { OBJ_SYSTEM_ACTION, 1, BcmObj_getSystem, BcmObj_doSystem, NULL },
   { PSX_OBJ_SOFTWARE_VERSION, 0, BcmObj_getSwVersion, NULL, NULL },
   { PSX_OBJ_HARDWARE_VERSION, 0, BcmObj_getHwVersion, NULL, NULL },
   { PSX_OBJ_SYSLOG_DATA, 0, BcmObj_getSysLogData, NULL, NULL },
   { PSX_OBJ_SYS_USER_NAME, 0, BcmObj_getSysUserName, BcmObj_setSysUserName, NULL },
   { PSX_OBJ_SYS_PASSWORD, 0, BcmObj_getSysPassword, BcmObj_setSysPassword, NULL },
#ifndef SUPPORT_ONE_USER
   { PSX_OBJ_SPT_USER_NAME, 0, BcmObj_getSptUserName, NULL, NULL },
   { PSX_OBJ_SPT_PASSWORD, 0, BcmObj_getSptPassword, BcmObj_setSptPassword, NULL },
   { PSX_OBJ_USR_USER_NAME, 0, BcmObj_getUsrUserName, NULL, NULL },
   { PSX_OBJ_USR_PASSWORD, 0, BcmObj_getUsrPassword, BcmObj_setUsrPassword, NULL },
#endif
   { PSX_OBJ_DNS, 1, BcmObj_getDnsInfo, BcmObj_setDnsInfo, BcmObj_deleteDnsInfo },
   { PSX_OBJ_DEF_GATEWAY, 1, BcmObj_getDefaultGatewayInfo, BcmObj_setDefaultGatewayInfo, BcmObj_deleteDefaultGatewayInfo },
   { PSX_OBJ_SYSLOG, 0, BcmObj_getSysLogInfo, BcmObj_setSysLogInfo, NULL },
   { PSX_OBJ_PROTOCOL, 1, BcmObj_getSysFlagInfo, BcmObj_setSysFlagInfo, NULL },
   { LAN_APPID, 1, BcmObj_getLanInfo, BcmObj_setLanInfo, BcmObj_deleteLanInfo },
   { ADSL_APPID, 0, BcmObj_getAdslInfo, BcmObj_setAdslInfo, NULL },
   { ATM_CFG_APPID, 1, BcmObj_getAtmInitCfg, BcmObj_setAtmInitCfg, NULL },
   { ATM_TD_APPID, 1, BcmObj_getAtmTdInfo, BcmObj_setAtmTdInfo, BcmObj_deleteAtmTdInfo },
   { ATM_VCC_APPID, 1, BcmObj_getAtmVccInfo, BcmObj_setAtmVccInfo, BcmObj_deleteAtmVccInfo },
   { WAN_APPID, 1, BcmObj_getWanInfo, BcmObj_setWanInfo, BcmObj_deleteWanInfo },
   { APPNAME_PPPSRV_PREFIX, 1, BcmObj_getPppSrvInfo, BcmObj_setPppSrvInfo, BcmObj_deletePppSrvInfo },
   { APPNAME_IPSRV_PREFIX, 1, BcmObj_getIpSrvInfo, BcmObj_setIpSrvInfo, BcmObj_deleteIpSrvInfo },
   { OBJNAME_RIP_GLOBAL, 0, BcmObj_getRipGlobal, BcmObj_setRipGlobal, NULL },
   { TBLNAME_RIP_IFC, 0, BcmObj_getRipIfc, BcmObj_setRipIfc, BcmObj_deleteRipIfc },
   { TBLNAME_ROUTE, 0, BcmObj_getRouteCfg, BcmObj_setRouteCfg, BcmObj_deleteRouteCfg },
   { OBJNAME_DMZ_HOST, 0, BcmObj_getDmzHost, BcmObj_setDmzHost, BcmObj_deleteDmzHost },
   { OBJNAME_SRV_CTRL_LIST, 0, BcmObj_getSrvCntrInfo, BcmObj_setSrvCntrInfo, NULL },
   { TBLNAME_OUT_FILTER, 0, BcmObj_getFilterOut, BcmObj_setFilterOut, BcmObj_deleteFilterOut },
   { TBLNAME_IN_FILTER, 0, BcmObj_getFilterIn, BcmObj_setFilterIn, BcmObj_deleteFilterIn },
   { TBLNAME_MAC_FILTER, 0, BcmObj_getFilterMac, BcmObj_setFilterMac, BcmObj_deleteFilterMac },
   { TBLNAME_VRT_SRV, 0, BcmObj_getVirtualServer, BcmObj_setVirtualServer, BcmObj_deleteVirtualServer },
   { TBLNAME_PRT_TRIGGER, 0, BcmObj_getPortTrigger, BcmObj_setPortTrigger, BcmObj_deletePortTrigger },
   { TBLNAME_ACC_CTRL, 0, BcmObj_getAccessControl, BcmObj_setAccessControl, BcmObj_deleteAccessControl },
   { OBJNAME_ACC_CTRL_MODE, 0, BcmObj_getAccessControlMode, BcmObj_setAccessControlMode, NULL },
   { TBLNAME_QOS_CLS, 0, BcmObj_getQosClass, BcmObj_setQosClass, BcmObj_deleteQosClass },
#ifdef ETH_CFG
   { PSX_OBJ_ETHCFG, 1, BcmObj_getEthInfo, BcmObj_setEthInfo, NULL },
#endif   // ETH_CFG
#ifdef SUPPORT_PORT_MAP
   { TBLNAME_PMAP, 0, BcmObj_getPMapCfg, BcmObj_setPMapCfg, BcmObj_deletePMapCfg },
   { OBJNAME_PMAP_IFC_CFG, 0, BcmObj_getPMapIfcCfgSts, BcmObj_setPMapIfcCfgSts, NULL },
#endif   // SUPPORT_PORT_MAP
#ifdef SUPPORT_SNMP
   { PSX_OBJ_SNMP, 0, BcmObj_getSnmpInfo, BcmObj_setSnmpInfo, NULL },
#endif   // SUPPORT_SNMP
#ifdef SUPPORT_DDNSD
   { TBLNAME_DDNS, 0, BcmObj_getDDnsInfo, BcmObj_setDDnsInfo, BcmObj_deleteDDnsInfo },
#endif   // SUPPORT_DDNSD
#ifdef SUPPORT_SNTP
   { SNTP_APP_ID, 0, BcmObj_getSntpInfo, BcmObj_setSntpInfo, NULL },
#endif   // SUPPORT_SNTP
#ifdef SUPPORT_IPP
   { IPP_APP_ID, 0, BcmObj_getIppInfo, BcmObj_setIppInfo, NULL },
#endif   // SUPPORT_IPP
#ifdef SUPPORT_TOD
   { TBLNAME_PARENT_CNTRL, 0, BcmObj_getTodInfo, BcmObj_setTodInfo, BcmObj_deleteTodInfo },
#endif   // SUPPORT_TOD
#ifdef SUPPORT_MAC //add by l66195 for VDF
   { TBLNAME_MAC, 0, BcmObj_getMacInfo, BcmObj_setMacInfo, BcmObj_deleteMacInfo },
#endif   // SUPPORT_TOD
#ifdef SUPPORT_MACMATCHIP 
   { TBLNAME_MACMATCH, 0, BcmObj_getMacMatchInfo, BcmObj_setMacMatchInfo, BcmObj_deleteMacMatchInfo },
#endif   // SUPPORT_TOD
#ifdef VOXXXLOAD
   { VOICE_APPID, 0, BcmObj_getVoice, BcmObj_setVoice, NULL },
#endif   // VOXXXLOAD
#ifdef WIRELESS
   { WIRELESS_APPID, 0, BcmObj_getWireless, BcmObj_setWireless, NULL },
#endif   // WIRELESS
#ifdef PORT_MIRRORING
   { OBJNAME_PORT_MIRROR_STATUS, 0, BcmObj_getPortMirrorStatus, BcmObj_setPortMirrorStatus, NULL },
   { TBLNAME_PORT_MIRROR_CFG, 0, BcmObj_getPortMirrorCfg, BcmObj_setPortMirrorCfg, BcmObj_deletePortMirrorCfg },
#endif
   { NULL, 0, NULL, NULL, NULL }
};

//**************************************************************************
// Function Name: BcmDb_objGet
// Description  : get object value from PSI for the given object name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmDb_objGet(char *objName, ...) {
    int  i = 0;
    char *ret = NULL;
    OBJ_STATUS status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
   
    for ( i = 0, ret = DbObjectTable[i].objName;
          ret != NULL;
          i++, ret = DbObjectTable[i].objName )
        if ( strcmp(objName, DbObjectTable[i].objName) == 0 )
            break;

    if ( ret != NULL ) {
        if ( DbObjectTable[i].fncGet != NULL ) {
            va_list ap;
            va_start(ap, objName);
            status = (*(DbObjectTable[i].fncGet))(ap);
            va_end(ap);
        } else
            status = OBJ_STS_ERR_FUNCTION_NOT_FOUND;
    }
    
    return status;
}


//**************************************************************************
// Function Name: BcmDb_objSet
// Description  : set object value to PSI for the given object name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmDb_objSet(char *objName, ...) {
    int  i = 0;
    char *ret = NULL;
    OBJ_STATUS status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
   
	 printf ("In BcmDb_objSet ....... ObjName = %s \n", objName) ;

    for ( i = 0, ret = DbObjectTable[i].objName;
          ret != NULL;
          i++, ret = DbObjectTable[i].objName )
        if ( strcmp(objName, DbObjectTable[i].objName) == 0 )
            break;

    if ( ret != NULL ) {
        if ( DbObjectTable[i].fncSet != NULL ) {
            va_list ap;
            va_start(ap, objName);
            status = (*(DbObjectTable[i].fncSet))(ap);
            va_end(ap);
            if ( status == OBJ_STS_OK && DbObjectTable[i].rebootState == TRUE )
                status = OBJ_STS_NEED_REBOOT;
        } else
            status = OBJ_STS_ERR_FUNCTION_NOT_FOUND;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmDb_objDelete
// Description  : remove object out of PSI for the given object name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmDb_objDelete(char *objName, ...) {
    int  i = 0;
    char *ret = NULL;
    OBJ_STATUS status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
   
    for ( i = 0, ret = DbObjectTable[i].objName;
          ret != NULL;
          i++, ret = DbObjectTable[i].objName )
        if ( strcmp(objName, DbObjectTable[i].objName) == 0 )
            break;

    if ( ret != NULL ) {
        if ( DbObjectTable[i].fncDelete != NULL ) {
            va_list ap;
            va_start(ap, objName);
            status = (*(DbObjectTable[i].fncDelete))(ap);
            va_end(ap);
            if ( status == OBJ_STS_OK && DbObjectTable[i].rebootState == TRUE )
                status = OBJ_STS_NEED_REBOOT;
        } else
            status = OBJ_STS_ERR_FUNCTION_NOT_FOUND;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getSystem
// Description  : get specific system information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSystem(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    int parameter = va_arg(ap, int);    
    if ( parameter < OBJ_SYSTEM_PSI_CHANGE_NUM || parameter > OBJ_SYSTEM_WAN_DEVICE_NUM )
        return status;
    
    int field = 0;
    int *pNumInt = NULL;
    unsigned long *pNumUL = NULL;
    char *interface = NULL;
        
    switch ( parameter ) {
    case OBJ_SYSTEM_PSI_CHANGE_NUM:
        pNumUL = va_arg(ap, unsigned long *);    
        if ( pNumUL == NULL ) return status;
        *pNumUL = (unsigned long) BcmPsi_getNumberOfChanges();
        break;
    case OBJ_SYSTEM_LAN_DEVICE_NUM:
        pNumInt = va_arg(ap, int *);
        if ( pNumInt == NULL ) return status;
        *pNumInt = BcmDb_getNumberOfLanConnections();
        break;
    case OBJ_SYSTEM_WAN_DEVICE_NUM:
        pNumInt = va_arg(ap, int *);
        if ( pNumInt == NULL ) return status;
        *pNumInt = BcmDb_getNumberOfWanConnections();
        break;
    case OBJ_SYSTEM_LAN_STATISTICS:
        // lan interface name, "all" means all lan interfaces
        interface = va_arg(ap, char *);
        if ( interface == NULL ) return status;
        // OBJ_STS_RX_BYTE, OBJ_STS_RX_PACKET, ... or OBJ_STS_ALL for all fields
        field = va_arg(ap, int);
        if ( field < OBJ_STS_RX_BYTE || field > OBJ_STS_ALL ) return status;
        // pointer to statistic values
        pNumUL = va_arg(ap, unsigned long *);
        if ( pNumUL == NULL ) return status;  
        BcmDb_getSpecIfcStats(STS_FROM_LAN, interface, field, pNumUL);      
        break;
    case OBJ_SYSTEM_WAN_STATISTICS:
        // wan interface name, "all" means all wan interfaces
        interface = va_arg(ap, char *);
        if ( interface == NULL ) return status;
        // OBJ_STS_RX_BYTE, OBJ_STS_RX_PACKET, ... or OBJ_STS_ALL for all fields
        field = va_arg(ap, int);
        if ( field < OBJ_STS_RX_BYTE || field > OBJ_STS_ALL ) return status;
        // pointer to statistic values
        pNumUL = va_arg(ap, unsigned long *);
        if ( pNumUL == NULL ) return status;  
        BcmDb_getSpecIfcStats(STS_FROM_WAN, interface, field, pNumUL);      
        break;
    }
    
    status = OBJ_STS_OK;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_doSystem
// Description  : handle system action (reboot, save, kill apps).
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_doSystem(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    int parameter = va_arg(ap, int);    
    if ( parameter < OBJ_SYSTEM_REBOOT || parameter > OBJ_SYSTEM_KILL_APPS )
        return status;
    
    int imageSize = 0;
    char *pImage = NULL;
    PARSE_RESULT result = NO_IMAGE_FORMAT;
    
    switch ( parameter ) {
    case OBJ_SYSTEM_REBOOT:
        /* BEGIN: Added by y67514, 2008/7/23   PN:AU8D00825:将hspa模块中Mobile Connection Type从3G only切换为其他模式时,hspa不能拨号成功*/
        Reboot_HSPAStick();
        /* END:   Added by y67514, 2008/7/23 */
        sysMipsSoftReset();
        break;
    case OBJ_SYSTEM_SAVE_CONFIG:
        BcmPsi_flush();
        break;
    case OBJ_SYSTEM_SAVE_IMAGE:
        pImage = va_arg(ap, char *);
        if ( pImage == NULL ) return status;
        imageSize = va_arg(ap, int);
        result = parseImageData(pImage, imageSize, BUF_ALL_TYPES);
        if ( result == NO_IMAGE_FORMAT ) return status;
        flashImage(pImage, result, imageSize);
        break;
    case OBJ_SYSTEM_RESTORE_DEFAULT:
        BcmDb_restoreDefault();
        break;
    case OBJ_SYSTEM_KILL_APPS:
        bcmKillAllApps();
        break;
    }
    
    status = OBJ_STS_OK;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getSwVersion
// Description  : retrieve system software version.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSwVersion(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    bcmGetSwVer(objValue, *size);

    if ( objValue != NULL )
        *size = strlen(objValue);
    status = OBJ_STS_OK;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getHwVersion
// Description  : retrieve system hardware version.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getHwVersion(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    sysGetBoardIdName(objValue, *size);

    if ( objValue != NULL )
        *size = strlen(objValue);
    status = OBJ_STS_OK;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getSysLogData
// Description  : retrieve system log.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSysLogData(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    bcmSystemMute("logread 1> /var/syslog");
    FILE *fp = fopen("/var/syslog", "r");

    if ( fp != NULL ) {
        char line[IFC_LARGE_LEN];
        int len = 0;
        objValue[0] = '\0';
        while ( fgets(line, IFC_LARGE_LEN, fp) != NULL ) {
            len = strlen(line) + strlen(objValue);
            if ( len >= *size ) break;
            strcat(objValue, line);
        }
        fclose(fp);
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OJB_STS_ERR_MEMORY;
        
    unlink("/var/syslog");

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getSysUserName
// Description  : retrieve system user name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSysUserName(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(SYS_USER_NAME_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSysUserName
// Description  : store system user name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSysUserName(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int size = va_arg(ap, int);    
    if ( size == 0 ) return status;
    
    BcmDb_setLoginInfo(SYS_USER_NAME_ID, objValue, size);

    return OBJ_STS_OK;
}

#ifndef SUPPORT_ONE_USER
//**************************************************************************
// Function Name: BcmObj_getSptUserName
// Description  : retrieve support user name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSptUserName(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(SPT_USER_NAME_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getUsrUserName
// Description  : retrieve read-only user name.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getUsrUserName(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(USR_USER_NAME_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}
#endif
//**************************************************************************
// Function Name: BcmObj_getSysPassword
// Description  : retrieve system password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSysPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(SYS_PASSWORD_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSysPassword
// Description  : store system password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSysPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int size = va_arg(ap, int);    
    if ( size == 0 ) return status;
    
    BcmDb_setLoginInfo(SYS_PASSWORD_ID, objValue, size);

    return OBJ_STS_OK;
}
#ifndef SUPPORT_ONE_USER
//**************************************************************************
// Function Name: BcmObj_getSptPassword
// Description  : retrieve support password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSptPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(SPT_PASSWORD_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSptPassword
// Description  : store support password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSptPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int size = va_arg(ap, int);    
    if ( size == 0 ) return status;
    
    BcmDb_setLoginInfo(SPT_PASSWORD_ID, objValue, size);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getUsrPassword
// Description  : retrieve read-only user password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getUsrPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int *size = va_arg(ap, int *);    
    if ( size == NULL ) return status;
    
    int ret = BcmDb_getLoginInfo(USR_PASSWORD_ID, objValue, *size);

    if ( ret == DB_GET_OK ) {
        *size = strlen(objValue);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setUsrPassword
// Description  : store read-only user password.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setUsrPassword(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *objValue = va_arg(ap, char *);    
    if ( objValue == NULL ) return status;
    
    int size = va_arg(ap, int);    
    if ( size == 0 ) return status;
    
    BcmDb_setLoginInfo(USR_PASSWORD_ID, objValue, size);

    return OBJ_STS_OK;
}
#endif
//**************************************************************************
// Function Name: BcmObj_getDnsInfo
// Description  : retrieve DNS information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getDnsInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    PIFC_DNS_INFO pDnsInfo = va_arg(ap, PIFC_DNS_INFO);    
    if ( pDnsInfo == NULL ) return status;
    
    int ret = BcmDb_getDnsInfo(pDnsInfo);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setDnsInfo
// Description  : store DNS information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setDnsInfo(va_list ap) {
    PIFC_DNS_INFO pDnsInfo = va_arg(ap, PIFC_DNS_INFO);    
    
    if ( pDnsInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmDb_setDnsInfo(pDnsInfo);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_deleteDnsInfo
// Description  : remove DNS information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteDnsInfo(va_list ap) {
    BcmDb_removeDnsInfo();

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getDefaultGatewayInfo
// Description  : retrieve default gateway information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getDefaultGatewayInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    PIFC_DEF_GW_INFO pGtwyInfo = va_arg(ap, PIFC_DEF_GW_INFO);    
    if ( pGtwyInfo == NULL ) return status;
    
    int ret = BcmDb_getDefaultGatewayInfo(pGtwyInfo);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setDefaultGatewayInfo
// Description  : store default gateway information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setDefaultGatewayInfo(va_list ap) {
    PIFC_DEF_GW_INFO pGtwyInfo = va_arg(ap, PIFC_DEF_GW_INFO);    
    
    if ( pGtwyInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmDb_setDefaultGatewayInfo(pGtwyInfo);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_deleteDefaultGatewayInfo
// Description  : remove default gateway information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteDefaultGatewayInfo(va_list ap) {
    BcmDb_removeDefaultGatewayInfo();

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getSysLogInfo
// Description  : retrieve system log information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSysLogInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BCM_SYSLOG_CB *pSysLog = va_arg(ap, BCM_SYSLOG_CB *);    
    if ( pSysLog == NULL ) return status;
    
    int ret = BcmDb_getSysLogInfo(pSysLog);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSysLogInfo
// Description  : store system log information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSysLogInfo(va_list ap) {
    BCM_SYSLOG_CB *pSysLog = va_arg(ap, BCM_SYSLOG_CB *);    
    
    if ( pSysLog == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmDb_setSysLogInfo(pSysLog);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getSysFlagInfo
// Description  : retrieve system flag specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSysFlagInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int *value = va_arg(ap, int *);    
    if ( value == NULL ) return status;
    
    int parameter = va_arg(ap, int);
    if ( parameter != SYS_FLAG_AUTO_SCAN && 
         parameter != SYS_FLAG_UPNP &&
         parameter != SYS_FLAG_MAC_POLICY &&
         parameter != SYS_FLAG_SIPROXD &&
         parameter != SYS_FLAG_ENCODE_PASSWORD &&
         parameter != SYS_FLAG_ENET_WAN &&
         parameter != SYS_FLAG_DDNS) //add by l66195 for VDF
       return status;    
       
    int ret = BcmDb_getSpecificSysFlagInfo(value, parameter);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSysFlagInfo
// Description  : store system flag specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSysFlagInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int value = va_arg(ap, int);    
    
    int parameter = va_arg(ap, int);
    if ( parameter != SYS_FLAG_AUTO_SCAN && 
         parameter != SYS_FLAG_UPNP &&
         parameter != SYS_FLAG_MAC_POLICY &&
         parameter != SYS_FLAG_SIPROXD &&
         parameter != SYS_FLAG_ENCODE_PASSWORD &&
         parameter != SYS_FLAG_ENET_WAN &&
         parameter != SYS_FLAG_DDNS) //add by l66195 for VDF
       return status;    
       
    BcmDb_setSpecificSysFlagInfo(value, parameter);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getLanInfo
// Description  : retrieve LAN information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getLanInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int lanId = va_arg(ap, int);
    if ( lanId != IFC_ENET_ID && 
         lanId != IFC_SUBNET_ID &&
         lanId != IFC_USB_ID )
       return status;    
       
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_LAN_INFO || parameter > OBJ_DHCP_SRV_END_ADDR )
        return status;
        
    int ret = DB_GET_OK;
    
    if ( parameter == OBJ_LAN_INFO ) {
        PIFC_LAN_INFO pLanInfo = va_arg(ap, PIFC_LAN_INFO);        
        if ( pLanInfo == NULL ) return status;
        ret = BcmDb_getLanInfo((UINT16)lanId, pLanInfo);
    } else {
        IFC_LAN_INFO lanInfo;    
        char *valueStr = NULL;
        int *valueInt = NULL;
        ret = BcmDb_getLanInfo((UINT16)lanId, &lanInfo);
        switch ( parameter ) {
        case OBJ_LAN_IP_ADDRESS:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, inet_ntoa(lanInfo.lanAddress));
            break;
        case OBJ_LAN_SUBNET_MASK:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, inet_ntoa(lanInfo.lanMask));
            break;
        case OBJ_DHCP_SRV_ENABLE:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = lanInfo.dhcpSrv.enblDhcpSrv;
            break;
        case OBJ_DHCP_SRV_LEASE_TIME:
            valueInt = va_arg(ap, int *);        
            if ( valueInt == NULL ) return status;
            *valueInt = lanInfo.dhcpSrv.leasedTime;
            break;
        case OBJ_DHCP_SRV_START_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, inet_ntoa(lanInfo.dhcpSrv.startAddress));
            break;
        case OBJ_DHCP_SRV_END_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, inet_ntoa(lanInfo.dhcpSrv.endAddress));
            break;
       /*start of support to the dhcp relay  function by l129990,2009,11,12*/
        case OBJ_DHCP_RELAY_ENABLE:
            valueInt = va_arg(ap, int *);        
            if ( valueInt == NULL ) return status;
            *valueInt = lanInfo.dhcpRelay;
            break;
       /*end of support to the dhcp relay  function by l129990,2009,11,12*/
        default :
            return status;
        }
    }
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setLanInfo
// Description  : store LAN information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setLanInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int lanId = va_arg(ap, int);
    if ( lanId != IFC_ENET_ID && 
         lanId != IFC_SUBNET_ID &&
         lanId != IFC_USB_ID )
        return status;    
       
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_LAN_INFO || parameter > OBJ_DHCP_SRV_END_ADDR )
        return status;
        
    if ( parameter == OBJ_LAN_INFO ) {
        PIFC_LAN_INFO pLanInfo = va_arg(ap, PIFC_LAN_INFO);        
        if ( pLanInfo == NULL ) return status;
        BcmDb_setLanInfo((UINT16)lanId, pLanInfo);
    } else {
        IFC_LAN_INFO lanInfo;    
        char *valueStr = NULL;
        int valueInt = 0;
        BcmDb_getLanInfo((UINT16)lanId, &lanInfo);
        switch ( parameter ) {
        case OBJ_LAN_IP_ADDRESS:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            lanInfo.lanAddress.s_addr = inet_addr(valueStr);
            break;
        case OBJ_LAN_SUBNET_MASK:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            lanInfo.lanMask.s_addr = inet_addr(valueStr);
            break;
        case OBJ_DHCP_SRV_ENABLE:
            valueInt = va_arg(ap, int);
            lanInfo.dhcpSrv.enblDhcpSrv = valueInt;        
            break;
        case OBJ_DHCP_SRV_LEASE_TIME:
            valueInt = va_arg(ap, int);        
            lanInfo.dhcpSrv.leasedTime = valueInt;        
            break;
        case OBJ_DHCP_SRV_START_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            lanInfo.dhcpSrv.startAddress.s_addr = inet_addr(valueStr);
            break;
        case OBJ_DHCP_SRV_END_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            lanInfo.dhcpSrv.endAddress.s_addr = inet_addr(valueStr);
            break;
        /*start of support to the dhcp relay  function by l129990,2009,11,12*/
        case OBJ_DHCP_RELAY_ENABLE:
            valueInt = va_arg(ap, int);        
            lanInfo.dhcpRelay = valueInt;
            break;
        /*end of support to the dhcp relay  function by l129990,2009,11,12*/
        default :
            break;
        }
        BcmDb_setLanInfo((UINT16)lanId, &lanInfo);
    }

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_deleteLanInfo
// Description  : remove LAN information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteLanInfo(va_list ap) {
    int lanId = va_arg(ap, int);
    
    if ( lanId != IFC_ENET_ID && 
         lanId != IFC_SUBNET_ID &&
         lanId != IFC_USB_ID )
       return OBJ_STS_ERR_INVALID_ARGUMENT;    
    
    BcmDb_removeLanInfo((UINT16)lanId);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getAtmTdInfo
// Description  : get specific paramter for ATM TD information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAtmTdInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_TD_INFO || parameter > OBJ_TD_ID_AVAILABLE )
        return status;
        
    if ( parameter == OBJ_TD_ID_AVAILABLE ) {
        // the next parameters should be "&tdObjId"
        int *objId = va_arg(ap, int *);
        if ( objId == NULL ) return status;
        *objId = AtmTd_getAvailableObjectId();
        status = OBJ_STS_OK;
    } else if ( parameter == OBJ_TD_ID_FROM_INFO ) {
        // the next parameters should be "&tdInfo &objId"
        PIFC_ATM_TD_INFO pInfo = va_arg(ap, PIFC_ATM_TD_INFO);        
        if ( pInfo == NULL ) return status;
        int *objId = va_arg(ap, int *);
        if ( objId == NULL ) return status;
        *objId = AtmTd_getObjectIdFromInfo(pInfo);
        status = OBJ_STS_OK;
    } else if ( parameter ==  OBJ_TD_ID_FROM_INDEX ) {
        // the next parameters should be "tdIndex &objId"
        unsigned long tdId = va_arg(ap, unsigned long);
        int *objId = va_arg(ap, int *);
        if ( objId == NULL ) return status;
        *objId = AtmTd_getObjectIdFromIndex(tdId);
        status = OBJ_STS_OK;
    } else if ( parameter ==  OBJ_TD_INFO ) {
        // the next parameters should be "tdObjId &tdInfo"
        int objId = va_arg(ap, int);
        PIFC_ATM_TD_INFO pInfo = va_arg(ap, PIFC_ATM_TD_INFO);        
        if ( pInfo == NULL ) return status;
        if ( AtmTd_getTdInfoFromObjectId((UINT16)objId, pInfo) == TRUE )
            status = OBJ_STS_OK;
        else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    } else {
        // the next parameters should be "tdObjId &paramValue"
        IFC_ATM_TD_INFO info;
        char *valueStr = NULL;
        unsigned long *valueUl = NULL;
        int objId = va_arg(ap, int);
        if ( AtmTd_getTdInfoFromObjectId((UINT16)objId, &info) == TRUE ) {
            switch ( parameter ) {
            case OBJ_TD_PEAK_CELL_RATE:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.peakCellRate;
                break;
            case OBJ_TD_SUSTAINED_CELL_RATE:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.sustainedCellRate;
                break;
            case OBJ_TD_MAX_BURST_SIZE:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.maxBurstSize;
                break;
            case OBJ_TD_SERVICE_CATEGORY:
                valueStr = va_arg(ap, char *);
                if ( valueStr == NULL ) return status;
                strcpy(valueStr, info.serviceCategory);
                break;
            default:
                return status;
            }
            status = OBJ_STS_OK;
        } else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setAtmTdInfo
// Description  : set specific paramter for ATM TD information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAtmTdInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_TD_INFO || parameter > OBJ_TD_SERVICE_CATEGORY )
        return status;
        
    if ( parameter == OBJ_TD_INFO ) {
        // if parameter is OBJ_TD_INFO then the next parameters should be 
        // "tdObjId &tdInfo"
        int objId = va_arg(ap, int);
        PIFC_ATM_TD_INFO pInfo = va_arg(ap, PIFC_ATM_TD_INFO);        
        if ( pInfo == NULL ) return status;
        AtmTd_setTdInfoFromObjectId(objId, pInfo);
        status = OBJ_STS_OK;
    } else {
        // if parameter is not OBJ_TD_INFO then the next parameters 
        // should be "tdObjId paramValue"
        IFC_ATM_TD_INFO info;
        char *valueStr = NULL;
        unsigned long valueUl = 0;
        int objId = va_arg(ap, int);
        if ( AtmTd_getTdInfoFromObjectId((UINT16)objId, &info) == TRUE ) {
            switch ( parameter ) {
            case OBJ_TD_PEAK_CELL_RATE:
                valueUl = va_arg(ap, unsigned long);
                info.peakCellRate = valueUl;
                break;
            case OBJ_TD_SUSTAINED_CELL_RATE:
                valueUl = va_arg(ap, unsigned long);
                info.sustainedCellRate = valueUl;
                break;
            case OBJ_TD_MAX_BURST_SIZE:
                valueUl = va_arg(ap, unsigned long);
                info.maxBurstSize = valueUl;
                break;
            case OBJ_TD_SERVICE_CATEGORY:
                valueStr = va_arg(ap, char *);
                if ( valueStr == NULL ) return status;
                if ( BcmDb_validateObj(DB_TYPE_ATM_CATEGORY, valueStr) != DB_OBJ_VALID_OK ) return status;
                strcpy(info.serviceCategory, valueStr);
                break;
            default:
                return status;
            }
            AtmTd_setTdInfoFromObjectId(objId, &info);
            status = OBJ_STS_OK;
        } else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteAtmTdInfo
// Description  : remove ATM TD information using object ID.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteAtmTdInfo(va_list ap) {
    int objId = va_arg(ap, int);
        
    AtmTd_removeFromPsiOnly((UINT16)objId);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getAtmVccInfo
// Description  : retrieve specific ATM VCC information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAtmVccInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;

    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_VCC_INFO || parameter > OBJ_VCC_ID_AVAILABLE )
        return status;
        
    if ( parameter == OBJ_VCC_ID_AVAILABLE ) {
        // the next parameter should be "&vccObjId"
        int *objId = va_arg(ap, int *);
        if ( objId == NULL ) return status;
        *objId = AtmVcc_getAvailableObjectId();
        if ( *objId != 0 )
            status = OBJ_STS_OK;        
        else
            status = OBJ_STS_ERR_VCC_NOT_AVAILABLE;
    } else if ( parameter == OBJ_VCC_ID_FROM_ADDRESS ) {
        // the next parameter should be "vpi vci &vccObjId"
        char buf[IFC_TINY_LEN];
        int vpi = va_arg(ap, int);
        sprintf(buf, "%d", vpi);
        if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;    
        int vci = va_arg(ap, int);
        sprintf(buf, "%d", vci);
        if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
        
        ATM_VCC_ADDR vccAddr;
        vccAddr.usVpi = vpi;
        vccAddr.usVci = vci;
        vccAddr.ulInterfaceId = IFC_PORT_ID_BASE + 1;
    
        int *objId = va_arg(ap, int *);
        if ( objId == NULL ) return status;
        *objId = AtmVcc_findByAddress(&vccAddr);
        if ( *objId != 0 )
            status = OBJ_STS_OK;        
        else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;        
    } else if ( parameter == OBJ_VCC_INFO ) {
        // the next parameters should be "vccObjId &vccInfo"
        int objId = va_arg(ap, int);
        PIFC_ATM_VCC_INFO pInfo = va_arg(ap, PIFC_ATM_VCC_INFO);        
        if ( pInfo == NULL ) return status;
        if ( AtmVcc_getByObjectId(objId, pInfo) != 0 )
            status = OBJ_STS_OK;
        else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;        
    } else {
        // the next parameters should be "vccObjId paramValue"
        IFC_ATM_VCC_INFO info;
        int *valueInt = NULL;
        unsigned long *valueUl = NULL;
        int objId = va_arg(ap, int);
        if ( AtmVcc_getByObjectId(objId, &info) != 0 ) {
            switch ( parameter ) {
            case OBJ_VCC_TD_ID:
                valueInt = va_arg(ap, int *);
                if ( valueInt == NULL ) return status;
                *valueInt = info.tdId;
                break;
            case OBJ_VCC_AAL_TYPE:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.aalType;
                break;
            case OBJ_VCC_ADMIN_STATUS:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.adminStatus;
                break;
            case OBJ_VCC_ENCAP_MODE:
                valueInt = va_arg(ap, int *);
                if ( valueInt == NULL ) return status;
                *valueInt = info.encapMode;
                break;
            case OBJ_VCC_ENABLE_QOS:
                valueInt = va_arg(ap, int *);
                if ( valueInt == NULL ) return status;
                *valueInt = info.enblQos;
                break;
            case OBJ_VCC_VPI:
                valueInt = va_arg(ap, int *);
                if ( valueInt == NULL ) return status;
                *valueInt = info.vccAddr.usVpi;
                break;
            case OBJ_VCC_VCI:
                valueInt = va_arg(ap, int *);
                if ( valueInt == NULL ) return status;
                *valueInt = info.vccAddr.usVci;
                break;
            case OBJ_VCC_IFC_ID:
                valueUl = va_arg(ap, unsigned long *);
                if ( valueUl == NULL ) return status;
                *valueUl = info.vccAddr.ulInterfaceId;
                break;
            default:
                return status;
            }
            status = OBJ_STS_OK;
        } else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    }
    
    return status;    
}

//**************************************************************************
// Function Name: BcmObj_setAtmVccInfo
// Description  : store specific ATM VCC information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAtmVccInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;

    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_VCC_INFO || parameter > OBJ_VCC_IFC_ID )
        return status;
        
    if ( parameter == OBJ_VCC_INFO ) {
        // the next parameters should be "vccObjId &vccInfo"
        int objId = va_arg(ap, int);
        PIFC_ATM_VCC_INFO pInfo = va_arg(ap, PIFC_ATM_VCC_INFO);        
        if ( pInfo == NULL ) return status;
        if ( AtmVcc_setByObjectIdToPsiOnly(objId, pInfo) == TRUE )
            status = OBJ_STS_OK;
    } else {
        // the next parameters should be "vccObjId paramValue"
        IFC_ATM_VCC_INFO info;
        char buf[IFC_TINY_LEN];
        int valueInt = 0;
        unsigned long valueUl = 0;
        int objId = va_arg(ap, int);
        if ( AtmVcc_getByObjectId(objId, &info) != 0 ) {
            switch ( parameter ) {
            case OBJ_VCC_TD_ID:
                valueInt = va_arg(ap, int);
                info.tdId = valueInt;
                break;
            case OBJ_VCC_AAL_TYPE:
                valueUl = va_arg(ap, unsigned long);
                info.aalType = valueUl;
                break;
            case OBJ_VCC_ADMIN_STATUS:
                valueUl = va_arg(ap, unsigned long);
                info.adminStatus = valueUl;
                break;
            case OBJ_VCC_ENCAP_MODE:
                valueInt = va_arg(ap, int);
                info.encapMode = valueInt;
                break;
            case OBJ_VCC_ENABLE_QOS:
                valueInt = va_arg(ap, int);
                info.enblQos = valueInt;
                break;
            case OBJ_VCC_VPI:
                valueInt = va_arg(ap, int);
                sprintf(buf, "%d", valueInt);
                if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
                info.vccAddr.usVpi = valueInt;
                break;
            case OBJ_VCC_VCI:
                valueInt = va_arg(ap, int);
                sprintf(buf, "%d", valueInt);
                if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
                info.vccAddr.usVci = valueInt;
                break;
            case OBJ_VCC_IFC_ID:
                valueUl = va_arg(ap, unsigned long);
                info.vccAddr.ulInterfaceId = valueUl;
                break;
            default:
                return status;
            }
            if ( AtmVcc_setByObjectIdToPsiOnly(objId, &info) == TRUE )
                status = OBJ_STS_OK;
        } else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    }
    
    return status;    
}

//**************************************************************************
// Function Name: BcmObj_deleteAtmVccInfo
// Description  : remove ATM VCC information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteAtmVccInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    char buf[IFC_TINY_LEN];
    
    int parameter = va_arg(ap, int);
    
    if ( parameter == OBJ_VCC_DELETE_BY_ADDRESS ) {
        // the next parameters should be "vpi vci"
        int vpi = va_arg(ap, int);
        sprintf(buf, "%d", vpi);
        if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
    
        int vci = va_arg(ap, int);
        sprintf(buf, "%d", vci);
        if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
        
        if ( AtmVcc_isVccInUsed(vpi, vci) == TRUE ) return OBJ_STS_ERR_VCC_IN_USED;
    
        ATM_VCC_ADDR vccAddr;
        vccAddr.usVpi = vpi;
        vccAddr.usVci = vci;
        vccAddr.ulInterfaceId = IFC_PORT_ID_BASE + 1;
    
        int objId = AtmVcc_findByAddress(&vccAddr);
        if ( objId == 0 ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
        AtmVcc_removeByObjectIdFromPsiOnly(objId);
        status = OBJ_STS_OK;
    } else if ( parameter == OBJ_VCC_DELETE_BY_ID ) {
        // the next parameters should be "vccObjId"
        IFC_ATM_VCC_INFO info;
        int objId = va_arg(ap, int);
        if ( AtmVcc_getByObjectId(objId, &info) != 0 )
            status = BcmDb_objDelete(ATM_VCC_APPID, OBJ_VCC_DELETE_BY_ADDRESS,
                                     info.vccAddr.usVpi, info.vccAddr.usVci);
        else
            status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_getWanInfo
// Description  : retrieve specific WAN connection information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getWanInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;

    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_WAN_CON_INFO || parameter > OBJ_WAN_ID_AVAILABLE )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 2 parameters should be "vpi vci"
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    char appWan[IFC_TINY_LEN];
    sprintf(appWan, "wan_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiWan = BcmPsi_appOpen(appWan);
    UINT16 retLen = 0;
    WAN_CON_INFO wanInfo;
    PSI_STATUS sts = PSI_STS_OK;
    
    if ( parameter == OBJ_WAN_ID_AVAILABLE ) {
        // the next parameter should be "&conId"
        int *conId = va_arg(ap, int *);
        if ( conId == NULL ) return status;
        int id = 0;
        for ( id = 1; id <= IFC_WAN_MAX; id++ ) {
            sts = BcmPsi_objRetrieve(psiWan, id, &wanInfo,
                                     sizeof(wanInfo), &retLen);
            if ( sts != PSI_STS_OK )
                break;
        }    
        if ( id <= IFC_WAN_MAX ) {
            *conId = id;
            status = OBJ_STS_OK;
        } else
            status = OBJ_STS_ERR_WAN_NOT_AVAILABLE;
    } else {
        // the next parameter should be "conId"
        wanId.conId = va_arg(ap, int);
        if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
        
        sts = BcmPsi_objRetrieve(psiWan, wanId.conId, &wanInfo,
                                 sizeof(wanInfo), &retLen);
        if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
        
        int *valueInt = NULL;
        char *valueStr = NULL;
        PWAN_CON_INFO pInfo = NULL;
        switch ( parameter ) {
        case OBJ_WAN_CON_INFO:
            // the next parameter should be "&wanInfo"
            pInfo = va_arg(ap, PWAN_CON_INFO);
            if ( pInfo == NULL ) return status;
            memcpy(pInfo, &wanInfo, sizeof(WAN_CON_INFO));
            break;
        case OBJ_WAN_VCC_ID:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.vccId;
            break;
        case OBJ_WAN_CON_ID:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.conId;
            break;
        case OBJ_WAN_PROTOCOL:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.protocol;
            break;
        case OBJ_WAN_ENCAP_MODE:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.encapMode;
            break;
        case OBJ_WAN_CON_NAME:
            // the next parameter should be "&paramValue"
            valueStr = va_arg(ap, char *);
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, wanInfo.conName);
            break;
        case OBJ_WAN_FLAG_FIREWALL:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.flag.firewall;
            break;
        case OBJ_WAN_FLAG_NAT:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.flag.nat;
            break;
/*start of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
   #if 0
        case OBJ_WAN_FLAG_IGMP:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.flag.igmp;
            break;
   #endif
/*end of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
        case OBJ_WAN_FLAG_SERVICE:
            // the next parameter should be "&paramValue"
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = wanInfo.flag.service;
            break;
        default:
            return status;
        }
        status = OBJ_STS_OK;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setWanInfo
// Description  : store specific WAN connection information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setWanInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);

	 printf ("DBOBJ :- In Set Wan info... \n") ;

    if ( parameter < OBJ_WAN_CON_INFO || parameter > OBJ_WAN_FLAG_SERVICE )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 3 parameters should be "vpi vci conId" for wanId
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
    
    char appWan[IFC_TINY_LEN];
    sprintf(appWan, "wan_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiWan = BcmPsi_appOpen(appWan);
    
    if ( parameter == OBJ_WAN_CON_INFO ) {
        // the next parameter should be "&wanInfo"
        PWAN_CON_INFO pInfo = va_arg(ap, PWAN_CON_INFO);
        if ( pInfo == NULL ) return status;
        BcmPsi_objStore(psiWan, wanId.conId, pInfo, sizeof(WAN_CON_INFO));
        status = OBJ_STS_OK;
    } else {
        UINT16 retLen = 0;
        WAN_CON_INFO wanInfo;
        PSI_STATUS sts = BcmPsi_objRetrieve(psiWan, wanId.conId, &wanInfo,
                                            sizeof(wanInfo), &retLen);
        if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
        // the next parameter should be "paramValue"
        int valueInt = 0;
        char *valueStr = NULL;
        switch ( parameter ) {
        case OBJ_WAN_VCC_ID:
            valueInt = va_arg(ap, int);
            if ( valueInt < 1 || valueInt > ATM_VCC_MAX ) return status;
            wanInfo.vccId = valueInt;
            break;
        case OBJ_WAN_CON_ID:
            valueInt = va_arg(ap, int);
            if ( valueInt < 1 || valueInt > ATM_VCC_MAX ) return status;
            wanInfo.conId = valueInt;
            break;
        case OBJ_WAN_PROTOCOL:
            valueInt = va_arg(ap, int);
            if ( valueInt < PROTO_PPPOE || valueInt > PROTO_NONE ) return status;
            wanInfo.protocol = (short)valueInt;
            break;
        case OBJ_WAN_ENCAP_MODE:
            valueInt = va_arg(ap, int);
            wanInfo.encapMode = (short)valueInt;
            break;
        case OBJ_WAN_CON_NAME:
            valueStr = va_arg(ap, char *);
            if ( BcmDb_validateObj(DB_TYPE_SERVICE_NAME, valueStr) != DB_OBJ_VALID_OK ) return status;
            strcpy(wanInfo.conName, valueStr);
            break;
        case OBJ_WAN_FLAG_FIREWALL:
            valueInt = va_arg(ap, int);
            if ( valueInt != FALSE && valueInt != TRUE ) return status;
            wanInfo.flag.firewall = valueInt;
            break;
        case OBJ_WAN_FLAG_NAT:
            valueInt = va_arg(ap, int);
            if ( valueInt != FALSE && valueInt != TRUE ) return status;
            wanInfo.flag.nat = valueInt;
            break;
/*start of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
    #if 0
        case OBJ_WAN_FLAG_IGMP:
            valueInt = va_arg(ap, int);
            if ( valueInt != FALSE && valueInt != TRUE ) return status;
            wanInfo.flag.igmp = valueInt;
            break;
    #endif
 /*end of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
        case OBJ_WAN_FLAG_SERVICE:
            valueInt = va_arg(ap, int);
            if ( valueInt != FALSE && valueInt != TRUE ) return status;
            wanInfo.flag.service = valueInt;
            break;
        default:
            return status;
        }
        BcmPsi_objStore(psiWan, wanId.conId, &wanInfo, sizeof(wanInfo));
        status = OBJ_STS_OK;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteWanInfo
// Description  : remove WAN connection information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteWanInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next parameters should be "vpi vci conId"
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;

    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, id = 0;
    WAN_CON_INFO wanInfo;
    PSI_HANDLE psiWan = NULL;

    sprintf(appWan, "wan_%d_%d", wanId.vpi, wanId.vci);
    psiWan = BcmPsi_appOpen(appWan);
      
    PSI_STATUS sts = BcmPsi_objRetrieve(psiWan, wanId.conId, &wanInfo,
                                        sizeof(wanInfo), &retLen);
    if ( sts == PSI_STS_OK ) {
        // remove wan info in PSI
        BcmPsi_objRemove(psiWan, wanId.conId);
        // is there any existed wan in this wan app?
        for ( id = 1; id <= IFC_WAN_MAX; id++ ) { 
            sts = BcmPsi_objRetrieve(psiWan, id, &wanInfo,
                                        sizeof(wanInfo), &retLen);
            if ( sts == PSI_STS_OK )
                break; 
        }
        // there is no existed wan in this wan app so
        // remove wan app but not remove pvc itself
        if ( id > IFC_WAN_MAX )
            BcmPsi_appRemove(psiWan);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getPppSrvInfo
// Description  : retrieve WAN PPP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPppSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_PPP_INFO || parameter > OBJ_PPP_LOCAL_IP_ADDR )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 3 parameters should be "vpi vci conId" for wanId
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
    
    char appPpp[IFC_TINY_LEN];
    sprintf(appPpp, "pppsrv_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiPpp = BcmPsi_appOpen(appPpp);
    
    UINT16 retLen = 0;
    WAN_PPP_INFO pppInfo;
    PSI_STATUS sts = BcmPsi_objRetrieve(psiPpp, wanId.conId, &pppInfo,
                                        sizeof(pppInfo), &retLen);
    if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    // the next parameter should be "&paramValue"
    int *valueInt = NULL;
    char *valueStr = NULL;
    PWAN_PPP_INFO pInfo = NULL;
    switch ( parameter ) {
        case OBJ_PPP_INFO:
            // the next parameter should be "&pppInfo"
            pInfo = va_arg(ap, PWAN_PPP_INFO);
            if ( pInfo == NULL ) return status;
            memcpy(pInfo, &pppInfo, sizeof(WAN_PPP_INFO));
            break;
        case OBJ_PPP_IDLE_TIMEOUT:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = pppInfo.idleTimeout;
            break;
        case OBJ_PPP_IP_EXTENSION:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = pppInfo.enblIpExtension;
            break;
        case OBJ_PPP_AUTH_METHOD:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = pppInfo.authMethod;
            break;
        case OBJ_PPP_USER_NAME:
            valueStr = va_arg(ap, char *);
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, pppInfo.userName);
            break;
        case OBJ_PPP_PASSWORD:
            valueStr = va_arg(ap, char *);
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, pppInfo.password);
            break;
        case OBJ_PPP_SERVICE_NAME:
            valueStr = va_arg(ap, char *);
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, pppInfo.serverName);
            break;
        case OBJ_PPP_USE_STATIC_IP_ADDR:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = pppInfo.useStaticIpAddress;
            break;
        case OBJ_PPP_LOCAL_IP_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;
            strcpy(valueStr, inet_ntoa(pppInfo.pppLocalIpAddress));
            break;
        case OBJ_PPP_DEBUG:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = pppInfo.enblDebug;
            break;
        default:
            return status;
    }

    status = OBJ_STS_OK;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setPppSrvInfo
// Description  : store WAN PPP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPppSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_PPP_INFO || parameter > OBJ_PPP_LOCAL_IP_ADDR )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 3 parameters should be "vpi vci conId" for wanId
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
    
    char appPpp[IFC_TINY_LEN];
    sprintf(appPpp, "pppsrv_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiPpp = BcmPsi_appOpen(appPpp);
    
    if ( parameter == OBJ_PPP_INFO ) {
        // the next parameter should be "&pppInfo"
        PWAN_PPP_INFO pInfo = va_arg(ap, PWAN_PPP_INFO);
        if ( pInfo == NULL ) return status;
        BcmPsi_objStore(psiPpp, wanId.conId, pInfo, sizeof(WAN_PPP_INFO));
        status = OBJ_STS_OK;
    } else {
        UINT16 retLen = 0;
        WAN_PPP_INFO pppInfo;
        PSI_STATUS sts = BcmPsi_objRetrieve(psiPpp, wanId.conId, &pppInfo,
                                            sizeof(pppInfo), &retLen);
        if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
        // the next parameter should be "paramValue"
        int valueInt = 0;
        char *valueStr = NULL;
        switch ( parameter ) {
        case OBJ_PPP_IDLE_TIMEOUT:
            valueInt = va_arg(ap, int);
            sprintf(buf, "%d", valueInt);
            if ( BcmDb_validateObj(DB_TYPE_PPP_TIMEOUT, buf) != DB_OBJ_VALID_OK ) return status;
            pppInfo.idleTimeout = valueInt;
            break;
        case OBJ_PPP_IP_EXTENSION:
            valueInt = va_arg(ap, int);
            if ( valueInt != TRUE && valueInt != FALSE ) return status;
            pppInfo.enblIpExtension = (short)valueInt;
            break;
        case OBJ_PPP_AUTH_METHOD:
            valueInt = va_arg(ap, int);
            if ( valueInt < PPP_AUTH_METHOD_AUTO || valueInt > PPP_AUTH_METHOD_MSCHAP ) return status;
            pppInfo.authMethod = (short)valueInt;
            break;
        case OBJ_PPP_USER_NAME:
            valueStr = va_arg(ap, char *);
            if ( BcmDb_validateObj(DB_TYPE_PPP_USERNAME, valueStr) != DB_OBJ_VALID_OK ) return status;
            strcpy(pppInfo.userName, valueStr);
            break;
        case OBJ_PPP_PASSWORD:
            valueStr = va_arg(ap, char *);
            if ( BcmDb_validateObj(DB_TYPE_PPP_PASSWORD, valueStr) != DB_OBJ_VALID_OK ) return status;
            strcpy(pppInfo.password, valueStr);
            break;
        case OBJ_PPP_SERVICE_NAME:
            valueStr = va_arg(ap, char *);
            if ( BcmDb_validateObj(DB_TYPE_SERVICE_NAME, valueStr) != DB_OBJ_VALID_OK ) return status;
            strcpy(pppInfo.serverName, valueStr);
            break;
        case OBJ_PPP_USE_STATIC_IP_ADDR:
            valueInt = va_arg(ap, int);
            if ( valueInt != FALSE && valueInt != TRUE ) return status;
            pppInfo.useStaticIpAddress = valueInt;
            break;
        case OBJ_PPP_LOCAL_IP_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            pppInfo.pppLocalIpAddress.s_addr = inet_addr(valueStr);
            break;
        case OBJ_PPP_DEBUG:
            valueInt = va_arg(ap, int);
            if ( valueInt != TRUE && valueInt != FALSE ) return status;
            pppInfo.enblDebug = (short)valueInt;
            break;
        default:
            return status;
        }
        BcmPsi_objStore(psiPpp, wanId.conId, &pppInfo, sizeof(pppInfo));
        status = OBJ_STS_OK;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_deletePppSrvInfo
// Description  : remove WAN PPP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deletePppSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next parameters should be "vpi vci conId"
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;

    char appPpp[IFC_TINY_LEN];
    UINT16 retLen = 0, id = 0;
    WAN_PPP_INFO pppInfo;
    PSI_HANDLE psiPpp = NULL;

    sprintf(appPpp, "pppsrv_%d_%d", wanId.vpi, wanId.vci);
    psiPpp = BcmPsi_appOpen(appPpp);
      
    PSI_STATUS sts = BcmPsi_objRetrieve(psiPpp, wanId.conId, &pppInfo,
                                        sizeof(pppInfo), &retLen);
    if ( sts == PSI_STS_OK ) {
        // remove ppp service info in PSI
        BcmPsi_objRemove(psiPpp, wanId.conId);
        // is there any existed wan in this wan app?
        for ( id = 1; id <= IFC_WAN_MAX; id++ ) { 
            sts = BcmPsi_objRetrieve(psiPpp, id, &pppInfo,
                                        sizeof(pppInfo), &retLen);
            if ( sts == PSI_STS_OK )
                break; 
        }
        // there is no existed ppp service in this ppp app so
        // remove this pp app
        if ( id > IFC_WAN_MAX )
            BcmPsi_appRemove(psiPpp);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getIpSrvInfo
// Description  : retrieve WAN IP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getIpSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_IP_INFO || parameter > OBJ_IP_WAN_SUBNET_MASK )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 3 parameters should be "vpi vci conId" for wanId
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
    
    char appIp[IFC_TINY_LEN];
    sprintf(appIp, "ipsrv_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiIp = BcmPsi_appOpen(appIp);
    
    UINT16 retLen = 0;
    WAN_IP_INFO ipInfo;
    PSI_STATUS sts = BcmPsi_objRetrieve(psiIp, wanId.conId, &ipInfo,
                                        sizeof(ipInfo), &retLen);
    if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
    
    // the next parameter should be "&paramValue"
    int *valueInt = NULL;
    char *valueStr = NULL;
    PWAN_IP_INFO pInfo = NULL;
    switch ( parameter ) {
        case OBJ_IP_INFO:
            pInfo = va_arg(ap, PWAN_IP_INFO);
            if ( pInfo == NULL ) return status;
            memcpy(pInfo, &ipInfo, sizeof(WAN_IP_INFO));
            break;
        case OBJ_IP_ENABLE_DHCP_CLNT:
            valueInt = va_arg(ap, int *);
            if ( valueInt == NULL ) return status;
            *valueInt = ipInfo.enblDhcpClnt;
            break;
        case OBJ_IP_WAN_IP_ADDR:
            valueStr = va_arg(ap, char *); 
            if ( valueStr == NULL ) return status;       
            strcpy(valueStr, inet_ntoa(ipInfo.wanAddress));
            break;
        case OBJ_IP_WAN_SUBNET_MASK:
            valueStr = va_arg(ap, char *);        
            if ( valueStr == NULL ) return status;       
            strcpy(valueStr, inet_ntoa(ipInfo.wanMask));
            break;
        default:
            return status;
    }
    
    status = OBJ_STS_OK;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setIpSrvInfo
// Description  : store WAN IP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setIpSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_IP_INFO || parameter > OBJ_IP_WAN_SUBNET_MASK )
        return status;
        
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next 3 parameters should be "vpi vci conId" for wanId
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;
    
    char appIp[IFC_TINY_LEN];
    sprintf(appIp, "ipsrv_%d_%d", wanId.vpi, wanId.vci);
    PSI_HANDLE psiIp = BcmPsi_appOpen(appIp);
    
    if ( parameter == OBJ_IP_INFO ) {
        // the next parameter should be "&ipInfo"
        PWAN_IP_INFO pInfo = va_arg(ap, PWAN_IP_INFO);
        if ( pInfo == NULL ) return status;
        BcmPsi_objStore(psiIp, wanId.conId, pInfo, sizeof(WAN_IP_INFO));
        status = OBJ_STS_OK;
    } else {
        UINT16 retLen = 0;
        WAN_IP_INFO ipInfo;
        PSI_STATUS sts = BcmPsi_objRetrieve(psiIp, wanId.conId, &ipInfo,
                                            sizeof(ipInfo), &retLen);
        if ( sts != PSI_STS_OK ) return OBJ_STS_ERR_OBJECT_NOT_FOUND;
        // the next parameter should be "paramValue"
        int valueInt = 0;
        char *valueStr = NULL;
        switch ( parameter ) {
        case OBJ_IP_ENABLE_DHCP_CLNT:
            valueInt = va_arg(ap, int);
            if ( valueInt != TRUE && valueInt != FALSE ) return status;
            ipInfo.enblDhcpClnt = (short)valueInt;
            break;
        case OBJ_IP_WAN_IP_ADDR:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, valueStr) != DB_OBJ_VALID_OK ) return status;
            ipInfo.wanAddress.s_addr = inet_addr(valueStr);
            break;
        case OBJ_IP_WAN_SUBNET_MASK:
            valueStr = va_arg(ap, char *);        
            if ( BcmDb_validateObj(DB_TYPE_SUBNET_MASK, valueStr) != DB_OBJ_VALID_OK ) return status;
            ipInfo.wanMask.s_addr = inet_addr(valueStr);
            break;
        default:
            return status;
        }
        BcmPsi_objStore(psiIp, wanId.conId, &ipInfo, sizeof(ipInfo));
        status = OBJ_STS_OK;
    }

    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteIpSrvInfo
// Description  : remove WAN IP data information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteIpSrvInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    char buf[IFC_TINY_LEN];
    WAN_CON_ID wanId;
            
    // the next parameters should be "vpi vci conId"
    wanId.vpi = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vpi);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VPI, buf) != DB_OBJ_VALID_OK ) return status;
        
    wanId.vci = va_arg(ap, int);
    sprintf(buf, "%d", wanId.vci);
    if ( BcmDb_validateObj(DB_TYPE_ATM_VCI, buf) != DB_OBJ_VALID_OK ) return status;
    
    wanId.conId = va_arg(ap, int);
    if ( wanId.conId < 1 || wanId.conId > IFC_WAN_MAX ) return status;

    char appIp[IFC_TINY_LEN];
    UINT16 retLen = 0, id = 0;
    WAN_IP_INFO ipInfo;
    PSI_HANDLE psiIp = NULL;

    sprintf(appIp, "ipsrv_%d_%d", wanId.vpi, wanId.vci);
    psiIp = BcmPsi_appOpen(appIp);
      
    PSI_STATUS sts = BcmPsi_objRetrieve(psiIp, wanId.conId, &ipInfo,
                                        sizeof(ipInfo), &retLen);
    if ( sts == PSI_STS_OK ) {
        // remove ip service info in PSI
        BcmPsi_objRemove(psiIp, wanId.conId);
        // is there any existed wan in this wan app?
        for ( id = 1; id <= IFC_WAN_MAX; id++ ) { 
            sts = BcmPsi_objRetrieve(psiIp, id, &ipInfo,
                                     sizeof(ipInfo), &retLen);
            if ( sts == PSI_STS_OK )
                break; 
        }
        // there is no existed ip service in this ip app so
        // remove this ip app
        if ( id > IFC_WAN_MAX )
            BcmPsi_appRemove(psiIp);
        status = OBJ_STS_OK;
    } else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
        
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getAdslInfo
// Description  : retrieve ADSL configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAdslInfo(va_list ap) {
    unsigned int *value = va_arg(ap, unsigned int *);
    if ( value == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmAdsl_getAdslProfile(value);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setAdslInfo
// Description  : store ADSL profile information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAdslInfo(va_list ap) {
    unsigned int value = va_arg(ap, unsigned int);
    
    BcmAdsl_setAdslProfile(value);

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getAtmInitCfg
// Description  : retrieve ATM initialization information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAtmInitCfg(va_list ap) {
    PATM_INITIALIZATION_PARMS value = va_arg(ap, PATM_INITIALIZATION_PARMS);
    if ( value == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    AtmInit_getInitCfgDefault(value);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setAtmInitCfg
// Description  : store ATM initialization information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAtmInitCfg(va_list ap) {
    AtmInit_setInitCfg();

    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getRipGlobal
// Description  : retrieve RIP global configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getRipGlobal(va_list ap) {
    // pointer to RIP global object
    RT_RIP_CFG *pObject = va_arg(ap, RT_RIP_CFG *);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    if ( BcmRcm_getRipConfiguration(pObject) != ROUTE_STS_OK ) 
        pObject->globalMode = atoi(BcmDb_getDefaultValue("gRipMode"));
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setRipGlobal
// Description  : store RIP global configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setRipGlobal(va_list ap) {
    if ( BcmRcm_isInitialized() == FALSE ) return OBJ_STS_ERR_GENERAL;

    // pointer to RIP global object
    RT_RIP_CFG *pObject = va_arg(ap, RT_RIP_CFG *);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    OBJ_STATUS status = OBJ_STS_OK;
    RT_RIP_CFG currentRipCfg;

    if ( BcmRcm_getRipConfiguration(&currentRipCfg) != ROUTE_STS_OK ) 
        currentRipCfg.globalMode = atoi(BcmDb_getDefaultValue("gRipMode"));

    if ( currentRipCfg.globalMode != pObject->globalMode ) {
        ROUTE_STATUS sts = BcmRcm_setRipConfiguration(pObject);
        if ( sts == ROUTE_STS_OK )
            BcmRcm_startRip();      // do it only if changed
        else 
            status = OJB_STS_ERR_MEMORY;
    } 
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getRipIfc
// Description  : retrieve static RIP interface configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getRipIfc(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // pointer to RIP interface node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // pointer to RIP interface object
    PRT_RIP_IFC_ENTRY pObject = va_arg(ap, PRT_RIP_IFC_ENTRY);
    if ( pObject == NULL ) return status;
    
    *value = (unsigned long)BcmRcm_getRipInterface((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setRipIfc
// Description  : store static RIP interface configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setRipIfc(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // pointer to RIP interface object
    PRT_RIP_IFC_ENTRY pObject = va_arg(ap, PRT_RIP_IFC_ENTRY);
    if ( pObject == NULL ) return status;
    
    ROUTE_STATUS sts = BcmRcm_addRipInterface(pObject);

    sts = BcmRcm_addRipInterface(pObject);
    if ( sts == ROUTE_STS_OK ) {
        BcmRcm_startRip();      // do it only if changed
        status = OBJ_STS_OK;
    } else 
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteRipIfc
// Description  : remove static RIP interface configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteRipIfc(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // RIP interface name
    char *name = va_arg(ap, char *);
    if ( name == NULL ) return status;
    
    ROUTE_STATUS sts = BcmRcm_removeRipInterface(name);
    
    if ( sts == ROUTE_STS_OK ) {
        BcmRcm_startRip();      // do it only if changed
        status = OBJ_STS_OK;
    } else 
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getRouteCfg
// Description  : retrieve static route configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getRouteCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    uint32 id = 0;
    
    // pointer to route node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // route ip address
    char *addr = va_arg(ap, char *);
    if ( addr == NULL ) return status;
    
    // route subnet mask
    char *mask = va_arg(ap, char *);
    if ( mask == NULL ) return status;
    
    // route gateway
    char *gtwy = va_arg(ap, char *);
    if ( gtwy == NULL ) return status;
    /*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
    uint32 entryControl = va_arg(ap, uint32);
    /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
    // route interface
    char *interface = va_arg(ap, char *);
    if ( interface == NULL ) return status;
    /*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
    *value = (unsigned long)BcmRcm_getRouteCfg((void *)*value, &id, addr, mask, gtwy, interface, &entryControl);
    /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setRouteCfg
// Description  : store static route configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setRouteCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    uint32 id = 0;   // need to have function to getAvailableRouteIndex()
    
    // route ip address
    char *addr = va_arg(ap, char *);
    if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, addr) != DB_OBJ_VALID_OK ) return status;   
    
    // route subnet mask
    char *mask = va_arg(ap, char *);
    if ( BcmDb_validateObj(DB_TYPE_SUBNET_MASK, mask) != DB_OBJ_VALID_OK ) return status;   
    
    // route gateway
    char *gtwy = va_arg(ap, char *);
    if ( gtwy == NULL ) return status;
    
    // route interface
    char *interface = va_arg(ap, char *);
    if ( interface == NULL ) return status;
    /*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
    uint32 entryControl = va_arg(ap, uint32);
    /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
    
    ROUTE_STATUS sts = ROUTE_STS_OK;
    char cmd[IFC_LARGE_LEN], str[IFC_LARGE_LEN];
    FILE* errFs = NULL;

    cmd[0] = '\0'; str[0] = '\0';
    sts = BcmRcm_addRouteCfg(id, addr, mask, gtwy, interface, entryControl);
    if ( sts == ROUTE_STS_OK ) {
        /*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
        if(entryControl == 1){
        /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
            sprintf(cmd, "route add -net %s netmask %s metric 1", addr, mask);
            if ( gtwy[0] != '\0' ) {
                strcat(cmd, " gw ");
                strcat(cmd, gtwy);
            }
            if ( interface[0] != '\0') {
                strcat(cmd, " dev ");
                strcat(cmd, interface);
            }
            strcat(cmd, " 2> /var/addrt");
            bcmSystemMute(cmd);
            errFs = fopen("/var/addrt", "r");
            if (errFs != NULL ) {
                fgets(str, IFC_LARGE_LEN, errFs);
                fclose(errFs);
                unlink("/var/addrt");
            }
            if ( str[0] != '\0' ) {
                BcmRcm_removeRouteCfg(addr, mask);
                status = OJB_STS_ERR_EXECUTION;
            } else
                status = OBJ_STS_OK;
        }
    } else
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteRouteCfg
// Description  : remove static route configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteRouteCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // route ip address
    char *addr = va_arg(ap, char *);
    if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, addr) != DB_OBJ_VALID_OK ) return status;   
    
    // route subnet mask
    char *mask = va_arg(ap, char *);
    if ( BcmDb_validateObj(DB_TYPE_SUBNET_MASK, addr) != DB_OBJ_VALID_OK ) return status;   
    
    ROUTE_STATUS sts = ROUTE_STS_OK;
    char cmd[IFC_LARGE_LEN], str[IFC_LARGE_LEN];
    FILE* errFs = NULL;

    cmd[0] = '\0'; str[0] = '\0';
    sts = BcmRcm_removeRouteCfg(addr, mask);
    if ( sts == ROUTE_STS_OK ) {
        sprintf(cmd, "route del -net %s netmask %s 2> /var/delrt", addr, mask);
        bcmSystemMute(cmd);
        errFs = fopen("/var/delrt", "r");
        if (errFs != NULL ) {
            fgets(str, IFC_LARGE_LEN, errFs);
            fclose(errFs);
            unlink("/var/delrt");
        }
        if ( str[0] != '\0')
            status = OJB_STS_ERR_EXECUTION;
        else
            status = OBJ_STS_OK;
    } else
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}
/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
//**************************************************************************
// Function Name: BcmObj_getDmzHost
// Description  : retrieve DMZ Host information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getDmzHost(va_list ap) {
    // DMZ Host ip address
    
    PSEC_DMZ_ENTRY pInfo = va_arg(ap, PSEC_DMZ_ENTRY);
    if ( pInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmScm_getDmzHost(pInfo);    
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setDmzHost
// Description  : store DMZ Host information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setDmzHost(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // DMZ Host ip address
    char *enable = va_arg(ap, char *);
    char *addr = va_arg(ap, char *);
    if ( BcmDb_validateObj(DB_TYPE_IP_ADDRESS, addr) != DB_OBJ_VALID_OK ) return status;   

    BcmScm_setDmzHost(enable, addr);    
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_deleteDmzHost
// Description  : remove DMZ Host information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteDmzHost(va_list ap) {
    BcmScm_setDmzHost("","");    
    
    return OBJ_STS_OK;
}
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
//**************************************************************************
// Function Name: BcmObj_getSrvCntrInfo
// Description  : retrieve service control list information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSrvCntrInfo(va_list ap) {
    PSEC_SRV_CNTR_INFO pInfo = va_arg(ap, PSEC_SRV_CNTR_INFO);
    if ( pInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;

    BcmScm_getSrvCntrInfo(pInfo);    
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setSrvCntrInfo
// Description  : store service control list information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSrvCntrInfo(va_list ap) {
    PSEC_SRV_CNTR_INFO pInfo = va_arg(ap, PSEC_SRV_CNTR_INFO);
    if ( pInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;

    BcmScm_setSrvCntrInfo(pInfo);    
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getFilterOut
// Description  : retrieve outgoing IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getFilterOut(va_list ap) {
    // pointer to outgoing IP filter node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // outgoing IP filter info
    PSEC_FLT_ENTRY pObject = va_arg(ap, PSEC_FLT_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getFilterOut((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setFilterOut
// Description  : store outgoing IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setFilterOut(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // outgoing IP filter info
    PSEC_FLT_ENTRY pObject = va_arg(ap, PSEC_FLT_ENTRY);
    if ( pObject == NULL ) return status;

    /*modified by z67625 增加outgoing ip过滤开关判断 start*/
    SEC_STATUS sts = BcmScm_addFilterOut(pObject, BcmDb_getSecEnbl(SCM_SEC_OUTFLT_ENABLE));
    /*modified by z67625 增加outgoing ip过滤开关判断 end*/
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteFilterOut
// Description  : remove outgoing IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteFilterOut(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // outgoing IP filter info
    char *fltName = va_arg(ap, char *);
    if ( fltName == NULL ) return status;

    /*modified by z67625 增加outgoing ip过滤开关判断 start*/
    SEC_STATUS sts = BcmScm_removeFilterOut(fltName, BcmDb_getSecEnbl(SCM_SEC_OUTFLT_ENABLE));
    /*modified by z67625 增加outgoing ip过滤开关判断 end*/
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getFilterIn
// Description  : retrieve incoming IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getFilterIn(va_list ap) {
    // pointer to incoming IP filter node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // incoming IP filter info
    PSEC_FLT_ENTRY pObject = va_arg(ap, PSEC_FLT_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getFilterIn((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setFilterIn
// Description  : store incoming IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setFilterIn(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // incoming IP filter info
    PSEC_FLT_ENTRY pObject = va_arg(ap, PSEC_FLT_ENTRY);
    if ( pObject == NULL ) return status;
    /*modified by z67625 增加incoming ip过滤开关判断 start*/
    SEC_STATUS sts = BcmScm_addFilterIn(pObject, BcmDb_getSecEnbl(SCM_SEC_INFLT_ENABLE));
    /*modified by z67625 增加incoming ip过滤开关判断 end*/
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteFilterIn
// Description  : remove incoming IP filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteFilterIn(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // incoming IP filter name
    char *fltName = va_arg(ap, char *);
    if ( fltName == NULL ) return status;
    /*modified by z67625 增加incoming ip过滤开关判断 start*/
    SEC_STATUS sts = BcmScm_removeFilterIn(fltName, BcmDb_getSecEnbl(SCM_SEC_INFLT_ENABLE));
    /*modified by z67625 增加incoming ip过滤开关判断 end*/
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getFilterMac
// Description  : retrieve MAC filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getFilterMac(va_list ap) {
    // pointer to MAC filter node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // incoming IP filter info
    PSEC_MAC_FLT_ENTRY pObject = va_arg(ap, PSEC_MAC_FLT_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getMacFilter((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setFilterMac
// Description  : store MAC filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setFilterMac(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // MAC filter info
    PSEC_MAC_FLT_ENTRY pObject = va_arg(ap, PSEC_MAC_FLT_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_addMacFilter(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteFilterMac
// Description  : remove MAC filter configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteFilterMac(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // MAC filter info
    PSEC_MAC_FLT_ENTRY pObject = va_arg(ap, PSEC_MAC_FLT_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_removeMacFilter(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getVirtualServer
// Description  : retrieve virtual server configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getVirtualServer(va_list ap) {
    // pointer to virtual server node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // virtual server info
    PSEC_VRT_SRV_ENTRY pObject = va_arg(ap, PSEC_VRT_SRV_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getVirtualServer((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setVirtualServer
// Description  : store virtual server configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setVirtualServer(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // virtual server info
    PSEC_VRT_SRV_ENTRY pObject = va_arg(ap, PSEC_VRT_SRV_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_addVirtualServer(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteVirtualServer
// Description  : remove virtual server configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteVirtualServer(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // virtual server info
    PSEC_VRT_SRV_ENTRY pObject = va_arg(ap, PSEC_VRT_SRV_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_removeVirtualServer(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getPortTrigger
// Description  : retrieve port triggering configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPortTrigger(va_list ap) {
    // pointer to port triggering node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // port triggering info
    PSEC_PRT_TRG_ENTRY pObject = va_arg(ap, PSEC_PRT_TRG_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getPortTrigger((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setPortTrigger
// Description  : store port triggering configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPortTrigger(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // port triggering info
    PSEC_PRT_TRG_ENTRY pObject = va_arg(ap, PSEC_PRT_TRG_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_addPortTrigger(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deletePortTrigger
// Description  : remove port triggering configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deletePortTrigger(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // port triggering info
    PSEC_PRT_TRG_ENTRY pObject = va_arg(ap, PSEC_PRT_TRG_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_removePortTrigger(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getAccessControl
// Description  : retrieve access control configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAccessControl(va_list ap) {
    // pointer to access control node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // access control info
    PSEC_ACC_CNTR_ENTRY pObject = va_arg(ap, PSEC_ACC_CNTR_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmScm_getAccessControl((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setAccessControl
// Description  : store access control configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAccessControl(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // access control info
    PSEC_ACC_CNTR_ENTRY pObject = va_arg(ap, PSEC_ACC_CNTR_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_addAccessControl(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteAccessControl
// Description  : remove access control configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteAccessControl(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // access control info
    PSEC_ACC_CNTR_ENTRY pObject = va_arg(ap, PSEC_ACC_CNTR_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmScm_removeAccessControl(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getAccessControlMode
// Description  : retrieve access control mode information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getAccessControlMode(va_list ap) {
    // access control mode
    int *mode = va_arg(ap, int *);
    if ( mode == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *mode = BcmScm_getAccessControlMode();
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setAccessControlMode
// Description  : store access control mode information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setAccessControlMode(va_list ap) {
    // access control mode
    int mode = va_arg(ap, int);
    
    BcmScm_setAccessControlMode(mode);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_getQosClass
// Description  : retrieve QoS class configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getQosClass(va_list ap) {
    // pointer to QoS class node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    // QoS class info
    PQOS_CLASS_ENTRY pObject = va_arg(ap, PQOS_CLASS_ENTRY);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmQos_getQosClass((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setQosClass
// Description  : store QoS class configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setQosClass(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // QoS class info
    PQOS_CLASS_ENTRY pObject = va_arg(ap, PQOS_CLASS_ENTRY);
    if ( pObject == NULL ) return status;
    
    SEC_STATUS sts = BcmQos_addQosClass(pObject);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteQosClass
// Description  : remove QoS class configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteQosClass(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // QoS class name
    char *clsName = va_arg(ap, char *);
    if ( clsName == NULL ) return status;
    
    SEC_STATUS sts = BcmQos_removeQosClass(clsName, QOS_TYPE_GENERAL);
    
    switch ( sts ) {
    case SEC_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case SEC_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

#ifdef ETH_CFG
//**************************************************************************
// Function Name: BcmObj_getEthInfo
// Description  : retrieve Ethernet configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getEthInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    PETH_CFG_INFO pEthInfo = va_arg(ap, PETH_CFG_INFO);    
    if ( pEthInfo == NULL ) return status;
    
    int ret = BcmDb_getEthInfo(pEthInfo);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setEthInfo
// Description  : store Ethernet configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setEthInfo(va_list ap) {
    PETH_CFG_INFO pEthInfo = va_arg(ap, PETH_CFG_INFO);    
    
    if ( pEthInfo == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BcmDb_setEthInfo(pEthInfo);

    return OBJ_STS_OK;
}
#endif   // ETH_CFG

#ifdef SUPPORT_PORT_MAP
//**************************************************************************
// Function Name: BcmObj_getPMapCfg
// Description  : retrieve PMap configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPMapCfg(va_list ap) {
    // pointer to PMap configuration node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    char *groupName = va_arg(ap, char *);
    if ( groupName == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *grpedIntf = va_arg(ap, char *);
    if ( grpedIntf == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmPMcm_getPMapCfg((void *)*value, groupName, grpedIntf);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setPMapCfg
// Description  : store PMap configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPMapCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *groupName = va_arg(ap, char *);
    char *grpedIntf = va_arg(ap, char *);
    PORT_MAP_DHCP_VENDORID_ENTRY *vid = va_arg(ap, PORT_MAP_DHCP_VENDORID_ENTRY *);
        
    PORT_MAP_STATUS sts = BcmPMcm_addPMapCfg(groupName, grpedIntf, vid);
    
    switch ( sts ) {
    case PORT_MAP_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case PORT_MAP_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    case PORT_MAP_STS_ERR_INVALID_OBJECT:
        status = OBJ_STS_ERR_INVALID_ARGUMENT;
        break;
    case PORT_MAP_STS_ERR_OBJECT_EXIST:
        status = OBJ_STS_ERR_OBJECT_EXIST;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deletePMapCfg
// Description  : remove PMap configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deletePMapCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // PMap group name
    char *groupName = va_arg(ap, char *);
    if ( groupName == NULL ) return status;
    
    PORT_MAP_STATUS sts = BcmPMcm_removePMapCfg(groupName);
    
    switch ( sts ) {
    case PORT_MAP_STS_ERR_GENERAL:
        status = OBJ_STS_ERR_GENERAL;
        break;
    case PORT_MAP_STS_ERR_MEMORY:
        status = OJB_STS_ERR_MEMORY;
        break;
    case PORT_MAP_STS_ERR_INVALID_OBJECT:
        status = OBJ_STS_ERR_INVALID_ARGUMENT;
        break;
    case PORT_MAP_STS_ERR_OBJECT_NOT_FOUND:
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;
        break;
    default:
        status = OBJ_STS_OK;
        break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getPMapIfcCfgSts
// Description  : retrieve PMap interface status configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPMapIfcCfgSts(va_list ap) {
    char *ifName = va_arg(ap, char *);
    if ( ifName == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    int status = va_arg(ap, int);
    
    BcmDb_getPMapIfcCfgSts(ifName, status);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setPMapIfcCfgSts
// Description  : store PMap interface status configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPMapIfcCfgSts(va_list ap) {
    char *ifName = va_arg(ap, char *);
    if ( ifName == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    int status = va_arg(ap, int);
    if ( status != PMAP_IFC_STS_DISABLE &&
         status != PMAP_IFC_STS_ENABLE )
        return OBJ_STS_ERR_INVALID_ARGUMENT;    
    
    BcmDb_setPMapIfcCfgSts(ifName, status);
    
    return OBJ_STS_OK;
}
#endif   // SUPPORT_PORT_MAP

#ifdef SUPPORT_SNMP
//**************************************************************************
// Function Name: BcmObj_getSnmpInfo
// Description  : retrieve SNMP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSnmpInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BCM_SNMP_STRUCT *pSnmp = va_arg(ap, BCM_SNMP_STRUCT *);    
    if ( pSnmp == NULL ) return status;
    
    int ret = BcmDb_getSnmpInfo(pSnmp);
    
    if ( ret == DB_GET_OK )
        status = OBJ_STS_OK;
    else
        status = OBJ_STS_ERR_OBJECT_NOT_FOUND;

    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSnmpInfo
// Description  : store SNMP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSnmpInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    BCM_SNMP_STRUCT *pSnmp = va_arg(ap, BCM_SNMP_STRUCT *);    
    if ( pSnmp == NULL ) return status;
    
    BcmDb_setSnmpInfo(pSnmp);

    return OBJ_STS_OK;
}
#endif

#ifdef SUPPORT_DDNSD
//**************************************************************************
// Function Name: ddnsStatusToObjStatus
// Description  : convert DDNS status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS ddnsStatusToObjStatus(DDNS_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case DDNS_INIT_FAILED:
        case DDNS_NOT_INIT:
        case DDNS_STORE_FAILED:
        case DDNS_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case DDNS_BAD_HOSTNAME:
        case DDNS_HOSTNAME_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getDDnsInfo
// Description  : retrieve DDNS configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getDDnsInfo(va_list ap) {
    // pointer to Ddns node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    char *hostname = va_arg(ap, char *);
    if ( hostname == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *username = va_arg(ap, char *);
    if ( username == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *password = va_arg(ap, char *);
    if ( password == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *iface = va_arg(ap, char *);
    if ( iface == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    unsigned int *service = va_arg(ap, unsigned int *);
    if ( service == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
// add by l66195 for VDF start
    unsigned int *bSelect = va_arg(ap, unsigned int *);
 //add by l66195 for VDF end
    if ( bSelect == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;    
    *value = (unsigned long)BcmDDns_getDDnsCfg((void *)*value, hostname,
        username, password, iface, (UINT16 *)service, (UINT16 *)bSelect);
                
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setDDnsInfo
// Description  : store DDNS specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setDDnsInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_DDNSD_INIT || parameter > OBJ_DDNSD_CLOSE )
        return status;

    char *hostname = NULL, *username = NULL;
    char *password = NULL, *iface = NULL;
    unsigned int service = 0;
    
    switch ( parameter ) {
    case OBJ_DDNSD_INIT:
        status = ddnsStatusToObjStatus(BcmDDns_init());
        break;
    case OBJ_DDNSD_RESTART:
        BcmDDns_serverRestart();
        status = OBJ_STS_OK;
        break;
    case OBJ_DDNSD_CLOSE:
        BcmDDns_close();
        status = OBJ_STS_OK;
        break;
    case OBJ_DDNSD_STORE:
        status = ddnsStatusToObjStatus(BcmDDns_Store());
        break;
    case OBJ_DDNSD_ADD:
        hostname = va_arg(ap, char *);
        if ( hostname == NULL ) return status;
        username = va_arg(ap, char *);
        if ( username == NULL ) return status;
        password = va_arg(ap, char *);
        if ( password == NULL ) return status;
        iface = va_arg(ap, char *);
        if ( iface == NULL ) return status;
        service = va_arg(ap, unsigned int);
   //     status = ddnsStatusToObjStatus(BcmDDns_add(hostname,
    //        username, password, iface, (UINT16)service));
        status = ddnsStatusToObjStatus(BcmDDns_add(hostname,
            username, password, iface, (UINT16)service, 0));
//modified by l66195 for VDF end
        break;
    }
                
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteDDnsInfo
// Description  : remove DDNS configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteDDnsInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *hostname = va_arg(ap, char *);    
    if ( hostname == NULL ) return status;
    
    status = ddnsStatusToObjStatus(BcmDDns_remove(hostname));
    
    return status;
}
#endif   // SUPPORT_DDNSD

#ifdef SUPPORT_SNTP
//BEGIN:modify by zhourongfei to config 3rd ntpserver
static char *SntpVarNameTbl[] = {
   OBJ_SNTP_SERVER_1,
   OBJ_SNTP_SERVER_2,
   OBJ_SNTP_SERVER_3
   OBJ_SNTP_TIMEZONE,
   OBJ_SNTP_ENABLED,
   OBJ_SNTP_USE_DST,
   OBJ_SNTP_OFFSET,
   OBJ_SNTP_DST_START,
   OBJ_SNTP_DST_END,
   OBJ_SNTP_REFRESH,
   NULL
};
//END:modify by zhourongfei to config 3rd ntpserver
//**************************************************************************
// Function Name: sntpStatusToObjStatus
// Description  : convert SNTP status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS sntpStatusToObjStatus(SNTP_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case SNTP_INIT_FAILED:
        case SNTP_NOT_INIT:
        case SNTP_STORE_FAILED:
        case SNTP_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case SNTP_BAD_RULE:
        case SNTP_RULE_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getSntpInfo
// Description  : retrieve SNTP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getSntpInfo(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *varName = va_arg(ap, char *);
    if ( varName == NULL ) return status;
    
    for ( ; SntpVarNameTbl[i] != NULL; i++ )
        if ( strcmp(varName, SntpVarNameTbl[i]) == 0 )
            break;
         
    if ( SntpVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);
    if ( varValue == NULL ) return status;
    
    status = sntpStatusToObjStatus(BcmSntp_GetVar(varName, varValue));
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setSntpInfo
// Description  : store SNTP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setSntpInfo(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_SNTP_INIT || parameter > OBJ_SNTP_CLOSE )
        return status;

    char *varName = NULL, *varValue = NULL;
    
    switch ( parameter ) {
    case OBJ_SNTP_INIT:
        status = sntpStatusToObjStatus(BcmSntp_init());
        break;
    case OBJ_SNTP_RESTART:
        BcmSntp_serverRestart();
        status = OBJ_STS_OK;
        break;
    case OBJ_SNTP_CLOSE:
        BcmSntp_close();
        status = OBJ_STS_OK;
        break;
    case OBJ_SNTP_STORE:
        status = sntpStatusToObjStatus(BcmSntp_Store());
        break;
    case OBJ_STNP_SET_VAR:
        varName = va_arg(ap, char *);
        if ( varName == NULL ) return status;
        for ( ; SntpVarNameTbl[i] != NULL; i++ )
            if ( strcmp(varName, SntpVarNameTbl[i]) == 0 )
                break;         
        if ( SntpVarNameTbl[i] == NULL ) return status;    
        varValue = va_arg(ap, char *);
        if ( varValue == NULL ) return status;
        status = sntpStatusToObjStatus(BcmSntp_SetVar(varName, varValue));
        break;
    }
                
    return status;
}
#endif   // SUPPORT_SNTP

#ifdef SUPPORT_IPP
static char *IppVarNameTbl[] = {
   OBJ_IPP_ENABLED,
   OBJ_IPP_REFRESH,
   OBJ_IPP_MAKE,
   OBJ_IPP_DEV,
   OBJ_IPP_NAME,
   NULL
};
//**************************************************************************
// Function Name: ippStatusToObjStatus
// Description  : convert IPP status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS ippStatusToObjStatus(IPP_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case IPP_INIT_FAILED:
        case IPP_NOT_INIT:
        case IPP_STORE_FAILED:
        case IPP_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case IPP_BAD_RULE:
        case IPP_RULE_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getIppInfo
// Description  : retrieve IPP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getIppInfo(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *varName = va_arg(ap, char *);
    if ( varName == NULL ) return status;
    
    for ( ; IppVarNameTbl[i] != NULL; i++ )
        if ( strcmp(varName, IppVarNameTbl[i]) == 0 )
            break;
         
    if ( IppVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);
    if ( varValue == NULL ) return status;
    
    status = ippStatusToObjStatus(BcmIpp_GetVar(varName, varValue));
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_setIppInfo
// Description  : store IPP configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setIppInfo(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_IPP_INIT || parameter > OBJ_IPP_CLOSE )
        return status;

    char *varName = NULL, *varValue = NULL;
    
    switch ( parameter ) {
    case OBJ_IPP_INIT:
        status = ippStatusToObjStatus(BcmIpp_init());
        break;
    case OBJ_IPP_RESTART:
        BcmIpp_serverRestart();
        status = OBJ_STS_OK;
        break;
    case OBJ_IPP_CLOSE:
        BcmIpp_close();
        status = OBJ_STS_OK;
        break;
    case OBJ_IPP_STORE:
        status = ippStatusToObjStatus(BcmIpp_Store());
        break;
    case OBJ_IPP_SET_VAR:
        varName = va_arg(ap, char *);
        if ( varName == NULL ) return status;
        for ( ; IppVarNameTbl[i] != NULL; i++ )
            if ( strcmp(varName, IppVarNameTbl[i]) == 0 )
                break;         
        if ( IppVarNameTbl[i] == NULL ) return status;    
        varValue = va_arg(ap, char *);
        if ( varValue == NULL ) return status;
        status = ippStatusToObjStatus(BcmIpp_SetVar(varName, varValue));
        break;
    }
                
    return status;
}
#endif   // SUPPORT_IPP

#ifdef SUPPORT_TOD
//**************************************************************************
// Function Name: todStatusToObjStatus
// Description  : convert TOD status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS todStatusToObjStatus(TOD_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case TOD_INIT_FAILED:
        case TOD_NOT_INIT:
        case TOD_STORE_FAILED:
        case TOD_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case TOD_BAD_RULE:
        case TOD_RULE_EXISTS:
        case TOD_RULE_NAME_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getTodInfo
// Description  : retrieve TOD configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getTodInfo(va_list ap) {
    // pointer to TOD node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    char *username = va_arg(ap, char *);
    if ( username == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *mac = va_arg(ap, char *);
    if ( mac == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    unsigned char *days = va_arg(ap, unsigned char *);
    if ( days == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    unsigned int *start_time = va_arg(ap, unsigned int *);
    if ( start_time == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    unsigned int *end_time = va_arg(ap, unsigned int *);
    if ( end_time == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    *value = (unsigned long)BcmTod_get((void *)*value, username, mac, days,
        (unsigned short int *)start_time, (unsigned short int *)end_time);
                
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setTodInfo
// Description  : store TOD specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setTodInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_TOD_INIT || parameter > OBJ_TOD_STORE )
        return status;

    char *username = NULL, *mac = NULL;
    unsigned int days = 0, start_time = 0, end_time = 0;
    
    switch ( parameter ) {
    case OBJ_TOD_INIT:
        status = todStatusToObjStatus(BcmTod_init());
        break;
    case OBJ_TOD_STORE:
        status = todStatusToObjStatus(BcmTod_Store());
        break;
    case OBJ_TOD_ADD:
        username = va_arg(ap, char *);
        if ( username == NULL ) return status;
        mac = va_arg(ap, char *);
        if ( mac == NULL ) return status;
        days = va_arg(ap, unsigned int);
        start_time = va_arg(ap, unsigned int);
        end_time = va_arg(ap, unsigned int);
        status = todStatusToObjStatus(BcmTod_add(username, mac, (unsigned char)days,
            (unsigned short int)start_time, (unsigned short int)end_time));
        break;
    }
                
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteTodInfo
// Description  : remove TOD configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteTodInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *username = va_arg(ap, char *);    
    if ( username == NULL ) return status;
    
    status = todStatusToObjStatus(BcmTod_remove(username));
    
    return status;
}
#endif   // SUPPORT_TOD

#ifdef SUPPORT_MAC  // add by l66195 for VDF
//**************************************************************************
// Function Name: todStatusToObjStatus
// Description  : convert TOD status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS macStatusToObjStatus(MAC_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case MAC_INIT_FAILED:
        case MAC_NOT_INIT:
        case MAC_STORE_FAILED:
        case MAC_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case MAC_BAD_RULE:
        case MAC_RULE_EXISTS:
        case MAC_RULE_NAME_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getTodInfo
// Description  : retrieve TOD configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getMacInfo(va_list ap) {
    // pointer to TOD node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    char *username = va_arg(ap, char *);
    if ( username == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *mac = va_arg(ap, char *);
    if ( mac == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
       
    *value = (unsigned long)BcmMac_get((void *)*value, username, mac);
                
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setTodInfo
// Description  : store TOD specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setMacInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_TOD_INIT || parameter > OBJ_TOD_STORE )
        return status;

    char *username = NULL, *mac = NULL;
      
    switch ( parameter ) {
    case OBJ_MAC_INIT:
        status = macStatusToObjStatus(BcmMac_init());
        break;
    case OBJ_MAC_STORE:
        status = macStatusToObjStatus(BcmMac_Store());
        break;
    case OBJ_MAC_ADD:
        username = va_arg(ap, char *);
        if ( username == NULL ) return status;
        mac = va_arg(ap, char *);
        if ( mac == NULL ) return status;
        status = macStatusToObjStatus(BcmMac_add(username, mac));
        break;
    }
                
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteTodInfo
// Description  : remove TOD configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteMacInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
 printf("BcmObj_deleteMacInfo si running......\n");   
    char *mac = va_arg(ap, char *);    
    if ( mac == NULL ) return status;
    
    status = macStatusToObjStatus(BcmMac_remove(mac));
    
    return status;
}
#endif   // SUPPORT_MAC

#ifdef SUPPORT_MACMATCHIP  
//**************************************************************************
// Function Name: macMatchStatusToObjStatus
// Description  : convert  status to object status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS macMatchStatusToObjStatus(MACMATCH_STATUS sts) {
    OBJ_STATUS status = OBJ_STS_OK;
    
    switch ( sts ) {
        case MACMATCH_INIT_FAILED:
        case MACMATCH_NOT_INIT:
        case MACMATCH_STORE_FAILED:
        case MACMATCH_ADD_FAIL:
            status = OJB_STS_ERR_EXECUTION;
            break;
        case MACMATCH_BAD_RULE:
        case MACMATCH_RULE_EXISTS:
        case MACMATCH_RULE_NAME_EXISTS:
            status = OBJ_STS_ERR_INVALID_ARGUMENT;
            break;
        default:
            status = OBJ_STS_OK;
            break;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getMacMatchInfo
// Description  : retrieve configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getMacMatchInfo(va_list ap) {
    // pointer to TOD node
    unsigned long *value = va_arg(ap, unsigned long *);
    
    char *username = va_arg(ap, char *);
    if ( username == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *mac = va_arg(ap, char *);
    if ( mac == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    char *ipaddress = va_arg(ap, char *);
    if ( mac == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
       
    *value = (unsigned long)BcmMacMatch_get((void *)*value, username, mac, ipaddress);
                
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setMacMatchInfo
// Description  : store  specific information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setMacMatchInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    int parameter = va_arg(ap, int);
    if ( parameter < OBJ_MACMATCH_INIT || parameter > OBJ_MACMATCH_STORE )
        return status;

    char *username = NULL, *mac = NULL, *ipaddress = NULL;
      
    switch ( parameter ) {
    case OBJ_MACMATCH_INIT:
        status = macMatchStatusToObjStatus(BcmMacMatch_init());
        break;
    case OBJ_MACMATCH_STORE:
        status = macMatchStatusToObjStatus(BcmMacMatch_Store());
        break;
    case OBJ_MACMATCH_ADD:
        username = va_arg(ap, char *);
        if ( username == NULL ) return status;
        mac = va_arg(ap, char *);
        if ( mac == NULL ) return status;
        ipaddress = va_arg(ap, char *);
        if ( ipaddress == NULL ) return status;
        status = macMatchStatusToObjStatus(BcmMacMatch_add(username, mac, ipaddress));
        break;
    }
                
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deleteMacMatchInfo
// Description  : remove configuration information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deleteMacMatchInfo(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
 printf("BcmObj_deleteMacInfo si running......\n");   
    char *mac = va_arg(ap, char *);    
    if ( mac == NULL ) return status;
    
    status = macMatchStatusToObjStatus(BcmMacMatch_remove(mac));
    
    return status;
}
#endif   

#ifdef VOXXXLOAD
static char *VoiceVarNameTbl[] = {
#ifdef IP_MGCP
   PSX_ATTR_VOICE_MGCP_CA_ADDR,
   PSX_ATTR_VOICE_MGCP_GW_NAME,
   PSX_ATTR_VOICE_MGCP_IF_NAME,
   PSX_ATTR_VOICE_MGCP_PREF_CODEC,
#elif defined(IP_SIP)
   PSX_ATTR_VOICE_SIP_PROXY_ADDR,
   PSX_ATTR_VOICE_SIP_PROXY_PORT,
   PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR,
   PSX_ATTR_VOICE_SIP_REGISTRAR_PORT,
   PSX_ATTR_VOICE_SIP_LOG_ADDR,
   PSX_ATTR_VOICE_SIP_LOG_PORT,
   PSX_ATTR_VOICE_SIP_EXT1,
   PSX_ATTR_VOICE_SIP_EXT2,
   PSX_ATTR_VOICE_SIP_EXT3,
   PSX_ATTR_VOICE_SIP_EXT4,
   PSX_ATTR_VOICE_SIP_PASSWORD1,
   PSX_ATTR_VOICE_SIP_PASSWORD2,
   PSX_ATTR_VOICE_SIP_PASSWORD3,
   PSX_ATTR_VOICE_SIP_PASSWORD4,
   PSX_ATTR_VOICE_SIP_IF_NAME,
   PSX_ATTR_VOICE_LOCALE_NAME,
   PSX_ATTR_VOICE_CODEC_NAME,
   PSX_ATTR_VOICE_PTIME, 
   PSX_ATTR_VOICE_SIP_OBPROXY_ADDR, 
   PSX_ATTR_VOICE_SIP_OBPROXY_PORT,
   PSX_ATTR_VOICE_SIP_DOMAIN_NAME,
   PSX_ATTR_VOICE_STORE,
#endif   // IP_MGCP
   NULL
};

//**************************************************************************
// Function Name: BcmObj_getVoice
// Description  : retrieve specific voice information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getVoice(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *varName = va_arg(ap, char *);    
    if ( varName == NULL ) return status;
    
    for ( ; VoiceVarNameTbl[i] != NULL; i++ )
      if ( strcmp(varName, VoiceVarNameTbl[i]) == 0 )
         break;
         
    if ( VoiceVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);    
    
    if ( varValue == NULL ) return status;
    
    BcmVoice_GetVar(varName, varValue);
    

    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setVoice
// Description  : store specific voice information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setVoice(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *varName = va_arg(ap, char *);    
    if ( varName == NULL ) return status;
    
    for ( ; VoiceVarNameTbl[i] != NULL; i++ )
      if ( strcmp(varName, VoiceVarNameTbl[i]) == 0 )
         break;
         
    if ( VoiceVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);    
    if ( varValue == NULL ) return status;

        
    if ( strcmp(varName,PSX_ATTR_VOICE_STORE) == 0 )  //Request to write data to flash
      BcmVoice_Store();
    else
      BcmVoice_SetVar(varName, varValue);
    
    return OBJ_STS_OK;
}
#endif

#ifdef WIRELESS
static char *WlVarNameTbl[] = {
   "wlSsidIdx",
   "wlSsid",
   "wlSsid_2",
   "wlCountry",
   "wlKeys",
   "wlPreambleType",
   "wlHide",
   "wlEnbl",
   "wlEnbl_2",
   "wlKeyIndex",
   "wlChannel",
   "wlchannelcheck",
   "wlFrgThrshld",
   "wlRtsThrshld",
   "wlDtmIntvl",
   "wlBcnIntvl",
   "wlFltMacMode",
   "wlAuthMode",
   "wlKeyBit",
   "wlRate",
   "wlPhyType",
   "wlBasicRate",
   "wlgMode",
   "wlProtection",
   "wlWpaPsk",
   "wlWpaGTKRekey",
   "wlRadiusServerIP",
   "wlRadiusPort",
   "wlRadiusKey",
   "wlWep",
   "wlWpa",
   "wlAuth",
   "wlMode",
   "wlLazyWds",
   "wlFrameBurst",
   "wlAPIsolation",
   "wlBand",
   "wlMCastRate",
   "wlAfterBurnerEn",
#if 0    /* 2008/01/28 Jiajun Weng : Remove them since 3.12L.01 dont' have them */
   "ezc_version",
   "ezc_enable",
   "is_default",
   "is_modified",
#endif 
   "wlInfra",
   "wlAntDiv",
   "wlWme", 
   "wlWmeNoAck",    
   /*start of enable or disable the access point radio by l129990,2009-10-9*/
   "wlRadioCtl",
   /*end of enable or disable the access point radio by l129990,2009-10-9*/
   "wlWmeApsd",    /* 2008/01/28 Jiajun Weng */
   "wlPreauth",   
   "wlNetReauth",
   "wlMaxAssoc",   
   "wlGlobalMaxAssoc",  /* 2008/01/28 Jiajun Weng */
   "wlDisableWme",     /* 2008/01/28 Jiajun Weng */
#ifdef SUPPORT_WLAN_PRNTCTL  
   "wlAutoSwOffdays", 
   "wlStarOfftTime",
   "wlEndOffTime", 
#endif
/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_SES) || defined (SUPPORT_ATHEROSWLAN)
   "wlSesEnable",
   "wlSesEvent",
   "wlSesStates",
   "wlSesSsid",
   "wlSesWpaPsk",
   "wlSesHide",
   "wlSesAuth", 
   "wlSesAuthMode",
   "wlSesWep",
   "wlSesWpa",
   "wlSesWdsMode",
   "wlSesClEnable",   
   "wlSesClEvent", 
   "wlWdsWsec",
#endif   
   "wlTxPwrPcnt",
   "wlRegMode",
   "wlDfsPreIsm",
   "wlDfsPostIsm",
   "wlTpcDb",
   "wlCsScanTimer",
#ifdef SUPPORT_TR69C
   "tr69cBeaconType",
   "tr69cBasicEncryptionModes",
   "tr69cBasicAuthenticationMode",
   "tr69cWPAEncryptionModes",
   "tr69cWPAAuthenticationMode",
   "tr69cIEEE11iEncryptionModes",
   "tr69cIEEE11iAuthenticationMode",
#endif   
#ifdef SUPPORT_MIMO
   "wlNBwCap",    /* 2008/01/28 Jiajun Weng : Modified from wlNBW to wlNBWCap */
   "wlNCtrlsb",
   "wlNBand",
   "wlNMcsidx",
   "wlProtection",
   "wlRifs", 
   "wlAmpdu", 
   "wlAmsdu",
   "wlNmode",
   "wlNReqd",
#endif   
   "wlan_ifname",
   NULL
};

//**************************************************************************
// Function Name: BcmObj_getWireless
// Description  : retrieve specific wireless information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getWireless(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
        
    char *varName = va_arg(ap, char *);    
    if ( varName == NULL ) return status;
    
    for ( ; WlVarNameTbl[i] != NULL; i++ )
      if ( strcmp(varName, WlVarNameTbl[i]) == 0 )
         break;
         
    if ( WlVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);    
    if ( varValue == NULL ) return status;
    
    BcmWl_GetVar(varName, varValue);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setWireless
// Description  : store specific wireless information.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setWireless(va_list ap) {
    int i = 0;
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    char *varName = va_arg(ap, char *);    
    if ( varName == NULL ) return status;
    
    for ( ; WlVarNameTbl[i] != NULL; i++ )
      if ( strcmp(varName, WlVarNameTbl[i]) == 0 )
         break;
         
    if ( WlVarNameTbl[i] == NULL ) return status;
    
    char *varValue = va_arg(ap, char *);    
    if ( varValue == NULL ) return status;
    
    BcmWl_SetVar(varName, varValue);
    
    return OBJ_STS_OK;
}
#endif

#ifdef PORT_MIRRORING
//**************************************************************************
// Function Name: BcmObj_getPortMirrorStatus
// Description  : retrieve Port Mirror Status.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPortMirrorStatus(va_list ap) {
    // pointer to Port Mirror Status

    DbgPortMirrorStatus *pObject = va_arg(ap, DbgPortMirrorStatus *);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    if ( BcmDcm_getPortMirrorStatus(pObject) != PORTMIRROR_STS_OK )
        pObject->count = 0 ;
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setPortMirrorStatus
// Description  : store Port Mirror Status
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPortMirrorStatus(va_list ap) {
    if ( BcmDcm_isInitialized() == FALSE ) return OBJ_STS_ERR_GENERAL;

    // pointer to Port Mirror Status
    DbgPortMirrorStatus *pObject = va_arg(ap, DbgPortMirrorStatus *);
    if ( pObject == NULL ) return OBJ_STS_ERR_INVALID_ARGUMENT;
    
    OBJ_STATUS status = OBJ_STS_OK;
    DbgPortMirrorStatus currentPortMirrorStatus;

    if ( BcmDcm_getPortMirrorStatus(&currentPortMirrorStatus) != PORTMIRROR_STS_OK ) 
        currentPortMirrorStatus.count = 0; 

    if ( currentPortMirrorStatus.count != pObject->count ) {
        PORTMIRROR_STATUS sts = BcmDcm_setPortMirrorStatus (pObject);
        if ( sts == PORTMIRROR_STS_OK ) {
            BcmDcm_effectPortMirrorCfg();      // do it only if changed
        }
        else 
            status = OJB_STS_ERR_MEMORY;
    }
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_getPortMirrorCfg.
// Description  : retrieve static Port Mirror Configuration.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_getPortMirrorCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // pointer to Port Mirror Cfg.
    unsigned long *value = va_arg(ap, unsigned long *);
    
    PortMirrorCfg* pObject = va_arg(ap, PortMirrorCfg*);
    if ( pObject == NULL ) return status;
    
    *value = (unsigned long)BcmDcm_getPortMirrorCfg((void *)*value, pObject);
    
    return OBJ_STS_OK;
}

//**************************************************************************
// Function Name: BcmObj_setPortMirrorCfg
// Description  : store static Port Mirror Configuration.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_setPortMirrorCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // pointer to Port Mirror Cfg.
    PortMirrorCfg* pObject = va_arg(ap, PortMirrorCfg*);
    if ( pObject == NULL ) return status;
    
    PORTMIRROR_STATUS sts ;

    sts = BcmDcm_addPortMirrorCfg(pObject);
    if ( sts == PORTMIRROR_STS_OK ) {
        BcmDcm_effectPortMirrorCfg();      // do it only if changed
        status = OBJ_STS_OK;
    } else 
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}

//**************************************************************************
// Function Name: BcmObj_deletePortMirrorCfg
// Description  : remove static Port Mirror Configuration.
// Returns      : status.
//**************************************************************************
OBJ_STATUS BcmObj_deletePortMirrorCfg(va_list ap) {
    OBJ_STATUS status = OBJ_STS_ERR_INVALID_ARGUMENT;
    
    // Port Mirror Cfg.
    int recNo = va_arg(ap, int);
    if ( recNo == 0 ) return status;
    
    PORTMIRROR_STATUS sts = BcmDcm_removePortMirrorCfg(recNo);
    
    if ( sts == PORTMIRROR_STS_OK ) {
        BcmDcm_effectPortMirrorCfg();      // do it only if changed
        status = OBJ_STS_OK;
    } else 
        status = OJB_STS_ERR_MEMORY;
    
    return status;
}
#endif /* End of Port Mirroring */
