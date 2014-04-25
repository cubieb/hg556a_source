/*****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
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
//  Filename:       psixml.cpp
//  Author:         yt
//  Creation Date:  5/11/04
//
******************************************************************************
//  Description:
//      Implement the PsiXml functions.
//      The funtion names are in respect to XML file.   The SETs routines
//          performs WRITE of PSI info to *.xml file.    The GETs routines
//          performs READ of *.xml file and convert data read to PSI structures.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "psiapi.h"
#include "psixml.h"
#include "ifcdefs.h"
#include "systemlog.h"
#include "snmpstruc.h"
#include "routedefs.h"
#include "secdefs.h"
#include "psidefs.h"
#include "psiifc.h"
#include "wldefs.h"
#include "atmprtcfg.h"
#include "todapi.h"
#include "ddnsapi.h"
#include "cgisec.h"
#include "adslcfgmngr.h"
#include "syslog.h"
#include "syscall.h"
#include "dbvalid.h"
#include "nanoxml.h"
#include "bcmcfm.h"
#include "dbapi.h"
#include "dbobject.h"
#include "bcmatmapi.h"
#include "sntpapi.h"
#ifdef SUPPORT_TR69C
#include "bcmtr69c.h"
#endif
#ifdef SUPPORT_IPP
#include "ippapi.h"
#endif /* SUPPORT_IPP */
#ifdef SUPPORT_PORT_MAP
#include "pmapdefs.h"
#endif // SUPPORT_PORT_MAP

#ifdef SUPPORT_MAC// add by l66195 for VDF
#include "macapi.h"
#endif

#ifdef SUPPORT_MACMATCHIP
#include "macmatchapi.h"
#endif

//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
#include "vlandefs.h"
/*end vlan bridge, s60000658, 20060627*/
//#endif
#ifdef VOXXXLOAD
#include <vodslapi.h>
#include <vodslsipcfg.h>
#include <vodslmgcpcfg.h>
#include <sipdef.h>
#endif
#ifdef SUPPORT_IPSEC
#include "vpnipsec.h"
#include "ipsecapi.h"
#endif
#ifdef SUPPORT_CERT
#include "certdefs.h"
#include "certapi.h"
#endif
#ifdef PORT_MIRRORING
#include "engdbgdefs.h"
#include "dbgcfgapi.h"
#endif
#ifdef WIRELESS
#include "wlapi.h"
#endif
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
#include "sipdef.h"
#endif
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

#ifdef SUPPORT_SAMBA
#include "sambadefs.h"
#endif
/* BEGIN: Added by y67514, 2009/12/10   PN:APNSecurity Enhancement*/
#include "board_api.h"
/* END:   Added by y67514, 2009/12/10 */

/* start 支持以太网口配置 by l68693 2009-02-17 该宏由Makefile定义*/  
//#define ETH_CFG
/* start 支持以太网口配置 by l68693 2009-02-17*/  

#define XML_SUPPORT_LEGACY_WL_CONFIG

//#define XML_DEBUG 
//#define XML_DEBUG1
//#define XML_DEBUG_xjx
#ifndef VDF_RESERVED
#define VDF_RESERVED
#endif
#define syslog(level, format,arg...)   printf(format, ## arg)
	
/*start of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
#define STATIC 1
#define DHCP   2
#define AUTOIP 3
/*end of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
static void *psiValue;
static XML_PARSE_STATE xmlState;
static const char *configBeginPtr = NULL;
#ifdef XML_SUPPORT_LEGACY_WL_CONFIG
PWIRELESS_MSSID_VAR wlMVars=NULL;
#endif

char app_name[64];
/* important limitation: when adding the appName or objectName in either direction, it is 
important not to have appName and objName having the same prefix.   The table entry search
only compare strings up to the prefix length, case not sensitive, to cover situation
such as APP_ID wan_vpi_vci, only wan is searched in the table.    Therefore, it is not
okay to have Lan as the appId, and objectName as lanSomethingSomething.    The search
entry would think lanSomethingSomething is an appId name. */
static const PSX_APP_ITEM apps[] = {
   { SYSTEM_APPID, xmlSetSysObjNode, xmlGetSysObjNode, xmlStartSysObjNode, xmlEndSysObjNode },
/* start of tr143 */
   { TR143_APPID, xmlSetTR143ObjNode, xmlGetTR143ObjNode, xmlStartTR143ObjNode, xmlEndTR143ObjNode },
/* end of tr143 */
   { LAN_APPID, xmlSetLanNode, xmlGetLanNode, xmlStartLanObj, xmlEndLanObj },
   { ATM_VCC_APPID, xmlSetAtmVccNode, xmlGetAtmVccObjNode, xmlStartAtmVccObjNode, xmlEndAtmVccObjNode },
   { ATM_TD_APPID, xmlSetAtmTdNode, xmlGetAtmTdObjNode, xmlStartAtmTdObjNode, xmlEndAtmTdObjNode },
   { ATM_CFG_APPID, xmlSetAtmCfgNode, xmlGetAtmCfgNode, xmlStartAtmCfgNode, xmlEndAtmCfgNode },
   { ADSL_APPID, xmlSetAdslObjNode, xmlGetAdslNode, xmlStartAdslObj, xmlEndAdslObj },
   { ROUTE_APPID, xmlSetRouteObjNode, xmlGetRouteObjNode, xmlStartRouteObjNode, xmlEndRouteObjNode },
//#ifdef VDF_PPPOU
//add by l66195 for pppousb start
   { E220_APPID, xmlSetE220Node, xmlGetE220Node, xmlStartE220Obj, xmlEndE220Obj},
   { PPP_PROFILE_APPID, xmlSetPppProfileObjNode, xmlGetPppProfileObjNode, xmlStartPppProfileObjNode, xmlEndPppProfileObjNode },
   { HSPA_SWITCH, xmlSetHspaNode, xmlGetHspaNode, xmlStartHspaObj, xmlEndHspaObj},
/*GLB:s53329,start add for DNS*/
   {PSI_APP_DNS, xmlSetPDnsObjNode, xmlGetDnsObjNode, xmlStartDnsObjNode, xmlEndDnsObjNode },
/*GLB:s53329,end add for DNS*/
//add by l66195 for pppousb end

//#endif
#if SUPPORT_PORT_MAP
   { PMAP_APPID, xmlSetPMapObjNode, xmlGetPMapObjNode, xmlStartPMapObjNode, xmlEndPMapObjNode },
#endif

#ifdef SUPPORT_VLAN_CONFIG
    { VLAN_APPID, xmlSetVlanObjNode, xmlGetVlanObjNode, xmlStartVlanObjNode, xmlEndVlanObjNode },
#else
#ifndef SUPPORT_ADSL
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
   { VLAN_APPID, xmlSetVlanObjNode, xmlGetVlanObjNode, xmlStartVlanObjNode, xmlEndVlanObjNode },
/*end vlan bridge, s60000658, 20060627*/
//#endif
#endif
#endif

   { SECURITY_APPID, xmlSetSecObjNode, xmlGetSecObjNode, xmlStartSecObjNode, xmlEndSecObjNode },
#ifdef SUPPORT_SNTP
   { SNTP_APP_ID, xmlSetSntpNode, xmlGetSntpNode, xmlStartSntpObj, xmlEndSntpObj },
#endif
#ifdef SUPPORT_TOD
   { TOD_APP_ID, xmlSetToDNode, xmlGetToDObjNode, xmlStartToDObjNode, xmlEndToDObjNode },
#endif
#ifdef SUPPORT_MAC //add by l66195 for VDF
   { MAC_APP_ID, xmlSetMacNode, xmlGetMacObjNode, xmlStartMacObjNode, xmlEndMacObjNode },
#endif
#ifdef SUPPORT_MACMATCHIP 
   { MACMATCH_APP_ID, xmlSetMacMatchNode, xmlGetMacMatchObjNode, xmlStartMacMatchObjNode, xmlEndMacMatchObjNode },
#endif
#ifdef SUPPORT_DDNSD
   { DDNS_APP_ID, xmlSetDdnsNode, xmlGetDdnsObjNode, xmlStartDdnsObjNode, xmlEndDdnsObjNode },
#endif
#ifdef WIRELESS
   { WIRELESS_APPID, xmlSetWirelessNode, xmlGetWirelessObjNode, xmlStartWirelessObjNode, xmlEndWirelessObjNode },
   { WIRELESS1_APPID, xmlSetWirelessNode, xmlGetWirelessObjNode, xmlStartWirelessObjNode, xmlEndWirelessObjNode },
#endif
   { WAN_APPID, xmlSetWanNode, xmlGetWanObjNode, xmlStartWanObjNode, xmlEndWanObjNode },
   { APPNAME_PPPSRV_PREFIX, xmlSetPppNode, xmlGetPppObjNode, xmlStartPppObjNode, xmlEndPppObjNode },
   { APPNAME_IPSRV_PREFIX, xmlSetDhcpClntNode, xmlGetDhcpClntNode, xmlStartDhcpClntNode, xmlEndDhcpClntNode },
#ifdef VOXXXLOAD
      { VOICE_APPID, xmlSetVoiceNode, xmlGetVoiceNode, xmlStartVoiceNode, xmlEndVoiceNode },
#endif
#ifdef SUPPORT_IPP
   { IPP_APP_ID, xmlSetIppNode, xmlGetIppNode, xmlStartIppObj, xmlEndIppObj },
#endif
#ifdef SUPPORT_IPSEC
   { IPSEC_APP_ID, xmlSetIPSecNode, xmlGetIPSecNode, xmlStartIPSecNode, xmlEndIPSecNode},
#endif
#ifdef SUPPORT_CERT
   { CERT_APP_ID, xmlSetCertNode, xmlGetCertNode, xmlStartCertNode, xmlEndCertNode},
#endif
#ifdef PORT_MIRRORING
   { ENGDBG_APPID, xmlSetPortMirrorObjNode, xmlGetPortMirrorObjNode, xmlStartPortMirrorObjNode, xmlEndPortMirrorObjNode},
#endif
   /* start of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
   { APPNAME_BRSRV_PREFIX, xmlSetBrNode, xmlGetBrNode, xmlStartBrNode, xmlEndBrNode },
   /* end of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
   /* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
   {APPNAME_AUTOUPGRADE, xmlSetAutoUpgradeNode, xmlGetAutoUpgradeNode, xmlStartAutoUpgradeNode, xmlEndAutoUpgradeNode },
   /* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
   #ifdef SUPPORT_DHCPOPTIONS
   {DHCPOption, xmlSetDhcpOptionNode, xmlGetDhcpOptionNode, xmlStartDhcpOptionNode, xmlEndDhcpOptionNode},
   #endif
   {VODAFONE_VIB_CFG_APPID, xmlSetVIBCfgInfoNode, xmlGetVIBCfgInfoNode, xmlStartVIBCfgInfoNode, xmlEndVIBCfgInfoNode},
   //{VODAFONE_VIB_INTERFACE_CFG, xmlSetVIBCfgInfoNode, xmlGetVIBCfgInfoNode, xmlStartVIBCfgInfoNode, xmlEndVIBCfgInfoNode},
#ifdef SUPPORT_SAMBA
   { SAMBA_APPID, xmlSetSambaObjNode, xmlGetSambaObjNode, xmlStartSambaObjNode, xmlEndSambaObjNode},
#endif
   /*start of 增加dhcp主机标识功能by l129990 */
 #ifdef SUPPORT_DHCPHOSTID                                                        
   {DHCPHOSTID, xmlSetDhcpHostNameNode, xmlGetDhcpHostNameNode, xmlStartDhcpHost12Node, xmlEndDhcpHostNameNode},
 #endif
 /*end of 增加dhcp主机标识功能by l129990 */
 /*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#ifdef SUPPORT_IGMPCONFIG
  {IGMPPROXY, xmlSetIgmpProxytNode, xmlGetIgmpProxyNode, xmlStartIgmpProxyNode, xmlEndIgmpProxyNode},
#endif
 /*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
   { "", NULL, NULL, NULL, NULL }
};

/* a list of table names */
static const char *tableNames[] = {
  TBLNAME_VRT_SRV,
  TBLNAME_OUT_FILTER,
  TBLNAME_IN_FILTER,
  TBLNAME_PRT_TRIGGER,
  TBLNAME_MAC_FILTER,
  TBLNAME_QOS_CLS,
  TBLNAME_ACC_CTRL,
  TBLNAME_ROUTE,
  TBLNAME_RIP_IFC,
#if SUPPORT_PORT_MAP
  TBLNAME_PMAP,
#endif
//#ifdef VDF_PPPOU
//add by l66195 for pppousb start
  TBLNAME_PPPPROFILE,
//add by l66195 for pppousb end
//#endif
/*GLB:s53329,start add for DNS*/
  PSI_OBJ_DNS_CFG,
/*GLB:s53329,end add for DNS*/
#ifdef SUPPORT_DDNSD
  TBLNAME_DDNS,
#endif
#ifdef WIRELESS
  TBLNAME_WL_MSSID_VARS,
  TBLNAME_WL_MAC_FILTER,
  TBLNAME_WDS_FILTER,
  TBLNAME_SCAN_WDS_FILTER,
#endif 
#ifdef SUPPORT_TOD
  TBLNAME_PARENT_CNTRL,
#endif
#ifdef SUPPORT_MAC  // add by l66195 for VDF
  TBLNAME_MAC,
#endif
#ifdef SUPPORT_MACMATCHIP  
  TBLNAME_MACMATCH,
#endif
#ifdef SUPPORT_IPSEC
  TBLNAME_IPSEC,
#endif
#ifdef SUPPORT_CERT
  TBLNAME_CERT_LOCAL,
  TBLNAME_CERT_CA,
#endif
#ifdef SUPPORT_SAMBA
  TBLNAME_SAMBA_ACCOUNT,
#endif
#ifdef PORT_MIRRORING
  TBLNAME_PORT_MIRROR_CFG,
#endif

#ifdef SUPPORT_VLAN_CONFIG
    TBLNAME_VLTB,
    TBLNAME_VLPORT,
#else
#ifndef SUPPORT_ADSL
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
  TBLNAME_VLTB,
  TBLNAME_VLPORT,
/*end vlan bridge, s60000658, 20060627*/
//#endif
#endif
#endif

/* start of voice 增加SIP配置项,表状结构 by z45221 zhangchen 2006年11月7日 */
#ifdef VOXXXLOAD
  TBLNAME_SIP_SRV,
  TBLNAME_SIP_BSC,
  TBLNAME_SIP_SPEEDDIAL_TB,
  TBLNAME_SIP_CALLFWD_TB,
#endif
/* end of voice 增加SIP配置项,表状结构 by z45221 zhangchen 2006年11月7日 */

 /*start add for vdf qos  by p44980 2008.01.08*/
 #ifdef SUPPORT_VDF_QOS
  TBLNAME_QOS_QUEUE,
  TBLNAME_QOS_SERVICE,
  #endif
 /*end add for vdf qos  by p44980 2008.01.08*/
#ifdef SUPPORT_POLICY
	TBLNAME_POLICY,
#endif
  TBLNAME_SEC_FW,
  TBLNAME_SEC_URLBLK,
  ""
};

static const PSX_OBJ_ITEM sysObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { PSX_OBJ_SYS_USER_NAME, xmlSetStrObjAttr, xmlGetSysUserNameAttr, xmlStartSysUserNamePasswordObj, xmlEndSysUserNamePasswordObj },
   { PSX_OBJ_SYS_PASSWORD, xmlSetStrObjAttr, xmlGetSysPasswordAttr, xmlStartSysUserNamePasswordObj, xmlEndSysUserNamePasswordObj },
#ifdef SUPPORT_ONE_USER
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
#else
   { PSX_OBJ_SPT_USER_NAME, xmlSetStrObjAttr, xmlGetSysUserNameAttr, xmlStartSysUserNamePasswordObj, xmlEndSysUserNamePasswordObj }, /* not shown until spt passwd changed */
   { PSX_OBJ_SPT_PASSWORD, xmlSetStrObjAttr, xmlGetSysPasswordAttr, xmlStartSysUserNamePasswordObj,xmlEndSysUserNamePasswordObj }, /* not shown until spt passwd changed */
   { PSX_OBJ_USR_USER_NAME, xmlSetStrObjAttr, xmlGetSysUserNameAttr, xmlStartSysUserNamePasswordObj,xmlEndSysUserNamePasswordObj }, /* not shown until user passwd changed */
   { PSX_OBJ_USR_PASSWORD, xmlSetStrObjAttr, xmlGetSysPasswordAttr, xmlStartSysUserNamePasswordObj,xmlEndSysUserNamePasswordObj }, /* not shown until user passwd changed */
#endif
   { PSX_OBJ_DEF_GATEWAY, xmlSetDftGwAttr, xmlGetDftGwAttr, xmlStartDftGwObj, xmlEndDftGwObj },
   { PSX_OBJ_DNS, xmlSetDnsAttr, xmlGetDnsAttr, xmlStartDnsObj, xmlEndDnsObj },
   { PSX_OBJ_SYSLOG, xmlSetSysLogAttr, xmlGetSyslogAttr, xmlStartSyslogObj, xmlEndSyslogObj },
#ifdef SUPPORT_SNMP
   { PSX_OBJ_SNMP, xmlSetSnmpAttr, xmlGetSnmpAttr, xmlStartSnmpObj, xmlEndSnmpObj },
#else
   { "skip", NULL, NULL, NULL, NULL },
#endif
   { PSX_OBJ_PROTOCOL, xmlSetOtherSysAttr, xmlGetOtherSysAttr, xmlStartOtherSysObj, xmlEndOtherSysObj },
#ifdef SUPPORT_TR69C
   { PSX_OBJ_TR69C, xmlSetTr69cAttr, xmlGetTr69cAttr, xmlStartTr69cObj, xmlEndTr69cObj },
#else
   { "skip", NULL, NULL, NULL, NULL },
#endif
   { PSX_OBJ_MGMT_LOCK, xmlSetMgmtLockAttr, xmlGetMgmtLockAttr, xmlStartMgmtLockObj, xmlEndMgmtLockObj },
    /*start at 增加Radius认证配置文件 by  s53329    20060714*/
#ifdef  SUPPORT_RADIUS
    {PSX_OBJ_RADIUS, xmlSetRadiusAttr, xmlGetRadiusAttr, xmlStartRadiusObj, xmlEndRadiusObj },
#else
   { "skip", NULL, NULL, NULL, NULL },
#endif
  /*end  at 增加Radius认证配置文件 by  s53329    20060714*/
/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
#ifdef SUPPORT_SIPROXD 
    {PSX_OBJ_SIPROXD_CFG, xmlSetSiproxdAttr, xmlGetSiproxdAttr, xmlStartSiproxdObj, xmlEndSiproxdObj },
#else
   { "skip", NULL, NULL, NULL, NULL },
#endif
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/
    { PSX_OBJ_FTP_INFO, xmlSetFtpinfoAttr, xmlGetFtpinfoAttr, xmlStartFtpinfoObj, xmlEndFtpinfoObj },
 #ifdef WEBCAM
    { PSX_OBJ_CAMERA_INFO, xmlSetCamerainfoAttr, xmlGetCamerainfoAttr, xmlStartCamerainfoObj, xmlEndCamerainfoObj },
 #else
    { "skip", NULL, NULL, NULL, NULL },
#endif
    {PSX_OBJ_HTTP_PORT, xmlSetHttpObjAttr, xmlGetHttpPortAttr, xmlStartSysHttpPortObj, xmlEndSysHttpPortObj},
#ifdef SUPPORT_GLB_ALG
    {PSX_OBJ_NATALG_CFG, xmlSetNatalgAttr, xmlGetNatalgAttr, xmlStartNatalgObj, xmlEndNatalgObj },
#else
    { "skip", NULL, NULL, NULL, NULL },
#endif /* SUPPORT_GLB_ALG */
/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
    {PSX_OBJ_AUTOSCAN_CFG, xmlSetAutoserchAttr, xmlGetAutoserchAttr, xmlStartAutoserchObj, xmlEndAutoserchObj },
/* END:   Added by y67514, 2008/10/27 */
/* BEGIN: Added by hKF20513, 2010/1/29   PN:增加Upnp Video控制*/
    {PSX_OBJ_UPNPV, xmlSetUpnpVideoAttr, xmlGetUpnpVideoAttr, xmlStartUpnpVideoObj, xmlEndUpnpVideoObj },
/* END:   Added by hKF20513, 2010/1/29 */
//BEGIN:added by zhourongfei to config web at 2011/05/31
	{ PSX_OBJ_WEB, xmlSetWebAttr, xmlGetWebAttr, xmlStartWebObj, xmlEndWebObj },
//END:added by zhourongfei to config web

    /*Begin:Added by luokunling l192527 for IMSI inform,2011/9/23*/
   { PSX_OBJ_IMSIINFORM,xmlSetIMSIAttr,xmlGetIMSIAttr,xmlStartIMSIObj,xmlEndIMSIObj},
   /*End:Added by luokunling l192527,2011/9/23*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
   {PSX_OBJ_HTTPINTERCEPTVISIBLE,xmlSetHttpInterceptAttr,xmlGetHttpInterceptAttr,xmlStartHttpInterceptAttr,xmlEndHttpInterceptAttr},
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
    { "", NULL, NULL, NULL, NULL }
};

static const PSX_OBJ_ITEM routeObjs[] = {
#ifdef SUPPORT_RIP
   { TBLNAME_RIP_IFC, NULL, xmlGetRipIfcAttr, NULL, xmlEndRipIfcObj },
   { OBJNAME_RIP_GLOBAL, xmlSetRipAttr, xmlGetRipAttr, xmlStartRipObj, xmlEndRipObj },
   { ENTRYNAME_RIP_IFC, xmlSetRipIfcAttr, xmlGetRipIfcEntryAttr, NULL, xmlEndRipIfcEntryObj },
#else
   { TBLNAME_RIP_IFC, NULL, NULL, NULL, NULL },
   { OBJNAME_RIP_GLOBAL, NULL, NULL, NULL, NULL },
   { ENTRYNAME_RIP_IFC, NULL, NULL, NULL, NULL },
#endif
   { TBLNAME_ROUTE, xmlSetRouteAttr, xmlGetRouteAttr, NULL, xmlEndRouteObj },
   { ENTRYNAME_ROUTE, xmlSetRouteAttr, xmlGetRouteEntryAttr, NULL, xmlEndRouteEntryObj },
   /* j00100803 Add Begin 2008-02-23 */
   #ifdef SUPPORT_POLICY
   {TBLNAME_POLICY, xmlSetRtPolicyAttr, xmlGetRtPolicyAttr, NULL , xmlEndRtPolicyObj},
   {ENTRYNAME_POLICY, xmlSetRtPolicyAttr, xmlGetRtPolicyEntryObj, xmlStartRtPolicyEntryObj, xmlEndRtPolicyEntryObj},
   #endif
   /* j00100803 Add End 2008-02-23 */
   { "", NULL, NULL, NULL, NULL }
};

#if SUPPORT_PORT_MAP
static const PSX_OBJ_ITEM pmapObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { TBLNAME_PMAP, xmlSetPMapAttr, xmlGetPMapAttr, NULL, xmlEndPMapObj },
   { ENTRYNAME_PMAP, xmlSetPMapAttr, xmlGetPMapEntryAttr, NULL, xmlEndPMapEntryObj },
   { OBJNAME_PMAP_IFC_CFG, xmlSetPMapIfcCfgAttr, xmlGetPMapIfcCfgAttr, xmlStartPMapIfcCfgObj, xmlEndPMapIfcCfgObj },
   { "", NULL, NULL, NULL, NULL }
};
#endif

#ifdef SUPPORT_VLAN_CONFIG
    static const PSX_OBJ_ITEM vlanObjs[] = {
       { "*", NULL, NULL, NULL, NULL },
       { TBLNAME_VLAN, xmlSetVlanAttr, xmlGetVlanAttr, xmlStartVlanAttr, xmlEndVlanObj },
       { TBLNAME_VLTB, xmlSetVltbAttr, xmlGetVltbAttr, NULL, xmlEndVltbObj },
       { ENTRYNAME_VLTB, xmlSetVltbAttr, xmlGetVlanEntryAttr, NULL, xmlEndVlanEntryObj },
       { TBLNAME_VLPORT, xmlSetVLPortAttr, xmlGetVLPortAttr, NULL, xmlEndVLPortObj },
       { ENTRYNAME_VLPORT, xmlSetVLPortAttr, xmlGetVLPortEntryAttr, NULL, xmlEndVLPortEntryObj },
       { "", NULL, NULL, NULL, NULL }
    };
#else
#ifndef SUPPORT_ADSL
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
static const PSX_OBJ_ITEM vlanObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { TBLNAME_VLAN, xmlSetVlanAttr, xmlGetVlanAttr, xmlStartVlanAttr, xmlEndVlanObj },

   { TBLNAME_VLTB, xmlSetVltbAttr, xmlGetVltbAttr, NULL, xmlEndVltbObj },
   { ENTRYNAME_VLTB, xmlSetVltbAttr, xmlGetVlanEntryAttr, NULL, xmlEndVlanEntryObj },
   { TBLNAME_VLPORT, xmlSetVLPortAttr, xmlGetVLPortAttr, NULL, xmlEndVLPortObj },
   { ENTRYNAME_VLPORT, xmlSetVLPortAttr, xmlGetVLPortEntryAttr, NULL, xmlEndVLPortEntryObj },
   { "", NULL, NULL, NULL, NULL }
};
/*end vlan bridge, s60000658, 20060627*/
//#endif
#endif
#endif

#ifdef WIRELESS
static const PSX_OBJ_ITEM wirelessObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { "vars", xmlSetWirelessVarsAttr, xmlGetWirelessVarsAttr, xmlStartWirelessVarsObj, xmlEndWirelessVarsObj },
   { TBLNAME_WL_MSSID_VARS, xmlSetWirelessMssidVarsAttr, xmlGetWirelessMssidVarsAttr, NULL, xmlEndWirelessMssidVarsObj },
   { ENTRYNAME_WL_MSSID_VARS, xmlSetWirelessMssidVarsAttr, xmlGetWirelessMssidVarsEntryAttr, NULL, xmlEndWirelessMssidVarsEntryObj },
   { TBLNAME_WL_MAC_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterAttr, NULL, xmlEndWirelessMacFilterObj },
   { ENTRYNAME_WL_MAC_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterEntryAttr, NULL, xmlEndWirelessMacFilterEntryObj },
   { TBLNAME_WDS_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterAttr, NULL, xmlEndWirelessMacFilterObj },
   { ENTRYNAME_WDS_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterEntryAttr, NULL, xmlEndWirelessMacFilterEntryObj },
   { TBLNAME_SCAN_WDS_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterAttr, NULL, xmlEndWirelessMacFilterObj },
   { ENTRYNAME_SCAN_WDS_FILTER, xmlSetWirelessMacFilterAttr, xmlGetWirelessMacFilterEntryAttr, NULL, xmlEndWirelessMacFilterEntryObj }
};
#endif /* #ifdef WIRELESS */

static const PSX_OBJ_ITEM atmCfgObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { "port", xmlSetAtmPortAttr, NULL, NULL, NULL },      /* obsolete */
   { "initCfg", xmlSetAtmInitAttr, xmlGetAtmInitAttr, xmlStartAtmInitObj, xmlEndAtmInitObj }, 
   { "tdNum", xmlSetAtmTdNumAttr, NULL, NULL, NULL },    /* obsolete */
   { "tdTable", xmlSetAtmTdTblAttr, NULL, NULL, NULL },  /* obsolete */
   { "interface", xmlSetAtmIfcAttr, NULL, NULL, NULL },  /* obsolete */
   { "vccNum", xmlSetAtmVccNumAttr, NULL, NULL, NULL },  /* obsolete */
   { "vccTable", xmlSetAtmVccTblAttr, NULL, NULL, NULL }, /* obsolete */
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifdef SUPPORT_VDSL
   { "trafficmode", xmlSetAtmTrafficModeAttr, xmlGetAtmTrafficModeAttr, xmlStartAtmTrafficModeObj, xmlEndAtmTrafficModeObj },
#else
   { "*", NULL, NULL, NULL, NULL },
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
   { "", NULL, NULL, NULL, NULL }
};

//#ifdef VDF_PPPOU
//add by l66195 for pppousb start
static const PSX_OBJ_ITEM pppProfileObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { TBLNAME_PPPPROFILE, xmlSetPppProfileAttr, xmlGetPppProfileAttr, NULL, xmlEndPppProfileObj },
   { ENTRYNAME_PPPPROFILE, xmlSetPppProfileAttr, xmlGetPppProfileEntryAttr, NULL, xmlEndPppProfileEntryObj },
   { "", NULL, NULL, NULL, NULL }
};
//add by l66195 for pppousb start


/*GLB:s53329,start add for DNS*/
static const PSX_OBJ_ITEM dnsObjs[] = {
    { "*", NULL, NULL, NULL, NULL },
    {PSI_OBJ_DNS_CFG,  xmlSetDnsEntryAttr, xmlGetDnsInfoAttr, NULL, xmlEndDnsInfoObj},
    {PSI_OBJ_DNS_ENTRY, xmlSetDnsEntryAttr, xmlGetDnsEntryAttr, NULL, xmlDnsEntryObj },
    { "*", NULL, NULL, NULL, NULL }
};
/*GLB:s53329,end add for DNS*/

static const PSX_OBJ_ITEM secObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { TBLNAME_VRT_SRV, xmlSetVirtualSrvAttr, xmlGetVrtSrvAttr, NULL, xmlEndVrtSrvObj }, 
   { ENTRYNAME_VRT_SRV, xmlSetVirtualSrvAttr, xmlGetVrtSrvEntryAttr, NULL, xmlEndVrtSrvEntryObj } ,
   { TBLNAME_OUT_FILTER, xmlSetIpFilterAttr, xmlGetIpFilterAttr, NULL, xmlEndIpFilterObj },    
   { ENTRYNAME_OUT_FILTER, xmlSetIpFilterAttr, xmlGetIpFilterEntryAttr, NULL, xmlEndIpFilterEntryObj },  
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   { OBJNAME_DMZ_HOST, xmlSetDmzHostAttr , xmlGetDmzHostAttr, xmlStartDmzHostObj, xmlEndDmzHostObj }, 
   { TBLNAME_PRT_TRIGGER, xmlSetPrtTriggerAttr, xmlGetPrtTriggerAttr, NULL, xmlEndPrtTriggerObj },
   { ENTRYNAME_PRT_TRIGGER, xmlSetPrtTriggerAttr, xmlGetPrtTriggerEntryAttr, NULL, xmlEndPrtTriggerEntryObj }, 
   { TBLNAME_MAC_FILTER, xmlSetMacFilterAttr, xmlGetMacFilterAttr, NULL, xmlEndMacFilterObj },
   { ENTRYNAME_MAC_FILTER, xmlSetMacFilterAttr, xmlGetMacFilterEntryAttr, NULL, xmlEndMacFilterEntryObj },
   { TBLNAME_ACC_CTRL, xmlSetAccessControlAttr, xmlGetAccCntrlAttr, NULL, xmlEndAccCntrlObj },
   { ENTRYNAME_ACC_CTRL, xmlSetAccessControlAttr, xmlGetAccCntrlEntryAttr, NULL, xmlEndAccCntrlEntryObj },
   { OBJNAME_ACC_CTRL_MODE, xmlSetAccessControlModeAttr, xmlGetAccCntrlModeAttr, xmlStartAccCntrlModeObj, xmlEndAccCntrlModeObj },
   /*start modifying for vdf qos by p44980 2008.01.08*/
   #ifdef SUPPORT_VDF_QOS
   { OBJNAME_QOS_CFG, xmlSetQosCfgAttr , xmlGetQosCfgAttr, xmlStartQosCfgObj, xmlEndQosCfgObj }, 
   { TBLNAME_QOS_QUEUE, xmlSetQosQueueAttr, xmlGetQosQueueAttr, NULL, xmlEndQosQueueObj },
   { ENTRYNAME_QOS_QUEUE, xmlSetQosQueueAttr, xmlGetQosQueueEntryAttr, xmlStartQosQueueEntryObj, xmlEndQosQueueEntryObj },
   { TBLNAME_QOS_SERVICE, xmlSetQosServiceAttr, xmlGetQosServiceAttr, NULL, xmlEndQosServiceObj},
   { ENTRYNAME_QOS_SERVICE, xmlSetQosServiceAttr, xmlGetQosServiceEntryAttr, xmlStartQosServiceEntryObj, xmlEndQosServiceEntryObj},
   #else
   	{ "*", NULL, NULL, NULL, NULL },
   	{ "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   { "*", NULL, NULL, NULL, NULL },
   #endif
  /*end modifying for vdf qos by p44980 2008.01.08*/ 
   { TBLNAME_QOS_CLS, xmlSetQosAttr, xmlGetQosClsAttr, NULL, xmlEndQosClsObj },
   { ENTRYNAME_QOS_CLS, xmlSetQosAttr, xmlGetQosClsEntryAttr, xmlStartQosClsEntryObj, xmlEndQosClsEntryObj },
   { TBLNAME_IN_FILTER, xmlSetIpFilterAttr, xmlGetIpFilterAttr, NULL, xmlEndIpFilterObj },
   { ENTRYNAME_IN_FILTER, xmlSetIpFilterAttr, xmlGetIpFilterEntryAttr, NULL, xmlEndIpFilterEntryObj },
   { OBJNAME_SRV_CTRL_LIST, xmlSetServiceCtrlAttr, xmlGetServiceCtrlAttr, xmlStartServiceCtrlObj, xmlEndServiceCtrlObj },
    /* start of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#if SUPPORT_KPN_QOS // macro QoS for KPN
   { OBJNAME_WRED_INFO, xmlSetWredInfoAttr, xmlGetWredInfoAttr, xmlStartWredInfoObj, xmlEndWredInfoObj },
#else
   { "*", NULL, NULL, NULL, NULL },
#endif
#else
   { "*", NULL, NULL, NULL, NULL },
#endif
    /* end of protocol 加入对 QoS  for KPN的支持 by z45221 zhangchen 2006年6月27日 */
   {OBJNAME_SEC_FW, xmlSetSecFwAttr, xmlGetSecFwAttr, xmlStartSecFwObj, xmlEndSecFwObj },
   {TBLNAME_SEC_FW, xmlSetSecFwTbAttr, xmlGetSecFwTbAttr, NULL, xmlEndSecFwTbObj },
   {ENTRYNAME_SEC_FW, xmlSetSecFwTbAttr, xmlGetSecFwEntryAttr, xmlStartSecFwEntryObj, xmlEndSecFwEntryObj },
   {OBJNAME_SEC_DOS, xmlSetSecDosAttr, xmlGetSecDosAttr, xmlStartSecDosObj, xmlEndSecDosObj },
   {TBLNAME_SEC_URLBLK, xmlSetSecUrlBlkTbAttr, xmlGetSecUrlBlkTbAttr, NULL, xmlEndSecUrlBlkTbObj },
   {ENTRYNAME_SEC_URLBLK, xmlSetSecUrlBlkTbAttr, xmlGetSecUrlBlkEntryAttr, xmlStartSecUrlBlkEntryObj, xmlEndSecUrlBlkEntryObj },
   /*add by z67625 增加支持过滤开关设置 start*/
   {OBJNAME_SEC_ENABLE,xmlSetSecEnblAttr, xmlGetSecEnblAttr, xmlStartSecEnblObj, xmlEndSecEnblObj},
   /*add by z67625 增加支持过滤开关设置 end*/
   { "", NULL, NULL, NULL, NULL }
};

#ifdef SUPPORT_TOD
static const PSX_OBJ_ITEM ToDObjs[] = {
   { ENTRYNAME_PARENT_CNTRL, xmlSetToDCntrlAttr, xmlGetToDEntryAttr, NULL, xmlEndToDEntryObj }, 
   { TBLNAME_PARENT_CNTRL, xmlSetToDCntrlAttr, xmlGetToDAttr, NULL, xmlEndToDObj }, 
};
#endif

#ifdef SUPPORT_MAC // add by l66195 for VDF
static const PSX_OBJ_ITEM MacObjs[] = {
   { ENTRYNAME_MAC, xmlSetMacAttr, xmlGetMacEntryAttr, NULL, xmlEndMacEntryObj }, 
   { TBLNAME_MAC, xmlSetMacAttr, xmlGetMacAttr, NULL, xmlEndMacObj }, 
};
#endif

#ifdef SUPPORT_MACMATCHIP 
static const PSX_OBJ_ITEM MacMatchObjs[] = {
   { ENTRYNAME_MACMATCH, xmlSetMacMatchAttr, xmlGetMacMatchEntryAttr, NULL, xmlEndMacMatchEntryObj }, 
   { TBLNAME_MACMATCH, xmlSetMacMatchAttr, xmlGetMacMatchAttr, NULL, xmlEndMacMatchObj }, 
};
#endif

#ifdef SUPPORT_DDNSD
static const PSX_OBJ_ITEM DDnsObjs[] = {
   { "ddns_error", NULL, NULL, NULL, NULL },
   { TBLNAME_DDNS, xmlSetDdnsAttr, xmlGetDdnsAttr, NULL, xmlEndDdnsObj }, 
   { ENTRYNAME_DDNS, xmlSetDdnsAttr, xmlGetDdnsEntryAttr, NULL, xmlEndDdnsEntryObj }, 
};
#endif 

#ifdef SUPPORT_SAMBA
static const PSX_OBJ_ITEM sambaObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { OBJNAME_SAMBA_STATUS, xmlSetSambaAttr, xmlGetSambaStatusAttr, xmlStartSambaStatusObj, xmlEndSambaStatusObj},
   { TBLNAME_SAMBA_ACCOUNT, xmlSetSambaAttr, xmlGetSambaAccntAttr, NULL, xmlEndSambaAccntObj },
   { ENTRYNAME_SAMBA_ACCOUNT, xmlSetSambaAttr, xmlGetSambaAccntEntryAttr, xmlStartSambaAccntEntryObj, xmlEndSambaAccntEntryObj},
   { "", NULL, NULL, NULL, NULL },
};
#endif

#ifdef VOXXXLOAD
static const PSX_OBJ_ITEM voiceObjs[] = {
   { "*", NULL, NULL, NULL, NULL },
   { PSX_OBJ_MGCP_CFG, xmlSetMgcpAttr, xmlGetMgcpAttr, xmlStartMgcpObjNode, xmlEndMgcpObjNode },
   { PSX_OBJ_SIP_CFG, xmlSetSipAttr, xmlGetSipAttr, xmlStartSipObjNode, xmlEndSipObjNode },
   /* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
   { "*", NULL, NULL, NULL, NULL }, // VOICE_LESINFO_ID
   { TBLNAME_SIP_SRV,   xmlSetSipSrvAttr, xmlGetSipSrvAttr,     NULL,                   xmlEndSipSrvObjNode },
   { ENTRYNAME_SIP_SRV, xmlSetSipSrvAttr, xmlGetSipSrvEntryAttr,NULL,                   xmlEndSipSrvObjEntryNode },
   { TBLNAME_SIP_BSC,   xmlSetSipBscAttr, xmlGetSipBscAttr,     NULL,                   xmlEndSipBscObjNode },
   { ENTRYNAME_SIP_BSC, xmlSetSipBscAttr, xmlGetSipBscEntryAttr,NULL,                   xmlEndSipBscObjEntryNode },
   { OBJNAME_SIP_ADV,   xmlSetSipAdvAttr, xmlGetSipAdvAttr,     xmlStartSipAdvObjNode,  xmlEndSipAdvObjNode },
   { OBJNAME_SIP_QOS,   xmlSetSipQosAttr, xmlGetSipQosAttr,     xmlStartSipQosObjNode,  xmlEndSipQosObjNode },
   {OBJNAME_SIP_PHONECFG, xmlSetSipPhoneCfgAttr, xmlGetSipPhoneCfgAttr, xmlStartSipPhoneCfgObjNode, xmlEndSipPhoneCfgObjNode},
   {TBLNAME_SIP_SPEEDDIAL_TB, xmlSetSipSpeedDialAttr, xmlGetSipSpeedDialTbAttr, NULL, xmlEndSipSpeedDialTbNode},
   {ENTRYNAME_SIP_SPEEDDIAL, xmlSetSipSpeedDialAttr, xmlGetSipSpeedDialEntryAttr, NULL, xmlEndSipSpeedDialEntryNode},
   {TBLNAME_SIP_CALLFWD_TB, xmlSetSipCallFWDAttr, xmlGetSipCallFWDTbAttr, NULL, xmlEndSipCallFWDTbNode},
   {ENTRYNAME_SIP_CALLFWD, xmlSetSipCallFWDAttr, xmlGetSipCallFWDEntryAttr, NULL, xmlEndSipCallFWDEntryNode},
   /* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
   /* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
   { OBJNAME_SIP_PROV,xmlSetSipProvAttr, xmlGetSipProvAttr,xmlStartSipProvObjNode,xmlEndSipProvObjNode }
   /* END:   Added by y67514, 2009/11/16 */
};
#endif /* VOXXXLOAD */

/*add by z67625 for lint start*/
#if defined(__cplusplus)
extern "C" {
#endif
/*add by z67625 for lint end*/

PSX_STATUS xmlMemProblem()
{
  xmlState.errorFlag = PSX_STS_ERR_FATAL;
  syslog(LOG_ERR,"xmlMemProblem(): error allocating memory for configuration.\n");
  printf("Configuration Error: xmlMemProblem(): error allocating memory for configuration!\n");
  return PSX_STS_ERR_FATAL;
}
/*Begin:Added by luokunling l192527 for IMSI inform,2011/9/23*/
void xmlSetIMSIAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{   
      PIMSI_Notification imsi_notification = NULL;

     if(NULL == name || NULL == value )
     	{
     	   printf("name and value is NULL!\n");
     	   return ;    
     	}

     imsi_notification = (PIMSI_Notification)value;
	 
     //printf("the value is %d\n",imsi_notification->Imsi_Notification_Enable);
     fprintf(file,"<%s Notificationenable = \"%d\" />\n",name,imsi_notification->Imsi_Notification_Enable);	 
}
PSX_STATUS xmlGetIMSIAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    PIMSI_Notification Imsi_notification = (PIMSI_Notification)psiValue; // 获取psiValue的地址

   if((attrValue == NULL) || (attrName == NULL))
   {
	return PSX_STS_ERR_FATAL;
   }

    if (NULL == psiValue)
    {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIgmpProxyNode(): psiValue is NULL\n");
	 return PSX_STS_ERR_FATAL;
    }
	 
    if(0 == strncmp(attrName,"Notificationenable",strlen("Notificationenable")))
    {
       if(xmlState.verify)
       {
          status = BcmDb_validateNumber(attrValue);
       }
	 if(status != DB_OBJ_VALID_OK)
	 {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid IMSInotification %s\n",attrValue);
	 }
	 else
	 { 
	   /* printf("xmlGetIMSIAttr(char *attrName, char* attrValue)  %s\n",attrValue);
	    printf("###################################\n");
           printf("###################################\n");*/
	    Imsi_notification->Imsi_Notification_Enable = atoi(attrValue);
	 }
    }

    return PSX_STS_OK;
}
PSX_STATUS xmlStartIMSIObj(char *appName, char *objName)
{
      PIMSI_Notification Imsi_notification = NULL;

     if ((NULL == appName) || (NULL == objName))
    {
        return PSX_STS_ERR_GENERAL;
    }
   
    psiValue = (void*)malloc(sizeof(IMSI_Notification)); // 给psiValue分配内存;
    if (psiValue == NULL) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
     memset(psiValue, 0, sizeof(IMSI_Notification));
     /*printf("in xmlStartIMSIObj(char *appName, char *objName)\n");
     printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
     printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");*/
	 
     Imsi_notification = (PIMSI_Notification)psiValue;                      // 准备给psiValue赋值;
     Imsi_notification->Imsi_Notification_Enable =atoi(BcmDb_getDefaultValue("Notificationenable"));  // 下面是获取缺省值，没有就注释掉

    return PSX_STS_OK;
}
PSX_STATUS xmlEndIMSIObj(char *appName, char *objName)
{
   PSI_STATUS status = PSI_STS_OK;
   PIMSI_Notification Imsi_notification ;
   Imsi_notification= (PIMSI_Notification)psiValue;


   printf("xmlEndIMSIObj(calling cfm to get appName %s, objName %s)\n",appName,objName);

     if (!xmlState.verify) 
    {
    /* printf("back to psi OK!\n");
     printf("in xmlEndIMSIObj(char *appName, char *objName)\n");
     printf("+++++++++++++++++++++++++++++++++\n");
     printf("+++++++++++++++++++++++++++++++++\n");*/
     status = BcmDb_setIMSINotification(Imsi_notification);
    }

     if(status != PSI_STS_OK)
     {
          printf("BcmDb_setIMSINotification() failed !\n");     
     }
     
     free(psiValue);
     psiValue = NULL;
	 
    return PSX_STS_OK;
 
}
/*End:Added by luokunling l192527,2011/9/23*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
void xmlSetHttpInterceptAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
	PHTTP_InterceptVisible pHttp_InterceptVisible = NULL;

	if(NULL == name || NULL == value )
	{
	     printf("name and value is NULL!\n");
	     return ;    
	}

	pHttp_InterceptVisible = (PHTTP_InterceptVisible)value;
		 
	//printf("the value is %d\n",pHttp_InterceptVisible->iHttpInterceptVisible);
	fprintf(file,"<%s HTTPInterceptVisible = \"%d\" />\n",name,pHttp_InterceptVisible->iHttpInterceptVisible);	
}

PSX_STATUS xmlGetHttpInterceptAttr(char *attrName, char* attrValue)
{
	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
	PHTTP_InterceptVisible pHttp_InterceptVisible = (PHTTP_InterceptVisible)psiValue; // 获取psiValue的地址

	if((attrName == NULL) || (attrValue == NULL))
	{
           return PSX_STS_ERR_FATAL;
	}

	if (NULL == psiValue)
	{
           xmlState.errorFlag = PSX_STS_ERR_FATAL;
           syslog(LOG_ERR,"xmlGetIgmpProxyNode(): psiValue is NULL\n");
           return PSX_STS_ERR_FATAL;
	}
	 
	if(0 == strncmp(attrName,"HTTPInterceptVisible",strlen("HTTPInterceptVisible")))
	{
           if(xmlState.verify)
           {
               status = BcmDb_validateNumber(attrValue);
           }
           if(status != DB_OBJ_VALID_OK)
           {
               xmlState.errorFlag = PSX_STS_ERR_FATAL;
               syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid HTTPInterceptVisible %s\n",attrValue);
           }
           else
           { 
               printf("xmlGetIMSIAttr(char *attrName, char* attrValue)  %s\n",attrValue);
               //printf("###################################\n");
               //printf("###################################\n");
               pHttp_InterceptVisible->iHttpInterceptVisible = atoi(attrValue);
           }
	}

	return PSX_STS_OK;
}

PSX_STATUS xmlStartHttpInterceptAttr(char *appName, char *objName)
{
    PHTTP_InterceptVisible pHttp_InterceptVisible = NULL;

     if ((NULL == appName) || (NULL == objName))
    {
        return PSX_STS_ERR_GENERAL;
    }
   
    psiValue = (void*)malloc(sizeof(HTTP_InterceptVisible)); // 给psiValue分配内存;
    if (psiValue == NULL) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
     memset(psiValue, 0, sizeof(HTTP_InterceptVisible));
     printf("in xmlStartIMSIObj(char *appName, char *objName)\n");
     //printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
     //printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	 
     pHttp_InterceptVisible = (PHTTP_InterceptVisible)psiValue;                      // 准备给psiValue赋值;
     pHttp_InterceptVisible->iHttpInterceptVisible = atoi(BcmDb_getDefaultValue("HTTPInterceptVisible"));  // 下面是获取缺省值，没有就注释掉
}

PSX_STATUS xmlEndHttpInterceptAttr(char *appName, char *objName)
{
    PSI_STATUS status = PSI_STS_OK;
    PHTTP_InterceptVisible pHttp_InterceptVisible = NULL;
    pHttp_InterceptVisible= (PHTTP_InterceptVisible)psiValue;


    printf("xmlEndIMSIObj(calling cfm to get appName %s, objName %s)\n",appName,objName);

     if (!xmlState.verify) 
    {
    printf("back to psi OK!\n");
     printf("in xmlEndIMSIObj(char *appName, char *objName)\n");
     //printf("+++++++++++++++++++++++++++++++++\n");
     //printf("+++++++++++++++++++++++++++++++++\n");
     status = BcmDb_setHTTPInterceptVisible(pHttp_InterceptVisible);
    }

     if(status != PSI_STS_OK)
     {
          printf("BcmDb_setIMSINotification() failed !\n");     
     }
     
     free(psiValue);
     psiValue = NULL;
	 
    return PSX_STS_OK;
}
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
void xmlConvertToLowerCase(char *str)
{
  int len = strlen(str);
  int i;

  for (i = 0; i < len; i++)
    str[i] = tolower(str[i]);
}

PSX_STATUS xmlSetObjNode(FILE* file, const char *appName, const UINT16 id,
   const PSI_VALUE value, const UINT16 length) {
   int i;
   PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

   for ( i = 0; apps[i].appName[0] != '\0'; i++ ) {
      /* there are a few special cases where the appId only needs to match with
	 the specified prefix (wan, ipsrv, pppsrv) */
      if (strncasecmp(apps[i].appName, appName, strlen(apps[i].appName)) == 0) {
        sts = PSX_STS_ERR_GENERAL;
 	 if (*(apps[i].setObjFnc) != NULL) {
	    sts = (*(apps[i].setObjFnc))(file, appName, id, value, length);
            break;
	 }
      } /* found */
   } /* for */

   if (sts == PSX_STS_ERR_FIND_OBJ) {
     /* to be removed later */
#ifdef XML_DEBUG
     fprintf(file,"<appName %s not implemented yet/>\n",appName);
#endif
   }

   return sts;
}

PSX_STATUS xmlSetSysObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length) 
{
   UINT16 size = sizeof(sysObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(sysObjs[id].setAttrFnc) != NULL)
     (*(sysObjs[id].setAttrFnc))(file, sysObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
} /* xmlSetSysObjNode */

void xmlSetStrObjAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                      const UINT16 length)
{
   char str[IFC_LARGE_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(str, (char *)value, length);
   str[length] = '\0';

#ifdef SUPPORT_ONE_USER
   if ( strcmp(name,PSX_OBJ_SYS_PASSWORD) == 0 )
#else
   if ( strcmp(name,PSX_OBJ_SYS_PASSWORD) == 0 ||
        strcmp(name,PSX_OBJ_SPT_PASSWORD) == 0 ||
        strcmp(name,PSX_OBJ_USR_PASSWORD) == 0 )
#endif
    {
      /*start of 用户密码不加密功能129990 */
	  #ifdef SUPPORT_SYSPWD_UNENCRYPTED
	  /*end of 用户密码不加密功能129990 */
	  fprintf(file,"<%s value=\"%s\"/>\n",name,str);
      #else
      char *base64 = NULL;
      base64Encode((unsigned char *)str, strlen(str), &base64);
      fprintf(file,"<%s value=\"%s\"/>\n",name,base64);
      free(base64);
      #endif
   } 
   else   
     fprintf(file,"<%s value=\"%s\"/>\n",name,str);
}



/*GLB:s53329,start add for DNS*/
PSX_STATUS xmlEndDnsObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndDnsObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; dnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(dnsObjs[i].objName, objName) == 0) {
      if (*(dnsObjs[i].endObjFnc) != NULL) {
	sts = (*(dnsObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndDnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 


PSX_STATUS xmlSetPDnsObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(dnsObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(dnsObjs[id].setAttrFnc) != NULL)
     (*(dnsObjs[id].setAttrFnc))(file, dnsObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}



PSX_STATUS xmlGetDnsObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

//#ifdef XML_DEBUG
  printf("xmlGetDnsObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
//#endif

  for ( i = 0; dnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(dnsObjs[i].objName, objName) == 0) {
      if (*(dnsObjs[i].getAttrFnc) != NULL) {
	sts = (*(dnsObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetDnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 


PSX_STATUS xmlStartDnsObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

//#ifdef XML_DEBUG
  printf("xmlStartDnsObjNode(appName %s, objName %s\n",appName,objName);
//#endif

  for ( i = 0; dnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(dnsObjs[i].objName, objName) == 0) {
      if (*(dnsObjs[i].startObjFnc) != NULL) {
        sts = (*(dnsObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartDnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

/*GLB:s53329,end add for DNS*/
void xmlSetHttpObjAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                      const UINT16 length)
{
   char str[IFC_LARGE_LEN];
   if ((name == NULL) || (value == NULL))
      return;

   memcpy(str, (char *)value, length);
   str[length] = '\0';
    if ( strcmp(name,PSX_OBJ_HTTP_PORT) == 0 )
    {
        fprintf(file,"<%s port=\"%s\"/>\n",name,str);
    } 
}
void xmlConvertLogLevel(FILE *file, char *str, int level)
{
  switch (level)
     {
     case LOG_EMERG:
       fprintf(file,"%s=\"EMERG\" ",str);
       break;
     case LOG_ALERT:
       fprintf(file,"%s=\"ALERT\" ",str);
       break;
     case LOG_CRIT:
       fprintf(file,"%s=\"CRIT\" ",str);
       break;
     case LOG_ERR:
       fprintf(file,"%s=\"ERR\" ",str);
       break;
     case LOG_WARNING:
       fprintf(file,"%s=\"WARNING\" ",str);
       break;
     case LOG_NOTICE:
       fprintf(file,"%s=\"NOTICE\" ",str);
       break;
     case LOG_INFO:
       fprintf(file,"%s=\"INFO\" ",str);
       break;
     case LOG_DEBUG:
       fprintf(file,"%s=\"DEBUG\" ",str);
       break;
     default:
       if (strcmp(str,PSX_ATTR_SYSLOG_DISPLAY_LEVEL) == 0)
         fprintf(file,"%s=\"ERR\" ",str);
       else
         fprintf(file,"%s=\"DEBUG\" ",str);
     }
}

int xmlStateToPsi(char *value)
{
  if ((strcmp("disable",value) == 0) || (strcmp("disabled",value) == 0))
    return 0;
  else
    return 1;
}

int xmlPolicyToPsi(char *value)
{
  if (strcmp("forward",value) == 0)
    return 0;
  else
    return 1;
}

int xmlAalTypeToPsi(char *value)
{
  if (strcmp(value,"AAL2") == 0)
    return AAL_2;
  else if (strcmp(value,"AAL_TRANSPARENT") == 0)
    return AAL_TRANSPARENT;
  else if (strcmp(value,"AAL0_PKT") == 0)
    return AAL_0_PACKET;
  else if (strcmp(value,"AAL0_CELL") == 0)
    return AAL_0_CELL_CRC;
  else if (strcmp(value,"AAL5") == 0)
    return AAL_5;
  return AAL_5;
}

int xmlWanProtocolToPsi(char *value)
{
  if (strcmp(value,"PPPOE") == 0)
    return PROTO_PPPOE;
  else if (strcmp(value,"PPPOA") == 0)
    return PROTO_PPPOA;
  else if (strcmp(value,"MER") == 0)
    return PROTO_MER;
  else if (strcmp(value,"BRIDGE") == 0)
    return PROTO_BRIDGE;
  else if (strcmp(value,"PPPOE_RELAY") == 0)
    return PROTO_PPPOE_RELAY;
  else if (strcmp(value,"IPOA") == 0)
    return PROTO_IPOA;
#if SUPPORT_ETHWAN
  else if (strcmp(value,"IPoW") == 0)
    return PROTO_IPOWAN;
#endif
//#ifdef VDF_PPPOU
  /*start, add by sxg, pppousb*/
  else if (strcmp(value,"PPPOU") == 0)
  	return PROTO_PPPOU;
  /*end, add by sxg, pppousb*/
//#endif
  else if (strcmp(value,"NONE") == 0)
    return PROTO_NONE;

  return PROTO_NONE;
}

int xmlWanEncapToPsi(char *value, int protocol)
{
  // encapMode for other protocols: LLC==0, vcMux==1
  // but for PPPOA vcMux is the default and should =0, LLC=1, so swap
  if ( protocol != PROTO_PPPOA ) {
    if (strcmp(value,"LLC") == 0)
      return 0;
    else if (strcmp(value,"VCMUX") == 0)
      return 1;
  } else {
    if (strcmp(value,"LLC") == 0)
      return 1;
    else if (strcmp(value,"VCMUX") == 0)
      return 0;
  }

  return 0;
}

int xmlEncapToPsi(char *value)
{
  if (strcmp(value,"vcMuxRouted") == 0)
    return ET_VC_MULTIPLEX_ROUTED_PROTOCOL;
  else if (strcmp(value,"vcMuxBr8023") == 0)
    return ET_VC_MULTIPLEX_BRG_PROTOCOL_8023;
  else if (strcmp(value,"vcMuxBr8025") == 0)
    return ET_VC_MULTIPLEX_BRG_PROTOCOL_8025;
  else if (strcmp(value,"vcMuxBr8026") == 0)
    return ET_VC_MULTIPLEX_BRG_PROTOCOL_8026;
  else if (strcmp(value,"vcMuxLanEmu8023") == 0)
    return ET_VC_MULTIPLEX_LAN_EMULATION_8023;
  else if (strcmp(value,"vcMuxLanEmu8025") == 0)
    return ET_VC_MULTIPLEX_LAN_EMULATION_8025;
  else if (strcmp(value,"llc") == 0)
    return ET_LLC_ENCAPSULATION;
  else if (strcmp(value,"frameRelaySscs") == 0)
    return ET_MULTI_PROTOCOL_FRAME_RELAY_SSCS;
  else if (strcmp(value,"other") == 0)
    return ET_OTHER;
  else if (strcmp(value,"unknown") == 0)
    return ET_UNKNOWN;
  return ET_UNKNOWN;
}

int xmlAtmAdminStatusToPsi(char *value)
{
  if (strcmp(value,"up") == 0)
    return ADMSTS_UP;
  else if (strcmp(value,"down") == 0)
    return ADMSTS_DOWN;
  else if (strcmp(value,"testing") == 0)
    return ADMSTS_TESTING;
  return ADMSTS_DOWN;
}

int xmlLogLevelToPsi(char *value)
{
  if (strcmp(value,"EMERG") == 0)
    return LOG_EMERG;
  else if (strcmp(value,"ALERT") == 0)
    return LOG_ALERT;
  else if (strcmp(value,"CRIT") == 0)
    return LOG_CRIT;
  else if (strcmp(value,"ERR") == 0)
    return LOG_ERR;
  else if (strcmp(value,"WARNING") == 0)
    return LOG_WARNING;
  else if (strcmp(value,"NOTICE") == 0)
    return LOG_NOTICE;
  else if (strcmp(value,"INFO") == 0)
    return LOG_INFO;
  else if (strcmp(value,"DEBUG") == 0)
    return LOG_DEBUG;
  return LOG_DEBUG;
}

int xmlLogOptionToPsi(char *value)
{
  int option = BCM_SYSLOG_LOCAL;
  
  if (strcmp(value,"remote") == 0)
    option = BCM_SYSLOG_REMOTE;
  else if (strcmp(value,"both") == 0)
    option = BCM_SYSLOG_LOCAL_REMOTE;
  else
    option = BCM_SYSLOG_LOCAL;
  return option;
}

int xmlPppAuthToPsi(char *value)
{
  if (strcmp(value,"auto") == 0)
    return PPP_AUTH_METHOD_AUTO;
  else if (strcmp(value,"PAP") == 0)
    return PPP_AUTH_METHOD_PAP;
  else if (strcmp(value,"CHAP") == 0)
    return PPP_AUTH_METHOD_CHAP;
  else if (strcmp(value,"MSCHAP") == 0)
    return PPP_AUTH_METHOD_MSCHAP;

  return PPP_AUTH_METHOD_AUTO;
}

int xmlPairToPsi(char *value)
{
  if (strcmp("inner",value) == 0)
    return 0;
  else
    return 1;
}

PSX_STATUS xmlVerifyVccId(PWAN_CON_INFO wan, UINT16 *vpi, UINT16 *vci)
{
  PSX_STATUS status = PSX_STS_OK;
  char *ptr = NULL, *start = NULL, *end = NULL;
  char vccId[20], vpiStr[10], vciStr[10];
  int len;

  /* knowing the connId and vccId, try to find out vpi/vci to fill into wanId */
  /* look for connId in configuration buffer */
  sprintf(vccId,"vccId%d ",wan->vccId);
  ptr = strstr(configBeginPtr,vccId);

  if (ptr == NULL) {
    status = PSX_STS_ERR_FATAL;
  }
  else {
    start = strstr(ptr,"vpi=\"");
    start += strlen("vpi=\""); /* skipping until vpi number */
    end = strchr(start,'"');
    len = end - start;
    strncpy(vpiStr,(char*)start,len);
    vpiStr[len] = '\0';

    start = strstr(ptr,"vci=\"");
    start += strlen("vci=\""); /* skipping until vpi number */
    end = strchr(start,'"');
    len = end - start;
    strncpy(vciStr,(char*)start,len);
    vciStr[len] = '\0';

#ifdef XML_DEBUG
    printf("xmlVerifyVccId(look for vccId %s), found.  vpi %s, vci %s\n",vccId,vpiStr, vciStr);
#endif

    if ((BcmDb_validateAtmVpi(vpiStr) != DB_OBJ_VALID_OK) ||
        (BcmDb_validateAtmVci(vciStr) != DB_OBJ_VALID_OK))
      status = PSX_STS_ERR_FATAL;
    else {
      *vpi = atoi(vpiStr);
      *vci = atoi(vciStr);
    }
  }
  return status;
}

int xmlVerifyConIdExist(PWAN_CON_INFO wan)
{
  char *ptr = NULL;
  char conId[10];

  /* knowing the connId and vccId, try to find out vpi/vci to fill into wanId */
  /* look for connId in configuration buffer */
  sprintf(conId,"conId%d",wan->conId);
  ptr = strstr(configBeginPtr,conId);
  if (ptr == NULL) 
    return PSX_STS_ERR_FATAL;
  else
    return PSX_STS_OK;
}

int xmlVerifyConIdProtocol(PWAN_CON_ID wanId, char *protocol)
{
  char *conPtr = NULL;
  char name[16];
    
  memset(name,0,sizeof(name));
  /* start of maintain PSI移植：纯桥和MER方式下使能dhcp by xujunxia 43813 2006年5月5日"
  if (strcmp(protocol,"PPP") == 0)
    sprintf(name,"ppp_conId%d",wanId->conId);
  else
    sprintf(name,"dhcpc_conId%d",wanId->conId);
  */
  if (0 == strcmp(protocol,"PPP"))
  {
    sprintf(name,"ppp_conId%d",wanId->conId);
  }
  else if (0 == strcmp(protocol,"PROTO_BRIDGE"))
  {
    sprintf(name,"br_conId%d",wanId->conId);
  }
  else
  {
    sprintf(name,"dhcpc_conId%d",wanId->conId);
  }
  /* end of maintain PSI移植：纯桥和MER方式下使能dhcp by xujunxia 43813 2006年5月5日 */

  conPtr = strstr(configBeginPtr,name);      
    /* search for the right conId entry */

  if (conPtr == NULL)
    return PSX_STS_ERR_FATAL;
  else
    return PSX_STS_OK;
} /* xmlVerifyConIdProtocol */

PSX_STATUS xmlVerifyVccExist(UINT16 vpi, UINT16 vci)
{
  PSX_STATUS status = PSX_STS_OK;
  char *ptr = NULL, *start = NULL, *end = NULL;
  char vpiStr[10], vciStr[10];
  int len, found = 0;

#ifdef XML_DEBUG
  printf("xmlVerifyVccExist(vpi/vci %d/%d)\n",vpi,vci);
#endif

  /* first verify with AtmCfgVcc */
  end = strstr(configBeginPtr,"vpi");
  while (!found) {
    ptr = strstr(end,"vpi");
    if (ptr == NULL) {
      status = PSX_STS_ERR_FATAL;
      break;  /* cannot even find vpi, quit */
    }
    else {

#ifdef XML_DEBUG
  printf("xmlVerifyVccExist(found vpi %d string)\n",vpi);
#endif

      start = ptr + strlen("vpi=\""); /* skipping until vpi number */
      end = strchr(start,'"');
      len = end - start;
      strncpy(vpiStr,(char*)start,len);
      vpiStr[len] = '\0';

#ifdef XML_DEBUG
      printf("xmlVerifyVccExist(len %d, vpiStr %s)\n",len,vpiStr);
#endif

      if (atoi(vpiStr) != vpi) {
        continue;
      }
      else {
        /* is the vci the same too? */
        start = strstr(ptr,"vci=\"");
        start += strlen("vci=\""); /* skipping until vpi number */
        end = strchr(start,'"');
        len = end - start;
        strncpy(vciStr,(char*)start,len);
        vciStr[len] = '\0';

#ifdef XML_DEBUG
        printf("xmlVerifyVccExist(len %d, vciStr %s)\n",len,vciStr);
#endif

        if (atoi(vciStr) != vci) {
          continue;
        }
        else {
          found = 1;
          break;
        }
      }
    }
  } /* loop */

#ifdef XML_DEBUG
  printf("xmlVerifyVccExist(found %d) returns %d\n",found,status);
#endif

  return status;
}

int xmlGetTableSize(FILE *file) {
#define XML_TABLE_SIZE_LEN 16
   int size = 0;
   char buf[XML_TABLE_SIZE_LEN+1];
   char *firstQuote = NULL;
   char *secondQuote = NULL;
   long curr = ftell(file);
   
   // put file descriptor back XML_TABLE_SIZE_LEN
   fseek(file, curr - XML_TABLE_SIZE_LEN, SEEK_SET);
   // read XML_TABLE_SIZE_LEN characters to put
   // file descriptor to current position
   fgets(buf, XML_TABLE_SIZE_LEN, file);
   
   // buf = ...tableSize="1"...
   buf[XML_TABLE_SIZE_LEN] = '\0';
   if ( strstr(buf, "Size") == NULL )
      return size;
   firstQuote = strchr(buf, '"');
   if ( firstQuote == NULL )
      return size;
   secondQuote = strchr(firstQuote+1, '"');
   if ( secondQuote == NULL )
      return size;
   *secondQuote = '\0';
   size = atoi(firstQuote + 1);
   
   return size;
}

/* 
   syslog configuration consists of:
   1. log: enable/disable
   2. displayLevel: 1-7
   3. logLevel: 1-7
   4. option: local/remote
   5. serverIp: x.x.x.x
   6. serverPort: 514 (default)
*/
void xmlSetSysLogAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   BCM_SYSLOG_CB sysLog;
   static char* state[] = {"disable","enable"};
   /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
   char addr[IFC_TINY_LEN];
   */
   char addr[IFC_LARGE_LEN];
   /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&sysLog, (char *)value, length);
   /* start of maintain syslog sever 可以通过dhcp option7  by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
   fprintf(file,"<%s %s=\"%s\" CfgServer=\"%s\" " ,name,PSX_ATTR_STATE,state[sysLog.status], state[sysLog.CfgServerEnable]);
#else
   fprintf(file,"<%s %s=\"%s\" " ,name,PSX_ATTR_STATE,state[sysLog.status]);
#endif
   /* end of maintain syslog sever 可以通过dhcp option7  by xujunxia 43813 2006年6月29日 */
   xmlConvertLogLevel(file, PSX_ATTR_SYSLOG_DISPLAY_LEVEL, sysLog.localDisplayLevel);
   xmlConvertLogLevel(file, PSX_ATTR_SYSLOG_LOG_LEVEL, sysLog.localLogLevel);
   if ((sysLog.option & BCM_SYSLOG_LOCAL_REMOTE_MASK) == BCM_SYSLOG_LOCAL_REMOTE) 
     fprintf(file,"%s=\"both\" ",PSX_ATTR_SYSLOG_OPTION);
   else if ((sysLog.option & BCM_SYSLOG_REMOTE_MASK) == BCM_SYSLOG_REMOTE)
     fprintf(file,"%s=\"remote\" ",PSX_ATTR_SYSLOG_OPTION);
   else
     fprintf(file,"%s=\"local\" ",PSX_ATTR_SYSLOG_OPTION);
   /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
   strncpy(addr,inet_ntoa(sysLog.serverIP),IFC_TINY_LEN);
   */
   strncpy(addr,sysLog.serverAddr,IFC_LARGE_LEN);
   /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */
   /*modified by z67625 保存防火墙日志写flash开关 start*/
   /*start of 增加保存系统日志开关by c00131380 at 080926*/
   fprintf(file,"%s=\"%s\" %s=\"%d\" %s=\"%s\" %s=\"%s\"/>\n",PSX_ATTR_SYSLOG_SERVER_IP,addr,PSX_ATTR_SYSLOG_SERVER_PORT,sysLog.serverPort,
            PSX_ATTR_SYSLOG_FWLOG, state[sysLog.fwlog],PSX_ATTR_SYSLOG_SAVE,state[sysLog.Syslog]);
   /*modified by z67625 保存防火墙日志写flash开关 end*/
}

void xmlSetDftGwAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		     const UINT16 length)
{
   PIFC_DEF_GW_INFO gw;
   char addr[IFC_TINY_LEN];
   static char* state[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL))
      return;

   gw = (PIFC_DEF_GW_INFO)value;
   strncpy(addr,inet_ntoa(gw->defaultGateway),IFC_TINY_LEN);
   fprintf(file,"<%s enblGwAutoAssign=\"%s\" ifName=\"%s\" ipAddr=\"%s\"/>\n",
	   name,state[gw->enblGwAutoAssign],gw->ifName,addr);
}

void xmlSetDnsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   IFC_DNS_INFO dnsInfo;
   char primary[IFC_TINY_LEN], secondary[IFC_TINY_LEN];
   static char* state[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&dnsInfo, (char *)value, length);
   strncpy(primary,inet_ntoa(dnsInfo.preferredDns),IFC_TINY_LEN);
   strncpy(secondary,inet_ntoa(dnsInfo.alternateDns),IFC_TINY_LEN);	
   /* modify by y00183561 for lan dns proxy enable/disable 2011/10/24 */ 
   fprintf(file,"<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",name,PSX_ATTR_DNS_DYNAMIC,state[dnsInfo.dynamic],PSX_ATTR_DNS_PRIMARY,primary,PSX_ATTR_DNS_SECONDARY,secondary,PSX_ATTR_DNS_DOMAIN,dnsInfo.domainName,PSX_ATTR_DNS_PROXY_ENABLE,state[dnsInfo.enbdnsProxy]);
}

void xmlSetAutoScanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   UINT16 mode;
   static char* state[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&mode, (char *)value, length);
   
   /* interprete autoscan data */
   fprintf(file,"<%s %s=\"%s\"/>\n",name,PSX_ATTR_STATE,state[mode]);
}

#ifdef SUPPORT_SNMP
void xmlSetSnmpAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
   BCM_SNMP_STRUCT snmp;
   static char* state[] = {"disable","enable"};
   char addr[IFC_TINY_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&snmp, (char *)value, length);
   strncpy(addr,inet_ntoa(snmp.trapIp),IFC_TINY_LEN);
   /* interprete autoscan data */
   fprintf(file,"<%s %s=\"%s\" readCommunity=\"%s\" writeCommunity=\"%s\" sysName=\"%s\" sysLocation=\"%s\" sysContact=\"%s\" trapIP=\"%s\" debug=\"%x\" configId=\"%.02x%.02x%.02x%.02x\"/>\n",
	   name,PSX_ATTR_STATE,state[snmp.status],snmp.roCommunity,snmp.rwCommunity,snmp.sysName,
	   snmp.sysLocation,snmp.sysContact,addr,snmp.debug,
           (unsigned char)snmp.configId[0],(unsigned char)snmp.configId[1],
           (unsigned char)snmp.configId[2],(unsigned char)snmp.configId[3]);
}
#endif /* #ifdef SUPPORT_SNMP */

#ifdef SUPPORT_TR69C
void xmlSetTr69cAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
   BCM_TR69C_INFO tr69c;
   static char* state[] = {"disable","enable"};
   char *pcAcsPwbase64 = NULL;
   char *pcConReqPwbase64 = NULL;
   char str[IFC_LARGE_LEN];

   if ((name == NULL) || (value == NULL))
      return;
 
   memcpy(&tr69c, (char *)value, length);

   /*add by w00135351 09.3.13 密码明文显示在配置文件中*/
   #ifndef SUPPORT_TR69CPWD_UNENCRYPTED
   /*Start:ACS pw, Conreq pw 加密 ，2007.3.21*/
   memcpy(str, tr69c.acsPwd, strlen(tr69c.acsPwd));
   str[strlen(tr69c.acsPwd)] = '\0';
   base64Encode((unsigned char *)str, strlen(str), &pcAcsPwbase64);

   memcpy(str, tr69c.connReqPwd, strlen(tr69c.connReqPwd));
   str[strlen(tr69c.connReqPwd)] = '\0';
   base64Encode((unsigned char *)str, strlen(str), &pcConReqPwbase64);
   /*End:ACS pw 加密 ，2007.3.21*/
   
   fprintf(file,"<%s %s=\"%s\" upgradesManaged=\"%d\" upgradeAvailable=\"%d\" \
informEnbl=\"%d\" informTime=\"%lu\" informInterval=\"%lu\" \
acsURL=\"%s\" acsUser=\"%s\" acsPwd=\"%s\" parameterKey=\"%s\" \
connReqURL=\"%s\" connReqUser=\"%s\" connReqPwd=\"%s\" \
kickURL=\"%s\" provisioningCode=\"%s\"/>\n",
       name, PSX_ATTR_STATE, state[tr69c.status], tr69c.upgradesManaged, tr69c.upgradeAvailable,
	   tr69c.informEnable, tr69c.informTime, tr69c.informInterval,
	   tr69c.acsURL, tr69c.acsUser, pcAcsPwbase64, tr69c.parameterKey,
	   tr69c.connReqURL, tr69c.connReqUser, pcConReqPwbase64,
	   tr69c.kickURL, tr69c.provisioningCode);

	   free(pcAcsPwbase64);
	   free(pcConReqPwbase64);
   #else
   fprintf(file,"<%s %s=\"%s\" upgradesManaged=\"%d\" upgradeAvailable=\"%d\" \
informEnbl=\"%d\" informTime=\"%lu\" informInterval=\"%lu\" \
acsURL=\"%s\" acsUser=\"%s\" acsPwd=\"%s\" parameterKey=\"%s\" \
connReqURL=\"%s\" connReqUser=\"%s\" connReqPwd=\"%s\" \
kickURL=\"%s\" provisioningCode=\"%s\"/>\n",
       name, PSX_ATTR_STATE, state[tr69c.status], tr69c.upgradesManaged, tr69c.upgradeAvailable,
	   tr69c.informEnable, tr69c.informTime, tr69c.informInterval,
	   tr69c.acsURL, tr69c.acsUser, tr69c.acsPwd, tr69c.parameterKey,
	   tr69c.connReqURL, tr69c.connReqUser, tr69c.connReqPwd,
	   tr69c.kickURL, tr69c.provisioningCode);
   #endif
   /*end by w00135351 09.3.13 密码明文显示在配置文件中*/
}
#endif // #ifdef SUPPORT_TR69C

#ifdef SUPPORT_RADIUS
void xmlSetRadiusAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
   PRADIUS_CLIENT_INFO  RadiusClient;
   

   if ((name == NULL) || (value == NULL))
      return;
    RadiusClient = (PRADIUS_CLIENT_INFO)value;
   
   fprintf(file,"<%s WebEnable=\"%d\"  TelnetEnable=\"%d\" SshEnable=\"%d\" PrimaryServer=\"%s\" "
                "PrimaryPort=\"%d\" SecondServer=\"%s\" SecondPort=\"%d\" Retransmit=\"%d\" Timeout=\"%d\""
                "ShareKey=\"%s\"/>\n",  name, RadiusClient->WebEnable, RadiusClient->TelnetEnable, RadiusClient->SshEnable, 
                RadiusClient->PrimaryServer, RadiusClient->PrimaryPort, RadiusClient->SecondServer, RadiusClient->SecondPort,
                RadiusClient->Retransmit, RadiusClient->Timeout, RadiusClient->ShareKey);

}

#endif // #ifdef SUPPORT_TR69C
void xmlSetMgmtLockAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
   SYS_MGMT_LOCK_INFO mgmtLock;
   static char* actions[] = {"unlock","lock"};
   static char* types[] = {"unknown","CLI","menuCLI","WEB","SNMP","TR069","UPNP"};

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&mgmtLock, (char *)value, length);
   fprintf(file,"<%s action=\"%s\" type=\"%s\"/>\n",
       name, actions[mgmtLock.action], types[mgmtLock.type]);
}

/* start of maintain 去除配置文件中的参数encodePassword by xujunxia 43813 2006年10月8日
void xmlSetOtherSysAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   PSYS_FLAG_INFO flag;
   static char* state[] = {"disable","enable"};
   static char* policy[] = {"forward","block"};
   
   if ((name == NULL) || (value == NULL))
      return;

   flag = (PSYS_FLAG_INFO)value;

#ifdef SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD
#ifdef SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" siproxd=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],
           state[flag->siproxd],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" siproxd=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],state[flag->siproxd]);
#endif // SUPPORT_ETHWAN
#else   // SUPPORT_SIPROXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],state[flag->enetWan]);
#else //SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword]);
#endif //SUPPORT_ETHWAN
#endif  // SUPPORT_SIPROXD
#else   // SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD  // START SUPPORT SIPROXD by c47036
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" siproxd=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],state[flag->siproxd],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" siproxd=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],state[flag->siproxd]);
#endif // //SUPPORT_ETHWAN
#else  // ELSE SUPPORT SIPPOXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" encodePassword=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->encodePassword]);
#endif // //SUPPORT_ETHWAN
#endif  // END SUPPORT SIPPOXD
#endif  // SUPPORT_UPNP
}

*/
void xmlSetOtherSysAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   PSYS_FLAG_INFO flag;
   static char* state[] = {"disable","enable"};
   static char* policy[] = {"forward","block"};
   
   if ((name == NULL) || (value == NULL))
      return;

   flag = (PSYS_FLAG_INFO)value;

/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if 0
// modified by l66195 for VDF start
#ifdef SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD
#ifdef SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],
           state[flag->siproxd], state[flag->ddns], state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->siproxd]
           ,state[flag->ddns]);
#endif // SUPPORT_ETHWAN
#else   // SUPPORT_SIPROXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],
           state[flag->ddns],state[flag->enetWan]);
#else //SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->ddns]);
#endif //SUPPORT_ETHWAN
#endif  // SUPPORT_SIPROXD
#else   // SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD  // START SUPPORT SIPROXD by c47036
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->siproxd],
           state[flag->ddns],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->siproxd],state[flag->ddns]);
#endif // //SUPPORT_ETHWAN
#else  // ELSE SUPPORT SIPPOXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->ddns],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\" igmpMode =\"%s\" macFilterPolicy=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], state[flag->igmpMode], policy[flag->macPolicy],state[flag->ddns]);
#endif // //SUPPORT_ETHWAN
#endif  // END SUPPORT SIPPOXD
#endif  // SUPPORT_UPNP
// modified by l66195 for VDF end
#endif
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/

/*start of 重新设置snooping启动接口by l129990 2008,9,9*/
// modified by l66195 for VDF start
#ifdef SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD
#ifdef SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], policy[flag->macPolicy],
           state[flag->siproxd], state[flag->ddns], state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->siproxd]
           ,state[flag->ddns]);
#endif // SUPPORT_ETHWAN
#else   // SUPPORT_SIPROXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\" macFilterPolicy=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], policy[flag->macPolicy],
           state[flag->ddns],state[flag->enetWan]);
#else //SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" upnp=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->upnp],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->ddns]);
#endif //SUPPORT_ETHWAN
#endif  // SUPPORT_SIPROXD
#else   // SUPPORT_UPNP
#ifdef SUPPORT_SIPROXD  // START SUPPORT SIPROXD by c47036
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->siproxd],
           state[flag->ddns],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" siproxd=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->siproxd],state[flag->ddns]);
#endif // //SUPPORT_ETHWAN
#else  // ELSE SUPPORT SIPPOXD
#if SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" ddns=\"%s\" enetwan=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->ddns],state[flag->enetWan]);
#else // SUPPORT_ETHWAN
   fprintf(file,"<%s autoScan=\"%s\" igmpSnp =\"%s\"  macFilterPolicy=\"%s\" ddns=\"%s\"/>\n",
           name,state[flag->autoScan],state[flag->igmpSnp], policy[flag->macPolicy],state[flag->ddns]);
#endif // //SUPPORT_ETHWAN
#endif  // END SUPPORT SIPPOXD
#endif  // SUPPORT_UPNP
// modified by l66195 for VDF end
/*end of 重新设置snooping启动接口by l129990 2008,9,9*/
}

/* end of maintain 去除配置文件中的参数encodePassword by xujunxia 43813 2006年10月8日 */

PSX_STATUS xmlSetRouteObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(routeObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(routeObjs[id].setAttrFnc) != NULL)
     (*(routeObjs[id].setAttrFnc))(file, routeObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

/* j00100803 Add Begin 2008-02-23 */
#ifdef SUPPORT_POLICY
void xmlSetRtPolicyAttr(FILE* file, const char *name, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{   
	PIFC_RoutePolicy_Info pTmp = NULL;
	PIFC_RoutePolicy_Info pRtPolicyInfo = NULL;
	UINT16 usNum = 0;
	if((NULL == file) || (NULL == name) || (NULL == value))
	{
		fprintf(stderr, "error: %s\n", "invalid forwarding type xmlSetRtPolicyCfgNode" );
		return;
	}

	if (strcmp(name,TBLNAME_POLICY) == 0) 
	{
	    usNum = *(UINT16*)value;
	    if ( usNum != 0 )
	      	fprintf(file,"<%s tableSize=\"%d\">\n",name, usNum);
	}
	else
	{
		usNum = xmlGetTableSize(file);
		pTmp = (PIFC_RoutePolicy_Info)value;
		
		for(int i = 0; i < usNum; i++)
		{
			pRtPolicyInfo = &pTmp[i];
			if((pRtPolicyInfo->enType <= BcmCfm_ENUM_StartBased) || 
				(pRtPolicyInfo->enType >= BcmCfm_ENUM_EndBased))
			{
				fprintf(stderr, "error: %s\n", "invalid forwarding type xmlSetRtPolicyCfgNode");
				return;
			}
			if(BcmCfm_ENUM_MacBased == pRtPolicyInfo->enType)
			{
				char szMac[18];
				bcmMacNumToStr(pRtPolicyInfo->szSrcMacAddress, szMac);
				fprintf(file,
						"<policy id=\"%d\" type=\"MAC\" mac_addr=\"%s\" src_addr=\"0\" src_ifc=\"0\" protocol=\"0\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"0\"/>\n", 
						pRtPolicyInfo->uId, 
						szMac, 
						pRtPolicyInfo->uWanIndex,
						pRtPolicyInfo->szWanIfcName);
			}
			if(BcmCfm_ENUM_SrcIPBased == pRtPolicyInfo->enType)
			{
				fprintf(file,
						"<policy id=\"%d\" type=\"IP di origine\" mac_addr=\"0\" src_addr=\"%s\" src_ifc=\"0\" protocol=\"0\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"0\"/>\n", 
						pRtPolicyInfo->uId, 
						pRtPolicyInfo->szIPSrcAddress, 
						pRtPolicyInfo->uWanIndex,
						pRtPolicyInfo->szWanIfcName);
			}
			else if(BcmCfm_ENUM_SrcIfcBased == pRtPolicyInfo->enType)
			{
				/* get lan interface name by interface id  */
				fprintf(file,
						"<policy id=\"%d\" type=\"Interfaccia di or.\" mac_addr=\"0\" src_addr=\"0\" src_ifc=\"%s\" protocol=\"0\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"0\"/>\n", 
						pRtPolicyInfo->uId, 
						pRtPolicyInfo->szSrcIfcName, 
						pRtPolicyInfo->uWanIndex,
						pRtPolicyInfo->szWanIfcName);
			}
			else if(BcmCfm_ENUM_ProtocolBased == pRtPolicyInfo->enType)
			{
				fprintf(file,
						"<policy id=\"%d\" type=\"Tipo protocollo\" mac_addr=\"0\" src_addr=\"0\" src_ifc=\"0\" protocol=\"%s\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"0\"/>\n", 
						pRtPolicyInfo->uId, 
						pRtPolicyInfo->szProtocolType, 
						pRtPolicyInfo->uWanIndex,
						pRtPolicyInfo->szWanIfcName);
			}
			else if(BcmCfm_ENUM_DestIPBased == pRtPolicyInfo->enType)
			{
				fprintf(file,
						"<policy id=\"%d\" type=\"IP di destinazione\" mac_addr=\"0\" src_addr=\"0\" src_ifc=\"0\" protocol=\"0\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"%s\"/>\n", 
						pRtPolicyInfo->uId, 
						pRtPolicyInfo->uWanIndex,
						pRtPolicyInfo->szWanIfcName,
						pRtPolicyInfo->szIPDestAddress);
			}
            else if(BcmCfm_ENUM_IpDscpBased == pRtPolicyInfo->enType)
			{
				fprintf(file, "<policy id=\"%d\" type=\"IpDscp\" mac_addr=\"0\" src_addr=\"0\" src_ifc=\"0\" protocol=\"0\" wanifc=\"%d\" wanname=\"%s\" dest_addr=\"0\" dscp=\"%d\"/>\n", 
                		                pRtPolicyInfo->uId, 
                		                pRtPolicyInfo->uWanIndex,
                		                pRtPolicyInfo->szWanIfcName,
                		                pRtPolicyInfo->nDscpValue);
			}
			else
			{
				
			}
		}
		if ( usNum != 0 )
          	fprintf(file,"</%s>\n",routeObjs[id - 1].objName);
	}
	return;
}

PSX_STATUS xmlGetRtPolicyAttr(char *attrName,
                        					   char* attrValue)
{
	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

	if (strncmp(attrName, "tableSize", strlen("tableSize")) == 0) 
	{
		if (xmlState.verify)
	  		status = BcmDb_validateRange(attrValue, 0, POLICY_MAX_SIZE);
		if (status == DB_OBJ_VALID_OK) 
		{
	  		xmlState.tableIndex = 0;
	  		xmlState.tableSize = atoi(attrValue);
	  		psiValue = (void *) malloc(sizeof(IFC_RoutePolicy_Info) * xmlState.tableSize);
			if (psiValue == NULL) 
			{
	     		xmlMemProblem();
	     		return PSX_STS_ERR_FATAL;
	  		}
	  		memset(psiValue, 0, sizeof(QOS_QUEUE_ENTRY) * xmlState.tableSize);
		}  
		else
		{
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetRtPolicyAttr(): invalid number %s\n",attrValue);
			printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
			      atoi(attrValue), POLICY_MAX_SIZE);
		}
	}
	return(PSX_STS_OK);
}

PSX_STATUS xmlEndRtPolicyObj(char *appName, char *objName)
{
	int error = 0;
	/* check to see if all mandatory object attribute are set */  
	if (xmlState.verify) 
	{
		if (xmlState.tableSize != xmlState.tableIndex) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlEndRtPolicyObj(): table size does not match with number of entries.\n");
	  		error = 1;
		}
	}
	if (!error && !xmlState.verify) 
	{
		BcmDb_setRtPolicySize(xmlState.tableSize);
		BcmDb_setRtPolicyInfo((PIFC_RoutePolicy_Info)psiValue, xmlState.tableSize);
	}

	free(psiValue);
	psiValue = NULL;
	xmlState.tableSize = 0;
	xmlState.tableIndex = 0;
	return PSX_STS_OK;
}

PSX_STATUS xmlGetRtPolicyEntryObj(char *attrName,
                        					   char* attrValue)
{
	PIFC_RoutePolicy_Info pRtPolicyInfo = NULL;
	PIFC_RoutePolicy_Info pRtPolicyTmp = NULL;
  	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
	char * pSubnetMask = NULL;
	char szIPAddress[IFC_SMALL_LEN];
	char szDestIPAddress[IFC_SMALL_LEN];
	memset(szIPAddress, 0, IFC_SMALL_LEN);
	memset(szDestIPAddress, 0, IFC_SMALL_LEN);
#ifdef XML_DEBUG
  printf("xmlGetRtPolicyEntryObj(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

	if ( psiValue == NULL ) 
	{
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		syslog(LOG_ERR,"xmlGetRtPolicyEntryObj(): psiValue is NULL\n");
		return PSX_STS_ERR_FATAL;
	}

	int i = xmlState.tableIndex;
	pRtPolicyTmp = (PIFC_RoutePolicy_Info)psiValue;
	if (xmlState.verify) 
	{
		if ( i >= xmlState.tableSize ) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlGetRtPolicyEntryObj(): invalid entry index %d, tableSize %d\n",
	         	i, xmlState.tableSize);
			fprintf(stdout, "xmlGetRtPolicyEntryObj invalid entry index\n");
	  		return PSX_STS_ERR_FATAL;
		}
	}

	pRtPolicyInfo = pRtPolicyTmp;
	if (strcmp(attrName, "id") == 0) 
	{
		if (xmlState.verify)
		  	status = BcmDb_validateNumber(attrValue);
		if (status == DB_OBJ_VALID_OK )
		  	pRtPolicyInfo[i].uId = atoi(attrValue);
		else 
		{
     		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		  	syslog(LOG_ERR,"xmlGetRtPolicyEntryObj(): invalid id %s\n",attrValue);
			fprintf(stdout,"xmlGetRtPolicyEntryObj(): invalid id %s\n",attrValue);
			return PSX_STS_ERR_FATAL;
		}
	}
  	else if(strncmp(attrName, "type", strlen("type")) == 0) 
  	{
    	if (xmlState.verify) 
      		status = BcmDb_validateLength(attrValue, strlen("Interfaccia di or.") + 1);
    	if (status != DB_OBJ_VALID_OK) 
		{
      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
      		syslog(LOG_ERR,"xmlGetRtPolicyEntryObj(): invalid string %s\n",attrValue);
			fprintf(stdout,"xmlGetRtPolicyEntryObj(): invalid string %s\n",attrValue);
			return PSX_STS_ERR_FATAL;
		}
    	else
		{
			if(0 == strcmp(attrValue, "MAC"))
			{
      			pRtPolicyInfo[i].enType = BcmCfm_ENUM_MacBased;
			}
			else if(0 == strcmp(attrValue, "IP di origine"))
			{
				pRtPolicyInfo[i].enType = BcmCfm_ENUM_SrcIPBased;
			}
			else if(0 == strcmp(attrValue, "Interfaccia di or."))
			{
				pRtPolicyInfo[i].enType = BcmCfm_ENUM_SrcIfcBased;
			}
			else if(0 == strcmp(attrValue , "Tipo protocollo"))
			{
				pRtPolicyInfo[i].enType = BcmCfm_ENUM_ProtocolBased;
			}
			else if(0 == strcmp(attrValue, "IP di destinazione"))
			{
				pRtPolicyInfo[i].enType = BcmCfm_ENUM_DestIPBased;
			}
            else if (0 == strcmp(attrValue, "IpDscp"))
            {
                    pRtPolicyInfo[i].enType = BcmCfm_ENUM_IpDscpBased;
            }
			else
			{
	      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): error type in psi %s\n",attrValue);
				fprintf(stdout, "xmlGetRtPolicyEntryObj(): error type in psi %s\n",attrValue);
				return PSX_STS_ERR_FATAL;
			}
    	}
  	}
  	else if (strncmp(attrName, "mac_addr", strlen("mac_addr")) == 0) 
	{
		if(strcmp(attrValue, "0") == 0)
		{
			strcpy(pRtPolicyInfo[i].szSrcMacAddress, "0");
		}
		else
		{
		    if (xmlState.verify)
		      	status = BcmDb_validateMacAddress(attrValue);
		      
	    	if (status != DB_OBJ_VALID_OK) 
			{
	      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): mac_addr invalid string %s\n", attrValue);
				fprintf(stdout, "xmlGetRtPolicyEntryObj(): mac_addr invalid string %s\n",attrValue);
				return PSX_STS_ERR_FATAL;
			}
		    else 
			{
				char szMac[18];
				memcpy(szMac, attrValue, 18);
				bcmMacStrToNum(pRtPolicyInfo[i].szSrcMacAddress, szMac);
		    }
		}
  	}
	else if(0 == strncmp(attrName, "wanifc", strlen("wanifc")))
	{
		int iTmp = atoi(attrValue);
		pRtPolicyInfo[i].uWanIndex = iTmp;
	}
	else if(0 == strncmp(attrName, "wanname", strlen("wanname")))
	{
	    if (xmlState.verify)
	      	status = BcmDb_validateInterfaceName(attrValue);
	      
    	if (status != DB_OBJ_VALID_OK) 
		{
      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): wanifcname invalid string %s\n", attrValue);
			fprintf(stdout, "xmlGetRtPolicyEntryObj(): wanifcname invalid string %s\n", attrValue);
			return PSX_STS_ERR_FATAL;
    	}
	    else 
		{
	   		strncpy(pRtPolicyInfo[i].szWanIfcName, attrValue, IFC_TINY_LEN);
	    }
	}	
	else if(0 == strncmp(attrName, "src_addr", strlen("src_addr")))
	{
		if(strcmp(attrValue, "0") == 0)
		{
			strcpy(pRtPolicyInfo[i].szIPSrcAddress, "0");
		}
		else
		{
			strcpy(szIPAddress, attrValue);
			pSubnetMask = strstr(szIPAddress, "/");
			if(NULL == pSubnetMask)
			{
				if (xmlState.verify)
			      	status = BcmDb_validateIpAddress(szIPAddress);
			      
		    	if (status != DB_OBJ_VALID_OK) 
				{
		      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): src_addr invalid address %s\n", attrValue);
					fprintf(stdout, "xmlGetRtPolicyEntryObj(): src_addr invalid address %s\n", attrValue);
					return PSX_STS_ERR_FATAL;
				}
			}
			else
			{
				*pSubnetMask = '\0';
				pSubnetMask ++;
			    if (xmlState.verify)
			      	status = BcmDb_validateNumber(pSubnetMask);
			      
		    	if (status != DB_OBJ_VALID_OK) 
				{
		      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): src_addr invalid string %s\n", attrValue);
					fprintf(stdout, "xmlGetRtPolicyEntryObj(): src_addr invalid string %s\n", attrValue);
					return PSX_STS_ERR_FATAL;
				}
			}
	   		strncpy(pRtPolicyInfo[i].szIPSrcAddress, attrValue, IFC_SMALL_LEN);
		}
	}
	else if(0 == strncmp(attrName, "src_ifc", strlen("src_ifc")))
	{
		if(strcmp(attrValue, "0") == 0)
		{
			strcpy(pRtPolicyInfo[i].szSrcIfcName, "0");
		}
		else
		{
		    if (xmlState.verify)
		      	status = BcmDb_validateInterfaceName(attrValue);
		      
	    	if (status != DB_OBJ_VALID_OK) 
			{
	      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): src_ifc invalid string %s\n", attrValue);
				fprintf(stdout, "xmlGetRtPolicyEntryObj(): src_ifc invalid string %s\n", attrValue);
				return PSX_STS_ERR_FATAL;
	    	}
		    else 
			{
		   		strncpy(pRtPolicyInfo[i].szSrcIfcName, attrValue, IFC_TINY_LEN);
		    }
		}
	}	
	else if(0 == strncmp(attrName, "protocol", strlen("protocol")))
	{
		if(strcmp(attrValue, "0") == 0)
		{
			strcpy(pRtPolicyInfo[i].szProtocolType, "0");
		}
		else
		{
			if (xmlState.verify)
				status = BcmDb_validateIpProtocolForRouting(attrValue);
				      
	    	if (status != DB_OBJ_VALID_OK) 
			{
	      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): Protocol invalid string %s\n", attrValue);
				fprintf(stdout, "xmlGetRtPolicyEntryObj(): Protocol invalid string %s\n", attrValue);
				return PSX_STS_ERR_FATAL;
			}
		    else 
			{
	  			strncpy(pRtPolicyInfo[i].szProtocolType, attrValue, IFC_TINY_LEN);
		    }	
		}
	}
	else if(0 == strncmp(attrName, "dest_addr", strlen("dest_addr")))
	{
		if(strcmp(attrValue, "0") == 0)
		{
			strcpy(pRtPolicyInfo[i].szIPDestAddress, "0");
		}
		else
		{
			strcpy(szDestIPAddress, attrValue);
			pSubnetMask = strstr(szDestIPAddress, "/");
			if(NULL == pSubnetMask)
			{
				if (xmlState.verify)
			      	status = BcmDb_validateIpAddress(szDestIPAddress);
			      
		    	if (status != DB_OBJ_VALID_OK) 
				{
		      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): dest_addr invalid address %s\n", attrValue);
					fprintf(stdout, "xmlGetRtPolicyEntryObj(): dest_addr invalid address %s\n", attrValue);
					return PSX_STS_ERR_FATAL;
				}
			}
			else
			{
				*pSubnetMask = '\0';
				pSubnetMask ++;
			    if (xmlState.verify)
			      	status = BcmDb_validateNumber(pSubnetMask);
			      
		    	if (status != DB_OBJ_VALID_OK) 
				{
		      		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		      		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): dest_addr invalid string %s\n", attrValue);
					fprintf(stdout, "xmlGetRtPolicyEntryObj(): dest_addr invalid string %s\n", attrValue);
					return PSX_STS_ERR_FATAL;
				}
			}
	   		strncpy(pRtPolicyInfo[i].szIPDestAddress, attrValue, IFC_SMALL_LEN);
		}
	}
    /*start ：2008-11-11 panhuiming :vdf需求102（指定dscp值的报文通过指定pvc转发）*/
        else if (0 == strncmp(attrName, "dscp", strlen("dscp")))
        {
             if (xmlState.verify && attrName[0] != '\0')
             {
                 status = BcmDb_validateRange(attrValue, 0, RT_MAX_DSCP - 1);
             }

              if (status != DB_OBJ_VALID_OK)
             {
                 xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      		 syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): dscp value is invalid: %s\n", attrValue);
		         fprintf(stdout, "xmlGetRtPolicyEntryObj(): dscp value is invalid: %s\n", attrValue);
				 return PSX_STS_ERR_FATAL;
             }
              else
             {
                 pRtPolicyInfo[i].nDscpValue = atoi(attrValue);
             }            
        }
    /*end：2008-11-11 panhuiming :vdf需求102（指定dscp值的报文通过指定pvc转发）*/

	else
	{
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		syslog(LOG_ERR, "xmlGetRtPolicyEntryObj(): invalid attrname %s\n", attrName);
		fprintf(stdout, "xmlGetRtPolicyEntryObj(): invalid attrname %s\n", attrName);
		return PSX_STS_ERR_FATAL;
	}

	return PSX_STS_OK;
}

PSX_STATUS xmlStartRtPolicyEntryObj(char *appName, char *objName)
{
	int i = xmlState.tableIndex;
	PIFC_RoutePolicy_Info pRtPolicyInfo = (PIFC_RoutePolicy_Info)psiValue;
	if ( psiValue == NULL ) 
	{
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		syslog(LOG_ERR,"xmlStartRtPolicyEntryObj(): psiValue is NULL\n");
		return PSX_STS_ERR_FATAL;
	}

	memset((char*)(pRtPolicyInfo + i), 0, sizeof(IFC_RoutePolicy_Info));
	return PSX_STS_OK;
}

PSX_STATUS xmlEndRtPolicyEntryObj(char *appName, char* objName)
{
	if (xmlState.verify) 
	{
	    if ( psiValue == NULL ) 
		{
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        xmlState.tableIndex = 0;
	        return (PSX_STS_ERR_FATAL);
	    }  
	}
	xmlState.tableIndex++;
	return PSX_STS_OK;
}
#endif
/* j00100803 Add End 2008-02-23 */
#if SUPPORT_PORT_MAP
PSX_STATUS xmlSetPMapObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(pmapObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(pmapObjs[id].setAttrFnc) != NULL)
     (*(pmapObjs[id].setAttrFnc))(file, pmapObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}
#endif

PSX_STATUS xmlSetSecObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(secObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(secObjs[id].setAttrFnc) != NULL)
     (*(secObjs[id].setAttrFnc))(file, secObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlSetLanNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
  switch (id)
     {
     case IFC_ENET_ID:
     case IFC_USB_ID:
     case IFC_HPNA_ID:
     case IFC_SUBNET_ID:
     case INSTID_OBJID:
       xmlSetLanAttr(file,appName,id,value,length);
       break;
     case IFC_WIRELESS_ID:       
     default:
#ifdef ETH_CFG
       if( id >= ETH_CFG_ID && id < ETH_CFG_ID + IFC_SUBNET_ID )
         xmlSetEthAttr(file,appName,id,value,length);
#endif
       /* unknown id */
       break;
     } /* switch */
   return PSX_STS_OK;
}

#ifdef WIRELESS
PSX_STATUS xmlSetWirelessNode(FILE* file, const char *appName, const UINT16 id,
			      const PSI_VALUE value, const UINT16 length)
{
  UINT16 size;

  size = sizeof(wirelessObjs) / sizeof(PSX_OBJ_ITEM);

  if ( id >= size ) {
#ifdef XML_DEBUG
    fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
    return PSX_STS_ERR_FIND_HDL_FNC;
  }
  
  if (*(wirelessObjs[id].setAttrFnc) != NULL)
    (*(wirelessObjs[id].setAttrFnc))(file, wirelessObjs[id].objName, id,  value, length);
  return PSX_STS_OK;
}
#endif /* #ifdef WIRELESS */

PSX_STATUS xmlSetWanNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
   PWAN_CON_INFO wan;
#if SUPPORT_ETHWAN
   static char* protocol[] = { "PPPOE", "PPPOA", "MER", "BRIDGE",
#ifdef VDF_PPPOU
                               "PPPOE_RELAY", "IPOA", "IPoW", "PPPOU",//modify by sxg, pppousb; NONE -> PPPOU
#else
                               "PPPOE_RELAY", "IPOA", "IPoW", "NONE",
#endif
                               "NONE", "NONE", "NONE"};
#else
   static char* protocol[] = { "PPPOE", "PPPOA", "MER", "BRIDGE",
#ifdef VDF_PPPOU
                               "PPPOE_RELAY", "IPOA", "NONE", "PPPOU",//modify by sxg, pppousb; NONE -> PPPOU
#else
                               "PPPOE_RELAY", "IPOA", "NONE", "NONE",
#endif
                               "NONE", "NONE", "NONE"};
#endif
    // encapMode for other protocols: LLC==0, vcMux==1
   static char* encapMode[] = {"LLC", "VCMUX"};
   // encapMode for pppoa: vcMux is the default and should = 0, LLC=1, so swap
   static char* encapModeForPppoa[] = {"VCMUX", "LLC"};
   static char* state[] = { "disable", "enable" };
   char mode[IFC_TINY_LEN];
	/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
	char primary[IFC_TINY_LEN], secondary[IFC_TINY_LEN], gateway[IFC_TINY_LEN];
	/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   wan = (PWAN_CON_INFO)value;

   if ( wan->protocol != PROTO_PPPOA )
      strcpy(mode, encapMode[wan->encapMode]);
   else
      strcpy(mode, encapModeForPppoa[wan->encapMode]);

#if !defined(SUPPORT_VLAN)
   wan->vlanId = -1;
#endif
	/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 AU8D00758 */
	memset(primary, 0, sizeof(primary));
	memset(secondary, 0, sizeof(secondary));
	memset(gateway, 0, sizeof(gateway));
	strncpy(primary, inet_ntoa(wan->stDns.preferredDns), IFC_TINY_LEN);
	strncpy(secondary, inet_ntoa(wan->stDns.alternateDns), IFC_TINY_LEN);	  
	strncpy(gateway, inet_ntoa(wan->ipGateWay), IFC_TINY_LEN);
/*start of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
    #if 0
    fprintf(file, "<entry%d vccId=\"%d\" conId=\"%d\" name=\"%s\" protocol=\"%s\" encap=\"%s\" firewall=\"%s\" nat=\"%s\" igmp=\"%s\" vlanId=\"%d\" service=\"%s\" instanceId=\"%lu\" %s=\"%s\" %s=\"%s\" %s=\"%s\" gw=\"%s\"/>\n",
           id, wan->vccId, wan->conId, wan->conName, protocol[wan->protocol], mode,
           state[wan->flag.firewall], state[wan->flag.nat], state[wan->flag.igmp], wan->vlanId,
           state[wan->flag.service], wan->id, 
			PSX_ATTR_DNS_PRIMARY, 
			primary, 
			PSX_ATTR_DNS_SECONDARY, 
			secondary, 
			PSX_ATTR_DNS_DOMAIN, 
			wan->stDns.domainName,
			gateway);
    #endif
    fprintf(file, "<entry%d vccId=\"%d\" conId=\"%d\" name=\"%s\" protocol=\"%s\" encap=\"%s\" firewall=\"%s\" nat=\"%s\" vlanId=\"%d\" service=\"%s\" instanceId=\"%lu\" %s=\"%s\" %s=\"%s\" %s=\"%s\" gw=\"%s\"/>\n",
           id, wan->vccId, wan->conId, wan->conName, protocol[wan->protocol], mode,
           state[wan->flag.firewall], state[wan->flag.nat], wan->vlanId,
           state[wan->flag.service], wan->id, 
			PSX_ATTR_DNS_PRIMARY, 
			primary, 
			PSX_ATTR_DNS_SECONDARY, 
			secondary, 
			PSX_ATTR_DNS_DOMAIN, 
			wan->stDns.domainName,
			gateway);
   /*end of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
	/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 AU8D00758 */

   return PSX_STS_OK;
}

PSX_STATUS xmlSetAtmVccNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   PIFC_ATM_VCC_INFO vcc;
   static char* aalType[] = { "AAL2", "AAL_TRANSPARENT", "AAL0_PKT", "AAL0_CELL", "UNKNOWN", "UNKNOWN", "UNKNOWN", "AAL5"};
   static char* adminStatus[] = {"", "up", "down", "testing"};
   static char* encap[] = { "", "vcMuxRouted", "vcMuxBr8023", "vcMuxBr8025", "vcMuxBr8026", "vcMuxLanEmu8023", "vcMuxLanEmu8025", "llc", "frameRelaySscs", "other", "unknown" };
   static char* state[] = { "disable", "enable" };

   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   vcc = (PIFC_ATM_VCC_INFO)value;

   /* start of protocol QoS for TdE存储限速数据 by z45221 zhangchen 2006年8月14日
   fprintf(file, "<vccId%d vpi=\"%d\" vci=\"%d\" tdId=\"%d\" aalType=\"%s\" adminStatus=\"%s\" encap=\"%s\" qos=\"%s\" instanceId=\"%lu\"/>\n",
	   id, (int)vcc->vccAddr.usVpi,(int)vcc->vccAddr.usVci,(int)vcc->tdId,
	   aalType[vcc->aalType], adminStatus[vcc->adminStatus], encap[vcc->encapMode],
	   state[vcc->enblQos], vcc->id);
   */
#ifndef SUPPORT_TDE_QOS
   fprintf(file, "<vccId%d vpi=\"%d\" vci=\"%d\" tdId=\"%d\" aalType=\"%s\" adminStatus=\"%s\" encap=\"%s\" qos=\"%s\" instanceId=\"%lu\"/>\n",
	   id, (int)vcc->vccAddr.usVpi,(int)vcc->vccAddr.usVci,(int)vcc->tdId,
	   aalType[vcc->aalType], adminStatus[vcc->adminStatus], encap[vcc->encapMode],
	   state[vcc->enblQos], vcc->id);
#else
   fprintf(file, "<vccId%d vpi=\"%d\" vci=\"%d\" tdId=\"%d\" aalType=\"%s\" adminStatus=\"%s\" encap=\"%s\" qos=\"%s\" instanceId=\"%lu\" QosIfcRate=\"%lu\" QosIfcCeil=\"%lu\"/>\n",
	   id, (int)vcc->vccAddr.usVpi,(int)vcc->vccAddr.usVci,(int)vcc->tdId,
	   aalType[vcc->aalType], adminStatus[vcc->adminStatus], encap[vcc->encapMode],
	   state[vcc->enblQos], vcc->id, vcc->QosIfcRate, vcc->QosIfcCeil);
#endif
   /* end of protocol QoS for TdE存储限速数据 by z45221 zhangchen 2006年8月14日 */

   return PSX_STS_OK;
}

PSX_STATUS xmlSetAtmTdNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   PIFC_ATM_TD_INFO td;

   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   td = (PIFC_ATM_TD_INFO)value;

   fprintf(file, "<td%d cat=\"%s\" PCR=\"%lu\" SCR=\"%lu\" MBS=\"%lu\"/>\n",
	   id, td->serviceCategory, td->peakCellRate, td->sustainedCellRate,
	   td->maxBurstSize);

   return PSX_STS_OK;
}

PSX_STATUS xmlSetAtmCfgNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(atmCfgObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(atmCfgObjs[id].setAttrFnc) != NULL)
     (*(atmCfgObjs[id].setAttrFnc))(file, atmCfgObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlSetAdslObjNode(FILE* file, const char *appName, const UINT16 id,
			     const PSI_VALUE value, const UINT16 length)
{
   unsigned int adslConfig;
   int dmt=0, lite=0, t1=0, adsl2=0, annexl=0, adsl2p=0, annexm=0, pair=0, bitswap=1, sra=0;
   static char* pairStr[] = {"inner", "outer"};
   static char* state[] = {"disable","enable"};

   memcpy(&adslConfig,value,sizeof(adslConfig));
   if (adslConfig&ANNEX_A_MODE_GDMT)
     dmt=1;
   if (adslConfig&ANNEX_A_MODE_GLITE)
     lite=1;
   if (adslConfig&ANNEX_A_MODE_T1413)
     t1=1;
   if (adslConfig&ANNEX_A_MODE_ADSL2)
     adsl2=1;
   if (adslConfig&ANNEX_A_MODE_ANNEXL)
     annexl=1;
   if (adslConfig&ANNEX_A_MODE_ADSL2PLUS)
     adsl2p=1;
   if (adslConfig&ANNEX_A_MODE_ANNEXM)
     annexm=1;
   if (adslConfig&ANNEX_A_LINE_PAIR_OUTER)
     pair=1;
   if (adslConfig&ANNEX_A_BITSWAP_DISENABLE)
     bitswap=0;
   if (adslConfig&ANNEX_A_SRA_ENABLE)
     sra=1;
   fprintf(file,"<settings G.Dmt=\"%s\" G.lite=\"%s\" T1.413=\"%s\" ADSL2=\"%s\" AnnexL=\"%s\" ADSL2plus=\"%s\" AnnexM=\"%s\" pair=\"%s\" bitswap=\"%s\" SRA=\"%s\"/>\n",
	   state[dmt],state[lite],state[t1],state[adsl2],state[annexl],
	   state[adsl2p],state[annexm],pairStr[pair],state[bitswap],state[sra]);

   return PSX_STS_OK;
}

#ifdef SUPPORT_SNTP
PSX_STATUS xmlSetSntpNode(FILE* file, const char *appName, const UINT16 id,
			  const PSI_VALUE value, const UINT16 length)
{
   PNTP_CONFIG sntp;
   static char* state[] = { "disable", "enable" };

   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   sntp = (PNTP_CONFIG)value;
   int idx = sntp->ntp_enabled;

#if 0
   fprintf(file, "<cfg state=\"%s\" server1=\"%s\" server2=\"%s\" timezone=\"%s\" offset=\"%d\" useDst=\"%d\" dstStartMonth=\"%d\" dstStartDay=\"%d\" dstStartDayType=\"%d\" dstStartDayTimeofDay=\"%d\"/>\n",
	   state[sntp->ntp_enabled], sntp->ntpServer1, sntp->ntpServer2, sntp->timezone,
	   sntp->timezone_offset, (int)sntp->use_dst,(int)sntp->dst_start.month,(int)sntp->dst_start.day,
	   (int)sntp->dst_start.day_type,sntp->dst_start.timeofday,
	   (int)sntp->dst_end.month,(int)sntp->dst_end.day,
	   (int)sntp->dst_end.day_type,sntp->dst_end.timeofday);
#else
   /* start of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SNTP_AUTOCONF
//BEGIN:modify by zhourongfei to config 3rd ntpserver
   fprintf(file, "<cfg %s=\"%s\" CfgServer=\"%s\" server1=\"%s\" server2=\"%s\" server3=\"%s\" timezone=\"%s\" FailTime=\"%d\" SucTime=\"%d\" EnableSum=\"%d\"/>\n",
	   PSX_ATTR_STATE,state[idx], state[sntp->CfgServerEnable], sntp->ntpServer1, sntp->ntpServer2, sntp->ntpServer3, sntp->timezone, sntp->iFailTimes, sntp->iSucTimes, sntp->iEnableSum);
#else
   fprintf(file, "<cfg %s=\"%s\" server1=\"%s\" server2=\"%s\" server3=\"%s\" timezone=\"%s\" FailTime=\"%d\" SucTime=\"%d\" EnableSum=\"%d\"/>\n",
	   PSX_ATTR_STATE,state[idx], sntp->ntpServer1, sntp->ntpServer2, sntp->ntpServer3, sntp->timezone, sntp->iFailTimes, sntp->iSucTimes, sntp->iEnableSum);
//END:modify by zhourongfei to config 3rd ntpserver
#endif
   /* end of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
#endif
   //printf("ntp Server2 xmlSetSntpNode(-----------------------------): %s\n",sntp->ntpServer2);
   return PSX_STS_OK;
}
#endif /* support_sntp */

PSX_STATUS xmlSetPppNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
   PWAN_PPP_INFO ppp;
   static char* state[] = {"disable","enable"};
   static char* method[] = {"auto", "PAP", "CHAP", "MSCHAP"};
   char *base64 = NULL;

   char pppLocalIpAddress[IFC_TINY_LEN];
   char pppVoBitstreamIPAddr[IFC_TINY_LEN];
   memset(pppLocalIpAddress, 0, IFC_TINY_LEN);
   memset(pppVoBitstreamIPAddr, 0, IFC_TINY_LEN);
   
   if ((appName == NULL) || (value == NULL))
      return PSX_STS_ERR_GENERAL;

   ppp = (PWAN_PPP_INFO)value;
   strcpy(pppLocalIpAddress, inet_ntoa(ppp->pppLocalIpAddress));
   strcpy(pppVoBitstreamIPAddr, inet_ntoa(ppp->pppVoBitstreamIPAddr));
   /* BEGIN: Added by y67514, 2008/9/8   问题单号:GLB:增加参数mtu，mixed*/
   //#ifdef SUPPORT_PASSWORD_UNENCRYPTED
   //BEGIN:modify by zhourongfei to config number of PPP keepalive attempts
   #ifdef SUPPORT_PPPSRVPWD_UNENCRYPTED //modified by w00135351 09.3.14
   fprintf(file,"<ppp_conId%d userName=\"%s\" password=\"%s\" serviceName=\"%s\" idleTimeout=\"%d\" pppNumber=\"%d\" ipExt=\"%s\" auth=\"%s\" useStaticIpAddr=\"%d\" localIpAddr=\"%s\" Debug=\"%s\" ProName=\"%s\" automanualConnect=\"%d\" delayRedial=\"%d\" timeout=\"%d\" delayTime=\"%d\" BridgeMixed=\"%s\" Mtu=\"%d\" LcpEchoInterval=\"%d\" IpExtMac=\"%s\" enblpppVoB=\"%s\" pppVoBitstreamIPAddr=\"%s\"/>\n",
        id, ppp->userName,ppp->password,ppp->serverName,ppp->idleTimeout,ppp->pppNumber,
        state[ppp->enblIpExtension], method[ppp->authMethod], ppp->useStaticIpAddress, pppLocalIpAddress,
        state[ppp->enblDebug], ppp->acProName, ppp->automanualConnect, ppp->delayRedial, ppp->timeout, ppp->delayTime, state[ppp->enblBridgeMixed],ppp->Mtu,ppp->lcpEchoInterval,ppp->ipExtMac,state[ppp->enblpppVoB],pppVoBitstreamIPAddr); //modified by l66195 for pppousb 
   #else
   base64Encode((unsigned char *)ppp->password, strlen(ppp->password), &base64);

   fprintf(file,"<ppp_conId%d userName=\"%s\" password=\"%s\" serviceName=\"%s\" idleTimeout=\"%d\" pppNumber=\"%d\" ipExt=\"%s\" auth=\"%s\" useStaticIpAddr=\"%d\" localIpAddr=\"%s\" Debug=\"%s\" ProName=\"%s\" automanualConnect=\"%d\" delayRedial=\"%d\" timeout=\"%d\" delayTime=\"%d\" BridgeMixed=\"%s\" Mtu=\"%d\" LcpEchoInterval=\"%d\" IpExtMac=\"%s\" enblpppVoB=\"%s\" pppVoBitstreamIPAddr=\"%s\"/>\n",
        id, ppp->userName,base64,ppp->serverName,ppp->idleTimeout,ppp->pppNumber,
        state[ppp->enblIpExtension], method[ppp->authMethod], ppp->useStaticIpAddress, pppLocalIpAddress,
        state[ppp->enblDebug], ppp->acProName, ppp->automanualConnect, ppp->delayRedial, ppp->timeout, ppp->delayTime, state[ppp->enblBridgeMixed],ppp->Mtu,ppp->lcpEchoInterval,ppp->ipExtMac,state[ppp->enblpppVoB],pppVoBitstreamIPAddr); //modified by l66195 for pppousb 
           
   free(base64);
   #endif
   //END:modify by zhourongfei to config number of PPP keepalive attempts
   /* END:   Added by y67514, 2008/9/8 */
   return PSX_STS_OK;
}

#ifdef SUPPORT_TOD
PSX_STATUS xmlSetToDNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
   int size;

   size = sizeof(ToDObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(ToDObjs[id].setAttrFnc) != NULL)
      (*(ToDObjs[id].setAttrFnc))(file, ToDObjs[id].objName, id,  value, length);

   return PSX_STS_OK;
}
#endif /* #ifdef SUPPORT_TOD */

#ifdef SUPPORT_MAC // add by l66195 for VDF
PSX_STATUS xmlSetMacNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
   int size;

   size = sizeof(MacObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(MacObjs[id].setAttrFnc) != NULL)
      (*(MacObjs[id].setAttrFnc))(file, MacObjs[id].objName, id,  value, length);

   return PSX_STS_OK;
}
#endif /* #ifdef SUPPORT_MAC */

#ifdef SUPPORT_MACMATCHIP 
PSX_STATUS xmlSetMacMatchNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
   int size;

   size = sizeof(MacMatchObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(MacMatchObjs[id].setAttrFnc) != NULL)
      (*(MacMatchObjs[id].setAttrFnc))(file, MacMatchObjs[id].objName, id,  value, length);

   return PSX_STS_OK;
}
#endif 

#ifdef SUPPORT_DDNSD
PSX_STATUS xmlSetDdnsNode(FILE* file, const char *appName, const UINT16 id,
			  const PSI_VALUE value, const UINT16 length)
{
   int size;

   size = sizeof(DDnsObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(DDnsObjs[id].setAttrFnc) != NULL)
      (*(DDnsObjs[id].setAttrFnc))(file, DDnsObjs[id].objName, id,  value, length);

   return PSX_STS_OK;
}
#endif /* #ifdef SUPPORT_DDNSD */

PSX_STATUS xmlSetDhcpClntNode(FILE* file, const char *appName, const UINT16 id,
			      const PSI_VALUE value, const UINT16 length)
{
   PWAN_IP_INFO dhcp;
   static char* state[] = {"disable","enable"};
   char addr[IFC_TINY_LEN];

   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   dhcp = (PWAN_IP_INFO)value;

   strncpy(addr,inet_ntoa(dhcp->wanAddress),IFC_TINY_LEN);
   #ifdef SUPPORT_GLB_MTU
   fprintf(file, "<dhcpc_conId%d %s=\"%s\" dhcpClassIdentifier=\"%s\" mtu=\"%d\" wanAddress=\"%s\" ",
	   id, PSX_ATTR_STATE, state[dhcp->enblDhcpClnt], dhcp->dhcpClassIdentifier, dhcp->mtu,addr);
   #else
   /* start of maintain PSI移植:  增加dhcp option60的描述 by xujunxia 43813 2006年5月8日"
   fprintf(file, "<dhcpc_conId%d %s=\"%s\" wanAddress=\"%s\" ",
	   id,PSX_ATTR_STATE,state[dhcp->enblDhcpClnt], addr);
  */
   fprintf(file, "<dhcpc_conId%d %s=\"%s\" dhcpClassIdentifier=\"%s\" wanAddress=\"%s\" ",
	   id, PSX_ATTR_STATE, state[dhcp->enblDhcpClnt], dhcp->dhcpClassIdentifier, addr);
   /* end of maintain PSI移植:  增加dhcp option60的描述 by xujunxia 43813 2006年5月8日 */
    #endif /* SUPPORT_GLB_MTU */
   strncpy(addr,inet_ntoa(dhcp->wanMask),IFC_TINY_LEN);
   fprintf(file, "wanMask=\"%s\"/>\n",addr);
   return PSX_STS_OK;
}
#ifdef SUPPORT_DHCPOPTIONS
PSX_STATUS xmlStartDhcpOptionNode(char *appName, char *objName)
{
    PDHCPOptions pstDhcpOption;

#ifdef XML_DEBUG_xjx
    printf("xmlStartDhcpOptionNode(calling cfm to get appName %s, objName %s\n",
              appName, objName);
#endif

    psiValue = (void*)malloc(sizeof(DHCPOptions));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset((char*)psiValue, 0, sizeof(DHCPOptions));
    pstDhcpOption = (PDHCPOptions)psiValue;

    return PSX_STS_OK;
}
PSX_STATUS xmlSetDhcpOptionNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length)
{
    PDHCPOptions pstDhcpOption;
    
    if ((NULL == appName) || (NULL == value))
    {
        return PSX_STS_ERR_GENERAL;
    }
    
    pstDhcpOption = (PDHCPOptions)value;

    //w44771 modify for A36D02779, 增加option 241 和option 242
    fprintf(file, "<dhcp_option option240=\"%s\" option241=\"%s\" option242=\"%s\" option243=\"%s\" option244=\"%s\" option245=\"%s\"/>\n", 
               pstDhcpOption->option240, pstDhcpOption->option241, pstDhcpOption->option242, pstDhcpOption->option243, pstDhcpOption->option244, pstDhcpOption->option245);

    return PSX_STS_OK;
}
PSX_STATUS xmlGetDhcpOptionNode(char *appName, char *objName, 
                char *attrName, char* attrValue)
{
    PDHCPOptions pstDhcpOption = (PDHCPOptions)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG_xjx
    printf("xmlGetDhcpOptionNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif

    if (NULL == psiValue)
    {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetDhcpOptionNode(): psiValue is NULL\n");
    }
    //w44771修改校验长度为127并增加校验option 241和option 242
    if (0 == strncmp(attrName, "option240", strlen("option240"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option240 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option240, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "option241", strlen("option241"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option241 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option241, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "option242", strlen("option242"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
           printf("pstDhcpOption->option242 error!\n");
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option242 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option242, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "option243", strlen("option243"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option243 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option243, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "option244", strlen("option244"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option244 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option244, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "option245", strlen("option245"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, 2*DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetDhcpOptionNode(): invalid option245 %s\n", attrValue);
        }
        else
        {
            strcpy(pstDhcpOption->option245, attrValue);
        }
    }
    return (PSX_STS_OK);
}
PSX_STATUS xmlEndDhcpOptionNode(char *appName, char *objName)
{
    PDHCPOptions pstDhcpOption = (PDHCPOptions)psiValue;
    
#ifdef XML_DEBUG_xjx
    printf("xmlEndDhcpOptionNode(calling cfm to get appName %s, objName %s)\n",
	 appName,objName);
#endif

    if (xmlState.verify) 
    {
        if(NULL == pstDhcpOption)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndDhcpOptionNode(): psiValue is NULL\n");
        }
    }
    else 
    {
        BcmDb_setDhcpOptionInfo(pstDhcpOption);
    }


#ifdef XML_DEBUG_xjx
  printf("\n============End auto upgrade info=========\n");
  printf("xmlEndDhcpOptionNode(): psiValue->option240 %s, option243 %s, option244 %s, option245 %s\n",
            pstDhcpOption->option240, pstDhcpOption->option243, pstDhcpOption->option244, pstDhcpOption->option245);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}   
#endif

/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlStartAutoUpgradeNode(char *appName, char *objName)
  描述: 解析文件时处理到自动升级字段，分配相应的内存
  输入: 应用名，对象名
  输出: 无
  返回值: 分配内存是否成功
-------------------------------------------------------------*/
PSX_STATUS xmlStartAutoUpgradeNode(char *appName, char *objName)
{
    PAUTO_UPGRADE_INFO pstAutoUpgrade;

#ifdef XML_DEBUG
    printf("xmlStartAutoUpgradeNode(calling cfm to get appName %s, objName %s\n",
              appName, objName);
#endif

    psiValue = (void*)malloc(sizeof(AUTO_UPGRADE_INFO));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset((char*)psiValue, 0, sizeof(AUTO_UPGRADE_INFO));
    pstAutoUpgrade = (PAUTO_UPGRADE_INFO)psiValue;

    return PSX_STS_OK;
}
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlSetAutoUpgradeNode(FILE* file, const char *appName, const UINT16 id,
  描述: 
  输入: 
  输出: 
  返回值: 
-------------------------------------------------------------*/
PSX_STATUS xmlSetAutoUpgradeNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length)
{
    PAUTO_UPGRADE_INFO pstAutoUpgrade;
    static char* state[] = {"disable","enable"};

    if ((NULL == appName) || (NULL == value))
    {
        return PSX_STS_ERR_GENERAL;
    }
    
    pstAutoUpgrade = (PAUTO_UPGRADE_INFO)value;

    fprintf(file, "<cfg CustomUpdate=\"%s\" UpdateServer=\"%s\" VDFName=\"%s\"/>\n", 
               state[pstAutoUpgrade->ulCustomUpdate], pstAutoUpgrade->szUpdateServer, pstAutoUpgrade->szVDFName);

    return PSX_STS_OK;
}
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlGetAutoUpgradeNode(char *appName, char *objName, 
  描述: 
  输入: 
  输出: 
  返回值: 
-------------------------------------------------------------*/
PSX_STATUS xmlGetAutoUpgradeNode(char *appName, char *objName, 
                char *attrName, char* attrValue)
{
    PAUTO_UPGRADE_INFO pstAuUpgrade = (PAUTO_UPGRADE_INFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetAutoUpgradeNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif

    if (NULL == psiValue)
    {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetAutoUpgradeNode(): psiValue is NULL\n");
    }
    if (0 == strncmp(attrName, "CustomUpdate", strlen("CustomUpdate"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateState(attrValue);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAutoUpgradeNode(): invalid CustomUpdate %s\n", attrValue);
        }
        else
        {
            pstAuUpgrade->ulCustomUpdate = xmlStateToPsi(attrValue);
        }
    }
    else if (0 == strncmp(attrName, "UpdateServer", strlen("UpdateServer"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, IFC_MAX_SERVER_LEN);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAutoUpgradeNode(): invalid UpdateServer %s\n", attrValue);
        }
        else
        {
            strcpy(pstAuUpgrade->szUpdateServer, attrValue);
        }
    }
    else if (0 == strncmp(attrName, "VDFName", strlen("VDFName"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, IFC_MAX_VDF_LEN);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAutoUpgradeNode(): invalid VDFName %s\n", attrValue);
        }
        else
        {
            strcpy(pstAuUpgrade->szVDFName, attrValue);
        }
    }
    return (PSX_STS_OK);
}

/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlEndAutoUpgradeNode(char *appName, char *objName)
  描述: 
  输入: 
  输出: 
  返回值: 
-------------------------------------------------------------*/
PSX_STATUS xmlEndAutoUpgradeNode(char *appName, char *objName)
{
    PAUTO_UPGRADE_INFO pstAutoUpgrade = (PAUTO_UPGRADE_INFO)psiValue;
    
#ifdef XML_DEBUG
    printf("xmlEndAutoUpgradeNode(calling cfm to get appName %s, objName %s)\n",
	 appName,objName);
#endif

    if (xmlState.verify) 
    {
        if(NULL == pstAutoUpgrade)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndAutoUpgradeNode(): psiValue is NULL\n");
        }
    }
    else 
    {
        BcmDb_setAutoUpgradeInfo(pstAutoUpgrade);
    }


#ifdef XML_DEBUG
  printf("\n============End auto upgrade info=========\n");
  printf("xmlEndAutoUpgradeNode(): psiValue->ulCustomUpdate %ld, szUpdateServer %s, szVDFName %s\n",
            pstAutoUpgrade->ulCustomUpdate, pstAutoUpgrade->szUpdateServer, pstAutoUpgrade->szVDFName);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

/* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */

/* start of  增加 tr143 功能2009.11.7*/
/*start of download*/
PSX_STATUS xmlStartDownloadDiagnosticsObj(char *appName, char *objName)
{
	#ifdef XML_DEBUG
			printf("xmlStartDownloadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
     		appName,objName);
	#endif
	psiValue = (void*)malloc(sizeof(DIAG_TR143_Down));
	if(NULL== psiValue)
	{
		return PSX_STS_ERR_MEMORY;
	}
	memset((char *)psiValue,0,sizeof(DIAG_TR143_Down));
	
	return PSX_STS_OK;
}

void xmlSetDownloadDiagnosticsAttr(FILE* file, const char *Name, const UINT16 id,
	               const PSI_VALUE value, const UINT16 length)
{

#ifdef XML_DEBUG
  printf("xmlSetDownloadDiagnosticsAttr(attrName %s, attrValue %s)\n",
	 Name,value);
#endif

	DIAG_TR143_Down *node = (DIAG_TR143_Down *)value;
	if( 0 == strcmp( Name, "DownloadDiagnostics" ) )
	{
		fprintf(file,"<%s DiagnosticsState=\"%s\"  Interface=\"%s\"   DownloadURL=\"%s\"  DSCP=\"%d\"  EthernetPriority=\"%d\"/>\n" ,
				Name, node->diag_state,node->diag_inter,node->diag_downURL,node->diag_dscp,node->diag_etherPriority);
	}	
}

PSX_STATUS xmlGetDownloadDiagnosticsObj(char *attrName,char* attrValue)
{ 
#ifdef XML_DEBUG
  printf("xmlGetUDPEchoConfigAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

	DIAG_TR143_Down *node = (DIAG_TR143_Down *)psiValue;
 	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

	if( 0 == strcmp( attrName, "DiagnosticsState" ) )
	{

		 status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid DiagnosticsState %s\n",attrValue);
		}
		else
		{
	 	strncpy( node->diag_state, attrValue, strlen(attrValue) );
		}
	}
	else if(0 == strcmp( attrName, "Interface" ))
	{
		 status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid downInterface %s\n",attrValue);
		}
		else
		{
	 		strncpy(node->diag_inter, attrValue, strlen(attrValue) );
		}
		
	}
	else if( 0 == strcmp( attrName, "DownloadURL" ) )
	{
		 status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid DownloadURL %s\n",attrValue);
		}
		else
		{
	 	strncpy( node->diag_downURL, attrValue, strlen(attrValue) );
		}
		
	}
	else if( 0 == strcmp( attrName, "DSCP") )
	{
		if(atoi(attrValue) > 63 || atoi(attrValue) < 0 )
		{
				syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid DownloadDSCP %d\n",atoi(attrValue));
				return PSX_STS_ERR_GENERAL ;	
		}
		else
		{
	 			node->diag_dscp= atoi(attrValue);
		}
	}
	else if (0 == strcmp(attrName,"EthernetPriority"))
	{
		if(atoi(attrValue) > 7 || atoi(attrValue) < 0 )
		{
				syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid EthernetPriority %d\n",atoi(attrValue));
				return PSX_STS_ERR_GENERAL;	
		}
		else
		{
	 			node->diag_etherPriority= atoi(attrValue );
		}
	
	}
	return PSX_STS_OK;

}

PSX_STATUS xmlEndDownloadDiagnosticsObj(char *appName, char *objName)
{

      PSI_STATUS status;
	#ifdef XML_DEBUG
  		printf("xmlEndDownloadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
	 		appName,objName);
	#endif

       DIAG_TR143_Down* pstState_node = (DIAG_TR143_Down *)psiValue;
	
	PSI_HANDLE sysPsi = BcmPsi_appOpen(TR143_APPID);

       status = BcmPsi_objStore(sysPsi, DownloadDiagnostics_ID, pstState_node, sizeof(DIAG_TR143_Down));
	
 	free(psiValue);
  	psiValue = NULL;
  	return PSX_STS_OK;
}



PSX_STATUS xmlStartUploadDiagnosticsObj(char *appName, char *objName)
{
	#ifdef XML_DEBUG
		printf("xmlStartUploadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
     		appName,objName);
	#endif
	psiValue = (void*)malloc(sizeof(DIAG_TR143_UP));
	if(NULL == psiValue)
	{
		return PSX_STS_ERR_MEMORY;
	}
	memset((char *)psiValue,0,sizeof(DIAG_TR143_UP));
	return PSX_STS_OK;
}

void xmlSetUploadDiagnosticsObj(FILE* file, const char *Name, const UINT16 id,
	               const PSI_VALUE value, const UINT16 length)
{

	#ifdef XML_DEBUG
		printf("xmlSetUploadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
     		appName,objName);
	#endif
	
	DIAG_TR143_UP *node = (DIAG_TR143_UP *)value;
	
	if( 0 == strcmp( Name, "UploadDiagnostics" ) )
	{
		fprintf(file,"<%s DiagnosticsState=\"%s\"  Interface=\"%s\"   UploadURL=\"%s\"  DSCP=\"%d\"  EthernetPriority=\"%d\"  TestFileLength=\"%d\"/>\n",
			    Name, node->diag_state_up,node->diag_inter_up,node->diag_upURL,node->diag_dscp_up,node->diag_ether_up,node->diag_test_up);
	}	

}

 PSX_STATUS xmlGetUploadDiagnosticsObj( char *attrName, char* attrValue)
 {
	
	#ifdef XML_DEBUG
  printf("xmlGetUDPEchoConfigAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

	DIAG_TR143_UP *node = (DIAG_TR143_UP *)psiValue;
 	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

	if( 0 == strcmp( attrName, "DiagnosticsState" ) )
	{
		status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid DiagnosticsState %s\n",attrValue);
		}
		else
		{
	 	strncpy( node->diag_state_up, attrValue, strlen(attrValue) );
		}
		
	}
	else if( 0 == strcmp( attrName , "Interface" ))
	{
		status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlGetDownloadDiagnosticsObj(): invalid DiagnosticsState %s\n",attrValue);
		}
		else
		{
	 	strncpy(node->diag_inter_up, attrValue, strlen(attrValue) );
		}
		
	}
	else if( 0 == strcmp( attrName, "UploadURL" ) )
	{
		status = BcmDb_validateLength(attrValue,256);
		if(status !=DB_OBJ_VALID_OK)
		{
			syslog(LOG_ERR,"xmlUPDiagnosticsObj(): invalid DiagnosticsState %s\n",attrValue);
		}
		else
		{
	 	strncpy( node->diag_upURL, attrValue, strlen(attrValue) );
		}
		
	}
	else if( 0 == strcmp( attrName, "DSCP" ) )
	{
		if(atoi(attrValue) > 63 || atoi(attrValue) < 0 )
		{
				syslog(LOG_ERR,"xmlUPDiagnosticsObj(): invalid UPloadDSCP %d\n",atoi(attrValue));
				return PSX_STS_ERR_GENERAL;	
		}
		else
		{
	 			node->diag_dscp_up= atoi(attrValue); ;
		}
	}
	else if (0 == strcmp(attrName,"EthernetPriority"))
	{
		if(atoi(attrValue) > 7 || atoi(attrValue) < 0 )
		{
				syslog(LOG_ERR,"xmlUPDiagnosticsObj(): invalid UPloadEthernetPriority %d\n",atoi(attrValue));
				return PSX_STS_ERR_GENERAL;	
		}
		else
		{
	 			node->diag_ether_up= atoi(attrValue); ;
		}
		
	}
	else if ( 0 == strcmp(attrName,"TestFileLength"))
	{
		node->diag_test_up= atoi(attrValue);;
	}
	return PSX_STS_OK;
 }
 
PSX_STATUS xmlEndUploadDiagnosticsObj(char *appName, char *objName)
{

	#ifdef XML_DEBUG
  		printf("xmlEndUploadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
	 		appName,objName);
	#endif
	DIAG_TR143_UP* pstState_node = (DIAG_TR143_UP *)psiValue;
	
	PSI_HANDLE sysPsi = BcmPsi_appOpen(TR143_APPID);
	if(NULL == sysPsi)
	{
		return PSX_STS_ERR_GENERAL;
	}
       BcmPsi_objStore(sysPsi, UploadDiagnostics_ID, pstState_node, sizeof(DIAG_TR143_UP));
	 free(psiValue);
  	psiValue = NULL;
  	return PSX_STS_OK;
}

/*start of UDPEchoConfig*/
void  xmlSetUDPEchoConfigAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{ 

 
   DIAG_TR143_UDP *flag;
   //static char* state[] = {"disable","enable"};
   if ((name == NULL) || (value == NULL))
      return ;
   flag = (DIAG_TR143_UDP*)value;
  
  fprintf(file,"<%s Enable=\"%d\" Interface=\"%s\" SourceIPAddress =\"%s\"  UDPPort=\"%d\" EchoPlusEnabled=\"%d\"/>\n",
              name,flag->Enable,flag->Interface,flag->SourceIPAddress, flag->UDPPort,flag->EchoPlusEnabled);

   //  return PSX_STS_OK;
}

PSX_STATUS xmlGetUDPEchoConfigAttr(char *attrName, char* attrValue)
{

  DIAG_TR143_UDP *flag = (DIAG_TR143_UDP*)psiValue;
   DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetUDPEchoConfigAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif


  if (status != DB_OBJ_VALID_OK) 
    xmlState.errorFlag = PSX_STS_ERR_FATAL;

  if (strncmp(attrName,"Enable",strlen("Enable")) == 0) 
 {
    if (xmlState.verify)
      //status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetUDPEchoConfigAttr(): invalid state %s\n",attrValue);
    }
    else
      flag->Enable= atoi(attrValue);
  }

else if (strncmp(attrName,"Interface",strlen("Interface")) == 0)
{
    if( xmlState.verify)
         status = BcmDb_validateLength(attrValue,256);
    if( status != DB_OBJ_VALID_OK){
        xmlState.errorFlag = PSX_STS_ERR_FATAL; 	
        syslog(LOG_ERR,"xmlGetUDPEchoConfigAttr(): invalid serverAddr %s\n",attrValue);
	} 
	else {
        strcpy(flag->Interface,attrValue);
	}
	
}
else if (strncmp(attrName,"SourceIPAddress",strlen("SourceIPAddress")) == 0) 
{

    if (xmlState.verify)
   {
      status = BcmDb_validateLength(attrValue, 64);
    }
    if (status != DB_OBJ_VALID_OK) 
		{	
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetUDPEchoConfigAttr(): invalid serverAddr %s\n",attrValue);
    }
    else
    	{		
      strcpy(flag->SourceIPAddress, attrValue);
    	}
}

else if (strncmp(attrName,"UDPPort",strlen("UDPPort")) == 0) 
{
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetUDPEchoConfigAttr(): invalid serverPort %s\n",attrValue);
    }
    else
    	{
      flag->UDPPort= atoi(attrValue);	  
    	}
}

else if (strncmp(attrName,"EchoPlusEnabled",strlen("EchoPlusEnabled")) == 0) 
{
    if (xmlState.verify)
      //status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetUDPEchoConfigAttr(): invalid state %s\n",attrValue);
    }
    else
      flag->EchoPlusEnabled = atoi(attrValue);
}
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartUDPEchoConfigObj(char *appName, char *objName)
{
	DIAG_TR143_UDP *flag;
	//flag = (DIAG_TR143_UDP )psiValue;

#ifdef XML_DEBUG
	printf("xmlStartUDPEchoConfigObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
	psiValue = (void*)malloc(sizeof(DIAG_TR143_UDP));
	if (psiValue == NULL) {
	  xmlMemProblem();
	  return PSX_STS_ERR_FATAL;
	}
	memset(psiValue, 0, sizeof(DIAG_TR143_UDP));
	flag = (DIAG_TR143_UDP * )psiValue;
	Bcm_Db_getUdpEchoDefaultInfo(flag);
	 return PSX_STS_OK;
}

PSX_STATUS xmlEndUDPEchoConfigObj(char *appName, char *objName)
{
	#ifdef XML_DEBUG
  		printf("xmlEndUploadDiagnosticsObj(calling cfm to get appName %s, objName %s\n",
	 		appName,objName);
	#endif
	DIAG_TR143_UDP* pstState_node = (DIAG_TR143_UDP *)psiValue;
	
	PSI_HANDLE sysPsi = BcmPsi_appOpen(TR143_APPID);
       BcmPsi_objStore(sysPsi, UDPEchoConfig_ID, pstState_node, sizeof(DIAG_TR143_UDP));
 	 free(psiValue);
  	psiValue = NULL;
  	return PSX_STS_OK;
}
/*end of UDPEchoConfig*/


static const PSX_OBJ_ITEM Tr143Objs[] = {
    { "*", NULL, NULL, NULL, NULL },
    {"DownloadDiagnostics", xmlSetDownloadDiagnosticsAttr, xmlGetDownloadDiagnosticsObj, xmlStartDownloadDiagnosticsObj, xmlEndDownloadDiagnosticsObj },
    {"UploadDiagnostics", xmlSetUploadDiagnosticsObj, xmlGetUploadDiagnosticsObj, xmlStartUploadDiagnosticsObj, xmlEndUploadDiagnosticsObj },
    {"UDPEchoConfig", xmlSetUDPEchoConfigAttr, xmlGetUDPEchoConfigAttr, xmlStartUDPEchoConfigObj, xmlEndUDPEchoConfigObj },
    { "*", NULL, NULL, NULL, NULL }
};

PSX_STATUS xmlStartTR143ObjNode(char *appName, char *objName)
{
    PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlStartTR143ObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; Tr143Objs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(Tr143Objs[i].objName, objName, strlen(Tr143Objs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(Tr143Objs[i].startObjFnc) != NULL) {
	sts = (*(Tr143Objs[i].startObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartTR143ObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
   }
   return sts;
}

PSX_STATUS xmlGetTR143ObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
     int i;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetTR143ObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 1; Tr143Objs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(Tr143Objs[i].objName, objName, strlen(Tr143Objs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(Tr143Objs[i].getAttrFnc) != NULL) {
	sts = (*(Tr143Objs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetTR143ObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

  }
  return sts;
}

PSX_STATUS xmlSetTR143ObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length) 
{
   UINT16 size = sizeof(Tr143Objs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(Tr143Objs[id].setAttrFnc) != NULL)
  {
     (*(Tr143Objs[id].setAttrFnc))(file, Tr143Objs[id].objName, id,  value, length);
  }
   return PSX_STS_OK;
}

PSX_STATUS xmlEndTR143ObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlEndObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; Tr143Objs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(Tr143Objs[i].objName, objName, strlen(Tr143Objs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(Tr143Objs[i].endObjFnc) != NULL) {
	sts = (*(Tr143Objs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndTR143ObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   return sts;
}



/* end of 增加tr143 功能2009.11.7 */
/* start of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlStartBrNode(char *appName, char *objName)
  描述: 分配相应的内存(结构)，为后继xml解析做准备。
  输入: 应用名和对象名
  输出: 全局变量指针psiValue
  返回值: 函数处理状态
-------------------------------------------------------------*/
PSX_STATUS xmlStartBrNode(char *appName, char *objName)
{
    PWAN_BRDHCPC_INFO brInfo;

#ifdef XML_DEBUG
    printf("xmlStartBrNode(calling cfm to get appName %s, objName %s\n",
              appName, objName);
#endif

    psiValue = (void*)malloc(sizeof(WAN_BRDHCPC_INFO));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset((char*)psiValue, 0, sizeof(WAN_BRDHCPC_INFO));
    brInfo = (PWAN_BRDHCPC_INFO)psiValue;

    return PSX_STS_OK;
}
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlSetBrNode(FILE* file, const char *appName, const UINT16 id,
  描述: 将对应结构中的属性值转换成xml格式
  输入:应用名，结构指针，文件指针
  输出: 无
  返回值: 函数处理状态
-------------------------------------------------------------*/
PSX_STATUS xmlSetBrNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length)
{
    PWAN_BRDHCPC_INFO brInfo;
    static char* state[] = {"disable","enable"};

    if ((NULL == appName) || (NULL == value))
    {
        return PSX_STS_ERR_GENERAL;
    }
    
    brInfo = (PWAN_BRDHCPC_INFO)value;

    fprintf(file, "<br_conId%d %s=\"%s\" dhcpClassIdentifier=\"%s\"/>\n", 
                id, PSX_ATTR_STATE, state[brInfo->enableDhcpc], brInfo->dhcpClassIdentifier);
    return PSX_STS_OK;
}

/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlGetBrAttr(char *attrName, char* attrValue)
  描述: 从xml文件中解析出属性值，填充到结构中
  输入: 属性名，属性值
  输出: 填充属性值到结构中对应的属性中。
  返回值: 函数处理状态
-------------------------------------------------------------*/
PSX_STATUS xmlGetBrAttr(char *attrName, char* attrValue)
{
    PWAN_BRDHCPC_INFO brInfo = (PWAN_BRDHCPC_INFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetBrAttr(attrName %s, attrValue %s)\n",
                attrName, attrValue);
#endif

    if (0 == strncmp(attrName, PSX_ATTR_STATE, strlen(PSX_ATTR_STATE)))
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateState(attrValue);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetBrAttr(): invalid state %s\n", attrValue);
        }
        else 
        {
            brInfo->enableDhcpc = xmlStateToPsi(attrValue);
        }
    }
    if (0 == strncmp(attrName, "dhcpClassIdentifier", strlen("dhcpClassIdentifier")))
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, DHCP_CLASS_ID_MAX);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetBrAttr(): invalid dhcpClassIdentifier %s\n", attrValue);
        }
        else
        {
            strcpy(brInfo->dhcpClassIdentifier, attrValue);
        }
    }
    return (PSX_STS_OK);
}
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlGetBrNode(char *appName, char *objName, char *attrName,
  描述: 获取桥节点的属性值
  输入: 应用名，对象名，属性名，属性值
  输出: 填充结构值
  返回值: 函数处理状态
-------------------------------------------------------------*/
PSX_STATUS xmlGetBrNode(char *appName, char *objName, char *attrName,
                        char* attrValue)
{
    PSX_STATUS sts;

#ifdef XML_DEBUG
    printf("xmlGetBrNode(appName %s, objName %s, attrName %s, attrValue %s\n",
                appName, objName, attrName, attrValue);
#endif

    sts = xmlGetBrAttr(attrName, attrValue);

    return sts;
} /* xmlGetBrNode */
/*------------------------------------------------------------
  函数原型: PSX_STATUS xmlEndBrNode(char *appName, char *objName)
  描述: xml解析节点结束后，将对应的值设置到业务模块，
                同时释放在start时分配的内存。
  输入: 应用名，对象名
  输出: 无
  返回值: 函数处理状态
-------------------------------------------------------------*/
PSX_STATUS xmlEndBrNode(char *appName, char *objName)
{
    PWAN_BRDHCPC_INFO brInfo = (PWAN_BRDHCPC_INFO)psiValue;
    WAN_CON_ID wanId;
    int vpi = 0, vci = 0, conId = 0;

#ifdef XML_DEBUG
    printf("xmlEndBrNode(calling cfm to get appName %s, objName %s\n",
                appName, objName);
#endif

    /* obtain vpi/vci from objName */
    sscanf(appName, APPNAME_BRSRV_ENTRY_FORMAT, &vpi, &vci);
    wanId.vpi = (UINT16)vpi;
    wanId.vci = (UINT16)vci;

    /* obtain conId from objId */
    sscanf(objName, OBJNAME_BRSRV_ENTRY_FORMAT, &conId);
    wanId.conId = (UINT16)conId;

    if (xmlState.verify)
    {
        if (xmlVerifyVccExist(wanId.vpi, wanId.vci) == PSX_STS_ERR_FATAL) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndBrNode(%s): brSrv PVC %d/%d doesn't exist in config file\n",
                                objName, wanId.vpi, wanId.vci);
        }
        if (xmlVerifyConIdProtocol(&wanId, "PROTO_BRIDGE") == PSX_STS_ERR_FATAL) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndBrNode(%s): wan_%d_%d conId %d running bridge doesn't exist in config file\n",
                        objName, wanId.vpi, wanId.vci, wanId.conId);
        }
    } /* verify */
    else if (PSX_STS_OK == xmlState.errorFlag)
    {
        BcmDb_setBrDhcpcInfo(&wanId, brInfo);
    }

#ifdef XML_DEBUG
    printf("\n============End BRSRV info=========\n");
    printf("xmlEndBrNode(): vpi %d, vci %d, conId %d, state %d dhcpClassIdentifier %s\n",
    wanId.vpi, wanId.vci, wanId.conId, brInfo->enableDhcpc, brInfo->dhcpClassIdentifier);
    printf("===============================================\n");
#endif

    free(psiValue);
    psiValue = NULL;
    return PSX_STS_OK;
}
/* end of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */

#ifdef SUPPORT_RIP
void xmlSetRipAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PRT_RIP_CFG rip;
   static char* state[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL))
      return;

   rip = (PRT_RIP_CFG)value;
   
   fprintf(file,"<%s %s=\"%s\" ripIfcTableSize=\"%d\"/>\n",
           name,PSX_ATTR_STATE,state[rip->globalMode],rip->ifcCount);
}

void xmlSetRipIfcAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   PRT_RIP_IFC_ENTRY pRipIfc, ptr;
   static char* state[] = {"disable","enable"};
   static char* version[] = {"","1","2","1_2"};
   static char* op[] ={"active","passive"};
   int ifCount, i;

   if ((name == NULL) || (value == NULL))
      return;

   pRipIfc = (PRT_RIP_IFC_ENTRY)value;
   ifCount = length/sizeof(RT_RIP_IFC_ENTRY);

   if ( ifCount != 0 )   
      fprintf(file,"<%s tableSize=\"%d\">\n",TBLNAME_RIP_IFC,ifCount);
   for (i=0; i < ifCount; i++) {
     ptr = &pRipIfc[i];
     fprintf(file,"<%s id=\"%d\" name=\"%s\" %s=\"%s\" version=\"%s\" operation=\"%s\"/>\n",
	     name, (i+1),ptr->name,PSX_ATTR_STATE,state[ptr->flag.mode],version[ptr->flag.version],
	     op[ptr->flag.operation]);

   }
   if ( ifCount != 0 )
      fprintf(file,"</%s>\n",TBLNAME_RIP_IFC);
}
#endif /*  SUPPORT_RIP */

void xmlSetRouteAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   PRT_ROUTE_CFG_ENTRY route, rt;
   int entry = 0;
   int num = 0;
   char addr[IFC_TINY_LEN], mask[IFC_TINY_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_ROUTE) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     route = (PRT_ROUTE_CFG_ENTRY)value;
     /* determine the size of route table */
     //num = length/sizeof(RT_ROUTE_CFG_ENTRY);
     num = xmlGetTableSize(file);
     
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       rt = &route[entry];
       strncpy(addr,inet_ntoa(rt->ipAddress),IFC_TINY_LEN);
       strncpy(mask,inet_ntoa(rt->subnetMask),IFC_TINY_LEN);
       fprintf(file,"<%s id=\"%d\" instanceId=\"%lu\" addr=\"%s\" mask=\"%s\" ",
	       name,(entry+1),rt->id,addr,mask);

       strncpy(addr,inet_ntoa(rt->gateway),IFC_TINY_LEN);
       /*start of Enables or disable the forwarding entry by l129990,2009-11-6*/
       fprintf(file,"gateway=\"%s\" ifName=\"%s\" routeCtl=\"%d\"/>\n",
	       addr,rt->ifName,rt->entryControl);
       /*end of Enables or disable the forwarding entry by l129990,2009-11-6*/
     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",routeObjs[id-1].objName); 
   } /* route table */
}

void xmlSetLanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PIFC_LAN_INFO lanInfo;
   static char* dhcpState[] = {"disable","enable","relay"};
   char addr[IFC_TINY_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   lanInfo = (PIFC_LAN_INFO)value;
   //   lanAddr[0]='\0';
   //   lanMask[0]='\0';
   strncpy(addr,inet_ntoa(lanInfo->lanAddress),IFC_TINY_LEN);



   /*Start -- w00135358 add for HG556a 20090520 -- 规避一线LAN IP CHANGED 问题*/
   if(strcmp(addr, "255.255.255.255") == 0 || strcmp(addr, "0.0.0.0") == 0)
   {	   	
	   //printf("==DEBUG==%s,%d===LAN IP CHANGE\n",__func__,__LINE__);
	   strncpy(addr,BcmDb_getDefaultValue("ethIpAddress"),IFC_TINY_LEN);
	   lanInfo->lanMask.s_addr = inet_addr(BcmDb_getDefaultValue("ethSubnetMask"));
   }
   /*End -- w00135358 add for HG556a 20090520*/


   /*   fprintf(file,"%s ",lanInfo->ifcName);  ifcName is never stored in PSI, lanIdNum will be
	displayed instead, IdNum is the object ID of the device */
   fprintf(file,"<entry%d address=\"%s\" ", id, addr);
   strncpy(addr,inet_ntoa(lanInfo->lanMask),IFC_TINY_LEN);
   /*start of support to the dhcp relay  function by l129990,2009,11,12*/
   fprintf(file,"mask=\"%s\" dhcpServer=\"%s\" dhcpRelay=\"%d\" leasedTime=\"%d\" ",
	   addr,dhcpState[lanInfo->dhcpSrv.enblDhcpSrv],lanInfo->dhcpRelay,lanInfo->dhcpSrv.leasedTime);
   /*end of support to the dhcp relay  function by l129990,2009,11,12*/
   strncpy(addr,inet_ntoa(lanInfo->dhcpSrv.startAddress),IFC_TINY_LEN);
   fprintf(file,"startAddr=\"%s\" ",addr);
   strncpy(addr,inet_ntoa(lanInfo->dhcpSrv.endAddress),IFC_TINY_LEN);
   fprintf(file,"endAddr=\"%s\" ",addr, lanInfo->id);
   #ifdef	VDF_RESERVED
   fprintf(file,"reservedAddr=\"%s\" ",lanInfo->dhcpSrv.reservedAddress);
   #endif
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
   if(IFC_ENET_ID == id)
   {
	   fprintf(file, "poolIndex=\"%d\" ", lanInfo->dhcpSrv.poolIndex);

	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_1), IFC_TINY_LEN);
	   fprintf(file, "dhcpStart1_1=\"%s\" ", addr);
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_1), IFC_TINY_LEN);
	   fprintf(file, "dhcpEnd1_1=\"%s\" ", addr);
	   fprintf(file, "dhcpLease1_1=\"%d\" ", lanInfo->dhcpSrv.dhcpLease1_1);
	   fprintf(file, "dhcpSrv1Option60_1=\"%s\" ", lanInfo->dhcpSrv.dhcpSrv1Option60_1);
	   
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_2), IFC_TINY_LEN);
	   fprintf(file, "dhcpStart1_2=\"%s\" ", addr);
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_2), IFC_TINY_LEN);
	   fprintf(file, "dhcpEnd1_2=\"%s\" ", addr);
	   fprintf(file, "dhcpLease1_2=\"%d\" ", lanInfo->dhcpSrv.dhcpLease1_2);
	   fprintf(file, "dhcpSrv1Option60_2=\"%s\" ", lanInfo->dhcpSrv.dhcpSrv1Option60_2);
	   
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_3), IFC_TINY_LEN);
	   fprintf(file, "dhcpStart1_3=\"%s\" ", addr);
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_3), IFC_TINY_LEN);
	   fprintf(file, "dhcpEnd1_3=\"%s\" ", addr);
	   fprintf(file, "dhcpLease1_3=\"%d\" ", lanInfo->dhcpSrv.dhcpLease1_3);
	   fprintf(file, "dhcpSrv1Option60_3=\"%s\" ", lanInfo->dhcpSrv.dhcpSrv1Option60_3);
	   
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_4), IFC_TINY_LEN);
	   fprintf(file, "dhcpStart1_4=\"%s\" ", addr);
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_4), IFC_TINY_LEN);
	   fprintf(file, "dhcpEnd1_4=\"%s\" ", addr);
	   fprintf(file, "dhcpLease1_4=\"%d\" ", lanInfo->dhcpSrv.dhcpLease1_4);
	   fprintf(file, "dhcpSrv1Option60_4=\"%s\" ", lanInfo->dhcpSrv.dhcpSrv1Option60_4);
	   
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_5), IFC_TINY_LEN);
	   fprintf(file, "dhcpStart1_5=\"%s\" ", addr);
	   strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_5), IFC_TINY_LEN);
	   fprintf(file, "dhcpEnd1_5=\"%s\" ", addr);
	   fprintf(file, "dhcpLease1_5=\"%d\" ", lanInfo->dhcpSrv.dhcpLease1_5);
	   fprintf(file, "dhcpSrv1Option60_5=\"%s\" ", lanInfo->dhcpSrv.dhcpSrv1Option60_5);   

#ifdef SUPPORT_CHINATELECOM_DHCP
	   fprintf(file, "cameraport=\"%s\" ", lanInfo->dhcpSrv.cameraport);   
         fprintf(file, "ccategory=\"%s\" ", lanInfo->dhcpSrv.ccategory);   
         fprintf(file, "cmodel=\"%s\" ", lanInfo->dhcpSrv.cmodel);   
#endif
   }
 #endif
/*w44771 add for 第一IP支持5段地址池，end*/  
/*DHCP Server支持第二地址池, s60000658, 20060616*/
 #ifdef SUPPORT_PORTMAPING
    fprintf(file, "dhcpoption60=\"%s\" ", lanInfo->dhcpSrv.option60);
 #endif
#ifdef SUPPORT_VDF_DHCP
 	strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpNtpAddr1), IFC_TINY_LEN);
 	fprintf(file, "dhcpNtpServer1=\"%s\" ", addr);
	strncpy(addr, inet_ntoa(lanInfo->dhcpSrv.dhcpNtpAddr2), IFC_TINY_LEN);
	fprintf(file, "dhcpNtpServer2=\"%s\" ", addr);
#endif
	/* start of VDF 2008.4.17 V100R001C02B013 j00100803 AU8D00423 */
	strncpy(addr, inet_ntoa(lanInfo->lanDnsAddress1), IFC_TINY_LEN);
   	fprintf(file, "lanDns1=\"%s\" ", addr);
   	strncpy(addr, inet_ntoa(lanInfo->lanDnsAddress2), IFC_TINY_LEN);
   	fprintf(file, "lanDns2=\"%s\" ", addr);
   	/* end of VDF 2008.4.17 V100R001C02B013 j00100803 AU8D00423 */
	/* j00100803 Add Begin 2008-05-20 for option43 */
	fprintf(file, "opt43=\"%s\" ", lanInfo->szLanOption43);
	/* j00100803 Add End 2008-05-20 for option43 */
    /*start of support to configure the option66,67,160 by l129990,2009,12,22*/
    fprintf(file, "opt66=\"%s\" ", lanInfo->szLanOption66);
    fprintf(file, "opt67=\"%s\" ", lanInfo->szLanOption67);
    fprintf(file, "opt160=\"%s\" ", lanInfo->szLanOption160);
    /*end of support to configure the option66,67,160 by l129990,2009,12,22*/
        /* BEGIN: Added by y67514, 2008/9/22   PN:GLB:lan dns*/
        fprintf(file, "lanDomain=\"%s\" ", lanInfo->lanDomain);
        /* END:   Added by y67514, 2008/9/22 */
   fprintf(file,"dhcpClassId=\"%s\" instanceId=\"%lu\" ",lanInfo->dhcpSrv.dhcpSrvClassId, lanInfo->id);
   /*start of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
   switch(lanInfo->addressType)
   {
       case STATIC:
           fprintf(file, "addrType=\"%d\"/>\n", STATIC);
           break;
       case DHCP:
           fprintf(file, "addrType=\"%d\"/>\n", DHCP);
           break;
       case AUTOIP:
           fprintf(file, "addrType=\"%d\"/>\n", AUTOIP);
           break;
   
       default:
           fprintf(file, "addrType=\"%d\"/>\n", AUTOIP);
           break;
   }

   /*end of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/   
}

#ifdef ETH_CFG
int xmlEthSpeedToPsi(char *value)
{
  if (strcmp("100Mbps",value) == 0)
    return ETH_SPEED_100;
  else if (strcmp("10Mbps",value) == 0)
    return ETH_SPEED_10;
  return ETH_SPEED_AUTO;
}

int xmlEthTypeToPsi(char *value)
{
  if (strcmp("fullDuplex",value) == 0)
    return ETH_TYPE_FULL_DUPLEX;
  else if (strcmp("halfDuplex",value) == 0)
    return ETH_TYPE_HALF_DUPLEX;
  return ETH_TYPE_AUTO;
}

void xmlSetEthAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PETH_CFG_INFO ethInfo;
   static char* type[] = {"auto","fullDuplex","halfDuplex"};
   static char* speed[] = {"auto","100Mbps","10Mbps"};

   if ((name == NULL) || (value == NULL))
      return;

   ethInfo = (PETH_CFG_INFO)value;
   fprintf(file,"<ethCfg%u speed=\"%s\" type=\"%s\" MTU=\"%d\" disabled=\"%d\"/>\n", 
       id, speed[ethInfo->ethSpeed], type[ethInfo->ethType], ethInfo->ethMtu,
       ethInfo->ethIfcDisabled);
}

PSX_STATUS xmlGetEthAttr(char *attrName, char* attrValue)
{
  PETH_CFG_INFO ethInfo = (PETH_CFG_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetEthAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"speed") == 0) {
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetEthAttr(): invalid speed %s\n",attrValue);
    }
    else {
      ethInfo->ethSpeed = xmlEthSpeedToPsi(attrValue);
    }
  }
  else if (strcmp(attrName,"type") == 0) {
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetEthAttr(): invalid type %s\n",attrValue);
    }
    else {
      ethInfo->ethType = xmlEthTypeToPsi(attrValue);
    }
  }
  else if (strcmp(attrName,"MTU") == 0) {
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetEthAttr(): invalid MTU %s, valid range is %d-%d\n",
             attrValue,ETH_CFG_MTU_MIN,ETH_CFG_MTU_MAX);
    }
    else {
      ethInfo->ethMtu = atoi(attrValue);
    }
  }
  /*start of Global HG556a AU8D00948 by c00131380 at 081016*/
  else if (strcmp(attrName,"disabled") == 0)
  {
     if (status != DB_OBJ_VALID_OK) 
     {
         xmlState.errorFlag = PSX_STS_ERR_FATAL;
         syslog(LOG_ERR,"xmlGetEthAttr(): invalid disabled %s\n",attrValue);
     }
     else
     {
         ethInfo->ethIfcDisabled = atoi(attrValue);
     }
  }
  /*end of Global HG556a AU8D00948 by c00131380 at 081016*/
  
  return(PSX_STS_OK);
}
#endif /* ETH_CFG */

#ifdef WIRELESS
void xmlWlSetWebAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars)
{
  if ((strcasecmp(wlVars->wlWep, WL_DISABLED) == 0))
    fprintf(file," wep=\"disabled\"");
  else
    fprintf(file," wep=\"enabled\"");
    
  fprintf(file, " auth=\"%d\"", wlVars->wlAuth);  
}

void xmlWlSetEncrytionAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars)
{
  if (strcasecmp(wlVars->wlWpa, "tkip") == 0)
    fprintf(file," wpa=\"tkip\"");
  else if (strcasecmp(wlVars->wlWpa, "aes") == 0)
    fprintf(file," wpa=\"aes\"");
  else if (strcasecmp(wlVars->wlWpa, "tkip+aes") == 0)
    fprintf(file," wpa=\"tkip+aes\"");
}

#ifdef SUPPORT_TR69C 
void xmlWlSetTr069SecurityAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars) 
{

  char* tr69cBeaconType[] = {"None","Basic","WPA","11i","BasicandWPA","Basicand11i","WPAand11i","BasicandWPAand11i"};
  char* tr69cBasicEncryptionModes[]={"None","WEPEncryption"};
  /*start of VDF  2008.03.07 HG553V100R001  w45260:AU8D00478 增加对share认证方式的支持 */	
  char* tr69cBasicAuthenticationMode[]={"None","EAPAuthentication","SharedAuthentication"};
  /*end of VDF  2008.04.17 HG553V100R001  w45260 */
  char* tr69cWPAEncryptionModes[]={"WEPEncryption","TKIPEncryption","WEPandTKIPEncryption","AESEncryption",\
                                   "WEPandAESEncryption","TKIPandAESEncryption","WEPandTKIPandAESEncryption"};
  char* tr69cWPAAuthenticationMode[]={"PSKAuthentication","EAPAuthentication"};
  char* tr69cIEEE11iEncryptionModes[]={"WEPEncryption","TKIPEncryption","WEPandTKIPEncryption","AESEncryption",\
                                   "WEPandAESEncryption","TKIPandAESEncryption","WEPandTKIPandAESEncryption"};
  char* tr69cIEEE11iAuthenticationMode[]={"PSKAuthentication","EAPAuthentication","EAPandPSKAuthentication"}; 

  fprintf(file," tr69cBeaconType=\"%s\"", tr69cBeaconType[wlVars->tr69cBeaconType]);
  fprintf(file," tr69cBasicEncryptionModes=\"%s\"", tr69cBasicEncryptionModes[wlVars->tr69cBasicEncryptionModes]);
  fprintf(file," tr69cBasicAuthenticationMode=\"%s\"", tr69cBasicAuthenticationMode[wlVars->tr69cBasicAuthenticationMode]);
  fprintf(file," tr69cWPAEncryptionModes=\"%s\"", tr69cWPAEncryptionModes[wlVars->tr69cWPAEncryptionModes]);  
  fprintf(file," tr69cWPAAuthenticationMode=\"%s\"", tr69cWPAAuthenticationMode[wlVars->tr69cWPAAuthenticationMode]);
  fprintf(file," tr69cIEEE11iEncryptionModes=\"%s\"", tr69cIEEE11iEncryptionModes[wlVars->tr69cIEEE11iEncryptionModes]);  
  fprintf(file," tr69cIEEE11iAuthenticationMode=\"%s\"", tr69cIEEE11iAuthenticationMode[wlVars->tr69cIEEE11iAuthenticationMode]);
}
#endif

void xmlWlSetEncryptionStrengthAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars)
{
  int i;

  if (wlVars->wlKeyBit == WL_BIT_KEY_64 ) {
    fprintf(file," keyBit=\"64-bit\"");
  }
  else
    fprintf(file," keyBit=\"128-bit\"");

  /* display all keys  */
  for (i = 0; i < WL_KEY_NUM; i++ ) {
    /* if key is empty or active key then do nothing */
    fprintf(file," key64_%d=\"%s\"",i+1,wlVars->wlKeys64[i]);
  }
  /* display active key index */
  fprintf(file," key64Index=\"%d\"",wlVars->wlKeyIndex64);

  /* display all 128 keys */
  for (i = 0; i < WL_KEY_NUM; i++ ) {
    /* if key is empty or active key then do nothing */
    fprintf(file," key128_%d=\"%s\" ",i+1,wlVars->wlKeys128[i]);
  }
  /* display action key index */
  fprintf(file," key128Index=\"%d\"",wlVars->wlKeyIndex128);
}  /* xmlWlSetEncryptionStrengthAttr */

void xmlWlSetRadiusServerAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars)
{
  char addr[IFC_TINY_LEN];
  
  strncpy(addr,inet_ntoa(wlVars->wlRadiusServerIP),IFC_TINY_LEN);

  fprintf(file," radiusServerIP=\"%s\" radiusServerPort=\"%d\" radiusServerKey=\"%s\"",
	  addr,wlVars->wlRadiusPort,wlVars->wlRadiusKey);
}

void xmlWlSetAllSecurityAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars) 
{
  char* genOnOffState[] ={"off","on"};
  
  fprintf(file," authMode=\"%s\"", wlVars->wlAuthMode);
  xmlWlSetRadiusServerAttr(file,wlVars);
  xmlWlSetWebAttr(file, wlVars);
  xmlWlSetEncryptionStrengthAttr(file,wlVars);
  fprintf(file," wpaRekey=\"%d\"",wlVars->wlWpaGTKRekey);
  fprintf(file," wpakey=\"%s\" ",wlVars->wlWpaPsk);
  fprintf(file," Preauthentication=\"%s\" ReauthTimeout=\"%d\"", genOnOffState[wlVars->wlPreauth], wlVars->wlNetReauth);
  xmlWlSetEncrytionAttr(file,wlVars);
#ifdef SUPPORT_TR69C   
  xmlWlSetTr069SecurityAttr(file, wlVars);
#endif  
}

void xmlWlSetMacFilterAttr(FILE *file, PWIRELESS_VAR wlVars) 
{
  fprintf(file," fltMacMode=\"%s\" ",wlVars->wlFltMacMode);
}

void xmlWlSetBridgeAttr(FILE *file, PWIRELESS_VAR wlVars) 
{
  /* bridge restrict: enabled, enabled(scan), disabled */
  static char *bridgeRestrict[] = {"enabled", "disabled", "enabled_scan" };

  fprintf(file," apMode=\"%s\" ",wlVars->wlMode);
  fprintf(file," bridgeRestrict=\"%s\" ",bridgeRestrict[wlVars->wlLazyWds]);
  for(int i=0; i<4; i++) {
    fprintf(file," wdsMAC_%d=\"%s\" ", i, wlVars->wlWds[i]);
  }
}
/* wl_add_var_check */
void xmlWlSetAdvanceAttr(FILE *file, PWIRELESS_VAR wlVars) 
{
   char* band[] = {"auto", "a", "b"};
   /* BEGIN: Modified by c106292, 2008/9/18   PN:Global 增加了"802.11ng","802.11na"*/
   char* gMode[] = { "802.11b", "auto", NULL, NULL, "performance", "lrs" ,"802.11ng","802.11na"};
   /* END:   Modified by c106292, 2008/9/18 */
   char* genOnOffState[] ={"off","on"};
   char* regMode[]={"off", "802.11h", "802.11d"};

#ifdef SUPPORT_MIMO   
   char* nsb[]={"lower", "none", "upper"};
#endif
//BEGIN:add by zhourongfei to periodically check the best channel
   fprintf(file," band=\"%s\" channel=\"%d\" channelcheck=\"%d\"", band[wlVars->wlBand], wlVars->wlChannel, wlVars->wlchannelcheck);
//END:add by zhourongfei to periodically check the best channel
   if (wlVars->wlRate == (long int)0)
     fprintf(file," rate=\"auto\"");
   else 
     fprintf(file," rate=\"%3.1f\"",((float)wlVars->wlRate/1000000));
   if (wlVars->wlMCastRate == (long int)0)
     fprintf(file," multicastRate=\"auto\"");
   else 
     fprintf(file," multicastRate=\"%3.1f\"",((float)wlVars->wlMCastRate/1000000));          

    fprintf(file," basicRate=\"%s\" fragThreshold=\"%d\" RTSThreshold=\"%d\" DTIM=\"%d\" beacon=\"%d\" XPress=\"%s\" gMode=\"%s\" gProtection=\"%s\" preamble=\"%s\" AfterBurner=\"%s\" TxPowerPercent=\"%d\"",
              wlVars->wlBasicRate,wlVars->wlFrgThrshld,wlVars->wlRtsThrshld, wlVars->wlDtmIntvl,wlVars->wlBcnIntvl,wlVars->wlFrameBurst,
              gMode[wlVars->wlgMode],wlVars->wlProtection,wlVars->wlPreambleType, wlVars->wlAfterBurnerEn, wlVars->wlTxPwrPcnt);
    if(wlVars->wlWme == -1)
         fprintf(file," WME=\"auto\"");
    else
         fprintf(file," WME=\"%s\"", genOnOffState[wlVars->wlWme]);

    fprintf(file," WMENoAck=\"%s\" WMEApsd=\"%s\" ", genOnOffState[wlVars->wlWmeNoAck], genOnOffState[wlVars->wlWmeApsd]);

    /*start of enable or disable the access point radio by l129990,2009-10-9*/
    fprintf(file," RadioCtl=\"%s\"", genOnOffState[wlVars->wlRadioCtl]);
    /*end of enable or disable the access point radio by l129990,2009-10-9*/
              
    fprintf(file," RegulatoryMode=\"%s\" PreNetRadarDectTime=\"%d\" InNetRadarDectTime=\"%d\"", \
              regMode[wlVars->wlRegMode], wlVars->wlDfsPreIsm, wlVars->wlDfsPostIsm);
    fprintf(file," TpcMitigation=\"%d\"", wlVars->wlTpcDb);                                  
    fprintf(file," AutoChannelTimer=\"%d\"", wlVars->wlCsScanTimer);    
    fprintf(file," globalMaxAssoc=\"%d\"", wlVars->wlGlobalMaxAssoc);     
#ifdef SUPPORT_WLAN_PRNTCTL 
    fprintf(file," AutoSwOffdays=\"%d\"", wlVars->wlAutoSwOffdays);
    fprintf(file," StarOfftTime=\"%d\"", wlVars->wlStarOfftTime);
    fprintf(file," EndOffTime=\"%d\"", wlVars->wlEndOffTime);
#endif    
#ifdef SUPPORT_MIMO
    fprintf(file," NBandwidthCap=\"%d\"", wlVars->wlNBwCap);
    fprintf(file," NCtrlSideband=\"%s\"", nsb[wlVars->wlNCtrlsb+1]);
    fprintf(file," NBand=\"%d\"", wlVars->wlNBand);
    fprintf(file," NMCSIdx=\"%d\"", wlVars->wlNMcsidx);
    fprintf(file," NProtection=\"%s\"", wlVars->wlNProtection);
    fprintf(file," NMode=\"%s\"", wlVars->wlNmode);    
    fprintf(file," NReqd=\"%s\"", genOnOffState[wlVars->wlNReqd]);    
    /*
    fprintf(file," RIFS=\"%s\"", wlVars->wlRifs);
    fprintf(file," AMPDU=\"%s\"", wlVars->wlAmpdu);
    fprintf(file," AMSDU=\"%s\"", wlVars->wlAmsdu);
    */
#endif

    fprintf(file," />\n");           
}

void xmlSetWirelessVarsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			     const UINT16 length)
{
   PWIRELESS_VAR wlVars;
   static char* state[] = {"disabled","enabled"};

   if ((name == NULL) || (value == NULL))
      return;
   wlVars = (PWIRELESS_VAR)value;
   fprintf(file,"<%s %s=\"%s\" curState=\"%s\" ssIdIndex=\"%d\" country=\"%s\"",
	   name,PSX_ATTR_STATE,state[wlVars->wlEnbl],state[wlVars->wlEnbl],wlVars->wlSsidIdx,wlVars->wlCountry);
   /* BEGIN: Modified by c106292, 2008/10/5   PN:Global*/
#if defined (SUPPORT_SES ) || defined (SUPPORT_ATHEROSWLAN)
  /* END:   Modified by c106292, 2008/10/5 */
   fprintf(file," ses_enable=\"%d\" ses_event=\"%d\" ses_states=\"%s\"", \
		wlVars->wlSesEnable,wlVars->wlSesEvent,wlVars->wlSesStates);   

   fprintf(file," ses_ssid=\"%s\" ses_hide=\"%d\" ses_wpa_psk=\"%s\"", \
		wlVars->wlSesSsid,wlVars->wlSesHide,wlVars->wlSesWpaPsk);   

   fprintf(file," ses_auth=\"%d\" ses_auth_mode=\"%s\" ses_wep=\"%s\"", \
		wlVars->wlSesAuth,wlVars->wlSesAuthMode,wlVars->wlSesWep);   

   fprintf(file," ses_wpa=\"%s\"", \
		wlVars->wlSesWpa);      
		
   fprintf(file," ses_client_enable=\"%d\" ses_client_event=\"%d\"", \
		wlVars->wlSesClEnable, wlVars->wlSesClEvent); 
		
   fprintf(file," ses_wds_mode=\"%d\"  ses_wds_wsec=\"%s\"", \
		wlVars->wlSesWdsMode, wlVars->wlWdsWsec); 		
#endif

   /* wireless bridge */
   xmlWlSetBridgeAttr(file,wlVars);

   /* advance parameter */
   xmlWlSetAdvanceAttr(file,wlVars);
}

/* start of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
/*------------------------------------------------------------
  函数原型: void EncodeSsid(char* Ssid)
  描述: wlan用户使用的ssid中可以含有"、<、>、/、=  字符，
                这俩字符在本代码中xml解析过程中会导致
                错误解析结果ssid中"后的字符无效。
                本函数对这俩字符进行特殊处理。与DecodeSsid成对使用。
  输入: 业务模块使用的ssid
  输出: 存入flash中的ssid
  返回值: 无
-------------------------------------------------------------*/
/* start of maintain 特殊字符处理 by xujunxia 43813 2006年5月10日
void EncodeSsid(char* Ssid)
{
    for ( unsigned int i = 0 ; i < strlen(Ssid) ; i++ )
    {
        //把双引号转为ASCII字符2
        if ( Ssid[i] == '"')
        {
            Ssid[i] = 2;
        }
    }
}
*/
void EncodeSsid(char* Ssid)
{
    for ( unsigned int i = 0 ; i < strlen(Ssid) ; i++ )
    {
        //把双引号转为ASCII字符2
        switch(Ssid[i])
        {
            case '<':
                Ssid[i] = 1;
                break;
            case '"':
                Ssid[i] = 2;
                break;
            case '>':
                Ssid[i] = 3;
                break;
            case '=':
                Ssid[i] = 4;
                break;
            case '/':
                Ssid[i] = 5;
                break;
            case '&':
                Ssid[i] = 6;
                break;
        }
    }
}
/* end of maintain 特殊字符处理 by xujunxia 43813 2006年5月10日 */
/*------------------------------------------------------------
  函数原型:void DecodeSsid(char* Ssid)
  描述: 与EncodeSsid程度使用
  输入: 从flash读取的xml文件中解析出的ssid属性
  输出: 对"  特殊处理反转换之后的ssid
  返回值: 无
-------------------------------------------------------------*/
/* start of maintain 特殊字符处理 by xujunxia 43813 2006年5月10日
void DecodeSsid(char* Ssid)
{
    for (unsigned int i = 0 ; i < strlen(Ssid) ; i++ )
    {
        //把ASCII字符2转为双引号
        if ( Ssid[i] == 2)
        {
            Ssid[i] = '"';
        }
    }
}
*/
void DecodeSsid(char* Ssid)
{
    for (unsigned int i = 0 ; i < strlen(Ssid) ; i++ )
    {
        switch(Ssid[i])
        {
            case 1:
                Ssid[i] = '<';
                break;
            case 2:
                Ssid[i] = '"';
                break;
            case 3:
                Ssid[i] = '>';
                break;
            case 4:
                Ssid[i] = '=';
                break;
            case 5:
                Ssid[i] = '/';
                break;
            case 6:
                Ssid[i] = '&';
                break;
        }
    }
}
/* end of maintain 特殊字符处理 by xujunxia 43813 2006年5月10日 */

/* end of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */

void xmlSetWirelessMssidVarsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,  const UINT16 length)
{
   int i, num=WL_NUM_SSID;
   PWIRELESS_MSSID_VAR entry;
   char* genOnOffState[] ={"off","on"};
   /* start of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
   char pszSsid[WL_SSID_SIZE_MAX];
   /* end of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
   
#ifdef XML_DEBUG1
   printf("xmlSetWirelessMssidVarsAttr: name %s, id %d, value %d num %d\n",name,id,*(UINT16*)value, num);
#endif

   if ((name == NULL) || (value == NULL)) {
      return;
   }

   if ((strcmp(name,TBLNAME_WL_MSSID_VARS) == 0) ) {
     /* beginning of table */
     fprintf(file,"<%s tableSize=\"%d\">\n",name, num);
   }
   else {
     entry = (PWIRELESS_MSSID_VAR)value;
     for (i = 0; i < num; i++) {
       /* start of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日"
       fprintf(file,"<%s enblSsId=\"%d\" ssId=\"%s\"", wirelessObjs[id].objName, entry[i].wlEnblSsid, entry[i].wlSsid);
       */
       #ifdef XML_DEBUG
       printf("before EncodeSsid, the ssId is %s\n", entry[i].wlSsid);
       #endif
       strcpy(pszSsid, entry[i].wlSsid);
       EncodeSsid(pszSsid);
       fprintf(file,"<%s enblSsId=\"%d\" ssId=\"%s\"", 
                  wirelessObjs[id].objName, entry[i].wlEnblSsid, pszSsid);
       #ifdef XML_DEBUG
       printf("after EncodeSsid, the ssId is %s\n", pszSsid);
       #endif
       /* end of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
       /* MBSS */
       fprintf(file," hide=\"%d\" apIsolation=\"%s\" fltMacMode=\"%s\" disableWme=\"%s\" MaxAssoc = \"%d\" ",     
          entry[i].wlHide, genOnOffState[entry[i].wlAPIsolation], entry[i].wlFltMacMode, genOnOffState[entry[i].wlDisableWme], entry[i].wlMaxAssoc);
          
  /* BEGIN: Modified by c106292, 2008/9/10   PN:Global*/
/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_WSC ) || defined (SUPPORT_ATHEROSWLAN)
       fprintf(file," wsc_mode=\"%s\" ",  entry[i].wsc_mode);
       fprintf(file," wsc_config_state=\"%s\" ",  entry[i].wsc_config_state);
#endif
/* END:   Modified by c106292, 2008/9/10 */
       /* security */
       xmlWlSetAllSecurityAttr(file, entry+i);
       fprintf(file,"/>\n");
     }
     /* end of table */
     if ( num != 0 ) {
       fprintf(file,"</%s>\n",wirelessObjs[id-1].objName); 
     }
   } 
}

void xmlSetWirelessMacFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   PWL_FLT_MAC_STORE_ENTRY entry, ptr;
   int num = 0, i = 0;
   /* 1. somehow, wireless num and tables do not come in order.   2. When wds and scanWds are being
      configured on the WEBUI even though one is only used, the num=0, but the entry is not
      nulled; this caused the table and entries to be displayed incorrectly. So if the num =0,
      then close out the table, don't try to print based on the length of the returned table */

#ifdef XML_DEBUG1
   printf("xmlSetWirelessMacFilterAttr: name %s, id %d, value %d\n",name,id,*(UINT16*)value);
#endif

   if ((name == NULL) || (value == NULL))
      return;

   if ((strcmp(name,TBLNAME_WL_MAC_FILTER) == 0) ||
       (strcmp(name,TBLNAME_WDS_FILTER) == 0) ||
       (strcmp(name,TBLNAME_SCAN_WDS_FILTER) == 0)) {
     num = *(UINT16*)value;
     /* beginning of table */
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     entry = (PWL_FLT_MAC_STORE_ENTRY)value;
     //num = length/sizeof(WL_FLT_MAC_STORE_ENTRY);
     num = xmlGetTableSize(file);
     
     for (i = 0; i < num; i++) {
       /* each entry is one object, even though PSI is not that way */
       ptr = &entry[i];

       if(*(ptr->ifcName))
          fprintf(file,"<%s id=\"%d\" macAddr=\"%s\" ifc=\"%s\"/>\n", wirelessObjs[id].objName,(i+1),ptr->macAddress, ptr->ifcName);          
       else
          fprintf(file,"<%s id=\"%d\" macAddr=\"%s\"/>\n", wirelessObjs[id].objName,(i+1),ptr->macAddress);

     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",wirelessObjs[id-1].objName); 
   } /* route table */
}
#endif /* #ifdef WIRELESS */

void xmlSetAtmInitAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length)
{
   PATM_INITIALIZATION_PARMS initCfg;
 
  if ((name == NULL) || (value == NULL))
      return;

   initCfg = (PATM_INITIALIZATION_PARMS)value;
   
   fprintf(file,"<%s ",name);

   fprintf(file,"structureId=\"%d\" threadPriority=\"%d\" freeCellQSize=\"%d\" freePktQSize=\"%d\" freePktQBufSize=\"%d\" freePktQBufOffset=\"%d\" rxCellQSize=\"%d\" rxPktQSize=\"%d\" txFifoPriority=\"%d\" aal5MaxSduLen=\"%d\" aal2MaxSduLen=\"%d\"/>\n",
	   (int)initCfg->ulStructureId,(int)initCfg->ulThreadPriority,(int)initCfg->usFreeCellQSize,(int)initCfg->usFreePktQSize,
	   (int)initCfg->usFreePktQBufferSize, (int)initCfg->usFreePktQBufferOffset,(int)initCfg->usReceiveCellQSize,
	   (int)initCfg->usReceivePktQSize,(int)initCfg->ucTransmitFifoPriority,(int)initCfg->ucTransmitFifoPriority,
	   (int)initCfg->usAal2SscsMaxSsarSduLength);
}
#ifdef SUPPORT_VDSL
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
void xmlSetAtmTrafficModeAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				 const UINT16 length)
{
   int mode;

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&mode, value, length);
   fprintf(file,"<%s %s=\"%d\"/>\n", name,PSX_ATTR_STATE,mode);
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#endif
void xmlSetAtmPortAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length)
{
   PATM_PORT_CFG portCfg, port;
   static char* type[] = {"disabled", "utopia", "loopback", "tc", "adslInterleaved", "adslFast"};
   int i, num;

   if ((name == NULL) || (value == NULL))
      return;

   portCfg = (PATM_PORT_CFG)value;   
   num = length/sizeof(ATM_PORT_CFG);

   /* beginning of table */
   fprintf(file,"<%s num=\"%d\">\n",name,num);
   for (i = 0; i < NUM_PORTS; i++) {
      port = &portCfg[i];
      fprintf(file,"<port%d ifcId=\"%d\" type=\"%s\" flags=\"%d\"/>\n",
	      i+1,(int)port->ulInterfaceId,type[port->ucPortType],(int)port->ucPortFlags);
   }
   /* end of table */
   fprintf(file,"</%s>\n",name);
}

void xmlSetAtmTdNumAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
  fprintf(file,"xmlSetAtmTdNumAttr(): name %s, id %d\n",name,id);
}

void xmlSetAtmTdTblAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
  fprintf(file,"xmlSetAtmTdTblAttr(): name %s, id %d\n",name,id);
}

void xmlSetAtmIfcAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   PAPC_INTERFACE_CFG ifcCfg, ifc;
   PATM_INTERFACE_CFG atmIfc;
   int i, num;

   if ((name == NULL) || (value == NULL))
      return;

   ifcCfg = (PAPC_INTERFACE_CFG)value;
   num = length/sizeof(APC_INTERFACE_CFG);
   fprintf(file,"<%s num=\"%d\">",name,num);
   for (i = 0; i < NUM_PORTS; i++) {
      ifc = &ifcCfg[i];
      atmIfc = &(ifc->sIfcCfg);
      fprintf(file,"<ifc%d ifcId=\"%lu\" structureId=\"%lu\" maxVccs=\"%lu\" maxConfVccs=\"%lu\" maxActiveVpiBits=\"%lu\" maxActiveVciBits=\"%lu\" currMaxVpiBits=\"%lu\" currMaxVciBits=\"%lu\" adminStatus=\"%lu\" operStatus=\"%lu\" nullCells=\"%lu\" tcScramble=\"%lu\" portType=\"%lu\" lastChange=\"%lu\"/>\n",
	      i+1,ifc->ulInterfaceId,atmIfc->ulStructureId,atmIfc->ulAtmInterfaceMaxVccs,
	      atmIfc->ulAtmInterfaceConfVccs,atmIfc->ulAtmInterfaceMaxActiveVpiBits,
	      atmIfc->ulAtmInterfaceMaxActiveVciBits,atmIfc->ulAtmInterfaceCurrentMaxVpiBits,
	      atmIfc->ulAtmInterfaceCurrentMaxVciBits,atmIfc->ulIfAdminStatus,atmIfc->ulIfOperStatus,
	      atmIfc->ulSendNullCells,atmIfc->ulTcScramble,atmIfc->ulPortType,atmIfc->ulIfLastChange);
   }
}

void xmlSetAtmVccNumAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			 const UINT16 length)
{
  fprintf(file,"xmlSetAtmVccNumAttr: name %s, id %d\n",name,id);
}

void xmlSetAtmVccTblAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
  fprintf(file,"xmlSetAtmVccTblAttr: name %s, id %d\n",name,id);
}

void xmlSetServiceCtrlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   PSEC_SRV_CNTR_INFO info;
   static char* mode[] = {"disable", "enable", "lan", "wan"};
   
   if ((name == NULL) || (value == NULL))
      return;

   info = (PSEC_SRV_CNTR_INFO)value;
/* BEGIN: Modified by weishi kf33269, 2011/6/26   PN:Issue407:TR-069 service is not protected with access control*/
#ifdef SUPPORT_SNMP   
   fprintf(file,"<%s ftp=\"%s\" http=\"%s\" icmp=\"%s\" snmp=\"%s\" ssh=\"%s\" telnet=\"%s\" tftp=\"%s\" tr069=\"%s\"/>\n",
	   name,mode[info->modeFtp],mode[info->modeHttp],mode[info->modeIcmp],mode[info->modeSnmp],
	   mode[info->modeSsh],mode[info->modeTelnet],mode[info->modeTftp],mode[info->modeTr069]);
#else
   fprintf(file,"<%s ftp=\"%s\" http=\"%s\" icmp=\"%s\" ssh=\"%s\" telnet=\"%s\" tftp=\"%s\" tr069=\"%s\"/>\n",
	   name,mode[info->modeFtp],mode[info->modeHttp],mode[info->modeIcmp],
	   mode[info->modeSsh],mode[info->modeTelnet],mode[info->modeTftp],mode[info->modeTr069]);
#endif /* #ifdef SUPPORT_SNMP */
/* END:   Modified by weishi kf33269, 2011/6/26 */
}

/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
void xmlSetWredInfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   PSEC_WRED_INFO info;

   if ((name == NULL) || (value == NULL))
      return;

   info = (PSEC_WRED_INFO)value;

   fprintf(file,"<%s weightratio=\"%s\"/>\n", name,info->WredWeightRatio);
}

PSX_STATUS xmlGetWredInfoAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_WRED_INFO wredInfo = (PSEC_WRED_INFO)psiValue;
#ifdef XML_DEBUG
  printf("xmlGetWredInfoAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"weightratio") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, 10);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(wredInfo->WredWeightRatio, attrValue, SEC_BUFF_MAX_LEN - 1);
      wredInfo->WredWeightRatio[SEC_BUFF_MAX_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWredInfoAttr(): invalid access %s for weightratio\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartWredInfoObj(char *appName, char *objName)
{
  PSEC_WRED_INFO wredInfo;

#ifdef XML_DEBUG
  printf("xmlStartServiceCtrlObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SEC_WRED_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(SEC_WRED_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  wredInfo = (PSEC_WRED_INFO)psiValue;
  BcmDb_getDftWredInfo(wredInfo);

  return PSX_STS_OK;
}

PSX_STATUS xmlEndWredInfoObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndWredInfoObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  if (!xmlState.verify)
      BcmDb_setWredInfo((PSEC_WRED_INFO)psiValue);

#ifdef XML_DEBUG1
  PSEC_WRED_INFO info = (PSEC_WRED_INFO)psiValue;
  printf("\n============End Wred Info=========\n");
  printf("xmlEndWredInfoObj(): weightratio %s\n", info->WredWeightRatio);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
#endif  // #ifdef SUPPORT_KPN_QOS
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */

/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
void xmlSetDmzHostAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length)
{
   PSEC_DMZ_ENTRY pstSecDmzEntry;
   char addr[IFC_TINY_LEN];
    
   if ((name == NULL) || (value == NULL))
      return;
  
   pstSecDmzEntry = (PSEC_DMZ_ENTRY)value;
   strncpy(addr,inet_ntoa(pstSecDmzEntry->dmzAddress),IFC_TINY_LEN);
   fprintf(file,"<%s dmzEnable=\"%d\" ipAddr=\"%s\"/>\n",name,pstSecDmzEntry->dmzEnable,addr);
}
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/

void xmlSetVirtualSrvAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   PSEC_VRT_SRV_ENTRY vrtSrvs;
   int entry = 0;
   int num = 0;
   char addr[IFC_TINY_LEN];
   static char* protocols[] = {"TCP/UDP","TCP","UDP"};

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_VRT_SRV) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     vrtSrvs = (PSEC_VRT_SRV_ENTRY)value;
     //num = length/sizeof(SEC_VRT_SRV_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       memset(addr,0,IFC_TINY_LEN);
       strncpy(addr,inet_ntoa(vrtSrvs[entry].addr),IFC_TINY_LEN);
       if ( vrtSrvs[entry].protocol > 2 )
         vrtSrvs[entry].protocol = 0;
//modified by l66195 for VDF start
    //   printf("<...%s id=\"%d\" instanceId=\"%lu\" name=\"%s\" addr=\"%s\" protocol=\"%s\" eStart=\"%d\" eEnd=\"%d\" iStart=\"%d\" iEnd=\"%d\" bEnable=\"%d\"..../>\n",
	//       ENTRYNAME_VRT_SRV, entry+1,vrtSrvs[entry].id,vrtSrvs[entry].srvName,addr,protocols[vrtSrvs[entry].protocol],
	//       vrtSrvs[entry].eStart,vrtSrvs[entry].eEnd,vrtSrvs[entry].iStart,vrtSrvs[entry].iEnd, vrtSrvs[entry].bEnable);
       fprintf(file,"<%s id=\"%d\" instanceId=\"%lu\" name=\"%s\" addr=\"%s\" protocol=\"%s\" eStart=\"%d\" eEnd=\"%d\" iStart=\"%d\" iEnd=\"%d\" bEnable=\"%d\"/>\n",
	       ENTRYNAME_VRT_SRV, entry+1,vrtSrvs[entry].id,vrtSrvs[entry].srvName,addr,protocols[vrtSrvs[entry].protocol],
	       vrtSrvs[entry].eStart,vrtSrvs[entry].eEnd,vrtSrvs[entry].iStart,vrtSrvs[entry].iEnd, vrtSrvs[entry].bEnable);
//modified by l66195 for VDF end
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}

void xmlSetIpFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   PSEC_FLT_ENTRY filter, fptr;
   int entry = 0;
   int num = 0;
   /*modified by z67625 IP过滤支持IGMP start*/
   static char* protocol[] = {"TCP/UDP", "TCP", "UDP", "ICMP" ,"IGMP",""};
   /*modified by z67625 IP过滤支持IGMP end*/
   if ((name == NULL) || (value == NULL))
      return;

   if ((strcmp(name,TBLNAME_OUT_FILTER) == 0) || (strcmp(name,TBLNAME_IN_FILTER) == 0)) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     filter = (PSEC_FLT_ENTRY)value;
     //num = length/sizeof(SEC_FLT_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       fptr = &filter[entry];
       if ( strcmp(name, ENTRYNAME_OUT_FILTER) == 0 )
         fprintf(file,"<%s id=\"%d\" fltName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\"/>\n",
	       name,entry+1,fptr->fltName,protocol[fptr->protocol],fptr->srcAddr,fptr->srcMask,
	       fptr->srcPort,fptr->dstAddr,fptr->dstMask,fptr->dstPort);
       else if ( strcmp(name, ENTRYNAME_IN_FILTER) == 0 )
         fprintf(file,"<%s id=\"%d\" fltName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" wanIf=\"%s\"/>\n",
	       name,entry+1,fptr->fltName,protocol[fptr->protocol],fptr->srcAddr,fptr->srcMask,
	       fptr->srcPort,fptr->dstAddr,fptr->dstMask,fptr->dstPort,fptr->wanIf);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}

void xmlSetPrtTriggerAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			  const UINT16 length)
{
   PSEC_PRT_TRG_ENTRY prtTrigger, ptr;
   int entry = 0;
   int num = 0;
   static char* protocol[] = {"TCP/UDP", "TCP", "UDP", "ICMP"};

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_PRT_TRIGGER) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     prtTrigger = (PSEC_PRT_TRG_ENTRY)value;
     //num = length/sizeof(SEC_PRT_TRG_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &prtTrigger[entry];
//modifyed by l66195 for VDF start
 //      printf("<%s id=\"%d\" appName=\"%s\" triggerProtocol=\"%s\" openProtocol=\"%s\" triggerStart=\"%d\" triggerEnd=\"%d\" openStart=\"%d\" openEnd=\"%d\" bEnable=\"%d\"/>.........\n",
//	       name, entry+1,ptr->appName,protocol[ptr->flag.tProto],protocol[ptr->flag.oProto],
//	       ptr->tStart,ptr->tEnd,ptr->oStart,ptr->oEnd,ptr->bEnable);
       fprintf(file,"<%s id=\"%d\" appName=\"%s\" triggerProtocol=\"%s\" openProtocol=\"%s\" triggerStart=\"%d\" triggerEnd=\"%d\" openStart=\"%d\" openEnd=\"%d\" bEnable=\"%d\"/>\n",
	       name, entry+1,ptr->appName,protocol[ptr->flag.tProto],protocol[ptr->flag.oProto],
	       ptr->tStart,ptr->tEnd,ptr->oStart,ptr->oEnd,ptr->bEnable);
//modifyed by l66195 for VDF end
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}


void xmlSetMacFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			  const UINT16 length)
{
   PSEC_MAC_FLT_ENTRY macFilter, ptr;
   int entry = 0;
   int num = 0;
   static char* protocol[] = {"none", "PPPoE", "IPv4", "IPv6", "AppleTalk", "IPX", "NetBEUI", "IGMP"};
   static char* direction[] = {"LanToWan", "WanToLan", "both"};
   char srcMac[32], destMac[32];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_MAC_FILTER) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     macFilter = (PSEC_MAC_FLT_ENTRY)value;
     //num = length/sizeof(SEC_MAC_FLT_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &macFilter[entry];

       fprintf(file,"<%s id=\"%d\" ",ENTRYNAME_MAC_FILTER,entry+1);
       if (strcmp(ptr->wanIf,"ALL") == 0)
	 fprintf(file,"wanIf=\"ALL\" ");
       else {
//	 wanIfToPvc(ptr->wanIf,wanIfc);
	 fprintf(file,"wanIf=\"%s\" ",ptr->wanIf);
       }
       if ( strcmp(ptr->srcMac, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->srcMac, srcMac);
       else
         srcMac[0] = '\0';
       if ( strcmp(ptr->destMac, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->destMac, destMac);
       else
         destMac[0] = '\0';
       fprintf(file,"protocol=\"%s\" direction=\"%s\" destMac=\"%s\" srcMac=\"%s\"/>\n",
	       protocol[ptr->flag.protocol],direction[ptr->flag.direction],
	       destMac,srcMac);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}


void xmlSetAccessControlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			     const UINT16 length)
{
   PSEC_ACC_CNTR_ENTRY accCntrl, ptr;
   int entry = 0;
   int num = 0;

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_ACC_CTRL) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     accCntrl = (PSEC_ACC_CNTR_ENTRY)value;
     //num = length/sizeof(SEC_ACC_CNTR_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &accCntrl[entry];
       fprintf(file,"<%s id=\"%d\" ipAddr=\"%s\"/>\n",ENTRYNAME_ACC_CTRL,ptr->iHostId,ptr->address);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}

void xmlSetAccessControlModeAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				 const UINT16 length)
{
   int mode;
   static char* modeStr[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL))
      return;

   memcpy(&mode, value, length);
   fprintf(file,"<%s %s=\"%s\"/>\n", name,PSX_ATTR_STATE,modeStr[mode]);
}

void xmlSetQosAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PQOS_CLASS_ENTRY qos, ptr;
   int entry = 0;
   int num = 0;

   /*start -- w00135358 add for VDF Global QoS 20080908*/
   static char* etheProt[] = {"IP", "802.1Q", "PPPoE"};  
   /*end -- w00135358 add for VDF Global QoS 20080908*/
   
   /*start modifying for vdf qos by p44980 2008.01.11*/
   #ifdef SUPPORT_VDF_QOS
   static char* protocol[] = {"TCP/UDP", "TCP", "UDP", "ICMP", "IGMP"};
   #else
   static char* protocol[] = {"TCP/UDP", "TCP", "UDP", "ICMP"};
   #endif
   /*end  modifying for vdf qos by p44980 2008.01.11*/
#ifdef FOUR_LEVEL_QOS
   static char* priority[] = {"", "low", "medium", "high", "highest"};
#else
   /* start of protocol QoS for KPN by zhangchen z45221 2006年7月17日
   static char* priority[] = {"", "low", "medium", "high"};
   char proto[IFC_TINY_LEN];
   char srcMac[32];
   char dstMac[32];
   char srcMacMask[32];
   char dstMacMask[32];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     qos = (PQOS_CLASS_ENTRY)value;
     //num = length/sizeof(QOS_CLASS_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &qos[entry];
       if ( ptr->protocol >= 0 && ptr->protocol <= 3 )
         strcpy(proto, protocol[ptr->protocol]);
       else
         proto[0] = '\0';
       if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->srcMacAddr, srcMac);
       else
         srcMac[0] = '\0';
       if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->dstMacAddr, dstMac);
       else
         dstMac[0] = '\0';
       if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
       else
         srcMacMask[0] = '\0';
       if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
         bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
       else
         dstMacMask[0] = '\0';
       fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\"/>\n",
	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
               ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
               ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
               srcMacMask, dstMac, dstMacMask, ptr->dscpMark);
     } // for entry 
     if ( num != 0 )
       fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } // table 
   */
   #ifdef SUPPORT_VDSL
       #ifdef SUPPORT_KPN_QOS
       if (TM_ATM_BR== g_trafficmode || TM_PTM_BR == g_trafficmode)
       {
           static char* priority[] = {"", "low and low weight", "low and high weight", "medium", "high"};
           char proto[IFC_TINY_LEN];
           char srcMac[32];
           char dstMac[32];
           char srcMacMask[32];
           char dstMacMask[32];

           if ((name == NULL) || (value == NULL))
              return;

           if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
             num = *(UINT16*)value;
             if ( num != 0 )
               fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
           }
           else {
             qos = (PQOS_CLASS_ENTRY)value;
             //num = length/sizeof(QOS_CLASS_ENTRY);
             num = xmlGetTableSize(file);

             for (entry = 0; entry < num; entry++) {
               ptr = &qos[entry];
               if ( ptr->protocol >= 0 && ptr->protocol <= 3 )
                 strcpy(proto, protocol[ptr->protocol]);
               else
                 proto[0] = '\0';
               if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->srcMacAddr, srcMac);
               else
                 srcMac[0] = '\0';
               if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->dstMacAddr, dstMac);
               else
                 dstMac[0] = '\0';
               if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
               else
                 srcMacMask[0] = '\0';
               if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
               else
                 dstMacMask[0] = '\0';
               fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\"/>\n",
        	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
                       ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
                       ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                       srcMacMask, dstMac, dstMacMask, ptr->dscpMark);
             } /* for entry */
             if ( num != 0 )
               fprintf(file,"</%s>\n",secObjs[id-1].objName);
           } /* table */
       }
       else
       {
           static char* priority[] = {"", "low", "medium", "high"};
           char proto[IFC_TINY_LEN];
           char srcMac[32];
           char dstMac[32];
           char srcMacMask[32];
           char dstMacMask[32];

           if ((name == NULL) || (value == NULL))
              return;

           if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
             num = *(UINT16*)value;
             if ( num != 0 )
               fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
           }
           else {
             qos = (PQOS_CLASS_ENTRY)value;
             //num = length/sizeof(QOS_CLASS_ENTRY);
             num = xmlGetTableSize(file);

             for (entry = 0; entry < num; entry++) {
               ptr = &qos[entry];
               if ( ptr->protocol >= 0 && ptr->protocol <= 3 )
                 strcpy(proto, protocol[ptr->protocol]);
               else
                 proto[0] = '\0';
               if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->srcMacAddr, srcMac);
               else
                 srcMac[0] = '\0';
               if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->dstMacAddr, dstMac);
               else
                 dstMac[0] = '\0';
               if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
               else
                 srcMacMask[0] = '\0';
               if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
                 bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
               else
                 dstMacMask[0] = '\0';
               fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\"/>\n",
        	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
                       ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
                       ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                       srcMacMask, dstMac, dstMacMask, ptr->dscpMark);
             } /* for entry */
             if ( num != 0 )
               fprintf(file,"</%s>\n",secObjs[id-1].objName);
           } /* table */

       }
       /* start of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */
       #elif (defined(SUPPORT_TDE_QOS))
       static char* priority[] = {"", "low", "medium", "high"};
       static char* state[] = { "disable", "enable" };
       char proto[IFC_TINY_LEN];
       char srcMac[32];
       char dstMac[32];
       char srcMacMask[32];
       char dstMacMask[32];

       if ((name == NULL) || (value == NULL))
          return;

       if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
         num = *(UINT16*)value;
         if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
       }
       else {
         qos = (PQOS_CLASS_ENTRY)value;
         //num = length/sizeof(QOS_CLASS_ENTRY);
         num = xmlGetTableSize(file);

         for (entry = 0; entry < num; entry++) {
           ptr = &qos[entry];
           if ( ptr->protocol >= 0 && ptr->protocol <= 3 )
             strcpy(proto, protocol[ptr->protocol]);
           else
             proto[0] = '\0';
           if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacAddr, srcMac);
           else
             srcMac[0] = '\0';
           if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacAddr, dstMac);
           else
             dstMac[0] = '\0';
           if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
           else
             srcMacMask[0] = '\0';
           if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
           else
             dstMacMask[0] = '\0';
           fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\" trafficLimit=\"%s\" limitRate=\"%d\"/>\n",
    	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
                   ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
                   ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                   srcMacMask, dstMac, dstMacMask, ptr->dscpMark, state[ptr->enbTrafficLimit] , ptr->limitRate);
         } /* for entry */
         if ( num != 0 )
           fprintf(file,"</%s>\n",secObjs[id-1].objName);
       } /* table */
       /* end of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */
       #else
       static char* priority[] = {"", "low", "medium", "high"};
       char proto[IFC_TINY_LEN];
       char srcMac[32];
       char dstMac[32];
       char srcMacMask[32];
       char dstMacMask[32];

       if ((name == NULL) || (value == NULL))
          return;

       if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
         num = *(UINT16*)value;
         if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
       }
       else {
         qos = (PQOS_CLASS_ENTRY)value;
         //num = length/sizeof(QOS_CLASS_ENTRY);
         num = xmlGetTableSize(file);

         for (entry = 0; entry < num; entry++) {
           ptr = &qos[entry];
           if ( ptr->protocol >= 0 && ptr->protocol <= 3 )
             strcpy(proto, protocol[ptr->protocol]);
           else
             proto[0] = '\0';
           if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacAddr, srcMac);
           else
             srcMac[0] = '\0';
           if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacAddr, dstMac);
           else
             dstMac[0] = '\0';
           if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
           else
             srcMacMask[0] = '\0';
           if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
           else
             dstMacMask[0] = '\0';
           fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\"/>\n",
    	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
                   ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
                   ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                   srcMacMask, dstMac, dstMacMask, ptr->dscpMark);
         } /* for entry */
         if ( num != 0 )
           fprintf(file,"</%s>\n",secObjs[id-1].objName);
       } /* table */

       #endif
   #else
       static char* priority[] = {"", "low", "medium", "high"};
       char proto[IFC_TINY_LEN];
       char srcMac[32];
       char dstMac[32];
       char srcMacMask[32];
       char dstMacMask[32];

	   /*start -- w00135358 add for VDF Global QoS 20080908*/
       char etheProtTmp[IFC_TINY_LEN];
	   /*end -- w00135358 add for VDF Global QoS 20080908*/

       if ((name == NULL) || (value == NULL))
          return;

       if (strcmp(name,TBLNAME_QOS_CLS) == 0) {
         num = *(UINT16*)value;
         if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
       }
       else {
         qos = (PQOS_CLASS_ENTRY)value;
         //num = length/sizeof(QOS_CLASS_ENTRY);
         num = xmlGetTableSize(file);

         for (entry = 0; entry < num; entry++) {
           ptr = &qos[entry];
        /*start modifying for vdf qos by p44980 2008.01.12*/
           if ( (ptr->protocol >= 0 && ptr->protocol <= 3) 
          #ifdef SUPPORT_VDF_QOS
          || ptr->protocol == 4
          #endif
           	)
        /*start modifying for vdf qos by p44980 2008.01.12*/
             strcpy(proto, protocol[ptr->protocol]);
           else
             proto[0] = '\0';

		   /*start -- w00135358 add for VDF Global QoS 20080908*/
		   if( ptr->etheProt >= 0 && ptr->etheProt < ETHENET_PROTO_MAX )
           {	
           		strcpy( etheProtTmp, etheProt[ptr->etheProt] );
           }
		   else
		   {
 				etheProtTmp[0] = '\0';
		   }
		   /*end -- w00135358 add for VDF Global QoS 20080908*/

           
           if ( strcmp(ptr->srcMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacAddr, srcMac);
           else
             srcMac[0] = '\0';
           if ( strcmp(ptr->dstMacAddr, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacAddr, dstMac);
           else
             dstMac[0] = '\0';
           if ( strcmp(ptr->srcMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->srcMacMask, srcMacMask);
           else
             srcMacMask[0] = '\0';
           if ( strcmp(ptr->dstMacMask, WEB_SPACE) != 0 )
             bcmMacNumToStr(ptr->dstMacMask, dstMacMask);
           else
             dstMacMask[0] = '\0';

		   /*w00135358 start-modify for Global QoS 20080903*/           
           /*start modifying for vdf qos by p44980 2008.01.08*/
           #ifdef SUPPORT_VDF_QOS
           fprintf(file,"<%s id=\"%u\" type=\"%d\" clsName=\"%s\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" etheProt=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpCheck=\"%d\" dscpMark=\"%d\" vlanidCheck=\"%d\" vlanidMark=\"%d\" queueId=\"%u\" serviceName=\"%s\"/>\n",
    	       name,ptr->ulClsInstId,ptr->type,ptr->clsName,ptr->wanVlan8021p,ptr->lanIfcName,etheProtTmp,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
    	       	   ptr->dstAddr, ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                   srcMacMask, dstMac, dstMacMask, ptr->dscpCheck, ptr->dscpMark, ptr->vlanidCheck, ptr->vlanidMark,
                   ptr->ulQueueInstId, ptr->acServiecName);
           #else
           fprintf(file,"<%s id=\"%d\" type=\"%d\" clsName=\"%s\" priority=\"%s\" wlpriority=\"%d\" precedence=\"%d\" tos=\"%d\" wanVlan8021p=\"%d\" lanIfcName=\"%s\" protocol=\"%s\" srcAddr=\"%s\" srcMask=\"%s\" srcPort=\"%s\" dstAddr=\"%s\" dstMask=\"%s\" dstPort=\"%s\" vlan8021p=\"%d\" srcMacAddr=\"%s\" srcMacMask=\"%s\" dstMacAddr=\"%s\" dstMacMask=\"%s\" dscpMark=\"%d\"/>\n",
    	       name,entry+1,ptr->type,ptr->clsName,priority[ptr->priority],ptr->wlpriority,ptr->precedence,ptr->tos,
                   ptr->wanVlan8021p,ptr->lanIfcName,proto,ptr->srcAddr,ptr->srcMask,ptr->srcPort,
                   ptr->dstAddr,ptr->dstMask,ptr->dstPort,ptr->vlan8021p, srcMac, 
                   srcMacMask, dstMac, dstMacMask, ptr->dscpMark);
           #endif
           /*end modifying for vdf qos by p44980 2008.01.08*/
		   /*w00135358 end-modify for Global QoS 20080903*/  
         } /* for entry */
         if ( num != 0 )
           fprintf(file,"</%s>\n",secObjs[id-1].objName);
       } /* table */

   #endif
   /* end of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */
#endif

}

#ifdef SUPPORT_TOD
void xmlSetToDCntrlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   PTOD_ENTRY tod, ptr;
   int entry = 0;
   int num = 0;
   char mac[32];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_PARENT_CNTRL) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     tod = (PTOD_ENTRY)value;
     //num = length/sizeof(TOD_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &tod[entry];
       bcmMacNumToStr(ptr->mac,mac);

       fprintf(file,"<%s id=\"%d\" username=\"%s\" mac=\"%s\" startTime=\"%.2d:%.2d\" endTime=\"%.2d:%.2d\" days=",
	       name,entry+1,ptr->username,mac,
               (ptr->start_time/60),(ptr->start_time%60),
	       (ptr->end_time/60),(ptr->end_time%60));
       fprintf(file,"\""); /* beginning of days */
       if (ptr->days & MONDAY)
	 fprintf(file,"Mon");
       if (ptr->days & TUESDAY)
	 fprintf(file,"Tue");
       if (ptr->days & WEDNSDAY)
	 fprintf(file,"Wed");
       if (ptr->days & THURSDAY)
	 fprintf(file,"Thu");
       if (ptr->days & FRIDAY)
	 fprintf(file,"Fri");
       if (ptr->days & SATURDAY)
	 fprintf(file,"Sat");
       if (ptr->days & SUNDAY)
	 fprintf(file,"Sun");
       fprintf(file,"\"/>\n");
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",ToDObjs[id+1].objName);
   } /* table */
}
#endif /* #ifdef SUPPORT_TOD */

#ifdef SUPPORT_MAC
void xmlSetMacAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   PMAC_ENTRY tod, ptr;
   int entry = 0;
   int num = 0;
   char mac[32];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_MAC) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     tod = (PMAC_ENTRY)value;
     //num = length/sizeof(TOD_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &tod[entry];
       bcmMacNumToStr(ptr->mac,mac);

       fprintf(file,"<%s id=\"%d\" username=\"%s\" mac=\"%s\"/>\n",
	       name,entry+1,ptr->username,mac);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",MacObjs[id+1].objName);
   } /* table */
}
#endif /* #ifdef SUPPORT_MAC */

#ifdef SUPPORT_MACMATCHIP
void xmlSetMacMatchAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length)
{
   PMACMATCH_ENTRY tod, ptr;
   int entry = 0;
   int num = 0;
   char mac[32];
   char ipaddress[16];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_MACMATCH) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     tod = (PMACMATCH_ENTRY)value;
     //num = length/sizeof(TOD_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &tod[entry];
       bcmMacNumToStr(ptr->mac,mac);
       strcpy(ipaddress, ptr->ipaddress);
       fprintf(file,"<%s id=\"%d\" username=\"%s\" mac=\"%s\" ip=\"%s\"/>\n",
	       name,entry+1,ptr->username,mac,ipaddress);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",MacMatchObjs[id+1].objName);
   } /* table */
}
#endif 

#ifdef SUPPORT_DDNSD
void xmlSetDdnsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
   PDDNS_SERVICE_INFO ddns, ptr;
   int entry = 0;
   int num = 0;
   static char* serviceStr[] = {"tzo", "dyndns"};

   if ((name == NULL) || (value == NULL))
      return;
   char *base64 = NULL; // add by l66195 for VDF

   if (strcmp(name,TBLNAME_DDNS) == 0) {
     /* if the table size is 0, then we just ignore this */
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     ddns = (PDDNS_SERVICE_INFO)value;
     //num = length/sizeof(DDNS_SERVICE_INFO);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &ddns[entry];
  
	 #ifndef SUPPORT_DDNSWD_UNENCRYPTED
       base64Encode((unsigned char *)ptr->password, strlen(ptr->password), &base64);
  //modified by l66195 for VDF start
     //  fprintf(file,"<%s id=\"%d\" hostname=\"%s\" username=\"%s\" password=\"%s\" ifc=\"%s\" service=\"%s\"/>\n",
	 //      name,entry+1,ptr->hostname,ptr->username,ptr->password,ptr->iface,serviceStr[ptr->service]);
       fprintf(file,"<%s id=\"%d\" hostname=\"%s\" username=\"%s\" password=\"%s\" ifc=\"%s\" service=\"%s\" bSelect=\"%d\"/>\n",
	       name,entry+1,ptr->hostname,ptr->username,base64,ptr->iface,serviceStr[ptr->service],ptr->bSelect);
       free(base64);
	#else
	fprintf(file,"<%s id=\"%d\" hostname=\"%s\" username=\"%s\" password=\"%s\" ifc=\"%s\" service=\"%s\" bSelect=\"%d\"/>\n",
		name,entry+1,ptr->hostname,ptr->username,ptr->password,ptr->iface,serviceStr[ptr->service],ptr->bSelect);
	#endif
    //modified by l66195 for VDF end
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",DDnsObjs[id-1].objName);
   } /* table */
}
#endif /* #ifdef SUPPORT_DDNSD */

#ifdef VOXXXLOAD
PSX_STATUS xmlSetVoiceNode(FILE* file, const char *appName, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(voiceObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(voiceObjs[id].setAttrFnc) != NULL)
     (*(voiceObjs[id].setAttrFnc))(file, voiceObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlGetVoiceNode(char *appName, char *objName, char *attrName, char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetVoiceNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; voiceObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(voiceObjs[i].objName, objName) == 0) {
      if (*(voiceObjs[i].getAttrFnc) != NULL) {
        sts = (*(voiceObjs[i].getAttrFnc))(attrName, attrValue);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetVoiceNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  return sts;
} 

PSX_STATUS xmlStartVoiceNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartVoiceNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; voiceObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(voiceObjs[i].objName, objName) == 0) {
      if (*(voiceObjs[i].startObjFnc) != NULL) {
        sts = (*(voiceObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartVoiceNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  return sts;
} 

PSX_STATUS xmlEndVoiceNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlEndVoiceNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; voiceObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(voiceObjs[i].objName, objName) == 0) {
      if (*(voiceObjs[i].endObjFnc) != NULL) {
        sts = (*(voiceObjs[i].endObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndVoiceNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  return sts;
} 

void xmlSetMgcpAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PVODSL_MGCP_PARMS mgcp;

   if ((name == NULL) || (value == NULL))
     return;

   mgcp = (PVODSL_MGCP_PARMS)value;

   fprintf(file,
#if PSTN_SUPPORT
           "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#else
           "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#endif /* PSTN_SUPPORT */
           PSX_ATTR_VOICE_MGCP_TAG,
           PSX_ATTR_VOICE_MGCP_CA_ADDR,mgcp->caIpAddrStr,
           PSX_ATTR_VOICE_MGCP_GW_NAME,mgcp->gwName,
           PSX_ATTR_VOICE_MGCP_IF_NAME,mgcp->ifName,
           PSX_ATTR_VOICE_MGCP_PREF_CODEC,mgcp->prefCodec,
           PSX_ATTR_VOICE_LOCALE_NAME,mgcp->localeName
#if PSTN_SUPPORT
          , PSX_ATTR_VOICE_PSTN_ROUTE_RULE,mgcp->pstnRoutingRule,
          PSX_ATTR_VOICE_PSTN_ROUTE_DATA,mgcp->pstnRoutingData
#endif /* PSTN_SUPPORT */
          );
}

void xmlSetSipAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
                   
{
  PVODSL_SIP_PARMS sip;

  if ((name == NULL) || (value == NULL))
    return;

  sip = (PVODSL_SIP_PARMS)value;

  fprintf(file,
#ifdef STUN_CLIENT
#   if PSTN_SUPPORT
          "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#   else
          "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#   endif /* PSTN_SUPPORT */
#else
#   if PSTN_SUPPORT
          "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#   else
          "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
#   endif /* PSTN_SUPPORT */
#endif /* STUN_CLIENT */
          PSX_ATTR_VOICE_SIP_TAG,
          PSX_ATTR_VOICE_SIP_PROXY_ADDR,sip->proxyAddrStr,
          PSX_ATTR_VOICE_SIP_PROXY_PORT,sip->proxyPortStr,
          PSX_ATTR_VOICE_SIP_OBPROXY_ADDR,sip->obProxyAddrStr,
          PSX_ATTR_VOICE_SIP_OBPROXY_PORT,sip->obProxyPortStr,
          PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR,sip->registrarAddrStr,
          PSX_ATTR_VOICE_SIP_REGISTRAR_PORT,sip->registrarPortStr,
		  PSX_ATTR_VOICE_SIP_TAG_MATCHING, sip->tagMatchStr,
          PSX_ATTR_VOICE_SIP_LOG_ADDR,sip->logAddrStr,
          PSX_ATTR_VOICE_SIP_LOG_PORT,sip->logPortStr,
          PSX_ATTR_VOICE_SIP_EXT1,sip->extension1Str,
          PSX_ATTR_VOICE_SIP_PASSWORD1,sip->password1Str,
          PSX_ATTR_VOICE_SIP_EXT2,sip->extension2Str,
          PSX_ATTR_VOICE_SIP_PASSWORD2,sip->password2Str,
          PSX_ATTR_VOICE_SIP_EXT3,sip->extension3Str,
          PSX_ATTR_VOICE_SIP_PASSWORD3,sip->password3Str,
          PSX_ATTR_VOICE_SIP_EXT4,sip->extension4Str,
          PSX_ATTR_VOICE_SIP_PASSWORD4,sip->password4Str,
          PSX_ATTR_VOICE_SIP_IF_NAME,sip->ifName,
          PSX_ATTR_VOICE_SIP_DOMAIN_NAME,sip->domainName,
          PSX_ATTR_VOICE_LOCALE_NAME,sip->localeName,
          PSX_ATTR_VOICE_CODEC_NAME,sip->voiceCodec,
          PSX_ATTR_VOICE_PTIME,sip->voicePtime
#if PSTN_SUPPORT
          , PSX_ATTR_VOICE_PSTN_ROUTE_RULE,sip->pstnRoutingRule,
          PSX_ATTR_VOICE_PSTN_ROUTE_DATA,sip->pstnRoutingData
#endif /* PSTN_SUPPORT */
#ifdef STUN_CLIENT
		  , PSX_ATTR_VOICE_STUN_SRV_ADDR, sip->stunServerAddrStr,
		  PSX_ATTR_VOICE_STUN_SRV_PORT, sip->stunServerPortStr
#endif /* STUN_CLIENT */
          );
}

PSX_STATUS xmlStartMgcpObjNode(char *appName, char *objName)
{
  PVODSL_MGCP_PARMS mgcp;

#ifdef XML_DEBUG
  printf("xmlStartMgcpObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(VODSL_MGCP_PARMS));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain  PSI移植: 分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(VODSL_MGCP_PARMS));
  mgcp = (PVODSL_MGCP_PARMS)psiValue;
  strcpy(mgcp->caIpAddrStr,DEFAULT_CA_IPADDR_STRING);
  strcpy(mgcp->gwName,DEFAULT_GWNAME_STRING);
  strcpy(mgcp->ifName,DEFAULT_IFNAME_STRING);
  strcpy(mgcp->prefCodec,DEFAULT_PREFCODEC_STRING);
  strcpy(mgcp->localeName,DEFAULT_LOCALE_NAME_STRING);
#if PSTN_SUPPORT
  strcpy(mgcp->pstnRoutingRule,DEFAULT_PSTN_ROUTING_RULE_STRING);
  strcpy(mgcp->pstnRoutingData,DEFAULT_PSTN_ROUTING_DATA_STRING);
#endif /* PSTN_SUPPORT */

  return PSX_STS_OK;
}

PSX_STATUS xmlEndMgcpObjNode(char *appName, char *objName)
{
  PVODSL_MGCP_PARMS mgcp = (PVODSL_MGCP_PARMS)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlEndMgcpObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  if (xmlState.verify) {
    status = BcmDb_validateIpAddress(mgcp->caIpAddrStr);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndMgcpObj(): invalid %s %s\n",PSX_ATTR_VOICE_MGCP_CA_ADDR,mgcp->caIpAddrStr);
    }
    status = BcmDb_validateCodec(mgcp->prefCodec);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndMgcpObj(): invalid %s %s\n",PSX_ATTR_VOICE_MGCP_PREF_CODEC,mgcp->prefCodec);
    }
    status = BcmDb_validateLocale(mgcp->localeName);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndMgcpObj(): invalid %s %s\n",PSX_ATTR_VOICE_LOCALE_NAME,mgcp->localeName);
    }
  }
  else {
    BcmPsi_objStore(BcmPsi_appOpen(VOICE_APPID), VOICE_MGCPINFO_ID, mgcp, sizeof(VODSL_MGCP_PARMS));
  }

#ifdef XML_DEBUG1
  printf("\n============End MGCP info=========\n");
  printf("xmlEndMgcpObj(): caIpAddr %s, gwName %s, ifName %s, prefCodec %s, locale %s\n",
         mgcp->caIpAddrStr,mgcp->gwName,mgcp->ifName,mgcp->prefCodec, mgcp->localeName);
#if PSTN_SUPPORT
  printf("xmlEndMgcpObj(): pstnRoutingRule %s, pstnRoutingData %s\n",
         mgcp->pstnRoutingRule,mgcp->pstnRoutingData);
#endif /* PSTN_SUPPORT */
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetMgcpAttr(char *attrName, char* attrValue)
{
  PVODSL_MGCP_PARMS mgcp = (PVODSL_MGCP_PARMS)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetMgcpAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strcasecmp(attrName,PSX_ATTR_VOICE_MGCP_CA_ADDR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMgcpAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->caIpAddrStr,attrValue,len);
      mgcp->caIpAddrStr[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_MGCP_GW_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_GWNAME_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_MGCP_GW_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->gwName,attrValue,len);
      mgcp->gwName[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_MGCP_IF_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_IFNAME_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_MGCP_IF_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->ifName,attrValue,len);
      mgcp->ifName[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_MGCP_PREF_CODEC) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateCodec(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_MGCP_PREF_CODEC,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->prefCodec,attrValue,len);
      mgcp->prefCodec[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_LOCALE_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLocale(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_LOCALE_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->localeName,attrValue,len);
      mgcp->localeName[len] = '\0';
    }
  }
#if PSTN_SUPPORT
  if (strcasecmp(attrName,PSX_ATTR_VOICE_PSTN_ROUTE_RULE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PORT_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_PSTN_ROUTE_RULE,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->pstnRoutingRule,attrValue,len);
      mgcp->pstnRoutingRule[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_PSTN_ROUTE_DATA) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange( mgcp->pstnRoutingData, 0, XML_PORT_RANGE_MAX );
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_PSTN_ROUTE_DATA,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(mgcp->pstnRoutingData,attrValue,len);
      mgcp->pstnRoutingData[len] = '\0';
    }
  }
#endif /* PSTN_SUPPORT */
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSipObjNode(char *appName, char *objName)
{
  PVODSL_SIP_PARMS sip;

#ifdef XML_DEBUG
  printf("xmlStartSipObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(VODSL_SIP_PARMS));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(VODSL_SIP_PARMS));
  sip = (PVODSL_SIP_PARMS)psiValue;
  strcpy( sip->proxyAddrStr, DEFAULT_PROXY_IPADDR_STRING );
  strcpy( sip->proxyPortStr, DEFAULT_PROXY_PORT_STRING );
  strcpy( sip->obProxyAddrStr, DEFAULT_PROXY_IPADDR_STRING );
  strcpy( sip->obProxyPortStr, DEFAULT_PROXY_PORT_STRING );
  strcpy( sip->registrarAddrStr, DEFAULT_REGISTRAR_IPADDR_STRING );
  strcpy( sip->registrarPortStr, DEFAULT_REGISTRAR_PORT_STRING );
  strcpy( sip->logAddrStr, DEFAULT_LOG_IPADDR_STRING );
  strcpy( sip->tagMatchStr, DEFAULT_TAG_MATCH_VALUE );	
  strcpy( sip->logPortStr, DEFAULT_LOG_PORT_STRING );
  strcpy( sip->extension1Str,  DEFAULT_EXTENSION_STRING );
  strcpy( sip->extension2Str,  DEFAULT_EXTENSION_STRING );
  strcpy( sip->extension3Str, DEFAULT_EXTENSION_STRING );
  strcpy( sip->extension4Str, DEFAULT_EXTENSION_STRING );
  strcpy( sip->password1Str, DEFAULT_PASSWORD_STRING );
  strcpy( sip->password2Str, DEFAULT_PASSWORD_STRING );
  strcpy( sip->password3Str, DEFAULT_PASSWORD_STRING );
  strcpy( sip->password4Str,  DEFAULT_PASSWORD_STRING );
  strcpy( sip->ifName,  DEFAULT_IFNAME_STRING );
  strcpy( sip->domainName,  DEFAULT_DOMAIN_NAME_STRING );
  strcpy( sip->localeName,  DEFAULT_LOCALE_NAME_STRING );
  strcpy( sip->voiceCodec,  DEFAULT_CODEC_NAME_STRING );
  strcpy( sip->voicePtime,  DEFAULT_PTIME_STRING );
#if PSTN_SUPPORT
  strcpy( sip->pstnRoutingRule, DEFAULT_PSTN_ROUTING_RULE_STRING );
  strcpy( sip->pstnRoutingData, DEFAULT_PSTN_ROUTING_DATA_STRING );
#endif /* PSTN_SUPPORT */
#ifdef STUN_CLIENT
  strcpy( sip->stunServerAddrStr, DEFAULT_STUN_SERVER_IPADDR_STRING );
  strcpy( sip->stunServerPortStr, DEFAULT_STUN_SERVER_PORT_STRING );
#endif /* STUN_CLIENT */

  return PSX_STS_OK;
}

PSX_STATUS xmlEndSipObjNode(char *appName, char *objName)
{
  PVODSL_SIP_PARMS sip = (PVODSL_SIP_PARMS)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlEndSipObj(calling cfm to get appName %s, objName %s, verify %d\n",
         appName,objName, xmlState.verify);
#endif

  if (xmlState.verify) {
    status = BcmDb_validateLength(sip->proxyAddrStr,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PROXY_ADDR,sip->proxyAddrStr);
    }
    status = BcmDb_validateRange(sip->proxyPortStr,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PROXY_PORT,sip->proxyPortStr);
    }
    status = BcmDb_validateLength(sip->obProxyAddrStr,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_OBPROXY_ADDR,sip->obProxyAddrStr);
    }
    status = BcmDb_validateRange(sip->obProxyPortStr,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_OBPROXY_PORT,sip->obProxyPortStr);
    }
    status = BcmDb_validateLength(sip->registrarAddrStr,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR,sip->registrarAddrStr);
    }
    status = BcmDb_validateRange(sip->registrarPortStr,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_REGISTRAR_PORT,sip->registrarPortStr);
    }
	status = BcmDb_validateLength(sip->tagMatchStr,CFG_MAX_TAG_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_TAG_MATCHING,sip->tagMatchStr);
    }
    status = BcmDb_validateIpAddress(sip->logAddrStr);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_LOG_ADDR,sip->logAddrStr);
    }
    status = BcmDb_validateRange(sip->logPortStr,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_LOG_PORT,sip->logPortStr);
    }
    status = BcmDb_validateLength(sip->domainName,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_DOMAIN_NAME,sip->domainName);
    }
    status = BcmDb_validateLocale(sip->localeName);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_LOCALE_NAME,sip->localeName);
    }
    status = BcmDb_validateVoiceCodec(sip->voiceCodec);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_CODEC_NAME,sip->voiceCodec);
    }
    status = BcmDb_validateLength(sip->voicePtime,CFG_MAX_PTIME_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_PTIME,sip->voicePtime);
    }
#if PSTN_SUPPORT
    status = BcmDb_validateLength( sip->pstnRoutingRule, CFG_MAX_PORT_STR_LEN );
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlEndSipObj(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_PSTN_ROUTE_RULE, sip->pstnRoutingRule );
    }
    status = BcmDb_validateRange( sip->pstnRoutingData, 0, XML_PORT_RANGE_MAX );
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlEndSipObj(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_PSTN_ROUTE_DATA, sip->pstnRoutingData );
    }
#endif /* PSTN_SUPPORT */
#ifdef STUN_CLIENT
    status = BcmDb_validateLength( sip->stunServerAddrStr, CFG_MAX_HOST_STR_LEN );
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlEndSipObj(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_STUN_SRV_ADDR, sip->stunServerAddrStr );
    }
    status = BcmDb_validateRange( sip->stunServerPortStr, 0, XML_PORT_RANGE_MAX );
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlEndSipObj(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_STUN_SRV_PORT, sip->stunServerPortStr );
    }
#endif /* STUN_CLIENT */
  }
  else {
    if (xmlState.errorFlag !=  PSX_STS_ERR_FATAL)
    BcmPsi_objStore(BcmPsi_appOpen(VOICE_APPID), VOICE_SIPINFO_ID, sip, sizeof(VODSL_SIP_PARMS));
  }

#ifdef XML_DEBUG1
  printf("\n============End SIP info=========\n");
  printf("xmlEndSipObj(): proxyAddrStr %s, proxyPortStr %s, registrarAddrStr %s, registrarPortStr %s\n",
         sip->proxyAddrStr,sip->proxyPortStr,sip->registrarAddrStr,sip->registrarPortStr);
  printf("xmlEndSipObj(): logAddrStr %s, logPortStr%s,  extension1Str %s,  password1Str %s\n",
         sip->logAddrStr,sip->logPortStr,sip->extension1Str,sip->extension1Str);
  printf("xmlEndSipObj(): extension2Str %s,  password2Str %s, extension3Str %s,  password3Str %s\n",
         sip->extension2Str,sip->password2Str,sip->extension3Str,sip->password3Str);
  printf("xmlEndSipObj(): extension4Str %s,  password4Str %s, ifName %s, domain %s, locale %s\n",
         sip->extension4Str,sip->password4Str,sip->ifName,sip->domainName,sip->localeName);
  printf("xmlEndSipObj(): pref. codec %s, pref. ptime %s, obProxyAddrStr %s, obProxyPortStr %s\n",
         sip->voiceCodec,sip->voicePtime,sip->obProxyAddrStr,sip->obProxyPortStr);
#if PSTN_SUPPORT
  printf("xmlEndSipObj(): pstnRoutingRule %s, pstnRoutingData %s\n",
         sip->pstnRoutingRule,sip->pstnRoutingData);
#endif /* PSTN_SUPPORT */
#ifdef STUN_CLIENT
  printf("xmlEndSipObj(): stunSrvAddr %s,  stunSrvPort %s\n",
         sip->stunServerAddrStr, sip->stunServerPortStr );
#endif /* STUN_CLIENT */
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetSipAttr(char *attrName, char* attrValue)
{
  PVODSL_SIP_PARMS sip = (PVODSL_SIP_PARMS)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetSipAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PROXY_ADDR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PROXY_ADDR,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->proxyAddrStr,attrValue,len);
      sip->proxyAddrStr[len] = '\0';
    }
  } /* proxyAddrStr */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PROXY_PORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PROXY_PORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->proxyPortStr,attrValue,len);
      sip->proxyPortStr[len] = '\0';
    }
  } /* proxy Port */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_OBPROXY_ADDR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_OBPROXY_ADDR,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->obProxyAddrStr,attrValue,len);
      sip->obProxyAddrStr[len] = '\0';
    }
  } /* outbound proxy address */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_OBPROXY_PORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_OBPROXY_PORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->obProxyPortStr,attrValue,len);
      sip->obProxyPortStr[len] = '\0';
    }
  } /* outbound proxy port */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->registrarAddrStr,attrValue,len);
      sip->registrarAddrStr[len] = '\0';
    }
  } /* registrar addr */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_REGISTRAR_PORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_REGISTRAR_PORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->registrarPortStr,attrValue,len);
      sip->registrarPortStr[len] = '\0';
    }
  } /* registrar port */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_TAG_MATCHING) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_TAG_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_TAG_MATCHING,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->tagMatchStr,attrValue,len);
      sip->tagMatchStr[len] = '\0';
    }
  } /* tag matching */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_LOG_ADDR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_LOG_ADDR,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->logAddrStr,attrValue,len);
      sip->logAddrStr[len] = '\0';
    }
  } /* logAddr */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_LOG_PORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_LOG_PORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->logPortStr,attrValue,len);
      sip->logPortStr[len] = '\0';
    }
  } /* logPort */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_EXT1) == 0) {
    if (xmlState.verify)
	{
      if ((status = (BcmDb_validateLength(attrValue,CFG_MAX_EXTENSION_STR_LEN))) == DB_OBJ_VALID_OK)
	  {
	    if( strlen( attrValue ) > 0 )
		{
        	status = DB_OBJ_VALID_OK;
		}
	  }
	}
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_EXT1,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->extension1Str,attrValue,len);
      sip->extension1Str[len] = '\0';
    }
  } /* extension1 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_EXT2) == 0) {
    if (xmlState.verify)
	{
      if ((status = (BcmDb_validateLength(attrValue,CFG_MAX_EXTENSION_STR_LEN))) == DB_OBJ_VALID_OK)
	  {
	    if( strlen( attrValue ) > 0 )
		{
        	status = DB_OBJ_VALID_OK;
		}
	  }
	}
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_EXT2,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->extension2Str,attrValue,len);
      sip->extension2Str[len] = '\0';
    }
  } /* extension2 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_EXT3) == 0) {
    if (xmlState.verify)
	{
      if ((status = (BcmDb_validateLength(attrValue,CFG_MAX_EXTENSION_STR_LEN))) == DB_OBJ_VALID_OK)
	  {
	    if( strlen( attrValue ) > 0 )

		{
        	status = DB_OBJ_VALID_OK;
		}
	  }
	}
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_EXT3,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->extension3Str,attrValue,len);
      sip->extension3Str[len] = '\0';
    }
  } /* extension3 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_EXT4) == 0) {
    if (xmlState.verify)
	{
      if ((status = (BcmDb_validateLength(attrValue,CFG_MAX_EXTENSION_STR_LEN))) == DB_OBJ_VALID_OK)
	  {
	    if( strlen( attrValue ) > 0 )
		{
        	status = DB_OBJ_VALID_OK;
		}
	  }
	}
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_EXT4,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->extension4Str,attrValue,len);
      sip->extension4Str[len] = '\0';
    }
  } /* extension4 */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PASSWORD1) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PASSWORD_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PASSWORD1,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->password1Str,attrValue,len);
      sip->password1Str[len] = '\0';
    }
  } /* password1 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PASSWORD2) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PASSWORD_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PASSWORD2,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->password2Str,attrValue,len);
      sip->password2Str[len] = '\0';
    }
  } /* password2 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PASSWORD3) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PASSWORD_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PASSWORD3,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->password3Str,attrValue,len);
      sip->password3Str[len] = '\0';
    }
  } /* password3 */
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_PASSWORD4) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PASSWORD_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PASSWORD4,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->password4Str,attrValue,len);
      sip->password4Str[len] = '\0';
    }
  } /* password4 */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_IF_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateInterfaceName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_IF_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->ifName,attrValue,len);
      sip->ifName[len] = '\0';
    }
  }  /* ifname */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_DOMAIN_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_HOST_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_DOMAIN_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->domainName,attrValue,len);
      sip->domainName[len] = '\0';
    }
  } /* domainName */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_LOCALE_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLocale(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_LOCALE_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->localeName,attrValue,len);
      sip->localeName[len] = '\0';
    }
  } /* localName */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_CODEC_NAME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateVoiceCodec(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_CODEC_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->voiceCodec,attrValue,len);
      sip->voiceCodec[len] = '\0';
    }
  } /* voiceCodec */

  if (strcasecmp(attrName,PSX_ATTR_VOICE_PTIME) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PTIME_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipObj(): invalid %s %s\n",PSX_ATTR_VOICE_PTIME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->voicePtime,attrValue,len);
      sip->voicePtime[len] = '\0';
    }
  } /* voicePtime */

#if PSTN_SUPPORT
  if (strcasecmp(attrName,PSX_ATTR_VOICE_PSTN_ROUTE_RULE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,CFG_MAX_PORT_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_PSTN_ROUTE_RULE,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->pstnRoutingRule,attrValue,len);
      sip->pstnRoutingRule[len] = '\0';
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_PSTN_ROUTE_DATA) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 0, XML_PORT_RANGE_MAX );
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVoiceAttr(): invalid %s %s\n",PSX_ATTR_VOICE_PSTN_ROUTE_DATA,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sip->pstnRoutingData,attrValue,len);
      sip->pstnRoutingData[len] = '\0';
    }
  }
#endif /* PSTN_SUPPORT */

#ifdef STUN_CLIENT
  if( strcasecmp( attrName, PSX_ATTR_VOICE_STUN_SRV_ADDR ) == 0 ) 
  {
    if( xmlState.verify )
    { 
      status = BcmDb_validateLength( attrValue, CFG_MAX_HOST_STR_LEN );
    }
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlEndSipObj(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_STUN_SRV_ADDR, attrValue );
    }
    else 
    {
      len = strlen( attrValue );
      strncpy( sip->stunServerAddrStr, attrValue, len );
      sip->stunServerAddrStr[len] = '\0';
    }
  } /* stunSrvAddr */
  if( strcasecmp( attrName, PSX_ATTR_VOICE_STUN_SRV_PORT) == 0 ) 
  {
    if( xmlState.verify )
    { 
      status = BcmDb_validateRange( attrValue, 0, XML_PORT_RANGE_MAX );
    }
    if( status != DB_OBJ_VALID_OK ) 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog( LOG_ERR, "xmlGetSipAttr(): invalid %s %s\n",
      		  PSX_ATTR_VOICE_STUN_SRV_PORT, attrValue );
    }
    else 
    {
      len = strlen( attrValue );
      strncpy( sip->stunServerPortStr, attrValue, len );
      sip->stunServerPortStr[len] = '\0';
    }
  } /* stunSrvPort */
#endif /* STUN_CLIENT */

  return(PSX_STS_OK);
}

/* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
// SIP Server Table
void  xmlSetSipSrvAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length)
{
   PSIP_WEB_SERVER_S pstSipSrv, ptr;
   int entry = 0;
   int num = 0;

   if (( name == NULL) || (value == NULL))
   {
      return;
   }

   if (strcmp(name,TBLNAME_SIP_SRV) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     pstSipSrv = (PSIP_WEB_SERVER_S)value;
     num = xmlGetTableSize(file);
     for (entry = 0; entry < num; entry++) {
       ptr = &pstSipSrv[entry];
       fprintf(file,"<%s id=\"%d\" %s=\"%s\" %s=\"%d\" %s=\"%s\" %s=\"%d\" %s=\"%s\"/>\n",
        ENTRYNAME_SIP_SRV,              entry+1,
        PSX_ATTR_VOICE_SIP_REGSRV_IP,   ptr->stSipRegAddr.szIPAddr,
        PSX_ATTR_VOICE_SIP_REGSRV_PORT, ptr->stSipRegAddr.uwPort,
        PSX_ATTR_VOICE_SIP_PRXYSRV_IP,  ptr->stSipProxyAddr.szIPAddr,
        PSX_ATTR_VOICE_SIP_PRXYSRV_PORT,ptr->stSipProxyAddr.uwPort,
        PSX_ATTR_VOICE_SIP_DOMAIN,      ptr->acDomainName);
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",voiceObjs[id-1].objName);
   } /* table */
}

PSX_STATUS xmlGetSipSrvAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG_SRV
  printf("xmlGetSipSrvAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SIP_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SIP_WEB_SERVER_S) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SIP_WEB_SERVER_S) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}
PSX_STATUS xmlEndSipSrvObjNode(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG_SRV
  printf("xmlEndSipSrvObjNode(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipSrvObjNode(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
      BcmDb_setSipSrvSize(xmlState.tableSize);
      BcmDb_setSipSrvInfo((PSIP_WEB_SERVER_S)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG_SRV
  printf("\n============End Access Control=========\n");
  printf("xmlEndSipSrvObjNode(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

//SIP Server Entry
PSX_STATUS xmlGetSipSrvEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSIP_WEB_SERVER_S   pstSipSrvTbl = (PSIP_WEB_SERVER_S)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG_SRV
  printf("xmlGetSipSrvEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,PSX_ATTR_VOICE_SIP_REGSRV_IP) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_NAME_LEN);
    if (status == DB_OBJ_VALID_OK){
      strncpy(pstSipSrvTbl[i].stSipRegAddr.szIPAddr, attrValue, SIP_MAX_NAME_LEN - 1);
      pstSipSrvTbl[i].stSipRegAddr.szIPAddr[SIP_MAX_NAME_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid SIP Registar Address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_REGSRV_PORT) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      pstSipSrvTbl[i].stSipRegAddr.uwPort = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid SIP Registar Port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_PRXYSRV_IP) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_NAME_LEN);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipSrvTbl[i].stSipProxyAddr.szIPAddr, attrValue, SIP_MAX_NAME_LEN - 1);
      pstSipSrvTbl[i].stSipProxyAddr.szIPAddr[SIP_MAX_NAME_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid SIP Registar Address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_PRXYSRV_PORT) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      pstSipSrvTbl[i].stSipProxyAddr.uwPort = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid SIP Registar Address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_DOMAIN) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, SIP_MAX_NAME_LEN);
    if (status == DB_OBJ_VALID_OK) {
      strncpy(pstSipSrvTbl[i].acDomainName, attrValue, SIP_MAX_NAME_LEN - 1);
      pstSipSrvTbl[i].acDomainName[SIP_MAX_NAME_LEN - 1] = '\0';
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipSrvEntryAttr(): invalid SIP Domain %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
} 

PSX_STATUS xmlEndSipSrvObjEntryNode(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

// SIP BASIC
void  xmlSetSipBscAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length)
{
   PSIP_WEB_USER_INFO_S pstSipBsc, ptr;
   int entry = 0;
   int num = 0;
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
	char *wbuf = NULL;
	char *pBuf = NULL;
	int i=0;
/* BEGIN: Added by chenzhigang, 2008/3/24   问题单号:将SIP密码加密*/
    char *base64 = NULL;
    char str[IFC_LARGE_LEN];
/* END:   Added by chenzhigang, 2008/3/24 */	
#endif  

   if (( name == NULL) || (value == NULL))
   {
      return;
   }

   if (strcmp(name,TBLNAME_SIP_BSC) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     pstSipBsc = (PSIP_WEB_USER_INFO_S)value;
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       ptr = &pstSipBsc[entry];
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
		wbuf = (char *)malloc(PSX_SIP_STRPAPD_BUFSIZE);
		if (NULL == wbuf)
		{
	         xmlMemProblem();
	         return;
		}
		memset(wbuf,0,PSX_SIP_STRPAPD_BUFSIZE);
		/* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
		pBuf = wbuf;
		/*add by w00135351 09.3.14 去掉加密*/
		#ifndef SUPPORT_SIPBSCPWD_UNENCRYPTED
/* BEGIN: Added by chenzhigang, 2008/3/24   问题单号:将SIP密码加密*/
		memcpy(str, ptr->szPsw, strlen(ptr->szPsw));
		str[strlen(ptr->szPsw)] = '\0';

		base64Encode((unsigned char *)str, strlen(str), &base64);
/* END:   Added by chenzhigang, 2008/3/24 */
		#endif
		/*end by w00135351 09.3.14 去掉加密*/
		sprintf(pBuf,"<%s id=\"%d\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%d\" %s=\"%d\" %s=\"%d\" \
%s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" \
%s=\"%d\" %s=\"%d\" %s=\"%s\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" \
%s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" \
%s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" ",
	        ENTRYNAME_SIP_BSC,               entry+1,
	        PSX_ATTR_VOICE_SIP_USER_ID,      ptr->szId,
	        PSX_ATTR_VOICE_SIP_USER_NAME,    ptr->szName,
	        /*add by w00135351 09.3.14 去掉加密*/
			#ifndef SUPPORT_SIPBSCPWD_UNENCRYPTED
	        /* BEGIN: Added by chenzhigang, 2008/3/24   问题单号:将SIP密码加密*/
	        PSX_ATTR_VOICE_SIP_USER_PASSWD,  base64,
	        /* END:   Added by chenzhigang, 2008/3/24 */
			#else
			PSX_ATTR_VOICE_SIP_USER_PASSWD,  ptr->szPsw,
			#endif
			/*end by w00135351 09.3.14 去掉加密*/
	        PSX_ATTR_VOICE_SIP_LOCAL_PORT,   ptr->uwPort,
	        PSX_ATTR_VOICE_SIP_FWDTBL_ID,    ptr->ulFwdTbl,
	        PSX_ATTR_VOICE_SIP_CLIP_ENABLE,  ptr->usCliStatus,
			PSX_ATTR_VOICE_SIP_LINE_ENABLE,  ptr->usLineEnable,
            /*start of addition by chenyong 2009-03-19*/
			PSX_ATTR_VOICE_SIP_LINE_CALLHOLD,ptr->usLineCallHoldEnable,
            /*end of addition by chenyong 2009-03-19*/			
			PSX_ATTR_VOICE_SIP_LINE_CALLWAIT,ptr->usLineCWEnable,
/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
			PSX_ATTR_VOICE_SIP_LINE_3WAY,         ptr->usLine3WayEnable,
			PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER, ptr->usLineCallTransferEnable,
/*END: Added by chenyong 65116 2008-09-15 web配置*/
/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
            PSX_ATTR_VOICE_SIP_LINE_MWI, ptr->usLineMWIEnable, 
            PSX_ATTR_VOICE_SIP_LINE_MWIACCOUNT, ptr->szLineMWIAccount, 
/* END:   Added by chenzhigang, 2008/11/4 */
			PSX_ATTR_VOICE_SIP_FAX_DETECT,   ptr->usLineFaxDetect,
		    PSX_ATTR_VOICE_SIP_VAD, ptr->ulVad,
		    PSX_ATTR_VOICE_SIP_EC, ptr->ulEC,
		    PSX_ATTR_VOICE_SIP_CNG, ptr->ulCNG,
			/*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
            //modified by z67625
		    PSX_ATTR_VOICE_SIP_VOICE_CODEC1,  (ptr->stVoiceCfg.stCodecList[0].ulCodecCap == 104)? 2:ptr->stVoiceCfg.stCodecList[0].ulCodecCap,
		    PSX_ATTR_VOICE_SIP_VOICE_CODEC2,  (ptr->stVoiceCfg.stCodecList[1].ulCodecCap == 104)? 2:ptr->stVoiceCfg.stCodecList[1].ulCodecCap,
		    PSX_ATTR_VOICE_SIP_VOICE_CODEC3,  (ptr->stVoiceCfg.stCodecList[2].ulCodecCap == 104)? 2:ptr->stVoiceCfg.stCodecList[2].ulCodecCap,
		    PSX_ATTR_VOICE_SIP_VOICE_CODEC4,  (ptr->stVoiceCfg.stCodecList[3].ulCodecCap == 104)? 2:ptr->stVoiceCfg.stCodecList[3].ulCodecCap,
		    PSX_ATTR_VOICE_SIP_VOICE_CODEC5,  (ptr->stVoiceCfg.stCodecList[4].ulCodecCap == 104)? 2:ptr->stVoiceCfg.stCodecList[4].ulCodecCap,

			PSX_ATTR_VOICE_SIP_VOICE_PT1,     ptr->stVoiceCfg.stCodecList[0].ulPTime,
			PSX_ATTR_VOICE_SIP_VOICE_PT2,     ptr->stVoiceCfg.stCodecList[1].ulPTime,
			PSX_ATTR_VOICE_SIP_VOICE_PT3,     ptr->stVoiceCfg.stCodecList[2].ulPTime,
			PSX_ATTR_VOICE_SIP_VOICE_PT4,     ptr->stVoiceCfg.stCodecList[3].ulPTime,
			PSX_ATTR_VOICE_SIP_VOICE_PT5,     ptr->stVoiceCfg.stCodecList[4].ulPTime,

			PSX_ATTR_VOICE_SIP_VOICE_PRIORITY1,  ptr->stVoiceCfg.stCodecList[0].ulPriority,
			PSX_ATTR_VOICE_SIP_VOICE_PRIORITY2,  ptr->stVoiceCfg.stCodecList[1].ulPriority,
			PSX_ATTR_VOICE_SIP_VOICE_PRIORITY3,  ptr->stVoiceCfg.stCodecList[2].ulPriority,
			PSX_ATTR_VOICE_SIP_VOICE_PRIORITY4,  ptr->stVoiceCfg.stCodecList[3].ulPriority,
			PSX_ATTR_VOICE_SIP_VOICE_PRIORITY5,  ptr->stVoiceCfg.stCodecList[4].ulPriority,

			PSX_ATTR_VOICE_SIP_VOICE_IFENABLE1,  ptr->stVoiceCfg.stCodecList[0].bEnable,
			PSX_ATTR_VOICE_SIP_VOICE_IFENABLE2,  ptr->stVoiceCfg.stCodecList[1].bEnable,
			PSX_ATTR_VOICE_SIP_VOICE_IFENABLE3,  ptr->stVoiceCfg.stCodecList[2].bEnable,
			PSX_ATTR_VOICE_SIP_VOICE_IFENABLE4,  ptr->stVoiceCfg.stCodecList[3].bEnable,
			PSX_ATTR_VOICE_SIP_VOICE_IFENABLE5,  ptr->stVoiceCfg.stCodecList[4].bEnable,
			
			/*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
            

            PSX_ATTR_VOICE_SIP_VOICE_DtmfType,ptr->stVoiceCfg.ulDtmfType,
	//BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
            PSX_ATTR_VOICE_SIP_VOICE_PayloadType,ptr->stVoiceCfg.ulPayloadType,
	//END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
            PSX_ATTR_VOICE_SIP_FAX_TYPE,      ptr->ulFaxType
          /* END:   Modified by p00102297, 2008/2/20 */
        	);
/* BEGIN: Added by chenzhigang, 2008/3/24   问题单号:将SIP密码加密*/
		free(base64);           
/* END:   Added by chenzhigang, 2008/3/24 */
        pBuf += strlen(pBuf);
        for (i=0; i<SIP_MAX_DAILNUM_CHANGE_NUM;i++)
        {
	        sprintf(pBuf,"%s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" \
				%s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" ",
        		PSX_ATTR_VOICE_SIP_STRP_PRENUM, i,ptr->stStripPrefixNum[i].szAsideNum,
        		PSX_ATTR_VOICE_SIP_STRP_PRENAME,i,ptr->stStripPrefixNum[i].szMatchNum,
        		PSX_ATTR_VOICE_SIP_APD_PRENUM,  i,ptr->stAppandPrefixNum[i].szAsideNum,
        		PSX_ATTR_VOICE_SIP_APD_PRENAME, i,ptr->stAppandPrefixNum[i].szMatchNum,
        		PSX_ATTR_VOICE_SIP_STRP_SUFNUM, i,ptr->stStripSuffixNum[i].szAsideNum,
        		PSX_ATTR_VOICE_SIP_STRP_SUFNAME,i,ptr->stStripSuffixNum[i].szMatchNum,
        		PSX_ATTR_VOICE_SIP_APD_SUFNUM,  i,ptr->stAppandSuffixNum[i].szAsideNum,
        		PSX_ATTR_VOICE_SIP_APD_SUFNAME, i,ptr->stAppandSuffixNum[i].szMatchNum
        		);
        #if 0
        	sprintf(pBuf,"%s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" %s%02d=\"%s\" ",
        		PSX_ATTR_VOICE_SIP_STRP_NUM, i,ptr->stStripNum[i].szPrefixNum,
        		PSX_ATTR_VOICE_SIP_STRP_NAME,i,ptr->stStripNum[i].szMatchNum,
        		PSX_ATTR_VOICE_SIP_APD_NUM,  i,ptr->stAppNum[i].szPrefixNum,
        		PSX_ATTR_VOICE_SIP_APD_NAME, i,ptr->stAppNum[i].szMatchNum
        		);
		#endif
        	pBuf += strlen(pBuf);
        }
        wbuf[strlen(wbuf)-1] = '\0';
        fprintf(file,"%s/>\n",wbuf);
        free(wbuf);
        wbuf = NULL;
#else
       fprintf(file,"<%s id=\"%d\" %s=\"%s\" %s=\"%s\" %s=\"%s\" %s=\"%d\" %s=\"%d\"/>\n",
        ENTRYNAME_SIP_BSC,              entry+1,
        PSX_ATTR_VOICE_SIP_USER_ID,     ptr->szId,
        PSX_ATTR_VOICE_SIP_USER_NAME,   ptr->szName,
        PSX_ATTR_VOICE_SIP_USER_PASSWD, ptr->szPsw,
        PSX_ATTR_VOICE_SIP_LOCAL_PORT,  ptr->uwPort,
        PSX_ATTR_VOICE_SIP_FWDTBL_ID,   ptr->ulFwdTbl
        );
#endif  
     } /* for entry */
     if ( num != 0 )
       fprintf(file,"</%s>\n",voiceObjs[id-1].objName);
   } /* table */
}
PSX_STATUS xmlGetSipBscAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetSipBscAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SIP_USER_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SIP_WEB_USER_INFO_S) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SIP_WEB_USER_INFO_S) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndSipBscObjNode(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndSipBscObjNode(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSipBscObjNode(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
      BcmDb_setSipBscSize(xmlState.tableSize);
      BcmDb_setSipBscInfo((PSIP_WEB_USER_INFO_S)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Access Control=========\n");
  printf("xmlEndSipBscObjNode(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

// SIP Basic Entry
PSX_STATUS xmlGetSipBscEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSIP_WEB_USER_INFO_S    pstSipBscTbl = (PSIP_WEB_USER_INFO_S)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetSipBscEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,PSX_ATTR_VOICE_SIP_USER_ID) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
    if (status == DB_OBJ_VALID_OK){
      strncpy(pstSipBscTbl[i].szId, attrValue, SIP_MAX_NUM_LEN - 1);
      pstSipBscTbl[i].szId[SIP_MAX_NUM_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid SIP User ID Address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_USER_NAME) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_NAME_LEN);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].szName, attrValue, SIP_MAX_NAME_LEN - 1);
      pstSipBscTbl[i].szName[SIP_MAX_NAME_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid SIP User Name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_USER_PASSWD) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_PSW_LEN*2);
    if (status == DB_OBJ_VALID_OK)
    {
    /*add by w00135351 09.3.14 去掉加密*/
    #ifndef SUPPORT_SIPBSCPWD_UNENCRYPTED
    /* BEGIN: Added by chenzhigang, 2008/3/24   问题单号:将SIP密码加密*/
	   char dest_string[SIP_MAX_PSW_LEN];
	  memset(dest_string, 0, sizeof(dest_string));
	  base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
      strncpy(pstSipBscTbl[i].szPsw, dest_string, SIP_MAX_PSW_LEN - 1);
	  /* END:   Added by chenzhigang, 2008/3/24 */
	#else
	  strncpy(pstSipBscTbl[i].szPsw,attrValue,SIP_MAX_PSW_LEN - 1);
	#endif
	/*end by w00135351 09.3.14 去掉加密*/
      pstSipBscTbl[i].szPsw[SIP_MAX_PSW_LEN - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid SIP Password %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_LOCAL_PORT) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      pstSipBscTbl[i].uwPort = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid SIP Local Port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_VOICE_SIP_FWDTBL_ID) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].ulFwdTbl = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid SIP Table ID %s\n",attrValue);
    }
  }
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_CLIP_ENABLE) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usCliStatus = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid CLIP option %s\n",attrValue);
    }
  }
  /* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_ENABLE) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLineEnable = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Line_Enable option %s\n",attrValue);
    }
  }
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLWAIT) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLineCWEnable = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid CallWaiting option %s\n",attrValue);
    }
  }
/*start of addition by chenyong 2009-03-19*/
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLHOLD) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLineCallHoldEnable = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid CallHold option %s\n",attrValue);
    }
  }
/*end of addition by chenyong 2009-03-19*/
/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_3WAY) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLine3WayEnable = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid CallWaiting option %s\n",attrValue);
    }
  }
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLineCallTransferEnable = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid CallWaiting option %s\n",attrValue);
    }
  }
/*END: Added by chenyong 65116 2008-09-15 web配置*/
/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_MWI) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
  	status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
  	pstSipBscTbl[i].usLineMWIEnable = atoi(attrValue);
    } else {
  	xmlState.errorFlag = PSX_STS_ERR_FATAL;
  	syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid MWI option %s\n",attrValue);
    }
  }
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_MWIACCOUNT) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
    if (status == DB_OBJ_VALID_OK) {
  	strncpy(pstSipBscTbl[i].szLineMWIAccount, attrValue, SIP_MAX_NUM_LEN - 1);
    pstSipBscTbl[i].szLineMWIAccount[SIP_MAX_NUM_LEN - 1] = '\0';
    } else {
  	xmlState.errorFlag = PSX_STS_ERR_FATAL;
  	syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid MWI account option %s\n",attrValue);
    }
  }
/* END:   Added by chenzhigang, 2008/11/4 */
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_FAX_DETECT) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].usLineFaxDetect = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid FaxDetect option %s\n",attrValue);
    }
  }
  else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_VAD) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      pstSipBscTbl[i].ulVad = atoi(attrValue);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid VAD option %s\n",attrValue);
    }
  } 
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_EC) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_EC,attrValue);
    }
    else {
      pstSipBscTbl[i].ulEC = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_CNG) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_CNG,attrValue);
    }
    else {
      pstSipBscTbl[i].ulCNG = atoi(attrValue);
    }
  }
  /*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_CODEC1) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_CODEC1,attrValue);
    }
    else {
        //modified by z67625
        if(atoi(attrValue) == 2)
        {
            printf("===%d=attrvalue is 2 set to 104\n", __LINE__);
            pstSipBscTbl[i].stVoiceCfg.stCodecList[0].ulCodecCap = 104;
        }
        else
        {
            pstSipBscTbl[i].stVoiceCfg.stCodecList[0].ulCodecCap = atoi(attrValue);
        }
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_CODEC2) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_CODEC2,attrValue);
    }
    else {
         //modified by z67625
        if(atoi(attrValue) == 2)
        {
            printf("===%d=attrvalue is 2 set to 104\n", __LINE__);
            pstSipBscTbl[i].stVoiceCfg.stCodecList[1].ulCodecCap = 104;
        }
        else
        {
            pstSipBscTbl[i].stVoiceCfg.stCodecList[1].ulCodecCap = atoi(attrValue);
        }
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_CODEC3) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_CODEC3,attrValue);
    }
    else {
         //modified by z67625
        if(atoi(attrValue) == 2)
        {
            printf("===%d=attrvalue is 2 set to 104\n", __LINE__);
            pstSipBscTbl[i].stVoiceCfg.stCodecList[2].ulCodecCap = 104;
        }
        else
        {
            pstSipBscTbl[i].stVoiceCfg.stCodecList[2].ulCodecCap = atoi(attrValue);
        }
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_CODEC4) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_CODEC4,attrValue);
    }
    else {
         //modified by z67625
        if(atoi(attrValue) == 2)
        {
            printf("===%d=attrvalue is 2 set to 104\n", __LINE__);
            pstSipBscTbl[i].stVoiceCfg.stCodecList[3].ulCodecCap = 104;
        }
        else
        {
            pstSipBscTbl[i].stVoiceCfg.stCodecList[3].ulCodecCap = atoi(attrValue);
        }
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_CODEC5) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_CODEC5,attrValue);
    }
    else {
         //modified by z67625
        if(atoi(attrValue) == 2)
        {
            printf("===%d=attrvalue is 2 set to 104\n", __LINE__);
            pstSipBscTbl[i].stVoiceCfg.stCodecList[4].ulCodecCap = 104;
        }
        else
        {
            pstSipBscTbl[i].stVoiceCfg.stCodecList[4].ulCodecCap = atoi(attrValue);
        }
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PT1) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PT1,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[0].ulPTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PT2) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PT2,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[1].ulPTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PT3) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PT3,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[2].ulPTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PT4) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PT4,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[3].ulPTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PT5) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PT5,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[4].ulPTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PRIORITY1) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PRIORITY1,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[0].ulPriority = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PRIORITY2) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PRIORITY2,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[1].ulPriority = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PRIORITY3) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PRIORITY3,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[2].ulPriority = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PRIORITY4) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PRIORITY4,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[3].ulPriority = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PRIORITY5) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PRIORITY5,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[4].ulPriority = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_IFENABLE1) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_IFENABLE1,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[0].bEnable = (1==atoi(attrValue)?VOS_TRUE:VOS_FALSE);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_IFENABLE2) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_IFENABLE2,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[1].bEnable = (1==atoi(attrValue)?VOS_TRUE:VOS_FALSE);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_IFENABLE3) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_IFENABLE3,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[2].bEnable = (1==atoi(attrValue)?VOS_TRUE:VOS_FALSE);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_IFENABLE4) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_IFENABLE4,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[3].bEnable = (1==atoi(attrValue)?VOS_TRUE:VOS_FALSE);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_IFENABLE5) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_IFENABLE5,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.stCodecList[4].bEnable = (1==atoi(attrValue)?VOS_TRUE:VOS_FALSE);
    }
  }

  /*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_DtmfType) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_DtmfType,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.ulDtmfType = atoi(attrValue);
    }
  }
  //BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_VOICE_PayloadType) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_VOICE_PayloadType,attrValue);
    }
    else {
      pstSipBscTbl[i].stVoiceCfg.ulPayloadType = atoi(attrValue);
    }
  }
  //END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_FAX_TYPE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_FAX_TYPE,attrValue);
    }
    else {
      pstSipBscTbl[i].ulFaxType = atoi(attrValue);
    }
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_STRP_PRENUM)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_STRP_PRENUM);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_ASIDENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripPrefixNum[iIndex].szAsideNum, attrValue, SIP_MAX_ASIDENUM_SIZE - 1);
      pstSipBscTbl[i].stStripPrefixNum[iIndex].szAsideNum[SIP_MAX_ASIDENUM_SIZE - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Strip Number %s\n",attrValue);
    }
#if 0
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_PRENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripNum[iIndex].szPrefixNum, attrValue, SIP_PRENUM_SIZE - 1);
      pstSipBscTbl[i].stStripNum[iIndex].szPrefixNum[SIP_PRENUM_SIZE - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Strip Number %s\n",attrValue);
    }
#endif
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_STRP_PRENAME)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_STRP_PRENAME);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripPrefixNum[iIndex].szMatchNum, attrValue, SIP_MAX_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stStripPrefixNum[iIndex].szMatchNum[SIP_MAX_MATCHNUM_SIZE - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Incoming Number %s\n",attrValue);
    }
#if 0
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripNum[iIndex].szMatchNum, attrValue, SIP_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stStripNum[iIndex].szMatchNum[SIP_MATCHNUM_SIZE - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Incoming Number %s\n",attrValue);
    }
#endif
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_APD_PRENUM)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_APD_PRENUM);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_ASIDENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppandPrefixNum[iIndex].szAsideNum, attrValue, SIP_MAX_ASIDENUM_SIZE - 1);
      pstSipBscTbl[i].stAppandPrefixNum[iIndex].szAsideNum[SIP_MAX_ASIDENUM_SIZE - 1] = '\0';

    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Append Number %s\n",attrValue);
    }
#if 0
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_PRENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppNum[iIndex].szPrefixNum, attrValue, SIP_PRENUM_SIZE - 1);
      pstSipBscTbl[i].stAppNum[iIndex].szPrefixNum[SIP_PRENUM_SIZE - 1] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Append Number %s\n",attrValue);
    }
#endif
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_APD_PRENAME)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_APD_PRENAME);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppandPrefixNum[iIndex].szMatchNum, attrValue, SIP_MAX_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stAppandPrefixNum[iIndex].szMatchNum[SIP_MAX_MATCHNUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
#if 0
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppNum[iIndex].szMatchNum, attrValue, SIP_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stAppNum[iIndex].szMatchNum[SIP_MATCHNUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
#endif
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_STRP_SUFNUM)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_STRP_SUFNUM);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_ASIDENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripSuffixNum[iIndex].szAsideNum, attrValue, SIP_MAX_ASIDENUM_SIZE - 1);
      pstSipBscTbl[i].stStripSuffixNum[iIndex].szAsideNum[SIP_MAX_ASIDENUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_STRP_SUFNAME)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_STRP_SUFNAME);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stStripSuffixNum[iIndex].szMatchNum, attrValue, SIP_MAX_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stStripSuffixNum[iIndex].szMatchNum[SIP_MAX_MATCHNUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_APD_SUFNUM)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_APD_SUFNUM);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_ASIDENUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppandSuffixNum[iIndex].szAsideNum, attrValue, SIP_MAX_ASIDENUM_SIZE - 1);
      pstSipBscTbl[i].stAppandSuffixNum[iIndex].szAsideNum[SIP_MAX_ASIDENUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
  }
  else if (strstr(attrName,PSX_ATTR_VOICE_SIP_APD_SUFNAME)!=NULL)
  {
  	char *puc = attrName;
  	int iIndex = 0;
  	puc += strlen(PSX_ATTR_VOICE_SIP_APD_SUFNAME);
  	iIndex = atoi(puc);
	if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SIP_MAX_MATCHNUM_SIZE);
    if (status == DB_OBJ_VALID_OK)
    {
      strncpy(pstSipBscTbl[i].stAppandSuffixNum[iIndex].szMatchNum, attrValue, SIP_MAX_MATCHNUM_SIZE - 1);
      pstSipBscTbl[i].stAppandSuffixNum[iIndex].szMatchNum[SIP_MAX_MATCHNUM_SIZE - 1] = '\0';
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipBscEntryAttr(): invalid Dialing Number %s\n",attrValue);
    }
  }
  /* END:   Modified by p00102297, 2008/3/11 VDF C02*/
#endif  
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndSipBscObjEntryNode(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
void  xmlSetSipProvAttr(FILE* file, const char *name, const UINT16 id,
    const PSI_VALUE value, const UINT16 length)
{
    PSIP_WEB_PROV_CFG_S pstSipProv;

    if ((name == NULL) || (value == NULL))
    {
        return;
    }

    pstSipProv = (PSIP_WEB_PROV_CFG_S)value;

    fprintf(file,
        "<%s %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\"/>\n",
        OBJNAME_SIP_PROV,
        PSX_ATTR_VOICE_SIP_CLIP_ENABLE_PROV,       pstSipProv->usCliStatusProv,
        PSX_ATTR_VOICE_SIP_LINE_CALLHOLD_PROV, pstSipProv->usLineCallHoldEnableProv,
        PSX_ATTR_VOICE_SIP_LINE_CALLWAIT_PROV,    pstSipProv->usLineCWEnableProv,
        PSX_ATTR_VOICE_SIP_LINE_3WAY_PROV,    pstSipProv->usLine3WayEnableProv,
        PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER_PROV,    pstSipProv->usLineCallTransferEnableProv,
        PSX_ATTR_VOICE_SIP_LINE_MWI_PROV,  pstSipProv->usLineMWIEnableProv,
        PSX_ATTR_VOICE_SIP_LINE_AUTOCALL_PROV,  pstSipProv->usLineAutoCallEnableProv);
}

PSX_STATUS xmlGetSipProvAttr(char *attrName, char* attrValue)
{
    PSIP_WEB_PROV_CFG_S pstSipProv = (PSIP_WEB_PROV_CFG_S)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    int len = 0;

#ifdef XML_DEBUG
    printf("xmlGetSipProvAttr(attrName %s, attrValue %s)\n",
        attrName,attrValue);
#endif

    if (strcmp(attrName, PSX_ATTR_VOICE_SIP_CLIP_ENABLE_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usCliStatusProv= atoi(attrValue);
        } 
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid CLIP PROV option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLHOLD_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usLineCallHoldEnableProv = atoi(attrValue);
        } 
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid CallHold PROV option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLWAIT_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usLineCWEnableProv = atoi(attrValue);
        } 
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid CallWaiting option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_3WAY_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) {
            pstSipProv->usLine3WayEnableProv = atoi(attrValue);
        } 
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid CallWaiting option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usLineCallTransferEnableProv = atoi(attrValue);
        } 
        else
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid CallWaiting option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_MWI_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usLineMWIEnableProv= atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid MWI option %s\n",attrValue);
        }
    }
    else if (strcmp(attrName, PSX_ATTR_VOICE_SIP_LINE_AUTOCALL_PROV) == 0) 
    {
        if (xmlState.verify && attrValue[0] != '\0')
            status = BcmDb_validateNumber(attrValue);
        if (status == DB_OBJ_VALID_OK) 
        {
            pstSipProv->usLineAutoCallEnableProv= atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSipProvAttr(): invalid AutomaticCall option %s\n",attrValue);
        }
    }

    return(PSX_STS_OK);
}
PSX_STATUS xmlStartSipProvObjNode(char *appName, char *objName)
{
  PSIP_WEB_PROV_CFG_S pstSipProv;

#ifdef XML_DEBUG
  printf("xmlStartSipProvObjNode(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SIP_WEB_PROV_CFG_S));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(SIP_WEB_PROV_CFG_S));
  pstSipProv = (PSIP_WEB_PROV_CFG_S)psiValue;

  pstSipProv->usCliStatusProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLineCallHoldEnableProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLineCWEnableProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLine3WayEnableProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLineCallTransferEnableProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLineMWIEnableProv = DEFAULT_PROV_DISABLE;
  pstSipProv->usLineAutoCallEnableProv = DEFAULT_PROV_DISABLE;

  return PSX_STS_OK;
}
PSX_STATUS xmlEndSipProvObjNode(char *appName, char *objName)
{
  PSIP_WEB_PROV_CFG_S  pstSipProv = (PSIP_WEB_PROV_CFG_S)psiValue;

#ifdef XML_DEBUG

  printf("xmlEndSipProvObjNode(calling cfm to get appName %s, objName %s, verify %d\n",
         appName,objName, xmlState.verify);
#endif

  if (!xmlState.verify)
    BcmDb_setSipProvInfo(pstSipProv);

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
/* END:   Added by y67514, 2009/11/16 */

// SIP Advanced
void  xmlSetSipAdvAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length)
{
  PSIP_WEB_ADV_CFG_S pstSipAdv;

  if ((name == NULL) || (value == NULL))
    return;

  pstSipAdv = (PSIP_WEB_ADV_CFG_S)value;

  fprintf(file,
          "<%s %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\" \
          %s=\"%d\" %s=\"%d\" %s=\"%d\"\ 
          %s=\"%d\" %s=\"%d\" \
          %s=\"%d\" %s=\"%d\" \
          %s=\"%s\" %s=\"%d\" %s=\"%s\" %s=\"%d\" %s=\"%d\" />\n",
          OBJNAME_SIP_ADV,
          /* BEGIN: Added by y67514, 2009/11/4   问题单号:增加语音tr069的配置*/
          PSX_ATTR_VOICE_SIP_SRV_REGPERIOD,    pstSipAdv->stServerCfg.ulRegPeriod,
          /* END:   Added by y67514, 2009/11/4 */
          PSX_ATTR_VOICE_SIP_SRV_EXPIRE,    pstSipAdv->stServerCfg.ulExpiration,
          PSX_ATTR_VOICE_SIP_SRV_REREG,     pstSipAdv->stServerCfg.ulReRegister,
          PSX_ATTR_VOICE_SIP_SRV_SEEXP,     pstSipAdv->stServerCfg.SessionExpires,
          PSX_ATTR_VOICE_SIP_SRV_MINSE,     pstSipAdv->stServerCfg.MinSE,
          /* BEGIN: Modified by p00102297, 2008/1/29 */
          PSX_ATTR_VOICE_SIP_SRV_MAXRETRY,  pstSipAdv->stServerCfg.ulMaxRetryTime,
          PSX_ATTR_VOICE_SIP_SRV_PRICHECK,  pstSipAdv->stServerCfg.ulPrimaryCheckTime,
          /* END:   Modified by p00102297, 2008/1/29 */
          /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
          PSX_ATTR_VOICE_SIP_SRV_MWIEXPIRE, pstSipAdv->stServerCfg.ulMWIExpire,
          /* END:   Added by chenzhigang, 2008/11/5 */
          PSX_ATTR_VOICE_SIP_RTP_STARTPORT, pstSipAdv->stRTPCfg.ulStartPort,
          PSX_ATTR_VOICE_SIP_RTP_ENDPORT,   pstSipAdv->stRTPCfg.ulEndPort,
          /* BEGIN: Modified by p00102297, 2008/1/18 */
		  PSX_ATTR_VOICE_SIP_RTP_TXREINTVAL,pstSipAdv->stRTPCfg.ulTxRepeatInterval,
		  /* END:   Modified by p00102297, 2008/1/18 */
          /*start of modification by chenyong 2009-03-05*/
		  PSX_ATTR_VOICE_SIP_RTCP_MONITOR,pstSipAdv->stRTPCfg.ulRtcpMonitor,
          /*end of modification by chenyong 2009-03-05*/
          PSX_ATTR_SIP_INTERFACE,           pstSipAdv->szIfName,
          PSX_ATTR_SIP_LOCAL_PORT,          pstSipAdv->ulSipLocalPort,
          PSX_ATTR_PSTN_CALL_PREFIX,        pstSipAdv->szPstnPrefix,
          PSX_ATTR_SIP_TEST_ENBL,           pstSipAdv->ulEnblSipTest,
          PSX_ATTR_SIP_DSCP_MARK,           pstSipAdv->ulDscp);    /* Added by s00125931, 2008/9/16   问题单号:vhg556*/
}

PSX_STATUS xmlGetSipAdvAttr(char *attrName, char* attrValue)
{
  PSIP_WEB_ADV_CFG_S pstSipAdv = (PSIP_WEB_ADV_CFG_S)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetSipAdvAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_EXPIRE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulExpiration = atoi(attrValue);
    }
  }
  /* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_REGPERIOD) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulRegPeriod= atoi(attrValue);
    }
  }
  /* END:   Added by y67514, 2009/11/4 */
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_REREG) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_REREG,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulReRegister = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_SEEXP) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_SEEXP,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.SessionExpires = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_MINSE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_MINSE,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.MinSE = atoi(attrValue);
    }
  }
  /* BEGIN: Modified by p00102297, 2008/1/29 */
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_MAXRETRY) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_MAXRETRY,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulMaxRetryTime = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_PRICHECK) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_PRICHECK,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulPrimaryCheckTime = atoi(attrValue);
    }
  }
  /* END:   Modified by p00102297, 2008/1/29 */
  /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_SRV_MWIEXPIRE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_SRV_MWIEXPIRE,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stServerCfg.ulMWIExpire = atoi(attrValue);
    }
  }  
  /* END:   Added by chenzhigang, 2008/11/5 */
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_RTP_STARTPORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_RTP_STARTPORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stRTPCfg.ulStartPort = atoi(attrValue);
    }
  }
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_RTP_ENDPORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_RTP_ENDPORT,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stRTPCfg.ulEndPort = atoi(attrValue);
    }
  }
  /* BEGIN: Modified by p00102297, 2008/1/18 */
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_RTP_TXREINTVAL) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_RTP_TXREINTVAL,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stRTPCfg.ulTxRepeatInterval = atoi(attrValue);
    }
  }
  /*start of modification by chenyong 2009-03-05*/
  else if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_RTCP_MONITOR) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_RTCP_MONITOR,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->stRTPCfg.ulRtcpMonitor = atoi(attrValue);
    }
  }
  /*end of modification by chenyong 2009-03-05*/
  else if (strcasecmp(attrName,PSX_ATTR_SIP_INTERFACE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue, SIP_MAX_IFNAME_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(pstSipAdv->szIfName, attrValue, SIP_MAX_IFNAME_STR_LEN - 1);
      pstSipAdv->szIfName[SIP_MAX_IFNAME_STR_LEN - 1] = '\0';
    }
  }

  else if (strcasecmp(attrName,PSX_ATTR_SIP_LOCAL_PORT) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->ulSipLocalPort = atoi(attrValue);
    }
  }

  else if (strcasecmp(attrName,PSX_ATTR_PSTN_CALL_PREFIX) == 0) {
    if (xmlState.verify) 
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
      status = BcmDb_validateLength(attrValue, PSTN_MAX_PREFIX_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      memcpy(pstSipAdv->szPstnPrefix,attrValue,strlen(attrValue));
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
      pstSipAdv->szPstnPrefix[PSTN_MAX_PREFIX_LEN-1]='\0';
    }
  }  
/* BEGIN: Added by s00125931, 2008/9/16   问题单号:vhg556*/
  else if (strcasecmp(attrName,PSX_ATTR_SIP_DSCP_MARK) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->ulDscp=atoi(attrValue);
    }
  }
/* END:   Added by s00125931, 2008/9/16 */
  else if (strcasecmp(attrName,PSX_ATTR_SIP_TEST_ENBL) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipAdvAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipAdv->ulEnblSipTest = atoi(attrValue);
    }
  }

  return(PSX_STS_OK);
}
PSX_STATUS xmlStartSipAdvObjNode(char *appName, char *objName)
{
  PSIP_WEB_ADV_CFG_S pstSipAdv;

#ifdef XML_DEBUG
  printf("xmlStartSipAdvObjNode(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SIP_WEB_ADV_CFG_S));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(SIP_WEB_ADV_CFG_S));
  pstSipAdv = (PSIP_WEB_ADV_CFG_S)psiValue;

/* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
  pstSipAdv->stServerCfg.ulRegPeriod = (((DEFAULT_EXPIRATION_DURATION-15) > 0)?(DEFAULT_EXPIRATION_DURATION-15):(DEFAULT_EXPIRATION_DURATION/2));
/* END:   Added by y67514, 2009/11/4 */
  pstSipAdv->stServerCfg.ulExpiration   = DEFAULT_EXPIRATION_DURATION;
  pstSipAdv->stServerCfg.ulReRegister   = DEFAULT_REREGISTER_TIMER;
  pstSipAdv->stServerCfg.SessionExpires = DEFAULT_SESSION_EXPIRES;
  pstSipAdv->stServerCfg.MinSE          = DEFAULT_MIN_SE;
  /* BEGIN: Modified by p00102297, 2008/1/29 */
  pstSipAdv->stServerCfg.ulMaxRetryTime  = DEFAULT_MAXRETRY_TIME;
  pstSipAdv->stServerCfg.ulPrimaryCheckTime = DEFAULT_PRIMARY_CHECK_TIME;
  /* END:   Modified by p00102297, 2008/1/29 */
  /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
  pstSipAdv->stServerCfg.ulMWIExpire  = DEFAULT_MWI_EXPIRE; 
  /* END:   Added by chenzhigang, 2008/11/5 */
  pstSipAdv->stRTPCfg.ulStartPort       = DEFAULT_RTP_START_PORT;
  pstSipAdv->stRTPCfg.ulStartPort       = DEFAULT_RTP_END_PORT;
  /* BEGIN: Modified by p00102297, 2008/1/18 */
  pstSipAdv->stRTPCfg.ulTxRepeatInterval = DEFAULT_VOICE_RTCPREPEAT_INTERVAL;
  /*start of modification by chenyong 2009-03-05*/
  pstSipAdv->stRTPCfg.ulRtcpMonitor      = DEFAULT_RTCP_MONITOR;
  /*end of modification by chenyong 2009-03-05*/

  strncpy(pstSipAdv->szIfName, DEFAULT_SIP_INTERFACE_STRING, SIP_MAX_IFNAME_STR_LEN - 1);
  pstSipAdv->szIfName[SIP_MAX_IFNAME_STR_LEN - 1] = '\0';
  pstSipAdv->ulSipLocalPort             = DEFAULT_SIP_LOCAL_PORT;
  pstSipAdv->szPstnPrefix[0]            = '\0';
  pstSipAdv->ulEnblSipTest              = DEFAULT_SIP_TEST_ENBL_FLAG;
  pstSipAdv->ulDscp                     = DEFAULT_SIP_DSCP_MARK;     /* Added by s00125931, 2008/9/16   问题单号:vhg556*/
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSipAdvObjNode(char *appName, char *objName)
{
  PSIP_WEB_ADV_CFG_S  pstSipAdv = (PSIP_WEB_ADV_CFG_S)psiValue;
//  DB_OBJ_VALID_STATUS status    = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlEndSipAdvObj(calling cfm to get appName %s, objName %s, verify %d\n",
         appName,objName, xmlState.verify);
#endif

  if (!xmlState.verify)
    BcmDb_setSipAdvInfo(pstSipAdv);

#ifdef XML_DEBUG1
  printf("\n============End SIP Advanced info=========\n");
  printf("xmlEndSipAdvObj(): sipAdvExpiration  %d, sipAdvReRegiter %d, sipAdvSeExpires %d sipAdvMinSe %d \
		 sipAdvMaxRetryTime %d sipAdvPrimaryCheckTime %d \
	  	 sipAdvRtpStartPort %d sipAdvRtpEndPort %d \
	     sipAdvRtpTxReptatInterval %d sipRtcpMonitor %d \
	  	 interface %s sipLocalPort %d ulEnblSipTest %d ulDscp %d\n",
         pstSipAdv->stServerCfg.ulExpiration, pstSipAdv->stServerCfg.ulReRegister, 
         pstSipAdv->stServerCfg.SessionExpires, pstSipAdv->stServerCfg.MinSE,
         /* BEGIN: Modified by p00102297, 2008/1/29 */
		 pstSipAdv->stServerCfg.ulMaxRetryTime, pstSipAdv->stServerCfg.ulPrimaryCheckTime,
	     /* END:   Modified by p00102297, 2008/1/29 */
         /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
         pstSipAdv->stServerCfg.ulMWIExpire,
         /* END:   Added by chenzhigang, 2008/11/5 */
         pstSipAdv->stRTPCfg.ulStartPort, pstSipAdv->stRTPCfg.ulEndPort,
         /* BEGIN: Modified by p00102297, 2008/1/18 */
		 pstSipAdv->stRTPCfg.ulTxRepeatInterval,
		 /* END:   Modified by p00102297, 2008/1/18 */
         /*start of modification by chenyong 2009-03-05*/
		 pstSipAdv->stRTPCfg.ulRtcpMonitor,
         /*end of modification by chenyong 2009-03-05*/
         pstSipAdv->szIfName,
         pstSipAdv->ulSipLocalPort,
         pstSipAdv->ulEnblSipTest,
		 /* BEGIN:   Modified by s00125931, 2008/9/12 */
         pstSipAdv->ulDscp);
		 /* END:   Modified by s00125931, 2008/9/12 */  
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

// SIP QoS
void  xmlSetSipQosAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length)
{
  PSIP_WEB_QOS_CFG_S pstSipQos;

  if ((name == NULL) || (value == NULL))
    return;

  pstSipQos = (PSIP_WEB_QOS_CFG_S)value;

  fprintf(file,
          "<%s %s=\"%d\" %s=\"%d\" %s=\"%d\" %s=\"%d\"/>\n",
          OBJNAME_SIP_QOS,
          PSX_ATTR_VOICE_SIP_QOS_TOS,       pstSipQos->ulTos,
          PSX_ATTR_VOICE_SIP_QOS_VLAN_ENBL, pstSipQos->ulEnblVlan,
          PSX_ATTR_VOICE_SIP_QOS_VLANID,    pstSipQos->ulVlan,
          PSX_ATTR_VOICE_SIP_QOS_PRIORITY,  pstSipQos->ulPriority);
}

PSX_STATUS xmlGetSipQosAttr(char *attrName, char* attrValue)
{
  PSIP_WEB_QOS_CFG_S pstSipQos = (PSIP_WEB_QOS_CFG_S)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetSipQosAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_QOS_TOS) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipQosAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipQos->ulTos = atoi(attrValue);
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_QOS_VLAN_ENBL) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipQosAttr(): invalid state %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipQos->ulEnblVlan = atoi(attrValue);
    }
  }
  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_QOS_VLANID) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipQosAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_QOS_VLANID,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipQos->ulVlan = atoi(attrValue);
    }
  }

  if (strcasecmp(attrName,PSX_ATTR_VOICE_SIP_QOS_PRIORITY) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 0, 3);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSipQosAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_QOS_PRIORITY,attrValue);
    }
    else {
      len = strlen(attrValue);
      pstSipQos->ulPriority = atoi(attrValue);
    }
  }

  return(PSX_STS_OK);
}
PSX_STATUS xmlStartSipQosObjNode(char *appName, char *objName)
{
  PSIP_WEB_QOS_CFG_S pstSipQos;

#ifdef XML_DEBUG
  printf("xmlStartSipQosObjNode(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SIP_WEB_QOS_CFG_S));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(SIP_WEB_QOS_CFG_S));
  pstSipQos = (PSIP_WEB_QOS_CFG_S)psiValue;

  pstSipQos->ulTos      = DEFAULT_QOS_TOS;
  pstSipQos->ulEnblVlan = DEFAULT_QOS_VLAN_ENBL;
  pstSipQos->ulVlan     = DEFAULT_QOS_VLANID;
  pstSipQos->ulPriority = DEFAULT_QOS_PRIORITY;

  return PSX_STS_OK;
}
PSX_STATUS xmlEndSipQosObjNode(char *appName, char *objName)
{
  PSIP_WEB_QOS_CFG_S  pstSipQos = (PSIP_WEB_QOS_CFG_S)psiValue;
//  DB_OBJ_VALID_STATUS status    = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG

  printf("xmlEndSipQosObj(calling cfm to get appName %s, objName %s, verify %d\n",
         appName,objName, xmlState.verify);
#endif

  if (!xmlState.verify)
    BcmDb_setSipQosInfo(pstSipQos);

#ifdef XML_DEBUG1
  printf("\n============End SIP Qos info=========\n");
  printf("xmlEndSipQosObj(): sipQosTos %d, sipQosEnblVlan %d, sipQosVlanId %d sipQosPriority %d\n",
         pstSipQos->ulTos,pstSipQos->ulEnblVlan ,pstSipQos->ulVlan, pstSipQos->ulPriority);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
/* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
#endif /* VOXXXLOAD */

/* Caller to this function:
   1. had read XML file content and written content to char* xmlBuffer.
   2. a. call xmlParse to do verification of XML content OR 
      b. to do conversion of XML content to PSI database.
   3. a. after calling for (2a), caller flashes text file to flash, and reboot modem.
      b. caller runs with new PSI as normal.

   Input to xmlParse: xmlBuffer containing XML data.
                      xmlSize is size of xmlBuffer.
                      verify = 1, this is operation (2a); otherwise, this is (2b).
   Output: SUCCESS or FAILURE of parsing status.
*/
PSX_STATUS xmlParse(char *xmlBuffer, int xmlSize, int verify) 
{   
  nxml_t handle = NULL;
  nxml_settings settings;
  int index = 0;
  static char *error[] = {"none", "general_err", "memory_err", "appfind_err", "objfind_err", "hdlFuncfind_err", "fatal_err", "mild_err", "unknown" };

#ifdef XML_DEBUG
  int i;
  printf("xmlParse(profileSize %d, calling BcmPsi_unInit\n)",xmlSize);
#endif

  /* save beginning of buffer position */
  configBeginPtr = (const char *)xmlBuffer;

  settings.tag_begin = xmlTagBegin;
  settings.attribute_begin = xmlAttrBegin;
  settings.attribute_value = xmlAttrValue;
  settings.data = xmlData;
  settings.tag_end = xmlTagEnd;

#ifdef XML_DEBUG
  printf("calling nxml_open\n");
#endif

  /* open parser */
  nxml_open(&handle,&settings);
  if (handle == NULL) {
#ifdef XML_DEBUG
    printf("xmlParse(): error register nxml parser.\n");
#endif
    return PSX_STS_ERR_GENERAL;
  }

#ifdef XML_DEBUG
  printf("xmlParse(): nxml_open ok\n");
  printf("xmlBuffer:\n");
  for (i=0; i<xmlSize; i++) {
    printf("%c",xmlBuffer[i]);
  }
#endif

  memset(&xmlState,0,sizeof(xmlState));
  xmlState.verify = verify;
  xmlState.state = state_start;

#ifdef XML_DEBUG
  printf("xmlParse(): calling nxml_write\n");
#endif

  /* nxml_write returns 0, that means it's done or there's something wrong */
  if ((nxml_write(handle, xmlBuffer, xmlSize) == 0) &&
      (xmlState.state != state_psi_end)) {
      syslog(LOG_ERR,"xmlParse(): fatal error, invalid syntax, config file must end with </psitree>\n");
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
  }
#ifdef XML_DEBUG
  printf("xmlParse(): calling nxml_close\n");
#endif

  /* close parser */
  nxml_close(handle);

  if ((xmlState.errorFlag >= PSX_STS_OK) && (xmlState.errorFlag <= PSX_STS_ERR_MILD))
    index = (int)xmlState.errorFlag;
  else
    index = (int)PSX_STS_ERR_MILD + 1;
  if (xmlState.errorFlag != PSX_STS_OK) 
    printf("Error parsing XML configuration: error %s.\n",error[index]);
#ifdef BRCM_DEBUG
  else 
    printf("XML configuration parsed successfully: error %s.\n",error[xmlState.errorFlag]);
#endif /* BRCM_DEBUG */

  return (xmlState.errorFlag);
}

/* search for tag_name in appList, return type */
int xmlTagType(char *tag_name, unsigned len)
{
  int i, found = 0;
  char name[len+1];
  int type = PSX_OBJ_TYPE;

  for ( i = 0; apps[i].appName[0] != '\0'; i++ ) {
    if (strncasecmp(apps[i].appName, tag_name, strlen(apps[i].appName)) == 0) {
      type = PSX_APP_TYPE;
      found = 1;
      break;
    } /* apps */
  } /* for */
  if (!found) {
    strncpy(name,tag_name,len);
    name[len] = '\0';
    for ( i = 0; tableNames[i][0] != '\0'; i++ ) {
      if (strcmp(tableNames[i], name) == 0) {
	type = PSX_TABLE_TYPE;
	break;
      } /* tables */
    } /* for */
  }
  return (type);
}

/* when xmlTagBegin is called,
   1. tag_name = a) appID name b) objectId name
   2. action needed:
   determine if this is an appId or objectId or nothing
   if (appId)  
      a) check to see if App exists; if no, return and continue
      b) if (verify), return and continue; else do appOpen
*/
void xmlTagBegin(nxml_t handle, const char *tag_name, unsigned len)
{
  int tagType;

#ifdef XML_DEBUG
  printf("xmlTagBegin(tagLen %d, handle->state %d, xmlState.state %d)\n",
	 len,handle->state,xmlState.state);
#endif

  if (xmlState.state == state_start) {
    if (strncmp(tag_name,"psitree",strlen("psitree")) != 0) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlTagBegin(): fatal error, invalid syntax, config file must begin with <psitree>\n");
    }
    else 
      xmlState.state = state_psi_begin;
    return;
  } 
  else {
    if ((strncmp(tag_name,"psitree",strlen("psitree"))) == 0) {
#ifdef XML_DEBUG
      printf("Config file has invalid syntax, psitree encountered one more time.\n");
#endif
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlTagBegin(): fatal error, invalid syntax, psitree encountered again\n");
      /* log error to syslog */
      return;
    }
  } 

  tagType = xmlTagType((char*)tag_name,len);


  if (tagType == PSX_APP_TYPE) {
    //  if (xmlState.state == state_app_begin) {
    /* application tag */
    strncpy(xmlState.appName,tag_name,len);
    xmlState.appName[len] = '\0';    
#ifdef XML_DEBUG
    printf("xmlTagBegin(appType): tag_name %s, verify %d\n",xmlState.appName,
	   xmlState.verify);
#endif
  }
  else if (tagType == PSX_OBJ_TYPE) {
    /* tag of object */
    strncpy(xmlState.objName,tag_name,len);
    xmlState.objName[len] = '\0';    
    xmlStartObjNode(xmlState.appName,xmlState.objName);

#ifdef XML_DEBUG
    printf("xmlTagBegin(objectType): tag_name %s\n",xmlState.objName);
#endif
  } 
  else if (tagType == PSX_TABLE_TYPE) {
    /* table name tag */
    strncpy(xmlState.objName,tag_name,len);
    xmlState.objName[len] = '\0';    
    strncpy(xmlState.tableName,tag_name,len);
    xmlState.tableName[len] = '\0';    
    xmlStartObjNode(xmlState.appName,xmlState.tableName);

#ifdef XML_DEBUG
    printf("xmlTagBegin(tableType): tag_name %s\n",xmlState.tableName);
#endif
  } 
}

/* when xmlTagEnd is called,
   1. signals end of  a) app (handle->state == state_end_tag_name), tagName is name of appId
                      b) object (handle->state is state_attr_name), tagName is invalid, len = 0
   2. action needed:
      if (end of app), move internal state to state_app_begin.
      if (end of object), do 2nd phase verification to make sure the whole object configuration
         is set.   If mandatory field is missing, log error and return fatal right away. Otherwise,
         if (!verify), call CFM's objStore for this object.
*/
void xmlTagEnd(nxml_t handle, const char *tag_name, unsigned len)
{
#ifdef XML_DEBUG
  printf("****************************************xmlTagEnd(): len %d\n",len);
#endif

  if (len != 0) {
    /* end of app or end of table */
#ifdef XML_DEBUG
    printf("xmlTagEnd(): end of app %s, tag_name %s\n",xmlState.appName, tag_name);
#endif

    if (strncmp(tag_name, "psitree",len) == 0) {
      xmlState.state = state_psi_end;
      return;
    }

    if (xmlState.tableName[0] == '\0') {
      /* must be end of app */
      if (strncmp(tag_name, xmlState.appName,strlen(xmlState.appName)) != 0) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"psx: fatal error, end appName %s doesn't match begin appName %s.\n",
	       tag_name,xmlState.appName);

#ifdef XML_DEBUG
      printf("xmlTagEnd(): fatal error, end appName %s doesn't match begin appName %s.\n",
	     tag_name,xmlState.appName);
#endif
      } /* checking app Name */
      else {
        memset(xmlState.appName, 0, sizeof(xmlState.appName));
      }
    } /* app */
    else {
      /* must be end of table */
      if (strncmp(tag_name, xmlState.tableName,strlen(xmlState.tableName)) != 0) {
	xmlState.errorFlag = PSX_STS_ERR_FATAL;
	syslog(LOG_ERR,"psx: fatal error, end tableName %s doesn't match begin tableName %s.\n",
	       tag_name,xmlState.tableName);

#ifdef XML_DEBUG
	printf("xmlTagEnd(): fatal error, end tableName %s doesn't match begin tableName %s.\n",
	       tag_name,xmlState.tableName);
#endif
      } /* checking table Name */
      else {
        xmlEndObjNode(xmlState.appName,xmlState.tableName);
	memset(xmlState.tableName, 0, sizeof(xmlState.tableName));
      }
    } /* table */
  } /* len != 0 */
  else {
    /* end of object;  or empty app */
#ifdef XML_DEBUG
      printf("xmlTagEnd(): end of obj %s, verify %d\n",xmlState.objName,xmlState.verify);
#endif
      /* make sure it's not an empty app before calling obj handlers */
      if (xmlState.objName[0] != '\0') {
        xmlEndObjNode(xmlState.appName,xmlState.objName);
        memset(xmlState.objName, 0, sizeof(xmlState.objName));
      }
  }
}

/* when xmlAttrBegin is called,
   a) object field name is received; save this name, expect value to be return next.
      Action: change internal state to state_attribute_value?
*/
void xmlAttrBegin(nxml_t handle, const char *tag_name, unsigned len)
{
  /* name contains attribute name of objectId i.d. vpi="0", name is vpi */
  strncpy(xmlState.attrName,tag_name,len);
  xmlState.attrName[len] = '\0';
#ifdef XML_DEBUG
  printf("xmlAttrBegin(): attrName %s, len %d\n",xmlState.attrName,len);
#endif
}

/* when xmlAttrValue is called,
   a) object field value is received; do atomic set for objName, attrName.
      Action: change internal state to ?
*/
void xmlAttrValue(nxml_t handle, const char *tag_name, unsigned len)
{
  char attrValue[len+1];

  /* name contains attribute name of objectId i.d. vpi="0", name is vpi */
  strncpy(attrValue,tag_name,len);
  attrValue[len] = '\0';

#ifdef XML_DEBUG
  printf("xmlAttrValue(): attrValue %s, len %d\n",attrValue,len);
#endif
  /* do atomic set of attributes */
  xmlGetObjNode(xmlState.appName,xmlState.objName,xmlState.attrName,attrValue);
  memset(xmlState.attrName, 0, sizeof(xmlState.attrName));
}

/* this is not applicable to us. */
void xmlData(nxml_t handle, const char *tag_name, unsigned len)
{
#ifdef XML_DEBUG
   printf("xmlData(): this function should never be called\n");
#endif
   return;
}

/* This function assumes verification has been passed.   It gets the value from the
   XML text file, and store it into PSI data base for modem operation. */
PSX_STATUS xmlGetObjNode(char *appName, char *objTagName,
			 char *attrName, char *value) {
   int i;
   PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
   printf("xmlGetObjNode(): appName %s, objTagName %s, attrName %s, value %s\n",
	  appName,objTagName,attrName,value);
#endif

   for ( i = 0; apps[i].appName[0] != '\0'; i++ ) {
      /* there are a few special cases where the appId only needs to match with
	 the specified prefix (wan, ipsrv, pppsrv) */
      if (strncasecmp(apps[i].appName, appName, strlen(apps[i].appName)) == 0) {
        sts = PSX_STS_ERR_GENERAL;
	if (*(apps[i].getObjFnc) != NULL) {
	  sts = (*(apps[i].getObjFnc))(appName, objTagName, attrName, value );
	  break;
	}
      } /* found */
   } /* for */

   if (sts == PSX_STS_ERR_FIND_OBJ) {
     syslog(LOG_ERR,"xmlGetObjNode(): unrecognizable appName %s, ignore.\n",appName);
#ifdef XML_DEBUG
     printf("Unrecognizable appName %s\n",appName);
#endif
   }

   return sts;
}

PSX_STATUS xmlStartObjNode(char *appName, char *objTagName) {
   int i;
   PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
   printf("xmlStartObjNode(): appName %s, objTagName %s\n",
	  appName,objTagName);
#endif

   for ( i = 0; apps[i].appName[0] != '\0'; i++ ) {
      if (strncasecmp(apps[i].appName, appName, strlen(apps[i].appName)) == 0) {
        sts = PSX_STS_ERR_GENERAL;
	if (*(apps[i].startObjFnc) != NULL) {
	  sts = (*(apps[i].startObjFnc))(appName, objTagName);
	  break;
	}
      } /* found */
   } /* for */

   if (sts == PSX_STS_ERR_FIND_OBJ) {
#ifdef XML_DEBUG
     printf("Unrecognizable appName %s\n",appName);
#endif
     syslog(LOG_ERR,"startObjNode(): Unrecognizable appName %s, ignored\n",appName);
   }

   return sts;
}

PSX_STATUS xmlEndObjNode(char *appName, char *objTagName) {
   int i;
   PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
   printf("xmlEndObjNode(): appName %s, objTagName %s\n",
	  appName,objTagName);
#endif
   for ( i = 0; apps[i].appName[0] != '\0'; i++ ) {
      if (strncasecmp(apps[i].appName, appName, strlen(apps[i].appName)) == 0) {
        sts = PSX_STS_ERR_GENERAL;
	if (*(apps[i].endObjFnc) != NULL) {
	  sts = (*(apps[i].endObjFnc))(appName, objTagName);
	  break;
	}
      } /* found */
   } /* for */

   if (sts == PSX_STS_ERR_FIND_OBJ) {
#ifdef XML_DEBUG
     printf("Unrecognizable appName %s\n",appName);
#endif
     syslog(LOG_ERR,"xmlEndObjNode(): Unrecognizable appName %s, ignored\n",appName);
   }

   return sts;
}


PSX_STATUS xmlGetSysObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetSysObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 1; sysObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(sysObjs[i].objName, objName, strlen(sysObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(sysObjs[i].getAttrFnc) != NULL) {
	sts = (*(sysObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetSysObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

  }
  return sts;
} /* xmlGetSysObjNode */

PSX_STATUS xmlStartSysObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlStartSysObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; sysObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(sysObjs[i].objName, objName, strlen(sysObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(sysObjs[i].startObjFnc) != NULL) {
	sts = (*(sysObjs[i].startObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartSysObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

PSX_STATUS xmlEndSysObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlEndObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; sysObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(sysObjs[i].objName, objName, strlen(sysObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(sysObjs[i].endObjFnc) != NULL) {
	sts = (*(sysObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndSysObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

/* ======================================================================================= */
/* ========================================sysObjs======================================== */
PSX_STATUS xmlGetSysUserNameAttr(char *attrName, char* attrValue)
{
  int len = 0;
  char *ptr = (char*)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetSysUserNameAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif

  if (xmlState.verify)
    status = BcmDb_validateUserName(attrValue);
  if (status != DB_OBJ_VALID_OK) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndSysUserNameAttr(): invalid user name %s\n",ptr);
  }
  else {
    len = strlen(attrValue);
    strncpy(ptr,attrValue,len);
    ptr[len] = '\0';
  }

#ifdef XML_DEBUG
  printf("xmlGetSysUserNameAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif

  return(PSX_STS_OK);
}

PSX_STATUS xmlGetHttpPortAttr(char *attrName, char* attrValue)
{
  int iPort = 0;
  int iLen = 0;
  char *data = NULL;
  char *ptr = (char*)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetHttpPortAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif
   
        iLen = strlen(attrValue);
        if (iLen >= IFC_PASSWORD_LEN || iLen == 0)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetHttpPortAttr(): invalid user name %s\n",ptr);
            return PSX_STS_ERR_FATAL;
        }
        status = BcmDb_validateNumber(attrValue);
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetHttpPortAttr(): invalid user name %s\n",ptr);
        }
        else
        {
            iPort = atoi(attrValue);
            if (iPort >65535 ||(iPort<1024&&iPort!=80))
            {
                xmlState.errorFlag = PSX_STS_ERR_FATAL;
                syslog(LOG_ERR,"xmlGetHttpPortAttr(): invalid user name %s\n",ptr);
            }
            else
            {
                strcpy(ptr, attrValue);
            }
        }
       return(PSX_STS_OK);
}
PSX_STATUS xmlGetSysPasswordAttr(char *attrName, char* attrValue)
{
  int len = 0, dataLen = 0;
  char *data = NULL;
  char *ptr = (char*)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetSysPasswordAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif

  // for backward compatible with previous configuration file that 
  // have passwords without encoding, need to check encode password
  // info, if passwords are encoded then decode passwords, otherwise
  // do not decode passwords and set encode password info to true
  // so that when configuration file is saved, passwords are encoded
  // and encode password info is set to true.
  len = strlen(attrValue);
  if ( BcmDb_getEncodePasswordInfo() == TRUE ) {
    data = (char *)malloc(len * 3/4 + 8);
    /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
    }
    memset(data, 0, len * 3/4 + 8);
    /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
	/*start of 用户密码不加密功能l129990 */
    #ifdef SUPPORT_SYSPWD_UNENCRYPTED
	/*end of 用户密码不加密功能l129990 */
    strcpy(data, attrValue);
    #else
    base64Decode((unsigned char *)data, attrValue, &dataLen);
    data[dataLen] = '\0';
    #endif
  } else {
    data = (char *)malloc(len);
    /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
    }
    memset(data, 0, len);
    /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    strcpy(data, attrValue);
    BcmDb_setEncodePasswordInfo(TRUE);
  }

  if (xmlState.verify)
    status = BcmDb_validatePassword(data);
  if (status != DB_OBJ_VALID_OK) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndSysPasswordAttr(): invalid password %s\n",data);
  }
  else
    strcpy(ptr,data);

  free(data);
#ifdef XML_DEBUG
  printf("xmlGetSysPasswordAttr(psiValue %s, validationStatus %d)\n",
	 (char*)psiValue,status);
#endif
  return(PSX_STS_OK);
}
PSX_STATUS xmlStartSysUserNamePasswordObj(char *appName, char *objName)
{
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日"
  char *ptr;
  end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
#ifdef XML_DEBUG
  printf("xmlStartSysUserNamePasswordObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(IFC_PASSWORD_LEN);
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日"
  ptr = (char*)psiValue;
  ptr[0] = '\0';
  */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, IFC_PASSWORD_LEN);
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  return PSX_STS_OK;
}
PSX_STATUS xmlStartSysHttpPortObj(char *appName, char *objName)
{
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日"
  char *ptr;
  end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
#ifdef XML_DEBUG
  printf("xmlStartSysHttpPortObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(IFC_PASSWORD_LEN);
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日"
  ptr = (char*)psiValue;
  ptr[0] = '\0';
  */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, IFC_PASSWORD_LEN);
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSysHttpPortObj(char *appName, char *objName)
{
  char *ptr=(char*)psiValue;
  int len = strlen(ptr) + 1;

#ifdef XML_DEBUG
  printf("xmlEndSysHttpPortObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
    
  if (!xmlState.verify) {
    if (strcmp(objName,PSX_OBJ_HTTP_PORT) == 0) {
        if (strlen(ptr)>0)
        {
            BcmDb_setHttpPortInfo(ptr);
        }
#ifdef XML_DEBUG
      printf("called xmlEndSysHttpPortObj(%s,%d)\n",ptr, len);
#endif
    }
   
  } /* not verify */

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSysUserNamePasswordObj(char *appName, char *objName)
{
  char *ptr=(char*)psiValue;
  int len = strlen(ptr) + 1;

#ifdef XML_DEBUG
  printf("xmlEndSysUserNamePasswordObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  
  if (!xmlState.verify) {
    if (strcmp(objName,PSX_OBJ_SYS_USER_NAME) == 0) {
      BcmDb_setSysUserName(ptr, len);

#ifdef XML_DEBUG
      printf("called BcmDb_setSysUserName(%s,%d)\n",ptr, len);
#endif
    }
    else if (strcmp(objName,PSX_OBJ_SYS_PASSWORD) == 0) {
      BcmDb_setSysPassword(ptr, len);

#ifdef XML_DEBUG
      printf("called BcmDb_setSysPassword(%s,%d)\n",ptr, len);
#endif
    }
#ifndef SUPPORT_ONE_USER
    else if (strcmp(objName,PSX_OBJ_SPT_USER_NAME) == 0) {
      BcmDb_setSptUserName(ptr, len);
#ifdef XML_DEBUG
      printf("called BcmDb_setSptUserName(%s,%d)\n",ptr, len);
#endif
    }
    else if (strcmp(objName,PSX_OBJ_SPT_PASSWORD) == 0) {
      BcmDb_setSptPassword(ptr, len);
#ifdef XML_DEBUG
      printf("called BcmDb_setSptPasword(%s,%d)\n",ptr, len);
#endif
    }
    else if (strcmp(objName,PSX_OBJ_USR_USER_NAME) == 0) {
      BcmDb_setUsrUserName(ptr, len);
#ifdef XML_DEBUG
      printf("calling BcmDb_setUsrUserName(%s,%d)\n",ptr, len);
#endif
    }
    else if (strcmp(objName,PSX_OBJ_USR_PASSWORD) == 0) {
#ifdef XML_DEBUG
      printf("calling BcmDb_setusrPassword(%s,%d)\n",ptr, len);
#endif
      BcmDb_setUsrPassword(ptr,len);
    }
#endif
  } /* not verify */

#ifdef XML_DEBUG1
  printf("\n============End system objName %s=========\n",objName);
  printf("psiValue %s\n",ptr);
  printf("==========================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#ifdef SUPPORT_TR69C
PSX_STATUS xmlGetTr69cAttr(char *attrName, char* attrValue)
{
  PBCM_TR69C_INFO pTr69c = (PBCM_TR69C_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetTr69cAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_STATE,strlen(PSX_ATTR_STATE)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid state %s\n",attrValue);
    }
    else {
      if ( strcmp(attrValue, "disable") == 0 )
        pTr69c->status = BcmCfm_CfgDisabled;
      else
        pTr69c->status = BcmCfm_CfgEnabled;
    }
  }
  else if (strncmp(attrName,"upgradesManaged",strlen("upgradesManaged")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradesManaged %s\n",attrValue);
    }
    else
      pTr69c->upgradesManaged = atoi(attrValue);
  }
  else if (strncmp(attrName,"upgradeAvailable",strlen("upgradeAvailable")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradeAvailable %s\n",attrValue);
    }
    else
      pTr69c->upgradeAvailable = atoi(attrValue);
  }
  else if (strncmp(attrName,"informEnbl",strlen("informEnbl")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informEnbl %s\n",attrValue);
    }
    else
      pTr69c->informEnable = atoi(attrValue);
  }
  else if (strncmp(attrName,"informTime",strlen("informTime")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informTime %s\n",attrValue);
    }
    else
      pTr69c->informTime = strtoul(attrValue, (char **)NULL, 10);
  }
  else if (strncmp(attrName,"informInterval",strlen("informInterval")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informInterval %s\n",attrValue);
    }
    else
      pTr69c->informInterval = strtoul(attrValue, (char **)NULL, 10);
  }
  else if (strncmp(attrName,"acsURL",strlen("acsURL")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsURL %s\n",attrValue);
    }
    else
      strcpy(pTr69c->acsURL, attrValue);
  }
  else if (strncmp(attrName,"acsUser",strlen("acsUser")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsUser %s\n",attrValue);
    }
    else
      strcpy(pTr69c->acsUser, attrValue);
  }
  else if (strncmp(attrName,"acsPwd",strlen("acsPwd")) == 0) {
    if (xmlState.verify)
    /*start of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);*/
      status = BcmDb_validateLength(attrValue, TR69C_PASS_LEN);
    /*end of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsPwd %s\n",attrValue);
    }
    else{
	  /*add by w00135351 09.3.13 密码明文显示在配置文件中*/
	  #ifndef SUPPORT_TR69CPWD_UNENCRYPTED
      /*start of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715
      char dest_string[IFC_PASSWORD_LEN];
      */
      char dest_string[TR69C_URL_LEN];
       /*end of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */
      memset(dest_string, 0, sizeof(dest_string));
      base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
      strcpy(pTr69c->acsPwd, dest_string);
	  #else
	  strcpy(pTr69c->acsPwd,attrValue);
	  #endif
	  /*end by w00135351 09.3.13 密码明文显示在配置文件中*/
      }
  }
  else if (strncmp(attrName,"parameterKey",strlen("parameterKey")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_KEY_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid parameterKey %s\n",attrValue);
    }
    else
      strcpy(pTr69c->parameterKey, attrValue);
  }
  else if (strncmp(attrName,"connReqURL",strlen("connReqURL")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid connReqURL %s\n",attrValue);
    }
    else
      strcpy(pTr69c->connReqURL, attrValue);
  }
  else if (strncmp(attrName,"connReqUser",strlen("connReqUser")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid connReqUser %s\n",attrValue);
    }
    else
      strcpy(pTr69c->connReqUser, attrValue);
  }
  else if (strncmp(attrName,"connReqPwd",strlen("connReqPwd")) == 0) {
    if (xmlState.verify)
      /*start of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);*/
      status = BcmDb_validateLength(attrValue, TR69C_PASS_LEN);
    /*end of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid connReqPwd %s\n",attrValue);
    }
    else{
		/*add by w00135351 09.3.13 密码明文显示在配置文件中*/
		#ifndef SUPPORT_TR69CPWD_UNENCRYPTED
       /*start of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715
      char dest_string[IFC_PASSWORD_LEN];
      */
      char dest_string[TR69C_URL_LEN];
      /*end of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */
      memset(dest_string, 0, sizeof(dest_string));
      base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
      strcpy(pTr69c->connReqPwd, dest_string);
	  #else
	  strcpy(pTr69c->connReqPwd,attrValue);
	  #endif
    }
  }
  else if (strncmp(attrName,"kickURL",strlen("kickURL")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_URL_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid kickURL %s\n",attrValue);
    }
    else
      strcpy(pTr69c->kickURL, attrValue);
  }
  else if (strncmp(attrName,"provisioningCode",strlen("provisioningCode")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, TR69C_CODE_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid provisioningCode %s\n",attrValue);
    }
    else
      strcpy(pTr69c->provisioningCode, attrValue);
  }

#ifdef XML_DEBUG
  printf("xmlGetTr69cAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif

  return(PSX_STS_OK);
}

PSX_STATUS xmlStartTr69cObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartTr69cObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(BCM_TR69C_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(BCM_TR69C_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
  PBCM_TR69C_INFO pTr69c = (PBCM_TR69C_INFO)psiValue;
  
  if ( atoi(BcmDb_getDefaultValue("tr69cStatus")) == FALSE )
    pTr69c->status = BcmCfm_CfgDisabled;
  else
    pTr69c->status = BcmCfm_CfgEnabled;
  pTr69c->upgradesManaged = atoi(BcmDb_getDefaultValue("tr69cUpgradesManaged"));
  pTr69c->upgradeAvailable = atoi(BcmDb_getDefaultValue("tr69cUpgradeAvailable"));
  pTr69c->informEnable = atoi(BcmDb_getDefaultValue("tr69cInformEnable"));
  pTr69c->informTime = strtoul(BcmDb_getDefaultValue("tr69cInformTime"), (char **)NULL, 10);
  pTr69c->informInterval = strtoul(BcmDb_getDefaultValue("tr69cInformInterval"), (char **)NULL, 10);
  strcpy(pTr69c->acsURL, BcmDb_getDefaultValue("tr69cAcsURL"));
  strcpy(pTr69c->acsUser, BcmDb_getDefaultValue("tr69cAcsUser"));        
  strcpy(pTr69c->acsPwd, BcmDb_getDefaultValue("tr69cAcsPwd"));        
  strcpy(pTr69c->parameterKey, BcmDb_getDefaultValue("tr69cParameterKey"));
  strcpy(pTr69c->connReqURL, BcmDb_getDefaultValue("tr69cConnReqURL"));
  strcpy(pTr69c->connReqUser, BcmDb_getDefaultValue("tr69cConnReqUser"));
  strcpy(pTr69c->connReqPwd, BcmDb_getDefaultValue("tr69cConnReqPwd"));
  strcpy(pTr69c->kickURL, BcmDb_getDefaultValue("tr69cKickURL"));
  strcpy(pTr69c->provisioningCode, BcmDb_getDefaultValue("tr69cProvisioningCode"));
  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndTr69cObj(char *appName, char *objName)
{
  PBCM_TR69C_INFO pTr69c = (PBCM_TR69C_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndTr69cObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setTr69cInfo(pTr69c);

  free(psiValue);
  psiValue = NULL;

  return PSX_STS_OK;
}
#endif   // SUPPORT_TR69C


#ifdef        SUPPORT_RADIUS
/*start   of 增加Radius 认证功能by s53329  at  20060714 */
//**************************************************************************
// Function Name: BcmDb_setRadiuscInfo
// Description  : store Radius configurations
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlGetRadiusAttr(char *attrName, char* attrValue)
{
  PRADIUS_CLIENT_INFO  pRadiusInfo = (PRADIUS_CLIENT_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetTr69cAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif
  if (strncmp(attrName,"PrimaryServer",strlen("PrimaryServer")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, RADIUS_SERVER_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradesManaged %s\n",attrValue);
    }
    else
      strcpy(pRadiusInfo->PrimaryServer, attrValue) ;
  }
  else if (strncmp(attrName,"PrimaryPort",strlen("PrimaryPort")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradeAvailable %s\n",attrValue);
    }
    else
      pRadiusInfo->PrimaryPort = atoi(attrValue);
  }
  else if (strncmp(attrName,"SecondServer",strlen("SecondServer")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, RADIUS_SERVER_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informEnbl %s\n",attrValue);
    }
    else
        strcpy(pRadiusInfo->SecondServer, attrValue) ;
  }
  else if (strncmp(attrName,"SecondPort",strlen("SecondPort")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradeAvailable %s\n",attrValue);
    }
    else
      pRadiusInfo->SecondPort = atoi(attrValue);
  }
  else if (strncmp(attrName,"WebEnable",strlen("WebEnable")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informTime %s\n",attrValue);
    }
    else
      pRadiusInfo->WebEnable = atoi(attrValue);
  }
  else if (strncmp(attrName,"TelnetEnable",strlen("TelnetEnable")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informInterval %s\n",attrValue);
    }
    else
       pRadiusInfo->TelnetEnable = atoi(attrValue);
  }
  else if (strncmp(attrName,"SshEnable",strlen("SshEnable")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsURL %s\n",attrValue);
    }
    else
      pRadiusInfo->SshEnable = atoi(attrValue);
  }
  else if (strncmp(attrName,"Retransmit",strlen("Retransmit")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsUser %s\n",attrValue);
    }
    else
    {
        pRadiusInfo->Retransmit = atoi(attrValue);
        if (pRadiusInfo->Retransmit <1 || pRadiusInfo->Retransmit >9)
        {
             xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsUser %s\n",attrValue);
        }
    }
  }
  else if (strncmp(attrName,"Timeout",strlen("Timeout")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsPwd %s\n",attrValue);
    }
    else
    {
        pRadiusInfo->Timeout = atoi(attrValue);
        if (pRadiusInfo->Timeout <1 || pRadiusInfo->Timeout >9)
        {
             xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsUser %s\n",attrValue);
        }
    }
  }
  else if (strncmp(attrName,"ShareKey",strlen("ShareKey")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, RADIUS_SHAREKEY_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid parameterKey %s\n",attrValue);
    }
    else
      strcpy(pRadiusInfo->ShareKey, attrValue);
  }
  #ifdef XML_DEBUG
  printf("xmlGetTr69cAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
 #endif

  return(PSX_STS_OK);
}


//**************************************************************************
// Function Name: BcmDb_setRadiuscInfo
// Description  : store Radius configurations
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlStartRadiusObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartTr69cObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  PRADIUS_CLIENT_INFO       pRadiusInfo;
  psiValue = (void*)malloc(sizeof(RADIUS_CLIENT_INFO));
  memset(psiValue, 0 , sizeof(RADIUS_CLIENT_INFO));
  pRadiusInfo = (PRADIUS_CLIENT_INFO)psiValue;
  pRadiusInfo->Timeout = atoi(BcmDb_getDefaultValue("RadiusTimeout"));
  pRadiusInfo->Retransmit = atoi(BcmDb_getDefaultValue("RadiusRetransmit"));
  pRadiusInfo->PrimaryPort= atoi(BcmDb_getDefaultValue("RadiusPrimaryServerPort"));
  pRadiusInfo->SecondPort = atoi(BcmDb_getDefaultValue("RadiusSecondServerPort"));
  return PSX_STS_OK;
}



//**************************************************************************
// Function Name: BcmDb_setRadiuscInfo
// Description  : store Radius configurations
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlEndRadiusObj(char *appName, char *objName)
{
  PRADIUS_CLIENT_INFO pRadiusInfo  = (PRADIUS_CLIENT_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndTr69cObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setRadiusInfo(pRadiusInfo);

  free(psiValue);
  psiValue = NULL;

  return PSX_STS_OK;
}
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
#endif

/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
#ifdef SUPPORT_SIPROXD 
void xmlSetSiproxdAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
    PSIPPROXYINFO pSipInfo;
   static char* state[] = {"disable","enable"};
   if ((name == NULL) || (value == NULL))
       return;
    pSipInfo = (PSIPPROXYINFO)value;
	
    fprintf(file, "<%s enable=\"%s\" port=\"%d\"/>\n",name, state[pSipInfo->enable], pSipInfo->portnumber);

}

//**************************************************************************
// Function Name: xmlGetSiproxdAttr
// Description  : get Sip Proxy Deamon configurations
// Returns      : DB_OBJ_VALID_OK: sucess, OTHER: fail.
//**************************************************************************
PSX_STATUS xmlGetSiproxdAttr(char *attrName, char* attrValue)
{
  PSIPPROXYINFO pSipInfo = (PSIPPROXYINFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetSiproxdAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif
  if (strncmp(attrName,"enable",strlen("enable")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSiproxdAttr(): invalid upgradesManaged %s\n",attrValue);
    }
    else
    {    
          pSipInfo->enable = xmlStateToPsi(attrValue);
    }
  }
  else if (strncmp(attrName,"port",strlen("port")) == 0) 
  {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSiproxdAttr(): invalid %s %s\n",PSX_ATTR_VOICE_SIP_PROXY_PORT,attrValue);
    }
    else {
      pSipInfo->portnumber = strtoul(attrValue, (char **)NULL, 10) ;
      }
  }
  
#ifdef XML_DEBUG
  printf("xmlGetSiproxdAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif
  return (PSX_STS_OK);
}

//**************************************************************************
// Function Name: xmlStartSiproxdObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlStartSiproxdObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartSiproxdObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  PSIPPROXYINFO pSipInfo;
  psiValue = (void*)malloc(sizeof(SIPPROXYINFO));
  if(psiValue == NULL)
  {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0 , sizeof(SIPPROXYINFO));
  pSipInfo = (PSIPPROXYINFO)psiValue;
  pSipInfo->enable = atoi(BcmDb_getDefaultValue("enblSiproxd"));
  pSipInfo->portnumber = atoi(BcmDb_getDefaultValue("sipProxyPort"));
  return PSX_STS_OK;
}

//**************************************************************************
// Function Name: xmlEndSiproxdObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlEndSiproxdObj(char *appName, char *objName)
{
  PSIPPROXYINFO pSipInfo = (PSIPPROXYINFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndSiproxdObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
  {
	BcmDb_setSiproxdInfo(pSipInfo->enable, pSipInfo->portnumber);
  }

  free(psiValue);
  psiValue = NULL;

  return PSX_STS_OK;
} 
#endif
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/

#ifdef SUPPORT_GLB_ALG
void xmlSetNatalgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,const UINT16 length)
{
    PNATALGINFO pNatAlgInfo;
    static char* state[] = {"disable","enable"};
    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    pNatAlgInfo = (PNATALGINFO)value;
     
    fprintf(file, "<%s H323Alg=\"%s\" RtspAlg=\"%s\" FtpAlg=\"%s\" TftpAlg=\"%s\" L2tpAlg=\"%s\" PptpAlg=\"%s\" IpsecAlg=\"%s\"/>\n",
        name, state[pNatAlgInfo->H323Alg], state[pNatAlgInfo->RtspAlg], state[pNatAlgInfo->FtpAlg], state[pNatAlgInfo->TftpAlg], 
        state[pNatAlgInfo->L2tpAlg], state[pNatAlgInfo->PptpAlg], state[pNatAlgInfo->IpsecAlg]);

}

//**************************************************************************
// Function Name: xmlGetNatalgAttr
// Description  : get NATALG configurations
// Returns      : DB_OBJ_VALID_OK: sucess, OTHER: fail.
//**************************************************************************
PSX_STATUS xmlGetNatalgAttr(char *attrName, char* attrValue)
{
    //PSIPPROXYINFO pSipInfo = (PSIPPROXYINFO)psiValue;
    PNATALGINFO pNatAlgInfo = (PNATALGINFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetNatalgAttr(attrName %s, attrValue %s)\n",
        attrName, attrValue);
#endif

    if ( 0 == strncmp(attrName,"H323Alg",strlen("H323Alg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->H323Alg = xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"RtspAlg",strlen("RtspAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->RtspAlg= xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"FtpAlg",strlen("FtpAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->FtpAlg= xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"TftpAlg",strlen("TftpAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->TftpAlg= xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"L2tpAlg",strlen("L2tpAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->L2tpAlg= xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"PptpAlg",strlen("PptpAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->PptpAlg= xmlStateToPsi(attrValue);
        }
    }
    else if ( 0 == strncmp(attrName,"IpsecAlg",strlen("IpsecAlg")) )
    {
        if ( xmlState.verify)
        {
            status = BcmDb_validateState(attrValue);
        }
        if ( DB_OBJ_VALID_OK != status )
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetNatalgAttr(): invalid %s %s\n",attrName,attrValue);
        }
        else
        {
            pNatAlgInfo->IpsecAlg= xmlStateToPsi(attrValue);
        }
    }
 
#ifdef XML_DEBUG
    printf("xmlGetNatalgAttr(psiValue %s), validationStatus %d\n",
        (char*)psiValue,status);
#endif
    return (PSX_STS_OK);
}

//**************************************************************************
// Function Name: xmlStartNatalgObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlStartNatalgObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
    printf("xmlStartNatalgObj(calling cfm to get appName %s, objName %s\n",
        appName,objName);
#endif
    //PSIPPROXYINFO pSipInfo;
    PNATALGINFO pNatAlgInfo;
//lint -save -e10
    psiValue = (void*)malloc(sizeof(NATALGINFO));
//lint -restore
    if(psiValue == NULL)
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }

    memset(psiValue, 0 , sizeof(NATALGINFO));
    pNatAlgInfo = (PNATALGINFO)psiValue;
    pNatAlgInfo->H323Alg= atoi(BcmDb_getDefaultValue("enblH323"));
    pNatAlgInfo->RtspAlg= atoi(BcmDb_getDefaultValue("enblRtsp"));
    pNatAlgInfo->FtpAlg= atoi(BcmDb_getDefaultValue("enblFtp"));
    pNatAlgInfo->TftpAlg= atoi(BcmDb_getDefaultValue("enblTftp"));
    pNatAlgInfo->L2tpAlg= atoi(BcmDb_getDefaultValue("enblL2tp"));
    pNatAlgInfo->PptpAlg= atoi(BcmDb_getDefaultValue("enblPptp"));
    pNatAlgInfo->IpsecAlg= atoi(BcmDb_getDefaultValue("enblIpsec"));
    return PSX_STS_OK;
}

//**************************************************************************
// Function Name: xmlEndNatalgObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlEndNatalgObj(char *appName, char *objName)
{
    PNATALGINFO pNatAlgInfo = (PNATALGINFO)psiValue;
#ifdef XML_DEBUG
    printf("xmlEndSiproxdObj(calling cfm to get appName %s, objName %s\n",
        appName,objName);
#endif

    if (!xmlState.verify)
    {
        BcmDb_setNatAlgInfo(pNatAlgInfo);
    }

    free(psiValue);
    psiValue = NULL;

    return PSX_STS_OK;
} 
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
void xmlSetAutoserchAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,const UINT16 length)
{
    AUTOSERCHINFO AutoSerchInfo;
    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    
    memset(&AutoSerchInfo,0,sizeof(AUTOSERCHINFO));
    memcpy(&AutoSerchInfo,(char *)value,length);

    fprintf(file, "<%s ScanList=\"%s\"/>\n",name, AutoSerchInfo.ScanList);
}

//**************************************************************************
// Function Name: xmlGetAutoscanAttr
// Description  : get AUTOSCAN LIST
// Returns      : DB_OBJ_VALID_OK: sucess, OTHER: fail.
//**************************************************************************
PSX_STATUS xmlGetAutoserchAttr(char *attrName, char* attrValue)
{
    PAUTOSERCHINFO pAutoSerchInfo = (PAUTOSERCHINFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetAutoscanAttr(attrName %s, attrValue %s)\n",
        attrName, attrValue);
#endif

    if (strncmp(attrName,"ScanList",strlen("ScanList")) == 0) 
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateLength(attrValue, IFC_SCANLIST_LEN);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAutoscanAttr(): invalid ScanList %s\n",attrValue);
        }
        else
        {
            strcpy(pAutoSerchInfo->ScanList, attrValue);
        }
    }
   
#ifdef XML_DEBUG
    printf("xmlGetNatalgAttr(psiValue %s), validationStatus %d\n",
        (char*)psiValue,status);
#endif
    return (PSX_STS_OK);
}

//**************************************************************************
// Function Name: xmlStartAutoscanObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlStartAutoserchObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
    printf("xmlStartAutoscanObj(calling cfm to get appName %s, objName %s\n",
        appName,objName);
#endif
    //char scanList[IFC_SCANLIST_LEN];
//lint -save -e10
    psiValue = (void*)malloc(sizeof(AUTOSERCHINFO));
//lint -restore
    if(psiValue == NULL)
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }

    memset(psiValue, 0 , sizeof(AUTOSERCHINFO));

    PAUTOSERCHINFO pAutoSerchInfo = (PAUTOSERCHINFO)psiValue;
    strcpy(pAutoSerchInfo->ScanList,BcmDb_getDefaultValue("autoScanList"));

    return PSX_STS_OK;
}

//**************************************************************************
// Function Name: xmlEndAutoscanObj
// Description  : 
// Returns      : none.
//**************************************************************************
PSX_STATUS xmlEndAutoserchObj(char *appName, char *objName)
{
    PAUTOSERCHINFO pAutoSerchInfo = (PAUTOSERCHINFO)psiValue;
#ifdef XML_DEBUG
    printf("xmlEndAutoscanObj(calling cfm to get appName %s, objName %s\n",
        appName,objName);
#endif

    if (!xmlState.verify)
    {
        BcmDb_setAutoSerchInfo(pAutoSerchInfo);
    }

    free(psiValue);
    psiValue = NULL;

    return PSX_STS_OK;
} 
/* END:   Added by y67514, 2008/10/27 */

/*start of add the Upnp Video Provision function by l129990,2010,2,21*/
PSX_STATUS xmlStartUpnpVideoObj(char *appName, char *objName)
{
    PUPNP_VIDEO_INFO pstUpnpVideoInfo;
    if ((NULL == appName) || (NULL == objName))
    {
        return PSX_STS_ERR_GENERAL;
    }
     
    psiValue = (void*)malloc(sizeof(UPNP_VIDEO_INFO));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    
    memset(psiValue, 0 , sizeof(UPNP_VIDEO_INFO));
    pstUpnpVideoInfo = (PUPNP_VIDEO_INFO)psiValue;
    pstUpnpVideoInfo->serviceEnable = atoi(BcmDb_getDefaultValue("upnpVideoServ"));
    pstUpnpVideoInfo->provEnable = atoi(BcmDb_getDefaultValue("upnpVideoProv"));
    return PSX_STS_OK;
}

void xmlSetUpnpVideoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
    PUPNP_VIDEO_INFO pstUpnpVideoInfo;
    static char* state[] = {"disable","enable"};

    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    
    pstUpnpVideoInfo = (PUPNP_VIDEO_INFO)value;
    fprintf(file, "<%s service=\"%s\" provision=\"%s\"/>\n",name, state[pstUpnpVideoInfo->serviceEnable], state[pstUpnpVideoInfo->provEnable]);

}

PSX_STATUS xmlGetUpnpVideoAttr(char *attrName, char* attrValue)
{
    PUPNP_VIDEO_INFO pstUpnpVideoInfo = (PUPNP_VIDEO_INFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetUpnpVideoAttr(attrName %s, attrValue %s)\n",
	    attrName, attrValue);
#endif
    if (strncmp(attrName,"service",strlen("service")) == 0)
    {
        if (xmlState.verify)
        {
          status = BcmDb_validateState(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK) 
        {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetUpnpVideoAttr(): invalid upgradesManaged %s\n",attrValue);
        }
        else
        {    
              pstUpnpVideoInfo->serviceEnable = xmlStateToPsi(attrValue);
        }
   }
   else if (strncmp(attrName,"provision",strlen("provision")) == 0) 
   {
        if (xmlState.verify)
        {
          status = BcmDb_validateState(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK) 
        {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetUpnpVideoAttr(): invalid upgradesManaged %s\n",attrValue);
        }
        else
        {    
              pstUpnpVideoInfo->provEnable = xmlStateToPsi(attrValue);
        }
   }
  
#ifdef XML_DEBUG
  printf("xmlGetUpnpVideoAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif
  return (PSX_STS_OK);
}

PSX_STATUS xmlEndUpnpVideoObj(char *appName, char *objName)
{
    PUPNP_VIDEO_INFO pstUpnpVideoInfo = (PUPNP_VIDEO_INFO)psiValue;

#ifdef XML_DEBUG
    printf("xmlEndUpnpVideoObj(calling cfm to get appName %s, objName %s\n",
	    appName,objName);
#endif

    if (!xmlState.verify)
    {
	    AtpDb_setUpnpVEnable(pstUpnpVideoInfo);
    }

    free(psiValue);
    psiValue = NULL;
  
    return PSX_STS_OK;
} 
/*end of add the Upnp Video Provision function by l129990,2010,2,21*/

//BEGIN:added by zhourongfei to config web at 2011/05/31
PSX_STATUS xmlStartWebObj(char *appName, char *objName)
{
    PWEB_CFG_INFO pstWebInfo;
    if ((NULL == appName) || (NULL == objName))
    {
        return PSX_STS_ERR_GENERAL;
    }
     
    psiValue = (void*)malloc(sizeof(WEB_CFG_INFO));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    
    memset(psiValue, 0 , sizeof(WEB_CFG_INFO));
    pstWebInfo = (PWEB_CFG_INFO)psiValue;
    pstWebInfo->LANDisplayCFG= atoi(BcmDb_getDefaultValue("LANDisplayCFG"));
    pstWebInfo->PINDisplayCFG= atoi(BcmDb_getDefaultValue("PINDisplayCFG"));
    pstWebInfo->HSPADisplayCFG= atoi(BcmDb_getDefaultValue("HSPADisplayCFG"));
    return PSX_STS_OK;
}

void xmlSetWebAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
    PWEB_CFG_INFO pstwebInfo;
    static char* state[] = {"disable","enable"};

    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    
    pstwebInfo = (PWEB_CFG_INFO)value;
    fprintf(file, "<%s LANDisplayCFG=\"%s\" PINDisplayCFG=\"%s\" HSPADisplayCFG=\"%s\"/>\n",name, state[pstwebInfo->LANDisplayCFG], state[pstwebInfo->PINDisplayCFG], state[pstwebInfo->HSPADisplayCFG]);

}

PSX_STATUS xmlGetWebAttr(char *attrName, char* attrValue)
{
    PWEB_CFG_INFO pstWebInfo = (PWEB_CFG_INFO)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

    if (strncmp(attrName,"LANDisplayCFG",strlen("LANDisplayCFG")) == 0)
    {
        if (xmlState.verify)
        {
          status = BcmDb_validateState(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK) 
        {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
        }
        else
        {    
              pstWebInfo->LANDisplayCFG= xmlStateToPsi(attrValue);
        }
   }
   else if (strncmp(attrName,"PINDisplayCFG",strlen("PINDisplayCFG")) == 0) 
   {
        if (xmlState.verify)
        {
          status = BcmDb_validateState(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK) 
        {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
        }
        else
        {    
              pstWebInfo->PINDisplayCFG= xmlStateToPsi(attrValue);
        }
   }
   else if (strncmp(attrName,"HSPADisplayCFG",strlen("HSPADisplayCFG")) ==0)
   {
	   if (xmlState.verify)
	   {
		 status = BcmDb_validateState(attrValue);
	   }
	   
	   if (status != DB_OBJ_VALID_OK) 
	   {
		 xmlState.errorFlag = PSX_STS_ERR_FATAL;
	   }
	   else
	   {	
			 pstWebInfo->HSPADisplayCFG= xmlStateToPsi(attrValue);
	   }
   }
  
  return (PSX_STS_OK);
}

PSX_STATUS xmlEndWebObj(char *appName, char *objName)
{
    PWEB_CFG_INFO pstWebInfo = (PWEB_CFG_INFO)psiValue;

    if (!xmlState.verify)
    {
	    BcmDb_setWebInfo(pstWebInfo);
    }

    free(psiValue);
    psiValue = NULL;
  
    return PSX_STS_OK;
} 
//END:added by zhourongfei to config web at 2011/05/31

void xmlSetFtpinfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
    FTP_INFO *pFtpInfo;
    char *base64 = NULL;
    char str[IFC_LARGE_LEN];

    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    
    pFtpInfo = (FTP_INFO *)value;

	/*add by w00135351 09.3.13 密码明文显示在配置文件中*/
	#ifndef SUPPORT_FTPPWD_UNENCRYPTED
    memcpy(str, pFtpInfo->ftpUserPassWord, strlen(pFtpInfo->ftpUserPassWord));
    str[strlen(pFtpInfo->ftpUserPassWord)] = '\0';

    base64Encode((unsigned char *)str, strlen(str), &base64);
    
    fprintf(file, "<%s enable=\"%s\" username=\"%s\" password=\"%s\" port=\"%s\" path=\"%s\"/>\n",
            name, pFtpInfo->ftpEnable, pFtpInfo->ftpUserName, base64, pFtpInfo->ftpPort, pFtpInfo->ftpPath);
    
    free(base64);
	#else
	fprintf(file, "<%s enable=\"%s\" username=\"%s\" password=\"%s\" port=\"%s\" path=\"%s\"/>\n",
            name, pFtpInfo->ftpEnable, pFtpInfo->ftpUserName, pFtpInfo->ftpUserPassWord, pFtpInfo->ftpPort, pFtpInfo->ftpPath);
	#endif
	/*end by w00135351 09.3.13 密码明文显示在配置文件中*/
    return;
}

PSX_STATUS xmlGetFtpinfoAttr(char *attrName, char* attrValue)
{
    FTP_INFO *pFtpInfo = (FTP_INFO *)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetFtpinfoAttr(attrName %s, attrValue %s)\n", attrName, attrValue);
#endif

    if (strncmp(attrName, "enable", strlen("enable")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateUserName(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
        }
        else
        {   
            strcpy(pFtpInfo->ftpEnable, attrValue);
        }
    }
    else if (strncmp(attrName, "username", strlen("username")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateUserName(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
        }
        else
        {    
            strcpy(pFtpInfo->ftpUserName, attrValue);
        }
    }
    else if (strncmp(attrName, "password", strlen("password")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validatePassword(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
        }
        else
        {
        	/*add by w00135351 09.3.13 密码明文显示在配置文件中*/
			#ifndef SUPPORT_FTPPWD_UNENCRYPTED
            char dest_string[IFC_PASSWORD_LEN];
            memset(dest_string, 0, sizeof(dest_string));
            base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
            strcpy(pFtpInfo->ftpUserPassWord, dest_string);
			#else
			strcpy(pFtpInfo->ftpUserPassWord, attrValue);
			#endif
			/*end by w00135351 09.3.13 密码明文显示在配置文件中*/
        }
    }
    else if (strncmp(attrName, "port", strlen("port")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateUserName(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
        }
        else
        {    
            strcpy(pFtpInfo->ftpPort, attrValue);
        }
    }
    else if (strncmp(attrName, "path", strlen("path")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateFtpPath(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
        }
        else
        {    
            strcpy(pFtpInfo->ftpPath, attrValue);
        }
    }

#ifdef XML_DEBUG
    printf("xmlGetFtpinfoAttr(psiValue %s), validationStatus %d\n", (char*)psiValue, status);
#endif
    return PSX_STS_OK;
}

PSX_STATUS xmlStartFtpinfoObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
    printf("xmlStartFtpinfoObj(calling cfm to get appName %s, objName %s\n", appName, objName);
#endif
    FTP_INFO *pFtpInfo;

    psiValue = (void*)malloc(sizeof(FTP_INFO));
    if(psiValue == NULL)
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }

    memset(psiValue, 0 , sizeof(FTP_INFO));

    pFtpInfo = (FTP_INFO *)psiValue;

    strcpy(pFtpInfo->ftpUserName, BcmDb_getDefaultValue("sysUserName"));
    strcpy(pFtpInfo->ftpUserPassWord, BcmDb_getDefaultValue("sysPassword"));
    strcpy(pFtpInfo->ftpPath, "/var");
    strcpy(pFtpInfo->ftpEnable, "0");
    strcpy(pFtpInfo->ftpPort, "21");
    return PSX_STS_OK;
}

PSX_STATUS xmlEndFtpinfoObj(char *appName, char *objName)
{
    FTP_INFO *pFtpInfo = (FTP_INFO *)psiValue;

#ifdef XML_DEBUG
    printf("xmlEndFtpinfoObj(calling cfm to get appName %s, objName %s\n", appName, objName);
#endif

    if (!xmlState.verify)
    {
        AtpDb_setFTPInfo(pFtpInfo);
    }

    free(psiValue);
    psiValue = NULL;

    return PSX_STS_OK;
} 
/*d00104343 USB Camera*/
#ifdef WEBCAM
void xmlSetCamerainfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length)
{
    USB_CAMERA_INFO *pCameraInfo;
    char *pcNetcamPwbase64 = NULL;
    char *pcRemotePwbase64 = NULL;
    char str[IFC_LARGE_LEN];

    if ((name == NULL) || (value == NULL))
    {
        return;
    }
    pCameraInfo = (USB_CAMERA_INFO *)value;
    /*Start:Camera pw 加密 ，2007.3.21*/
    memcpy(str, pCameraInfo->netcam_userpass, strlen(pCameraInfo->netcam_userpass));
    str[strlen(pCameraInfo->netcam_userpass)] = '\0';
    base64Encode((unsigned char *)str, strlen(str), &pcNetcamPwbase64);

    memcpy(str, pCameraInfo->remote_pass, strlen(pCameraInfo->remote_pass));
    str[strlen(pCameraInfo->remote_pass)] = '\0';
    base64Encode((unsigned char *)str, strlen(str), &pcRemotePwbase64);
    /*End:Camera pw 加密 ，2007.3.21*/
    
    fprintf(file, "<%s enable=\"%d\" auto_bright=\"%d\" brightness=\"%d\" netcam_url=\"%s\" netcam_userpass=\"%s\" port=\"%d\" framerate=\"%d\" remote_enable=\"%d\" remote_pass=\"%s\" height=\"%d\" width=\"%d\"/>\n",
            name, pCameraInfo->enable, pCameraInfo->auto_brightness, pCameraInfo->brightness, 
            pCameraInfo->netcam_url, pcNetcamPwbase64, pCameraInfo->port,
            pCameraInfo->framerate, pCameraInfo->remote_enable, pcRemotePwbase64,
            pCameraInfo->height, pCameraInfo->width );
            
    free(pcNetcamPwbase64);
    free(pcRemotePwbase64);

    return;
}

PSX_STATUS xmlGetCamerainfoAttr(char *attrName, char* attrValue)
{
    USB_CAMERA_INFO *pCameraInfo = (USB_CAMERA_INFO *)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetFtpinfoAttr(attrName %s, attrValue %s)\n", attrName, attrValue);
#endif

    if (strncmp(attrName, "enable", strlen("enable")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {   
            pCameraInfo->enable = atoi(attrValue);
        }
    }

    else if (strncmp(attrName, "brightness", strlen("brightness")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {
            pCameraInfo->brightness = atoi(attrValue);
        }
    }
    else if (strncmp(attrName, "auto_bright", strlen("auto_bright")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateBoolean(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->auto_brightness = atoi(attrValue);
        }
    }
    else if (strncmp(attrName, "framerate", strlen("framerate")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->framerate = atoi(attrValue);
        }
    }
    else if (strncmp(attrName, "netcam_url", strlen("netcam_url")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            strcpy(pCameraInfo->netcam_url, attrValue);
        }
    }
    else if (strncmp(attrName, "netcam_userpass", strlen("netcam_userpass")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            char dest_string[IFC_PASSWORD_LEN];
            memset(dest_string, 0, sizeof(dest_string));
            base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
            strcpy(pCameraInfo->netcam_userpass, dest_string);
        }
    }
    else if (strncmp(attrName, "remote_pass", strlen("remote_pass")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            char dest_string[IFC_PASSWORD_LEN];
            memset(dest_string, 0, sizeof(dest_string));
            base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
            strcpy(pCameraInfo->remote_pass, dest_string);
        }
    }
    else if (strncmp(attrName, "height", strlen("height")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->height =  atoi(attrValue);
        }
    }
    else if (strncmp(attrName, "width", strlen("width")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->width = atoi(attrValue);
        }
    }
    else if (strncmp(attrName, "port", strlen("port")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->port = atoi(attrValue);
        }
    }

    else if (strncmp(attrName, "remote_enable", strlen("remote_enable")) == 0)
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateBoolean(attrValue);
        }
        
        if (status != DB_OBJ_VALID_OK)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR, "xmlGetFtpinfoAttr(): invalid %s %s\n", PSX_OBJ_FTP_INFO, attrValue);
            printf("%s, %d\n", __FILE__, __LINE__);
        }
        else
        {    
            pCameraInfo->remote_enable = atoi(attrValue);
        }
    }


#ifdef XML_DEBUG
    printf("xmlGetCamerainfoAttr(psiValue %s), validationStatus %d\n", (char*)psiValue, status);
#endif
    return PSX_STS_OK;
}

PSX_STATUS xmlStartCamerainfoObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
    printf("xmlStartFtpinfoObj(calling cfm to get appName %s, objName %s\n", appName, objName);
#endif
    USB_CAMERA_INFO *pCameraInfo;

    psiValue = (void*)malloc(sizeof(USB_CAMERA_INFO));
    if(psiValue == NULL)
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }

    memset(psiValue, 0 , sizeof(USB_CAMERA_INFO));

    pCameraInfo = (USB_CAMERA_INFO *)psiValue;

    pCameraInfo->enable = 0;//BcmDb_getDefaultValue("sysUserName"));
    pCameraInfo->auto_brightness = 1;//BcmDb_getDefaultValue("sysPassword"));
    pCameraInfo->brightness = 128;
    strcpy(pCameraInfo->netcam_url,         "http://");
    strcpy(pCameraInfo->netcam_userpass,    "Huawei:huawei");
    strcpy(pCameraInfo->remote_pass,        "Huawei:huawei");
    pCameraInfo->port = 18080;
    pCameraInfo->framerate = 2;
    pCameraInfo->remote_enable = 0;
    pCameraInfo->height = 240;
    pCameraInfo->width  = 320;

    return PSX_STS_OK;
}

PSX_STATUS xmlEndCamerainfoObj(char *appName, char *objName)
{
    USB_CAMERA_INFO *pCameraInfo = (USB_CAMERA_INFO *)psiValue;

#ifdef XML_DEBUG
    printf("xmlEndFtpinfoObj(calling cfm to get appName %s, objName %s\n", appName, objName);
#endif

    if (!xmlState.verify)
    {
        HWDb_setCameraInfo(pCameraInfo);
    }

    free(psiValue);
    psiValue = NULL;

    return PSX_STS_OK;
} 
#endif
/*d00104343 USB Camera End */
PSX_STATUS xmlGetMgmtLockAttr(char *attrName, char* attrValue)
{
  PSYS_MGMT_LOCK_INFO pMgmtLock = (PSYS_MGMT_LOCK_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  static char* types[] = {"unknown","CLI","menuCLI","WEB","SNMP","TR069","UPNP", NULL};

#ifdef XML_DEBUG
  printf("xmlGetMgmtLockAttr(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif

  if (strncmp(attrName,"action",strlen("action")) == 0) {
    if (xmlState.verify) {
      if ( strcmp(attrValue, "unlock") != 0 && strcmp(attrValue, "lock") != 0 )
        status = DB_OBJ_VALID_ERR_INVALID_VALUE;
    }
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMgmtLockAttr(): invalid action %s\n",attrValue);
    }
    else {
      if ( strcmp(attrValue, "unlock") == 0 )
        pMgmtLock->action = BcmCfm_MgmtUnlocked;
      else
        pMgmtLock->action = BcmCfm_MgmtLocked;
    }
  }
  else if (strncmp(attrName,"type",strlen("type")) == 0) {
    int i = 0;
    for ( i = 0; types[i] != NULL; i++ )
      if ( strcmp(attrValue, types[i]) == 0 )
        break;
    if (xmlState.verify) {
      if ( types[i] == NULL )
        status = DB_OBJ_VALID_ERR_INVALID_VALUE;
    }
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMgmtLockAttr(): invalid management type %s\n",attrValue);
    }
    else
      pMgmtLock->type = (BcmCfm_TypeMgmt)i;
  }

#ifdef XML_DEBUG
  printf("xmlGetMgmtLockAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif

  return(PSX_STS_OK);
}

PSX_STATUS xmlStartMgmtLockObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartMgmtLockObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SYS_MGMT_LOCK_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(SYS_MGMT_LOCK_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
  PSYS_MGMT_LOCK_INFO pMgmtLock = (PSYS_MGMT_LOCK_INFO)psiValue;
  
  pMgmtLock->action = 0;
  pMgmtLock->type = 0;
    
  return PSX_STS_OK;
}

PSX_STATUS xmlEndMgmtLockObj(char *appName, char *objName)
{
  PSYS_MGMT_LOCK_INFO pMgmtLock = (PSYS_MGMT_LOCK_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndMgmtLockObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setMgmtLockInfo(pMgmtLock);

  free(psiValue);
  psiValue = NULL;

  return PSX_STS_OK;
}

PSX_STATUS xmlGetDftGwAttr(char *attrName, char* attrValue)
{
  PIFC_DEF_GW_INFO gw= (PIFC_DEF_GW_INFO)psiValue;
  int len = 0;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetDftGwAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"enblGwAutoAssign",strlen("enblGwAutoAssign")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK)
      gw->enblGwAutoAssign = xmlStateToPsi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDftGwAttr(): invalid state %s\n",attrValue);
    }
  }
  else if (strncmp(attrName,"ifName",strlen("ifName")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateInterfaceName(attrName);
    if (status == DB_OBJ_VALID_OK) {
      len = strlen(attrValue);
      strncpy(gw->ifName,attrValue,len);
      gw->ifName[len] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDftGwAttr(): invalid ifName %s\n",attrValue);
    }
  }
  else if (strncmp(attrName,"ipAddr",strlen("ipAddr")) == 0) {
    status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) 
      inet_aton((const char*)attrValue,&gw->defaultGateway);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDftGwAttr(): invalid ipAddr %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartDftGwObj(char *appName, char *objName)
{
  PIFC_DEF_GW_INFO gw;

#ifdef XML_DEBUG
  printf("xmlStartDftGwObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(IFC_DEF_GW_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(IFC_DEF_GW_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  gw = (PIFC_DEF_GW_INFO)psiValue;
  gw->enblGwAutoAssign = atoi(BcmDb_getDefaultValue("enblAuto"));
  gw->defaultGateway.s_addr = inet_addr(BcmDb_getDefaultValue(PSX_OBJ_DEF_GATEWAY));
  strcpy(gw->ifName, BcmDb_getDefaultValue("wanIfName"));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndDftGwObj(char *appName, char *objName)
{
  PIFC_DEF_GW_INFO gw= (PIFC_DEF_GW_INFO)psiValue;
  int error = 0;
  char addr[IFC_TINY_LEN];

#ifdef XML_DEBUG
  printf("xmlEndDftGwdObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (!gw->enblGwAutoAssign) {
      strncpy(addr,inet_ntoa(gw->defaultGateway),IFC_TINY_LEN);
      if ((gw->ifName[0] = '\0') && (strncmp(addr,"0.0.0.0",strlen("0.0.0.0")) == 0)) {
	xmlState.errorFlag = PSX_STS_ERR_FATAL;
	syslog(LOG_ERR,"xmlEndDftGwdObj(): autoAssign disabled, but ifName and ipAddr not specified.\n");
	error = 1;
      }
    }
  }
  if (!error && !xmlState.verify)
    BcmDb_setDefaultGatewayInfo(gw);

#ifdef XML_DEBUG1
  printf("\n============End system default gateway=========\n");
  printf("xmlEndDftGwObj(): psiValue->enblGwAutoAssign %d, defaultGateway %s, ifName %s\n",
	 gw->enblGwAutoAssign,inet_ntoa(gw->defaultGateway),gw->ifName);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetDnsAttr(char *attrName, char* attrValue)
{
  PIFC_DNS_INFO dns = (PIFC_DNS_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetDnsAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_DNS_DYNAMIC,strlen(PSX_ATTR_DNS_DYNAMIC)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsAttr(): invalid state %s\n",attrValue);
    }
    else
      dns->dynamic = xmlStateToPsi(attrValue);
  }
  else if (strncmp(attrName,PSX_ATTR_DNS_PRIMARY,strlen(PSX_ATTR_DNS_PRIMARY)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsAttr(): invalid primary DNS IP address %s\n",attrValue);
    }
    else {
      dns->preferredDns.s_addr = inet_addr((const char*)attrValue);
    }
  }
  else if (strncmp(attrName,PSX_ATTR_DNS_SECONDARY,strlen(PSX_ATTR_DNS_SECONDARY)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsAttr(): invalid secondary DNS IP address %s\n",attrValue);
    }
    else {
      dns->alternateDns.s_addr = inet_addr((const char*)attrValue);
    }
  }
  else if (strncmp(attrName,PSX_ATTR_DNS_DOMAIN,strlen(PSX_ATTR_DNS_DOMAIN)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,IFC_DOMAIN_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsAttr(): invalid domain name %s\n",attrValue);
    }
    else {
      strncpy(dns->domainName, (const char*)attrValue, IFC_DOMAIN_LEN);
    }
  }
  /* start: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
  else if (strncmp(attrName,PSX_ATTR_DNS_PROXY_ENABLE,strlen(PSX_ATTR_DNS_PROXY_ENABLE)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsAttr(): invalid lan dns proxy enable %s\n",attrValue);
    }
    else {
      dns->enbdnsProxy = xmlStateToPsi(attrValue);
    }
  }
  /* end: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */


  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartDnsObj(char *appName, char *objName)
{
  PIFC_DNS_INFO dns;

#ifdef XML_DEBUG
  printf("xmlStartDftGwObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(IFC_DNS_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(IFC_DNS_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  dns = (PIFC_DNS_INFO)psiValue;
  dns->dynamic = atoi(BcmDb_getDefaultValue("dnsDynamic"));
  dns->preferredDns.s_addr = inet_addr(BcmDb_getDefaultValue("dnsPrimary"));
  dns->alternateDns.s_addr = inet_addr(BcmDb_getDefaultValue("dnsSecondary"));
  strcpy(dns->domainName, BcmDb_getDefaultValue("dnsDomainName"));
  /* start: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
  dns->enbdnsProxy = atoi(BcmDb_getDefaultValue("enbdnsProxy"));  
  /* start: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndDnsObj(char *appName, char *objName)
{
  PIFC_DNS_INFO dns = (PIFC_DNS_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndDnsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  /* no mandatory attribute */
  if (!xmlState.verify)
    BcmDb_setDnsInfo(dns);

#ifdef XML_DEBUG1
  printf("\n============End system DNS Info=========\n");
  /* add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
  printf("xmlEndDnsObj(): dynamic %d, psiValue->primary %s, secondary %s, domain %s, enbdnsProxy %d\n",
	dns->dynamic,inet_ntoa(dns->preferredDns),inet_ntoa(dns->alternateDns),dns->domainName, dns->enbdnsProxy);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetSyslogAttr(char *attrName, char* attrValue)
{
  pBCM_SYSLOG_CB pSyslog=(pBCM_SYSLOG_CB)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetSyslogAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_STATE,strlen(PSX_ATTR_STATE)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid state %s\n",attrValue);
    }
    else
      pSyslog->status = xmlStateToPsi(attrValue);
  }
  /* start of maintain syslog sever 可以通过dhcp option7 by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
  else if (strncmp(attrName, "CfgServer",strlen("CfgServer")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid state %s\n",attrValue);
    }
    else
      pSyslog->CfgServerEnable = xmlStateToPsi(attrValue);
  }
#endif
  /* end of maintain syslog sever 可以通过dhcp option7 by xujunxia 43813 2006年6月29日 */
  else if (strncmp(attrName,PSX_ATTR_SYSLOG_DISPLAY_LEVEL,strlen(PSX_ATTR_SYSLOG_DISPLAY_LEVEL)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateSysLogLevel(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid displayLevel %s\n",attrValue);
    }
    else
      pSyslog->localDisplayLevel = xmlLogLevelToPsi(attrValue);
  }
  else if (strncmp(attrName,PSX_ATTR_SYSLOG_LOG_LEVEL,strlen(PSX_ATTR_SYSLOG_LOG_LEVEL)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateSysLogLevel(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid logLevel %s\n",attrValue);
    }
    else
      pSyslog->localLogLevel = xmlLogLevelToPsi(attrValue);
  }
  else if (strncmp(attrName,PSX_ATTR_SYSLOG_OPTION,strlen(PSX_ATTR_SYSLOG_OPTION)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateSysLogOption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid option %s\n",attrValue);
    }
    else
      pSyslog->option = xmlLogOptionToPsi(attrValue);
  }
  else if (strncmp(attrName,PSX_ATTR_SYSLOG_SERVER_IP,strlen(PSX_ATTR_SYSLOG_SERVER_IP)) == 0) {
    /* start of maintain 允许log服务器设置域名 by liuzhijie 00028714 2006年7月11日
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid serverIP %s\n",attrValue);
    }
    else
    */
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, 64);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid serverAddr %s\n",attrValue);
    }
    else
      strcpy(pSyslog->serverAddr, attrValue);
    /* end of maintain 允许log服务器设置域名 by liuzhijie 00028714 2006年7月11日 */

  }
  else if (strncmp(attrName,PSX_ATTR_SYSLOG_SERVER_PORT,strlen(PSX_ATTR_SYSLOG_SERVER_PORT)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,XML_PORT_RANGE_MIN,XML_PORT_RANGE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid serverPort %s\n",attrValue);
    }
    else
      pSyslog->serverPort = atoi(attrValue);
  }
  /*add by z67625 增加获取防火墙日志写flash开关参数 start*/
  else if (strncmp(attrName, PSX_ATTR_SYSLOG_FWLOG, strlen(PSX_ATTR_SYSLOG_FWLOG)) == 0)
  {
      if (xmlState.verify)
          status = BcmDb_validateState(attrValue);
      if (status != DB_OBJ_VALID_OK) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid state %s\n",attrValue);
    }
    else
      pSyslog->fwlog = xmlStateToPsi(attrValue);
  }
  /*add by z67625 增加获取防火墙日志写flash开关参数 end*/

  /*start of 增加获取系统日志保存FLASH开关参数by c00131380 at 080926*/
  else if (strncmp(attrName, PSX_ATTR_SYSLOG_SAVE, strlen(PSX_ATTR_SYSLOG_SAVE)) == 0)
  {
      if (xmlState.verify)
          status = BcmDb_validateState(attrValue);
      if (status != DB_OBJ_VALID_OK) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSyslogAttr(): invalid state %s\n",attrValue);
    }
    else
      pSyslog->Syslog= xmlStateToPsi(attrValue);
  }
  /*end of 增加获取系统日志保存FLASH开关参数by c00131380 at 080926*/
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSyslogObj(char *appName, char *objName)
{
  pBCM_SYSLOG_CB pSyslog;

#ifdef XML_DEBUG
  printf("xmlStartSyslogObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(BCM_SYSLOG_CB));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(BCM_SYSLOG_CB));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  pSyslog = (pBCM_SYSLOG_CB)psiValue;
  pSyslog->status = atoi(BcmDb_getDefaultValue("logStatus"));
  pSyslog->option = atoi(BcmDb_getDefaultValue("logMode"));
  pSyslog->serverPort = atoi(BcmDb_getDefaultValue("logPort"));
  pSyslog->localDisplayLevel = atoi(BcmDb_getDefaultValue("logDisplay"));
  pSyslog->localLogLevel = atoi(BcmDb_getDefaultValue(PSX_ATTR_SYSLOG_LOG_LEVEL));
  pSyslog->remoteLogLevel = atoi(BcmDb_getDefaultValue(PSX_ATTR_SYSLOG_LOG_LEVEL));
  /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
  pSyslog->serverIP.s_addr = inet_addr(BcmDb_getDefaultValue("logIpAddress"));
  */
  strcpy(pSyslog->serverAddr, BcmDb_getDefaultValue("logIpAddress"));
  /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */

  /*add by z67625 增加支持防火期日志写flash开关参数初始化 start*/
  pSyslog->fwlog = atoi(BcmDb_getDefaultValue("fwlog"));
  /*add by z67625 增加支持防火期日志写flash开关参数初始化 end*/
  
  /*start of 增加系统日志保存FLASH开关参数初始化by c00131380 at 080926*/
  pSyslog->Syslog = atoi(BcmDb_getDefaultValue("Syslog"));
  /*end of 增加系统日志保存FLASH开关参数初始化by c00131380 at 080926*/  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSyslogObj(char *appName, char *objName)
{
  pBCM_SYSLOG_CB pSyslog = (pBCM_SYSLOG_CB)psiValue;
  int error = 0;
  struct in_addr ipAddr;
    
#ifdef XML_DEBUG
  printf("xmlEndSyslogObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (xmlState.verify) {
    /* check to see if all mandatory object attribute are set */  
    if (((pSyslog->option & BCM_SYSLOG_LOCAL_REMOTE_MASK) == BCM_SYSLOG_LOCAL_REMOTE) ||
        ((pSyslog->option & BCM_SYSLOG_REMOTE_MASK) == BCM_SYSLOG_REMOTE)) {
      /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
      inet_aton("0.0.0.0", &ipAddr);
      if ((pSyslog->serverPort == 0) ||
          (memcmp(&(pSyslog->serverIP), &(ipAddr), sizeof(pSyslog->serverIP)) == 0)) {
      */
#ifdef SUPPORT_SYSLOG_AUTOCONF
      if ((pSyslog->CfgServerEnable) && ((pSyslog->serverPort == 0) ||
#else
      if (((pSyslog->serverPort == 0) ||
#endif
          (memcmp(pSyslog->serverAddr, "0.0.0.0", sizeof(pSyslog->serverAddr)) == 0))) {
      /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlEndSyslogObj(): serverIP and serverPort must be defined for remote log.\n");
        error = 1;
      }
    }
  }
  
  if (!error && !xmlState.verify)
    BcmDb_setSysLogInfo(pSyslog);

#ifdef XML_DEBUG1
  printf("\n============End system syslog info=========\n");
  /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
  printf("xmlEndSyslogObj(): psiValue->status %d, option %d, serverPort %d, displayLevel %d, logLevel %d, remoteLogLevel %d, serverIP %s\n",
	 pSyslog->status,pSyslog->option,pSyslog->serverPort,pSyslog->localDisplayLevel,
         pSyslog->localLogLevel,pSyslog->remoteLogLevel,inet_ntoa(pSyslog->serverIP));
  */
  /*modified by z67625 增加防火墙日志开关值输出 start*/
  printf("xmlEndSyslogObj(): psiValue->status %d, option %d, serverPort %d, displayLevel %d, logLevel %d, remoteLogLevel %d, serverIP %s fwlog %d Syslog %d\n",
	 pSyslog->status,pSyslog->option,pSyslog->serverPort,pSyslog->localDisplayLevel,
         pSyslog->localLogLevel,pSyslog->remoteLogLevel,pSyslog->serverAddr,pSyslog->fwlog,pSyslog->Syslog);
  /*modified by z67625 增加防火墙日志开关值输出 end*/
  /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#ifdef SUPPORT_SNMP
PSX_STATUS xmlGetSnmpAttr(char *attrName, char* attrValue)
{
  PBCM_SNMP_STRUCT snmp = (PBCM_SNMP_STRUCT)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;
  int input;

#ifdef XML_DEBUG
  printf("xmlGetSnmpAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_STATE,strlen(PSX_ATTR_STATE)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid state %s\n",attrValue);
    }
    else
      snmp->status = xmlStateToPsi(attrValue);
  }
  else if (strncmp(attrName,"readCommunity",strlen("readCommunity")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,XML_MAX_SNMP_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid readCommunity %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(snmp->roCommunity,attrValue,len);
      snmp->roCommunity[len] = '\0';
    }
  }
  else if (strncmp(attrName,"writeCommunity",strlen("writeCommunity")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,XML_MAX_SNMP_STR_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid writeCommunity %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(snmp->rwCommunity,attrValue,len);
      snmp->rwCommunity[len] = '\0';
    }
  }
  else if (strncmp(attrName,"sysName",strlen("sysName")) == 0) {
    if (xmlState.verify)
      /*start of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717
      status = BcmDb_validateLength(attrValue,XML_MAX_SNMP_STR_LEN);
      */
      status = BcmDb_validateLength(attrValue,IFC_STRING_LEN);
      /*end of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717  */
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid sysName %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(snmp->sysName,attrValue,len);
      snmp->sysName[len] = '\0';
    }
  }
  else if (strncmp(attrName,"sysLocation",strlen("sysLocation")) == 0) {
    if (xmlState.verify)
      /*start of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717
      status = BcmDb_validateLength(attrValue,XML_MAX_SNMP_STR_LEN);
      */
      status = BcmDb_validateLength(attrValue,IFC_STRING_LEN);
      /*end of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717  */
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid sysLocation %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(snmp->sysLocation,attrValue,len);
      snmp->sysLocation[len] = '\0';
    }
  }
  else if (strncmp(attrName,"sysContact",strlen("sysContact")) == 0) {
    if (xmlState.verify)
      /*start of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717
      status = BcmDb_validateLength(attrValue,XML_MAX_SNMP_STR_LEN);
      */
      status = BcmDb_validateLength(attrValue,IFC_STRING_LEN);
      /*end of 增加对snmp sysname字符 长度 的 限制  by chenyu  at  20080717  */
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid sysContact %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(snmp->sysContact,attrValue,len);
      snmp->sysContact[len] = '\0';
    }
  }
  else if (strncmp(attrName,"trapIP",strlen("trapIP")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) 
      inet_aton((const char*)attrValue,&snmp->trapIp);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSnmpAttr(): invalid trapIP %s\n",attrValue);
    }
  }
  else if (strncmp(attrName,"debug",strlen("debug")) == 0) {
    input = strtol(attrValue,NULL,16);
    memcpy(&snmp->debug,&input,sizeof(snmp->debug));
  }
  else if (strncmp(attrName,"configId",strlen("configId")) == 0) {
    input = strtol(attrValue,NULL,16);
    /*this is not a configurable parameter though*/
    snmp->configId[0] = (char)(input >> 24);
    snmp->configId[1] = (char)(input >> 16);
    snmp->configId[2] = (char)(input >> 8);
    snmp->configId[3] = (char)(input);
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSnmpObj(char *appName, char *objName)
{
  PBCM_SNMP_STRUCT snmp;

#ifdef XML_DEBUG
  printf("xmlStartSnmpObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(BCM_SNMP_STRUCT));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(BCM_SNMP_STRUCT));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  snmp = (PBCM_SNMP_STRUCT)psiValue;
  snmp->status = atoi(BcmDb_getDefaultValue("snmpStatus"));
  strcpy(snmp->roCommunity, BcmDb_getDefaultValue("snmpRoCommunity"));
  strcpy(snmp->rwCommunity, BcmDb_getDefaultValue("snmpRwCommunity"));
  strcpy(snmp->sysName, BcmDb_getDefaultValue("snmpSysName"));
  strcpy(snmp->sysLocation, BcmDb_getDefaultValue("snmpSysLocation"));
  strcpy(snmp->sysContact, BcmDb_getDefaultValue("snmpSysContact"));
  snmp->trapIp.s_addr = inet_addr(BcmDb_getDefaultValue("snmpTrapIp"));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSnmpObj(char *appName, char *objName)
{
  PBCM_SNMP_STRUCT snmp = (PBCM_SNMP_STRUCT)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndSnmpObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setSnmpInfo(snmp);

#ifdef XML_DEBUG1
  printf("\n============End system snmp info=========\n");
  printf("xmlEndSnmpObj(): psiValue->status %d, readCommunity %s, wCommunity %s, sysName %s, sysContatct %s, sysLocation %s, trapIP %s\n",
         snmp->status,snmp->roCommunity,snmp->rwCommunity,snmp->sysName,snmp->sysContact,
         snmp->sysLocation, inet_ntoa(snmp->trapIp));
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
#endif /* SUPPORT_SNMP */

PSX_STATUS xmlGetOtherSysAttr(char *attrName, char* attrValue)
{
  PSYS_FLAG_INFO flag = (PSYS_FLAG_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetOtherSysAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (xmlState.verify) {
    if ( strcmp(attrName,"macFilterPolicy") != 0 )
       status = BcmDb_validateState(attrValue);
    else
       status = BcmDb_validateMacFilterPolicy(attrValue);
  }

  if (status != DB_OBJ_VALID_OK) 
    xmlState.errorFlag = PSX_STS_ERR_FATAL;

  if (strncmp(attrName,"autoScan",strlen("autoScan")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid autoScan state %s\n",attrValue);
    else
      flag->autoScan = xmlStateToPsi(attrValue);
  }
  if (strncmp(attrName,"macFilterPolicy",strlen("macFilterPolicy")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid macFilterPolicy state %s\n",attrValue);
    else
      flag->macPolicy = xmlPolicyToPsi(attrValue);
  }
  if (strncmp(attrName,"encodePassword",strlen("encodePassword")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid encodePassword state %s\n",attrValue);
    else
      flag->encodePassword = xmlStateToPsi(attrValue);
  }
#if SUPPORT_ETHWAN
  if (strncmp(attrName,"enetwan",strlen("enetwan")) == 0) {
    if (status != DB_OBJ_VALID_OK)
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid Enet WAN Config state %s\n",attrValue);
    else
      flag->enetWan = xmlStateToPsi(attrValue);
  }
#endif
#ifdef SUPPORT_UPNP
  if (strncmp(attrName,"upnp",strlen("upnp")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid upnp state %s\n",attrValue);
    else
      flag->upnp = xmlStateToPsi(attrValue);
  }
#endif /* SUPPORT_UPNP */
#ifdef SUPPORT_SIPROXD
  if (strncmp(attrName,"siproxd",strlen("siproxd")) == 0) {
    if (status != DB_OBJ_VALID_OK)      
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid siproxd state %s\n",attrValue);
    else
      flag->siproxd = xmlStateToPsi(attrValue);
  }
#endif /* SUPPORT_SIPROXD */
  if (strncmp(attrName,"igmpSnp",strlen("igmpSnp")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid igmpSnp state %s\n",attrValue);
    else
      flag->igmpSnp = xmlStateToPsi(attrValue);
  }
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if 0
  if (strncmp(attrName,"igmpMode",strlen("igmpMode")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid igmpMode state %s\n",attrValue);
    else
      flag->igmpMode = xmlStateToPsi(attrValue);
  }
 #endif 0
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
 /*start of增加ddns使能功能by l129990,2009,3,4*/
 if (strncmp(attrName,"ddns",strlen("ddns")) == 0) {
    if (status != DB_OBJ_VALID_OK) 
      syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid ddns state %s\n",attrValue);
    else
      flag->ddns = xmlStateToPsi(attrValue);
  }
/*end of增加ddns使能功能by l129990,2009,3,4*/
  /*begin add by p44980 2008.10.22*/
   if (strncmp(attrName,"switchMode",strlen("switchMode")) == 0) {
       if (status != DB_OBJ_VALID_OK) 
       {
            syslog(LOG_ERR,"xmlGetOtherSysAttr(): invalid igmpMode state %s\n",attrValue);
            printf("xmlGetOtherSysAttr(): invalid switchMode state %s\n",attrValue);
       }
       else
       {
            flag->connSwitchMode = xmlStateToPsi(attrValue);
       }
  }
  /*end add by p44980 2008.10.22*/
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartOtherSysObj(char *appName, char *objName)
{
  PSYS_FLAG_INFO flag;

#ifdef XML_DEBUG
  printf("xmlStartOtherSysObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SYS_FLAG_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(SYS_FLAG_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  flag = (PSYS_FLAG_INFO)psiValue;
  flag->autoScan = atoi(BcmDb_getDefaultValue("enblAutoScan"));
  flag->upnp = atoi(BcmDb_getDefaultValue("enblUpnp"));
  flag->igmpSnp = atoi(BcmDb_getDefaultValue("enblIgmpSnp"));
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
  //flag->igmpMode = atoi(BcmDb_getDefaultValue("enblIgmpMode"));
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
  flag->macPolicy = atoi(BcmDb_getDefaultValue("macPolicy"));
  flag->encodePassword = FALSE;
  flag->siproxd = atoi(BcmDb_getDefaultValue("enblSiproxd"));
#if SUPPORT_ETHWAN
  flag->enetWan = atoi(BcmDb_getDefaultValue("enblEnetWan"));
#endif
//add by l66195 for VDF start  
/*start of增加ddns使能功能by l129990,2009,3,4*/
  flag->ddns = atoi(BcmDb_getDefaultValue("enblDdns"));
/*end of增加ddns使能功能by l129990,2009,3,4*/
//add by l66195 for VDF end
/*begin add by p44980 2008.10.24*/
flag->connSwitchMode = atoi(BcmDb_getDefaultValue("connSwitchMode"));
/*end add by p44980 2008.10.24*/
  return PSX_STS_OK;
}

PSX_STATUS xmlEndOtherSysObj(char *appName, char *objName)
{
  PSYS_FLAG_INFO flag = (PSYS_FLAG_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndOtherSysObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setSysFlagInfo(flag);

#ifdef XML_DEBUG1
  printf("\n============End system flag info=========\n");
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if 0
#if SUPPORT_ETHWAN
  printf("xmlEndOtherSysObj(): autoScan %d, upnp %d, igmpSnp %d, igmpMode %d, macFilterPolicy %d, siproxd %d enetwan %d\n",
         flag->autoScan,flag->upnp,flag->igmpSnp,flag->igmpMode, flag->macPolicy,flag->siproxd, flag->enetWan);
#else
  printf("xmlEndOtherSysObj(): autoScan %d, upnp %d, igmpSnp %d, igmpMode %d, macFilterPolicy %d, siproxd %d\n",
         flag->autoScan,flag->upnp,flag->igmpSnp,flag->igmpMode, flag->macPolicy,flag->siproxd);
#endif
#endif
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/

/*start of 重新设置snooping启动接口by l129990 2008,9,9*/
#if SUPPORT_ETHWAN
  printf("xmlEndOtherSysObj(): autoScan %d, upnp %d, igmpSnp %d,  macFilterPolicy %d, siproxd %d enetwan %d\n",
         flag->autoScan,flag->upnp,flag->igmpSnp, flag->macPolicy,flag->siproxd, flag->enetWan);
#else
  printf("xmlEndOtherSysObj(): autoScan %d, upnp %d, igmpSnp %d, macFilterPolicy %d, siproxd %d\n",
         flag->autoScan,flag->upnp,flag->igmpSnp, flag->macPolicy,flag->siproxd);
#endif
/*end of 重新设置snooping启动接口by l129990 2008,9,9*/
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetAtmVccAttr(char *attrName, char* attrValue)
{
  PIFC_ATM_VCC_INFO vcc = (PIFC_ATM_VCC_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetAtmVccAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strncmp(attrName,"vpi",strlen("vpi")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmVpi(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid vpi %s\n",attrValue);
    }
    else
      vcc->vccAddr.usVpi = atoi(attrValue);
  }
  if (strncmp(attrName,"vci",strlen("vci")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmVci(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid vci %s\n",attrValue);
    }
    else
      vcc->vccAddr.usVci = atoi(attrValue);
  }
  if (strncmp(attrName,"tdId",strlen("tdId")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid tdId %s\n",attrValue);
    }
    else
      vcc->tdId = atoi(attrValue);
  }
  if (strncmp(attrName,"aalType",strlen("aalType")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmAalType(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid aalType %s\n",attrValue);
    }
    else
      vcc->aalType = xmlAalTypeToPsi(attrValue);
  }
  if (strncmp(attrName,"adminStatus",strlen("adminStatus")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmAdminStatus(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid adminStatus %s\n",attrValue);
    }
    else
      vcc->adminStatus = xmlAtmAdminStatusToPsi(attrValue);
  }
  if (strncmp(attrName,"encap",strlen("encap")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmEncapsulation(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid encap %s\n",attrValue);
    }
    else
      vcc->encapMode = xmlEncapToPsi(attrValue);
  }
  if (strncmp(attrName,"qos",strlen("qos")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid qos state %s\n",attrValue);
    }
    else
      vcc->enblQos = xmlStateToPsi(attrValue);
  }
  /* start of protocol QoS for TdE获取配置属性 by z45221 zhangchen 2006年8月14日 */
#ifdef SUPPORT_TDE_QOS
  if (strncmp(attrName,"QosIfcRate",strlen("QosIfcRate")) == 0) {
    if (xmlState.verify) 
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid QosIfcRate %s\n",attrValue);
    }
    else
    {
      vcc->QosIfcRate = atoi(attrValue);
    }
  }

  if (strncmp(attrName,"QosIfcCeil",strlen("QosIfcCeil")) == 0) {
    if (xmlState.verify) 
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmVccAttr(): invalid QosIfcCeil %s\n",attrValue);
    }
    else
    {
      vcc->QosIfcCeil= atoi(attrValue);
    }
  }
#endif
  /* end of protocol QoS for TdE获取配置属性 by z45221 zhangchen 2006年8月14日 */
  if (strncmp(attrName,"instanceId",strlen("instanceId")) == 0) {
      vcc->id = atoi(attrValue);
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartAtmVccObjNode(char *appName, char *objName)
{
  PIFC_ATM_VCC_INFO vcc;

#ifdef XML_DEBUG
  printf("xmlStartAtmVccObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(IFC_ATM_VCC_INFO));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(IFC_ATM_VCC_INFO));
  vcc = (PIFC_ATM_VCC_INFO)psiValue;
  vcc->tdId = 1; /* a default UBR TD entry is always created */
  vcc->adminStatus = xmlAtmAdminStatusToPsi("up");
  vcc->aalType = xmlAalTypeToPsi("AAL5");
  vcc->encapMode = xmlEncapToPsi("other");
  vcc->vccAddr.ulInterfaceId = 1001;
  /* start of protocol QoS for TdE by z45221 zhangchen 2006年8月14日 */
#ifdef SUPPORT_TDE_QOS
  vcc->QosIfcRate = 0;
  vcc->QosIfcRate = 0;
#endif
  /* end of protocol QoS for TdE by z45221 zhangchen 2006年8月14日 */
  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndAtmVccObjNode(char *appName, char *objName)
{
  PIFC_ATM_VCC_INFO vcc = (PIFC_ATM_VCC_INFO)psiValue;
  UINT16 objectId;

#ifdef XML_DEBUG
  printf("xmlEndAtmVccObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  if (xmlState.verify) {
    if (vcc->vccAddr.usVci == 0) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndAtmVccObj(): invalid vci %d\n",vcc->vccAddr.usVci);
    }
  }
  else {
    objectId = atoi(objName+strlen("vccId")); /* extracting objectId from objName */
    BcmPsi_objStore(BcmPsi_appOpen(ATM_VCC_APPID), objectId, vcc, sizeof(IFC_ATM_VCC_INFO));
  }

#ifdef XML_DEBUG1
  printf("\n============End ATM VCC info=========\n");
  /* start of protocol QoS for TdE by z45221 zhangchen 2006年8月14日
  printf("xmlEndAtmVccObj(): vpi %d, vci %d, tdId %d, aalType %d, adminStatus %d, encap %d, qos %d\n",
         vcc->vccAddr.usVpi,vcc->vccAddr.usVci,vcc->tdId,(int)vcc->aalType,(int)vcc->adminStatus,
         (int)(vcc->encapMode),(int)(vcc->enblQos));
  */
#ifndef SUPPORT_TDE_QOS
  printf("xmlEndAtmVccObj(): vpi %d, vci %d, tdId %d, aalType %d, adminStatus %d, encap %d, qos %d\n",
         vcc->vccAddr.usVpi,vcc->vccAddr.usVci,vcc->tdId,(int)vcc->aalType,(int)vcc->adminStatus,
         (int)(vcc->encapMode),(int)(vcc->enblQos));
#else
  printf("xmlEndAtmVccObj(): vpi %d, vci %d, tdId %d, aalType %d, adminStatus %d, encap %d, qos %d, QosIfcRate %lu QosIfcCeil %lu\n",
         vcc->vccAddr.usVpi,vcc->vccAddr.usVci,vcc->tdId,(int)vcc->aalType,(int)vcc->adminStatus,
         (int)(vcc->encapMode),(int)(vcc->enblQos), vcc->QosIfcRate, vcc->QosIfcCeil);
#endif
  /* end of protocol QoS for TdE by z45221 zhangchen 2006年8月14日 */
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetAtmVccObjNode(char *appName, char *objName, char *attrName,
                            char* attrValue)
{
  PSX_STATUS sts;

#ifdef XML_DEBUG
  printf("xmlGetAtmVccObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif
  strcpy(app_name, appName);

  sts = xmlGetAtmVccAttr(attrName, attrValue);
  return sts;
} /* xmlGetAtmVccObjNode */

//=====================================================================================
//=============================== Start SecObj Functions ==============================
//=====================================================================================

PSX_STATUS xmlGetSecObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetSecObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 1; secObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(secObjs[i].objName, objName) == 0) {
      if (*(secObjs[i].getAttrFnc) != NULL) {
	sts = (*(secObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetSecObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} /* xmlGetSecObjNode */

PSX_STATUS xmlStartSecObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartSecObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; secObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(secObjs[i].objName, objName) == 0) {
      if (*(secObjs[i].startObjFnc) != NULL) {
        sts = (*(secObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartSecObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndSecObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndSecObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; secObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(secObjs[i].objName, objName) == 0) {
      if (*(secObjs[i].endObjFnc) != NULL) {
	sts = (*(secObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndSecObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetVrtSrvAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVrtSrvAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_VRT_SRV_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
      memset(psiValue,0,(sizeof(SEC_VRT_SRV_ENTRY) * xmlState.tableSize));
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVrtSrvObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndVrtSrvObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVrtSrvObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setVirtualServerSize(xmlState.tableSize);
    BcmDb_setVirtualServerInfo((PSEC_VRT_SRV_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End virtual server=========\n");
  printf("xmlEndVrtSrvObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetVrtSrvEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_VRT_SRV_ENTRY vrtSrvTbl = (PSEC_VRT_SRV_ENTRY)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetVrtSrvEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetVrtSrvEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"instanceId") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].id = strtoul(attrValue, (char **)NULL, 10);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid instance ID %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"name") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SEC_BUFF_SRVNAME_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(vrtSrvTbl[i].srvName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid length %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"addr") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].addr.s_addr = inet_addr(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_OBJ_PROTOCOL) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpProtocol(attrValue);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].protocol = BcmDb_getIpProtocol(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid protocol %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"eStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].eStart = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid external start port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"eEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].eEnd = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid external end port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"iStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].iStart = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid internal start port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"iEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      vrtSrvTbl[i].iEnd = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVrtSrvAttr(): invalid internal end port %s\n",attrValue);
    }
  }
  //add by l66195 for VDF start
  else if (strcmp(attrName,"bEnable") == 0) {
      vrtSrvTbl[i].bEnable = atoi(attrValue);
  } 
  //add by l66195 for VDF end
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVrtSrvEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PSEC_VRT_SRV_ENTRY vrtSrvTbl = (PSEC_VRT_SRV_ENTRY)psiValue;
  
  /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndVrtSrvEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
  if (xmlState.verify) {
    if ( vrtSrvTbl[i].eStart > vrtSrvTbl[i].eEnd ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVrtSrvEntryObj(): invalid external port range [%d - %d]\n",
             vrtSrvTbl[i].eStart, vrtSrvTbl[i].eEnd);
    }
    else if ( vrtSrvTbl[i].iStart > vrtSrvTbl[i].iEnd ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVrtSrvEntryObj(): invalid internal port range [%d - %d]\n",
           vrtSrvTbl[i].iStart, vrtSrvTbl[i].iEnd);
    }
  }
  if ( vrtSrvTbl[i].id == 0 )
    vrtSrvTbl[i].id = i+1;
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetIpFilterAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetIpFilterAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_FLT_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
        return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_FLT_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), SEC_VIR_SRV_MAX);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndIpFilterObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndIpFilterObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndIpFilterObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    if ( strcmp(objName, TBLNAME_OUT_FILTER) == 0 ) {
      BcmDb_setOutgoingFilterSize(xmlState.tableSize);
      BcmDb_setOutgoingFilterInfo((PSEC_FLT_ENTRY)psiValue, xmlState.tableSize);
    } else if ( strcmp(objName, TBLNAME_IN_FILTER) == 0 ) {
      BcmDb_setIncomingFilterSize(xmlState.tableSize);
      BcmDb_setIncomingFilterInfo((PSEC_FLT_ENTRY)psiValue, xmlState.tableSize);
    }
  }

#ifdef XML_DEBUG1
  printf("\n============End IP fileter=========\n");
  printf("xmlEndIpFilterObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetIpFilterEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_FLT_ENTRY ipFltTbl = (PSEC_FLT_ENTRY)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetIpFilterEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"fltName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
    if (status == DB_OBJ_VALID_OK && attrValue[0] != '\0')
      strcpy(ipFltTbl[i].fltName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid filter name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_OBJ_PROTOCOL) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpProtocol(attrValue);
    if (status == DB_OBJ_VALID_OK)
      ipFltTbl[i].protocol = BcmDb_getIpProtocol(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid protocol %s\n",attrValue);
    }

  }
  else if (strcmp(attrName,"srcAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].srcAddr,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid source IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateSubnetMask(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].srcMask,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid source subnet mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcPort") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].srcPort,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid srcPort %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].dstAddr,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid destination IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateSubnetMask(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].dstMask,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid destination subnet mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstPort") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ipFltTbl[i].dstPort,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid destination port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"wanIf") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK && attrValue[0] != '\0')
      strcpy(ipFltTbl[i].wanIf,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIpFilterEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndIpFilterEntryObj(char *appName, char *objName)
{
  if (xmlState.verify) {
    int i = xmlState.tableIndex;
    PSEC_FLT_ENTRY fltTbl = (PSEC_FLT_ENTRY)psiValue;

    if ( fltTbl == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        xmlState.tableIndex = 0;
        return (PSX_STS_ERR_FATAL);
    }

    if ( fltTbl[i].srcAddr[0] == '\0' && fltTbl[i].srcMask[0] == '\0' &&
         fltTbl[i].dstAddr[0] == '\0' && fltTbl[i].dstMask[0] == '\0' &&
         fltTbl[i].srcPort[0] == '\0' && fltTbl[i].dstPort[0] == '\0' &&
         fltTbl[i].protocol == SEC_PROTO_MAX ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndIpFilterEntryObj(): At least one condition must be specified.\n");
    }      
    else if ( fltTbl[i].srcAddr[0] == '\0' && fltTbl[i].srcMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndIpFilterEntryObj(): Cannot have source subnet mask without source IP address.\n");
    }
    else if ( fltTbl[i].dstAddr[0] == '\0' && fltTbl[i].dstMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndIpFilterEntryObj(): Cannot have destination subnet mask without destination IP address.\n");
    }
    else if ( (fltTbl[i].srcPort[0] != '\0' || fltTbl[i].dstPort[0] != '\0') && 
        /*modified by z67625 IP过滤增加支持IGMP协议 start*/
              (fltTbl[i].protocol == SEC_PROTO_MAX || fltTbl[i].protocol == SEC_PROTO_ICMP|| fltTbl[i].protocol == SEC_PROTO_IGMPV) ) {
        /*modified by z67625 IP过滤增加支持IGMP协议 end*/
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndIpFilterEntryObj(): TCP or UDP Protocol must be selected together with port number.\n");
    }       
  }

  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetPrtTriggerAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetPrtTriggerAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_PTR_TRG_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_PRT_TRG_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_PRT_TRG_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPrtTriggerObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndPrtTriggerObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPrtTriggerObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
      BcmDb_setPortTriggerSize(xmlState.tableSize);
      BcmDb_setPortTriggerInfo((PSEC_PRT_TRG_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Port Trigger=========\n");
  printf("xmlEndPrtTriggerObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetPrtTriggerEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_PRT_TRG_ENTRY prtTrgTbl = (PSEC_PRT_TRG_ENTRY)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetPrtTriggerEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"appName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(prtTrgTbl[i].appName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid app name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"triggerProtocol") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpProtocol(attrValue);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].flag.tProto = BcmDb_getIpProtocol(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid trigger protocol %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"openProtocol") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpProtocol(attrValue);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].flag.oProto = BcmDb_getIpProtocol(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid open protocol %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"triggerStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].tStart = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid trigger port range [start] %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"triggerEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].tEnd = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid trigger port range [end] %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"openStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].oStart = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid open port range [start] %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"openEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      prtTrgTbl[i].oEnd = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPrtTriggerEntryAttr(): invalid open port range [end] %s\n",attrValue);
    }
  }
  //add by l66195 for VDF start
  else if (strcmp(attrName,"bEnable") == 0) {
      prtTrgTbl[i].bEnable = atoi(attrValue);
  } 
  //add by l66195 for VDF end
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPrtTriggerEntryObj(char *appName, char *objName)
{
  /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndPrtTriggerEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
  
  if (xmlState.verify) {
    int i = xmlState.tableIndex;
    PSEC_PRT_TRG_ENTRY prtTrgTbl = (PSEC_PRT_TRG_ENTRY)psiValue;
    if ( prtTrgTbl[i].tStart > prtTrgTbl[i].tEnd ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPrtTriggerEntryObj(): invalid trigger port range [%d - %d]\n",
             prtTrgTbl[i].tStart, prtTrgTbl[i].tEnd);
    }
    else if ( prtTrgTbl[i].oStart > prtTrgTbl[i].oEnd ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPrtTriggerEntryObj(): invalid open port range [%d - %d]\n",
             prtTrgTbl[i].oStart, prtTrgTbl[i].oEnd);
    }
  }

  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetMacFilterAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetMacFilterAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_PTR_TRG_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_MAC_FLT_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_MAC_FLT_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size should not exceed %d\n",
              atoi(attrValue), SEC_PTR_TRG_MAX);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacFilterObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndMacFilterObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndMacFilterObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
      BcmDb_setMacFilterSize(xmlState.tableSize);
      BcmDb_setMacFilterInfo((PSEC_MAC_FLT_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End MAC filter=========\n");
  printf("xmlEndMacFilterObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetMacFilterEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_MAC_FLT_ENTRY macFltTbl = (PSEC_MAC_FLT_ENTRY)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetMacFilterEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"wanIf") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(macFltTbl[i].wanIf, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid wan interface %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_OBJ_PROTOCOL) == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacProtocol(attrValue);
    if (status == DB_OBJ_VALID_OK)
      macFltTbl[i].flag.protocol = BcmDb_getMacProtocol(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid MAC filter protocol %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"direction") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateMacFilterDirection(attrValue);
    if (status == DB_OBJ_VALID_OK)
      macFltTbl[i].flag.direction = BcmDb_getMacFilterDirection(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid MAC filter direction %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"destMac") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(macFltTbl[i].destMac, attrValue);
      else
        strcpy(macFltTbl[i].destMac, WEB_SPACE);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid destination MAC address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcMac") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(macFltTbl[i].srcMac, attrValue);
      else
        strcpy(macFltTbl[i].srcMac, WEB_SPACE);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetMacFilterEntryAttr(): invalid source MAC address %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacFilterEntryObj(char *appName, char *objName)
{
  if (xmlState.verify) {
    int i = xmlState.tableIndex;
    PSEC_MAC_FLT_ENTRY macFltTbl = (PSEC_MAC_FLT_ENTRY)psiValue;
    if (macFltTbl == NULL) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        xmlState.tableIndex = 0;
        return (PSX_STS_ERR_FATAL);
    }
    if ( macFltTbl[i].flag.protocol == SEC_PROTO_NONE && 
         strcmp(macFltTbl[i].destMac, WEB_SPACE) == 0 &&
         strcmp(macFltTbl[i].srcMac, WEB_SPACE) == 0 ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndMacFilterEntryObj(): At least one condition must be specified\n");
    }
  }

  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetQosClsAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetQosClsAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(QOS_CLASS_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(QOS_CLASS_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), SEC_VIR_SRV_MAX);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndQosClsObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndQosClsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setQosClassSize(xmlState.tableSize);
    BcmDb_setQosClassInfo((PQOS_CLASS_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End QoS Class=========\n");
  printf("xmlEndQosClsObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlStartQosClsEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PQOS_CLASS_ENTRY qosClsTbl = (PQOS_CLASS_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlStartQosClsEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }

  memset((char*)(qosClsTbl+i), 0, sizeof(QOS_CLASS_ENTRY));
  qosClsTbl[i].type = QOS_TYPE_DIFFSERV;
  qosClsTbl[i].wanVlan8021p = QOS_8021P_NONE;
  qosClsTbl[i].precedence = PRECEDENCE_NONE;
  qosClsTbl[i].tos = TOS_NONE;
  qosClsTbl[i].protocol = QOS_PROTO_NONE;
  qosClsTbl[i].vlan8021p = QOS_8021P_NONE;
  strcpy(qosClsTbl[i].dstMacAddr, WEB_SPACE);
  strcpy(qosClsTbl[i].dstMacMask, WEB_SPACE);
  strcpy(qosClsTbl[i].srcMacAddr, WEB_SPACE);
  strcpy(qosClsTbl[i].srcMacMask, WEB_SPACE);
  return PSX_STS_OK;
}

PSX_STATUS xmlGetQosClsEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PQOS_CLASS_ENTRY qosClsTbl = (PQOS_CLASS_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetQosClsEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"type") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosClsTbl[i].type = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid type %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"clsName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
    if (status == DB_OBJ_VALID_OK 
    	/*start modifying for vdf qos by p44980 2008.01.11*/
      #ifndef SUPPORT_VDF_QOS
    	&& attrValue[0] != '\0'
    	#endif
    	/*start modifying for vdf qos by p44980 2008.01.11*/
    	)
      strcpy(qosClsTbl[i].clsName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid class name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"protocol") == 0) {

    /*start modifying for vdf qos by p44980 2008.03.14*/
    //#ifndef SUPPORT_VDF_QOS
    #ifdef SUPPORT_VDF_QOS /*w00135358 modify for VDF GLOBAL QOS 20080910*/
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpProtocol(attrValue);

    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].protocol = BcmDb_getIpProtocol(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid protocol %s\n",attrValue);
    }
    #else
    qosClsTbl[i].protocol = atoi(attrValue);
    #endif
    /*end modifying for vdf qos by p44980 2008.03.14*/
  }
  else if (strcmp(attrName,"priority") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateQosPriority(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosClsTbl[i].priority = BcmDb_getQosClassPriority(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid priority %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"wlpriority") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosClsTbl[i].wlpriority = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid wlpriority %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"tos") == 0) {
    qosClsTbl[i].tos = atoi(attrValue);
  }
  else if (strcmp(attrName,"precedence") == 0) {
    qosClsTbl[i].precedence = atoi(attrValue);
  }
  else if (strcmp(attrName,"wanVlan8021p") == 0) {
    qosClsTbl[i].wanVlan8021p = atoi(attrValue);
  }
  else if (strcmp(attrName,"lanIfcName") == 0) {
    strcpy(qosClsTbl[i].lanIfcName, attrValue);
  }
  else if (strcmp(attrName,"switchport") == 0) {        // for backward compatible with 3.0
    strcpy(qosClsTbl[i].lanIfcName, "");
  }
  else if (strcmp(attrName,"vlan8021p") == 0) {
    qosClsTbl[i].vlan8021p = atoi(attrValue);
  }
  else if (strcmp(attrName,"srcAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].srcAddr,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid source IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateSubnetMask(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].srcMask,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid source subnet mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcPort") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].srcPort,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid src port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].dstAddr,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid destination IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateSubnetMask(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].dstMask,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid destination subnet mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstPort") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      strcpy(qosClsTbl[i].dstPort,attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid destination port %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dscpMark") == 0) {
    qosClsTbl[i].dscpMark = atoi(attrValue);
  }
  else if (strcmp(attrName,"srcMacAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(qosClsTbl[i].srcMacAddr, attrValue);
      else
        strcpy(qosClsTbl[i].srcMacAddr, WEB_SPACE);
    } else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid source MAC address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstMacAddr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
       if ( attrValue[0] != '\0' )
          bcmMacStrToNum(qosClsTbl[i].dstMacAddr,attrValue);
       else
          strcpy(qosClsTbl[i].dstMacAddr, WEB_SPACE);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid destination MAC address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcMacMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
       if ( attrValue[0] != '\0' )
          bcmMacStrToNum(qosClsTbl[i].srcMacMask,attrValue);
       else
          strcpy(qosClsTbl[i].srcMacMask, WEB_SPACE);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid source MAC mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstMacMask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
       if ( attrValue[0] != '\0' )
          bcmMacStrToNum(qosClsTbl[i].dstMacMask,attrValue);
       else
          strcpy(qosClsTbl[i].dstMacMask, WEB_SPACE);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid destination MAC mask %s\n",attrValue);
    }
  }
  /* start of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */
#ifdef SUPPORT_TDE_QOS
  else if (strcmp(attrName,"trafficLimit") == 0) 
  {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateState(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
       qosClsTbl[i].enbTrafficLimit = xmlStateToPsi(attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid traffic  limit %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"limitRate") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].limitRate= atoi(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid limitRate %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"id") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].clsId = atoi(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid id %s\n",attrValue);
    }
  }
#endif
  /* end of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */
  
  /*start modifying for vdf qos by p44980 2008.01.08*/
  #ifdef SUPPORT_VDF_QOS
  else if (strcmp(attrName,"id") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].ulClsInstId = atoi(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid id %s\n",attrValue);
    }
  }
  else if (strcmp(attrName, "srcMaxPort") == 0)
  {
  	if(xmlState.verify && attrValue[0] != '\0')
  	{
  	  status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
  	}
  	if(status == DB_OBJ_VALID_OK)
  	{
  	    strcpy(qosClsTbl[i].srcMaxPort, attrValue);
  	}
  	else
  	{
  	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid srcMaxPort %s\n",attrValue);
  	}
  }
  else if (strcmp(attrName, "dstMaxPort") == 0)
  {
  	if(xmlState.verify && attrValue[0] != '\0')
  	{
  	  status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
  	}
  	if(status == DB_OBJ_VALID_OK)
  	{
  	    strcpy(qosClsTbl[i].dstMaxPort, attrValue);
  	}
  	else
  	{
  	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid dstMaxPort %s\n",attrValue);
  	}
  }
  else if (strcmp(attrName,"queueId") == 0) 
  {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].ulQueueInstId = atoi(attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid queueId %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"serviceName") == 0) 
  {
    if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN + 1);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      strcpy(qosClsTbl[i].acServiecName, attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid serviceName %s\n",attrValue);
    }
  }

  /*w00135358 start-add for Global QoS 20080905*/  
  else if (strcmp(attrName,"dscpCheck") == 0) {
	if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].dscpCheck = atoi(attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid dscpCheck %s\n",attrValue);
    }    
  }
  else if (strcmp(attrName,"vlanidCheck") == 0) {
	if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].vlanidCheck = atoi(attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid vlanidCheck %s\n",attrValue);
    }       
  }
  else if (strcmp(attrName,"vlanidMark") == 0) {
	if (xmlState.verify && attrValue[0] != '\0')
    {
      status = BcmDb_validateNumber(attrValue);
    }
    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].vlanidMark = atoi(attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid vlanidMark %s\n",attrValue);
    }  

  }
  /*w00135358 end-add for Global QoS 20080905*/  
  else if (strcmp(attrName,"etheProt") == 0) {

    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateEtheProtocol(attrValue);

    if (status == DB_OBJ_VALID_OK)
    {
      qosClsTbl[i].etheProt= BcmDb_getEtheProtocol(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosClsEntryAttr(): invalid ethenet protocol %s\n",attrValue);
    }

  }
  #endif
 /*end modifying for vdf qos by p44980 2008.01.08*/

  return(PSX_STS_OK);
}

PSX_STATUS xmlEndQosClsEntryObj(char *appName, char *objName)
{
  if (xmlState.verify) {
    int i = xmlState.tableIndex;
    PQOS_CLASS_ENTRY qosClsTbl = (PQOS_CLASS_ENTRY)psiValue;

    if ( qosClsTbl == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        xmlState.tableIndex = 0;
        return (PSX_STS_ERR_FATAL);
    }

    /*start modifying for vdf qos by p44980 2008.01.11*/
    #ifndef SUPPORT_VDF_QOS
    if ( qosClsTbl[i].srcAddr[0] == '\0' && qosClsTbl[i].srcMask[0] == '\0' &&
         qosClsTbl[i].dstAddr[0] == '\0' && qosClsTbl[i].dstMask[0] == '\0' &&
         qosClsTbl[i].srcPort[0] == '\0' && qosClsTbl[i].dstPort[0] == '\0' &&
         qosClsTbl[i].protocol == SEC_PROTO_MAX && qosClsTbl[i].vlan8021p == -1 && 
         qosClsTbl[i].lanIfcName[0] == '\0' && qosClsTbl[i].srcMacAddr[0] == '\0'  &&
         qosClsTbl[i].srcMacMask[0] == '\0' && qosClsTbl[i].dstMacAddr[0] == '\0' &&
         qosClsTbl[i].dstMacMask[0] == '\0') {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): At least one condition must be specified.\n");
    }      
    else if ( qosClsTbl[i].vlan8021p != -1 &&
              (qosClsTbl[i].srcAddr[0] != '\0' || qosClsTbl[i].srcMask[0] != '\0' ||
               qosClsTbl[i].dstAddr[0] != '\0' || qosClsTbl[i].dstMask[0] != '\0' ||
               qosClsTbl[i].srcPort[0] != '\0' || qosClsTbl[i].dstPort[0] != '\0' ||
               qosClsTbl[i].protocol != SEC_PROTO_MAX || qosClsTbl[i].lanIfcName[0] != '\0')) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): 802.1p Priority cannot be selected together with IP traffic condition.\n");
    }      
    else if ( qosClsTbl[i].srcAddr[0] == '\0' && qosClsTbl[i].srcMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have source subnet mask without source IP address.\n");
    }
    else if ( qosClsTbl[i].dstAddr[0] == '\0' && qosClsTbl[i].dstMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have destination subnet mask without destination IP address.\n");
    }
    else if ( (qosClsTbl[i].srcPort[0] != '\0' || qosClsTbl[i].dstPort[0] != '\0') && 
              (qosClsTbl[i].protocol == SEC_PROTO_MAX || qosClsTbl[i].protocol == SEC_PROTO_ICMP) ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): TCP or UDP Protocol must be selected together with port number.\n");
    }       
    /* start of  问题单号：A36D02074 by xujunxia 43813 2006年6月13日
    else if ( qosClsTbl[i].srcMacAddr[0] == '\0' && qosClsTbl[i].srcMacMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have source MAC mask without source MAC address.\n");
    }
    else if ( qosClsTbl[i].dstMacAddr[0] == '\0' && qosClsTbl[i].dstMacMask[0] != '\0' ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have destination MAC mask without destination MAC address.\n");
    }
    */
    else if ( (strcmp(qosClsTbl[i].srcMacAddr, WEB_SPACE) == 0) && (strcmp(qosClsTbl[i].srcMacMask, WEB_SPACE) != 0)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have source MAC mask without source MAC address.\n");
    }
    else if (( strcmp(qosClsTbl[i].dstMacAddr, WEB_SPACE) == '\0') && (strcmp(qosClsTbl[i].dstMacMask, WEB_SPACE) != 0)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosClsEntryObj(): Cannot have destination MAC mask without destination MAC address.\n");
    }
    /* end of 问题单号：A36D02074 by xujunxia 43813 2006年6月13日 */

    #endif //support_vdf_qos
   /*end modifying for vdf qos by p44980 2008.01.11*/
  }

  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetAccCntrlAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetAccCntrlAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_PTR_TRG_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_ACC_CNTR_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_ACC_CNTR_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndAccCntrlObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndAccCntrlObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndAccCntrlObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
      BcmDb_setAccessControlSize(xmlState.tableSize);
      BcmDb_setAccessControlInfo((PSEC_ACC_CNTR_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Access Control=========\n");
  printf("xmlEndAccCntrlObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetAccCntrlEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_ACC_CNTR_ENTRY accCntrlTbl = (PSEC_ACC_CNTR_ENTRY)psiValue;
  
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetAccCntrlEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetAccCntrlEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

 /*start-add by 00112761 for VDF_C02*/

  if (strcmp(attrName,"id") == 0) 
  {
      if (xmlState.verify && attrValue[0] != '\0')
      {
          status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK)
      {
          accCntrlTbl[i].iHostId= atoi(attrValue);
      }
      else 
	  {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetAccCntrlEntryAttr(): invalid id %s\n",attrValue);
       }
  }
/*end-add by 00112761 for VDF_C02*/
  if (strcmp(attrName,"ipAddr") == 0)
  {
    /*start of acl ip 按地址段配置
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    */
    char ipAddr[SEC_BUFF_MAX_LEN] = {0};
    char *pcIpEnd = NULL;
    
    if (xmlState.verify)
    {
        strcpy(ipAddr, attrValue);
        pcIpEnd = strstr(ipAddr, "-");
        if (NULL == pcIpEnd)
        {
            status = BcmDb_validateIpAddress(ipAddr);
        }
        else
        {
            *pcIpEnd = '\0';
            status = BcmDb_validateIpAddress(ipAddr);
            if (DB_OBJ_VALID_OK == status)
            {
                pcIpEnd++;
                status = BcmDb_validateIpAddress(pcIpEnd);
            }
        }
    }
    /*end of acl ip 按地址段配置*/
	
    if (status == DB_OBJ_VALID_OK)
      strcpy(accCntrlTbl[i].address, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlEntryAttr(): invalid access control IP address %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndAccCntrlEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

PSX_STATUS xmlGetAccCntrlModeAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetAccCntrlModeAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( strcmp(attrValue, "enable") == 0 )
         *((int *)psiValue) = 1;
      else
         *((int *)psiValue) = 0;
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAccCntrlModeAttr(): invalid access control state %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartAccCntrlModeObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartAccCntrlModeObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(int));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(int));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  return PSX_STS_OK;
}

PSX_STATUS xmlEndAccCntrlModeObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndAccCntrlModeObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  int mode = *((int *)psiValue);
  
  if (!xmlState.verify)
    //BcmScm_setAccessControlMode(mode);
    BcmDb_setAccessControlMode(mode);

#ifdef XML_DEBUG1
  printf("\n============End Access Control Mode=========\n");
  printf("xmlEndAccCntrlModeObj(): mode = %d\n", mode);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
PSX_STATUS xmlGetDmzHostAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_DMZ_ENTRY pstSecDmzEntry;

  if(( NULL == attrName ) || ( NULL==attrValue ))
    return PSX_STS_ERR_GENERAL;

#ifdef XML_DEBUG
  printf("xmlGetDmzHostAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  pstSecDmzEntry = (PSEC_DMZ_ENTRY)psiValue;
  if(pstSecDmzEntry == NULL)
  {
      printf("xmlGetDmzHostAttr: pstSecDmzEntry==NULL\n");
      return PSX_STS_ERR_FATAL;
  }

  if (strcmp(attrName,"dmzEnable") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
        pstSecDmzEntry->dmzEnable = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDmzHostAttr(): invalid dmzEnable %s\n",attrValue);
    }
  }  else if (strcmp(attrName,"ipAddr") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      pstSecDmzEntry->dmzAddress.s_addr = inet_addr(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDmzHostAttr(): invalid DMZ Host IP address %s\n",attrValue);
    }
  }  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartDmzHostObj(char *appName, char *objName)
{
   if ((NULL == appName) || (NULL == objName))
   {
       return PSX_STS_ERR_GENERAL;
   }
#ifdef XML_DEBUG
  printf("xmlStartDmzHostObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SEC_DMZ_ENTRY));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue, 0, sizeof(SEC_DMZ_ENTRY));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndDmzHostObj(char *appName, char *objName)
{
    PSEC_DMZ_ENTRY pstSecDmzEntry;
    pstSecDmzEntry = (PSEC_DMZ_ENTRY)psiValue;
#ifdef XML_DEBUG
  printf("xmlEndDmzHostObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

    if (xmlState.verify) 
    {
        if(NULL == pstSecDmzEntry)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndDmzHostObj(): psiValue is NULL\n");
        }
    }
    else 
    {
        BcmDb_setDmzHost(pstSecDmzEntry);
    }
   free(psiValue);
   psiValue = NULL;
   return PSX_STS_OK;
}
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/

PSX_STATUS xmlGetServiceCtrlAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_SRV_CNTR_INFO srvCntr = (PSEC_SRV_CNTR_INFO)psiValue;
#ifdef XML_DEBUG
  printf("xmlGetServiceCtrlAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,"ftp") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeFtp = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for ftp\n",attrValue);
    }
  }  
  if (strcmp(attrName,"http") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeHttp = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for http\n",attrValue);
    }
  }  
  if (strcmp(attrName,"icmp") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeIcmp = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for icmp\n",attrValue);
    }
  }  
#ifdef SUPPORT_SNMP
  if (strcmp(attrName,PSX_OBJ_SNMP) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeSnmp = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for snmp\n",attrValue);
    }
  }  
#endif /* SUPPORT_SNMP */
  if (strcmp(attrName,"ssh") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeSsh = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for ssh\n",attrValue);
    }
  }  
  if (strcmp(attrName,"telnet") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeTelnet = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for telnet\n",attrValue);
    }
  }  
  if (strcmp(attrName,"tftp") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
      srvCntr->modeTftp = BcmDb_getSrvCntrAccess(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for tftp\n",attrValue);
    }
  }  

/* BEGIN: Added by weishi kf33269, 2011/6/26   PN:Issue407:TR-069 service is not protected with access control*/
  if (strcmp(attrName,"tr069") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateAccess(attrValue);
    if (status == DB_OBJ_VALID_OK)
    {
      srvCntr->modeTr069 = BcmDb_getSrvCntrAccess(attrValue);
	//printf("\n=====%s:%d===srvCntr->modeTr069 IS %d========\n",__FILE__,__LINE__,srvCntr->modeTr069);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetServiceCtrlAttr(): invalid access %s for telnet\n",attrValue);
    }
  }  
  /* END:   Added by weishi kf33269, 2011/6/26 */
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartServiceCtrlObj(char *appName, char *objName)
{
  PSEC_SRV_CNTR_INFO srvInfo;

#ifdef XML_DEBUG
  printf("xmlStartServiceCtrlObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SEC_SRV_CNTR_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(SEC_SRV_CNTR_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  srvInfo = (PSEC_SRV_CNTR_INFO)psiValue;
  BcmDb_getDftSrvCntrInfo(srvInfo);

  return PSX_STS_OK;
}

PSX_STATUS xmlEndServiceCtrlObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndServiceCtrlObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  if (!xmlState.verify)
      BcmDb_setSrvCntrInfo((PSEC_SRV_CNTR_INFO)psiValue);

#ifdef XML_DEBUG1
  PSEC_SRV_CNTR_INFO info = (PSEC_SRV_CNTR_INFO)psiValue;
  printf("\n============End Service Control=========\n");
  /* BEGIN: Modified by weishi kf33269, 2011/6/26   PN:Issue407:TR-069 service is not protected with access control*/
  printf("xmlEndServiceCtrlObj(): ftp %d, http %d, icmp %d snmp %d ssh %d, telnet %d tftp %d tr069 %d\n",
         info->modeFtp,info->modeHttp,info->modeIcmp,info->modeSnmp,info->modeSsh,
         info->modeTelnet,info->modeTftp,info->modeTr069);
  /* END:   Modified by weishi kf33269, 2011/6/26 */
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

//=====================================================================================
//================================End SecObj Functions ================================
//=====================================================================================

/* wan */
PSX_STATUS xmlGetWanAttr(char *attrName, char* attrValue)
{
  PWAN_CON_INFO wan = (PWAN_CON_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetWanAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strncmp(attrName,"vccId",strlen("vccId")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,ATM_VCC_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid vccId %s\n",attrValue);
    }
    else
      wan->vccId = atoi(attrValue);
  }
  if (strncmp(attrName,"conId",strlen("conId")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,ATM_VCC_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid conId %s\n",attrValue);
    }
    else
      wan->conId = atoi(attrValue);
  }
  if (strncmp(attrName,"name",strlen("name")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateServiceName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid name %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(wan->conName, attrValue, len);
      wan->conName[len] = '\0';
    }
  }
  if (strncmp(attrName,PSX_OBJ_PROTOCOL,strlen(PSX_OBJ_PROTOCOL)) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateProtocol(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid protocol %s\n",attrValue);
    }
    else
      wan->protocol = xmlWanProtocolToPsi(attrValue);
  }
  if (strncmp(attrName,"encap",strlen("encap")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateEncapsulation(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid encap %s\n",attrValue);
    }
    else
      wan->encapMode = xmlWanEncapToPsi(attrValue, wan->protocol);
  }
  if (strncmp(attrName,"firewall",strlen("firewall")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid firewall state %s\n",attrValue);
    }
    else
      wan->flag.firewall = xmlStateToPsi(attrValue);
  }
  if (strncmp(attrName,"nat",strlen("nat")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid nat state %s\n",attrValue);
    }
    else
      wan->flag.nat = xmlStateToPsi(attrValue);
  }
/*start of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
 #if 0
  if (strncmp(attrName,"igmp",strlen("igmp")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid igmp state %s\n",attrValue);
    }
    else
      wan->flag.igmp = xmlStateToPsi(attrValue);
  }
 #endif
 /*end of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
  if (strncmp(attrName,"vlanId",strlen("vlanId")) == 0) {
      wan->vlanId = atoi(attrValue);
  }
  if (strncmp(attrName,"service",strlen("service")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWanAttr(): invalid service state %s\n",attrValue);
    }
    else
      wan->flag.service = xmlStateToPsi(attrValue);
  }
  if (strncmp(attrName,"instanceId",strlen("instanceId")) == 0) {
      wan->id = atoi(attrValue);
  }
  	/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
	if (strncmp(attrName,PSX_ATTR_DNS_PRIMARY,strlen(PSX_ATTR_DNS_PRIMARY)) == 0) 
	{
		if (xmlState.verify)
		{
	  		status = BcmDb_validateIpAddress(attrValue);
		}
		if (status != DB_OBJ_VALID_OK) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlGetWanAttr(): invalid primary DNS IP address %s\n",attrValue);
		}
		else 
		{
	  		wan->stDns.preferredDns.s_addr = inet_addr((const char*)attrValue);
		}
	}
	else if (strncmp(attrName,PSX_ATTR_DNS_SECONDARY,strlen(PSX_ATTR_DNS_SECONDARY)) == 0) 
	{
		if (xmlState.verify)
		{
	  		status = BcmDb_validateIpAddress(attrValue);
		}
		if (status != DB_OBJ_VALID_OK) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlGetWanAttr(): invalid secondary DNS IP address %s\n",attrValue);
		}
		else 
		{
	  		wan->stDns.alternateDns.s_addr = inet_addr((const char*)attrValue);
		}
	}
	else if (strncmp(attrName,PSX_ATTR_DNS_DOMAIN,strlen(PSX_ATTR_DNS_DOMAIN)) == 0) 
	{
		if (xmlState.verify)
		{
	  		status = BcmDb_validateLength(attrValue,IFC_DOMAIN_LEN);
		}
		if (status != DB_OBJ_VALID_OK) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlGetWanAttr(): invalid domain name %s\n",attrValue);
		}
		else 
		{
	  		strncpy(wan->stDns.domainName, (const char*)attrValue, sizeof(wan->stDns.domainName));/* Modify for codeview */
		}
	}
  	/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
	/*start of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
	else if(0 == strncmp(attrName, "gw", strlen("gw")))
	{
		if (xmlState.verify)
		{
	  		status = BcmDb_validateIpAddress(attrValue);
		}
		if (status != DB_OBJ_VALID_OK) 
		{
	  		xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  		syslog(LOG_ERR,"xmlGetWanAttr(): invalid gateway IP address %s\n",attrValue);
		}
		else 
		{
	  		wan->ipGateWay.s_addr = inet_addr((const char*)attrValue);
		}
	}
	/*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartWanObjNode(char *appName, char *objName)
{
  PWAN_CON_INFO wan;

#ifdef XML_DEBUG
  printf("xmlStartWanObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(WAN_CON_INFO));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(WAN_CON_INFO));
  wan = (PWAN_CON_INFO)psiValue;
  wan->protocol = PROTO_BRIDGE;
  wan->encapMode = xmlWanEncapToPsi("LLC", PROTO_BRIDGE);
  wan->flag.firewall = TRUE;
  wan->flag.nat = TRUE;
/*start of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
 // wan->flag.igmp = FALSE;
/*end of删除brcom igmp proxy 使能功能by l129990 ,2008,9,28*/
  wan->flag.service = TRUE;
  wan->vlanId = -1;
	/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
	memset(&(wan->stDns), 0, sizeof(IFC_DNS_INFO));
	/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
	/*start of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
	wan->ipGateWay.s_addr = 0;
	/*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
  return PSX_STS_OK;
}

PSX_STATUS xmlEndWanObjNode(char *appName, char *objName)
{
  PWAN_CON_INFO wan = (PWAN_CON_INFO)psiValue;
  WAN_CON_ID wanId;
  
#ifdef XML_DEBUG
  printf("xmlEndWanObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  memset((void*)&wanId,0,sizeof(WAN_CON_ID));

  if (xmlVerifyVccId(wan,&wanId.vpi,&wanId.vci) == PSX_STS_ERR_FATAL) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndWanObjNode(): vccId%d doesn't exist in config file\n",wan->vccId);
  }
  if (wan->protocol != PROTO_BRIDGE) {
    if (xmlVerifyConIdExist(wan) == PSX_STS_ERR_FATAL) {
      syslog(LOG_ERR,"xmlEndWanObjNode(): conId %d doesn't exist in config file\n",wan->conId);
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
    }
  }
  if (xmlState.errorFlag != PSX_STS_ERR_FATAL) {
    if (!xmlState.verify) {
      wanId.conId = wan->conId;

#ifdef XML_DEBUG
      printf("xmlEndWanObjNode(): BcmDb_setWanInfo(wanId.vpi %d, wanId.vci %d, wan.conId %d)\n",
             wanId.vpi,wanId.vci,wanId.conId);
#endif
      BcmDb_setWanInfo(&wanId,wan);
    }
  }

#ifdef XML_DEBUG1
  printf("\n============End WAN info=========\n");
  printf("xmlEndWanObj(): vpi %d, vci %d, vccId %d, conId %d, protocol %d, encapMode %d, firewall %d, nat %d igmp %d, vlanId %d, service %d, conName %s\n", 
     wanId.vpi, wanId.vci, wan->vccId, wan->conId, wan->protocol, wan->encapMode, wan->flag.firewall, wan->flag.nat, wan->flag.igmp, wan->vlanId, wan->flag.service, wan->conName);
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetWanObjNode(char *appName, char *objName, char *attrName,
                            char* attrValue)
{
  PSX_STATUS sts = PSX_STS_ERR_FATAL;

#ifdef XML_DEBUG
  printf("xmlGetWanObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif
  if (strncasecmp(objName,OBJNAME_GEN_ENTRY_FORMAT, strlen(OBJNAME_GEN_ENTRY_FORMAT)) == 0)
    sts = xmlGetWanAttr(attrName, attrValue);
  else {
    syslog(LOG_ERR,"xmlGetWanObjNode(): invalid object name %s, expect %s followed by entry number.\n",objName,
           OBJNAME_GEN_ENTRY_FORMAT);
  }
  return sts;
} /* xmlGetWanObjNode */

/* ppp */
PSX_STATUS xmlGetPppAttr(char *attrName, char* attrValue)
{
  PWAN_PPP_INFO ppp = (PWAN_PPP_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetPppAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strncmp(attrName,"userName",strlen("userName")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePppUserName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid userName %s\n",attrValue);
    }
    else
      strcpy(ppp->userName,attrValue);
  }
  if (strncmp(attrName,"password",strlen("password")) == 0) {
    int dataLen = 0;
    char *data = NULL;
  
    // for backward compatible with previous configuration file that 
    // have passwords without encoding, need to check encode password
    // info, if passwords are encoded then decode passwords, otherwise
    // do not decode passwords and set encode password info to true
    // so that when configuration file is saved, passwords are encoded
    // and encode password info is set to true.
    int len = strlen(attrValue);
    if ( BcmDb_getEncodePasswordInfo() == TRUE ) {
      data = (char *)malloc(len * 3/4 + 8);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(data, 0, len *3/4 + 8);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      //#ifdef SUPPORT_PASSWORD_UNENCRYPTED
	  #ifdef SUPPORT_PPPSRVPWD_UNENCRYPTED  //modified by w00135351 09.3.14
      strcpy(data, attrValue);
      #else
      base64Decode((unsigned char *)data, attrValue, &dataLen);
      data[dataLen] = '\0';
      #endif
    } else {
      data = (char *)malloc(len);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(data, 0, len);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      strcpy(data, attrValue);
      BcmDb_setEncodePasswordInfo(TRUE);
    }
      
    if (xmlState.verify) 
      status = BcmDb_validatePppPassword(data);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid password %s\n",attrValue);
    }
    else
      strcpy(ppp->password,data);
    free(data);
  }
  if (strncmp(attrName,"serviceName",strlen("serviceName")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateServiceName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid serviceName %s\n",attrValue);
    }
    else
      strcpy(ppp->serverName,attrValue);
  }
  if (strncmp(attrName,"idleTimeout",strlen("idleTimeout")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,655636);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid idleTimeout %s\n",attrValue);
    }
    else
      ppp->idleTimeout = atoi(attrValue);
  }

//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
if (strncmp(attrName,"pppNumber",strlen("pppNumber")) == 0) {
  if (xmlState.verify) 
	status = BcmDb_validateRange(attrValue,0,655636);
  if (status != DB_OBJ_VALID_OK) {
	xmlState.errorFlag = PSX_STS_ERR_FATAL;
	syslog(LOG_ERR,"xmlGetPppAttr(): invalid pppNumber %s\n",attrValue);
  }
  else
	ppp->pppNumber = atoi(attrValue);
}
//END:add by zhourongfei to config number of PPP keepalive attempts

//start modify by xia 133940 2008年9月8日
//加入四个if段，分别用来读取四个参数在PSI中的值

  if (strncmp(attrName,"automanualConnect",strlen("automanualConnect")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,10);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid automanualConnect %s\n",attrValue);
    }
    else
      ppp->automanualConnect = atoi(attrValue);
  }


  if (strncmp(attrName,"delayRedial",strlen("delayRedial")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,60);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid delayRedial %s\n",attrValue);
    }
    else
      ppp->delayRedial = atoi(attrValue);
  }


  if (strncmp(attrName,"timeout",strlen("timeout")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,60);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid timeout %s\n",attrValue);
    }
    else
      ppp->timeout = atoi(attrValue);
  }


  if (strncmp(attrName,"delayTime",strlen("delayTime")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,60);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid delayTime %s\n",attrValue);
    }
    else
      ppp->delayTime = atoi(attrValue);
  }

//end modify by xia 133940 2008年9月8日


  /*Start -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/
  if (strncmp(attrName,"enblpppVoB",strlen("enblpppVoB")) == 0) {
    if (xmlState.verify) 
		status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid enblpppVoB %s\n",attrValue);
    }
    else
      ppp->enblpppVoB = xmlStateToPsi(attrValue);
  }
  if (strncmp(attrName,"pppVoBitstreamIPAddr",strlen("pppVoBitstreamIPAddr")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid pppVoBitstreamIPAddr %s\n",attrValue);
    }
    else {
      ppp->pppVoBitstreamIPAddr.s_addr = inet_addr(attrValue);
    }
  }
  /*End -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/

  
  if (strncmp(attrName,"ipExt",strlen("ipExt")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid ipExt state %s\n",attrValue);
    }
    else
      ppp->enblIpExtension = xmlStateToPsi(attrValue);
  }
  if (strncmp(attrName,"auth",strlen("auth")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePppAuthMethod(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid auth %s\n",attrValue);
    }
    else
      ppp->authMethod = xmlPppAuthToPsi(attrValue);
  }
  if (strncmp(attrName,"useStaticIpAddr",strlen("useStaticIpAddr")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid enableIpAddress %s\n",attrValue);
    }
    else
      ppp->useStaticIpAddress = atoi(attrValue);
  }
  if (strncmp(attrName,"localIpAddr",strlen("localIpAddr")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid IpAddress %s\n",attrValue);
    }
    else {
      ppp->pppLocalIpAddress.s_addr = inet_addr(attrValue);
    }
  }
  if (strncmp(attrName,"Debug",strlen("Debug")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid Debug state %s\n",attrValue);
    }
    else
      ppp->enblDebug = xmlStateToPsi(attrValue);
  }
  //add by l66195 for pppousb start
  if (strncmp(attrName,"ProName",strlen("ProName")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validatePppUserName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPppAttr(): invalid ProName %s\n",attrValue);
    }
    else
      strcpy(ppp->acProName,attrValue);
  }
  //add by l66195 for pppousb end
  /* BEGIN: Added by y67514, 2008/9/8   PN:GLB:增加参数mtu，mixed*/
    if ( 0 == strncmp(attrName,"BridgeMixed",strlen("BridgeMixed")) )
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateState(attrValue);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetPppAttr(): invalid BridgeMixed state %s\n",attrValue);
        }
        else
        {
            ppp->enblBridgeMixed= xmlStateToPsi(attrValue);
        }
    }
    if ( 0 == strncmp(attrName,"Mtu",strlen("Mtu")) )
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateRange(attrValue,MIN_MTU,PPP_MAX_MTU);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetPppAttr(): invalid Mtu %s\n",attrValue);
        }
        else
        {
            ppp->Mtu= atoi(attrValue);
        }
    }
    /* BEGIN: Added by y67514, 2008/11/1   问题单号:GLB:KeepAlive需求*/
    if ( 0 == strncmp(attrName,"LcpEchoInterval",strlen("LcpEchoInterval")) )
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateRange(attrValue,MIN_LCP_ECHO_INT,MAX_LCP_ECHO_INT);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetPppAttr(): invalid LcpEchoInterval %s\n",attrValue);
            printf("==KeepAlive:%s:%s:%d:invalid LcpEchoInterval %s!!!==\n",__FILE__,__FUNCTION__,__LINE__,attrValue);
        }
        else
        {
            ppp->lcpEchoInterval= atoi(attrValue);
        }
    }
    /* END:   Added by y67514, 2008/11/1 */
    if (0 == strncmp(attrName,"IpExtMac",strlen("IpExtMac")) ) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateIpExtMac(attrValue);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetPppAttr(): invalid IpExtMac %s\n",attrValue);
        }
        else
        {
            strcpy(ppp->ipExtMac,attrValue);
            printf("==IpExt:%s:%s:%d:mac is %s==\n",__FILE__,__FUNCTION__,__LINE__,attrValue);
        }
    }
  /* END:   Added by y67514, 2008/9/8 */
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartPppObjNode(char *appName, char *objName)
{
  PWAN_PPP_INFO ppp;

#ifdef XML_DEBUG
  printf("xmlStartPppObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(WAN_PPP_INFO));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(WAN_PPP_INFO));
  ppp = (PWAN_PPP_INFO)psiValue;
  ppp->idleTimeout = atoi(BcmDb_getDefaultValue("pppTimeOut"));
//start modify by xia 133940 2008年9月8日
	ppp->automanualConnect = atoi(BcmDb_getDefaultValue("automanualConnect"));
	ppp->delayRedial = atoi(BcmDb_getDefaultValue("delayRedial"));
	ppp->timeout = atoi(BcmDb_getDefaultValue("timeout"));
	ppp->delayTime = atoi(BcmDb_getDefaultValue("delayTime"));
//end modify by xia 133940 2008年9月8日

    /*Start -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/
	ppp->enblpppVoB = atoi(BcmDb_getDefaultValue("enblpppVoB"));
	ppp->pppVoBitstreamIPAddr.s_addr = inet_addr(BcmDb_getDefaultValue("pppVoBitstreamIPAddr"));
    /*End -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/


  ppp->enblIpExtension = atoi(BcmDb_getDefaultValue("pppIpExtension"));
  ppp->authMethod = atoi(BcmDb_getDefaultValue("pppAuthMethod"));
  strcpy(ppp->userName, BcmDb_getDefaultValue("pppUserName"));
  strcpy(ppp->password, BcmDb_getDefaultValue("pppPassword"));
  ppp->useStaticIpAddress = atoi(BcmDb_getDefaultValue("useStaticIpAddress"));
  ppp->pppLocalIpAddress.s_addr = inet_addr(BcmDb_getDefaultValue("pppLocalIpAddress"));
  ppp->enblDebug = atoi(BcmDb_getDefaultValue("enblPppDebug"));
// add by l66195 for pppousb start
  strcpy(ppp->acProName, BcmDb_getDefaultValue("acProName"));
// add by l66195 for pppousb end
//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
	ppp->pppNumber = atoi(BcmDb_getDefaultValue("pppNumber"));
//END:add by zhourongfei to config number of PPP keepalive attempts
/* BEGIN: Added by y67514, 2008/9/8   PN:GLB增加参数mtu，mixed*/
    ppp->enblBridgeMixed= atoi(BcmDb_getDefaultValue("enblBridgeMixed"));
    ppp->Mtu= atoi(BcmDb_getDefaultValue("PppMtu"));
    /* BEGIN: Added by y67514, 2008/11/12   问题单号:GLB:KeepAlive需求*/
    ppp->lcpEchoInterval = atoi(BcmDb_getDefaultValue("lcpEchoInterval"));
    /* END:   Added by y67514, 2008/11/12 */
    strcpy(ppp->ipExtMac, BcmDb_getDefaultValue("PppExtMac"));
    printf("==MTU:%s:%s:%d:Mtu is %d==\n",__FILE__,__FUNCTION__,__LINE__,ppp->Mtu);

/* END:   Added by y67514, 2008/9/8 */
  return PSX_STS_OK;
}

PSX_STATUS xmlEndPppObjNode(char *appName, char *objName)
{
  PWAN_PPP_INFO ppp = (PWAN_PPP_INFO)psiValue;
  WAN_CON_ID wanId;
  int vpi=0, vci=0, conId = 0;

#ifdef XML_DEBUG
  printf("xmlEndPppObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  /* obtain vpi/vci from objName */
  sscanf(appName,APPNAME_PPP_FORMAT,&vpi,&vci);
  wanId.vpi = (UINT16)vpi;
  wanId.vci = (UINT16)vci;

  /* obtain conId from objId */
  sscanf(objName,OBJNAME_PPP_ENTRY_FORMAT,&conId);
  wanId.conId = (UINT16)conId;

  if (xmlState.verify) {
    if (xmlVerifyVccExist(wanId.vpi,wanId.vci) == PSX_STS_ERR_FATAL) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPppObjNode(%s): PPP PVC %d/%d doesn't exist in config file\n",
             objName,wanId.vpi,wanId.vci);
    }
    if (xmlVerifyConIdProtocol(&wanId,"PPP") == PSX_STS_ERR_FATAL) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPppObjNode(%s): wan_%d_%d conId %d running PPP doesn't exist in config file\n",
             objName,wanId.vpi,wanId.vci, wanId.conId);
    }
  } /* verify */
  if (xmlState.errorFlag != PSX_STS_ERR_FATAL) {
    if (!xmlState.verify) {
      BcmDb_setPppSrvInfo(&wanId,ppp);
    }
  }
  //BEGIN:modify by zhourongfei to config number of PPP keepalive attempts
#ifdef XML_DEBUG1
  printf("\n============End PPP info=========\n");
  printf("xmlEndPppObj(): vpi %d, vci %d, userName %s, password %s, serviceName %s, idleTimeout %d, ipExt %d, auth %d, pppNumber %d\n",
         wanId.vpi, wanId.vci, ppp->userName, ppp->password, ppp->serverName, ppp->idleTimeout, ppp->enblIpExtension, ppp->authMethod, ppp->pppNumber);
  printf("===============================================\n");
#endif
  //END:modify by zhourongfei to config number of PPP keepalive attempts
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetPppObjNode(char *appName, char *objName, char *attrName,
                            char* attrValue)
{
  PSX_STATUS sts;

#ifdef XML_DEBUG
  printf("xmlGetPppObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif

  sts = xmlGetPppAttr(attrName, attrValue);
  return sts;
} /* xmlGetPppObjNode */

/* ipsrv */
PSX_STATUS xmlGetDhcpClntAttr(char *attrName, char* attrValue)
{
  PWAN_IP_INFO dhcp = (PWAN_IP_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  
#ifdef XML_DEBUG
  printf("xmlGetDhcpClntAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_STATE,strlen(PSX_ATTR_STATE)) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDhcpClntAttr(): invalid state %s\n",attrValue);
    }
    else {
      dhcp->enblDhcpClnt = xmlStateToPsi(attrValue);
    }
  }
  /* start of maintain PSI移植:  增加dhcp option60的描述   by xujunxia 43813 2006年5月5日 */
  if (0 == strncmp(attrName, "dhcpClassIdentifier", strlen("dhcpClassIdentifier"))) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue, DHCP_CLASS_ID_MAX);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR, "xmlGetDhcpClntAttr(): invalid wanAddress %s\n", attrValue);
    }
    else {
      strcpy(dhcp->dhcpClassIdentifier, attrValue);
    }
  }
   /* end of maintain PSI移植:  增加dhcp option60的描述   by xujunxia 43813 2006年5月5日 */
   #ifdef SUPPORT_GLB_MTU
   if ( 0 == strncmp(attrName,"mtu",strlen("mtu")) )
   {
       if (xmlState.verify) 
       {
           status = BcmDb_validateRange(attrValue,MIN_MTU,MAX_MTU);
       }
       if (status != DB_OBJ_VALID_OK) 
       {
           xmlState.errorFlag = PSX_STS_ERR_FATAL;
           syslog(LOG_ERR,"xmlGetDhcpClntAttr(): invalid Mtu %s\n",attrValue);
       }
       else
       {
           dhcp->mtu= atoi(attrValue);
       }
   }
   #endif /* SUPPORT_GLB_MTU */
  if (strncmp(attrName,"wanAddress",strlen("wanAddress")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);

    if ((!dhcp->enblDhcpClnt) && (status != DB_OBJ_VALID_OK)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDhcpClntAttr(): invalid wanAddress %s\n",attrValue);
    }
    else {
      dhcp->wanAddress.s_addr = inet_addr(attrValue);
    }
  }
  if (strncmp(attrName,"wanMask",strlen("wanMask")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateSubnetMask(attrValue);
#ifdef CPE_DSL_MIB
    /* for CT MIB, subnet mask 0.0.0.0 pass checking */
    if ((!dhcp->enblDhcpClnt) && (strcmp(attrValue,"0.0.0.0") == 0))
	status = DB_OBJ_VALID_OK;
#endif


    if ((!dhcp->enblDhcpClnt) && (status != DB_OBJ_VALID_OK)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDhcpClntAttr(): invalid wanMask %s\n",attrValue);
    }
    else {
      dhcp->wanMask.s_addr = inet_addr(attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartDhcpClntNode(char *appName, char *objName)
{
  PWAN_IP_INFO dhcp;

#ifdef XML_DEBUG
  printf("xmlStartDhcpClnt(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(WAN_IP_INFO));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(WAN_IP_INFO));
  dhcp = (PWAN_IP_INFO)psiValue;
  dhcp->wanAddress.s_addr =  inet_addr(BcmDb_getDefaultValue("wanIpAddress"));
  dhcp->wanMask.s_addr = inet_addr(BcmDb_getDefaultValue("wanSubnetMask"));
  dhcp->enblDhcpClnt = atoi(BcmDb_getDefaultValue("enblDhcpClnt"));
#ifdef SUPPORT_GLB_MTU
    dhcp->mtu = atoi(BcmDb_getDefaultValue("IpoeMtu"));
#endif /* SUPPORT_GLB_MTU */

  return PSX_STS_OK;
}

PSX_STATUS xmlEndDhcpClntNode(char *appName, char *objName)
{
  PWAN_IP_INFO dhcp = (PWAN_IP_INFO)psiValue;
  WAN_CON_ID wanId;
  int vpi=0, vci=0, conId = 0;

#ifdef XML_DEBUG
  printf("xmlEndDhcpClnt(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  /* obtain vpi/vci from objName */
  sscanf(appName,APPNAME_IPSRV_ENTRY_FORMAT,&vpi,&vci);
  wanId.vpi = (UINT16)vpi;
  wanId.vci = (UINT16)vci;

  /* obtain conId from objId */
  sscanf(objName,OBJNAME_IPSRV_ENTRY_FORMAT,&conId);
  wanId.conId = (UINT16)conId;

  if (xmlState.verify) {
    if (xmlVerifyVccExist(wanId.vpi,wanId.vci) == PSX_STS_ERR_FATAL) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndDhcpClntNode(%s): ipSrv PVC %d/%d doesn't exist in config file\n",
             objName,wanId.vpi,wanId.vci);
    }
    if (xmlVerifyConIdProtocol(&wanId,"PROTO_MER") == PSX_STS_ERR_FATAL) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndDhcpClntNode(%s): wan_%d_%d conId %d running dhcpc doesn't exist in config file\n",
             objName,wanId.vpi,wanId.vci, wanId.conId);
    }
  } /* verify */
  if (xmlState.errorFlag != PSX_STS_ERR_FATAL) {
    if (!xmlState.verify) {
      BcmDb_setIpSrvInfo(&wanId,dhcp);
    }
  }

#ifdef XML_DEBUG1
  printf("\n============End IPSRV info=========\n");
  printf("xmlEndDhcpClnt(): vpi %d, vci %d, conId %d, state %d, wanAddress %s, wanMask %s dhcpClassIdentifier %s\n",
         wanId.vpi, wanId.vci, wanId.conId, dhcp->enblDhcpClnt, inet_ntoa(dhcp->wanAddress), 
         inet_ntoa(dhcp->wanMask), dhcp->dhcpClassIdentifier);
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetDhcpClntNode(char *appName, char *objName, char *attrName,
                              char* attrValue)
{
  PSX_STATUS sts;

#ifdef XML_DEBUG
  printf("xmlGetDhcpClntNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif

  sts = xmlGetDhcpClntAttr(attrName, attrValue);
  return sts;
} /* xmlGetDhcpClntNode */

PSX_STATUS xmlGetAtmTdAttr(char *attrName, char* attrValue)
{
  PIFC_ATM_TD_INFO td = (PIFC_ATM_TD_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetAtmTdAttr(attrName %s, attrValue %s)\n",
         attrName,attrValue);
#endif

  if (strncmp(attrName,"cat",strlen("cat")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAtmCategory(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmTdAttr(): invalid cat %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(td->serviceCategory, attrValue, len);
      td->serviceCategory[len] = '\0';
    }
  }
  if (strncmp(attrName,"PCR",strlen("PCR")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmTdAttr(): invalid PCR %s\n",attrValue);
    }
    else
      td->peakCellRate = atoi(attrValue);
  }
  if (strncmp(attrName,"SCR",strlen("SCR")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmTdAttr(): invalid SCR %s\n",attrValue);
    }
    else
      td->sustainedCellRate = atoi(attrValue);
  }
  if (strncmp(attrName,"MBS",strlen("MBS")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmTdAttr(): invalid MBS %s\n",attrValue);
    }
    else
      td->maxBurstSize = atoi(attrValue);
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartAtmTdObjNode(char *appName, char *objName)
{
  PIFC_ATM_TD_INFO td;

#ifdef XML_DEBUG
  printf("xmlStartAtmTdObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(IFC_ATM_TD_INFO));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue,0,sizeof(IFC_ATM_TD_INFO));
  td = (PIFC_ATM_TD_INFO)psiValue;
  td->peakCellRate = atoi(BcmDb_getDefaultValue("atmPeakCellRate"));
  td->sustainedCellRate = atoi(BcmDb_getDefaultValue("atmSustainedCellRate"));
  td->maxBurstSize = atoi(BcmDb_getDefaultValue("atmMaxBurstSize"));
  strcpy(td->serviceCategory, BcmDb_getDefaultValue("atmServiceCategory"));

  return PSX_STS_OK;
}

PSX_STATUS xmlEndAtmTdObjNode(char *appName, char *objName)
{
  PIFC_ATM_TD_INFO td = (PIFC_ATM_TD_INFO)psiValue;
  UINT16 tdId;

#ifdef XML_DEBUG
  printf("xmlEndAtmTdObj(calling cfm to get appName %s, objName %s\n",
         appName,objName);
#endif
  tdId = atoi(objName+strlen("td")); /* extracting tdId from objName */
  /* start of 支撑 问题单: A36D03168 by xujunxia 43813 2006年11月28
  if (xmlState.verify) {
  */
  if (!xmlState.verify) {
  /* end of 支撑 问题单: A36D03168 by xujunxia 43813 2006年11月28*/
    /* just checking scr < pcr, pcr cannot be checked since maxPcr = 0 when ADSL is down */
    /* just checking to see all the mandatory fields are configured */
    if ((strcmp(td->serviceCategory,"UBR")) != 0) {
      if (td->peakCellRate == 0) {
        /* start of 支撑 问题单: A36D03168 by xujunxia 43813 2006年11月28
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetAtmTdAttr(): PCR cannot be 0 for cat %s\n",td->serviceCategory);
        */
        td->peakCellRate = ATM_MAX_PCR / 6;
        /* end of 支撑 问题单: A36D03168 by xujunxia 43813 2006年11月28 */
      } /* pcr == 0 */
      else {
        if (((strcmp(td->serviceCategory,"NRT_VBR")) == 0) ||
            ((strcmp(td->serviceCategory,"RT_VBR")) == 0)) {
          if (td->sustainedCellRate == 0) {
            /* start of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAtmTdAttr(): SCR cannot be 0 for cat %s\n",td->serviceCategory);
            */
            td->sustainedCellRate = ATM_MAX_SCR / 6;
            /* end of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28 */
          }
          if (td->maxBurstSize == 0) {
            /* start of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAtmTdAttr(): MBS cannot be 0 for cat %s\n",td->serviceCategory);
            */
            td->maxBurstSize = ATM_MAX_MBS / 6;
            /* end of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28 */
          }
          if (td->sustainedCellRate > td->peakCellRate) {
            /* start of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetAtmTdAttr(): SCR %lu must be less than PCR %lu\n",
                   td->sustainedCellRate,td->peakCellRate);
            */
            td->sustainedCellRate = td->peakCellRate;
            /* end of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28 */
          }
        } /* vbr */
      } /* pcr != 0 */
    } /* != ubr */
    /* start of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28*/
//  } /* verify */
//  else {
    /* end of 支撑 问题单： A36D03168 by xujunxia 43813 2006年11月28*/
    if (xmlState.errorFlag != PSX_STS_ERR_FATAL) {
      BcmPsi_objStore(BcmPsi_appOpen(ATM_TD_APPID), tdId, td, sizeof(IFC_ATM_TD_INFO));
    }
  } /* !verify */ 

#ifdef XML_DEBUG1
  printf("\n============End ATM VCC info=========\n");
  printf("xmlEndAtmTdObj(): tdId %d, cat %s, pcr = %lu, scr = %lu, mbs = %lu\n",
         tdId, td->serviceCategory, td->peakCellRate, td->sustainedCellRate,
         td->maxBurstSize);
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetAtmTdObjNode(char *appName, char *objName, char *attrName,
                              char* attrValue)
{
  PSX_STATUS sts;

#ifdef XML_DEBUG
  printf("xmlGetAtmTdObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
         appName,objName,attrName,attrValue);
#endif

  sts = xmlGetAtmTdAttr(attrName, attrValue);
  return sts;
} /* xmlGetAtmTdObjNode */

/* lan */
PSX_STATUS xmlGetLanNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
#ifdef ETH_CFG
  if (strncmp(objName,PSX_OBJ_ETHCFG, strlen(PSX_OBJ_ETHCFG)) == 0)
    return(xmlGetEthAttr(attrName,attrValue));
  else
#endif
    return (xmlGetLanAttr(attrName,attrValue));
}

int xmlDhcpSrvStateToPsi(char *value)
{
  if (strcmp("disable",value) == 0)
    return 0;
  else if (strcmp("enable",value) == 0)
    return 1;
  else return 2;
}

PSX_STATUS xmlGetLanAttr(char *attrName, char* attrValue)
{
  PIFC_LAN_INFO lanInfo = (PIFC_LAN_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetLanAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"address",strlen("address")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid address %s\n",attrValue);
    }
    else {
      lanInfo->lanAddress.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"mask",strlen("mask")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateSubnetMask(attrValue);
      
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid mask %s\n",attrValue);
    }
    else {
      lanInfo->lanMask.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpServer",strlen("dhcpServer")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateDhcpState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpServer state %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.enblDhcpSrv = xmlDhcpSrvStateToPsi(attrValue);
    }
  }
  if (strncmp(attrName,"startAddr",strlen("startAddr")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
      
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid startAddr %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.startAddress.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"endAddr",strlen("endAddr")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid endAddr %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.endAddress.s_addr = inet_addr(attrValue);
    }
  }
  #ifdef	VDF_RESERVED
	else if (strncmp(attrName,"reservedAddr",strlen("reservedAddr")) == 0) 
	{
		if (xmlState.verify)
			status = (strlen(attrValue) <= DHCP_RESERVED_MAXCHAR) ?DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    
		if (status != DB_OBJ_VALID_OK) 
		{
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetLanAttr(): invalid reservedAddr %s\n",attrValue);
		}
		else 
		{
			strcpy(lanInfo->dhcpSrv.reservedAddress,attrValue);
		}
	}
  #endif
  else if (strncmp(attrName,"leasedTime",strlen("leasedTime")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid leasedTime %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.leasedTime = atoi(attrValue);
    }
  }
  /*start of support to the dhcp relay  function by l129990,2009,11,12*/
  else if (strncmp(attrName,"dhcpRelay",strlen("dhcpRelay")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpRelay %s\n",attrValue);
    }
    else {
      lanInfo->dhcpRelay = atoi(attrValue);
    }
  }
  /*end of support to the dhcp relay  function by l129990,2009,11,12*/
#ifdef SUPPORT_PORTMAPING
    else if (strncmp(attrName,"dhcpoption60",strlen("dhcpoption60")) == 0) {
    if (xmlState.verify)
    	/*start of 修改option60的大小by  s53329 at  20070616
      status = (strlen(attrValue) < RADIUS_SERVER_LEN) ?
     end  of 修改option60的大小by  s53329 at  20070616 */
      status = (strlen(attrValue) < IFC_LARGE_LEN) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid leasedTime %s\n",attrValue);
    }
    else {
       strcpy(lanInfo->dhcpSrv.option60, attrValue);
        }
  }
#endif
  /*start DHCP Server支持第二地址池,s60000658, 20060616*/
  else if (strncmp(attrName,"dhcpClassId",strlen("dhcpClassId")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrvClassId %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrvClassId, attrValue);
    }
  }
  /*end DHCP Server支持第二地址池,s60000658, 20060616*/
  
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
  else if (strncmp(attrName,"poolIndex",strlen("poolIndex")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,1,5);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid poolIndex %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.poolIndex = atoi(attrValue);
    }
  }  
  else if (strncmp(attrName,"dhcpStart1_1",strlen("dhcpStart1_1")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpStart1_1 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpStart1_1.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpEnd1_1",strlen("dhcpEnd1_1")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpEnd1_1 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpEnd1_1.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpLease1_1",strlen("dhcpLease1_1")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpLease1_1 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpLease1_1 = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpSrv1Option60_1",strlen("dhcpSrv1Option60_1")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrv1Option60_1 %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrv1Option60_1, attrValue);
    }
  }
    else if (strncmp(attrName,"dhcpStart1_2",strlen("dhcpStart1_2")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpStart1_2 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpStart1_2.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpEnd1_2",strlen("dhcpEnd1_2")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpEnd1_2 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpEnd1_2.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpLease1_2",strlen("dhcpLease1_2")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpLease1_2 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpLease1_2 = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpSrv1Option60_2",strlen("dhcpSrv1Option60_2")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrv1Option60_2 %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrv1Option60_2, attrValue);
    }
  }
else if (strncmp(attrName,"dhcpStart1_3",strlen("dhcpStart1_3")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpStart1_3 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpStart1_3.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpEnd1_3",strlen("dhcpEnd1_3")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpEnd1_3 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpEnd1_3.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpLease1_3",strlen("dhcpLease1_3")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpLease1_3 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpLease1_3 = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpSrv1Option60_3",strlen("dhcpSrv1Option60_3")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrv1Option60_3 %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrv1Option60_3, attrValue);
    }
  }
else if (strncmp(attrName,"dhcpStart1_4",strlen("dhcpStart1_4")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpStart1_4 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpStart1_4.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpEnd1_4",strlen("dhcpEnd1_4")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpEnd1_4 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpEnd1_4.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpLease1_4",strlen("dhcpLease1_4")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpLease1_4 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpLease1_4 = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpSrv1Option60_4",strlen("dhcpSrv1Option60_4")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrv1Option60_4 %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrv1Option60_4, attrValue);
    }
  }
else if (strncmp(attrName,"dhcpStart1_5",strlen("dhcpStart1_5")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpStart1_5 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpStart1_5.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpEnd1_5",strlen("dhcpEnd1_5")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpEnd1_5 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpEnd1_5.s_addr = inet_addr(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpLease1_5",strlen("dhcpLease1_5")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,0,65535);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpLease1_5 %s\n",attrValue);
    }
    else {
      lanInfo->dhcpSrv.dhcpLease1_5 = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"dhcpSrv1Option60_5",strlen("dhcpSrv1Option60_5")) == 0) {
    if (xmlState.verify)
      status = (strlen(attrValue) < DHCP_CLASS_ID_MAX) ?
               DB_OBJ_VALID_OK : DB_OBJ_VALID_ERR_INVALID_VALUE;
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpSrv1Option60_5 %s\n",attrValue);
    }
    else {
      strcpy(lanInfo->dhcpSrv.dhcpSrv1Option60_5, attrValue);
    }
  }
#ifdef SUPPORT_CHINATELECOM_DHCP
  else if (strncmp(attrName,"cameraport",strlen("cameraport")) == 0) {
      strcpy(lanInfo->dhcpSrv.cameraport, attrValue);    
  }
  else if (strncmp(attrName,"ccategory",strlen("ccategory")) == 0) {
      strcpy(lanInfo->dhcpSrv.ccategory, attrValue);    
  }
  else if (strncmp(attrName,"cmodel",strlen("cmodel")) == 0) {
      strcpy(lanInfo->dhcpSrv.cmodel, attrValue);    
  }
#endif
#endif
#ifdef SUPPORT_VDF_DHCP
	else if(strncmp(attrName, "dhcpNtpServer1", strlen("dhcpNtpServer1")) == 0)
	{
		if (xmlState.verify)
			status = BcmDb_validateIpAddress(attrValue);
		if (status != DB_OBJ_VALID_OK) 
		{
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpNtpServer1 %s\n",attrValue);
		}
		else
		{
			lanInfo->dhcpSrv.dhcpNtpAddr1.s_addr = inet_addr(attrValue);
		}
	}
	else if(strncmp(attrName, "dhcpNtpServer2", strlen("dhcpNtpServer2")) == 0)
	{
		if (xmlState.verify)
			status = BcmDb_validateIpAddress(attrValue);
		if (status != DB_OBJ_VALID_OK) 
		{
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetLanAttr(): invalid dhcpNtpServer2 %s\n",attrValue);
		}
		else
		{
			lanInfo->dhcpSrv.dhcpNtpAddr2.s_addr = inet_addr(attrValue);
		}
	}
#endif
        /* BEGIN: Added by y67514, 2008/9/22   问题单号:GLB:lan dns*/
        else if ( 0 == strncmp(attrName, "lanDomain", strlen("lanDomain")) )
        {
            if (xmlState.verify)
            {
                /* BEGIN: Modified by y67514, 2008/11/17   问题单号:AU8D01212:网关域名设置为32个字符，网关重起自动恢复出厂设置。*/
                //status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
                status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN + 1);
                /* END:   Modified by y67514, 2008/11/17 */
            }
            if (status == DB_OBJ_VALID_OK)
            {
                strcpy(lanInfo->lanDomain, attrValue);
            }
            else 
            {
                xmlState.errorFlag = PSX_STS_ERR_FATAL;
                syslog(LOG_ERR,"xmlGetLanAttr(): invalid lanDomain name %s\n",attrValue);
            }
        }
        /* END:   Added by y67514, 2008/9/22 */

/*w44771 add for 第一IP支持5段地址池，end*/
 
  if (strncmp(attrName,"instanceId",strlen("instanceId")) == 0) {
      lanInfo->id = atoi(attrValue);
  }
	/* start of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
	if(0 == strncmp(attrName, "lanDns1", strlen("lanDns1")))
	{
		lanInfo->lanDnsAddress1.s_addr = inet_addr(attrValue);
	}
	if(0 == strncmp(attrName, "lanDns2", strlen("lanDns2")))
	{
		lanInfo->lanDnsAddress2.s_addr = inet_addr(attrValue);
	}
	/* end of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
	/* j00100803 Add Begin 2008-05-20 for option43 */
	if(0 == strncmp(attrName, "opt43", strlen("opt43")))
	{
		strcpy(lanInfo->szLanOption43, attrValue);
	}
	/* j00100803 Add End 2008-05-20 for option43 */
    /*start of support to configure the option66,67,160 by l129990,2009,12,22*/
    if(0 == strncmp(attrName, "opt66", strlen("opt66")))
	{
         if (xmlState.verify)
         {
              status = BcmDb_validateLength(attrValue, DHCP_CLASS_ID_MAX);  
         }
         if (status == DB_OBJ_VALID_OK)
         {
              strcpy(lanInfo->szLanOption66, attrValue);
         }
         else 
         {
              xmlState.errorFlag = PSX_STS_ERR_FATAL;
              syslog(LOG_ERR,"xmlGetLanAttr(): invalid option66 name %s\n",attrValue);
         }
    }
    
    if(0 == strncmp(attrName, "opt67", strlen("opt67")))
	{
         if (xmlState.verify)
         {
              status = BcmDb_validateLength(attrValue, DHCP_CLASS_ID_MAX);  
         }
         if (status == DB_OBJ_VALID_OK)
         {
              strcpy(lanInfo->szLanOption67, attrValue);
         }
         else 
         {
              xmlState.errorFlag = PSX_STS_ERR_FATAL;
              syslog(LOG_ERR,"xmlGetLanAttr(): invalid option67 name %s\n",attrValue);
         }
    }
    
    if(0 == strncmp(attrName, "opt160", strlen("opt160")))
	{
         if (xmlState.verify)
         {
              status = BcmDb_validateLength(attrValue, DHCP_CLASS_ID_MAX);  
         }
         if (status == DB_OBJ_VALID_OK)
         {
              strcpy(lanInfo->szLanOption160, attrValue);
         }
         else 
         {
              xmlState.errorFlag = PSX_STS_ERR_FATAL;
              syslog(LOG_ERR,"xmlGetLanAttr(): invalid option120 name %s\n",attrValue);
         }
    }
    /*end of support to configure the option66,67,160 by l129990,2009,12,22*/
    /*start of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
    if (0 == strncmp(attrName, "addrType", strlen("addrType")))
    {
        if (xmlState.verify)
            status = BcmDb_validateNumber(attrValue);
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetLanAttr(): invalid addrType %s\n",attrValue);
        }
        else 
        {
            lanInfo->addressType = atoi(attrValue);
        }

    }
    /*end of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartLanObj(char *appName, char *objName)
{
  PIFC_LAN_INFO lanInfo;
#ifdef ETH_CFG
  PETH_CFG_INFO ethInfo;
#endif
  int lanId;

#ifdef XML_DEBUG
  printf("xmlStartLanObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

#ifdef ETH_CFG
  if (strncmp(objName,PSX_OBJ_ETHCFG, strlen(PSX_OBJ_ETHCFG)) == 0) {
    lanId = atoi(objName + strlen(PSX_OBJ_ETHCFG));
    psiValue = (void*)malloc(sizeof(ETH_CFG_INFO));
    /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
    }
    memset(psiValue, 0, sizeof(ETH_CFG_INFO));
    /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    ethInfo = (PETH_CFG_INFO)psiValue;
    ethInfo->ethSpeed = atoi(BcmDb_getDefaultValue("ethSpeed"));
    ethInfo->ethType = atoi(BcmDb_getDefaultValue("ethType"));
    ethInfo->ethMtu = atoi(BcmDb_getDefaultValue("ethMtu"));
    return PSX_STS_OK;
  }
  else {
#endif
    sscanf(objName,OBJNAME_LAN_ENTRY_FORMAT,&lanId);
    psiValue = (void*)malloc(sizeof(IFC_LAN_INFO));
    /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
    }
    memset(psiValue, 0, sizeof(IFC_LAN_INFO));
    /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    lanInfo = (PIFC_LAN_INFO)psiValue;
#ifdef ETH_CFG
  }
#endif

  if ( lanId == IFC_ENET_ID ) {
    lanInfo->lanAddress.s_addr = inet_addr(BcmDb_getDefaultValue("ethIpAddress"));
    lanInfo->lanMask.s_addr = inet_addr(BcmDb_getDefaultValue("ethSubnetMask"));
    lanInfo->dhcpSrv.enblDhcpSrv = atoi(BcmDb_getDefaultValue("enblDhcpSrv"));
    lanInfo->dhcpSrv.leasedTime = atoi(BcmDb_getDefaultValue("dhcpLeasedTime"));
    /*start of support to the dhcp relay  function by l129990,2009,11,12*/
    lanInfo->dhcpRelay = atoi(BcmDb_getDefaultValue("dhcpRelay"));
    /*end of support to the dhcp relay  function by l129990,2009,11,12*/
    lanInfo->dhcpSrv.startAddress.s_addr = inet_addr(BcmDb_getDefaultValue("dhcpEthStart"));
    lanInfo->dhcpSrv.endAddress.s_addr = inet_addr(BcmDb_getDefaultValue("dhcpEthEnd"));
	#ifdef	VDF_RESERVED
	memset(lanInfo->dhcpSrv.reservedAddress,0,DHCP_RESERVED_MAXCHAR);
	strcpy(lanInfo->dhcpSrv.reservedAddress,BcmDb_getDefaultValue("ReservedIpstr"));
	#endif
    lanInfo->addressType = atoi(BcmDb_getDefaultValue("addrType"));
	/*start of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
	lanInfo->lanDnsAddress1.s_addr = inet_addr("192.168.1.1");
	/* start of VDF 2008.6.19V100R001C02B018 j00100803AU8D00742 */
	lanInfo->lanDnsAddress2.s_addr = inet_addr("192.168.1.1");
	/* start of VDF 2008.6.19V100R001C02B018 j00100803AU8D00742 */
	/*end of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
        /* BEGIN: Added by y67514, 2008/9/22   PN:GLB:lan dns*/
        strcpy(lanInfo->lanDomain,BcmDb_getDefaultValue("lanDomain"));
        /* END:   Added by y67514, 2008/9/22 */
    /*start of support to configure the option66,67,160 by l129990,2009,12,22*/
        strcpy(lanInfo->szLanOption66,BcmDb_getDefaultValue("lanOption66"));
        strcpy(lanInfo->szLanOption67,BcmDb_getDefaultValue("lanOption67"));
        strcpy(lanInfo->szLanOption160,BcmDb_getDefaultValue("lanOption160"));
    /*end of support to configure the option66,67,160 by l129990,2009,12,22*/
  }
  else {
    memset(lanInfo,0,sizeof(IFC_LAN_INFO));
  }

  return PSX_STS_OK;
}
/* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add begin:*/
/*------------------------------------------------------------
  函数原型: BOOL BcmCli_isSameSubNet(char *Ip1, char *Mask1, char *Ip2, char *Mask2)
  描述: 检查Ip1和Ip2是否在同一网段
  输入: 第一和第二个IP和子网掩码
  输出: 无
  返回值: 是则返回CLI_TRUE，否则返回CLI_FALSE
-------------------------------------------------------------*/
BOOL xmlisSameSubNet(char *Ip1, char *Mask1, char *Ip2, char *Mask2)
{
    int count = 0;
    int i = 0;
    int p1, p2, p3, p4;
    char *pTokenIp1[4];
    char *pTokenMask1[4];
    char *pTokenIp2[4];
    char *pTokenMask2[4];
    char *pLastIp1 = NULL;
    char *pLastMask1 = NULL;
    char *pLastIp2 = NULL;
    char *pLastMask2 = NULL;
    char bufIp1[IFC_TINY_LEN];
    char bufMask1[IFC_TINY_LEN];
    char bufIp2[IFC_TINY_LEN];
    char bufMask2[IFC_TINY_LEN];
    strcpy(bufIp1, Ip1);
    strcpy(bufMask1, Mask1);
    strcpy(bufIp2, Ip2);
    strcpy(bufMask2, Mask2);
    
    pTokenIp1[0] = strtok_r(bufIp1, ".", &pLastIp1);
    for (i = 1; i <= 3; i++)
    {
        pTokenIp1[i] = strtok_r(NULL, ".", &pLastIp1);
    }
    
    pTokenMask1[0] = strtok_r(bufMask1, ".", &pLastMask1);
    for (i = 1; i <= 3; i++)
    {
        pTokenMask1[i] = strtok_r(NULL, ".", &pLastMask1);
    }
  
    pTokenIp2[0] = strtok_r(bufIp2, ".", &pLastIp2);
    for (i = 1; i <= 3; i++)
    {
        pTokenIp2[i] = strtok_r(NULL, ".", &pLastIp2);
    }
    
    pTokenMask2[0] = strtok_r(bufMask2, ".", &pLastMask2);
    for (i = 1; i <= 3; i++)
    {
        pTokenMask2[i] = strtok_r(NULL, ".", &pLastMask2);
    }
   
    for (i = 0; i <= 3; i++)
    {
        p1 = atoi(pTokenIp1[i]);
        p2 = atoi(pTokenMask1[i]);
        p3 = atoi(pTokenIp2[i]);
        p4 = atoi(pTokenMask2[i]);
        if ((p1 & p2) == (p3 & p4))
        {
            count++;
        }
    }
   
    if (count > 3)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add end.*/
PSX_STATUS xmlEndLanObj(char *appName, char *objName)
{
  PIFC_LAN_INFO lanInfo = (PIFC_LAN_INFO)psiValue;
#ifdef ETH_CFG
  PETH_CFG_INFO ethInfo = NULL;
#endif
  int error = 0, lanId = 0;
  in_addr_t ipZero = inet_addr("0.0.0.0");
  /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add begin:*/
  char  ipaddr[IP_LEN];
  char ipmask[IP_LEN];
  char ipbegin[IP_LEN];
  char ipend[IP_LEN];
  /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add end.*/
#ifdef XML_DEBUG
  printf("xmlEndLandObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif 
  /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add begin.*/
  memset(ipaddr,0, IP_LEN);
  memset(ipmask,0, IP_LEN);
  memset(ipbegin,0, IP_LEN);
  memset(ipend,0, IP_LEN);
  /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add end.*/
#ifdef ETH_CFG
  if (strncmp(objName,PSX_OBJ_ETHCFG, strlen(PSX_OBJ_ETHCFG)) == 0) {
    lanId = atoi(objName + strlen(PSX_OBJ_ETHCFG));
    ethInfo = (PETH_CFG_INFO)psiValue;
  }
  else
#endif
    sscanf(objName,OBJNAME_LAN_ENTRY_FORMAT,&lanId);

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
#ifdef ETH_CFG
    if (lanId < ETH_CFG_ID || lanId >= ETH_CFG_ID + IFC_SUBNET_ID) {
#endif
    if ((memcmp((void*)&(lanInfo->lanAddress.s_addr),(void*)&ipZero,sizeof(in_addr_t)) == 0)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndLandObj(): address must be specified.\n");
      error = 1;
    }
    /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add begin.*/
    strcpy(ipbegin,inet_ntoa(lanInfo->dhcpSrv.startAddress));
    strcpy(ipmask,inet_ntoa(lanInfo->lanMask));
    strcpy(ipaddr,inet_ntoa(lanInfo->lanAddress));
    strcpy(ipend,inet_ntoa(lanInfo->dhcpSrv.endAddress));
    /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add end.*/
    if ((memcmp(&(lanInfo->lanMask.s_addr),(void*)&ipZero,sizeof(in_addr_t)) == 0)) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndLandObj(): mask must be specified.\n");
      error = 1;
    }
    if (lanInfo->dhcpSrv.enblDhcpSrv != 0) { /* disable */
      if ((memcmp(&(lanInfo->dhcpSrv.startAddress.s_addr),(void*)&ipZero,sizeof(in_addr_t)) == 0) ||
          (memcmp(&(lanInfo->dhcpSrv.endAddress.s_addr),(void*)&ipZero,sizeof(in_addr_t)) == 0)) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlEndLandObj(): dhcpSrv enabled, start and end address  must be specified.\n");
        error = 1;
      }
    } /* dhcpsrv */
    /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add begin.*/
    if (lanInfo->dhcpSrv.enblDhcpSrv == 1)
    {
        if((ipaddr=="")||(ipmask=="")||(ipbegin=="")||(ipend==""))
        {
	      return(PSX_STS_OK);	
        }
        printf("ip is  %smask is  %s, ipbegin is  %s, ipend is  %s\n", ipaddr, ipmask,ipbegin,ipend);
        
        if (!(xmlisSameSubNet(ipaddr, ipmask,ipbegin, ipmask)
            && xmlisSameSubNet(ipaddr, ipmask, ipend, ipmask)))
        {
	      xmlState.errorFlag = PSX_STS_ERR_FATAL;
             syslog(LOG_ERR,"xmlGetLanAttr(): invalid ipdAddr\n");
          
        }
    }
    /* HUAWEI HGW s53329 2006年5月16?问题单号：A36D01125 add end.*/
#ifdef ETH_CFG
    }
#endif
  }

  if (!error && !xmlState.verify) {
#ifdef ETH_CFG
    if (lanId >= ETH_CFG_ID && lanId < ETH_CFG_ID + IFC_SUBNET_ID)
      BcmDb_setEthInfoEx(ethInfo, (lanId - ETH_CFG_ID) + IFC_ENET_ID);
    else 
#endif
      BcmDb_setLanInfo(lanId,lanInfo);
  }

#ifdef XML_DEBUG1
#ifdef ETH_CFG
  if (lanId >= ETH_CFG_ID && lanId < ETH_CFG_ID + IFC_SUBNET_ID) {
    printf("\n============End Eth Cfg=========\n");
    printf("xmlEndLanObj(): speed %d, type %d, mtu %d\n",
           ethInfo->ethSpeed,ethInfo->ethType,ethInfo->ethMtu);
    printf("===============================================\n");
  } 
  else {
#endif
    printf("\n============End LAN=========\n");
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
    printf("xmlEndLanObj(): address %s, mask %s, dhcpSrvState %d, leasedTime %d,dhcpRelay %d,start %s, end %s\n",
           inet_ntoa(lanInfo->lanAddress),inet_ntoa(lanInfo->lanMask),
           lanInfo->dhcpSrv.enblDhcpSrv,lanInfo->dhcpSrv.leasedTime,lanInfo->dhcpRelay,
           inet_ntoa(lanInfo->dhcpSrv.startAddress),inet_ntoa(lanInfo->dhcpSrv.endAddress));
/*end of support to the dhcp relay  function by l129990,2009,11,12*/
    printf("===============================================\n");
#ifdef ETH_CFG
  }
#endif
#endif

  free(psiValue);
  psiValue = NULL;

  return PSX_STS_OK;
}

 /*start of VDF  2008.3.28 HG553V100R001 w45260:AU8D00327 Wlan配置项错误导致网关配置会恢复出厂配置.修改:提供出错默认配置，避免网关配置项的整体恢复*/

#ifdef WIRELESS
/* wireless */
PSX_STATUS xmlGetWirelessObjNode(char *appName, char *objName, char *attrName,
                                 char* attrValue)
{
  int i;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetWirelessObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif
strcpy(app_name, appName);
  for ( i = 1; wirelessObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(wirelessObjs[i].objName, objName, strlen(wirelessObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(wirelessObjs[i].getAttrFnc) != NULL) {
	sts = (*(wirelessObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("\nxmlGetWirelessObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

  }
  return sts;
} /* xmlGetWirelessObjNode */

PSX_STATUS xmlStartWirelessObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlStartWirelessObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; wirelessObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(wirelessObjs[i].objName, objName, strlen(wirelessObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(wirelessObjs[i].startObjFnc) != NULL) {
	sts = (*(wirelessObjs[i].startObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("\nxmlStartWirelessObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

PSX_STATUS xmlEndWirelessObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlEndObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; wirelessObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(wirelessObjs[i].objName, objName, strlen(wirelessObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(wirelessObjs[i].endObjFnc) != NULL) {
	sts = (*(wirelessObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("\nxmlEndWirelessObjNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

int xmlWirelessGenOnOffAutoToPsi(char *value, int on, int off, int automatic)
{
  if (strcasecmp("on",value) == 0)
    return on;	
  else if (strcasecmp("off",value) == 0)
    return off;
  else if (strcasecmp("auto",value) == 0)
    return automatic;
  else 
    return off;
	
}

int xmlWirelessAPToPsi(char *value) 
{
  if (strcasecmp("off",value) == 0)
    return 0;
  else 
    return 1;
}

int xmlWirelessBandToPsi(char *value) 
{
  if (strcasecmp("auto",value) == 0)
    return 0;
  else if (strcasecmp("a",value) == 0)
    return 1;
  else
    return 2;
}

int xmlWirelessGmodeToPsi(char *value) 
{
  if (strcasecmp("auto",value) == 0)
    return WL_MODE_G_AUTO;
  else if (strcasecmp("performance",value) == 0)
    return WL_MODE_G_PERFORMANCE;
  else if (strcasecmp("lrs",value) == 0)
    return WL_MODE_G_LRS;
  /* BEGIN: Modified by c106292, 2008/9/18   PN:Global*/
  else if (strcasecmp("802.11ng",value) == 0)
    return WL_MODE_NG;
  else if (strcasecmp("802.11na",value) == 0)
    return WL_MODE_NA;
  /* END:   Modified by c106292, 2008/9/18 */
  else
    return WL_MODE_B_ONLY;

}

int xmlWirelessBridgeRestrictModeToPsi(char *value) 
{
  if (strcasecmp("enabled",value) == 0)
    return WL_BRIDGE_RESTRICT_ENABLE;
  else if (strcasecmp("enabled_scan",value) == 0)
    return WL_BRIDGE_RESTRICT_ENABLE_SCAN;
  else
    return WL_BRIDGE_RESTRICT_DISABLE;
}

int xmlWirelessWlKeyBitToPsi(char *value) 
{
  if (strcasecmp("64-bit",value) == 0)
    return WL_BIT_KEY_64;
  else 
    return WL_BIT_KEY_128;
}

int xmlWirelessWlNctrlsbToPsi(char *value) 
{
  if (strcasecmp("lower",value) == 0)
    return -1;
  else if (strcasecmp("upper",value) == 0)
    return 1;
  else    
    return 0;
}
/* wl_add_var_check */
PSX_STATUS xmlGetWirelessVarsAttr(char *attrName, char* attrValue)
{
  int len = 0, index;
  PWIRELESS_VAR wlVars = (PWIRELESS_VAR)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int wl_ap_index = 0;

#ifdef XML_DEBUG
  printf("xmlGetWirelessVarsAttr(attrName %s, attrValue %s)\n", attrName, attrValue);
#endif

#if 0 //debug error
#define syslog(a, fmt, arg...) printf(fmt,## arg)
#endif

  if (strcasecmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid state %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlEnbl );
    }
    wlVars->wlEnbl = xmlStateToPsi(attrValue);
  }
  if (strcasecmp(attrName,"curState") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid state %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlCurEnbl );
    }
    wlVars->wlCurEnbl = xmlStateToPsi(attrValue);
  }  
  if (strcasecmp(attrName,"ssIdIndex") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 0, WL_NUM_SSID-1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
      printf("\nxmlGetWirelessVarsAttr(): invalid ssIdIndex option %s (0 to %d expected)\n",attrValue, WL_NUM_SSID-1);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSsidIdx );
    }
    else
      wlVars->wlSsidIdx= atoi(attrValue);
  }  
  if (strcasecmp(attrName,"country") == 0) {
    if (xmlState.verify) 
    {
  //  printf("\r\napp_name:%s.file[%s] line[%d] ",app_name, __FILE__, __LINE__);

//printf("\r\nwl_ap_index:%d.file[%s] line[%d] ",wl_ap_index, __FILE__, __LINE__);
         if(!strcmp(app_name, WIRELESS_APPID))
	      wl_ap_index = 0;
		 
	 else 
             sscanf(app_name, WIRELESS_APP_FMT, &wl_ap_index);
	// printf("\r\napp_name:%s.file[%s] line[%d] ",app_name, __FILE__, __LINE__);

//printf("\r\nwl_ap_index:%d.file[%s] line[%d] ",wl_ap_index, __FILE__, __LINE__);
	  
//fprintf(stdout,"\r\nattrValue=%s;wl_ap_index=%d:file[%s] line[%d] ",attrValue,wl_ap_index, __FILE__, __LINE__);
       //  status = BcmDb_validateWirelessCountryEx(attrValue, wl_ap_index);
	         status = DB_OBJ_VALID_OK;	 
//fprintf(stdout,"\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
    }
 
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid country %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlCountry );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlCountry, attrValue, len);
      wlVars->wlCountry[len] = '\0';
    }
  } /* country */
  if (strcasecmp(attrName,"apMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessApMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
       // xmlState.errorFlag = PSX_STS_ERR_FATAL;
       printf("\nxmlGetWirelessVarsAttr(): invalid apMode %s (ap/wds)\n",attrValue);
       printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlMode );
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlMode,attrValue,len);
      wlVars->wlMode[len] = '\0';
    }
  } /* wlMode */
  if (strcasecmp(attrName,"bridgeRestrict") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessBridgeRestrictMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid bridgeRestrict %s (enabled, enabled_scan, disabled expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlLazyWds );	  
    }
    else {
      wlVars->wlLazyWds = xmlWirelessBridgeRestrictModeToPsi(attrValue);
    }
  } /* wlLazyWds */
  if (strncasecmp(attrName,"wdsMAC_", 7) == 0) {
    if (sscanf(attrName,"wdsMAC_%d",&index) == 1) {
      if (index >= 0 && index <=3 ) {
        if (attrValue[0] != '\0') {
          if(BcmDb_validateMacAddress(attrValue) == DB_OBJ_VALID_OK) {
            strcpy(wlVars->wlWds[index], attrValue);
          }
          else {
            //xmlState.errorFlag = PSX_STS_ERR_FATAL;
            printf("\nxmlGetWirelessVars(): invalid WDS MAC address %s\n",attrValue);
	     printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlWds[index] );	  
          }
        }
        else {
          strcpy(wlVars->wlWds[index], attrValue);
        }
      }
      else {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        printf("\nxmlGetWirelessVars(): WDS key number out of range\n");	 	
      }
    }
    else {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid WDS key\n");
    }
  }
  /* advance parameters */
  if (strcasecmp(attrName,"band") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessBand(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid band %s (auto/a/b expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): use default band : b\n");	  
      wlVars->wlBand = xmlWirelessBandToPsi("b");
    }
    else {
      wlVars->wlBand = xmlWirelessBandToPsi(attrValue);
    }
  } /* wlBand */
  if (strcasecmp(attrName,"channel") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,14);
    if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
        printf("\nxmlGetWirelessVarsAttr(): invalid channel %s (0-14)\n",attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlChannel);	  
    }
    else {
      wlVars->wlChannel = atoi(attrValue);
    }
  } /* wlChannel */
  
  //BEGIN:add by zhourongfei to periodically check the best channel
  if (strcasecmp(attrName,"channelcheck") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
        printf("\nxmlGetWirelessVarsAttr(): invalid channelcheck %s (0-14)\n",attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlchannelcheck);	  
    }
    else {
      wlVars->wlchannelcheck= atoi(attrValue);
    }
  } /*Channelcheck */
  //END:add by zhourongfei to periodically check the best channel
  
  if (strcasecmp(attrName,"rate") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessRate(attrValue);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;
       printf("\nxmlGetWirelessVarsAttr(): invalid rate %s (expect 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54)\n",attrValue);
	printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlRate );	  
    }
    else {
      wlVars->wlRate = (long int)(atof(attrValue) * 1000000);
    }
  } /* wlRate */
  if (strcasecmp(attrName,"multicastRate") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessRate(attrValue);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;
       printf("\nxmlGetWirelessVarsAttr(): invalid multicastRate %s (expect 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54)\n",attrValue);
       printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlMCastRate );
    }
    else {
      wlVars->wlMCastRate = (long int)(atof(attrValue) * 1000000);
    }
  } /* wlMCastRate */
  if (strcasecmp(attrName,"basicRate") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessBasicRate(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid basicRate %s (expect default/all/12))\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlBasicRate );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlBasicRate,attrValue,len);
      wlVars->wlBasicRate[len] = '\0';
    }
  } /* wlBasicRate */
  if (strcasecmp(attrName,"fragThreshold") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,WL_MIN_FRAGMENTATION_THRESHOLD,WL_MAX_FRAGMENTATION_THRESHOLD);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid fragThreshold %s (expect %d-%d)\n", attrValue,WL_MIN_FRAGMENTATION_THRESHOLD,WL_MAX_FRAGMENTATION_THRESHOLD);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlFrgThrshld );	  
    }
    else {
      wlVars->wlFrgThrshld = atoi(attrValue);
    }
  } /* wlFrgThrshld */
  if (strcasecmp(attrName,"RTSThreshold") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,WL_MIN_RTS_THRESHOLD,WL_MAX_RTS_THRESHOLD);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid RTSThreshold %s (expect %d-%d)\n", attrValue,WL_MIN_RTS_THRESHOLD,WL_MAX_RTS_THRESHOLD);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlRtsThrshld );	  
    }
    else {
      wlVars->wlRtsThrshld = atoi(attrValue);
    }
  } /* wlRtsThrshld */
  if (strcasecmp(attrName,"DTIM") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,WL_MIN_DTIM,WL_MAX_DTIM);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid DTIM %s (expect %d-%d)\n",attrValue,WL_MIN_DTIM,WL_MAX_DTIM);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlDtmIntvl );	  
    }
    else {
      wlVars->wlDtmIntvl = atoi(attrValue);
    }
  } /* wlDtmIntvl */
  if (strcasecmp(attrName,"beacon") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,WL_MIN_BEACON_INTERVAL,WL_MAX_BEACON_INTERVAL);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid beacon %s (expect %d-%d)\n", attrValue,WL_MIN_BEACON_INTERVAL,WL_MAX_BEACON_INTERVAL);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlBcnIntvl );	  
    }
    else {
      wlVars->wlBcnIntvl = atoi(attrValue);
    }
  } /* wlBcnIntvl */
  if (strcasecmp(attrName,"XPress") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessXpress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid XPress state %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlFrameBurst );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlFrameBurst,attrValue,len);
      wlVars->wlFrameBurst[len] = '\0';
    }
  } /* XPress */
  if (strcasecmp(attrName,"AfterBurner") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessAfterBurner(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid Afterburner state %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlAfterBurnerEn );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlAfterBurnerEn,attrValue,len);
      wlVars->wlAfterBurnerEn[len] = '\0';
    }
  } /* AfterBurner */
  if (strcasecmp(attrName,"gMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGmode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid gMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlgMode );	  
    }
    else {
      wlVars->wlgMode = xmlWirelessGmodeToPsi(attrValue);
    }
  } /* wlgMode */
  if (strcasecmp(attrName,"gProtection") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
     // xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid gProtection %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlProtection );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlProtection,attrValue,len);
      wlVars->wlProtection[len] = '\0';
    }
  } /* wlProtection */
  if (strcasecmp(attrName,"preamble") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessPreamble(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid preamble %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlPreambleType );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlPreambleType,attrValue,len);
      wlVars->wlPreambleType[len] = '\0';
    }
  } /* wlProtection */ 
  if (strcasecmp(attrName,"WME") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid wme mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlWme );	  
    }
    else {
      wlVars->wlWme = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,-1);
    }
  } /* wlWme */
  if (strcasecmp(attrName,"WMENoAck") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid wme noack mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlWmeNoAck );	  
    }
    else {
      wlVars->wlWmeNoAck = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } /* wlWmeNoAck */

  /*start of enable or disable the access point radio by l129990,2009-10-9*/
  if (strcasecmp(attrName,"RadioCtl") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid wme noack mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlRadioCtl );	  
    }
    else {
      wlVars->wlRadioCtl = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } 
  /*end of enable or disable the access point radio by l129990,2009-10-9*/
  
  if (strcasecmp(attrName,"globalMaxAssoc") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 1, WL_MAX_ASSOC_STA);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
       printf("\nxmlGetWirelessVarsAttr(): invalid globalMaxAssoc %s \n", attrValue); 
       printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlGlobalMaxAssoc );	  
    }
    else {
      wlVars->wlGlobalMaxAssoc = atoi(attrValue);
    }
  } /* globalMaxAssoc */ 

/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_SES ) || defined (SUPPORT_ATHEROSWLAN)
   if (strcasecmp(attrName,"AutoSwOffdays") == 0) {
      if (xmlState.verify) 
        status = BcmDb_validateNumber(attrValue);
      if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
        printf("\nxmlGetWirelessVarsAttr(): invalid AutoSwOffdays option %s", attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n", wlVars->wlAutoSwOffdays );
      }
      else
        wlVars->wlAutoSwOffdays = atoi(attrValue);
    } /* AutoSwOffdays */
    if (strcasecmp(attrName,"StarOfftTime") == 0) {
      if (xmlState.verify) 
        status = BcmDb_validateRange(attrValue, 0, 86400-1);
      if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
        printf("\nxmlGetWirelessVarsAttr(): invalid StarOfftTime option %s (0 to %d expected)\n", attrValue, 86400-1);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n", wlVars->wlStarOfftTime );
      }
      else
        wlVars->wlStarOfftTime = atoi(attrValue);
    } /* StarOfftTime */
    if (strcasecmp(attrName,"EndOffTime") == 0) {
      if (xmlState.verify) 
        status = BcmDb_validateRange(attrValue, 0, 86400-1);
      if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
        printf("\nxmlGetWirelessVarsAttr(): invalid EndOffTime option %s (0 to %d expected)\n", attrValue, 86400-1);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n", wlVars->wlEndOffTime );
      }
      else
        wlVars->wlEndOffTime = atoi(attrValue);
    }  /* EndOffTime */
#endif  

 
#ifdef SUPPORT_SES  
  if (strcasecmp(attrName,"ses_enable") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid ses_enable %s (expect 0-1)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesEnable );	  
    }
    else {
      wlVars->wlSesEnable = atoi(attrValue);
    }
  }/* wlSesEnable */
  if (strcasecmp(attrName,"ses_event") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,10);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid ses_event %s (expect 0-9)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesEvent );	  
    }
    else {
      wlVars->wlSesEvent = atoi(attrValue);
    }
  }/* wlSesEvent */ 
  if (strcasecmp(attrName,"ses_states") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue, WL_SIZE_8_MAX);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVars(): invalid ses_states %s (expect xx:xx)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesStates );	  
    }
    else {
      strcpy(wlVars->wlSesStates, attrValue);
    }
  }/* wlSesStates */ 
  if (strcasecmp(attrName,"ses_ssid") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateLength(attrValue,WL_SSID_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_ssid %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesSsid );	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSesSsid, attrValue, len);
      wlVars->wlSesSsid[len] = '\0';
    }
  }/* wlSesSsid */  
  if (strcasecmp(attrName,"ses_hide") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ses_hide option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_hide option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesHide );	   
    }
    else
      wlVars->wlSesHide = atoi(attrValue);
  }/* wlSesHide */
  if (strcasecmp(attrName,"ses_wpa_psk") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateLength(attrValue,WL_WPA_PSK_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ses_wpa_psk %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_wpa_psk %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesWpaPsk );	   
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSesWpaPsk, attrValue, len);
      wlVars->wlSesWpaPsk[len] = '\0';
    }
  } /* wlSesWpaPsk */     
  if (strcasecmp(attrName,"ses_auth") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ses_auth %s( 0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_auth %s( 0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesAuth );	   
    }
    else {
      wlVars->wlSesAuth = atoi(attrValue);
    }
  } /* wlSesAuth */  
  if (strcasecmp(attrName,"ses_auth_mode") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateWirelessAuthMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ses_auth_mode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_auth_mode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesAuthMode );	   
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSesAuthMode, attrValue, len);
      wlVars->wlSesAuthMode[len] = '\0';
    }
  } /* wlSesAuthMode */  
  if (strcasecmp(attrName,"ses_wep") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ses_wep %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ses_wep %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesWep );	   
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSesWep, attrValue, len);
      wlVars->wlSesWep[len] = '\0';
      xmlConvertToLowerCase(wlVars->wlSesWep);
    }
  } /* wlSesWep/encryption */  
  if (strcasecmp(attrName,"ses_wpa") == 0) {
    if (xmlState.verify && strcmp(attrValue,"")) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid encryption ses_wpa %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid encryption ses_wpa %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlSesWpa );	   
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSesWpa, attrValue, len);
      wlVars->wlSesWpa[len] = '\0';
      xmlConvertToLowerCase(wlVars->wlSesWpa);
    }
  } /* wlSesWpa/encryption */  
  if (strcasecmp(attrName,"ses_client_enable") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid option ses_client_enable %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid option ses_client_enable %s\n",attrValue);
      printf("xmlGetWirelessVarsAttr(): invalid option ses_client_enable  %s\n",attrValue);      
    }
    else {
      wlVars->wlSesClEnable = atoi(attrValue);
    }
  } /* wlSesClEnable */   
  if (strcasecmp(attrName,"ses_client_event") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,2);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid input ses_client_event %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid input ses_client_event %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesClEvent );	   
    }
    else {
      wlVars->wlSesClEvent = atoi(attrValue);
    }
  } /* wlSesClEvent */  
  if (strcasecmp(attrName,"ses_wds_mode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,4);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid mode ses_wds_mode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid mode ses_wds_mode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlSesWdsMode );	   
    }
    else {
      wlVars->wlSesWdsMode = atoi(attrValue);
    }
  } /* wlSesWdsMode */  
  if (strcasecmp(attrName,"ses_wds_wsec") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_SIZE_132_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid length ses_wds_wsec %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid length ses_wds_wsec %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlWdsWsec);	   
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlWdsWsec, attrValue, len);
      wlVars->wlWdsWsec[len] = '\0';
    }
  } /* wlWdsWsec */         
  
#endif
  if (strcasecmp(attrName,"TxPowerPercent") == 0) {
    if (xmlState.verify) {
      status = BcmDb_validateRange(attrValue, 0, 100);
     }
    if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVars(): invalid TxPowerPercent %s \n",attrValue);
        printf("\nxmlGetWirelessVars(): invalid TxPowerPercent %s \n",attrValue);
	 printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlTxPwrPcnt);	   
    }
    else {
      wlVars->wlTxPwrPcnt = atoi(attrValue);
    }
  } /* wlTxPwrPcnt */   
  if (strcasecmp(attrName,"RegulatoryMode") == 0) {
    const char* mode[] = { "off", "802.11h","802.11d", NULL };
    int index = 0;        	
    if (xmlState.verify)
      status = BcmDb_validateWirelessGenStr(attrValue, mode, &index);

    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid RegulatoryMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid RegulatoryMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlRegMode);	   	  
    }
    else {
      wlVars->wlRegMode = index;
    }
  } /* wlRegMode */ 
  if (strcasecmp(attrName,"PreNetRadarDectTime") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, 99);
 
    if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVars(): invalid PreNetRadarDectTime %s \n",attrValue);
        printf("\nxmlGetWirelessVars(): invalid PreNetRadarDectTime %s \n",attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlDfsPreIsm);	   	     
    }
    else {
      wlVars->wlDfsPreIsm = atoi(attrValue);
    }
  } /* wlDfsPreIsm */   
  if (strcasecmp(attrName,"InNetRadarDectTime") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 10, 99);
 
    if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVars(): invalid InNetRadarDectTime %s \n",attrValue);
        printf("\nxmlGetWirelessVars(): invalid InNetRadarDectTime %s \n",attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlDfsPostIsm);	   	     
    }
    else {
      wlVars->wlDfsPostIsm = atoi(attrValue);
    }
  } /* wlDfsPostIsm */  
  if (strcasecmp(attrName,"TpcMitigation") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, 4);
 
    if (status != DB_OBJ_VALID_OK) {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVars(): invalid TpcMitigation %s \n",attrValue);
        printf("\nxmlGetWirelessVars(): invalid TpcMitigation %s \n",attrValue);
        printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlTpcDb);	   	     
    }
    else {
      wlVars->wlTpcDb = atoi(attrValue);
    }
  } /* wlTpcDb */    
  if (strcasecmp(attrName,"AutoChannelTimer") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid AutoChannelTimer %s \n", attrValue);
      printf("\nxmlGetWirelessVars(): invalid AutoChannelTimer %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlCsScanTimer);	   	     
    }
    else {
      wlVars->wlCsScanTimer = atoi(attrValue);
    }
  } /* wlCsScantimer */  
#ifdef SUPPORT_MIMO
  if (strcasecmp(attrName,"NBandwidthCap") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessBandwidth(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid Bandwidth %s \n", attrValue);
      printf("\nxmlGetWirelessVars(): invalid Bandwidth %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlNBwCap);	   	     
    }
    else {
      wlVars->wlNBwCap = atoi(attrValue);
    }
  } /* wlNBw */ 
  if (strcasecmp(attrName,"NCtrlSideband") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessCtrlSideband(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid ControlSideband %s \n", attrValue);
      printf("\nxmlGetWirelessVars(): invalid ControlSideband %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlNCtrlsb);	   	     
    }
    else {
      wlVars->wlNCtrlsb = xmlWirelessWlNctrlsbToPsi(attrValue);
    }
  } /* wlNCtrlsb */
  if (strcasecmp(attrName,"NBand") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 1, 2);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid NBand %s \n", attrValue);
      printf("\nxmlGetWirelessVars(): invalid NBand %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlNBand);	   	     
    }
    else {
      wlVars->wlNBand = atoi(attrValue);
    }
  } /* wlNBand */
  if (strcasecmp(attrName,"NMCSIdx") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, -2, 32);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid NMCSIdx %s \n", attrValue);
      printf("\nxmlGetWirelessVars(): invalid NMCSIdx %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlNMcsidx);	   	     
    }
    else {
      wlVars->wlNMcsidx = atoi(attrValue);
    }
  } /* NMCSIdx */
  if (strcasecmp(attrName,"NProtection") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid NProtection %s\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid NProtection %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlNProtection);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlNProtection,attrValue,len);
      wlVars->wlNProtection[len] = '\0';
    }
  } /* wlNProtection */
  if (strcasecmp(attrName,"RIFS") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid RIFS %s\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid RIFS %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlRifs);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlRifs,attrValue,len);
      wlVars->wlRifs[len] = '\0';
    }
  } /* wlRifs */
  if (strcasecmp(attrName,"AMPDU") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid AMPDU %s\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid AMPDU %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlAmpdu);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlAmpdu,attrValue,len);
      wlVars->wlAmpdu[len] = '\0';
    }
  } /* wlAmpdu */
  if (strcasecmp(attrName,"AMSDU") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessOnOffAuto(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid AMSDU %s\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid AMSDU %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlAmsdu);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlAmsdu,attrValue,len);
      wlVars->wlAmsdu[len] = '\0';
    }
  } /* wlAmsdu */
  if (strcasecmp(attrName,"NMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessNmode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid NMode %s\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid NMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlVars->wlNmode);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlNmode,attrValue,len);
      wlVars->wlNmode[len] = '\0';
    }
  } /* wlNmode */      
  if (strcasecmp(attrName,"NReqd") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid NReqd mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid NReqd mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %d\n",wlVars->wlNReqd);	   	     
    }
    else {
      wlVars->wlNReqd = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } /* wlNReqd */   
#endif  /* SUPPORT_MIMO */
#ifdef XML_SUPPORT_LEGACY_WL_CONFIG
  /* wl_add_var_check_mbss */
  if (wlMVars == NULL) {
    printf("xmlGetWirelessVarsAttr(): memory not allocated for legacy support\n");
    return PSX_STS_ERR_FATAL;
  }
  if (strcasecmp(attrName,"ssId") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_SSID_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid ssId %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid ssId %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlMVars->wlSsid);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlSsid, attrValue, len);
      wlMVars->wlSsid[len] = '\0';
    }
  }
  else if (strcasecmp(attrName,"authMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessAuthMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.      
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid authMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid authMode %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value %s\n",wlMVars->wlAuthMode);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlAuthMode, attrValue, len);
      wlMVars->wlAuthMode[len] = '\0';
    }
  } /* authMode */
#if defined (SUPPORT_WSC ) || defined (SUPPORT_ATHEROSWLAN)
  else if (strcasecmp(attrName,"wsc_mode") == 0) {
    if (xmlState.verify) {
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wsc_mode, attrValue, len);
      wlMVars->wsc_mode[len]='\0';
    }  
  }
  else if (strcasecmp(attrName,"wsc_config_state") == 0) {
    if (xmlState.verify) {
    }
    else {
      strncpy(wlMVars->wsc_config_state, attrValue, len);
      wlMVars->wsc_config_state[len]='\0';
    }
  }
 #endif
  else if (strcasecmp(attrName,"radiusServerIP") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid radiusServerIP %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid radiusServerIP %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :0.0.0.0\n");	   	     
    }
    else {
      wlMVars->wlRadiusServerIP.s_addr = inet_addr(attrValue);
    }
  }
  else if (strcasecmp(attrName,"radiusServerPort") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      wlMVars->wlRadiusPort = atoi(attrValue);
    else {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid radiusServerPort %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid radiusServerPort %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%d\n",wlMVars->wlRadiusPort);	   	     
    }
  }
  if (strcasecmp(attrName,"radiusServerKey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_RADIUS_KEY_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid radiusServerKey %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid radiusServerKey %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%s\n",wlMVars->wlRadiusKey);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlRadiusKey, attrValue, len);
      wlMVars->wlRadiusKey[len] = '\0';
    }
  } /* wlRadiusKey */
  if (strcasecmp(attrName,"wep") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid encryption wep %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid encryption wep %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%s\n",wlMVars->wlWep);	   	     
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlWep, attrValue, len);
      wlMVars->wlWep[len] = '\0';
      xmlConvertToLowerCase(wlMVars->wlWep);
    }
  } /* wlWep/encryption */
  if (strcasecmp(attrName,"wpa") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid encryption wpa %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid encryption wpa %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%s\n",wlMVars->wlWpa);	   	     	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlWpa, attrValue, len);
      wlMVars->wlWpa[len] = '\0';
      xmlConvertToLowerCase(wlMVars->wlWpa);
    }
  } /* wlWpa/encryption */
  if (strcasecmp(attrName,"key64Index") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,WL_KEY_NUM);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid key64Index %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid key64Index %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%d\n",wlMVars->wlKeyIndex64);	   	     	  
    }
    else {
      wlMVars->wlKeyIndex64 = atoi(attrValue);
    }
  } /* key64index */
  if (strcasecmp(attrName,"key128Index") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,WL_KEY_NUM);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid key128Index %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid key128Index %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%d\n",wlMVars->wlKeyIndex128);	   	     	  
    }
    else {
      wlMVars->wlKeyIndex128 = atoi(attrValue);
    }
  } /* key128index */
  if (strcasecmp(attrName,"keyBit") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessKeybit(attrValue);
    if (status != DB_OBJ_VALID_OK) {
     // xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid keyBit %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid keyBit %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value :%d\n",wlMVars->wlKeyBit);	   	     	  
    }
    else {
      wlMVars->wlKeyBit = xmlWirelessWlKeyBitToPsi(attrValue);
    }
  } /* keybit strength */
  if (strncasecmp(attrName,"key64_",strlen("key64_")) == 0) {
    if ((xmlState.verify) && (strlen(attrValue) != 0))
      status = BcmDb_validateWirelessLength(attrValue,WL_KEY64_SIZE_CHAR,WL_KEY64_SIZE_HEX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY64_SIZE_HEX,WL_KEY64_SIZE_CHAR);
      printf("\nxmlGetWirelessVarsAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY64_SIZE_HEX,WL_KEY64_SIZE_CHAR);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value \n");	   	     	  
    }
    else {
      sscanf(attrName,"key64_%d",&index);
      if ((index > 0) && (index <= WL_KEY_NUM)) {
        len = strlen(attrValue);
        strncpy(wlMVars->wlKeys64[index-1], attrValue, len);
        wlMVars->wlKeys64[index-1][len] = '\0';
      }
      else {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid keys64_%d (expect keys64_1 to wlKeys64_4)\n",index);
        printf("\nxmlGetWirelessVarsAttr(): invalid keys64_%d (expect keys64_1 to wlKeys64_4)\n",index);
      }
    } /* key length ok */
  }  /* WL_BIT_KEY_64 */
  if (strncasecmp(attrName,"key128_",strlen("key128_")) == 0) {
    if ((xmlState.verify) && (strlen(attrValue) != 0))
      status = BcmDb_validateWirelessLength(attrValue,WL_KEY128_SIZE_CHAR,WL_KEY128_SIZE_HEX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY128_SIZE_HEX,WL_KEY128_SIZE_CHAR);
      printf("\nxmlGetWirelessVarsAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY128_SIZE_HEX,WL_KEY128_SIZE_CHAR);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value \n");	   	     	  
    }
    else {
      sscanf(attrName,"key128_%d",&index);
      if ((index > 0) && (index <= WL_KEY_NUM)) {
        len = strlen(attrValue);
        strncpy(wlMVars->wlKeys128[index-1], attrValue, len);
        wlMVars->wlKeys128[index-1][len] = '\0';
      }
      else {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid wlKeys128_%d (expect wlKeys128_1 to wlKeys128_4)\n",index);
        printf("\nxmlGetWirelessVarsAttr(): invalid wlKeys128_%d (expect wlKeys128_1 to wlKeys128_4)\n",index);
      }
    } /* key length ok */
  }  /* WL_BIT_KEY_128 */
  if (strcasecmp(attrName,"wpaRekey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid wpaRekey %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid wpaRekey %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlWpaGTKRekey);	   	     	  
    }
    else
      wlMVars->wlWpaGTKRekey = atoi(attrValue);
  } /* wlWpaGTKRekey */
  if (strcasecmp(attrName,"wpakey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_WPA_PSK_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid key %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid key %s\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%s \n",wlMVars->wlWpaPsk);	
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlWpaPsk, attrValue, len);
      wlMVars->wlWpaPsk[len] = '\0';
    }
  } /* wlWpaPsk */
  if (strcasecmp(attrName,"auth") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid auth %s( 0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid auth %s( 0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlAuth);	
    }
    else {
      wlMVars->wlAuth = atoi(attrValue);
    }
  } /* wlAuth */  
  if (strcasecmp(attrName,"Preauthentication") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVars(): invalid preauth mode %s expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVars(): invalid preauth mode %s expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlPreauth);	
    }
    else {
      wlMVars->wlPreauth = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } /* wlPreauth */
  if (strcasecmp(attrName,"ReauthTimeout") == 0) {
    if (xmlState.verify) {
      status = DB_OBJ_VALID_OK; //whatever range;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid wlNetReauth mode %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid wlNetReauth mode %s \n", attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlNetReauth);	
    }
    else {
      wlMVars->wlNetReauth = atoi(attrValue);
    }
  } /* wlNetReauth */  
  /* MBSS */
  if (strcasecmp(attrName,"fltMacMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessFltMacMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid fltMacMode %s (off/allow/deny expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid fltMacMode %s (off/allow/deny expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%s \n",wlMVars->wlFltMacMode);	
    }
    else {
      len = strlen(attrValue);
      strncpy(wlMVars->wlFltMacMode, attrValue, len);
      wlMVars->wlFltMacMode[len] = '\0';
    }
  } /* wlFltMacMode */
  if (strcasecmp(attrName,"hide") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid hide option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid hide option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlHide);	
    }
    else
      wlMVars->wlHide = atoi(attrValue);
  }
  if (strcasecmp(attrName,"apIsolation") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessAP(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid apIsolation %s (off/on expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr(): invalid apIsolation %s (off/on expected)\n",attrValue);
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlAPIsolation);	
    }
    else {
      wlMVars->wlAPIsolation = xmlWirelessAPToPsi(attrValue);
    }
  } /* wlAPIsolation */
  if (strcasecmp(attrName,"MaxAssoc") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 1, WL_MAX_ASSOC_STA);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;  //use default value ,Not to report Error to avoiding  total configure restoring.
      syslog(LOG_ERR,"xmlGetWirelessVarsAttr(): invalid MaxAssoc %s \n", attrValue);      
      printf("\nxmlGetWirelessVarsAttr(): invalid MaxAssoc %s \n", attrValue);  
      printf("\nxmlGetWirelessVarsAttr():Use DbDefaultTable[] default value:%d \n",wlMVars->wlMaxAssoc);		  
    }
    else {
      wlMVars->wlMaxAssoc = atoi(attrValue);
    }
  } /* wlMaxAssoc */    
#endif
  
#if 0
#undef syslog
#endif
  return(PSX_STS_OK);
}

/* BEGIN: Modified by c106292, 2008/10/8   PN:Global*/
#ifdef SUPPORT_ATHEROSWLAN
/* these code are duplicates of codes in main/wlmngr/wlmngr.cpp; they needs to be
   moved out to a common routine that can be called everywhere */
char *xmlWirelessGetPhyType(void) {
   char  *phytype = WL_PHY_TYPE_N;

   // get phytype
   return phytype;
}
int xmlWirelessGetCoreRev(void) {
   int  corerev = 3; //待修改
   // get core revision
   return corerev;
}

int xmlWirelessGetBands()
{

  int bands = 0;
  bands |= BAND_B;
  return bands;
}

int xmlWirelessGetValidBand(int band)
{
  int vbands = xmlWirelessGetBands();
  if ((vbands & BAND_A) && (vbands & BAND_B)) {
    if (!(band == 1 || band == 2)) {
      band = 1; // 2.4G
    }
  }
  else if (vbands & BAND_B) {
    if (!(band == 1)) {
      band = 1; // 2.4G
    }
  }
  else if (vbands & BAND_A) {
    if (!(band == 2)) {
      band = 2; // 5G
    }
  }
  return band;
}

#else  //Broadcom Wlan
char *xmlWirelessGetPhyType(void) {
   char  *phytype = WL_PHY_TYPE_B;
   char cmd[WL_LG_SIZE_MAX];
   char *dp = NULL;
   FILE *fp = NULL;

   // get phytype
   bcmSystemMute("wlctl phytype > /var/wl");
   // parse the phytype
   fp = fopen("/var/wl", "r");
   if ( fp != NULL ) {
      if (fgets(cmd, WL_LG_SIZE_MAX-1, fp)) {
         dp = strstr(cmd, "phytype is ");
         if (dp) {
            dp = dp + strlen("phytype is ");
            switch(atoi(dp)) {
               case 0:
                  phytype = WL_PHY_TYPE_A;
                  break;
               case 1:
                  phytype = WL_PHY_TYPE_B;
                  break;
               case 2:
                  phytype = WL_PHY_TYPE_G;
                  break;
            }
         }
      }
      fclose(fp);
   }
   return phytype;
}
int xmlWirelessGetCoreRev(void) {
   int  corerev = 3;
   char cmd[WL_LG_SIZE_MAX];
   char *dp = NULL;
   FILE *fp = NULL;

   // get core revision
   bcmSystemMute("wlctl corerev > /var/wl");
   // parse the core revision
   fp = fopen("/var/wl", "r");
   if ( fp != NULL ) {
      if (fgets(cmd, WL_LG_SIZE_MAX-1, fp)) {
         dp = strstr(cmd, "corerev is ");
         if (dp) {
            dp = dp + strlen("corerev is ");
            corerev = atoi(dp);
         }
      }
      fclose(fp);
   }
   return corerev;
}

int xmlWirelessGetBands()
{
  char buf[WL_LG_SIZE_MAX];
  int bands = 0;

  bcmSystemMute("wlctl bands > /var/wlbands");
  FILE *fp = fopen("/var/wlbands", "r");
  if ( fp != NULL ) {
    if (fgets(buf, sizeof(buf), fp)) {
      if (strchr(buf, 'a')) {
        bands |= BAND_A;
      }
      if (strchr(buf, 'b')) {
        bands |= BAND_B;
      }
    }
  }
  return bands;
}

int xmlWirelessGetValidBand(int band)
{
  int vbands = xmlWirelessGetBands();
  if ((vbands & BAND_A) && (vbands & BAND_B)) {
    if (!(band == 1 || band == 2)) {
      band = 1; // 2.4G
    }
  }
  else if (vbands & BAND_B) {
    if (!(band == 1)) {
      band = 1; // 2.4G
    }
  }
  else if (vbands & BAND_A) {
    if (!(band == 2)) {
      band = 2; // 5G
    }
  }
  return band;
}

#endif
/* END:   Modified by c106292, 2008/10/8 */
PSX_STATUS xmlStartWirelessVarsObj(char *appName, char *objName)
{
  PWIRELESS_VAR wlVars;

#ifdef XML_DEBUG
   printf("xmlStartWirelessVarsObj(calling cfm to get appName %s, objName %s)\n", appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(WIRELESS_VAR));  
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  wlVars = (PWIRELESS_VAR)psiValue;
  memset(wlVars,0,sizeof(WIRELESS_VAR));
   wlVars->wlSsidIdx = atoi(BcmDb_getDefaultValue("wlSsidIdx"));
  strcpy(wlVars->wlCountry, BcmDb_getDefaultValue("wlCountry"));
  strcpy(wlVars->wlPreambleType, BcmDb_getDefaultValue("wlPreambleType"));
  strcpy(wlVars->wlWds[0], BcmDb_getDefaultValue(""));
  strcpy(wlVars->wlWds[1], BcmDb_getDefaultValue(""));
  strcpy(wlVars->wlWds[2], BcmDb_getDefaultValue(""));
  strcpy(wlVars->wlWds[3], BcmDb_getDefaultValue(""));
  strcpy(wlVars->wlMode, BcmDb_getDefaultValue("wlMode"));
  wlVars->wlLazyWds = atoi(BcmDb_getDefaultValue("wlLazyWds"));
  wlVars->wlEnbl = atoi(BcmDb_getDefaultValue("wlEnbl"));
  wlVars->wlCurEnbl = atoi(BcmDb_getDefaultValue("wlCurEnbl"));
  wlVars->wlChannel = atoi(BcmDb_getDefaultValue("wlChannel"));
  //BEGIN:add by zhourongfei to periodically check the best channel
  wlVars->wlchannelcheck= atoi(BcmDb_getDefaultValue("wlchannelcheck"));
  //END:add by zhourongfei to periodically check the best channel
  wlVars->wlFrgThrshld = atoi(BcmDb_getDefaultValue("wlFrgThrshld"));
  wlVars->wlRtsThrshld = atoi(BcmDb_getDefaultValue("wlRtsThrshld"));
  wlVars->wlDtmIntvl = atoi(BcmDb_getDefaultValue("wlDtmIntvl"));
  wlVars->wlBcnIntvl = atoi(BcmDb_getDefaultValue("wlBcnIntvl"));
  strcpy(wlVars->wlFrameBurst, BcmDb_getDefaultValue("wlFrameBurst"));
  strcpy(wlVars->wlFltMacMode, BcmDb_getDefaultValue("wlFltMacMode"));
  wlVars->wlRate = atol(BcmDb_getDefaultValue("wlRate"));
  strcpy(wlVars->wlPhyType,BcmDb_getDefaultValue("wlPhyType"));
  strcpy(wlVars->wlBasicRate, BcmDb_getDefaultValue("wlBasicRate"));
  wlVars->wlgMode = atoi(BcmDb_getDefaultValue("wlgMode"));
  strcpy(wlVars->wlProtection, BcmDb_getDefaultValue("wlProtection"));
  wlVars->wlCoreRev = xmlWirelessGetCoreRev();
  wlVars->wlBand = xmlWirelessGetValidBand(atoi(BcmDb_getDefaultValue("wlBand")));
  wlVars->wlMCastRate = atoi(BcmDb_getDefaultValue("wlMCastRate"));
  strcpy(wlVars->wlAfterBurnerEn, BcmDb_getDefaultValue("wlAfterBurnerEn"));
  wlVars->ezc_version = atoi(BcmDb_getDefaultValue("ezc_version"));
  wlVars->ezc_enable = atoi(BcmDb_getDefaultValue("ezc_enable"));
  wlVars->wlInfra = atoi(BcmDb_getDefaultValue("wlInfra"));
  wlVars->wlAntDiv = atoi(BcmDb_getDefaultValue("wlAntDiv"));
  wlVars->wlWme = atoi(BcmDb_getDefaultValue("wlWme")); 
  wlVars->wlWmeNoAck = atoi(BcmDb_getDefaultValue("wlWmeNoAck")); 
  /*start of enable or disable the access point radio by l129990,2009-10-9*/
  wlVars->wlRadioCtl = atoi(BcmDb_getDefaultValue("wlRadioCtl"));
  /*end of enable or disable the access point radio by l129990,2009-10-9*/
  wlVars->wlWmeApsd = atoi(BcmDb_getDefaultValue("wlWmeApsd"));
  wlVars->wlGlobalMaxAssoc = atoi(BcmDb_getDefaultValue("wlGlobalMaxAssoc"));  

#ifdef SUPPORT_WLAN_PRNTCTL  
  wlVars->wlAutoSwOffdays = atoi(BcmDb_getDefaultValue("wlAutoSwOffdays"));   
  wlVars->wlStarOfftTime = atoi(BcmDb_getDefaultValue("wlStarOfftTime"));
  wlVars->wlEndOffTime = atoi(BcmDb_getDefaultValue("wlEndOffTime"));  
#endif
/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_SES ) || defined (SUPPORT_ATHEROSWLAN)
  wlVars->wlSesEnable = atoi(BcmDb_getDefaultValue("wlSesEnable")); 
  wlVars->wlSesEvent = atoi(BcmDb_getDefaultValue("wlSesEvent"));
  strcpy(wlVars->wlSesStates, BcmDb_getDefaultValue("wlSesStates"));       
  strcpy(wlVars->wlSesSsid, BcmDb_getDefaultValue("wlSesSsid"));  
  strcpy(wlVars->wlSesWpaPsk, BcmDb_getDefaultValue("wlSesWpaPsk"));  
   wlVars->wlSesHide = atoi(BcmDb_getDefaultValue("wlSesHide"));      
  wlVars->wlSesAuth = atoi(BcmDb_getDefaultValue("wlSesAuth"));   
  strcpy(wlVars->wlSesAuthMode, BcmDb_getDefaultValue("wlSesAuthMode"));  
  strcpy(wlVars->wlSesWep, BcmDb_getDefaultValue("wlSesWep"));  
  strcpy(wlVars->wlSesWpa, BcmDb_getDefaultValue("wlSesWpa"));  
  wlVars->wlSesClEnable = atoi(BcmDb_getDefaultValue("wlSesClEnable"));
  wlVars->wlSesClEvent =  atoi(BcmDb_getDefaultValue("wlSesClEvent"));
  wlVars->wlSesWdsMode = atoi(BcmDb_getDefaultValue("wlSesWdsMode"));
  strcpy(wlVars->wlWdsWsec, BcmDb_getDefaultValue("wlWdsWsec"));            
#endif  
  wlVars->wlTxPwrPcnt = atoi(BcmDb_getDefaultValue("wlTxPwrPcnt"));
  wlVars->wlRegMode = atoi(BcmDb_getDefaultValue("wlRegMode"));
  wlVars->wlDfsPreIsm = atoi(BcmDb_getDefaultValue("wlDfsPreIsm"));
  wlVars->wlDfsPostIsm = atoi(BcmDb_getDefaultValue("wlDfsPostIsm"));
  wlVars->wlTpcDb = atoi(BcmDb_getDefaultValue("wlTpcDb"));
  wlVars->wlCsScanTimer = atoi(BcmDb_getDefaultValue("wlCsScanTimer"));
#ifdef SUPPORT_MIMO

  wlVars->wlNBwCap = atoi(BcmDb_getDefaultValue("wlNBwCap"));
  wlVars->wlNCtrlsb = atoi(BcmDb_getDefaultValue("wlNCtrlsb"));
  wlVars->wlNBand = atoi(BcmDb_getDefaultValue("wlNBand"));
  wlVars->wlNMcsidx = atoi(BcmDb_getDefaultValue("wlNMcsidx"));
  strcpy(wlVars->wlNProtection, BcmDb_getDefaultValue("wlNProtection"));
  strcpy(wlVars->wlRifs, BcmDb_getDefaultValue("wlRifs"));
  strcpy(wlVars->wlAmpdu, BcmDb_getDefaultValue("wlAmpdu"));
  strcpy(wlVars->wlAmsdu, BcmDb_getDefaultValue("wlAmsdu"));
  strcpy(wlVars->wlNmode, BcmDb_getDefaultValue("wlNmode"));   
  wlVars->wlNReqd = atoi(BcmDb_getDefaultValue("wlNReqd"));  
#endif

#ifdef XML_SUPPORT_LEGACY_WL_CONFIG
  if ((wlMVars = (PWIRELESS_MSSID_VAR) malloc(sizeof(WIRELESS_MSSID_VAR)*WL_NUM_SSID)) != NULL) {
    memset((char*)wlMVars,0,sizeof(WIRELESS_MSSID_VAR)*WL_NUM_SSID);
  //  memset((char*)wlMVars,0,sizeof(WIRELESS_MSSID_VAR)*4);	
    PWIRELESS_MSSID_VAR pwlv = wlMVars;
    wlMVars[0].wlEnblSsid = atoi(BcmDb_getDefaultValue("wlEnbl"));
    wlMVars[1].wlEnblSsid = atoi(BcmDb_getDefaultValue("wlEnbl_2"));
    strcpy(wlMVars[0].wlSsid, BcmDb_getDefaultValue("wlSsid"));
    strcpy(wlMVars[1].wlSsid, BcmDb_getDefaultValue("wlSsid_2"));
    for (int i=0; i<WL_NUM_SSID; i++, pwlv++) {
      strcpy(pwlv->wlKeys128[0], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys128[1], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys128[2], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys128[3], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys64[0], BcmDb_getDefaultValue("wlKeys64"));
      strcpy(pwlv->wlKeys64[1], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys64[2], BcmDb_getDefaultValue("wlWepKey128"));
      strcpy(pwlv->wlKeys64[3], BcmDb_getDefaultValue("wlWepKey128"));
      pwlv->wlKeyIndex128 = atoi(BcmDb_getDefaultValue("wlKeyIndex"));
      pwlv->wlKeyIndex64 = atoi(BcmDb_getDefaultValue("wlKeyIndex"));
      strcpy(pwlv->wlAuthMode, BcmDb_getDefaultValue("wlAuthMode"));
      pwlv->wlKeyBit = atoi(BcmDb_getDefaultValue("wlKeyBit"));
      strcpy(pwlv->wlWpaPsk, BcmDb_getDefaultValue("wlWpaPsk"));
      pwlv->wlWpaGTKRekey = atoi(BcmDb_getDefaultValue("wlWpaGTKRekey"));
      pwlv->wlRadiusServerIP.s_addr = inet_addr(BcmDb_getDefaultValue("wlRadiusServerIP"));
      pwlv->wlRadiusPort = atoi(BcmDb_getDefaultValue("wlRadiusPort"));
      strcpy(pwlv->wlRadiusKey, BcmDb_getDefaultValue("wlRadiusKey"));
      strcpy(pwlv->wlWep, BcmDb_getDefaultValue("wlWep"));
      strcpy(pwlv->wlWpa, BcmDb_getDefaultValue("wlWpa"));
      pwlv->wlAuth = atoi(BcmDb_getDefaultValue("wlAuth"));
      pwlv->wlPreauth = atoi(BcmDb_getDefaultValue("wlPreauth")); 
      pwlv->wlNetReauth = atoi(BcmDb_getDefaultValue("wlNetReauth")); 
      pwlv->wlNasWillrun = 0; /*runtime*/
#if defined (SUPPORT_WSC ) || defined (SUPPORT_ATHEROSWLAN)
      strcpy(pwlv->wsc_mode, BcmDb_getDefaultValue("wsc_mode"));
      strcpy(pwlv->wsc_config_state, BcmDb_getDefaultValue("wsc_config_state"));
#endif      
      /* MBSS */                  
      if(i==MAIN_BSS_IDX) {
         pwlv->wlEnblSsid = atoi(BcmDb_getDefaultValue("wlEnbl"));
         strcpy(pwlv->wlSsid, BcmDb_getDefaultValue("wlSsid"));
      } else if(i==MAIN_BSS_IDX+1) {
         pwlv->wlEnblSsid = atoi(BcmDb_getDefaultValue("wlEnbl_2"));
         strcpy(pwlv->wlSsid, BcmDb_getDefaultValue("wlSsid_2"));
      } else {
         pwlv->wlEnblSsid = atoi(BcmDb_getDefaultValue("wlEnbl_2")); 
         sprintf(pwlv->wlSsid, "%s%d", BcmDb_getDefaultValue("wlSsid_2"), i-1);
      }              
      pwlv->wlHide = atoi(BcmDb_getDefaultValue("wlHide"));
      pwlv->wlAPIsolation = atoi(BcmDb_getDefaultValue("wlAPIsolation"));
      strcpy(pwlv->wlFltMacMode, BcmDb_getDefaultValue("wlFltMacMode"));
      pwlv->wlMaxAssoc = atoi(BcmDb_getDefaultValue("wlMaxAssoc"));  
      pwlv->wlDisableWme = atoi(BcmDb_getDefaultValue("wlDisableWme"));   
	  
#ifdef SUPPORT_TR69C 
      pwlv->tr69cBeaconType = atoi(BcmDb_getDefaultValue("tr69cBeaconType"));
      pwlv->tr69cBasicEncryptionModes = atoi(BcmDb_getDefaultValue("tr69cBasicEncryptionModes"));
      pwlv->tr69cBasicAuthenticationMode = atoi(BcmDb_getDefaultValue("tr69cBasicAuthenticationMode"));
      pwlv->tr69cBasicAuthenticationMode = atoi(BcmDb_getDefaultValue("tr69cBasicAuthenticationMode"));
      pwlv->tr69cWPAAuthenticationMode = atoi(BcmDb_getDefaultValue("tr69cWPAAuthenticationMode"));
      pwlv->tr69cIEEE11iEncryptionModes = atoi(BcmDb_getDefaultValue("tr69cIEEE11iEncryptionModes"));
      pwlv->tr69cIEEE11iAuthenticationMode = atoi(BcmDb_getDefaultValue("tr69cIEEE11iAuthenticationMode"));                              
#endif      
    }
  }  
#endif
  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndWirelessVarsObj(char *appName, char *objName)
{
  PWIRELESS_VAR wlVars = (PWIRELESS_VAR)psiValue;
  int error = 0;

#ifdef XML_DEBUG
  printf("xmlEndWirelessVarsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  
  /* verify mandatory fields */
  if (xmlState.verify) {
  } /* verify */

  if (!error && !xmlState.verify) {
    BcmPsi_objStore(BcmPsi_appOpen(appName), WL_VARS_ID, wlVars, sizeof(WIRELESS_VAR));
  }
  
#ifdef XML_SUPPORT_LEGACY_WL_CONFIG
  if (wlMVars != NULL) {
    if (!error && !xmlState.verify) {
      UINT16 size = WL_NUM_SSID;
      BcmPsi_objStore(BcmPsi_appOpen(appName), WL_MSSID_VARS_NUM_ID, (void*)&size, sizeof(size));
      BcmPsi_objStore(BcmPsi_appOpen(appName), WL_MSSID_VARS_TBL_ID, wlMVars, sizeof(WIRELESS_MSSID_VAR) * size);
    }
    free(wlMVars);
    wlMVars = NULL;
  }
#endif


#ifdef XML_DEBUG1
  int i;
  printf("\n============End system Wireless Vars ===============\n");
  printf("wlEnbl %d, wlMode %s\n", wlVars->wlEnbl, wlVars->wlMode);
  printf("wlCountry %s, wlPreambleType %s\n",
         wlVars->wlCountry,wlVars->wlPreambleType);
  for (i = 0; i < WL_KEY_NUM; i++) {
    printf("wlWds[%d] = %s\n",i+1,wlVars->wlWds[i]);
  }
  //BEGIN:modify by zhourongfei to periodically check the best channel
  printf("wlChannel %d, wlchannelcheck %d, wlFrgThrshld %d, wlRtsThrshld %d, wlDtmIntvl %d, wlBcnIntvl %d\n",
         wlVars->wlChannel,wlVars->wlchannelcheck,wlVars->wlFrgThrshld,wlVars->wlRtsThrshld,
         wlVars->wlDtmIntvl,wlVars->wlBcnIntvl);
  //END:modify by zhourongfei to periodically check the best channel
  printf("wlFrameBurst %s\n", wlVars->wlFrameBurst);
  printf("wlRate %d, phytype %s, wlBasicRate %s, wlgMode %d, wlProtection %s \n",
         (int)wlVars->wlRate,wlVars->wlPhyType,wlVars->wlBasicRate,wlVars->wlgMode, wlVars->wlProtection);
 /*start of enable or disable the access point radio by l129990,2009-10-9*/
  printf("wlLazyWds %d, wlBand %d, wlAfterBurnerEn %s, wlWme %d, wlWmeNoAck %d wlRadioCtl %d wlWmeApsd %d\n",
         wlVars->wlLazyWds, wlVars->wlBand,wlVars->wlAfterBurnerEn, wlVars->wlWme,wlVars->wlWmeNoAck,wlVars->wlRadioCtl,wlVars->wlWmeApsd);
 /*end of enable or disable the access point radio by l129990,2009-10-9*/
  fprintf(stdout,"\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
  fprintf(stdout,"\r\nwlGlobalMaxAssoc=:%dfile[%s] line[%d] ",wlVars->wlGlobalMaxAssoc, __FILE__, __LINE__);
  printf("globalMaxAssoc %d\n", wlVars->wlGlobalMaxAssoc);    

#ifdef SUPPORT_WLAN_PRNTCTL  
  printf("wlAutoSwOffdays %d\n", wlVars->wlAutoSwOffdays); 
  printf("wlStarOfftTime %d\n", wlVars->wlStarOfftTime); 
  printf("wlEndOffTime %d\n", wlVars->wlEndOffTime); 
#endif  
/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_SES ) || defined (SUPPORT_ATHEROSWLAN)

  printf("wlSesEnable %d, wlSesEvent=%d, wlSesStates=%s ", \
  	 "wlSesSsid %s, wlSesHide=%d, wlSesWpaPsk=%s ", \
	 "wlSesAuth %d, wlSesAuthMode=%s, wlSesWep=%s ", \
	 "wlSesWpa %s, wlCsScanTimer %d, \n", \
	 wlVars->wlSesEnable, wlVars->wlSesEvent, wlVars->wlSesStates, \
         wlVars->wlSesSsid, wlVars->wlSesHide, wlVars->wlSesWpaPsk, \
         wlVars->wlSesAuth, wlVars->wlSesAuthMode, wlVars->wlSesWep, \
         wlVars->wlSesWpa, wlVars->wlCsScanTimer);                  
#endif         
//#ifdef SUPPORT_MIMO
#if 0
  printf("NBandwidthCap %d, NCtrlSideband=%d, NBand=%s , NMCSIdx %s, NProtection %s, NMode %s, NReqd %d\n", \
	 wlVars->wlNBwCap, wlVars->wlNCtrlsb, wlVars->wlNBand, \
         wlVars->wlNMcsidx, wlVars->wlNProtection, wlVars->wlNmode, wlVars->wlNReqd); 
  /*            
  printf("Wlrifs %d, wlAmpdu=%d, wlAmsdu=%s \n", \
	 wlVars->Wlrifs, wlVars->wlAmpdu, wlVars->wlAmsdu);
  */       
#endif
  printf("======================================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

/* wl_add_var_check_mbss */
PSX_STATUS xmlGetWirelessMssidVarsAttr(char * attrName, char * attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int size;
  UINT16 retLen=0;
#ifdef XML_DEBUG1
   printf("xmlGetWirelessMssidVarsAttr: attrName %s, attrValue %s\n", attrName,attrValue);
#endif

  if (strcasecmp(attrName,"tableSize") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = WL_NUM_SSID;
      size = sizeof(WIRELESS_MSSID_VAR) * WL_NUM_SSID;
      if ((psiValue = (void *) malloc(size)) != NULL) {
        memset((char*)psiValue,0,size);
        PWIRELESS_MSSID_VAR wlVars = (PWIRELESS_MSSID_VAR) psiValue;  
        BcmPsi_objRetrieve(BcmPsi_appOpen(app_name), WL_MSSID_VARS_TBL_ID, wlVars, size, &retLen);
      }
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidVarsAttr(): invalid tableSize %s\n",attrValue);
      printf("\nxmlGetWirelessMssidVarsAttr(): invalid tableSize %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndWirelessMssidVarsObj(char *appName, char *objName)
{
  int error = 0;
  UINT16 size = 0;
  PSI_HANDLE wlPsi = BcmPsi_appOpen(appName);

  /* this handles end of the entire table */

#ifdef XML_DEBUG
  printf("xmlEndWirelessMssidVarsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize < xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): number of entries exceeds table size\n");
      printf("\nxmlEndWirelessMssidVarsObj()[%d][%d]: number of entries exceeds table size\n",xmlState.tableSize,xmlState.tableIndex);
      error = 1;
    }

    PWIRELESS_MSSID_VAR wlVars = (PWIRELESS_MSSID_VAR) psiValue;
    
    if (wlVars->wlSsid == '\0') {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): ssId cannot be empty.\n");
      printf("\nxmlEndWirelessMssidVarsObj(): ssId cannot be empty.\n");
      error = 1;
    }
    
    if ((strcmp(wlVars->wlWep, WL_ENABLED) == 0) && strcmp(wlVars->wlAuthMode, WL_AUTH_RADIUS)) { /* Need valid wep key */
      if ((strcmp(wlVars->wlAuthMode, WL_AUTH_WPA) == 0) || (strcmp(wlVars->wlAuthMode, WL_AUTH_WPA_PSK) == 0) ||
      	  (strcmp(wlVars->wlAuthMode, WL_AUTH_WPA2) == 0) || (strcmp(wlVars->wlAuthMode, WL_AUTH_WPA2_PSK) == 0) ||
      	  (strcmp(wlVars->wlAuthMode, WL_AUTH_WPA2_MIX) == 0) || (strcmp(wlVars->wlAuthMode, WL_AUTH_WPA2_PSK_MIX) == 0)) {
        if (((wlVars->wlKeyBit == WL_BIT_KEY_64) && ((wlVars->wlKeyIndex64 != 2) && (wlVars->wlKeyIndex64 != 3))) ||
            ((wlVars->wlKeyBit == WL_BIT_KEY_128) && ((wlVars->wlKeyIndex128 != 2) && (wlVars->wlKeyIndex128 != 3))) ) {
          //xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): only key index 2 and 3 are available.\n");
          printf("\nxmlEndWirelessMssidVarsObj(): only key index 2 and 3 are available.\n");
          error = 1;
        }
      } /* not disabled */
	  
      /*去掉检查(开始) modified by chenyong 2007-4-25*/
      #if 0
      if ((wlVars->wlKeyBit == WL_BIT_KEY_64) && (wlVars->wlKeys64[wlVars->wlKeyIndex64-1][0] == '\0')) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): key64_%d must be entered.\n",wlVars->wlKeyIndex64-1);
        printf("\nxmlEndWirelessMssidVarsObj(): key64_%d must be entered.\n",wlVars->wlKeyIndex64-1);
        error = 1;
      }
      else if ((wlVars->wlKeyBit == WL_BIT_KEY_128) && (wlVars->wlKeys128[wlVars->wlKeyIndex128-1][0] == '\0')) {
        /* start of 1.0 修改wlan默认配置后，由于key1为空，导致校验不通过。修改校验条件 by xjx 2006年8月23日
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): key128_%d must be entered.\n",wlVars->wlKeyIndex128-1);
        printf("\nxmlEndWirelessMssidVarsObj(): key128_%d must be entered.\n",wlVars->wlKeyIndex128-1);
        error = 1;
        */
        if (wlVars->wlKeyIndex128 != 1)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndWirelessMssidVarsObj(): key128_%d must be entered.\n",wlVars->wlKeyIndex128-1);
            printf("\nxmlEndWirelessMssidVarsObj(): key128_%d must be entered.\n",wlVars->wlKeyIndex128-1);
            error = 1;
        }
        /* end of 1.0 修改wlan默认配置后，由于key1为空，导致校验不通过。修改校验条件 by xjx 2006年8月23日 */
      }
	  #endif
      /*去掉检查(结束) modified by chenyong 2007-4-25*/
    } /* wlAuthMode not disabled*/

  }
  
  if (!error && !xmlState.verify) {
    size = xmlState.tableSize;
    BcmPsi_objStore(wlPsi, WL_MSSID_VARS_NUM_ID, (void*)&size, sizeof(size));
    BcmPsi_objStore(wlPsi, WL_MSSID_VARS_TBL_ID, psiValue, sizeof(WIRELESS_MSSID_VAR) * xmlState.tableSize);
  }

  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

/* wl_add_var_check_mbss */
PSX_STATUS xmlGetWirelessMssidVarsEntryAttr(char *attrName, char* attrValue)
{
  /* getting an entry from the table */
  PWIRELESS_MSSID_VAR wlVars = (PWIRELESS_MSSID_VAR)psiValue + xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int index;
  int len = 0;

  /* start of maintain 判断添加错误 by xujunxia 43813 2006年5月22日
  if (wlVars == NULL) {
  */
  if (psiValue== NULL) {
  /* end of maintain 判断添加错误 by xujunxia 43813 2006年5月22日 */
    syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr():  %s\n", attrValue);
    printf("\nxmlGetWirelessMssidEntryAttr():  %s\n", attrValue);
    return PSX_STS_ERR_FATAL;
  }
  /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
  if (xmlState.verify) {
    if ( xmlState.tableIndex >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid entry index %d, tableSize %d\n",
             xmlState.tableIndex, xmlState.tableSize);
      return PSX_STS_ERR_FATAL;
    }
  }
  /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
  
  if (strcasecmp(attrName,"EnblssId") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 0, 1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid ssIdIndex option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid ssIdIndex option %s (0 or 1 expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n", wlVars->wlEnblSsid);
    }
    else
      wlVars->wlEnblSsid= atoi(attrValue);
  }  
  else if (strcasecmp(attrName,"ssId") == 0) {
    /* start of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
    #ifdef XML_DEBUG
    printf("before DecodeSsid, the ssId is %s\n", attrValue);
    #endif
    DecodeSsid(attrValue);
    #ifdef XML_DEBUG
    printf("after DecodeSsid, the ssId is %s\n", attrValue);
    #endif
    /* end of maintain PSI移植：增加对ssId特殊字符（空格、引号）的处理 by xujunxia 43813 2006年5月6日 */
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_SSID_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid ssId %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid ssId %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",  wlVars->wlSsid);
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlSsid, attrValue, len);
      wlVars->wlSsid[len] = '\0';
    }
  }
  else if (strcasecmp(attrName,"authMode") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessAuthMode(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid authMode %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid authMode %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",  wlVars->wlAuthMode);	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlAuthMode, attrValue, len);
      wlVars->wlAuthMode[len] = '\0';
    }
  } /* authMode */
#if defined (SUPPORT_WSC ) || defined (SUPPORT_ATHEROSWLAN)
   else if (strcasecmp(attrName,"wsc_mode") == 0) {
    if ( xmlState.verify) {
      /* May need paramter check*/
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wsc_mode, attrValue, len);
      wlVars->wsc_mode[len] = '\0';
    }
  }
  else if (strcasecmp(attrName,"wsc_config_state") == 0) { 
    if ( xmlState.verify) {
      /* May need paramter check*/
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wsc_config_state, attrValue, len);
      wlVars->wsc_config_state[len] = '\0';
    }
  }
#endif
  else if (strcasecmp(attrName,"radiusServerIP") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid radiusServerIP %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid radiusServerIP %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value: 0.0.0.0\n");  	  
    }
    else {
      wlVars->wlRadiusServerIP.s_addr = inet_addr(attrValue);
    }
  }
  else if (strcasecmp(attrName,"radiusServerPort") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
    if (status == DB_OBJ_VALID_OK)
      wlVars->wlRadiusPort = atoi(attrValue);
    else {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid radiusServerPort %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid radiusServerPort %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",  wlVars->wlRadiusPort);
    }
  }
  if (strcasecmp(attrName,"radiusServerKey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_RADIUS_KEY_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid radiusServerKey %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid radiusServerKey %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",  wlVars->wlRadiusKey); 	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlRadiusKey, attrValue, len);
      wlVars->wlRadiusKey[len] = '\0';
    }
  } /* wlRadiusKey */
  if (strcasecmp(attrName,"wep") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid encryption wep %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid encryption wep %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value :%s\n",wlVars->wlWep);
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlWep, attrValue, len);
      wlVars->wlWep[len] = '\0';
      xmlConvertToLowerCase(wlVars->wlWep);
    }
  } /* wlWep/encryption */
  if (strcasecmp(attrName,"wpa") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessEncryption(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid encryption wpa %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid encryption wpa %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",  wlVars->wlWpa); 	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlWpa, attrValue, len);
      wlVars->wlWpa[len] = '\0';
      xmlConvertToLowerCase(wlVars->wlWpa);
    }
  } /* wlWpa/encryption */
  if (strcasecmp(attrName,"key64Index") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,WL_KEY_NUM);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid key64Index %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid key64Index %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",  wlVars->wlKeyIndex64); 	 
    }
    else {
      wlVars->wlKeyIndex64 = atoi(attrValue);
    }
  } /* key64index */
  if (strcasecmp(attrName,"key128Index") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,1,WL_KEY_NUM);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid key128Index %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid key128Index %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",  wlVars->wlKeyIndex128); 
    }
    else {
      wlVars->wlKeyIndex128 = atoi(attrValue);
    }
  } /* key128index */
  if (strcasecmp(attrName,"keyBit") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessKeybit(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid keyBit %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid keyBit %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",  wlVars->wlKeyBit); 
    }
    else {
      wlVars->wlKeyBit = xmlWirelessWlKeyBitToPsi(attrValue);
    }
  } /* keybit strength */
  if (strncasecmp(attrName,"key64_",strlen("key64_")) == 0) {
    if ((xmlState.verify) && (strlen(attrValue) != 0))
      status = BcmDb_validateWirelessLength(attrValue,WL_KEY64_SIZE_CHAR,WL_KEY64_SIZE_HEX);
    if (status != DB_OBJ_VALID_OK) {
       //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY64_SIZE_HEX,WL_KEY64_SIZE_CHAR);
       printf("\nxmlGetWirelessMssidEntryAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY64_SIZE_HEX,WL_KEY64_SIZE_CHAR);
	printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value\n"); 
    }
    else {
      sscanf(attrName,"key64_%d",&index);
      if ((index > 0) && (index <= WL_KEY_NUM)) {
        len = strlen(attrValue);
        strncpy(wlVars->wlKeys64[index-1], attrValue, len);
        wlVars->wlKeys64[index-1][len] = '\0';
      }
      else {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid keys64_%d (expect keys64_1 to wlKeys64_4)\n",index);
        printf("\nxmlGetWirelessMssidEntryAttr(): invalid keys64_%d (expect keys64_1 to wlKeys64_4)\n",index);
      }
    } /* key length ok */
  }  /* WL_BIT_KEY_64 */
  if (strncasecmp(attrName,"key128_",strlen("key128_")) == 0) {
    if ((xmlState.verify) && (strlen(attrValue) != 0))
      status = BcmDb_validateWirelessLength(attrValue,WL_KEY128_SIZE_CHAR,WL_KEY128_SIZE_HEX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY128_SIZE_HEX,WL_KEY128_SIZE_CHAR);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid %s %s (expect %d hex or %d ascii characters)\n",attrName,attrValue,WL_KEY128_SIZE_HEX,WL_KEY128_SIZE_CHAR);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value\n"); 
    }
    else {
      sscanf(attrName,"key128_%d",&index);
      if ((index > 0) && (index <= WL_KEY_NUM)) {
        len = strlen(attrValue);
        strncpy(wlVars->wlKeys128[index-1], attrValue, len);
        wlVars->wlKeys128[index-1][len] = '\0';
      }
      else {
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid wlKeys128_%d (expect wlKeys128_1 to wlKeys128_4)\n",index);
        printf("\nxmlGetWirelessMssidEntryAttr(): invalid wlKeys128_%d (expect wlKeys128_1 to wlKeys128_4)\n",index);
      }
    } /* key length ok */
  }  /* WL_BIT_KEY_128 */
  if (strcasecmp(attrName,"wpaRekey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid wpaRekey %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid wpaRekey %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlWpaGTKRekey); 
    }
    else
      wlVars->wlWpaGTKRekey = atoi(attrValue);
  } /* wlWpaGTKRekey */
  if (strcasecmp(attrName,"wpakey") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateLength(attrValue,WL_WPA_PSK_SIZE_MAX);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid key %s\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid wpakey %s\n",attrValue); 
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",wlVars->wlWpaPsk); 	  
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlWpaPsk, attrValue, len);
      wlVars->wlWpaPsk[len] = '\0';
    }
  } /* wlWpaPsk */
  if (strcasecmp(attrName,"auth") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue,0,1);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid auth %s( 0 or 1 expected)\n",attrValue);    
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid auth %s( 0 or 1 expected)\n",attrValue);  
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlAuth); 	
    }
    else {
      wlVars->wlAuth = atoi(attrValue);
    }
  } /* wlAuth */  
  if (strcasecmp(attrName,"Preauthentication") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid preauth mode %s expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid preauth mode %s expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlPreauth); 	  
    }
    else {
      wlVars->wlPreauth = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } /* wlPreauth */
  if (strcasecmp(attrName,"ReauthTimeout") == 0) {
    if (xmlState.verify) {
      status = DB_OBJ_VALID_OK; //whatever range;
     }
    if (status != DB_OBJ_VALID_OK) {      
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        printf("\nxmlGetWirelessMssidEntryAttr(): invalid wlNetReauth mode %s \n", attrValue);
	 printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlNetReauth); 		
    }
    else {
      wlVars->wlNetReauth = atoi(attrValue);
    }
  } /* wlNetReauth */  
  /* MBSS */
  if (strcasecmp(attrName,"hide") == 0) {
    if (xmlState.verify) {
      status = BcmDb_validateRange(attrValue,0,1);
     }
    if (status != DB_OBJ_VALID_OK) {      
        //xmlState.errorFlag = PSX_STS_ERR_FATAL;
        printf("\nxmlGetWirelessMssidEntryAttr(): invalid hide option %s (0 or 1 expected)\n",attrValue);
        printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlHide); 	
    }
    else {
      wlVars->wlHide = atoi(attrValue);
    }
  } /* wlHide */
  if (strcasecmp(attrName,"apIsolation") == 0) {
    if (xmlState.verify) {
      status = BcmDb_validateWirelessAP(attrValue);
     }
    if (status != DB_OBJ_VALID_OK) {     
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid apIsolation %s (off/on expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid apIsolation %s (off/on expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlAPIsolation); 	
    }
    else {
      wlVars->wlAPIsolation = xmlWirelessAPToPsi(attrValue);
    }
  } /* wlAPIsolation */
  if (strcasecmp(attrName,"MaxAssoc") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateRange(attrValue, 1, WL_MAX_ASSOC_STA);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid MaxAssoc %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid MaxAssoc %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlMaxAssoc); 		  
    }
    else {
      wlVars->wlMaxAssoc = atoi(attrValue);
    }
  } /* wlMaxAssoc */ 
  if (strcasecmp(attrName,"fltMacMode") == 0) {
    if (xmlState.verify) {
      status = BcmDb_validateWirelessFltMacMode(attrValue);
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid fltMacMode mode %s (off/allow/deny expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid fltMacMode mode %s (disabled/allow/deny expected)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%s\n",wlVars->wlFltMacMode); 	
    }
    else {
      len = strlen(attrValue);
      strncpy(wlVars->wlFltMacMode, attrValue, len);
      wlVars->wlFltMacMode[len] = '\0';
    }
  } /* wlFltMacMode */
  if (strcasecmp(attrName,"disableWme") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateWirelessGenOnOff(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL; 
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid disableWme mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): invalid disableWme mode %s (expect off/on)\n",attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->wlDisableWme); 	  
    }
    else {
      wlVars->wlDisableWme = xmlWirelessGenOnOffAutoToPsi(attrValue,1,0,0);
    }
  } /* disableWme */  
#ifdef SUPPORT_TR69C   

  if (strcasecmp(attrName,"tr69cBeaconType") == 0) {
    int val; 
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cBeaconType mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cBeaconType mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cBeaconType); 
    }
    else {
      wlVars->tr69cBeaconType = val;
    }
  } /* tr69cBeaconType */   
  if (strcasecmp(attrName,"tr69cBasicEncryptionModes") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cBasicEncryptionModes mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cBasicEncryptionModes mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cBasicEncryptionModes);   
    }
    else {
      wlVars->tr69cBasicEncryptionModes = val;
    }
  } /* tr69cBasicEncryptionModes */ 
  if (strcasecmp(attrName,"tr69cBasicAuthenticationMode") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cBasicAuthenticationMode mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cBasicAuthenticationMode mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cBasicAuthenticationMode);   
    }
    else {
      wlVars->tr69cBasicAuthenticationMode = val;
    }
  } /* tr69cBasicAuthenticationMode */ 
  if (strcasecmp(attrName,"tr69cWPAEncryptionModes") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cWPAEncryptionModes mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cWPAEncryptionModes mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cWPAEncryptionModes);    
    }
    else {
      wlVars->tr69cWPAEncryptionModes = val;
    }
  } /* tr69cWPAEncryptionModes */ 
  if (strcasecmp(attrName,"tr69cWPAAuthenticationMode") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cWPAAuthenticationMode mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cWPAAuthenticationMode mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cWPAAuthenticationMode);    
    }
    else {
      wlVars->tr69cWPAAuthenticationMode = val;
    }
  } /* tr69cWPAAuthenticationMode */ 
  if (strcasecmp(attrName,"tr69cIEEE11iEncryptionModes") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cIEEE11iEncryptionModes mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cIEEE11iEncryptionModes mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cIEEE11iEncryptionModes);     
    }
    else {
      wlVars->tr69cIEEE11iEncryptionModes = val;
    }
  } /* tr69cBeaconType */ 
  if (strcasecmp(attrName,"tr69cIEEE11iAuthenticationMode") == 0) {
    int val;    	
    DB_OBJ_VALID_STATUS retVal;    
    retVal = BcmDb_validateXmlAndGetWirelessTR69cObjs(attrValue, attrName, &val, NULL);
    
    if (xmlState.verify) {
      status = retVal;
     }
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMssidEntryAttr(): invalid tr69cIEEE11iAuthenticationMode mode %s \n", attrValue);
      printf("xmlGetWirelessMssidEntryAttr(): invalid tr69cIEEE11iAuthenticationMode mode %s \n", attrValue);
      printf("\nxmlGetWirelessMssidEntryAttr(): Use DbDefaultTable[] default value:%d\n",wlVars->tr69cIEEE11iAuthenticationMode);          	  
    }
    else {
      wlVars->tr69cIEEE11iAuthenticationMode = val;
    }
  } /* tr69cIEEE11iAuthenticationMode */              
#endif /*#ifdef SUPPORT_TR69C */ 

  return(PSX_STS_OK);
}

PSX_STATUS xmlEndWirelessMssidVarsEntryObj(char *appName, char *objName)
{
#ifdef XML_DEBUG1
  PWIRELESS_MSSID_VAR wlVars = (PWIRELESS_MSSID_VAR)psiValue + xmlState.tableIndex;
  int i;
  printf("\n============End One Entry of Multiple SSID Wireless Vars ===============\n");
  printf("wlSsid %s, wlWpaPsk %s, wlRadiusKey %s\n",
         wlVars->wlSsid, wlVars->wlWpaPsk, wlVars->wlRadiusKey);
  for (i = 0; i < WL_KEY_NUM; i++) {
    printf("keys128[%d] = %s\n",i+1,wlVars->wlKeys128[i]);
  }
  for (i = 0; i < WL_KEY_NUM; i++) {
    printf("keys64[%d] = %s\n",i+1,wlVars->wlKeys64[i]);
  }
  printf("wlRadiusKey %s, wlWep %s, wlWpa %s\n",
         wlVars->wlRadiusKey,wlVars->wlWep,wlVars->wlWpa);
  printf("radiusIp %s, wlWpaGTKRekey %d, wlRadiusPort %d, wlAuth %d\n",
         inet_ntoa(wlVars->wlRadiusServerIP), wlVars->wlWpaGTKRekey,
         wlVars->wlRadiusPort,wlVars->wlAuth);
  printf("wlRadiusPort %d, wlAuth %d, EnblSsid %d, wlKeyIndex128 %d, wlKeyIndex64 %d\n",
         wlVars->wlRadiusPort,wlVars->wlAuth, wlVars->wlEnblSsid,
         wlVars->wlKeyIndex128, wlVars->wlKeyIndex64);
  printf("wlPreAuth %d\n",
         wlVars->wlPreauth);
  printf("======================================================\n");
#endif

  /* getting an entry from the table */
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

/* wireless table */
PSX_STATUS xmlGetWirelessMacFilterAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int size;

#ifdef XML_DEBUG1
   printf("xmlGetWirelessMacFilterAttr: attrName %s, attrValue %s\n",
          attrName,attrValue);
#endif

  if (strcasecmp(attrName,"tableSize") == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    */
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_WL_MAC_FILTER_MAX);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      size = sizeof(WL_FLT_MAC_STORE_ENTRY) * xmlState.tableSize;
      /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日"
      if ((psiValue = (void *) malloc(size)) != NULL)
        memset((char*)psiValue,0,size);
      */
      if ((psiValue = (void *) malloc(size)) != NULL)
      {
          memset((char*)psiValue,0,size);
      }
      else
      {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMacFilterAttr(): invalid tableSize %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndWirelessMacFilterObj(char *appName, char *objName)
{
  int error = 0;
  UINT16 numObjId=0, tblObjId=0, size = 0;
  PWL_FLT_MAC_STORE_ENTRY macFilterTbl = (PWL_FLT_MAC_STORE_ENTRY)psiValue;
  PSI_HANDLE wlPsi = BcmPsi_appOpen(appName);

  /* this handles end of the entire table */

#ifdef XML_DEBUG
  printf("xmlEndWirelessMacFilterEntryObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndWirelessMacFilterObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }

  if (strcmp(objName,TBLNAME_WL_MAC_FILTER) == 0) {
    numObjId = WL_FLT_MAC_NUM_ID;
    tblObjId = WL_FLT_MAC_TBL_ID;
  }
  else if (strcmp(objName,TBLNAME_WDS_FILTER) == 0) {
    numObjId = WL_WDS_MAC_NUM_ID;
    tblObjId = WL_WDS_MAC_TBL_ID;
  }
  else {
    numObjId = WL_SCAN_WDS_MAC_NUM_ID;
    tblObjId = WL_SCAN_WDS_MAC_TBL_ID;
  }
  if (!error && !xmlState.verify) {
    size = xmlState.tableSize;
    BcmPsi_objStore(wlPsi, numObjId, (void*)&size, sizeof(size));
    BcmPsi_objStore(wlPsi, tblObjId, macFilterTbl,
                    (sizeof(WL_FLT_MAC_STORE_ENTRY) * xmlState.tableSize));
  }

#ifdef XML_DEBUG1
  printf("\n============End Wireless %s Table=========\n",objName);
  printf("xmlEndWirelessMacFilterObj(): numObjId %d, tblObjId %d, tableSize = %d\n",
         numObjId, tblObjId, xmlState.tableSize);
  printf("table:\n");
  for (int i=0; i< xmlState.tableSize; i++) {
    printf("entry %d: macAddr %s\n",i,macFilterTbl[i].macAddress);
  }
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetWirelessMacFilterEntryAttr(char *attrName, char* attrValue)
{
  /* getting an entry from the table */
  PWL_FLT_MAC_STORE_ENTRY macFilterTbl = (PWL_FLT_MAC_STORE_ENTRY)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

  if (macFilterTbl == NULL) {
    syslog(LOG_ERR,"xmlGetWirelessMacFilterEntryAttr():  %s\n", attrValue);
    return PSX_STS_ERR_FATAL;
  }
  /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
  if (xmlState.verify) {
    if ( xmlState.tableIndex >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMacFilterEntryAttr(): invalid entry index %d, tableSize %d\n",
             xmlState.tableIndex, xmlState.tableSize);
      return PSX_STS_ERR_FATAL;
    }
  }
  /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
  if (strcmp(attrName,"macAddr") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateMacAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      //xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("\nxmlGetWirelessMacFilterEntryAttr(): invalid macAddr %s\n", attrValue);
      printf("\nxmlGetWirelessMacFilterEntryAttr(): Set macAddr :%s\n", "00:00:00:00:00:00");
      len = strlen("00:00:00:00:00:00");
      strncpy(macFilterTbl[xmlState.tableIndex].macAddress,"00:00:00:00:00:00",len);
      macFilterTbl[xmlState.tableIndex].macAddress[len] = '\0';
      /* default to main wireles interface */
      strncpy(macFilterTbl[xmlState.tableIndex].ifcName,WL_DEFAULT_IFC,strlen(WL_DEFAULT_IFC));
      macFilterTbl[xmlState.tableIndex].ifcName[strlen(WL_DEFAULT_IFC)] = '\0';
	  
    }
    else {
      len = strlen(attrValue);
      strncpy(macFilterTbl[xmlState.tableIndex].macAddress,attrValue,len);
      macFilterTbl[xmlState.tableIndex].macAddress[len] = '\0';
      /* default to main wireles interface */
      strncpy(macFilterTbl[xmlState.tableIndex].ifcName,WL_DEFAULT_IFC,strlen(WL_DEFAULT_IFC));
      macFilterTbl[xmlState.tableIndex].ifcName[strlen(WL_DEFAULT_IFC)] = '\0';
    }
  }

  if (strcmp(attrName,"ifc") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateInterfaceName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetWirelessMacFilterEntryAttr(): invalid ifc %s\n",
             attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(macFilterTbl[xmlState.tableIndex].ifcName,attrValue,len);
      macFilterTbl[xmlState.tableIndex].ifcName[len] = '\0';
    }
  }  

  return(PSX_STS_OK);
}

PSX_STATUS xmlEndWirelessMacFilterEntryObj(char *appName, char *objName)
{
  /* getting an entry from the table */
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#endif /* #ifdef WIRELESS */
 /*end of VDF  2008.3.28 HG553V100R001 w45260:AU8D00327 Wlan配置项错误导致网关配置会恢复出厂配置.修改:提供出错默认配置，避免网关配置项的整体恢复*/

//=====================================================================================
//=============================== Start RouteObj Functions ==============================
//=====================================================================================

PSX_STATUS xmlGetRouteObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetRouteObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; routeObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(routeObjs[i].objName, objName) == 0) {
      if (*(routeObjs[i].getAttrFnc) != NULL) {
	sts = (*(routeObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetRouteObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartRouteObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartRouteObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; routeObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(routeObjs[i].objName, objName) == 0) {
      if (*(routeObjs[i].startObjFnc) != NULL) {
        sts = (*(routeObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartRouteObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndRouteObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndRouteObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; routeObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(routeObjs[i].objName, objName) == 0) {
      if (*(routeObjs[i].endObjFnc) != NULL) {
	sts = (*(routeObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndRouteObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetRouteAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetRouteAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(RT_ROUTE_CFG_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
      memset(psiValue,0,(sizeof(RT_ROUTE_CFG_ENTRY) * xmlState.tableSize));
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndRouteObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndRouteObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndRouteObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setStaticRouteSize(xmlState.tableSize);
    BcmDb_setStaticRouteInfo((PRT_ROUTE_CFG_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static Route=========\n");
  printf("xmlEndRouteObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetRouteEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PRT_ROUTE_CFG_ENTRY routeTbl = (PRT_ROUTE_CFG_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetRouteEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetRouteEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"instanceId") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      routeTbl[i].id = strtoul(attrValue, (char **)NULL, 10);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid instance ID %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid instance ID %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"ifName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(routeTbl[i].ifName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid interface name %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"addr") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      inet_aton(attrValue, &(routeTbl[i].ipAddress));
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid IP address %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid IP address %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"mask") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateSubnetMask(attrValue);
    if (status == DB_OBJ_VALID_OK)
      inet_aton(attrValue, &(routeTbl[i].subnetMask));
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid subnet mask %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid subnet mask %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"gateway") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      inet_aton(attrValue, &(routeTbl[i].gateway));
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid gateway %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid gateway %s\n",attrValue);
    }
  }
  /*start of Enables or disable the forwarding entry by l129990,2009-11-6*/
  else if (strcmp(attrName,"routeCtl") == 0)
  {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      routeTbl[i].entryControl = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRouteEntryAttr(): invalid instance ID %s\n",attrValue);
      printf("xmlGetRouteEntryAttr(): invalid instance ID %s\n",attrValue);
    }
  }
  /*end of Enables or disable the forwarding entry by l129990,2009-11-6*/
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndRouteEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PRT_ROUTE_CFG_ENTRY routeTbl = (PRT_ROUTE_CFG_ENTRY)psiValue;

  /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlEndRouteEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */

  if (routeTbl[i].id == 0)
    routeTbl[i].id = i+1;

  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

#ifdef SUPPORT_RIP
PSX_STATUS xmlGetRipAttr(char *attrName, char* attrValue)
{
  PRT_RIP_CFG rip= (PRT_RIP_CFG)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetRipAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK)
      rip->globalMode = xmlStateToPsi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipAttr(): invalid state %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"ripIfcTableSize") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      rip->ifcCount = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipAttr(): invalid number of interfaces %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartRipObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartRipObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(RT_RIP_CFG));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue, 0, sizeof(RT_RIP_CFG));
  ((PRT_RIP_CFG)psiValue)->globalMode = atoi(BcmDb_getDefaultValue("gRipMode"));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndRipObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndRipObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setRipInfo((PRT_RIP_CFG)psiValue);

#ifdef XML_DEBUG1
  printf("\n============End global rip=========\n");
  printf("xmlEndRipObj(): globalMode %d\n", ((PRT_RIP_CFG)psiValue)->globalMode);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetRipIfcAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetRipIfcAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(RT_RIP_IFC_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(RT_RIP_IFC_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndRipIfcObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndRipIfcObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndRipIfcObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify)
    BcmDb_setRipIfcInfo((PRT_RIP_IFC_ENTRY)psiValue, xmlState.tableSize);

#ifdef XML_DEBUG1
  printf("\n============End Static Route=========\n");
  printf("xmlEndRipIfcObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetRipIfcEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PRT_RIP_IFC_ENTRY ripTbl = (PRT_RIP_IFC_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetRipIfcEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"name") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, RIP_IFC_NAME_MAX_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ripTbl[i].name, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK)
      ripTbl[i].flag.mode = xmlStateToPsi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid state %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"version") == 0) {
    ripTbl[i].flag.version = BcmDb_getRipVersion(attrValue);
  }
  else if (strcmp(attrName,"operation") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRipOperation(attrValue);
    if (status == DB_OBJ_VALID_OK)
      ripTbl[i].flag.operation = BcmDb_getRipOperation(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid operation %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndRipIfcEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#endif /* #ifdef SUPPORT_RIP */

//=====================================================================================
//=============================== End RouteObj Functions ==============================
//=====================================================================================

#ifdef SUPPORT_DDNSD
//=====================================================================================
//=============================== Start DDNSCfg Functions ==============================
//=====================================================================================

PSX_STATUS xmlGetDdnsObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetDdnsObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 1; DDnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(DDnsObjs[i].objName, objName) == 0) {
      if (*(DDnsObjs[i].getAttrFnc) != NULL) {
	sts = (*(DDnsObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetDdnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartDdnsObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartDdnsObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; DDnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(DDnsObjs[i].objName, objName) == 0) {
      if (*(DDnsObjs[i].startObjFnc) != NULL) {
        sts = (*(DDnsObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartDdnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndDdnsObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndDdnsObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; DDnsObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(DDnsObjs[i].objName, objName) == 0) {
      if (*(DDnsObjs[i].endObjFnc) != NULL) {
	sts = (*(DDnsObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndDdnsObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetDdnsAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetDdnsAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(DDNS_SERVICE_INFO) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(DDNS_SERVICE_INFO) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndDdnsObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndDdnsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndDdnsObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setDdnsSize(xmlState.tableSize);
    BcmDb_setDdnsInfo((PDDNS_SERVICE_INFO)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End DDNSCfg=========\n");
  printf("xmlEndDdnsObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetDdnsEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PDDNS_SERVICE_INFO ddnsTbl = (PDDNS_SERVICE_INFO)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetDdnsEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"hostname") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_MEDIUM_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ddnsTbl[i].hostname, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid hostname %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"username") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_MEDIUM_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ddnsTbl[i].username, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid username %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"password") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_PASSWORD_LEN);
    if (status == DB_OBJ_VALID_OK)
    {

#ifndef SUPPORT_DDNSWD_UNENCRYPTED
	  char dest_string[IFC_PASSWORD_LEN];
	  memset(dest_string, 0, sizeof(dest_string));
	  base64Decode((unsigned char *)dest_string, attrValue, (int *)NULL);
      strcpy(ddnsTbl[i].password, dest_string);
#else
      strcpy(ddnsTbl[i].password, attrValue);
#endif


	  
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid password %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"ifc") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(ddnsTbl[i].iface, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"service") == 0) {
    if ( strcasecmp(attrValue, "tzo") == 0 )
       ddnsTbl[i].service = 0;
    else if ( strcasecmp(attrValue, "dyndns") == 0 )
       ddnsTbl[i].service = 1;
    else if (xmlState.verify) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDdnsEntryAttr(): invalid service %s\n",attrValue);
    }
  }
// add by l66195 for VDF start
  if (strcmp(attrName,"bSelect") == 0) {
  	ddnsTbl[i].bSelect = atoi(attrValue);
  }
// add by l66195 for VDF end
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndDdnsEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

//=====================================================================================
//=============================== End DDNSCfg Functions ==============================
//=====================================================================================
#endif /* #ifdef SUPPORT_DDNSD */

/* atmCfg */
PSX_STATUS xmlGetAtmCfgNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetAtmCfgNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 1; atmCfgObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(atmCfgObjs[i].objName, objName, strlen(atmCfgObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(atmCfgObjs[i].getAttrFnc) != NULL) {
	sts = (*(atmCfgObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetAtmCfgNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

  }
  return sts;
} /* xmlGetAtmCfgNode */

PSX_STATUS xmlStartAtmCfgNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlStartAtmCfgNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; atmCfgObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(atmCfgObjs[i].objName, objName, strlen(atmCfgObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(atmCfgObjs[i].startObjFnc) != NULL) {
	sts = (*(atmCfgObjs[i].startObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartAtmCfgNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

PSX_STATUS xmlEndAtmCfgNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i;

#ifdef XML_DEBUG
  printf("xmlEndObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 1; atmCfgObjs[i].objName[0] != '\0'; i++ ) {
    if (strncmp(atmCfgObjs[i].objName, objName, strlen(atmCfgObjs[i].objName)) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(atmCfgObjs[i].endObjFnc) != NULL) {
	sts = (*(atmCfgObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndAtmCfgNode(): Unrecognizable objName %s, ignored\n",objName);
    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif

   }
   return sts;
} 

PSX_STATUS xmlGetAtmInitAttr(char *attrName, char* attrValue)
{
  PIFC_DEF_GW_INFO gw= (PIFC_DEF_GW_INFO)psiValue;
  int len = 0;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetAtmInitAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"enblGwAutoAssign",strlen("enblGwAutoAssign")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK)
      gw->enblGwAutoAssign = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmInitAttr(): invalid state %s\n",attrValue);
    }
  }
  else if (strncmp(attrName,"ifName",strlen("ifName")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateInterfaceName(attrName);
    if (status == DB_OBJ_VALID_OK) {
      len = strlen(attrValue);
      strncpy(gw->ifName,attrValue,len);
      gw->ifName[len] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmInitAttr(): invalid ifName %s\n",attrValue);
    }
  }
  else if (strncmp(attrName,"ipAddr",strlen("ipAddr")) == 0) {
    status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) 
      inet_aton((const char*)attrValue,&gw->defaultGateway);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAtmInitAttr(): invalid ipAddr %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}
#ifdef SUPPORT_VDSL
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
PSX_STATUS xmlGetAtmTrafficModeAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetAtmTrafficModeAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK) {
         *((int *)psiValue) = atoi(attrValue);
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("***********mlGetAtmTrafficModeAttr(): err fatal");
      syslog(LOG_ERR,"xmlGetAtmTrafficModeAttr(): invalid traffic mode state %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#endif
void xmlSetAtmInitDefault(PATM_INITIALIZATION_PARMS initCfg)
{
  /* copying these from atmmngr/atminitcfg.cpp */
  /* these constant should be defined somewhere so that it's shared with atminitcfg.cpp */
  initCfg->ulStructureId = ID_ATM_INITIALIZATION_PARMS;
  initCfg->ulThreadPriority = 25;
  initCfg->ucTransmitFifoPriority = 64;
  initCfg->usFreeCellQSize = 10;
  initCfg->usFreePktQSize = 200;
  initCfg->usFreePktQBufferSize = 1600;
  initCfg->usFreePktQBufferOffset = 32;
  initCfg->usReceivePktQSize = 200;
  initCfg->usReceiveCellQSize = 10;
  initCfg->usAal5CpcsMaxSduLength = 65535;
  for ( int i = 0; i < NUM_PORTS; i++ ) {
    initCfg->PortCfg[i].ulInterfaceId = IFC_PORT_ID_BASE + i + 1;
    /* port 1 is always enable */
    if (i == 0)
      initCfg->PortCfg[i].ucPortType = PT_ADSL_INTERLEAVED;
    else
      initCfg->PortCfg[i].ucPortType = PT_DISABLED;
  }
}

PSX_STATUS xmlStartAtmInitObj(char *appName, char *objName)
{
  PATM_INITIALIZATION_PARMS initCfg;

#ifdef XML_DEBUG
  printf("xmlStartAtmInitObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(ATM_INITIALIZATION_PARMS));
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  memset(psiValue, 0, sizeof(ATM_INITIALIZATION_PARMS));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  initCfg = (PATM_INITIALIZATION_PARMS)psiValue;
  xmlSetAtmInitDefault(initCfg);

  return PSX_STS_OK;
}
#ifdef SUPPORT_VDSL
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
PSX_STATUS xmlStartAtmTrafficModeObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartAtmTrafficModeObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(int));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(int));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  return PSX_STS_OK;
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
PSX_STATUS xmlEndAtmTrafficModeObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndAtmTrafficModeObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  int mode = *((int *)psiValue);
  
  // if (!xmlState.verify)
    BcmDb_setTrafficMode(mode);

#ifdef XML_DEBUG1
  printf("\n============End traffic Mode=========\n");
  printf("xmlEndAtmTrafficModeObj(): mode = %d\n", mode);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#endif
PSX_STATUS xmlEndAtmInitObj(char *appName, char *objName)
{
  int error = 0;
 
#ifdef XML_DEBUG
  printf("xmlEndAtmInitdObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  /* this ACU_INIT_CFG_ID, object id 2 from main/atmcfgmngr/atmcfgmngr.h also needs to be moved */
  if (!error && !xmlState.verify)
    BcmPsi_objStore(BcmPsi_appOpen("AtmCfg"),2,psiValue,
                    sizeof(ATM_INITIALIZATION_PARMS));

#ifdef XML_DEBUG1
  PATM_INITIALIZATION_PARMS initCfg = (PATM_INITIALIZATION_PARMS)psiValue;
  printf("\n============End system default gateway=========\n");
  printf("xmlEndAtmInitObj(): psiValue->ulStructureId %d, ulThreadPriority %d, ucTransmitFifoPriority %d\n",
         (int)initCfg->ulStructureId,(int)initCfg->ulThreadPriority,(int)initCfg->ucTransmitFifoPriority);
  printf("usFreeCellQSize %d, usFreePktQSize %d, usFreePktQBufferSize %d, usFreePktQBufferOffset %d\n",
         (int)initCfg->usFreeCellQSize,(int)initCfg->usFreePktQSize,
         (int)initCfg->usFreePktQBufferSize,(int)initCfg->usFreePktQBufferSize);
  printf("usReceiveCellQSize %d, usAal5CpcsMaxSduLength %d\n",
         (int)initCfg->usReceiveCellQSize,(int)initCfg->usAal5CpcsMaxSduLength);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

/* ADSL */
PSX_STATUS xmlGetAdslNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
  return (xmlGetAdslAttr(attrName,attrValue));
}

PSX_STATUS xmlGetAdslAttr(char *attrName, char* attrValue)
{
  int bit = 0;
  unsigned int *adslConfig = (unsigned int*)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetAdslAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcasecmp(attrName,"G.Dmt") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid G.Dmt state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_GDMT;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_GDMT;
    }
  }
  if (strcasecmp(attrName,"G.lite") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid G.lite state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_GLITE;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_GLITE;
    }
  }
  if (strcasecmp(attrName,"T1.413") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid T1.413 state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_T1413;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_T1413;
    }
  } 
  if (strcmp(attrName,"ADSL2") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid ADSL2 state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit) 
        (*adslConfig) |= ANNEX_A_MODE_ADSL2;
      else 
        (*adslConfig) &= ~ANNEX_A_MODE_ADSL2;
    }
  } 
  if (strcasecmp(attrName,"AnnexL") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid AnnexL state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_ANNEXL;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_ANNEXL;
    }
  } 
  if (strcasecmp(attrName,"ADSL2plus") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid ADSL2plus state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_ADSL2PLUS;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_ADSL2PLUS;
    }
  } 
  if (strcasecmp(attrName,"AnnexM") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid AnnexM state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_MODE_ANNEXM;
      else
        (*adslConfig) &= ~ANNEX_A_MODE_ANNEXM;
    }
  } 
  if (strcasecmp(attrName,"pair") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateAdslPair(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid pair %s\n",attrValue);
    }
    else {
      bit = xmlPairToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_LINE_PAIR_OUTER;
      else
        (*adslConfig) &= ~ANNEX_A_LINE_PAIR_OUTER;
    }
  } 
  if (strcasecmp(attrName,"bitswap") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid bitswap state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (!bit)
        (*adslConfig) |= ANNEX_A_BITSWAP_DISENABLE;
      else
        (*adslConfig) &= ~ANNEX_A_BITSWAP_DISENABLE;
    }
  } 
  if (strcasecmp(attrName,"SRA") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetAdslAttr(): invalid SRA state %s\n",attrValue);
    }
    else {
      bit = xmlStateToPsi(attrValue);
      if (bit)
        (*adslConfig) |= ANNEX_A_SRA_ENABLE;
      else
        (*adslConfig) &= ~ANNEX_A_SRA_ENABLE;
    }
  } 
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartAdslObj(char *appName, char *objName)
{
  unsigned int *adslConfig;

#ifdef XML_DEBUG
  printf("xmlStartAdslObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(int));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  adslConfig = (unsigned int*)psiValue;
  *adslConfig = 0;

  (*adslConfig) |= ANNEX_A_MODE_ALL_MOD_48;
  
  return PSX_STS_OK;
}

PSX_STATUS xmlEndAdslObj(char *appName, char *objName)
{
  unsigned int *adslConfig = (unsigned int*)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndAdsldObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify) {
    BcmPsi_objStore(BcmPsi_appOpen(ADSL_APPID),ADSL_CONFIG_FLAG,adslConfig,sizeof(int));
  }

#ifdef XML_DEBUG1
  printf("\n============End ADSL =========\n");
  printf("xmlEndAdslObj(): adslConfig %x\n",*adslConfig);
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#ifdef SUPPORT_SNTP
/* sntp */
PSX_STATUS xmlGetSntpNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
  return (xmlGetSntpAttr(attrName,attrValue));
}

PSX_STATUS xmlGetSntpAttr(char *attrName, char* attrValue)
{
  PNTP_CONFIG sntp = (PNTP_CONFIG)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetSntpAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_STATE,strlen(PSX_ATTR_STATE)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid state %s\n",attrValue);
    }
    else
      sntp->ntp_enabled = xmlStateToPsi(attrValue);
  }
  /* start of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SNTP_AUTOCONF
  else if (strncmp(attrName, "CfgServer",strlen("CfgServer")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid CfgServer %s\n",attrValue);
    }
    else
      sntp->CfgServerEnable = xmlStateToPsi(attrValue);
  }
#endif
  /* end of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
  else if (strncmp(attrName,"server1",strlen("server1")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,IFC_DOMAIN_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid server1 %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sntp->ntpServer1,attrValue,len);
      sntp->ntpServer1[len] = '\0';
    }
  }
  else if (strncmp(attrName,"server2",strlen("server2")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,IFC_DOMAIN_LEN);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid server2 %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sntp->ntpServer2,attrValue,len);
      sntp->ntpServer2[len] = '\0';
    }
  }
//BEGIN:add by zhourongfei to config 3rd ntpserver
else if (strncmp(attrName,"server3",strlen("server3")) == 0) {
  if (xmlState.verify)
	status = BcmDb_validateLength(attrValue,IFC_DOMAIN_LEN);
  if (status != DB_OBJ_VALID_OK) {
	xmlState.errorFlag = PSX_STS_ERR_FATAL;
	syslog(LOG_ERR,"xmlGetSntpAttr(): invalid server3 %s\n",attrValue);
  }
  else {
	len = strlen(attrValue);
	strncpy(sntp->ntpServer3,attrValue,len);
	sntp->ntpServer3[len] = '\0';
  }
}
//END:add by zhourongfei to config 3rd ntpserver
  else if (strncmp(attrName,"timezone",strlen("timezone")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,64);  /* 64 is from ntpConfig struct definition */

    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid timezone %s\n",attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(sntp->timezone,attrValue,len);
      sntp->timezone[len] = '\0';
    }
  }
   /*start of 增加sntp 刷新时间配置 by s53329 at  20080908*/
  else if (strncmp(attrName,"FailTime",strlen("FailTime")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);  /* 64 is from ntpConfig struct definition */

    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid FailTimes %s\n",attrValue);
    }
    else {
        sntp->iFailTimes = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"SucTime",strlen("SucTime")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);  /* 64 is from ntpConfig struct definition */

    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid SucTimes %s\n",attrValue);
    }
    else {
        sntp->iSucTimes = atoi(attrValue);
    }
  }
  else if (strncmp(attrName,"EnableSum",strlen("EnableSum")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);  /* 64 is from ntpConfig struct definition */

    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): invalid SucTimes %s\n",attrValue);
    }
    else {
        sntp->iEnableSum= atoi(attrValue);
    }
  }
   /*end  of 增加sntp 刷新时间配置 by s53329 at  20080908*/
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSntpObj(char *appName, char *objName)
{
  PNTP_CONFIG sntp;

#ifdef XML_DEBUG
  printf("xmlStartSntpObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(NTP_CONFIG));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  sntp = (PNTP_CONFIG)psiValue;
  memset(sntp,0,sizeof(NTP_CONFIG));
  strcpy(sntp->ntpServer1, "clock.fmt.he.net");
  strcpy(sntp->ntpServer2, "None");
  //BEGIN:add by zhourongfei to config 3rd ntpserver
  strcpy(sntp->ntpServer3, "None");
  //END:add by zhourongfei to config 3rd ntpserver
  strcpy(sntp->timezone,"International Date Line West");
  /*start of 增加sntp 刷新时间配置 by s53329 at  20080908*/
  sntp->iFailTimes  = 5;
  sntp->iSucTimes  = 6*60*60;
  sntp->iEnableSum= 0;
  /*end  of 增加sntp 刷新时间配置 by s53329 at  20080908*/

  return PSX_STS_OK;
}

PSX_STATUS xmlEndSntpObj(char *appName, char *objName)
{
  PNTP_CONFIG sntp = (PNTP_CONFIG)psiValue;
  struct ntpConfig temp;
  UINT16 retLen=0;
  PSI_HANDLE sntpApp = NULL;

#ifdef XML_DEBUG
  printf("xmlEndSntpObj(calling cfm to get appName %s, objName %s)\n",
	 appName,objName);
#endif

  if (xmlState.verify) {
  	//BEGIN:modify by zhourongfei to config 3rd ntpserver
    if ((sntp->ntp_enabled) &&
        (((sntp->ntpServer1[0] == '\0') && (sntp->ntpServer2[0] == '\0') && (sntp->ntpServer3[0] == '\0')) ||
         (sntp->timezone[0] == '\0'))) {
	//END:modify by zhourongfei to config 3rd ntpserver
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSntpAttr(): sntp enabled, server and timezone must be set\n");
    }
  }
  else {
    sntpApp = BcmPsi_appOpen(SNTP_APP_ID);
    if (sntpApp != NULL) {
      if (BcmPsi_objRetrieve(sntpApp,SNTP_CONFIG,&temp,sizeof(NTP_CONFIG),
                             &retLen) == PSI_STS_OK) {
        BcmPsi_objRemove(sntpApp,SNTP_CONFIG);
      }
      BcmPsi_objStore(sntpApp, SNTP_CONFIG, (void*)sntp, sizeof(NTP_CONFIG));
    }
  }

//BEGIN:add by zhourongfei config 3rd ntpserver
#ifdef XML_DEBUG1
  printf("\n============End system sntp info=========\n");
  printf("xmlEndSntpObj(): psiValue->status %d, server1 %s, server2 %s, server3 %s, timezone %s\n",
         (int)sntp->ntp_enabled,sntp->ntpServer1,sntp->ntpServer2,sntp->ntpServer3, sntp->timezone);
  printf("===============================================\n");
#endif
//END:add by zhourongfei config 3rd ntpserver
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
#endif /* sntp */

#ifdef SUPPORT_TOD
/* ToD */
unsigned char xmlToDDaysToPsi(char *value)
{
  unsigned char day = 0;
  int len = strlen(value);
  char *ptr = value;
                        
  while (len > 0) {
    if (strncasecmp("Mon", ptr, 3) == 0 )
      day |= MONDAY;
    else if (strncasecmp("Tue", ptr, 3) == 0 )
      day |= TUESDAY;
    else if (strncasecmp("Wed", ptr, 3) == 0 )
      day |= WEDNSDAY;
    else if (strncasecmp("Thu", ptr, 3) == 0 )
      day |= THURSDAY;
    else if (strncasecmp("Fri", ptr, 3) == 0 )
      day |= FRIDAY;
    else if (strncasecmp("Sat", ptr, 3) == 0 )
      day |= SATURDAY;
    else if (strncasecmp("Sun", ptr, 3) == 0 )
      day |= SUNDAY;
    ptr += 3;
    len -= 3;
  } /* WHILE */
  return day;
}

PSX_STATUS xmlGetToDObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetToDObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; ToDObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(ToDObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(ToDObjs[i].getAttrFnc) != NULL) {
	sts = (*(ToDObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartToDObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlStartToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; ToDObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(ToDObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(ToDObjs[i].startObjFnc) != NULL) {
        sts = (*(ToDObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndToDObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; ToDObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(ToDObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(ToDObjs[i].endObjFnc) != NULL) {
	sts = (*(ToDObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetToDAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetToDAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(TOD_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(TOD_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndToDObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndToDObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndToDObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setToDSize(xmlState.tableSize);
    BcmDb_setToDInfo((PTOD_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End ToD=========\n");
  printf("xmlEndToDObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetToDEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  int hour = 0, min = 0;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PTOD_ENTRY todTbl = (PTOD_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetToDEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetToDEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"username") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_MEDIUM_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(todTbl[i].username, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid username %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"mac") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(todTbl[i].mac,attrValue);
      else
        todTbl[i].mac[0] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid mac %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"startTime") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateTime(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      sscanf(attrValue,"%d:%d",&hour,&min);
      todTbl[i].start_time = (hour * 60) + min;
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid startTime %s (expect hh:mm in 24-hour format)\n",attrValue);
    }
  }
  if (strcmp(attrName,"endTime") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateTime(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      sscanf(attrValue,"%d:%d",&hour,&min);
      todTbl[i].end_time = (hour * 60) + min;
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid startTime %s (expect hh:mm in 24-hour format)\n",attrValue);
    }
  }
  if (strcmp(attrName,"days") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateDay(attrValue);
    if (status == DB_OBJ_VALID_OK)
      todTbl[i].days = xmlToDDaysToPsi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid days %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndToDEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#endif /* #ifdef SUPPORT_TOD */

#ifdef SUPPORT_MAC //add by l66195 for VDF

PSX_STATUS xmlGetMacObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetToDObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; MacObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacObjs[i].getAttrFnc) != NULL) {
	sts = (*(MacObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartMacObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlStartToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; MacObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacObjs[i].startObjFnc) != NULL) {
        sts = (*(MacObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndMacObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; MacObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacObjs[i].endObjFnc) != NULL) {
	sts = (*(MacObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetMacAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetToDAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_MAC_ENTRY);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(MAC_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(MAC_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndToDObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndToDObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setMacSize(xmlState.tableSize);
    BcmDb_setMacInfo((PMAC_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End ToD=========\n");
  printf("xmlEndToDObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetMacEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  int hour = 0, min = 0;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PMAC_ENTRY macTbl = (PMAC_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetToDEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetToDEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"username") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_MEDIUM_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(macTbl[i].username, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid username %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"mac") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(macTbl[i].mac,attrValue);
      else
        macTbl[i].mac[0] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid mac %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#endif /* #ifdef SUPPORT_MAC */

#ifdef SUPPORT_MACMATCHIP 

PSX_STATUS xmlGetMacMatchObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlGetToDObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; MacMatchObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacMatchObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacMatchObjs[i].getAttrFnc) != NULL) {
	sts = (*(MacMatchObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartMacMatchObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  printf("xmlStartToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; MacMatchObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacMatchObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacMatchObjs[i].startObjFnc) != NULL) {
        sts = (*(MacMatchObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndMacMatchObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndToDObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; MacMatchObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(MacMatchObjs[i].objName, objName) == 0) {
      sts = PSX_STS_ERR_GENERAL;
      if (*(MacMatchObjs[i].endObjFnc) != NULL) {
	sts = (*(MacMatchObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndToDObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetMacMatchAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetToDAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_MACMATCH_ENTRY);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(MACMATCH_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(MACMATCH_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacMatchObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndToDObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndToDObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setMacMatchSize(xmlState.tableSize);
    BcmDb_setMacMatchInfo((PMACMATCH_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End ToD=========\n");
  printf("xmlEndToDObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetMacMatchEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  int hour = 0, min = 0;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PMACMATCH_ENTRY macTbl = (PMACMATCH_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetToDEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetToDEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"username") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_MEDIUM_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(macTbl[i].username, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid username %s\n",attrValue);
    }
  }
  if (strcmp(attrName,"mac") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        bcmMacStrToNum(macTbl[i].mac,attrValue);
      else
        macTbl[i].mac[0] = '\0';
    }
  }
  if (strcmp(attrName,"ip") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( attrValue[0] != '\0' )
        memcpy( macTbl[i].ipaddress, attrValue, IP_ADDR_LEN );
        //strcpy(macTbl[i].ipaddress,attrValue);
      else
        macTbl[i].ipaddress[0] = '\0';
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetToDEntryAttr(): invalid mac %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndMacMatchEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#endif 

#ifdef SUPPORT_IPP
PSX_STATUS xmlSetIppNode(FILE* file, const char *appName, const UINT16 id,
                         const PSI_VALUE value, const UINT16 length)
{
   pIppConfig ipp;
   static char* state[] = { "disable", "enable" };
   int mode;

   if ((appName == NULL) || (value == NULL))
     return PSX_STS_ERR_GENERAL;

   ipp = (pIppConfig)value;
   mode = (int)ipp->enabled;
   
   fprintf(file, "<cfg %s=\"%s\" %s=\"%s\" %s=\"%s\"/>\n",
	   PSX_ATTR_STATE,state[mode], 
           PSX_ATTR_IPP_MAKE,ipp->make,
           PSX_ATTR_IPP_NAME,ipp->name);

   return PSX_STS_OK;
}

PSX_STATUS xmlGetIppNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
  return (xmlGetIppAttr(attrName,attrValue));
}

PSX_STATUS xmlGetIppAttr(char *attrName, char* attrValue)
{
  pIppConfig ipp = (pIppConfig)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int len = 0;

#ifdef XML_DEBUG
  printf("xmlGetIppAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strcmp(attrName,PSX_ATTR_STATE) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIppAttr(): invalid state %s\n",attrValue);
    }
    else
      ipp->enabled = xmlStateToPsi(attrValue);
  }
  else if (strcmp(attrName,PSX_ATTR_IPP_MAKE) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,IPP_MAX_LENGTH);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIppAttr(): invalid %s %s\n",PSX_ATTR_IPP_MAKE,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(ipp->make,attrValue,len);
      ipp->make[len] = '\0';
    }
  }
  else if (strcmp(attrName,PSX_ATTR_IPP_NAME) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue,IPP_NAME_MAX_LENGTH);

    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIppAttr(): invalid %s %s\n",PSX_ATTR_IPP_NAME,attrValue);
    }
    else {
      len = strlen(attrValue);
      strncpy(ipp->name,attrValue,len);
      ipp->name[len] = '\0';
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartIppObj(char *appName, char *objName)
{
  pIppConfig ipp;

#ifdef XML_DEBUG
  printf("xmlStartIppObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(ippConfig));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  ipp = (pIppConfig)psiValue;
  memset(ipp,0,sizeof(ippConfig));
  strcpy(ipp->dev,IPP_PRINTER_DEV_DEFAULT);

  return PSX_STS_OK;
}

PSX_STATUS xmlEndIppObj(char *appName, char *objName)
{
  pIppConfig ipp = (pIppConfig)psiValue;
  PSI_HANDLE ippApp = NULL;

#ifdef XML_DEBUG
  printf("xmlEndIppObj(calling cfm to get appName %s, objName %s)\n",
	 appName,objName);
#endif

  if (xmlState.verify) {
    if ((ipp->enabled) &&
        ((ipp->make[0] == '\0') || (ipp->name[0] == '\0'))) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetIppAttr(): ipp enabled, make and name must be set\n");
    }
  }
  else {
    ippApp = BcmPsi_appOpen(IPP_APP_ID);
    if (ippApp != NULL) {
      BcmPsi_objRemove(ippApp, IPP_CONFIG);
      BcmPsi_objStore(ippApp, IPP_CONFIG, (void*)ipp, sizeof(ippConfig));
    }
  }


#ifdef XML_DEBUG1
  printf("\n============End system ipp info=========\n");
  printf("xmlEndIppObj(): psiValue->enabled %d, make %s, name %s\n",
         (int)ipp->enabled,ipp->make,ipp->name);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#endif /* SUPPORT_IPP */

//=====================================================================================
//=============================== Start PMapObj Functions ==============================
//=====================================================================================
#if SUPPORT_PORT_MAP
void xmlSetPMapAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   PPORT_MAP_CFG_ENTRY pmap, pent;
   int entry = 0;
   int num = 0;
   char groupName[IFC_TINY_LEN], ifList[IFC_LARGE_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_PMAP) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     pmap = (PPORT_MAP_CFG_ENTRY)value;
     /* determine the size of route table */
     //num = length/sizeof(PORT_MAP_CFG_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       pent = &pmap[entry];
       strncpy(groupName,pent->groupName,IFC_TINY_LEN);
       strncpy(ifList,pent->grpedIntf,IFC_LARGE_LEN);
       fprintf(file,"<%s id=\"%d\" groupName=\"%s\" ifList=\"%s\" vendorid0=\"%s\" vendorid1=\"%s\" vendorid2=\"%s\" vendorid3=\"%s\" vendorid4=\"%s\"/>\n",
	       name,(entry+1),groupName,ifList,
               pent->vid[0].vendorid, pent->vid[1].vendorid,
               pent->vid[2].vendorid, pent->vid[3].vendorid,
               pent->vid[4].vendorid);

     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",pmapObjs[id-1].objName); 
   } /* pmap table */
}


PSX_STATUS xmlGetPMapObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetPMapObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; pmapObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pmapObjs[i].objName, objName) == 0) {
      if (*(pmapObjs[i].getAttrFnc) != NULL) {
	sts = (*(pmapObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartPMapObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartPMapObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; pmapObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pmapObjs[i].objName, objName) == 0) {
      if (*(pmapObjs[i].startObjFnc) != NULL) {
        sts = (*(pmapObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndPMapObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndPMapObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; pmapObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pmapObjs[i].objName, objName) == 0) {
      if (*(pmapObjs[i].endObjFnc) != NULL) {
	sts = (*(pmapObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

PSX_STATUS xmlGetPMapAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetPMapAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    */
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(PORT_MAP_CFG_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(PORT_MAP_CFG_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPMapObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndPMapObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPMapObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setStaticPMapSize(xmlState.tableSize);
    BcmDb_setStaticPMapInfo((PPORT_MAP_CFG_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndPMapObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetPMapEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PPORT_MAP_CFG_ENTRY pmapTbl = (PPORT_MAP_CFG_ENTRY)psiValue;
  int index = 0;
  char vendid[IFC_TINY_LEN];

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetPMapEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetPMapEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"groupName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pmapTbl[i].groupName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapEntryAttr(): invalid VLAN name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"ifList") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pmapTbl[i].grpedIntf, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapEntryAttr(): invalid interface list %s\n",attrValue);
    }
  } else {
    for (index = 0; index < DHCP_MAX_VENDOR_IDS; index++) {
      memset(vendid, 0, IFC_TINY_LEN);
      sprintf(vendid, "vendorid%d", index);
      if (strcmp(attrName, vendid) == 0) {
        if (xmlState.verify && attrValue[0] != '\0')
          status = BcmDb_validateLength(attrValue, DHCP_VENDOR_ID_LEN);
        if (status == DB_OBJ_VALID_OK)
          strcpy(pmapTbl[i].vid[index].vendorid, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetPMapEntryAttr(): invalid DHCP Vendor ID %s\n",attrValue);
        }
      }
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPMapEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}


void xmlSetPMapIfcCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PORT_MAP_IFC_STATUS_INFO pmapIfcCfgInfo;
   char ifName[IFC_TINY_LEN];
   static char* state[] = {"disable","enable"};

   if ((name == NULL) || (value == NULL)) {
      printf ("xmlSetPMapIfcCfgAttr: Name and Value are NULL\n");
      return;
   }

   memcpy(&pmapIfcCfgInfo, (char *)value, length);
   int idx = (int)pmapIfcCfgInfo.status;
   strncpy(ifName, pmapIfcCfgInfo.ifName,IFC_TINY_LEN);
   fprintf(file,"<%s %s=\"%s\" %s=\"%s\"/>\n",name,PSX_ATTR_PMAP_IFC_NAME,
           ifName, PSX_ATTR_PMAP_IFC_STATUS, state[idx]);
}

PSX_STATUS xmlGetPMapIfcCfgAttr(char *attrName, char* attrValue)
{
  PPORT_MAP_IFC_STATUS_INFO pmapIfcCfg = (PPORT_MAP_IFC_STATUS_INFO)psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetDnsAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,PSX_ATTR_PMAP_IFC_STATUS,strlen(PSX_ATTR_PMAP_IFC_STATUS)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapIfcCfgAttr(): invalid state %s\n",attrValue);
    }
    else {
      pmapIfcCfg->status = xmlStateToPsi(attrValue);
    }
  }
  else if (strncmp(attrName,PSX_ATTR_PMAP_IFC_NAME,strlen(PSX_ATTR_PMAP_IFC_NAME)) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateInterfaceName(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPMapIfcCfgAttr(): invalid interface %s\n",attrValue);
    }
    else {
      memset(pmapIfcCfg->ifName, 0, IFC_TINY_LEN);
      strncpy(pmapIfcCfg->ifName, (const char*)attrValue, IFC_TINY_LEN);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartPMapIfcCfgObj(char *appName, char *objName)
{
  PPORT_MAP_IFC_STATUS_INFO pmapIfcCfg;

#ifdef XML_DEBUG
  printf("xmlStartDftGwObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(PORT_MAP_IFC_STATUS_INFO));
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(PORT_MAP_IFC_STATUS_INFO));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  pmapIfcCfg = (PPORT_MAP_IFC_STATUS_INFO)psiValue;
  pmapIfcCfg->status = atoi(BcmDb_getDefaultValue("pmapIfcCfgStatus"));
  strncpy(pmapIfcCfg->ifName, BcmDb_getDefaultValue("pmapIfcCfgName"), IFC_TINY_LEN);
  return PSX_STS_OK;
}

PSX_STATUS xmlEndPMapIfcCfgObj(char *appName, char *objName)
{
  PPORT_MAP_IFC_STATUS_INFO pmapIfcCfg = (PPORT_MAP_IFC_STATUS_INFO)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndDnsObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  /* no mandatory attribute */
  if (!xmlState.verify)
    BcmDb_setPMapIfcCfgInfo(pmapIfcCfg);

#ifdef XML_DEBUG1
  printf("\n============End PORT MAPPING Info=========\n");
  printf("xmlEndPMapIfcCfgObj(): status %d, psiValue->ifName %s\n",
	pmapIfcCfg->status,pmapIfcCfg->ifName);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}
#endif // SUPPORT_PORT_MAP
//=====================================================================================
//=============================== End PMapObj Functions ==============================
//=====================================================================================

///////////////////////////////////////////////////////////////////////////////////
#if SUPPORT_IPSEC

PSX_STATUS xmlSetIPSecNode(FILE *file, const char *appName, const UINT16 id, const PSI_VALUE value, const UINT16 length)
{
  int i;
  UINT16 num;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlSetIPSecNode(appName %s, id %d, <value>, length %d)\n", appName, id, length);
#endif

  if ((appName == NULL) || (value == NULL)) {
    return PSX_STS_ERR_FIND_OBJ;
  }

  if (id == IPSEC_PSI_TABLESIZE) {
     num = *((UINT16*)value);
     if ( num > 0 ) {
       fprintf(file,"<%s tableSize=\"%d\">\n",TBLNAME_IPSEC, (int)num);
     }
  }
  else if (id == IPSEC_PSI_CONNECTIONS) {
    CIPSecConnParms *connections = (CIPSecConnParms *)value;
    num = xmlGetTableSize(file);
    for(i=0; i < num; i++) {
      fprintf(file,"<%s ", ENTRYNAME_IPSEC);
      fprintf(file,"en = \"%d\" ipsConnName=\"%s\" ipsRemoteGWAddr=\"%s\" ipsLocalIPMode=\"%s\" ipsLocalIP=\"%s\" ipsLocalMask=\"%s\" ",
            connections[i].en, connections[i].ipsConnName, connections[i].ipsRemoteGWAddr, connections[i].ipsLocalIPMode, connections[i].ipsLocalIP, connections[i].ipsLocalMask);
      /*modified by z67625 增加将传输模式和接口写入配置文件 start*/
      fprintf(file,"ipsRemoteIPMode=\"%s\" ipsRemoteIP=\"%s\" ipsRemoteMask=\"%s\" ipsTransMode=\"%s\" ipsProtocol=\"%s\" ipsInterface=\"%s\" ipsKeyExM=\"%s\" ipsAuthM=\"%s\" ipsPSK=\"%s\" ipsCertificateName=\"%s\" ",
            connections[i].ipsRemoteIPMode, connections[i].ipsRemoteIP, connections[i].ipsRemoteMask, connections[i].ipsTransMode, connections[i].ipsProtocol, connections[i].ipsInterface, connections[i].ipsKeyExM, connections[i].ipsAuthM, connections[i].ipsPSK, connections[i].ipsCertificateName);
      /*modified by z67625 增加将传输模式和接口写入配置文件 end*/
      fprintf(file,"ipsPerfectFSEn=\"%s\" ipsManualEncryptionAlgo=\"%s\" ipsManualEncryptionKey=\"%s\" ipsManualAuthAlgo=\"%s\" ",
            connections[i].ipsPerfectFSEn, connections[i].ipsManualEncryptionAlgo, connections[i].ipsManualEncryptionKey, connections[i].ipsManualAuthAlgo);
      fprintf(file,"ipsManualAuthKey=\"%s\" ipsSPI=\"%s\" ipsPh1Mode=\"%s\" ipsPh1EncryptionAlgo=\"%s\" ipsPh1IntegrityAlgo=\"%s\" ipsPh1DHGroup=\"%s\" ipsPh1KeyTime=\"%d\" ",
            connections[i].ipsManualAuthKey, connections[i].ipsSPI, connections[i].ipsPh1Mode, connections[i].ipsPh1EncryptionAlgo, connections[i].ipsPh1IntegrityAlgo, connections[i].ipsPh1DHGroup, connections[i].ipsPh1KeyTime);
      fprintf(file,"ipsPh2EncryptionAlgo=\"%s\" ipsPh2IntegrityAlgo=\"%s\" ipsPh2DHGroup=\"%s\" ipsPh2KeyTime=\"%d\" ",
            connections[i].ipsPh2EncryptionAlgo, connections[i].ipsPh2IntegrityAlgo, connections[i].ipsPh2DHGroup, connections[i].ipsPh2KeyTime);
      fprintf(file, "/>\n");

    }
    if (num > 0) {
      fprintf(file,"</%s>\n", TBLNAME_IPSEC);
    }
  }
  else {
    return PSX_STS_ERR_FIND_OBJ;
  }
    
  return PSX_STS_OK;
}

PSX_STATUS xmlGetIPSecNode(char *appName, char *objName, char *attrName, char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;
  DB_OBJ_VALID_STATUS status=DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlGetIPSecNode(appName %s, objName %s, attrName %s, attrValue %s)\n", appName,objName,attrName,attrValue);
#endif

  if (strcmp(objName, TBLNAME_IPSEC) == 0) {
    if (strcasecmp(attrName,"tableSize") == 0) {
      if (xmlState.verify) {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK) {
        xmlState.tableIndex = 0;
        xmlState.tableSize = atoi(attrValue);
        if (xmlState.tableSize > 0) {
          int size = sizeof(CIPSecConnParms) * xmlState.tableSize;
          if ((psiValue = (void *) malloc(size)) != NULL) {
            /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
            memset(psiValue, 0, size);
            /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
            CIPSecConnParms *entry = (CIPSecConnParms *) psiValue;
            for (i=0; i<xmlState.tableSize; i++) {
                BcmIPSec_GetDefaults(entry);
                entry++;
            }
          }
          else {
            syslog(LOG_ERR,"xmlGetIPSecNode(): out of memory\n");
            sts = PSX_STS_ERR_FATAL;
          }
        }
        else {
          fprintf(stderr, "psivalue=null\n");
          psiValue = NULL;
        }
      }  
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid tableSize %s\n",attrValue);
      }
    }
    else {
      syslog(LOG_ERR,"xmlGetIPSecNode(): Unrecognizable attrName %s, ignored\n", attrName);
    }
  }
  else if (strcmp(objName, ENTRYNAME_IPSEC) == 0) {
    if (psiValue == NULL) {
        syslog(LOG_ERR,"xmlGetIPSecNode(): no memory allocated to psiValue\n");
        return PSX_STS_ERR_FATAL;
    }
    CIPSecConnParms *connections = (CIPSecConnParms *) psiValue;
    if (strcmp(attrName, "en") == 0) {
      if (xmlState.verify) {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status != DB_OBJ_VALID_OK) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
      else {
        connections[xmlState.tableIndex].en = atoi(attrValue);
      }
    }
    else if (strcmp(attrName, "ipsConnName") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsConnName, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsRemoteGWAddr") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsRemoteGWAddr, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsLocalIPMode") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsLocalIPMode, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsLocalIP") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsLocalIP, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsLocalMask") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsLocalMask, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsRemoteIPMode") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsRemoteIPMode, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsRemoteIP") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsRemoteIP, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsRemoteMask") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsRemoteMask, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsKeyExM") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsKeyExM, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsAuthM") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsAuthM, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPSK") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPSK, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsCertificateName") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsCertificateName, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPerfectFSEn") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPerfectFSEn, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsManualEncryptionAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsManualEncryptionAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsManualEncryptionKey") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsManualEncryptionKey, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsManualAuthAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsManualAuthAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsManualAuthKey") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsManualAuthKey, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsSPI") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsSPI, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh1Mode") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh1Mode, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh1EncryptionAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh1EncryptionAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh1IntegrityAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh1IntegrityAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh1DHGroup") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh1DHGroup, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh1KeyTime") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK) {
        connections[xmlState.tableIndex].ipsPh1KeyTime = atoi(attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh2EncryptionAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh2EncryptionAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh2IntegrityAlgo") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh2IntegrityAlgo, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh2DHGroup") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(connections[xmlState.tableIndex].ipsPh2DHGroup, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "ipsPh2KeyTime") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK) {
        connections[xmlState.tableIndex].ipsPh2KeyTime = atoi(attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
      }
    }
    /*add by z67625 增加支持获取传输模式和网络接口 start*/
    else if(strcmp(attrName,"ipsTransMode") == 0) {
        if(xmlState.verify && attrValue[0] !='\0'){
            status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
        }
        if(status == DB_OBJ_VALID_OK) {
            strcpy(connections[xmlState.tableIndex].ipsTransMode, attrValue);
        }
        else{
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
        }
    }
    else if(strcmp(attrName,"ipsProtocol") == 0) {
        if(xmlState.verify && attrValue[0] !='\0'){
            status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
        }
        if(status == DB_OBJ_VALID_OK) {
            strcpy(connections[xmlState.tableIndex].ipsProtocol, attrValue);
        }
        else{
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
        }
    }
    else if(strcmp(attrName,"ipsInterface") == 0) {
        if(xmlState.verify && attrValue[0] !='\0'){
            status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
        }
        if(status == DB_OBJ_VALID_OK) {
            strcpy(connections[xmlState.tableIndex].ipsInterface, attrValue);
        }
        else{
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIPSecNode(): invalid parameter %s\n",attrValue);
        }
    }
    /*add by z67625 增加支持获取传输模式和网络接口 end*/
    else {
      syslog(LOG_ERR,"xmlGetIPSecNode(): Unrecognizable attrName %s, ignored\n", attrName);
    }
  }
  else {
    syslog(LOG_ERR,"xmlGetIPSecNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
  }
  return sts;
} 

PSX_STATUS xmlStartIPSecNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlStartIPSecNode(appName %s, objName %s)\n",appName,objName);
#endif

  if (strcmp(objName, TBLNAME_IPSEC) == 0) {

  }
  else if (strcmp(objName, ENTRYNAME_IPSEC) == 0) {
  }
  else {
    syslog(LOG_ERR,"xmlEndIPSecNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
    sts = PSX_STS_ERR_FIND_OBJ;
  }

  return sts;
} 

PSX_STATUS xmlEndIPSecNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlEndIPSecNode(appName %s, objName %s)\n", appName, objName);
#endif

  if (strcmp(objName, TBLNAME_IPSEC) == 0) {
    if (!xmlState.verify) {
      PSI_HANDLE psiIPSec = BcmPsi_appOpen(IPSEC_APP_ID);
      UINT16 tablesize = xmlState.tableSize;
      BcmPsi_objStore(psiIPSec, IPSEC_PSI_TABLESIZE, &tablesize, sizeof(tablesize));
      if (tablesize > 0) {
        BcmPsi_objStore(psiIPSec, IPSEC_PSI_CONNECTIONS, psiValue, tablesize*sizeof(CIPSecConnParms));
      }
      BcmPsi_appClose(psiIPSec);
    }
    if (psiValue != NULL) {
      free(psiValue);
      psiValue=NULL;
    }
  }
  else if (strcmp(objName, ENTRYNAME_IPSEC) == 0) {
    xmlState.tableIndex++;
  }
  else {
    syslog(LOG_ERR,"xmlEndIPSecNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
    sts = PSX_STS_ERR_FIND_OBJ;
  }
    
  return sts;
} 

#endif


///////////////////////////////////////////////////////////////////////////////////

#if SUPPORT_CERT

PSX_STATUS xmlSetCertNode(FILE *file, const char *appName, const UINT16 id, const PSI_VALUE value, const UINT16 length)
{
  int i;
  UINT16 num;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlSetCertNode(appName %s, id %d, <value>, length %d)\n", appName, id, length);
#endif

  if ((appName == NULL) || (value == NULL)) {
    return PSX_STS_ERR_FIND_OBJ;
  }

  if (id == CERT_LOCAL_TBL_SIZE) {
     num = *((UINT16*)value);
     if ( num > 0 ) {
       fprintf(file,"<%s tableSize=\"%d\">\n",TBLNAME_CERT_LOCAL, (int)num);
     }
  }
  else if (id == CERT_CA_TBL_SIZE) {
     num = *((UINT16*)value);
     if ( num > 0 ) {
       fprintf(file,"<%s tableSize=\"%d\">\n",TBLNAME_CERT_CA, (int)num);
     }
  }
  else if (id == CERT_LOCAL_TBL || id == CERT_CA_TBL) {
    CERT_DATA_ENTRY *table = (CERT_DATA_ENTRY *)value;
    num = xmlGetTableSize(file);
    char tblname[IFC_SMALL_LEN];
    if (id == CERT_CA_TBL) {
      strcpy(tblname, ENTRYNAME_CERT_CA);
    }
    else {
      strcpy(tblname, ENTRYNAME_CERT_LOCAL);
    }
    for(i=0; i < num; i++) {
      fprintf(file,"<%s\n ", tblname);
      //modified by z67625
      fprintf(file,"certName = \"%s\"\ncertSubject=\"%s\"\ncertType=\"%s\"\ncertPassword=\"%s\"\ncertContent=\"%s\"\ncertPrivKey=\"%s\"\ncertRContent=\"%s\"\ncertReqPub=\"%s\"\nrefCount=\"%d\"",
            table[i].certName, table[i].certSubject, table[i].certType, table[i].certPassword, table[i].certContent, table[i].certPrivKey, table[i].certRContent, table[i].certReqPub, table[i].refCount);
      //modified by z67625
      fprintf(file, "/>\n");

    }
    if (num > 0) {
      if (id == CERT_CA_TBL) {
        fprintf(file,"</%s>\n", TBLNAME_CERT_CA);
      }
      else {
        fprintf(file,"</%s>\n", TBLNAME_CERT_LOCAL);
      }
    }
  }
  else {
    return PSX_STS_ERR_FIND_OBJ;
  }
    
  return PSX_STS_OK;
}

PSX_STATUS xmlGetCertNode(char *appName, char *objName, char *attrName, char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;
  DB_OBJ_VALID_STATUS status=DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlGetCertNode(appName %s, objName %s, attrName %s, attrValue %s)\n", appName,objName,attrName,attrValue);
#endif

  if (strcmp(objName, TBLNAME_CERT_LOCAL) == 0 || strcmp(objName, TBLNAME_CERT_CA) == 0) {
    if (strcasecmp(attrName,"tableSize") == 0) {
      if (xmlState.verify) {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK) {
        xmlState.tableIndex = 0;
        xmlState.tableSize = atoi(attrValue);
        if (xmlState.tableSize > 0) {
          int size = sizeof(CERT_DATA_ENTRY) * xmlState.tableSize;
          if ((psiValue = (void *) malloc(size)) != NULL) {
            /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
            memset(psiValue, 0, size);
            /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
            PCERT_DATA_ENTRY entry = (CERT_DATA_ENTRY *) psiValue;
            for (i=0; i<xmlState.tableSize; i++) {
                BcmCcm_getDefaults(entry);
                entry++;
            }
          }
          else {
            syslog(LOG_ERR,"xmlGetIPSecNode(): out of memory\n");
            sts = PSX_STS_ERR_FATAL;
          }
        }
        else {
          fprintf(stderr, "psivalue=null\n");
          psiValue = NULL;
        }
      }  
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIPSecNode(): invalid tableSize %s\n",attrValue);
      }
    }
    else {
      syslog(LOG_ERR,"xmlGetIPSecNode(): Unrecognizable attrName %s, ignored\n", attrName);
    }
  }
  else if (strcmp(objName, ENTRYNAME_CERT_LOCAL) == 0 || strcmp(objName, ENTRYNAME_CERT_CA) == 0) {
    if (psiValue == NULL) {
        syslog(LOG_ERR,"xmlGetIPSecNode(): no memory allocated to psiValue\n");
        return PSX_STS_ERR_FATAL;
    }
    CERT_DATA_ENTRY *table = (CERT_DATA_ENTRY *) psiValue;
    if (strcmp(attrName, "certName") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_NAME_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certName, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certSubject") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_NAME_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certSubject, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certType") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_TYPE_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certType, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certPassword") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_BUFF_MAX_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certPassword, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certContent") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_KEY_MAX_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certContent, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certPrivKey") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_KEY_MAX_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certPrivKey, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    //add by z67625
    else if (strcmp(attrName, "certRContent") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_KEY_MAX_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certRContent, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "certReqPub") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateLength(attrValue, CERT_KEY_MAX_LEN);
      }
      if (status == DB_OBJ_VALID_OK) {
        strcpy(table[xmlState.tableIndex].certReqPub, attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else if (strcmp(attrName, "refCount") == 0) {
      if (xmlState.verify && attrValue[0] != '\0') {
        status = BcmDb_validateNumber(attrValue);
      }
      if (status == DB_OBJ_VALID_OK) {
        table[xmlState.tableIndex].refCount = atoi(attrValue);
      }
      else {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetCertNode(): invalid parameter %s\n",attrValue);
      }
    }
    else {
      syslog(LOG_ERR,"xmlGetCertNode(): Unrecognizable attrName %s, ignored\n", attrName);
    }
  }
  else {
    syslog(LOG_ERR,"xmlGetCertNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
  }
  return sts;
} 

PSX_STATUS xmlStartCertNode(char *appName, char *objName)
{
#ifdef XML_DEBUG
  fprintf(stderr, "xmlStartCertNode(appName %s, objName %s)\n", appName, objName);
#endif
  return PSX_STS_OK;
} 

PSX_STATUS xmlEndCertNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_ERR_FIND_OBJ;

#ifdef XML_DEBUG
  fprintf(stderr, "xmlEndCertNode(appName %s, objName %s)\n", appName, objName);
#endif

  if (strcmp(objName, TBLNAME_CERT_LOCAL) == 0) {
    if (!xmlState.verify) {
      PSI_HANDLE psiIPSec = BcmPsi_appOpen(CERT_APP_ID);
      UINT16 tablesize = xmlState.tableSize;
      BcmPsi_objStore(psiIPSec, CERT_LOCAL_TBL_SIZE, &tablesize, sizeof(tablesize));
      if (tablesize > 0) {
        BcmPsi_objStore(psiIPSec, CERT_LOCAL_TBL, psiValue, tablesize*sizeof(CERT_DATA_ENTRY));
      }
      BcmPsi_appClose(psiIPSec);
    }
    if (psiValue != NULL) {
      free(psiValue);
      psiValue=NULL;
    }
  }
  else if (strcmp(objName, TBLNAME_CERT_CA) == 0) {
    if (!xmlState.verify) {
      PSI_HANDLE psiIPSec = BcmPsi_appOpen(CERT_APP_ID);
      UINT16 tablesize = xmlState.tableSize;
      BcmPsi_objStore(psiIPSec, CERT_CA_TBL_SIZE, &tablesize, sizeof(tablesize));
      if (tablesize > 0) {
        BcmPsi_objStore(psiIPSec, CERT_CA_TBL, psiValue, tablesize*sizeof(CERT_DATA_ENTRY));
      }
      BcmPsi_appClose(psiIPSec);
    }
    if (psiValue != NULL) {
      free(psiValue);
      psiValue=NULL;
    }
  }
  else if (strcmp(objName, ENTRYNAME_CERT_LOCAL) == 0 || strcmp(objName, ENTRYNAME_CERT_CA) == 0) {
    xmlState.tableIndex++;
  }
  else {
    syslog(LOG_ERR,"xmlEndIPSecNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
    sts = PSX_STS_ERR_FIND_OBJ;
  }
    
  return sts;
} 

#endif


#ifdef PORT_MIRRORING

static const PSX_OBJ_ITEM portMirrorObjs[] = {
   { TBLNAME_PORT_MIRROR_CFG, NULL, xmlGetPortMirrorCfgAttr, NULL, xmlEndPortMirrorCfgObj },
   { OBJNAME_PORT_MIRROR_STATUS, xmlSetPortMirrorStatusAttr, xmlGetPortMirrorStatusAttr, xmlStartPortMirrorStatusAttr, xmlEndPortMirrorStatusAttr },
   { ENTRYNAME_PORT_MIRROR_CFG, xmlSetPortMirrorCfgEntryAttr, xmlGetPortMirrorCfgEntryAttr, NULL, xmlEndPortMirrorCfgEntryAttrObj },
   { "", NULL, NULL, NULL, NULL }
};

PSX_STATUS xmlGetPortMirrorCfgAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetPortMirrorCfgAttr(attrName %s, attrValue %s)\n",
    attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_MAX_PORT_MIRRORS);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(PortMirrorCfg) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
            xmlMemProblem();
            return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(PortMirrorCfg) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPortMirrorCfgAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPortMirrorCfgObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndPortMirrorCfgObj(calling cfm to get appName %s, objName %s\n",
    appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndPortMirrorCfgObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify)
    BcmDb_setPortMirrorCfg((PortMirrorCfg *)psiValue, xmlState.tableSize);

#ifdef XML_DEBUG
  printf("\n============End Port Mirror Cfg=========\n");
  printf("xmlEndPortMirrorCfgObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

void xmlSetPortMirrorStatusAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
         const UINT16 length)
{
   DbgPortMirrorStatus *PMStatus;

   if ((name == NULL) || (value == NULL))
      return;

   PMStatus = (DbgPortMirrorStatus *)value;
   
   fprintf(file,"<%s PortMirrorTableSize=\"%d\"/>\n", name,PMStatus->count);
}

PSX_STATUS xmlGetPortMirrorStatusAttr(char *attrName, char* attrValue)
{
  DbgPortMirrorStatus *PMstatus= (DbgPortMirrorStatus *) psiValue;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetPortMirrorStatusAttr(attrName %s, attrValue %s)\n", attrName,attrValue);
#endif

  if (strcmp(attrName,"PortMirrorTableSize") == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      PMstatus->count = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPortMirrorStatusAttr(): invalid number of Port mirror entries %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartPortMirrorStatusAttr(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartPortMirrorStatusAttr(calling cfm to get appName %s, objName %s\n",
    appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(DbgPortMirrorStatus));
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue, 0, sizeof(DbgPortMirrorStatus));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndPortMirrorStatusAttr(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlEndPortMirrorStatusAttr(calling cfm to get appName %s, objName %s\n",
    appName,objName);
#endif

  if (!xmlState.verify)
    BcmDb_setPortMirrorStatus((DbgPortMirrorStatus *)psiValue);

#ifdef XML_DEBUG
  printf("\n============End Port Mirror Status=========\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

void xmlSetPortMirrorCfgEntryAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
            const UINT16 length)
{
   PortMirrorCfg  *pPortMirrorCfg, *ptr ;
   static char* monitorPort[] = {"WAN0"} ;
   static char* rec[] = {"1","2"} ;
   static char* dir[] = {"IN","OUT"} ;
   static char* flowType[] = {"802.3"} ;
   static char* mirrorPort[] = {"eth0","eth1","usb"} ;
   static char* status[] = {"disabled","enabled"} ;
   int ifCount, i ;

   if ((name == NULL) || (value == NULL))
      return;

   pPortMirrorCfg = (PortMirrorCfg *)value;
   ifCount = length/sizeof(PortMirrorCfg);

   if ( ifCount != 0 )   
      fprintf(file,"<%s tableSize=\"%d\">\n",TBLNAME_PORT_MIRROR_CFG,ifCount);
   for (i=0; i < ifCount; i++) {
     ptr = &pPortMirrorCfg[i];
     fprintf(file,"<%s rec=\"%s\" monitorPort=\"%s\" dir=\"%s\" flowType=\"%s\" mirrorPort=\"%s\" status=\"%s\"/>\n",
        name,
        rec[ptr->recNo-1],
        monitorPort[ptr->monitorPort-1],
        dir[ptr->mirrorCfg.dir-1],
        flowType[ptr->mirrorCfg.flowType-1],
        mirrorPort[ptr->mirrorCfg.mirrorPort-1],
        status[ptr->mirrorCfg.status-1]) ;
   }

   if ( ifCount != 0 )
      fprintf(file,"</%s>\n",TBLNAME_PORT_MIRROR_CFG);
}

PSX_STATUS xmlGetPortMirrorCfgEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  PortMirrorCfg* pmTbl = (PortMirrorCfg*)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetPortMirrorCfgEntryAttr (): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetPortMirrorCfgEntryAttr (): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetPortMirrorCfgEntryAttr(attrName %s, attrValue %s)\n",
    attrName,attrValue);
#endif
  if (strcmp(attrName,"rec") == 0) {
    pmTbl[i].recNo = BcmDb_getPortMirrorCfgRecNo(attrValue);
  }
  else if (strcmp(attrName,"monitorPort") == 0) {
    pmTbl[i].monitorPort = BcmDb_getPortMirrorCfgMonitorPort(attrValue);
  }
  else if (strcmp(attrName,"dir") == 0) {
    pmTbl[i].mirrorCfg.dir = BcmDb_getPortMirrorCfgDir(attrValue);
  }
  else if (strcmp(attrName,"flowType") == 0) {
    pmTbl[i].mirrorCfg.flowType = BcmDb_getPortMirrorCfgFlowType(attrValue);
  }
  else if (strcmp(attrName,"mirrorPort") == 0) {
    pmTbl[i].mirrorCfg.mirrorPort = BcmDb_getPortMirrorCfgMirrorPort(attrValue);
  }
  else if (strcmp(attrName,"status") == 0) {
    pmTbl[i].mirrorCfg.status = BcmDb_getPortMirrorCfgStatus(attrValue);
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPortMirrorCfgEntryAttrObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}


PSX_STATUS xmlSetPortMirrorObjNode (FILE* file, const char *appName, const UINT16 id,
             const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(portMirrorObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(portMirrorObjs[id].setAttrFnc) != NULL)
     (*(portMirrorObjs[id].setAttrFnc))(file, portMirrorObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlGetPortMirrorObjNode(char *appName, char *objName, char *attrName,
             char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetPortMirrorObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
    appName,objName,attrName,attrValue);
#endif

  for ( i = 0; portMirrorObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(portMirrorObjs[i].objName, objName) == 0) {
      if (*(portMirrorObjs[i].getAttrFnc) != NULL) {
   sts = (*(portMirrorObjs[i].getAttrFnc))(attrName, attrValue);
   break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetPortMirrorObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartPortMirrorObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartPortMirrorObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; portMirrorObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(portMirrorObjs[i].objName, objName) == 0) {
      if (*(portMirrorObjs[i].startObjFnc) != NULL) {
        sts = (*(portMirrorObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartPortMirrorObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndPortMirrorObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndPortMirrorObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; portMirrorObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(portMirrorObjs[i].objName, objName) == 0) {
      if (*(portMirrorObjs[i].endObjFnc) != NULL) {
   sts = (*(portMirrorObjs[i].endObjFnc))(appName, objName);
   break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndPortMirrorObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 

#endif /*  PORT_MIRRORING */


#ifdef SUPPORT_SAMBA
PSX_STATUS xmlSetSambaObjNode (FILE* file, const char *appName, const UINT16 id,
             const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(sambaObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(sambaObjs[id].setAttrFnc) != NULL)
     (*(sambaObjs[id].setAttrFnc))(file, sambaObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlGetSambaObjNode(char *appName, char *objName, char *attrName,
             char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetSambaNode(appName %s, objName %s, attrName %s, attrValue %s\n",
    appName,objName,attrName,attrValue);
#endif

  for ( i = 0; sambaObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(sambaObjs[i].objName, objName) == 0) {
      if (*(sambaObjs[i].getAttrFnc) != NULL) {
   sts = (*(sambaObjs[i].getAttrFnc))(attrName, attrValue);
   break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetSambaObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlStartSambaObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartSambaObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; sambaObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(sambaObjs[i].objName, objName) == 0) {
      if (*(sambaObjs[i].startObjFnc) != NULL) {
        sts = (*(sambaObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartSambaObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndSambaObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndSambaObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; sambaObjs[i].objName[0] != '\0'; i++ ) {
#ifdef XML_DEBUG
     printf("xmlEndSambaObjNode(objName 1 %s, objName %s\n",sambaObjs[i].objName,objName);
#endif
    if (strcmp(sambaObjs[i].objName, objName) == 0) { 
      if (*(sambaObjs[i].endObjFnc) != NULL) {
   sts = (*(sambaObjs[i].endObjFnc))(appName, objName);
   break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndSambaObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
}

void xmlSetSambaAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				 const UINT16 length)
{
   PSAMBA_MGMT_CFG sambaMgmtCfg;
   char *status[] = {"disable", "enable"};
   int num = 0;
   PSAMBA_ACCNT_ENTRY accntEntry = NULL, pEntry = NULL;
   int entry = 0;
   char *base64 = NULL;

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name, OBJNAME_SAMBA_STATUS) == 0)
   {
      sambaMgmtCfg = (PSAMBA_MGMT_CFG)value;
      fprintf(file,"<%s enableSamba=\"%s\" workgrp=\"%s\" svrname=\"%s\" netbname=\"%s\"/>\n", 
            name, status[sambaMgmtCfg->enableSamba], sambaMgmtCfg->workgrp, sambaMgmtCfg->svrname, sambaMgmtCfg->netbname);
   }
   else if (strcmp(name, TBLNAME_SAMBA_ACCOUNT) == 0)
   {
      num = *(UINT16*)value;
      if ( num != 0 )
         fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   
   }
   else if (strcmp(name, ENTRYNAME_SAMBA_ACCOUNT) == 0)
   {
       accntEntry = (PSAMBA_ACCNT_ENTRY)value;
       num = xmlGetTableSize(file);
 
       for (entry = 0; entry < num; entry++) {
           pEntry = (accntEntry+entry);
#ifdef SUPPORT_SAMBA_ACCOUNT_UNENCRYPTED
		   fprintf(file,"<%s id=\"%d\" instanceId=\"%d\" username=\"%s\" password=\"%s\" privilege=\"%d\" directory= \"%s\"/>\n",
                 name,entry+1,pEntry->instId,pEntry->userName, pEntry->passwd, pEntry->privilege, pEntry->dir);
#else
           base64Encode((unsigned char *)pEntry->passwd, strlen(pEntry->passwd), &base64);
           fprintf(file,"<%s id=\"%d\" instanceId=\"%d\" username=\"%s\" password=\"%s\" privilege=\"%d\" directory= \"%s\"/>\n",
                 name,entry+1,pEntry->instId,pEntry->userName, base64, pEntry->privilege, pEntry->dir);
           free(base64);
#endif
       }
  
       if ( num != 0 )
         fprintf(file,"</%s>\n",sambaObjs[id-1].objName);
      
   }
}

PSX_STATUS xmlGetSambaAccntAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetSambaAccntAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SAMBA_MAX_ACCOUNT);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SAMBA_ACCNT_ENTRY) * xmlState.tableSize);
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), SAMBA_MAX_ACCOUNT);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndSambaAccntObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndSambaAccntObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSambaAccntObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setSmbAccntSize(xmlState.tableSize);
    BcmDb_setSmbAccntInfo((PSAMBA_ACCNT_ENTRY)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End Samba Class=========\n");
  printf("xmlEndSambaAccntObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
  printf("LEON DEBUG: xmlEndSambaAccntObj: %d %d\n", xmlState.verify, xmlState.errorFlag);
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlStartSambaAccntEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PSAMBA_ACCNT_ENTRY smbAccntTbl = (PSAMBA_ACCNT_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlStartSambaAccntEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }

  memset((char*)(smbAccntTbl+i), 0, sizeof(SAMBA_ACCNT_ENTRY));
  smbAccntTbl[i].userName[0]     = '\0';
  smbAccntTbl[i].passwd[0]       = '\0';
  smbAccntTbl[i].dir[0]          = '\0';
  smbAccntTbl[i].privilege       = 0;
  smbAccntTbl[i].instId          = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetSambaAccntEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSAMBA_ACCNT_ENTRY smbAccntTbl = (PSAMBA_ACCNT_ENTRY)psiValue;
  char* data = NULL;
  int   dataLen = 0;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetSambaAccntEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"instanceId") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
    {
       smbAccntTbl[i].instId = atoi(attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid instance Id %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"username") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
    if (status == DB_OBJ_VALID_OK)
    {  
      strcpy(smbAccntTbl[i].userName, attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid username %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"password") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN);
    if (status == DB_OBJ_VALID_OK)
    {
#ifdef SUPPORT_SAMBA_ACCOUNT_UNENCRYPTED
	   strcpy(smbAccntTbl[i].passwd, attrValue);
#else
	   data = (char *)malloc(strlen(attrValue) * 3/4 + 8);
       base64Decode((unsigned char *)data, attrValue, &dataLen);
       data[dataLen] = '\0';
       strcpy(smbAccntTbl[i].passwd, data);
       free(data);
#endif
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid password %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"privilege") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      smbAccntTbl[i].privilege = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid privilege %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"directory") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(smbAccntTbl[i].dir, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaAccntEntryAttr(): invalid privilege %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndSambaAccntEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}


PSX_STATUS xmlGetSambaStatusAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSAMBA_MGMT_CFG smbMgmtCfg = (PSAMBA_MGMT_CFG)psiValue;
#ifdef XML_DEBUG
  printf("xmlGetSambaStatusAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"enableSamba") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateState(attrValue);
    if (status == DB_OBJ_VALID_OK) {
      if ( strcmp(attrValue, "enable") == 0 )
         smbMgmtCfg->enableSamba= 1;
      else
         smbMgmtCfg->enableSamba= 0;
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaStatusAttr(): invalid Samba state %s\n",attrValue);
    }
  } else if (strcmp(attrName,"svrname") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_HOST_LEN);
    if (status == DB_OBJ_VALID_OK)
    {  
       strcpy(smbMgmtCfg->svrname, attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaStatusAttr(): invalid svrname %s\n",attrValue);
    }
  } else if (strcmp(attrName,"workgrp") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_HOST_LEN);
    if (status == DB_OBJ_VALID_OK)
    {  
       strcpy(smbMgmtCfg->workgrp, attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaStatusAttr(): invalid workgrp %s\n",attrValue);
    }
  } else if (strcmp(attrName,"netbname") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_HOST_LEN);
    if (status == DB_OBJ_VALID_OK)
    {  
       strcpy(smbMgmtCfg->netbname, attrValue);
    }
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSambaStatusAttr(): invalid netbname %s\n",attrValue);
    }
  }
  

  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSambaStatusObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartSambaStatusObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(SAMBA_MGMT_CFG));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndSambaStatusObj(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartSambaStatusObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  PSAMBA_MGMT_CFG pSmbMgmtCfg = (PSAMBA_MGMT_CFG)psiValue;
  
  if (!xmlState.verify)
     BcmDb_setSmbCfgInfo(pSmbMgmtCfg);

#ifdef XML_DEBUG1
  printf("\n============End Samba Management Cfg=========\n");
  printf("xmlEndSambaStatusObj(): enableSamba = %d\n", 
          pSmbMgmtCfg->enableSamba);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#endif /* SUPPORT_SAMBA */


/********************** start base64 decode and encode functions **********************/

void decodeBlock(unsigned char *dest, char *src)
{
  unsigned int x = 0;
  int i;
  for(i = 0; i < 4; i++) {
    if(src[i] >= 'A' && src[i] <= 'Z')
      x = (x << 6) + (unsigned int)(src[i] - 'A' + 0);
    else if(src[i] >= 'a' && src[i] <= 'z')
      x = (x << 6) + (unsigned int)(src[i] - 'a' + 26);
    else if(src[i] >= '0' && src[i] <= '9') 
      x = (x << 6) + (unsigned int)(src[i] - '0' + 52);
    else if(src[i] == '+')
      x = (x << 6) + 62;
    else if(src[i] == '/')
      x = (x << 6) + 63;
    else if(src[i] == '=')
      x = (x << 6);
  }

  dest[2] = (unsigned char)(x & 255); x >>= 8;
  dest[1] = (unsigned char)(x & 255); x >>= 8;
  dest[0] = (unsigned char)(x & 255); x >>= 8;
}

/* base64Decode
 * Given a base64 string at src, decode it into the memory pointed
 * to by dest. If rawLength points to a valid address (ie not NULL),
 * store the length of the decoded data to it.
 */
void base64Decode(unsigned char *dest, char *src, int *rawLength)
{
  int length = 0;
  int equalsTerm = 0;
  int i;
  int numBlocks;
  unsigned char lastBlock[3];

  if(0 == strcmp(src,""))/*add by w00135351 20081201*/
  {
  	 dest[0] = '\0';
	 return;
  }
	
  while((src[length] != '=') && src[length])
    length++;
  while(src[length+equalsTerm] == '=')
    equalsTerm++;
  
  numBlocks = (length + equalsTerm) / 4;
  if(rawLength)
    *rawLength = (numBlocks * 3) - equalsTerm;
  
  for(i = 0; i < numBlocks - 1; i++) {
    decodeBlock(dest, src);
    dest += 3; src += 4;
  }

  decodeBlock(lastBlock, src);
  for(i = 0; i < 3 - equalsTerm; i++)
    dest[i] = lastBlock[i];
	
}

/* ---- Base64 Encoding --- */
static char table64[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  
/*
 * base64Encode()
 *
 * Returns the length of the newly created base64 string. The third argument
 * is a pointer to an allocated area holding the base64 data. If something
 * went wrong, -1 is returned.
 *
 */
int base64Encode(const void *inp, int insize, char **outptr)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;
  char *output;
  char *base64data;

  char *indata = (char *)inp;

  if(0 == insize)
    insize = strlen(indata);

  base64data = output = (char*)malloc(insize*4/3+4);
  if(NULL == output)
    return -1;
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  memset(output, 0, insize*4/3+4);
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
  while(insize > 0) {
    for (i = inputparts = 0; i < 3; i++) { 
      if(insize > 0) {
        inputparts++;
        ibuf[i] = *indata;
        indata++;
        insize--;
      }
      else
        ibuf[i] = 0;
    }
                       
    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;

    switch(inputparts) {
    case 1: /* only one byte read */
      sprintf(output, "%c%c==", 
              table64[obuf[0]],
              table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      sprintf(output, "%c%c%c=", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]]);
      break;
    default:
      sprintf(output, "%c%c%c%c", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]],
              table64[obuf[3]] );
      break;
    }
    output += 4;
  }
  *output=0;
  *outptr = base64data; /* make it return the actual data memory */

  return strlen(base64data); /* return the length of the new data */
}

#ifdef SUPPORT_VLAN_CONFIG
/*vlan app entry funcs*/
PSX_STATUS xmlSetVlanObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(vlanObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(vlanObjs[id].setAttrFnc) != NULL)
     (*(vlanObjs[id].setAttrFnc))(file, vlanObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlGetVlanObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetVlanObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].getAttrFnc) != NULL) {
	sts = (*(vlanObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetVlanObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
}


PSX_STATUS xmlStartVlanObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartVlanObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].startObjFnc) != NULL) {
        sts = (*(vlanObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
}

PSX_STATUS xmlEndVlanObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndVlanObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].endObjFnc) != NULL) {
	sts = (*(vlanObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
}

/*vlan config  funcs*/
PSX_STATUS xmlStartVlanAttr(char *appName, char *objName)
{
  VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;

#ifdef XML_DEBUG
  printf("xmlStartVlanAttr(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  psiValue = (void*)malloc(sizeof(VLAN_8021Q_CFG_ENTRY));
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(VLAN_8021Q_CFG_ENTRY));
  pvlcfg = (VLAN_8021Q_CFG_ENTRY*)psiValue;
  //可以添加缺省配置
  return PSX_STS_OK;
}

void xmlSetVlanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif
     
   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_VLAN) == 0) {
         pvlcfg = (VLAN_8021Q_CFG_ENTRY*)value;
         fprintf(file,"<%s mngrVID=\"%d\" garpAction=\"%d\" garpFDIF=\"%s\" bpduAction=\"%d\" bpduFDIF=\"%s\" enabled=\"%d\"/>\n",
              name, pvlcfg->mngrid, pvlcfg->garpAct, pvlcfg->garpFDIF, pvlcfg->bpduAct, pvlcfg->bpduFDIF, pvlcfg->enabled);
   }
}

PSX_STATUS xmlGetVlanAttr(char *attrName, char* attrValue)
{
    VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;
#ifdef XML_DEBUG
  printf("xmlGetVlanAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif 
  pvlcfg = (VLAN_8021Q_CFG_ENTRY *)psiValue; 

  if(strncmp(attrName,"mngrVID",strlen("mngrVID")) == 0)
  {
      pvlcfg->mngrid = (unsigned short)atoi(attrValue);
  }
  else if(strncmp(attrName,"garpAction",strlen("garpAction")) == 0)
  {
      pvlcfg->garpAct = (unsigned char)atoi(attrValue);
  }
  else if(strncmp(attrName,"garpFDIF",strlen("garpFDIF")) == 0)
  {
      strncpy(pvlcfg->garpFDIF, attrValue, IFC_TINY_LEN);
      pvlcfg->garpFDIF[IFC_TINY_LEN - 1] = '\0';
  }
  else if(strncmp(attrName,"bpduAction",strlen("bpduAction")) == 0)
  {
      pvlcfg->bpduAct = (unsigned char)atoi(attrValue);
  }
  else if(strncmp(attrName,"bpduFDIF",strlen("bpduFDIF")) == 0)
  {
      strncpy(pvlcfg->bpduFDIF, attrValue, IFC_TINY_LEN);
      pvlcfg->bpduFDIF[IFC_TINY_LEN - 1] = '\0';
  }
  else if(strncmp(attrName,"enabled",strlen("enabled")) == 0)
  {
      pvlcfg->enabled = (unsigned char)atoi(attrValue);
  }
  return(PSX_STS_OK);
}


PSX_STATUS xmlEndVlanObj(char *appName, char *objName)
{
  PSI_HANDLE vlanPsi;
  PSI_STATUS sts = PSI_STS_OK;
  
#ifdef XML_DEBUG
  printf("xmlEndVlanObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID);    
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_ID, psiValue, sizeof(VLAN_8021Q_CFG_ENTRY));      
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan cfg data from xml to psi, err %d\n", sts);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVlanObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

/*vlan item entry funcs*/
void xmlSetVltbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_ITEM_ENTRY *pvlan, *pent;
   int entry = 0;
   int num = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif

    
   if ((name == NULL) || (value == NULL))
       return;

   if (strcmp(name,TBLNAME_VLTB) == 0) {
       num = *(int*)value;
     
       if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pvlan = (VLAN_8021Q_ITEM_ENTRY*)value;
     /* determine the size of route table */
     num = xmlGetTableSize(file);
        
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       pent = &pvlan[entry];
       fprintf(file,"<%s no=\"%d\" grpIf=\"%s\" vid=\"%d\" macln=\"%d\" ifList=\"%s\"/>\n",
                      name,pent->no, pent->grpIf, pent->vlanid, pent->macln, pent->ifList);
     }
     /* end of table */
     if ( num != 0 )
         fprintf(file,"</%s>\n",vlanObjs[id-1].objName);
         
   } /* pmap table */
}

PSX_STATUS xmlGetVltbAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVLPortAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    */
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}


PSX_STATUS xmlEndVltbObj(char *appName, char *objName)
{
  PSI_STATUS sts = PSI_STS_OK;
    int error = 0;
  PSI_HANDLE vlanPsi;
  
#ifdef XML_DEBUG
  printf("xmlEndVltbObj(calling cfm to get appName %s, objName %s)errorFlag:%d\n",
	 appName,objName, xmlState.errorFlag);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVltbObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_NUM_ID, &(xmlState.tableSize), 
                      sizeof(int));
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan table size %d from xml to psi, err %d\n", xmlState.tableSize, sts);
        
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_TBL_ID, psiValue, 
                      sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize); 
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan table data from xml to psi, err %d\n", sts);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVltbObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetVlanEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  VLAN_8021Q_ITEM_ENTRY *pvlan = NULL;
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetVlanEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVlanEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetVlanEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  pvlan = (VLAN_8021Q_ITEM_ENTRY*)psiValue + xmlState.tableIndex ;
  
  if (strcmp(attrName,"no") == 0) {
      pvlan->no = atoi(attrValue);
  }  
  else  if (strcmp(attrName,"grpIf") == 0)
  {
      strncpy(pvlan->grpIf, attrValue, IFC_TINY_LEN);
      pvlan->grpIf[IFC_TINY_LEN - 1] = '\0';
  }
  else  if (strcmp(attrName,"vid") == 0)
  {
      pvlan->vlanid = (unsigned short)atoi(attrValue);
  }
  else  if (strcmp(attrName,"macln") == 0)
  {
      pvlan->macln = (unsigned char)atoi(attrValue);
  }
  else  if (strcmp(attrName,"ifList") == 0)
  {
      if(strlen(attrValue) < (VLAN_INTF_LIST_SIZE - 1))
          strcpy(pvlan->ifList, attrValue);
      else
      {
          strncpy(pvlan->ifList, attrValue, VLAN_INTF_LIST_SIZE);
          pvlan->ifList[VLAN_INTF_LIST_SIZE - 1] = '\0';
      }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVlanEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

/*vlan port entry funcs*/
void xmlSetVLPortAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_PORT_ENTRY *pmap, *pent;
   int entry = 0;
   int num = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
	 __FUNCTION__,name,value);
#endif

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_VLPORT) == 0) {
     num = *(int*)value;
     
     #ifdef XML_DEBUG
      printf("%s(vlan port num %d)\n",
     __FUNCTION__,num);
    #endif
    
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pmap = (VLAN_8021Q_PORT_ENTRY*)value;
     /* determine the size of route table */
     //num = length/sizeof(PORT_MAP_CFG_ENTRY);
     num = xmlGetTableSize(file);
     
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       pent = &pmap[entry];
       //<vlPortAttr ifName='eth0.2' mode='0' pvid='12' prio='-1'></vlPortAttr>
       fprintf(file,"<%s ifName=\"%s\" mode=\"%d\" pvid=\"%d\" prio=\"%d\"/>\n",
	       name,pent->name,pent->mode,pent->pvid, (int)pent->prio);
     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",vlanObjs[id-1].objName); 
   } /* pmap table */
}

PSX_STATUS xmlGetVLPortAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVLPortAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    */
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVLPortObj(char *appName, char *objName)
{
  int error = 0;
  PSI_HANDLE vlanPsi;
  
#ifdef XML_DEBUG
  printf("xmlEndVLPortObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVLPortObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    BcmPsi_objStore(vlanPsi, VLCM_PORT_NUM_ID, &xmlState.tableSize, 
                      sizeof(int)); 
    BcmPsi_objStore(vlanPsi, VLCM_PORT_CFG_TBL_ID, psiValue, 
                      sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize); 
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVLPortObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetVLPortEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  VLAN_8021Q_PORT_ENTRY *pport = (VLAN_8021Q_PORT_ENTRY*)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetVLPortEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"ifName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pport[i].name, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"mode") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
        pport[i].mode = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid mode  %s\n",attrValue);
    }
  }  
  else if (strcmp(attrName,"pvid") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
        pport[i].pvid = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid pvid  %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"prio") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
      pport[i].prio = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid prio  %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVLPortEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
#else
#ifndef SUPPORT_ADSL
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
/*vlan app entry funcs*/
PSX_STATUS xmlSetVlanObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(vlanObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(vlanObjs[id].setAttrFnc) != NULL)
     (*(vlanObjs[id].setAttrFnc))(file, vlanObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}

PSX_STATUS xmlGetVlanObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetVlanObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].getAttrFnc) != NULL) {
	sts = (*(vlanObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlGetVlanObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
}


PSX_STATUS xmlStartVlanObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartVlanObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].startObjFnc) != NULL) {
        sts = (*(vlanObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlStartPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
}

PSX_STATUS xmlEndVlanObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndVlanObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; vlanObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(vlanObjs[i].objName, objName) == 0) {
      if (*(vlanObjs[i].endObjFnc) != NULL) {
	sts = (*(vlanObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    syslog(LOG_ERR,"xmlEndPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
}

/*vlan config  funcs*/
PSX_STATUS xmlStartVlanAttr(char *appName, char *objName)
{
  VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;

#ifdef XML_DEBUG
  printf("xmlStartVlanAttr(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif
  psiValue = (void*)malloc(sizeof(VLAN_8021Q_CFG_ENTRY));
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(VLAN_8021Q_CFG_ENTRY));
  pvlcfg = (VLAN_8021Q_CFG_ENTRY*)psiValue;
  //可以添加缺省配置
  return PSX_STS_OK;
}

void xmlSetVlanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif
     
   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_VLAN) == 0) {
         pvlcfg = (VLAN_8021Q_CFG_ENTRY*)value;
         fprintf(file,"<%s mngrVID=\"%d\" garpAction=\"%d\" garpFDIF=\"%s\" bpduAction=\"%d\" bpduFDIF=\"%s\" enabled=\"%d\"/>\n",
              name, pvlcfg->mngrid, pvlcfg->garpAct, pvlcfg->garpFDIF, pvlcfg->bpduAct, pvlcfg->bpduFDIF, pvlcfg->enabled);
   }
}

PSX_STATUS xmlGetVlanAttr(char *attrName, char* attrValue)
{
    VLAN_8021Q_CFG_ENTRY *pvlcfg = NULL;
#ifdef XML_DEBUG
  printf("xmlGetVlanAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif 
  pvlcfg = (VLAN_8021Q_CFG_ENTRY *)psiValue; 

  if(strncmp(attrName,"mngrVID",strlen("mngrVID")) == 0)
  {
      pvlcfg->mngrid = (unsigned short)atoi(attrValue);
  }
  else if(strncmp(attrName,"garpAction",strlen("garpAction")) == 0)
  {
      pvlcfg->garpAct = (unsigned char)atoi(attrValue);
  }
  else if(strncmp(attrName,"garpFDIF",strlen("garpFDIF")) == 0)
  {
      strncpy(pvlcfg->garpFDIF, attrValue, IFC_TINY_LEN);
      pvlcfg->garpFDIF[IFC_TINY_LEN - 1] = '\0';
  }
  else if(strncmp(attrName,"bpduAction",strlen("bpduAction")) == 0)
  {
      pvlcfg->bpduAct = (unsigned char)atoi(attrValue);
  }
  else if(strncmp(attrName,"bpduFDIF",strlen("bpduFDIF")) == 0)
  {
      strncpy(pvlcfg->bpduFDIF, attrValue, IFC_TINY_LEN);
      pvlcfg->bpduFDIF[IFC_TINY_LEN - 1] = '\0';
  }
  else if(strncmp(attrName,"enabled",strlen("enabled")) == 0)
  {
      pvlcfg->enabled = (unsigned char)atoi(attrValue);
  }
  return(PSX_STS_OK);
}


PSX_STATUS xmlEndVlanObj(char *appName, char *objName)
{
  PSI_HANDLE vlanPsi;
  PSI_STATUS sts = PSI_STS_OK;
  
#ifdef XML_DEBUG
  printf("xmlEndVlanObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  if (!xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID);    
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_ID, psiValue, sizeof(VLAN_8021Q_CFG_ENTRY));      
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan cfg data from xml to psi, err %d\n", sts);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVlanObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

/*vlan item entry funcs*/
void xmlSetVltbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_ITEM_ENTRY *pvlan, *pent;
   int entry = 0;
   int num = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif

    
   if ((name == NULL) || (value == NULL))
       return;

   if (strcmp(name,TBLNAME_VLTB) == 0) {
       num = *(int*)value;
     
       if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pvlan = (VLAN_8021Q_ITEM_ENTRY*)value;
     /* determine the size of route table */
     num = xmlGetTableSize(file);
        
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       pent = &pvlan[entry];
       fprintf(file,"<%s no=\"%d\" grpIf=\"%s\" vid=\"%d\" macln=\"%d\" ifList=\"%s\"/>\n",
                      name,pent->no, pent->grpIf, pent->vlanid, pent->macln, pent->ifList);
     }
     /* end of table */
     if ( num != 0 )
         fprintf(file,"</%s>\n",vlanObjs[id-1].objName);
         
   } /* pmap table */
}

PSX_STATUS xmlGetVltbAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVLPortAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    */
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}


PSX_STATUS xmlEndVltbObj(char *appName, char *objName)
{
  PSI_STATUS sts = PSI_STS_OK;
    int error = 0;
  PSI_HANDLE vlanPsi;
  
#ifdef XML_DEBUG
  printf("xmlEndVltbObj(calling cfm to get appName %s, objName %s)errorFlag:%d\n",
	 appName,objName, xmlState.errorFlag);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVltbObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_NUM_ID, &(xmlState.tableSize), 
                      sizeof(int));
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan table size %d from xml to psi, err %d\n", xmlState.tableSize, sts);
        
    sts = BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_TBL_ID, psiValue, 
                      sizeof(VLAN_8021Q_ITEM_ENTRY) * xmlState.tableSize); 
    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: cant save vlan table data from xml to psi, err %d\n", sts);
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVltbObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetVlanEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  VLAN_8021Q_ITEM_ENTRY *pvlan = NULL;
  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetVlanEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVlanEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetVlanEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  pvlan = (VLAN_8021Q_ITEM_ENTRY*)psiValue + xmlState.tableIndex ;
  
  if (strcmp(attrName,"no") == 0) {
      pvlan->no = atoi(attrValue);
  }  
  else  if (strcmp(attrName,"grpIf") == 0)
  {
      strncpy(pvlan->grpIf, attrValue, IFC_TINY_LEN);
      pvlan->grpIf[IFC_TINY_LEN - 1] = '\0';
  }
  else  if (strcmp(attrName,"vid") == 0)
  {
      pvlan->vlanid = (unsigned short)atoi(attrValue);
  }
  else  if (strcmp(attrName,"macln") == 0)
  {
      pvlan->macln = (unsigned char)atoi(attrValue);
  }
  else  if (strcmp(attrName,"ifList") == 0)
  {
      if(strlen(attrValue) < (VLAN_INTF_LIST_SIZE - 1))
          strcpy(pvlan->ifList, attrValue);
      else
      {
          strncpy(pvlan->ifList, attrValue, VLAN_INTF_LIST_SIZE);
          pvlan->ifList[VLAN_INTF_LIST_SIZE - 1] = '\0';
      }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVlanEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

/*vlan port entry funcs*/
void xmlSetVLPortAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
   VLAN_8021Q_PORT_ENTRY *pmap, *pent;
   int entry = 0;
   int num = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
	 __FUNCTION__,name,value);
#endif

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_VLPORT) == 0) {
     num = *(int*)value;
     
     #ifdef XML_DEBUG
      printf("%s(vlan port num %d)\n",
     __FUNCTION__,num);
    #endif
    
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pmap = (VLAN_8021Q_PORT_ENTRY*)value;
     /* determine the size of route table */
     //num = length/sizeof(PORT_MAP_CFG_ENTRY);
     num = xmlGetTableSize(file);
     
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       pent = &pmap[entry];
       //<vlPortAttr ifName='eth0.2' mode='0' pvid='12' prio='-1'></vlPortAttr>
       fprintf(file,"<%s ifName=\"%s\" mode=\"%d\" pvid=\"%d\" prio=\"%d\"/>\n",
	       name,pent->name,pent->mode,pent->pvid, (int)pent->prio);
     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",vlanObjs[id-1].objName); 
   } /* pmap table */
}

PSX_STATUS xmlGetVLPortAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVLPortAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    /* start of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日"
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    */

    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    /* end of maintain PSI移植：记录超长处理 by xujunxia 43813 2006年5月9日 */
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVLPortObj(char *appName, char *objName)
{
  int error = 0;
  PSI_HANDLE vlanPsi;
  
#ifdef XML_DEBUG
  printf("xmlEndVLPortObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndVLPortObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    BcmPsi_objStore(vlanPsi, VLCM_PORT_NUM_ID, &xmlState.tableSize, 
                      sizeof(int)); 
    BcmPsi_objStore(vlanPsi, VLCM_PORT_CFG_TBL_ID, psiValue, 
                      sizeof(VLAN_8021Q_PORT_ENTRY) * xmlState.tableSize); 
  }

#ifdef XML_DEBUG1
  printf("\n============End Static PMap=========\n");
  printf("xmlEndVLPortObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetVLPortEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  VLAN_8021Q_PORT_ENTRY *pport = (VLAN_8021Q_PORT_ENTRY*)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetVLPortEntryAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
  if (strcmp(attrName,"ifName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_TINY_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pport[i].name, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"mode") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
        pport[i].mode = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid mode  %s\n",attrValue);
    }
  }  
  else if (strcmp(attrName,"pvid") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
        pport[i].pvid = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid pvid  %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"prio") == 0) {
    if (xmlState.verify)
        ;/*do nothing*/
    if (status == DB_OBJ_VALID_OK)
      pport[i].prio = (unsigned short)atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetVLPortEntryAttr(): invalid prio  %s\n",attrValue);
    }
  }
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndVLPortEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}
/*end vlan bridge, s60000658, 20060627*/
//#endif
#endif

#endif
#ifdef VOXXXLOAD
void xmlSetSipPhoneCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length)
{
    SIP_WEB_PHONE_CFG_S *pPhone = NULL;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif
     
   if ((name == NULL) || (value == NULL))
      return;
   /*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
   if (strcmp(name,OBJNAME_SIP_PHONECFG) == 0) {
         pPhone = (SIP_WEB_PHONE_CFG_S*)value;
         //modified by z67625 for automatic call
         fprintf(file,"<%s speak=\"%d\" listen=\"%d\" interval=\"%d\" region=\"%d\" minHookFlash=\"%d\" maxHookFlash=\"%d\" digitmap=\"%s\" \
		 	offhooktime=\"%d\" onhooktime=\"%d\" fstdialtime=\"%d\" autocallenbl=\"%d\" autocallnum=\"%s\" autocallinterval=\"%d\" reinjection=\"%d\" innercall=\"%d\" modemservice=\"%d\" VoicePromptVisible=\"%d\"/>\n",
              name, pPhone->lSpkVol, pPhone->lLstVol, pPhone->ulDialInterval, pPhone->ulRegion, pPhone->minHookFlash, pPhone->maxHookFlash, pPhone->szDigitMap,
              pPhone->ulOffHookTime, pPhone->ulOnHookTime, pPhone->ulFstDialTime, pPhone->lAutoEnbl, pPhone->acAutoDialNum,pPhone->ulAutoInterval,pPhone->ulReInjection, pPhone->ulInnerCall, pPhone->ulModemService, pPhone->bVoicePromptVisible);
   }
   /*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
}

PSX_STATUS xmlGetSipPhoneCfgAttr(char *attrName, char* attrValue)
{
    SIP_WEB_PHONE_CFG_S *pPhone = NULL;
    
    #ifdef XML_DEBUG
      printf("xmlGetVlanAttr(attrName %s, attrValue %s)\n",
    	 attrName,attrValue);
    #endif 
      pPhone = (SIP_WEB_PHONE_CFG_S *)psiValue; 

      if(strncmp(attrName,"speak",strlen("speak")) == 0)
      {
          pPhone->lSpkVol = atoi(attrValue);
      }
      else if(strncmp(attrName,"listen",strlen("listen")) == 0)
      {
          pPhone->lLstVol = atoi(attrValue);
      }
      else if(strncmp(attrName,"interval",strlen("interval")) == 0)
      {
          pPhone->ulDialInterval = atoi(attrValue);
      }
	  /* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
      else if(strncmp(attrName,"vad",strlen("vad")) == 0)
      {
          pPhone->ulVad = atoi(attrValue);
      }
#endif
      else if(strncmp(attrName,"region",strlen("region")) == 0)
      {
          pPhone->ulRegion = atoi(attrValue);
      }
      else if(strncmp(attrName,"maxHookFlash",strlen("maxHookFlash")) == 0)
      {
          pPhone->maxHookFlash= atoi(attrValue);
      }
      else if(strncmp(attrName,"minHookFlash",strlen("minHookFlash")) == 0)
      {
          pPhone->minHookFlash = atoi(attrValue);
      }
			/* END:   Modified by p00102297, 2008/2/20 */

      else if(strncmp(attrName,"digitmap",strlen("digitmap")) == 0)
      {
          strncpy((char*)pPhone->szDigitMap, attrValue, SIP_MAX_DIGIT_MAP);
      }
	  /* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
	  else if(strncmp(attrName,"ec",strlen("ec")) == 0)
      {
          pPhone->ulEC = atoi(attrValue);
      }
	  else if(strncmp(attrName,"cng",strlen("cng")) == 0)
      {
          pPhone->ulCNG = atoi(attrValue);
      }
#endif
	  else if(strncmp(attrName,"offhooktime",strlen("offhooktime")) == 0)
      {
          pPhone->ulOffHookTime = atoi(attrValue);
      }
	  else if(strncmp(attrName,"onhooktime",strlen("onhooktime")) == 0)
      {
          pPhone->ulOnHookTime = atoi(attrValue);
      }
	  else if(strncmp(attrName,"fstdialtime",strlen("fstdialtime")) == 0)
      {
          pPhone->ulFstDialTime = atoi(attrValue);
      }
	  /* END:   Modified by p00102297, 2008/2/20 */
      //add by z67625 for automatic call
      else if(strncmp(attrName,"autocallenbl",strlen("autocallenbl")) == 0)
      {
          pPhone->lAutoEnbl= atoi(attrValue);
      }
      else if(strncmp(attrName,"autocallinterval",strlen("autocallinterval")) == 0)
      {
          pPhone->ulAutoInterval = atoi(attrValue);
      }
      else if(strncmp(attrName,"autocallnum",strlen("autocallnum")) == 0)
      {
          strcpy((char *)pPhone->acAutoDialNum, attrValue);
      }
      /*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	  else if(strncmp(attrName,"reinjection",strlen("reinjection")) == 0)
      {
          pPhone->ulReInjection = atoi(attrValue);
      }
	  /*END: Added by chenyong 65116 2008-09-15 web配置*/
	  /*start of additon by chenyong 2008-10-28 for Inner call*/
	  else if(strncmp(attrName,"innercall",strlen("innercall")) == 0)
      {
          pPhone->ulInnerCall = atoi(attrValue);
      }
	  /*end of additon by chenyong 2008-10-28 for Inner call*/
	  else if(strncmp(attrName,"modemservice",strlen("modemservice")) == 0)
      {
          pPhone->ulModemService = atoi(attrValue);
      }
	  /*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
      else if(strncmp(attrName,"VoicePromptVisible",strlen("VoicePromptVisible")) == 0)
      {
      	  pPhone->bVoicePromptVisible = atoi(attrValue);
      }
      /*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/

    return (PSX_STS_OK);
} 
PSX_STATUS xmlStartSipPhoneCfgObjNode(char *appName, char *objName)
{
#ifdef XML_DEBUG
  printf("xmlStartVlanAttr(calling cfm to get appName %s, objName %s\n",
     appName,objName);
#endif
  psiValue = (void*)malloc(sizeof(SIP_WEB_PHONE_CFG_S));
  if (psiValue == NULL) {
    xmlMemProblem();
    return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(SIP_WEB_PHONE_CFG_S));
  return (PSX_STS_OK);
} 
PSX_STATUS xmlEndSipPhoneCfgObjNode(char *appName, char *objName)
{
    PSI_HANDLE voicePsi;
    PSI_STATUS sts = PSI_STS_OK;
  
    #ifdef XML_DEBUG
      printf("xmlEndSipPhoneCfgObjNode(calling cfm to get appName %s, objName %s\n",
    	 appName,objName);
    #endif

      if (!xmlState.verify) {
        voicePsi = BcmPsi_appOpen(VOICE_APPID);    
        sts = BcmPsi_objStore(voicePsi, VOICE_SIP_PHONECFG_ID, psiValue, sizeof(SIP_WEB_PHONE_CFG_S));      
      }

      free(psiValue);
      psiValue = NULL;
      xmlState.tableSize = 0;
      xmlState.tableIndex = 0;
    return (PSX_STS_OK);
}

// #define XML_DEBUG
void xmlSetSipSpeedDialAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length)
{
   SIP_WEB_DIAL_PLAN_S *pDialPlan = NULL;
   int entry = 0;
   int num = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif

    
   if ((name == NULL) || (value == NULL))
       return;

   if (strcmp(name,TBLNAME_SIP_SPEEDDIAL_TB) == 0) {
       num = *(int*)value;
     
       if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pDialPlan = (SIP_WEB_DIAL_PLAN_S*)value;
     /* determine the size of route table */
     num = xmlGetTableSize(file);
        
     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       fprintf(file,"<%s no=\"%d\" num=\"%s\" name=\"%s\" dest=\"%s\"/>\n",
                      name, entry, pDialPlan[entry].szDialNum, pDialPlan[entry].szName, pDialPlan[entry].szRealNum);
     }
     /* end of table */
     if ( num != 0 )
         fprintf(file,"</%s>\n",voiceObjs[id-1].objName);
         
   } /* pmap table */
}
PSX_STATUS xmlGetSipSpeedDialTbAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetVLPortAttr(attrName %s, attrValue %s)\n",
	 attrName,attrValue);
#endif
    if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
        if (xmlState.verify)
          status = BcmDb_validateRange(attrValue, 0, SIP_MAX_DIAL_PLAN);
        if (status == DB_OBJ_VALID_OK) {
          xmlState.tableIndex = 0;
          xmlState.tableSize = atoi(attrValue);
          psiValue = (void *) malloc(sizeof(SIP_WEB_DIAL_PLAN_S) * xmlState.tableSize);
          if (psiValue == NULL) {
              xmlMemProblem();
              return PSX_STS_ERR_FATAL;
          }
          memset(psiValue, 0, sizeof(SIP_WEB_DIAL_PLAN_S) * xmlState.tableSize);
        }  
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
        }
    }      
    return (PSX_STS_OK);
} 
PSX_STATUS xmlEndSipSpeedDialTbNode(char *appName, char *objName)
{
    PSI_STATUS sts = PSI_STS_OK;
    int error = 0;
    PSI_HANDLE voicePsi;
      
    #ifdef XML_DEBUG
      printf("xmlEndVltbObj(calling cfm to get appName %s, objName %s)errorFlag:%d\n",
    	 appName,objName, xmlState.errorFlag);
    #endif

      /* check to see if all mandatory object attribute are set */  
      if (xmlState.verify) {
        if (xmlState.tableSize != xmlState.tableIndex) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlEndVltbObj(): table size does not match with number of entries.\n");
          error = 1;
        }
      }
      if (!error && !xmlState.verify) {
        voicePsi = BcmPsi_appOpen(VOICE_APPID); 
        sts = BcmPsi_objStore(voicePsi, VOICE_SIP_SPEEDDIAL_NUM_ID, &(xmlState.tableSize), 
                          sizeof(int));
#ifdef DEBUG_SIP 
        if(sts != PSI_STS_OK)
            printf("SIP: cant save speeddial size %d from xml to psi, err %d\n", xmlState.tableSize, sts);
#endif
        sts = BcmPsi_objStore(voicePsi, VOICE_SIP_SPEEDDIAL_TBL_ID, psiValue, 
                          sizeof(SIP_WEB_DIAL_PLAN_S) * xmlState.tableSize); 

#ifdef DEBUG_SIP 
        if(sts != PSI_STS_OK)
            printf("SIP: cant save speeddial data from xml to psi, err %d\n", sts);
#endif
      }

    #ifdef XML_DEBUG1
      printf("\n============End Static PMap=========\n");
      printf("xmlEndSipSpeedDialTbNode(): table size = %d\n", xmlState.tableSize);
      printf("===============================================\n");
    #endif
      free(psiValue);
      psiValue = NULL;
      xmlState.tableSize = 0;
      xmlState.tableIndex = 0;

    return (PSX_STS_OK);
}
PSX_STATUS xmlGetSipSpeedDialEntryAttr(char *attrName, char* attrValue)
{
      int i = xmlState.tableIndex;
      DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
      SIP_WEB_DIAL_PLAN_S *pDialPlan = (SIP_WEB_DIAL_PLAN_S*)psiValue;

      if ( psiValue == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetSipSpeedDialTbAttr(): psiValue is NULL\n");
        return PSX_STS_ERR_FATAL;
      }
        
      if (xmlState.verify) {
        if ( i >= xmlState.tableSize ) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipSpeedDialTbAttr(): invalid entry index %d, tableSize %d\n",
                 i, xmlState.tableSize);
          return PSX_STS_ERR_FATAL;
        }
      }

    #ifdef XML_DEBUG
      printf("xmlGetSipSpeedDialTbAttr(attrName %s, attrValue %s)\n",
    	 attrName,attrValue);
    #endif
    
      if (strcmp(attrName,"num") == 0) {
        if (xmlState.verify)
/*BEGIN guwenbin 20080705 AU8D00774  将SIP_MAX_NUM_LEN/4改为SIP_MAX_DAILNUM_LEN*/
		//status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN/4);
		status = BcmDb_validateLength(attrValue, SIP_MAX_DAILNUM_LEN);
/*END guwenbin 20080705 AU8D00774  QuickDialNumber 需配置40位长度*/
        if (status == DB_OBJ_VALID_OK)
          strcpy((char*)pDialPlan[i].szDialNum, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipSpeedDialTbAttr(): invalid dial number %s\n",attrValue);
        }
      }
      else if (strcmp(attrName,"name") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
        if (status == DB_OBJ_VALID_OK)
            strcpy((char*)pDialPlan[i].szName, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipSpeedDialTbAttr(): invalid dial name  %s\n",attrValue);
        }
      }  
      else if (strcmp(attrName,"dest") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
        if (status == DB_OBJ_VALID_OK)
            strcpy((char*)pDialPlan[i].szRealNum, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipSpeedDialTbAttr(): invalid dial real number  %s\n",attrValue);
        }
      }
    return (PSX_STS_OK);
} 
PSX_STATUS xmlEndSipSpeedDialEntryNode(char *appName, char *objName)
{
    xmlState.tableIndex++;
    return (PSX_STS_OK);
}
void xmlSetSipCallFWDAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length)
{   
   SIP_WEB_FWD_NUM_S *pFWD = NULL;
   int entry = 0;
   int num = 0;
   int advance = 0;

#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
           __FUNCTION__,name,value);
#endif

    
   if ((name == NULL) || (value == NULL))
       return;

   if (strcmp(name,TBLNAME_SIP_CALLFWD_TB) == 0) {
       num = *(int*)value;
     
       if ( num != 0 )
           fprintf(file,"<%s tableSize=\"%d\">\n",name,num);
   }
   else {
     pFWD = (SIP_WEB_FWD_NUM_S*)value;
     /* determine the size of route table */
     num = xmlGetTableSize(file);

     //table1
     for (entry = 0; entry < num/2; entry++, pFWD++) {
       /* each entry is one object, even though PSI is not that way */
       if(entry ==  (num/2) - 1)
        {
               fprintf(file,"<%s no=\"%d\" nawt=\"%d\"/>\n",
                             name, entry, *((VOS_UINT32*)pFWD));
        }
        else
        {
               fprintf(file,"<%s no=\"%d\" active=\"%d\" incoming=\"%s\" forward=\"%s\" condition=\"%d\"/>\n",
                             name, entry, pFWD->bIsActive, pFWD->szIncoming, pFWD->szForward, pFWD->enFwdType);
        }
     }
     
     //table2
     for (; entry < num; entry++, pFWD++) {
       /* each entry is one object, even though PSI is not that way */
       if(entry ==  (num - 1))
        {
               fprintf(file,"<%s no=\"%d\" nawt=\"%d\"/>\n",
                             name, entry, *((VOS_UINT32*)pFWD));
        }
        else
        {
               fprintf(file,"<%s no=\"%d\"  active=\"%d\" incoming=\"%s\" forward=\"%s\" condition=\"%d\"/>\n",
                             name, entry, pFWD->bIsActive, pFWD->szIncoming, pFWD->szForward, pFWD->enFwdType);
        }
     }
     /* end of table */
     if ( num != 0 )
         fprintf(file,"</%s>\n",voiceObjs[id-1].objName);
         
   } /* pmap table */
} 
PSX_STATUS xmlGetSipCallFWDTbAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("%s(attrName %s, attrValue %s)\n",
	 __FUNCTION__,attrName,attrValue);
#endif
    if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
        if (xmlState.verify)
          status = BcmDb_validateRange(attrValue, 0, 2 * (SIP_WEB_FWD_ADVANCED_NUM + SIP_WEB_FWD_BASIC_NUM + 1));
        if (status == DB_OBJ_VALID_OK) {
          xmlState.tableIndex = 0;
          xmlState.tableSize = atoi(attrValue);
          psiValue = (void *) malloc(sizeof(SIP_WEB_FWD_NUM_S) * xmlState.tableSize);
          if (psiValue == NULL) {
              xmlMemProblem();
              return PSX_STS_ERR_FATAL;
          }
          memset(psiValue, 0, sizeof(SIP_WEB_FWD_NUM_S) * xmlState.tableSize);
        }  
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetVLPortAttr(): invalid number %s\n",attrValue);
        }
    }    
    
    return (PSX_STS_OK);
} 
PSX_STATUS xmlEndSipCallFWDTbNode(char *appName, char *objName)
{
    PSI_STATUS sts = PSI_STS_OK;
    int error = 0;
    PSI_HANDLE voicePsi;
      
    #ifdef XML_DEBUG
      printf("xmlEndSipCallFWDTbNode(calling cfm to get appName %s, objName %s)errorFlag:%d\n",
    	 appName,objName, xmlState.errorFlag);
    #endif

      /* check to see if all mandatory object attribute are set */  
      if (xmlState.verify) {
        if (xmlState.tableSize != xmlState.tableIndex) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlEndSipCallFWDTbNode(): table size does not match with number of entries.\n");
          error = 1;
        }
      }
      if (!error && !xmlState.verify) {
        voicePsi = BcmPsi_appOpen(VOICE_APPID); 
        sts = BcmPsi_objStore(voicePsi, VOICE_SIP_CALLFWD_NUM_ID, &(xmlState.tableSize), 
                          sizeof(int));
#ifdef DEBUG_SIP 
        if(sts != PSI_STS_OK)
            printf("SIP: cant save callforward size %d from xml to psi, err %d\n", xmlState.tableSize, sts);
#endif
        sts = BcmPsi_objStore(voicePsi, VOICE_SIP_CALLFWD_TBL_ID, psiValue, 
                          sizeof(SIP_WEB_FWD_NUM_S) * xmlState.tableSize); 

#ifdef DEBUG_SIP
        if(sts != PSI_STS_OK)
            printf("SIP: cant save callforward data from xml to psi, err %d\n", sts);
#endif
      }

    #ifdef XML_DEBUG1
      printf("\n============End Static PMap=========\n");
      printf("xmlEndSipCallFWDTbNode(): table size = %d\n", xmlState.tableSize);
      printf("===============================================\n");
    #endif
      free(psiValue);
      psiValue = NULL;
      xmlState.tableSize = 0;
      xmlState.tableIndex = 0;
    return (PSX_STS_OK);
}
PSX_STATUS xmlGetSipCallFWDEntryAttr(char *attrName, char* attrValue)
{
      int i = xmlState.tableIndex;
      DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
      SIP_WEB_FWD_NUM_S *pFWD = (SIP_WEB_FWD_NUM_S*)psiValue + i;

      if ( psiValue == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): psiValue is NULL\n");
        return PSX_STS_ERR_FATAL;
      }
        
      if (xmlState.verify) {
        if ( i >= xmlState.tableSize ) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid entry index %d, tableSize %d\n",
                 i, xmlState.tableSize);
          return PSX_STS_ERR_FATAL;
        }
      }

    #ifdef XML_DEBUG
      printf("xmlGetSipCallFWDEntryAttr(attrName %s, attrValue %s)\n",
    	 attrName,attrValue);
    #endif
    
      if (strcmp(attrName,"active") == 0) {
        if (xmlState.verify)
          status = BcmDb_validateBoolean(attrValue);
        if (status == DB_OBJ_VALID_OK)
          pFWD->bIsActive = atoi(attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid active flag %s\n",attrValue);
        }
      }
      else if (strcmp(attrName,"incoming") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
        if (status == DB_OBJ_VALID_OK)
            strcpy((char*)pFWD->szIncoming, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid incoming call number  %s\n",attrValue);
        }
      }  
      else if (strcmp(attrName,"forward") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateLength(attrValue, SIP_MAX_NUM_LEN);
        if (status == DB_OBJ_VALID_OK)
            strcpy((char*)pFWD->szForward, attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid incoming call number  %s\n",attrValue);
        }
      }
      else if (strcmp(attrName,"condition") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateRange(attrValue, SIP_WEB_UNCON_FWD, SIP_WEB_BUUTT_FWD -1);
        if (status == DB_OBJ_VALID_OK)
            pFWD->enFwdType = (SIP_WEB_FWD_TYPE_E)atoi(attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid forward condition  %s\n",attrValue);
        }
      }
      else if (strcmp(attrName,"nawt") == 0) {
        if (xmlState.verify)
            status = BcmDb_validateRange(attrValue, SIP_MIN_NOREPLY_TIME, SIP_MAX_NOREPLY_TIME);
        if (status == DB_OBJ_VALID_OK)
            *((VOS_UINT32*)pFWD) = (VOS_UINT32)atoi(attrValue);
        else {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetSipCallFWDEntryAttr(): invalid no reply waiting time  %s, valid is <%d,%d>\n",
                          attrValue, SIP_MIN_NOREPLY_TIME, SIP_MAX_NOREPLY_TIME);
        }
      }

    return (PSX_STS_OK);
}
PSX_STATUS xmlEndSipCallFWDEntryNode(char *appName, char *objName)
{
    xmlState.tableIndex++;
    return (PSX_STS_OK);
}
#endif
/*start   of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
extern "C" PSX_STATUS xmlSetHspaNode(FILE * file, const char *appName, const UINT16 id,
			const PSI_VALUE value, const UINT16 length)
{
	HSPA_INFO *pstHspa = (HSPA_INFO *)value;
   	
	fprintf(file,"<Entry InstantMode=\"%d\" ProbeMode=\"%d\" IcmpSpeedCyle=\"%d\" SwitchConfirm=\"%d\" DaHspaDelayCS=\"%d\" DaHspaDelayPS=\"%d\"  HspaDaDelayCS=\"%d\" HspaDaDelayPS=\"%d\" HspaVoiceDomain=\"%d\" HspaVoiceServiceFlag=\"%d\" OperSelect=\"%s\" Operator=\"%s\"/>\n", 
		pstHspa->iInstantMode, pstHspa->iProbeMode,pstHspa->iIcmpSpeedCyle, pstHspa->iSwitchConfirm, 
		pstHspa->iDaHspaDelayCS, pstHspa->iDaHspaDelayPS, pstHspa->iHspaDaDelayCS, pstHspa->iHspaDaDelayPS, pstHspa->iHspaVoiceDomain, pstHspa->cHspaVoiceServiceFlag,
		pstHspa->cOperSelect, pstHspa->cOperatorList);
	return PSX_STS_OK;
}


extern "C" PSX_STATUS xmlGetHspaNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
    HSPA_INFO *pstHspa = (HSPA_INFO *)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
  printf("xmlGetHspaNode(attrName %s, attrValue %s)\n",
	 attrName, attrValue);
#endif

 
  if (strncmp(attrName,"InstantMode",strlen("InstantMode")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradesManaged %s\n",attrValue);
    }
    else
      pstHspa->iInstantMode = atoi(attrValue);
  }
  else if (strncmp(attrName,"IcmpSpeedCyle",strlen("IcmpSpeedCyle")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradeAvailable %s\n",attrValue);
    }
    else
      pstHspa->iIcmpSpeedCyle = atoi(attrValue);
  }
  else if (strncmp(attrName,"ProbeMode",strlen("ProbeMode")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue,PROBE_MODE_PING , PROBE_MODE_BOTH);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid upgradeAvailable %s\n",attrValue);
    }
    else
      pstHspa->iProbeMode = atoi(attrValue);
  }
  else if (strncmp(attrName,"SwitchConfirm",strlen("SwitchConfirm")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateBoolean(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informEnbl %s\n",attrValue);
    }
    else
      pstHspa->iSwitchConfirm = atoi(attrValue);
  }
  else if (strncmp(attrName,"DaHspaDelayCS",strlen("DaHspaDelayCS")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informTime %s\n",attrValue);
    }
    else
      pstHspa->iDaHspaDelayCS = strtoul(attrValue, (char **)NULL, 10);
      printf("value  is  %d\n", pstHspa->iDaHspaDelayCS );
  }
  else if (strncmp(attrName,"DaHspaDelayPS",strlen("DaHspaDelayPS")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid informInterval %s\n",attrValue);
    }
    else
      pstHspa->iDaHspaDelayPS = strtoul(attrValue, (char **)NULL, 10);
  }
  else if (strncmp(attrName,"HspaDaDelayCS",strlen("HspaDaDelayCS")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsURL %s\n",attrValue);
    }
    else
     pstHspa->iHspaDaDelayCS = strtoul(attrValue, (char **)NULL, 10);
  }
  else if (strncmp(attrName,"HspaDaDelayPS",strlen("HspaDaDelayPS")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid acsURL %s\n",attrValue);
    }
    else
     pstHspa->iHspaDaDelayPS = strtoul(attrValue, (char **)NULL, 10);
  }


   /*START -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/
   else if (strncmp(attrName,"HspaVoiceDomain",strlen("HspaVoiceDomain")) == 0) {
	   if (xmlState.verify)
		 status = BcmDb_validateBoolean(attrValue);
	   if (status != DB_OBJ_VALID_OK) {
		 xmlState.errorFlag = PSX_STS_ERR_FATAL;
		 syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid HspaVoiceDomain %s\n",attrValue);
	   }
	   else
		 pstHspa->iHspaVoiceDomain = atoi(attrValue);

   }
   /*END -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/
   else if (strncmp(attrName,"HspaVoiceServiceFlag",strlen("HspaVoiceServiceFlag")) == 0) {
	   if (xmlState.verify)
		 status = BcmDb_validateNumber(attrValue);
	   if (status != DB_OBJ_VALID_OK) {
		 xmlState.errorFlag = PSX_STS_ERR_FATAL;
		 syslog(LOG_ERR,"xmlGetTr69cAttr(): invalid HspaVoiceServiceFlag %s\n",attrValue);
	   }
	   else
		 pstHspa->cHspaVoiceServiceFlag = atoi(attrValue);
   }
   /* add mobile operator selection. <tanyin 2009.4.9> */
   else if (strncmp(attrName,"OperSelect",strlen("OperSelect")) == 0) {
		 strncpy(pstHspa->cOperSelect, attrValue, sizeof(pstHspa->cOperSelect)-1);

   }
   else if (strncmp(attrName,"Operator",strlen("Operator")) == 0) {
		 strncpy(pstHspa->cOperatorList, attrValue, sizeof(pstHspa->cOperatorList)-1);
   }


#ifdef XML_DEBUG
  printf("xmlGetTr69cAttr(psiValue %s), validationStatus %d\n",
	 (char*)psiValue,status);
#endif

  return(PSX_STS_OK);
}

//给psiValue分配内存，并赋值

extern "C" PSX_STATUS xmlStartHspaObj(char *appName, char *objName)
{
    HSPA_INFO *pstE220;
#ifdef XML_DEBUG
    printf("xmlStartHspaObj(calling cfm to get appName %s)\n", appName);
#endif
    if(appName == NULL)
    {
        printf("xmlStartHspaObj: error");
    }

    psiValue = malloc(sizeof(HSPA_INFO));
    if (psiValue == NULL) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset(psiValue, 0, sizeof(HSPA_INFO));
    pstE220 = (HSPA_INFO*)psiValue;
    pstE220->iInstantMode = 1;
    pstE220->iDaHspaDelayCS = 0;
    pstE220->iDaHspaDelayPS = 0;
    pstE220->iHspaDaDelayCS = 0;
    pstE220->iHspaDaDelayPS = 0;
    pstE220->iIcmpSpeedCyle =0 ;
    pstE220->iSwitchConfirm = 1;

    /*START -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/
    pstE220->iHspaVoiceDomain = 0;
    /*END -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/

    pstE220->cHspaVoiceServiceFlag = 3;  /*[0:1] 11 means cs and ps all enable*/
	/* operator selection. <tanyin 2009.4.9> */
	strcpy(pstE220->cOperSelect, "auto");
	pstE220->cOperatorList[0] = 0;

    return PSX_STS_OK;
}

extern "C" PSX_STATUS xmlEndHspaObj(char *appName, char *objName)
{
	HSPA_INFO* pstE220 = (HSPA_INFO*)psiValue;
  	int error = 0;
#ifdef XML_DEBUG
  	printf("xmlEndE220Obj(calling cfm to get appName %s)\n", appName);
#endif
	
    BcmDb_setHspaInfo(pstE220);
  	free(psiValue);
  	psiValue = NULL;

  	return PSX_STS_OK;
}
/*end  of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
/********************** end base64 decode and encode functions **********************/

// add by l66195 for pppousb start
PSX_STATUS xmlSetE220Node(FILE * file, const char *appName, const UINT16 id,
			const PSI_VALUE value, const UINT16 length)
{
	if((appName == NULL) || (file == NULL) || (value == NULL))
	{
		return PSX_STS_ERR_FATAL;
	}
    xmlSetE220Data(file, appName, id, value, length);
	return PSX_STS_OK;
}


PSX_STATUS xmlGetE220Node(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
	if((appName == NULL) || (attrName == NULL))
	{
		return PSX_STS_ERR_FATAL;
	}
	return(xmlGetE220Data(attrName, attrValue));
}

//给psiValue分配内存，并赋值

PSX_STATUS xmlStartE220Obj(char *appName, char *objName)
{
	E220_INFO *pstE220;
#ifdef XML_DEBUG
  	printf("xmlStartE220Obj(calling cfm to get appName %s)\n", appName);
#endif
	if(appName == NULL)
	{
		printf("xmlStartE220Obj: error");
	}
    // 给psiValue分配内存
    psiValue = (void*)malloc(sizeof(E220_INFO));
    if (psiValue == NULL) 
	{
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset(psiValue, 0, sizeof(E220_INFO));
	// 准备给psiValue赋值
    pstE220 = (E220_INFO*)psiValue;
// 下面是获取缺省值，没有就注释掉
	pstE220->usLinkMode = atoi(BcmDb_getDefaultValue("LinkMode"));
/* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
	pstE220->usVoiceEnable = atoi(BcmDb_getDefaultValue("VoiceEnable"));
/* END:   Added by s00125931, 2008/9/12 */
//    strcpy(pstE220->pszOpenWifiGWanIfc, BcmDb_getDefaultValue("openWifiGWanIfc"));
//    strcpy(pstE220->pszOpenWifiRWanIfc, BcmDb_getDefaultValue("openWifiRWanIfc"));
	
  	return PSX_STS_OK;
}


//将保存在psiValue中的信息拷贝到psi中
PSX_STATUS xmlEndE220Obj(char *appName, char *objName)
{
	E220_INFO* pstE220 = (E220_INFO*)psiValue;
  	int error = 0;
#ifdef XML_DEBUG
  	printf("xmlEndE220Obj(calling cfm to get appName %s)\n", appName);
#endif
	if(appName == NULL)
	{
		printf("xmlEndE220Obj: error");
		return PSX_STS_ERR_FATAL;
	}
	
    BcmDb_setE220Info(IFC_E220_ID, pstE220);

  	free(psiValue);
  	psiValue = NULL;

  	return PSX_STS_OK;
}

//将字段信息写入psiValue
PSX_STATUS xmlGetE220Data(char * attrName, char * attrValue)
{
	if(attrName == NULL)
	{
		printf("xmlGetE220Data: error\n");
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		return PSX_STS_ERR_FATAL;
	}
	// 获取psiValue的地址
    E220_INFO* pstE220 = (E220_INFO*)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
	if(pstE220 == NULL)
	{
		printf("xmlGetE220Data: pstE220==NULL\n");
	}
	
#ifdef XML_DEBUG
  	printf("xmlGetE220Data(attrName %s, attrValue %s)\n", attrName,attrValue);
#endif
	// 拷贝LinkMode
  	if (strncmp(attrName, "LinkMode", strlen("LinkMode")) == 0) 
  	{

        pstE220->usLinkMode = atoi(attrValue);

  	}
	// 拷贝OperationMode
  	else if (strncmp(attrName,"OperationMode", strlen("OperationMode")) == 0) 
	{
        pstE220->usOperMode = atoi(attrValue);
    }
	// 拷贝ConType
  	else if (strncmp(attrName,"ConType", strlen("ConType")) == 0) 
	{
        pstE220->usConType = atoi(attrValue);
    }
	else if ( 0 == strncmp(attrName, "Channel", strlen("Channel")))
	{
        pstE220->usChannelSelect = atoi(attrValue);
	}
    else if (0 == strncmp(attrName, "VoiceChannel", strlen("VoiceChannel")))
    {
        pstE220->usVoiceChannel = atoi(attrValue);
    }
    /* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
    else if (0 == strncmp(attrName, "VoiceEnable", strlen("VoiceEnable")))
	{
        pstE220->usVoiceEnable = atoi(attrValue);
    }
    /* END:   Added by s00125931, 2008/9/12 */
	/*
	else if ( 0 == strncmp(attrName, "OpenWifiDestIp10", strlen("OpenWifiDestIp10")))
	{
	    if ( xmlState.verify && (strlen(attrValue) > 0))
	    {
	        status = BcmDb_validateIpAddress(attrValue);
	    }
		if ( DB_OBJ_VALID_OK != status)
		{
		    xmlState.errorFlag = PSX_STS_ERR_FATAL;
      		syslog(LOG_ERR,"xmlGetOpenWifiData(): invalid OpenWifi Dest Ip %s\n",attrValue);
		}
		else
		{
		    strcpy(pstE220->pszOpenWifiDestIP10, attrValue);
		}
	}
	*/
       /*start of  增加 西班牙 语音接口选择 功能 by s53329 at  20091017*/
       else if(0 == strncmp(attrName, "VoiceInterface", strlen("VoiceInterface")))
       {
                pstE220->usVoiceInterface = atoi(attrValue);
       }
       /*end  of  增加 西班牙 语音接口选择 功能 by s53329 at  20091017*/
  	return(PSX_STS_OK);
}

//将psi中的配置设置到flash
void xmlSetE220Data(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
	E220_INFO* pstE220;
   	if ((file == NULL) || (name == NULL) || (value == NULL))
    {  
        printf("xmlSetE220Data: error\n");
    	return;
   	}

		
	pstE220 = (E220_INFO*)value;

		/* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
	fprintf(file,"<Entry LinkMode=\"%d\" OperationMode=\"%d\" ConType=\"%d\" " 
	"Channel=\"%d\" VoiceChannel =\"%d\" VoiceEnable =\"%d\"  VoiceInterface=\"%d\"/>\n", 
		pstE220->usLinkMode, pstE220->usOperMode, pstE220->usConType, 
		pstE220->usChannelSelect, pstE220->usVoiceChannel, pstE220->usVoiceEnable, pstE220->usVoiceInterface);
                /* END:   Added by s00125931, 2008/9/12 */

}


PSX_STATUS xmlSetPppProfileObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length)
{
   UINT16 size = sizeof(pppProfileObjs) / sizeof(PSX_OBJ_ITEM);

   if ( id >= size ) {
#ifdef XML_DEBUG
      fprintf(file,"<appName %s, objectId %d not implemented/>\n",appName,id);
#endif
      return PSX_STS_ERR_FIND_HDL_FNC;
   }

   if (*(pppProfileObjs[id].setAttrFnc) != NULL)
     (*(pppProfileObjs[id].setAttrFnc))(file, pppProfileObjs[id].objName, id,  value, length);
   return PSX_STS_OK;
}


PSX_STATUS xmlGetPppProfileObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlGetPMapObjNode(appName %s, objName %s, attrName %s, attrValue %s\n",
	 appName,objName,attrName,attrValue);
#endif

  for ( i = 0; pppProfileObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pppProfileObjs[i].objName, objName) == 0) {
      if (*(pppProfileObjs[i].getAttrFnc) != NULL) {
	sts = (*(pppProfileObjs[i].getAttrFnc))(attrName, attrValue);
	break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("xmlGetPppProfileObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 


PSX_STATUS xmlStartPppProfileObjNode(char *appName, char *objName)
{
  int i = 0;
  PSX_STATUS sts = PSX_STS_OK;

#ifdef XML_DEBUG
  printf("xmlStartPppProfileObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; pppProfileObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pppProfileObjs[i].objName, objName) == 0) {
      if (*(pppProfileObjs[i].startObjFnc) != NULL) {
        sts = (*(pppProfileObjs[i].startObjFnc))(appName, objName);
        break;
      } /* apps */
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("xmlStartPppProfileObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
     printf("Unregconizable objName %s\n",objName);
#endif
  }
  
  return sts;
} 

PSX_STATUS xmlEndPppProfileObjNode(char *appName, char *objName)
{
  PSX_STATUS sts = PSX_STS_OK;
  int i = 0;

#ifdef XML_DEBUG
  printf("xmlEndPppProfileObjNode(appName %s, objName %s\n",appName,objName);
#endif

  for ( i = 0; pppProfileObjs[i].objName[0] != '\0'; i++ ) {
    if (strcmp(pppProfileObjs[i].objName, objName) == 0) {
      if (*(pppProfileObjs[i].endObjFnc) != NULL) {
	sts = (*(pppProfileObjs[i].endObjFnc))(appName, objName);
	break;
      }
    } /* found */
  } /* for */
  
  if (sts == PSX_STS_ERR_FIND_OBJ) {
    printf("xmlEndPMapObjNode(): Unrecognizable objName %s, ignored\n",objName);    
#ifdef XML_DEBUG
    printf("Unregconizable objName %s\n",objName);
#endif
   }
   
   return sts;
} 



void xmlSetPppProfileAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length)
{
 //  printf("xmlSetPppProfileAttr..................\n");
   PROFILE_INFO_ST *pPro, *pEntry;
   int entry = 0;
   int num = 0;
   static char* method[] = {"AUTO", "PAP", "CHAP", "MSCHAP"};
   char addr1[IFC_TINY_LEN], addr2[IFC_TINY_LEN];
   static char* state[] = {"disable","enable"};
 //  char groupName[IFC_TINY_LEN], ifList[IFC_LARGE_LEN];

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name, TBLNAME_PPPPROFILE) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     pPro = (PROFILE_INFO_ST*)value;
     /* determine the size of route table */
     //num = length/sizeof(PORT_MAP_CFG_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       pEntry = &pPro[entry];
/* BEGIN: Modified by y67514, 2009/12/10   问题单号:APNSecurity Enhancement*/
	   #ifndef SUPPORT_PPPPWD_UNENCRYPTED
       base64Encode((unsigned char *)pEntry->cPassword, strlen(pEntry->cPassword), &base64);

     //  printf("autoapn = %d ...........\n",pEntry->ulautoApn);
       fprintf(file,"<%s id=\"%d\" ProName=\"%s\" UserName=\"%s\" PassWord=\"%s\" PhoneNum=\"%s\" auth=\"%d\" autodns=\"%d\" preferredDns=\"%s\" alternateDns=\"%s\" autoApn=\"%d\" acApn=\"%s\"/>\n",
	       name,(entry+1),pEntry->cProfileName,pEntry->cUserName, base64,
               pEntry->cDialNumber,pEntry->ulauthMethod, pEntry->ulautoDns,
               pEntry->cPrimaryDNS, pEntry->cSecondaryDNS,  pEntry->ulautoApn, pEntry->cAPN);
             
       free(base64);
	   #else
	   fprintf(file,"<%s id=\"%d\" ProName=\"%s\" UserName=\"%s\" PassWord=\"%s\" PhoneNum=\"%s\" auth=\"%d\" autodns=\"%d\" preferredDns=\"%s\" alternateDns=\"%s\" autoApn=\"%d\" acApn=\"%s\"/>\n",
	       name,(entry+1),pEntry->cProfileName,pEntry->cUserName, pEntry->cPassword,
               pEntry->cDialNumber,pEntry->ulauthMethod, pEntry->ulautoDns,
               pEntry->cPrimaryDNS, pEntry->cSecondaryDNS,  pEntry->ulautoApn, pEntry->cAPN);
	   #endif
/* END:   Modified by y67514, 2009/12/10 */
     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",pppProfileObjs[id-1].objName); 
   } /* pmap table */
}

PSX_STATUS xmlGetPppProfileAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
#ifdef XML_DEBUG
  printf("xmlGetPppProfileAttr(attrName %s, attrValue %s)............\n",
	 attrName,attrValue);
#endif

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, XML_MAX_TOD_ENTRY);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(PROFILE_INFO_ST) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(PROFILE_INFO_ST) * xmlState.tableSize);
      /* BEGIN: Added by y67514, 2010/1/2   PN:APNSecurity Enhancement*/
      PROFILE_INFO_ST* pProTbl = (PROFILE_INFO_ST*)psiValue;
      for ( int i = 0 ; i < xmlState.tableSize ; i++ )
      {
          HspaGetApn(pProTbl[i].cAPN);
          HspaGetUserName(pProTbl[i].cUserName);
          HspaGetPwd(pProTbl[i].cPassword);
      }
      /* END:   Added by y67514, 2010/1/2 */
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPppProfileObj(char *appName, char *objName)
{
  int error = 0;
  
#ifdef XML_DEBUG
  printf("xmlEndPppProfileObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlEndPppProfileObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setPppProfileCfgSize(xmlState.tableSize);
    BcmDb_setPppProfileCfgInfo((PROFILE_INFO_ST*)psiValue, xmlState.tableSize);
  }

#ifdef XML_DEBUG1
  printf("\n============End PPP Profile=========\n");
  printf("xmlEndPppProfileObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
#endif

  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetPppProfileEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PROFILE_INFO_ST* pProTbl = (PROFILE_INFO_ST*)psiValue;
  int index = 0;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    printf("xmlGetPppProfileEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

#ifdef XML_DEBUG
  printf("xmlGetPppProfileEntryAttr(attrName %s, attrValue %s).............\n",
	 attrName,attrValue);
#endif      
  if (strcmp(attrName,"ProName") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pProTbl[i].cProfileName, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileEntryAttr(): invalid Proname %s\n",attrValue);
    }
  }
  /* BEGIN: Modified by y67514, 2009/12/11   问题单号:APNSecurity Enhancement*/
  else if (strcmp(attrName,"UserName") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, PPP_PROFILE_USERNAME_LEN);
    if (status == DB_OBJ_VALID_OK)
    {
        char acUserName[PPP_PROFILE_USERNAME_LEN +1];   
        memset(acUserName,0,sizeof(acUserName));
        HspaGetUserName(acUserName);
        if ( 0 == strlen(acUserName) && attrValue[0] != '\0')
        {
            HspaSetUserName(attrValue);
        }
      strcpy(pProTbl[i].cUserName, attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileEntryAttr(): invalid username %s\n",attrValue);
    }
  } 
  else if (strncmp(attrName,"PassWord",strlen("PassWord")) == 0) 
  {
    int dataLen = 0;
    char *data = NULL;
  
    int len = strlen(attrValue);
    if ( BcmDb_getEncodePasswordInfo() == TRUE ) {
      data = (char *)malloc(len * 3/4 + 8);
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(data, 0, len *3/4 + 8);
	  /*add by w00135351 09.3.14*/
	  #ifndef SUPPORT_PPPPWD_UNENCRYPTED
      base64Decode((unsigned char *)data, attrValue, &dataLen);
      data[dataLen] = '\0';
	  #else
	  strcpy(data,attrValue);
	  #endif
	  /*end by w00135351 09.3.14*/
    } else {
      data = (char *)malloc(len);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(data, 0, len);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      strcpy(data, attrValue);
      BcmDb_setEncodePasswordInfo(TRUE);
    }
      
    if (xmlState.verify) 
      status = BcmDb_validatePppPassword(data);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppAttr(): invalid password %s\n",attrValue);
    }
    else
    {
        char acPwd[PPP_PROFILE_PASSWORD_LEN + 1];   
        memset(acPwd,0,sizeof(acPwd));
        HspaGetPwd(acPwd);
        if ( 0 == strlen(acPwd) && data[0] != '\0' )
        {
            HspaSetPwd(data);
        }
      strcpy(pProTbl[i].cPassword,data);
    }
    free(data);
  }
  /* END:   Modified by y67514, 2009/12/11 */
  else if (strcmp(attrName,"PhoneNum") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, IFC_LARGE_LEN);
    if (status == DB_OBJ_VALID_OK)
      strcpy(pProTbl[i].cDialNumber, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileEntryAttr(): invalid Phone %s\n",attrValue);
    }
  } 
  else if (strncmp(attrName,"auth",strlen("auth")) == 0) {
      pProTbl[i].ulauthMethod = atoi(attrValue);
  }
  else if (strncmp(attrName,"autodns",strlen("autodns")) == 0) {
      pProTbl[i].ulautoDns = atoi(attrValue);
  }
  else if (strncmp(attrName,"preferredDns",strlen("preferredDns")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppAttr(): invalid auth %s\n",attrValue);
    }
    else
      strcpy(pProTbl[i].cPrimaryDNS, attrValue);
  }
  else if (strncmp(attrName,"alternateDns",strlen("alternateDns")) == 0) {
    if (xmlState.verify) 
      status = BcmDb_validateIpAddress(attrValue);
    if (status != DB_OBJ_VALID_OK) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppAttr(): invalid auth %s\n",attrValue);
    }
    else
     strcpy(pProTbl[i].cSecondaryDNS, attrValue);
  }
  else if (strncmp(attrName,"autoApn",strlen("autoApn")) == 0) {
      pProTbl[i].ulautoApn = atoi(attrValue);
  }
  else if (strcmp(attrName,"acApn") == 0) {
    /* BEGIN: Modified by y67514, 2009/12/10   问题单号:APNSecurity Enhancement*/
    if (xmlState.verify && attrValue[0] != '\0')
    {
        status = BcmDb_validateLength(attrValue, PPP_PROFILE_APN_LEN);
    }
    if (status == DB_OBJ_VALID_OK)
    {
        char acApn[PPP_PROFILE_APN_LEN +1];   
        memset(acApn,0,sizeof(acApn));
        HspaGetApn(acApn);
        if ( 0 == strlen(acApn) && attrValue[0]!='\0' )
        {
            HspaSetApn(attrValue);
        }
      strcpy(pProTbl[i].cAPN, attrValue);
    }
    else 
    {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      printf("xmlGetPppProfileEntryAttr(): invalid APN %s\n",attrValue);
    }
    /* END:   Modified by y67514, 2009/12/10 */
  } 
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndPppProfileEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return(PSX_STS_OK);
}

// add by l66195 for pppousb end
PSX_STATUS xmlStartVIBCfgInfoNode(char *appName, char *objName)
{
  PBCMCFM_VIBINTERFACECFG pstVibCfg;

#ifdef XML_DEBUG
  printf("xmlStartVIBCfgInfoNode(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  psiValue = (void*)malloc(sizeof(BCMCFM_VIBINTERFACECFG));
  /* 初始化HSPA 控制参数节点 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset(psiValue, 0, sizeof(BCMCFM_VIBINTERFACECFG));
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  pstVibCfg = (PBCMCFM_VIBINTERFACECFG)psiValue;
  memset(pstVibCfg, 0, sizeof(BCMCFM_VIBINTERFACECFG));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndVIBCfgInfoNode(char *appName, char *objName)
{
  PBCMCFM_VIBINTERFACECFG pstVibCfg = (PBCMCFM_VIBINTERFACECFG)psiValue;

#ifdef XML_DEBUG
  printf("xmlEndVIBCfgInfoNode(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

  /* check to see if all mandatory object attribute are set */
  /* no mandatory attribute */
  //if (!xmlState.verify)
  HWDb_setVDFWanIfCfgInfo(pstVibCfg);

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

PSX_STATUS xmlSetVIBCfgInfoNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
    PBCMCFM_VIBINTERFACECFG stInfo;
#ifdef XML_DEBUG
    printf("Writing...............\n");
#endif
    if ((appName == NULL) || (value == NULL))
       return PSX_STS_OK;

    stInfo = (PBCMCFM_VIBINTERFACECFG)value;
    fprintf(file,"<vdfWanCfg ConnectOrder=\"%d\" Band=\"%s\"/>\n",
        stInfo->iConnectOrder, stInfo->cBand);
#ifdef XML_DEBUG
    printf("Writing.....over..........\n");
#endif
   return PSX_STS_OK;
}

PSX_STATUS xmlGetVIBCfgInfoNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
    PBCMCFM_VIBINTERFACECFG stInfo = (PBCMCFM_VIBINTERFACECFG)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetVIBCfgInfoNode(attrName %s, attrValue %s)\n",
     attrName,attrValue);
#endif

    if (strcmp(attrName,"ConnectOrder") == 0)
    {
       status = BcmDb_validateNumber(attrValue);
       if (DB_OBJ_VALID_OK != status)
       {
           return PSX_STS_ERR_FATAL;
       }
       stInfo->iConnectOrder = atoi(attrValue);
    }
    else if (strcmp(attrName, "Band") == 0)
    {
        status = BcmDb_validateHexNumber(attrValue);
        if (DB_OBJ_VALID_OK != status)
        {
            return PSX_STS_ERR_FATAL;
        }
        strcpy(stInfo->cBand, attrValue);
    }
    

    return(PSX_STS_OK);

}



PSX_STATUS xmlStartVIBInterfaceCfgNode(char *appName, char *objName)
{
	PBCMCFM_VIBINTERFACECFG pstVibCfg;

#ifdef XML_DEBUG
	printf("xmlStartVIBCfgInfoNode(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
#endif

	psiValue = (void*)malloc(sizeof(PBCMCFM_VIBINTERFACECFG));
	/* 初始化HSPA 控制参数节点 */
	if (psiValue == NULL) {
	  xmlMemProblem();
	  return PSX_STS_ERR_FATAL;
	}
	memset(psiValue, 0, sizeof(PBCMCFM_VIBINTERFACECFG));
	/* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
	pstVibCfg = (PBCMCFM_VIBINTERFACECFG)psiValue;
	pstVibCfg->iConnectOrder   = 0;
	pstVibCfg->cBand[0]  = 0;

	return PSX_STS_OK;
}

PSX_STATUS xmlEndVIBInterfaceCfgCfgNode(char *appName, char *objName)
{
    PBCMCFM_VIBINTERFACECFG pstVibCfg = (PBCMCFM_VIBINTERFACECFG)psiValue;

#ifdef XML_DEBUG
    printf("xmlEndVIBCfgInfoNode(calling cfm to get appName %s, objName %s\n",
    appName,objName);
#endif
    HWDb_setVIBInterfaceCfgInfo(pstVibCfg);
    free(psiValue);
    psiValue = NULL;
    return PSX_STS_OK;
}

PSX_STATUS xmlSetVIBInterfaceCfgCfgNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length)
{
    PBCMCFM_VIBINTERFACECFG pstInfo;
#ifdef XML_DEBUG
    printf("Writing...............\n");
#endif
    if ((appName == NULL) || (value == NULL))
       return PSX_STS_OK;

    pstInfo = (PBCMCFM_VIBINTERFACECFG)value;
    fprintf(file,"<VibInterfaceCfg ConnectOrder=\"%d\" VibBand=\"%s\"/>\n",
        pstInfo->iConnectOrder, pstInfo->cBand);
#ifdef XML_DEBUG
    printf("Writing.....over..........\n");
#endif
   return PSX_STS_OK;
}

PSX_STATUS xmlGetVIBInterfaceCfgCfgNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
    PBCMCFM_VIBINTERFACECFG pstInfo = (PBCMCFM_VIBINTERFACECFG)psiValue;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

#ifdef XML_DEBUG
    printf("xmlGetVIBCfgInfoNode(attrName %s, attrValue %s)\n",
     attrName,attrValue);
#endif

    if (strcmp(attrName,"ConnectOrder") == 0)
    {
       
       pstInfo->iConnectOrder = atoi(attrValue);
    }
    else if (strcmp(attrName, "VibBand"))
    {
        strcpy(pstInfo->cBand, attrName);
    }
    return(PSX_STS_OK);

}

//start add for vdf qos by p44980 2008.01.08
#ifdef SUPPORT_VDF_QOS
void xmlSetQosQueueAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PQOS_QUEUE_ENTRY qos, ptr;
   int objIndex = 0;
   int num = 0;

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_QOS_QUEUE) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
       qos = (PQOS_QUEUE_ENTRY)value;
       num = xmlGetTableSize(file);
       for (objIndex = 0; objIndex < num; objIndex++) {
       ptr = &qos[objIndex];
       fprintf(file,"<%s id=\"%u\" precedence=\"%u\" weight=\"%u\" mark=\"%u\" schAlg=\"%s\"/>\n",
          name, ptr->ulQueueInstId, ptr->ulPrecedence, ptr->ulWeight, ptr->ulMark, ptr->acSchAlg);
      } /* for entry */
     if ( num != 0 )
          fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}


PSX_STATUS xmlGetQosQueueAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, QOS_MAX_QUEUE);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(QOS_QUEUE_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(QOS_QUEUE_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), QOS_MAX_QUEUE);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlEndQosQueueObj(char *appName, char *objName)
{
  int error = 0;

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosQueueObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
  if (!error && !xmlState.verify) {
    BcmDb_setQosQueueSize(xmlState.tableSize);
    BcmDb_setQosQueueInfo((PQOS_QUEUE_ENTRY)psiValue, xmlState.tableSize);
  }

  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

PSX_STATUS xmlGetQosQueueEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PQOS_QUEUE_ENTRY qosQueueTbl = (PQOS_QUEUE_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
      return PSX_STS_ERR_FATAL;
      /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
    }
  }

  if (strcmp(attrName,"id") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK )
      qosQueueTbl[i].ulQueueInstId = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid id %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"precedence") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosQueueTbl[i].ulPrecedence = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid precedence %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"weight") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosQueueTbl[i].ulWeight = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid weight %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"mark") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      qosQueueTbl[i].ulMark = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid mark %s\n",attrValue);
    }
  }
 else if (strcmp(attrName,"schAlg") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN);
    if (status == DB_OBJ_VALID_OK)
    	strcpy(qosQueueTbl[i].acSchAlg, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosQueueEntryAttr(): invalid schAlg%s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartQosQueueEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PQOS_QUEUE_ENTRY qosQueueTbl = (PQOS_QUEUE_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlStartQosQueueEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }

  memset((char*)(qosQueueTbl+i), 0, sizeof(QOS_QUEUE_ENTRY));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndQosQueueEntryObj(char *appName, char *objName)
{
  if (xmlState.verify) {
  	int i = xmlState.tableIndex;
    PQOS_QUEUE_ENTRY qosQueueTbl = (PQOS_QUEUE_ENTRY)psiValue;

    if ( qosQueueTbl == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        xmlState.tableIndex = 0;
        return (PSX_STS_ERR_FATAL);
    }

    if ((qosQueueTbl[i].ulMark < EN_QOS_EF) && (qosQueueTbl[i].ulMark > EN_QOS_AF4)) 
   {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosQueueEntryObj: queue mark is invalid .\n");
    }      
  }
  xmlState.tableIndex++;
  return PSX_STS_OK;
}

void xmlSetQosCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length)
{
   int bEnable = FALSE;
    
   if ((name == NULL) || (value == NULL))
      return;

   bEnable = *(int*)value;
   fprintf(file,"<%s enable=\"%d\"/>\n",name,bEnable);
}

PSX_STATUS xmlGetQosCfgAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  int *pbEnable = (int*)psiValue;
  
  if (strcmp(attrName,"enable") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK)
      *pbEnable = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetQosCfgAttr(): invalid qos cfg %s\n",attrValue);
    }
  }  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartQosCfgObj(char *appName, char *objName)
{
  psiValue = (void*)malloc(IFC_TINY_LEN);
  /* start of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  /* end of maintain PSI移植:  分配内存后需要判断是否为空 by xujunxia 43813 2006年5月5日 */
  memset((char*)psiValue, '\0', sizeof(IFC_TINY_LEN));
  return PSX_STS_OK;
}

PSX_STATUS xmlEndQosCfgObj(char *appName, char *objName)
{
  if (!xmlState.verify)
      BcmDb_setQosCfgInfo(*(int*)psiValue);

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}



void xmlSetQosServiceAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,  const UINT16 length)
{
    PQOS_SERVICE_CFG_ENTRY qos, ptr;
    int objIndex = 0;
    int num = 0;

   if ((name == NULL) || (value == NULL))
      return;

   if (strcmp(name,TBLNAME_QOS_SERVICE) == 0) {
     num = *(UINT16*)value;
     if ( num != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
       qos = (PQOS_SERVICE_CFG_ENTRY)value;
       num = xmlGetTableSize(file);
       for (objIndex = 0; objIndex < num; objIndex++) {
       ptr = &qos[objIndex];
       fprintf(file,"<%s id=\"%u\" parent=\"%u\" serviceName=\"%s\" protocol=\"%d\" srcPort=\"%s\" srcMaxPort=\"%s\" dstPort=\"%s\" dstMaxPort=\"%s\"/>\n",
          name, ptr->ulInstanceId, ptr->ulParentId, ptr->acServiceName, ptr->protocol, ptr->srcPort, ptr->srcMaxPort, ptr->dstPort, ptr->dstMaxPort);
      } /* for entry */
     if ( num != 0 )
          fprintf(file,"</%s>\n",secObjs[id-1].objName);
   } /* table */
}


PSX_STATUS xmlGetQosServiceAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) 
  {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(QOS_SERVICE_CFG_ENTRY) * xmlState.tableSize);
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(QOS_SERVICE_CFG_ENTRY) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  }

  return(PSX_STS_OK);  
}


PSX_STATUS xmlEndQosServiceObj(char *appName, char *objName)
{
    int error = 0;
	/* check to see if all mandatory object attribute are set */  
	if (xmlState.verify) {
	  if (xmlState.tableSize != xmlState.tableIndex) {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlEndQosServiceObj(): table size does not match with number of entries.\n");
	    error = 1;
	  }
	}
	if (!error && !xmlState.verify) {
	  BcmDb_setQosServiceSize(xmlState.tableSize);
	  BcmDb_setQosServiceInfo((PQOS_SERVICE_CFG_ENTRY)psiValue, xmlState.tableSize);
	}

	free(psiValue);
	psiValue = NULL;
	xmlState.tableSize = 0;
	xmlState.tableIndex = 0;
	
	return PSX_STS_OK;
}


PSX_STATUS xmlGetQosServiceEntryAttr(char *attrName, char* attrValue)
{
    int i = xmlState.tableIndex;
	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
	PQOS_SERVICE_CFG_ENTRY qosServiceTbl = (PQOS_SERVICE_CFG_ENTRY)psiValue;


	if ( psiValue == NULL ) {
	  xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): psiValue is NULL\n");
	  return PSX_STS_ERR_FATAL;
	}
	  
	if (xmlState.verify) {
	  if ( i >= xmlState.tableSize ) {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid entry index %d, tableSize %d\n",
	           i, xmlState.tableSize);
	    /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
	    return PSX_STS_ERR_FATAL;
	    /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
	  }
	}

	if (strcmp(attrName,"id") == 0) {
	  if (xmlState.verify)
	    status = BcmDb_validateNumber(attrValue);
	  if (status == DB_OBJ_VALID_OK )
	    qosServiceTbl[i].ulInstanceId = atoi(attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid id %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"parent") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validateNumber(attrValue);
	  if (status == DB_OBJ_VALID_OK)
	    qosServiceTbl[i].ulParentId = atoi(attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid parent id %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"serviceName") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validateLength(attrValue, SEC_BUFF_MAX_LEN + 1);
	  if (status == DB_OBJ_VALID_OK)
	    strcpy(qosServiceTbl[i].acServiceName, attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid service name %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"protocol") == 0) {
	    qosServiceTbl[i].protocol = atoi(attrValue);
	}
	else if (strcmp(attrName,"srcPort") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
	  if (status == DB_OBJ_VALID_OK)
	  	strcpy(qosServiceTbl[i].srcPort, attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid srcPort %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"srcMaxPort") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
	  if (status == DB_OBJ_VALID_OK)
	  	strcpy(qosServiceTbl[i].srcMaxPort, attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid srcMaxPort %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"dstPort") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
	  if (status == DB_OBJ_VALID_OK)
	  	strcpy(qosServiceTbl[i].dstPort, attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid dstPort %s\n",attrValue);
	  }
	}
	else if (strcmp(attrName,"dstMaxPort") == 0) {
	  if (xmlState.verify && attrValue[0] != '\0')
	    status = BcmDb_validatePort(attrValue, XML_PORT_RANGE_MIN, XML_PORT_RANGE_MAX);
	  if (status == DB_OBJ_VALID_OK)
	  	strcpy(qosServiceTbl[i].dstMaxPort, attrValue);
	  else {
	    xmlState.errorFlag = PSX_STS_ERR_FATAL;
	    syslog(LOG_ERR,"xmlGetQosServiceEntryAttr(): invalid dstMaxPort %s\n",attrValue);
	  }
	}

	return(PSX_STS_OK);
}



PSX_STATUS xmlStartQosServiceEntryObj(char *appName, char *objName)
{
   int i = xmlState.tableIndex;
   PQOS_SERVICE_CFG_ENTRY qosServiceTbl = (PQOS_SERVICE_CFG_ENTRY)psiValue;

	if ( psiValue == NULL ) {
	  xmlState.errorFlag = PSX_STS_ERR_FATAL;
	  syslog(LOG_ERR,"xmlStartQosServiceEntryObj(): psiValue is NULL\n");
	  return PSX_STS_ERR_FATAL;
	}

	memset((char*)(qosServiceTbl+i), 0, sizeof(QOS_SERVICE_CFG_ENTRY));
	return PSX_STS_OK;
}



PSX_STATUS xmlEndQosServiceEntryObj(char *appName, char *objName)
{
   if (xmlState.verify) {
   	int i = xmlState.tableIndex;
    PQOS_SERVICE_CFG_ENTRY qosServiceTbl = (PQOS_SERVICE_CFG_ENTRY)psiValue;

    if ( qosServiceTbl == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        xmlState.tableIndex = 0;
        return (PSX_STS_ERR_FATAL);
    }

    if (((qosServiceTbl[i].srcPort[0] != '\0') || (qosServiceTbl[i].srcMaxPort[0] != '\0') 
    	|| (qosServiceTbl[i].dstPort[0] != '\0') || (qosServiceTbl[i].dstMaxPort[0] != '\0'))
    	&& qosServiceTbl[i].protocol != QOS_PROTO_TCP 
    	&& qosServiceTbl[i].protocol != QOS_PROTO_UDP)
   {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndQosServiceEntryObj: qos service  is invalid, id = %u .\n", qosServiceTbl[i].ulInstanceId);
    }      
  }
  xmlState.tableIndex++;
  return PSX_STS_OK;
}
#endif
//end add for vdf qos by p44980 2008.01.08

#pragma SECFW_CFG
void xmlSetSecFwAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   char *fwLevelStr[] = {"disable", "standard", "low", "medium", "high", "blockall", "custom", ""};
   unsigned short fwLevel = SCM_FW_LEVEL_DISABLE;
    
   if ((name == NULL) || (value == NULL))
      return;

   fwLevel = *(unsigned short*)value;

   if(fwLevel < SCM_FW_LEVEL_MAX)
	   fprintf(file,"<%s level=\"%s\"/>\n",name,fwLevelStr[fwLevel]);

}
PSX_STATUS xmlGetSecFwAttr(char *attrName, char* attrValue)
{
  char *fwLevelStr[] = {"disable", "standard", "low", "medium", "high", "blockall", "custom", ""};
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  unsigned short *pLevel = (unsigned short*)psiValue;
  unsigned short i;

  if (strcmp(attrName,"level") == 0) {
    if (!xmlState.verify)
	{
    	status = DB_OBJ_VALID_ERR_OUT_OF_RANGE;
    	for(i = 0; strcmp(fwLevelStr[i], ""); i++)
		{
			if(strcmp(fwLevelStr[i], attrValue))
				continue;
			status = DB_OBJ_VALID_OK;
			break;
		}
    	if(DB_OBJ_VALID_OK == status)
	    	*pLevel = i;
		else
			*pLevel = SCM_FW_LEVEL_DISABLE;
	}
    if (status == DB_OBJ_VALID_OK)
	{/*do nothing*/}
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwAttr(): invalid firewall level %s\n",attrValue);
    }
  } 
  
  return(PSX_STS_OK);
}

PSX_STATUS xmlStartSecFwObj(char *appName, char *objName)
{
  psiValue = (void*)malloc(sizeof(unsigned short));
  if (psiValue == NULL) {
      xmlMemProblem();
      return PSX_STS_ERR_FATAL;
  }
  memset((void*)psiValue, 0, sizeof(unsigned short));
  return PSX_STS_OK;
}
PSX_STATUS xmlEndSecFwObj(char *appName, char *objName)
{
  if (!xmlState.verify)
	{
      BcmDb_setSecFwCfg(*(unsigned short*)psiValue);
	}

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#pragma SECFW_TB
void xmlSetSecFwTbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
	PSEC_FW_ENTRY  pfw = NULL;
	PSEC_FW_ENTRY  ptr = NULL;
	char *act[] = {"Allow","Deny",""};
	char strAction[16];
	int entry, num = 0;
	unsigned char *ip = NULL;//for big end

	memset(strAction, 0, 16);
	if (strcmp(name,TBLNAME_SEC_FW) == 0) {
		num = *(UINT16*)value;
		if ( num != 0 )
			fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
	}
	else {
		pfw = (PSEC_FW_ENTRY)value;
		num = xmlGetTableSize(file);

		for (entry = 0; entry < num; entry++) {
			ptr = &pfw[entry];
		if(ptr->action != SCM_FW_ACTION_NONE)
		{
		    strcpy(strAction, act[ptr->action]);
		}
		else
		{
		    strAction[0] = '\0';
		}
            fprintf(file,"<%s id=\"%d\" name=\"%s\"", name, ptr->instID, ptr->name);
            ip = (unsigned char*)&ptr->srcipStart;
            fprintf(file," srcipStart=\"%u.%u.%u.%u\"", (unsigned  char)ip[0], (unsigned  char)ip[1], (unsigned  char)ip[2], (unsigned  char)ip[3]);
            ip = (unsigned char*)&ptr->srcipEnd;            
            fprintf(file," srcipEnd=\"%u.%u.%u.%u\"", (unsigned  char)ip[0], (unsigned  char)ip[1], (unsigned  char)ip[2], (unsigned  char)ip[3]);
            ip = (unsigned char*)&ptr->dstipStart;                       
            fprintf(file," dstipStart=\"%u.%u.%u.%u\"", (unsigned  char)ip[0], (unsigned  char)ip[1], (unsigned  char)ip[2], (unsigned  char)ip[3]);
            ip = (unsigned char*)&ptr->dstipEnd;                       
            fprintf(file," dstipEnd=\"%u.%u.%u.%u\"", (unsigned  char)ip[0], (unsigned  char)ip[1], (unsigned  char)ip[2], (unsigned  char)ip[3]);
            fprintf(file," srcIfc=\"%s\" dstIfc=\"%s\" proto=\"%d\" action=\"%s\"/>\n",
					ptr->srcIfc, ptr->dstIfc, ptr->proto, strAction);
		} /* for entry */
		
		if ( num != 0 )
            fprintf(file,"</%s>\n",secObjs[id-1].objName);
	}
}
PSX_STATUS xmlGetSecFwTbAttr(char *attrName, char* attrValue)
{
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_FW_CUSTOM_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_FW_ENTRY) * xmlState.tableSize);
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_FW_ENTRY) * xmlState.tableSize);
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwTbAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), SEC_VIR_SRV_MAX);
    }
  }
  
  return PSX_STS_OK;
}
PSX_STATUS xmlEndSecFwTbObj(char *appName, char *objName)
{
	int error = 0; 

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSecFwTbObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }

 if (!error && !xmlState.verify) {
    BcmDb_setSecFwSize(xmlState.tableSize);
    BcmDb_setSecFwInfo((PSEC_FW_ENTRY)psiValue, xmlState.tableSize);
  }
 
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}

#pragma SECFW_ENTRY
PSX_STATUS xmlGetSecFwEntryAttr(char *attrName, char* attrValue)
{
  int i = xmlState.tableIndex;
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
  PSEC_FW_ENTRY fwTbl = (PSEC_FW_ENTRY)psiValue;
  char *act[] = {"Allow","Deny"};

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }
    
  if (xmlState.verify) {
    if ( i >= xmlState.tableSize ) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid entry index %d, tableSize %d\n",
             i, xmlState.tableSize);
      return PSX_STS_ERR_FATAL;
    }
  }

	fwTbl += i;
  if (strcmp(attrName,"id") == 0) {
    if (xmlState.verify)
	{
    	status = BcmDb_validateNumber(attrValue);
	}
    if (status == DB_OBJ_VALID_OK)
      fwTbl->instID = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid instance id %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"name") == 0) {
    if (xmlState.verify)
	{
    	if(strlen(attrValue) < SCM_FW_NAME_MAX)
	        status = DB_OBJ_VALID_OK;
        else
	      	status = DB_OBJ_VALID_ERR_INVALID_LENGTH;
	}
    if (status == DB_OBJ_VALID_OK)
      strcpy(fwTbl->name, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcipStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK )
	{
		struct in_addr addr;
		inet_aton(attrValue, &addr);
		fwTbl->srcipStart = addr.s_addr;
	}
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid srcipStart %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcipEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK )
    {
	  struct in_addr addr;
	  inet_aton(attrValue, &addr);
	  fwTbl->srcipEnd= addr.s_addr;
	}
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid srcipEnd %s\n",attrValue);
    }
  }
    else if (strcmp(attrName,"dstipStart") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK )
	{
		struct in_addr addr;
		inet_aton(attrValue, &addr);
		fwTbl->dstipStart = addr.s_addr;
	}
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid dstipStart %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstipEnd") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateIpAddress(attrValue);
    if (status == DB_OBJ_VALID_OK )
    {
	  struct in_addr addr;
	  inet_aton(attrValue, &addr);
	  fwTbl->dstipEnd= addr.s_addr;
	}
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid dstipEnd %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"srcIfc") == 0) {
    if (xmlState.verify)
	{
    	if(strlen(attrValue) < IFC_TINY_LEN)
            status = DB_OBJ_VALID_OK;
    	else
	    	status = DB_OBJ_VALID_ERR_INVALID_LENGTH;
	}
    if (status == DB_OBJ_VALID_OK)
      strcpy(fwTbl->srcIfc, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid srcIfc %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"dstIfc") == 0) {
    if (xmlState.verify)
	{
    	if(strlen(attrValue) < IFC_TINY_LEN)
            status = DB_OBJ_VALID_OK;
    	else
	    	status = DB_OBJ_VALID_ERR_INVALID_LENGTH;
	}
    if (status == DB_OBJ_VALID_OK)
      strcpy(fwTbl->dstIfc, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid dstIfc %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"proto") == 0) {
    if (xmlState.verify && attrValue[0] != '\0')
      status = BcmDb_validateNumber(attrValue);
    if (status == DB_OBJ_VALID_OK && atoi(attrValue) <= SCM_FW_PROTOAPP_MAX)
      fwTbl->proto = atoi(attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid protocol %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"action") == 0) {
    if(xmlState.verify)
	{
	    if(attrValue[0] != '\0')
	    {
		if(strcmp(attrValue, act[0]) && strcmp(attrValue, act[1]))
			status = DB_OBJ_VALID_ERR_INVALID_VALUE;
		else
			status = DB_OBJ_VALID_OK;
	    }
	}
	if(status == DB_OBJ_VALID_OK)
	{
		if(attrValue[0] != '\0')
		{
		    fwTbl->action = (!strcmp(attrValue, act[0])) ? SCM_FW_ACT_ALLOW : SCM_FW_ACT_DENY;
		}
		else
		{
		    fwTbl->action = SCM_FW_ACTION_NONE;
		}
	}
	else{
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecFwEntryAttr(): invalid action %s\n",attrValue);
    }
  }
  return PSX_STS_OK;
}
PSX_STATUS xmlStartSecFwEntryObj(char *appName, char *objName)
{
  int i = xmlState.tableIndex;
  PSEC_FW_ENTRY fwTbl = (PSEC_FW_ENTRY)psiValue;

  if ( psiValue == NULL ) {
    xmlState.errorFlag = PSX_STS_ERR_FATAL;
    syslog(LOG_ERR,"xmlStartSecFwEntryObj(): psiValue is NULL\n");
    return PSX_STS_ERR_FATAL;
  }

  memset((char*)(fwTbl+i), 0, sizeof(SEC_FW_ENTRY));

  return PSX_STS_OK;
}
PSX_STATUS xmlEndSecFwEntryObj(char *appName, char *objName)
{
  xmlState.tableIndex++;
  return PSX_STS_OK;
}

#pragma SECDOS_CFG
void xmlSetSecDosAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
   PSEC_DOS_ENTRY dos = (PSEC_DOS_ENTRY)value;
   
   if ((name == NULL) || (value == NULL))
      return;
    /*modified by z67625 增加记录5种攻击方式开关值 start*/
   fprintf(file,"<%s enable=\"%d\" syn=\"%d\" fin=\"%d\" udp=\"%d\" icmp=\"%d\" synPerIp=\"%d\" finPerIp=\"%d\" udpPerIp=\"%d\" icmpPerIp=\"%d\" "
   	              "portScan=\"%d\" icmpSmurf=\"%d\" ipLand=\"%d\" SameSrcDst=\"%d\" ipSproof=\"%d\" ipTeardrop=\"%d\" pingOfDeath=\"%d\" tcpScan=\"%d\" SynWithData=\"%d\" "
   	              "udpBomb=\"%d\" ptScanSens=\"%d\" synSpd=\"%d\" finSpd=\"%d\" udpSpd=\"%d\" icmpSpd=\"%d\" synPerIpSpd=\"%d\" finPerIpSpd=\"%d\" "
   	              "udpPerIpSpd=\"%d\" icmpPerIpSpd=\"%d\" BrdcstSrc=\"%d\" LanSrcIp=\"%d\" Frgmthdr=\"%d\" InvalidTcpFlg=\"%d\"/>\n",
   	                name,
   	                dos->enable,
					dos->fldTcpSyn,
					dos->fldTcpFin,
					dos->fldUdp,
					dos->fldIcmp,
					dos->fldPerIpTcpSyn,
					dos->fldPerIpTcpFin,
					dos->fldPerIpUdp,
					dos->fldPerIpIcmp,

					dos->portScan,
					dos->icmpSmurf,
					dos->ipLand,
					/*add by z67625 增加记录5种攻击方式开关值 start*/
					dos->SameSrcDst,
					/*add by z67625 增加记录5种攻击方式开关值 end*/
					dos->ipSproof,
					dos->ipTeardrop,
					dos->pingOfDeath,
					dos->tcpScan,
					dos->SynWithData,
					dos->udpBomb,
					dos->ptScanSensitivity,

					dos->fldTcpSynSpd,
					dos->fldTcpFinSpd,
					dos->fldUdpSpd,
					dos->fldIcmpSpd,
					dos->fldPerIpTcpSynSpd,
					dos->fldPerIpTcpFinSpd,
					dos->fldPerIpUdpSpd,
					dos->fldPerIpIcmpSpd,
					/*add by z67625 增加记录5种攻击方式开关值 start*/
					dos->BrdcstSrc,
					dos->LanSrcIp,
					dos->Frgmthdr,
					dos->InvalidTcpFlg
					/*add by z67625 增加记录5种攻击方式开关值 end*/
    );
    /*modified by z67625 增加记录5种攻击方式开关值 end*/
}
PSX_STATUS xmlGetSecDosAttr(char *attrName, char* attrValue)
{  
	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    PSEC_DOS_ENTRY dos = (PSEC_DOS_ENTRY)psiValue;

	if (strcmp(attrName,"enable") == 0) {
	    if (xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	      dos->enable = atoi(attrValue);
	    else {
	      xmlState.errorFlag = PSX_STS_ERR_FATAL;
	      syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid firewall enable %s\n",attrValue);
	    }
	 }  
	else if(strcmp(attrName, "syn") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldTcpSyn = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid syn %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "fin") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldTcpFin = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid fin %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "udp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldUdp = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid udp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "icmp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldIcmp = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid icmp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "synPerIp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpTcpSyn = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid synPerIp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "finPerIp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpTcpFin = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid finPerIp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "udpPerIp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpUdp = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid udpPerIp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "icmpPerIp") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpIcmp = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid icmpPerIp %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "portScan") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->portScan = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid portScan %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "icmpSmurf") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->icmpSmurf = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid icmpSmurf %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "ipLand") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->ipLand = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid ipLand %s\n",attrValue);
	    }
	}
    /*add by z67625 增加获取5种攻击方式开关值 start*/
    else if(strcmp(attrName, "SameSrcDst") == 0){
        if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->SameSrcDst = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid SameSrcDst %s\n",attrValue);
	    }
    }
    else if(strcmp(attrName, "BrdcstSrc") == 0){
        if(xmlState.verify)
    		status = BcmDb_validateBoolean(attrValue);
        if (status == DB_OBJ_VALID_OK)
            dos->BrdcstSrc = atoi(attrValue);
        else {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid BrdcstSrc %s\n",attrValue);
        }
    }
    else if(strcmp(attrName, "LanSrcIp") == 0){
        if(xmlState.verify)
    		status = BcmDb_validateBoolean(attrValue);
        if (status == DB_OBJ_VALID_OK)
            dos->LanSrcIp= atoi(attrValue);
        else {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid LanSrcIp %s\n",attrValue);
        }
    }
    else if(strcmp(attrName, "Frgmthdr") == 0){
        if(xmlState.verify)
    		status = BcmDb_validateBoolean(attrValue);
        if (status == DB_OBJ_VALID_OK)
            dos->Frgmthdr = atoi(attrValue);
        else {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid Frgmthdr %s\n",attrValue);
        }
    }
    else if(strcmp(attrName, "InvalidTcpFlg") == 0){
        if(xmlState.verify)
    		status = BcmDb_validateBoolean(attrValue);
        if (status == DB_OBJ_VALID_OK)
            dos->InvalidTcpFlg = atoi(attrValue);
        else {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid InvalidTcpFlg %s\n",attrValue);
        }
    }
    /*add by z67625 增加获取5种攻击方式开关值 end*/
	else if(strcmp(attrName, "ipSproof") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->ipSproof = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid ipSproof %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "ipTeardrop") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->ipTeardrop = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid ipTeardrop %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "pingOfDeath") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->pingOfDeath = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid pingOfDeath %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "tcpScan") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->tcpScan = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid tcpScan %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "SynWithData") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->SynWithData = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid SynWithData %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "udpBomb") == 0){
		if(xmlState.verify)
			status = BcmDb_validateBoolean(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->udpBomb = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid udpBomb %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "ptScanSens") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->ptScanSensitivity = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid ptScanSens %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "synSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldTcpSynSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid synSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "finSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldTcpFinSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid finSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "udpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldUdpSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid udpSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "icmpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldIcmpSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid icmpSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "synPerIpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpTcpSynSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid synPerIpSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "finPerIpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpTcpFinSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid finPerIpSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "udpPerIpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpUdpSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid udpPerIpSpd %s\n",attrValue);
	    }
	}
	else if(strcmp(attrName, "icmpPerIpSpd") == 0){
		if(xmlState.verify)
			status = BcmDb_validateNumber(attrValue);
	    if (status == DB_OBJ_VALID_OK)
	        dos->fldPerIpIcmpSpd = atoi(attrValue);
	    else {
	        xmlState.errorFlag = PSX_STS_ERR_FATAL;
	        syslog(LOG_ERR,"xmlGetSecDosAttr(): invalid icmpPerIpSpd %s\n",attrValue);
	    }
	}
	return PSX_STS_OK;
}
PSX_STATUS xmlStartSecDosObj(char *appName, char *objName)
{  
	psiValue = (void*)malloc(sizeof(SEC_DOS_ENTRY));
	
	if (psiValue == NULL) {
		xmlMemProblem();
		return PSX_STS_ERR_FATAL;
	}
	
	memset((void*)psiValue, 0, sizeof(SEC_DOS_ENTRY));
	return PSX_STS_OK;
}
PSX_STATUS xmlEndSecDosObj(char *appName, char *objName)
{ 
  if (!xmlState.verify)
      BcmDb_setSecDosInfo((PSEC_DOS_ENTRY)psiValue);

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}

#pragma SECURLBLK_TB
void xmlSetSecUrlBlkTbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
	PSEC_URLBLK_ENTRY  purl = NULL;
	PSEC_URLBLK_ENTRY  ptr = NULL;
	int entry, num = 0;
	
	if (strcmp(name,TBLNAME_SEC_URLBLK) == 0) {
		num = *(UINT16*)value;
		if ( num != 0 )
			fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
	}
	else {
		purl = (PSEC_URLBLK_ENTRY)value;
		num = xmlGetTableSize(file);

		for (entry = 0; entry < num; entry++) {
			ptr = &purl[entry];			
            fprintf(file,"<%s id=\"%d\" url=\"%s\"/>\n", name, ptr->instID, ptr->url);
		} /* for entry */
		
		if ( num != 0 )
			fprintf(file,"</%s>\n",secObjs[id-1].objName);
	}
}
PSX_STATUS xmlGetSecUrlBlkTbAttr(char *attrName, char* attrValue)
{  
  DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_URLBLK_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      psiValue = (void *) malloc(sizeof(SEC_URLBLK_ENTRY) * xmlState.tableSize);
      if (psiValue == NULL) {
         xmlMemProblem();
         return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(SEC_URLBLK_ENTRY) * xmlState.tableSize);
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetSecUrlBlkTbAttr(): invalid number %s\n",attrValue);
      printf("Error: Invalid table size %d maximum table size must not exceed %d\n",
              atoi(attrValue), SEC_VIR_SRV_MAX);
    }
  }
	return PSX_STS_OK;
}
PSX_STATUS xmlEndSecUrlBlkTbObj(char *appName, char *objName)
{  
   int error = 0; 

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndSecUrlBlkTbObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }

 if (!error && !xmlState.verify) {
    BcmDb_setSecUrlBlkSize(xmlState.tableSize);
    BcmDb_setSecUrlBlkInfo((PSEC_URLBLK_ENTRY)psiValue, xmlState.tableSize);
  }
 
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;

  return PSX_STS_OK;
}

#pragma SECURLBLK_ENTRY
PSX_STATUS xmlGetSecUrlBlkEntryAttr(char *attrName, char* attrValue)
{  
	int i = xmlState.tableIndex;
	DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
	PSEC_URLBLK_ENTRY urlTbl = (PSEC_URLBLK_ENTRY)psiValue;

	if ( psiValue == NULL ) {
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		syslog(LOG_ERR,"xmlGetSecUrlBlkEntryAttr(): psiValue is NULL\n");
		return PSX_STS_ERR_FATAL;
	}

	if (xmlState.verify) {
		if ( i >= xmlState.tableSize ) {
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetSecUrlBlkEntryAttr(): invalid entry index %d, tableSize %d\n",
			i, xmlState.tableSize);
			return PSX_STS_ERR_FATAL;
		}
	}

	urlTbl += i;

	if (strcmp(attrName,"id") == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        if (status == DB_OBJ_VALID_OK)
            urlTbl->instID = atoi(attrValue);
        else {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecUrlBlkEntryAttr(): invalid url %s\n",attrValue);
        }
    }
    else if (strcmp(attrName,"url") == 0) {
		if (xmlState.verify)
		{
		    //support max SCM_URLBLK_MAX characters. <2008.12.22 tanyin>
			if(strlen(attrValue) <= SCM_URLBLK_MAX)
			    status = DB_OBJ_VALID_OK;
			else
			  	status = DB_OBJ_VALID_ERR_INVALID_LENGTH;
		}
		if (status == DB_OBJ_VALID_OK)
			strcpy(urlTbl->url, attrValue);
		else {
			xmlState.errorFlag = PSX_STS_ERR_FATAL;
			syslog(LOG_ERR,"xmlGetSecUrlBlkEntryAttr(): invalid url %s\n",attrValue);
		}
  }
	return PSX_STS_OK;
}
PSX_STATUS xmlStartSecUrlBlkEntryObj(char *appName, char *objName)
{  
	int i = xmlState.tableIndex;
    PSEC_URLBLK_ENTRY urlTbl = (PSEC_URLBLK_ENTRY)psiValue;

	if ( psiValue == NULL ) {
		xmlState.errorFlag = PSX_STS_ERR_FATAL;
		syslog(LOG_ERR,"xmlStartSecUrlBlkEntryObj(): psiValue is NULL\n");
		return PSX_STS_ERR_FATAL;
	}

	memset((char*)(urlTbl+i), 0, sizeof(SEC_URLBLK_ENTRY));

	return PSX_STS_OK;
}
PSX_STATUS xmlEndSecUrlBlkEntryObj(char *appName, char *objName)
{ 
	xmlState.tableIndex++;
	return PSX_STS_OK;
}

/*add by z67625 将过滤开关值写入配置文件 start*/
void xmlSetSecEnblAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length)
{
    PBcmCfm_SecEnblCfg_t pSecEnbl = NULL;
    if(NULL == name || NULL == value)
    {
        return;
    }
    pSecEnbl = (PBcmCfm_SecEnblCfg_t)value;
    /* BEGIN: Modified by y67514, 2009/2/7   问题单号:GLB WEB:portfowding*/
    fprintf(file, "<%s fw_enable=\"%d\" inflt_enable=\"%d\" outflt_enable=\"%d\" macflt_enable=\"%d\" macmngr_enable=\"%d\" nat_enable=\"%d\"/>\n", 
        name, pSecEnbl->fw_enable, pSecEnbl->inflt_enable, pSecEnbl->outflt_enable, pSecEnbl->macflt_enable, pSecEnbl->macmngr_enable,pSecEnbl->nat_enable);
    /* END:   Modified by y67514, 2009/2/7 */
}

PSX_STATUS xmlGetSecEnblAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

    PBcmCfm_SecEnblCfg_t pSecEnbl = (PBcmCfm_SecEnblCfg_t)psiValue;

    if(NULL == attrName || NULL == attrValue)
    {
		xmlMemProblem();
		return PSX_STS_ERR_FATAL;   
    }

    if (strncmp(attrName,"fw_enable",strlen("fw_enable")) == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->fw_enable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl fw_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl fw_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    else if (strncmp(attrName,"inflt_enable",strlen("inflt_enable")) == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->inflt_enable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl inflt_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl inflt_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    else if (strncmp(attrName,"outflt_enable",strlen("outflt_enable")) == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->outflt_enable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl outflt_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl outflt_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    else if (strncmp(attrName,"macflt_enable",strlen("macflt_enable")) == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->macflt_enable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl macflt_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl macflt_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    else if (strncmp(attrName,"macmngr_enable",strlen("macmngr_enable")) == 0) {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->macmngr_enable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl macmgnr_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl macmgnr_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    /* BEGIN: Added by y67514, 2009/2/7   PN:GLB WEB:portfowding*/
    else if (strncmp(attrName,"nat_enable",strlen("nat_enable")) == 0) 
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }

        if (status == DB_OBJ_VALID_OK)
        {
            pSecEnbl->nat_enable= atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetSecEnblAttr(): invalid SecEnbl nat_enable %s\n",attrValue);
            printf("xmlGetSecEnblAttr(): invalid SecEnbl nat_enable %s\n",attrValue);
            return PSX_STS_ERR_FATAL; 
        }
    }
    /* END:   Added by y67514, 2009/2/7 */
	return PSX_STS_OK;
}

PSX_STATUS xmlStartSecEnblObj(char *appName, char *objName)
{
    psiValue = (void*)malloc(sizeof(BcmCfm_SecEnblCfg_t));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }

    memset((char*)psiValue, 0, sizeof(BcmCfm_SecEnblCfg_t));

    return PSX_STS_OK;
}

PSX_STATUS xmlEndSecEnblObj(char *appName, char *objName)
{

    if (!xmlState.verify) 
    {
        BcmDb_setSecEnblCfg((PBcmCfm_SecEnblCfg_t)psiValue);
    }
   free(psiValue);
   psiValue = NULL;
   return PSX_STS_OK;
}

/*add by z67625 将过滤开关值写入配置文件 end*/

/*start of 增加dhcp主机标识功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
PSX_STATUS xmlStartDhcpHost12Node(char *appName, char *objName)
{
	//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);

    psiValue = (void*)malloc(DHCP_HOSTID_LENGTH);
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset((char*)psiValue, 0, DHCP_HOSTID_LENGTH);

    return PSX_STS_OK;
}

PSX_STATUS xmlSetDhcpHostNameNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length)
{
    
	//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
	
    if ((NULL == appName) || (NULL == value))
    {
    
        return PSX_STS_ERR_GENERAL;
    }
    

    fprintf(file, "<Entry option12=\"%s\"/>\n", value);
               
    return PSX_STS_OK;
}

PSX_STATUS xmlGetDhcpHostNameNode(char *appName, char *objName, 
                char *attrName, char* attrValue)
{
     DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;

//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);

     printf("xmlGetDhcpHostNameNode(appName %s, objName %s, attrName %s, attrValue %s\n",
     appName,objName,attrName,attrValue);

	 
     if (NULL == psiValue)
     {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetDhcpHostNameNode(): psiValue is NULL\n");
		
//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
		return PSX_STS_ERR_FATAL;
		
     }
    if (0 == strncmp(attrName, "option12", strlen("option12"))) 
    {
        if (xmlState.verify) 
        {
            status = BcmDb_validateLength(attrValue, DHCP_HOSTID_LENGTH);
			
//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
        }
        if (status != DB_OBJ_VALID_OK) 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
			
//printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
            syslog(LOG_ERR,"xmlGetDhcpHostNameNode(): invalid option12 %s\n", attrValue);
        }
        else
        {
//printf("%s %s %d  attrValue %s\n",__FILE__,__FUNCTION__,__LINE__,attrValue);
			 
            strcpy((char*)psiValue, attrValue);
			
//printf("%s %s %d  attrValue %s\n",__FILE__,__FUNCTION__,__LINE__,attrValue);
        }
     }
	 return PSX_STS_OK;	
}

PSX_STATUS xmlEndDhcpHostNameNode(char *appName, char *objName)
{

// printf("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);

 if (xmlState.verify) 
    {
        if(NULL == psiValue)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndDhcpHostNameNode(): psiValue is NULL\n");
        }
    }
    else 
    {
        BcmDb_setDhcpHostNameInfo((char*)psiValue);
    }
   free(psiValue);
   psiValue = NULL;
   return PSX_STS_OK;
}
#endif
/*end of 增加dhcp主机标识功能by l129990 */

/*GLB:s53329,start add for DNS*/
PSX_STATUS xmlDnsEntryObj(char *appName, char *objName)
{
    xmlState.tableIndex++;
    return(PSX_STS_OK);
}
PSX_STATUS xmlGetDnsEntryAttr(char *attrName, char* attrValue)
{
    int i = xmlState.tableIndex;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    PDNS_CFG dnsTbl = (PDNS_CFG)psiValue;
    printf("FIEL is  %s, LINE is  %d  attrName is  %s, attrValue  is  %s\n", __FILE__, __LINE__, attrName, attrValue);
    if ( psiValue == NULL ) {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetDnsEntryAttr(): psiValue is NULL\n");
        return PSX_STS_ERR_FATAL;
    }
    
    if (xmlState.verify) {
        if ( i >= xmlState.tableSize ) {
          xmlState.errorFlag = PSX_STS_ERR_FATAL;
          syslog(LOG_ERR,"xmlGetDnsEntryAttr(): invalid entry index %d, tableSize %d\n",
                 i, xmlState.tableSize);
          /* start of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
          return PSX_STS_ERR_FATAL;
          /* end of maintain PSI：配置文件错误（记录数大于在tableSize中声明的值导致http进程挂死） by xujunxia 43813 2006年5月22日 */
        }
    }

#ifdef XML_DEBUG
    printf("xmlGetDnsEntryAttr(attrName %s, attrValue %s)\n",
    attrName,attrValue);
#endif
  if (strcmp(attrName,"dnsdomain") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateLength(attrValue, IFC_SMALL_LEN+1);
    if (status == DB_OBJ_VALID_OK)
      strcpy(dnsTbl[i].cDnsDomain, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid interface name %s\n",attrValue);
    }
  }
  else if (strcmp(attrName,"macaddr") == 0) {
    if (xmlState.verify)
      status = BcmDb_validateMacAddress(attrValue);
    if (status == DB_OBJ_VALID_OK)
      strcpy(dnsTbl[i].cMac, attrValue);
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetRipIfcEntryAttr(): invalid state %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}
void xmlSetDnsEntryAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length)
{
   PDNS_CFG pDns, ptr;
   int ifCount, i;
   int entry, num;

   if ((name == NULL) || (value == NULL))
      return;

if (strcmp(name,PSI_OBJ_DNS_CFG) == 0) {
     ifCount = *(UINT16*)value;
     if ( ifCount != 0 )
       fprintf(file,"<%s tableSize=\"%d\">\n",name,*(UINT16*)value);
   }
   else {
     pDns = (PDNS_CFG)value;
     /* determine the size of route table */
     //num = length/sizeof(PORT_MAP_CFG_ENTRY);
     num = xmlGetTableSize(file);

     for (entry = 0; entry < num; entry++) {
       /* each entry is one object, even though PSI is not that way */
       ptr = &pDns[entry];
        fprintf(file,"<%s id=\"%d\" dnsdomain=\"%s\" macaddr=\"%s\"/>\n",
	     name, (entry+1),ptr->cDnsDomain, ptr->cMac);

     }
     /* end of table */
     if ( num != 0 )
       fprintf(file,"</%s>\n",dnsObjs[id-1].objName); 
   } /* pmap table */
   
}
PSX_STATUS xmlGetDnsInfoAttr(char *attrName, char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    printf("xmlGetDnsInfoAttr(attrName %s, attrValue %s)\n",attrName,attrValue);


  if (strncmp(attrName,"tableSize",strlen("tableSize")) == 0) {
    if (xmlState.verify)
      status = BcmDb_validateRange(attrValue, 0, SEC_VIR_SRV_MAX);
    if (status == DB_OBJ_VALID_OK) {
      xmlState.tableIndex = 0;
      xmlState.tableSize = atoi(attrValue);
      //lint -save -e10
      psiValue = (void *) malloc(sizeof(DNS_CFG) * xmlState.tableSize);
      //lint -restore
      /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
      if (psiValue == NULL) {
          xmlMemProblem();
          return PSX_STS_ERR_FATAL;
      }
      memset(psiValue, 0, sizeof(DNS_CFG) * xmlState.tableSize);
      /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
    }  
    else {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlGetDnsInfoAttr(): invalid number %s\n",attrValue);
    }
  }
  return(PSX_STS_OK);
}



PSX_STATUS xmlEndDnsInfoObj(char *appName, char *objName)
{
  int error = 0;
  
///#ifdef XML_DEBUG
  printf("xmlEndDnsInfoObj(calling cfm to get appName %s, objName %s\n",
	 appName,objName);
//#endif

  /* check to see if all mandatory object attribute are set */  
  if (xmlState.verify) {
    if (xmlState.tableSize != xmlState.tableIndex) {
      xmlState.errorFlag = PSX_STS_ERR_FATAL;
      syslog(LOG_ERR,"xmlEndDnsInfoObj(): table size does not match with number of entries.\n");
      error = 1;
    }
  }
    if (!error && !xmlState.verify)
    {
        /*顺序不能反*/
        BcmDb_setDnsdomainSize(xmlState.tableSize);
        BcmDb_setDnsDomainInfo((PDNS_CFG)psiValue, xmlState.tableSize*(sizeof(DNS_CFG)));
    }

//#ifdef XML_DEBUG1
  printf("\n============End Static Route=========\n");
  printf("xmlEndDnsInfoObj(): table size = %d\n", xmlState.tableSize);
  printf("===============================================\n");
//#endif
  free(psiValue);
  psiValue = NULL;
  xmlState.tableSize = 0;
  xmlState.tableIndex = 0;
  return PSX_STS_OK;
}
/*GLB:s53329,end add for DNS*/
 /*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#ifdef SUPPORT_IGMPCONFIG
PSX_STATUS xmlStartIgmpProxyNode(char *appName, char *objName)
{
    if ((NULL == appName) || (NULL == objName))
    {
        return PSX_STS_ERR_GENERAL;
    }
     
    psiValue = (void*)malloc(sizeof(IgmpProxyNode));
    if (NULL == psiValue) 
    {
        xmlMemProblem();
        return PSX_STS_ERR_FATAL;
    }
    memset((char*)psiValue, 0, sizeof(IgmpProxyNode));
    return PSX_STS_OK;
}
PSX_STATUS xmlSetIgmpProxytNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length)
{
    
	PIgmpProxyNode pstIgmpProxyNode;
     
    if ((NULL == appName) || (NULL == value))
    {
        return PSX_STS_ERR_GENERAL;
    }
    
     pstIgmpProxyNode = (PIgmpProxyNode)value;

     fprintf(file, "<proxy_node igmpEnable=\"%d\" interface=\"%s\" version=\"%d\" groups=\"%d\" logSwitch=\"%d\"/>\n", 
         pstIgmpProxyNode->igmpEnable, pstIgmpProxyNode->igmpInterface, pstIgmpProxyNode->igmpVersion, pstIgmpProxyNode->igmpGroups, pstIgmpProxyNode->igmpLogSwitch);

    return PSX_STS_OK;
}
PSX_STATUS xmlGetIgmpProxyNode(char *appName, char *objName, char *attrName,
                         char* attrValue)
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_OK;
    PIgmpProxyNode pstIgmpProxyNode;
    
	if((appName == NULL) || (attrName == NULL))
	{
		return PSX_STS_ERR_FATAL;
	}
  
	if (NULL == psiValue)
    {
        xmlState.errorFlag = PSX_STS_ERR_FATAL;
        syslog(LOG_ERR,"xmlGetIgmpProxyNode(): psiValue is NULL\n");
		return PSX_STS_ERR_FATAL;
    }
	
    pstIgmpProxyNode = (PIgmpProxyNode)psiValue;
 
	if(pstIgmpProxyNode == NULL)
	{
		printf("xmlGetIgmpProxyNode: pstIgmpProxyNode==NULL\n");
        return PSX_STS_ERR_FATAL;
	}
	
#ifdef XML_DEBUG
  	printf("xmlGetIgmpProxyNode(attrName %s, attrValue %s)\n", attrName,attrValue);
#endif
    if (strncmp(attrName, "igmpEnable", strlen("igmpEnable")) == 0) 
    {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        if (status == DB_OBJ_VALID_OK)
        {
            pstIgmpProxyNode->igmpEnable = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIgmpProxyNode(): invalid igmpEnable %s\n",attrValue);
        }
    }
    else if (strncmp(attrName, "interface", strlen("interface")) == 0) 
  	{
        if (xmlState.verify) 
        {
           status = BcmDb_validateLength(attrValue, IFC_TINY_LEN); 
        }
        if (status == DB_OBJ_VALID_OK) 
        {
            strcpy(pstIgmpProxyNode->igmpInterface, attrValue);
        }
        else
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIgmpProxyNode(): invalid igmpInterface %s\n", attrValue); 
        }  
  	}
    else if (strncmp(attrName, "version", strlen("version")) == 0) 
  	{
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        if (status == DB_OBJ_VALID_OK)
        {
            pstIgmpProxyNode->igmpVersion = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIgmpProxyNode(): invalid igmpVersion %s\n",attrValue);
        }
    }
	else if (strncmp(attrName,"groups", strlen("groups")) == 0) 
	{
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        if (status == DB_OBJ_VALID_OK)
        {
           pstIgmpProxyNode->igmpGroups = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIgmpProxyNode(): invalid igmpGroups  %s\n",attrValue);
        }
        
   }
   else if (strncmp(attrName,"logSwitch", strlen("logSwitch")) == 0) 
   {
        if (xmlState.verify)
        {
            status = BcmDb_validateNumber(attrValue);
        }
        if (status == DB_OBJ_VALID_OK)
        {
           pstIgmpProxyNode->igmpLogSwitch = atoi(attrValue);
        }
        else 
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlGetIgmpProxyNode(): invalid igmpLogSwitch  %s\n",attrValue);  
        }
        
   }
	return PSX_STS_OK;
}

PSX_STATUS xmlEndIgmpProxyNode(char *appName, char *objName)
{
    PIgmpProxyNode pstIgmpProxyNode;
    pstIgmpProxyNode = (PIgmpProxyNode)psiValue;
    

    printf("xmlEndIgmpProxyNode(calling cfm to get appName %s, objName %s)\n",
	 appName,objName);


    if (xmlState.verify) 
    {
        if(NULL == pstIgmpProxyNode)
        {
            xmlState.errorFlag = PSX_STS_ERR_FATAL;
            syslog(LOG_ERR,"xmlEndIgmpProxyNode(): psiValue is NULL\n");
        }
    }
    else 
    {
        BcmDb_setIgmpProxyInfo(pstIgmpProxyNode);
    }

  free(psiValue);
  psiValue = NULL;
  return PSX_STS_OK;
}   
#endif
/*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
/*add by z67625 for lint start*/
#if defined(__cplusplus)
}
#endif
/*add by z67625 for lint end*/
