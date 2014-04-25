
/******************************************************************************

                  版权所有 (C), 2006-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sipdef.h
  版 本 号   : V1.0
  作    者   : shuxuegang
  生成日期   : 2006年11月6日
  最近修改   :
  功能描述   : 处理SIP业务相关参数定义，这些定义在web配置处理和psi数据库处理中将
               被用到
  函数列表   :
  修改历史   :

******************************************************************************/
#ifndef _SIP_DEF_H_
#define _SIP_DEF_H_

#include "typedef.h"
#include "tapi_def.h"
#include "macro.h"

// #define DEBUG_SIP

/*SIP业务数据在配置管理和业务模块之间进行共享用到的目录宏*/
// 属性前缀
#define SIP_CFGDIR_SIP_SERVER    "/var/sip/server"
#define SIP_CFGDIR_SIP_BASIC     "/var/sip/basic"
#define SIP_CFGDIR_SIP_ADVANCED  "/var/sip/advanced"
#define SIP_CFGDIR_PSTN_PREFIX   "/var/sip/pstncallprefix"
#define SIP_CFGDIR_SIP_QOS       "/var/sip/qos"
#define SIP_CFGDIR_PHONE_COMMON  "/var/sip/phonecommon"
#define SIP_CFGDIR_SPEED_DIAL    "/var/sip/speeddial"
#define SIP_CFGDIR_CALL_FORWARD  "/var/sip/callforward"
// 文件名
#define SIP_CFG_FILE             "/var/sip"
#define SIP_STATUS_FILE          "/var/sipstatus"
/* BEGIN: Added by p00102297, 2008/3/11   问题单号:VDF C02*/
#define SIP_FXS_STATUS_FILE      "/var/fxsstatus"
/* END:   Modified by p00102297, 2008/3/11 VDF C02*/

#define SIP_TINY_LEN            16
#define SIP_SMALL_LEN           32
#define SIP_MEDIUM_LEN          40
#define SIP_LARGE_LEN           264
#define SIP_GIANT_LEN           1024
#define SIP_MAX_IFNAME_STR_LEN  17


// 实现主备server的配置长度为2的数组,0是主,1是备
#define SIP_SRV_MAX         2 
#define SIP_SRV_PRIMARY     0
#define SIP_SRV_SECONDARY   1

// 支持两个用户, 0-phone1, 1-phone2
#define SIP_USER_MAX        2 
#define SIP_FIRST_USER      0
#define SIP_SECOND_USER     1

/*BEGIN guwenbin 20080705 AU8D00774  QuickDialNumber 需配置40位长度*/
#define SIP_MAX_DAILNUM_LEN                    41    /*用户dial号码的最大长度  */
/*END guwenbin 20080705 AU8D00774  QuickDialNumber 需配置40位长度*/

/* BEGIN:   Added by hKF20513, 2009/11/12 */
#define SIP_LOCAL_PORT_START                   1024   
#define SIP_LOCAL_PORT_END                       65535   
/* END:   Added by hKF20513, 2009/11/12 */

/* BEGIN:   Modify by hKF20513, 2009/11/12 */
#define SIP_MAX_NAME_LEN                   65//68    /*用户名称的最大长度*/
#define SIP_MAX_NUM_LEN                    65//68    /*用户号码的最大长度  */
#define SIP_MAX_PSW_LEN                    65//68    /*用户密码的最大长度*/
/* END:   Modify by hKF20513, 2009/11/12 */

#define SIP_MAX_DIAL_PLAN                  10   /*快速拨号条目最大个数*/
#define SIP_MAX_CALL_FWD                   2     /*呼叫前转表个数*/
#define SIP_MAX_DIGIT_MAP                  1024
#define SIP_MIN_NOREPLY_TIME               20
#define SIP_MAX_NOREPLY_TIME               60
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
#define PSTN_MAX_PREFIX_LEN                    9    /*最大pstn号首长度*/


// SIP server default
#define  DEFAULT_PROXY_ADDR_STRING      ""
#define  DEFAULT_PROXY_PORT             5060
#define  DEFAULT_REG_ADDR_STRING        ""
#define  DEFAULT_REG_PORT               5060
#define  DEFAULT_DOMAIN_NAME_STRING     ""

// SIP basic default
#define  DEFAULT_SIP_ID_STRING          ""
#define  DEFAULT_USR_NAME_STRING        ""
#define  DEFAULT_USR_PASSWD_STRING      ""
#define  DEFAULT_SIP_LOCAL_PORT         6050
// #define  DEFAULT_CALL_FW_TBLID          1
#define  DEFAULT_USR1_CALL_FW_TBLID          1
#define  DEFAULT_USR2_CALL_FW_TBLID          2
#define  DEFAULT_SIP_CLIP_ENABLE		1			

// SIP Advanced default
#define  DEFAULT_EXPIRATION_DURATION      3600
#define  DEFAULT_REREGISTER_TIMER         10
#define  DEFAULT_SESSION_EXPIRES          32000  //default value modified by chenyong for RFC4028 
#define  DEFAULT_MIN_SE                   90    //default value modified by chenyong for RFC4028
/* BEGIN: Modified by p00102297, 2008/1/29 */
#define  DEFAULT_MAXRETRY_TIME            32
#define  DEFAULT_PRIMARY_CHECK_TIME       300
/* END:   Modified by p00102297, 2008/1/29 */
/* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
#define  DEFAULT_MWI_EXPIRE               3600
/* END:   Added by chenzhigang, 2008/11/5 */
#define  DEFAULT_RTP_START_PORT           50000
#define  DEFAULT_RTP_END_PORT             65535
/*start of modification by chenyong 2009-03-05*/
#define  DEFAULT_RTCP_MONITOR             0  //default this function is disabled
/*start of modification by chenyong 2009-03-05*/

#define  DTMF_TYPE_PCM                    0
#define  DTMF_TYPE_RFC2833                1
#define  DTMF_TYPE_SIP_INFO               2
#define  DEFAULT_VOICE_DTMF_TYPE          DTMF_TYPE_PCM
//BEGIN:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069
#define	 DEFAULT_VOICE_PAYLOAD_TYPE		  101
//END:Added by zhourongfei to config the PT type via HTTP, telnet and TR-069

#define  FAX_TYPE_G711A_PASSTHROUGH       0
#define  FAX_TYPE_G711U_PASSTHROUGH       1
#define  FAX_TYPE_T38                     2
#define  FAX_TYPE_PASSTHROUGH             FAX_TYPE_G711A_PASSTHROUGH
#define  DEFAULT_FAX_TYPE                 FAX_TYPE_PASSTHROUGH
#define  DEFAULT_SIP_INTERFACE_STRING     "br0"
#define  DEFAULT_SIP_PSTN_PRFFIX_STRING   "0000"
#define  SIP_TEST_DISABLED                0
#define  SIP_TEST_ENABLED                 1
#define  DEFAULT_SIP_TEST_ENBL_FLAG       SIP_TEST_DISABLED
#define  DEFAULT_SIP_DSCP_MARK            46         /* Added by s00125931, 2008/9/16   问题单号:vhg556*/
/* BEGIN: Modified by p00102297, 2008/1/17 */
#define PACKET_PERIOD_FIRST               10
#define PACKET_PERIOD_SECOND              20
#define PACKET_PERIOD_THIRD               30
#define DEFAULT_VOICE_PACKET_PERIOD       PACKET_PERIOD_SECOND
#define DEFAULT_VOICE_RTCPREPEAT_INTERVAL 3000
/* END:   Modified by p00102297, 2008/1/17 */

/* BEGIN: Modified by p00102297, 2008/2/21 */
/* 摘机的默认时间OffHookTime */
#define DEFAULT_VOICE_OFFHOOK_TIME        250
/* 挂机的默认时间OnHookTime */
#define DEFAULT_VOICE_ONHOOK_TIME         250
/* VAD的默认配置值 */
#define DEFAULT_SIP_VAD_VALUE             SIP_VAD_DISABLE
/* EC的默认配置值 */
#define DEFAULT_SIP_EC_VALUE              SIP_EC_ENABLE
/* CNG的默认配置值*/
#define DEFAULT_SIP_CNG_VALUE             SIP_CNG_DISABLE
/* 收听音量LstVol的默认配置值 */
#define DEFAULT_VOICE_LISTEN_VOL          0
/* 说话放音SpkVol的默认配置值 */
#define DEFAULT_VOICE_SPEAK_VOL           0
/* 按键等待时间的默认配置值，单位:100ms */
#define DEFAULT_VOICE_DIAL_INTERVAL       50
/* 摘机拨号等待时间的默认配置值，单位:100ms */
#define DEFAULT_VOICE_FIRST_DIAL          150
/* 最小拍叉时间的默认配置值 */
#define DEFAULT_VOICE_MINHOOK_FLASH       80
/* 最大拍叉时间的默认配置值 */
#define DEFAULT_VOICE_MAXHOOK_FLASH       250
/* 数图配置的默认值 */
#define DEFAULT_VOICE_DIGITMAP            "[X*#ABCD].T"

//add by z67625 for automatic call
/*自动拨号时间间隔*/
#define DEFAULT_VOICE_AUTOCALLINTERVAL    50
/*自动拨号号码*/
#define DEFAULT_VOICE_AUTOCALLNUM         ""
#define DEFAULT_VOICE_AUTOCALLENBL        0

/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
/* reInjection 默认值 */
#define DEFAULT_VOICE_REINJECTION         VOICE_REINJECTION_ENABLE
/*END: Added by chenyong 65116 2008-09-15 web配置*/
/*start of additon by chenyong 2008-10-28 for Inner call*/
#define DEFAULT_VOICE_INNERCALL           INNER_CALL_ENABLE
/*end of additon by chenyong 2008-10-28 for Inner call*/
#define DEFAULT_VOICE_MODEMSERVICE        MODEM_SERVICE_ENABLE
/*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
#define DEFAULT_VOICE_VOICEPROMPTVISIBLE  VOICEPROMPTVISIBLE_ENABLE
/*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/

/* 国家码的默认配置值 */
/* BEGIN: Added by guwenbin, 2008/7/17 Spain特殊定制*/	
//#define DEFAULT_SIP_REGION_LOCAL          SIP_ITALY
#define DEFAULT_SIP_REGION_LOCAL          SIP_SPAIN
/* END:	   Added by guwenbin, 2008/7/17 */
/* 线路使能值的默认配置值 */
#define DEFAULT_SIP_LINE_ENABLE           SIP_LINE_ENABLE
/* 线路呼叫等待的默认配置值 */
#define DEFAULT_SIP_LINE_CALLWAIT         SIP_LINE_CALLWAIT_ENABLE

/*start of addition by chenyong 2009-03-19*/
#define DEFAULT_SIP_LINE_CALLHOLD         SIP_LINE_CALLHOLD_ENABLE
/*end of addition by chenyong 2009-03-19*/

/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
/* 线路三方通话的默认配置值 */
#define DEFAULT_SIP_LINE_3WAY             SIP_LINE_3WAY_ENABLE
/* 线路呼叫转移的默认配置值 */
#define DEFAULT_SIP_LINE_CALLTRANSFER     SIP_LINE_CALLTRANSFER_ENABLE
/*END: Added by chenyong 65116 2008-09-15 web配置*/
/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
#define DEFAULT_SIP_LINE_MWI              SIP_LINE_MWI_ENABLE
#define DEFAULT_SIP_MWIACCOUNT_STRING     ""
/* END:   Added by chenzhigang, 2008/11/4 */
/* 传真检测方向的默认值 */
#define DEFAULT_SIP_FAX_DETECT            SIP_FAX_DETECT_DIRECTION_BOTH

/* END:   Modified by p00102297, 2008/2/21 */

// SIP QoS default
#define  DEFAULT_QOS_TOS                  0
#define  DEFAULT_QOS_VLAN_ENBL            0
#define  DEFAULT_QOS_VLANID               0
#define  DEFAULT_QOS_PRIORITY             3

/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
#define  DEFAULT_PROV_DISABLE                  0
/* END:   Added by y67514, 2009/11/16 */

// 1.SIP服务器配置 - b. SIP服务器配置
// a.SIP Server配置,包括Registar和Proxy：
typedef struct tagSIP_WEB_ADDR_S
{
    VOS_UINT16  uwPort;                     /* PORT in local Byte Order */
    VOS_CHAR    szIPAddr[SIP_MAX_NAME_LEN]; /* IP address or FQHN */
}SIP_WEB_ADDR_S;

// b. SIP服务器配置
typedef struct tagSIP_WEB_SERVER_S
{
    SIP_WEB_ADDR_S  stSipRegAddr;                         /* 主用registar  */
    SIP_WEB_ADDR_S  stSipProxyAddr;                       /* 主用proxy  */
    VOS_CHAR        acDomainName[SIP_MAX_NAME_LEN];            /* 域的名子, gvt对接要求  */
}SIP_WEB_SERVER_S, *PSIP_WEB_SERVER_S;

/*可以变换(增加或减少前缀后)的电话号码的最大个数*/
#define SIP_MAX_DAILNUM_CHANGE_NUM 	16

/*BEGIN guwenbin 20080705 AU8D00774  将stAppandPrefixNum对应的
   SIP_MAX_MATCHNUM_SIZE改为64+1*/
 /*增加或减少的前后缀名的最大字符个数*/
//#define SIP_MAX_ASIDENUM_SIZE		(16)
#define SIP_MAX_ASIDENUM_SIZE		(42+1)

/*待匹配号码的最大字符个数*/
//#define SIP_MAX_MATCHNUM_SIZE		(32)
#define SIP_MAX_MATCHNUM_SIZE		(64+1)
/*END guwenbin 20080705 AU8D00774  */

typedef struct tagSIP_NUM_TRAN_S
{
    VOS_CHAR szAsideNum[SIP_MAX_ASIDENUM_SIZE]; /*所需要加或减的前后缀名*/
    VOS_CHAR szMatchNum[SIP_MAX_MATCHNUM_SIZE]; /*待匹配的号码*/
}SIP_NUM_TRAN_S;

/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
#if 0
#define SIP_APPAND_DIAL_NUM        16     /*支持最大号码增加个数*/
#define SIP_STRIP_DIAL_NUM         16     /*支持最大号码减少个数*/
#endif
/*BEGIN guwenbin 20080705 AU8D00774  将SIP_MAX_NUM_LEN/4改为SIP_MAX_DAILNUM_LEN notice!!*/
// #define SIP_PRENUM_SIZE			   (SIP_MAX_NUM_LEN/4)
// #define SIP_MATCHNUM_SIZE		   (SIP_MAX_NUM_LEN*3/4+1)
#define SIP_PRENUM_SIZE			   (SIP_MAX_DAILNUM_LEN)
#define SIP_MATCHNUM_SIZE		   (SIP_MAX_MATCHNUM_SIZE)
/*END guwenbin 20080705 AU8D00774  QuickDialNumber 需配置40位长度*/
#if 0
typedef struct tagSIP_NUM_TRAN_S
{
    VOS_CHAR szPrefixNum[SIP_PRENUM_SIZE];     /*所需要加或减前缀*/
    VOS_CHAR szMatchNum[SIP_MATCHNUM_SIZE];    /*待匹配的号码*/
}SIP_NUM_TRAN_S;
#endif
#endif

// 3. SIP Advanced - d.SIP高级配置
// a.服务器相关的高级配置
typedef struct tagSIP_WEB_SVR_CFG_S
{
    /* BEGIN: Added by y67514, 2009/11/4   PN:增加语音tr069的配置*/
    VOS_UINT32  ulRegPeriod;
    /* END:   Added by y67514, 2009/11/4 */
    VOS_UINT32  ulExpiration;
    VOS_UINT32  ulReRegister;  
    VOS_UINT32  SessionExpires;
    VOS_UINT32  MinSE;
    /* BEGIN: Modified by p00102297, 2008/1/17 */ 
    VOS_UINT32  ulMaxRetryTime;          /*参见RFC3261 64T1重传时间*/
    VOS_UINT32  ulPrimaryCheckTime;      /*周期检测主服务器时间*/
    /* END:   Modified by p00102297, 2008/1/17 */
    /* BEGIN: Added by chenzhigang, 2008/11/5   问题单号:MWI_SUPPORT*/
    VOS_UINT32 ulMWIExpire;
    /* END:   Added by chenzhigang, 2008/11/5 */
}SIP_WEB_SVR_CFG_S;

// b. RTP配置,起止端口号
typedef struct tagSIP_WEB_RTP_CFG_S
{
    VOS_UINT32  ulStartPort;
    VOS_UINT32  ulEndPort;
    /* BEGIN: Modified by p00102297, 2008/1/18 */
    VOS_UINT32  ulTxRepeatInterval;
    /* END:   Modified by p00102297, 2008/1/18 */
    /*start of modification by chenyong 2009-03-05*/
	VOS_UINT32  ulRtcpMonitor;
	/*end of modification by chenyong 2009-03-05*/
}SIP_WEB_RTP_CFG_S;

typedef struct tagSIP_WEB_VOICE_CODEC_LIST_S
{
    VOS_UINT32  ulCodecCap;      /*codec类型,详细见CMD_PARA_MEDIA_PROTOCAL_E*/
	VOS_UINT32  ulPTime;         /*codec打包时长*/
	VOS_UINT32  ulPriority;      /*codec 优先级,不超过SIP_CODEC_TYPE_MAX*/
	VOS_BOOL    bEnable;         /*codec 使能*/
}SIP_WEB_VOICE_CODEC_LIST_S;

/* BEGIN: Added by chenzhigang, 2007/12/28   问题单号:VDF SDP部分详细设计*/
// c. 优选编解码顺序
typedef struct tagSIP_WEB_VOICE_CFG_S
{
    SIP_WEB_VOICE_CODEC_LIST_S stCodecList[SIP_MAX_VOIP_CODE_TYPE];
    VOS_UINT32  ulDtmfType;      /*DTMF传输方式*/
	VOS_UINT32	ulPayloadType;	/*add by zhourongfei to config the PT type via HTTP, telnet and TR-069*/
}SIP_WEB_VOICE_CFG_S;
/* END:   Added by chenzhigang, 2007/12/28 */

// d.SIP高级配置
typedef struct tagSIP_WEB_ADV_CFG_S
{
    SIP_WEB_SVR_CFG_S    stServerCfg;
    SIP_WEB_RTP_CFG_S    stRTPCfg;
//    SIP_WEB_VOICE_CFG_S  stVoiceCfg;
//    VOS_UINT32           ulFaxType;
    VOS_CHAR             szIfName[SIP_MAX_IFNAME_STR_LEN];          /* Interface name */
    VOS_UINT32           ulSipLocalPort;
//s00125931 规范命名PSTN_MSX_PREFIX 改为PSTN_MAX_PREFIX_LEN
    VOS_CHAR             szPstnPrefix[PSTN_MAX_PREFIX_LEN];          /* Interface name */    
    VOS_UINT32           ulEnblSipTest;
    VOS_UINT32           ulDscp;                                 /*Added by s00125931, 2008/9/10   问题单号:vhg556*/
}SIP_WEB_ADV_CFG_S, *PSIP_WEB_ADV_CFG_S;

// 4. SIP QOS
typedef struct tagSIP_WEB_QOS_CFG_S
{
    VOS_UINT32    ulTos;
    VOS_UINT32    ulEnblVlan;
    VOS_UINT32    ulVlan;
    VOS_UINT32    ulPriority;
}SIP_WEB_QOS_CFG_S, *PSIP_WEB_QOS_CFG_S;

/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
// 5. SIP provisioned
typedef struct tagSIP_WEB_PROV_CFG_S
{
    VOS_UINT16   usCliStatusProv; 
    VOS_UINT16   usLineCallHoldEnableProv;
    VOS_UINT16   usLineCWEnableProv;
    VOS_UINT16   usLine3WayEnableProv;  
    VOS_UINT16   usLineCallTransferEnableProv;
    VOS_UINT16   usLineMWIEnableProv;
    VOS_UINT16   usLineAutoCallEnableProv;
}SIP_WEB_PROV_CFG_S, *PSIP_WEB_PROV_CFG_S;
/* END:   Added by y67514, 2009/11/16 */

#define SIP_MAX_DIGIT_MAP                  1024
//Phone页面：
typedef struct tagSIP_WEB_PHONE_CFG_S
{
    VOS_INT32     lSpkVol;
    VOS_INT32     lLstVol;
    VOS_UINT32    ulDialInterval;
    //VOS_UINT32    ulVad;
    VOS_UINT32    ulRegion;
    VOS_UINT32    minHookFlash;    /* 拍叉下限 */
    VOS_UINT32    maxHookFlash;    /* 拍叉上限 */
    VOS_UINT8     szDigitMap[SIP_MAX_DIGIT_MAP];
    /* BEGIN: Modified by p00102297, 2008/1/17 */
    //VOS_UINT32    ulEC;
    //VOS_UINT32    ulCNG;
    VOS_UINT32    ulOffHookTime;
    VOS_UINT32    ulOnHookTime;
    VOS_UINT32    ulFstDialTime;
    /* END:   Modified by p00102297, 2008/1/17 */
	/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
	VOS_UINT32    ulReInjection;     /*详细见VOICE_REINJECTION_E , Added by s00125931, 2008/9/10   问题单号:vhg556*/
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
	/*start of additon by chenyong 2008-10-28 for Inner call*/
	VOS_UINT32    ulInnerCall;      /*默认为INNER_CALL_ENABLE,通过配置文件控制*/
	/*end of additon by chenyong 2008-10-28 for Inner call*/
	VOS_UINT32    ulModemService;   /*默认为MODEM_SERVICE_ENABLE,通过配置文件控制*/
    /*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
    VOS_BOOL      bVoicePromptVisible;
    /*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
    //add by z67625 for automatic call
    VOS_INT32     lAutoEnbl;        /*自动拨号开关*/
    VOS_UINT32    ulAutoInterval;   /*自动拨号时间*/    
    VOS_CHAR      acAutoDialNum[SIP_SMALL_LEN]; /*自动拨号号码*/
}SIP_WEB_PHONE_CFG_S;

// 2. SIP用户信息 - SIP Basic
typedef struct tagSIP_WEB_USER_INFO_S
{
    /* BEGIN: Modified by p00102297, 2008/2/19 */
    SIP_WEB_VOICE_CFG_S  stVoiceCfg;
    VOS_UINT32    ulVad;
    VOS_UINT32    ulEC;
    VOS_UINT32    ulCNG;
    VOS_UINT32    ulFaxType;
    /* END:   Modified by p00102297, 2008/2/19 */
    VOS_CHAR    szId[SIP_MAX_NUM_LEN];      /* 用户标识 */
    VOS_CHAR    szName[SIP_MAX_NAME_LEN];   /* 用户别名 */
    VOS_CHAR    szPsw[SIP_MAX_PSW_LEN];     /* 注册密钥 */
    VOS_UINT16  uwPort;			            /* PORT in local Byte Order */
    VOS_UINT32  ulFwdTbl;                   /* 呼叫前转表id:1 or 2 */

/*如果为意大利版本 zhoubaiyun 64446 for ITALY*/
#ifdef SUPPORT_CUSTOMER_ITALY
    VOS_UINT16   usCliStatus;              /* TRUE:传送主叫号码;FALSE:隐藏主叫号码 */
    /* BEGIN: Added by lihua 68693, 2007/10/20   问题单号:A36D03793*/
    //VOS_UINT16   usCliPrivacyType;         /* 主叫号码隐藏时的隐藏方式 */
    /* END:   Added by lihua 68693, 2007/10/20 */
    /* BEGIN: Modified by p00102297, 2008/2/3 */
    VOS_UINT16   usLineEnable;
    /* END:   Modified by p00102297, 2008/2/3 */
	/*start of addition by chenyong 2009-03-19*/
    VOS_UINT16   usLineCallHoldEnable;
	/*end of addition by chenyong 2009-03-19*/
    /* BEGIN: Modified by p00102297, 2008/3/14   问题单号:VDF C02*/
    VOS_UINT16   usLineCWEnable;
    /* END:   Modified by p00102297, 2008/3/14 */
	/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
    VOS_UINT16   usLine3WayEnable;              /*启闭三方通话功能,见SIP_LINE_3WAY_E*/
    VOS_UINT16   usLineCallTransferEnable;      /*呼叫转移功能,    见SIP_LINE_CALLTRANSFER_E*/
	/*END: Added by chenyong 65116 2008-09-15 web配置*/
    /* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
    VOS_UINT16   usLineMWIEnable;                  /*启动MWI,见SIP_LINE_MWI_ENABLE*/
    VOS_CHAR     szLineMWIAccount[SIP_MAX_NUM_LEN];
    /* END:   Added by chenzhigang, 2008/11/4 */

    /* BEGIN: Modified by p00102297, 2008/4/7   问题单号:VDF C02*/
    VOS_UINT16   usLineFaxDetect;
    /* END:   Modified by p00102297, 2008/4/7 */
    
    SIP_NUM_TRAN_S		   stAppandPrefixNum[SIP_MAX_DAILNUM_CHANGE_NUM];	/*增加前缀*/
	SIP_NUM_TRAN_S		   stStripPrefixNum[SIP_MAX_DAILNUM_CHANGE_NUM];	/*减少前缀*/
	SIP_NUM_TRAN_S		   stAppandSuffixNum[SIP_MAX_DAILNUM_CHANGE_NUM];	/*增加后缀*/
	SIP_NUM_TRAN_S		   stStripSuffixNum[SIP_MAX_DAILNUM_CHANGE_NUM];	/*减少后缀*/
#if 0
    SIP_NUM_TRAN_S         stAppNum[SIP_APPAND_DIAL_NUM];       /*匹配增加号码的记录*/
    SIP_NUM_TRAN_S         stStripNum[SIP_STRIP_DIAL_NUM]; 	    /*匹配减少号码的记录*/
#endif
#endif  
}SIP_WEB_USER_INFO_S, *PSIP_WEB_USER_INFO_S;


//Phone Region页面：
typedef enum tagSIP_PHONE_REGION_E
{
    SIP_BELGIUM = 0,
    SIP_BRAZIL,
    SIP_CHILE,
    SIP_CHINA,
    SIP_DENMARK,
    SIP_ETSI,           // 5
    SIP_FINLAND,
    SIP_FRANCE,
    SIP_GERMANY,
    SIP_HUNGARY,
    SIP_ITALY,          // 10
    SIP_JAPAN,
    SIP_NETHERLANDS,
    SIP_NORTH_AMERICA,
    SIP_SPAIN,
    SIP_SWEDEN,         // 15
    SIP_SWITZERLAND,
    SIP_TR57,
    SIP_UK,              // 18
    SIP_SINGAPORE		//19
}SIP_WEB_PHONE_REGION_E;


//Speed Dial配置
typedef struct tagSIP_WEB_DIAL_PLAN_S
{
/*BEGIN guwenbin 20080705 AU8D00774  将SIP_MAX_NUM_LEN/4改为SIP_MAX_DAILNUM_LEN*/
   //VOS_UINT8 szDialNum[SIP_MAX_NUM_LEN/4];
   VOS_UINT8 szDialNum[SIP_MAX_DAILNUM_LEN];
/*END guwenbin 20080705 AU8D00774  QuickDialNumber 需配置40位长度*/
    VOS_UINT8 szName[SIP_MAX_NUM_LEN];
    VOS_UINT8 szRealNum[SIP_MAX_NUM_LEN];
}SIP_WEB_DIAL_PLAN_S;

//呼叫前转表：
typedef enum tagSIP_WEB_FWD_TYPE_E
{
    SIP_WEB_UNCON_FWD,
    SIP_WEB_NOREP_FWD,
    SIP_WEB_ONBSY_FWD,
    SIP_WEB_BUUTT_FWD
}SIP_WEB_FWD_TYPE_E;


/*Privacy's Tokens*/
typedef enum _EN_SIP_PRIVACY_HDR_TOKEN_ID
{
    SIP_PRIVACY_TOKEN_HEADER = 1,
    SIP_PRIVACY_TOKEN_USER,
    SIP_PRIVACY_TOKEN_SESSION,
    SIP_PRIVACY_TOKEN_NONE,
    SIP_PRIVACY_TOKEN_CRITICAL,
    SIP_PRIVACY_TOKEN_ID,
    SIP_PRIVACY_TOKEN_OTHER,
    SIP_PRIVACY_TOKEN_BUTT

}EN_SIP_PRIVACY_HDR_TOKEN_ID;

/* BEGIN: Modified by p00102297, 2008/2/3 */
typedef enum tag_SIP_LINE_ENABLE_EN
{
    SIP_LINE_ENABLE    = 0,
    SIP_LINE_DISABLE   = 1,
    SIP_LINE_QUIESCENT = 2
}EN_SIP_LINE_ENABLE_ST;

typedef enum tag_SIP_LINECLIP_EN
{
    SIP_LINECLIP_DISABLE = 0,
    SIP_LINECLIP_ENABLE  = 1
}SIP_LINECLIP_EN;

typedef enum tag_SIP_VAD_EN
{
    SIP_VAD_DISABLE = 0,
    SIP_VAD_ENABLE  = 1
}SIP_VAD_EN;

typedef enum tag_SIP_EC_EN
{
    SIP_EC_DISABLE = 0,
    SIP_EC_ENABLE  = 1
}SIP_EC_EN;

typedef enum tag_SIP_CNG_EN
{
    SIP_CNG_DISABLE = 0,
    SIP_CNG_ENABLE  = 1
}SIP_CNG_EN;

typedef enum tag_SIP_PREFIX_TYPE_EN
{
    SIP_PREFIX_APPEND = 0,
    SIP_PREFIX_STRIP  = 1,
    SIP_SUFFIX_APPEND = 2,
    SIP_SUFFIX_STRIP  = 3
}SIP_PREFIX_TYPE_EN;

typedef enum tag_SIP_LINE_CALLWAIT_E
{
    SIP_LINE_CALLWAIT_DISABLE = 0,
    SIP_LINE_CALLWAIT_ENABLE
}SIP_LINE_CWEN_E;
/* END:   Modified by p00102297, 2008/2/3 */

/*start of addition by chenyong 2009-03-19*/
typedef enum tag_SIP_LINE_CALLHOLD_E
{
    SIP_LINE_CALLHOLD_DISABLE = 0,
    SIP_LINE_CALLHOLD_ENABLE
}SIP_LINE_CALLHOLD_E;
/*end of addition by chenyong 2009-03-19*/

/* BEGIN: Added by chenzhigang, 2008/11/4   问题单号:MWI_SUPPORT*/
typedef enum tag_SIP_LINE_MWI_E
{
    SIP_LINE_MWI_DISABLE = 0,
    SIP_LINE_MWI_ENABLE
}SIP_LINE_MWI_E;
/* END:   Added by chenzhigang, 2008/11/4 */
/*BEGIN: Added by chenyong 65116 2008-09-15 web配置*/
typedef enum tag_SIP_LINE_3WAY_E
{
    SIP_LINE_3WAY_DISABLE = 0,
    SIP_LINE_3WAY_ENABLE
}SIP_LINE_3WAY_E;

typedef enum tag_SIP_LINE_CALLTRANSFER_E
{
    SIP_LINE_CALLTRANSFER_DISABLE = 0,
    SIP_LINE_CALLTRANSFER_ENABLE
}SIP_LINE_CALLTRANSFER_E;

typedef enum tag_VOICE_REINJECTION_En
{
    VOICE_REINJECTION_DISABLE = 0,
    VOICE_REINJECTION_ENABLE,
    VOICE_REINJECTION_FORCE
}VOICE_REINJECTION_E;
/*END: Added by chenyong 65116 2008-09-15 web配置*/

/* start: add by y00183561 for automatic call tr069 node 2011/10/12 */
typedef enum tag_AUTO_CALL_ENABLE_E
{
    AUTO_CALL_DISABLE = 0,
    AUTO_CALL_ENABLE
}AUTO_CALL_ENABLE_E;
/* end: add by y00183561 for automatic call tr069 node 2011/10/12 */

/* BEGIN: Modified by p00102297, 2008/3/13   问题单号:VDF C02*/
typedef enum tag_Voice_FXS_Status
{
    VOICE_FXS_STAT_START = -1,
    VOICE_FXS_STAT_ONHOOK = 0,
	VOICE_FXS_STAT_OFFHOOK,
	VOICE_FXS_STAT_BUTT
}VOICE_FXS_STATUS_E;

/* BEGIN: Added by chenzhigang, 2008/4/7   问题单号:传真主被叫方向检测*/
typedef enum tag_SIP_FAX_DETECT_DIRECTION_TYPE_E
{
    SIP_FAX_DETECT_DIRECTION_DISABLE = 0,
    SIP_FAX_DETECT_DIRECTION_CALLEE,
    SIP_FAX_DETECT_DIRECTION_CALLER,
    SIP_FAX_DETECT_DIRECTION_BOTH,
    SIP_FAX_DETECT_DIRECTION_BUTT
}SIP_FAX_DETECT_DIRECTION_TYPE;
/* END:   Added by chenzhigang, 2008/4/7 */

/* END:   Modified by p00102297, 2008/3/13 */

/*   T A G   S   I   P _   A   D   V   A   N   C   E   D _   F   W   D   */
/*-------------------------------------------------------------------------
    呼叫前转配置结构
-------------------------------------------------------------------------*/
typedef struct tagSIP_WEB_FWD_NUM_S
{
    VOS_BOOL       bIsActive;
    VOS_UINT8      szIncoming[SIP_MAX_NUM_LEN];
    VOS_UINT8      szForward[SIP_MAX_NUM_LEN];
    SIP_WEB_FWD_TYPE_E enFwdType; 
}SIP_WEB_FWD_NUM_S;

#define SIP_WEB_FWD_ADVANCED_NUM  10
#define SIP_WEB_FWD_BASIC_NUM      3
/*   T A G   S   I   P _   F   W   D _   N   U   M _   S   */
/*-------------------------------------------------------------------------
    呼叫前转表
-------------------------------------------------------------------------*/
typedef struct tagSIP_WEB_FWD_S
{
    SIP_WEB_FWD_NUM_S  pstAdvance[SIP_WEB_FWD_ADVANCED_NUM];    /*10个*/
    SIP_WEB_FWD_NUM_S  pstBasic[SIP_WEB_FWD_BASIC_NUM];         /*3个*/
    VOS_UINT32         ulNoReplyTime;
}SIP_WEB_FWD_S;

#endif
