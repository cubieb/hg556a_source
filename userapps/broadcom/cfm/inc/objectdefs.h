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
//  Filename:       objectdefs.h
//  Author:         Peter Tran
//  Creation Date:  03/14/05
//
******************************************************************************
//  Description:
//      Define the global types, enums. and constants for PSI in object format
//
*****************************************************************************/

#ifndef __OBJECT_DEFS_H__
#define __OBJECT_DEFS_H__
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

/* important limitation: when adding the appName or objectName in either direction, it is 
important not to have appName and objName having the same prefix.   The table entry search
only compare strings up to the prefix length, case not sensitive, to cover situation
such as APP_ID wan_vpi_vci, only wan is searched in the table.    Therefore, it is not
okay to have Lan as the appId, and objectName as lanSomethingSomething.    The search
entry would think lanSomethingSomething is an appId name. */

#define APPNAME_WAN_PREFIX           "wan"
#define APPNAME_IPSRV_PREFIX         "ipsrv"
#define APPNAME_PPPSRV_PREFIX        "pppsrv"
#define APPNAME_PPP_FORMAT           "pppsrv_%d_%d"
#define APPNAME_IPSRV_ENTRY_FORMAT   "ipsrv_%d_%d"
#define OBJNAME_LAN_ENTRY_FORMAT     "entry%d"   /* cannot be lan, need to change */
#define OBJNAME_PPP_ENTRY_FORMAT     "ppp_conId%d"
#define OBJNAME_IPSRV_ENTRY_FORMAT   "dhcpc_conId%d"
#define OBJNAME_WAN_ENTRY_FORMAT     "entry%d"   /* cannot be wan, need to change */
#define OBJNAME_GEN_ENTRY_FORMAT     "entry" 

/* start of 增加tr143 功能by 2009.11.7*/
#define  TR143_APPID   "TR143Cfg"

#define DownloadDiagnostics_ID 	1
#define UploadDiagnostics_ID 	2
#define UDPEchoConfig_ID 	3
/* end of 增加tr143 功能by 2009.11.7*/


/* start of maintain PSI 移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端by xujunxia 43813 2006年5月5日 */
#define APPNAME_BRSRV_PREFIX         "brsrv"
#define APPNAME_BRSRV_ENTRY_FORMAT   "brsrv_%d_%d"
#define OBJNAME_BRSRV_ENTRY_FORMAT   "br_conId%d"
/* end of maintain PSI 移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端by xujunxia 43813 2006年5月5日 */
/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
#define APPNAME_AUTOUPGRADE         "AutoUpgrade"
#define AUTOUPGRADE_CONFIG            1
/* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
#ifdef SUPPORT_DHCPOPTIONS
#define DHCPOption        "DHCPOption"
#define DHCPOption_CONFIG            1
#endif
/*start of 增加dhcp主机标识功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
#define DHCPHOSTID        "dhcphostname"
#define DHCP_CONFIG     1 
#endif
/*end of 增加dhcp主机标识功能by l129990 */ 
/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#define IGMPPROXY        "igmpproxy"
#define IGMPPROXY_CONFIG     1 

/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
/*end of 增加dhcp主机标识功能by l129990 */ 
/*start  of  增加Raduis 认证功能 by  s53329  at   20070714*/
#ifdef  SUPPORT_RADIUS
#define   PSX_OBJ_RADIUS              "RadiusServer"
#define RADIUS_CONFIG                              1
#endif
/*end  of  增加Raduis 认证功能 by  s53329  at   20070714*/
#define OBJNAME_DMZ_HOST             "dmzHost"
#define OBJNAME_SRV_CTRL_LIST        "srvCtrlList"
/* start of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#if SUPPORT_KPN_QOS     // macro QoS for KPN
#define OBJNAME_WRED_INFO            "WredInfo"
#endif
#endif
/* end of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#define TBLNAME_VRT_SRV              "vrtSrv"
#define ENTRYNAME_VRT_SRV            "vrtSrvEntry"
#define TBLNAME_OUT_FILTER           "outFilter"
#define ENTRYNAME_OUT_FILTER         "outFilterEntry"
#define TBLNAME_IN_FILTER            "inFilter"
#define ENTRYNAME_IN_FILTER          "inFilterEntry"
#define TBLNAME_PRT_TRIGGER          "prtTrigger"
#define ENTRYNAME_PRT_TRIGGER        "prtTriggerEntry"
#define TBLNAME_MAC_FILTER           "macFilter"
#define ENTRYNAME_MAC_FILTER         "macFilterEntry"
#define TBLNAME_QOS_CLS              "qosCls"
#define ENTRYNAME_QOS_CLS            "qosClsEntry"
/*start add for vdf qos by p44980 2008.01.08*/
#ifdef SUPPORT_VDF_QOS
#define TBLNAME_QOS_QUEUE         "qosQueue"
#define ENTRYNAME_QOS_QUEUE         "qosQueueEntry"
#define OBJNAME_QOS_CFG              "qosCfg"
#define TBLNAME_QOS_SERVICE       "qosService"
#define ENTRYNAME_QOS_SERVICE   "qosServiceEntry"
#endif
/*end add for vdf qos by p44980 2008.01.08*/
#define TBLNAME_ACC_CTRL             "accCtrl"
#define ENTRYNAME_ACC_CTRL           "accCtrlEntry"
#define OBJNAME_ACC_CTRL_MODE        "accCtrlMode"
#define TBLNAME_ROUTE                "route"
#define ENTRYNAME_ROUTE              "routeEntry"
#define TBLNAME_RIP_IFC              "ripIfc"
#define ENTRYNAME_RIP_IFC            "ripIfcEntry"
#define OBJNAME_RIP_GLOBAL           "ripGlobal"
/* j00100803 Add Begin 2008-02-28 */
#ifdef SUPPORT_POLICY
#define TBLNAME_POLICY              "policyrouting"
#define ENTRYNAME_POLICY            "policy"
#endif
/*j00100803 Add End 2008-02-28*/
#if SUPPORT_PORT_MAP
#define TBLNAME_PMAP                 "pmap"
#define ENTRYNAME_PMAP               "pmapEntry"
#define OBJNAME_PMAP_IFC_CFG         "pmapIfcCfg"
#endif
#ifdef VDF_PPPOU

//add by l66195 for pppousb start
#define TBLNAME_PPPPROFILE           "pppPro"
#define ENTRYNAME_PPPPROFILE         "pppProEntry"
//add by l66195 for pppousb end

#endif
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
#define TBLNAME_VLAN               "vlan"
#define TBLNAME_VLTB               "vlTable"
#define TBLNAME_VLPORT             "vlPort"
#define ENTRYNAME_VLTB             "vlanEntry"
#define ENTRYNAME_VLPORT           "vlPortEntry"
/*end vlan bridge, s60000658, 20060627*/
//#endif
#ifdef SUPPORT_DDNSD
#define TBLNAME_DDNS                 "ddns"
#define ENTRYNAME_DDNS               "ddnsEntry"
#endif
#ifdef WIRELESS
#define TBLNAME_WL_MSSID_VARS        "wlMssidVars"
#define TBLNAME_WL_MAC_FILTER        "wlMacFilter"
#define TBLNAME_WDS_FILTER           "wdsFilter"
#define TBLNAME_SCAN_WDS_FILTER      "scanWdsFilter"
#define ENTRYNAME_WL_MSSID_VARS      "wlMssidEntry"
#define ENTRYNAME_WL_MAC_FILTER      "wlMacEntry"
#define ENTRYNAME_WDS_FILTER         "wdsEntry"
#define ENTRYNAME_SCAN_WDS_FILTER    "scanWdsEntry"
#endif 
#ifdef SUPPORT_TOD
#define TBLNAME_PARENT_CNTRL         "parentCntrl"
#define ENTRYNAME_PARENT_CNTRL       "parentCntrlTbl"
#endif
#ifdef SUPPORT_MAC//add by l66195 for VDF
#define TBLNAME_MAC         "mac"
#define ENTRYNAME_MAC       "macTbl"
#endif
#ifdef SUPPORT_MACMATCHIP
#define TBLNAME_MACMATCH         "macmatch"
#define ENTRYNAME_MACMATCH       "macmatchTbl"
#endif
#ifdef SUPPORT_IPSEC
#define TBLNAME_IPSEC                "ipsconn"
#define ENTRYNAME_IPSEC              "ipsconnEntry"
#endif
#ifdef SUPPORT_CERT
#define TBLNAME_CERT_LOCAL           "certLocal"
#define ENTRYNAME_CERT_LOCAL         "certLocalEntry"
#define TBLNAME_CERT_CA              "certCA"
#define ENTRYNAME_CERT_CA            "certCAEntry"
#endif

#ifdef PORT_MIRRORING
#define TBLNAME_PORT_MIRROR_CFG      "portMirrorCfg"
#define ENTRYNAME_PORT_MIRROR_CFG    "portMirrorCfgEntry"
#define OBJNAME_PORT_MIRROR_STATUS   "portMirrorStatus"
#endif

/* start of voice 增加SIP相关的配置项 by z45221 zhangchen 2006年11月7日 */
#define TBLNAME_SIP_SRV            "sipServer"
#define ENTRYNAME_SIP_SRV          "sipServerEntry"
#define TBLNAME_SIP_BSC            "sipBasic"
#define ENTRYNAME_SIP_BSC          "sipBasicEntry"
#define OBJNAME_SIP_ADV            "sipAdvanced"
#define OBJNAME_SIP_QOS            "sipQos"
/* end of voice 增加SIP相关的配置项 by z45221 zhangchen 2006年11月7日 */
#ifdef SUPPORT_SAMBA
#define TBLNAME_SAMBA_ACCOUNT        "sambaAccnt"
#define ENTRYNAME_SAMBA_ACCOUNT      "sambaAccntEntry"
#define OBJNAME_SAMBA_STATUS         "sambaStatus"
#endif

/* system info */
#define PSX_OBJ_SYS_USER_NAME      "sysUserName"
#define PSX_OBJ_SYS_PASSWORD       "sysPassword"
#ifndef SUPPORT_ONE_USER
#define PSX_OBJ_SPT_USER_NAME      "sptUserName"
#define PSX_OBJ_SPT_PASSWORD       "sptPassword"
#define PSX_OBJ_USR_USER_NAME      "usrUserName"
#define PSX_OBJ_USR_PASSWORD       "usrPassword"
#endif

#define PSX_OBJ_FTP_INFO           "ftpinfo"
#define PSX_OBJ_CAMERA_INFO        "camerainfo"

#define PSX_OBJ_HTTP_PORT              "sysHttp"

/*GLB:s53329,start add for DNS*/
#define PSI_APP_DNS                          "dnscfgInfo"
#define PSI_OBJ_DNS_CFG                  "dnscfg"
#define PSI_OBJ_DNS_ENTRY               "dnscfgEntry"
/*GLB:s53329,end add for DNS*/
#ifdef SUPPORT_TR69C
#define PSX_OBJ_TR69C                       "tr69c"
#endif
#define PSX_OBJ_MGMT_LOCK          "mgmtLock"
#define PSX_OBJ_SOFTWARE_VERSION   "swVersion"
#define PSX_OBJ_HARDWARE_VERSION   "hwVersion"
#define PSX_OBJ_SYSLOG_DATA        "sysLogData"
#define PSX_OBJ_DEF_GATEWAY        "defaultGateway"
#define PSX_OBJ_DNS                "dns"
#define PSX_OBJ_SYSLOG             "sysLog"
//BEGIN:add by zhourongfei to config web at 2011/05/31
#define PSX_OBJ_WEB           "web"
//END:add by zhourongfei to config web at 2011/05/31

 /*Begin:Added by luokunling l192527 for IMSI inform,2011/9/23*/
#define PSX_OBJ_IMSIINFORM  "Imsinotification"
/*End:Added by luokunling l00192527,2011/9/22*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
#define PSX_OBJ_HTTPINTERCEPTVISIBLE "HTTPInterceptVisible"
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
#define PSX_OBJ_SNMP               "snmp"
#define PSX_OBJ_PROTOCOL           "protocol"
#define PSX_OBJ_ETHCFG             "ethCfg"
#define PSX_OBJ_MGCP_CFG           "mgcpCfg"
#define PSX_OBJ_SIP_CFG            "sipCfg"
#define OBJNAME_SIP_PHONECFG       "sip_phonecfg"
#define TBLNAME_SIP_SPEEDDIAL_TB  "sip_speeddial_tb"
#define ENTRYNAME_SIP_SPEEDDIAL   "sip_speeddial"
#define TBLNAME_SIP_CALLFWD_TB    "sip_callfwd_tb"
#define ENTRYNAME_SIP_CALLFWD     "sip_callfwd"

#define OBJNAME_SEC_FW             "secFwCfg"
#define TBLNAME_SEC_FW             "secFwTb"
#define ENTRYNAME_SEC_FW           "secFwEntry"
#define OBJNAME_SEC_DOS            "secDosCfg"
#define TBLNAME_SEC_URLBLK         "secUrlBlkTb"
#define ENTRYNAME_SEC_URLBLK       "secUrlBlkEntry"
/*add by z67625 过滤使能开关宏定义 start*/
#define OBJNAME_SEC_ENABLE         "secEnblCfg"
/*add by z67625 过滤使能开关宏定义 end*/
//#define TBLNAME_SEC_DOS            "secDos"

/*Begin: Add para of ALG Sip Proxy Port, by d37981, 2006.12.15*/
#ifdef SUPPORT_SIPROXD 
#define PSX_OBJ_SIPROXD_CFG            "siproxd"
#endif
/*Endof: Add para of ALG Sip Proxy Port, by d37981, 2006.12.15*/

#ifdef SUPPORT_GLB_ALG
#define PSX_OBJ_NATALG_CFG            "natAlg"
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
#define PSX_OBJ_AUTOSCAN_CFG            "autoScan"
/* END:   Added by y67514, 2008/10/27 */

/* BEGIN: Added by hKF20513, 2010/1/29   PN:增加Upnp Video控制*/
#define PSX_OBJ_UPNPV            "upnpvideo"
/* END:   Added by hKF20513, 2010/1/29 */
#define PSX_ATTR_DNS_PRIMARY       "primary"
#define PSX_ATTR_DNS_SECONDARY     "secondary"
#define PSX_ATTR_DNS_DYNAMIC       "dynamic"
#define PSX_ATTR_DNS_DOMAIN        "domain"
/* start: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
#define PSX_ATTR_DNS_PROXY_ENABLE        "enbdnsproxy"
/* end: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */

#define PSX_ATTR_SYSLOG_DISPLAY_LEVEL "displayLevel"
#define PSX_ATTR_SYSLOG_LOG_LEVEL     "logLevel"
#define PSX_ATTR_SYSLOG_OPTION        "option"
#define PSX_ATTR_SYSLOG_SERVER_IP     "serverIP"
#define PSX_ATTR_SYSLOG_SERVER_PORT   "serverPort"
/*add by z67625 防火墙日志记录写flash开关宏定义 start*/
#define PSX_ATTR_SYSLOG_FWLOG         "fwlog"
/*add by z67625 防火墙日志记录写flash开关宏定义 start*/

/*start of 增加系统日志保存FLASH开关宏定义by c00131380 at 080926*/
#define PSX_ATTR_SYSLOG_SAVE          "Syslog"        
/*end of 增加系统日志保存FLASH开关宏定义by c00131380 at 080926*/

#define PSX_ATTR_PMAP_IFC_NAME        "pmapIfName"
#define PSX_ATTR_PMAP_IFC_STATUS      "pmapIfcStatus"

#define PSX_ATTR_VOICE_MGCP_TAG            "mgcpCfg"
#define PSX_ATTR_VOICE_MGCP_CA_ADDR        "caAddr"
#define PSX_ATTR_VOICE_MGCP_GW_NAME        "gwName"
#define PSX_ATTR_VOICE_MGCP_IF_NAME        "ifName"
#define PSX_ATTR_VOICE_MGCP_PREF_CODEC     "prefCodec"
#define PSX_ATTR_VOICE_MGCP_AALN           "aaln"
#define PSX_ATTR_VOICE_SIP_TAG             "sipCfg"
#define PSX_ATTR_VOICE_SIP_PROXY_ADDR      "proxyAddr"
#define PSX_ATTR_VOICE_SIP_PROXY_PORT      "proxyPort"
#define PSX_ATTR_VOICE_SIP_OBPROXY_ADDR    "obProxyAddr"
#define PSX_ATTR_VOICE_SIP_OBPROXY_PORT    "obProxyPort"
#define PSX_ATTR_VOICE_SIP_REGISTRAR_ADDR  "regAddr"
#define PSX_ATTR_VOICE_SIP_REGISTRAR_PORT  "regPort"
#define PSX_ATTR_VOICE_SIP_TAG_MATCHING    "enblTagMat"
#define PSX_ATTR_VOICE_SIP_LOG_ADDR        "sipLogAddr"
#define PSX_ATTR_VOICE_SIP_LOG_PORT        "sipLogPort"
#define PSX_ATTR_VOICE_SIP_EXT1            "extension1"
#define PSX_ATTR_VOICE_SIP_PASSWORD1       "password1"
#define PSX_ATTR_VOICE_SIP_EXT2            "extension2"
#define PSX_ATTR_VOICE_SIP_PASSWORD2       "password2"
#define PSX_ATTR_VOICE_SIP_EXT3            "extension3"
#define PSX_ATTR_VOICE_SIP_PASSWORD3       "password3"
#define PSX_ATTR_VOICE_SIP_EXT4            "extension4"
#define PSX_ATTR_VOICE_SIP_PASSWORD4       "password4"
#define PSX_ATTR_VOICE_SIP_IF_NAME         "ifName"
#define PSX_ATTR_VOICE_SIP_DOMAIN_NAME     "domainName"
#define PSX_ATTR_VOICE_LOCALE_NAME         "localeName"
#define PSX_ATTR_VOICE_CODEC_NAME          "voiceCodec"
#define PSX_ATTR_VOICE_PTIME               "voicePtime"
#define PSX_ATTR_VOICE_PSTN_ROUTE_RULE     "pstnRouteRule"
#define PSX_ATTR_VOICE_PSTN_ROUTE_DATA     "pstnRouteData"
#define PSX_ATTR_VOICE_STORE               "store" // To indicate a write to flash

/* start of voice SIP业务配置数据 by z45221 zhangchen 2006年11月6日 */
// 1.SIP server related
#define PSX_ATTR_VOICE_SIP_SRV_TAG        "sipServerEntry"
#define PSX_ATTR_VOICE_SIP_REGSRV_IP      "sipRegIP"
#define PSX_ATTR_VOICE_SIP_REGSRV_PORT    "sipRegPort"
#define PSX_ATTR_VOICE_SIP_PRXYSRV_IP     "sipPrxyIP"
#define PSX_ATTR_VOICE_SIP_PRXYSRV_PORT   "sipPrxyPort"
#define PSX_ATTR_VOICE_SIP_DOMAIN         "sipDomain"

// 2.SIP Basic
#define PSX_ATTR_VOICE_SIP_BSC_TAG        "sipBasicEntry"
#define PSX_ATTR_VOICE_SIP_USER_ID        "sipUserId"
#define PSX_ATTR_VOICE_SIP_USER_NAME      "sipUserName"
#define PSX_ATTR_VOICE_SIP_USER_PASSWD    "sipUserPasswd"
#define PSX_ATTR_VOICE_SIP_LOCAL_PORT     "sipLocalPort"
#define PSX_ATTR_VOICE_SIP_FWDTBL_ID      "sipFwdTblId"

/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
#define PSX_ATTR_VOICE_SIP_CLIP_ENABLE    "sipClipEnable"
#define PSX_ATTR_VOICE_SIP_PRIVACY_TYPE   "sipPrivacyType"
#define PSX_ATTR_VOICE_SIP_STRP_PRENUM		  "sipSpNum"
#define PSX_ATTR_VOICE_SIP_STRP_PRENAME	  "sipSpName"
#define PSX_ATTR_VOICE_SIP_APD_PRENUM		  "sipApNum"
#define PSX_ATTR_VOICE_SIP_APD_PRENAME	  	  "sipApName"
/* BEGIN: Modified by p00102297, 2008/1/30 */
#define PSX_ATTR_VOICE_SIP_STRP_SUFNUM		  "sipSsNum"
#define PSX_ATTR_VOICE_SIP_STRP_SUFNAME	  "sipSsName"
#define PSX_ATTR_VOICE_SIP_APD_SUFNUM		  "sipAsNum"
#define PSX_ATTR_VOICE_SIP_APD_SUFNAME	  	  "sipAsName"
/* END:   Modified by p00102297, 2008/1/30 */
/* BEGIN: Modified by p00102297, 2008/2/3 */
#define PSX_ATTR_VOICE_SIP_LINE_ENABLE    "sipLineEnable"
/*start of addition by chenyong 2009-03-19*/
#define PSX_ATTR_VOICE_SIP_LINE_CALLHOLD  "sipLineCallHold"
/*end of addition by chenyong 2009-03-19*/
#define PSX_ATTR_VOICE_SIP_LINE_CALLWAIT  "sipLineCallWait"
/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
#define PSX_ATTR_VOICE_SIP_LINE_3WAY          "sipLine3Way"
#define PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER  "sipLineCallTransfer"
/*END: Added by chenyong 65116 2008-09-15 web配置*/
/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
#define PSX_ATTR_VOICE_SIP_LINE_MWI          "sipLineMwi"
#define PSX_ATTR_VOICE_SIP_LINE_MWIACCOUNT   "sipLineMwiAccount"
/* END:   Added by chenzhigang, 2008/11/4 */
#define PSX_ATTR_VOICE_SIP_FAX_DETECT     "sipLineFaxDetect"
/* END:   Modified by p00102297, 2008/2/3 */
/* BEGIN: Modified by p00102297, 2008/2/20 */
#define PSX_ATTR_VOICE_SIP_VAD            "vad"
//#define PSX_ATTR_VOICE_SIP_REGION         "region"
#define PSX_ATTR_VOICE_SIP_EC             "ec"
#define PSX_ATTR_VOICE_SIP_CNG            "cng"
//#define PSX_ATTR_VOICE_SIP_OFFHOOKTIME    "offhooktime"
//#define PSX_ATTR_VOICE_SIP_ONHOOKTIME     "onhooktime"
//#define PSX_ATTR_VOICE_SIP_MINHOOKFLASH   "minHookFlash"
//#define PSX_ATTR_VOICE_SIP_MAXHOOKFLASH   "maxHookFlash"
/* END:   Modified by p00102297, 2008/2/20 */

#define PSX_SIP_STRPAPD_BUFSIZE  5*1024
#endif  

// 3. SIP Advanced
#define PSX_ATTR_VOICE_SIP_ADV_TAG        "sipAdvanced"
// SIP Server Advanced
/* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
#define PSX_ATTR_VOICE_SIP_SRV_REGPERIOD     "sipSrvRegPeriod"            // Expiration
/* END:   Added by y67514, 2009/11/4 */
#define PSX_ATTR_VOICE_SIP_SRV_EXPIRE     "sipSrvExpire"            // Expiration
#define PSX_ATTR_VOICE_SIP_SRV_REREG      "sipSrvRegister"          // ReRegister
#define PSX_ATTR_VOICE_SIP_SRV_SEEXP      "sipSrvSessionExpires"    // SessionExpires
#define PSX_ATTR_VOICE_SIP_SRV_MINSE      "sipSrvMinSe"             // MinSE
/* BEGIN: Modified by p00102297, 2008/1/29 */
#define PSX_ATTR_VOICE_SIP_SRV_MAXRETRY   "sipMaxRetryTime"         // MaxRetryTime
#define PSX_ATTR_VOICE_SIP_SRV_PRICHECK   "sipPrimaryCheckTime"     // PrimaryCheckTime
/* END:   Modified by p00102297, 2008/1/29 */
/* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
#define PSX_ATTR_VOICE_SIP_SRV_MWIEXPIRE   "sipMWIExpire" 
/* END:   Added by chenzhigang, 2008/11/5 */
// RTP advanced
#define PSX_ATTR_VOICE_SIP_RTP_STARTPORT  "sipRtpStartPort"
#define PSX_ATTR_VOICE_SIP_RTP_ENDPORT    "sipRtpEndPort"
/* BEGIN: Modified by p00102297, 2008/1/18 */
#define PSX_ATTR_VOICE_SIP_RTP_TXREINTVAL "sipRtpTxReptatInterval"
/* END:   Modified by p00102297, 2008/1/18 */
/*start of modification by chenyong 2009-03-05*/
#define PSX_ATTR_VOICE_SIP_RTCP_MONITOR   "sipRtcpMonitor"
/*end of modification by chenyong 2009-03-05*/

// Voice advanced
/*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
#define PSX_ATTR_VOICE_SIP_VOICE_CODEC1     "sipVoiceCapCodec1"
#define PSX_ATTR_VOICE_SIP_VOICE_CODEC2     "sipVoiceCapCodec2"
#define PSX_ATTR_VOICE_SIP_VOICE_CODEC3     "sipVoiceCapCodec3"
#define PSX_ATTR_VOICE_SIP_VOICE_CODEC4     "sipVoiceCapCodec4"
#define PSX_ATTR_VOICE_SIP_VOICE_CODEC5     "sipVoiceCapCodec5"

#define PSX_ATTR_VOICE_SIP_VOICE_PT1        "sipVoiceCapPTime1"
#define PSX_ATTR_VOICE_SIP_VOICE_PT2        "sipVoiceCapPTime2"
#define PSX_ATTR_VOICE_SIP_VOICE_PT3        "sipVoiceCapPTime3"
#define PSX_ATTR_VOICE_SIP_VOICE_PT4        "sipVoiceCapPTime4"
#define PSX_ATTR_VOICE_SIP_VOICE_PT5        "sipVoiceCapPTime5"

#define PSX_ATTR_VOICE_SIP_VOICE_PRIORITY1  "sipVoiceCapPriority1"
#define PSX_ATTR_VOICE_SIP_VOICE_PRIORITY2  "sipVoiceCapPriority2"
#define PSX_ATTR_VOICE_SIP_VOICE_PRIORITY3  "sipVoiceCapPriority3"
#define PSX_ATTR_VOICE_SIP_VOICE_PRIORITY4  "sipVoiceCapPriority4"
#define PSX_ATTR_VOICE_SIP_VOICE_PRIORITY5  "sipVoiceCapPriority5"

#define PSX_ATTR_VOICE_SIP_VOICE_IFENABLE1  "sipVoiceCapIfEnable1"
#define PSX_ATTR_VOICE_SIP_VOICE_IFENABLE2  "sipVoiceCapIfEnable2"
#define PSX_ATTR_VOICE_SIP_VOICE_IFENABLE3  "sipVoiceCapIfEnable3"
#define PSX_ATTR_VOICE_SIP_VOICE_IFENABLE4  "sipVoiceCapIfEnable4"
#define PSX_ATTR_VOICE_SIP_VOICE_IFENABLE5  "sipVoiceCapIfEnable5"
/*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/


/* END:   Added by s00125931, 2008/9/11 */
#define PSX_ATTR_VOICE_SIP_VOICE_DtmfType "sipVoiceDtmfType"           // DtmfType
//BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
#define PSX_ATTR_VOICE_SIP_VOICE_PayloadType "sipVoicePayloadType"           // payloadType
//END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
// other advanced configuration
#define PSX_ATTR_VOICE_SIP_FAX_TYPE       "sipFaxType"                 // FaxType
#define PSX_ATTR_SIP_INTERFACE            "sipInterface"               // InterfaceName
#define PSX_ATTR_SIP_LOCAL_PORT           "sipAdvLocalPort"            // sipLocalPort
#define PSX_ATTR_PSTN_CALL_PREFIX         "pstnCallPrefix"             //pstn call prefix
#define PSX_ATTR_SIP_TEST_ENBL            "sipTestEnabled"             // Sip Test enabled flag
#define PSX_ATTR_SIP_DSCP_MARK            "dscpmark"	               /* Added by s00125931, 2008/9/16   问题单号:vhg556*/
// 4. SIP QoS
#define PSX_ATTR_VOICE_SIP_QOS_TAG        "sipQos"                     // Sip QoS
#define PSX_ATTR_VOICE_SIP_QOS_TOS        "sipQosTos"
#define PSX_ATTR_VOICE_SIP_QOS_VLAN_ENBL  "sipQosVlanEnbled"
#define PSX_ATTR_VOICE_SIP_QOS_VLANID     "sipQosVlanId"
#define PSX_ATTR_VOICE_SIP_QOS_PRIORITY   "sipQosPriority"
/* end of voice SIP业务配置数据 by z45221 zhangchen 2006年11月6日 */

/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
#define OBJNAME_SIP_PROV            "sipProvisioned"

#define PSX_ATTR_VOICE_SIP_CLIP_ENABLE_PROV    "sipClipEnableProv"
#define PSX_ATTR_VOICE_SIP_LINE_CALLHOLD_PROV  "sipLineCallHoldProv"
#define PSX_ATTR_VOICE_SIP_LINE_CALLWAIT_PROV  "sipLineCallWaitProv"
#define PSX_ATTR_VOICE_SIP_LINE_3WAY_PROV          "sipLine3WayProv"
#define PSX_ATTR_VOICE_SIP_LINE_CALLTRANSFER_PROV  "sipLineCallTransferProv"
#define PSX_ATTR_VOICE_SIP_LINE_MWI_PROV          "sipLineMwiProv"
#define PSX_ATTR_VOICE_SIP_LINE_AUTOCALL_PROV     "sipLineAutoCallProv"
/* END:   Added by y67514, 2009/11/16 */

#ifdef STUN_CLIENT
#define PSX_ATTR_VOICE_STUN_SRV_ADDR       "stunSrvAddr"
#define PSX_ATTR_VOICE_STUN_SRV_PORT       "stunSrvPort"
#endif /* STUN_CLIENT */

#define PSX_ATTR_ETH_CFG_SPEED        "speed"
#define PSX_ATTR_ETH_CFG_TYPE         "type"
#define PSX_ATTR_ETH_CFG_MTU          "MTU"
#define PSX_ATTR_IPP_MAKE             "make"
#define PSX_ATTR_IPP_DEV              "dev"
#define PSX_ATTR_IPP_NAME             "name"

// parameter for OBJ_SYSTEM_ACTION
// get operation
#define OBJ_SYSTEM_PSI_CHANGE_NUM     0
#define OBJ_SYSTEM_LAN_DEVICE_NUM     1
#define OBJ_SYSTEM_LAN_STATISTICS     2
#define OBJ_SYSTEM_WAN_DEVICE_NUM     3
#define OBJ_SYSTEM_WAN_STATISTICS     4

// wan statistic fields
#define STS_FROM_LAN                  0
#define STS_FROM_WAN                  1
#define OBJ_STS_IFC_COLS              8
#define OBJ_STS_RX_BYTE               0
#define OBJ_STS_RX_PACKET             1
#define OBJ_STS_RX_ERR                2
#define OBJ_STS_RX_DROP               3
#define OBJ_STS_TX_BYTE               4
#define OBJ_STS_TX_PACKET             5
#define OBJ_STS_TX_ERR                6
#define OBJ_STS_TX_DROP               7
#define OBJ_STS_ALL                   8

// set operation
#define OBJ_SYSTEM_REBOOT             0
#define OBJ_SYSTEM_SAVE_CONFIG        1
#define OBJ_SYSTEM_SAVE_IMAGE         2
#define OBJ_SYSTEM_RESTORE_DEFAULT    3
#define OBJ_SYSTEM_KILL_APPS          4
// get operation
// variable name
#define OBJ_SYSTEM_ACTION             "systemAction"

// parameters for IFC_LAN_INFO
#define OBJ_LAN_INFO               0
#define OBJ_LAN_IP_ADDRESS         1
#define OBJ_LAN_SUBNET_MASK        2
#define OBJ_DHCP_SRV_ENABLE        3
#define OBJ_DHCP_SRV_LEASE_TIME    4
#define OBJ_DHCP_SRV_START_ADDR    5
#define OBJ_DHCP_SRV_END_ADDR      6
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
#define OBJ_DHCP_RELAY_ENABLE      7
/*end of support to the dhcp relay  function by l129990,2009,11,12*/

// parameter for IFC_ATM_TD_INFO
// get & set operation
#define OBJ_TD_INFO                0
#define OBJ_TD_PEAK_CELL_RATE      1
#define OBJ_TD_SUSTAINED_CELL_RATE 2
#define OBJ_TD_MAX_BURST_SIZE      3
#define OBJ_TD_SERVICE_CATEGORY    4
// get operation
#define OBJ_TD_ID_FROM_INFO        5
#define OBJ_TD_ID_FROM_INDEX       6
#define OBJ_TD_ID_AVAILABLE        7

// parameter for IFC_ATM_VCC_INFO
// get & set operation
#define OBJ_VCC_INFO               0
#define OBJ_VCC_TD_ID              1
#define OBJ_VCC_AAL_TYPE           2
#define OBJ_VCC_ADMIN_STATUS       3
#define OBJ_VCC_ENCAP_MODE         4
#define OBJ_VCC_ENABLE_QOS         5
#define OBJ_VCC_VPI                6
#define OBJ_VCC_VCI                7
#define OBJ_VCC_IFC_ID             8
// get operation
#define OBJ_VCC_ID_FROM_ADDRESS    9
#define OBJ_VCC_ID_AVAILABLE       10
// delete operation
#define OBJ_VCC_DELETE_BY_ID       11
#define OBJ_VCC_DELETE_BY_ADDRESS  12

// parameter for WAN_CON_INFO
// get & set operation
#define OBJ_WAN_CON_INFO           0
#define OBJ_WAN_VCC_ID             1
#define OBJ_WAN_CON_ID             2
#define OBJ_WAN_PROTOCOL           3
#define OBJ_WAN_ENCAP_MODE         4
#define OBJ_WAN_CON_NAME           5
#define OBJ_WAN_FLAG_FIREWALL      6
#define OBJ_WAN_FLAG_NAT           7
#define OBJ_WAN_FLAG_IGMP          8
#define OBJ_WAN_FLAG_SERVICE       9
// get operation
#define OBJ_WAN_ID_AVAILABLE       20

// parameter for WAN_PPP_INFO
// get & set operation
#define OBJ_PPP_INFO               0
#define OBJ_PPP_IDLE_TIMEOUT       1
#define OBJ_PPP_IP_EXTENSION       2
#define OBJ_PPP_AUTH_METHOD        3
#define OBJ_PPP_USER_NAME          4
#define OBJ_PPP_PASSWORD           5
#define OBJ_PPP_SERVICE_NAME       6
#define OBJ_PPP_USE_STATIC_IP_ADDR 7
#define OBJ_PPP_LOCAL_IP_ADDR      8
#define OBJ_PPP_DEBUG              9

// parameter for WAN_IP_INFO
// get & set operation
#define OBJ_IP_INFO                0
#define OBJ_IP_ENABLE_DHCP_CLNT    1
#define OBJ_IP_WAN_IP_ADDR         2
#define OBJ_IP_WAN_SUBNET_MASK     3
/* start of maintain PSI 移植: 增加dhcp option60的描述 by xujunxia 43813 2006年5月5日 */
#define OBJ_IP_DHCP_CLASS_IDENTIFIER  4;
/* end of maintain PSI 移植: 增加dhcp option60的描述 by xujunxia 43813 2006年5月5日 */
    
// parameter for DDNSD
// set operation
#ifdef SUPPORT_DDNSD
#define OBJ_DDNSD_INIT             0
#define OBJ_DDNSD_RESTART          1
#define OBJ_DDNSD_ADD              2
#define OBJ_DDNSD_STORE            3
#define OBJ_DDNSD_CLOSE            4
#endif   // SUPPORT_DDNSD

// parameter for SNTP
// set operation
#ifdef SUPPORT_SNTP
#define OBJ_SNTP_INIT              0
#define OBJ_SNTP_RESTART           1
#define OBJ_STNP_SET_VAR           2
#define OBJ_SNTP_STORE             3
#define OBJ_SNTP_CLOSE             4
// variable name
#define OBJ_SNTP_SERVER_1          "ntpServer1"
#define OBJ_SNTP_SERVER_2          "ntpServer2"
//BEGIN:add by zhourongfei to config 3rd ntpserver
#define OBJ_SNTP_SERVER_3          "ntpServer3"
//END:add by zhourongfei to config 3rd ntpserver
#define OBJ_SNTP_TIMEZONE          "timezone"
#define OBJ_SNTP_ENABLED           "ntp_enabled"
#define OBJ_SNTP_USE_DST           "use_dst"
#define OBJ_SNTP_OFFSET            "timezone_offset"
#define OBJ_SNTP_DST_START         "dst_start"
#define OBJ_SNTP_DST_END           "dst_end"
#define OBJ_SNTP_REFRESH           "sntpRefresh"
#endif   // SUPPORT_SNTP

// parameter for IPP
// set operation
#ifdef SUPPORT_IPP
#define OBJ_IPP_INIT               0
#define OBJ_IPP_RESTART            1
#define OBJ_IPP_SET_VAR            2
#define OBJ_IPP_STORE              3
#define OBJ_IPP_CLOSE              4
// variable name
#define OBJ_IPP_ENABLED            "ipp_enabled"
#define OBJ_IPP_REFRESH            "ippRefresh"
#define OBJ_IPP_MAKE               "ippMake"
#define OBJ_IPP_DEV                "ippDev"
#define OBJ_IPP_NAME               "ippName"
#endif   // SUPPORT_IPP

// parameter for TOD
// set operation
#ifdef SUPPORT_TOD
#define OBJ_TOD_INIT               0
#define OBJ_TOD_ADD                1
#define OBJ_TOD_STORE              2
#endif   // SUPPORT_TOD

#ifdef SUPPORT_MAC // add by l66195 for VDF
#define OBJ_MAC_INIT               0
#define OBJ_MAC_ADD                1
#define OBJ_MAC_STORE              2
#endif   // SUPPORT_MAC

#ifdef SUPPORT_MACMATCHIP 
#define OBJ_MACMATCH_INIT               0
#define OBJ_MACMATCH_ADD                1
#define OBJ_MACMATCH_STORE              2
#endif   // SUPPORT_MAC

/* HSDPA d00104343*/
#define PSX_ATTR_HSDPA_WANIFCONFIG_STS  "vdfWanCfgSts"
#endif /* __OBJECT_DEFS_H__ */
