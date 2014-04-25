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
//  Filename:       dbapi.h
//  Author:         Peter T. Tran
//  Creation Date:  04/29/04
//
******************************************************************************
//  Description:
//      Define the BcmDb_* functions.
//
*****************************************************************************/

#ifndef __DB_API_H__
#define __DB_API_H__

/********************** Include Files ***************************************/

#include "bcmtypes.h"
#include "bcmatmapi.h"
#include "ifcdefs.h"
#include "psidefs.h"
#include "systemlog.h"
#include "snmpstruc.h"
#include "secdefs.h"
#include "routedefs.h"
#include "ddnsapi.h"
#include "todapi.h"
#if SUPPORT_PORT_MAP
#include "pmapapi.h"
#endif
#ifdef SUPPORT_TR69C
#include "bcmtr69c.h"
#endif
#ifdef SUPPORT_IPSEC
#include "ipsectbl.h"
#endif
#ifdef PORT_MIRRORING
#include "engdbgdefs.h"
#endif
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
#include "vlandefs.h"
/*end vlan bridge, s60000658, 20060627*/
//#endif
#if SUPPORT_MAC //add by l66195 for VDF
#include "macapi.h"
#endif

#ifdef SUPPORT_MACMATCHIP 
#include "macmatchapi.h"
#endif

/* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
#if VOXXXLOAD
#include "vodslapi.h"
#include "sipdef.h"
#endif
/* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
#include "board_api.h"

#ifdef SUPPORT_SAMBA
#include "sambadefs.h"
#endif

#include "psixml.h"

#define DB_WAN_ADD_OK                     0
#define DB_WAN_ADD_OUT_OF_PVC             1
#define DB_WAN_ADD_OUT_OF_MEMORY          2
#define DB_WAN_NOT_FOUND                  3
#define DB_WAN_ADD_INVALID_QOS            4
#define DB_WAN_ADD_OUT_OF_QUEUES          5
#define DB_WAN_ADD_OUT_OF_CONNECTION      6
#define DB_WAN_ADD_EXISTED_PVC            7
#define DB_WAN_ADD_PPPOE_ONLY             8
#define DB_WAN_ADD_PPPOE_OR_MER_ONLY      9
#define DB_WAN_REMOVE_OK                  10
/*start: by xkf19988 2009.11.19*/
#define DB_ERROR    1000
/*end: by xkf19988 2009.11.19*/


#define DB_WAN_GET_OK                     1
#define DB_WAN_GET_NOT_FOUND              2
#define DB_VCC_GET_OK                     3
#define DB_VCC_GET_NOT_FOUND              4

#define DB_GET_OK                         5
#define DB_GET_NOT_FOUND                  6

/*start: by xkf19988 2009.11.11 */
#define ULEXPIRATION_MIN 	20 
#define ULEXPIRATION_MAX	65535
#define SESSIONEXPIRES_MIN		30
#define SESSIONEXPIRES_MAX		36000
#define MINSE_MIN			20
#define MINSE_MAX			1800
#define RETRY_TIME_MIN		16
#define RETRY_TIME_MAX		1800
#define RTP_START_PORT_MIN		50000
#define RTP_START_PORT_MAX		65514
#define RTP_END_PORT_MIN		50000
#define RTP_END_PORT_MAX		65514
#define REPTINTVAL_MIN			1000
#define REPTINTVAL_MAX			16000
#define LOCAL_PORT_MIN			1024
#define LOCAL_PORT_MAX			65535
#define DSCP_MIN				0
#define DSCP_MAX				63
/*end: by xkf19988 2009.11.11 */

#define WAN_PROTOCOL_FIELD                1
#define WAN_ADMIN_FIELD                   2
#define WAN_BRIDGE_FIELD                  3
#define WAN_MAC_LEARNING_FIELD            4
#define WAN_IGMP_FIELD                    5
#define WAN_NAT_FIELD                     6
#define WAN_DHCPC_FIELD                   7

#define FTP_PATH_LEN                      256
#define CAMERA_PATH_LEN                   1024
#define CAMERA_BOOL_LEN                   4

#define CONN_SWITCH_MODE_PLAIN      0  //不需要定时通过访问dns服务检测adsl连接的有效性
#define CONN_SWITCH_MODE_COMPLEX     1 //需要定时访问dns服务来检测adsl连接的有效性
typedef enum tagDnsDetactFlag
{
	BCM_ENUM_DNSDetact_Failed = 0,
	BCM_ENUM_DNSDetact_Well,
}BCM_ENUM_DNSDetactFlag;

typedef struct {
   char *varName;
   char *varValue;
} DB_DEFAULT_ITEM, *PDB_DEFAULT_ITEM;

typedef struct{
    char ftpEnable[4];
	char ftpUserName[IFC_TINY_LEN];
	char ftpUserPassWord[IFC_PASSWORD_LEN];
    char ftpPort[IFC_TINY_LEN];
	char ftpPath[FTP_PATH_LEN];
}FTP_INFO,*pFTP_INFO;
/*for sip*/

/*d00104343, USB Camera*/
#ifdef WEBCAM
typedef enum tag_CAMERA_ENABLE_TYPE_EN
{
    CAMERA_ENABLE_DISABLE,
    CAMERA_ENABLE_LOCAL  ,
    CAMERA_ENABLE_NETCAM   
}CAMERA_ENABLE_TYPE_EN;

typedef enum tag_CAMERA_REMOTE_ENABLE_EN
{
    CAMERA_REMOTE_DISABLE,
    CAMERA_REMOTE_ENABLE
}CAMERA_REMOTE_ENABLE_EN;


typedef struct tag_USB_CAMERA_INFO{
    uint8   enable;               //0/1使能USB Camera
    uint8   auto_brightness;      //0/1自动亮度使能开关
    uint8   framerate;                  //刷新率
    uint8   remote_enable;        //0/1作为网络设备，远程访问使能
    char    remote_pass[IFC_SMALL_LEN];            //user:pass作为网络设备，远程访问安全
    char    netcam_url[IFC_SMALL_LEN];           //Max 4095 characters(=后面为空就是本地camera)
    char    netcam_userpass[IFC_SMALL_LEN];        //user:pass
    uint32  height;                     //320*240, 352*288,192*144 三种，320*240为默认
    uint32  width;                      
    uint32  port;                       //port of webcam
    uint32  brightness;              //亮度0 - 255
}USB_CAMERA_INFO;
#endif
/*d00104343, USB Camera*/

#ifdef SUPPORT_POLICY
typedef struct tagWanRouteMapping
{
    WAN_CON_ID stWanId;
    int iRouteTableIndex;
    int iMark;
}WanRouteMapping;

extern WanRouteMapping gl_stPolicyRoutingMap[IFC_TINY_LEN];
#endif

#define SIP_DEBUG_FILE           "/var/sipdebug"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
    char *BcmDb_getDefaultValue(char *var);
    void BcmDb_removeWanInfo(PWAN_CON_ID pWanId);
    int  BcmDb_deleteWanInfo(PWAN_CON_ID pWanId, const int isInitialized);
    int  BcmDb_renameWanInfo( PWAN_CON_ID pOldWanId, PWAN_CON_ID pNewWanId );
    int  BcmDb_getAvailableWanConId(PWAN_CON_ID pWanId);
    int  BcmDb_getWanInfo(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanIfo);
    void BcmDb_setWanInfo(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanIfo);
    int  BcmDb_getWanInfoNext(PWAN_CON_ID pWanId, PWAN_CON_INFO pWanIfo);
    int  BcmDb_getPppSrvInfo(PWAN_CON_ID pWanId, PWAN_PPP_INFO pPppIfo);
    void BcmDb_setPppSrvInfo(PWAN_CON_ID pWanId, PWAN_PPP_INFO pPppIfo);
    int  BcmDb_getIpSrvInfo(PWAN_CON_ID pWanId, PWAN_IP_INFO pIpIfo);
    void BcmDb_setIpSrvInfo(PWAN_CON_ID pWanId, PWAN_IP_INFO pIpIfo);
    /* start of maintain PSI移植: 允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
    int BcmDb_getBrDhcpcInfo(PWAN_CON_ID pWanId, PWAN_BRDHCPC_INFO pBrInfo);
    void BcmDb_setBrDhcpcInfo(PWAN_CON_ID pWanId, PWAN_BRDHCPC_INFO pBrInfo);
    /* end of maintain PSI移植: 允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
    /*start of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
    int BcmDb_getHspaInfo(HSPA_INFO *pHspaInfo);
    void BcmDb_setHspaInfo(HSPA_INFO *pHspaInfo);
    /*end  of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
#ifdef SUPPORT_DHCPOPTIONS
int BcmDb_getDhcpOptionInfo(PDHCPOptions pstDhcpOption);
void BcmDb_setDhcpOptionInfo(PDHCPOptions pstDhcpOption); 
#endif
/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月11日 */
    int BcmDb_getAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade);
    void BcmDb_setAutoUpgradeInfo(PAUTO_UPGRADE_INFO pstAuUpgrade);
    /* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月11日 */
    int  BcmDb_getVccInfo(PWAN_CON_ID pWanId, PIFC_ATM_VCC_INFO pVccInfo);
    int  BcmDb_getVccInfoNext(UINT16 *pVccId, PIFC_ATM_VCC_INFO pVccInfo);
    int  BcmDb_getTdInfo(PWAN_CON_ID pWanId, PIFC_ATM_TD_INFO pTdInfo);
    void BcmDb_getWanInterfaceName(PWAN_CON_ID pWanId, UINT8 protocol, char *interface);
    void BcmDb_getWanProtocolName(UINT8 protocol, char *name);
    int  BcmDb_getNoBridgeNtwkPrtcl(void);
    int  BcmDb_isUbrPvcExisted(void);
    int  BcmDb_getNumberOfUsedPvc(void);
    int  BcmDb_getNumberOfUsedQueue(void);
    int  BcmDb_getNumberOfWanConnections(void);
    int  BcmDb_checkPvc(void);
    int  BcmDb_isBridgedWanExisted(void);
    int  BcmDb_isRoutedWanExisted(void);
    int  BcmDb_isFirewallEnabled(void);
    int  BcmDb_isInterfaceFirewallEnabled(char *interface);
    int  BcmDb_isNatEnabled(void);
    int  BcmDb_isInterfaceNatEnabled(char *interface);
    int  BcmDb_isIgmpEnabled(void);
    int  BcmDb_isQosEnabled(void);
    int  BcmDb_isWanExisted(PWAN_CON_ID pWanId, int protocol);
    int  BcmDb_isMultiplePppoeExisted(PWAN_CON_ID pWanId);
    int  BcmDb_isWanConnectionExisted(PWAN_CON_ID pWanId);
    int  BcmDb_isFirstPppoeOnPvc(PWAN_CON_ID pWanId);
    UINT32 BcmDb_getNewNtwkInstId(void);
    int  BcmDb_getLanInfo(UINT16 lanId, PIFC_LAN_INFO pLanInfo);
    void BcmDb_setLanInfo(UINT16 lanId, PIFC_LAN_INFO pLanInfo);
    void BcmDb_removeLanInfo(UINT16 lanId);
    int  BcmDb_getLanAddrMask(char *addr, char *mask);
    int  BcmDb_getNumberOfLanConnections(void);
    /*Begin:Added by luokunling l192527 for IMSI inform requirement,2011/9/24*/
    PSI_STATUS BcmDb_setIMSINotification(IMSI_Notification *imsinotification);
    int BcmDb_getIMSINotification(IMSI_Notification *imsinotification);
    /*End:Added by luokunling l192527,2011/9/24*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
     PSI_STATUS BcmDb_setHTTPInterceptVisible( HTTP_InterceptVisible *phttp_InterceptVisible);
    int BcmDb_getHTTPInterceptVisible(HTTP_InterceptVisible *phttp_InterceptVisible);
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
#ifdef SUPPORT_GLB_PVC
    int BcmDb_checkWan( void );
#endif /* SUPPORT_GLB_PVC */
#ifdef ETH_CFG
    int  BcmDb_getEthInfo(PETH_CFG_INFO pEthInfo);
    void BcmDb_setEthInfo(PETH_CFG_INFO pEthInfo);
#endif
    int BcmDb_getEthInfoEx(PETH_CFG_INFO pEthInfo, UINT32 ethId);
    void BcmDb_setEthInfoEx(PETH_CFG_INFO pEthInfo, UINT32 ethId);
    int  BcmDb_getSysLogInfo(BCM_SYSLOG_CB *pSysLog);
    void BcmDb_setSysLogInfo(BCM_SYSLOG_CB *pSysLog);
#ifdef SUPPORT_SNMP
    int  BcmDb_getSnmpInfo(BCM_SNMP_STRUCT *pSnmp);
    void BcmDb_setSnmpInfo(BCM_SNMP_STRUCT *pSnmp);
    void BcmDb_setSnmpSpecificInfo(char* newValue, int len, int specific);
#endif
#ifdef SUPPORT_TR69C
    int  BcmDb_getTr69cInfo(PBCM_TR69C_INFO pTr69c);
    void BcmDb_setTr69cInfo(PBCM_TR69C_INFO pTr69c);
#endif

#ifdef SUPPORT_GLB_ALG
int BcmDb_getNatAlgInfo(PNATALGINFO pNatAlgInfo) ;
void BcmDb_setNatAlgInfo(PNATALGINFO pNatAlgInfo) ;
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
int BcmDb_getAutoSerchInfo(PAUTOSERCHINFO pAutoSerchInfo) ;
void BcmDb_setAutoSerchInfo(PAUTOSERCHINFO pAutoSerchInfo) ;
/* END:   Added by y67514, 2008/10/27 */

 /*start of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */
void BcmDb_setHttpPortInfo(char  *cPort);
void BcmDb_getHttpPortInfo(char  *pcPort);
 /*end of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */
/*start  of 增加Radius 认证功能by s53329  at  20060714 */
void BcmDb_setRadiusInfo(PRADIUS_CLIENT_INFO pRadiusInfo) ;
int BcmDb_getRadiusInfo(PRADIUS_CLIENT_INFO pRadiusInfo);
/*end   of 增加Radius 认证功能by s53329  at  20060714 */

  /*start of 增加dhcp主机标识功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
void BcmDb_setDhcpHostNameInfo(char *hostName);
int BcmDb_getDhcpHostNameInfo(char *hostName);
#endif
/*end of 增加dhcp主机标识功能by l129990 */
/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#ifdef SUPPORT_IGMPCONFIG
int BcmDb_getIgmpProxyInfo(PIgmpProxyNode pstIgmpOption);
void BcmDb_setIgmpProxyInfo(PIgmpProxyNode pstIgmpOption);
/*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
/*start of增加igmp接口选择功能 by l129990 2008,9,23*/
void BcmDb_clearIgmpProxyInterface(char *wanIf);
/*end of增加igmp接口选择功能 by l129990 2008,9,23*/
#endif
    int  BcmDb_getDnsInfo(PIFC_DNS_INFO pDnsInfo);
/*start of 解决AU8D01490问题单:增加PPP IP Extension获取dns信息 by l129990,2008,12,16*/
    int BcmDb_getIpExtDnsInfo(char *interface, PIFC_DNS_INFO pDnsInfo);
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取dns信息 by l129990,2008,12,16*/
    void BcmDb_setDnsInfo(PIFC_DNS_INFO pDnsInfo);
    void BcmDb_removeDnsInfo(void);
    int  BcmDb_getDefaultGatewayInfo(PIFC_DEF_GW_INFO pGtwyInfo);
    void BcmDb_setDefaultGatewayInfo(PIFC_DEF_GW_INFO pGtwyInfo);
    void BcmDb_removeDefaultGatewayInfo(void);
    int  BcmDb_getSysFlagInfo(PSYS_FLAG_INFO pSysFlag);
    void BcmDb_setSysFlagInfo(PSYS_FLAG_INFO pSysFlag);
    int  BcmDb_getSpecificSysFlagInfo(int *value, int type);
    void BcmDb_setSpecificSysFlagInfo(int value, int type);
    int  BcmDb_getAutoScanInfo(void);
    void BcmDb_setAutoScanInfo(int autoScan);
    int  BcmDb_getEncodePasswordInfo(void);
    void BcmDb_setEncodePasswordInfo(int encodePassword);
#if SUPPORT_ETHWAN
    void BcmDb_setEnetWanInfo(int enetWan);
    int  BcmDb_getEnetWanInfo(void);
#endif
#ifdef SUPPORT_UPNP
    int  BcmDb_getUpnpInfo(void);
    void BcmDb_setUpnpInfo(int upnp);
#endif
    int  BcmDb_getIgmpSnpInfo(void);
    void BcmDb_setIgmpSnpInfo(int enable);
    int  BcmDb_getIgmpModeInfo(void);
    void BcmDb_setIgmpModeInfo(int enable);
//add by l66195 for VDF start
    int BcmDb_getDdnsEblInfo(void); 
    void BcmDb_setDdnsEblInfo(int ddns);
//add by l66195 for VDF end
    int  BcmDb_getMacPolicyInfo(void);
    void BcmDb_setMacPolicyInfo(int macPolicy);
//#ifdef SUPPORT_SIPROXD
/*Begin: Add para of sip proxy port, by d37981 2006.12.12. */
//    int  BcmDb_getSiproxdInfo(void);
//    void BcmDb_setSiproxdInfo(int siproxd);
    int  BcmDb_getSiproxdInfo(int *enable, UINT32 *portnumber);
    void BcmDb_setSiproxdInfo(int enable, UINT32 portnumber);
/*Endof: Add para of sip proxy port, by d37981 2006.12.12. */
//#endif
    int  BcmDb_getMgmtLockInfo(PSYS_MGMT_LOCK_INFO pMgmtLock);
    void BcmDb_setMgmtLockInfo(PSYS_MGMT_LOCK_INFO pMgmtLock);
    int  BcmDb_getLoginInfo(int id, char *info, int len);
    void BcmDb_setLoginInfo(int id, char *info, int len);
    int  BcmDb_getSysUserName(char *userName, int len);
    void BcmDb_setSysUserName(char *userName, int len);
    int  BcmDb_getSysPassword(char *password, int len);
    void BcmDb_setSysPassword(char *password, int len);
#ifndef SUPPORT_ONE_USER
    int  BcmDb_getSptUserName(char *userName, int len);
    void BcmDb_setSptUserName(char *userName, int len);
    int  BcmDb_getSptPassword(char *password, int len);
    void BcmDb_setSptPassword(char *password, int len);
    int  BcmDb_getUsrUserName(char *userName, int len);
    void BcmDb_setUsrUserName(char *userName, int len);
    int  BcmDb_getUsrPassword(char *password, int len);
    void BcmDb_setUsrPassword(char *password, int len);
#endif
    int  BcmDb_getIpProtocol(char *info);
    int  BcmDb_getEtheProtocol(char *info);//add by w00135358 for VDF GLOBAL QOS 20080908
    int  BcmDb_getMacProtocol(char *info);
    int  BcmDb_getSecuritySize(int id, int *info);
    void BcmDb_setSecuritySize(int id, int info);
    int  BcmDb_getSecurityInfo(int id, PSI_VALUE info, int size);
    void BcmDb_setSecurityInfo(int id, PSI_VALUE info, int size);
    int  BcmDb_getVirtualServerSize(int *info);
    void BcmDb_setVirtualServerSize(int size);
    int  BcmDb_getVirtualServerInfo(PSEC_VRT_SRV_ENTRY tbl, int size);
    void BcmDb_setVirtualServerInfo(PSEC_VRT_SRV_ENTRY tbl, int size);
    int  BcmDb_getOutgoingFilterSize(int *info);
    void BcmDb_setOutgoingFilterSize(int size);
    int  BcmDb_getOutgoingFilterInfo(PSEC_FLT_ENTRY tbl, int size);
    void BcmDb_setOutgoingFilterInfo(PSEC_FLT_ENTRY tbl, int size);
    int  BcmDb_getIncomingFilterSize(int *info);
    void BcmDb_setIncomingFilterSize(int size);
    int  BcmDb_getIncomingFilterInfo(PSEC_FLT_ENTRY tbl, int size);
    void BcmDb_setIncomingFilterInfo(PSEC_FLT_ENTRY tbl, int size);
    int  BcmDb_getPortTriggerSize(int *info);
    void BcmDb_setPortTriggerSize(int size);
    int  BcmDb_getPortTriggerInfo(PSEC_PRT_TRG_ENTRY tbl, int size);
    void BcmDb_setPortTriggerInfo(PSEC_PRT_TRG_ENTRY tbl, int size);
    int  BcmDb_getMacFilterDirection(char *info);
    int  BcmDb_getMacFilterSize(int *info);
    void BcmDb_setMacFilterSize(int size);
    int  BcmDb_getMacFilterInfo(PSEC_MAC_FLT_ENTRY tbl, int size);
    void BcmDb_setMacFilterInfo(PSEC_MAC_FLT_ENTRY tbl, int size);
    int  BcmDb_getQosClassSize(int *info);
    void BcmDb_setQosClassSize(int size);
    int  BcmDb_getQosClassInfo(PQOS_CLASS_ENTRY tbl, int size);
    void BcmDb_setQosClassInfo(PQOS_CLASS_ENTRY tbl, int size);
    int  BcmDb_getQosClassPriority(char *info);
    int  BcmDb_getQosClassDscpMark(char *info);
    int  BcmDb_getAccessControlSize(int *info);
    void BcmDb_setAccessControlSize(int size);
    int  BcmDb_getAccessControlInfo(PSEC_ACC_CNTR_ENTRY tbl, int size);
    void BcmDb_setAccessControlInfo(PSEC_ACC_CNTR_ENTRY tbl, int size);
    int  BcmDb_getAccessControlMode(int *info);
    void BcmDb_setAccessControlMode(int mode);
    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    int  BcmDb_getTrafficMode(int *info);
    void BcmDb_setTrafficMode(int info);
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif
#ifdef VDF_PPPOU

    // add begin by l66195 for pppousb 增加与psi交互函数
    int  BcmDb_getE220Info(UINT16 uE220Id, E220_INFO *pE220Info);
    void BcmDb_setE220Info(UINT16 uE220Id, E220_INFO *pE220Info);
    int  BcmDb_getPppProfileSize(int id, int *info);
    void BcmDb_setPppProfileSize(int id, int info);
    int  BcmDb_getPppProfileInfo(int id, PSI_VALUE info, int size);
    void BcmDb_setPppProfileInfo(int id, PSI_VALUE info, int size);
    int  BcmDb_getPppProfileCfgSize(int *info);
    void BcmDb_setPppProfileCfgSize(int size);
    int  BcmDb_getPppProfileCfgInfo(PROFILE_INFO_ST* tbl, int size);
    void BcmDb_setPppProfileCfgInfo(PROFILE_INFO_ST* tbl, int size);
    // add end by l66195 for pppousb

#endif
    void BcmDb_getDftSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo);
    int  BcmDb_getSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo);
    void BcmDb_setSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo);
    int  BcmDb_getSrvCntrAccess(char *info);
    /* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月28日 */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
    void BcmDb_getDftWredInfo(PSEC_WRED_INFO pInfo);
    int  BcmDb_getWredInfo(PSEC_WRED_INFO pInfo);
    void BcmDb_setWredInfo(PSEC_WRED_INFO pInfo);
#endif
#endif
    /* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月28日 */
/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
    int  BcmDb_getDmzHost(char *enable, char *address);
    void BcmDb_setDmzHost(PSEC_DMZ_ENTRY pInfo);
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
    int  BcmDb_getRouteSize(int id, int *info);
    void BcmDb_setRouteSize(int id, int info);
    int  BcmDb_getRouteInfo(int id, PSI_VALUE info, int size);
    void BcmDb_setRouteInfo(int id, PSI_VALUE info, int size);
    int  BcmDb_getStaticRouteSize(int *info);
    void BcmDb_setStaticRouteSize(int size);
    int  BcmDb_getStaticRouteInfo(PRT_ROUTE_CFG_ENTRY tbl, int size);
    void BcmDb_setStaticRouteInfo(PRT_ROUTE_CFG_ENTRY tbl, int size);
    int  BcmDb_getRipInfo(PRT_RIP_CFG rip);
    void BcmDb_setRipInfo(PRT_RIP_CFG rip);
    int  BcmDb_getRipIfcInfo(PRT_RIP_IFC_ENTRY tbl, int size);

/*GLB:s53329,start add for DNS*/
    int BcmDb_getDnsdomainSize(int *info);
    void BcmDb_setDnsdomainSize(int info);
    int BcmDb_getDnsDomainInfo(PDNS_CFG info, int size) ;
    void BcmDb_setDnsDomainInfo(PDNS_CFG info, int size);
/*GLB:s53329,end add for DNS*/

    void BcmDb_setRipIfcInfo(PRT_RIP_IFC_ENTRY tbl, int size);
    int  BcmDb_getRipOperation(char *info);
    int  BcmDb_getRipVersion(char *info);
    //add by z67625
    int BcmDb_restoreDefault(void);
    void BcmDb_getLanAllStats(unsigned long sts[]);
    void BcmDb_getWanAllStats(unsigned long sts[]);
    void BcmDb_getIfcStats(int from, char *device, unsigned long sts[]);
    void BcmDb_getSpecIfcStats(int from, char *device, int field, unsigned long sts[]);
#ifdef SUPPORT_DDNSD
    int  BcmDb_getDdnsSize(int *info);
    void BcmDb_setDdnsSize(int size);
    int  BcmDb_getDdnsInfo(PDDNS_SERVICE_INFO tbl, int size);
    void BcmDb_setDdnsInfo(PDDNS_SERVICE_INFO tbl, int size);
#endif
#ifdef SUPPORT_TOD
    void BcmDb_setToDInfo(PTOD_ENTRY pToDInfo, int size);
    void BcmDb_setToDSize(int size);
#endif
#ifdef SUPPORT_MAC  //add by l66195 for VDF
    void BcmDb_setMacInfo(PMAC_ENTRY pMacInfo, int size);
    void BcmDb_setMacSize(int size);
#endif
#ifdef SUPPORT_MACMATCHIP  
    void BcmDb_setMacMatchInfo(PMACMATCH_ENTRY pMacInfo, int size);
    void BcmDb_setMacMatchSize(int size);
#endif
#ifdef SUPPORT_PORT_MAP
    char BcmDb_checkIfcString(char *list, char *ifcname, char **loc);
    void BcmDb_addBridgeIf(char *bridgeName, char *ifName);
    void BcmDb_getBridgeInfo(const char *bridgeName, char *ifList, int len);//modified by z67625 for compile
    void BcmDb_delBridgeIf(const char *bridgeName, char *ifName);
    void BcmDb_delBridge(const char *bridgeName);
    char *BcmDb_getBridgeInfoByIndex(int index, char *groupName, char *ifList);
    void BcmDb_delIfFromBridge(char *ifName);
    int  BcmDb_getPMapSize(int id, int *info);
    void BcmDb_setPMapSize(int id, int info);
    void BcmDb_setPMapInfo(int id, PSI_VALUE info, int size);
    void  BcmDb_setStaticPMapSize(int size);
    void BcmDb_setStaticPMapInfo(PPORT_MAP_CFG_ENTRY tbl, int size);
    int BcmDb_getPMapIfcCfgSts(char *ifName, int status);
    int BcmDb_setPMapIfcCfgSts(char *ifName, int status);
    void BcmDb_setPMapIfcCfgInfo(PPORT_MAP_IFC_STATUS_INFO pPMapIfcCfgInfo);
    int  BcmDb_allocBridge(char *grpName, char *brIfName);
    int  BcmDb_getSwitchIfcInfo(char *ifName);
    void BcmDb_getDHCPVendorId(int index, char *vendorid, char *grpName);
    void BcmDb_setDHCPVendorId(int index, char *vendorid, char *grpName);
    void  BcmDb_setStaticPMapDhcpVendIdSize(int size);
    void BcmDb_setStaticPMapDhcpVendIdInfo(PPORT_MAP_VENDORID_CFG_ENTRY tbl, int size);
    void BcmDb_getDhcpVendorGroupName(char *vendorid, char *groupName);
    void BcmDb_removePMapIfcCfgInfo(void);
    void BcmDb_getDynIntf(char *groupName, char *dynIntf, int len);
    void BcmDb_addBridgeDynIf(char *groupName, char *dynIntf);    
    int BcmDb_checkIfInBridge(char *ifName, char *bridgeName);
     /*start of 增加 wl 接口设备信息 by s53329 at  20081104*/
    void BcmDb_findGrByIfc(char * ifc, char *grp);
      /*end of 增加 wl 接口设备信息 by s53329 at  20081104*/
#endif

    int BcmDb_getVccInfoByVpiVci(int vpi, int vci, PIFC_ATM_VCC_INFO pVccInfo);
    void BcmDb_removeVcc(int port, int vpi, int vci);
    int BcmDb_addVcc(IFC_ATM_VCC_INFO *pNewVccInfo); 
    int BcmDb_setVccInfo(int vccId, PIFC_ATM_VCC_INFO pVccInfo);
    int BcmDb_configWanMode(PWAN_CON_ID pWanId, int type, int value);
    void BcmDb_wanIfToPvc(char *wanIf, char *pvc);
	//BEGIN:add by zhourongfei to config web
	void BcmDb_getWebDefaultInfo(WEB_CFG_INFO *pWebInfo);
	//END:add by zhourongfei to config web 
    void BcmDb_getSysLogDefaultInfo(BCM_SYSLOG_CB *pSysLog);
/* start of 增加tr143 功能*/
    void Bcm_Db_getUdpEchoDefaultInfo(DIAG_TR143_UDP *flag);
    void Bcm_Db_getUploadDefaultInfo(DIAG_TR143_UP * flag);
    void Bcm_Db_getDownDiagInfo(DIAG_TR143_Down * flag);
/* end of 增加tr143 功能*/
#ifdef PORT_MIRRORING
    
   int BcmDb_getPortMirrorStatus (DbgPortMirrorStatus* pm) ;
   void BcmDb_setPortMirrorStatus(DbgPortMirrorStatus* pm) ;
   int BcmDb_getPortMirrorCfg (PortMirrorCfg* tbl, int size) ;
   void BcmDb_setPortMirrorCfg(PortMirrorCfg* tbl, int size) ;
   int BcmDb_getPortMirrorCfgRecNo(char *info) ;
   int BcmDb_getPortMirrorCfgMonitorPort(char *info) ;
   int BcmDb_getPortMirrorCfgDir (char *info) ;
   int BcmDb_getPortMirrorCfgFlowType (char *info) ;
   int BcmDb_getPortMirrorCfgMirrorPort (char *info) ;
   int BcmDb_getPortMirrorCfgStatus (char *info) ;
   int BcmDb_getPortMirrorInfo (int id, PSI_VALUE info, int size) ;
   void BcmDb_setPortMirrorInfo (int id, PSI_VALUE info, int size) ;

#endif

//start for vdf qos by p44980 2008.01.08
#ifdef SUPPORT_VDF_QOS
int BcmDb_getQosQueueSize(int *info);
void BcmDb_setQosQueueSize(int info);
int BcmDb_getQosQueueInfo(PQOS_QUEUE_ENTRY tbl, int size);
void BcmDb_setQosQueueInfo(PQOS_QUEUE_ENTRY tbl, int size);
int BcmDb_getQosCfgInfo(int *pbEnable);
void BcmDb_setQosCfgInfo(int bEnable);

int BcmDb_getQosServiceSize(int *info);
void BcmDb_setQosServiceSize(int info);
int BcmDb_getQosServiceInfo(PQOS_SERVICE_CFG_ENTRY tbl, int size);
void BcmDb_setQosServiceInfo(PQOS_SERVICE_CFG_ENTRY tbl, int size);

/*start-w00135358 add for VDF Global QoS 20080904*/
int  BcmDb_getPortRange(char *ports, int *minPort, int *maxPort);
void BcmDb_setPortRange(char *ports, int  minPort, int maxPort);
/*end-w00135358 add for VDF Global QoS 20080904*/

#endif
//end for vdf qos by p44980 2008.01.08

/*start of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/
void BcmDb_GetBrInfoByWanId(char ** ppBrName,UINT16 vpi,UINT16 vci,UINT16 conId);
int BcmDb_isAnyBridgeDhcpcEnable(PWAN_CON_ID  pWanId) ;
/*end of protocol WAN <3.4.5桥使能dhcp> porting by shuxuegang 60000658 20060504*/

//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/
VLAN_8021Q_PORT_ENTRY* BcmDb_getVlanPort(char *portName);
VLAN_8021Q_PORT_ENTRY* BcmDb_getVlanPortHead(void);
VLAN_8021Q_PORT_ENTRY* BcmDb_regVlanPort(char *portName); 
VLAN_8021Q_ITEM_ENTRY* BcmDb_getVlanbyVID(unsigned short vid);
VLAN_8021Q_ITEM_ENTRY* BcmDb_getVlanbyNo(int vlanNo);
VLAN_8021Q_ITEM_ENTRY* BcmDb_setVlanbyVID(VLAN_8021Q_ITEM_ENTRY* pvlan);
VLAN_8021Q_ITEM_ENTRY* BcmDb_setVlanbyNo(VLAN_8021Q_ITEM_ENTRY* pvlan);
VLAN_8021Q_ITEM_ENTRY* BcmDb_addVlan(VLAN_8021Q_ITEM_ENTRY* pvlan);
VLAN_8021Q_CFG_ENTRY*  BcmDb_getVlanCfg(void);
void BcmDb_unregVlanPort(char *portName);
void BcmDb_removeVlanbyVID(unsigned short vid);
int  BcmDb_addPorttoVlan(unsigned short vid, char* portName);
int  BcmDb_removePortFromVlan(unsigned short vid, char* portName);
void BcmDb_removeVlanbyNo(int vlanNo);
int  BcmDb_applyVlanCfg(void);
int  BcmDb_applyVLPortAttr(void);
int  BcmDb_applyVltbCfg(void);
int BcmDb_loadVLPortAttr(void);
int BcmDb_loadVltbCfg(void);
int BcmDb_loadVlanCfg(void);
void BcmDb_makeVlanEffec(void);
void BcmDb_makeVLPortAttrEffec(void);
void BcmDb_setVlanToEthChip(char *ifList);
/*end vlan bridge, s60000658, 20060627*/
//#endif

/* start of voice 增加SIP的配置项 by z45221 zhangchen 2006年11月7日 */
#ifdef VOXXXLOAD
void BcmDb_setVoiceSize(int id, int info);
int BcmDb_getVoiceSize(int id, int *info);
void BcmDb_setVoiceInfo(int id, PSI_VALUE info, int size) ;
int BcmDb_getVoiceInfo(int id, PSI_VALUE info, int size);
void BcmDb_setSipSrvSize(int info);
int BcmDb_getSipSrvSize(int *info);
void BcmDb_setSipSrvInfo(PSIP_WEB_SERVER_S tbl, int size);
int BcmDb_getSipSrvInfo(PSIP_WEB_SERVER_S tbl, int size);
void BcmDb_setSipBscSize(int info);
int BcmDb_getSipBscSize(int *info);
void BcmDb_setSipBscInfo(PSIP_WEB_USER_INFO_S tbl, int size);
int BcmDb_getSipBscInfo(PSIP_WEB_USER_INFO_S tbl, int size);
void BcmDb_setSipQosInfo(PSIP_WEB_QOS_CFG_S info);
int BcmDb_getSipQosInfo(SIP_WEB_QOS_CFG_S *info);
/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
void BcmDb_setSipProvInfo(PSIP_WEB_PROV_CFG_S info);
int BcmDb_getSipProvInfo(SIP_WEB_PROV_CFG_S *info);
/* END:   Added by y67514, 2009/11/16 */
void BcmDb_setSipAdvInfo(PSIP_WEB_ADV_CFG_S info);
int BcmDb_getSipAdvInfo(SIP_WEB_ADV_CFG_S *info);
void BcmDb_getSipAdvDefaultValue(SIP_WEB_ADV_CFG_S *info);
void BcmDb_getSipBscDefaultValue(SIP_WEB_USER_INFO_S *tbl);
void BcmDb_getSipSrvDefaultValue(SIP_WEB_SERVER_S *tbl);
void BcmDb_getPhoneCommonDefaultValue(SIP_WEB_PHONE_CFG_S *info);
int BcmDb_getPhoneCommonInfo(SIP_WEB_PHONE_CFG_S *info);
void BcmDb_setPhoneCommonInfo(SIP_WEB_PHONE_CFG_S *info);

extern SIP_WEB_PHONE_CFG_S* HWDb_getPhoneCommon(void);
extern VOS_VOID HWDb_applyPhoneCommon(void);
extern VOS_VOID HWDb_applyPhoneFlashCommon(CAS_CTL_FXO_PARMS fxoParams);     
extern VOS_VOID HWDb_ShowPhoneFlashInfo(void);      
extern VOS_VOID HWDb_loadPhoneCommon(void);
extern SIP_WEB_DIAL_PLAN_S* HWDb_getAllDialPlan(void);
extern VOS_VOID HWDb_applyAllDialPlan(void);
extern VOS_VOID HWDb_loadAllDialPlan(void);
extern SIP_WEB_FWD_S* HWDb_getAllCallForwardInfo(void);
extern VOS_VOID HWDb_applyAllCallForwardInfo(void);
extern VOS_VOID HWDb_loadAllCallForwardInfo(void);

extern int SipSetDebugStatus(VOS_UINT32 ulUeSn, VOS_UINT32 ulStatus);

extern void HWDb_writeSipSrvToVar(PSIP_WEB_SERVER_S pstSipServer);
extern void HWDb_writeSipBscToVar(PSIP_WEB_USER_INFO_S pstSipBasic);
extern void HWDb_writeSipAdvToVar(PSIP_WEB_ADV_CFG_S pstSipAdv);
extern void HWDb_writeSipQosToVar(PSIP_WEB_QOS_CFG_S pstSipQos);
extern void HWDb_writePhoneCommonToVar(void);
extern void HWDb_writeSpeedDialToVar(void);
extern void HWDb_writeCallForwardToVar(void);

extern VOS_UINT32  SipGetServiceStatus(VOS_UINT32 ulUeSn, VOS_UINT32 *pulStatus);

extern VOS_UINT32 VoiceGetSipAdvFromVar(SIP_WEB_ADV_CFG_S *pstSipAdv);

/* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
VOS_UINT32 VoiceGetFxsStatusFromVar( VOS_UINT32 ulUeSn, VOS_UINT32 * pStatus );
//void VoiceSetFxsStatusInfoToVar( VOS_UINT32 ulIndex, VOS_UINT32 * pStatus );
/* END:   Modified by p00102297, 2008/3/11 VDF C02*/
#endif

void AtpDb_getFTPInfo(FTP_INFO* pFtpInfo);
void AtpDb_setFTPInfo(FTP_INFO* pFtpInfo);

/* BEGIN: Added by hKF20513, 2010/1/29   PN:增加Upnp Video控制*/
int AtpDb_getUpnpVEnable(PUPNP_VIDEO_INFO pstUpnpVideoInfo);
void AtpDb_setUpnpVEnable(PUPNP_VIDEO_INFO pstUpnpVideoInfo);
/* END:   Added by hKF20513, 2010/1/29 */

/* BEGIN: Added by zhourongfei to config web */
void BcmDb_setWebInfo(WEB_CFG_INFO *pWebInfo);
int BcmDb_getWebInfo(WEB_CFG_INFO *pWebInfo);
/* END: Added by zhourongfei to config web */

#ifdef WEBCAM
PSI_STATUS HWDb_getCameraInfo(USB_CAMERA_INFO* pCameraInfo);
PSI_STATUS HWDb_setCameraInfo(USB_CAMERA_INFO* pCameraInfo);
#endif

void HWDb_setVDFWanIfCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo);
int HWDb_getVDFWanIfCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo);
void HWDb_setVIBInterfaceCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo);
int HWDb_getVIBInterfaceCfgInfo(PBCMCFM_VIBINTERFACECFG pVibInfo);

/* end of voice 增加SIP的配置项 by z45221 zhangchen 2006年11月7日 */
#ifdef SUPPORT_SAMBA
   void BcmDb_setSmbCfgInfo(PSAMBA_MGMT_CFG pSmbMgmtCfg);
   void BcmDb_setSmbAccntSize(int info);
   void BcmDb_setSmbAccntInfo(PSAMBA_ACCNT_ENTRY tbl, int size);
#endif
/* j00100803 Add Begin 2008-02-27 */
#ifdef SUPPORT_POLICY
int BcmDb_getRtPolicySize(int *info);
void BcmDb_setRtPolicySize(int info);
int BcmDb_getRtPolicyInfo(PIFC_RoutePolicy_Info pRtPolicy, int size);
void BcmDb_setRtPolicyInfo(PIFC_RoutePolicy_Info pRtPolicy, int size);
void BcmDb_setRtPolicy(int id, PIFC_RoutePolicy_Info *pRtPolicyInfo);
void BcmDb_getRtPolicy(int id, PIFC_RoutePolicy_Info *pRtPolicyInfo);
int BcmDb_getRtMappingIndex(char * pWanIfcName, unsigned int * pIndex);
int BcmDb_getRtMappingRouteTable(unsigned int uIndex, unsigned int * pTable);
int BcmDb_getRtMappingMark(unsigned int uIndex, int * pMark);
int BcmDb_setRtMappingIfcName(int iIndex, WAN_CON_ID * pWanIfcId);
void BcmDb_initRtMapping();
/*start of VDF 2008.4.23 V100R001C02B013 j00100803 AU8D00546 */
int BcmDb_getIfcNameForIgmp(void);
/*end of VDF 2008.4.23 V100R001C02B013 j00100803 AU8D00546 */
#endif
/* j00100803 Add End 2008-02-27 */
#pragma PRG_SECFW
extern void BcmDb_setSecFwCfg(unsigned short usLevel);
extern void BcmDb_getSecFwCfg(unsigned short *pusLevel);
extern void BcmDb_setSecFwLevel(unsigned short usLevel) ;
extern unsigned short BcmDb_getSecFwLevel(void) ;
extern int  BcmDb_setSecFwEntry(PSEC_FW_ENTRY pEntry, int optType);
extern int  BcmDb_getSecFwEntry(PSEC_FW_ENTRY pEntry, int optType);
extern int  BcmDb_getNextSecFwEntry(PSEC_FW_ENTRY pEntry, PSEC_FW_ENTRY pNext);
extern int  BcmDb_removeSecFwEntry(PSEC_FW_ENTRY pEntry, int optType);
extern void BcmDb_setSecFwSize(int info) ;
extern void BcmDb_setSecFwInfo(PSEC_FW_ENTRY tbl, int size) ;
extern int  BcmDb_getSecFwValidInstID(void);
/*add by z67625 设置过滤使能开关函数声明 start*/
extern void BcmDb_setSecEnbl(int value, int type);
extern int BcmDb_getSecEnbl(int type);
extern void BcmDb_setSecEnblCfg(PBcmCfm_SecEnblCfg_t value);
extern void BcmDb_getSecEnblCfg(PBcmCfm_SecEnblCfg_t value);
/*add by z67625 设置过滤使能开关函数声明 end*/
#pragma PRG_SECDOS
extern void BcmDb_setSecDosInfo(PSEC_DOS_ENTRY pEntry);
extern void BcmDb_getSecDosInfo(PSEC_DOS_ENTRY pEntry);
extern int  BcmDb_setSecDosEntry(PSEC_DOS_ENTRY pEntry);
extern int  BcmDb_getSecDosEntry(PSEC_DOS_ENTRY pEntry);
#pragma PRG_SECURLBLK
extern int BcmDb_setSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType);
extern int BcmDb_getSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType);
extern int BcmDb_getNextSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, PSEC_URLBLK_ENTRY pNext);
extern int BcmDb_removeSecUrlBlkEntry(PSEC_URLBLK_ENTRY pEntry, int optType);
extern void BcmDb_setSecUrlBlkSize(int info) ;
extern void BcmDb_setSecUrlBlkInfo(PSEC_URLBLK_ENTRY tbl, int size) ;
extern int BcmDb_getSecUrlBlkValidInstID(void);
/*begin add by p44980 2008.10.27*/
int BcmDb_getConnSwitchModeInfo(void);
BCM_ENUM_DNSDetactFlag BcmDb_IsDnsStateUp(void);
/*end add by p44980 2008.10.27*/

/*Start -- w00135358 modify for HG556a- store Hareware Version - 20090720*/
int  BcmDb_getHwVersion(char *pHwVersion);
/*End -- w00135358 modify for HG556a- store Hareware Version - 20090720*/
#if defined(__cplusplus)
}
#endif

#endif
