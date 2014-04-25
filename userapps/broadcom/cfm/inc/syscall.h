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
//  Filename:       syscall.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the global constants, and functions for sysCall utility.
//
*****************************************************************************/

#ifndef __SYS_CALL_H__
#define __SYS_CALL_H__
#include <netinet/in.h>
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
#include "ifcdefs.h"
#endif
/*w44771 add for 第一IP支持5段地址池，end*/
#ifdef SUPPORT_MACMATCHIP
#include "macmatchapi.h"
#endif
#ifndef VDF_RESERVED
#define VDF_RESERVED
#endif
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

/********************** Global Functions ************************************/

#define FILE_OPEN_OK			0
#define FILE_OPEN_ERR			-1

#define MNTR_STS_OK			0
#define MNTR_STS_ADSL_DOWN		1
#define MNTR_STS_PPP_AUTH_ERR		2
#define MNTR_STS_PPP_DOWN		3
#define MNTR_STS_OPEN_ERR		4
#define MNTR_STS_DHCPC_PROGRESS  	5
#define MNTR_STS_DHCPC_FAIL   		6
#define MNTR_STS_ADSL_TRAINING		7
#define MNTR_STS_BRIDGE       		8

#define DHCPC_STS_FAIL           	0
#define DHCPC_STS_SUCCESS        	1
/*start of 修改wan 状态判断函数 by s53329  at  20080311*/
#define DHCP_STS_UP                    6
/*end  of 修改wan 状态判断函数 by s53329  at  20080311 */
#define DHCPC_FAIL_MAX           	15

// Values for network protocol
#define PROTO_PPPOE			0
#define PROTO_PPPOA			1
#define PROTO_MER			2
#define PROTO_BRIDGE			3
#define PROTO_PPPOE_RELAY	        4
#define PROTO_IPOA			5
#define PROTO_IPOWAN                    6
#ifdef VDF_PPPOU
#define PROTO_PPPOU            7 //add by sxg, pppousb
#endif
#define PROTO_NONE 			10

#define SYSTEM_APPID       "SystemInfo"
#define SYS_USER_NAME_ID   1
#define SYS_PASSWORD_ID    2
#define SPT_USER_NAME_ID   3
#define SPT_PASSWORD_ID    4
#define USR_USER_NAME_ID   5
#define USR_PASSWORD_ID    6
#define SYS_DFT_GTWY_ID    7
#define SYS_DNS_ID         8
#define SYS_LOG_ID         9
#define SYS_SNMP_ID        10
#define SYS_FLAG_ID        11
#define SYS_TR69C_ID       12
#define SYS_MGMT_LOCK_ID   13
/*start  of 增加Radius 认证功能by s53329  at  20060714 */
#define SYS_RADIUS_ID           14
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
#define SYS_SIPROXD_ID		15
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/
#define FTP_USER_INFO		16
#define CAMERA_USER_INFO    17
#define HTTP_PORT                  18
/* BEGIN: Added by y67514, 2008/9/3   问题单号:GLB:增加NAT ALG的开关*/
#define NAT_ALG                         19
/* END:   Added by y67514, 2008/9/3 */
/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
#define AUTO_SEARCH                         20
/* END:   Added by y67514, 2008/10/27 */
/* BEGIN: Added by hKF20513, 2010/1/29   PN:增加Upnp Video控制*/
#define SYS_UPNPV_ID                         21
/* END:   Added by hKF20513, 2010/1/29 */
/*BEGIN: added by zkf34753 zhourongfei at 2011/5/30*/
#define SYS_WEB_ID			22
/*END: added by zkf34753 zhourongfei to config web at 2011/5/30*/

 /*Begin:Added by luokunling l192527 for IMSI inform,2011/9/23*/
#define IMSI_INFORM_ID         23
/*End:Added by luokunling l00192527,2011/9/22*/
/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
#define SYS_HTTPINTERCEPT_ID         24
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
/*GLB:s53329,start add for DNS*/
#define MAC_BCAST_ADDR	"\xff\xff\xff\xff\xff\xff"
#define  DNS_SIZE                    1
#define  DNS_INFO                   2
/*GLB:s53329,end add for DNS*/
// Values that are used in syscall.c
/* start of protocol QoS for TdE cmd by z45221 zhangchen 2006年9月15日
#define SYS_CMD_LEN        256
*/
#define SYS_CMD_LEN        512
/* end of protocol QoS for TdE cmd by z45221 zhangchen 2006年9月15日 */

#define IP_TBL_COL_NUM     0
#define IP_TBL_COL_PKTS    1
#define IP_TBL_COL_BYTES   2
#define IP_TBL_COL_TARGET  3
#define IP_TBL_COL_PROT    4
#define IP_TBL_COL_OPT     5
#define IP_TBL_COL_IN      6
#define IP_TBL_COL_OUT     7
#define IP_TBL_COL_SRC     8
#define IP_TBL_COL_DST     9
#define IP_TBL_COL_MAX     10

/* start of maintain 增加对PTM 的支持 by zhangliang 60003055 2006年7月7日" */
//#ifdef SUPPORT_VDSL                            
#define TM_ATM              0
#define TM_PTM_VLAN         1
#define TM_PTM_PRTCL        2
#define TM_ATM_BR           3
#define TM_PTM_BR           4
#define TM_EWAN_PRTCL     5
#define TM_EWAN_VLAN       6

#define TM_PTM_VPI          0
#define TM_PTM_VCI          32

/* end of maintain 增加对PTM 的支持 by zhangliang 60003055 2006年7月7日" */

#define UPNP_IP_ADDRESS    "239.255.255.250"


#define FTP_CLIENT_TINY_SIZE    48
#define FTP_CLIENT_LARGE_SIZE   512
#define FTP_CLIENT_MID_SIZE     256


/*本地web camera视频流的端口号*/
#define   WEBCAM_PORT_NUM 	       18080
/*当前web camera的业务配置文件*/
#define   WEBCAM_CURCFG_FILE       "var/motion/curcfg.conf" 
/*拍照的照片存储路径及文件名*/
#define   WEBCAM_SNAPSHOT_FILE     "var/motion/photocap.jpg"
/*USB Camera pid 文件*/
#define   WEBCAM_PROESS_PID_FILE   "/var/motion/motion.pid"

#define WAN_DATA_INTERFACE         "data"
#define WAN_DATA_VOICE_INTERFACE   "data/voice"
#define WAN_HSPA_INTERFACE         "ppp_255_65535_1"
#define WAN_VOICE_SERVICENAME      "Voice"
#define WAN_VIDEO_INTERFACE        "Video"

/* WLAN 芯片类型 */
typedef enum  {
	E_RALINK_WLAN = 0,        
	E_ATHEROS_WLAN = 1,
	E_BCM_WLAN = 2,
}E_WLAN_VENDOR_TYPE;

#define SHELL_CMD_HW_VERSION_ATHEROS "HG55VDFA"
#define SHELL_CMD_HW_VERSION_RALINK  "HG56BZRB"


#ifdef SUPPORT_GLB_MTU
#define MIN_MTU 296
#define MAX_MTU 1500
#define PPP_MAX_MTU 1492
#endif /* SUPPORT_GLB_MTU */
/* BEGIN: Added by y67514, 2008/11/1   PN:GLB:KeepAlive需求*/
/*Start of 问题单AU8D01488:ppp keepalive报文发送间隔的配置范围为10~60s by l129990,2008,12,15*/
#define MIN_LCP_ECHO_INT    10
#define MAX_LCP_ECHO_INT    60
/*end of 问题单AU8D01488:ppp keepalive报文发送间隔的配置范围为10~60s by l129990,2008,12,15*/
/* END:   Added by y67514, 2008/11/1 */
#ifdef VDF_PPPOU

/*start, ppp over usb, add by sxg*/
#ifndef ATCFG_PROFILE  
#define ATCFG_PROFILE "/var/at_profile"
#endif
#ifndef _AT_FILES_H_
/*运营商选择方式*/
enum
{
	ATCFG_OPTOR_AUTO   = 0,                     //手自动选择运营商
	ATCFG_OPTOR_MANUAL                          //动选择运营商
};
/*链接类型*/
enum
{
    ATCFG_CONNTYPE_GPRS_FIRST = 0,              //GPRS优先
    ATCFG_CONNTYPE_3G_FIRST,                    // 3G优先
    ATCFG_CONNTYPE_GPRS_ONLY,                   //仅仅使用GPRS
    ATCFG_CONNTYPE_3G_ONLY,                       //仅仅使用3G
    ATCFG_CONNTYPE_AUTO                             //自动
};
/*通道选择类型*/
/*
enum
{
    ATCFG_CHANNEL_UNLIMITED = 0,                //通道不限制
    ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100,   //GSM900/GSM1800/WCDMA2100
    ATCFG_CHANNEL_GSM1900                        //GSM1900
};
*/
#endif
/*end, ppp over usb, add by sxg*/
#endif

#if defined(__cplusplus)
extern "C" {
#endif
#define bcmSystem(cmd)		bcmSystemEx (cmd,1)
#define bcmSystemMute(cmd)	bcmSystemEx (cmd,0)
int bcmSystemEx (char *command, int printFlag);
int bcmSystemNoHang (char *command);
int bcmSystemNoHangForIptables(char *command);
int bcmSystemNice (char *command, int nice) ;

void setDlnaStatus(int status);
/*END: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/
void startDlna(char *cDlnaDevice );
void restartDlna(char *cDlnaDevice );
void stopDlna(void );
/*END: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/
int bcmGetPid (char *command);
int bcmGetPidForFork(char * command);
int *bcmGetPidList(char * command);
/*start DHCP Server支持第二地址池, s60000658, 20060616*/
/*int bcmCreateDhcpCfg(char *ipAddr, char *mask, char *addrStart, char *addrEnd,
                     char *dns1, char *dns2, int leasedTime, int protocol,
                     int enblNat, int enblFirewall);*/
/*end DHCP Server支持第二地址池, s60000658, 20060616*/
/*w44771 add for 第一IP支持5段地址池，begin*/
/*j00100803 Add Begin 2008-04-15*/
#ifdef SUPPORT_VDF_DHCP
int bcmGetSubMaskWidth(int ipAddr);
#endif
/*j00100803 Add End 2008-04-15*/
#ifdef SUPPORT_DHCP_FRAG
int bcmCreateDhcpCfg(char *ipAddr, char *mask, char *addrStart, char *addrEnd, int leasedTime,
                     char *ipAddr2, char *mask2, char *addrStart2, char *addrEnd2, int leasedTime2,
                     char * classid2, char *dns1, char *dns2, 
              #ifdef SUPPORT_PORTMAPING
                    char *option60,
              #endif
		#ifdef	VDF_RESERVED
			char *reservedIp,
		#endif	  
        #ifdef SUPPORT_MACMATCHIP
            PMACMATCH_ENTRY macmatch_entry,
            int   num_entries,
        #endif
		#ifdef SUPPORT_VDF_DHCP
            char * ntp1,
            char * ntp2,
            unsigned char * pOption121,
            char * pOption43,
        #endif
                    int protocol, int enblNat, int enblFirewall,
                     int enblSer1, int enblSer2, PIFC_LAN_INFO lanInfo);
#else
int bcmCreateDhcpCfg(char *ipAddr, char *mask, char *addrStart, char *addrEnd, int leasedTime,
                     char *ipAddr2, char *mask2, char *addrStart2, char *addrEnd2, int leasedTime2,
                     char * classid2, char *dns1, char *dns2, char * pOption66,char * pOption67,char * pOption160,
            #ifdef SUPPORT_PORTMAPING
                    char *option60,
            #endif
		#ifdef	VDF_RESERVED
				char *reservedIp,
		#endif	
        #ifdef SUPPORT_MACMATCHIP
                PMACMATCH_ENTRY macmatch_entry,
                int   num_entries,
        #endif
		#ifdef SUPPORT_VDF_DHCP
            char * ntp1,
            char * ntp2,
            unsigned char * pOption121,
            char * pOption43,
		#endif	
                    int protocol, int enblNat, int enblFirewall,
                    int enblSer1, 
                    int enblSer2);
#endif

/*w44771 add for 第一IP支持5段地址池，end*/
/* BEGIN: Added by y67514, 2008/9/11   问题单号:GLB:Ip Extension*/
int bcmCreateIpExtDhcpCfg(char *lanAddr, char *mask, char *wanAddr,char *interface,char *mac);
/* END:   Added by y67514, 2008/9/11 */
int bcmCreateLocalDhcpCfg(char *addr, char *mask,char *mac);
int bcmCreateResolvCfg(char *dns1, char *dns2);
#ifdef SUPPORT_ONE_USER
int bcmCreateLoginCfg(char *username, char *password);
#else
int bcmCreateLoginCfg(char *cp_admin, char *cp_support, char *cp_user);
#endif
#ifdef   SUPPORT_RADIUS
/*start  of 增加Radius 认证功能by s53329  at  20060714 */
int bcmAddLoginCfg(char *cp_admin, char *cp_pass) ;
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
#endif
int bcmGetAdslStatus(void);
int bcmGetVdslStatus(void);
int bcmGetXdslStatus(void); 
int bcmGetPppStatus(char *str, int len, char *devName);
int bcmGetDhcpcStatus(char *str, int len);
int bcmGetSystemStatus(char *str, int len);
int bcmSetSystemStatus(int status);
int bcmSetIpExtension(int ipExt);
int bcmGetIpExtension(char *str, int len);
void bcmDisplayLed(int status);
void bcmGetDns(char *dns);
void bcmGetDns2(char *dns);
/*start of 解决AU8D01490问题单: 增加PPP IP Extension 获取dns信息 by l129990,2008,12,16*/
void bcmGetIpExtDynamicDnsAddr(char *interface, char *dns, int primary);
void bcmGetIpExtDns(char *interface, char *dns);
void bcmGetIpExtDns2(char *interface, char *dns);
/*end of 解决AU8D01490问题单: 增加PPP IP Extension 获取dns信息 by l129990,2008,12,16*/
/*start of waninfo by c65985 for vdf 080314*/
#ifdef SUPPORT_VDF_WANINFO
void bcmGetDSLInfInfo(char *info);
void bcmGetDataGw(char *dataGw);
void bcmGetVoiceGw(char *voiceGw);
void bcmGetHspaGw(char *HspaGw);
void bcmGetDataIp(char *dataIp);
void bcmGetVoiceIp(char *voiceIp);
void bcmGetHspaIp(char *HspaIp);
int bcmGetGWByInf(char *gtwy, char *wanIf);
/*start of HG553 2008.03.24 V100R001C02B010 AU8D00297 by c65985 */
void bcmGetHspaDNS(char *HspaIp);
int bcmGetDNSByInf(char *dns, char *wanIf);
/*end of HG553 2008.03.24 V100R001C02B010 AU8D00297 by c65985 */
int bcmGetIpByIfc(char *address, char *ifcName);
#endif
/*end of waninfo by c65985 for vdf 080314*/
void bcmGetDnsSettings(int *mode, char *primary, char *secondary);
void bcmRestartDnsProbe(void);
void bcmConfigDns(char *dns1, char *dns2, int dynamic);
int bcmGetSntpExisted(void);
unsigned long getAvailableRouteIndex(void);
void bcmRemoveDefaultGatewayByWanIf(char *wanIf);
void bcmGetDefaultGateway(char *gtwy, char *wanIf);
/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
int bcmGetWanDNSByIdIfc(char *pDnsAddr, void* pWanId, char * pWanIfcName) ;
/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
/*start of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */ 
void bcmGetStaticDefaultGatewayByWanIfc(char * pszGateWay, void * pWanId, char * pszGwWanIfc);
/*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */ 
void bcmGetDefaultGatewaySts(char *gtwy, char *wanIf);

/*start of ROUTE 3.1.7 porting by z37589 20060511*/
int bcmGetProcDefaultGateway(char *gtwy, char *wanIf);
/*end of ROUTE 3.1.7 porting by z37589 20060511*/

/* start of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */
void bcmShowDefaultGatewayInfo(void);
/* end of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */

/*start of ROUTE 3.1.9 porting by z37589 20060531*/
int bcmIsdstRouteExisted(char *dstAddr, char *dstMask);
/*end of ROUTE 3.1.9 porting by z37589 20060531*/
int  bcmIsDefaultGatewayExisted(char *gw, char *wanIf);
void bcmSetAutoDefaultGateway(char *errMsg);
void bcmSetStaticDefaultGateway(char *gw, char *wanIf, char *errMsg);
void bcmKillAllApps(void);
/* BEGIN: Added by y67514, 2010/3/31   PN:DLNA for Spain*/
void SetPidNice(char* pidName,int nice);
/* END:   Added by y67514, 2010/3/31 */
void disconnectPPP(void);
void setWanLinkStatus(int up);
int bcmCheckInterfaceUp(char *devname);
int bcmSetIpExtInfo(char *wan, char *gateway, char *dns);
void bcmGetIpExtInfo(char *buf, int type);
int bcmGetIfcIndexByName(char *ifcName);
int bcmIsValidIfcName(char *ifcName);
char *bcmGetIfcNameById(int ifcId, char *ifcName);
char *bcmGetIfcNameByIpAddr(unsigned long ipAddr, char *ifcName);
/*d104343 webcam start */
char *bcmGetIpAddrByIfcName(char* ipAddr, char *ifcName);
/*d104343 webcam end */
void getPppoeServiceName(char *service, char *ifName);
void bcmGetDefaultRouteInterfaceName(char *ifcName);

void bcmSetConnTrackMax(void);
void bcmResetConnTrackTable(void);
void bcmHandleConnTrack(void);
void bcmInsertModules(char *path);
void bcmInsertModule(char *modName);
int bcmIsModuleInserted(char *modName);
int bcmRemoveIpTableRule(char *device, char *table, char *chain);
void bcmRemoveAllIpTableRules(char *device);
#if SUPPORT_TC
void bcmRemoveTrafficControlRules(unsigned short vpi, unsigned short vci,
                                  unsigned short conId, unsigned char protocol);
void bcmAddTrafficControlRules(unsigned short vpi, unsigned short vci,
                               unsigned short conId, unsigned char protocol);
/* start PPPOE/PPPOA方式时，QOS不生效 for A36D03573 */
void bcmAddTrafficControlRules_doDefaultPolicy();
/* end PPPOE/PPPOA方式时，QOS不生效 for A36D03573 */
#endif
int bcmRemoveDnsIpTableRule(void);
int bcmRemoveRipIpTableRule(void);
int bcmRemoveUpnpIpTableRule(void);
void bcmInsertAllUpnpIpTableRules(void);
int bcmGetIntfNameSocket(int socketfd, char *intfname);
int bcmCheckEnable(char *appName, struct in_addr clntAddr);

int bcmRemoveEbTableRule(char *device, char *table, char *chain);
void bcmRemoveAllEbTableRules(char *device);

void parseStrInfo(char *info, char *var, char *val, int len);
void bcmConvertStrToShellStr(char *str, char *buf);
void bcmProcessMarkStrChars(char *str);
int  bcmIsMarkStrChar(char c);
void bcmSetVdslSwVer(char *swVer);
void bcmGetSwVer(char *swVer, int size);
/* start : s48571 add for VDF project to get firmware version 2007-11-16 */
void bcmGetfirmwareVer(char *swVer, int size);
/* end  : s48571 add for VDF project to get firmware version 2007-11-16 */
void bcmGetVdslSwVer(char *swVer, int size);
int bcmGetLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);
int isAccessFromLan(struct in_addr clntAddr);
int checkChipId(char *strTagChipId, char *sig2);
void bcmRemoveModules(int lanIf);
int bcmWaitIntfExists(char *ifName);

int bcmMacStrToNum(char *macAddr, char *str);
int bcmMacNumToStr(char *macAddr, char *str);

int bcmWanEnetQuerySwitch(char *ifName);

int bcmConfigPortMirroring (void *pMirrorCfg) ;

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
int bcmGetTrafficMode() ;

void bcmSetTrafficMode(int trafficmode);

void bcmSetAtmConfigMode(int trafficmode);
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

int bcmAddAccount(char *account, char *passwd);

int bcmIsOnePVCUp(void);

#define BCM_PPPOE_CLIENT_STATE_PADO          0   /* waiting for PADO */
#define BCM_PPPOE_CLIENT_STATE_PADS          1   /* got PADO, waiting for PADS */
#define BCM_PPPOE_CLIENT_STATE_CONFIRMED     2   /* got PADS, session ID confirmed */
#define BCM_PPPOE_CLIENT_STATE_DOWN          3   /* totally down */
#define BCM_PPPOE_CLIENT_STATE_UP            4   /* totally up */
#define BCM_PPPOE_SERVICE_AVAILABLE          5   /* ppp service is available on the remote */

#define BCM_PPPOE_AUTH_FAILED                7

#define BCM_PRIMARY_DNS                      1
#define BCM_SECONDARY_DNS                    0

/* start of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */
/* 表项名的最大长度 */
#define TMPFILE_MAX_ITEM_NAME       16
/* 表项值的最大长度 */
#define TMPFILE_MAX_ITEM_VALUE      512
/* 表项的最大长度 */
#define TMPFILE_MAX_ITEM_LEN        530

/* 消息类型，包括写，读，读反馈 */
#define TMPFILE_WRITE_OPT   1
#define TMPFILE_READ_OPT    2
#define TMPFILE_READ_ACK    3

/* 消息队列key值生成需要的参数 */
#define TMPFILE_KEY_PATH    "/var"
#define TMPFILE_KEY_SEED    't'

/* var目录下文件名和临时文件名 */
#define TMPFILE_FILE_NAME   "/var/homegateway.conf"
/* begin --- Modify by w69233: Fixed temporaty file name conflict with samba temporary directory name issue */
#define TMPFILE_TMPFILE_NAME    "/var/tempfile"
/* end ----- Modify by w69233: Fixed temporaty file name conflict with samba temporary directory name issue */

/* 表项查找标志 */
#define TMPFILE_NOT_FOUND_ITEM   0
#define TMPFILE_FOUND_ITEM       1

/* start of maintain TR069消息处理 by liuzhijie 00028714 2006年7月26日 */
#define TR069_KEY_PATH  "/var"
#define TR069_KEY_SEED  '9'

#define TR069_MAX_CONTENT 1520

#define WDF_HSPA_DATA_VPI    255
#define WDF_HSPA_DATA_VCI    65535

/* TR069消息 */
typedef struct tag_TR069_MSG
{
    long lMsgType;                          //消息类型
    char achContent[TR069_MAX_CONTENT];     //消息内容
}S_TR069_MSG;

/* end of maintain TR069消息处理 by liuzhijie 00028714 2006年7月26日 */

/* var目录下文件读写消息 */
typedef struct tag_TMPFILE_MSG
{
    long lMsgType;                          //消息类型，包括写，读，读反馈
    int iResult;                            //操作结果
    int iPid;                               //发送消息的PID，作为读响应消息的类型值
    int iSerial;                            //消息序列号，避免同一进程读取消息顺序错误
    char achName[TMPFILE_MAX_ITEM_NAME];    //读写变量名
    char achValue[TMPFILE_MAX_ITEM_VALUE];  //读写变量值
}S_TMPFILE_MSG;

/* for configure ttyUsb with file(/var/hspasys.cfg). <tanyin 2009.4.9> */
typedef struct HSPA_SYS
{
    unsigned short usConType;
    unsigned short usVoiceEnable;
    char cBand[32];
    char cOperator[32];   
} HSPA_SYS_ST;

extern int tmpfile_writevalue(char *pchName, char *pchValue);
extern int tmpfile_getvalue(char *pchName, char *pchValue, int *piLen);
extern int tmpfile_getRadiusStatus(char *name);

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern int g_trafficmode;
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* end of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */

#ifdef SUPPORT_TR69C
/* start of maintain TR069消息处理 by liuzhijie 00028714 2006年7月26日 */
void TR069_CreateMsgQueue(void);
int TR069_SendMessage(long msg_type, void *msg_content, int content_len);
int TR069_ReceiveMessage(long *msg_type, void *msg_content, int content_len);
/* end of maintain TR069消息处理 by liuzhijie 00028714 2006年7月26日 */
#endif

int AtpCreateLoginforFtp(char *username, char *password, char* path);

#ifdef VDF_PPPOU
/*start, ppp over usb, add by sxg*/
int HW_CreatePppouCfg(char*, char*, char*, unsigned int, unsigned int, char *);
/*end, ppp over usb, add by sxg*/
 /*start of 增加VDF 数据/语音自动切换功能 by s53329 at  20071230*/
void killHspaPid();
void killWanPid();
int getHspaStatus(void);
int JudgeDefaultInterface(char *interface);
int bcmRemoveDnsForwardRule(char *table, char *chain, char  *cDnsPrim, char *cDnsSe);
int  bcmGetDnsByInterface(char *pInterface,  char  *cDnsPrim, char *cDnsSe);
void bcmSetPppouDown(char *name);
int  BcmGetDefaultInterface(char *interface);
void BcmNtwk_fakeHttpForwarding(void);
void BcmNtwk_UnfakeHttpForwarding(void);
/*add for dns proxy by a00169427 at 20111016*/
void BcmNtwk_addLanDnsProxy(void);
void BcmNtwk_deleteLanDnsProxy(void);
/*end add for dns proxy by a00169427 at 20111016*/
void  bcmRemoveDnsInfo(char *interface);
 /*end of 增加VDF 数据/语音自动切换功能 by s53329 at  20071230*/
 int AtRcvOnce(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType);
 int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType);
 int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam);
 /*start of 修改http 页面跳转太慢问题 by s53329  at  20080717 */
 void Hspa_StopDial(int  sig );
  /*end of 修改http 页面跳转太慢问题 by s53329  at  20080717 */

/*add by z67625 清除防火墙日志函数声明 start*/
void Reset_FwLog(void);
/*add by z67625 清除防火墙日志函数声明 end*/
  
  /* BEGIN: Added by y67514, 2008/7/23   PN:AU8D00825:将hspa模块中Mobile Connection Type从3G only切换为其他模式时,hspa不能拨号成功*/
  void Reboot_HSPAStick(void);
  /* END:   Added by y67514, 2008/7/23 */
/*start of VDF 2008.7.15 V100R001C02B020 j00100803 AU8D00798 AU8D00818 */
int bcmIsDataPvc(char * pszWanIfcName);
/*end  of VDF 2008.7.15 V100R001C02B020 j00100803 AU8D00798 AU8D00818 */

#ifdef SUPPORT_VDF_WANINFO
void hspasysInit( void );
/* BEGIN: Added by y67514, 2008/7/5   问题单号:没拨上号时不能将状态置为开始拨*/
int Hspa_Link_up();
/* END:   Added by y67514, 2008/7/5 */
#endif

#endif
/* j00100803 Add Begin 2008-02-25 */
#ifdef SUPPORT_POLICY
int bcmUpperStr(char * pString);
int bcmUpperMacStr(char * pString);
#endif
/* j00100803 Add End 2008-02-25 */
/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
#define DNS_CFG_FILE    "/var/dnscfg"
int creatDnsCfg( void );
/* END:   Added by y67514, 2008/9/18 */

int bcmGetBrZeroMacAddr(unsigned char * pMacAddr);
int bcmCodingAdminPwdForCd(char * pInString, int iInLen, char * pOutString, int iOutLen);

int bcmGetWlanChipType(int bMute);
/* BEGIN: Added by y67514, 2009/12/11   PN:APNSecurity Enhancement*/
int HspaSetEncodedApn( char * cEnCodedApn );
int HspaSetApn( char* cApn );
int HspaGetEncodedApn( char * cEnCodedApn );
int HspaGetApn( char * cApn );
int HspaSetEncodedUserName( char * cEnCodedUserName );
int HspaSetUserName( char* cUserName );
int HspaGetEncodedUserName( char * cEnCodedUserName );
int HspaGetUserName( char * cApn );
int HspaSetEncodedPwd( char * cEnCodedPwd );
int HspaSetPwd( char* cPwd );
int HspaGetEncodedPwd( char * cEnCodedPwd );
int HspaGetPwd( char * cPwd );
void DecodeStr( char * EncodedStr ,char * DecodedStr);
/* END:   Added by y67514, 2009/12/11 */
#if defined(__cplusplus)
}
#endif

#endif
