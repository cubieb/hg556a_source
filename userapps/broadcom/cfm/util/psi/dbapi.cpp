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
//  Filename:       dbapi.cpp
//  Author:         Peter T. Tran
//  Creation Date:  04/29/04
//
//  Description:
//      Implement the BcmDb_* class and functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <time.h>
#include "bcmtypes.h"
#include "bcmcfm.h"
#include "dbapi.h"
#include "objectdefs.h"
#include "syscall.h"
#include "secmngr.h"
#include "routeapi.h"
#include "atmvcccfgtbl.h"
#include "todapi.h"
#include "ezc.h"
#include "pmapdefs.h"
#include "pmapapi.h"
#include "sysdiag.h"
#include "ifcdefs.h"
#include "psixml.h"
#include "board_api.h"   //Added by luokunling l00192527 ,2011/10/14
#ifdef SUPPORT_MACMATCHIP  
#include "macmatchapi.h"
#endif
/*start-add by 00112761 for VDF*/
#include "cgimain.h"
#include "ttyUSB.h"
#include <sys/msg.h>
/*end-add by 00112761 for VDF*/

/*start  of 增加Radius 认证功能by s53329  at  20060714 */
#ifdef   SUPPORT_RADIUS
#include "ifcdefs.h"
#endif
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
/* start of maintain 增加对PTM 的支持 by zhangliang 60003055 2006年7月8日" */
#ifdef SUPPORT_VDSL
#include "cgimain.h"
extern "C" WEB_NTWK_VAR glbWebVar;
#endif
/*start of增加igmp接口选择功能 by l129990 2008,9,27*/
#ifdef SUPPORT_IGMPCONFIG
extern "C" WEB_NTWK_VAR glbWebVar;
#endif
/*end of增加igmp接口选择功能 by l129990 2008,9,27*/
/* end of maintain 增加对PTM 的支持 by zhangliang 60003055 2006年7月8日" */
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
#include "vlandefs.h"
#include "ifclanapi.h"
/*end vlan bridge, s60000658, 20060627*/
//#endif
#include "autoupgrade.h"
#include "board_api.h"
/*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
#include "macro.h"
/*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
/* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月10日 */

#ifdef SUPPORT_POLICY
WanRouteMapping gl_stPolicyRoutingMap[IFC_TINY_LEN];
#endif

/* BEGIN: Added by y67514, 2009/2/17   PN:GLB WEB:portfowding*/
#include "ifcwanapi.h"
/* END:   Added by y67514, 2009/2/17 */
/*add by z67625 mac过滤新页面增加支持开关 start*/
#include "macapi.h"
/*add by z67625 mac过滤新页面增加支持开关 end*/
#ifdef VOXXXLOAD
#include <vodslapi.h>
#endif
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif
#ifndef VDF_RESERVED
#define VDF_RESERVED
#endif
/* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月10日 */
#ifdef SUPPORT_SAMBA
#include "sambadefs.h"
#endif
static DB_DEFAULT_ITEM DbDefaultTable[] = {
   { "sysUserName", "admin" },
   { "sysPassword", "admin" },
#ifndef SUPPORT_ONE_USER
   { "sptUserName", "support" },
   { "sptPassword", "support" },
   { "usrUserName", "vodafone" },
   { "usrPassword", "vodafone" },
#endif
   //{ "curUserName", "admin" }, // default "vodafone". <tanyin 2009.2.16>
   { "curUserName", "vodafone" },
#ifdef SUPPORT_TR69C
   { "tr69cStatus", "1" },
   { "tr69cUpgradesManaged", "0" },
   { "tr69cUpgradeAvailable", "0" },
   { "tr69cInformEnable", "0" },
   { "tr69cInformTime", "0" },
   /*start of HGW 2005.11.12 V300R001B03D092 A36D02711 
   { "tr69cInformInterval", "300" },
   end  of HGW 2005.11.12 V300R001B03D092 A36D02711 */
   { "tr69cInformInterval", "7200" },
   { "tr69cAcsURL", "" },
    /*start of HGW 2005.11.12 V300R001B03D092 A36D02711 
   { "tr69cAcsUser", "admin" },
   { "tr69cAcsPwd", "admin" },
    end  of HGW 2005.11.12 V300R001B03D092 A36D02711 */
   { "tr69cAcsUser", "" },
   { "tr69cAcsPwd", "" },
   { "tr69cParameterKey", "12345" },
    /*start of HGW 2005.11.12 V300R001B03D092 A36D02711 
   { "tr69cConnReqURL", "http://www.broadcom.com/acs" },
   { "tr69cConnReqUser", "admin" },
   { "tr69cConnReqPwd", "admin" },
   { "tr69cKickURL", "http://www.broadcom.com/acs" },
    end of HGW 2005.11.12 V300R001B03D092 A36D02711 */
   { "tr69cConnReqURL", "http://www.huawei.com/acs" },
   { "tr69cConnReqUser", "" },
   { "tr69cConnReqPwd", "" },
   { "tr69cKickURL", "http://www.huawei.com/acs" },
   { "tr69cProvisioningCode", "12345" },
#endif
    /* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
    { "autoScanList", "" },
    /* END:   Added by y67514, 2008/10/27 */
/*start of add the Upnp Video Provision function by l129990,2010,2,21*/
    { "upnpVideoServ", "1" },
    { "upnpVideoProv", "1" },
/*end of add the Upnp Video Provision function by l129990,2010,2,21*/
   { "ethIpAddress", "192.168.0.1" },
   { "ethSubnetMask", "255.255.255.0" },
   /* BEGIN: Added by y67514, 2008/9/22   PN:GLB:lan dns*/
   { "lanDomain", "Vodafone.DSLRouter" },
   /* END:   Added by y67514, 2008/9/22 */
   /*start of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
   { "addrType", "2" },
   /*end of Global V100R001C01B020 by c00131380 AU8D00949 at 2008.10.20*/
   { "lan2IpAddress", "" },
   { "lan2SubnetMask", "" },
   { "enblLan2", "0" },
   { "wanIpAddress", "0.0.0.0" },
   { "wanSubnetMask", "0.0.0.0" },
   { "defaultGateway", "0.0.0.0" },
   { "enblAuto", "1" },
   { "wanIfName", "" },
   /* BEGIN: Added by y67514, 2008/10/21   PN:AU8D01013:网关在使用动态IPOE上网时，当本地设置的DNS和局端下发的DNS不同时，不能优先采用本地设置*/
    { "wanDns1", "0.0.0.0" },
    { "wanDns2", "0.0.0.0" },
    { "wanGateway", "0.0.0.0" },
   /* END:   Added by y67514, 2008/10/21 */
   { "dnsPrimary", "0.0.0.0" },
   { "dnsSecondary", "0.0.0.0" },
   { "dnsDynamic", "1" },
   /* start: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
   /* enbdnsProxy should be enable by default by y00183561 2011/11/3 */
   { "enbdnsProxy", "1" },
   /* end: add by y00183561 for lan dns proxy enable/disable 2011/10/24 */
   { "dnsDomainName", "" },
   { "dhcpEthStart", "192.168.1.2" },
   { "dhcpEthEnd", "192.168.1.254" },
   /* start of maintain 大T规格，dhcpLeaseTime为72小时 by xujunxia 43813 2006年5月17日
   { "dhcpLeasedTime", "24" },
   */
   { "dhcpLeasedTime", "72" },
   /* end of maintain 大T规格，dhcpLeaseTime为72小时 by xujunxia 43813 2006年5月17日 */
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
   { "dhcpRelay", "0" },
/*end of support to the dhcp relay  function by l129990,2009,11,12*/
   /*start HDPC Server 支持第二地址池, by s60000658, 20060616*/
   {"dhcpEthStart2", ""},
   {"dhcpEthEnd2", ""},
   {"dhcpLeasedTime2", "72"},
   {"dhcpSrv2ClassId", ""},
   {"enblDhcpSrv2", "0"},
   /*start HDPC Server 支持第二地址池, by s60000658, 20060616*/
/*start of support to configure the option66,67,160 by l129990,2009,12,22*/
   { "lanOption66", "" },
   { "lanOption67", "" },
   { "lanOption160", "" },
/*end of support to configure the option66,67,160 by l129990,2009,12,22*/
   { "pppUserName", "" },
   { "pppPassword", "" },
   { "serviceName", "" },
   { "serviceId", "0" },
   //{ "enblIgmp", "0" },
  /*start of删除brcom igmp proxy 使能功能by l129990 ,2008,9,27*/
// { "enblIgmp", "1" },
  /*end of删除brcom igmp proxy 使能功能by l129990 ,2008,9,27*/
   { "enblService", "1" },
   { "ntwkPrtcl", "2" },
   { "encapMode", "0" },
   { "enblDhcpClnt", "1" },
   { "enblDhcpSrv", "1" },
   { "enblNat", "1" },
   { "enblFirewall", "1" },
   { "enblAutoScan", "1" },
   { "enblOnDemand", "0" },
   { "useStaticIpAddress", "0" },
   { "pppTimeOut", "30" },
   { "pppIpExtension", "0" },
   { "pppAuthMethod", "0" },
   { "pppLocalIpAddress", "0.0.0.0" },
   { "enblPppDebug", "0" },
   
//start modify by xia 133940 2008年9月8日
	{ "automanualConnect", "1" },
	/* j00100803 解决pppoe验证失败期间,网页响应慢的问题,将15改成25 */
	{ "delayRedial", "25" },
	{ "timeout", "3" },
	{ "delayTime", "15" },
//end modify by xia 133940 2008年9月8日

/*Start -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/
	{ "enblpppVoB", "0" },
    { "pppVoBitstreamIPAddr", "0.0.0.0" },
/*End -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/





#ifdef VDF_PPPOU
    /* BEGIN: Added by y67514, 2009/12/15   PN:APNSecurity Enhancement*/
    { "acEncodedProfileUsName", "" },
    { "acEncodedProfilePwd", "" },
    /* END:   Added by y67514, 2009/12/15 */
//add by l66195 for pppousb start
   { "acProName", ""},
   { "LinkMode", "0"},
//add by l66195 for pppousb end
#endif
/* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
{ "VoiceEnable", "0"},
/* END:   Added by s00125931, 2008/9/12 */
   { "logIpAddress", "0.0.0.0" },
   { "logLevel", "7" },
   { "logDisplay", "3" },
   { "logMode", "1" },
   { "logPort", "514" },
   { "logStatus", "0" },
   /*add by z67625 防火墙日志写flash开关默认值设置 start*/
   { "fwlog", "0" },
   /*add by z67625 防火墙日志写flash开关默认值设置 end*/

/* start of 增加tr143 功能*/
  {"UdpEnable","0"},
  {"UdpInterface",""},
  {"UdpSourceIPAddress",""},
  {"UdpPort","0"},
  {"UdpEchoPlusEnabled","0"},
  {"UploadDiagState","None"},
  {"UploadInterface",""},
  {"UploadURL",""},
  {"UploadDSCP","0"},
  {"UploadEthPriority","0"},
  {"UploadTestFileLen","0"},
  {"DownDiagState","None"},
  {"DownInterface",""},
  {"DownloadURL",""},
  {"DownDSCP","0"},
  {"DownEthPriority","0"},
/* end of 增加tr143 功能*/

   /*start of 增加系统日志保存FLASH开关默认值设置by c00131380 at 080926*/
   { "Syslog", "0" },
   /*end of 增加系统日志保存FLASH开关默认值设置by c00131380 at 080926*/
   { "gRipMode", "0" },
   { "ripMode", "0" },
   { "ripVersion", "2" },
   { "ripOperation", "1" },
   { "atmVpi", "0" },
   { "atmVci", "35" },
   { "atmPeakCellRate", "0" },
   { "atmSustainedCellRate", "0" },
   { "atmMaxBurstSize", "0" },
   { "atmServiceCategory", "UBR" },
   { "snmpStatus", "1" },
   { "snmpRoCommunity", "s0l0tuya" },
   { "snmpRwCommunity", "4rEAd&wrItE" },
   { "snmpSysName", "unknown" },
   { "snmpSysLocation", "unknown" },
   { "snmpSysContact", "unknown" },
   { "snmpTrapIp", "0.0.0.0" },
   { "macPolicy", "0" },
   { "encodePassword", "1" },
   { "enblQos", "0" },
   { "enblUpnp", "1" },
   { "enblIgmpSnp", "0" },
  /*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
   //{ "enblIgmpMode", "0" },
  /*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
/*start of增加igmp版本号和组播组数可配功能 by l129990 2008,9,10*/
#ifdef SUPPORT_IGMPCONFIG
   { "enblIgmpFunction", "0" },
   { "enblIgmpInterface", "none" },
   { "enblIgmpVersion", "2" },
   { "enblIgmpGroups", "32" },
   { "enblIgmpLog", "0" },
#endif
/*end of增加igmp版本号和组播组数可配功能 by l129990 2008,9,10*/
   { "enblSiproxd", "0" },
   { "sipProxyPort", "5060" },
// add by l66195 for VDF start
   { "enblDdns", "0"},
// add by l66195 for VDF end
/* start 支持以太网口配置 by l68693 2009-02-17*/  
/*begin add by p44980 2008.10.22*/
   { "connSwitchMode", "0"},
/*end add by p44980 2008.10.22*/
#if defined (ETH_CFG ) || defined (SUPPORT_GLB_MTU)
   { "ethSpeed", "0" },
   { "ethType", "0" },
   { "ethMtu", "1500" },
#endif
/* end 支持以太网口配置 by l68693 2009-02-17*/  

/*start of VDF  2008.3.28 HG553V100R001 w45260:AU8D00327、 Wlan配置项错误导致网关配置会恢复出厂配置.
  修改:提供出错默认配置，避免网关配置项的整体恢复 同时需要与targets/defaultcfg目录下的默认配置文件中
  的设置保持一致。.
*/
#ifdef WIRELESS
   { "wlSsidIdx", "0" },
   //{ "wlSsid", "Huawei" },
   { "wlSsid", "" },  /*默认为""表示需要读取生产装备中设置的SSID,参见wlmngr.cpp中的retrieve()函数*/
   { "wlSsid_2", "Guest" },
   { "wlCountry", "IT" },
   { "wlWepKey64", "" },
   { "wlWepKey128", "" }, /*默认为""表示需要读取生产装备中设置的wepkey，参见wlmngr.cpp中的retrieve()函数*/
   { "wlPreambleType", "long" },
   { "wlHide", "0" },
   { "wlEnbl", "1" },
   { "wlCurEnbl", "1" },   
   { "wlEnbl_2", "0" },
   { "wlKeyIndex", "1" },
   { "wlChannel", "0" },
//BEGIN:add by zhourongfei to periodically check the best channel
   { "wlchannelcheck", "0" },
//END:add by zhourongfei to periodically check the best channel
   { "wlFrgThrshld", "2346" },
/* BEGIN: Modified by c106292, 2008/10/4   PN:Global RTS Atheros 最大值为2346*/
#ifdef SUPPORT_ATHEROSWLAN
   { "wlRtsThrshld", "2346" },
#else
   { "wlRtsThrshld", "2347" },
#endif
   { "wlDtmIntvl", "1" },
   { "wlBcnIntvl", "100" },
   { "wlFltMacMode", "disabled" },
   { "wlAuthMode", "open" },
   { "wlKeyBit", "0" },   //128-bits
   { "wlRate", "0" },
/* BEGIN: Modified by c106292, 2008/10/8   PN:Global*/
 #ifdef SUPPORT_ATHEROSWLAN   
   { "wlPhyType", "n" },
   #else
   { "wlPhyType", "g" },
   #endif
/* END:   Modified by c106292, 2008/10/8 */
   { "wlBasicRate", "default" },
/* BEGIN: Modified by c106292, 2008/9/18   PN:Global*/
    #ifdef SUPPORT_ATHEROSWLAN   
   { "wlgMode", "6" },  //54g Auto
   #else
   { "wlgMode", "1" },  //54g Auto
   #endif
   { "wlProtection", "auto" },
   { "wlWpaPsk", "" },/*默认为""表示需要读取生产装备中设置的psk，参见wlmngr.cpp中的retrieve()函数*/
   { "wlWpaGTKRekey", "0" },
   { "wlRadiusServerIP", "0.0.0.0" },
   { "wlRadiusPort", "1812" },
   { "wlRadiusKey", "" },
   { "wlWep", "disabled" },
   { "wlWpa", "aes" },
   { "wlAuth", "0" },
   { "wlMode", "ap" },
   { "wlLazyWds", "0" },
   { "wlFrameBurst", "off" },
   { "wlAPIsolation", "0"},
   { "wlBand", "0" },
   { "wlMCastRate", "0"},
   { "wlAfterBurnerEn", "off"},
#if 0
   { "ezc_version", EZC_VERSION_STR},
   { "ezc_enable", "1"},
   { "is_default", "1"},
   { "is_modified", "0"},
#endif 
   { "wlInfra", "1"},
   { "wlAntDiv", "3"},
#ifdef SUPPORT_MIMO   
   { "wlWme","-1"},  /* 2008/01/28 Jiajun Weng */
#else
   { "wlWme", "1"}, 
#endif    
   { "wlWmeNoAck", "0"}, 
   /*start of enable or disable the access point radio by l129990,2009-10-9*/
   { "wlRadioCtl", "1"},
    /*end of enable or disable the access point radio by l129990,2009-10-9*/
   { "wlWmeApsd", "1"},      /* 2008/01/28 Jiajun Weng */
   { "wlPreauth", "0"},  
   { "wlMaxAssoc","32"},         /*global max clients allowed: from 128 to 16*/
   { "wlGlobalMaxAssoc","128"},  /*max clients allowed: from 128 to 16*/
   { "wlDisableWme","0"},    /* 2008/01/28 Jiajun Weng */
#ifdef SUPPORT_WLAN_PRNTCTL  
   { "wlAutoSwOffdays", "127"},       /* monday~sunday set as 1 bit */
   { "wlStarOfftTime", "0"},      /* 00:00 default*/
   { "wlEndOffTime", "28800"},    /* 08:00 default*/
#endif   
/*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_SES) || defined (SUPPORT_ATHEROSWLAN)
   { "wlSesEnable", "1"},
   { "wlSesEvent", "2"},
   { "wlSesStates", "01:01"},
   { "wlSesSsid",""}, 
   { "wlSesWpaPsk",""},             
   { "wlSesClosed","0"},   
   { "wlSesAuth","0"}, 
   { "wlSesAuthMode",""},     
   { "wlSesWep",""}, 
   { "wlSesAuth",""},
   { "wlSesWpa",""},            
   { "wlSesWdsMode","2"},   
   { "wlSesClEnable","1"},
   { "wlSesClEvent","0"},
   { "wlWdsWsec",""},                 
#endif   
   { "wlNetReauth","36000"},
   { "wlTxPwrPcnt", "100"},
   { "wlRegMode","0"},
   { "wlDfsPreIsm","60"},
   { "wlDfsPostIsm","60"}, 
   { "wlTpcDb","0"},     	 
   { "wlCsScanTimer","0"},    	 
   { "wlan_ifname", "ath0"},
#ifdef SUPPORT_TR69C
   { "tr69cBeaconType", "1"},                  //basic
   { "tr69cBasicEncryptionModes", "0"},        //None
   { "tr69cBasicAuthenticationMode", "0"},     //None
   { "tr69cWPAEncryptionModes", "3"},          //AES
   { "tr69cWPAAuthenticationMode", "0"},       //psk
   { "tr69cIEEE11iEncryptionModes", "3"},      //AES
   { "tr69cIEEE11iAuthenticationMode", "0"},   //psk2
#endif 
/* BEGIN: Modified by c106292, 2008/9/10   PN:Global*/
   /*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_WSC) || defined (SUPPORT_ATHEROSWLAN)
   {"wsc_mode","enabled"},
   {"wsc_config_state", "1"},
#endif
/* END:   Modified by c106292, 2008/9/10 */
/*end of VDF  2008.3.28 HG553V100R001 w45260:AU8D00327 Wlan配置项错误导致网关配置会恢复出厂配置.修改:提供出错默认配置，避免网关配置项的整体恢复*/

#ifdef SUPPORT_MIMO
   { "wlNBwCap", "2"},    /* 2008/01/28 Jiajun Weng : Modified to wlNBwCap */
   { "wlNCtrlsb", "0"}, 
   { "wlNBand", "2"},
   { "wlNMcsidx", "-1"},
   { "wlNProtection", "auto" },
   { "wlRifs", "auto" }, 
   { "wlAmpdu", "auto" }, 
   { "wlAmsdu", "auto" },
   { "wlNmode", "auto" },
   { "wlNReqd", "0" },   
#endif   
   { "wlan_ifname", "ath0"},
#endif /* WIRELESS */
#if SUPPORT_PORT_MAP
   { "pmapIfcCfgStatus", "0"},
   { "pmapIfcCfgName", "unknown"},
#endif
#if SUPPORT_ETHWAN
   { "enblEnetWan", "0"},
#endif
#if SUPPORT_VLAN
   { "enblVlan", "1"},
   { "vlanId", "-1"},
#endif
#ifdef SUPPORT_IPSEC
   { "ipsConnEn", "0"},    
   { "ipsConnName", "new connection"},
   { "ipsRemoteGWAddr", "0.0.0.0"},
   { "ipsLocalIPMode", "subnet"},
   { "ipsLocalIP", "0.0.0.0"},
   { "ipsLocalMask", "255.255.255.0"},
   { "ipsRemoteIPMode", "subnet"},
   { "ipsRemoteIP", "0.0.0.0"},
   { "ipsRemoteMask", "255.255.255.0"},
   { "ipsKeyExM", "auto"},
   { "ipsAuthM", "pre_shared_key"},
   { "ipsPSK", "key"},
   { "ipsCertificateName", ""},
   { "ipsPerfectFSEn", "disable"},
   { "ipsManualEncryptionAlgo", "3des-cbc"},
   { "ipsManualEncryptionKey", ""},
   { "ipsManualAuthAlgo", "hmac-md5"},
   { "ipsManualAuthKey", ""},
   { "ipsSPI", "101"},
   { "ipsPh1Mode", "main"},
   { "ipsPh1EncryptionAlgo", "3des"},
   { "ipsPh1IntegrityAlgo", "md5"},
   { "ipsPh1DHGroup", "modp1024"},
   { "ipsPh1KeyTime", "3600"},
   { "ipsPh2EncryptionAlgo", "3des"},
   { "ipsPh2IntegrityAlgo", "hmac_md5"},
   { "ipsPh2DHGroup", "modp1024"},
   { "ipsPh2KeyTime", "3600"},
   /*add by z67625 默认值获取增加支持传输模式和接口 start*/
   {"ipsTransMode", "tunnel"},
   {"ipsProtocol", "esp"},
   {"ipsInterface",""},
   /*add by z67625 默认值获取增加支持传输模式和接口 end*/
#endif
#ifdef PORT_MIRRORING
   { "mirrorPort", "eth0"},
   { "mirrorStatus", "disabled"},
#endif
#ifdef  SUPPORT_RADIUS
    {"WebEnable", "0"},
    {"TelnetEnable", "0"},
    {"SshEnable", "0"},
    {"RadiusTimeout", "3"},
    {"RadiusRetransmit", "3"},
    {"RadiusPrimaryServerPort", "1812"},
    {"RadiusSecondServerPort", "1812"},
#endif
/* start of protocol QoS for TdE by z45221 zhangchen 2006年8月15日 */
#ifdef SUPPORT_TDE_QOS
    {"QosIfcRate", "0"},
    {"QosIfcCeil", "0"},
#endif
/* end of protocol QoS for TdE by z45221 zhangchen 2006年8月15日 */
#ifdef SUPPORT_PORTMAPING
    { "option60str",""},
#endif
#ifdef	VDF_RESERVED
    { "ReservedIpstr",""},
#endif
#ifdef SUPPORT_SAMBA
   {"smbEnable","0"},
   {"smbWorkgroup", "vodafone"},
   {"smbNetbiosname", "vodafone"},
   {"smbServername", "vodafone"},
#endif
//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
	{"pppNumber","3"},
//END:add by zhourongfei to config number of PPP keepalive attempts
/* BEGIN: Added by y67514, 2008/9/8   PN:GLB:增加参数mtu，mixed*/
    {"enblBridgeMixed","0"},
    {"lcpEchoInterval","30"},
    {"PppMtu", "1492"},
    {"IpoeMtu", "1500"},
    {"PppExtMac", ""},
/* END:   Added by y67514, 2008/9/8 */
#ifdef SUPPORT_GLB_ALG
    {"enblH323","1"},
    {"enblRtsp","1"},
    {"enblFtp","1"},
    {"enblTftp","1"},
    {"enblL2tp","1"},
    {"enblPptp","1"},
    {"enblIpsec","1"},
#endif /* SUPPORT_GLB_ALG */
//BEGIN:added by zhourongfei to config web at 2011/05/30
   { "LANDisplayCFG", "1" },
   { "PINDisplayCFG", "1" },
   { "HSPADisplayCFG", "1" },
//END:added by zhourongfei to config web at 2011/05/30
/*Begin:Added by luokunling l00192527 for IMSI imform requirement,2011/9/24*/
   { "Notificationenable","1"},
/*End:Added by luokunling l00192527,2011/9/24*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
   {"HTTPInterceptVisible","1"},
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
   { NULL, NULL }
};
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
static VLAN_8021Q_ITEM_ENTRY g_vlanList[VLAN_8021Q_NUM_MAX] = 
{
    {0, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {1, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {2, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {3, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {4, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {5, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {6, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {7, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {8, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {9, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {10, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {11, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {12, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {13, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {14, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}, 
    {15, 0, 1, VLAN_ENTRY_NOTINUSE, "br0", ""}
};
static VLAN_8021Q_PORT_ENTRY g_vlPortList[VLAN_8021Q_PORT_MAX] =
{
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE},
    {"", BR_PORT_MODE_TRUNK, 1, 65535, VLAN_ENTRY_NOTINUSE}
};
static VLAN_8021Q_CFG_ENTRY g_vlanCfg = 
{1, 0, 0, 0, 0, 0, "", "", &g_vlanList[0]};
/*end vlan bridge, s60000658, 20060627*/
//#endif

#if VOXXXLOAD
/* BEGIN: Modified by p00102297, 2008/2/21 */
//static SIP_WEB_PHONE_CFG_S g_phoneCfg = {0, 0, 100, 0, 0, 80, 250,"[X*#ABCD].T", 0, 0, 250, 250, 100};
static SIP_WEB_PHONE_CFG_S g_phoneCfg = 
{
	DEFAULT_VOICE_SPEAK_VOL, 
	DEFAULT_VOICE_LISTEN_VOL, 
	DEFAULT_VOICE_DIAL_INTERVAL, 
	//DEFAULT_SIP_VAD_VALUE, 
	DEFAULT_SIP_REGION_LOCAL, 
	DEFAULT_VOICE_MINHOOK_FLASH, 
	DEFAULT_VOICE_MAXHOOK_FLASH,
	DEFAULT_VOICE_DIGITMAP, 
	//DEFAULT_SIP_EC_VALUE, 
	//DEFAULT_SIP_CNG_VALUE, 
	DEFAULT_VOICE_OFFHOOK_TIME, 
	DEFAULT_VOICE_ONHOOK_TIME, 
	DEFAULT_VOICE_FIRST_DIAL,
	/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	DEFAULT_VOICE_REINJECTION,
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
	/*start of additon by chenyong 2008-10-28 for Inner call*/
	DEFAULT_VOICE_INNERCALL,
	/*end of additon by chenyong 2008-10-28 for Inner call*/
    DEFAULT_VOICE_MODEMSERVICE,
    //add by z67625 for automatic call
    /*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
	DEFAULT_VOICE_VOICEPROMPTVISIBLE,
	/*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
    DEFAULT_VOICE_AUTOCALLENBL,
    DEFAULT_VOICE_AUTOCALLINTERVAL,
    DEFAULT_VOICE_AUTOCALLNUM
};

/* END:   Modified by p00102297, 2008/2/21 */

static SIP_WEB_DIAL_PLAN_S g_dialPlan[SIP_MAX_DIAL_PLAN] = 
{
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""}
};
static SIP_WEB_FWD_S       g_callFWD[SIP_MAX_CALL_FWD] =
    {
        //table1
        {
            {
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD}
            },
            {
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_ONBSY_FWD},
                {FALSE, "", "", SIP_WEB_NOREP_FWD},
            },
            30
        },
        //table2
        {
            {
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_UNCON_FWD}
            },
            {
                {FALSE, "", "", SIP_WEB_UNCON_FWD},
                {FALSE, "", "", SIP_WEB_ONBSY_FWD},
                {FALSE, "", "", SIP_WEB_NOREP_FWD},
            },
            30
        }
    };
#endif

#ifdef SUPPORT_PORT_MAP
//static BRIDGE_LIST bridgeList[PORT_MAP_MAX_ENTRY]; Pavan
static PORT_MAP_CFG_ENTRY bridgeList[PORT_MAP_MAX_ENTRY] = 
{
  // GroupName GroupInterfaceName GroupedInterfaces DynamicInterfaces VendorIds
   {"Default", "br0", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_INUSE},
   {"", "br1", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br2", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br3", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br4", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br5", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br6", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br7", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br8", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br9", "",  "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br10", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br11", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br12", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br13", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br14", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE},
   {"", "br15", "", "", {"", "", "", "", ""}, PMAP_CFG_ENTRY_NOTINUSE}
   };

static PORT_MAP_IFC_STATUS_INFO bridgeIfcCfgSts = {0, "unknown"};
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define SLEEPEX(seconds, nanoseconds) ({              \
                struct timespec tv;                \
                tv.tv_sec = (seconds);             \
                tv.tv_nsec = (nanoseconds);        \
                while (nanosleep(&tv, &tv) == -1); \
        }) 

static void RebootHSPAStick(void)
{
    int iMsgKeySend;
    int iSleep = 0;
    int time = 0;
    struct stAtSndMsg stSndFunMsg;                      //发送消息(放入发送消息队列) 
    iMsgKeySend = msgget(MSG_AT_QUEUE, 0666);
    printf("%s:%s:%d:reset the HspaStick!***\n",__FILE__,__FUNCTION__,__LINE__);
    AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=4");
#if 0
     iSleep = 2;
    time = iSleep;
    while ( iSleep)
    {
        iSleep = sleep(iSleep);
        if ( 0 == time )
        {
            break;
        }
        time--;
    }
#endif
	
     SLEEPEX(3,1000);
	 
    AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=6");
    SLEEPEX(5,1000);
#if 0
    iSleep = 5;
    time = iSleep;
    while ( iSleep)
    {
        iSleep = sleep(iSleep);
        if ( 0 == time )
        {
            break;
        }
        time--;
    }
#endif
}
//**************************************************************************
// Function Name: BcmDb_getDefaultValue
// Description  : get default value by variable name.
// Parameters   : var - variable name.
// Returns      : value - variable value.
//**************************************************************************
char *BcmDb_getDefaultValue(char *var) {
   int  i = 0;
   char *ret = "";

   for ( i = 0, ret = DbDefaultTable[i].varValue;
         ret != NULL;
         i++, ret = DbDefaultTable[i].varValue )
      if ( strcmp(var, DbDefaultTable[i].varName) == 0 )
         break;

   if ( ret == NULL )
      ret = "";

   return ret;
}

//**************************************************************************
// Function Name: BcmDb_removeWanInfo
// Description  : remove Wan data and object as well as all the configuration
//                related to this wan interface
// Returns      : None.
//**************************************************************************
void BcmDb_removeWanInfo(PWAN_CON_ID pWanId) {
    char wanIf[IFC_TINY_LEN];
    WAN_CON_INFO wanInfo;

    if ( BcmDb_getWanInfo(pWanId, &wanInfo) == DB_WAN_GET_NOT_FOUND )
        return;
      
    if ( wanInfo.protocol == PROTO_NONE )
        return;
      
    BcmDb_getWanInterfaceName(pWanId, wanInfo.protocol, wanIf);
   
    // Remove all this wan interfaces related configurations
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

    if ( wanInfo.protocol != PROTO_BRIDGE ) {
        // Incoming IP Filter: remove the incoming ip filter if the pvc is gone	
        SecFltInTbl *objSecFltInTbl =  objSecMngr->getFltInTbl();
        /*modified by z67625 修改接口增加开关判断 start*/
        objSecFltInTbl->removeByInterface(wanIf, BcmDb_getSecEnbl(SCM_SEC_INFLT_ENABLE));
        /*modified by z67625 修改接口增加开关判断 end*/

        /*start modifying  for layer3 mac filter by p44980 2008.2.14*/
        // Mac Filtering: remove the mac filter is the pvc is gone
        SecMacFltTbl *objSecMacFltTbl = objSecMngr->getMacFltTbl();
        objSecMacFltTbl->removeByInterface(wanIf);
        /*end modifying  for layer3 mac filter by p44980 2008.2.14*/
        
#ifdef SUPPORT_RIP
        // Rip: remove the rip entry if the pvc is gone
        BcmRcm_removeRipInterface(wanIf);  
#endif
        // Static Route:  remove the static route if the pvc is gone
        BcmRcm_removeRouteCfgByWanIf(wanIf); 

        // cleanup iptables entry related to this interface
        bcmRemoveAllIpTableRules(wanIf);

        bcmRemoveDefaultGatewayByWanIf(wanIf);
    } else {
        // Mac Filtering: remove the mac filter is the pvc is gone
        SecMacFltTbl *objSecMacFltTbl = objSecMngr->getMacFltTbl();
        objSecMacFltTbl->removeByInterface(wanIf);

        // cleanup ebtables entry related to this interface
        bcmRemoveAllEbTableRules(wanIf);
    }   
   
    // remove wanInfo from PSI
    BcmDb_deleteWanInfo(pWanId, FALSE);
    /*start of 增加删除一条WAN与删除组播接口同步功能 by l129990 2008,9,27*/
    BcmDb_clearIgmpProxyInterface(wanIf);
     /*end of 增加删除一条WAN与删除组播接口同步功能 by l129990 2008,9,27*/
}

/*start of 增加删除一条WAN与删除组播接口同步功能 by l129990 2008,9,27*/
//**************************************************************************
// Function Name: BcmDb_clearIgmpProxyInterface
// Description  : clear igmp proxy interface when the related wan 
//                is deleted              
// Returns      : None.
//**************************************************************************
void BcmDb_clearIgmpProxyInterface(char *wanIf)
{   
    IgmpProxyNode igmpProxy;
    
    if (NULL == wanIf) 
	{
    	return ;
	}
   
    BcmDb_getIgmpProxyInfo(&igmpProxy);
    
    if (NULL == strcmp(wanIf, igmpProxy.igmpInterface))
    { 
        strcpy(glbWebVar.enblIgmpInterface, "none");
        strcpy(igmpProxy.igmpInterface, "none");
    }

    BcmDb_setIgmpProxyInfo(&igmpProxy);
}
/*end of 增加删除一条WAN与删除组播接口同步功能 by l129990 2008,9,27*/
//**************************************************************************
// Function Name: BcmDb_deleteWanInfo
// Description  : remove Wan data and object.
// Parameters   : wanId - wan ID.
//                isInitialized - true if is called when system is started
// Returns      : None.
//**************************************************************************
int BcmDb_deleteWanInfo(PWAN_CON_ID pWanId, const int isInitialized) {
    int ret = DB_WAN_REMOVE_OK;
    char appWan[IFC_TINY_LEN], appPpp[IFC_TINY_LEN], appIp[IFC_TINY_LEN];
    char wanIf[IFC_TINY_LEN];
    UINT16 retLen = 0, wanId = 0;
    WAN_CON_INFO wanInfo,secondWanInfo; // by z45221 QoS for TdE
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;
    PSI_HANDLE psiPpp = NULL;
    PSI_HANDLE psiIp = NULL;
    UINT16 flag = 0;
    UINT16 useSameVccflag = 0;
    

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);
      
    status = BcmPsi_objRetrieve(psiWan, pWanId->conId, &wanInfo,
                                sizeof(wanInfo), &retLen);
    if ( status == PSI_STS_OK ) {
        BcmDb_getWanInterfaceName(pWanId, wanInfo.protocol, wanIf);
        // remove wan info in PSI
        BcmPsi_objRemove(psiWan, pWanId->conId);
        // remove wan service info in PSI
        switch ( wanInfo.protocol ) {
            case PROTO_PPPOE:
            case PROTO_PPPOA:
#ifdef VDF_PPPOU
            case PROTO_PPPOU://add by l66195 for pppousb 
#endif
                sprintf(appPpp, "pppsrv_%d_%d", pWanId->vpi, pWanId->vci);
                psiPpp = BcmPsi_appOpen(appPpp);
                BcmPsi_objRemove(psiPpp, pWanId->conId);
                break;
            case PROTO_MER:
            case PROTO_IPOA:
#if SUPPORT_ETHWAN
            case PROTO_IPOWAN:
#endif
                sprintf(appIp, "ipsrv_%d_%d", pWanId->vpi, pWanId->vci);
                psiIp = BcmPsi_appOpen(appIp);
                BcmPsi_objRemove(psiIp, pWanId->conId);
                break;
            /* start of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月9日 */
            case PROTO_BRIDGE:
                sprintf(appIp, "brsrv_%d_%d", pWanId->vpi, pWanId->vci);
                psiIp = BcmPsi_appOpen(appIp);
                BcmPsi_objRemove(psiIp, pWanId->conId);
                break;
            /* end of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月9日 */
        }   
        // is there any existed wan in this pvc?
        for ( wanId = 1; wanId <= IFC_WAN_MAX; wanId++ ) { 
            /* start of protocol QoS for TdE by zhangchen z45221 2006年8月26日
            status = BcmPsi_objRetrieve(psiWan, wanId, &wanInfo,
                                        sizeof(wanInfo), &retLen);
            */
#if 1
            status = BcmPsi_objRetrieve(psiWan, wanId, &secondWanInfo,
                                       sizeof(secondWanInfo), &retLen);
#else
            status = BcmPsi_objRetrieve(psiWan, wanId, &wanInfo,
                                        sizeof(wanInfo), &retLen);
#endif
            /* end of protocol QoS for TdE by zhangchen z45221 2006年8月26日 */

            /* start of protocol QoS for TdE处理版本降级 by zhangchen z45221 2006年8月28日
            if ( status == PSI_STS_OK )
                break;
            */
            if ( status == PSI_STS_OK )
            {
                if ( secondWanInfo.vccId == wanInfo.vccId )
                {
                    useSameVccflag = 1;     // 还存在其他的wan接口,并且使用相同的vcc
                    break;
                }
                else
                {
                    flag = 1;   // wan接口存在但是使用另外的vcc,因此不能清零所有的wan接口
                }
            }
            /* end of protocol QoS for TdE处理版本降级 by zhangchen z45221 2006年8月28日 */

        }
        // there is no existed wan in this pvc so
        // need to remove this pvc too
        /* start of protocol QoS for TdE by zhangchen z45221 2006年8月26日 */
#ifdef SUPPORT_TDE_QOS
#ifdef SUPPORT_VDSL//add by s48571 for adsl porting 2006-10-26
		// PTM
        if ((TM_PTM_VLAN == g_trafficmode) 
           || (TM_PTM_PRTCL == g_trafficmode)
           || (TM_PTM_BR == g_trafficmode))
        {
            if (wanId <= IFC_WAN_MAX)
            {
                if (!useSameVccflag)
                {
                    /* 不存在使用相同vcc的wan接口才需要删除vcc */
                    wanId = IFC_WAN_MAX + 1;
                    flag = 1;
                }
            }
        }
        else
        {
            // atm
        }
        #else
        //adsl - (just the same as above)--s48571 for adsl porting 2006-10-26
        #endif
#endif
        /* end of protocol QoS for TdE by zhangchen z45221 2006年8月26日 */

        if ( wanId > IFC_WAN_MAX ) {
            // remove old VCC in VCC table
            if ( isInitialized == TRUE )
                AtmVcc_removeByObjectId(wanInfo.vccId);
            else
                AtmVcc_removeByObjectIdFromPsiOnly(wanInfo.vccId);
            // remove all wan info for this pvc
            /* start of protocol QoS for TdE by zhangchen z45221 2006年8月26日
            BcmPsi_appRemove(psiWan);
            BcmPsi_appRemove(psiPpp);
            BcmPsi_appRemove(psiIp);
            */
#if 1
            if (!flag)
            {
                BcmPsi_appRemove(psiWan);
                BcmPsi_appRemove(psiPpp);
                BcmPsi_appRemove(psiIp);
            }
#else
            if (!flag)
            {
                BcmPsi_appRemove(psiWan);
                BcmPsi_appRemove(psiPpp);
                BcmPsi_appRemove(psiIp);
            }
#endif
            /* end of protocol QoS for TdE by zhangchen z45221 2006年8月26日 */
        }
        // remove default gateway if this removed interface is used
        //bcmRemoveDefaultGatewayByWanIf(wanIf);
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_renameWanInfo
// Description  : Renames PSI applicatons names for WAN info.
// Parameters   : curWanId - current wan ID.
//                newWanId - current wan ID.
// Returns      : status.
//**************************************************************************
int BcmDb_renameWanInfo( PWAN_CON_ID pOldWanId, PWAN_CON_ID pNewWanId ) {
    int ret = DB_WAN_GET_OK;   
    char oldAppWan[IFC_TINY_LEN];
    char newAppWan[IFC_TINY_LEN];

    sprintf(oldAppWan, "wan_%d_%d", pOldWanId->vpi, pOldWanId->vci);
    sprintf(newAppWan, "wan_%d_%d", pNewWanId->vpi, pNewWanId->vci);
    BcmPsi_appRename(oldAppWan, newAppWan);

    sprintf(oldAppWan, "pppsrv_%d_%d", pOldWanId->vpi, pOldWanId->vci);
    sprintf(newAppWan, "pppsrv_%d_%d", pNewWanId->vpi, pNewWanId->vci);
    BcmPsi_appRename(oldAppWan, newAppWan);

    sprintf(oldAppWan, "ipsrv_%d_%d", pOldWanId->vpi, pOldWanId->vci);
    sprintf(newAppWan, "ipsrv_%d_%d", pNewWanId->vpi, pNewWanId->vci);
    BcmPsi_appRename(oldAppWan, newAppWan);

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_getAvailableWanConId
// Description  : retrieve available Wan connection ID of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getAvailableWanConId(PWAN_CON_ID pWanId) {
    int ret = DB_WAN_GET_OK;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, conId = 0;
    WAN_CON_INFO wanInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);     
    
    // find the first available WAN connection ID
    for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
        status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                    sizeof(wanInfo), &retLen);
        if ( status != PSI_STS_OK )
            break;
    }    
    if ( conId <= IFC_WAN_MAX )
        pWanId->conId = conId;
    else
        ret = DB_WAN_ADD_OUT_OF_CONNECTION;

    // if there is no existed WAN connection on this PVC then
    // remove the WAN app ID since user may change his mind
    // for not creating this available WAN connection
    if ( BcmDb_isWanConnectionExisted(pWanId) == FALSE )
        BcmPsi_appRemove(psiWan);
            
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_getWanInfo
// Description  : retrieve Wan data and object as well as all the configuration
//                related to this wan interface
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getWanInfo(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanInfo) {
    int ret = DB_WAN_GET_OK;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;

    // pWanInfo should be initialized first
    memset(pWanInfo, 0, sizeof(WAN_CON_INFO));
    pWanInfo->protocol = PROTO_NONE;
   
    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);     
    status = BcmPsi_objRetrieve(psiWan, pWanId->conId, pWanInfo,
                                sizeof(WAN_CON_INFO), &retLen);
    if ( status != PSI_STS_OK )
        ret = DB_WAN_GET_NOT_FOUND;

    // if there is no existed WAN connection on this PVC then
    // remove the WAN app ID 
    if ( BcmDb_isWanConnectionExisted(pWanId) == FALSE ){
        BcmPsi_appRemove(psiWan);
		}
            
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setWanInfo
// Description  : store Wan data and object as well as all the configuration
//                related to this wan interface
// Returns      : none.
//**************************************************************************
void BcmDb_setWanInfo(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanInfo) {
    char appWan[IFC_TINY_LEN];
    PSI_HANDLE psiWan = NULL;

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);     

    if( (pWanInfo->id & INSTID_VALID_MASK) != INSTID_VALID_VALUE )
       pWanInfo->id = BcmDb_getNewNtwkInstId();

  
    BcmPsi_objStore(psiWan, pWanId->conId, pWanInfo, sizeof(WAN_CON_INFO));
}

//**************************************************************************
// Function Name: BcmDb_getWanInfoNext
// Description  : Retrieve the next Wan data of the given WanId.
//                After call, the next WanId, and next Wan data are returned.
//                To get the first Wan data, initialize WanId as following
//                wanId.vpi = wanId.vci = wanId.conId = 0 before calling
//                this function.
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getWanInfoNext(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanInfo) {
    int ret = DB_WAN_GET_NOT_FOUND;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, vccId = 0, conId = 0;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = NULL;
    PSI_HANDLE psiWan = NULL;

    // pWanInfo should be initialized first
    memset(pWanInfo, 0, sizeof(WAN_CON_INFO));
    pWanInfo->protocol = PROTO_NONE;
   
    psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
   
    // if vpi = vci = 0 then find the first available vcc
    if ( pWanId->vpi == 0 && pWanId->vci == 0 ) {
        // find the first available vcc
        for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
            status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                        sizeof(vccInfo), &retLen);
            if ( status == PSI_STS_OK ) {
                pWanId->vpi = vccInfo.vccAddr.usVpi;
                pWanId->vci = vccInfo.vccAddr.usVci;
                break;   // break of vccId for loop
            }
        }
        // no available vcc
        if ( vccId > ATM_VCC_MAX )
            return ret;
    }
   
    // find the next available wan of the available vcc
    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
//printf("===> BcmDb_getWanInfoNext, before call appOpen for %s\n", appWan);
    psiWan = BcmPsi_appOpen(appWan);
    for ( conId = pWanId->conId + 1; conId <= IFC_WAN_MAX; conId++ ) {
        status = BcmPsi_objRetrieve(psiWan, conId, pWanInfo,
                                    sizeof(WAN_CON_INFO), &retLen);
        if ( status == PSI_STS_OK )
            break;   // break of conId for loop
    }
   
    // the next available wan of the available vcc is found
    if ( conId <= IFC_WAN_MAX ) {    
        pWanId->conId = conId;
        ret = DB_WAN_GET_OK;
    } else {
        /* start of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */
#ifdef SUPPORT_VDSL
			// PTM
            if ((TM_PTM_VLAN == g_trafficmode) 
               || (TM_PTM_PRTCL == g_trafficmode)
               || (TM_PTM_BR == g_trafficmode))
            {
                // #ifdef SUPPORT_TDE_QOS
                return ret;
                // #endif
            }else // ATM
            {
                // do noting
            }
#else
			// ATM
			// do noting
#endif
        /* end of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */

        // find the first available wan of the next available vcc
        // find the vccId of the available vcc
        for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ )
        {
            status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                        sizeof(vccInfo), &retLen);
            if ( status == PSI_STS_OK ) 
            {
                if ( pWanId->vpi == vccInfo.vccAddr.usVpi &&
                     pWanId->vci == vccInfo.vccAddr.usVci )
                break;
            }
        }
        // find the next available vcc
        for ( vccId = vccId + 1; vccId <= ATM_VCC_MAX; vccId++ ) {
            status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                        sizeof(vccInfo), &retLen);
            // if the next available vcc is found then find the first available wan
            if ( status == PSI_STS_OK ) {
                pWanId->vpi = vccInfo.vccAddr.usVpi;
                pWanId->vci = vccInfo.vccAddr.usVci;
                // find the first available wan    
                sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
//printf("===> BcmDb_getWanInfoNext, before call appOpen for %s\n", appWan);
                psiWan = BcmPsi_appOpen(appWan);
                for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                    status = BcmPsi_objRetrieve(psiWan, conId, pWanInfo,
                                                sizeof(WAN_CON_INFO), &retLen);
                    if ( status == PSI_STS_OK )
                        break;   // break of conId for loop
                }
                if ( conId <= IFC_WAN_MAX ) {    
                    pWanId->conId = conId;
                    break;   // break of vccId for loop
                }
            }
        }
        // the first available wan of the next available vcc is found
        if ( vccId <= ATM_VCC_MAX )
            ret = DB_WAN_GET_OK;
    }
   
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_getPppSrvInfo
// Description  : retrieve PPP data of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPppSrvInfo(PWAN_CON_ID pWanId, PWAN_PPP_INFO pPppInfo) {
    int ret = DB_WAN_GET_OK;   
    char appPpp[IFC_TINY_LEN];
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiPpp = NULL;

    sprintf(appPpp, "pppsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiPpp = BcmPsi_appOpen(appPpp);      
    status = BcmPsi_objRetrieve(psiPpp, pWanId->conId, pPppInfo,
                                sizeof(WAN_PPP_INFO), &retLen);
    if ( status != PSI_STS_OK )
        ret = DB_WAN_GET_NOT_FOUND;

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setPppSrvInfo
// Description  : store PPP data of the given WAN ID
// Returns      : none.
//**************************************************************************
void BcmDb_setPppSrvInfo(PWAN_CON_ID pWanId, PWAN_PPP_INFO pPppInfo) {
    char appPpp[IFC_TINY_LEN];
    PSI_HANDLE psiPpp = NULL;

    sprintf(appPpp, "pppsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiPpp = BcmPsi_appOpen(appPpp);      
    BcmPsi_objStore(psiPpp, pWanId->conId, pPppInfo, sizeof(WAN_PPP_INFO));
}

//**************************************************************************
// Function Name: BcmDb_getIpSrvInfo
// Description  : retrieve IP data of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getIpSrvInfo(PWAN_CON_ID pWanId, PWAN_IP_INFO pIpInfo) {
    int ret = DB_WAN_GET_OK;   
    char appIp[IFC_TINY_LEN];
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiIp = NULL;

    sprintf(appIp, "ipsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiIp = BcmPsi_appOpen(appIp);      
    status = BcmPsi_objRetrieve(psiIp, pWanId->conId, pIpInfo,
                                sizeof(WAN_IP_INFO), &retLen);
    if ( status != PSI_STS_OK )
        ret = DB_WAN_GET_NOT_FOUND;

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setIpSrvInfo
// Description  : store IP data of the given WAN ID
// Returns      : none.
//**************************************************************************
void BcmDb_setIpSrvInfo(PWAN_CON_ID pWanId, PWAN_IP_INFO pIpInfo) {
    char appIp[IFC_TINY_LEN];
    PSI_HANDLE psiIp = NULL;

    sprintf(appIp, "ipsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiIp = BcmPsi_appOpen(appIp);      
    BcmPsi_objStore(psiIp, pWanId->conId, pIpInfo, sizeof(WAN_IP_INFO));
}
#ifdef SUPPORT_DHCPOPTIONS
int BcmDb_getDhcpOptionInfo(PDHCPOptions pstDhcpOption)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE phDhcpOption = BcmPsi_appOpen(DHCPOption);

    memset(pstDhcpOption, 0, sizeof(DHCPOptions));
    status = BcmPsi_objRetrieve(phDhcpOption, DHCPOption_CONFIG, pstDhcpOption,
                                sizeof(DHCPOptions), &retLen);

    if ( status != PSI_STS_OK )
    {
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}
void BcmDb_setDhcpOptionInfo(PDHCPOptions pstDhcpOption) 
{
    PSI_HANDLE phDhcpOption = BcmPsi_appOpen(DHCPOption);
    
    BcmPsi_objStore(phDhcpOption, DHCPOption_CONFIG, pstDhcpOption, sizeof(DHCPOptions));
}
#endif

/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月11日 */
/*------------------------------------------------------------
  函数原型: int BcmDb_getAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade)
  描述: 从psi中获取自动升级相关信息
  输入: 自动升级结构指针(已分配内存)
  输出: 自动升级相关信息
  返回值: 获取信息是否成功
-------------------------------------------------------------*/
int BcmDb_getAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE phAuUpgrade = BcmPsi_appOpen(APPNAME_AUTOUPGRADE);

    memset(pstAuUpgrade, 0, sizeof(AUTO_UPGRADE_INFO));
    status = BcmPsi_objRetrieve(phAuUpgrade, AUTOUPGRADE_CONFIG, pstAuUpgrade,
                                sizeof(AUTO_UPGRADE_INFO), &retLen);

    if ( status != PSI_STS_OK )
    {
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}
/*------------------------------------------------------------
  函数原型: void BcmDb_setAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade) 
  描述: 设置自动升级的相关信息
  输入: 自动升级数据结构
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void BcmDb_setAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade) 
{
    PSI_HANDLE phAuUpgrade = BcmPsi_appOpen(APPNAME_AUTOUPGRADE);
    
    BcmPsi_objStore(phAuUpgrade, AUTOUPGRADE_CONFIG, pstAuUpgrade, sizeof(AUTO_UPGRADE_INFO));
}
/* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月11日 */


/* start of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
//**************************************************************************
// Function Name: BcmDb_getBrDhcpcInfo
// Description  : retrieve Bridge data of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getBrDhcpcInfo(PWAN_CON_ID pWanId, PWAN_BRDHCPC_INFO pBrInfo)
{
    int ret = DB_WAN_GET_OK;   
    char appBr[IFC_TINY_LEN];
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiBr = NULL;

    memset(pBrInfo, 0, sizeof(WAN_BRDHCPC_INFO));
    sprintf(appBr, "brsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiBr = BcmPsi_appOpen(appBr);      
    status = BcmPsi_objRetrieve(psiBr, pWanId->conId, pBrInfo,
                                sizeof(WAN_BRDHCPC_INFO), &retLen);
    if (PSI_STS_OK != status)
    {
        ret = DB_WAN_GET_NOT_FOUND;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setBrDhcpcInfo
// Description  : store Bridge data of the given WAN ID
// Returns      : none.
//**************************************************************************
void BcmDb_setBrDhcpcInfo(PWAN_CON_ID pWanId, PWAN_BRDHCPC_INFO pBrInfo) 
{
    char appBr[IFC_TINY_LEN];
    PSI_HANDLE psiBr = NULL;

    sprintf(appBr, "brsrv_%d_%d", pWanId->vpi, pWanId->vci);
    psiBr = BcmPsi_appOpen(appBr);      
    BcmPsi_objStore(psiBr, pWanId->conId, pBrInfo, sizeof(WAN_BRDHCPC_INFO));
}
/* end of maintain PSI移植:  允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */

//**************************************************************************
// Function Name: BcmDb_getVccInfo
// Description  : retrieve PVC data of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVccInfo(PWAN_CON_ID pWanId, PIFC_ATM_VCC_INFO pVccInfo) {
    int ret = DB_WAN_GET_NOT_FOUND;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0;
    WAN_CON_INFO wanInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;
    PSI_HANDLE psiVcc = NULL;

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);      
    status = BcmPsi_objRetrieve(psiWan, pWanId->conId, &wanInfo,
                                sizeof(wanInfo), &retLen);
    if ( status == PSI_STS_OK ) {
        psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
        status = BcmPsi_objRetrieve(psiVcc, wanInfo.vccId, pVccInfo,
                                    sizeof(IFC_ATM_VCC_INFO), &retLen);
        if ( status == PSI_STS_OK )
            ret = DB_WAN_GET_OK;
    }
   
    return ret;   
}

/*start of 增加dhcp主机标识功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
void BcmDb_setDhcpHostNameInfo(char *hostName)
{   
    if ( NULL == hostName) 
	{
    	return;
	}
    PSI_HANDLE phDhcp = BcmPsi_appOpen(DHCPHOSTID);
    BcmPsi_objStore(phDhcp, DHCP_CONFIG, hostName,DHCP_HOSTID_LENGTH);
}

int BcmDb_getDhcpHostNameInfo(char *hostName)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE phDhcp = BcmPsi_appOpen(DHCPHOSTID);

    if (NULL == hostName) 
	{
    	return -1;
	}

	status = BcmPsi_objRetrieve(phDhcp, DHCP_CONFIG, hostName,
                               DHCP_HOSTID_LENGTH, &retLen);

	if (status != PSI_STS_OK)
    {
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}
#endif	
/*end of 增加dhcp主机标识功能by l129990 */ 

/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#ifdef SUPPORT_IGMPCONFIG
int BcmDb_getIgmpProxyInfo(PIgmpProxyNode pstIgmpOption)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE phIgmpOption = BcmPsi_appOpen(IGMPPROXY);

    if (NULL == pstIgmpOption) 
	{
    	return -1;
	}

    memset(pstIgmpOption, 0, sizeof(IgmpProxyNode));
    status = BcmPsi_objRetrieve(phIgmpOption, IGMPPROXY_CONFIG, pstIgmpOption,
                                sizeof(IgmpProxyNode), &retLen);

    if ( status != PSI_STS_OK )
    {  
        pstIgmpOption->igmpEnable = atoi(BcmDb_getDefaultValue("enblIgmpFunction"));
        strcpy(pstIgmpOption->igmpInterface, BcmDb_getDefaultValue("enblIgmpInterface"));
        pstIgmpOption->igmpVersion = atoi(BcmDb_getDefaultValue("enblIgmpVersion"));
        pstIgmpOption->igmpGroups = atoi(BcmDb_getDefaultValue("enblIgmpGroups"));
        pstIgmpOption->igmpLogSwitch = atoi(BcmDb_getDefaultValue("enblIgmpLog"));
        BcmPsi_objStore(phIgmpOption, IGMPPROXY_CONFIG, pstIgmpOption, sizeof(IgmpProxyNode));
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}
void BcmDb_setIgmpProxyInfo(PIgmpProxyNode pstIgmpOption) 
{
    if (NULL == pstIgmpOption) 
	{
    	return ;
	}
    
    PSI_HANDLE phIgmpOption = BcmPsi_appOpen(IGMPPROXY);
    BcmPsi_objStore(phIgmpOption, IGMPPROXY_CONFIG, pstIgmpOption, sizeof(IgmpProxyNode)); 
}
#endif
/*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/

//**************************************************************************
// Function Name: BcmDb_getVccInfo
// Description  : retrieve PVC data given the vpi/vci 
// Returns      : vccId if found, otherwise return -1
//**************************************************************************
int BcmDb_getVccInfoByVpiVci(int vpi, int vci, PIFC_ATM_VCC_INFO pVccInfo) {
  int ret = -1;
  char appWan[IFC_TINY_LEN];
  UINT16 retLen = 0;
  WAN_CON_INFO wanInfo;
  PSI_STATUS status = PSI_STS_OK;
  PSI_HANDLE psiWan = NULL;
  PSI_HANDLE psiVcc = NULL;
  WAN_CON_ID wanId;

  wanId.vpi = vpi;
  wanId.vci = vci;
  wanId.conId = 0;
  // if there is no available wanId then return error
  if ( BcmDb_getAvailableWanConId(&wanId) == DB_WAN_ADD_OUT_OF_CONNECTION )
    return -1;

  sprintf(appWan, "wan_%d_%d", vpi, vci);
  psiWan = BcmPsi_appOpen(appWan);      
  status = BcmPsi_objRetrieve(psiWan, wanId.conId, &wanInfo,
                              sizeof(wanInfo), &retLen);
   
  if ( status == PSI_STS_OK ) {
    psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
    status = BcmPsi_objRetrieve(psiVcc, wanInfo.vccId, pVccInfo,
                                sizeof(IFC_ATM_VCC_INFO), &retLen);
    if ( status == PSI_STS_OK )
      ret = wanInfo.vccId;
  }
  
  return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setVccInfo
// Description  : store vccInfo
// Returns      : return 0 if ok, otherwise return -1
//**************************************************************************
int BcmDb_setVccInfo(int vccId, PIFC_ATM_VCC_INFO pVccInfo) {
  int ret = -1;
  UINT16 retLen = 0;
  PSI_STATUS status = PSI_STS_OK;
  PSI_HANDLE psiVcc = NULL;

  psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
  status = BcmPsi_objRetrieve(psiVcc, vccId, pVccInfo,
                              sizeof(IFC_ATM_VCC_INFO), &retLen);
  if ( status == PSI_STS_OK ) {
    ret = 0;
  }
  return ret;   
}

//**************************************************************************
// Function Name: BcmDb_getVccInfoNext
// Description  : retrieve PVC data of the next given VCC ID
// Returns      : DB_VCC_GET_OK or DB_VCC_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVccInfoNext(UINT16 *pVccId, PIFC_ATM_VCC_INFO pVccInfo) {
    int ret = DB_VCC_GET_NOT_FOUND;   
    UINT16 retLen = 0, id = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);

    // pVccInfo should be initialized first
    memset(pVccInfo, 0, sizeof(IFC_ATM_VCC_INFO));
    
    // find the next available vcc
    for ( id = *pVccId + 1; id <= ATM_VCC_MAX; id++ ) {
        status = BcmPsi_objRetrieve(psiVcc, id, pVccInfo,
                                    sizeof(IFC_ATM_VCC_INFO), &retLen);
        if ( status == PSI_STS_OK )        
            break;   // break of id for loop
    }
    
    if ( id <= ATM_VCC_MAX ) {
       *pVccId = id;
       ret = DB_VCC_GET_OK;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_getTdInfo
// Description  : retrieve traffic descriptor data of the given WAN ID
// Returns      : DB_WAN_GET_OK or DB_WAN_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getTdInfo(PWAN_CON_ID pWanId, PIFC_ATM_TD_INFO pTdInfo) {
    int ret = DB_WAN_GET_NOT_FOUND;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0;
    WAN_CON_INFO wanInfo;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;
    PSI_HANDLE psiVcc = NULL;
    PSI_HANDLE psiTd = NULL;

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);      
    status = BcmPsi_objRetrieve(psiWan, pWanId->conId, &wanInfo,
                                sizeof(wanInfo), &retLen);
    if ( status == PSI_STS_OK ) {
        psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
        status = BcmPsi_objRetrieve(psiVcc, wanInfo.vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK ) {
            psiTd = BcmPsi_appOpen(ATM_TD_APPID);
            status = BcmPsi_objRetrieve(psiTd, vccInfo.tdId, pTdInfo,
                                        sizeof(IFC_ATM_TD_INFO), &retLen);
            if ( status == PSI_STS_OK )
                ret = DB_WAN_GET_OK;
        }
    }
   
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_getWanInterfaceName
// Description  : Retrieve WAN interface name of the given WAN ID and its
//                protocol.
// Returns      : None.
//**************************************************************************
void BcmDb_getWanInterfaceName(PWAN_CON_ID pWanId, UINT8 protocol, char *interface) {
    if ( interface == NULL ) return;
    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    WAN_CON_INFO wanInfo;
    #ifdef SUPPORT_VDSL
       //for this function called by many functions, and judge with g_trafficmode or glbWebVar.trafficmode
       //so belows judge may be not correct, we must judge correct in calling function
       if ((g_trafficmode == TM_PTM_VLAN) || (g_trafficmode == TM_PTM_PRTCL) || (g_trafficmode == TM_PTM_BR))
       {
           if ( BcmDb_getWanInfo(pWanId, &wanInfo) == DB_WAN_GET_NOT_FOUND ) return;
       }
    #endif
    switch ( protocol ) {
        case PROTO_PPPOE:
        case PROTO_PPPOA:
#ifdef VDF_PPPOU
    	case PROTO_PPPOU:/*add by sxg, pppousb*/
#endif
            sprintf(interface, "ppp_%d_%d_%d", pWanId->vpi, pWanId->vci, pWanId->conId);
            break;
        case PROTO_IPOA:
            sprintf(interface, "ipa_%d_%d", pWanId->vpi, pWanId->vci);
            break;
#if SUPPORT_ETHWAN
        case PROTO_IPOWAN:
            sprintf(interface, "eth0");
            break;
#endif
        default:
        /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
        #ifdef SUPPORT_VDSL
        if ((g_trafficmode == TM_PTM_VLAN) || (g_trafficmode == TM_PTM_PRTCL) || (g_trafficmode == TM_PTM_BR))
        {
            if( wanInfo.vlanId < 0)
            {
                sprintf(interface, "nas_%d_%d_%d", pWanId->vpi, pWanId->vci,wanInfo.protocol);//PTM lyc  以Protocol区分。
            }
            else
            {
                sprintf(interface, "nas_%d_%d_%d", pWanId->vpi, pWanId->vci,wanInfo.vlanId);
            }
        }
        else
        {
            sprintf(interface, "nas_%d_%d", pWanId->vpi, pWanId->vci);
        }
        #else
            sprintf(interface, "nas_%d_%d", pWanId->vpi, pWanId->vci);
        #endif
        /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
            break;
    }
}

//**************************************************************************
// Function Name: BcmDb_getWanProtocolName
// Description  : Retrieve WAN protocol name of the given protocol ID
// Returns      : None.
//**************************************************************************
void BcmDb_getWanProtocolName(UINT8 protocol, char *name) {
    if ( name == NULL ) return;

    name[0] = '\0';
       
    switch ( protocol ) {
        case PROTO_PPPOE:
            strcpy(name, "PPPoE");
            break;
        case PROTO_PPPOA:
            strcpy(name, "PPPoA");
            break;
        case PROTO_MER:
            strcpy(name, "MER");
            break;
        case PROTO_IPOA:
            strcpy(name, "IPoA");
            break;
        case PROTO_BRIDGE:
            strcpy(name, "Bridge");
            break;
#if SUPPORT_ETHWAN
        case PROTO_IPOWAN:
            strcpy(name, "IPoW");
            break;
#endif
#ifdef VDF_PPPOU
        /*start, add by sxg, pppousb*/
        case PROTO_PPPOU:
        	strcpy(name, "PPPoU");
        	break;
        /*end, add by sxg, pppousb*/
#endif
        default:
            strcpy(name, "Not Applicable");
            break;
    }
}

//**************************************************************************
// Function Name: BcmDb_getNoBridgeNtwkPrtcl
// Description  : get the first wan protocol excluding bridge.
// Returns      : first wan protocol.
//**************************************************************************
int BcmDb_getNoBridgeNtwkPrtcl(void) {
    int ret = PROTO_NONE;
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, vccId = 0, conId = 0, brId = 0;
    WAN_CON_INFO wanInfo;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = NULL;
    PSI_HANDLE psiWan = NULL;

    psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
 
    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK ) {
            sprintf(appWan, "wan_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
            psiWan = BcmPsi_appOpen(appWan);
            for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                            sizeof(wanInfo), &retLen);
                if ( status == PSI_STS_OK ) {
                    if ( wanInfo.protocol == PROTO_BRIDGE )
                        brId = conId;
                    else
                        break;   // break of conId for loop
                }
            }
            if ( conId <= IFC_WAN_MAX )
               break;   // break of vccId for loop
        }
    }

    // cannot find any non-bridged WAN connection
    if ( vccId > ATM_VCC_MAX ) {
        if ( brId != 0 )
            ret = PROTO_BRIDGE;
    } else
        ret = wanInfo.protocol;
      
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isUbrPvcExisted
// Description  : find any existed UBR PVC without QoS enabled
// Returns      : TRUE or FALSE.
//**************************************************************************
int BcmDb_isUbrPvcExisted(void) {
    int ret = FALSE;
    UINT16 retLen = 0, vccId = 0;
    IFC_ATM_VCC_INFO vccInfo;
    IFC_ATM_TD_INFO tdInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
    PSI_HANDLE psiTd = BcmPsi_appOpen(ATM_TD_APPID);

    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK ) { 
            // skip PVC that has QoS enabled
            if ( vccInfo.enblQos == TRUE ) continue;    

			/*Start -- w00135358 add for HG556a 20090328 */
			/*HSPA的serviceCategory是UBR类型，但实际上它不会建ATM队列,因此其他的UBR类型PVC是不能与它共享队列的*/
            if ( (WDF_HSPA_DATA_VPI == vccInfo.vccAddr.usVpi) && (WDF_HSPA_DATA_VCI == vccInfo.vccAddr.usVci))
            {
                /*HSPA的情形则跳过*/
                continue;
            }
			/*End -- w00135358 add for HG556a 20090328 */


			
            // check service category        
            BcmPsi_objRetrieve(psiTd, vccInfo.tdId, &tdInfo, sizeof(tdInfo), &retLen);
            if ( strcmp(tdInfo.serviceCategory, "UBR") == 0 ) {
                ret = TRUE;
                break;  // break for vccId
            }
        }
    }
        
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_getNumberOfUsedPvc
// Description  : get number of current used PVC.
// Parameters   : none.
// Returns      : number of current used PVC.
//**************************************************************************
int BcmDb_getNumberOfUsedPvc(void) {
    int numPvc = 0;
    UINT16 retLen = 0, vccId = 0;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);

    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK )
            numPvc++;   
    }

    return numPvc;
}

//**************************************************************************
// Function Name: BcmDb_getNumberOfUsedQueue
// Description  : get number of current used queues.
// Parameters   : none.
// Returns      : number of current used queues.
//**************************************************************************
#if defined(CHIP_6348) || defined(CHIP_6358)
int BcmDb_getNumberOfUsedQueue(void) {
    int ubrPvcExist = FALSE;
    int numQueue = 0;
    UINT16 retLen = 0, vccId = 0;
    IFC_ATM_VCC_INFO vccInfo;
    IFC_ATM_TD_INFO tdInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);
    PSI_HANDLE psiTd = BcmPsi_appOpen(ATM_TD_APPID);

    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK ) 
        {   

			/*Start -- w00135358 add for HG556a 20090328 */
            if ( (WDF_HSPA_DATA_VPI == vccInfo.vccAddr.usVpi) && (WDF_HSPA_DATA_VCI == vccInfo.vccAddr.usVci))
            {
                /*HSPA的情形则跳过*/
                continue;
            }
			/*End -- w00135358 add for HG556a 20090328 */


		
            /*现有PTM版本都使能QOS*/
            if ( vccInfo.enblQos == FALSE ) 
            {
                BcmPsi_objRetrieve(psiTd, vccInfo.tdId, &tdInfo, sizeof(tdInfo), &retLen);
                if ( strcmp(tdInfo.serviceCategory, "UBR") == 0 ) 
                {
                    if ( ubrPvcExist == FALSE ) 
                    {
                        numQueue++;   // only first UBR PVC uses 1 queue
                        ubrPvcExist = TRUE;
                    }
                } 
                else
                {
                    numQueue++;   // use 1 queue for each shaped PVC
                }
            } 
            else
            {
#ifndef SUPPORT_TC
                /* start of protocol QoS for KPN对于KPN QoS启用4条队列 by zhangchen z45221 2006年7月17日
                numQueue += 3;  // use 3 queues if PVC has QoS enabled
                */
   #ifdef SUPPORT_VDSL
          #ifdef SUPPORT_KPN_QOS  // macro for KPN QoS
                if (TM_ATM_BR == g_trafficmode || TM_PTM_BR == g_trafficmode)
                {
                    numQueue += 4;    // 4 xmit queue per PVC for KPN QoS
                }
                else if(TM_ATM == g_trafficmode)
                {
                    numQueue += 3;    // 3 xmit queue per PVC for QoS
                }
                else 
                {
                    numQueue = 3;
                }
           #else
                if((TM_PTM_VLAN == g_trafficmode) 
                   || (TM_PTM_PRTCL == g_trafficmode)
                   || (TM_PTM_BR == g_trafficmode))
                 {
                      numQueue = 3;
                 }  
                 else
                 {
                      numQueue += 3;
                 } 
          #endif
      #else
                numQueue += 3;  // use 3 queues if PVC has QoS enabled
      #endif
                /* end of protocol QoS for KPN对于KPN QoS启用4条队列 by zhangchen z45221 2006年7月17日 */
#else
                /* start of protocol 规避96358平台QoS缺陷,启用tc进行限速,ATM SAR保证优先级 by z45221 zhangchen 2006年8月21日
                numQueue = 1;  // use 1 queues if PVC has QoS enabled
                */
    #ifdef SUPPORT_TDE_QOS
   /*Start modify : s48571 2006-10-26 for adsl porting*/                  
   #ifdef SUPPORT_VDSL
                if((TM_PTM_VLAN == g_trafficmode) 
                  || (TM_PTM_PRTCL == g_trafficmode)
                  || (TM_PTM_BR == g_trafficmode))
                {
                    numQueue= 3;
                }
                else
                {
                    numQueue += 3;
                }
     #else
     // adsl
                     numQueue += 3;

     #endif
         #else
         #ifdef SUPPORT_VDSL
         // vdsl
                if((TM_PTM_VLAN == g_trafficmode) 
                  || (TM_PTM_PRTCL == g_trafficmode)
                  || (TM_PTM_BR == g_trafficmode))
                {
                     numQueue = 1;
                }
                else
                {
                     numQueue++;
                }  
            #else
     // adsl
     			/*Start -- w00135358 modify for HG556a 20090328*/
                      //numQueue++;
					  numQueue += 3;//enable qos则使用3条队列
				/*End -- w00135358 modify for HG556a*/

     #endif
    #endif
   /*End modify : s48571 2006-10-26 for adsl porting*/  
                /* end of protocol 规避96358平台QoS缺陷,启用tc进行限速,ATM SAR保证优先级 by z45221 zhangchen 2006年8月21日 */
#endif  
            }
        }
    }

/* add start by KF19989: 临时修改，以后需完善 */
    return 3;
/* add end by KF19989: 临时修改，以后需完善 */

}
#else
int BcmDb_getNumberOfUsedQueue(void) {
    int numQueue = 0;
    UINT16 retLen = 0, vccId = 0;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);

    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        if ( status == PSI_STS_OK ) { 
            numQueue++;   // use 1 queue for each PVC
#ifndef SUPPORT_TC
            if ( vccInfo.enblQos == TRUE )
                numQueue += 2;  // use 2 more queues if PVC has QoS enabled
#endif

        }
    }

/* add start by KF19989: 临时修改，以后需完善 */
    return 3;
/* add end by KF19989: 临时修改，以后需完善 */

}
#endif // #if defined(CHIP_6348) || defined(CHIP_6358)

/*****************************************************************************
 函 数 名  : BcmDb_getTotolUsedQueueNum
 功能描述  : 从PSI中获取当前存在的所有service category, 计算已占用的队列总数
             hspa不计算在内
 输入参数  : 无  
 输出参数  : 无
 返 回 值  : 已占用的队列总数
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年3月17日
    作    者   : j00100803
    修改内容   : 新生成函数

*****************************************************************************/
int BcmDb_getTotolUsedQueueNum(void)
{
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;
    IFC_ATM_TD_INFO tdInfo;
	unsigned int ulMaxUsed = 0;
	
    /*初始化变量*/
    memset(&wanId,0,sizeof(wanId));
    memset(&wanInfo,0,sizeof(wanInfo));
    memset(&tdInfo,0,sizeof(tdInfo));

	while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
	    if (wanInfo.protocol == PROTO_PPPOU)
	    {
	        continue;
	    }
	  
	    if(DB_WAN_GET_NOT_FOUND == BcmDb_getTdInfo(&wanId, &tdInfo))
	    {
	        printf("Get tdinfo failed!\n");
	        continue;
	    }
	    if((strstr(tdInfo.serviceCategory, "CBR") != NULL)
			|| (strstr(tdInfo.serviceCategory, "RT_VBR") != NULL))
	    {
	    	ulMaxUsed += 1;
	    }
		else
		{
			ulMaxUsed += 3;
		}
	}
	
	return ulMaxUsed;
}

/*****************************************************************************
 函 数 名  : BcmDb_getQueueNumByPvc
 功能描述  : 从PSI中获取当前存在的所有service category, 
 			 返回指定的pvc占用的队列数
 输入参数  : int iVpi,int iVci
 输出参数  : 无
 返 回 值  : 指定pvc占用的队列数
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年3月17日
    作    者   : j00100803
    修改内容   : 新生成函数

*****************************************************************************/
int BcmDb_getQueueNumByPvc(int iVpi, int iVci)
{
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;
    IFC_ATM_TD_INFO tdInfo;

    /*初始化变量*/
    memset(&wanId,0,sizeof(wanId));
    memset(&wanInfo,0,sizeof(wanInfo));
    memset(&tdInfo,0,sizeof(tdInfo));
    
	while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
	    if (wanInfo.protocol == PROTO_PPPOU)
	    {
	        continue;
	    }
	  
	    if(DB_WAN_GET_NOT_FOUND == BcmDb_getTdInfo(&wanId, &tdInfo))
	    {
	        printf("Get tdinfo failed2!\n");
	        continue;
	    }
        if(wanId.vpi == iVpi && wanId.vci == iVci)
        {
            if((strstr(tdInfo.serviceCategory, "CBR") != NULL)
				|| (strstr(tdInfo.serviceCategory, "RT_VBR") != NULL))
		    {
		    	return 1;
		    }
			else
			{
				return 3;
			}
        }
	}

	return 0;
}


#ifdef SUPPORT_GLB_PVC
/*****************************************************************************
 函 数 名  : BcmDb_checkWan
 功能描述  : 判断是否可以继续添加WAN
 输入参数  : void
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月15日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int BcmDb_checkWan( void )
{
    int pppNum = 0;
    int counter = 0;
    char appPppsrv[IFC_TINY_LEN];
    UINT16 retLen = 0, vccId = 0, conId = 0;
    WAN_PPP_INFO pppInfo;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = NULL;
    PSI_HANDLE psiWan = NULL;
    int ret = DB_WAN_ADD_OUT_OF_PVC;

    psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);

    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) 
    {
        memset(&vccInfo,0,sizeof(vccInfo));
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,sizeof(vccInfo), &retLen);

        if ( status == PSI_STS_OK ) 
        {
            if ( (WDF_HSPA_DATA_VPI == vccInfo.vccAddr.usVpi) && (WDF_HSPA_DATA_VCI == vccInfo.vccAddr.usVci))
            {
                /*HSPA的情形则跳过*/
                continue;
            }
            counter++;
            memset(appPppsrv,0,sizeof(appPppsrv));
            sprintf(appPppsrv, "pppsrv_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
            psiWan = BcmPsi_appOpen(appPppsrv);
            if ( NULL == psiWan)
            {
                printf("==PVC:%s:%s:%d:there is no %s==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv);
            }
            else
            {
                for ( conId = 1,pppNum = 0; conId <= MAX_PPP_ON_PVC; conId++ ) 
                {
                    memset(&pppInfo,0,sizeof(pppInfo));
                    status = BcmPsi_objRetrieve(psiWan, conId, &pppInfo,sizeof(pppInfo), &retLen);
                    if ( status == PSI_STS_OK )
                    {
                        pppNum++;
                        //printf("==PVC:%s:%s:%d:==\n",__FILE__,__FUNCTION__,__LINE__);
                    }
                    else
                    {
                        //printf("==PVC:%s:%s:%d:(%s)status is %d==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv,status);
                    }
                }
                //printf("==PVC:%s:%s:%d:%s has %d PPP==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv,pppNum);
                if ( (pppNum > 0) && (pppNum < MAX_PPP_ON_PVC))
                {
                    /*如果是PPP的连接，而又没有达到同一PVC可配的最大PPP数时
                      ，允许继续添加*/
                    return DB_WAN_ADD_OK;
                }
            }
        }
    }

    //printf("==PVC:%s:%s:%d:counter is %d==\n",__FILE__,__FUNCTION__,__LINE__,counter);
    
    /*counter没有计算HSPA的wan*/
    //if ( counter < (ATM_VCC_MAX - 1))
    if ( (counter < sysGetNumofUnUsedMac()) && (counter < (ATM_VCC_MAX - 1)))   
    {
        /*如果pvc的个数没达到可以MAC数，也可以继续增加*/
        return DB_WAN_ADD_OK;
    }

    return DB_WAN_ADD_OUT_OF_PVC;
}

/*****************************************************************************
 函 数 名  : BcmDb_getNumberOfPppOnOnePvc
 功能描述  : 同一条pvc上PPP的数量
 输入参数  : PWAN_CON_ID pWanId 
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月27日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int BcmDb_getNumberOfPppOnOnePvc( PWAN_CON_ID pWanId  )
{
    int pppNum = 0;
    char appPppsrv[IFC_TINY_LEN];
    UINT16 retLen = 0, conId = 0;
    WAN_PPP_INFO pppInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;

    memset(appPppsrv,0,sizeof(appPppsrv));
    sprintf(appPppsrv, "pppsrv_%d_%d", pWanId->vpi,  pWanId->vci);
    psiWan = BcmPsi_appOpen(appPppsrv);
    if ( NULL == psiWan)
    {
        printf("==PVC:%s:%s:%d:there is no %s==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv);
    }
    else
    {
        for ( conId = 1,pppNum = 0; conId <= MAX_PPP_ON_PVC; conId++ ) 
        {
            memset(&pppInfo,0,sizeof(pppInfo));
            status = BcmPsi_objRetrieve(psiWan, conId, &pppInfo,sizeof(pppInfo), &retLen);
            if ( status == PSI_STS_OK )
            {
                pppNum++;
                //printf("==PVC:%s:%s:%d:==\n",__FILE__,__FUNCTION__,__LINE__);
            }
            else
            {
                printf("==PVC:%s:%s:%d:(%s)status is %d==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv,status);
            }
        }
        //printf("==PVC:%s:%s:%d:%s has %d PPP==\n",__FILE__,__FUNCTION__,__LINE__,appPppsrv,pppNum);
    }

    return pppNum;
}
    
#endif /* SUPPORT_GLB_PVC */

//**************************************************************************
// Function Name: BcmDb_getNumberOfWanConnections
// Description  : get number of WAN connections.
// Returns      : number of WAN connections.
//**************************************************************************
int BcmDb_getNumberOfWanConnections(void) {
    int counter = 0;
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, vccId = 0, conId = 0;
    WAN_CON_INFO wanInfo;
    IFC_ATM_VCC_INFO vccInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiVcc = NULL;
    PSI_HANDLE psiWan = NULL;

    psiVcc = BcmPsi_appOpen(ATM_VCC_APPID);

    /* start of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */
#if 0
        if ((TM_PTM_VLAN == g_trafficmode) 
        || (TM_PTM_PRTCL == g_trafficmode)
        || (TM_PTM_BR == g_trafficmode))
        {
            counter = -1;
        }
#endif
    /* end of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */
 
    for ( vccId = 1; vccId <= ATM_VCC_MAX; vccId++ ) {
        status = BcmPsi_objRetrieve(psiVcc, vccId, &vccInfo,
                                    sizeof(vccInfo), &retLen);
        /* start of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */
#ifdef SUPPORT_TDE_QOS
		// PTM
        if ((TM_PTM_VLAN == g_trafficmode) 
           || (TM_PTM_PRTCL == g_trafficmode)
           || (TM_PTM_BR == g_trafficmode))
        {
            if ( status == PSI_STS_OK ) {
                sprintf(appWan, "wan_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
                psiWan = BcmPsi_appOpen(appWan);
                for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                    status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                sizeof(wanInfo), &retLen);
                    if ( (status == PSI_STS_OK) && (wanInfo.vccId == vccId))
                    {
                        counter++;
                    }
                }
            }
        }
        else
        {
            if ( status == PSI_STS_OK ) {
                sprintf(appWan, "wan_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
                psiWan = BcmPsi_appOpen(appWan);
                for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                    status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                sizeof(wanInfo), &retLen);
                    if ( status == PSI_STS_OK )
                        counter++;
                }
            }
        }
#else
        if ( status == PSI_STS_OK ) {
            sprintf(appWan, "wan_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
            psiWan = BcmPsi_appOpen(appWan);
            for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                            sizeof(wanInfo), &retLen);
                if ( status == PSI_STS_OK )
                    counter++;
            }
        }
#endif
        /* end of protocol QoS for TdE by zhangchen z45221 2006年8月25日 */
        #if 0
        if ( status == PSI_STS_OK ) {
            sprintf(appWan, "wan_%d_%d", vccInfo.vccAddr.usVpi, vccInfo.vccAddr.usVci);
            psiWan = BcmPsi_appOpen(appWan);
            for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
                status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                            sizeof(wanInfo), &retLen);
                if ( status == PSI_STS_OK )
                    counter++;
            }
        }
        #endif
    }

    return counter;
}

//**************************************************************************
// Function Name: BcmDb_checkPvc
// Description  : check PVC and queue when new PVC is added.
// Returns      : status.
//**************************************************************************
int BcmDb_checkPvc(void) {
   int ret = DB_WAN_ADD_OUT_OF_PVC;

    #ifdef SUPPORT_GLB_PVC
    if ( DB_WAN_ADD_OUT_OF_PVC == BcmDb_checkWan( ))
    {
        ret = DB_WAN_ADD_OUT_OF_PVC;
    }
    #else
    if ( BcmDb_getNumberOfUsedPvc() >= ATM_VCC_MAX )
    {
        ret = DB_WAN_ADD_OUT_OF_PVC;
    }
    #endif /* SUPPORT_GLB_PVC */
   else {
#if defined(CHIP_6348) || defined(CHIP_6358)
      if ( BcmDb_getNumberOfUsedQueue() >= ATM_QUEUE_MAX && BcmDb_isUbrPvcExisted() == FALSE )
#else
      if ( BcmDb_getNumberOfUsedQueue() >= ATM_QUEUE_MAX )
#endif
         ret = DB_WAN_ADD_OUT_OF_QUEUES;
      else
         ret = DB_WAN_ADD_OK;
   }

   return ret;
}

//**************************************************************************
// Function Name: BcmDb_isBridgedWanExisted
// Description  : find any existed bridged WAN connection.
// Returns      : TRUE or FALSE.
//**************************************************************************
int BcmDb_isBridgedWanExisted() {
    int ret = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == TRUE &&
             wanInfo.protocol == PROTO_BRIDGE ) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isRoutedWanExisted
// Description  : find any existed routed WAN connection.
// Returns      : TRUE or FALSE.
//**************************************************************************
int BcmDb_isRoutedWanExisted() {
    int ret = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == TRUE &&
             wanInfo.protocol != PROTO_BRIDGE &&
             wanInfo.protocol != PROTO_NONE ) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isFirewallEnabled
// Description  : check to see any WAN has firewall enabled.
// Returns      : TRUE Or FALSE.
//**************************************************************************
int BcmDb_isFirewallEnabled(void) {
    int ret = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == FALSE ) continue;
        if ( wanInfo.flag.firewall == TRUE ) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isInterfaceFirewallEnabled
// Description  : check to see remote wan interface has firewall enabled.
// Returns      : TRUE Or FALSE.
//**************************************************************************
int BcmDb_isInterfaceFirewallEnabled(char *remote) {
    int ret = FALSE;
    char interface[IFC_TINY_LEN];
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    if ( remote == NULL || remote[0] == '\0' )
         return FALSE;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == FALSE ) continue;
        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
        if ( strcmp(interface, remote) == 0 ) {
            if ( wanInfo.flag.firewall == TRUE ) {
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isNatEnabled
// Description  : check to see any WAN has NAT enabled.
// Returns      : TRUE Or FALSE.
//**************************************************************************
int BcmDb_isNatEnabled(void) {
    int ret = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == FALSE ) continue;
        if ( wanInfo.flag.nat == TRUE ) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_isInterfaceNatEnabled
// Description  : check to see any WAN has NAT enabled.
// Returns      : TRUE Or FALSE.
//**************************************************************************
int BcmDb_isInterfaceNatEnabled(char *remote) {
    int ret = FALSE;
    char interface[IFC_TINY_LEN];
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    if ( remote == NULL || remote[0] == '\0' )
         return FALSE;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == FALSE ) continue;
        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
        if ( strcmp(interface, remote) == 0 ) {
            if ( wanInfo.flag.nat == TRUE ) {
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}
/*start of删除brcom igmp proxy 使能功能,重新设置igmp proxy使能功能by l129990 ,2008,9,28*/
#if 0
/***************************************************************************
// Function Name: BcmDb_isIgmpEnabled.
// Description  : check to see any wan connection has IGMP enabled.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isIgmpEnabled(void) {
    int ret = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
        if ( wanInfo.flag.service == FALSE ) continue;
        if ( wanInfo.protocol != PROTO_BRIDGE &&
            wanInfo.flag.igmp == TRUE ) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}
#endif
/***************************************************************************
// Function Name: BcmDb_isIgmpEnabled.
// Description  : check to see a wan connection has IGMP enabled.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isIgmpEnabled(void) {
    int ret = FALSE;
    IgmpProxyNode igmpProxy;
    
    BcmDb_getIgmpProxyInfo(&igmpProxy);
    
    if ((1 == igmpProxy.igmpEnable) && (NULL != strcmp(igmpProxy.igmpInterface, "none")))
    {
        ret = TRUE;
    }
    
    return ret;
}
/*end of删除brcom igmp proxy 使能功能,重新设置igmp proxy使能功能by l129990 ,2008,9,28*/
/***************************************************************************
// Function Name: BcmDb_isQosEnabled.
// Description  : check any PVC that has Qos enabled.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isQosEnabled(void) {
    int enabled = FALSE;
    UINT16 vccId = 0;   // set to 0 to search vcc from beginning
    IFC_ATM_VCC_INFO vccInfo;
   
    while ( BcmDb_getVccInfoNext(&vccId, &vccInfo) == DB_VCC_GET_OK ) {
        if ( vccInfo.enblQos == TRUE ) {
            enabled = TRUE;
            break;
        }
    }

    return enabled;
}

/***************************************************************************
// Function Name: BcmDb_isWanExisted.
// Description  : check any existed WAN connection that has the given WAN ID,
//                and the given protocol.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isWanExisted(PWAN_CON_ID pWanId, int protocol) {
    int existed = FALSE;
    WAN_CON_ID id;
    WAN_CON_INFO wanInfo;
   
    memcpy(&id, pWanId, sizeof(id));

    // if don't care protocol then find the first WAN connection
    // that has the same VPI/VCI with WAN ID
    if ( protocol == PROTO_NONE ) {
        if ( BcmDb_getWanInfoNext(&id, &wanInfo) == DB_WAN_GET_OK ) {
            if ( id.vpi == pWanId->vpi && id.vci == pWanId->vci )
                existed = TRUE;
        }
    } else {
        while ( BcmDb_getWanInfoNext(&id, &wanInfo) == DB_WAN_GET_OK ) {
            if ( id.vpi != pWanId->vpi || id.vci != pWanId->vci )
                break;
            if ( wanInfo.protocol == protocol ) {
                existed = TRUE;
                break;
            }
        }
    }

    return existed;
}

/***************************************************************************
// Function Name: BcmDb_isMultiplePppoeExisted.
// Description  : are there multiple PPPoE sessions on the given WAN ID?
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isMultiplePppoeExisted(PWAN_CON_ID pWanId) {
    int existed = FALSE, count = 0;
    WAN_CON_ID id;
    WAN_CON_INFO wanInfo;
   
    memcpy(&id, pWanId, sizeof(id));

    while ( BcmDb_getWanInfoNext(&id, &wanInfo) == DB_WAN_GET_OK ) {
        if ( id.vpi != pWanId->vpi || id.vci != pWanId->vci )
            break;
        if ( wanInfo.protocol == PROTO_PPPOE )
            count++;
    }

    if ( count > 1 )
        existed = TRUE;
        
    return existed;
}

//**************************************************************************
// Function Name: BcmDb_isWanConnectionExisted
// Description  : check any existed connection on the given WAN Id
// Returns      : TRUE or FALSE.
//**************************************************************************
int BcmDb_isWanConnectionExisted(PWAN_CON_ID pWanId) {
    int ret = FALSE;   
    char appWan[IFC_TINY_LEN];
    UINT16 retLen = 0, conId = 0;
    WAN_CON_INFO wanInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE psiWan = NULL;

    sprintf(appWan, "wan_%d_%d", pWanId->vpi, pWanId->vci);
    psiWan = BcmPsi_appOpen(appWan);     
    
    for ( conId = 1; conId <= IFC_WAN_MAX; conId++ ) {
        status = BcmPsi_objRetrieve(psiWan, conId, &wanInfo,
                                    sizeof(wanInfo), &retLen);
        if ( status == PSI_STS_OK )
            break;
    }
    if ( conId <= IFC_WAN_MAX )
        ret = TRUE;
            
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_isFirstPppoeOnPvc
// Description  : check the given WAN Id is first PPPoE on given PVC
// Returns      : TRUE or FALSE.
//**************************************************************************
int BcmDb_isFirstPppoeOnPvc(PWAN_CON_ID pWanId) {
    int first = FALSE;
    WAN_CON_ID id;
    WAN_CON_INFO wanInfo;
   
    memcpy(&id, pWanId, sizeof(id));
    id.conId = 0;   // to search from the begining of the given PVC

    // search for the first PPPoE on the given PVC
    while ( BcmDb_getWanInfoNext(&id, &wanInfo) == DB_WAN_GET_OK ) {
        if ( id.vpi != pWanId->vpi || id.vci != pWanId->vci )
            break;
        if ( wanInfo.protocol == PROTO_PPPOE )
            break;
    }

    if ( id.vpi == pWanId->vpi && 
         id.vci == pWanId->vci &&
         id.conId == pWanId->conId )
        first = TRUE;
        
    return first;
}

/***************************************************************************
// Function Name: BcmDb_getNewNtwkInstId.
// Description  : Get a new, unique instance id.
// Parameters   : None.
// Returns      : instance id
****************************************************************************/
UINT32 BcmDb_getNewNtwkInstId(void) {
    IFC_LAN_INFO lanInfo;
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE handle = BcmPsi_appOpen(LAN_APPID);

    status = BcmPsi_objRetrieve(handle, INSTID_OBJID, &lanInfo, sizeof(lanInfo),
        &retLen);
    if ( status != PSI_STS_OK ||
         (lanInfo.id & INSTID_VALID_VALUE) != INSTID_VALID_VALUE ) {
        memset(&lanInfo, 0x00, sizeof(lanInfo));
        lanInfo.id = INSTID_VALID_VALUE | 1;
        lanInfo.lanAddress.s_addr = 0x01010101;
        lanInfo.lanMask.s_addr = 0xffffff00;
        BcmPsi_objStore(handle, INSTID_OBJID, &lanInfo, sizeof(lanInfo));
        BcmPsi_flush();
    } else {
        lanInfo.id++;
        BcmPsi_objStore(handle, INSTID_OBJID, &lanInfo, sizeof(lanInfo));
    }

    return( lanInfo.id );
}

//**************************************************************************
// Function Name: BcmDb_getLanInfo
// Description  : retrieve Lan data and object as well as all the configuration
//                related to this Lan interface
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getLanInfo(UINT16 lanId, PIFC_LAN_INFO pLanInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);

    memset(pLanInfo, 0, sizeof(IFC_LAN_INFO));
   
    status = BcmPsi_objRetrieve(lanPsi, lanId, pLanInfo,
                                sizeof(IFC_LAN_INFO), &retLen);
    if ( status != PSI_STS_OK ) {
        if ( lanId == IFC_ENET_ID ) {
            pLanInfo->lanAddress.s_addr = inet_addr(BcmDb_getDefaultValue("ethIpAddress"));
            pLanInfo->lanMask.s_addr = inet_addr(BcmDb_getDefaultValue("ethSubnetMask"));
            pLanInfo->dhcpSrv.enblDhcpSrv = atoi(BcmDb_getDefaultValue("enblDhcpSrv"));
            pLanInfo->dhcpSrv.leasedTime = atoi(BcmDb_getDefaultValue("dhcpLeasedTime"));
            /*start of support to the dhcp relay  function by l129990,2009,11,12*/
            pLanInfo->dhcpRelay = atoi(BcmDb_getDefaultValue("dhcpRelay"));
            /*end of support to the dhcp relay  function by l129990,2009,11,12*/
            pLanInfo->dhcpSrv.startAddress.s_addr = inet_addr(BcmDb_getDefaultValue("dhcpEthStart"));
            pLanInfo->dhcpSrv.endAddress.s_addr = inet_addr(BcmDb_getDefaultValue("dhcpEthEnd"));
       #ifdef SUPPORT_PORTMAPING
            strcpy(pLanInfo->dhcpSrv.option60,  BcmDb_getDefaultValue("option60str"));
#endif
#ifdef VDF_RESERVED
            memset(pLanInfo->dhcpSrv.reservedAddress,0,DHCP_RESERVED_MAXCHAR);
            strcpy(pLanInfo->dhcpSrv.reservedAddress,BcmDb_getDefaultValue("ReservedIpstr"));
#endif
#ifdef SUPPORT_VDF_DHCP
            pLanInfo->dhcpSrv.dhcpNtpAddr1.s_addr = 0;
            pLanInfo->dhcpSrv.dhcpNtpAddr2.s_addr = 0;
            memset(pLanInfo->szLanOption43, 0, MAX_OPTION43_LEN);
#endif
            /* BEGIN: Added by y67514, 2008/9/22   PN:GLB:lan dns*/
            strcpy(pLanInfo->lanDomain,  BcmDb_getDefaultValue("landomain"));
            /* END:   Added by y67514, 2008/9/22 */
        /*start of support to configure the option66,67,160 by l129990,2009,12,22*/
            strcpy(pLanInfo->szLanOption66,  BcmDb_getDefaultValue("lanOption66"));
            strcpy(pLanInfo->szLanOption67,  BcmDb_getDefaultValue("lanOption67"));
            strcpy(pLanInfo->szLanOption160,  BcmDb_getDefaultValue("lanOption160"));
        /*end of support to configure the option66,67,160 by l129990,2009,12,22*/
        }        
        else
            ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setLanInfo
// Description  : store Lan data and object as well as all the configuration
//                related to this lan interface
// Returns      : none.
//**************************************************************************
void BcmDb_setLanInfo(UINT16 lanId, PIFC_LAN_INFO pLanInfo) {
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);
    
    if( (pLanInfo->id & INSTID_VALID_MASK) != INSTID_VALID_VALUE )
       pLanInfo->id = BcmDb_getNewNtwkInstId();

    BcmPsi_objStore(lanPsi, lanId, pLanInfo, sizeof(IFC_LAN_INFO));
}

//**************************************************************************
// Function Name: BcmDb_removeLanInfo
// Description  : remove Lan data and object as well as all the configuration
//                related to this lan interface
// Returns      : none.
//**************************************************************************
void BcmDb_removeLanInfo(UINT16 lanId) {
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);

    BcmPsi_objRemove(lanPsi, lanId);
}

//**************************************************************************
// Function Name: BcmDb_getLanAddrMask
// Description  : retrieve IP address and subnetmask of IFC_ENET_ID interface
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getLanAddrMask(char *addr, char *mask) {
    int ret = DB_GET_NOT_FOUND;   
    UINT16 retLen = 0;
    IFC_LAN_INFO lanInfo;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);

    if ( addr == NULL || mask == NULL) return ret;
    
    addr[0] = mask[0] = '\0';
   
    status = BcmPsi_objRetrieve(lanPsi, IFC_ENET_ID, &lanInfo,
                                sizeof(lanInfo), &retLen);
    if ( status != PSI_STS_OK ) {
        strcpy(addr, BcmDb_getDefaultValue("ethIpAddress"));
        strcpy(mask, BcmDb_getDefaultValue("ethSubnetMask"));
    } else {
        strcpy(addr, inet_ntoa(lanInfo.lanAddress));
        strcpy(mask, inet_ntoa(lanInfo.lanMask));
        ret = DB_GET_OK;
    }
    
    return ret;   
}
    
//**************************************************************************
// Function Name: BcmDb_getNumberOfLanConnections
// Description  : get number of LAN connections.
// Returns      : number of LAN connections.
//**************************************************************************
int BcmDb_getNumberOfLanConnections(void) {
   int num = 0, index = 0;
   char ifcName[IFC_TINY_LEN];
   IFC_LAN_INFO lanInfo;
   
   // USB
   index = 0;
   bcmGetIfcNameById(IFC_USB_ID + index, ifcName);
   while ( bcmIsValidIfcName(ifcName) == TRUE &&
           index < IFC_LAN_MAX ) {
      index++;
      bcmGetIfcNameById(IFC_USB_ID + index, ifcName);
   }
   num += index;
   // Ethernet
   index = 0;
   bcmGetIfcNameById(IFC_ENET_ID + index, ifcName);
   while ( bcmIsValidIfcName(ifcName) == TRUE &&
           index < IFC_LAN_MAX ) {
      index++;
      bcmGetIfcNameById(IFC_ENET_ID + index, ifcName);
   }
   num += index;

   // LAN 2
   if ( BcmDb_getLanInfo(IFC_SUBNET_ID, &lanInfo) == DB_GET_OK )
      num++;
      
   // Wireless
   bcmGetIfcNameById(IFC_WIRELESS_ID, ifcName);  
   if( bcmIsValidIfcName(ifcName) == TRUE )
      num++;
      
   return num;
}

#ifdef ETH_CFG
//**************************************************************************
// Function Name: BcmDb_getEthInfo
// Description  : retrieve Ethernet configurations data
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getEthInfo(PETH_CFG_INFO pEthInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);

    memset(pEthInfo, 0, sizeof(ETH_CFG_INFO));
   
    status = BcmPsi_objRetrieve(lanPsi, ETH_CFG_ID, pEthInfo,
                                sizeof(ETH_CFG_INFO), &retLen);
    if ( status != PSI_STS_OK ) {
        pEthInfo->ethSpeed = atoi(BcmDb_getDefaultValue("ethSpeed"));
        pEthInfo->ethType = atoi(BcmDb_getDefaultValue("ethType"));
        pEthInfo->ethMtu = atoi(BcmDb_getDefaultValue("ethMtu"));
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setEthInfo
// Description  : store ethernet configurations data
// Returns      : none.
//**************************************************************************
void BcmDb_setEthInfo(PETH_CFG_INFO pEthInfo) {
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);

    BcmPsi_objStore(lanPsi, ETH_CFG_ID, pEthInfo, sizeof(ETH_CFG_INFO));
}
#endif

//**************************************************************************
// Function Name: BcmDb_getEthInfo
// Description  : retrieve Ethernet configurations data
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getEthInfoEx(PETH_CFG_INFO pEthInfo, UINT32 ethId) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);
    UINT16 cfgId = ETH_CFG_ID + (ethId - IFC_ENET_ID);

    memset(pEthInfo, 0, sizeof(ETH_CFG_INFO));
   
    status = BcmPsi_objRetrieve(lanPsi, cfgId, pEthInfo,
                                sizeof(ETH_CFG_INFO), &retLen);
    if ( status != PSI_STS_OK ) {
        pEthInfo->ethSpeed = atoi(BcmDb_getDefaultValue("ethSpeed"));
        pEthInfo->ethType = atoi(BcmDb_getDefaultValue("ethType"));
        pEthInfo->ethMtu = atoi(BcmDb_getDefaultValue("ethMtu"));
        pEthInfo->ethIfcDisabled = FALSE;
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setEthInfo
// Description  : store ethernet configurations data
// Returns      : none.
//**************************************************************************
void BcmDb_setEthInfoEx(PETH_CFG_INFO pEthInfo, UINT32 ethId) {
    PSI_HANDLE lanPsi = BcmPsi_appOpen(LAN_APPID);
    UINT16 cfgId = ETH_CFG_ID + (ethId - IFC_ENET_ID);

    BcmPsi_objStore(lanPsi, cfgId, pEthInfo, sizeof(ETH_CFG_INFO));
}

//**************************************************************************
// Function Name: BcmDb_getSysLogInfo
// Description  : retrieve system log configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
void BcmDb_getSysLogDefaultInfo(BCM_SYSLOG_CB *pSysLog) {
    pSysLog->status = atoi(BcmDb_getDefaultValue("logStatus"));
    pSysLog->option = atoi(BcmDb_getDefaultValue("logMode"));
    pSysLog->serverPort = atoi(BcmDb_getDefaultValue("logPort"));
    pSysLog->localDisplayLevel = atoi(BcmDb_getDefaultValue("logDisplay"));
    pSysLog->localLogLevel = atoi(BcmDb_getDefaultValue("logLevel"));
    pSysLog->remoteLogLevel = atoi(BcmDb_getDefaultValue("logLevel"));
    /* start of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日
    pSysLog->serverIP.s_addr = inet_addr(BcmDb_getDefaultValue("logIpAddress"));
    */
    strcpy(pSysLog->serverAddr, BcmDb_getDefaultValue("logIpAddress"));
    /* end of maintain log server允许设置域名 by liuzhijie 00028714 2006年7月11日 */
    /*add by z67625防火墙日志写flash开关默认值获取 start*/
    pSysLog->fwlog = atoi(BcmDb_getDefaultValue("fwlog"));
    /*add by z67625防火墙日志写flash开关默认值获取 end*/

    /*start of 增加系统日志保存FLASH开关默认值获取by c00131380 at 080926*/
    pSysLog->Syslog = atoi(BcmDb_getDefaultValue("Syslog"));
    /*end of 增加系统日志保存FLASH开关默认值获取by c00131380 at 080926*/
}

//BEGIN:add by zhourongfei at 2011/05/30
//**************************************************************************
// Function Name: BcmDb_getWebDefaultInfo
// Description  : retrieve WEB default configurations
// Returns      :
//**************************************************************************
void BcmDb_getWebDefaultInfo(WEB_CFG_INFO *pWebInfo) {
    pWebInfo->LANDisplayCFG= atoi(BcmDb_getDefaultValue("LANDisplayCFG"));
    pWebInfo->PINDisplayCFG= atoi(BcmDb_getDefaultValue("PINDisplayCFG"));
    pWebInfo->HSPADisplayCFG= atoi(BcmDb_getDefaultValue("HSPADisplayCFG"));
}
//END:add by zhourongfei at 2011/05/30
/*start of 增加tr143 功能*/
void  Bcm_Db_getUdpEchoDefaultInfo(DIAG_TR143_UDP * flag)
{
    flag->Enable = atoi(BcmDb_getDefaultValue("UdpEnable"));
    flag->EchoPlusEnabled = atoi(BcmDb_getDefaultValue("UdpEchoPlusEnabled"));
    strcpy(flag->Interface, BcmDb_getDefaultValue("UdpInterface"));
    strcpy(flag->SourceIPAddress, BcmDb_getDefaultValue("UdpSourceIPAddress"));
    flag->UDPPort = atoi(BcmDb_getDefaultValue("UdpPort"));
}
/* end of 增加tr143 功能*/

/*start of apply to the old settings by l129990,2010-1-30*/
void  Bcm_Db_getUploadDefaultInfo(DIAG_TR143_UP * flag)
{ 
    strcpy(flag->diag_state_up, BcmDb_getDefaultValue("UploadDiagState"));
    strcpy(flag->diag_inter_up, BcmDb_getDefaultValue("UploadInterface"));
    strcpy(flag->diag_upURL, BcmDb_getDefaultValue("UploadURL"));
    flag->diag_dscp_up = atoi(BcmDb_getDefaultValue("UploadDSCP"));
    flag->diag_ether_up = atoi(BcmDb_getDefaultValue("UploadEthPriority"));
    flag->diag_test_up = atoi(BcmDb_getDefaultValue("UploadTestFileLen"));
}

void Bcm_Db_getDownDiagInfo(DIAG_TR143_Down * flag)
{
    strcpy(flag->diag_state, BcmDb_getDefaultValue("DownDiagState"));
    strcpy(flag->diag_inter, BcmDb_getDefaultValue("DownInterface"));
    strcpy(flag->diag_downURL, BcmDb_getDefaultValue("DownloadURL"));
    flag->diag_dscp = atoi(BcmDb_getDefaultValue("DownDSCP"));
    flag->diag_etherPriority = atoi(BcmDb_getDefaultValue("DownEthPriority"));
}
/*end of apply to the old settings by l129990,2010-1-30*/

//**************************************************************************
// Function Name: BcmDb_getSysLogInfo
// Description  : retrieve system log configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSysLogInfo(BCM_SYSLOG_CB *pSysLog) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pSysLog, 0, sizeof(BCM_SYSLOG_CB));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_LOG_ID,
        pSysLog, sizeof(BCM_SYSLOG_CB), &retLen);

    if ( status != PSI_STS_OK ) {
        BcmDb_getSysLogDefaultInfo(pSysLog);
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setSysLogInfo
// Description  : store system log configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setSysLogInfo(BCM_SYSLOG_CB *pSysLog) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_LOG_ID, pSysLog, sizeof(BCM_SYSLOG_CB));
}

//BEGIN:add by zhourongfei to config web at 2011/05/30
//**************************************************************************
// Function Name: BcmDb_getWebInfo
// Description  : retrieve WEB configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getWebInfo(WEB_CFG_INFO *pWebInfo) {
	
    int ret = DB_GET_OK;  
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pWebInfo, 0, sizeof(WEB_CFG_INFO));
    status = BcmPsi_objRetrieve(sysPsi, SYS_WEB_ID,
        pWebInfo, sizeof(WEB_CFG_INFO), &retLen);
    if ( status != PSI_STS_OK ) {
        BcmDb_getWebDefaultInfo(pWebInfo);
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setWebInfo
// Description  : store WEB configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setWebInfo(WEB_CFG_INFO *pWebInfo) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    BcmPsi_objStore(sysPsi, SYS_WEB_ID, pWebInfo, sizeof(WEB_CFG_INFO));
}
//END:add by zhourongfei to config web at 2011/05/30

#ifdef SUPPORT_SNMP
//**************************************************************************
// Function Name: BcmDb_getSnmpInfo
// Description  : retrieve snmp configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSnmpInfo(BCM_SNMP_STRUCT *pSnmp) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pSnmp, 0, sizeof(BCM_SNMP_STRUCT));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_SNMP_ID, pSnmp,
        sizeof(BCM_SNMP_STRUCT), &retLen);

    if ( status != PSI_STS_OK ) {
        pSnmp->status = atoi(BcmDb_getDefaultValue("snmpStatus"));
        strcpy(pSnmp->roCommunity, BcmDb_getDefaultValue("snmpRoCommunity"));
        strcpy(pSnmp->rwCommunity, BcmDb_getDefaultValue("snmpRwCommunity"));
        strcpy(pSnmp->sysName, BcmDb_getDefaultValue("snmpSysName"));
        strcpy(pSnmp->sysLocation, BcmDb_getDefaultValue("snmpSysLocation"));
        strcpy(pSnmp->sysContact, BcmDb_getDefaultValue("snmpSysContact"));
        pSnmp->trapIp.s_addr = inet_addr(BcmDb_getDefaultValue("snmpTrapIp"));
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setSnmpInfo
// Description  : store snmp configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setSnmpInfo(BCM_SNMP_STRUCT *pSnmp) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_SNMP_ID, pSnmp, sizeof(BCM_SNMP_STRUCT));
}

//**************************************************************************
// Function Name: BcmDb_setSnmpInfo
// Description  : store snmp specific information; specific specifies which 
//                information that needs that be set
// Returns      : none.
//**************************************************************************
void BcmDb_setSnmpSpecificInfo(char* newValue, int len, int specific) 
{
    BCM_SNMP_STRUCT snmp;
    char tmpBuf[IFC_STRING_LEN];
    
    /*start of TA B010 超过255个字符处理 by c00131380 at 20090225*/    
    BcmDb_getSnmpInfo(&snmp);
    
    strcpy(tmpBuf,newValue);
    
    switch (specific)
    {
      case BCM_SNMP_SYS_NAME:        
        strcpy(snmp.sysName,tmpBuf);      
        break;
      case BCM_SNMP_SYS_LOCATION:
        strcpy(snmp.sysLocation,tmpBuf);      
        break;
      case BCM_SNMP_SYS_CONTACT:
        strcpy(snmp.sysContact,tmpBuf);
        break;
      default:
        break;
    }
    tmpBuf[0] = '\0';
    
    BcmDb_setSnmpInfo(&snmp);
    /*end of TA B010 超过255个字符处理 by c00131380 at 20090225*/

    // flush from profile to flash
    BcmPsi_flush();
}
#endif

#ifdef SUPPORT_TR69C
//**************************************************************************
// Function Name: BcmDb_getTr69cInfo
// Description  : retrieve TR69c configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getTr69cInfo(PBCM_TR69C_INFO pTr69c) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pTr69c, 0, sizeof(BCM_TR69C_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_TR69C_ID, pTr69c,
        sizeof(BCM_TR69C_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
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
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setTr69cInfo
// Description  : store TR69C configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setTr69cInfo(PBCM_TR69C_INFO pTr69c) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_TR69C_ID, pTr69c, sizeof(BCM_TR69C_INFO));
}

#endif

#ifdef SUPPORT_GLB_ALG
//**************************************************************************
// Function Name: BcmDb_getNatAlgInfo
// Description  : retrieve NATALG configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getNatAlgInfo(PNATALGINFO pNatAlgInfo) 
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;

    if ( NULL == pNatAlgInfo)
    {
        return DB_GET_NOT_FOUND;
    }
    
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    memset(pNatAlgInfo, 0, sizeof(NATALGINFO));
   
    status = BcmPsi_objRetrieve(sysPsi, NAT_ALG, pNatAlgInfo,
        sizeof(NATALGINFO), &retLen);

    if ( status != PSI_STS_OK ) 
    {
        pNatAlgInfo->H323Alg = atoi(BcmDb_getDefaultValue("enblH323"));
        pNatAlgInfo->RtspAlg= atoi(BcmDb_getDefaultValue("enblRtsp"));
        pNatAlgInfo->FtpAlg= atoi(BcmDb_getDefaultValue("enblFtp"));
        pNatAlgInfo->TftpAlg= atoi(BcmDb_getDefaultValue("enblTftp"));
        pNatAlgInfo->L2tpAlg= atoi(BcmDb_getDefaultValue("enblL2tp"));
        pNatAlgInfo->PptpAlg= atoi(BcmDb_getDefaultValue("enblPptp"));
        pNatAlgInfo->IpsecAlg= atoi(BcmDb_getDefaultValue("enblIpsec"));
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setNatAlgInfo
// Description  : store NATALG configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setNatAlgInfo(PNATALGINFO pNatAlgInfo) 
{
    if ( NULL == pNatAlgInfo)
    {
        return;
    }
    
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    BcmPsi_objStore(sysPsi, NAT_ALG, pNatAlgInfo, sizeof(NATALGINFO));
}
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
int BcmDb_getAutoSerchInfo(PAUTOSERCHINFO pAutoSerchInfo) 
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;

    if ( NULL == pAutoSerchInfo)
    {
        return DB_GET_NOT_FOUND;
    }
    
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    memset(pAutoSerchInfo, 0, sizeof(AUTOSERCHINFO));
   
    status = BcmPsi_objRetrieve(sysPsi, AUTO_SEARCH, pAutoSerchInfo,
        sizeof(AUTOSERCHINFO), &retLen);

    if ( status != PSI_STS_OK ) 
    {
        strcpy(pAutoSerchInfo->ScanList,BcmDb_getDefaultValue("autoScanList"));
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setAutoSerchInfo
// Description  : store AUTOSCAN list
// Returns      : none.
//**************************************************************************
void BcmDb_setAutoSerchInfo(PAUTOSERCHINFO pAutoSerchInfo) 
{
    if ( NULL == pAutoSerchInfo)
    {
        return;
    }
    
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    BcmPsi_objStore(sysPsi, AUTO_SEARCH, pAutoSerchInfo, sizeof(AUTOSERCHINFO));
}
/* END:   Added by y67514, 2008/10/27 */

/*start  of 增加Radius 认证功能by s53329  at  20060714 */
//**************************************************************************
// Function Name: BcmDb_getRadiuscInfo
// Description  : retrieve Radius configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRadiusInfo(PRADIUS_CLIENT_INFO pRadiusInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pRadiusInfo, 0, sizeof(RADIUS_CLIENT_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_RADIUS_ID, pRadiusInfo,
        sizeof(RADIUS_CLIENT_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
       /*增加Radius 使能的默认值*/
        pRadiusInfo->WebEnable = atoi(BcmDb_getDefaultValue("WebEnable"));
        pRadiusInfo->TelnetEnable= atoi(BcmDb_getDefaultValue("TelnetEnable"));
        pRadiusInfo->SshEnable = atoi(BcmDb_getDefaultValue("SshEnable"));
        pRadiusInfo->Timeout = atoi(BcmDb_getDefaultValue("RadiusTimeout"));
        pRadiusInfo->Retransmit = atoi(BcmDb_getDefaultValue("RadiusRetransmit"));
	 pRadiusInfo->PrimaryPort= atoi(BcmDb_getDefaultValue("RadiusPrimaryServerPort"));
        pRadiusInfo->SecondPort = atoi(BcmDb_getDefaultValue("RadiusSecondServerPort"));
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setRadiuscInfo
// Description  : store Radius configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setRadiusInfo(PRADIUS_CLIENT_INFO pRadiusInfo) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    int  status = 0;
    char cFlag[IFC_TINY_LEN];
    memset(cFlag, 0 , IFC_TINY_LEN);
    sprintf(cFlag, "%d", pRadiusInfo->WebEnable);
    status = tmpfile_writevalue("WebRadius", cFlag);
    memset(cFlag, 0 , IFC_TINY_LEN);
    sprintf(cFlag, "%d", pRadiusInfo->TelnetEnable);
    status = tmpfile_writevalue("TelnetRadius", cFlag);
    memset(cFlag, 0 , IFC_TINY_LEN);
    sprintf(cFlag, "%d", pRadiusInfo->SshEnable);
    status = tmpfile_writevalue("SshRadius", cFlag);
    BcmPsi_objStore(sysPsi, SYS_RADIUS_ID, pRadiusInfo, sizeof(RADIUS_CLIENT_INFO));
}
/*end   of 增加Radius 认证功能by s53329  at  20060714 */



//**************************************************************************
// Function Name: BcmDb_getMgmtLockInfo
// Description  : retrieve lock management configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getMgmtLockInfo(PSYS_MGMT_LOCK_INFO pMgmtLockInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pMgmtLockInfo, 0, sizeof(SYS_MGMT_LOCK_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_MGMT_LOCK_ID, pMgmtLockInfo,
        sizeof(SYS_MGMT_LOCK_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        pMgmtLockInfo->action = BcmCfm_MgmtUnlocked;
        pMgmtLockInfo->type = BcmCfm_TypeMgmtUnknown;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setMgmtLockInfo
// Description  : store lock management configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setMgmtLockInfo(PSYS_MGMT_LOCK_INFO pMgmtLockInfo) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_MGMT_LOCK_ID, pMgmtLockInfo, sizeof(SYS_MGMT_LOCK_INFO));
}

//**************************************************************************
// Function Name: BcmDb_getDnsInfo
// Description  : retrieve dns configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getDnsInfo(PIFC_DNS_INFO pDnsInfo) {
    int ret = DB_GET_OK;   
    char str[IFC_LARGE_LEN], dns[IFC_TINY_LEN];
    FILE* fs = NULL;
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    str[0] = dns[0] = '\0';
    memset(pDnsInfo, 0, sizeof(IFC_DNS_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_DNS_ID, pDnsInfo,
        sizeof(IFC_DNS_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        // if dsn info does not exist yet then check the dynamic dns directory
        fs = fopen("/var/fyi/sys/dns", "r");
        if ( fs != NULL ) {
            // if dynamic info exists then retrieve values
            if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) {
                sscanf(str, "nameserver %s\n", dns);
               pDnsInfo->preferredDns.s_addr = inet_addr(dns);
            }
            if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) {
                sscanf(str, "nameserver %s\n", dns);
                pDnsInfo->alternateDns.s_addr = inet_addr(dns);
            }
            fclose(fs);
            pDnsInfo->dynamic = TRUE;
            pDnsInfo->domainName[0] = '\0';
			pDnsInfo->enbdnsProxy = TRUE;  // add by y00183561 for lan dns proxy enable/disable 2011/10/24
            ret = DB_GET_OK;   
        } else {
            // dns info does not exists in both PSI and dynamic dns directory
            // then assigne default value  which is router IP address
            pDnsInfo->dynamic = atoi(BcmDb_getDefaultValue("dnsDynamic"));
            bcmGetIfDestAddr("br0", dns);
            pDnsInfo->preferredDns.s_addr = inet_addr(dns);
            pDnsInfo->alternateDns.s_addr = inet_addr(dns);
            strcpy(pDnsInfo->domainName, BcmDb_getDefaultValue("dnsDomainName"));
			pDnsInfo->enbdnsProxy = atoi(BcmDb_getDefaultValue("enbdnsProxy"));  // add by y00183561 for lan dns proxy enable/disable 2011/10/24
            ret = DB_GET_NOT_FOUND;
        }
    }
    
    return ret;   
}

/*start of 解决AU8D01490问题单:增加PPP IP Extension获取dns信息 by l129990,2008,12,16*/
//**************************************************************************
// Function Name: BcmDb_getIpExtDnsInfo
// Description  : retrieve dns configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getIpExtDnsInfo(char *interface, PIFC_DNS_INFO pDnsInfo) {

    int ret = DB_GET_OK;   
    char str[IFC_LARGE_LEN], dns[IFC_TINY_LEN];
    FILE* fs = NULL;
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    char path[128] = {0};
  
    str[0] = dns[0] = '\0';
    memset(pDnsInfo, 0, sizeof(IFC_DNS_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_DNS_ID, pDnsInfo,
        sizeof(IFC_DNS_INFO), &retLen);
    
    if ( status != PSI_STS_OK ) 
    {
        // if dsn info does not exist yet then check the dynamic dns directory
        if( interface != NULL )
        {   
            sprintf(path,"/var/fyi/sys/%s/dns",interface);
            fs = fopen(path, "r");
            if ( fs != NULL ) 
            {
            // if dynamic info exists then retrieve values
                if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
                {
                    sscanf(str, "nameserver %s\n", dns);
                    pDnsInfo->preferredDns.s_addr = inet_addr(dns);
                }
                if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
                {
                    sscanf(str, "nameserver %s\n", dns);
                    pDnsInfo->alternateDns.s_addr = inet_addr(dns);
                }
                fclose(fs);
                pDnsInfo->dynamic = TRUE;
                pDnsInfo->domainName[0] = '\0';
                ret = DB_GET_OK;   
           } 
           else 
           {
                // dns info does not exists in both PSI and dynamic dns directory
                // then assigne default value  which is router IP address
             
                pDnsInfo->dynamic = atoi(BcmDb_getDefaultValue("dnsDynamic"));
                bcmGetIfDestAddr("br0", dns);
                pDnsInfo->preferredDns.s_addr = inet_addr(dns);
                pDnsInfo->alternateDns.s_addr = inet_addr(dns);
                strcpy(pDnsInfo->domainName, BcmDb_getDefaultValue("dnsDomainName"));
                ret = DB_GET_NOT_FOUND;
          }
      }
      else
      {
          // dns info does not exists in both PSI and dynamic dns directory
          // then assigne default value  which is router IP address
          
          pDnsInfo->dynamic = atoi(BcmDb_getDefaultValue("dnsDynamic"));
          bcmGetIfDestAddr("br0", dns);
          pDnsInfo->preferredDns.s_addr = inet_addr(dns);
          pDnsInfo->alternateDns.s_addr = inet_addr(dns);
          strcpy(pDnsInfo->domainName, BcmDb_getDefaultValue("dnsDomainName"));
          ret = DB_GET_NOT_FOUND;
      }
   }
    
   return ret;   
}
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取dns信息 by l129990,2008,12,16*/

//**************************************************************************
// Function Name: BcmDb_setDnsInfo
// Description  : store dns configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setDnsInfo(PIFC_DNS_INFO pDnsInfo) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_DNS_ID, pDnsInfo, sizeof(IFC_DNS_INFO));
}

//**************************************************************************
// Function Name: BcmDb_removeDnsInfo
// Description  : remove dns configurations
// Returns      : none.
//**************************************************************************
void BcmDb_removeDnsInfo(void) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objRemove(sysPsi, SYS_DNS_ID);
}

//**************************************************************************
// Function Name: BcmDb_getDefaultGatewayInfo
// Description  : retrieve default gateway configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getDefaultGatewayInfo(PIFC_DEF_GW_INFO pGtwyInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pGtwyInfo, 0, sizeof(IFC_DEF_GW_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_DFT_GTWY_ID,
        pGtwyInfo, sizeof(IFC_DEF_GW_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        pGtwyInfo->enblGwAutoAssign = atoi(BcmDb_getDefaultValue("enblAuto"));
        pGtwyInfo->defaultGateway.s_addr = inet_addr(BcmDb_getDefaultValue("defaultGateway"));
        strcpy(pGtwyInfo->ifName, BcmDb_getDefaultValue("wanIfName"));
        ret = DB_GET_NOT_FOUND;
    }

    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setDefaultGatewayInfo
// Description  : store default gateway configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setDefaultGatewayInfo(PIFC_DEF_GW_INFO pGtwyInfo) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_DFT_GTWY_ID, pGtwyInfo, sizeof(IFC_DEF_GW_INFO));
}

//**************************************************************************
// Function Name: BcmDb_removeDefaultGatewayInfo
// Description  : remove default gateway configurations
// Returns      : none.
//**************************************************************************
void BcmDb_removeDefaultGatewayInfo(void) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objRemove(sysPsi, SYS_DFT_GTWY_ID);
}

//**************************************************************************
// Function Name: BcmDb_getSysFlagInfo
// Description  : retrieve system flag configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSysFlagInfo(PSYS_FLAG_INFO pSysFlag) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    memset(pSysFlag, 0, sizeof(SYS_FLAG_INFO));
   
    status = BcmPsi_objRetrieve(sysPsi, SYS_FLAG_ID, pSysFlag,
        sizeof(SYS_FLAG_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        pSysFlag->autoScan = atoi(BcmDb_getDefaultValue("enblAutoScan"));
        pSysFlag->upnp = atoi(BcmDb_getDefaultValue("enblUpnp"));
        pSysFlag->igmpSnp = atoi(BcmDb_getDefaultValue("enblIgmpSnp"));
 /*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
       // pSysFlag->igmpMode = atoi(BcmDb_getDefaultValue("enblIgmpMode"));
 /*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
        pSysFlag->macPolicy = atoi(BcmDb_getDefaultValue("macPolicy"));
        pSysFlag->encodePassword = atoi(BcmDb_getDefaultValue("encodePassword"));
        pSysFlag->siproxd = atoi(BcmDb_getDefaultValue("enblSiproxd"));
#if SUPPORT_ETHWAN
        pSysFlag->enetWan = atoi(BcmDb_getDefaultValue("enblEnetWan"));
#endif
// add by l66195 for VDF start
       pSysFlag->ddns= atoi(BcmDb_getDefaultValue("enblDdns"));
// add by l66195 for VDF end
/*begin add by p44980 2008.10.22, 实现adsl 和hspa切换处理*/
       pSysFlag->connSwitchMode = atoi(BcmDb_getDefaultValue("connSwitchMode"));
/*end add by p44980 2008.10.22*/
        // store default sys flag info to psi in SDRAM memory
        BcmPsi_objStore(sysPsi, SYS_FLAG_ID, pSysFlag, sizeof(SYS_FLAG_INFO));
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setSysFlagInfo
// Description  : store system flag configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setSysFlagInfo(PSYS_FLAG_INFO pSysFlag) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    BcmPsi_objStore(sysPsi, SYS_FLAG_ID, pSysFlag, sizeof(SYS_FLAG_INFO));
}

 /*start of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */
void BcmDb_setHttpPortInfo(char  *cPort) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    BcmPsi_objStore(sysPsi, HTTP_PORT, cPort, strlen(cPort));
}


void BcmDb_getHttpPortInfo(char  *pcPort) {

    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    char  cPort[IFC_PASSWORD_LEN];
    memset(cPort, 0, IFC_PASSWORD_LEN);
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    status = BcmPsi_objRetrieve(sysPsi, HTTP_PORT,
                                        cPort, IFC_PASSWORD_LEN, &retLen);
    strcpy(pcPort, cPort);

}
 /*end of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */

//**************************************************************************
// Function Name: BcmDb_getSpecificSysFlagInfo
// Description  : retrieve specific system flag configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSpecificSysFlagInfo(int *value, int type) {
    SYS_FLAG_INFO sysFlag;

    int ret = BcmDb_getSysFlagInfo(&sysFlag);
    
    switch ( type ) {
        case SYS_FLAG_AUTO_SCAN:
            *value = sysFlag.autoScan;
            break;
        case SYS_FLAG_UPNP:
            *value = sysFlag.upnp;
            break;
        case SYS_FLAG_MAC_POLICY:
            *value = sysFlag.macPolicy;
            break;
        case SYS_FLAG_SIPROXD:
            *value = sysFlag.siproxd;
            break;
        case SYS_FLAG_ENCODE_PASSWORD:
            *value = sysFlag.encodePassword;
            break;
        case SYS_FLAG_IGMP_SNP:
            *value = sysFlag.igmpSnp;
            break;
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
        //case SYS_FLAG_IGMP_MODE:
           // *value = sysFlag.igmpMode;
            //break;
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
//add by l66195 for VDF start
        case SYS_FLAG_DDNS:
            *value = sysFlag.ddns;
            break;
//add by l66195 for VDF end
#if SUPPORT_ETHWAN
        case SYS_FLAG_ENET_WAN:
            *value = sysFlag.enetWan;
            break;
#endif
        default:
            break;
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setSpecificSysFlagInfo
// Description  : store specific system flag configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setSpecificSysFlagInfo(int value, int type) {
    SYS_FLAG_INFO sysFlag;

    BcmDb_getSysFlagInfo(&sysFlag);
    
    switch ( type ) {
        case SYS_FLAG_AUTO_SCAN:
            sysFlag.autoScan = value;
            break;
        case SYS_FLAG_UPNP:
            sysFlag.upnp = value;
            break;
        case SYS_FLAG_MAC_POLICY:
            sysFlag.macPolicy = value;
            break;
        case SYS_FLAG_SIPROXD:
            sysFlag.siproxd = value;
            break;
        case SYS_FLAG_ENCODE_PASSWORD:
            sysFlag.encodePassword = value;
            break;
        case SYS_FLAG_IGMP_SNP:
            sysFlag.igmpSnp = value;
            break;
   /*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
        //case SYS_FLAG_IGMP_MODE:
           // sysFlag.igmpMode = value;
            //break;
   /*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
//add by l66195 for VDF start
        case SYS_FLAG_DDNS:
            sysFlag.ddns = value;
            break;
//add by l66195 for VDF end
#if SUPPORT_ETHWAN
        case SYS_FLAG_ENET_WAN:
            sysFlag.enetWan = value;
            break;
#endif
        default:
            break;
    }
    
    BcmDb_setSysFlagInfo(&sysFlag);
}

//**************************************************************************
// Function Name: BcmDb_getAutoScanInfo
// Description  : retrieve auto scan configurations
// Returns      : TRUE or FALSE
//**************************************************************************
int BcmDb_getAutoScanInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.autoScan;   
}

//**************************************************************************
// Function Name: BcmDb_setAutoScanInfo
// Description  : store auto scan configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setAutoScanInfo(int autoScan) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.autoScan = autoScan;    
    BcmDb_setSysFlagInfo(&sysFlag);
}

//**************************************************************************
// Function Name: BcmDb_getEncodePasswordInfo
// Description  : retrieve encode password info. If TRUE then admin,
//     support, user, and ppp passwords are encoded in configuration files.
// Returns      : TRUE or FALSE
//**************************************************************************
int BcmDb_getEncodePasswordInfo(void) {
    /* start of maintain 去除配置文件中的参数encodePassword by xujunxia 43813 2006年10月8日
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.encodePassword;   
    */
    return  TRUE;   
    /* end of maintain 去除配置文件中的参数encodePassword by xujunxia 43813 2006年10月8日 */
}

//**************************************************************************
// Function Name: BcmDb_setEncodePasswordInfo
// Description  : store encode password info.
// Returns      : none.
//**************************************************************************
void BcmDb_setEncodePasswordInfo(int encodePassword) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.encodePassword = encodePassword;    
    BcmDb_setSysFlagInfo(&sysFlag);
}

#ifdef SUPPORT_UPNP
//**************************************************************************
// Function Name: BcmDb_getUpnpInfo
// Description  : retrieve upnp configurations
// Returns      : TRUE or FALSE
//**************************************************************************
int BcmDb_getUpnpInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.upnp;   
}

//**************************************************************************
// Function Name: BcmDb_setUpnpInfo
// Description  : store upnp configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setUpnpInfo(int upnp) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.upnp = upnp;    
    BcmDb_setSysFlagInfo(&sysFlag);
}
#endif

//**************************************************************************
// Function Name: BcmDb_getIgmpSnpInfo
// Description  : retrieve igmp snooping configurations
// Returns      : TRUE or FALSE
//**************************************************************************
int BcmDb_getIgmpSnpInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.igmpSnp;   
}

//**************************************************************************
// Function Name: BcmDb_setIgmpSnpInfo
// Description  : store igmp snooping configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setIgmpSnpInfo(int igmpSnp) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.igmpSnp = igmpSnp;    
    BcmDb_setSysFlagInfo(&sysFlag);
}

//**************************************************************************
// Function Name: BcmDb_getIgmpModeInfo
// Description  : retrieve igmp snooping mode configurations
// Returns      : TRUE or FALSE
//**************************************************************************
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if 0
int BcmDb_getIgmpModeInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.igmpMode;   
}
#endif
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
//**************************************************************************
// Function Name: BcmDb_setIgmpModeInfo
// Description  : store igmp snooping mode configurations
// Returns      : none.
//**************************************************************************
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if 0
void BcmDb_setIgmpModeInfo(int igmpMode) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.igmpMode = igmpMode;    
    BcmDb_setSysFlagInfo(&sysFlag);
}
#endif
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
#if SUPPORT_ETHWAN
//**************************************************************************
// Function Name: BcmDb_setEnetWanInfo
// Description  : store Ethernet WAN configuration
// Returns      : none.
//**************************************************************************
void BcmDb_setEnetWanInfo(int enetWan) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.enetWan = enetWan;
    BcmDb_setSysFlagInfo(&sysFlag);
}

//**************************************************************************
// Function Name: BcmDb_getEnetWanInfo
// Description  : store Ethernet WAN configuration
// Returns      : none.
//**************************************************************************
int BcmDb_getEnetWanInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    return sysFlag.enetWan;
}
#endif
// add by l66195 for VDF start
//**************************************************************************
// Function Name: BcmDb_getDdnsEblInfo
// Description  : retrieve ddns configurations
// Returns      : TRUE or FALSE
//**************************************************************************
extern "C"
{

int BcmDb_getDdnsEblInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.ddns;   
}

//**************************************************************************
// Function Name: BcmDb_setDdnsEblInfo
// Description  : store ddns configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setDdnsEblInfo(int ddns) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.ddns = ddns;    
    BcmDb_setSysFlagInfo(&sysFlag);
}
}
// add by l66195 for VDF end

//**************************************************************************
// Function Name: BcmDb_getMacPolicyInfo
// Description  : retrieve mac policy configurations
// Returns      : MAC_FORWARD: 0 or MAC_BLOCKED: 1
//**************************************************************************
int BcmDb_getMacPolicyInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.macPolicy;   
}

//**************************************************************************
// Function Name: BcmDb_setMacPolicyInfo
// Description  : store mac policy configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setMacPolicyInfo(int macPolicy) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.macPolicy = macPolicy;    
    BcmDb_setSysFlagInfo(&sysFlag);
}

/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
//**************************************************************************
// Function Name: BcmDb_getSiproxdInfo
// Description  : retrieve SIP proxy daemon configurations
// Returns      : DB_GET_OK: success or DB_GET_NOT_FOUND: fail
//**************************************************************************
int  BcmDb_getSiproxdInfo(int *enable, UINT32 *portnumber)
{    
#ifdef SUPPORT_SIPROXD
    int ret = DB_GET_OK;
    SIPPROXYINFO sipInfo;
    bzero(&sipInfo, sizeof(sipInfo));
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;

    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
      
    status = BcmPsi_objRetrieve(sysPsi, SYS_SIPROXD_ID, &sipInfo,
        sizeof(sipInfo), &retLen);

    if(status != PSI_STS_OK)
    {
        sipInfo.enable = atoi(BcmDb_getDefaultValue("enblSiproxd"));
        sipInfo.portnumber= atoi(BcmDb_getDefaultValue("sipProxyPort"));

        // store default sys flag info to psi in SDRAM memory
        BcmPsi_objStore(sysPsi, SYS_SIPROXD_ID, &sipInfo, sizeof(sipInfo));
        ret = DB_GET_NOT_FOUND;
    }
    else
    {
        ret = DB_GET_OK;
    } 
	
    *enable = (int)sipInfo.enable;
    *portnumber = sipInfo.portnumber;
	
    return ret;
#else
    int ret = DB_GET_NOT_FOUND;
    *enable = 0;
    *portnumber = DEFAULT_SIPPROXYPORT;
    return ret;    
#endif
}
void BcmDb_setSiproxdInfo(int enable, UINT32 portnumber)
{
#ifdef SUPPORT_SIPROXD
    SIPPROXYINFO sipInfo;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
	
    sipInfo.enable = (UINT16)enable;
    sipInfo.portnumber = portnumber;

    BcmPsi_objStore(sysPsi, SYS_SIPROXD_ID, &sipInfo, sizeof(sipInfo));
#endif
}
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/
/*begin add by p44980 2008.10.22, 实现adsl 和hspa切换处理*/
//**************************************************************************
// Function Name: BcmDb_getConnSwitchModeInfo
// Description  : retrieve connection (adsl and hspa) switch mode configurations
// Returns      : 0 - 不需要定时通过访问dns服务检测adsl连接的有效性
//                   1 - 需要定时访问dns服务来检测adsl连接的有效性
//**************************************************************************
int BcmDb_getConnSwitchModeInfo(void) {
    SYS_FLAG_INFO sysFlag;
    BcmDb_getSysFlagInfo(&sysFlag);   
    return sysFlag.connSwitchMode;   
}

//**************************************************************************
// Function Name: BcmDb_setConnSwitchModeInfo
// Description  : store connection (adsl and hspa) switch mode  configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setConnSwitchModeInfo(int nMode) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.connSwitchMode = nMode;    
    BcmDb_setSysFlagInfo(&sysFlag);
}
/***************************************************************************
// Function Name: bcmIsDnsStateUp().
// Description  : return TRUE if dns of data pvc  is up, else return FALSE.
// Parameters   : void.
// Returns      : return TRUE if dns of data pvc  is up, else return FALSE.
****************************************************************************/
BCM_ENUM_DNSDetactFlag BcmDb_IsDnsStateUp(void)
{
    char cValue[32] = {0};
    int nLen = 32;

    //检查当前的adsl和hspa的切换模式
    if (BcmDb_getConnSwitchModeInfo() == CONN_SWITCH_MODE_PLAIN)
    {
        //如果不需要检测dns，则直接返回1。
        return BCM_ENUM_DNSDetact_Well;
    }

    //检查DnsChkState值
    if (tmpfile_getvalue("DnsChkState", cValue, &nLen) == 0)
    {
        return ((atoi(cValue)) == 0) ? BCM_ENUM_DNSDetact_Failed : BCM_ENUM_DNSDetact_Well;
    }
    else
    {
        //查询失败，返回up
        return BCM_ENUM_DNSDetact_Well;
    }
}
/*end add by p44980 2008.10.22*/
#if 0

//**************************************************************************
// Function Name: BcmDb_getSiproxdInfo
// Description  : retrieve SIP proxy daemon configurations
// Returns      : MAC_FORWARD: 0 or MAC_BLOCKED: 1
//**************************************************************************
int BcmDb_getSiproxdInfo(void) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   

    return sysFlag.siproxd;   
}

//**************************************************************************
// Function Name: BcmDb_setSiproxdInfo
// Description  : store SIP proxy daemon configurations
// Returns      : none.
//**************************************************************************
void BcmDb_setSiproxdInfo(int siproxd) {
    SYS_FLAG_INFO sysFlag;
    
    BcmDb_getSysFlagInfo(&sysFlag);   
    sysFlag.siproxd = siproxd;    
    BcmDb_setSysFlagInfo(&sysFlag);
}
#endif

//**************************************************************************
// Function Name: BcmDb_getLoginInfo
// Description  : retrieve login info
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getLoginInfo(int id, char *info, int len) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    info[0] = '\0';        
    status = BcmPsi_objRetrieve(sysPsi, id, info, len, &retLen);

    if ( status != PSI_STS_OK ) {
        switch ( id ) {
        case SYS_USER_NAME_ID:
           strncpy(info, BcmDb_getDefaultValue("sysUserName"), len);
           break;
        case SYS_PASSWORD_ID:
           strncpy(info, BcmDb_getDefaultValue("sysPassword"), len);
           break;
#ifndef SUPPORT_ONE_USER
        case SPT_USER_NAME_ID:
           strncpy(info, BcmDb_getDefaultValue("sptUserName"), len);
           break;
        case SPT_PASSWORD_ID:
           strncpy(info, BcmDb_getDefaultValue("sptPassword"), len);
           break;
        case USR_USER_NAME_ID:
           strncpy(info, BcmDb_getDefaultValue("usrUserName"), len);
           break;
        case USR_PASSWORD_ID:
           strncpy(info, BcmDb_getDefaultValue("usrPassword"), len);
           break;
#endif
        }
        ret = DB_GET_NOT_FOUND;
    } else {
        // set terminate string character
        if ( retLen < len )
            info[retLen] = '\0';
    }
    
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setLoginInfo
// Description  : store login ifno
// Returns      : none.
//**************************************************************************
void BcmDb_setLoginInfo(int id, char *info, int len) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
    
    BcmPsi_objStore(sysPsi, id, info, len);
}

//**************************************************************************
// Function Name: BcmDb_getSysUserName
// Description  : retrieve system user name
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getSysUserName(char *userName, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( userName != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(SYS_USER_NAME_ID, userName, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSysUserName
// Description  : store system user name
// Returns      : none.
//**************************************************************************
void BcmDb_setSysUserName(char *userName, int len) {
    BcmDb_setLoginInfo(SYS_USER_NAME_ID, userName, len);
}

//**************************************************************************
// Function Name: BcmDb_getSysPassword
// Description  : retrieve system password
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getSysPassword(char *password, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( password != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(SYS_PASSWORD_ID, password, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSysPassword
// Description  : store system password
// Returns      : none.
//**************************************************************************
void BcmDb_setSysPassword(char *password, int len) {
    BcmDb_setLoginInfo(SYS_PASSWORD_ID, password, len);
}

#ifndef SUPPORT_ONE_USER
//**************************************************************************
// Function Name: BcmDb_getSptUserName
// Description  : retrieve support user name
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getSptUserName(char *userName, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( userName != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(SPT_USER_NAME_ID, userName, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSptUserName
// Description  : store support user name
// Returns      : none.
//**************************************************************************
void BcmDb_setSptUserName(char *userName, int len) {
    BcmDb_setLoginInfo(SPT_USER_NAME_ID, userName, len);
}

//**************************************************************************
// Function Name: BcmDb_getSptPassword
// Description  : retrieve support password
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getSptPassword(char *password, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( password != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(SPT_PASSWORD_ID, password, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSptPassword
// Description  : store support password
// Returns      : none.
//**************************************************************************
void BcmDb_setSptPassword(char *password, int len) {
    BcmDb_setLoginInfo(SPT_PASSWORD_ID, password, len);
}

//**************************************************************************
// Function Name: BcmDb_getUsrUserName
// Description  : retrieve user user name
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getUsrUserName(char *userName, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( userName != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(USR_USER_NAME_ID, userName, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setUsrUserName
// Description  : store user user name
// Returns      : none.
//**************************************************************************
void BcmDb_setUsrUserName(char *userName, int len) {
    BcmDb_setLoginInfo(USR_USER_NAME_ID, userName, len);
}

//**************************************************************************
// Function Name: BcmDb_getUsrPassword
// Description  : retrieve user password
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND
//**************************************************************************
int BcmDb_getUsrPassword(char *password, int len) {
    int ret = DB_GET_NOT_FOUND;
    
    if ( password != NULL && len > 0 )
       ret = BcmDb_getLoginInfo(USR_PASSWORD_ID, password, len);
       
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setUsrPassword
// Description  : store user password
// Returns      : none.
//**************************************************************************
void BcmDb_setUsrPassword(char *password, int len) {
    BcmDb_setLoginInfo(USR_PASSWORD_ID, password, len);
}
#endif

/*w00135358 start-add for Global QoS 20080908*/
//**************************************************************************
// Function Name: BcmDb_getEtheProtocol
// Description  : convert Ethenet protocol from text to integer
// Returns      : Ethenet protocol number.
//**************************************************************************
int BcmDb_getEtheProtocol(char *info) {
    int i = 0;    

    static char* prots[] = { "IP", "802.1Q", "PPPoE", NULL};


    
    for ( i = 0; prots[i] != NULL; i++ ) {
        if ( strcasecmp(prots[i], info) == 0 )
                return i;;
    }
	return QOS_PROTO_NONE;
}
/*w00135358 end-add for Global QoS 20080908*/

//**************************************************************************
// Function Name: BcmDb_getIpProtocol
// Description  : convert IP protocol from text to integer
// Returns      : IP protocol number.
//**************************************************************************
int BcmDb_getIpProtocol(char *info) {
    int i = 0;    
    /*start modifying for vdf qos by p44980 2008.01.11*/
    #ifdef SUPPORT_VDF_QOS
    static char* prots[] = { "TCP/UDP", "TCP", "UDP",  "ICMP", "IGMP" ,NULL};
    #else
    static char* prots[] = {"TCP/UDP", "TCP", "UDP", "ICMP", "", NULL};
    #endif
    /*end modifying for vdf qos by p44980 2008.01.11*/
    
    for ( i = 0; prots[i] != NULL; i++ ) {
        if ( strcasecmp(prots[i], info) == 0 )
            break;
    }

    /*start modfying for vdf qos by p44980 2008.01.08*/
    #ifdef SUPPORT_VDF_QOS
    if(i == 5)
    {
        i = QOS_PROTO_NONE;
    }
    #endif
    /*start modfying for vdf qos by p44980 2008.01.08*/
    return i;
}

//**************************************************************************
// Function Name: BcmDb_getMacProtocol
// Description  : convert mac protocol from text to integer
// Returns      : mac protocol number.
//**************************************************************************
int BcmDb_getMacProtocol(char *info) {
    int i = 0;    
    static char* prots[] = {"none", "PPPoE", "IPv4", "IPv6", "AppleTalk", "IPX", "NetBEUI", "IGMP", NULL};
    
    for ( i = 1; prots[i] != NULL; i++ ) {
        if ( strcasecmp(prots[i], info) == 0 )
            break;
    }
    if ( prots[i] == NULL )
       i = SEC_PROTO_NONE;
       
    return i;
}

//**************************************************************************
// Function Name: BcmDb_getSecuritySize
// Description  : retrieve security size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSecuritySize(int id, int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    status = BcmPsi_objRetrieve(secPsi, id, &size, sizeof(size), &retLen);

    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSecuritySize
// Description  : store security size
// Returns      : none.
//**************************************************************************
void BcmDb_setSecuritySize(int id, int info) {
    UINT16 size = info;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, id, &size, sizeof(size));
}

//**************************************************************************
// Function Name: BcmDb_getSecurityInfo
// Description  : retrieve security table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSecurityInfo(int id, PSI_VALUE info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    status = BcmPsi_objRetrieve(secPsi, id, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSecurityInfo
// Description  : store security table
// Returns      : none.
//**************************************************************************
void BcmDb_setSecurityInfo(int id, PSI_VALUE info, int size) {
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, id, info, size);
}

//**************************************************************************
// Function Name: BcmDb_getVirtualServerSize
// Description  : retrieve virtual server size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVirtualServerSize(int *info) {
    return BcmDb_getSecuritySize(SCM_VRT_SRV_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setVirtualServerSize
// Description  : store virtual server size
// Returns      : none.
//**************************************************************************
void BcmDb_setVirtualServerSize(int info) {
    BcmDb_setSecuritySize(SCM_VRT_SRV_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getVirtualServerInfo
// Description  : retrieve virtual server table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVirtualServerInfo(PSEC_VRT_SRV_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_VRT_SRV_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_VRT_SRV_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setVirtualServerInfo
// Description  : store virtual server table
// Returns      : none.
//**************************************************************************
void BcmDb_setVirtualServerInfo(PSEC_VRT_SRV_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_VRT_SRV_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_VRT_SRV_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getOutgoingFilterSize
// Description  : retrieve firewall outgoing filter size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getOutgoingFilterSize(int *info) {
    return BcmDb_getSecuritySize(SCM_FLT_OUT_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setOutgoingFilterSize
// Description  : store firewall outgoing filter size
// Returns      : none.
//**************************************************************************
void BcmDb_setOutgoingFilterSize(int info) {
    BcmDb_setSecuritySize(SCM_FLT_OUT_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getOutgoingFilterInfo
// Description  : retrieve firewall outgoing table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getOutgoingFilterInfo(PSEC_FLT_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_FLT_OUT_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setOutgoingFilterInfo
// Description  : store firewall outgoing table
// Returns      : none.
//**************************************************************************
void BcmDb_setOutgoingFilterInfo(PSEC_FLT_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_FLT_OUT_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getIncomingFilterSize
// Description  : retrieve firewall incoming filter size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getIncomingFilterSize(int *info) {
    return BcmDb_getSecuritySize(SCM_FLT_IN_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setIncomingFilterSize
// Description  : store firewall incoming filter size
// Returns      : none.
//**************************************************************************
void BcmDb_setIncomingFilterSize(int info) {
    BcmDb_setSecuritySize(SCM_FLT_IN_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getIncomingFilterInfo
// Description  : retrieve firewall incoming table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getIncomingFilterInfo(PSEC_FLT_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_FLT_IN_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setIncomingFilterInfo
// Description  : store firewall incoming table
// Returns      : none.
//**************************************************************************
void BcmDb_setIncomingFilterInfo(PSEC_FLT_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_FLT_IN_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getPortTriggerSize
// Description  : retrieve port trigger size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPortTriggerSize(int *info) {
    return BcmDb_getSecuritySize(SCM_PRT_TRG_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setPortTriggerSize
// Description  : store port trigger size
// Returns      : none.
//**************************************************************************
void BcmDb_setPortTriggerSize(int info) {
    BcmDb_setSecuritySize(SCM_PRT_TRG_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getPortTriggerInfo
// Description  : retrieve port trigger table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPortTriggerInfo(PSEC_PRT_TRG_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_PRT_TRG_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_PRT_TRG_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setPortTriggerInfo
// Description  : store port trigger table
// Returns      : none.
//**************************************************************************
void BcmDb_setPortTriggerInfo(PSEC_PRT_TRG_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_PRT_TRG_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_PRT_TRG_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getMacFilterDirection
// Description  : convert MAC filter direction from text to integer
// Returns      : port trigger protocol number.
//**************************************************************************
int BcmDb_getMacFilterDirection(char *info) {
    int i = 0;    
    static char* dirs[] = {"LanToWan", "WanToLan", "both", NULL};
    
    for ( i = 0; dirs[i] != NULL; i++ ) {
        if ( strcasecmp(dirs[i], info) == 0 )
            break;
    }
       
    return i;
}

//**************************************************************************
// Function Name: BcmDb_getMacFilterSize
// Description  : retrieve MAC filter size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getMacFilterSize(int *info) {
    return BcmDb_getSecuritySize(SCM_MAC_FLT_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setMacFilterSize
// Description  : store MAC filter size
// Returns      : none.
//**************************************************************************
void BcmDb_setMacFilterSize(int info) {
    BcmDb_setSecuritySize(SCM_MAC_FLT_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getMacFilterInfo
// Description  : retrieve MAC filter table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getMacFilterInfo(PSEC_MAC_FLT_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_MAC_FLT_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_MAC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setMacFilterInfo
// Description  : store MAC filter table
// Returns      : none.
//**************************************************************************
void BcmDb_setMacFilterInfo(PSEC_MAC_FLT_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_MAC_FLT_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_MAC_FLT_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getQosClassSize
// Description  : retrieve QoS class size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosClassSize(int *info) {
    return BcmDb_getSecuritySize(QOS_CLS_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setQosClassSize
// Description  : store QoS class size
// Returns      : none.
//**************************************************************************
void BcmDb_setQosClassSize(int info) {
    BcmDb_setSecuritySize(QOS_CLS_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getQosClassInfo
// Description  : retrieve QoS class table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosClassInfo(PQOS_CLASS_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(QOS_CLS_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(QOS_CLASS_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setQosClassInfo
// Description  : store QoS class table
// Returns      : none.
//**************************************************************************
void BcmDb_setQosClassInfo(PQOS_CLASS_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(QOS_CLS_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(QOS_CLASS_ENTRY) * size);
}

//start for vdf qos by p44980 2008.01.08
#ifdef SUPPORT_VDF_QOS
//**************************************************************************
// Function Name: BcmDb_getQosRuleSize
// Description  : retrieve QoS queue size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosQueueSize(int *info) {
    return BcmDb_getSecuritySize(QOS_QUEUE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setQosClassSize
// Description  : store QoS queue size
// Returns      : none.
//**************************************************************************
void BcmDb_setQosQueueSize(int info) {
    BcmDb_setSecuritySize(QOS_QUEUE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getQosClassInfo
// Description  : retrieve QoS queue table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosQueueInfo(PQOS_QUEUE_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(QOS_QUEUE_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(QOS_QUEUE_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setQosClassInfo
// Description  : store QoS queue table
// Returns      : none.
//**************************************************************************
void BcmDb_setQosQueueInfo(PQOS_QUEUE_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(QOS_QUEUE_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(QOS_QUEUE_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getQosClassInfo
// Description  : retrieve QoS configration table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosCfgInfo(int *pbEnable) 
{
    return BcmDb_getSecurityInfo(QOS_CFG_ID, (PSI_VALUE)pbEnable,sizeof(BOOL));
}

//**************************************************************************
// Function Name: BcmDb_setQosClassInfo
// Description  : store QoS configration table
// Returns      : none.
//**************************************************************************
void BcmDb_setQosCfgInfo(int bEnable) 
{
    BcmDb_setSecurityInfo(QOS_CFG_ID, (PSI_VALUE)&bEnable,sizeof(int));
}

//**************************************************************************
// Function Name: BcmDb_getQosServiceSize
// Description  : retrieve QoS service configrations size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosServiceSize(int *info)
{
    return BcmDb_getSecuritySize(QOS_SERVICE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setQosServiceSize
// Description  : store QoS service configrations size
// Returns      : none.
//**************************************************************************
void BcmDb_setQosServiceSize(int info)
{
    BcmDb_setSecuritySize(QOS_SERVICE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getQosServiceInfo
// Description  : retrieve QoS service table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getQosServiceInfo(PQOS_SERVICE_CFG_ENTRY tbl, int size)
{
    return BcmDb_getSecurityInfo(QOS_SERVICE_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(QOS_SERVICE_CFG_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setQosServiceInfo
// Description  : store QoS service table
// Returns      : none.
//**************************************************************************
void BcmDb_setQosServiceInfo(PQOS_SERVICE_CFG_ENTRY tbl, int size)
{
    BcmDb_setSecurityInfo(QOS_SERVICE_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(QOS_SERVICE_CFG_ENTRY) * size);
}
#endif
//end for vdf qos by p44980 2008.01.08

/* j00100803 Add Begin 2008-02-28 */
#ifdef SUPPORT_POLICY
int BcmDb_getRtPolicySize(int *info) {
    return BcmDb_getRouteSize(POLICY_ROUTE_NUM_ID, info);
}

//**************************************************************************

//**************************************************************************
void BcmDb_setRtPolicySize(int info) {
    BcmDb_setRouteSize(POLICY_ROUTE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getQosClassInfo
// Description  : retrieve QoS queue table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRtPolicyInfo(PIFC_RoutePolicy_Info pRtPolicy, int size) {
    return BcmDb_getRouteInfo(POLICY_ROUTE_CFG_TBL_ID, (PSI_VALUE)pRtPolicy,
                                 sizeof(IFC_RoutePolicy_Info) * size);
}

//**************************************************************************
// Function Name: BcmDb_setQosClassInfo
// Description  : store QoS queue table
// Returns      : none.
//**************************************************************************
void BcmDb_setRtPolicyInfo(PIFC_RoutePolicy_Info pRtPolicy, int size) {
    BcmDb_setRouteInfo(POLICY_ROUTE_CFG_TBL_ID, (PSI_VALUE)pRtPolicy,
                          sizeof(IFC_RoutePolicy_Info) * size);
}

int BcmDb_setRtMappingIfcName(int iIndex, WAN_CON_ID * pWanIfcId)
{
	if((iIndex < 0) || (iIndex >= IFC_TINY_LEN) || (NULL == pWanIfcId))
	{
		return -1;
	}
	gl_stPolicyRoutingMap[iIndex].stWanId.conId = pWanIfcId->conId;
	gl_stPolicyRoutingMap[iIndex].stWanId.vci = pWanIfcId->vci;
	gl_stPolicyRoutingMap[iIndex].stWanId.vpi = pWanIfcId->vpi;
	return 0;
}

int BcmDb_getRtMappingMark(unsigned int uIndex, int * pMark)
{
	if((uIndex < 0) || (uIndex >= IFC_TINY_LEN))
	{
		return -1;
	}
	*pMark = gl_stPolicyRoutingMap[uIndex].iMark;
	return 0;
}

int BcmDb_getRtMappingRouteTable(unsigned int uIndex, unsigned int * pTable)
{
	if((uIndex < 0) || (uIndex >= IFC_TINY_LEN))
	{
		return -1;
	}
	*pTable = gl_stPolicyRoutingMap[uIndex].iRouteTableIndex;
	return 0;
}

int BcmDb_getRtMappingIndex(char * pWanIfcName, unsigned int * pIndex)
{
	int i = 0;
	WAN_CON_ID stWanId;
	WAN_CON_INFO stWanInfo;
	char szWanIfcName[IFC_TINY_LEN];
	if(NULL == pWanIfcName)
	{
		return -1;
	}
	memset(szWanIfcName, 0, IFC_TINY_LEN);
	int iHaveSeek = 0;
	stWanId.conId = 0;
	stWanId.vci = 0;
	stWanId.vpi = 0;

	while(BcmDb_getWanInfoNext(&stWanId, &stWanInfo) == DB_WAN_GET_OK )
	{
       	BcmDb_getWanInterfaceName(&stWanId, stWanInfo.protocol, szWanIfcName);
		if(0 == strcmp(szWanIfcName, pWanIfcName))
		{
			iHaveSeek = 1;
			break;
		}
	}

	if(0 == iHaveSeek)
	{
		return -1;
	}
	for(i = 0; i < IFC_TINY_LEN; i++)
	{
		/*Start -- w00135358 modified for HG556 20090309*/
		//VCI VPI相同时还需要比较conid
		/*
		if((gl_stPolicyRoutingMap[i].stWanId.vci == stWanId.vci)
			&& (gl_stPolicyRoutingMap[i].stWanId.vpi == stWanId.vpi))
		*/
		if((gl_stPolicyRoutingMap[i].stWanId.vci == stWanId.vci)
			&& (gl_stPolicyRoutingMap[i].stWanId.vpi == stWanId.vpi) 
			  && (gl_stPolicyRoutingMap[i].stWanId.conId== stWanId.conId))	
		/*End -- w00135358 modified for HG556 20090309*/
		{
			*pIndex = i;
			return 0;
		}
	}
	return -1;
}

void BcmDb_initRtMapping()
{
	int i = 0;
	for(i = 0; i < IFC_TINY_LEN; i++)
	{
		gl_stPolicyRoutingMap[i].stWanId.conId = 0;
		gl_stPolicyRoutingMap[i].stWanId.vci = 0;
		gl_stPolicyRoutingMap[i].stWanId.vpi = 0;
		gl_stPolicyRoutingMap[i].iMark = (0x21 + i) << 24;
		gl_stPolicyRoutingMap[i].iRouteTableIndex = 101 + i;
	}
}

/*start of VDF 2008.4.23 V100R001C02B013 j00100803 AU8D00546 */
int BcmDb_getIfcNameForIgmp()
{
    void *node = NULL;
	IFC_RoutePolicy_Info entry;
    FILE* fs= NULL;

	int i = 0;
	
    while ( (node = BcmRcm_getRtPolicyCfg(node, &entry)) != NULL)
	{
        if ( (BcmCfm_ENUM_ProtocolBased == entry.enType) && 
			(0 == strcmp(entry.szProtocolType, "IGMP")) ) 
		{
     /*Start of重新设置组播启动接口 by l129990*/
            fs = fopen("/var/igmpolicy.conf", "a+");
            if (fs != NULL) {
                fputs(entry.szWanIfcName, fs);
                fputs("\n", fs);
                i ++;
            }
            
     /*end of重新设置组播启动接口 by l129990 2008,9,6*/ 
        }
    }
     /*Start of重新设置组播启动接口 by l129990*/
    if( i > 0)
    {
        fclose(fs);
    }
     /*end of重新设置组播启动接口 by l129990*/
    return i;
}
/*end of VDF 2008.4.23 V100R001C02B013 j00100803 AU8D00546 */
#endif
/* j00100803 Add End 2008-02-28 */

//**************************************************************************
// Function Name: BcmDb_getQosClassPriority
// Description  : convert QoS class priority from text to integer
// Returns      : QoS class priority.
//**************************************************************************
int BcmDb_getQosClassPriority(char *info) {
    int i = 0;    
    /* start of protocol QoS for KPN针对KPN QoS需求启用4条队列 by zhangchen z45221 2006年7月17日
    const char* priority[] = {"", "low", "medium", "high", NULL};
    for ( i = 0; priority[i] != NULL; i++ ) {
        if ( strcasecmp(priority[i], info) == 0 )
            break;
    }
    
    return i;
    */
#ifdef SUPPORT_VDSL
    #if SUPPORT_KPN_QOS   // macro for KPN QoS
    int trafficmode = 0;
    int ret = 0;

    ret = BcmDb_getTrafficMode(&trafficmode); //mode of flash data

    if (TM_ATM_BR == trafficmode || TM_PTM_BR == trafficmode)
    {
        const char* priority[] = {"", "low and low weight", "low and high weight", "medium", "high", NULL};        
        for ( i = 0; priority[i] != NULL; i++ ) {
            if ( strcasecmp(priority[i], info) == 0 )
                break;
        }
        
        return i;
    }
    else
    {
        const char* priority[] = {"", "low", "medium", "high", NULL};
        for ( i = 0; priority[i] != NULL; i++ ) {
            if ( strcasecmp(priority[i], info) == 0 )
                break;
        }
        
        return i;

    }
    #else
    const char* priority[] = {"", "low", "medium", "high", NULL};
    for ( i = 0; priority[i] != NULL; i++ ) {
        if ( strcasecmp(priority[i], info) == 0 )
            break;
    }
    
    return i;

    #endif
#else
    const char* priority[] = {"", "low", "medium", "high", NULL};
    for ( i = 0; priority[i] != NULL; i++ ) {
        if ( strcasecmp(priority[i], info) == 0 )
            break;
    }
    
    return i;

#endif
    /* end of protocol QoS for KPN针对KPN QoS需求启用4条队列 by zhangchen z45221 2006年7月17日 */
    
}

//**************************************************************************
// Function Name: BcmDb_getQosClassDscpMark
// Description  : convert QoS DSCP mark from text to integer
// Returns      : QoS classification DSCP mark.
//**************************************************************************
int BcmDb_getQosClassDscpMark(char *info) {
    int i = 0;
    static char* dscpMarkDesc[] = {"default", "AF13", "AF12", "AF11", "CS1",
                                   "AF23", "AF22", "AF21", "CS2",
                                   "AF33", "AF32", "AF31", "CS3",
                                   "AF43", "AF42", "AF41", "CS4",
                                   "EF", "CS5", "CS6", "CS7", NULL};

    for ( i = 0; dscpMarkDesc[i] != NULL; i++ ) {
        if ( strcasecmp(dscpMarkDesc[i], info) == 0 )
            break;
    }

    return i;
}
#ifdef SUPPORT_SAMBA
void BcmDb_setSmbCfgInfo(PSAMBA_MGMT_CFG pSmbMgmtCfg) {
    PSI_HANDLE smbPsi = BcmPsi_appOpen(SAMBA_APPID);
    
    BcmPsi_objStore(smbPsi, SAMBA_STATUS_ID, pSmbMgmtCfg, sizeof(SAMBA_MGMT_CFG));
}

void BcmDb_setSmbAccntSize(int info) {
    UINT16 size = info;
    PSI_HANDLE smbPsi = BcmPsi_appOpen(SAMBA_APPID);
    
    BcmPsi_objStore(smbPsi, SAMBA_ACCOUNT_NUM_ID, &size, sizeof(size));
}

void BcmDb_setSmbAccntInfo(PSAMBA_ACCNT_ENTRY tbl, int size) {

    PSI_HANDLE smbPsi = BcmPsi_appOpen(SAMBA_APPID);
    
    BcmPsi_objStore(smbPsi, SAMBA_ACCOUNT_TBL_ID, (PSI_VALUE)tbl, sizeof(SAMBA_ACCNT_ENTRY) * size);

}
#endif

//**************************************************************************
// Function Name: BcmDb_getAccessControlSize
// Description  : retrieve access control size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getAccessControlSize(int *info) {
    return BcmDb_getSecuritySize(SCM_ACC_CNTR_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setAccessControlSize
// Description  : store access control size
// Returns      : none.
//**************************************************************************
void BcmDb_setAccessControlSize(int info) {
    BcmDb_setSecuritySize(SCM_ACC_CNTR_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getAccessControlInfo
// Description  : retrieve access control table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getAccessControlInfo(PSEC_ACC_CNTR_ENTRY tbl, int size) {
    return BcmDb_getSecurityInfo(SCM_ACC_CNTR_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SEC_ACC_CNTR_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setAccessControlInfo
// Description  : store access control table
// Returns      : none.
//**************************************************************************
void BcmDb_setAccessControlInfo(PSEC_ACC_CNTR_ENTRY tbl, int size) {
    BcmDb_setSecurityInfo(SCM_ACC_CNTR_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SEC_ACC_CNTR_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getAccessControlMode
// Description  : retrieve access control mode
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getAccessControlMode(int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    status = BcmPsi_objRetrieve(secPsi, SCM_ACC_CNTR_MODE_ID, info,
                                sizeof(int), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setAccessControlMode
// Description  : store access control mode
// Returns      : none.
//**************************************************************************
void BcmDb_setAccessControlMode(int info) {
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_ACC_CNTR_MODE_ID, &info, sizeof(info));
}

#ifdef SUPPORT_VDSL
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
//**************************************************************************
// Function Name: BcmDb_getTrafficMode
// Description  : retrieve Traffic mode
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getTrafficMode(int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE atmcfgPsi = BcmPsi_appOpen("AtmCfg");
    
    status = BcmPsi_objRetrieve(atmcfgPsi, 8, info,
                                sizeof(int), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setTrafficMode
// Description  : store traffic mode
// Returns      : none.
//**************************************************************************
void BcmDb_setTrafficMode(int info) {
    PSI_HANDLE atmcfgPsi = BcmPsi_appOpen("AtmCfg");
    
    BcmPsi_objStore(atmcfgPsi, 8, &info, sizeof(info));
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#endif

/***************************************************************************
// Function Name: BcmDb_getDftSrvCntrInfo.
// Description  : retrieve default services control information.
// Returns      : none.
****************************************************************************/
void BcmDb_getDftSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo) {
    // initialize default value for all modes is SSC_MODE_ENABLE_LAN
/*    
   pInfo->modeFtp = SSC_MODE_DISABLE;
    pInfo->modeHttp = SSC_MODE_ENABLE_LAN;
    pInfo->modeIcmp = SSC_MODE_ENABLE_LAN;
    pInfo->modeSnmp = SSC_MODE_ENABLE_LAN;
    pInfo->modeSsh = SSC_MODE_ENABLE_LAN;
    pInfo->modeTelnet = SSC_MODE_ENABLE_LAN;
    pInfo->modeTftp = SSC_MODE_DISABLE;
 */
	/*start-add by 00112761 for VDF_C02*/
	pInfo->modeFtp = SSC_MODE_ENABLE_LAN;
    pInfo->modeHttp = SSC_MODE_ENABLE_LAN;
    pInfo->modeIcmp = SSC_MODE_DISABLE;
    pInfo->modeSnmp = SSC_MODE_DISABLE;
    pInfo->modeSsh = SSC_MODE_DISABLE;
    pInfo->modeTelnet = SSC_MODE_DISABLE;
    pInfo->modeTftp = SSC_MODE_DISABLE;
    /* BEGIN: Added by weishi kf33269, 2011/6/26   PN:Issue407:TR-069 service is not protected with access control*/
    //pInfo->modeTr069 = SSC_MODE_DISABLE;
	pInfo->modeTr069 = SSC_MODE_ENABLE_WAN;
   /* END:   Added by weishi kf33269, 2011/6/26 */
	/*end-add by 00112761 for VDF_C02*/
}

/***************************************************************************
// Function Name: BcmDb_getSrvCntrInfo.
// Description  : retrieve service control list information from PSI.
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
****************************************************************************/
int BcmDb_getSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( pInfo == NULL ) return DB_GET_NOT_FOUND;

    PSI_STATUS status = BcmPsi_objRetrieve(secPsi, SCM_SRV_CNTR_ID,
        pInfo, sizeof(SEC_SRV_CNTR_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        BcmDb_getDftSrvCntrInfo(pInfo);
        BcmDb_setSrvCntrInfo(pInfo);
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}

/***************************************************************************
// Function Name: BcmDb_setSrvCntrInfo.
// Description  : store service control list information from PSI.
// Returns      : none.
****************************************************************************/
void BcmDb_setSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo) {
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( pInfo == NULL ) return;

    BcmPsi_objStore(secPsi, SCM_SRV_CNTR_ID,
                    pInfo, sizeof(SEC_SRV_CNTR_INFO));
}

//**************************************************************************
// Function Name: BcmDb_getSrvCntrAccess
// Description  : convert access type from text to integer
// Returns      : allowed access type.
//**************************************************************************
int BcmDb_getSrvCntrAccess(char *info) {
    int i = 0;    
    const char* where[] = { "disable", "enable", "lan", "wan", NULL };
    
    for ( i = 0; where[i] != NULL; i++ ) {
        if ( strcasecmp(where[i], info) == 0 )
            break;
    }
    
    return i;
}

/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月28日 */
#ifdef SUPPORT_VDSL
#if SUPPORT_KPN_QOS     // macro QoS for KPN
/***************************************************************************
// Function Name: BcmDb_getDftWredInfo.
// Description  : retrieve default wred information.
// Returns      : none.
****************************************************************************/
void BcmDb_getDftWredInfo(PSEC_WRED_INFO pInfo){
    // initialize default value for WRED info
    strncpy(pInfo->WredWeightRatio, WRED_WEIGHT_RATIO, SEC_BUFF_MAX_LEN - 1);
    pInfo->WredWeightRatio[SEC_BUFF_MAX_LEN - 1] = '\0';
}

/***************************************************************************
// Function Name: BcmDb_getWredInfo.
// Description  : retrieve wred information from PSI.
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
****************************************************************************/
int BcmDb_getWredInfo(PSEC_WRED_INFO pInfo)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( pInfo == NULL ) return DB_GET_NOT_FOUND;

    PSI_STATUS status = BcmPsi_objRetrieve(secPsi, SCM_WRED_INFO_ID,
        pInfo, sizeof(SEC_WRED_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        BcmDb_getDftWredInfo(pInfo);
        BcmDb_setWredInfo(pInfo);
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}

/***************************************************************************
// Function Name: BcmDb_setWredInfo.
// Description  : store wred information from PSI.
// Returns      : none.
****************************************************************************/
void BcmDb_setWredInfo(PSEC_WRED_INFO pInfo){
    PSI_HANDLE wredPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( pInfo == NULL ) return;

    BcmPsi_objStore(wredPsi, SCM_WRED_INFO_ID,
                    pInfo, sizeof(SEC_WRED_INFO));
}
#endif
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月28日 */
/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
/***************************************************************************
// Function Name: BcmDb_getDmzHost.
// Description  : get IP address of the DMZ host from PSI.
// Returns      : none.
****************************************************************************/
int BcmDb_getDmzHost(char *enable, char *address) {
    int ret = DB_GET_OK;   
    PSEC_DMZ_ENTRY pInfo;
    UINT16 retLen = 0;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( (enable == NULL) && (address == NULL) ) 
    {
        return DB_GET_NOT_FOUND;
    }
    
    enable[0] = '\0';
    address[0] = '\0';

    PSI_STATUS status = BcmPsi_objRetrieve(secPsi, SCM_DMZ_HOST_ID,
                               pInfo, sizeof(SEC_DMZ_ENTRY), &retLen);
    if ( status == PSI_STS_OK && pInfo->dmzAddress.s_addr != 0 )
    { 
        if( pInfo->dmzEnable == 1)
        {
            strcpy(enable, "1");
        }
        else
        {
            strcpy(enable, "0");
        }
        
        strcpy(address, inet_ntoa(pInfo->dmzAddress));
    }
    else
    {
        ret = DB_GET_NOT_FOUND;
    }
    
    return ret;
}

/***************************************************************************
// Function Name: BcmDb_setDmzHost.
// Description  : store IP address of the DMZ host to PSI.
// Returns      : none.
****************************************************************************/
void BcmDb_setDmzHost(PSEC_DMZ_ENTRY pInfo) {
    
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    if ( pInfo == NULL ) return;

    // store the new DMZ host
    BcmPsi_objStore(secPsi, SCM_DMZ_HOST_ID, pInfo, sizeof(SEC_DMZ_ENTRY));
}
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
//**************************************************************************
// Function Name: BcmDb_getRouteSize
// Description  : retrieve route size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRouteSize(int id, int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE routePsi = BcmPsi_appOpen(ROUTE_APPID);
    
    status = BcmPsi_objRetrieve(routePsi, id, &size, sizeof(size), &retLen);

    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setRouteSize
// Description  : store route size
// Returns      : none.
//**************************************************************************
void BcmDb_setRouteSize(int id, int info) {
    UINT16 size = info;
    PSI_HANDLE routePsi = BcmPsi_appOpen(ROUTE_APPID);
    
    BcmPsi_objStore(routePsi, id, &size, sizeof(size));
}

//**************************************************************************
// Function Name: BcmDb_getRouteInfo
// Description  : retrieve route table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRouteInfo(int id, PSI_VALUE info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE routePsi = BcmPsi_appOpen(ROUTE_APPID);
    
    status = BcmPsi_objRetrieve(routePsi, id, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setRouteInfo
// Description  : store route table
// Returns      : none.
//**************************************************************************
void BcmDb_setRouteInfo(int id, PSI_VALUE info, int size) {
    PSI_HANDLE routePsi = BcmPsi_appOpen(ROUTE_APPID);
    
    BcmPsi_objStore(routePsi, id, info, size);
}

//**************************************************************************
// Function Name: BcmDb_getStaticRouteSize
// Description  : retrieve static route size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getStaticRouteSize(int *info) {
    return BcmDb_getRouteSize(RCM_ROUTE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setStaticRouteSize
// Description  : store static route size
// Returns      : none.
//**************************************************************************
void BcmDb_setStaticRouteSize(int info) {
    BcmDb_setRouteSize(RCM_ROUTE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getStaticRouteInfo
// Description  : retrieve static route table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getStaticRouteInfo(PRT_ROUTE_CFG_ENTRY tbl, int size) {
    return BcmDb_getRouteInfo(RCM_ROUTE_CFG_TBL_ID, (PSI_VALUE)tbl,
                              sizeof(RT_ROUTE_CFG_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setAccessControlInfo
// Description  : store access control table
// Returns      : none.
//**************************************************************************
void BcmDb_setStaticRouteInfo(PRT_ROUTE_CFG_ENTRY tbl, int size) {
    BcmDb_setRouteInfo(RCM_ROUTE_CFG_TBL_ID, (PSI_VALUE)tbl,
                       sizeof(RT_ROUTE_CFG_ENTRY) * size);
}

#ifdef SUPPORT_RIP
//**************************************************************************
// Function Name: BcmDb_getRipInfo
// Description  : retrieve global rip info
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRipInfo(PRT_RIP_CFG rip) {
    return BcmDb_getRouteInfo(RCM_RIP_CFG_ID, (PSI_VALUE)rip, sizeof(RT_RIP_CFG));
}

//**************************************************************************
// Function Name: BcmDb_setAccessControlInfo
// Description  : store global rip info
// Returns      : none.
//**************************************************************************
void BcmDb_setRipInfo(PRT_RIP_CFG rip) {
    BcmDb_setRouteInfo(RCM_RIP_CFG_ID, (PSI_VALUE)rip, sizeof(RT_RIP_CFG));
}

//**************************************************************************
// Function Name: BcmDb_getRipIfcInfo
// Description  : retrieve rip interface table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getRipIfcInfo(PRT_RIP_IFC_ENTRY tbl, int size) {
    return BcmDb_getRouteInfo(RCM_RIP_IFC_TBL_ID, (PSI_VALUE)tbl,
                              sizeof(RT_RIP_IFC_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setRipIfcInfo
// Description  : store rip interface table
// Returns      : none.
//**************************************************************************
void BcmDb_setRipIfcInfo(PRT_RIP_IFC_ENTRY tbl, int size) {
    BcmDb_setRouteInfo(RCM_RIP_IFC_TBL_ID, (PSI_VALUE)tbl,
                       sizeof(RT_RIP_IFC_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_getRipOperation
// Description  : convert rip operation from text to integer
// Returns      : rip operation.
//**************************************************************************
int BcmDb_getRipOperation(char *info) {
    int i = 0;    
    const char* opers[] = { "active", "passive", NULL };
    
    for ( i = 0; opers[i] != NULL; i++ ) {
        if ( strcasecmp(opers[i], info) == 0 )
            break;
    }
    
    return i;
}

//**************************************************************************
// Function Name: BcmDb_getRipVersion
// Description  : convert rip version from text to integer
// Returns      : rip version.
//**************************************************************************
int BcmDb_getRipVersion(char *info) {
    int i = 0;    
    const char* versions[] = {"", "1", "2", "1_2", NULL };
    
    for ( i = 0; versions[i] != NULL; i++ ) {
        if ( strcasecmp(versions[i], info) == 0 )
            break;
    }
    
    return i;
}
#endif /* SUPPORT_RIP */
/*GLB:s53329,start add for DNS*/
//**************************************************************************
// Function Name: BcmDb_getDnsdomainSize
// Description  : get dns size
// Returns      : none.
//**************************************************************************
int BcmDb_getDnsdomainSize(int *info) 
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;

    if ( NULL == info)
    {
        return DB_GET_NOT_FOUND;
    }
    PSI_HANDLE sysPsi = BcmPsi_appOpen(PSI_APP_DNS);
    
    status = BcmPsi_objRetrieve(sysPsi, DNS_SIZE, &size, sizeof(size), &retLen);
    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setDnsdomainSize
// Description  : store dns size
// Returns      : none.
//**************************************************************************
void BcmDb_setDnsdomainSize(int info) {
    UINT16 size = info;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(PSI_APP_DNS);
    
    BcmPsi_objStore(sysPsi, DNS_SIZE, &size, sizeof(size));
}
//**************************************************************************
// Function Name: BcmDb_getDnsDomainInfo
// Description  : retrieve rip interface table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getDnsDomainInfo(PDNS_CFG info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(PSI_APP_DNS);
    
    status = BcmPsi_objRetrieve(sysPsi, DNS_INFO, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setRipIfcInfo
// Description  : store rip interface table
// Returns      : none.
//**************************************************************************
void BcmDb_setDnsDomainInfo(PDNS_CFG info, int size) {
    PSI_HANDLE sysPsi = BcmPsi_appOpen(PSI_APP_DNS);
    BcmPsi_objStore(sysPsi, DNS_INFO, info, size);
}
/*GLB:s53329,end add for DNS*/
#if 0
/* HUAWEI HGW w69233 2008-03-29 Add begin */
/* Fix AU8D00334: EquipmentTest command "SetSsid/SetWepKey/SetWpaKey" for wlan does not work */
//**************************************************************************
// Function Name: BcmDb_setRestoreWlanDefaultFlag
// Description  : set the flag that restore wlan configuration(ssid/wepkey/wpakey) to default configuration
// Returns      : None.
//**************************************************************************
void BcmDb_setRestoreWlanDefaultFlag(void) 
{
   /* this is the implement on old flash partition
   sysVariableSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);*/
   sysAvailSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);
}
/* HUAWEI HGW w69233 2008-03-29 Add end */
#endif
//**************************************************************************
// Function Name: BcmDb_restoreDefault
// Description  : restore system configuration to default configuration
// Returns      : None.
//**************************************************************************
int getssid(char * ssid,int len ,int ssid_count )
{
	if( ssid == NULL )
	{
		printf("there is somthing error:\n");
		return CHECK_ERROR;
	}
	if( len < WLAN_SSID_LEN )
	{
		printf("there is somthing error:\n");
		return CHECK_ERROR;
	}
	memset(ssid,0,len);
	char acSsid[WLAN_SSID_LEN];   
    memset(acSsid, 0, sizeof(acSsid));
    sysFixGet(acSsid, WLAN_SSID_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN+RESTORE_FLAG_LEN); 
	if( !isgraph(acSsid[0]) )
	{
		printf("WARNING:SSID is ERROR,SSID isn't printable,you should config the ssid printable\n");
		return CHECK_ERROR;
	}
	
	strncpy(ssid,acSsid,strlen(acSsid));
	if( ssid_count == 2 && (strlen(acSsid)+6 < len))
	{
		strcat(ssid,"-Guest");
	}
	
		

    return 0;

}
//**************************************************************************
// Function Name: BcmDb_restoreDefault
// Description  : restore system configuration to default configuration
// Returns      : None.
//**************************************************************************
int getwpakey(char *wpakey,int len )
{
	if( wpakey == NULL )
	{
		printf("there is somthing error:\n");
		return CHECK_ERROR;
	}
	if( len < WLAN_WPA_PSK_LEN )
	{
		printf("there is somthing error:\n");
		return CHECK_ERROR;
	}
	memset(wpakey,0,len);
	char acwpakey[WLAN_WPA_PSK_LEN];
	memset(acwpakey,0,sizeof(acwpakey));
	sysFixGet(acwpakey, WLAN_WPA_PSK_LEN, WLAN_PARAMETERS_OFFSET+WLAN_WEP_KEY_LEN);
	if(strlen(acwpakey) < 8 )
	{
		printf("get wpakey error:\n");
		return CHECK_ERROR;
	}
	if( !isgraph(acwpakey[0]) )
	{
		printf("WARING:WPAKEY is error,must check it\n");
		return CHECK_ERROR;
	}

	strncpy(wpakey,acwpakey,len);
	return 0;
}

//**************************************************************************
// Function Name: BcmDb_restoreDefault
// Description  : restore system configuration to default configuration
// Returns      : None.
//**************************************************************************
int Check_restore(void )
{
	  int count_test = 0;
	
		struct stat statbuf_foretc;
		FILE *fp_etc = fopen(PSI_PATH_DEFAULTCONFIG,"r");
		if(NULL == fp_etc )
		{
			return CHECK_ERROR;
		}
		FILE *fp_var = fopen(PSI_XML_CHECK_RESTORE,"a+");
		if( NULL == fp_var )
		{
			fclose(fp_etc);
			return CHECK_ERROR;
		}
		fseek(fp_etc,0,SEEK_SET);
		fseek(fp_var,0,SEEK_SET);
		if( stat(PSI_PATH_DEFAULTCONFIG,&statbuf_foretc) != 0 )
		{
			fclose(fp_etc);
			fclose(fp_var);
			printf("GET file size error:\n");
			return CHECK_ERROR;
		}
		char temp_buf[LINE_LEN];
		char backup_buf[LINE_LEN];
		char acSsid[WLAN_SSID_LEN];
		char acBuf[WLAN_SSID_LEN];
		char wpakey[WLAN_WPA_PSK_LEN];
		memset(acBuf,0,sizeof(acBuf));
		memset(backup_buf,0,LINE_LEN);
		memset(temp_buf,0,LINE_LEN);
		int ssid_count = 0;
		int wpakey_count = 0;
		char *temp_ssid = NULL;
		char * temp_wpakey = NULL;
		char *delim = " ";
		int old_len = 0;
		char * p = NULL;
		int result = 0;
		while( fgets(temp_buf,LINE_LEN,fp_etc) != NULL )
		{
			if( ssid_count != 2 && wpakey_count != 2 )
			{
				memset(backup_buf,0,sizeof(backup_buf));
			//读取配置文件每一行;
				temp_ssid = strtok(temp_buf,delim);
				old_len = strlen(temp_ssid);
				memcpy(backup_buf,temp_ssid,old_len);
			
			
				while(( p = strtok(NULL,delim)) != NULL )
				{
					backup_buf[old_len] = ' ';
				//当找到ssid的时候，需要进行配置；
					if( strncmp(p,"ssId=",5) == 0 && ssid_count < 2 )
					{
				
					
						ssid_count ++;
				
						result = getssid(acSsid,WLAN_SSID_LEN,ssid_count);
						if( result != 0 )
						{
							printf("GET acSsid failed,should check it:\n");
							return CHECK_ERROR;
						}
						memcpy(backup_buf+old_len+1,p,strlen(p)-1);
					
						old_len += strlen(p);
						memcpy(backup_buf+old_len,acSsid,strlen(acSsid));
					
						old_len += strlen(acSsid);
						backup_buf[old_len] = '\"';
			
						old_len += 1;
						continue;
					}
					if( strncmp(p,"wpakey=",7) == 0 && wpakey_count < 2 )
					{
				
						wpakey_count ++;
				
						result = getwpakey(wpakey,WLAN_WPA_PSK_LEN);
					
						if( result != 0 )
						{
							printf("get WPAKEY failed:should check it:\n");
							return CHECK_ERROR;
						}
						memcpy(backup_buf+old_len+1,p,strlen(p) - 1);
					
						old_len += strlen(p) ;
						memcpy(backup_buf+old_len,wpakey,strlen(wpakey));
					
						old_len += strlen(wpakey);
						backup_buf[old_len] = '\"';

						sleep(1);
						old_len += 1;
						continue;
					}
				
					memcpy(backup_buf+old_len+1,p,strlen(p));
			
					old_len += strlen(p)+1;
				
				

				}
			}	
			else if( ssid_count == 2 && wpakey_count == 2 )
			{
				old_len = strlen(temp_buf);
				memcpy(backup_buf,temp_buf,old_len);
			}
			
			//backup_buf[old_len] = '\n';
			
		
			//old_len += 1;
	
			if(fwrite(backup_buf,1,old_len,fp_var) != old_len )
			{
				printf("write stream to file failed:\n");
				fclose(fp_etc);
				fclose(fp_var);
				return CHECK_ERROR;
			}
			/*处理SSID*/
	
			old_len = 0;
			
		}
	
		fclose(fp_etc);
		fclose(fp_var);
		/*for test*/
	
		return 0;
		
}
/*end:add by kf19988 for improve restore default*/
//**************************************************************************
// Function Name: BcmDb_restoreDefault
// Description  : restore system configuration to default configuration
// Returns      : None.
//**************************************************************************
//add by z67625
int BcmDb_restoreDefault(void) 
{
	
	int diag_check = 0;
   int iRet = 0; 
   char cUpgradeFlags[FAILURE_TAG_LEN];
   /*start-add by 00112761 for VDF*/
   char temp[IFC_PIN_PASSWORD_LEN];
   int iMsgKeySend;
   /*Begin:Added by luokunling l00192527,2011/10/13,BN:IMSI inform*/
   char DefaultIMSINum[16] = {0};
   /*End:Added by luokunling l00192527 ,2011/10/13.*/
   struct stAtSndMsg stSndFunMsg;                      //发送消息(放入发送消息队列) 

   /*end-add by 00112761 for VDF*/
    /*start of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
   char cBuf[CWMP_PARA_LEN];
    /*end  of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
   /*start of 增加清除激活标志位 by s53329 at  20080917*/
   char chActivation[GATEWAY_ACTIVATION_INFO_LEN];
   /*end  of 增加清除激活标志位 by s53329 at  20080917*/
   char tmp[8];
   //add by z67625
   int status = PSI_STS_OK;
   /*start:modified by kf19988 for improve restore default*/
	 FILE *fp = fopen(DIAG_MESSAGE,"r");
	 if( NULL == fp )
	 {
		printf("you will restore config and reboot the router:\n");
		
	 }
	 else
	 {
	 	char check[20];
		memset(check,0,sizeof(check));
		fgets(check,20,fp);
		if( strstr(check,"diag check=1"))
		{
			diag_check = 1;
			fclose(fp);
			system("rm -rf /var/message");
		}
		
		fclose(fp);
	 }
	 if( diag_check == 1 )
	 {
			if( Check_restore() != 0 )
			{
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:\n");
				printf("WARNING:the DiagEquipment Commond test  error,ssid or wpakey isn't exsit ,must check.......:\n");
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:\n");
				return CHECK_ERROR;
			}
		 	if ( BcmPsi_writeFileToFlash(PSI_XML_CHECK_RESTORE) != PSI_STS_OK )
  		 	{
      			status = PSI_STS_ERR_GENERAL;
      			if(BcmPsi_profileErase() != PSI_STS_OK)
      			{
        		 status = PSI_STS_ERR_GENERAL;
      			}
   			}
   		
	 }
	 else
	 {
	 
   			if ( BcmPsi_writeFileToFlash(PSI_PATH_DEFAULTCONFIG) != PSI_STS_OK )
   			{
      			status = PSI_STS_ERR_GENERAL;
      			if(BcmPsi_profileErase() != PSI_STS_OK)
      			{
         			status = PSI_STS_ERR_GENERAL;
      			}
   			}
   			
	 }
	if(BcmCfm_scratchPadClearAll() != 0)
   	{
       		status = PSI_STS_ERR_GENERAL;
   	}

	/*end:modified by kf19988 for improve restore default*/
    /*start of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
   memset(cBuf, 0, CWMP_PARA_LEN);
    /*end  of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
  /*start-add by 00112761 for VDF:清除FLASH中存储的PIN码*/
   memset(temp, 255, sizeof(temp));
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
   sysVariableSet(temp, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
   */
   sysAvailSet(temp, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
  /*end-add by 00112761 for VDF*/
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
   sysVariableSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);
   */
   /* HUAWEI HGW w69233 2008-03-29 Modify begin */
   /* Fix AU8D00334: EquipmentTest command "SetSsid/SetWepKey/SetWpaKey" for wlan does not work */
   /*sysAvailSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);*/
//   BcmDb_setRestoreWlanDefaultFlag();
   /* HUAWEI HGW w69233 2008-03-29 Modify end */  
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
   
   /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
   iRet = sysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
   */
   iRet = sysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
   if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
       && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
   {
       /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
       sysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
       */
       sysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
       /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
   }   
   /* BEGIN: Modified by y67514, 2009/11/29   问题单号:cwmp 0 bootstrap 问题*/
   //sysAvailSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
   sysVariableSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
   /* END:   Modified by y67514, 2009/11/29 */
   /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */

   /* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 add begin:*/
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
   sysVariableSet("restore", TELNET_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN );
   */
   sysAvailSet("restore", TELNET_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN );
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
   /* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 add end.*/

   /* HUAWEI HGW s48571 2008年3月10日 默认关闭ping功能 add begin:*/
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
   sysVariableSet("resping", PING_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN );
   */
   sysAvailSet("resping", PING_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN );
   /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
   /* HUAWEI HGW s48571 2008年3月10日 默认关闭ping功能 add end.*/
   /*start of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
   sysAvailSet(cBuf, CWMP_PARA_LEN, CWMP_PARA_OFFSET);
   /*start of 减少系统启动时写flash 动作 by s53329  at  20080420
   sysAvailSet("HGReset", CWMP_PARA_RESET, CWMP_PARA_OFFSET+CWMP_PARA_LEN);
   end of 减少系统启动时写flash 动作 by s53329  at  20080420*/
   sysAvailSet("HG553VF", CWMP_PARA_RESET, CWMP_PARA_OFFSET+CWMP_PARA_LEN); 
   /*end  of 需要清空flash 存贮的参数属性 by s53329  at  20080319*/
   /*start of 增加清除激活标志位 by s53329 at  20080917*/
   strcpy(chActivation, "0"); 
   sysAvailSet(chActivation, GATEWAY_ACTIVATION_INFO_LEN,GATEWAY_ACTIVATION_INFO);
   /*end of 增加清除激活标志位 by s53329 at  20080917*/
   
 /*Begin:Added by luokunling l00192527,2011/10/13,BN:IMSI inform*/
   memset(DefaultIMSINum,0,IFC_IMSI_LEN);
   sysAvailSet(DefaultIMSINum, IFC_IMSI_LEN, IMSI_OFFSET);       //擦除flash中存放的IMSI值
   /*End:Added by luokunling l00192527 ,2011/10/13.*/

   /* WebUI language. <tanyin 2009.2.16> */
   memset(tmp, 0x00, sizeof(tmp));
   sysAvailSet(tmp, WEB_LANGUAGE_LEN, WEB_LANGUAGE_OFFSET);
   /* WebUI language for admin. <tanyin 2009.5.3> */
   sysAvailSet(tmp, WEB_ADMIN_LANGUAGE_LEN, WEB_ADMIN_LANGUAGE_OFFSET);
   

   /* HUAWEI HGW s48571 2008年4月7日" AU8D00463 restoredefault命令不能恢复出厂配置 modify begin:*/
   printf("Reset the Hspa stick!\n");
   /*iMsgKeySend = msgget(MSG_AT_QUEUE, 0666);
   AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=4");
   AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=6");
   sysSetVoipServiceStatus(1);*/
   /*Begin: Added bu luokunling l00192527,2011/12/14*/
   RebootHSPAStick();
  /*End: Added bu luokunling l00192527,2011/12/14*/
   //add by z67625
   return status;
   /* HUAWEI HGW s48571 2008年4月7日" AU8D00463 restoredefault命令不能恢复出厂配置 modify end. */
   
}
//**************************************************************************
// Function Name: BcmDb_getLanAllStats
// Description  : get the statistics of all LAN interfaces
// Returns      : None.
//**************************************************************************
void BcmDb_getLanAllStats(unsigned long statistics[]) {
   unsigned long stats[OBJ_STS_IFC_COLS];
   char interface[IFC_TINY_LEN];
   
   statistics[OBJ_STS_RX_BYTE] = statistics[OBJ_STS_RX_PACKET] = 0;
   statistics[OBJ_STS_RX_ERR] = statistics[OBJ_STS_RX_DROP] = 0;
   statistics[OBJ_STS_TX_BYTE] = statistics[OBJ_STS_TX_PACKET] = 0;
   statistics[OBJ_STS_TX_ERR] = statistics[OBJ_STS_TX_DROP] = 0;

   // get 'eth0' statistics   
   bcmGetIfcNameById(IFC_ENET_ID, interface);
   BcmDb_getIfcStats(STS_FROM_LAN, interface, stats);
   statistics[OBJ_STS_RX_BYTE] += stats[OBJ_STS_RX_BYTE];
   statistics[OBJ_STS_RX_PACKET] += stats[OBJ_STS_RX_PACKET];
   statistics[OBJ_STS_RX_ERR] += stats[OBJ_STS_RX_ERR];
   statistics[OBJ_STS_RX_DROP] += stats[OBJ_STS_RX_DROP];
   statistics[OBJ_STS_TX_BYTE] += stats[OBJ_STS_TX_BYTE];
   statistics[OBJ_STS_TX_PACKET] += stats[OBJ_STS_TX_PACKET];
   statistics[OBJ_STS_TX_ERR] += stats[OBJ_STS_TX_ERR];
   statistics[OBJ_STS_TX_DROP] += stats[OBJ_STS_TX_DROP];

#ifdef USB
   // get 'usb0' statistics   
   bcmGetIfcNameById(IFC_USB_ID, ifcName);
   BcmDb_getIfcStats(STS_FROM_LAN, interface, stats);
   statistics[OBJ_STS_RX_BYTE] += stats[OBJ_STS_RX_BYTE];
   statistics[OBJ_STS_RX_PACKET] += stats[OBJ_STS_RX_PACKET];
   statistics[OBJ_STS_RX_ERR] += stats[OBJ_STS_RX_ERR];
   statistics[OBJ_STS_RX_DROP] += stats[OBJ_STS_RX_DROP];
   statistics[OBJ_STS_TX_BYTE] += stats[OBJ_STS_TX_BYTE];
   statistics[OBJ_STS_TX_PACKET] += stats[OBJ_STS_TX_PACKET];
   statistics[OBJ_STS_TX_ERR] += stats[OBJ_STS_TX_ERR];
   statistics[OBJ_STS_TX_DROP] += stats[OBJ_STS_TX_DROP];
#endif

#ifdef WIRELESS
   // get 'wl0' statistics   
   bcmGetIfcNameById(IFC_WIRELESS_ID, interface);
   BcmDb_getIfcStats(STS_FROM_LAN, interface, stats);
   statistics[OBJ_STS_RX_BYTE] += stats[OBJ_STS_RX_BYTE];
   statistics[OBJ_STS_RX_PACKET] += stats[OBJ_STS_RX_PACKET];
   statistics[OBJ_STS_RX_ERR] += stats[OBJ_STS_RX_ERR];
   statistics[OBJ_STS_RX_DROP] += stats[OBJ_STS_RX_DROP];
   statistics[OBJ_STS_TX_BYTE] += stats[OBJ_STS_TX_BYTE];
   statistics[OBJ_STS_TX_PACKET] += stats[OBJ_STS_TX_PACKET];
   statistics[OBJ_STS_TX_ERR] += stats[OBJ_STS_TX_ERR];
   statistics[OBJ_STS_TX_DROP] += stats[OBJ_STS_TX_DROP];
#endif
}

//**************************************************************************
// Function Name: BcmDb_getWanAllStats
// Description  : get the statistics of all WAN interfaces
// Returns      : None.
//**************************************************************************
void BcmDb_getWanAllStats(unsigned long statistics[]) {
   unsigned long stats[OBJ_STS_IFC_COLS];
   char interface[IFC_TINY_LEN];
   WAN_CON_ID wanId;
   WAN_CON_INFO wanInfo;
   
   statistics[OBJ_STS_RX_BYTE] = statistics[OBJ_STS_RX_PACKET] = 0;
   statistics[OBJ_STS_RX_ERR] = statistics[OBJ_STS_RX_DROP] = 0;
   statistics[OBJ_STS_TX_BYTE] = statistics[OBJ_STS_TX_PACKET] = 0;
   statistics[OBJ_STS_TX_ERR] = statistics[OBJ_STS_TX_DROP] = 0;
   
   // init wanId to get WAN info from the begining
   wanId.vpi = wanId.vci = wanId.conId = 0;
   while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
      BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
      BcmDb_getIfcStats(STS_FROM_WAN, interface, stats);
      statistics[OBJ_STS_RX_BYTE] += stats[OBJ_STS_RX_BYTE];
      statistics[OBJ_STS_RX_PACKET] += stats[OBJ_STS_RX_PACKET];
      statistics[OBJ_STS_RX_ERR] += stats[OBJ_STS_RX_ERR];
      statistics[OBJ_STS_RX_DROP] += stats[OBJ_STS_RX_DROP];
      statistics[OBJ_STS_TX_BYTE] += stats[OBJ_STS_TX_BYTE];
      statistics[OBJ_STS_TX_PACKET] += stats[OBJ_STS_TX_PACKET];
      statistics[OBJ_STS_TX_ERR] += stats[OBJ_STS_TX_ERR];
      statistics[OBJ_STS_TX_DROP] += stats[OBJ_STS_TX_DROP];
   }
}

//**************************************************************************
// Function Name: BcmDb_getIfcStats
// Description  : get the statistics of the specific interface
// Returns      : None.
//**************************************************************************
void BcmDb_getIfcStats(int from, char *device, unsigned long statistics[]) {
   int count = 0;
   char *pChar = NULL;
   char col[17][32];
   char line[512], buf[512];
   FILE* fs = NULL;

   // initialize interfaces statistics
   statistics[OBJ_STS_RX_BYTE] = statistics[OBJ_STS_RX_PACKET] = 0;
   statistics[OBJ_STS_RX_ERR] = statistics[OBJ_STS_RX_DROP] = 0;
   statistics[OBJ_STS_TX_BYTE] = statistics[OBJ_STS_TX_PACKET] = 0;
   statistics[OBJ_STS_TX_ERR] = statistics[OBJ_STS_TX_DROP] = 0;

   if ( device == NULL ) return;

   // get statistics for all WAN interfaces
   if ( strcasecmp(device, "all") == 0 ) {
      if ( from == STS_FROM_LAN )
         BcmDb_getLanAllStats(statistics);
      else
         BcmDb_getWanAllStats(statistics);
      return;
   }
   
   fs = fopen("/proc/net/dev", "r");
   if ( fs == NULL ) return;
   
   // find interface
   while ( fgets(line, sizeof(line), fs) ) {
      // read pass 2 header lines
      if ( count++ < 2 ) continue;
      // if interface is found then store statistic values
      if ( strstr(line, device) != NULL ) {
         // normally line will have the following example value
         // "eth0: 19140785 181329 0 0 0 0 0 0 372073009 454948 0 0 0 0 0 0"
         // but when the number is too big then line will have the following example value
         // "eth0:19140785 181329 0 0 0 0 0 0 372073009 454948 0 0 0 0 0 0"
         // so to make the parsing correctly, the following codes are added
         // to insert space between ':' and number
         pChar = strchr(line, ':');
         if ( pChar != NULL ) pChar++;
         if ( pChar != NULL && isdigit(*pChar) ) {
            strcpy(buf, pChar);
            *pChar = ' ';
            strcpy(++pChar, buf);
         }
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
            col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7], col[8],
            col[9], col[10], col[11], col[12], col[13], col[14], col[15], col[16]);
         statistics[OBJ_STS_RX_BYTE] = atol(col[1]);
         statistics[OBJ_STS_RX_PACKET] = atol(col[2]);
         statistics[OBJ_STS_RX_ERR] = atol(col[3]);
         statistics[OBJ_STS_RX_DROP] = atol(col[4]);
         statistics[OBJ_STS_TX_BYTE] = atol(col[9]);
         statistics[OBJ_STS_TX_PACKET] = atol(col[10]);
         statistics[OBJ_STS_TX_ERR] = atol(col[11]);
         statistics[OBJ_STS_TX_DROP] = atol(col[12]);
         break;
      }
   }

   fclose(fs);
}

//**************************************************************************
// Function Name: BcmDb_getSpecIfcStats
// Description  : get specific field WAN statistics of specific interface
// Returns      : None.
//**************************************************************************
void BcmDb_getSpecIfcStats(int from, char *device,
                           int field, unsigned long statistics[]) {
   unsigned long stats[OBJ_STS_IFC_COLS];
   
   BcmDb_getIfcStats(from, device, stats);
   
   switch ( field ) {
      case OBJ_STS_RX_BYTE:
         *statistics = stats[OBJ_STS_RX_BYTE];
         break;
      case OBJ_STS_RX_PACKET:
         *statistics = stats[OBJ_STS_RX_PACKET];
         break;
      case OBJ_STS_RX_ERR:
         *statistics = stats[OBJ_STS_RX_ERR];
         break;
      case OBJ_STS_RX_DROP:
         *statistics = stats[OBJ_STS_RX_DROP];
         break;
      case OBJ_STS_TX_BYTE:
         *statistics = stats[OBJ_STS_TX_BYTE];
         break;
      case OBJ_STS_TX_PACKET:
         *statistics = stats[OBJ_STS_TX_PACKET];
         break;
      case OBJ_STS_TX_ERR:
         *statistics = stats[OBJ_STS_TX_ERR];
         break;
      case OBJ_STS_TX_DROP:
         *statistics = stats[OBJ_STS_TX_DROP];
         break;
      case OBJ_STS_ALL:
         statistics[OBJ_STS_RX_BYTE] = stats[OBJ_STS_RX_BYTE];
         statistics[OBJ_STS_RX_PACKET] = stats[OBJ_STS_RX_PACKET];
         statistics[OBJ_STS_RX_ERR] = stats[OBJ_STS_RX_ERR];
         statistics[OBJ_STS_RX_DROP] = stats[OBJ_STS_RX_DROP];
         statistics[OBJ_STS_TX_BYTE] = stats[OBJ_STS_TX_BYTE];
         statistics[OBJ_STS_TX_PACKET] = stats[OBJ_STS_TX_PACKET];
         statistics[OBJ_STS_TX_ERR] = stats[OBJ_STS_TX_ERR];
         statistics[OBJ_STS_TX_DROP] = stats[OBJ_STS_TX_DROP];
         break;
   }
}

#ifdef SUPPORT_DDNSD
//**************************************************************************
// Function Name: BcmDb_getDdnsSize
// Description  : retrieve ddns size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getDdnsSize(int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE ddnsPsi = BcmPsi_appOpen(DDNS_APP_ID);
    
    status = BcmPsi_objRetrieve(ddnsPsi, DDNS_NUM_SERVICES, &size, sizeof(size), &retLen);

    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setDdnsSize
// Description  : store ddns size
// Returns      : none.
//**************************************************************************
void BcmDb_setDdnsSize(int info) {
    UINT16 size = info;
    PSI_HANDLE ddnsPsi = BcmPsi_appOpen(DDNS_APP_ID);
    
    BcmPsi_objStore(ddnsPsi, DDNS_NUM_SERVICES, &size, sizeof(size));
}

//**************************************************************************
// Function Name: BcmDb_getDdnsInfo
// Description  : retrieve ddns table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getDdnsInfo(PDDNS_SERVICE_INFO tbl, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE ddnsPsi = BcmPsi_appOpen(DDNS_APP_ID);
    
    status = BcmPsi_objRetrieve(ddnsPsi, DDNS_SERVICE_TABLE, tbl,
                                sizeof(DDNS_SERVICE_INFO) * size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setDdnsInfo
// Description  : store ddns table
// Returns      : none.
//**************************************************************************
void BcmDb_setDdnsInfo(PDDNS_SERVICE_INFO tbl, int size) {
    PSI_HANDLE ddnsPsi = BcmPsi_appOpen(DDNS_APP_ID);
    
    BcmPsi_objStore(ddnsPsi, DDNS_SERVICE_TABLE, tbl,
                    sizeof(DDNS_SERVICE_INFO) * size);
}
#endif /* #ifdef SUPPORT_DDNSD */

#ifdef SUPPORT_TOD
void BcmDb_setToDInfo(PTOD_ENTRY pToDInfo, int size) {
    PSI_HANDLE todPsi = BcmPsi_appOpen(TOD_APP_ID);

    BcmPsi_objStore(todPsi, TOD_ENTRY_TABLE, pToDInfo, sizeof(TOD_ENTRY) * size);
}

void BcmDb_setToDSize(int info) {
    UINT16 size = info;
    PSI_HANDLE todPsi = BcmPsi_appOpen(TOD_APP_ID);
    
    BcmPsi_objStore(todPsi, TOD_NUM_ENTRIES, &size, sizeof(size));
}
#endif /* #ifdef SUPPORT_TOD */

#ifdef SUPPORT_MAC  // add by l66195 for VDF 
void BcmDb_setMacInfo(PMAC_ENTRY pMacInfo, int size) {
    PSI_HANDLE macPsi = BcmPsi_appOpen(MAC_APP_ID);

    BcmPsi_objStore(macPsi, MAC_ENTRY_TABLE, pMacInfo, sizeof(MAC_ENTRY) * size);
}

void BcmDb_setMacSize(int info) {
    UINT16 size = info;
    PSI_HANDLE macPsi = BcmPsi_appOpen(MAC_APP_ID);
    
    BcmPsi_objStore(macPsi, MAC_NUM_ENTRIES, &size, sizeof(size));
}
#endif /* #ifdef SUPPORT_MAC */

#ifdef SUPPORT_MACMATCHIP  
void BcmDb_setMacMatchInfo(PMACMATCH_ENTRY pMacInfo, int size) {
    PSI_HANDLE macPsi = BcmPsi_appOpen(MACMATCH_APP_ID);

    BcmPsi_objStore(macPsi, MACMATCH_ENTRY_TABLE, pMacInfo, sizeof(MACMATCH_ENTRY) * size);
}

void BcmDb_setMacMatchSize(int info) {
    UINT16 size = info;
    PSI_HANDLE macPsi = BcmPsi_appOpen(MACMATCH_APP_ID);
    
    BcmPsi_objStore(macPsi, MACMATCH_NUM_ENTRIES, &size, sizeof(size));
}
#endif 

#if SUPPORT_PORT_MAP
//**************************************************************************
// Function Name: BcmDb_setPMapSize
// Description  : store port to PVC mapping size
// Returns      : none.
//**************************************************************************
void BcmDb_setPMapSize(int id, int info) {
    UINT16 size = info;
    PSI_HANDLE pmapPsi = BcmPsi_appOpen(PMAP_APPID);
    
    BcmPsi_objStore(pmapPsi, id, &size, sizeof(size));
}

//**************************************************************************
// Function Name: BcmDb_setPMapInfo
// Description  : store port to PVC mapping table
// Returns      : none.
//**************************************************************************
void BcmDb_setPMapInfo(int id, PSI_VALUE info, int size) {
    PSI_HANDLE pmapPsi = BcmPsi_appOpen(PMAP_APPID);
    
    BcmPsi_objStore(pmapPsi, id, info, size);
}

//**************************************************************************
// Function Name: BcmDb_setStaticPMapInfo
// Description  : store port tp PVC mapping table
// Returns      : none.
//**************************************************************************
void BcmDb_setStaticPMapInfo(PPORT_MAP_CFG_ENTRY tbl, int size) {
    BcmDb_setPMapInfo(PMCM_PMAP_CFG_TBL_ID, (PSI_VALUE)tbl,
                       sizeof(PORT_MAP_CFG_ENTRY) * size);
}

//**************************************************************************
// Function Name: BcmDb_setStaticPMapSize
// Description  : store port to PVC mapping size
// Returns      : none.
//**************************************************************************
void BcmDb_setStaticPMapSize(int info) {
    BcmDb_setPMapSize(PMCM_PMAP_NUM_ID, info);
}



//**************************************************************************
// Function Name: BcmDb_checkIfcString
// Description  : check if an IFC name exists in a list
//               (strstr won't work as we have names like wl0 and wl0.1)
//**************************************************************************
char BcmDb_checkIfcString(char *list, char *ifcname, char **loc) {
    char r1, r2;
    char *l;
    char *end = list + strlen(list)-1;
    l = strstr(list, ifcname);

    if (!strlen(ifcname))
	return 0;

    for (;;) {
        r1 =0; r2=0;
        if (l != NULL) {
            if (l == list) {
                r1 = 1;
            }
            else { // need to see | if not the first one
                if (*(l-1) == '|') {
                    r1 = 1;
                }
            }
            
            if (*(l + strlen(ifcname)) == 0 || *(l + strlen(ifcname)) == '|' ) { //need to see |if not last one
                r2 = 1;
            }

            if (r1 && r2) {
                if (loc != NULL) {
                    *loc = l;
                }
                return 1;
            }
            else {
                if (l < end) {
                    l = strstr(l+1, ifcname);
                }
            }
        }
        else {
            return 0;
        }
    }
}


//**************************************************************************
// Function Name: BcmDb_addBridgeIf
// Description  : add bridge interface configuration information
// Returns      : none.
//**************************************************************************
void BcmDb_addBridgeIf(char *groupName, char *ifName) 
{
    int i = 0;

    if (groupName == NULL || ifName == NULL)
        return;
   /*w44771 modified for A36D03264 begin, 2007-01-13*/
   #ifdef WIRELESS
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) 
    {
         if ((strcmp("ath0", ifName) == 0) && (BcmDb_checkIfcString(bridgeList[i].grpedIntf, ifName, NULL))) 
         {
            //printf("%s is already in %s.\n", ifName, bridgeList[i].groupName);
            return;
         }
    }
    #endif
   /*w44771 modified for A36D03264 begin, 2007-01-13*/

    // First we search if the bridge is already in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) 
    {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, groupName) == 0) 
        {
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            if (!BcmDb_checkIfcString(bridgeList[i].grpedIntf, ifName, NULL)) 
            {
                // Make sure we have enough space to fit in the new interface
                // and "|" token termination string.
                if (*bridgeList[i].grpedIntf == '\0') 
                {
                    strcat(bridgeList[i].grpedIntf, ifName);
                    return;
                } 
                else if ((strlen(bridgeList[i].grpedIntf) +
                    strlen(ifName) + 1) < PORT_MAP_INTF_LIST_SIZE) 
                {                                        
                    /*
                     * start of y42304 modified 20060801: 解决问题单A36D02308
                     * 在增加桥接口时插入最前面
                     */             

                    if (strstr(ifName, "eth") == NULL)   // nas和wlan设备插入追前面
                    {
                        char tmp_grpedIntf[PORT_MAP_INTF_LIST_SIZE];
                        memset(tmp_grpedIntf, 0, PORT_MAP_INTF_LIST_SIZE);
                        
                        tmp_grpedIntf[0] = '\0';
                        strcat(tmp_grpedIntf, ifName);
                        strcat(tmp_grpedIntf, "|");
                        strcat(tmp_grpedIntf, bridgeList[i].grpedIntf);
                        memset(bridgeList[i].grpedIntf, 0, PORT_MAP_INTF_LIST_SIZE);                        
                        strcat(bridgeList[i].grpedIntf, tmp_grpedIntf);
                    }
                    else    // eth设备追加到最后
                    {
                        strcat(bridgeList[i].grpedIntf, "|");
                        strcat(bridgeList[i].grpedIntf, ifName);                        
                    }
                    /* end of y42304 modified 20060801 */                   
                    
                    return;
                }
                else 
                {
                    // We do not have enough space to squeeze this interface
                    // into the list so just return;
                    //printf ("BcmDb_addBridgeIf: Not enough space\n");
                    return;
                }
            } 
            else 
            {
                //printf ("BcmDb_addBridgeIf: Already present\n");
                break;
            }
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_allocBridge
// Description  : Allocate new bridge entry in configuration
// Returns      : none.
//**************************************************************************
int BcmDb_allocBridge(char *bridgeName, char *ifName) {
    int ret = -1;
    int i = 0;

    if (bridgeName == NULL || ifName == NULL)
         return ret;

    // First we search if the bridge is already in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the entry in the list if present
        if (bridgeList[i].useFlag == 1) {
            if (strcmp(bridgeList[i].groupName, bridgeName) == 0) {
                // We found one with the same group name so return its
                // interface name.
                strncpy(ifName, bridgeList[i].ifName, IFC_TINY_LEN);
                ret = 0;
                return ret;
            }
        }
    }

    // First we search if the bridge is already in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate an unused entry in the list.
        if (bridgeList[i].useFlag == 0) {
            memset(bridgeList[i].groupName, '\0', IFC_TINY_LEN);
            // Copy and return interface allocated for this new bridge to the caller.
            strncpy(ifName, bridgeList[i].ifName, IFC_TINY_LEN);
            strncpy(bridgeList[i].groupName, bridgeName, IFC_TINY_LEN);
            bridgeList[i].useFlag = PMAP_CFG_ENTRY_INUSE;
            ret = 0;
            break;
        }
    }
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_getBridgeInfo
// Description  : retrieve bridge configuration information
// Returns      : none.
//**************************************************************************
void BcmDb_getBridgeInfo(const char *bridgeName, char *grpedIntf, int len) {
    int i = 0;

    if (bridgeName == NULL || grpedIntf == NULL)
        return;

    // Search if the bridge is in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, bridgeName) == 0) {
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            strncpy(grpedIntf, bridgeList[i].grpedIntf, len);
            return;
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_delBridgeIf
// Description  : delete bridge interface configuration information
// Returns      : none.
//**************************************************************************
void BcmDb_delBridgeIf(const char *bridgeName, char *ifName) {
    int i = 0;

    if (bridgeName == NULL || ifName == NULL)
        return;

    // Search if the bridge is in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, bridgeName) == 0) {
            char *prefix   = NULL;
            char *suffix   = NULL;
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            
            if (BcmDb_checkIfcString(bridgeList[i].grpedIntf, ifName, &prefix)) {
                char grpedIntf[PORT_MAP_INTF_LIST_SIZE];
                // We found the interface in the list. Now check if this is at
                // begining of the list.
                memset(grpedIntf, 0, PORT_MAP_INTF_LIST_SIZE);
                if (prefix == bridgeList[i].grpedIntf) {
                    if ((suffix = strstr(prefix, "|")) != NULL) {
                        // Skip the | and copy everything till the end.
                        strcpy (grpedIntf, (suffix + 1));
                    } else {
                        // This is the only interface for this group, so we can
                        // safely remove the group.
                        BcmDb_delBridge(bridgeList[i].groupName);
                        return;
                    }
                } else {
                    // Copy the prefix part of the string to the temp.
                    memcpy(grpedIntf, bridgeList[i].grpedIntf, ((prefix - 1) - bridgeList[i].grpedIntf));
                    if ((suffix = strstr(prefix, "|")) != NULL) {
                        // Append everything till the end including the | delimiter.
                        strcat(grpedIntf, suffix);
                    }
                }
                // Clear the original list and recopy it from the temp.
                memset(bridgeList[i].grpedIntf, 0, PORT_MAP_INTF_LIST_SIZE);
                memcpy(bridgeList[i].grpedIntf, grpedIntf, PORT_MAP_INTF_LIST_SIZE);
                return;
            }
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_delIfFromBridge
// Description  : deletes interface from bridge configuration
// Returns      : none.
//**************************************************************************
void BcmDb_delIfFromBridge(char *ifName) {
    int i = 0;
    int bridgeDelete = 0;
    int grpType = 0;
    char cmd[SYS_CMD_LEN];

    if (ifName == NULL)
        return;

//#ifdef SUPPORT_BRVLAN
    /*start vlan bridge, s60000658, 20060627*/
    BcmDb_unregVlanPort(ifName);
    /*end vlan bridge, s60000658, 20060627*/
//#endif

    // Search if the bridge is in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (bridgeList[i].useFlag == PMAP_CFG_ENTRY_INUSE) {
            char *prefix   = NULL;
            char *suffix   = NULL;
            char *grpList   = NULL;
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            if (BcmDb_checkIfcString(bridgeList[i].grpedIntf, ifName, &prefix)) {
                grpType = 0;
            } else if (BcmDb_checkIfcString(bridgeList[i].dynIntf, ifName, &prefix)) {
                   grpType = 1;
            } else {
               continue;
            }
            char grpedIntf[PORT_MAP_INTF_LIST_SIZE];
            // We found the interface in the list. Delet this from the
            // bridge.
            sprintf(cmd, "brctl delif %s %s ", bridgeList[i].ifName, ifName);
            bcmSystemNoHang(cmd);
            // If the bridge MAC address for br0 changes, with the deletion of
            // an interface, send an arp to its associated devices to propagate
            // this change.
            if (strcmp(bridgeList[i].ifName, IFC_BRIDGE_NAME) == 0) {
                sprintf(cmd, "sendarp -s %s -d %s", IFC_BRIDGE_NAME, IFC_BRIDGE_NAME);
                bcmSystemNoHang(cmd);
            }
            memset(grpedIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
            if (grpType) {
                grpList = bridgeList[i].dynIntf;
            } else {
                grpList = bridgeList[i].grpedIntf;
            }
            if (prefix == grpList) {
                if ((suffix = strstr(prefix, "|")) != NULL) {
                    // Skip the | and copy everything till the end.
                    strcpy (grpedIntf, (suffix + 1));
                } else {
                    // This is the only interface this bridge has, so set the bridge
                    // for deletion only if the statically configured interface list
                    // is empty.
                    if ((strlen(bridgeList[i].grpedIntf) == 0) &&
                        (strlen(bridgeList[i].dynIntf) == 0)) {
                       bridgeDelete = 1;
                    }
                }
            } else {
                // Copy the prefix part of the string to the temp.
                if (prefix != NULL) {
                    memcpy(grpedIntf, grpList, ((prefix - 1) - grpList));
                    if ((suffix = strstr(prefix, "|")) != NULL) {
                        // Append everything till the end including the | delimiter.
                        strcat(grpedIntf, suffix);
                    }
                } else {
                    memcpy(grpedIntf, grpList, strlen(grpList));
                }
            }
            // If the interface list is empty, then remove the bridge.
            if (bridgeDelete && bridgeList[i].useFlag == PMAP_CFG_ENTRY_INUSE) {
                if (strcmp(bridgeList[i].groupName, "Default") != 0) {
                    sprintf(cmd, "ifconfig %s down 2>/dev/null", bridgeList[i].ifName);
                    bcmSystem(cmd);
                    sprintf(cmd, "brctl delbr %s 2>/dev/null", bridgeList[i].ifName);
                    bcmSystem(cmd);
                    memset(bridgeList[i].groupName, '\0', IFC_TINY_LEN);
                    bridgeList[i].useFlag = PMAP_CFG_ENTRY_NOTINUSE;
                 }
                 bridgeDelete = 0;
                 memset(bridgeList[i].grpedIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
             } else {
                 // Clear the original list and recopy it from the temp.
                 if (grpType == 0) {
                     memset(bridgeList[i].grpedIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
                     strncpy(bridgeList[i].grpedIntf, grpedIntf, PORT_MAP_INTF_LIST_SIZE);
                 } else {
                     memset(bridgeList[i].dynIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
                     strncpy(bridgeList[i].dynIntf, grpedIntf, PORT_MAP_INTF_LIST_SIZE);
                }
            }
            return;
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_delBridge
// Description  : delete bridge interface configuration information
// Returns      : none.
//**************************************************************************
void BcmDb_delBridge(const char *bridgeName) {
    int i = 0;
    char cmd[IFC_LARGE_LEN];

    if (bridgeName == NULL)
        return;

    if (strcmp(bridgeName, "Default") == 0) {
        memset(bridgeList[i].grpedIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
        return;
    }

    // Search if the bridge is in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, bridgeName) == 0) {
            sprintf(cmd, "ifconfig %s down 2>/dev/null", bridgeList[i].ifName);
            bcmSystem(cmd);
            // Delete the bridge from the configuration
            sprintf(cmd, "brctl delbr %s 2>/dev/null", bridgeList[i].ifName);
            bcmSystem(cmd);
            // Clear the user friendly name
            memset(bridgeList[i].groupName, '\0', IFC_TINY_LEN);
            bridgeList[i].useFlag = PMAP_CFG_ENTRY_NOTINUSE;
            // Clear the original list and recopy it from the temp.
            memset(bridgeList[i].grpedIntf, '\0', PORT_MAP_INTF_LIST_SIZE);
            return;
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_getBridgeInfoByIndex
// Description  : retrieve bridge interface configuration information
// Returns      : none.
//**************************************************************************
char *BcmDb_getBridgeInfoByIndex(int index, char *groupName, char *grpedIntf) {

    if (index > PORT_MAP_MAX_ENTRY || groupName == NULL || grpedIntf == NULL) {
        return NULL;
    }

    // Search if the bridge is in the list.
    // Locate the entry in the list of bridges.
    if (bridgeList[index].useFlag == PMAP_CFG_ENTRY_INUSE) {
        strcpy(groupName, bridgeList[index].groupName);
        strcpy(grpedIntf, bridgeList[index].grpedIntf);
        return grpedIntf;
    } else {
        return NULL;
    }
}
 /*start of 增加 wl 接口设备信息 by s53329 at  20081104*/
void BcmDb_findGrByIfc(char * ifc, char *grp)
{
    PORT_MAP_STATUS sts = PORT_MAP_STS_OK;
    void *node = NULL;
    char groupName[IFC_TINY_LEN];
    char grpedIntf[IFC_LARGE_LEN];
    if(NULL == ifc || NULL == grp)
    {
        return ;
    }
    node = BcmPMcm_getPMapCfg(NULL, groupName, grpedIntf);
    while(node != NULL) {
        if(strstr(grpedIntf, ifc))
        {
            strcpy(grp, groupName);
            return;
        }
        node = BcmPMcm_getPMapCfg(node, groupName, grpedIntf);
    }
    return ;

}
/*end of 增加 wl 接口设备信息 by s53329 at  20081104*/
//**************************************************************************
// Function Name: BcmDb_applyBridgeCfg
// Description  : apply bridge interface configuration information
// Returns      : none.
//**************************************************************************
int BcmDb_applyBridgeCfg() {
    PORT_MAP_STATUS sts = PORT_MAP_STS_OK;
    int i = 0;
    void *node = NULL;
    char groupName[IFC_TINY_LEN];
    char grpedIntf[IFC_LARGE_LEN];
    FILE *fp = fopen(DHCPD_VENDORID_CONF_FILE, "r");
    char vendorId[DHCP_VENDOR_ID_LEN+2];
    int index = 0;

    memset(groupName, '\0', IFC_TINY_LEN);
    memset(grpedIntf, 0, IFC_LARGE_LEN);
    // Erase the existing configuration.
    node = BcmPMcm_getPMapCfg(NULL, groupName, grpedIntf);
    while(node != NULL) {
        sts = BcmPMcm_removePMapCfg(groupName);
        if (sts != PORT_MAP_STS_OK) {
            return (int)sts;
        }
        memset(groupName, '\0', IFC_TINY_LEN);
        node = BcmPMcm_getPMapCfg(node, groupName, grpedIntf);
    }

    // If there is an existing file delete that and overwite the new
    // configuration.
    if (fp != NULL) {
        fclose(fp);
        unlink(DHCPD_VENDORID_CONF_FILE);
    }

    // Open the file for writing
    if ((fp = fopen(DHCPD_VENDORID_CONF_FILE, "w")) == NULL) {
        printf("BcmDb_applyBridgeCfg: Error opening %s\n", DHCPD_VENDORID_CONF_FILE);
    }

    // Write the rest of the updated configuration.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Add all entries with useFlag set to INUSE
        if (bridgeList[i].useFlag == PMAP_CFG_ENTRY_INUSE) {
            sts = BcmPMcm_addPMapCfg(bridgeList[i].groupName,
                                     bridgeList[i].grpedIntf,
                                     bridgeList[i].vid);
            if (sts != PORT_MAP_STS_OK) {
                if (fp) {
                   fclose(fp);
                }
                return (int)sts;
            }

            // Start writing one record per line.
            for (index = 0; index < DHCP_MAX_VENDOR_IDS; index++) {
                if (strlen(bridgeList[i].vid[index].vendorid) > 0) {
                    if (fp) {
                        memset(vendorId, 0, DHCP_VENDOR_ID_LEN+2);
                        snprintf(vendorId, DHCP_VENDOR_ID_LEN, "%s",
                                 bridgeList[i].vid[index].vendorid);
                        fputs(vendorId, fp);
                        fflush(fp);
                    }
                }
            }
        }
    }
    fclose(fp);
    return (int)sts;
}

//**************************************************************************
// Function Name: BcmDb_setBridgeIfcCfgSts
// Description  : set bridge interface configuration status
// Returns      : none.
//**************************************************************************
int BcmDb_setPMapIfcCfgSts(char *ifName, int status) {
    int sts = PORT_MAP_STS_OK;
    PSI_HANDLE pmapPsi = BcmPsi_appOpen(PMAP_APPID);

    if (ifName == NULL || status < 0) {
        sts = PMAP_IFC_STS_ERROR;
        return sts;
    }

    if (status == 0) {
        bridgeIfcCfgSts.status = PMAP_IFC_STS_DISABLE;
        strcpy(bridgeIfcCfgSts.ifName, ifName);
    } else {
        bridgeIfcCfgSts.status = PMAP_IFC_STS_ENABLE;
        strcpy(bridgeIfcCfgSts.ifName, ifName);
    }
    // Store the Interface configuration
    BcmPsi_objStore(pmapPsi, PMCM_PMAP_IFC_STS_ID, (PSI_VALUE)&bridgeIfcCfgSts,
                    sizeof(PORT_MAP_IFC_STATUS_INFO));
    BcmPsi_flush();
    return sts;
}

//**************************************************************************
// Function Name: BcmDb_getBridgeIfcCfgSts
// Description  : set bridge interface configuration status
// Returns      : none.
//**************************************************************************
int BcmDb_getPMapIfcCfgSts(char *ifName, int status) {
    int sts = PORT_MAP_STS_OK;

    if (ifName == NULL) {
        sts = PMAP_IFC_STS_ERROR;
        return sts;
    }

    status = bridgeIfcCfgSts.status = PMAP_IFC_STS_DISABLE;
    strcpy(ifName, bridgeIfcCfgSts.ifName);
    return sts;
}


//**************************************************************************
// Function Name: BcmDb_getPMapIfcCfgInfo
// Description  : retrieve Port Mapping configuration
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPMapIfcCfgInfo(PPORT_MAP_IFC_STATUS_INFO pPMapIfcCfgInfo) {
    int ret = DB_GET_OK;
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE pMapPsi = BcmPsi_appOpen(PMAP_APPID);
    
    memset(pPMapIfcCfgInfo, 0, sizeof(PORT_MAP_IFC_STATUS_INFO));
   
    status = BcmPsi_objRetrieve(pMapPsi, PMCM_PMAP_IFC_STS_ID, pPMapIfcCfgInfo,
        sizeof(PORT_MAP_IFC_STATUS_INFO), &retLen);

    if ( status != PSI_STS_OK ) {
        pPMapIfcCfgInfo->status = atoi(BcmDb_getDefaultValue("pmapIfcCfgStatus"));
        strncpy(pPMapIfcCfgInfo->ifName, BcmDb_getDefaultValue("pmapIfcCfgName"), IFC_TINY_LEN);
        ret = DB_GET_NOT_FOUND;
    }
    return ret;   
}

//**************************************************************************
// Function Name: BcmDb_setPMapIfcCfgInfo
// Description  : store Port Mapping interface configuration
// Returns      : none.
//**************************************************************************
void BcmDb_setPMapIfcCfgInfo(PPORT_MAP_IFC_STATUS_INFO pPMapIfcCfgInfo) {
    PSI_HANDLE pMapPsi = BcmPsi_appOpen(PMAP_APPID);

    BcmPsi_objStore(pMapPsi, PMCM_PMAP_IFC_STS_ID, pPMapIfcCfgInfo,
                    sizeof(PORT_MAP_IFC_STATUS_INFO));
}

//**************************************************************************
// Function Name: BcmDb_removePMapIfcCfgInfo
// Description  : remove Port Mapping interface configuration
// Returns      : none.
//**************************************************************************
void BcmDb_removePMapIfcCfgInfo(void) {
    PSI_HANDLE pMapPsi = BcmPsi_appOpen(PMAP_APPID);

    BcmPsi_objRemove(pMapPsi, PMCM_PMAP_IFC_STS_ID);
}

int BcmDb_qrySwitch(char *ifName) {
   FILE *errFs = NULL;
   char cmd[IFC_LARGE_LEN];
   char str[IFC_LARGE_LEN];
   int  numIfc = 0;

   sprintf(cmd, "ethctl %s vport query 2>/var/vcfgerr\n", ifName);
   bcmSystem(cmd);
   // Check the status of the previous command
   errFs = fopen("/var/vcfgerr", "r");
   if (errFs != NULL ) {
      fgets(str, IFC_LARGE_LEN, errFs);
      numIfc = atoi(str);
      fclose(errFs);
      bcmSystem("rm /var/vcfgerr");
   }
   return numIfc;
}

int
BcmDb_getSwitchIfcInfo(char *ifName)
{
   char name[SYS_CMD_LEN];
   char info[IFC_TINY_LEN];
   int index = 0;
   int  numIfc = 0;

   for ( index = 0; index < PORT_MAP_MAX_ENTRY; index++ ) {
      char *pPfx = NULL;
      char *pSfx = NULL;
      if (bridgeList[index].useFlag != PMAP_CFG_ENTRY_INUSE ) {
         index++;
         continue;
      }
      strncpy(name, bridgeList[index].grpedIntf, SYS_CMD_LEN);
      pPfx = name;
      while (pPfx <= (name + strlen(name))) {
         if ((pSfx = strstr(pPfx, "|")) != NULL) {
            memset(info, '\0', IFC_TINY_LEN);
            memcpy(info, pPfx, (pSfx - pPfx));
            if ( info[0] != '\0' ) {
               // Query the number of ports on this interface
               numIfc = BcmDb_qrySwitch(info);
               if (numIfc > 1) {
                  strncpy(ifName, info, IFC_TINY_LEN);
                  return numIfc;
               }
            }
            pPfx = pSfx + 1;
         } else {
            memset(info, '\0', IFC_TINY_LEN);
            strcpy(info, pPfx);
            if ( info[0] != '\0' ) {
               // Query the number of ports on this interface
               numIfc = BcmDb_qrySwitch(info);
               if (numIfc > 1) {
                  strncpy(ifName, info, IFC_TINY_LEN);
                  return numIfc;
               }
            }
         }
      }
   }
   return numIfc;
}

//**************************************************************************
// Function Name: BcmDb_getDynIntf
// Description  : retrieves the dynamically created interface list for a
//                specific group.
// Returns      : none.
//**************************************************************************
void BcmDb_getDynIntf(char *groupName, char *dynIntf, int len) {
    int i = 0;

    if (groupName == NULL || dynIntf == NULL)
        return;

    // Search if the bridge is in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, groupName) == 0) {
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            strncpy(dynIntf, bridgeList[i].dynIntf, len);
            return;
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_addBridgeDynIf
// Description  : Dynamically add an interface to this bridge. The
//                dynamically added interfaces will not be saved to the
//                PSI configuration file.
// Returns      : none.
//**************************************************************************
void BcmDb_addBridgeDynIf(char *groupName, char *ifName) {
    int i = 0;

    if (groupName == NULL || ifName == NULL)
        return;

    // First we search if the bridge is already in the list.
    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        // Locate the bridge in the list of bridges.
        if (strcmp(bridgeList[i].groupName, groupName) == 0) {
            // Locate the interface name in the list of interfaces under this
            // bridge. If it is not there then add it to the end.
            if (!BcmDb_checkIfcString(bridgeList[i].dynIntf, ifName, NULL)) {
                // Make sure we have enough space to fit in the new interface
                // and "|" token termination string.
                if (*bridgeList[i].dynIntf == '\0') {
                    strcat(bridgeList[i].dynIntf, ifName);
                    return;
                } else if ((strlen(bridgeList[i].dynIntf) +
                    strlen(ifName) + 1) < PORT_MAP_INTF_LIST_SIZE) {
                    strcat(bridgeList[i].dynIntf, "|");
                    strcat(bridgeList[i].dynIntf, ifName);
                    return;
                }
                else {
                    // We do not have enough space to squeeze this interface
                    // into the list so just return;
                    //printf ("BcmDb_addBridgeIf: Not enough space\n");
                    return;
                }
            } else {
               //printf ("BcmDb_addBridgeIf: Already present\n");
               break;
            }
        }
    }
}


#endif

//**************************************************************************
// Function Name: BcmDb_removeVcc
// Description  : remove a vcc given its port, vpi, vci
// Returns      : none.
//**************************************************************************
void BcmDb_removeVcc(int port, int vpi, int vci) 
{
    ATM_VCC_ADDR addr;

    addr.ulInterfaceId = port;
    addr.usVpi = vpi;
    addr.usVci = vci;
    AtmVcc_removeByAddress(&addr);
}

int BcmDb_addVcc(IFC_ATM_VCC_INFO *pNewVccInfo) 
{
    int ret = -1;

    if ( BcmDb_checkPvc() != DB_WAN_ADD_OK )
        return ret;
  
    if ( AtmVcc_setByObject(pNewVccInfo) != 0 )
        ret = 0;
    
    return ret;
}

int BcmDb_configWanMode(PWAN_CON_ID pWanId, int type, int value) 
{
  WAN_CON_INFO wanInfo;
  WAN_IP_INFO ipInfo;

  if (type == WAN_DHCPC_FIELD) {
    if (BcmDb_getIpSrvInfo(pWanId, &ipInfo) == DB_WAN_GET_OK) {
      ipInfo.enblDhcpClnt = value;
      BcmDb_setIpSrvInfo(pWanId, &ipInfo);
    }
    else
      return -1;
  }
  else {
    if (BcmDb_getWanInfo(pWanId,&wanInfo) != DB_WAN_GET_OK)
      return -1;

    switch (type)
      {
      case WAN_PROTOCOL_FIELD:
        wanInfo.protocol = value;
        break;
      case WAN_ADMIN_FIELD:
        wanInfo.flag.service = value;
        break;
      case WAN_BRIDGE_FIELD:
        break;
      case WAN_MAC_LEARNING_FIELD:
        break;
/*start of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
#if 0
      case WAN_IGMP_FIELD:
        wanInfo.flag.igmp = value;
        break;
 #endif
 /*end of删除brcom igmp proxy 使能功能 by l129990，2008，9，28*/
      case WAN_NAT_FIELD:
        wanInfo.flag.nat = value;
        break;
      } /* switch */
    if (type != WAN_DHCPC_FIELD)
      BcmDb_setWanInfo(pWanId,&wanInfo);
  } 
  return 0;
}

/* Utility function to convert wan interface to VPI/VCI */
void BcmDb_wanIfToPvc(char *wanIf, char *pvc)
{
   char data[IFC_TINY_LEN];
   WAN_CON_ID wanId;
   WAN_CON_INFO wanInfo;

   wanId.vpi = wanId.vci = wanId.conId = 0;
   
   while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
   	  if (wanInfo.protocol == PROTO_PPPOU)  //for hspapppou
      {
        continue;
      }	  
      BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, data);
      if ( strcmp(data, wanIf) == 0 ) {
         sprintf(pvc, "%d/%d", wanId.vpi, wanId.vci);
            return;
      }
   }
}

#if SUPPORT_PORT_MAP
//**************************************************************************
// Function Name: BcmDb_getDhcpVendorGroupName
// Description  : Return the group name for this vendor ID.
// Returns      : none.
//**************************************************************************
void BcmDb_getDhcpVendorGroupName(char *vendorid, char *groupName) {
    int index = 0;
    int i     = 0;

    if (vendorid == NULL || groupName == NULL) {
        printf("Error: Illegal vendorid or group name\n");
        return;
    }

    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
      if (bridgeList[i].useFlag != PMAP_CFG_ENTRY_INUSE ) {
         continue;
      }
        for (index = 0; index < DHCP_MAX_VENDOR_IDS; index++) {
            if (strlen(bridgeList[i].vid[index].vendorid) != 0) {
                if (strncmp(bridgeList[i].vid[index].vendorid,
                            vendorid,
                            strlen(bridgeList[i].vid[index].vendorid)) == 0) {
                    strcpy(groupName, bridgeList[i].groupName);
                    return;
                }
            }
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_getDHCPVendorId
// Description  : retrieve DHCP Vendor ID
// Returns      : DHCP vendor ID for a given index.
//**************************************************************************
void BcmDb_getDHCPVendorId(int index, char *vendorid, char *grpName) {
    int i = 0;

    if (index > DHCP_MAX_VENDOR_IDS || vendorid == NULL || grpName == NULL) {
        strcpy(vendorid, "");
        return;
    }

    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        if (bridgeList[i].useFlag == PMAP_CFG_ENTRY_INUSE) {
            if (strcmp(bridgeList[i].groupName, grpName) == 0) {
                strcpy(vendorid, bridgeList[i].vid[index].vendorid);
                if (vendorid[strlen(vendorid)+1] == '\n') {
                   vendorid[strlen(vendorid)+1] = '\0';
                }
            }
        }
    }
}

//**************************************************************************
// Function Name: BcmDb_setDHCPVendorId
// Description  : Set DHCP Vendor ID
// Returns      : none.
//**************************************************************************
void BcmDb_setDHCPVendorId(int index, char *vendorid, char *grpName) {
    int i = 0;

    if (vendorid == NULL || grpName == NULL) {
        return;
    }
    //printf("BcmDb_setDHCPVendorId: Entering index %d vendorid %p grpName %p\n",
    //        index, vendorid, grpName);
    if (index > DHCP_MAX_VENDOR_IDS || vendorid == NULL || grpName == NULL) {
        printf("BcmDb_setDHCPVendorId: Invalid entry index %d vendorid %p grpName %p\n",
                index, vendorid, grpName);
        return;
    }

    for (i = 0; i < PORT_MAP_MAX_ENTRY; i++) {
        if (bridgeList[i].useFlag == PMAP_CFG_ENTRY_INUSE) {
            if (strcmp(bridgeList[i].groupName, grpName) == 0) {
                //printf("BcmDb_setDHCPVendorId: pointer value vendorid %p str %s grpName %s len %d\n",
                //          vendorid, vendorid, grpName, strlen(vendorid));
                //if (strlen(vendorid) != 0) {
                    //printf("BcmDb_setDHCPVendorId: Adding vendor id %s groupName %s\n",
                    //          vendorid, grpName);
                    strcpy(bridgeList[i].vid[index].vendorid, vendorid);
                    //printf("BcmDb_setDHCPVendorId: Done Adding vendor id %s groupName %s\n",
                    //       bridgeList[i].vid[index].vendorid, bridgeList[i].groupName);
                //}
            }
        }
    }
}
//**************************************************************************
// Function Name: BcmDb_checkIfInBridge
// Description  : 判断接口之前是否已存在某个桥接口下
// Returns      : 0: not exist;  1: exist
// Author       : yuyouqing42304
//**************************************************************************
int BcmDb_checkIfInBridge(char *ifName, char *bridgeName)
{    
    int i = 0;

    if ((NULL == ifName) || (NULL == bridgeName))
    {
        return 0;
    }

    
    for(i = 0; i < PORT_MAP_MAX_ENTRY; i++)
    {        
        if (BcmDb_checkIfcString(bridgeList[i].grpedIntf, ifName, NULL))
        {
            strcpy(bridgeName, bridgeList[i].groupName);
            return 1;
        }
    }
    return 0;
}
#endif // SUPPORT_PORT_MAP

#ifdef PORT_MIRRORING
//**************************************************************************
// Function Name: BcmDb_getPortMirrorStatus
// Description  : retrieve global PortMirror Cfg Status
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPortMirrorStatus (DbgPortMirrorStatus* pm) {
    return BcmDb_getPortMirrorInfo (DCM_PORT_MIRROR_STATUS_ID, 
                                    (PSI_VALUE) pm, sizeof(DbgPortMirrorStatus));
}

//**************************************************************************
// Function Name: BcmDb_setPortMirrorStatus
// Description  : store global PortMirror Cfg Status.
// Returns      : none.
//**************************************************************************
void BcmDb_setPortMirrorStatus(DbgPortMirrorStatus* pm) {
    BcmDb_setPortMirrorInfo (DCM_PORT_MIRROR_STATUS_ID, (PSI_VALUE) pm, sizeof(DbgPortMirrorStatus));
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfg
// Description  : retrieve Port Mirror Cfg
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPortMirrorCfg (PortMirrorCfg* tbl, int size) {
    return BcmDb_getPortMirrorInfo (DCM_PORT_MIRROR_CFG_TBL_ID, (PSI_VALUE)tbl,
                                    sizeof(PortMirrorCfg) * size);
}

//**************************************************************************
// Function Name: BcmDb_setPortMirrorCfg
// Description  : store Port Mirror Cfg.
// Returns      : none.
//**************************************************************************
void BcmDb_setPortMirrorCfg(PortMirrorCfg* tbl, int size) {
    BcmDb_setPortMirrorInfo (DCM_PORT_MIRROR_CFG_TBL_ID, (PSI_VALUE)tbl,
                               sizeof(PortMirrorCfg) * size);
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfgRecNo
// Description  : convert Port Mirror Cfg RecNo from text to integer.
// Returns      : Port Mirror RecNo.
//**************************************************************************
int BcmDb_getPortMirrorCfgRecNo(char *info) {
    int i = 0;    
    const char* opers[] = { "1", "2", NULL };
    
    for ( i = 0; opers[i] != NULL; i++ ) {
        if ( strcasecmp(opers[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorMonitorPort
// Description  : convert Port Mirror MonitorPort from text to integer.
// Returns      : Port Mirror Monitor Port.
//**************************************************************************
int BcmDb_getPortMirrorCfgMonitorPort(char *info) {
    int i = 0;    
    const char* opers[] = { "WAN0", NULL };
    
    for ( i = 0; opers[i] != NULL; i++ ) {
        if ( strcasecmp(opers[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfgDir
// Description  : convert Port Mirror Direction from text to integer.
// Returns      : Port Mirror Direction.
//**************************************************************************
int BcmDb_getPortMirrorCfgDir(char *info) {
    int i = 0;    
    const char* opers[] = { "IN", "OUT", NULL };
    
    for ( i = 0; opers[i] != NULL; i++ ) {
        if ( strcasecmp(opers[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfgFlowType
// Description  : convert Port Mirror Cfg Flow type from text to integer
// Returns      : Port Mirror flow type.
//**************************************************************************
int BcmDb_getPortMirrorCfgFlowType (char *info) {
    int i = 0;
    const char* versions[] = {"802.3", NULL };
    
    for ( i = 0; versions[i] != NULL; i++ ) {
        if ( strcasecmp(versions[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfgMirrorPort
// Description  : convert Port Mirror Cfg Mirror Port from text to integer
// Returns      : Port Mirror Mirror Port 
//**************************************************************************
int BcmDb_getPortMirrorCfgMirrorPort (char *info) {
    int i = 0;
    const char* versions[] = {"eth0", "eth1", "usb", NULL} ;

    for ( i = 0; versions[i] != NULL; i++ ) {
        if ( strcasecmp(versions[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorCfgStatus
// Description  : convert Port Mirror Cfg Status from text to integer
// Returns      : Port Mirror Status 
//**************************************************************************
int BcmDb_getPortMirrorCfgStatus (char *info) {
    int i = 0;
    const char* versions[] = {"disabled", "enabled", NULL} ;

    for ( i = 0; versions[i] != NULL; i++ ) {
        if ( strcasecmp(versions[i], info) == 0 )
            break;
    }
    
    return i+1;
}

//**************************************************************************
// Function Name: BcmDb_getPortMirrorInfo.
// Description  : retrieve Port Mirror table information.
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPortMirrorInfo (int id, PSI_VALUE info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE PmPsi = BcmPsi_appOpen(ENGDBG_APPID);
    
    status = BcmPsi_objRetrieve(PmPsi, id, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setPortMirrorInfo
// Description  : store Port Mirror Information
// Returns      : none.
//**************************************************************************
void BcmDb_setPortMirrorInfo (int id, PSI_VALUE info, int size) {
    PSI_HANDLE PmPsi = BcmPsi_appOpen (ENGDBG_APPID);
    
    BcmPsi_objStore(PmPsi, id, info, size);
}

#endif /* PORT_MIRRORING */
/*start of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/
void BcmDb_GetBrInfoByWanId(char ** ppBrName,UINT16 vpi,UINT16 vci,UINT16 conId)
{
    char wanIf[16];
    char * pBrName = * ppBrName;
    UINT16 retLen  = 0, pmapCfgNum = 0;
    PPORT_MAP_CFG_ENTRY pmapCfgs = NULL;
    PSI_HANDLE    pmapPsi;

    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月19日 */
    int ret = DB_WAN_GET_OK;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;

    wanId.conId = conId;
    wanId.vci = vci;
    wanId.vpi = vpi;
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月19日 */
    
    wanIf[0] = '\0';
    pBrName[0] = '\0';

    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月19日 */
    ret = BcmDb_getWanInfo(&wanId, &wanInfo); //DB_WAN_GET_OK=1;DB_WAN_GET_NOT_FOUND=2
//    printf("BcmDb_GetBrInfoByWanId BcmDb_getWanInfo:%d\r\n", ret);

    #ifdef SUPPORT_VDSL
    if ((g_trafficmode == TM_PTM_VLAN) || (g_trafficmode == TM_PTM_PRTCL) || (g_trafficmode == TM_PTM_BR)) //glbWebVar.TrafficMode
    {
          if( wanInfo.vlanId >= 0) //PTM VLAN
          {
              sprintf(wanIf, "nas_%d_%d_%d", vpi, vci, wanInfo.vlanId);
          }
          else //PTM PROTOCOL
          {
              sprintf(wanIf, "nas_%d_%d_%d", vpi, vci, wanInfo.protocol);
          }
    }
    else
    {
         sprintf(wanIf, "nas_%d_%d", vpi, vci);
    }
    #else
    sprintf(wanIf, "nas_%d_%d", vpi, vci);
    #endif
    //sprintf(wanIf, "nas_%d_%d", vpi, vci);
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月19日 */
    
    pmapPsi = BcmPsi_appOpen(PMAP_APPID);
    PSI_STATUS status = BcmPsi_objRetrieve(pmapPsi, PMCM_PMAP_NUM_ID,
                               &pmapCfgNum, sizeof(pmapCfgNum), &retLen);
    if ( status == PSI_STS_OK && pmapCfgNum > 0 ) {
       pmapCfgs = new PORT_MAP_CFG_ENTRY[pmapCfgNum];
       status = BcmPsi_objRetrieve(pmapPsi, PMCM_PMAP_CFG_TBL_ID,
                                  pmapCfgs,
                                  sizeof(PORT_MAP_CFG_ENTRY) * pmapCfgNum,
                                  &retLen);
       /*找到nas接口所在的桥返回*/
       if ( status == PSI_STS_OK ) {
          for ( int i = 0; i < pmapCfgNum; i++ ) {
             if (strstr(pmapCfgs[i].grpedIntf, wanIf) != NULL) {
                 if(strcmp(pmapCfgs[i].groupName, "Default") == 0)
                         strcpy(pBrName, "br0:1");
                 else if(strcmp(pmapCfgs[i].groupName, "default") == 0)
                         strcpy(pBrName, "br0:1");
                 else
                         strcpy(pBrName, pmapCfgs[i].groupName);
                 break;
             }
          }
       }

       delete[] pmapCfgs;
       pmapCfgs = NULL;
    }
}
/***************************************************************************
// Function Name: BcmDb_isWanDhcpclientExisted.
// Description  : check any existed DHCP CLIENT  that has the given WAN ID,
//                .
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmDb_isAnyBridgeDhcpcEnable(PWAN_CON_ID  pWanId) 
{
    int existed = FALSE;
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;
    WAN_BRDHCPC_INFO brInfo;
   
    wanId.vpi = 0;
    wanId.vci = 0;
    wanId.conId = 0;
    memset(&brInfo, 0, sizeof (WAN_BRDHCPC_INFO));
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
    {
        if ((pWanId->vpi == wanId.vpi) && (pWanId->vci == wanId.vci) && (pWanId->conId == wanId.conId) )
        {
            continue;
        }
        if ( wanInfo.protocol == PROTO_BRIDGE) 
        {
           if ( BcmDb_getBrDhcpcInfo(&wanId, &brInfo)== DB_WAN_GET_OK)
            {
                if (brInfo.enableDhcpc == TRUE)
                {
                    existed = TRUE;
                    break;
                 }
            }
        }
    }
    return existed;
}

/*end of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/

//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
VLAN_8021Q_PORT_ENTRY* BcmDb_getVlanPort(char *portName)
{
    int i;
    for(i = 0; i < VLAN_8021Q_PORT_MAX && portName; i++)
    {
        if(strcmp(portName, g_vlPortList[i].name) == 0
            && VLAN_ENTRY_INUSE == g_vlPortList[i].useFlag)
            return &g_vlPortList[i];
    }
    return NULL;
}
VLAN_8021Q_PORT_ENTRY* BcmDb_getVlanPortHead(void)
{
    return &g_vlPortList[0];
}
VLAN_8021Q_PORT_ENTRY* BcmDb_regVlanPort(char *portName) 
{
    int i = 0;    
    VLAN_8021Q_PORT_ENTRY* pport = NULL;
    
    pport = BcmDb_getVlanPort(portName);

    if(pport)
        return pport;

    for(i = 0; i < VLAN_8021Q_PORT_MAX && portName; i++)
    {   
        if(VLAN_ENTRY_NOTINUSE == g_vlPortList[i].useFlag)
        {
            g_vlPortList[i].useFlag = VLAN_ENTRY_INUSE;
            if(strlen(portName) < IFC_TINY_LEN)
            {
                strcpy(g_vlPortList[i].name, portName);
                return &g_vlPortList[i];
            }
            else
                return NULL;
        }
    }

    return NULL;    
}

void BcmDb_unregVlanPort(char *portName)
{
    VLAN_8021Q_PORT_ENTRY* pport = NULL;
    
    pport = BcmDb_getVlanPort(portName);

    if(pport)
        pport->useFlag = VLAN_ENTRY_NOTINUSE;
}

VLAN_8021Q_ITEM_ENTRY* BcmDb_getVlanbyVID(unsigned short vid)
{
    int i = 0;

    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(vid == g_vlanList[i].vlanid
            && VLAN_ENTRY_INUSE == g_vlanList[i].useFlag)
            return &g_vlanList[i];
    }
    return NULL;
    
}
void BcmDb_removeVlanbyVID(unsigned short vid)
{
    int i = 0;

    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(vid == g_vlanList[i].vlanid)            
        {
            g_vlanList[i].useFlag = VLAN_ENTRY_NOTINUSE;
            return;
        }
    }
}
VLAN_8021Q_ITEM_ENTRY* BcmDb_getVlanbyNo(int vlanNo)
{
    int i = 0;

    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(vlanNo == g_vlanList[i].no
            && VLAN_ENTRY_INUSE == g_vlanList[i].useFlag)
            return &g_vlanList[i];
    }
    return NULL;
}
void BcmDb_removeVlanbyNo(int vlanNo)
{
    int i = 0;

    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(vlanNo == g_vlanList[i].no)            
        {
            g_vlanList[i].useFlag = VLAN_ENTRY_NOTINUSE;
            return;
        }
    }
}
VLAN_8021Q_ITEM_ENTRY* BcmDb_setVlanbyVID(VLAN_8021Q_ITEM_ENTRY* pvlan)
{
    VLAN_8021Q_ITEM_ENTRY* p = NULL;

    if(!pvlan)
        return NULL;
    
    p = BcmDb_getVlanbyVID(pvlan->vlanid);

    if(!p)
        return NULL;

    memcpy(p, pvlan, sizeof(VLAN_8021Q_ITEM_ENTRY));

    return p;
}
VLAN_8021Q_ITEM_ENTRY* BcmDb_setVlanbyNo(VLAN_8021Q_ITEM_ENTRY* pvlan)
{
    VLAN_8021Q_ITEM_ENTRY* p = NULL;

    if(!pvlan)
        return NULL;
    
    p = BcmDb_getVlanbyVID(pvlan->no);

    if(!p)
        return NULL;

    memcpy(p, pvlan, sizeof(VLAN_8021Q_ITEM_ENTRY));

    return p;
}

VLAN_8021Q_ITEM_ENTRY* BcmDb_addVlan(VLAN_8021Q_ITEM_ENTRY* pvlan)
{
    int i;

    if(!pvlan)
        return NULL;
    
    /*start A36D02403 vlan No. 可以重复添加, s60000658, 2006/08/02*/
    if((pvlan->no >= 0) && (pvlan->no < VLAN_8021Q_NUM_MAX))
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlanList[pvlan->no].useFlag)
        {
            memcpy(&g_vlanList[pvlan->no], pvlan, sizeof(VLAN_8021Q_ITEM_ENTRY));
            g_vlanList[pvlan->no].useFlag = VLAN_ENTRY_INUSE;
            return &g_vlanList[pvlan->no];
        }
        else
            return NULL;
    }
    else   
    /*end A36D02403 vlan No. 可以重复添加, s60000658, 2006/08/02*/
    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlanList[i].useFlag)
        {
            memcpy(&g_vlanList[i], pvlan, sizeof(VLAN_8021Q_ITEM_ENTRY));
            g_vlanList[i].useFlag = VLAN_ENTRY_INUSE;
            return &g_vlanList[i];
        }        
    }
    
    return NULL;
}
int BcmDb_addPorttoVlan(unsigned short vid, char* portName)
{
    VLAN_8021Q_ITEM_ENTRY* pvlan = NULL;

    pvlan = BcmDb_getVlanbyVID(vid);

    if(!pvlan || !portName)
        return 0;

    if(strlen(portName)+strlen(pvlan->ifList) > VLAN_INTF_LIST_SIZE - 2)
        return 0;
    
    if(strstr(pvlan->ifList, portName))
        return 0;
    
    if(pvlan->ifList[0] = '\0')
        strcat(pvlan->ifList, portName);
    else
    {
        strcat(pvlan->ifList, ",");
        strcat(pvlan->ifList, portName);
    }

    return 1;

}
int BcmDb_removePortFromVlan(unsigned short vid, char* portName)
{
    VLAN_8021Q_ITEM_ENTRY* pvlan = NULL;
    char *p = NULL;
    char *sep = NULL;
    
    pvlan = BcmDb_getVlanbyVID(vid);

    if(!pvlan || !portName)
        return 0;

    p = strstr(pvlan->ifList, portName);

    if(!p)
        return 0;
    
    sep = strchr(p, ','); 

    if(sep)
        strcpy(p, sep+1);
    else 
        p[0] = 0;

    return 1;    
}

VLAN_8021Q_CFG_ENTRY* BcmDb_getVlanCfg(void)
{
    return &g_vlanCfg;
}

int BcmDb_applyVLPortAttr(void)
{
    PSI_HANDLE vlanPsi = NULL;
    char *pports = NULL;
    char *tmp    = NULL;
    int nPorts   = 0;
    int i;

    /*remove all at first*/
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    (void)BcmPsi_objRemove(vlanPsi, VLCM_PORT_NUM_ID);   
    (void)BcmPsi_objRemove(vlanPsi, VLCM_PORT_CFG_TBL_ID);
    
    /*get total*/
    for (i = 0, nPorts = 0; i < VLAN_8021Q_PORT_MAX; i++)
    {
        if(g_vlPortList[i].useFlag == VLAN_ENTRY_INUSE)
            nPorts++;
    }

    /*no item need save*/
    if(0 == nPorts)
        return VLAN_STS_OK;
    
    if(NULL == (pports = (char*)malloc(nPorts * sizeof(VLAN_8021Q_PORT_ENTRY))))
        return VLAN_STS_ERR_MEMORY;
    else
        memset(pports, 0, nPorts * sizeof(VLAN_8021Q_PORT_ENTRY));

    /*copy all items need save to a block*/
    for(i = 0, tmp = pports; i < VLAN_8021Q_PORT_MAX ; i++)
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlPortList[i].useFlag)
            continue;

        memcpy(tmp, &g_vlPortList[i], sizeof(VLAN_8021Q_PORT_ENTRY));
        tmp += sizeof(VLAN_8021Q_PORT_ENTRY);
    }

    /*save data*/
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    (void)BcmPsi_objStore(vlanPsi, VLCM_PORT_NUM_ID, &nPorts, sizeof(int)); 
    (void)BcmPsi_objStore(vlanPsi, VLCM_PORT_CFG_TBL_ID, pports, 
                            sizeof(VLAN_8021Q_PORT_ENTRY) * nPorts); 
    BcmPsi_appClose(vlanPsi);

    free(pports);
    return VLAN_STS_OK;
}

int BcmDb_applyVlanCfg(void)
{
    PSI_HANDLE vlanPsi = NULL;

    /*save data*/
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    (void)BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_ID, &g_vlanCfg, sizeof(VLAN_8021Q_CFG_ENTRY));   
    BcmPsi_appClose(vlanPsi);

    return VLAN_STS_OK;
}
int BcmDb_applyVltbCfg(void)
{
    PSI_HANDLE vlanPsi = NULL;
    char *pvlans = NULL;
    char *tmp    = NULL;
    int nVlan    = 0;
    int i; 

    /*remove all at first*/
    vlanPsi = BcmPsi_appOpen(VLAN_APPID); 
    (void)BcmPsi_objRemove(vlanPsi, VLCM_VLAN_NUM_ID);   
    (void)BcmPsi_objRemove(vlanPsi, VLCM_VLAN_CFG_TBL_ID);   
    
    /*get total*/
    for(i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(VLAN_ENTRY_INUSE == g_vlanList[i].useFlag)
            nVlan ++;
    }

    /*no item need save*/
    if(0 == nVlan)
        return VLAN_STS_OK;
    
    if( NULL == (pvlans = (char*)malloc(sizeof(VLAN_8021Q_ITEM_ENTRY) * nVlan)))
        return VLAN_STS_ERR_MEMORY;
    else    
        memset(pvlans, 0, sizeof(VLAN_8021Q_ITEM_ENTRY) * nVlan);

    /*copy all items need save to a block*/
    for(i = 0, tmp = pvlans; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlanList[i].useFlag)
            continue;

        memcpy(tmp, &g_vlanList[i], sizeof(VLAN_8021Q_ITEM_ENTRY));
        tmp += sizeof(VLAN_8021Q_ITEM_ENTRY);
    }
   
    /*save data*/
    (void)BcmPsi_objStore(vlanPsi, VLCM_VLAN_NUM_ID, &nVlan, sizeof(int));   
    (void)BcmPsi_objStore(vlanPsi, VLCM_VLAN_CFG_TBL_ID, pvlans, 
                             sizeof(VLAN_8021Q_ITEM_ENTRY) * nVlan);   

    free(pvlans);
    return VLAN_STS_OK;
}

int BcmDb_loadVLPortAttr(void)
{
    VLAN_8021Q_PORT_ENTRY* pport1 = NULL;
    VLAN_8021Q_PORT_ENTRY* pport2 = NULL;
    PSI_HANDLE vlanPsi = NULL;
    UINT16 retLen      = 0;
    char *pports = NULL;
    int nPorts   = 0;
    int i;

    if(NULL == (vlanPsi = BcmPsi_appOpen(VLAN_APPID)))
    {
        if(DEBUG_VBR)
            printf("BRVLAN: cant open psi app %s, in %s\n", VLAN_APPID, __FUNCTION__);
        
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
    
    (void)BcmPsi_objRetrieve(vlanPsi, VLCM_PORT_NUM_ID, &nPorts, sizeof(int), &retLen);

    /*no port be configrated*/
    if(0 == nPorts)
        return VLAN_STS_OK;
    else if(DEBUG_VBR)
        printf("BRVLAN: get port attr num %d from psi\n", nPorts);
    
    if(NULL == (pports = (char*)malloc(nPorts * sizeof(VLAN_8021Q_PORT_ENTRY))))
        return VLAN_STS_ERR_MEMORY;
    else
        memset(pports, 0, nPorts * sizeof(VLAN_8021Q_PORT_ENTRY));

    if(PSI_STS_OK == BcmPsi_objRetrieve(vlanPsi, VLCM_PORT_CFG_TBL_ID, pports, 
                     nPorts * sizeof(VLAN_8021Q_PORT_ENTRY), &retLen))
    {
        pport1 = (VLAN_8021Q_PORT_ENTRY*)pports; 
        for(i = 0; i < nPorts; i++)        
        {
            pport2 = BcmDb_regVlanPort(pport1[i].name);

            if(!pport2)
                continue;

            pport2->mode = pport1[i].mode;
            pport2->prio = pport1[i].prio;
            pport2->pvid = pport1[i].pvid;

            if(DEBUG_VBR)
                printf("BRVLAN: load port attr from psi, name %s, mode %d, pvid %d, prio%d\n",
                pport2->name, pport2->mode, pport2->pvid, pport2->prio);
        }
        free(pports);
        return VLAN_STS_OK;
    }
    else
    {
        free(pports);
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
}

int BcmDb_loadVltbCfg(void)
{
    VLAN_8021Q_ITEM_ENTRY *pvlan1 = NULL;
    VLAN_8021Q_ITEM_ENTRY *pvlan2 = NULL;
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE vlanPsi = NULL;
    UINT16 retLen   = 0;
    char *pvlans    = NULL;
    int nVlan       = 0;
    int i; 

    /*get total*/
    if(NULL == (vlanPsi = BcmPsi_appOpen(VLAN_APPID)))
    {
        if(DEBUG_VBR)
            printf("BRVLAN: cant open psi app %s, in %s\n", VLAN_APPID, __FUNCTION__);
        
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
        
    sts = BcmPsi_objRetrieve(vlanPsi, VLCM_VLAN_NUM_ID, &nVlan, sizeof(int), &retLen);

    if(DEBUG_VBR && sts != PSI_STS_OK)
        printf("BRVLAN: can't Retrieve psi object %d in app %s when vlan table size loading, errno %d\n",
                VLCM_VLAN_NUM_ID, VLAN_APPID, sts);

    /*no item need save*/
    if(0 == nVlan)
        return VLAN_STS_OK;
    else if(DEBUG_VBR)
        printf("BRVLAN: get vlan num %d from psi\n", nVlan);
    
    if( NULL == (pvlans = (char*)malloc(sizeof(VLAN_8021Q_ITEM_ENTRY) * nVlan)))
        return VLAN_STS_ERR_MEMORY;
    else    
        memset(pvlans, 0, sizeof(VLAN_8021Q_ITEM_ENTRY) * nVlan);

    /*get all vlans*/
    if(PSI_STS_OK ==(sts = BcmPsi_objRetrieve(vlanPsi, VLCM_VLAN_CFG_TBL_ID, pvlans, 
                     nVlan * sizeof(VLAN_8021Q_ITEM_ENTRY), &retLen)))
    {
        pvlan1 = (VLAN_8021Q_ITEM_ENTRY*)pvlans;
        for(i = 0; i < nVlan; i++)        
        {
            pvlan2 = BcmDb_getVlanbyNo(pvlan1[i].no);

            if(!pvlan2)
                (void)BcmDb_addVlan(&pvlan1[i]);
            else
            {
                strcpy(pvlan2->grpIf, pvlan1[i].grpIf);
                strcpy(pvlan2->ifList, pvlan1[i].ifList);
                pvlan2->macln = pvlan1[i].macln;
                pvlan2->vlanid = pvlan1[i].vlanid;
            }

            if(DEBUG_VBR)
                printf("BRVLAN: load vlan from psi, no %d, vlanid %d, macln %d, grpIf %s, ifList %s\n",
                pvlan1[i].no, pvlan1[i].vlanid, pvlan1[i].macln, pvlan1[i].grpIf, pvlan1[i].ifList);
        }
        free(pvlans);
        return VLAN_STS_OK;
    }
    else
    {
        free(pvlans);
        
        if(DEBUG_VBR && sts != PSI_STS_OK)
            printf("BRVLAN: can't Retrieve psi object %d in app %s when vlan table data loading, errno %d\n",
                    VLCM_VLAN_CFG_TBL_ID, VLAN_APPID, sts);
        
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
}

int BcmDb_loadVlanCfg(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE vlanPsi = NULL;
    VLAN_8021Q_CFG_ENTRY vlcfg;
    UINT16 retLen;    

    if(NULL == (vlanPsi = BcmPsi_appOpen(VLAN_APPID)))
    {
        if(DEBUG_VBR)
            printf("BRVLAN: cant open psi app %s, in %s\n", VLAN_APPID, __FUNCTION__);
        
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
        
    if(PSI_STS_OK ==(sts = BcmPsi_objRetrieve(vlanPsi, VLCM_VLAN_CFG_ID, &vlcfg, 
                      sizeof(VLAN_8021Q_CFG_ENTRY), &retLen)))
    {
        memcpy(&g_vlanCfg, &vlcfg, sizeof(VLAN_8021Q_CFG_ENTRY));
        g_vlanCfg.vlanItem = &g_vlanList[0];
        if(DEBUG_VBR)
            printf("BRVLAN: load vlan cfg, enabled %d, mngrid %d, bpduAct %d, bpduFDIF %s, garpAct %d, garpFDIF %s\n",
            g_vlanCfg.enabled, g_vlanCfg.mngrid,
            g_vlanCfg.bpduAct, g_vlanCfg.bpduFDIF,
            g_vlanCfg.garpAct, g_vlanCfg.garpFDIF);

        return VLAN_STS_OK;
    }
    else
    {
        if(DEBUG_VBR)
            printf("BRVLAN: can't Retrieve psi object %d in app %s, errno %d\n", VLCM_VLAN_CFG_ID, VLAN_APPID, sts);
        return VLAN_STS_ERR_OBJECT_NOT_FOUND;
    }
}

/*set vlan configration to kernel*/
void BcmDb_makeVlanEffec(void) {    
    char cmd[IFC_GIANT_LEN];
    int i;

    /*configration per vlan*/
    for (i = 0; i < VLAN_8021Q_NUM_MAX; i++)
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlanList[i].useFlag)
            continue;
        
        /*add vlan to bridge*/
        sprintf(cmd, "brctl addvl %s %d", g_vlanList[i].grpIf, g_vlanList[i].vlanid);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        
        /*set vlan info to eth chip*/
        BcmDb_setVlanToEthChip(g_vlanList[i].ifList);
        /*add vlan port to vlan*/
        sprintf(cmd, "brctl addvlp %s %d %s", 
                 g_vlanList[i].grpIf, 
                 g_vlanList[i].vlanid, 
                 g_vlanList[i].ifList);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        
        /*set mac learning flag*/
        sprintf(cmd, "brctl setmacln %s %d %d", 
                 g_vlanList[i].grpIf, 
                 g_vlanList[i].vlanid, 
                 g_vlanList[i].macln);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        
    }

    //globe vlan configration    
    /*vlan enable/disable*/
    sprintf(cmd, "brctl setbrvlan br0 %d",g_vlanCfg.enabled);
    bcmSystem( cmd );
    if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    
    /*management vlan id*/
    sprintf(cmd, "brctl setmngrvid br0 %d",g_vlanCfg.mngrid);
    bcmSystem( cmd );
    if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);

    /*garp*/
    if(VLAN_SKB_DISCARD == g_vlanCfg.garpAct)
    {
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -j DROP", VLAN_GVRP_MAC_STR);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -j DROP", VLAN_GMRP_MAC_STR);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    }
    else if(VLAN_SKB_FORWARD == g_vlanCfg.garpAct)
    {
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -o !%s -j DROP",VLAN_GVRP_MAC_STR, g_vlanCfg.garpFDIF);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -o !%s -j DROP",VLAN_GMRP_MAC_STR, g_vlanCfg.garpFDIF);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    }
    
    /*bpdu*/
    if(VLAN_SKB_DISCARD == g_vlanCfg.bpduAct)
    {
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -j DROP", VLAN_BPDU_MAC_STR);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    }
    else if(VLAN_SKB_FORWARD == g_vlanCfg.bpduAct)
    {
        sprintf(cmd, "ebtables -A FORWARD -d %s -i !nas+ -o !%s -j DROP", VLAN_BPDU_MAC_STR, g_vlanCfg.bpduFDIF);
        bcmSystem( cmd );
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    }
}
//**************************************************************************
// Function Name: BcmDb_makeVLPortAttrEffec
// Description  : set vlan port attributes to kernel
// Returns      : none.
//**************************************************************************
void BcmDb_makeVLPortAttrEffec(void) {    
    char cmd[IFC_SMALL_LEN];
    int i;
        
    for (i = 0; i < VLAN_8021Q_PORT_MAX; i++)
    {
        if(VLAN_ENTRY_NOTINUSE == g_vlPortList[i].useFlag)
            continue;

        /*port mode , refer to br_private.h*/
        sprintf(cmd, "brctl setpmd %s %d", g_vlPortList[i].name, g_vlPortList[i].mode);
        bcmSystem(cmd);
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        
        /*port pvid [0-4094]*/
        sprintf(cmd, "brctl setpvid %s %d", g_vlPortList[i].name, g_vlPortList[i].pvid);
        bcmSystem(cmd);
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
        
        /*port priority, [0,7] ,other is invalid*/
        if(VLAN_ISVALID_PRIO(g_vlPortList[i].prio))
            sprintf(cmd, "brctl setprio %s %d 1", g_vlPortList[i].name, g_vlPortList[i].prio);
        else
            sprintf(cmd, "brctl setprio %s 0 0", g_vlPortList[i].name);    
        bcmSystem(cmd);
        if(DEBUG_VBR)printf("BRVLAN: exec config cmd, %s\n", cmd);
    }

}
//**************************************************************************
// Function Name: BcmDb_setVlanToEthChip
// Description  : 将vlan配置信息设置到lanswitch芯片上，规避5325e虚接口无法
//                lan侧网口隔离
// Returns      : none.
//**************************************************************************
void BcmDb_setVlanToEthChip(char *ifList) {  
    char ifclst[VLAN_INTF_LIST_SIZE] = {0};
    unsigned short mask = 0;
    char *ptmp = NULL;
    char *pifc = NULL;

    if(NULL == ifList)
        return;

    strncpy(ifclst, ifList, VLAN_INTF_LIST_SIZE);
    ptmp = (char *)ifclst;
    while(NULL != (pifc = strsep(&ptmp, ",")))
    {
        if(strncmp(pifc, "eth", 3) != 0)
            continue;

        if(strstr(pifc, ".2") != NULL)
            mask |= 0x0001;
        else if(strstr(pifc, ".3") != NULL)
            mask |= 0x0002;
        else if(strstr(pifc, ".4") != NULL)
            mask |= 0x0004;
        else if(strstr(pifc, ".5") != NULL)
            mask |= 0x0008;
    }
    
    if(mask)
    {
        BcmLan_setKpnVlan(mask);
        if(DEBUG_VBR)printf("BRVLAN: set vlan to eth chip, mask = 0x%x\n", mask);
    }
    
}
/*end vlan bridge, s60000658, 20060627*/
//#endif

/* start of voice 增加SIP的配置项 by z45221 zhangchen 2006年11月7日 */
#ifdef VOXXXLOAD
//**************************************************************************
// Function Name: BcmDb_setVoiceSize
// Description  : store voice size
// Returns      : none.
//**************************************************************************
void BcmDb_setVoiceSize(int id, int info)
{
    UINT16 size = info;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    BcmPsi_objStore(voicePsi, id, &size, sizeof(size));

}

//**************************************************************************
// Function Name: BcmDb_getVoiceSize
// Description  : retrieve Voice size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVoiceSize(int id, int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE secPsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(secPsi, id, &size, sizeof(size), &retLen);

    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setVoiceInfo
// Description  : store voice table
// Returns      : none.
//**************************************************************************
void BcmDb_setVoiceInfo(int id, PSI_VALUE info, int size) {
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    BcmPsi_objStore(voicePsi, id, info, size);
}

//**************************************************************************
// Function Name: BcmDb_getVoiceInfo
// Description  : retrieve voice table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getVoiceInfo(int id, PSI_VALUE info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(voicePsi, id, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSipSrvSize
// Description  : store SIP Server size
// Returns      : none.
//**************************************************************************
void BcmDb_setSipSrvSize(int info){
    BcmDb_setVoiceSize(VOICE_SIP_SRV_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getSipSrvSize
// Description  : retrieve QoS class size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipSrvSize(int *info) {
    return BcmDb_getVoiceSize(VOICE_SIP_SRV_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setSipSrvInfo
// Description  : store sip server table
// Returns      : none.
//**************************************************************************
void BcmDb_setSipSrvInfo(PSIP_WEB_SERVER_S tbl, int size)
{
    BcmDb_setVoiceInfo(VOICE_SIP_SRV_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SIP_WEB_SERVER_S) * size);

}

//**************************************************************************
// Function Name: BcmDb_getSipSrvInfo
// Description  : retrieve sip server table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipSrvInfo(PSIP_WEB_SERVER_S tbl, int size) {
    return BcmDb_getVoiceInfo(VOICE_SIP_SRV_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SIP_WEB_SERVER_S) * size);
}

//**************************************************************************
// Function Name: BcmDb_setSipBscSize
// Description  : store SIP Basic size
// Returns      : none.
//**************************************************************************
void BcmDb_setSipBscSize(int info)
{
    BcmDb_setVoiceSize(VOICE_SIP_BSC_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getSipBscSize
// Description  : retrieve sip basic size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipBscSize(int *info) {
    return BcmDb_getVoiceSize(VOICE_SIP_BSC_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_setSipBscInfo
// Description  : store sip basic table
// Returns      : none.
//**************************************************************************
void BcmDb_setSipBscInfo(PSIP_WEB_USER_INFO_S tbl, int size)
{
    BcmDb_setVoiceInfo(VOICE_SIP_BSC_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(SIP_WEB_USER_INFO_S) * size);
}

//**************************************************************************
// Function Name: BcmDb_getSipBscInfo
// Description  : retrieve sip sip basic table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipBscInfo(PSIP_WEB_USER_INFO_S tbl, int size) {
    return BcmDb_getVoiceInfo(VOICE_SIP_BSC_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(SIP_WEB_USER_INFO_S) * size);
}

/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
void BcmDb_setSipProvInfo(PSIP_WEB_PROV_CFG_S info) 
{
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    BcmPsi_objStore(voicePsi, VOICE_SIP_PROV_ID, info, sizeof(SIP_WEB_PROV_CFG_S));
}

int BcmDb_getSipProvInfo(SIP_WEB_PROV_CFG_S *info) 
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_PROV_ID, info,
                                sizeof(SIP_WEB_PROV_CFG_S), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;

    return ret;
}
/* END:   Added by y67514, 2009/11/16 */

//**************************************************************************
// Function Name: BcmDb_setSipQos
// Description  : store sip qos info
// Returns      : none.
//**************************************************************************
void BcmDb_setSipQosInfo(PSIP_WEB_QOS_CFG_S info) {
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    BcmPsi_objStore(voicePsi, VOICE_SIP_QOS_ID, info, sizeof(SIP_WEB_QOS_CFG_S));
}

//**************************************************************************
// Function Name: BcmDb_getSipQosInfo
// Description  : retrieve sip qos info
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipQosInfo(SIP_WEB_QOS_CFG_S *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_QOS_ID, info,
                                sizeof(SIP_WEB_QOS_CFG_S), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setSipAdvInfo
// Description  : store sip advanced info
// Returns      : none.
//**************************************************************************
void BcmDb_setSipAdvInfo(PSIP_WEB_ADV_CFG_S info) {
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    BcmPsi_objStore(voicePsi, VOICE_SIP_ADV_ID, info, sizeof(SIP_WEB_ADV_CFG_S));
}

//**************************************************************************
// Function Name: BcmDb_getSipAdvInfo
// Description  : retrieve sip advanced info
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getSipAdvInfo(SIP_WEB_ADV_CFG_S *info){
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_ADV_ID, info,
                                sizeof(SIP_WEB_ADV_CFG_S), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_getPhoneCommonInfo
// Description  : retrieve phone common info
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPhoneCommonInfo(SIP_WEB_PHONE_CFG_S *info){
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    status = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_PHONECFG_ID, info,
                                sizeof(SIP_WEB_PHONE_CFG_S), &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;

    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setPhoneCommonInfo
// Description  : store phone common info
// Returns      : none.
//**************************************************************************
void BcmDb_setPhoneCommonInfo(SIP_WEB_PHONE_CFG_S *info) {
    PSI_HANDLE voicePsi = BcmPsi_appOpen(VOICE_APPID);
    
    BcmPsi_objStore(voicePsi, VOICE_SIP_PHONECFG_ID, info, sizeof(SIP_WEB_PHONE_CFG_S));
}

/*****************************************************************************
 函 数 名  : HWDb_getPhoneCommon
 功能描述  : 获得存储SIP PHONE机被配置信息数据结构的指针，调用者可以使用该指
             针来读取和修改SIP Phone基本配置信息
 输入参数  : void  
 输出参数  : 无
 返 回 值  : SIP_WEB_PHONE_CFG_S*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
SIP_WEB_PHONE_CFG_S* HWDb_getPhoneCommon(void)
{
    return &g_phoneCfg;
}
/*****************************************************************************
 功能描述  : 显示SIP Phone拍叉上下限信息
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2007年1月4日
    作    者   : y42304
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_ShowPhoneFlashInfo(void)      
{
    HWDb_loadPhoneCommon();
    printf("\n Current flash downLimit: %d; upLimit: %d.\n", g_phoneCfg.minHookFlash,  g_phoneCfg.maxHookFlash);
}

#if defined(SUPPORT_SLIC_3210) || defined(SUPPORT_SLIC_3215)|| defined(SUPPORT_SLIC_LE88221)
/*****************************************************************************
 功能描述  : 将SIP Phone拍叉上下限信息存入PSI数据库
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2007年1月4日
    作    者   : y42304
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_applyPhoneFlashCommon(CAS_CTL_FXO_PARMS fxoParams)      
{
    PSI_HANDLE voicePsi = NULL;
    PSI_STATUS sts = PSI_STS_OK;

    g_phoneCfg.minHookFlash = fxoParams.minHookFlash;
    g_phoneCfg.maxHookFlash = fxoParams.maxHookFlash;
    voicePsi = BcmPsi_appOpen(VOICE_APPID); 
    
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_PHONECFG_ID, &g_phoneCfg, sizeof(SIP_WEB_PHONE_CFG_S));   
    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save PhoneCommon cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_CALLFWD_NUM_ID, __FUNCTION__, sts);      
    }
}
#endif
/*****************************************************************************
 函 数 名  : HWDb_applyPhoneCommon
 功能描述  : 将SIP Phone基本配置信息存入PSI数据库
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_applyPhoneCommon(void)
{
    PSI_HANDLE voicePsi = NULL;
    PSI_STATUS sts = PSI_STS_OK;
    
    voicePsi = BcmPsi_appOpen(VOICE_APPID); 
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_PHONECFG_ID, &g_phoneCfg, sizeof(SIP_WEB_PHONE_CFG_S));   
    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save PhoneCommon cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_CALLFWD_NUM_ID, __FUNCTION__, sts);      
    }
}
/*****************************************************************************
 函 数 名  : HWDb_loadPhoneCommon
 功能描述  : 从PSI中加载SIP Phone基本配置信息
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_loadPhoneCommon(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE voicePsi = NULL;
    SIP_WEB_PHONE_CFG_S phoneCfg;
    UINT16 retLen;    

    if(NULL == (voicePsi = BcmPsi_appOpen(VOICE_APPID)))
    {
        printf("SIP: cant open psi app %s, in %s\n", VOICE_APPID, __FUNCTION__);
        return ;
    }
        
    if(PSI_STS_OK ==(sts = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_PHONECFG_ID, &phoneCfg, 
                      sizeof(SIP_WEB_PHONE_CFG_S), &retLen)))
    {
        memcpy(&g_phoneCfg, &phoneCfg, sizeof(SIP_WEB_PHONE_CFG_S));
#ifdef DEBUG_SIP
/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
/* BEGIN: Modified by p00102297, 2008/2/20 */
        printf("SIP: load phone cfg, speak %d, listen %d, interval %d, region %d, minHookFlash %d, maxHookFlash %d digitmap %s OffHookTime %d, OnHookTime %d, FirstDialingInterval %d, ReInjectionValue %d, InnerCall %d, ModemService %d\n",
        g_phoneCfg.lSpkVol, g_phoneCfg.lLstVol,
        g_phoneCfg.ulDialInterval, 
        g_phoneCfg.ulRegion, 
        g_phoneCfg.minHookFlash, g_phoneCfg.maxHookFlash,
        g_phoneCfg.szDigitMap,
        g_phoneCfg.ulOffHookTime, g_phoneCfg.ulOnHookTime, 
        g_phoneCfg.ulFstDialTime,
        g_phoneCfg.ulReInjection,
        g_phoneCfg.ulInnerCall,
        g_phoneCfg.ulModemService);
/* END:   Modified by p00102297, 2008/2/20 */
/*END: Added by chenyong 65116 2008-09-15 web配置*/
#endif
    }
    else
    {
        printf("SIP: can't Retrieve psi object %d in app %s, errno %d\n", VOICE_SIP_PHONECFG_ID, VOICE_APPID, sts);
    }
}
/*****************************************************************************
 函 数 名  : HWDb_getAllDialPlan
 功能描述  : 获得快速拨号数据存储的指针，该指针可以被调用者用来读取和修改快
             速拨号信息
 输入参数  : void  
 输出参数  : 无
 返 回 值  : SIP_WEB_DIAL_PLAN_S*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
SIP_WEB_DIAL_PLAN_S* HWDb_getAllDialPlan(void)
{
    return &g_dialPlan[0];
}
/*****************************************************************************
 函 数 名  : HWDb_applyAllDialPlan
 功能描述  : 将快速拨号信息存储到PSI
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_applyAllDialPlan(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE voicePsi = NULL;
    int nDialPlan    = SIP_MAX_DIAL_PLAN;

    /*remove all at first*/
    voicePsi = BcmPsi_appOpen(VOICE_APPID); 
    (void)BcmPsi_objRemove(voicePsi, VOICE_SIP_SPEEDDIAL_NUM_ID);   
    (void)BcmPsi_objRemove(voicePsi, VOICE_SIP_SPEEDDIAL_TBL_ID);   
       
    /*save data*/
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_SPEEDDIAL_NUM_ID, &nDialPlan, sizeof(int));  
    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save DialPlan cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_SPEEDDIAL_NUM_ID, __FUNCTION__, sts);      
        return;
    }
    
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_SPEEDDIAL_TBL_ID, &g_dialPlan, 
                             sizeof(SIP_WEB_DIAL_PLAN_S) * nDialPlan); 
    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save DialPlan cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_SPEEDDIAL_NUM_ID, __FUNCTION__, sts);      
        return;
    }

}
/*****************************************************************************
 函 数 名  : HWDb_loadAllDialPlan
 功能描述  : 从PSI中加载所有快速拨号配置信息，这些信息供配置管理工具使用，比如web
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_loadAllDialPlan(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE voicePsi = NULL;
    UINT16 retLen   = 0;
    char *pDialPlan    = NULL;
    int nDialPlan       = 0;
    int i; 

    /*get total*/
    if(NULL == (voicePsi = BcmPsi_appOpen(VOICE_APPID)))
    {
        printf("SIP: cant open psi app %s, in %s\n", VOICE_APPID, __FUNCTION__);
        
        return ;
    }

    sts = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_SPEEDDIAL_NUM_ID, &nDialPlan, sizeof(int), &retLen);
    if( PSI_STS_OK != sts)
    {
        printf("SIP: can't Retrieve psi object %d in app %s when DialPlan table size loading, errno %d\n",
                VOICE_SIP_SPEEDDIAL_NUM_ID, VOICE_APPID, sts);
        return;
    }
    
    /*no item need save*/
    if(0 == nDialPlan)
    {
        return ;
    }
    
#ifdef DEBUG_SIP
        printf("SIP: get dial plan num %d from psi\n", nDialPlan);
#endif
    
    if( NULL == (pDialPlan = (char*)malloc(sizeof(SIP_WEB_DIAL_PLAN_S) * nDialPlan)))
    {
        printf("SIP: memery not enough  %s, %d\n", __FILE__, __LINE__);
        return ;
    }
    else    
    {
        memset(pDialPlan, 0, sizeof(SIP_WEB_DIAL_PLAN_S) * nDialPlan);
    }

    /*get all vlans*/
    if(PSI_STS_OK ==(sts = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_SPEEDDIAL_TBL_ID, pDialPlan, 
                     nDialPlan * sizeof(SIP_WEB_DIAL_PLAN_S), &retLen)))
    {
        memcpy(&g_dialPlan, pDialPlan, retLen);

#ifdef DEBUG_SIP
        for(i = 0; i < retLen/sizeof(SIP_WEB_DIAL_PLAN_S); i++)
        {
            printf("SIP: load dialplan from psi, num %s, name %s, dest %s\n",
            g_dialPlan[i].szDialNum, g_dialPlan[i].szName, g_dialPlan[i].szRealNum);
        }
#endif
    }
    else
    {
        printf("SIP: can't Retrieve psi object %d in app %s , errno %d\n",
                VOICE_SIP_SPEEDDIAL_TBL_ID, VOICE_APPID, sts);
        
    }

    free(pDialPlan);
}

/*****************************************************************************
 函 数 名  : HWDb_getAllCallForwardInfo
 功能描述  : 获取呼叫前转表指针,调用者可以通过该函数返回的指针设置呼叫前转表配置
 输入参数  : void  
 输出参数  : 无
 返 回 值  : SIP_WEB_FWD_S*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
SIP_WEB_FWD_S* HWDb_getAllCallForwardInfo(void)
{
    return &g_callFWD[0];
}
/*****************************************************************************
 函 数 名  : HWDb_applyAllCallForwardInfo
 功能描述  : 将呼叫前转表配置数据存入PSI中
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_applyAllCallForwardInfo(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE voicePsi = NULL;
    SIP_WEB_FWD_S *pCallFWD = NULL;
    int nCallFWD       = SIP_MAX_CALL_FWD * (SIP_WEB_FWD_ADVANCED_NUM + SIP_WEB_FWD_BASIC_NUM + 1);
    int nAllNumMemLen  = 0;
    char *pFWD = NULL;
    char *pSrc = NULL;
    char *pDst = NULL;
    int i ;
    
    if(NULL == (pFWD = (char*)malloc(sizeof(SIP_WEB_FWD_NUM_S) * nCallFWD)))
    {
        printf("SIP: memery not enough  %s, %d\n", __FILE__, __LINE__);
        return;
    }

    pCallFWD = HWDb_getAllCallForwardInfo();
    pDst = pFWD;

    nAllNumMemLen = (sizeof(SIP_WEB_FWD_NUM_S) * (SIP_WEB_FWD_ADVANCED_NUM + SIP_WEB_FWD_BASIC_NUM));
    for(i = 0; i < SIP_MAX_CALL_FWD; i++)
    {
        pSrc = (char*)( &( pCallFWD[i] ) );
        memcpy(pDst, pSrc, nAllNumMemLen + sizeof(VOS_UINT32));
        pDst += nAllNumMemLen + sizeof(SIP_WEB_FWD_NUM_S);
    }
    
    /*remove all at first*/
    voicePsi = BcmPsi_appOpen(VOICE_APPID); 
    (void)BcmPsi_objRemove(voicePsi, VOICE_SIP_CALLFWD_NUM_ID);   
    (void)BcmPsi_objRemove(voicePsi, VOICE_SIP_CALLFWD_TBL_ID);   
       
    /*save data*/
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_CALLFWD_NUM_ID, &nCallFWD, sizeof(int));   
    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save CallForward cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_CALLFWD_NUM_ID, __FUNCTION__, sts);      
        free(pFWD);
        return;
    }
    sts = BcmPsi_objStore(voicePsi, VOICE_SIP_CALLFWD_TBL_ID, pFWD, 
                             sizeof(SIP_WEB_FWD_NUM_S) * nCallFWD); 

    if(PSI_STS_OK != sts)
    {
        printf("SIP: cant save CallForward cfg app %s, cfgid %d, in %s, err %d\n", 
                VOICE_APPID, VOICE_SIP_CALLFWD_NUM_ID, __FUNCTION__, sts);      
        free(pFWD);
        return;
    }

    free(pFWD);
}

/*****************************************************************************
 函 数 名  : HWDb_loadAllCallForwardInfo
 功能描述  : 从PSI中加载呼叫前转表配置数据到内存变量中供配置管理工具使用
 输入参数  : void  
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID HWDb_loadAllCallForwardInfo(void)
{
    PSI_STATUS sts = PSI_STS_OK;
    PSI_HANDLE voicePsi = NULL;
    UINT16 retLen       = 0;
    int nCallFWD        = 0;
    int nAllNumMemLen  = 0;
    char *pFWD = NULL;
    char *pSrc = NULL;
    char *pDst = NULL;
    int i ;

    /*get total*/
    if(NULL == (voicePsi = BcmPsi_appOpen(VOICE_APPID)))
    {
        printf("SIP: cant open psi app %s, in %s\n", VOICE_APPID, __FUNCTION__);        
        return ;
    }

    sts = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_CALLFWD_NUM_ID, &nCallFWD, sizeof(int), &retLen);
    if( PSI_STS_OK != sts)
    {
        printf("SIP: can't Retrieve psi object %d in app %s when CallForward table size loading, errno %d\n",
                VOICE_SIP_CALLFWD_NUM_ID, VOICE_APPID, sts);
        return;
    }
    
    /*no item need save*/
    if(0 == nCallFWD)
    {
        return ;
    }
    
#ifdef DEBUG_SIP
        printf("SIP: get AllCallForward num %d from psi retLen=%d\n", nCallFWD, retLen);
#endif
    
    if( NULL == (pFWD = (char*)malloc(sizeof(SIP_WEB_FWD_NUM_S) * nCallFWD)))
    {
        printf("SIP: memery not enough  %s, %d\n", __FILE__, __LINE__);
        return ;
    }
    else    
    {
        memset(pFWD, 0, sizeof(SIP_WEB_FWD_NUM_S) * nCallFWD);
    }

    /*get call forward tables*/
    if(PSI_STS_OK ==(sts = BcmPsi_objRetrieve(voicePsi, VOICE_SIP_CALLFWD_TBL_ID, pFWD, 
                     sizeof(SIP_WEB_FWD_NUM_S) * nCallFWD, &retLen)))
    {
        nAllNumMemLen = (sizeof(SIP_WEB_FWD_NUM_S) * (SIP_WEB_FWD_ADVANCED_NUM + SIP_WEB_FWD_BASIC_NUM));

        if(retLen != SIP_MAX_CALL_FWD * (nAllNumMemLen + sizeof(SIP_WEB_FWD_NUM_S)))
        {
            printf("SIP: call forward tables may be truncated, retlen %d, needlen %d ,%s, %d\n",
                     retLen, SIP_MAX_CALL_FWD * (nAllNumMemLen + sizeof(SIP_WEB_FWD_NUM_S)), __FILE__, __LINE__);
            free(pFWD);
            return;
        }
        
        pSrc = pFWD;
        pDst = (char*)HWDb_getAllCallForwardInfo();       

        for(i = 0; i < SIP_MAX_CALL_FWD; i++)
        {
            memcpy(pDst, pSrc, nAllNumMemLen + sizeof(VOS_UINT32));
            pSrc += nAllNumMemLen + sizeof(SIP_WEB_FWD_NUM_S);
            pDst += nAllNumMemLen + sizeof(VOS_UINT32);
        }

#ifdef DEBUG_SIP
        for(i = 0; i < SIP_MAX_CALL_FWD; i++)
        {
            int j;
            printf("SIP: fw table %d, nawt %d\n", i, g_callFWD[i].ulNoReplyTime);
            
            printf("    basic   fwtb: index active incoming forwar condition\n");
            for(j = 0; j < SIP_WEB_FWD_ADVANCED_NUM; j++)
            {
                printf("                   %d     ,  %d  ,%s  ,%s  ,%d\n", j,
                        g_callFWD[i].pstBasic[j].bIsActive, 
                        g_callFWD[i].pstBasic[j].szIncoming,
                        g_callFWD[i].pstBasic[j].szForward,
                        g_callFWD[i].pstBasic[j].enFwdType);
            }
            
            printf("    advance fwtb: index active incoming forwar condition\n");
            for(j = 0; j < SIP_WEB_FWD_ADVANCED_NUM; j++)
            {
                printf("                   %d     ,  %d  ,%s  ,%s  ,%d\n", j,
                        g_callFWD[i].pstAdvance[j].bIsActive, 
                        g_callFWD[i].pstAdvance[j].szIncoming,
                        g_callFWD[i].pstAdvance[j].szForward,
                        g_callFWD[i].pstAdvance[j].enFwdType);
            }            
        }
#endif
    }
    else
    {
        printf("SIP: can't Retrieve psi object %d in app %s , errno %d\n",
                VOICE_SIP_SPEEDDIAL_TBL_ID, VOICE_APPID, sts);        
    }

    free(pFWD);
}
/*****************************************************************************
 函 数 名  : appendParamToVar
 功能描述  : 将SIP业务配置数据写入指定文件中,以便在SIP业务模块启动后可以获取
             到这些配置数据
 输入参数  : VOS_CHAR *pFile  存储配置数据的文件
             VOS_CHAR *pName  要存储的参数名
             VOS_CHAR *pVal   要存储的参数值
 输出参数  : 无
 返 回 值  : static VOS_UINT32
             如果存储成功返回  VOS_OK
             如果存储失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
static VOS_UINT32 appendParamToVar(VOS_CHAR *pFile, VOS_CHAR *pName, VOS_CHAR *pVal)
{
    FILE *fs;

    if((VOS_NULL == pFile) || (VOS_NULL == pName) || (VOS_NULL == pVal) )
    {
        return VOS_ERR;
    }
    
    if ( (access(pFile, F_OK)) != 0 )
    {
       return VOS_ERR;
    }
       
    fs = fopen(pFile, "a+");

    if ( fs != NULL ) {
        fprintf(fs, "%s=%s\n", pName, pVal);
        fclose(fs);
        
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}

/*****************************************************************************
 函 数 名  : SipSetDebugStatus
 功能描述  : 设置调试开关状态
 输入参数  : VOS_UINT32 ulStatus  
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年12月4日
    作    者   : z37589
    修改内容   : 新生成函数

*****************************************************************************/
int SipSetDebugStatus(VOS_UINT32 ulUeSn, VOS_UINT32 ulStatus)
{
    char        szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char        szAttrVal[SIP_GIANT_LEN * 2] = {0};    
    VOS_CHAR    cmd[strlen(SIP_DEBUG_FILE)+strlen("echo >")+1];

    /*清空配置*/
    sprintf(cmd, "echo >%s", SIP_DEBUG_FILE);
    system(cmd);

    /*设置配置*/
    sprintf(szAttrName,"%d", 0);
    sprintf(szAttrVal, "%d", ulStatus);
    (void)appendParamToVar(SIP_DEBUG_FILE, szAttrName, szAttrVal);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : HWDb_writeSipSrvToVar
 功能描述  : 将sip server配置数据写入到共享目录,以便SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeSipSrvToVar(PSIP_WEB_SERVER_S pstSipServer)
{
    PSIP_WEB_SERVER_S pstPtr;
    int i;
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 
    
    for (i = 0; i < SIP_SRV_MAX; i++)
    {
        pstPtr = &pstSipServer[i];
        sprintf(szAttrName, "%s/%d/regaddr", SIP_CFGDIR_SIP_SERVER, i);
        sprintf(szAttrVal, "%s", pstPtr->stSipRegAddr.szIPAddr);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/regport", SIP_CFGDIR_SIP_SERVER, i);
        sprintf(szAttrVal, "%d", pstPtr->stSipRegAddr.uwPort);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/prxyaddr", SIP_CFGDIR_SIP_SERVER, i);
        sprintf(szAttrVal, "%s", pstPtr->stSipProxyAddr.szIPAddr);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/prxyport", SIP_CFGDIR_SIP_SERVER, i);
        sprintf(szAttrVal, "%d", pstPtr->stSipProxyAddr.uwPort);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/domain", SIP_CFGDIR_SIP_SERVER, i);
        sprintf(szAttrVal, "%s", pstPtr->acDomainName);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    } 
}

/*****************************************************************************
 函 数 名  : HWDb_writeSipBscToVar
 功能描述  : 将sip basic配置数据写入到共享目录,以便SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeSipBscToVar(PSIP_WEB_USER_INFO_S pstSipBasic)
{
    PSIP_WEB_USER_INFO_S pstPtr;
    int i;
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 
    
    for (i = 0; i < SIP_USER_MAX; i++)
    {
        pstPtr = &pstSipBasic[i];
        sprintf(szAttrName, "%s/%d/sipid", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%s", pstPtr->szId);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);        

        sprintf(szAttrName, "%s/%d/sipusrname", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%s", pstPtr->szName);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/sipusrpwd", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%s", pstPtr->szPsw);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/siplocalport", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%d", pstPtr->uwPort);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        sprintf(szAttrName, "%s/%d/incallfwtblid", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%d", pstPtr->ulFwdTbl);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
        sprintf(szAttrName, "%s/%d/sipclipenable", SIP_CFGDIR_SIP_BASIC, i);
        sprintf(szAttrVal, "%d", pstPtr->usCliStatus);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        /* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
		sprintf(szAttrName, "%s/%d/siplineenable", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

        /*start of addition by chenyong 2009-03-19*/
		sprintf(szAttrName, "%s/%d/siplinecallhold", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineCallHoldEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
        /*end of addition by chenyong 2009-03-19*/

		sprintf(szAttrName, "%s/%d/siplinecallwait", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineCWEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
		
		/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
		sprintf(szAttrName, "%s/%d/sipline3way", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLine3WayEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/siplinecalltransfer", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineCallTransferEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
		/*END: Added by chenyong 65116 2008-09-15 web配置*/
		/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
		sprintf(szAttrName, "%s/%d/siplinemwi", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineMWIEnable);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/siplinemwiaccount", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%s", pstPtr->szLineMWIAccount);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
        /* END:   Added by chenzhigang, 2008/11/4 */
		sprintf(szAttrName, "%s/%d/siplinefaxdetect", SIP_CFGDIR_SIP_BASIC, i);
		sprintf(szAttrVal, "%d", pstPtr->usLineFaxDetect);
		(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/vad", SIP_CFGDIR_SIP_BASIC, i);       
	    sprintf(szAttrVal, "%d", pstPtr->ulVad);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
		sprintf(szAttrName, "%s/%d/ec", SIP_CFGDIR_SIP_BASIC, i);      
	    sprintf(szAttrVal, "%d", pstPtr->ulEC);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/cng", SIP_CFGDIR_SIP_BASIC, i);      
	    sprintf(szAttrVal, "%d",  pstPtr->ulCNG);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		/*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
		sprintf(szAttrName, "%s/%d/sipBscVoiceCapCodec1",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[0].ulCodecCap);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapCodec2",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[1].ulCodecCap);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapCodec3",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[2].ulCodecCap);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapCodec4",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[3].ulCodecCap);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapCodec5",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[4].ulCodecCap);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPTime1",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[0].ulPTime);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPTime2",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[1].ulPTime);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPTime3",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[2].ulPTime);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPTime4",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[3].ulPTime);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPTime5",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[4].ulPTime);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPriority1",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[0].ulPriority);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPriority2",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[1].ulPriority);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
		
		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPriority3",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[2].ulPriority);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPriority4",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[3].ulPriority);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapPriority5",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[4].ulPriority);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapIfEnable1",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[0].bEnable);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapIfEnable2",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[1].bEnable);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapIfEnable3",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[2].bEnable);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapIfEnable4",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[3].bEnable);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		sprintf(szAttrName, "%s/%d/sipBscVoiceCapIfEnable5",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.stCodecList[4].bEnable);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		/*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
	    sprintf(szAttrName, "%s/%d/voicedtmftype",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.ulDtmfType);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		//BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
	    sprintf(szAttrName, "%s/%d/voicepayloadtype",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->stVoiceCfg.ulPayloadType);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
		//END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069

		sprintf(szAttrName, "%s/%d/faxtype",  SIP_CFGDIR_SIP_BASIC, i);
	    sprintf(szAttrVal, "%d", pstPtr->ulFaxType);
	    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
		/* END:   Modified by p00102297, 2008/3/11 VDF C02*/
		
        {
        	int j;
        	for (j=0;j<SIP_MAX_DAILNUM_CHANGE_NUM;j++)
        	{
	        	sprintf(szAttrName, "%s/%d/sipSpNum%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stStripPrefixNum[j].szAsideNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipSpName%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stStripPrefixNum[j].szMatchNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipApNum%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stAppandPrefixNum[j].szAsideNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipApName%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stAppandPrefixNum[j].szMatchNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

				/* BEGIN: Modified by p00102297, 2008/1/30 */
				sprintf(szAttrName, "%s/%d/sipSsNum%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stStripSuffixNum[j].szAsideNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipSsName%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stStripSuffixNum[j].szMatchNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipAsNum%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stAppandSuffixNum[j].szAsideNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

		        sprintf(szAttrName, "%s/%d/sipAsName%02d", SIP_CFGDIR_SIP_BASIC, i,j);
		        sprintf(szAttrVal, "%s", pstPtr->stAppandSuffixNum[j].szMatchNum);
		        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
        	}
        }
#endif
    } 
}

/*****************************************************************************
 函 数 名  : HWDb_writeSipAdvToVar
 功能描述  : 将SIP advanced配置数据存入共享目录,供SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeSipAdvToVar(PSIP_WEB_ADV_CFG_S pstSipAdv)
{
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 

    sprintf(szAttrName, "%s/srvexpire", SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulExpiration);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    /* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
    sprintf(szAttrName, "%s/srvregperiod", SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulRegPeriod);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    /* END:   Added by y67514, 2009/11/4 */
    
    sprintf(szAttrName, "%s/srvrereg",  SIP_CFGDIR_SIP_ADVANCED); 
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulReRegister);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/srvseexpire",  SIP_CFGDIR_SIP_ADVANCED);  
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.SessionExpires);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/minse",  SIP_CFGDIR_SIP_ADVANCED);       
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.MinSE);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	/* BEGIN: Modified by p00102297, 2008/1/29 */
	sprintf(szAttrName, "%s/maxretrytime",  SIP_CFGDIR_SIP_ADVANCED);       
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulMaxRetryTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	sprintf(szAttrName, "%s/primarychecktime",  SIP_CFGDIR_SIP_ADVANCED);       
    sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulPrimaryCheckTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/* END:   Modified by p00102297, 2008/1/29 */

	/* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
	sprintf(szAttrName, "%s/mwiexpire",	SIP_CFGDIR_SIP_ADVANCED);		
	sprintf(szAttrVal, "%d", pstSipAdv->stServerCfg.ulMWIExpire);
	(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    /* END:   Added by chenzhigang, 2008/11/5 */
	
    sprintf(szAttrName, "%s/rtpstartport",  SIP_CFGDIR_SIP_ADVANCED);      
    sprintf(szAttrVal, "%d",  pstSipAdv->stRTPCfg.ulStartPort);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	/* BEGIN: Modified by p00102297, 2008/1/18 */
	sprintf(szAttrName, "%s/rtptxrepeatinterval",	SIP_CFGDIR_SIP_ADVANCED);	   
	sprintf(szAttrVal, "%d",  pstSipAdv->stRTPCfg.ulTxRepeatInterval);
	(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/* END:   Modified by p00102297, 2008/1/18 */

	/*start of modification by chenyong 2009-03-05*/
	sprintf(szAttrName, "%s/rtcpmonitor",	SIP_CFGDIR_SIP_ADVANCED);	   
	sprintf(szAttrVal, "%d",  pstSipAdv->stRTPCfg.ulRtcpMonitor);
	(void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    /*end of modification by chenyong 2009-03-05*/

    sprintf(szAttrName, "%s/rtpendport",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stRTPCfg.ulEndPort);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    /* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
    sprintf(szAttrName, "%s/voicecodec1",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulPrimary);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/voicecodec2",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulSecondary);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/voicecodec3",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulThird);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/* BEGIN: Modified by p00102297, 2008/1/17 */
	sprintf(szAttrName, "%s/voiceg711aptime",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulG711APTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	sprintf(szAttrName, "%s/voiceg711uptime",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulG711UPTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	sprintf(szAttrName, "%s/voiceg729ptime",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulG729PTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/* END:   Modified by p00102297, 2008/1/17 */

    sprintf(szAttrName, "%s/voicedtmftype",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->stVoiceCfg.ulDtmfType);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/faxtype",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->ulFaxType);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
#endif
    /* END:	 Modified by p00102297, 2008/2/20 */

    sprintf(szAttrName, "%s/sipifcname",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%s", pstSipAdv->szIfName);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/advsiplocalport",  SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->ulSipLocalPort);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/pstncallprefix",SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%s", pstSipAdv->szPstnPrefix);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/siptestenable",SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->ulEnblSipTest);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);    

/* BEGIN: Added by s00125931, 2008/9/16   问题单号:vhg556*/
    sprintf(szAttrName, "%s/dscpmark", SIP_CFGDIR_SIP_ADVANCED);
    sprintf(szAttrVal, "%d", pstSipAdv->ulDscp);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);    
/* END:   Added by s00125931, 2008/9/16 */
#if 0
    //测试参数读取 
    {
    SIP_WEB_ADV_CFG_S tmp;
    memset(&tmp, 0, sizeof(SIP_WEB_ADV_CFG_S));
    VOS_UINT32 VoiceGetSipAdvFromVar(SIP_WEB_ADV_CFG_S *pstSipAdv);
    if(VOS_OK == VoiceGetSipAdvFromVar(&tmp))
    {
        printf("SIP Advanced: srvexpire %d, srvrereg %d, srvseexpire %d, minse %d, rtpstartport %d, rtpendport %d voicecodec1 %d voicecodec2 %d voicecodec3 %d voicedtmftype %d faxtype %d", 
            tmp.stServerCfg.ulExpiration,
            tmp.stServerCfg.ulReRegister,
            tmp.stServerCfg.SessionExpires,
            tmp.stServerCfg.MinSE,
            tmp.stRTPCfg.ulStartPort,
            tmp.stRTPCfg.ulEndPort,
            tmp.stVoiceCfg.ulPrimary,
            tmp.stVoiceCfg.ulSecondary,
            tmp.stVoiceCfg.ulThird,
            tmp.stVoiceCfg.ulDtmfType,
            tmp.ulFaxType);
    }
    else
    {
        printf("SIP Advanced: for test read sip Advanced fail");
    }
        
    }
    //for test end 
#endif
}

/*****************************************************************************
 函 数 名  : HWDb_writeSipQosToVar
 功能描述  : 将SIP qos配置数据存入共享目录,供SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeSipQosToVar(PSIP_WEB_QOS_CFG_S pstSipQos)
{
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 

    sprintf(szAttrName, "%s/sipqostos", SIP_CFGDIR_SIP_QOS);
    sprintf(szAttrVal, "%d", pstSipQos->ulTos);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/sipqosenblvlan",  SIP_CFGDIR_SIP_QOS); 
    sprintf(szAttrVal, "%d", pstSipQos->ulEnblVlan);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/sipqosvlanid",  SIP_CFGDIR_SIP_QOS);
    sprintf(szAttrVal, "%d", pstSipQos->ulVlan);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/sipqospriority",  SIP_CFGDIR_SIP_QOS);
    sprintf(szAttrVal, "%d", pstSipQos->ulPriority);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

#if 0
    //测试参数读取 
    {
    SIP_WEB_QOS_CFG_S tmp;
    memset(&tmp, 0, sizeof(SIP_WEB_QOS_CFG_S));
    VOS_UINT32 VoiceGetSipQosFromVar(SIP_WEB_QOS_CFG_S *pstSipQos);
    if(VOS_OK == VoiceGetSipQosFromVar(&tmp))
    {
        printf("SIP Qos: tos %d, vlanenbl %d, vlanid %d,
            tmp.ulTos,
            tmp.ulEnblVlan,
            tmp.ulVlan);
    }
    else
    {
        printf("SIP qos: for test read sip qos fail");
    }
        
    }
    //for test end 
#endif
}

/*****************************************************************************
 函 数 名  : HWDb_writePhoneCommonToVar
 功能描述  : 将SIP Phone基本配置数据存入共享目录,供SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writePhoneCommonToVar()
{
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 

    sprintf(szAttrName, "%s/speek", SIP_CFGDIR_PHONE_COMMON);
    sprintf(szAttrVal, "%d", g_phoneCfg.lSpkVol);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/listen",  SIP_CFGDIR_PHONE_COMMON); 
    sprintf(szAttrVal, "%d", g_phoneCfg.lLstVol);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/interval",  SIP_CFGDIR_PHONE_COMMON);  
    sprintf(szAttrVal, "%d", g_phoneCfg.ulDialInterval);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	/* BEGIN: Modified by p00102297, 2008/1/29 */
	sprintf(szAttrName, "%s/fstdialtime",  SIP_CFGDIR_PHONE_COMMON);  
    sprintf(szAttrVal, "%d", g_phoneCfg.ulFstDialTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/* END:   Modified by p00102297, 2008/1/29 */
    //add by z67625 for automatic call
    sprintf(szAttrName, "%s/autocallenbl",  SIP_CFGDIR_PHONE_COMMON);  
    sprintf(szAttrVal, "%d", g_phoneCfg.lAutoEnbl);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    
    sprintf(szAttrName, "%s/autocallinterval",  SIP_CFGDIR_PHONE_COMMON);  
    sprintf(szAttrVal, "%d", g_phoneCfg.ulAutoInterval);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/autocallnum",  SIP_CFGDIR_PHONE_COMMON);  
    sprintf(szAttrVal, "%s", g_phoneCfg.acAutoDialNum);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	/* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
    sprintf(szAttrName, "%s/vad",  SIP_CFGDIR_PHONE_COMMON);       
    sprintf(szAttrVal, "%d", g_phoneCfg.ulVad);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
#endif    
    sprintf(szAttrName, "%s/region",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulRegion);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    /* start of y42304 added 20070108: 处理拍叉上下限 */
    sprintf(szAttrName, "%s/maxHookFlash",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.maxHookFlash);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    sprintf(szAttrName, "%s/minHookFlash",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.minHookFlash);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    /* end of y42304 added 20070108 */    
    /* END:   Modified by p00102297, 2008/2/20 */

    sprintf(szAttrName, "%s/digitmap",  SIP_CFGDIR_PHONE_COMMON);
    sprintf(szAttrVal, "%s", g_phoneCfg.szDigitMap);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

    /* BEGIN: Modified by p00102297, 2008/2/21 */
#if 0
	sprintf(szAttrName, "%s/ec",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulEC);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	sprintf(szAttrName, "%s/cng",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulCNG);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
#endif
	sprintf(szAttrName, "%s/offhooktime",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulOffHookTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

	sprintf(szAttrName, "%s/onhooktime",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulOnHookTime);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);

/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	sprintf(szAttrName, "%s/reinjection",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulReInjection);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);	
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
	/*start of additon by chenyong 2008-10-28 for Inner call*/
	sprintf(szAttrName, "%s/innercall",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulInnerCall);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);	
	/*end of additon by chenyong 2008-10-28 for Inner call*/
	sprintf(szAttrName, "%s/modemservice",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.ulModemService);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);	
	/*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
	sprintf(szAttrName, "%s/VoicePromptVisible",  SIP_CFGDIR_PHONE_COMMON);      
    sprintf(szAttrVal, "%d",  g_phoneCfg.bVoicePromptVisible);
    (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
	/*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
#if 0
    //测试参数读取 
    {
    SIP_WEB_PHONE_CFG_S tmp;
    memset(&tmp, 0, sizeof(SIP_WEB_PHONE_CFG_S));
    VOS_UINT32 VoiceGetPhoneCommonFromVar(SIP_WEB_PHONE_CFG_S *pPhoneCfg);
    if(VOS_OK == VoiceGetPhoneCommonFromVar(&tmp))
    {
        printf("SIP : spk %d, lst %d, interval %d, vad %d, region %d, digitmap %s", 
            tmp.lSpkVol,
            tmp.lLstVol,
            tmp.ulDialInterval,
            tmp.ulVad,
            tmp.ulRegion,
            tmp.szDigitMap);
    }
    else
    {
        printf("SIP: for test read phonecfg fail");
    }
        
    }
    //for test end 
#endif
}
/*****************************************************************************
 函 数 名  : HWDb_writeSpeedDialToVar
 功能描述  : 将快速拨号配置数据写入到共享目录,以便SIP业务模块在启动后可以使
             用这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeSpeedDialToVar()
{
    int i;
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 
    
    for (i = 0; i < SIP_MAX_DIAL_PLAN; i++)
    {
        sprintf(szAttrName, "%s/%d/number", SIP_CFGDIR_SPEED_DIAL, i);
        sprintf(szAttrVal, "%s", g_dialPlan[i].szDialNum);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);        

        sprintf(szAttrName, "%s/%d/name", SIP_CFGDIR_SPEED_DIAL, i);
        sprintf(szAttrVal, "%s", g_dialPlan[i].szName);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);        

        sprintf(szAttrName, "%s/%d/destination", SIP_CFGDIR_SPEED_DIAL, i);
        sprintf(szAttrVal, "%s", g_dialPlan[i].szRealNum);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    } 
}
/*****************************************************************************
 函 数 名  : HWDb_writeCallForwardToVar
 功能描述  : 将呼叫前转表配置信息写入到共享目录以便SIP业务模块在启动后可以获
             得这些数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
void HWDb_writeCallForwardToVar()
{
    int i, j;
    char szAttrName[SIP_GIANT_LEN * 2] = {0}; 
    char szAttrVal[SIP_GIANT_LEN * 2] = {0}; 

    for (i = 0; i < SIP_MAX_CALL_FWD; i++)
    {
        for(j = 0; j < SIP_WEB_FWD_BASIC_NUM; j++)
        {
            sprintf(szAttrName, "%s/%d/basic/%d/active", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%d", g_callFWD[i].pstBasic[j].bIsActive);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);            

            sprintf(szAttrName, "%s/%d/basic/%d/forward", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%s", g_callFWD[i].pstBasic[j].szForward);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);    
        }
    
        for(j = 0; j < SIP_WEB_FWD_ADVANCED_NUM; j++)
        {           
            sprintf(szAttrName, "%s/%d/advance/%d/active", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%d", g_callFWD[i].pstAdvance[j].bIsActive);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);            

            sprintf(szAttrName, "%s/%d/advance/%d/incoming", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%s", g_callFWD[i].pstAdvance[j].szIncoming);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);            

            sprintf(szAttrName, "%s/%d/advance/%d/forward", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%s", g_callFWD[i].pstAdvance[j].szForward);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);            

            sprintf(szAttrName, "%s/%d/advance/%d/condition", SIP_CFGDIR_CALL_FORWARD, i, j);
            sprintf(szAttrVal, "%d", g_callFWD[i].pstAdvance[j].enFwdType);
            (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);  
        }    
        
        sprintf(szAttrName, "%s/%d/nawt", SIP_CFGDIR_CALL_FORWARD, i);
        sprintf(szAttrVal, "%d", g_callFWD[i].ulNoReplyTime);
        (void)appendParamToVar(SIP_CFG_FILE, szAttrName, szAttrVal);
    }
}

//以下代码需要放到sip业务的初始化中去
/*****************************************************************************
 函 数 名  : VoiceGetParamFromVar
 功能描述  : 从共享文件中读取SIP业务配置数据
 输入参数  : VOS_CHAR *pDir     共享文件的位置
             VOS_CHAR *pName    要读取的参数名
             VOS_CHAR *pVal     函数成功执行后存储参数值
             VOS_INT32 lBufLen  存储参数值的缓冲区长度
 输出参数  : 无
 返 回 值  : static VOS_UINT32
             成功返回 VOS_OK
             失败返回 VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
static VOS_UINT32 VoiceGetParamFromVar(VOS_CHAR *pDir, VOS_CHAR *pName, VOS_CHAR *pVal, VOS_INT32 lBufLen)
{
    FILE *fs;
    char file[SIP_SMALL_LEN] = {0}; 
    char line[SIP_GIANT_LEN * 2] = {0}; 
    char *pNameTmp = NULL;
    char *pValTmp = NULL;
    
    if((VOS_NULL == pDir) || (VOS_NULL == pName) || (VOS_NULL == pVal) || (0 == lBufLen))
    {
        return VOS_ERR;
    }

    sprintf(file,"%s", pDir);
    
    if ( (access(file, F_OK)) != 0 )
    {
       return VOS_ERR;
    }
       
    fs = fopen(file, "r");

    if ( fs != NULL ) {
        while(NULL != fgets((char*)line, (int)lBufLen, fs))
        {
            pNameTmp = line;
            if(NULL != (pValTmp = strchr(line, '=')))
            {
                *pValTmp = 0;
                pValTmp ++;
                if(strcasecmp(pNameTmp, pName) == 0)
                {
                    strcpy(pVal, pValTmp);
                    if(NULL != (pValTmp = strchr(pVal, '\r')))
                    {
                        *pValTmp = '\0';
                    }
                    if(NULL != (pValTmp = strchr(pVal, '\n')))
                    {
                        *pValTmp = '\0';
                    }
                    break;
                }
            }
        }
        fclose(fs);
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}

/*****************************************************************************
 函 数 名  : VoiceGetSipSrvFromVar
 功能描述  : 从共享目录文件中读取快速拨号配置数据供SIP业务模块使用
 输入参数  : VOS_UINT32 ulIndex              sip server条目索引，系统目前共支持2条
             SIP_WEB_SERVER_S *pstSipSrv     返回sip server条目的数据
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetSipSrvFromVar(VOS_UINT32 ulIndex, SIP_WEB_SERVER_S *pstSipSrv)
{
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    VOS_CHAR szVal[SIP_MAX_NUM_LEN] = {0};
    
    if(ulIndex >= SIP_SRV_MAX)
    {
        return VOS_ERR;
    }

    if(VOS_NULL == pstSipSrv)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/regaddr", SIP_CFGDIR_SIP_SERVER, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipSrv->stSipRegAddr.szIPAddr, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/regport", SIP_CFGDIR_SIP_SERVER, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
       pstSipSrv->stSipRegAddr.uwPort = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/prxyaddr", SIP_CFGDIR_SIP_SERVER, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipSrv->stSipProxyAddr.szIPAddr, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/prxyport", SIP_CFGDIR_SIP_SERVER, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipSrv->stSipProxyAddr.uwPort =  atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    sprintf(szName, "%s/%d/domain", SIP_CFGDIR_SIP_SERVER, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipSrv->acDomainName, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : VoiceGetSipBscFromVar
 功能描述  : 从共享目录文件中读取sip basic配置数据供SIP业务模块使用
 输入参数  : VOS_UINT32 ulIndex            sip basic目索引，系统目前共支持2个sip 帐号
             SIP_WEB_USER_INFO_S *pstSipBasic  sip basic条目的数据
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetSipBscFromVar(VOS_UINT32 ulIndex, SIP_WEB_USER_INFO_S *pstSipBasic)
{
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    VOS_CHAR szVal[SIP_MAX_NUM_LEN] = {0};
    
    if(ulIndex >= SIP_USER_MAX)
    {
        return VOS_ERR;
    }

    if(VOS_NULL == pstSipBasic)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/sipid", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipBasic->szId, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/sipusrname", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipBasic->szName, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/sipusrpwd", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipBasic->szPsw, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/siplocalport", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->uwPort = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    sprintf(szName, "%s/%d/incallfwtblid", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->ulFwdTbl = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
    sprintf(szName, "%s/%d/sipclipenable", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usCliStatus = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    /* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
	/* BEGIN: Added by lihua 68693, 2007/10/20   问题单号:A36D03793*/
#if 0
    sprintf(szName, "%s/%d/sipcliprivacytype", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usCliPrivacyType = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
#endif
    /* END:   Added by lihua 68693, 2007/10/20 */
    /* END:   Modified by p00102297, 2008/3/11 VDF C02*/

	/* BEGIN: Modified by p00102297, 2008/2/3 */
	sprintf(szName, "%s/%d/siplineenable", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineEnable = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    /*start of addition by chenyong 2009-03-19*/
	sprintf(szName, "%s/%d/siplinecallhold", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineCallHoldEnable = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    /*end of addition by chenyong 2009-03-19*/

	sprintf(szName, "%s/%d/siplinecallwait", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineCWEnable = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	sprintf(szName, "%s/%d/sipline3way", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLine3WayEnable = atoi(szVal);
		
    }
    else
    {
        return VOS_ERR;
    }
	sprintf(szName, "%s/%d/siplinecalltransfer", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineCallTransferEnable = atoi(szVal);

    }
    else
    {
        return VOS_ERR;
    }
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
	/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
	sprintf(szName, "%s/%d/siplinemwi", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineMWIEnable = atoi(szVal);		
    }
    else
    {
        return VOS_ERR;
    }
	sprintf(szName, "%s/%d/siplinemwiaccount", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pstSipBasic->szLineMWIAccount, szVal);
    }
    else
    {
        return VOS_ERR;
    }	
	/* END:   Added by chenzhigang, 2008/11/4 */
	sprintf(szName, "%s/%d/siplinefaxdetect", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->usLineFaxDetect = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/vad", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->ulVad = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	
	sprintf(szName, "%s/%d/ec", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->ulEC = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/cng", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->ulCNG = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
	sprintf(szName, "%s/%d/sipBscVoiceCapCodec1", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[0].ulCodecCap = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapCodec2", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[1].ulCodecCap = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	
	sprintf(szName, "%s/%d/sipBscVoiceCapCodec3", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[2].ulCodecCap = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapCodec4", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[3].ulCodecCap = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapCodec5", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[4].ulCodecCap = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPTime1", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[0].ulPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPTime2", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[1].ulPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPTime3", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[2].ulPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPTime4", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[3].ulPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPTime5", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[4].ulPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPriority1", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[0].ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPriority2", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[1].ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPriority3", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[2].ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPriority4", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[3].ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapPriority5", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[4].ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapIfEnable1", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[0].bEnable = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapIfEnable2", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[1].bEnable = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapIfEnable3", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[2].bEnable = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapIfEnable4", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[3].bEnable = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/%d/sipBscVoiceCapIfEnable5", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.stCodecList[4].bEnable = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
	sprintf(szName, "%s/%d/voicedtmftype", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.ulDtmfType = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	//BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
	sprintf(szName, "%s/%d/voicedpayloadype", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->stVoiceCfg.ulPayloadType = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	//END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069

	sprintf(szName, "%s/%d/faxtype", SIP_CFGDIR_SIP_BASIC, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipBasic->ulFaxType = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/2/3 */
	
    {
    	int j;
    	for (j=0;j<SIP_MAX_DAILNUM_CHANGE_NUM;j++)
    	{
		    sprintf(szName, "%s/%d/sipSpNum%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stStripPrefixNum[j].szAsideNum, szVal);
		    #if 0
				strcpy((char*)pstSipBasic->stStripNum[j].szPrefixNum, szVal);
			#endif
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipSpName%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stStripPrefixNum[j].szMatchNum, szVal);
		    #if 0
				strcpy((char*)pstSipBasic->stStripNum[j].szMatchNum, szVal);
			#endif
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipApNum%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stAppandPrefixNum[j].szAsideNum, szVal);
		    #if 0
				strcpy((char*)pstSipBasic->stAppNum[j].szPrefixNum, szVal);
			#endif
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipApName%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stAppandPrefixNum[j].szMatchNum, szVal);
		    #if 0
				strcpy((char*)pstSipBasic->stAppNum[j].szMatchNum, szVal);
			#endif
		    }
		    else
		    {
		        return VOS_ERR;
		    }
			/* BEGIN: Modified by p00102297, 2008/1/30 */
			sprintf(szName, "%s/%d/sipSsNum%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stStripSuffixNum[j].szAsideNum, szVal);
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipSsName%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stStripSuffixNum[j].szMatchNum, szVal);
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipAsNum%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stAppandSuffixNum[j].szAsideNum, szVal);
		    }
		    else
		    {
		        return VOS_ERR;
		    }

		    sprintf(szName, "%s/%d/sipAsName%02d", SIP_CFGDIR_SIP_BASIC, ulIndex,j);
		    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
		    {
			    strcpy((char*)pstSipBasic->stAppandSuffixNum[j].szMatchNum, szVal);
		    }
			/* END:   Modified by p00102297, 2008/1/30 */
    	}
    }


#endif  
    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetSipAdvFromVar
 功能描述  : 从共享文件中读取SIP Advanced配置数据供SIP业务模块使用
 输入参数  : SIP_WEB_ADV_CFG_S *pstSipAdv  
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetSipAdvFromVar(SIP_WEB_ADV_CFG_S *pstSipAdv)
{
    VOS_CHAR szVal[SIP_MAX_DIGIT_MAP] = {0};
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    
    if(VOS_NULL == pstSipAdv)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/srvexpire", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulExpiration = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    /* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
    sprintf(szName, "%s/srvregperiod", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulRegPeriod= atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    /* END:   Added by y67514, 2009/11/4 */

    sprintf(szName, "%s/srvrereg", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulReRegister = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/srvseexpire", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.SessionExpires = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/minse", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.MinSE = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/* BEGIN: Modified by p00102297, 2008/1/29 */
	sprintf(szName, "%s/maxretrytime", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulMaxRetryTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/primarychecktime", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulPrimaryCheckTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/1/29 */
	/* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
	sprintf(szName, "%s/mwiexpire", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stServerCfg.ulMWIExpire = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }	
    /* END:   Added by chenzhigang, 2008/11/5 */
	
    sprintf(szName, "%s/rtpstartport", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stRTPCfg.ulStartPort = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/rtpendport", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stRTPCfg.ulEndPort = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/* BEGIN: Modified by p00102297, 2008/1/18 */
	sprintf(szName, "%s/rtptxrepeatinterval", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stRTPCfg.ulTxRepeatInterval = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/1/18 */

	/*start of modification by chenyong 2009-03-05*/
	sprintf(szName, "%s/rtcpmonitor", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stRTPCfg.ulRtcpMonitor = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    /*end of modification by chenyong 2009-03-05*/
	
	/* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
    sprintf(szName, "%s/voicecodec1", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulPrimary = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/voicecodec2", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulSecondary = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/voicecodec3", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulThird = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/* BEGIN: Modified by p00102297, 2008/1/17 */
	sprintf(szName, "%s/voiceg711aptime", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulG711APTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/voiceg711uptime", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulG711UPTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/voiceg729ptime", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulG729PTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/1/17 */

    sprintf(szName, "%s/voicedtmftype", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->stVoiceCfg.ulDtmfType = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/faxtype", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->ulFaxType = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
#endif
	/* END:   Modified by p00102297, 2008/2/20 */

    sprintf(szName, "%s/sipifcname", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strncpy(pstSipAdv->szIfName, szVal, SIP_MAX_IFNAME_STR_LEN - 1);
        pstSipAdv->szIfName[SIP_MAX_IFNAME_STR_LEN - 1] = '\0';
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/advsiplocalport", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->ulSipLocalPort = (VOS_UINT32)atoi(szVal);;
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/pstncallprefix", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
        szVal[PSTN_MAX_PREFIX_LEN-1] = '\0';
        memcpy(pstSipAdv->szPstnPrefix,szVal,strlen(szVal));
    }    
    else
    {
        return VOS_ERR;
    }    
/* BEGIN: Added by s00125931, 2008/9/16   问题单号:vhg556*/
    sprintf(szName, "%s/dscpmark", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->ulDscp = (VOS_UINT32)atoi(szVal);
    }    
    else
    {
        return VOS_ERR;
    }    
/* END:   Added by s00125931, 2008/9/16 */
    sprintf(szName, "%s/siptestenable", SIP_CFGDIR_SIP_ADVANCED);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipAdv->ulEnblSipTest = (VOS_UINT32)atoi(szVal);;
    }    
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetSipQosFromVar
 功能描述  : 从共享文件中读取SIP qos配置数据供SIP业务模块使用
 输入参数  : SIP_WEB_QOS_CFG_S *pstSipQos  
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : z45221
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetSipQosFromVar(SIP_WEB_QOS_CFG_S *pstSipQos)
{
    VOS_CHAR szVal[SIP_MAX_DIGIT_MAP] = {0};
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    
    if(VOS_NULL == pstSipQos)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/sipqostos", SIP_CFGDIR_SIP_QOS);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipQos->ulTos = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/sipqosenblvlan", SIP_CFGDIR_SIP_QOS);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipQos->ulEnblVlan = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/sipqosvlanid", SIP_CFGDIR_SIP_QOS);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipQos->ulVlan = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/sipqospriority", SIP_CFGDIR_SIP_QOS);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pstSipQos->ulPriority = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetPhoneCommonFromVar
 功能描述  : 从共享文件中读取SIP Phone基本配置数据供SIP业务模块使用
 输入参数  : SIP_WEB_PHONE_CFG_S *pPhoneCfg  
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetPhoneCommonFromVar(SIP_WEB_PHONE_CFG_S *pPhoneCfg)
{
    VOS_CHAR szVal[SIP_MAX_DIGIT_MAP] = {0};
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    
    if(VOS_NULL == pPhoneCfg)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/speek", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->lSpkVol = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/listen", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->lLstVol = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/interval", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulDialInterval = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/* BEGIN: Modified by p00102297, 2008/1/29 */
	sprintf(szName, "%s/fstdialtime", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulFstDialTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/1/29 */

    //add by z67625 for automatic call
    sprintf(szName, "%s/autocallenbl", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->lAutoEnbl = atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
    sprintf(szName, "%s/autocallinterval", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulAutoInterval = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/autocallnum", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pPhoneCfg->acAutoDialNum, szVal);
         printf("%s,%d, get autodialnum %s\n", __FILE__,__LINE__, pPhoneCfg->acAutoDialNum);
    }
    else
    {
        return VOS_ERR;
    }

    /* BEGIN: Modified by p00102297, 2008/2/20 */
#if 0
    sprintf(szName, "%s/vad", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulVad = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
#endif
    sprintf(szName, "%s/region", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulRegion = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    /* start of y42304 added 20070108: 处理拍叉上下限 */
    sprintf(szName, "%s/minHookFlash", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->minHookFlash= (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/maxHookFlash", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->maxHookFlash = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    /* end of y42304 added 20070108 */
	/* END:   Modified by p00102297, 2008/2/20 */

    sprintf(szName, "%s/digitmap", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pPhoneCfg->szDigitMap, szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/* BEGIN: Modified by p00102297, 2008/1/17 */
#if 0 
	sprintf(szName, "%s/ec", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulEC = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/cng", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulCNG = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
#endif
	sprintf(szName, "%s/offhooktime", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulOffHookTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	sprintf(szName, "%s/onhooktime", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulOnHookTime = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }
	/* END:   Modified by p00102297, 2008/1/17 */
	/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	sprintf(szName, "%s/reinjection", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulReInjection = (VOS_UINT32)atoi(szVal);
    }
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
	/*start of additon by chenyong 2008-10-28 for Inner call*/
	sprintf(szName, "%s/innercall", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulInnerCall = (VOS_UINT32)atoi(szVal);
    }
	/*end of additon by chenyong 2008-10-28 for Inner call*/
	sprintf(szName, "%s/modemservice", SIP_CFGDIR_PHONE_COMMON);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pPhoneCfg->ulModemService = (VOS_UINT32)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

	/*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
	sprintf(szName, "%s/VoicePromptVisible", SIP_CFGDIR_PHONE_COMMON);
	if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
	{
		pPhoneCfg->bVoicePromptVisible = (VOS_UINT32)atoi(szVal);
	}
	else
	{
		return VOS_ERR;
	}
	/*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/		

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetSpeedDialFromVar
 功能描述  : 从共享目录文件中读取快速拨号配置数据供SIP业务模块使用
 输入参数  : VOS_UINT32 ulIndex              快速拨号条目索引，系统目前共支持10条
             SIP_WEB_DIAL_PLAN_S *pDialPlan  返回拨号条目的数据
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回  VOS_OK
             失败返回  VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetSpeedDialFromVar(VOS_UINT32 ulIndex, SIP_WEB_DIAL_PLAN_S *pDialPlan)
{
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    VOS_CHAR szVal[SIP_MAX_NUM_LEN] = {0};
    
    if(ulIndex >= SIP_MAX_DIAL_PLAN)
    {
        return VOS_ERR;
    }

    if(VOS_NULL == pDialPlan)
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/number", SIP_CFGDIR_SPEED_DIAL, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pDialPlan->szDialNum, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/name", SIP_CFGDIR_SPEED_DIAL, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pDialPlan->szName, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    sprintf(szName, "%s/%d/destination", SIP_CFGDIR_SPEED_DIAL, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        strcpy((char*)pDialPlan->szRealNum, szVal);
    }
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetCallForwardFromVar
 功能描述  : 从共享文件中读取呼叫前转表配置信息供SIP业务模块使用
 输入参数  : VOS_UINT32 ulIndex       呼叫前转表索引，目前共支持2张表
             SIP_WEB_FWD_S *pCallFWD  返回的呼叫前转表信息
 输出参数  : 无
 返 回 值  : VOS_UINT32
             成功返回 VOS_OK
             失败返回 VOS_ERR
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2006年11月11日
    作    者   : s00000658
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetCallForwardFromVar(VOS_UINT32 ulIndex, SIP_WEB_FWD_S *pCallFWD)
{
    VOS_CHAR szName[SIP_LARGE_LEN] = {0};
    VOS_CHAR szVal[SIP_MAX_NUM_LEN] = {0};
    VOS_UINT32 i;

    if(ulIndex >= SIP_MAX_CALL_FWD)
    {
        return VOS_ERR;
    }

    if(VOS_NULL == pCallFWD)
    {
        return VOS_ERR;
    }

    for(i = 0; i < SIP_WEB_FWD_BASIC_NUM; i++)
    {
        sprintf(szName, "%s/%d/basic/%d/active", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            pCallFWD->pstBasic[i].bIsActive = atoi(szVal);
        }
        else
        {
            return VOS_ERR;
        }

        sprintf(szName, "%s/%d/basic/%d/forward", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            strcpy((char*)pCallFWD->pstBasic[i].szForward, szVal);
        }
        else
        {
            return VOS_ERR;
        }

        switch(i)
        {
        case 0:
            pCallFWD->pstBasic[i].enFwdType = SIP_WEB_UNCON_FWD;
            break;
        case 1:
            pCallFWD->pstBasic[i].enFwdType = SIP_WEB_ONBSY_FWD;
            break;
        case 2:
            pCallFWD->pstBasic[i].enFwdType = SIP_WEB_NOREP_FWD;
            break;
        default:;
            return VOS_ERR;
        }
    }
    
    for(i = 0; i < SIP_WEB_FWD_BASIC_NUM; i++)
    {
        sprintf(szName, "%s/%d/advance/%d/active", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            pCallFWD->pstAdvance[i].bIsActive = atoi(szVal);
        }
        else
        {
            return VOS_ERR;
        }

        sprintf(szName, "%s/%d/advance/%d/incoming", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            strcpy((char*)pCallFWD->pstAdvance[i].szIncoming, szVal);
        }
        else
        {
            return VOS_ERR;
        }

        sprintf(szName, "%s/%d/advance/%d/forward", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            strcpy((char*)pCallFWD->pstAdvance[i].szForward, szVal);
        }
        else
        {
            return VOS_ERR;
        }

        sprintf(szName, "%s/%d/advance/%d/condition", SIP_CFGDIR_CALL_FORWARD, ulIndex, i);
        if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
        {
            pCallFWD->pstAdvance[i].enFwdType = (SIP_WEB_FWD_TYPE_E)atoi(szVal);
        }
        else
        {
            return VOS_ERR;
        }        
    }

    sprintf(szName, "%s/%d/nawt", SIP_CFGDIR_CALL_FORWARD, ulIndex);
    if(VOS_OK == VoiceGetParamFromVar(SIP_CFG_FILE, szName, szVal, sizeof(szVal)+strlen(szName)+strlen("=\n")))
    {
        pCallFWD->ulNoReplyTime = (VOS_UINT8)atoi(szVal);
    }
    else
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

#if 0
/*****************************************************************************
 函 数 名  : VoiceSetFxsStatusInfoToVar
 功能描述  : 设置FXS口的状态信息到共享文件：0表示挂机，1表示摘机。
 输入参数  : VOS_UINT32 ulIndex
             VOS_UINT32 * pStatus
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月10日
    作    者   : p00102297
    修改内容   : 新生成函数

*****************************************************************************/
void VoiceSetFxsStatusInfoToVar( VOS_UINT32 ulIndex, VOS_UINT32 * pStatus )
{
    char szAttrName[SIP_SMALL_LEN] = {0}; 
    char szAttrVal[SIP_SMALL_LEN] = {0};
	FILE *filep = NULL;

	if( VOS_NULL == pStatus )
    {
	    printf("Null Input Status Value!\n");
        return;
    }

	if ( ulIndex < 0 || ulIndex > SIP_USER_MAX )
	{
	    printf("Invalid FXS Port Index!\n");
		return;
	}
	else
	{
	    sprintf(szAttrName, "fxs%d=",  ulIndex);
	}
	
	if ( *pStatus > VOICE_FXS_STAT_START && *pStatus < VOICE_FXS_STAT_BUTT )
	{
	    sprintf(szAttrVal, "%d", *pStatus);
	}
	else
	{
	    printf("Invalid FXS Status Value!\n");
		return;
	}

	if( NULL == (filep = fopen(SIP_FXS_STATUS_FILE, "w")) )
    {
	    printf("Fail to Open the FXS-Status File!\n");
        return;
    }
    
    if(VOS_OK != SipLockFile(filep, F_WRLCK))
    {
        fclose(filep);
		printf("Fail to Lock the FXS-Status File!\n");
        return;
    }

	fprintf(filep, "%s=%s\n", szAttrName, szAttrVal);

	SipLockFile(filep, F_UNLCK);
    fclose(filep);

	printf("Function: %s, File: %s, Line: %d: Successful!\n", __FUNCTION__, __FILE__, __LINE__);
	return;
    //(void)appendParamToVar(SIP_FXS_STATUS_FILE, szAttrName, szAttrVal);
}
#endif

/*START ADD: liujianfeng 37298 for [A36D03372] at 2007-02-2 */
VOS_UINT32 SipLockFile(VOS_INT fd,VOS_INT mode)
{
    struct flock lock;

    lock.l_type = mode;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if( 0 != fcntl(fd,F_SETLKW,&lock))
    {
        printf( "Get file $s read lock fail");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : VoiceGetFxsStatusFromVar
 功能描述  : 获取S口的状态（摘机/挂机），0表示该S口正处于挂机状态，1表示该S
             口正处于摘机状态。
 输入参数  : VOS_UINT32 ulIndex
             VOS_UINT * pStatus
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月10日
    作    者   : p00102297
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 VoiceGetFxsStatusFromVar( VOS_UINT32 ulUeSn, VOS_UINT32 * pStatus )
{

#define  FXS_STATUS_LEN  1
    VOS_CHAR szLine[SIP_GIANT_LEN] = {0};
    VOS_CHAR szVarName[SIP_SMALL_LEN] = {0};
    VOS_CHAR* pPosition = NULL;
	VOS_INT filep;
    VOS_INT iReadByte;
	FILE *pFile = NULL;

    if(VOS_NULL == pStatus)
    {
        return VOS_ERR;
    }

	if ( (access(SIP_FXS_STATUS_FILE, F_OK)) != 0 )
    {
       return VOS_ERR;
    }

    /*START ADD: liujianfeng 37298 for [A36D03258] at 2007-01-16 */
    *pStatus = VOICE_FXS_STAT_OFFHOOK;    
    /*END ADD: liujianfeng 37298 for [A36D03258] at 2007-01-16 */

	if( (pFile = fopen(SIP_FXS_STATUS_FILE, "r")) < 0)
    {
        printf("Function: %s Line: %d File: %s Fail to open the FXS Status File!\n", __FUNCTION__, __LINE__, __FILE__);
        return VOS_ERR;
    }

    filep = fileno(pFile);
	
    if(VOS_OK != SipLockFile(filep, F_RDLCK))
    {
        fclose(pFile);
        return VOS_ERR;
    }

    sprintf(szVarName,"fxs%d=",ulUeSn);

	while( NULL != fgets(szLine,strlen(szVarName)+ FXS_STATUS_LEN + 1, pFile) )
    {
	    iReadByte = strlen(szLine);
        if(iReadByte != strlen(szVarName) + FXS_STATUS_LEN)
        {
            printf("\nREAD:%s error\n",szLine);
            pPosition = NULL;
            break;
        }
        
        szLine[iReadByte]=0;    
        if( NULL != (pPosition= strstr(szLine,szVarName)) )
        {
            if( pPosition != szLine)
            {
                printf("\nposition is not eqr %d != %d\n",pPosition,szLine);
                pPosition = NULL;
                break;
            }
            /*只支持状态码是一位的情况*/
            *pStatus = *(pPosition+strlen(szVarName))-'0';
            break;
        }
        fread(szLine, sizeof(char), 1, pFile);
    }

    SipLockFile(filep, F_UNLCK);
    fclose(pFile);

    if( NULL == pPosition)
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
    return VOS_ERR;
}


/*****************************************************************************
 Prototype    : SipGetServiceStatus
 Description  : 读取业务整体状态
 Input        : VOS_UINT32 ulUeSn      
                VOS_UINT32 *pulStatus  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2006/12/13
    Author       : Donald
    Modification : Created function

*****************************************************************************/
VOS_UINT32  SipGetServiceStatus(VOS_UINT32 ulUeSn, VOS_UINT32 *pulStatus)
{
    #define SIP_STATUS_LEN 1
    VOS_CHAR szLine[SIP_LARGE_LEN] = {0};
    VOS_CHAR szVarName[SIP_SMALL_LEN] = {0};
    VOS_CHAR* pPosition = NULL;
    VOS_INT filep;
    VOS_INT iReadByte;

    if(VOS_NULL == pulStatus)
    {
        return VOS_ERR;
    }

    /*START ADD: liujianfeng 37298 for [A36D03258] at 2007-01-16 */
    *pulStatus = 0;    
    /*END ADD: liujianfeng 37298 for [A36D03258] at 2007-01-16 */

    if( (filep = open(SIP_STATUS_FILE, O_RDONLY)) < 0)
    {
        return VOS_ERR;
    }
    
    if(VOS_OK != SipLockFile(filep, F_RDLCK))
    {
        close(filep);
        return VOS_ERR;
    }

    sprintf(szVarName,"endpt%d=",ulUeSn);

    while(0 != (iReadByte = read(filep,szLine,strlen(szVarName)+ SIP_STATUS_LEN)))
    {
        if(iReadByte != strlen(szVarName)+ SIP_STATUS_LEN)
        {
            printf("\nREAD:%s error\n",szLine);
            pPosition = NULL;
            break;
        }
        szLine[iReadByte]=0;    
        if( NULL != (pPosition= strstr(szLine,szVarName)) )
        {
            if( pPosition != szLine)
            {
                printf("\nposition is not eqr %d != %d\n",pPosition,szLine);
                pPosition = NULL;
                break;
            }
            /*只支持状态码是一位的情况*/
            *pulStatus = *(pPosition+strlen(szVarName))-'0';
            break;
        }
        read(filep,szLine,1);
    }

    SipLockFile(filep, F_UNLCK);
    close(filep);

    if( NULL == pPosition)
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
    return VOS_ERR;
}

//**************************************************************************
// Function Name: BcmDb_getSipAdvDefaultValue
// Description  : get default value of sip adv cfg.
// Parameters   : info(OUT) - SIP_WEB_ADV_CFG_S structure pointer .
// Returns      : void.
//**************************************************************************
void BcmDb_getSipAdvDefaultValue(SIP_WEB_ADV_CFG_S *info)
{
   if ( NULL == info)
   {
      return;
   }
   // give sip advanced default cfg
   /* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
   info->stServerCfg.ulRegPeriod = (((DEFAULT_EXPIRATION_DURATION-15) > 0)?(DEFAULT_EXPIRATION_DURATION-15):(DEFAULT_EXPIRATION_DURATION/2));
   /* END:   Added by y67514, 2009/11/4 */
   info->stServerCfg.ulExpiration     = DEFAULT_EXPIRATION_DURATION;
   info->stServerCfg.ulReRegister     = DEFAULT_REREGISTER_TIMER;
   info->stServerCfg.SessionExpires   = DEFAULT_SESSION_EXPIRES;
   info->stServerCfg.MinSE            = DEFAULT_MIN_SE;
   /* BEGIN: Modified by p00102297, 2008/1/29 */
   info->stServerCfg.ulMaxRetryTime   = DEFAULT_MAXRETRY_TIME;
   info->stServerCfg.ulPrimaryCheckTime = DEFAULT_PRIMARY_CHECK_TIME;
   /* END:   Modified by p00102297, 2008/1/29 */
   /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
   info->stServerCfg.ulMWIExpire     =  DEFAULT_MWI_EXPIRE;
   /* END:   Added by chenzhigang, 2008/11/5 */
   info->stRTPCfg.ulStartPort         = DEFAULT_RTP_START_PORT;
   info->stRTPCfg.ulEndPort           = DEFAULT_RTP_END_PORT;
   /* BEGIN: Modified by p00102297, 2008/1/18 */
   info->stRTPCfg.ulTxRepeatInterval  = DEFAULT_VOICE_RTCPREPEAT_INTERVAL;
   /* END:   Modified by p00102297, 2008/1/18 */

   /*start of modification by chenyong 2009-03-05*/
   info->stRTPCfg.ulRtcpMonitor  = DEFAULT_RTCP_MONITOR;
   /*end of modification by chenyong 2009-03-05*/

   /* BEGIN: Modified by p00102297, 2008/2/21 */
   //info->stVoiceCfg.ulPrimary         = DEFAULT_VOICE_CODEC_1;
   //info->stVoiceCfg.ulSecondary       = DEFAULT_VOICE_CODEC_2;
   //info->stVoiceCfg.ulThird           = DEFAULT_VOICE_CODEC_3;
   /* BEGIN: Modified by p00102297, 2008/1/18 */
   //info->stVoiceCfg.ulG711APTime      = DEFAULT_VOICE_PACKET_PERIOD;
   //info->stVoiceCfg.ulG711UPTime      = DEFAULT_VOICE_PACKET_PERIOD;
   //info->stVoiceCfg.ulG729PTime       = DEFAULT_VOICE_PACKET_PERIOD;
   /* END:   Modified by p00102297, 2008/1/18 */
   //info->stVoiceCfg.ulDtmfType        = DEFAULT_VOICE_DTMF_TYPE;
   /* END:   Modified by p00102297, 2008/2/21 */
   //info->ulFaxType                    = DEFAULT_FAX_TYPE;
   strncpy (info->szIfName, DEFAULT_SIP_INTERFACE_STRING, SIP_MAX_IFNAME_STR_LEN - 1);
   info->szIfName[SIP_MAX_IFNAME_STR_LEN - 1] = '\0';
   info->ulSipLocalPort               = DEFAULT_SIP_LOCAL_PORT;
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
   strncpy(info->szPstnPrefix, DEFAULT_SIP_PSTN_PRFFIX_STRING, PSTN_MAX_PREFIX_LEN - 1);
   info->szPstnPrefix[PSTN_MAX_PREFIX_LEN - 1] = '\0';
   info->ulEnblSipTest                = DEFAULT_SIP_TEST_ENBL_FLAG;
   info->ulDscp	=DEFAULT_SIP_DSCP_MARK;   /* Added by s00125931, 2008/9/16   问题单号:vhg556*/
}

//**************************************************************************
// Function Name: BcmDb_setPhoneCommonInfo
// Description  : get default value of phone common cfg.
// Parameters   : info(OUT) - SIP_WEB_PHONE_CFG_S structure pointer .
// Returns      : void.
//**************************************************************************
void BcmDb_getPhoneCommonDefaultValue(SIP_WEB_PHONE_CFG_S *info)
{
   if ( NULL == info)
   {
      return;
   }
   // give phone common default cfg
   /* BEGIN: Modified by p00102297, 2008/2/21 */
   info->lSpkVol        = DEFAULT_VOICE_SPEAK_VOL;
   info->lLstVol        = DEFAULT_VOICE_LISTEN_VOL;
   info->ulDialInterval = DEFAULT_VOICE_DIAL_INTERVAL;
   //info->ulVad          = 0;
   info->ulRegion       = DEFAULT_SIP_REGION_LOCAL;
   info->minHookFlash   = DEFAULT_VOICE_MINHOOK_FLASH;
   info->maxHookFlash   = DEFAULT_VOICE_MAXHOOK_FLASH;
   strncpy((char *)(info->szDigitMap), DEFAULT_VOICE_DIGITMAP, SIP_MAX_DIGIT_MAP - 1);
   info->szDigitMap[SIP_MAX_DIGIT_MAP - 1] = '\0';  
   /* BEGIN: Modified by p00102297, 2008/1/21 */
   //info->ulEC           = DEFAULT_SIP_EC_VALUE;
   //info->ulCNG          = DEFAULT_SIP_CNG_VALUE;
   info->ulOffHookTime  = DEFAULT_VOICE_OFFHOOK_TIME;
   info->ulOnHookTime   = DEFAULT_VOICE_ONHOOK_TIME;
   info->ulFstDialTime  = DEFAULT_VOICE_FIRST_DIAL;
   //add by z67625 for automatic call
   info->lAutoEnbl       = DEFAULT_VOICE_AUTOCALLENBL;
   info->ulAutoInterval  = DEFAULT_VOICE_AUTOCALLINTERVAL;
   strcpy((char *)(info->acAutoDialNum), DEFAULT_VOICE_AUTOCALLNUM);
   /*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
   info->ulReInjection   = DEFAULT_VOICE_REINJECTION;
   /*END: Added by chenyong 65116 2008-09-15 web配置*/
   /*start of additon by chenyong 2008-10-28 for Inner call*/
   info->ulInnerCall    = DEFAULT_VOICE_INNERCALL;
   /*end of additon by chenyong 2008-10-28 for Inner call*/
   info->ulModemService = DEFAULT_VOICE_MODEMSERVICE;
   /*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
   info->bVoicePromptVisible = DEFAULT_VOICE_VOICEPROMPTVISIBLE;
   /*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
}

//**************************************************************************
// Function Name: BcmDb_getSipBscDefaultValue
// Description  : get default value of sip bsc cfg.
// Parameters   : info(OUT) - SIP_WEB_USER_INFO_S structure pointer .
// Returns      : void.
//**************************************************************************
void BcmDb_getSipBscDefaultValue(SIP_WEB_USER_INFO_S *tbl)
{
   int i = 0;
   /*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
   int j = 0;  
   /*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
   /*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
   static SIP_WEB_VOICE_CODEC_LIST_S  g_stVoiceCodecList[SIP_MAX_VOIP_CODE_TYPE] =
   {
	   {CMDPARM_MEDIA_PROTOCOL_G711A,   20,  1,  VOS_TRUE},/*ulCodecCap,ulPriority,bEnable,ulPTime*/
	   {CMDPARM_MEDIA_PROTOCOL_G711U,   20,  2,  VOS_TRUE},
	   {CMDPARM_MEDIA_PROTOCOL_G722,    20,  3,  VOS_TRUE},
	   {CMDPARM_MEDIA_PROTOCOL_G726_32,    20,  4,  VOS_TRUE},
	   {CMDPARM_MEDIA_PROTOCOL_G729,    20,  5,  VOS_TRUE},
	   {CMDPARM_MEDIA_PROTOCOL_BUTT,    20,  6,  VOS_FALSE}/*end*/
   };
   /*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/


   if ( NULL == tbl)
   {
      return;
   }
   // give sip bassic default cfg
   for (i = 0; i < SIP_USER_MAX; i++)
   {
      // sip id
      strncpy(tbl[i].szId, DEFAULT_SIP_ID_STRING, SIP_MAX_NUM_LEN - 1);
      tbl[i].szId[SIP_MAX_NUM_LEN - 1] = '\0';
      // sip user name
      strncpy(tbl[i].szName, DEFAULT_USR_NAME_STRING, SIP_MAX_NAME_LEN - 1);
      tbl[i].szName[SIP_MAX_NAME_LEN - 1] = '\0';      
      // sip user password
      strncpy(tbl[i].szPsw, DEFAULT_USR_PASSWD_STRING, SIP_MAX_PSW_LEN - 1);
      tbl[i].szPsw[SIP_MAX_PSW_LEN - 1] = '\0'; 
      // registar port
      tbl[i].uwPort = DEFAULT_SIP_LOCAL_PORT;
      // call forward table id
/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
      tbl[i].usCliStatus      = DEFAULT_SIP_CLIP_ENABLE;
      /* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
	  tbl[i].usLineEnable = DEFAULT_SIP_LINE_ENABLE;
	  /*start of addition by chenyong 2009-03-19*/
	  tbl[i].usLineCallHoldEnable = DEFAULT_SIP_LINE_CALLHOLD;
	  /*end of addition by chenyong 2009-03-19*/
	  tbl[i].usLineCWEnable = DEFAULT_SIP_LINE_CALLWAIT;
	  /*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	  tbl[i].usLine3WayEnable         = DEFAULT_SIP_LINE_3WAY;
	  tbl[i].usLineCallTransferEnable = DEFAULT_SIP_LINE_CALLTRANSFER;
	  /*END: Added by chenyong 65116 2008-09-15 web配置*/
	  /* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
	  tbl[i].usLineMWIEnable = DEFAULT_SIP_LINE_MWI;
	  strncpy(tbl[i].szLineMWIAccount, DEFAULT_SIP_MWIACCOUNT_STRING, SIP_MAX_NUM_LEN - 1);
      tbl[i].szLineMWIAccount[SIP_MAX_NUM_LEN - 1] = '\0'; 
	  /* END:   Added by chenzhigang, 2008/11/4 */
	  tbl[i].usLineFaxDetect = DEFAULT_SIP_FAX_DETECT;
	  tbl[i].ulVad = DEFAULT_SIP_VAD_VALUE;
	  tbl[i].ulEC = DEFAULT_SIP_EC_VALUE;
	  tbl[i].ulCNG = DEFAULT_SIP_CNG_VALUE;

	  /*BEGIN: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/
	  for( j=0; j < SIP_MAX_VOIP_CODE_TYPE;j++ )
	  {
		  if( CMDPARM_MEDIA_PROTOCOL_BUTT != g_stVoiceCodecList[j].ulCodecCap )
		  {
		  	  tbl[i].stVoiceCfg.stCodecList[j].ulCodecCap = g_stVoiceCodecList[j].ulCodecCap;
		      tbl[i].stVoiceCfg.stCodecList[j].ulPTime    = g_stVoiceCodecList[j].ulPTime;
			  tbl[i].stVoiceCfg.stCodecList[j].ulPriority = g_stVoiceCodecList[j].ulPriority;
			  tbl[i].stVoiceCfg.stCodecList[j].bEnable    = g_stVoiceCodecList[j].bEnable;
		  }
		  else
		  {
		      printf("##user %d BcmDb_getSipBscDefaultValue voice cap init break##\n",i);
		  	  break;
		  }
	  }
	  /*END: Added by chenyong 65116 2008-09-10 增加722,726编解码配置项*/

	  //BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
	  tbl[i].stVoiceCfg.ulPayloadType = DEFAULT_VOICE_PAYLOAD_TYPE;
	  //END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
	  tbl[i].stVoiceCfg.ulDtmfType = DEFAULT_VOICE_DTMF_TYPE;
	  tbl[i].ulFaxType = DEFAULT_FAX_TYPE;
	  /* END:   Modified by p00102297, 2008/3/11 VDF C02*/
	  
	  {
	  	int j;
	  	for (j=0;j<SIP_MAX_DAILNUM_CHANGE_NUM;j++)
	  	{
		  	tbl[i].stAppandPrefixNum[j].szAsideNum[0] = '\0';
	  		tbl[i].stAppandPrefixNum[j].szMatchNum[0] = '\0';
	  		tbl[i].stStripPrefixNum[j].szAsideNum[0] = '\0';
	  		tbl[i].stStripPrefixNum[j].szMatchNum[0] = '\0';
	  		tbl[i].stAppandSuffixNum[j].szAsideNum[0] = '\0';
	  		tbl[i].stAppandSuffixNum[j].szMatchNum[0] = '\0';
	  		tbl[i].stStripSuffixNum[j].szAsideNum[0] = '\0';
	  		tbl[i].stStripSuffixNum[j].szMatchNum[0] = '\0';
	  	#if 0
	  		tbl[i].stStripNum[j].szPrefixNum[0] = '\0';
	  		tbl[i].stStripNum[j].szMatchNum[0] = '\0';
	  		tbl[i].stAppNum[j].szPrefixNum[0] = '\0';
	  		tbl[i].stAppNum[j].szMatchNum[0] = '\0';
		#endif
	  	}
      }
#endif
      if (0 == i)
      {
         tbl[i].ulFwdTbl = DEFAULT_USR1_CALL_FW_TBLID;
      }
      else
      {
         tbl[i].ulFwdTbl = DEFAULT_USR2_CALL_FW_TBLID;
      }
   }
}

//**************************************************************************
// Function Name: BcmDb_getSipSrvDefaultValue
// Description  : get default value of sip server cfg.
// Parameters   : info(OUT) - SIP_WEB_SERVER_S structure pointer .
// Returns      : void.
//**************************************************************************
void BcmDb_getSipSrvDefaultValue(SIP_WEB_SERVER_S *tbl)
{
   int i = 0;
   if ( NULL == tbl)
   {
      return;
   }
   // give sip server default cfg
   for (i = 0; i < SIP_SRV_MAX; i++)
   {
      // proxy address
      strncpy(tbl[i].stSipProxyAddr.szIPAddr, DEFAULT_PROXY_ADDR_STRING, SIP_MAX_NAME_LEN - 1);
      tbl[i].stSipProxyAddr.szIPAddr[SIP_MAX_NAME_LEN - 1] = '\0';
      // proxy port
      tbl[i].stSipProxyAddr.uwPort = DEFAULT_PROXY_PORT;
      // registar address
      strncpy(tbl[i].stSipRegAddr.szIPAddr, DEFAULT_REG_ADDR_STRING, SIP_MAX_NAME_LEN - 1);
      tbl[i].stSipRegAddr.szIPAddr[SIP_MAX_NAME_LEN - 1] = '\0';
      // registar port
      tbl[i].stSipRegAddr.uwPort = DEFAULT_REG_PORT;
      // domain
      strncpy(tbl[i].acDomainName, DEFAULT_DOMAIN_NAME_STRING, SIP_MAX_NAME_LEN - 1);
      tbl[i].acDomainName[SIP_MAX_NAME_LEN - 1] = '\0';
  }
}
#endif
/*END ADD: liujianfeng 37298 for [A36D03372] at 2007-02-2 */

/* end of voice 增加SIP的配置项 by z45221 zhangchen 2006年11月7日 */

/* BEGIN: Added by hKF20513, 2010/1/29   PN:增加Upnp Video控制*/
int AtpDb_getUpnpVEnable(PUPNP_VIDEO_INFO pstUpnpVideoInfo)
{    
    int ret = DB_GET_OK;
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;

    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    if (NULL == pstUpnpVideoInfo) 
	{
    	return -1;
	}
    
    memset(pstUpnpVideoInfo, 0, sizeof(UPNP_VIDEO_INFO));
    status = BcmPsi_objRetrieve(sysPsi, SYS_UPNPV_ID, pstUpnpVideoInfo,
        sizeof(UPNP_VIDEO_INFO), &retLen);

    if(status != PSI_STS_OK)
    {
        pstUpnpVideoInfo->serviceEnable = atoi(BcmDb_getDefaultValue("upnpVideoServ"));
        pstUpnpVideoInfo->provEnable = atoi(BcmDb_getDefaultValue("upnpVideoProv"));

        // store default sys flag info to psi in SDRAM memory
        BcmPsi_objStore(sysPsi, SYS_UPNPV_ID, pstUpnpVideoInfo, sizeof(UPNP_VIDEO_INFO));
        ret = DB_GET_NOT_FOUND;
    }
    else
    {
        ret = DB_GET_OK;
    } 
	
    return ret;
}

void AtpDb_setUpnpVEnable(PUPNP_VIDEO_INFO pstUpnpVideoInfo)
{
    if (NULL == pstUpnpVideoInfo) 
	{
    	return ;
	}
    
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);
	
    BcmPsi_objStore(sysPsi, SYS_UPNPV_ID, pstUpnpVideoInfo, sizeof(UPNP_VIDEO_INFO));
}
/* END:   Added by hKF20513, 2010/1/29 */

void AtpDb_getFTPInfo(FTP_INFO* pFtpInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    //printf("%s:%d get function coming...\n", __FUNCTION__, __LINE__);
    
    if ( pFtpInfo != NULL)
    {
        status = BcmPsi_objRetrieve(sysPsi, FTP_USER_INFO, pFtpInfo, sizeof(FTP_INFO), &retLen);
        if (PSI_STS_OK != status)
        {
            fprintf(stderr, "BcmPsi_objRetrieve(FTP_USER_INFO) error[%d]\r\n", status);
            return;
        }
    }

    return;
}

void AtpDb_setFTPInfo(FTP_INFO *pFtpInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    //printf("%s:%d set function coming...\n", __FUNCTION__, __LINE__);

    if ( pFtpInfo != NULL)
    {
        status = BcmPsi_objStore(sysPsi, FTP_USER_INFO, pFtpInfo, sizeof(FTP_INFO));
        if (PSI_STS_OK != status)
        {
            fprintf(stderr, "BcmPsi_objStore(FTP_USER_INFO) error[%d]\r\n", status);
        }
    }

    return;
}
/*d00104343 USB Camera*/
#ifdef WEBCAM
PSI_STATUS HWDb_getCameraInfo(USB_CAMERA_INFO* pCameraInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    //printf("%s:%d get function coming...\n", __FUNCTION__, __LINE__);

    if ( pCameraInfo != NULL)
    {
        status = BcmPsi_objRetrieve(sysPsi, CAMERA_USER_INFO, pCameraInfo, sizeof(USB_CAMERA_INFO), &retLen);
        if (PSI_STS_OK != status)
        {
            fprintf(stderr, "BcmPsi_objStore(USB_CAMERA_INFO) error[%d]\r\n", status);
            return status;
        }
    }

    return status;
}

PSI_STATUS HWDb_setCameraInfo(USB_CAMERA_INFO *pCameraInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(SYSTEM_APPID);

    //printf("%s:%d set function coming...\n", __FUNCTION__, __LINE__);

    if ( pCameraInfo != NULL)
    {
        status = BcmPsi_objStore(sysPsi, CAMERA_USER_INFO, pCameraInfo, sizeof(USB_CAMERA_INFO));
        if (PSI_STS_OK != status)
        {
            fprintf(stderr, "BcmPsi_objStore(CAMERA_USER_INFO) error[%d]\r\n", status);
        }
    }

    return status;
}
#endif

/*start of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
//**************************************************************************
// Function Name: BcmDb_getHspaInfo
// Description  : retrieve HSPA data and object as well as all the configuration
//                related to this E220 interface
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
extern "C" int BcmDb_getHspaInfo(HSPA_INFO *pHspaInfo)
{
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    if(NULL == pHspaInfo)
    {
        return  DB_GET_NOT_FOUND;
    }
    
    PSI_HANDLE hHspaHandle = BcmPsi_appOpen(HSPA_SWITCH);

    status = BcmPsi_objRetrieve(hHspaHandle, HSPA_INFOOBJ, pHspaInfo, sizeof(HSPA_INFO), &retLen);
     if ( status != PSI_STS_OK ) {
        return  DB_GET_NOT_FOUND;
    }
    return DB_GET_OK;
}

/*Begin:Added by luokunling l192527 for IMSI inform requirement,2011/9/24*/
//**************************************************************************
// Function Name: BcmDb_setIMSINotification
// Description  : 将IMSI 的通知enable属性存放到配置文件里面。
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
// 修改历史:
//                     新生成函数                        
//**************************************************************************
PSI_STATUS BcmDb_setIMSINotification( IMSI_Notification *imsinotification)
{
    PSI_STATUS status = PSI_STS_OK;
	
    printf("the enable value is %d\n",imsinotification->Imsi_Notification_Enable);
    PSI_HANDLE hdIMSI = BcmPsi_appOpen(SYSTEM_APPID);
    status = BcmPsi_objStore(hdIMSI,IMSI_INFORM_ID,imsinotification,sizeof(IMSI_Notification));
    return status;
}

//**************************************************************************
// Function Name: BcmDb_setIMSINotification
// Description  : 从配置文件里面获取IMSI的通知enable属性
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
// 修改历史:
//                     新生成函
//                     修改人:luokunling
//**************************************************************************
int BcmDb_getIMSINotification(IMSI_Notification *imsinotification)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE hdIMSI = BcmPsi_appOpen(SYSTEM_APPID);
   
    status = BcmPsi_objRetrieve(hdIMSI, IMSI_INFORM_ID, imsinotification, 
		sizeof(IMSI_Notification), &retLen);

	if ( status != PSI_STS_OK )
	{
          return DB_GET_NOT_FOUND;
	   //printf("Cannot find the IMSInotificationvalue!\n");
	}
	return DB_GET_OK;
}

/*End:Added by luokunling l192527,2011/9/24*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
PSI_STATUS BcmDb_setHTTPInterceptVisible( HTTP_InterceptVisible *phttp_InterceptVisible)
{
    PSI_STATUS status = PSI_STS_OK;
	
    printf("set httpinterceptvisible value is %d\n",phttp_InterceptVisible->iHttpInterceptVisible);
    PSI_HANDLE hdHttpIntercept = BcmPsi_appOpen(SYSTEM_APPID);
    status = BcmPsi_objStore(hdHttpIntercept,SYS_HTTPINTERCEPT_ID,phttp_InterceptVisible,sizeof(HTTP_InterceptVisible));
    //printf("LKL &&&&&&&&&&&&&&&&&& %s %s %d status =%d\n",__FUNCTION__,__FILE__,__LINE__,status);
    return status;
}
int BcmDb_getHTTPInterceptVisible( HTTP_InterceptVisible *phttp_InterceptVisible)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE hdHttpIntercept = BcmPsi_appOpen(SYSTEM_APPID);
   
    status = BcmPsi_objRetrieve(hdHttpIntercept, SYS_HTTPINTERCEPT_ID, phttp_InterceptVisible, 
		sizeof(HTTP_InterceptVisible), &retLen);

	if ( status != PSI_STS_OK )
	{
	    //printf("Cannot find the HTTPInterceptVisible!\n");
          return DB_GET_NOT_FOUND; 
	}
	return DB_GET_OK;
}
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/

//**************************************************************************
// Function Name: BcmDb_setHspaInfo
// Description  : config HSPA data and object as well as all the configuration
//                related to this E220 interface
//**************************************************************************
extern "C" void BcmDb_setHspaInfo(HSPA_INFO *pHspaInfo)
{
    PSI_HANDLE hHspaHandle  = BcmPsi_appOpen(HSPA_SWITCH);
    BcmPsi_objStore(hHspaHandle, HSPA_INFOOBJ, pHspaInfo, sizeof(HSPA_INFO));
}
/*end  of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
/*d00104343 USB Camera*/

#ifdef VDF_PPPOU

//add by l66195 for pppousb start
//**************************************************************************
// Function Name: BcmDb_getE220Info
// Description  : retrieve E220 data and object as well as all the configuration
//                related to this E220 interface
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getE220Info(UINT16 uE220Id, E220_INFO *pE220Info)
{
	int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE hdE220 = BcmPsi_appOpen(E220_APPID);

    memset(pE220Info, 0, sizeof(E220_INFO));
   
    status = BcmPsi_objRetrieve(hdE220, IFC_E220_ID, pE220Info, sizeof(E220_INFO), &retLen);

	if ( status != PSI_STS_OK )
	{
	    if(uE220Id == IFC_E220_ID)
	    {
	    	// 在这儿设置缺省值
//	    	strcpy(pOpenWifiInfo->pszSSID, BcmDb_getDefaultValue("openWifiSSID"));
//    		strcpy(pOpenWifiInfo->pszOpenWifiGWanIfc, BcmDb_getDefaultValue("openWifiGWanIfc"));
//    		strcpy(pOpenWifiInfo->pszOpenWifiRWanIfc, BcmDb_getDefaultValue("openWifiRWanIfc"));
			printf("getE220Info:usLinkMode=%d, usOperMode=%d, green=%s\n", pE220Info->usLinkMode, pE220Info->usOperMode);
	    }
		else
		{
            ret = DB_GET_NOT_FOUND;
		}
	}

	return ret;
}

//**************************************************************************
// Function Name: BcmDb_setE220Info
// Description  : config E220 data and object as well as all the configuration
//                related to this E220 interface
//**************************************************************************
void BcmDb_setE220Info(UINT16 uE220Id, E220_INFO *pE220Info)
{
	PSI_HANDLE hdE220 = BcmPsi_appOpen(E220_APPID);
    if(pE220Info->usLinkMode == 0)
    {
        tmpfile_writevalue("PDataMode", "0");
    }
    else
    {
        tmpfile_writevalue("PDataMode", "1");
    }
    BcmPsi_objStore(hdE220, IFC_E220_ID, pE220Info, sizeof(E220_INFO));
}

//**************************************************************************
// Description  : retrieve size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPppProfileSize(int id, int *info) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0, size = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE Psi = BcmPsi_appOpen(PPP_PROFILE_APPID);
    
    status = BcmPsi_objRetrieve(Psi, id, &size, sizeof(size), &retLen);

    if ( status != PSI_STS_OK ) {
        *info = 0;
        ret = DB_GET_NOT_FOUND;
    } else
        *info = size;
    
    return ret;
}

//**************************************************************************
// Description  : store  size
// Returns      : none.
//**************************************************************************
void BcmDb_setPppProfileSize(int id, int info) {
    UINT16 size = info;
    PSI_HANDLE Psi = BcmPsi_appOpen(PPP_PROFILE_APPID);
    
    BcmPsi_objStore(Psi, id, &size, sizeof(size));
}

//**************************************************************************
// Description  : retrieve table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPppProfileInfo(int id, PSI_VALUE info, int size) {
    int ret = DB_GET_OK;   
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE Psi = BcmPsi_appOpen(PPP_PROFILE_APPID);
    
    status = BcmPsi_objRetrieve(Psi, id, info, size, &retLen);

    if ( status != PSI_STS_OK )
        ret = DB_GET_NOT_FOUND;
    
    return ret;
}

//**************************************************************************
// Description  : store  table
// Returns      : none.
//**************************************************************************
void BcmDb_setPppProfileInfo(int id, PSI_VALUE info, int size) {
    PSI_HANDLE Psi = BcmPsi_appOpen(PPP_PROFILE_APPID);
    
    BcmPsi_objStore(Psi, id, info, size);
}

//**************************************************************************
// Function Name: BcmDb_getVirtualServerSize
// Description  : retrieve virtual server size
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPppProfileCfgSize(int *info) {
    return BcmDb_getPppProfileSize(PPP_PROFILE_NUM_ID, info);
}


//**************************************************************************
// Function Name: BcmDb_setVirtualServerSize
// Description  : store virtual server size
// Returns      : none.
//**************************************************************************
void BcmDb_setPppProfileCfgSize(int info) {
    BcmDb_setPppProfileSize(PPP_PROFILE_NUM_ID, info);
}

//**************************************************************************
// Function Name: BcmDb_getVirtualServerInfo
// Description  : retrieve virtual server table
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getPppProfileCfgInfo(PROFILE_INFO_ST* tbl, int size) {
    return BcmDb_getPppProfileInfo(PPP_PROFILE_CFG_TBL_ID, (PSI_VALUE)tbl,
                                 sizeof(PROFILE_INFO_ST) * size);
}

//**************************************************************************
// Function Name: BcmDb_setVirtualServerInfo
// Description  : store virtual server table
// Returns      : none.
//**************************************************************************
void BcmDb_setPppProfileCfgInfo(PROFILE_INFO_ST* tbl, int size) {
    BcmDb_setPppProfileInfo(PPP_PROFILE_CFG_TBL_ID, (PSI_VALUE)tbl,
                          sizeof(PROFILE_INFO_ST) * size);
}


//add by l66195 for pppousb end

#endif
/* Start of VDF d00104343*/

//**************************************************************************
// Function Name: HWDb_setVDFWanIfCfgInfo
// Description  : 设置HSDPA可写参数
// Returns      : none.
//**************************************************************************
void HWDb_setVDFWanIfCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo)
{
    PSI_HANDLE sysPsi = BcmPsi_appOpen(VODAFONE_VIB_CFG_APPID);
    printf("Before Writing ..........\n");
    BcmPsi_objStore(sysPsi, VIB_CONFIG_INFO_ID, (VOID *)pVibInfo, sizeof(BCMCFM_VIBINTERFACECFG));
}

//**************************************************************************
// Function Name: HWDb_getVDFWanIfCfgInfo
// Description  : 设置HSDPA可写参数
// Returns      : none.
//**************************************************************************

int HWDb_getVDFWanIfCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(VODAFONE_VIB_CFG_APPID);

    memset(pVibInfo, 0, sizeof(BCMCFM_VIBINTERFACECFG));

    status = BcmPsi_objRetrieve(sysPsi, VIB_CONFIG_INFO_ID, pVibInfo,
        sizeof(BCMCFM_VIBINTERFACECFG), &retLen);

    if ( status != PSI_STS_OK ) {
        return  DB_GET_NOT_FOUND;
    }

    return DB_GET_OK;
}


void HWDb_setVIBInterfaceCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo)
{
    PSI_HANDLE sysPsi = BcmPsi_appOpen(VODAFONE_VIB_INTERFACE_CFG);
    printf("Before Writing ..........\n");
    BcmPsi_objStore(sysPsi, VIB_CONFIG_INFO_ID, (VOID *)pVibInfo, sizeof(BCMCFM_VIBINTERFACECFG));
}

//**************************************************************************
// Function Name: HWDb_getVDFWanIfCfgInfo
// Description  : 设置HSDPA可写参数
// Returns      : none.
//**************************************************************************

int HWDb_getVIBInterfaceCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo)
{
    UINT16 retLen = 0;
    PSI_STATUS status = PSI_STS_OK;
    PSI_HANDLE sysPsi = BcmPsi_appOpen(VODAFONE_VIB_INTERFACE_CFG);

    memset(pVibInfo, 0, sizeof(BCMCFM_VIBINTERFACECFG));

    status = BcmPsi_objRetrieve(sysPsi, VIB_CONFIG_INFO_ID, pVibInfo,
        sizeof(BCMCFM_VIBINTERFACECFG), &retLen);

    if ( status != PSI_STS_OK ) {
        return  DB_GET_NOT_FOUND;
    }

    return DB_GET_OK;
}


#pragma PRG_SECFW
void BcmDb_setSecFwCfg(unsigned short usLevel)
{
	UINT16 level = usLevel;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    BcmPsi_objStore(secPsi, SCM_FW_ID, &level, sizeof(level));
}

void BcmDb_getSecFwCfg(unsigned short *pusLevel)
{
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    unsigned short ret;

    if(!pusLevel)
    	return;
    
    BcmPsi_objRetrieve(secPsi, SCM_FW_ID, (PSI_VALUE)pusLevel, sizeof(unsigned short), &ret);
}

void BcmDb_setSecFwLevel(unsigned short usLevel) 
{
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

    (objSecMngr->getFwTbl())->setFwLevel(usLevel);
}
unsigned short BcmDb_getSecFwLevel(void) 
{
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

    return (objSecMngr->getFwTbl())->getFwLevel();
}

/*add by z67625 增加支持过滤开关处理 start*/
/*过滤开关设置*/
void BcmDb_setSecEnbl(int value, int type)
{
    BcmCfm_SecEnblCfg_t SecEnbl;
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
    /* BEGIN: Added by y67514, 2009/2/17   PN:GLB WEB:portfowding*/
    char interface[IFC_TINY_LEN];
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;
    int status = IFC_STATUS_UNKNOWN;
    /* END:   Added by y67514, 2009/2/17 */
    
    BcmDb_getSecEnblCfg(&SecEnbl);

    printf("BcmDb_setSecEnbl :fw_enable %d, inflt %d, outflt %d, macflt %d, macmngr_enable %d, nat_enable %d\n",SecEnbl.fw_enable,
        SecEnbl.inflt_enable, SecEnbl.outflt_enable, SecEnbl.macflt_enable, SecEnbl.macmngr_enable,SecEnbl.nat_enable);

    switch(type)
    {
        case SCM_SEC_FW_ENABLE:
            if(SecEnbl.fw_enable == value)
            {
                printf("fw_enable the same ,so return\n");
                break;
            }
            
            //生效
            /*如果使能则添加规则*/
            if(1 == value)
            {
                objSecMngr->doFwFilter(SEC_COMMAND_TYPE_ADD);
            }
            /*如果非使能，则删除规则*/
            else
            {
                objSecMngr->doFwFilter(SEC_COMMAND_TYPE_REMOVE);
            }
            SecEnbl.fw_enable = value;
            break;
        case SCM_SEC_INFLT_ENABLE:
            if(SecEnbl.inflt_enable == value)
            {
                printf("inflt_enable the same ,so return\n");
                break;
            }
            
            //生效
            /*如果使能则添加规则*/
            if(1 == value)
            {
                objSecMngr->doIncomingIpFltPolicy(NULL, SEC_COMMAND_TYPE_ADD);
            }
            /*如果非使能，则删除规则*/
            else
            {
                objSecMngr->doIncomingIpFltPolicy(NULL, SEC_COMMAND_TYPE_REMOVE);
            }
            SecEnbl.inflt_enable = value;
            break;
        case SCM_SEC_OUTFLT_ENABLE:
            if(SecEnbl.outflt_enable == value)
            {
                printf("outflt_enable the same ,so return\n");
                break;
            }
            //生效
            /*如果使能则添加规则*/
            if(1 == value)
            {
                
                objSecMngr->doOutgoingIpFltPolicy(SEC_COMMAND_TYPE_ADD);
            }
            /*如果非使能，则删除规则*/
            else
            {
                objSecMngr->doOutgoingIpFltPolicy(SEC_COMMAND_TYPE_REMOVE);
            }
            SecEnbl.outflt_enable = value;
            break;
        case SCM_SEC_MACFLT_ENABLE:
            if(SecEnbl.macflt_enable == value)
            {
                printf("macflt_enable the same ,so return\n");
                break;
            }
            //生效
            /*如果使能则添加规则*/
            if(1 == value)
            {
                objSecMngr->doMacFltPolicy(MAC_FILTER_BRIDGE, SEC_COMMAND_TYPE_ADD);
                objSecMngr->doMacFltPolicy(MAC_FILTER_ROUTE, SEC_COMMAND_TYPE_ADD);         
            }
            /*如果非使能，则删除规则*/
            else
            {
                objSecMngr->doMacFltPolicy(MAC_FILTER_BRIDGE, SEC_COMMAND_TYPE_REMOVE);
                objSecMngr->doMacFltPolicy(MAC_FILTER_ROUTE, SEC_COMMAND_TYPE_REMOVE);
            }
            SecEnbl.macflt_enable = value;
            break;
        case SCM_SEC_MACMNGR_ENABLE:
            if(SecEnbl.macmngr_enable== value)
            {
                printf("macmngr_enable the same ,so return\n");
                break;
            }
            //生效
            /*如果使能则添加规则*/
            if(1 == value)
            { 
                BcmMac_ControlAllRule(MAC_COMMAND_TYPE_ADD);
            }
            /*如果非使能，则删除规则*/
            else
            {
                BcmMac_ControlAllRule(MAC_COMMAND_TYPE_REMOVE);
            }
            SecEnbl.macmngr_enable = value;
            break;
        /* BEGIN: Added by y67514, 2009/2/7   PN:GLB WEB:portfowding*/
        case SCM_SEC_NAT_ENABLE:
            if(SecEnbl.nat_enable == value)
            {
                printf("nat_enable the same ,so return\n");
                break;
            }
            //生效
            wanId.vpi = wanId.vci = wanId.conId = 0;
            while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) 
            {
                if ( wanInfo.flag.service == FALSE || wanInfo.flag.nat == FALSE ) 
                {
                    continue;
                }
                BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
                if ( bcmIsDataPvc(interface) )
                {
                    status = BcmWan_getStatus(&wanId);
                    if ( IFC_STATUS_UP != status )
                    {
                        /*pvc没up不加*/
                        continue;
                    }
                }
                else
                {
                    /*语音pvc不加*/
                    continue;
                }

                /*启用或禁用portforwarding*/
                printf("==vrtsrv:%s:%s:%d:%s 's vrtsrv set to %d==\n",__FILE__,__FUNCTION__,__LINE__,interface,value);
                objSecMngr->doVrtSrv(interface, value);
            }
            SecEnbl.nat_enable = value;
            break;
        /* END:   Added by y67514, 2009/2/7 */
        default:
            break;            
    }

    BcmDb_setSecEnblCfg(&SecEnbl);
}

int BcmDb_getSecEnbl(int type)
{
    BcmCfm_SecEnblCfg_t SecEnbl;

    memset(&SecEnbl, 0, sizeof(BcmCfm_SecEnblCfg_t));

    BcmDb_getSecEnblCfg(&SecEnbl);

    switch(type)
    {
        case SCM_SEC_FW_ENABLE:
            return SecEnbl.fw_enable;    
        case SCM_SEC_INFLT_ENABLE:
            return SecEnbl.inflt_enable;
        case SCM_SEC_OUTFLT_ENABLE:
            return SecEnbl.outflt_enable;
        case SCM_SEC_MACFLT_ENABLE:
            return SecEnbl.macflt_enable;
        case SCM_SEC_MACMNGR_ENABLE:
            return SecEnbl.macmngr_enable;
        /* BEGIN: Added by y67514, 2009/2/7   问题单号:GLB WEB:portfowding*/
        case SCM_SEC_NAT_ENABLE:
            return SecEnbl.nat_enable;
        /* END:   Added by y67514, 2009/2/7 */
        default:
            return SecEnbl.fw_enable;       
    }
}


void BcmDb_setSecEnblCfg(PBcmCfm_SecEnblCfg_t value)
{   
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);

    printf("BcmDb_setSecEnblCfg :fw_enable %d, inflt %d, outflt %d, macflt %d, macmngr %d, nat_enable %d\n",value->fw_enable,
        value->inflt_enable, value->outflt_enable, value->macflt_enable, value->macmngr_enable,value->nat_enable);

    
    BcmPsi_objStore(secPsi, SCM_SEC_ENBL_ID, (PSI_VALUE)value, sizeof(BcmCfm_SecEnblCfg_t));
}

void BcmDb_getSecEnblCfg(PBcmCfm_SecEnblCfg_t value)
{
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    unsigned short ret;
    
    BcmPsi_objRetrieve(secPsi, SCM_SEC_ENBL_ID, (PSI_VALUE)value, sizeof(BcmCfm_SecEnblCfg_t), &ret);
}
/*add by z67625 增加支持过滤开关处理 end*/

int BcmDb_setSecFwEntry(PSEC_FW_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

	if(!pEntry)
		return SEC_STS_ERR_GENERAL;
    
    //modified by z67625 生效时需判断开关是否开启
	if(SCM_CFG_OPTYPE_BYNAME == optType)
    	return (objSecMngr->getFwTbl())->set(pEntry, BcmDb_getSecEnbl(SCM_SEC_FW_ENABLE));
	else
        return (objSecMngr->getFwTbl())->setByID(pEntry, BcmDb_getSecEnbl(SCM_SEC_FW_ENABLE));
    //modified by z67625 生效时需判断开关是否开启
}
int  BcmDb_getSecFwEntry(PSEC_FW_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	SecFwNode *pNode = NULL;

	if(!pEntry)
		return DB_GET_NOT_FOUND;

	if(SCM_CFG_OPTYPE_BYNAME == optType)
    {
    	pNode = (objSecMngr->getFwTbl())->findByName(pEntry->name);

    	if(pNode && pNode->object && (SEC_STS_OK==pNode->object->get(pEntry)))
    		return DB_GET_OK;
    	else
    		return DB_GET_NOT_FOUND;
    }
	else
    {
    	pNode = (objSecMngr->getFwTbl())->findByID(pEntry->instID);

    	if(pNode && pNode->object && (SEC_STS_OK==pNode->object->get(pEntry)))
    		return DB_GET_OK;
    	else
    		return DB_GET_NOT_FOUND;
    }
}
int  BcmDb_getNextSecFwEntry(PSEC_FW_ENTRY pEntry, PSEC_FW_ENTRY pNext)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	SecFwNode *pNode = NULL;

	if(!pNext)
		return DB_GET_NOT_FOUND;

	if(!pEntry)
		pNode = (objSecMngr->getFwTbl())->getSecFwTbl();
	else
	{
	      /*start of  2008.04.17 HG553V100R001C02B013 AU8D00590*/
		pNode = (objSecMngr->getFwTbl())->findByID(pEntry->instID);
	      /*end of  2008.04.17 HG553V100R001C02B013 AU8D00590*/
		if(pNode)
		{
			pNode = pNode->next;
		}
	}

	if(pNode && pNode->object && (SEC_STS_OK == pNode->object->get(pNext)))
		return DB_GET_OK;
	else
		return DB_GET_NOT_FOUND;
	
}
int  BcmDb_removeSecFwEntry(PSEC_FW_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

	if(!pEntry)
		return SEC_STS_ERR_GENERAL;

    //modified by z67625 生效时需判断开关是否开启
    if(SCM_CFG_OPTYPE_BYNAME == optType)
        return (objSecMngr->getFwTbl())->remove(pEntry->name, BcmDb_getSecEnbl(SCM_SEC_FW_ENABLE));
    else
        return (objSecMngr->getFwTbl())->removeByID(pEntry->instID, BcmDb_getSecEnbl(SCM_SEC_FW_ENABLE));
    //modified by z67625 生效时需判断开关是否开启
}
void BcmDb_setSecFwSize(int info) {
    UINT16 size = info;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_FW_NUM_ID, &size, sizeof(size));
}

void BcmDb_setSecFwInfo(PSEC_FW_ENTRY tbl, int size) {
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_FW_TBL_ID, (PSI_VALUE)tbl, sizeof(SEC_FW_ENTRY)*size);
}

int BcmDb_getSecFwValidInstID(void)
{
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
    return (objSecMngr->getFwTbl())->getValidInstID();
}

#pragma PRG_SECDOS
int  BcmDb_setSecDosEntry(PSEC_DOS_ENTRY pEntry)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

	if(!pEntry)
		return SEC_STS_ERR_GENERAL;

	return (objSecMngr->getDosTbl())->set(pEntry);
}
int BcmDb_getSecDosEntry(PSEC_DOS_ENTRY pEntry)
{
    /*start of HG553 2008.05.23 V100R001C02B013 AU8D00551 by c65985 */
    #if 0
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();

	if(!pEntry)
		return DB_GET_NOT_FOUND;

	if(SEC_STS_OK == ((objSecMngr->getDosTbl())->getSecDosTbl())->object->get(pEntry))
		return DB_GET_OK;
	else
		return DB_GET_NOT_FOUND;
    #endif
    unsigned short ret;

    if(!pEntry)
    {
		return DB_GET_NOT_FOUND;
    }
    
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    if ( NULL == secPsi )
    {
        return DB_GET_NOT_FOUND;
    }
    
    BcmPsi_objRetrieve(secPsi, SCM_DOS_ID, (PSI_VALUE)pEntry, sizeof(SEC_DOS_ENTRY), &ret);
 
    return DB_GET_OK;
    /*start of HG553 2008.05.23 V100R001C02B013 AU8D00551 by c65985 */
}

void BcmDb_setSecDosInfo(PSEC_DOS_ENTRY pEntry)
{
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_DOS_ID, (PSI_VALUE)pEntry, sizeof(SEC_DOS_ENTRY));
}

void BcmDb_getSecDosInfo(PSEC_DOS_ENTRY pEntry)
{
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    unsigned short ret;
    
    BcmPsi_objRetrieve(secPsi, SCM_DOS_ID, (PSI_VALUE)pEntry, sizeof(SEC_DOS_ENTRY), &ret);
}

#pragma PRG_SECURLBLK
int  BcmDb_setSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	if(!pEntry)
		return SEC_STS_ERR_GENERAL;

    if(SCM_CFG_OPTYPE_BYNAME == optType)
    	return (objSecMngr->getUrlBlkTbl())->set(pEntry);
    else
        return (objSecMngr->getUrlBlkTbl())->setByID(pEntry);
}
int BcmDb_getSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	SecUrlBlkNode    *pNode = NULL;
	if(!pEntry)
		return DB_GET_NOT_FOUND;

    if(SCM_CFG_OPTYPE_BYNAME == optType)
    	pNode = (objSecMngr->getUrlBlkTbl())->findByName(pEntry->url);
    else
        pNode = (objSecMngr->getUrlBlkTbl())->findByID(pEntry->instID);

	if(pNode && pNode->object && (SEC_STS_OK==pNode->object->get(pEntry)))
		return DB_GET_OK;
	else
		return DB_GET_NOT_FOUND;
}
int BcmDb_getNextSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, PSEC_URLBLK_ENTRY pNext)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	SecUrlBlkNode *pNode = NULL;

	if(!pNext)
		return DB_GET_NOT_FOUND;

	if(!pEntry)
		pNode = (objSecMngr->getUrlBlkTbl())->getSecUrlBlkTbl();
	else
	{
		pNode = (objSecMngr->getUrlBlkTbl())->findByName(pEntry->url);
		if(pNode)
		{
			pNode = pNode->next;
		}
	}
	
	if(pNode && pNode->object && (SEC_STS_OK == pNode->object->get(pNext)))
		return DB_GET_OK;
	else
		return DB_GET_NOT_FOUND;
}

int  BcmDb_removeSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType)
{
	SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
	if(!pEntry)
		return SEC_STS_ERR_GENERAL;

	if(SCM_CFG_OPTYPE_BYNAME == optType)
    	return (objSecMngr->getUrlBlkTbl())->remove(pEntry->url);
    else
    	return (objSecMngr->getUrlBlkTbl())->removeByID(pEntry->instID);            
}

void BcmDb_setSecUrlBlkSize(int info) {
    UINT16 size = info;
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_URL_NUM_ID, &size, sizeof(size));
}

void BcmDb_setSecUrlBlkInfo(PSEC_URLBLK_ENTRY tbl, int size) {
    PSI_HANDLE secPsi = BcmPsi_appOpen(SECURITY_APPID);
    
    BcmPsi_objStore(secPsi, SCM_URL_TBL_ID, (PSI_VALUE)tbl, sizeof(SEC_URLBLK_ENTRY)*size);
}

int BcmDb_getSecUrlBlkValidInstID(void)
{
    SecCfgMngr *objSecMngr = SecCfgMngr::getInstance();
    return (objSecMngr->getUrlBlkTbl())->getValidInstID();
}

#ifdef SUPPORT_VDF_QOS
//**************************************************************************
// Function Name: BcmDb_getPortRange
// Description  : get port(min, max) from format "a:b" or "a"
// Returns      : true for sucess, false for failure with wrong format
//**************************************************************************
int BcmDb_getPortRange(char *ports, int* minPort, int* maxPort)
{
    char minStr[IFC_TINY_LEN];
    char maxStr[IFC_TINY_LEN];
    uint16 minValue;
    uint16 maxValue;
    char *pCol = NULL; 
    int ret = TRUE;
    
    if(ports == NULL || minPort == NULL || maxPort == NULL)
        return ret = FALSE;
    
    if(*ports == '\0')
    {
        *minPort = QOS_DEFAULT_PORT;
        *maxPort = QOS_DEFAULT_PORT;
        return ret = TRUE;
    }
    
    if((pCol = strstr(ports, ":")) != NULL)
    {
        memset(minStr, 0, IFC_TINY_LEN);
        memset(maxStr, 0, IFC_TINY_LEN);
        strncpy(minStr, ports, pCol - ports);
        strncpy(maxStr, pCol+1, ports + strlen(ports) - pCol);

        minValue = atoi(minStr);
        maxValue = atoi(maxStr);

        if(maxValue < minValue)
            return ret = FALSE;
        else
        {
            *minPort = minValue;
            *maxPort = maxValue;
        }
    }
    else
    {
        *minPort = atoi(ports);
        *maxPort = QOS_DEFAULT_PORT;
    }
         
    return ret;
}

//**************************************************************************
// Function Name: BcmDb_setPortRange
// Description  : build port string with the format "a:b" or "a" 
// Returns      : none
//**************************************************************************
void BcmDb_setPortRange(char *ports, int minPort, int maxPort)
{
    if(ports == NULL || (minPort > maxPort && maxPort != QOS_DEFAULT_PORT))
        return;

    if(minPort == QOS_DEFAULT_PORT)
        ports[0] = '\0';
    else if(maxPort == QOS_DEFAULT_PORT)
        sprintf(ports, "%d", minPort);
    else
        sprintf(ports, "%d:%d", minPort, maxPort);
}

#endif

/*start-w00135358 add for VDF Global QoS 20080904*/

/*Start -- w00135358 modify for HG556a- store Hareware Version - 20090720*/
//**************************************************************************
// Function Name: BcmDb_getSnmpInfo
// Description  : retrieve snmp configurations
// Returns      : DB_GET_OK or DB_GET_NOT_FOUND.
//**************************************************************************
int BcmDb_getHwVersion(char *pHwVersion) {
    int ret = DB_GET_OK;   
    
    memset(pHwVersion, 0, sizeof(FTP_CLIENT_TINY_SIZE));

    sysGetBoardVersion((unsigned char *)pHwVersion);
    return ret;   
}
/*End -- w00135358 modify for HG556a- store Hareware Version - 20090720*/
#if defined(__cplusplus)
}
#endif

