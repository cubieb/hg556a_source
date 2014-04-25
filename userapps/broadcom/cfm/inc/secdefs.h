/*****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
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
//  Filename:       secdefs.h
//  Author:         Peter T. Tran
//  Creation Date:  06/10/02
//
//  Description:
//      Define the Global structs, enumerations for security configuations.
//
*****************************************************************************/

#ifndef __SEC_DEFS_H
#define __SEC_DEFS_H

#include "ifcdefs.h"

/********************** Include Files ***************************************/

/********************** Global Types ****************************************/

typedef enum {
   SEC_STS_OK = 0,
   SEC_STS_ERR_GENERAL,
   SEC_STS_ERR_MEMORY,
   SEC_STS_ERR_OBJECT_NOT_FOUND
} SEC_STATUS;

typedef enum {
   SEC_COMMAND_TYPE_ADD = 0,
   SEC_COMMAND_TYPE_INSERT,
   SEC_COMMAND_TYPE_REMOVE
} SEC_COMMAND_TYPE;

/********************** Global Constants ************************************/

#define SECURITY_APPID           "SecCfg"
#define SCM_VRT_SRV_NUM_ID       1
#define SCM_VRT_SRV_TBL_ID       2
#define SCM_FLT_OUT_NUM_ID       3
#define SCM_FLT_OUT_TBL_ID       4
#define SCM_DMZ_HOST_ID          7
#define SCM_PRT_TRG_NUM_ID       8
#define SCM_PRT_TRG_TBL_ID       9
#define SCM_MAC_FLT_NUM_ID       10
#define SCM_MAC_FLT_TBL_ID       11
#define SCM_ACC_CNTR_NUM_ID      12
#define SCM_ACC_CNTR_TBL_ID      13
#define SCM_ACC_CNTR_MODE_ID     14
/*start modifying for vdf qos by p44980 2008.01.07*/
#ifdef SUPPORT_VDF_QOS
#define QOS_CFG_ID                         15
#define QOS_QUEUE_NUM_ID           16
#define QOS_QUEUE_TBL_ID            17
#define QOS_SERVICE_NUM_ID        18
#define QOS_SERVICE_TBL_ID         19
#endif
/*end modifying for vdf qos by p44980 2008.01.07*/
#define QOS_CLS_NUM_ID           20
#define QOS_CLS_TBL_ID           21
#define SCM_FLT_IN_NUM_ID        22
#define SCM_FLT_IN_TBL_ID        23
#define SCM_SRV_CNTR_ID          24
// 加入对tc QoS的支持
#define SCM_WRED_INFO_ID          25
//for vdf firewall
#define SEC_VDF_BASE              26
#define SCM_FW_ID                  (SEC_VDF_BASE+0)
#define SCM_FW_NUM_ID             (SEC_VDF_BASE+1)
#define SCM_FW_TBL_ID             (SEC_VDF_BASE+2)
#define SCM_DOS_ID                 (SEC_VDF_BASE+3)
#define SCM_URL_NUM_ID            (SEC_VDF_BASE+4)
#define SCM_URL_TBL_ID            (SEC_VDF_BASE+5)
/*add by z67625 过滤开关宏定义 start*/
#define SCM_SEC_ENBL_ID            (SEC_VDF_BASE+6)
/*add by z67625 过滤开关宏定义 end*/
//end vdf

/*start of HG553V100R001C02B020 2008.07.15  AU8D00806 */
#define SEC_BUFF_SRVNAME_LEN     257 
/*end of HG553V100R001C02B020 2008.07.15  AU8D00806 */

 /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  begin*/
//#define SEC_BUFF_MAX_LEN          16
//modified by z67625 acl保存问题 32->64
#define SEC_BUFF_MAX_LEN          64//网段表示格式: 192.168.111.112-192.168.111.113  
                                             //最大长度32个字符(含'\0')
/* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  end*/
#define SEC_CMD_MAX_LEN          2048
#define SEC_PROTO_BOTH           0
#define SEC_PROTO_TCP            1
#define SEC_PROTO_UDP            2
#define SEC_PROTO_ICMP           3
/*add by z67625 IP过滤支持IGMP协议 start*/
#define SEC_PROTO_IGMPV          4
/*add by z67625 IP过滤支持IGMP协议 start*/
/*start--w00135358 modify for VDF Global QoS 20080908*/
#define SEC_QOS_PROTO_IGMP           SEC_PROTO_IGMPV
//#define SEC_PROTO_MAX            4
#define SEC_PROTO_MAX            5
#define ETHENET_PROTO_IP		 0
#define ETHENET_PROTO_8021Q      1
#define ETHENET_PROTO_PPPOE		 2
#define ETHENET_PROTO_MAX        3
/*end--w00135358 modify for VDF Global QoS 20080908*/

#define FTP_SERVER_PORT_21       "21"
#define FTP_SERVER_PORT_2121     "2121"
#define WEB_SERVER_PORT_80       "80"
#define WEB_SERVER_PORT_8080     "8080"
#define SNMP_AGENT_PORT_161      "161"
#define SNMP_AGENT_PORT_16116    "16116"
#define SSH_SERVER_PORT_22       "22"
#define SSH_SERVER_PORT_2222     "2222"
#define TELNET_SERVER_PORT_23    "23"
#define TELNET_SERVER_PORT_2323  "2323"
#define TFTP_SERVER_PORT_69      "69"
#define TFTP_SERVER_PORT_6969    "6969"

#define TR69C_REQUEST_PORT_8081  "8081"
#define TR69C_REQUEST_PORT_80818  "80818"

#define SEC_MAC_MAX_LEN          6

#define SEC_PROTO_NONE           0
#define SEC_PROTO_PPPOE          1        // require PPP_DISC
#define SEC_PROTO_IPV4           2        // reuire ARP
#define SEC_PROTO_IPV6           3	      // require ARP
#define SEC_PROTO_ATALK	         4
#define SEC_PROTO_IPX	         5
#define SEC_PROTO_NEtBEUI        6
#define SEC_PROTO_IGMP           7

/* BEGIN: Added by hKF20513, 2009/11/12   PN:增加X_AccessHostCfg实例*/
#define ACC_CTL_HOST          32       
/* END:   Added by hKF20513, 2009/11/12 */
#define SEC_PTR_TRG_MAX          32       // max port triggering entries.
/* BEGIN: Modified by s00125931, 2008/11/5   问题单号:AU8D01120 */
#define SEC_WL_MAC_FILTER_MAX          128       // max wl macFilter entries.
//#define SEC_WL_MAC_FILTER_MAX          32       // max wl macFilter entries.
/* END:   Modified by s00125931, 2008/11/5 */
#define SEC_VIR_SRV_MAX          32       // max virtual server entries.
#define SEC_ADD_REMOVE_ROW       6        // display add remove button if row > 6

#define SEC_MAX_PORT_MIRRORS     2       // max virtual server entries.
#define QOS_TYPE_GENERAL         1
#define QOS_TYPE_WL              2        // wireless
#define QOS_TYPE_DIFFSERV        3        // DiffServ

#define SEC_TOD_MAX_DAY          7
#define SEC_ENTRIES_MAX          32
#define SEC_ENTRIES_TOD_MAX      16
/*start of VDF 2008.5.7 V100R001C02B013 j00100803 AU8D00495 */
#define SEC_FW_CUSTOM_MAX        64 //vdf firewall custom rules max
/*end of VDF 2008.5.7 V100R001C02B013 j00100803 AU8D00495 */
#define SEC_URLBLK_MAX           32 //vdf firewall custom rules max
/********************** Global Structs ************************************/

typedef struct {
   uint32 id;
   char srvName[SEC_BUFF_SRVNAME_LEN];
   struct in_addr addr;
//add by l66195 for VDF start
   unsigned short  bEnable;
//add by l66195 for VDF end
   unsigned short protocol;
   unsigned short eStart;
   unsigned short eEnd;
   unsigned short iStart;
   unsigned short iEnd;
} SEC_VRT_SRV_ENTRY, *PSEC_VRT_SRV_ENTRY;

typedef struct {
   int protocol;
   int fromPort;
   int toPort;
} SEC_FLT_PRT_ENTRY, *PSEC_FLT_PRT_ENTRY;

typedef struct {
   char fromAddress[SEC_BUFF_MAX_LEN];
   char toAddress[SEC_BUFF_MAX_LEN];
} SEC_FLT_ADDR_ENTRY, *PSEC_FLT_ADDR_ENTRY;

typedef struct {
   int protocol;
   char fltName[SEC_BUFF_MAX_LEN];
   char srcAddr[SEC_BUFF_MAX_LEN];
   char srcMask[SEC_BUFF_MAX_LEN];
   char srcPort[SEC_BUFF_MAX_LEN];
   char dstAddr[SEC_BUFF_MAX_LEN];
   char dstMask[SEC_BUFF_MAX_LEN];
   char dstPort[SEC_BUFF_MAX_LEN];
   char wanIf[IFC_TINY_LEN];   
} SEC_FLT_ENTRY, *PSEC_FLT_ENTRY;


// PROTOCOL flag
// 16 bit: bit 0-7:  trigger protocol type -- defined above such as SEC_PROTO_TCP...
//         bit 8-15: open protocol type -- defined above such as SEC_PROTO_TCP...
typedef struct {
   unsigned short
   tProto:8,
   oProto:8;
} PROTO_FLAG;

typedef struct {
   PROTO_FLAG flag;
//add by l66195 for VDF start
   unsigned short bEnable;
//add by l66195 for VDF end
   unsigned short tStart;
   unsigned short tEnd;
   unsigned short oStart;
   unsigned short oEnd;
   char appName[SEC_BUFF_MAX_LEN];
} SEC_PRT_TRG_ENTRY, *PSEC_PRT_TRG_ENTRY;

#define ACL_FILE_NAME     "/var/acl.conf"
/* setup icmp ctrl to kernel  --c47036 */
#define ICMP_ACL_SVC_FILE "/proc/sys/net/ipv4/icmp_acl_svc"
#define ICMP_ACL_ADDR_FILE "/proc/sys/net/ipv4/icmp_acl_addr"
#define ICMP_ACL_ADDR_MAX 65

typedef struct {
 /*start-add by 00112761 for VDF_C02*/
   uint32	 iHostId;
 /*end-add  by 00112761 for VDF_C02*/
   char address[SEC_BUFF_MAX_LEN];
} SEC_ACC_CNTR_ENTRY, *PSEC_ACC_CNTR_ENTRY;

#define QOS_PROTO_NONE    -1

#define QOS_PRIORITY_LOW        1
#define QOS_PRIORITY_MED        2
#define QOS_PRIORITY_HIGH       3
#define QOS_PRIORITY_HIGHEST    4

#define QOS_PHY_PORT_NONE -1
#define QOS_PHY_PORT_MIN  1
#define QOS_PHY_PORT_MAX  4
#define QOS_PHY_NAME_NONE	""

#define TOS_NONE                  -1
#define TOS_NORMAL_SERVICE        0
#define TOS_MINIMIZE_COST         2
#define TOS_MAXIMIZE_RELIABILITY  4
#define TOS_MAXIMIZE_THROUGHPUT   8
#define TOS_MINIMIZE_DELAY        16

#define DSCP_NONE                 -1
#define DSCP_AUTO                 -2
#define DSCP_MASK                 0xFC
#define DSCP_DEFAULT              0x00
#define DSCP_AF13                 0x38
#define DSCP_AF12                 0x30
#define DSCP_AF11                 0x28
#define DSCP_CS1                  0x20
#define DSCP_AF23                 0x58
#define DSCP_AF22                 0x50
#define DSCP_AF21                 0x48
#define DSCP_CS2                  0x40
#define DSCP_AF33                 0x78
#define DSCP_AF32                 0x70
#define DSCP_AF31                 0x68
#define DSCP_CS3                  0x60
#define DSCP_AF43                 0x98
#define DSCP_AF42                 0x90
#define DSCP_AF41                 0x88
#define DSCP_CS4                  0x80
#define DSCP_EF                   0xB8
#define DSCP_CS5                  0xA0
#define DSCP_CS6                  0xC0
#define DSCP_CS7                  0xE0

/*start -- w00135358 add for VDF Global QoS 20080908*/
#define QOS_8021P_MAX				8    //8021P [0,7]
#define DSCP_MAX					64   //DSCP  [0,63]
#define VLANID_MAX					4095 //vlanid [1,4094]
#define VLANID_NONE                 -1
/*end -- w00135358 add for VDF Global QoS 20080908*/

/*
int8 dscpMarkValues[] = {DSCP_DEFAULT, DSCP_AF13, DSCP_AF12, DSCP_AF11, DSCP_CS1,
                         DSCP_AF23, DSCP_AF22, DSCP_AF21, DSCP_CS2,
                         DSCP_AF33, DSCP_AF32, DSCP_AF31, DSCP_CS3,
                         DSCP_AF43, DSCP_AF42, DSCP_AF41, DSCP_CS4,
                         DSCP_EF, DSCP_CS5, DSCP_CS6, DSCP_CS7};

char* dscpMarkDesc[] = {"default", "AF13", "AF12", "AF11", "CS1",
                         "AF23", "AF22", "AF21", "CS2",
                         "AF33", "AF32", "AF31", "CS3",
                         "AF43", "AF42", "AF41", "CS4",
                         "EF", "CS5", "CS6", "CS7", NULL};
*/

#define PRECEDENCE_NONE   -1
#define PRECEDENCE_MIN    0
#define PRECEDENCE_MAX    7

typedef struct {
   int8 type;
   int16 protocol;
   int8 priority;
   int8 wlpriority;
   int8 tos;
   int8 precedence;
   int8 wanVlan8021p;
   int8 vlan8021p;
   char lanIfcName[IFC_TINY_LEN];
   char clsName[SEC_BUFF_MAX_LEN];
   char srcAddr[SEC_BUFF_MAX_LEN];
   char srcMask[SEC_BUFF_MAX_LEN];
   char srcPort[SEC_BUFF_MAX_LEN];
   char dstAddr[SEC_BUFF_MAX_LEN];
   char dstMask[SEC_BUFF_MAX_LEN];
   char dstPort[SEC_BUFF_MAX_LEN];
   int  diffservcfg;
   int  dscpMark;
   char srcMacAddr[SEC_MAC_MAX_LEN];
   char dstMacAddr[SEC_MAC_MAX_LEN];
   char srcMacMask[SEC_MAC_MAX_LEN];
   char dstMacMask[SEC_MAC_MAX_LEN];
   /* start of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */
#ifdef SUPPORT_TDE_QOS
   int clsId;
   UINT16 enbTrafficLimit;
   UINT32 limitRate;
#endif
   /* end of protocol QoS for TdE by zhangchen z45221 2006年9月7日" */

   /*start add for vdf qos by p44980 2007.12.31*/
   #ifdef SUPPORT_VDF_QOS
  char srcMaxPort[SEC_BUFF_MAX_LEN];
  char dstMaxPort[SEC_BUFF_MAX_LEN];
  uint32 ulClsInstId;  //流分类的实例号
  uint32 ulQueueInstId; //进入哪个队列
  char acServiecName[SEC_BUFF_MAX_LEN + 1];

/*start--w00135358 add for VDF Global QoS 20080905*/
  int8   dscpCheck;                   //DSCP Check
  int    vlanidCheck;                 //VLAN ID Check
  int	 vlanidMark;				  //VLAN ID Mark
  int    etheProt;					  //Ethenet Protocol: IP/VLAN/PPPOE
/*start--w00135358 end for VDF Global QoS 20080905*/

  #endif
   /*end add for vdf qos by p44980 2007.12.31*/
} QOS_CLASS_ENTRY, *PQOS_CLASS_ENTRY;
#ifdef WIRELESS
#define MAX_QOS_QUEUE_ENTRY   24    // max QoS Queue entries inclue 4 wireless queues.
#else
#define MAX_QOS_QUEUE_ENTRY   16    // max QoS Queue entries.
#endif

#ifdef WIRELESS
#define WIRELESS_PREDEFINE_QUEUE 8
#else
#define WIRELESS_PREDEFINE_QUEUE 0
#endif
#define WIRELESS_QUEUE_INTF      "wireless"

/*start add for vdf qos by p44980 2007.12.31*/
#ifdef SUPPORT_VDF_QOS
typedef struct 
{
    uint32 ulQueueInstId;
    uint32 ulPrecedence;
    uint32 ulWeight;
    uint32 ulMark;
    char acSchAlg[SEC_BUFF_MAX_LEN];
}QOS_QUEUE_ENTRY, *PQOS_QUEUE_ENTRY;

#define QOS_DEFAULT_WEIGHT 0
#define QOS_MAX_WEIGHT        100
#define QOS_MAX_QUEUE 6    //最多支持6个队列
#define QOS_MAX_CLASS  32  //最多支持32个流分类规则

#define QOS_PRECEDENCE_HIGH      1
#define QOS_PRECEDENCE_MEDIUM 2
#define QOS_PRECEDENCE_LOW       3

#define QOS_SCH_SP   "SP"
#define QOS_SCH_WRR "WRR"

#define QOS_PROTO_BOTH  SEC_PROTO_BOTH
#define QOS_PROTO_TCP   6
#define QOS_PROTO_UDP   17
#define QOS_PROTO_ICMP  1
#define QOS_PROTO_IGMP  2
#define QOS_PROTO_MAX   255

#define QOS_8021P_NONE   -1
#define QOS_DSCP_MAX 64
#define QOS_PORT_MAX 65535
#define QOS_PORT_NONE -1
#define QOS_QUEUE_NONE 0

#define QOS_QUEUE_NONE_TR069 -1

#define QOS_WRR_MIN 0.000001
#define QOS_UPSTREAM_IFC  "br9"

#define TR69C_CONN_REQUEST_PORT  8081
//#define TR69C_CONN_REQ_PORT      19909
#define QOS_DSCP_VOICE 0xB8
#define QOS_DSCP_MNGT 0x88

#define QOS_DNS_PORT 53
#define QOS_DSCP_DNS 0x48

#define QOS_CONNTYPE_NONE    0
#define QOS_CONNTYPE_ADSL     1
#define QOS_CONNTYPE_HSPA    2
#define QOS_CONNSTATE_UP      0
#define QOS_CONNSTATE_DOWN 1

#define QOS_DEFAULT_PORT       -1   /* Default port */

typedef enum 
{
    EN_QOS_EF = 1,
    EN_QOS_AF1,
    EN_QOS_AF2,
    EN_QOS_BE,
    EN_QOS_AF3,
    EN_QOS_AF4
}QOS_QUEUE_MARK;
/*end add for vdf qos by p44980 2007.12.31*/

/*start add for vdf qos by p44980 2007.2.28*/
#define QOS_STATUS_DISABLED "Disabled"
#define QOS_STATUS_ENABLED   "Enabled"
#define QOS_STATUS_ERROR        "Error"

#define QOS_TEMP_FILE       "qosCfg"
#define QOS_CFG_TRUE        1
#define QOS_CFG_FALSE       0

#define QOS_SERVICE_MAX           32    //最大服务数量
#define QOS_SERVICE_CFG_MAX   256  //每个服务最大可配置条件

typedef enum
{
    EN_QOS_STATUS_DISABLED,
    EN_QOS_STATUS_ENABLED,
}QOS_CFG_STATUS;

typedef struct
{
     uint32 ulInstanceId;
     char acServiceName[SEC_BUFF_MAX_LEN + 1]; //服务名称
     QOS_CFG_STATUS  eServiceStatus;
     uint32 ulServiceCfgNum;
}QOS_SERVICE_ENTRY, *PQOS_SERVICE_ENTRY;

typedef struct
{
     uint32 ulInstanceId;
     uint32 ulParentId;    //所属服务Id
     char  acServiceName[SEC_BUFF_MAX_LEN + 1]; //所属服务名称
     QOS_CFG_STATUS  eStatus;   //状态
     int16  protocol;
     char srcPort[SEC_BUFF_MAX_LEN];
     char dstPort[SEC_BUFF_MAX_LEN];
     char srcMaxPort[SEC_BUFF_MAX_LEN];
     char dstMaxPort[SEC_BUFF_MAX_LEN];
}QOS_SERVICE_CFG_ENTRY, *PQOS_SERVICE_CFG_ENTRY;

typedef struct
{
    char serviceCategory[IFC_TINY_LEN];
    int   ldscpMark;
}QOS_AUTODSCP_MAP, *PQOS_AUTODSCP_MAP;
/*end add for vdf qos by p44980 2007.2.28*/
#endif

#define SSC_MODE_DISABLE          0
#define SSC_MODE_ENABLE           1
#define SSC_MODE_ENABLE_LAN       2
#define SSC_MODE_ENABLE_WAN       3

typedef struct {
   int modeFtp;
   int modeHttp;
   int modeIcmp;
   int modeSnmp;
   int modeSsh;
   int modeTelnet;
   int modeTftp;
/* BEGIN: Added by weishi kf33269, 2011/6/26   PN:Issue407:TR-069 service is not protected with access control*/
   int modeTr069;
/* END:   Added by weishi kf33269, 2011/6/26 */
} SEC_SRV_CNTR_INFO, *PSEC_SRV_CNTR_INFO;

/* start of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
    #ifdef SUPPORT_KPN_QOS // macro QoS for KPN
    #define WRED_WEIGHT_RATIO          "1:1"

    typedef struct {
      char WredWeightRatio[SEC_BUFF_MAX_LEN];
    } SEC_WRED_INFO, *PSEC_WRED_INFO;
    #endif
#endif
/* end of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */

#define MAC_FORWARD 0
#define MAC_BLOCKED 1
#define LAN_TO_WAN  0
#define WAN_TO_LAN  1
#define BOTH        2
#define MAC_DIR_MAX 3

/*start modifying for layer 3 mac filter by p44980 2008.02.03*/
#define MAC_FILTER_BRIDGE "bridge"
#define MAC_FILTER_ROUTE  "route"
/*end modifying for layer 3 mac filter by p44980 2008.02.03*/
// mac entry flag define
// 32 bit
// bit 0-15 bit: protocal type -- defined above such as SEC_PROTO_PPPOE...
// bit 16-17   : direction 0=lan->wan, 1=wan->lan, 2=lan<->wan
// bit 18-31   : reserved
typedef struct {
   unsigned int
   protocol:16,
   direction:2,
   reserved:14;
} MAC_FIL_FLAG;

typedef struct {
   MAC_FIL_FLAG flag;         
   char destMac[SEC_MAC_MAX_LEN];
   char srcMac[SEC_MAC_MAX_LEN];
   char wanIf[IFC_TINY_LEN];
} SEC_MAC_FLT_ENTRY, *PSEC_MAC_FLT_ENTRY;

//for vdf firewall
#define SCM_FW_NAME_MAX 64
#define SCM_URLBLK_MAX  128
#define SCM_DOS_PTSCAN_SENSI_MAX 8
#define SCM_SEC_INIT_INSTID  1

#define SCM_FW_ACTION_NONE  -1
enum{
    SCM_CFG_OPTYPE_BYNAME,
    SCM_CFG_OPTYPE_BYID        
};

/*add by z67625 过滤使能开关枚举类型 start*/
enum{
    SCM_SEC_FW_ENABLE,         //新页面ip过滤
    SCM_SEC_INFLT_ENABLE,      //huawei页面incoming ip过滤
    SCM_SEC_OUTFLT_ENABLE,     //huawei页面outgoing ip过滤
    SCM_SEC_MACFLT_ENABLE,     //huawei页面mac过滤
    SCM_SEC_MACMNGR_ENABLE,    //新页面mac过滤
    SCM_SEC_ENABLE_MAX,
    /* BEGIN: Added by y67514, 2009/2/7   PN:GLB WEB:portfowding*/
    SCM_SEC_NAT_ENABLE                  /*glb新页面portforwarding使能*/
    /* END:   Added by y67514, 2009/2/7 */
};
/*add by z67625 过滤使能开关枚举类型 end*/

enum{
    SCM_FW_LEVEL_DISABLE,
    SCM_FW_LEVEL_STANDARD,
    SCM_FW_LEVEL_LOW,
    SCM_FW_LEVEL_MEDIUM,
    SCM_FW_LEVEL_HIGH,
    SCM_FW_LEVEL_BLOCKALL,
    SCM_FW_LEVEL_CUSTOM,
    
    SCM_FW_LEVEL_MAX
};
enum{
    SCM_FW_PROTO_HTTP,
    SCM_FW_PROTO_HTTPS,
    SCM_FW_PROTO_FTP,
    SCM_FW_PROTO_RIP,
    SCM_FW_PROTO_DNS,
    SCM_FW_PROTO_DHCP,
    SCM_FW_PROTO_ICMP,
    SCM_FW_PROTO_IGMP,
    SCM_FW_PROTO_TELNET,
    SCM_FW_PROTO_HTTPPROXY,
    SCM_FW_PROTO_POP2,
    SCM_FW_PROTO_POP3,
    SCM_FW_PROTO_SMTP,
    SCM_FW_PROTO_SSH,
    SCM_FW_PROTO_IPSEC,
    SCM_FW_PROTO_SIP,
    SCM_FW_PROTO_RTP,
    SCM_FW_PROTO_RTCP,
    SCM_FW_APP_AOL ,
    SCM_FW_APP_AIM,
    SCM_FW_APP_AOLICQ,
    SCM_FW_APP_BAYVPN ,
    SCM_FW_APP_CBC32, 
    SCM_FW_APP_CITRIX  ,
    SCM_FW_APP_CUSEEME2 ,
    SCM_FW_APP_DCONN ,
    SCM_FW_APP_FW1VPN ,
    SCM_FW_APP_ICQ,
    SCM_FW_APP_IPTV, 
    SCM_FW_APP_LAOLNK  ,
    SCM_FW_APP_NOTES ,
    SCM_FW_APP_MSN ,
    SCM_FW_APP_NETGEAR,
    SCM_FW_APP_NETMEETING3,
    SCM_FW_APP_PCAW ,
    SCM_FW_APP_REALP ,
    SCM_FW_APP_RTATHING ,
    SCM_FW_APP_SHIVA ,
    SCM_FW_APP_VIRNTCOMPUTING ,
    SCM_FW_APP_VDOLIVE ,
    SCM_FW_APP_VONAGEVOIP,

    SCM_FW_PROTOAPP_MAX
};    

enum{
    SCM_FW_ACT_ALLOW = 0,
    SCM_FW_ACT_DENY
};
enum{
    SCM_DOS_PTSCAN_LOW = 0,
    SCM_DOS_PTSCAN_HIGH
};
typedef struct {
    int  instID;
    char name[SCM_FW_NAME_MAX];
    unsigned int srcipStart;
    unsigned int srcipEnd;
    unsigned int dstipStart;
    unsigned int dstipEnd;
    char srcIfc[IFC_TINY_LEN];
    char dstIfc[IFC_TINY_LEN];
    int  proto;
    int  action; //ALLOW, DENY
}SEC_FW_ENTRY, *PSEC_FW_ENTRY;

typedef struct {
    int enable;

    unsigned char fldTcpSyn:1;
    unsigned char fldTcpFin:1;
    unsigned char fldUdp:1;
    unsigned char fldIcmp:1;
    unsigned char fldPerIpTcpSyn:1;
    unsigned char fldPerIpTcpFin:1;
    unsigned char fldPerIpUdp:1;
    unsigned char fldPerIpIcmp:1;

    unsigned char portScan:1;
    unsigned char icmpSmurf:1;
    unsigned char ipLand:1;
    unsigned char ipSproof:1;
    unsigned char ipTeardrop:1;
    unsigned char pingOfDeath:1;
    unsigned char tcpScan:1;
    unsigned char SynWithData:1;
    unsigned char udpBomb:1;
    unsigned char ptScanSensitivity:1;
    /*add by z67625 新增支持5种防攻击方式 start*/
    unsigned char SameSrcDst:1;
    unsigned char BrdcstSrc:1;
    unsigned char LanSrcIp:1;
    unsigned char InvalidTcpFlg:1;
    unsigned char Frgmthdr:1;
    /*add by z67625 新增支持5种防攻击方式 end*/
    
    unsigned short fldTcpSynSpd;
    unsigned short fldTcpFinSpd;
    unsigned short fldUdpSpd;
    unsigned short fldIcmpSpd;
    unsigned short fldPerIpTcpSynSpd;
    unsigned short fldPerIpTcpFinSpd;
    unsigned short fldPerIpUdpSpd;
    unsigned short fldPerIpIcmpSpd;
}SEC_DOS_ENTRY, *PSEC_DOS_ENTRY;

/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
typedef struct {
    int dmzEnable;
    struct in_addr dmzAddress;
}SEC_DMZ_ENTRY, *PSEC_DMZ_ENTRY;
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/

typedef struct {
    int  instID;
    char url[SCM_URLBLK_MAX+4]; // 4-bytes safety. <2008.12.22 tanyin>
}SEC_URLBLK_ENTRY, *PSEC_URLBLK_ENTRY;
//end vdf

#endif

