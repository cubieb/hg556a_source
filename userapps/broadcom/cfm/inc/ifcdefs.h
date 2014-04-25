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
//  Filename:       ifcdefs.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/2001
//
//  Description:
//      Define the Driver Interface class and functions.
//
*****************************************************************************/

#ifndef __IFC_DEFS_H
#define __IFC_DEFS_H

/********************** Include Files ***************************************/

#include "bcmtypes.h"
#include "bcmatmapi.h"
#include <netinet/in.h>

/********************** Global Constants ************************************/

/**************************************************************************
*    BCM_STATUS and its values. BCM_STATUS is used by all the private BCM
*    HAL and BCM MAC functions to return status.
*
***************************************************************************/

typedef UINT32                               BCM_STATUS, *PBCM_STATUS;

/*
 * BCM_STATUS values
 *
 *   Status values are 32 bit values laid out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +---+-+-------------------------+-------------------------------+
 *  |Sev|C|       Facility          |               Code            |
 *  +---+-+-------------------------+-------------------------------+
 *
 *  where
 *
 *    Sev - is the severity code
 *      00 - Success
 *      01 - Informational
 *      10 - Warning
 *      11 - Error
 *
 *    C - is the Customer code flag
 *
 *    Facility - is the facility code
 *
 *    Code - is the facility's status code
 */

#define STATUS_UNSUCCESSFUL                 ((BCM_STATUS)0xC0000001L)
#define STATUS_SUCCESS                      ((BCM_STATUS)0x00000000L)
#define STATUS_PENDING                      ((BCM_STATUS)0x00000103L)
#define STATUS_BUFFER_OVERFLOW              ((BCM_STATUS)0xC0000104L)
#define STATUS_INSUFFICIENT_RESOURCES       ((BCM_STATUS)0x80000105L)
#define STATUS_NOT_SUPPORTED                ((BCM_STATUS)0x40000106L)

#define BCM_STATUS_SUCCESS                  ((BCM_STATUS)STATUS_SUCCESS)
#define BCM_STATUS_PENDING                  ((BCM_STATUS)STATUS_PENDING)

#define BCM_STATUS_FAILURE                  ((BCM_STATUS)STATUS_UNSUCCESSFUL)
#define BCM_STATUS_RESOURCES                ((BCM_STATUS)STATUS_INSUFFICIENT_RESOURCES)
#define BCM_STATUS_NOT_SUPPORTED            ((BCM_STATUS)STATUS_NOT_SUPPORTED)
#define BCM_STATUS_INVALID_LENGTH           ((BCM_STATUS)0xC0010014L)
#define BCM_STATUS_INVALID_OID              ((BCM_STATUS)0xC0010015L)

#define NVRAM_SERIAL_NUMBER_LEN         32
#define IFC_LEN_6               6
#define IFC_LEN_8               8
#define IFC_TINY_LEN            16
#define IFC_SMALL_LEN           32
/*start of Display  the version of the modem firmware by l129990 2009,11,3*/
#define ADSL_VERSION_LEN        65
#define FILE_CMD_LEN            512
/*end of Display  the version of the modem firmware by l129990 2009,11,3*/
/*start of HG_Support 2008.7.15 HG553V100R001C03 c131380 */
#define IFC_STRING_LEN          256
/*end of HG_Support 2007.01.15 HG553V100R001C03 c131380 */

/*start of HG_Support 2007.01.17 HG550V100R002C01B010 A36D03276 */
#define IFC_MEDIUM_LEN          240
/*end of HG_Support 2007.01.17 HG550V100R002C01B010 A36D03276 */
/*start of 支持global lanDhcpSrv配置页面 by l129990 2009,2,13*/
#define IFC_RESRVDHCP_LEN       320
#define IFC_RESRVMAC_LEN        18
/*end of 支持global lanDhcpSrv配置页面 by l129990 2009,2,13*/
/*start of 修改问题单AU8D00817  by s53329  at   20080715*/
#define IFC_PWD_LEN               256+1
/*end  of 修改问题单AU8D00817  by s53329  at   20080715*/
/* BEGIN: Added by y67514, 2008/9/11   PN:GLB:Ip Extension*/
#define IFC_MAC_LEN         18
/* END:   Added by y67514, 2008/9/11 */
#define IFC_LARGE_LEN           264
#define IFC_GIANT_LEN           1024
#define IFC_HOST_LEN            128
#define IFC_DOMAIN_LEN          64
#define IFC_PASSWORD_LEN        24
#define IFC_PPP_SESSION_LEN     18
#define IFC_PPP_SESSIONS_NAME   "sessions"
#define WEB_SPACE               "&nbsp"
/*start  of 增加Radius 认证功能by s53329  at  20060714 */
#define  RADIUS_SERVER_LEN                     256
#define  RADIUS_SHAREKEY_LEN                 49
/*end  of 增加Radius 认证功能by s53329  at  20060714 */

/* start of 增加tr143 功能*/
#define SOURCE_IP_ADDR_LEN   64
#define INTER_LEN                      256
/* end of 增加tr143 功能*/

/*start of 支持pppou apn配置功能by l129990,2009,3,7 */
#ifdef SUPPORT_PPPOUCONFIG
#define PPPOU_USERNAME_LENGTH    65
#define PPPOU_PASSWORD_LENGTH    33
#define PPPOU_APNNAME_LENGTH     65
#endif
/*Begin:Added by luokunling 00192527,2011/12/28*/
#define IFC_DATAPVCSTATUS_UP                    1
#define IFC_DATAPVCSTATUS_DOWN               0
#define IFC_DATAPVCSTATUS_UNKNOWN         2    
/*End:Added by luokunling 00192527,2011/12/28*/
/*end of 支持pppou apn配置功能by l129990,2009,3,7 */
#define IFC_STATUS_UNKNOWN          0
#define IFC_STATUS_NEW              1
#define IFC_STATUS_AUTH_ERR         2
#define IFC_STATUS_DOWN             3
#define IFC_STATUS_DHCPC_PROGRESS   4
#define IFC_STATUS_DHCPC_FAIL       5
#define IFC_STATUS_UP               6

#define IFC_WAN_MAX                 8
#define ATM_QUEUE_MAX               8

#define IFC_WLAN_MAX                 4
/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
#define IFC_SCANLIST_LEN            640
#define MAXNUMOFSCANLIST        64
/* END:   Added by y67514, 2008/10/27 */
#if defined(CHIP_6348) || defined(CHIP_6358)
/* start of maintain 按照规格，最大PVC 数改为8 条 by z60003055 2006年7月13日
#define ATM_VCC_MAX                 16
#define IFC_PPP_MAX                 16
*/

#ifdef SUPPORT_GLB_PVC
/*8条PVC和一条PPOU*/
#define ATM_VCC_MAX                 9   /*modify by w00135351 20080926 改为支持最大8条PVC*/
#define MAX_PPP_ON_PVC             2        /*同一PVC上最大能配的PPP连接数*/
#else
#define ATM_VCC_MAX                 5
#endif /* SUPPORT_GLB_PVC */


#define IFC_PPP_MAX                 8
/* end of maintain 按照规格，最大PVC 数改为8 条 by z60003055 2006年7月13日 */
#else
#define ATM_VCC_MAX                 8
#define IFC_PPP_MAX                 8
#endif

#define ATM_VCC_APPID 			  "AtmCfgVcc"
#define ATM_TD_APPID 			  "AtmCfgTd"
#define ATM_CFG_APPID 			  "AtmCfg"
#define PPP_APPID 			     "PppInfo"
#define DHCP_CLT_APPID		     "DhcpClntInfo"
//#define MAX_UPGRADE_FLAG               8
#define LAN_APPID 			     "Lan"
#define IFC_NAME_LEN		        16
#define IFC_IP_ADDR_SIZE   	  16
#define IFC_LAN_MAX        	  4
#define IFC_UNKNOWN_ID     	  0
#define IFC_ENET_ID	          1
#define IFC_ENET1_ID            (IFC_ENET_ID+1)
#define IFC_USB_ID		        (IFC_ENET_ID + IFC_LAN_MAX)
#define IFC_HPNA_ID		        (IFC_USB_ID + IFC_LAN_MAX)
#define IFC_WIRELESS_ID		    (IFC_HPNA_ID + IFC_LAN_MAX)
#define IFC_WIRELESS1_ID	    (IFC_WIRELESS_ID + IFC_LAN_MAX)
#define IFC_WIRELESS2_ID	    (IFC_WIRELESS1_ID + IFC_LAN_MAX)
#define IFC_WIRELESS3_ID	    (IFC_WIRELESS2_ID + IFC_LAN_MAX)
#define IFC_SUBNET_ID           (IFC_WIRELESS_ID + IFC_LAN_MAX*IFC_WLAN_MAX) // for sencond LAN subnet
#define IFC_ENET0_VNET_ID       (IFC_SUBNET_ID + IFC_LAN_MAX)
#define IFC_ENET1_VNET_ID       (IFC_ENET0_VNET_ID + IFC_LAN_MAX)
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

#ifdef VDF_PPPOU
//add by l66195 for pppousb start
#define E220_APPID          "E220"
#define IFC_E220_ID         (IFC_ENET1_VNET_ID + IFC_LAN_MAX)   // 29
//add by l66195 for pppousb end
#endif

#define HSPA_SWITCH                 "Hspacfg"
#define HSPA_INFOOBJ                       1
#define WAN_APPID 			     "Wan"

#define BR2684_APPID 			  "BR2684"
#define IFC_BR2684_ID		     16
#define IFC_BRIDGE_NAME			  "br0"
#define IFC_BRIDGE_DHCP_NAME	  "br0:1"

#define MER2684_APPID 			  "MER2684"
#define IFC_RFC2684_MAX    	  8
#define IFC_MER2684_ID		     (IFC_BR2684_ID + IFC_RFC2684_MAX)

#define PPPOE_APPID 			     "PPPOE"
#define IFC_PPPOE2684_ID   	  (IFC_MER2684_ID + IFC_RFC2684_MAX)

#define PPPOA_APPID 			     "PPPOA"
#define IFC_PPPOA2684_ID   	  (IFC_PPPOE2684_ID + IFC_RFC2684_MAX)

#define IPOA_APPID 			     "IPOA"
#define IFC_IPOA2684_ID   	     (IFC_PPPOA2684_ID + IFC_RFC2684_MAX)

#define PPP_AUTH_METHOD_AUTO    0
#define PPP_AUTH_METHOD_PAP     1
#define PPP_AUTH_METHOD_CHAP    2
#define PPP_AUTH_METHOD_MSCHAP  3

/*start of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/
#define DHCP_CLASS_ID_MAX        64
/*end of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/

/*start of 增加dhcp主机标识功能by l129990 */
#define DHCP_HOSTID_LENGTH        65
/*end of 增加dhcp主机标识功能by l129990 */

#ifndef VDF_RESERVED
#define VDF_RESERVED
#endif

#ifdef	VDF_RESERVED
#define DHCP_RESERVED_MAXCHAR 128
#endif

#define ATM_AUX_CHANNEL_VPI     1
#define ATM_AUX_CHANNEL_VCI     39

#define INSTID_OBJID            9999
#define INSTID_VALID_VALUE      0x5a000000
#define INSTID_VALID_MASK       0xff000000
/* start of maintain 增加消息处理进程，处理与非cfm模块的通信 by liuzhijie 00028714 2006年7月7日" */
#define CFM_MSG_PATH    "\var"
#define CFM_MSG_SEED    'c'
#ifdef SUPPORT_SNTP_AUTOCONF
#define SNTPSRV_DHCP_TYPE   1
#endif
#ifdef SUPPORT_SYSLOG_AUTOCONF
#define LOGSRV_DHCP_TYPE    2
#endif
/* end of maintain 增加消息处理进程，处理与非cfm模块的通信 by liuzhijie 00028714 2006年7月7日" */

/* Start of VDF d00104343 */
#define VODAFONE_VIB_CFG_APPID        "VibCfg"    //HSDPA的配置信息
#define VODAFONE_VIB_INTERFACE_CFG        "VibInterfaceCfg"    //HSDPA的配置信息
#define VIB_CONFIG_INFO_ID               1

/* maxium 32 URL-blocking rules in firewall */
#define FW_BLOCK_URL_MAX                 32
/* maxium 32 port forwarding rules in firewall */
#define FW_PORTFORWARDING_MAX            32
/* maxium 32 static route rules in firewall */
#define FW_STATIC_ROUTE_MAX              32
/* j00100803 Add Begin 2008-05-20 for option43 */
#define MAX_OPTION43_LEN    256
/* j00100803 Add End 2008-05-20 for option43 */

#define CONN_SWITCH_MODE_PLAIN      0  //不需要定时通过访问dns服务检测adsl连接的有效性
#define CONN_SWITCH_MODE_COMPLEX    1 //需要定时访问dns服务来检测adsl连接的有效性

typedef struct IfcAtmTdInfo {
   UINT32 peakCellRate;
   UINT32 sustainedCellRate;
   UINT32 maxBurstSize;
   char serviceCategory[IFC_TINY_LEN];
} IFC_ATM_TD_INFO, *PIFC_ATM_TD_INFO;

typedef struct IfcAtmVccInfo {
   UINT32 id;
   UINT16 tdId;
   UINT32 aalType;
   UINT32 adminStatus;
   UINT16 encapMode;
   UINT16 enblQos;
   ATM_VCC_ADDR vccAddr;
   /* start of protocol QoS for TdE by z45221 zhangchen 2006年8月14日 */
#ifdef SUPPORT_TDE_QOS
   UINT32 QosIfcRate;
   UINT32 QosIfcCeil;
#endif
   /* end of protocol QoS for TdE by z45221 zhangchen 2006年8月14日 */
} IFC_ATM_VCC_INFO, *PIFC_ATM_VCC_INFO;

#define DHCP_SRV_DISABLE   0
#define DHCP_SRV_ENABLE    1
#define DHCP_SRV_RELAY     2

/*start of support to the dhcp relay  function by l129990,2009,11,12*/
#define DHCP_RELAY_ENABLE   1
#define DHCP_RELAY_DISABLE  0
/*end of support to the dhcp relay  function by l129990,2009,11,12*/

typedef struct IfcDhcpSrvInfo {
   UINT16 enblDhcpSrv;
   UINT16 leasedTime;
   struct in_addr startAddress;
   struct in_addr endAddress;
   #ifdef	VDF_RESERVED
   char reservedAddress[DHCP_RESERVED_MAXCHAR];
   #endif
/*w44771 add for 第一IP支持5段地址池，begin*/   
#ifdef SUPPORT_DHCP_FRAG
   int poolIndex;

   struct in_addr dhcpStart1_1;
   struct in_addr dhcpEnd1_1;
   int dhcpLease1_1;
   char dhcpSrv1Option60_1[DHCP_CLASS_ID_MAX];
   struct in_addr dhcpStart1_2;
   struct in_addr dhcpEnd1_2;
   int dhcpLease1_2;
   char dhcpSrv1Option60_2[DHCP_CLASS_ID_MAX];
   struct in_addr dhcpStart1_3;
   struct in_addr dhcpEnd1_3;
   int dhcpLease1_3;
   char dhcpSrv1Option60_3[DHCP_CLASS_ID_MAX];
   struct in_addr dhcpStart1_4;
   struct in_addr dhcpEnd1_4;
   int dhcpLease1_4;
   char dhcpSrv1Option60_4[DHCP_CLASS_ID_MAX];
   struct in_addr dhcpStart1_5;
   struct in_addr dhcpEnd1_5;
   int dhcpLease1_5;
   char dhcpSrv1Option60_5[DHCP_CLASS_ID_MAX];   

#ifdef SUPPORT_CHINATELECOM_DHCP
   char cameraport[IFC_TINY_LEN];
   char ccategory[IFC_SMALL_LEN];
   char cmodel[IFC_SMALL_LEN];
#endif

#endif
/*w44771 add for 第一IP支持5段地址池，end*/
#ifdef  SUPPORT_PORTMAPING
   char option60[IFC_LARGE_LEN];
#endif
   char dhcpSrvClassId[DHCP_CLASS_ID_MAX];/*DHCP Server支持第二地址池, by s60000658, 20060616*/
/* j00100803 Add Begin 2008-03-05 */
#ifdef SUPPORT_VDF_DHCP
   struct in_addr dhcpNtpAddr1;
   struct in_addr dhcpNtpAddr2;
#endif
/* j00100803 Add End 2008-03-05 */
} IFC_DHCP_SRV_INFO, *PIFC_DHCP_SRV_INFO;

typedef struct IfcLanInfo {
   UINT32 id;
   /*start of Global V100R001C01B020 AU8D00949 by c00131380 at 2008.10.20*/
   UINT32 addressType;  //地址类型定义 1:Staic 2:DHCP 3:AutoIP
   /*end of Global V100R001C01B020 AU8D00949 by c00131380 at 2008.10.20*/
   /*start of support to the dhcp relay  function by l129990,2009,11,12*/
   UINT32 dhcpRelay;
   /*end of support to the dhcp relay  function by l129990,2009,11,12*/
   struct in_addr lanAddress;
   struct in_addr lanMask;
   IFC_DHCP_SRV_INFO dhcpSrv;
   /*start of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
   struct in_addr lanDnsAddress1;
   struct in_addr lanDnsAddress2;
   /*end of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
   /* j00100803 Add Begin 2008-05-20 for option43 */
   char szLanOption43[MAX_OPTION43_LEN];
   /* j00100803 Add End 2008-05-20 for option43 */
   /*start of support to configure the option66,67,160*/
    char szLanOption66[DHCP_CLASS_ID_MAX];
    char szLanOption67[DHCP_CLASS_ID_MAX];
    char szLanOption160[DHCP_CLASS_ID_MAX];
   /*end of support to configure the option66,67,160*/
   /* BEGIN: Added by y67514, 2008/9/22   问题单号:GLB:lan dns*/
   char lanDomain[IFC_DOMAIN_LEN];
   /* END:   Added by y67514, 2008/9/22 */
} IFC_LAN_INFO, *PIFC_LAN_INFO;

#ifdef SUPPORT_DHCPOPTIONS
typedef struct DhcpOptions{//w44771 modify for option 241 and 242 and length
char option240[2*DHCP_CLASS_ID_MAX];
char option241[2*DHCP_CLASS_ID_MAX];
char option242[2*DHCP_CLASS_ID_MAX];
char option243[2*DHCP_CLASS_ID_MAX];
char option244[2*DHCP_CLASS_ID_MAX];
char option245[2*DHCP_CLASS_ID_MAX];
}DHCPOptions, *PDHCPOptions;
#endif
/*start of add the Upnp Video Provision function by l129990,2010,2,21*/
typedef struct UpnpVideoNode
{
   int serviceEnable;
   int provEnable;
}UPNP_VIDEO_INFO, *PUPNP_VIDEO_INFO;
/*end of add the Upnp Video Provision function by l129990,2010,2,21*/
//BEGIN:add by zhourongfei to config web at 2011/05/31
typedef struct WebCfgNode
{
	int LANDisplayCFG;
	int HSPADisplayCFG;
	int PINDisplayCFG;
}WEB_CFG_INFO, *PWEB_CFG_INFO;
//END:add by zhourongfei to config web at 2011/05/31
/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
typedef struct IgmpProxyNode
{
   int igmpEnable;
   char igmpInterface[IFC_TINY_LEN];
   int igmpVersion;
   int igmpGroups;
   int igmpLogSwitch;
}IgmpProxyNode, *PIgmpProxyNode;
/*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
typedef struct IfcDnsInfo {
   struct in_addr preferredDns;
   struct in_addr alternateDns;
   UINT16 dynamic;
   char domainName[IFC_DOMAIN_LEN];
   UINT16 enbdnsProxy;  // add by y00183561 for lan dns proxy enable/disable 2011/10/22
} IFC_DNS_INFO, *PIFC_DNS_INFO;

typedef struct IfcDefGWInfo {
   UINT16 enblGwAutoAssign;     // ==1 use auto assigned def. gw
   char ifName[IFC_TINY_LEN];
   struct in_addr defaultGateway;
} IFC_DEF_GW_INFO, *PIFC_DEF_GW_INFO;

#define ETH_SPEED_AUTO           0
#define ETH_SPEED_100            1
#define ETH_SPEED_10             2
#define ETH_PORT_DISABLE         3
#define ETH_TYPE_AUTO            0
#define ETH_TYPE_FULL_DUPLEX     1
#define ETH_TYPE_HALF_DUPLEX     2
#define ETH_CFG_ID               1024
#define ETH_CFG_MTU_MIN          46
#define ETH_CFG_MTU_MAX          1500
typedef struct EthCfgInfo {
   UINT16 ethSpeed;
   UINT16 ethType;
   UINT16 ethMtu;
   UINT16 ethIfcDisabled;
} ETH_CFG_INFO, *PETH_CFG_INFO;


// multiple protocols changes
// new database structures for multiple protocols
// over single PVC behind this line

// WAN flag define
// 8 bit
// bit 0 : enable firewall
// bit 1 : enable nat
// bit 2 : enable igmp
// bit 3 : enable service
// bit 4-7: reserve
typedef struct WanFlagInfo {
   UINT8
   firewall:1,
   nat:1,
/*start of删除brcom igmp proxy 使能功能by l129990 ,2008,9,27*/
   //igmp:1,
/*end of删除brcom igmp proxy 使能功能by l129990 ,2008,9,27*/
   service:1,
   reserved:4;
} WAN_FLAG_INFO;

typedef struct WanPppInfo {
   UINT16 idleTimeout;  // 0 disable onDemand, > 0 timeout in second
   UINT8 enblIpExtension;
   UINT8 authMethod;
   UINT8 automanualConnect;   //modify by xia 133940 2008年9月6日,  PPP拨号时，在WEB页面中选择手动还是自动连接
   UINT8 delayRedial;        //modify by xia 133940 2008年9月6日, 拨号失败后再次拨号的间隔时间（手自动拨号）
   UINT8 timeout;             //modify by xia 133940 2008年9月6日,  报文重发的间隔时间
   UINT8 delayTime;        //modify by xia 133940 2008年9月6日,  第一次拨号时，延迟x秒进行拨号（自动拨号）
   char userName[IFC_LARGE_LEN];
   /*start of 修改问题单AU8D00817  ppp 密码过长问题by s53329 at   20080715
   char password[IFC_MEDIUM_LEN];
   */
   char password[IFC_PWD_LEN];
    /*end of 修改问题单AU8D00817  ppp 密码过长问题by s53329 at   20080715 */
   char serverName[IFC_MEDIUM_LEN];
   UINT16 useStaticIpAddress;
   struct in_addr pppLocalIpAddress;
   UINT8 enblDebug;    // 0 disable debugging, 1 enable debugging
#ifdef VDF_PPPOU
// add by l66195 for pppousb start
   char acProName[IFC_LARGE_LEN];  //profile name
// add by l66195 for pppousb end
//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
	UINT16 pppNumber;
//END:add by zhourongfei to config number of PPP keepalive attempts
#endif
/* BEGIN: Added by y67514, 2008/9/8   PN:GLB:增加参数mtu，mixed*/
    UINT8 enblBridgeMixed;
    UINT16 Mtu;
    UINT16 lcpEchoInterval;
    char ipExtMac[IFC_MAC_LEN];
/* END:   Added by y67514, 2008/9/8 */

/*Start -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/
 	UINT8 enblpppVoB;
 	struct in_addr pppVoBitstreamIPAddr;
/*End -- w00135358 add for HG556 20090309 -- VoIP over Bitstream*/

} WAN_PPP_INFO, *PWAN_PPP_INFO;

typedef struct WanIpInfo {
   UINT8 enblDhcpClnt;
   struct in_addr wanAddress;
   struct in_addr wanMask;
   char dhcpClassIdentifier[DHCP_CLASS_ID_MAX];   /*WAN <3.4.5MER使能dhcp, option60> porting by shuxuegang 60000658 20060506*/   
   #ifdef SUPPORT_GLB_MTU
   UINT16 mtu; 
   #endif /* SUPPORT_GLB_MTU */
} WAN_IP_INFO, *PWAN_IP_INFO;
/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
#define IFC_MAX_SERVER_LEN	       128
#define IFC_MAX_VDF_LEN		128

typedef struct AutoUpgradeInfo
{
    UINT32 ulCustomUpdate;
    char szUpdateServer[IFC_MAX_SERVER_LEN];
    char szVDFName[IFC_MAX_VDF_LEN];
    UINT32 ulUpdateInterval;
}AUTO_UPGRADE_INFO, *PAUTO_UPGRADE_INFO;
/* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */

typedef struct WanConInfo {
   UINT32 id;
   UINT16 vccId;
   UINT16 conId;
   UINT8 protocol;
   UINT8 encapMode;
   INT16 vlanId;           // 0 - 4095, -1 -> vlan is not enabled
   WAN_FLAG_INFO flag;
    /*start of 修改问题单AU8D00817 by s53329  at   20080715
    char conName[IFC_MEDIUM_LEN];
    */
    char conName[IFC_PWD_LEN];
    /*end of 修改问题单AU8D00817 by s53329  at   20080715*/
    /*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 AU8D00758*/
    IFC_DNS_INFO stDns;
    struct in_addr ipGateWay;
    /*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00739 AU8D00758 */
} WAN_CON_INFO, *PWAN_CON_INFO;

typedef struct WanConId {
   UINT16 vpi;
   UINT16 vci;
   UINT16 conId;
} WAN_CON_ID, *PWAN_CON_ID;

typedef struct IfcPppSessionInfo {
   WAN_CON_ID wanId;
   char session[IFC_PPP_SESSION_LEN];
} IFC_PPP_SESSION_INFO, *PIFC_PPP_SESSION_INFO;

// System flag define
#define SYS_FLAG_AUTO_SCAN           0
#define SYS_FLAG_UPNP                1
#define SYS_FLAG_MAC_POLICY          2
#define SYS_FLAG_SIPROXD             3
#define SYS_FLAG_ENCODE_PASSWORD     4
#define SYS_FLAG_IGMP_SNP            5
#define SYS_FLAG_IGMP_MODE           6
//modifyed by l66195 for VDF start
//#define SYS_FLAG_ENET_WAN            7
#define SYS_FLAG_DDNS                7
#define SYS_FLAG_ENET_WAN            8
//modifyed by l66195 for VDF end
typedef struct SysFlagInfo {
//modifyed by l66195 for VDF start
 //  UINT8
   UINT16
//modifyed by l66195 for VDF end
   autoScan:1,
   upnp:1,
   macPolicy:1,
   siproxd:1,
   encodePassword:1,
   igmpSnp:1,
/*start of删除snooping的模式选择功能 by l129990 2008,9,9*/
   //igmpMode:1,
/*end of删除snooping的模式选择功能 by l129990 2008,9,9*/
//modifyed by l66195 for VDF start
    ddns:1,
//begin add by p44980 2008.10.22 
    connSwitchMode:1,         //0:mode1   1:mode2
//end add by p44980 2008.10.22
#if 0
#if SUPPORT_ETHWAN
   enetWan:1;
#else
   reserved:1;
#endif
#endif
#if SUPPORT_ETHWAN
   enetWan:1;
   reserved:7;
#else
   reserved:8;
#endif
//modifyed by l66195 for VDF end
} SYS_FLAG_INFO, *PSYS_FLAG_INFO;
/*start  of 增加Radius 认证功能by s53329  at  20060714 */
typedef struct  SysRadiusInfo
{
    UINT32          PrimaryPort;
    UINT32          SecondPort;
    UINT32          Retransmit;
    UINT32         Timeout;
    UINT32         WebEnable;
    UINT32         TelnetEnable;
    UINT32         SshEnable;
    char            PrimaryServer[RADIUS_SERVER_LEN];
    char            SecondServer[RADIUS_SERVER_LEN];
    /*start of HGW  2006.08.09  V100R001B01D010 A36D02447 */
    char            ShareKey[RADIUS_SHAREKEY_LEN *2];
    /*end of HGW  2006.08.09  V100R001B01D010 A36D02447 */
    /* start of maintain 不启动radius的主服务器，启动从服务器时web刷新很慢 by xujunxia 43813 2006年9月5日" */
    char  PrimaryServerInactive;
    /* end of maintain 不启动radius的主服务器，启动从服务器时web刷新很慢 by xujunxia 43813 2006年9月5日" */
}RADIUS_CLIENT_INFO, *PRADIUS_CLIENT_INFO;
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
typedef struct SysMgmtLockInfo {
   UINT8 action;
   UINT8 type;
} SYS_MGMT_LOCK_INFO, *PSYS_MGMT_LOCK_INFO;

/*start of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/
typedef struct BridgeDhcpcInfo {
   UINT16 enableDhcpc;
   char dhcpClassIdentifier[DHCP_CLASS_ID_MAX];   //option60
} WAN_BRDHCPC_INFO, *PWAN_BRDHCPC_INFO;
/*end of protocol   WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/

/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
#define CUSTOM_PORT_SHRESHOLD 1024
#define DEFAULT_SIPPROXYPORT 5060
typedef struct 
{
    UINT16 enable;
    UINT32 portnumber;
}SIPPROXYINFO, *PSIPPROXYINFO;
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/

#ifdef SUPPORT_GLB_ALG
typedef struct 
{
    UINT16 H323Alg;
    UINT16 RtspAlg;
    UINT16 FtpAlg;
    UINT16 TftpAlg;
    UINT16 L2tpAlg;
    UINT16 PptpAlg;
    UINT16 IpsecAlg;
}NATALGINFO, *PNATALGINFO;
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
typedef struct 
{
    char ScanList[IFC_SCANLIST_LEN];
}AUTOSERCHINFO, *PAUTOSERCHINFO;

typedef struct PVC
{
    UINT8   vpi;
    UINT16 vci;
}PVC_STRU;

/* END:   Added by y67514, 2008/10/27 */

#ifdef VDF_PPPOU


// add by l66195 for pppousb start

#define PPP_PROFILE_APPID          "PPPProfile"
#define PPP_PROFILE_NUM_ID         1
#define PPP_PROFILE_CFG_TBL_ID     2

#define PPP_PROFILE_APN_LEN   64
#define PPP_PROFILE_USERNAME_LEN   64
#define PPP_PROFILE_PASSWORD_LEN   64



typedef enum {
   PPP_PROFILE_STS_OK = 0,
   PPP_PROFILE_STS_ERR_GENERAL,
   PPP_PROFILE_STS_ERR_MEMORY,
   PPP_PROFILE_STS_ERR_INVALID_OBJECT,
   PPP_PROFILE_STS_ERR_OBJECT_NOT_FOUND,
   PPP_PROFILE_STS_ERR_OBJECT_EXIST,
   PPP_PROFILE_STS_ERR_OPEN_FILE
} PPP_PROFILE_STATUS;

//profile参数结构
typedef struct 
{
    UINT32 ulauthMethod;
    UINT32 ulautoDns; //是否自动获取dns服务器
    UINT32 ulautoApn;
    UINT32  bPasswordRequest;
    char cProfileName[IFC_SMALL_LEN];
    char cAPN[IFC_DOMAIN_LEN + 4];
    char cDialNumber[IFC_SMALL_LEN];
    char cHeaderCompression[IFC_SMALL_LEN];
    char cIPAddress[IFC_TINY_LEN];
    char cPrimaryDNS[IFC_TINY_LEN];
    char cSecondaryDNS[IFC_TINY_LEN];
    char cPassword[IFC_MEDIUM_LEN];
    char cUserName[IFC_LARGE_LEN];
} PROFILE_INFO_ST;


typedef struct E220Info 
{
//   UINT32  id;
   UINT16  usLinkMode ;  // 0: ADSL mode, 1: USB mode
   UINT16  usOperMode;  // 0: 自动,  1:手动
   UINT16  usConType;  // 0:   1:  2:  3:
   UINT16  usChannelSelect;  // 0:   1:  2:  3:   
   UINT16  usVoiceChannel;  // 0:   1:  2:  3:   
   /*start of 增加标记位记录业务开启 by s53329 at  20080718*/
   UINT16  usService;          // 1表示取消了， 0表示开启了
  /*end of 增加标记位记录业务开启 by s53329 at  20080718*/
/* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
   UINT16  usVoiceEnable;
/* END:   Added by s00125931, 2008/9/12 */
   /*start of  增加 西班牙 VOICE 指定接口选择功能 by s53329 at  20091017*/
    UINT16  usVoiceInterface;               //0:Auto    1: ADSL   2: HSPA CS
   /*end of  增加 西班牙 VOICE 指定接口选择功能 by s53329 at  20091017*/
} E220_INFO;

#define PROBE_MODE_PING         0
#define PROBE_MODE_DNS           1
#define PROBE_MODE_BOTH        2

/*start of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
typedef struct HspaInfo 
{
    int    iInstantMode;
    int    iProbeMode;               /*0－ping探测；1－dns探测；2－both*/
    int    iIcmpSpeedCyle;
    int    iSwitchConfirm;
    int    iDaHspaDelayCS;      /*voice dsl to hspa*/
    int    iDaHspaDelayPS;      /*data dsl to hspa*/
    int    iHspaDaDelayCS;      /*voice hspa to dsl*/
    int    iHspaDaDelayPS;      /*data hspa to dsl*/
	
    /*START -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/
    int    iHspaVoiceDomain;    /*0－cs；1－ps*/
    /*END -- 增加SIP走HSPA PS域功能by w00135358 at 20090222*/
    /*[0:1] 00 all disable; 
     *      11 all enable; 
     *      01 cs disable and ps enable;
     *      10 cs enable and ps disable;
     */
    char   cHspaVoiceServiceFlag;
    /* add mobile operator selection. <tanyin 2009.4.9> */
    char   cOperSelect[IFC_SMALL_LEN]; /* auto, or operator in operator list, ...*/
    char   cOperatorList[IFC_HOST_LEN]; /* e.g. CMCC|CNC|CUM */
	
} HSPA_INFO;

/*Begin:Added by luokunling l192527 for IMSI inform requirement,2011/9/24*/
typedef struct ImsiNotification
{

   int Imsi_Notification_Enable;
   
}IMSI_Notification,*PIMSI_Notification;
/*End:Added by luokunling l192527,2011/9/24*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
typedef struct HTTPInterceptVisible
{

   int iHttpInterceptVisible;
   
}HTTP_InterceptVisible,*PHTTP_InterceptVisible;
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
/*end of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
typedef struct WANINTERFACECFG
{
   UINT32   ulConnectionOrder ;  
   UINT32   ulBand;  
} VIB_WANINTERFACECFG;
typedef struct WANINTERFACECFGSTATS
{
    bool   bDataWorking ;  
    bool   bVoiceWorking;  
    char   cRegistedConnectionType[IFC_PASSWORD_LEN];
    int     iRegistedBand;
    char   cRegistedNetwork[IFC_SMALL_LEN];
    char  cRequestedQuality[IFC_SMALL_LEN];
    char  cAssignedQuality[IFC_SMALL_LEN];
    int    iSignalQuality;
} VIB_WANINTERFACECFGSTATS;


typedef struct WANDEVICEINFO
{
    char cCardIMSI[IFC_TINY_LEN];
    char cCardIMEI[IFC_TINY_LEN];
    char cManufacturer[IFC_SMALL_LEN];
    char cHardVersion[IFC_SMALL_LEN];
    char cSoftVersion[IFC_SMALL_LEN];
    char cSerialNumber[IFC_SMALL_LEN];
} VIB_WANDEVICEINFO;


/*GLB:s53329,start add for DNS*/
typedef  struct DNSCFGINFO
{
    char cDnsDomain[IFC_SMALL_LEN+1];
    char  cMac[IFC_SMALL_LEN];
}DNS_CFG, *PDNS_CFG;
/*GLB:s53329,end add for DNS*/
//add by l66195 for pppousb end

#endif
#endif   //  __IFC_DEFS_H

