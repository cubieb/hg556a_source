/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : tapi_def.h
  版 本 号   : V1.21(该版本号和TAPI-Interworking document.doc版本号报保持一致)
  作    者   : lilei 22608
  生成日期   : 2004年9月21日
  最近修改   :
  功能描述   : TAPI定义头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2004年9月21日
    作    者   : lilei 22608
    修改内容   : 生成
  2.日    期   : 2004年10月27日
    作    者   : lilei 22608
    修改内容   : 修改若干定义


******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifndef TAPI_DEF_H
#define  TAPI_DEF_H
#include "typedef.h"

#define SERVICE_MANAGE_TONE_BASE 61

typedef enum tagTONE_TYPE_E
{
    TONE_TYPE_BUSY_TONE = 1,                /*语音1:忙音*/
    TONE_TYPE_HOWL_TONE,                    /*语音2:嚎鸣音*/
    TONE_TYPE_DIAL_TONE,                    /*语音3:拨号音*/
    TONE_TYPE_RING_BACK_TONE,               /*语音4:回铃音*/
    TONE_TYPE_DUAL_DIAL_TONE,               /*语音5:二次拨号音*/
    TONE_TYPE_SPECIAL_DIAL_TONE,            /*语音6:特殊拨号音*/
    TONE_TYPE_SHORT_BUSY_TONE,              /*语音7:短忙音*/
    TONE_TYPE_CW_TONE,                      /*语音8:来话提醒音*/
    TONE_TYPE_MUTE_TONE,                    /*语音9:静音*/
    TONE_TYPE_CAS,                          /*语音10:CAS通话中主叫号码显示*/
    /* Start of ouyangyueling+29638 2005-03-23 TS0001 V300R001B01D030 FNRD04333 */
    TONE_TYPE_SAS,                          /*语音11:SAS*/
    /* End of ouyangyueling+29638 2005-03-23 TS0001 V300R001B01D030 FNRD04333 */
    TONE_TYPE_VOICEMAIL_ON_TONE,            /*语音12:点灯*/
    TONE_TYPE_VOICEMAIL_OFF_TONE,           /*语音13:熄灯*/
    /* Start of ouyangyueling+29638 2005-03-23 TS0001 V300R001B01D030 FNRD04333 */
    TONE_TYPE_CONGESTION_TONE,              /*语音14:阻塞音 cg*/
    TONE_TYPE_CONFIRM_TONE,                 /*语音15:确认音*/
    /* End of ouyangyueling+29638 2005-03-23 TS0001 V300R001B01D030 FNRD04333 */
    TONE_TYPE_RT_ALWAYS_TONE,               /*语音16:持续回铃音*/
    TONE_TYPE_USER_DEFINE_TONE1,   /*语音17:应用层定义语音（音乐）文件1*/
    TONE_TYPE_USER_DEFINE_TONE2,   /*语音18:应用层定义语音（音乐）文件2*/
    TONE_TYPE_USER_DEFINE_TONE3,   /*语音19:应用层定义语音（音乐）文件3*/
    TONE_TYPE_USER_DEFINE_TONE4,   /*语音20:应用层定义语音（音乐）文件4*/
    TONE_TYPE_USER_DEFINE_TONE5,   /*语音21:应用层定义语音（音乐）文件5*/
    /* Start of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
    TONE_TYPE_CW_CURRENT_TONE,              /*语音22:通用来话提醒音*/
    TONE_TYPE_EPHONE_RING_TONE,             /*语音23:EPHONE中的振铃接口*/
    /* End of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
    /* Start of ouyangyueling+29638 2005-05-30 TS0001 V300R001B01D070 A41D00415 */
    TONE_TYPE_ALARM_TONE,                   /*语音24:闹钟接口*/
    TONE_TYPE_ZERO_CONFIG_SHORT_ONE_TONE,   /*语音25:0配置短“嘀”声接口*/
    TONE_TYPE_ZERO_CONFIG_LONG_ONE_TONE,    /*语音26:0配置长“嘀”声接口*/
    TONE_TYPE_ZERO_CONFIG_THREE_TONE,       /*语音27:0配置三声“嘀”声接口*/  
    /* End of ouyangyueling+29638 2005-05-30 TS0001 V300R001B01D070 A41D00415 */
	/* BEGIN: Added by guwenbin, 2008/7/10	 Spain特殊定制*/
		TONE_TYPE_NORMAL_ALARM_TONE,			/*alarm tone*/
		TONE_TYPE_NORMAL_ACKNOWLEDGEMENT_TONE,	/*Positive Acknowledgement	tone*/
	/* END:   Added by guwenbin, 2008/7/10 */  
/* BEGIN: Added by chenzhigang, 2008/3/13   问题单号:CW使能关闭由终端来控制*/
    TONE_TYPE_ENABLE_CW_TONE = SERVICE_MANAGE_TONE_BASE,  /*文件音*/
    TONE_TYPE_DISABLE_CW_TONE,
    TONE_TYPE_CW_STATE_ENABLE_TONE,
    TONE_TYPE_CW_STATE_DISABLE_TONE,
/* END:   Added by chenzhigang, 2008/3/13 */    
    TONE_TYPE_TONE_BUTT                    /*最大语音值*/
}TONE_TYPE_E;

/******************************************************************************************
TAPI_PortOperate相关定义
当下发操作类型为PORT_CMD_DOWN_SET_TALK时，
下发参数pParam定义为SET_TALK_PARA_S *类型

当下发操作类型为PORT_CMD_DOWN_KC时，
下发参数pParam定义为SET_DOWN_KC_S *类型

******************************************************************************************/
typedef struct tagSET_TALK_PARA_S
{
    VOS_UINT32   ulDspChannelID;
}SET_TALK_PARA_S;

/* Start of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
typedef struct tagSET_DOWN_KC_S
{
    VOS_UINT32  ulSendCounts;                               /*次数*/
    VOS_UINT32  ulBreadth;                                  /*幅值*/
}SET_DOWN_KC_S;
/* End of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */

typedef enum  tagPORT_COMMAND_DOWN_E
{
    PORT_CMD_DOWN_SET_TALK,                      /*置通话，连小网*/
    PORT_CMD_DOWN_REL_TALK,                      /*释放通话，拆小网*/
    /*START ADD:liujianfeng 37298 2005-04-10 for IAD104I*/
    PORT_CMD_DOWN_B_CH_CONN_REQ = PORT_CMD_DOWN_SET_TALK, 		    /*B通道联网请求，L2->L1*/ 
    PORT_CMD_DOWN_B_CH_DISCONN_REQ= PORT_CMD_DOWN_REL_TALK,         /*B通道拆网请求，L2->L1*/ 
    PORT_CMD_DOWN_ACTIVATE_REQ,				    /*端口激活请求，L2->L1*/
    PORT_CMD_DOWN_DEACTIVATE_REQ,           	/*端口去激活请求，L2->L1*/
    PORT_CMD_DOWN_DATA_TX_REQ,				    /*D通道数据接收请求，L2->L1*/
    PORT_CMD_DOWN_DATA_RX_RST_REQ,              /*复位通道请求*/
   /*END ADD:liujianfeng 37298 2005-04-10 for IAD104I*/
    PORT_CMD_DOWN_START_RING,                   /*振铃*/
    PORT_CMD_DOWN_RING_SPLASH,                  /*短振铃*/
    PORT_CMD_DOWN_RING_SEND_TELNUM,             /*振铃送号,为AG预留*/
    PORT_CMD_DOWN_INIT_RING,                    /*初始振铃*/
    PORT_CMD_DOWN_SEND_TELNUM,                  /*送号,为AG预留*/
    PORT_CMD_DOWN_STOP_RING,                    /*停振铃*/
    PORT_CMD_DOWN_START_SERVICE,                /*启动业务,为AG预留*/ 
    PORT_CMD_DOWN_END_SERVICE,                  /*终止业务,为AG预留*/
    PORT_CMD_DOWN_LOCK,                         /*端口锁定*/
    PORT_CMD_DOWN_UNLOCK,                       /*端口解锁定*/
    PORT_CMD_DOWN_ENABLE_ONHOOK_TX,             /*挂机传输使能*/
    PORT_CMD_DOWN_REVERSE,                      /*反极*/
    /* Start of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
    PORT_CMD_DOWN_KC,                           /*12KC和16KC计费信息*/    
    /* End of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
    PORT_CMD_DOWN_SET_TALK_REVERSE,             /*置通话并且反极*/
    PORT_CMD_DOWN_SEND_AOCE,                    /*话费立显*/
    PORT_CMD_DOWN_CDI_CALL_OFFHOOK,             /*主叫摘机 CDI*/
    PORT_CMD_DOWN_CDI_DIAL_NUM,                 /*主叫拨号 CDI*/
    PORT_CMD_DOWN_CDI_CALL_ONHOOK,              /*主叫挂机 CDI*/
    PORT_CMD_DOWN_PORT_RESET,                   /* 复位指定端口命令*/
    PORT_CMD_DOWN_SEND_TTX_PULSE,               /* 送反极脉冲命令 */
    PORT_CMD_DOWN_DISC_FEEDBACK                 /*特殊业务断馈电命令*/
}PORT_COMMAND_DOWN_E;
/******************************************************************************************/



/******************************************************************************************
TAPI_SendCallerID相关定义
当下发操作类型为PORT_CMD_DOWN_SET_TALK时，
下发参数pParam定义为SET_TALK_PARA_S *类型
******************************************************************************************/
#define LENGTH_OF_DATE_BUF          5
#define LENGTH_OF_TIME_BUF          5
#define LENGTH_OF_CALLING_NUMBER    32
#define LENGTH_OF_CALLER_NAME       64
#define LENGTH_OF_DTMF_PADDING      3
typedef struct tagCALLER_ID_REPORT_S
{
    VOS_UINT8   aucDate[LENGTH_OF_DATE_BUF];                    /* The caller ID date field (ASCII - MMDD'\0') */
    VOS_UINT8   aucTime[LENGTH_OF_TIME_BUF];                    /* The caller ID time field (ASCII - HHMM'\0') */
    VOS_UINT8   aucNumber[LENGTH_OF_CALLING_NUMBER];            /* The calling directory number ,以'\0'为结束 */
    VOS_UINT8   aucName[LENGTH_OF_CALLER_NAME];                 /* The caller ID name */
    VOS_UINT8   ucStatus;                                                     /* Caller ID message status，值定义尚未确认 */
    VOS_UINT8   ucNumberAvailability;                           /* enum CALLER_ID_AVAIL_REASON_E */
    VOS_UINT8   ucNameAvailability;                             /* enum CALLER_ID_AVAIL_REASON_E */
    VOS_UINT8   ucHookStat;                                                 /* CID需要根据是否摘机态,来设定标志信
                                                                                             号的个数,定义见enum CALLERID_HOOK_STAT_E*/
}CALLER_ID_REPORT_S;

/*标识是否可以显示主机号码或者主叫姓名，根据IAD的代码定义出来*/
typedef enum tagCALLER_ID_AVAIL_REASON_E
{
     CALLERID_AVAIL     =   0,       /*可以显示*/
     CALLERID_UNAVAIL   =   'O',     /*不能获得主叫号码，国标中ydn069.pdf规定*/
     CALLERID_PRIVATE   =   'P'      /*不允许显示(处理和不能获得主叫号码相同)*/          
}CALLER_ID_AVAIL_REASON_E;

typedef enum tagCALLERID_HOOK_STAT_E
{
     CALLERID_ONHOOK,       /*挂机态下的主叫号码显示*/
     CALLERID_OFFHOOK,      /*通话态下的主叫号码显示*/
     CALLERID_BUTT
}CALLERID_HOOK_STAT_E;
/******************************************************************************************/


/******************************************************************************************
TAPI_ChannelOpen相关定义
******************************************************************************************/
typedef enum tagWORK_MODE_DEFINE_E
{
    WORK_MODE_DTMF,         /*收号模式，所有变更标志位必须为PARA_CHANGE_NO（0）*/
    WORK_MODE_VOICE,        /*语音模式*/
    WORK_MODE_FAX,          /*传真模式,编解码方式必须设置为T.38或G.711 */
    WORK_MODE_MODEM,        /*modem模式，编解码方式必须设置为V.150或G.711*/
    WORK_MODE_BUTT
}WORK_MODE_DEFINE_E;
 

typedef struct tagDSP_CONFIG_PARA_S
{
    VOS_UINT32  ulSilence;                              /*静音*/
    VOS_UINT32  ulEcho;                                 /*回声*/
    /* BEGIN: Added by chenzhigang, 2008/1/25   问题单号:VDF SDP部分详细设计*/
    VOS_UINT32  ulComfortNoise;                         /*舒适噪音*/
    /* END:   Added by chenzhigang, 2008/1/25 */
    VOS_UINT32  ulDtmfTransferMode;                     /*DTMF传输模式*/
    VOS_UINT32  ulRemoteCode;                           /*远端编解码方式*/
    VOS_UINT32  ulRemotePeriod;                         /*远端打包时间长度，单位MS*/
    VOS_UINT32  ulRemotePlType;                         /*对端PT值*/
    VOS_UINT32  ulLocalCode;                            /*本端编解码方式*/
    VOS_UINT32  ulLocalPlType;                          /*本端PT值*/
    VOS_UINT32  ulLocalPeriod;                          /*本端打包时间长度，单位MS*/
    VOS_UINT32  ulMode;                                 /*连接模式*/
 /*Modify begin HG550V100R002C02B010 A36D03254 by Donald 2007-1-10*/
    VOS_UINT32  ulWorkMode;                             /*工作方式 WORK_MODE_DEFINE_E*/
 /*Modify end HG550V100R002C02B010 A36D03254 by Donald 2007-1-10*/
    VOS_INT32    lOutputGain;                           /* DSP通道发送增益，单位DB */
    VOS_INT32    lInputGain;                            /* DSP通道接收增益，单位DB */
}DSP_CONFIG_PARA_S;
/*ulSilence的填写规则*/
#define CHANNEL_PARA_SILENCE_DISABLE   0                    /*关闭静音*/
#define CHANNEL_PARA_SILENCE_ENABLE   1                     /*打开静音*/
/*ulEcho的填写规则*/
#define CHANNEL_PARA_EC_DISABLE   0                         /*关闭回声抑止*/
#define CHANNEL_PARA_EC_ENABLE    1                         /*打开回声抑止*/
/*ulDtmfTransferMode的填写规则*/
#define CHANNEL_PARA_DTMF_DIRECT_TRANS_MODE    0            /*透传*/
#define CHANNEL_PARA_DTMF_2833_MODE                    1    /*2833*/
#define CHANNEL_PARA_DTMF_OUTBAND_MODE             2        /*带外*/
/* BEGIN: Modified by tKF19817, 2010/1/3   PN:AU8D03807 726协商*/
//#define CMDPARM_MEDIA_PLTYPE_G726_32               103      /*G726-32 本端为103*/
#define CMDPARM_MEDIA_PLTYPE_G726_32               104      
/* END:   Modified by tKF19817, 2010/1/3 */

typedef enum tagCMD_PARA_MEDIA_PROTOCAL_E
{
    CMDPARM_MEDIA_PROTOCOL_G711U        = 0,
    CMDPARM_MEDIA_PROTOCOL_G7231        = 4,
    CMDPARM_MEDIA_PROTOCOL_G723         = CMDPARM_MEDIA_PROTOCOL_G7231,
    CMDPARM_MEDIA_PROTOCOL_G711A        = 8,
    CMDPARM_MEDIA_PROTOCOL_G722,
    /* BEGIN: Modified by tKF19817, 2010/1/3   PN:AU8D03807 726协商*/
   // CMDPARM_MEDIA_PROTOCOL_G726_32      = 103,
    CMDPARM_MEDIA_PROTOCOL_G726_16      = 102,
    CMDPARM_MEDIA_PROTOCOL_G726_24      = 103,
    CMDPARM_MEDIA_PROTOCOL_G726_32      = 104,
  /*  CMDPARM_MEDIA_PROTOCOL_G726         = 2,*/ /* 默认的G726就是32位 */
    CMDPARM_MEDIA_PROTOCOL_G726_40      = 105,
    /* END:   Modified by tKF19817, 2010/1/3 */    
    CMDPARM_MEDIA_PROTOCOL_G728         = 15,
    CMDPARM_MEDIA_PROTOCOL_G729         = 18,
    CMDPARM_MEDIA_PROTOCOL_G729A        = CMDPARM_MEDIA_PROTOCOL_G729,
    CMDPARM_MEDIA_PROTOCOL_G723LOW      = 20,       /*非标准定义*/
    CMDPARM_MEDIA_PROTOCOL_R2198        = 96,
    CMDPARM_MEDIA_PROTOCOL_R2833        = 97,
    CMDPARM_MEDIA_PROTOCOL_T38          = 98,       /*传真协议，CSMV/6使用98*/
    CMDPARM_MEDIA_PROTOCOL_V150,
    CMDPARM_MEDIA_PROTOCOL_T38_ADD_2    = 256,      /*T.38+2定义*/
    CMDPARM_MEDIA_PROTOCOL_BUTT
}CMD_PARA_MEDIA_PROTOCAL_E;

 /* 打开通道的流模式 */
typedef enum tagCHANNEL_STREAM_MODE_E
{
    MODE_INACTIVE   = 0,        /* InActive */
    MODE_SENDONLY   = 1,        /* SendOnly */
    MODE_RECVONLY   = 2,        /* ReceiveOnly */
    MODE_SENDRECV   = 3,        /* SendReceive */
    MODE_LOOPBACK   = 4,        /* LoopBack */
    MODE_CONFRNCE,     
    MODE_CONTTEST,     
    MODE_NETWLOOP,     
    MODE_NETWTEST,     
    MODE_DATA,     
    MODE_BUTT
}CHANNEL_STREAM_MODE_E;

/*2833加密的最大密钥长度*/
#define MAX_2833_KEY_LEN 256

typedef struct tagRFC2833_CONFIG_PARA_S
{
    VOS_UINT32 ulPtFor2833;                         /*2833的PT值，带冗余为96，不带冗余为97*/
    VOS_UINT32 ulReSendCount;                       /*重发次数*/
    VOS_UINT32 ulSupport2833;                       /*是否支持2833 ，1-支持，0-不支持*/
    VOS_UINT32 ulSupportEny;                        /*是否支持加密，1-支持，0-不支持*/
    VOS_UINT32 ul2833KeyLen;
    VOS_UCHAR uc2833Key[MAX_2833_KEY_LEN + 1];      /*2833密钥,不支持可以任意填写*/
}RFC2833_CONFIG_PARA_S;

#define RFC2833_PARA_REDUN                96        /*冗余*/
#define RFC2833_PARA_UNREDUN            97          /*非冗余*/

typedef struct tagRFC2198_CONFIG_PARA_S
{
    VOS_UINT32 ulSupport2198;                       /*是否支持2198*/
    VOS_UINT32 ulRFC2198RedCount;                   /*冗余个数*/
    VOS_UINT32 ulBlockPtFor2198;                    /*主包pt类型，带冗余为96，不带冗余为97*/
    VOS_UINT32 ulPayloadPtFor2198;                  /*冗余包pt类型，带冗余为96，不带冗余为97*/
}RFC2198_CONFIG_PARA_S;

#define RFC2198_PARA_UNSUPPORT          0           /*不支持*/
#define RFC2198_PARA_SUPPORT              1         /*支持*/

#define RFC2198_PARA_REDUN                   96     /*冗余*/
#define RFC2198_PARA_UNREDUN              97        /*非冗余*/

typedef struct tagRTP_CONFIG_PARA_S
{
    VOS_UINT32  ulRtpRemoteIP;      /*远端IP*/
    VOS_UINT32  ulRtpRemotePort;    /*远端端口*/
    VOS_UINT32  ulRtpLocalPort;     /*本端端口*/
    VOS_UINT32  ulJitterBuffer;     /* JitterBUffer，单位MS */
}RTP_CONFIG_PARA_S;

typedef struct tagRTCP_CONFIG_PARA_S
{
    VOS_UINT32 ulRemoteRtcpPort;
    VOS_UINT32 ulLocalRtcpPort;
}RTCP_CONFIG_PARA_S;

typedef struct tagMODEM_CONFIG_PARA_S
{
    VOS_UINT32 ulModemTransferMode;   /*MODEM的传输模式,暂时不使用*/
}MODEM_CONFIG_PARA_S;

typedef struct tagFAX_CONFIG_PARA_S
{
    /*训练模式、网关最高速率、ECM支持*/
    VOS_UINT32 ulMaxRate;               /* 网关最高速度, voipFaxRate_EN */
    VOS_UINT32 ulTrainMode;             /* 训练方式, 0-端到端, 1-本地 */
    VOS_UINT32 ulECM;                   /* ECM方式，0-不支持, 1-支持 */  
}FAX_CONFIG_PARA_S;

typedef enum tagIPP_FAX_MAX_RATE_E
{
     FAX_RATE_NULL  = 0,
     NO_LIMIT	    = 1,         	/* 没有速率限制(缺省) */
     FAX_RATE_2400	= 2,	        /* 2,400bps */
     FAX_RATE_4800	= 3,        	/* 4,800bps */
     FAX_RATE_7200	= 4,	        /* 7,200bps */
     FAX_RATE_9600	= 5,	        /* 9,600bps */
     FAX_RATE_12000	= 6,	        /* 12,000bps */
     FAX_RATE_14400	= 7	            /* 14,400bps */
}IPP_FAX_MAX_RATE_E;

#define FAX_PARA_PPP_TRAIN       0          /*PPP*/
#define FAX_PARA_LOCAL_TRAIN     1          /*LOCAL*/

#define FAX_PARA_ECM_UNSUPPORT     0        /*不支持*/
#define FAX_PARA_ECM_SUPPORT          1     /*支持*/

typedef struct tagCHANNEL_PARA_BIT_CHANGE_FLAG_S
{
    /*DSP参数变更标志*/
    VOS_UINT16  bSilence:1;                 /*0、静音变更标志*/
    VOS_UINT16  bEcho:1;                    /*1、回声变更标志*/
    VOS_UINT16  bDtmfTransferMode:1;        /*2、DTMF传输模式变更标志*/
    VOS_UINT16  bRemoteCode:1;              /*3、远端编解码方式变更标志*/
    VOS_UINT16  bRemotePtType:1;            /*4、远端ploadType改变标志*/
    VOS_UINT16  bRemotePeriod:1;            /*5、远端打包时间长度变更标志*/
    VOS_UINT16  bLocalCode:1;               /*6、本端编解码方式变更标志*/
    VOS_UINT16  bLocalPtType:1;             /*7、本端ploadType改变标志*/
    VOS_UINT16  bLocalPeriod:1;             /*8、本端打包时间长度变更标志*/
    VOS_UINT16  bMode:1;                    /*9、连接模式变更标志*/
    VOS_UINT16  bWorkMode:1;                /*10、工作模式变更标识*/
    VOS_UINT16  bOutputGain:1;              /*11、DSP通道发送增益变更标志*/
    VOS_UINT16  bInputGain:1;               /*12、DSP通道接收增益变更标志*/
    /* BEGIN: Added by chenzhigang, 2008/1/25   问题单号:VDF SDP部分详细设计*/
    VOS_UINT16  bComfortNoise:1;            /*13、舒适噪音配置*/
    VOS_UINT16  bDspFill:3;                 /*填充*/
    /* END:   Added by chenzhigang, 2008/1/25 */

    /*2833参数变更标志, 2833和2198合并，都不常用*/
    VOS_UINT16  bPtFor2833:1;               /*0、PT值变更标志*/
    VOS_UINT16  bReSendCount:1;             /*1、2833重传次数变更标志*/
    /*2198参数变更标志*/
    VOS_UINT16  bSupport2198:1;             /*2、支持变更标志*/
    VOS_UINT16  bRFC2198RedCount:1;         /*3、冗余个数*/
    VOS_UINT16  bBlockPtFor2198:1;          /*4、主包pt类型，带冗余为96，不带冗余为97*/
    VOS_UINT16  bPayloadPtFor2198:1;        /*5、冗余包pt类型，带冗余为96，不带冗余为97*/
    VOS_UINT16  bRFCFill:10;                /*填充*/

    /*RTP参数变更标志, RTP和RTCP合并，不会有单独的RTCP设置*/
    VOS_UINT8   bRtpRemoteIP:1;             /*0、远端IP变更标志*/
    VOS_UINT8   bRtpRemotePort:1;           /*1、远端端口变更标志*/
    VOS_UINT8   bRtpLocalPort:1;            /*2、本端端口变更标志*/
    VOS_UINT8   bJitterBuffer:1;            /*3、JitterBUffer，变更标志*/
    VOS_UINT8   bRemoteRtcpPort:1;          /*4、远端端口变更标志*/
    VOS_UINT8   bLocalRtcpPort:1;           /*5、本地端口变更标志*/
    VOS_UINT8   bRtpFill:2;                 /*填充*/


    /*Modem参数变更标志*/
    VOS_UINT8  bModemTransferMode:1;        /*0、传输模式变更标志*/
    VOS_UINT8  bModemFill:7;                /*填充*/

    /*FAX参数变更标志*/
    VOS_UINT8  bMaxRate:1;                  /*0、网关最高速度, voipFaxRate_EN */
    VOS_UINT8  bTrainMode:1;                /*1、训练方式, 0-端到端, 1-本地 */
    VOS_UINT8  bECM:1;                      /*2、ECM方式，0-不支持, 1-支持 */
    VOS_UINT8  bFaxFill:5;                  /*填充*/
                  
    VOS_UINT8 ucFill;                       /*保证结构4字节对齐，增加标志时需要同步修改*/
}CHANNEL_PARA_BIT_CHANGE_FLAG_S;

#define   PARA_CHANGE_YES      1            /*发生变更*/
#define   PARA_CHANGE_NO       0            /*未变更*/

typedef struct tagCHANNEL_PARA_BLOCK_CHANGE_FLAG_S
{
    VOS_UINT16  uwDspParaChange;            /*DSP参数变更标志，不为0，则标志发生变更*/
    VOS_UINT16  uwRFCParaChange;            /*2833参数变更标志 2198参数变更标志，不为0，则标志发生变更*/
    VOS_UINT8    ucRtpParaChange;           /*RTP参数变更标志，不为0，则标志发生变更*/
    VOS_UINT8    ucModemParaChange;         /*Modem参数变更标志，不为0，则标志发生变更*/
    VOS_UINT8    ucFaxParaChange;           /*FAX参数变更标志，不为0，则标志发生变更*/
    VOS_UINT8    ucFill;                    /*保证结构4字节对齐，增加标志时需要同步修改*/
}CHANNEL_PARA_BLOCK_CHANGE_FLAG_S;

typedef struct tagCHANNEL_OPEN_PARA_S
{ 
    union
    {
        CHANNEL_PARA_BIT_CHANGE_FLAG_S       stParaBitChangeFlag;
        CHANNEL_PARA_BLOCK_CHANGE_FLAG_S  stParaBlockChangeFlag;
    }stParaChangeFlag;
    DSP_CONFIG_PARA_S                stDspCommonPara;
    RFC2833_CONFIG_PARA_S         stRFC2833Para;
    RFC2198_CONFIG_PARA_S         stRFC2198Para;
    RTP_CONFIG_PARA_S                stRtpPara; 
    RTCP_CONFIG_PARA_S              stRtcpPara;
    MODEM_CONFIG_PARA_S           stModemPara;
    FAX_CONFIG_PARA_S                stFaxPara;
}CHANNEL_OPEN_PARA_S;
/******************************************************************************************/


/******************************************************************************************
TAPI_GetRtpStatistic相关定义
******************************************************************************************/
typedef struct tagRTP_STATISTIC_DATA_S
{
    VOS_UINT32      ulSndPktNum;                /*发送的RTP包总数*/
    VOS_UINT32      ulSndOctetNum;              /*发送的RTP字节总数*/
    VOS_UINT32      ulRecvPktNum;               /*收到的RTP包总数*/
    VOS_UINT32      ulRecvOctetNum;             /*收到的RTP字节总数*/
    VOS_UINT32      ulCumLost;                  /*丢失的包总数，是本端应该与实际接收的包的个数的偏差*/
    VOS_UINT32      ulJitter;                   /*包间隔平均偏差， 单位MS*/
    VOS_UINT32      ulLoop;                     /*在两个报告间隔期间的环路时延,单位MS*/
    VOS_UINT32      ulFractionLost;             /*在两个报告间隔期间的丢包比率*/
    VOS_UINT32      ulDuration;                 /*持续时间nt/duration*/
}RTP_STATISTIC_DATA_S;
/******************************************************************************************/

/******************************************************************************************
TAPI_SendDtmf相关定义
******************************************************************************************/
typedef enum tagDTMF_TONE_VALUE_E
{
    DTMF_CODE_0,                /*表示0*/
    DTMF_CODE_1,                /*表示1*/
    DTMF_CODE_2,                /*表示2*/
    DTMF_CODE_3,                /*表示3*/
    DTMF_CODE_4,                /*表示4*/
    DTMF_CODE_5,                /*表示5*/
    DTMF_CODE_6,                /*表示6*/
    DTMF_CODE_7,                /*表示7*/
    DTMF_CODE_8,                /*表示8*/
    DTMF_CODE_9,                /*表示9*/
    DTMF_CODE_A,                /*表示**/
    DTMF_CODE_B,                /*表示#*/
    DTMF_CODE_C,                /*表示A*/
    DTMF_CODE_D,                /*表示B*/
    DTMF_CODE_E,                /*表示C*/
    DTMF_CODE_F,                /*表示D*/
    DTMF_CODE_BUTT

}DTMF_TONE_VALUE_E;
/******************************************************************************************/



/******************************************************************************************
TAPI_NetOperate相关定义
******************************************************************************************/
typedef enum tagNET_OPER_TYPE_E
{
    FORWARD_CONNECT,            /* 前向连网   */
    BACKWARD_CONNECT,           /* 后向连网   */
    DUAL_CONNECT,               /* 双向连网   */
    DISCONNECT,                 /* 拆除网连接 */
    NET_OPERATION_BUTT
}NET_OPER_TYPE_E;

typedef enum tagNET_CHANNEL_TYPE_E
{
    CHAN_TYPE_DSP,
    CHAN_TYPE_RTP,
    CHAN_TYPE_HANDLE,           /*含义是"HW/TS"*/
    CHAN_TYPE_BUTT
}NET_CHANNEL_TYPE_E;
/******************************************************************************************/


/******************************************************************************************
TAPI_UserPortEventReport相关定义
******************************************************************************************/
typedef enum tagUSER_PORT_EVENT_REPORT_E
{
    USER_CALL_OFFHOOK,                  /* 0 用户事件:主叫摘机*/
    USER_CALLED_OFFHOOK,                /* 1 用户事件:被叫摘机        */
    USER_HOOKFLASH,                     /* 2 用户事件:拍叉*/
    USER_ONHOOK,                        /* 3 用户事件:挂机*/
    USER_DISCONNECT,                    /* 4 用户事件: 拆除连接请求*/  
    
    /* Start of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
    USER_TRANSFER,                      /* 5用户事件:转移*/
    USER_CONFERENCE,                    /* 6 用户事件:会议*/
    USER_CANCEL,                        /* 7用户事件:拒绝*/
    /* End of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */

    USER_START_SERVICE,                 /* 8用户事件: 启动业务*/
    USER_END_SERVICE,                   /* 9用户事件: 终止业务*/
    USER_PORT_FAULT,                    /* 10用户事件:端口故障*/
    USER_PORT_NORMAL,                   /* 11用户事件:端口恢复正常*/

    USER_PULSE_CODE,                    /* 12脉冲号码上报*/

    /* Start of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
    USER_CODE_KEY,                      /* 13按键号码,一个或多个,用字符串'\0'结束; 当使用此字段时pReport定义见CODE_KEY_S*/    
    /* End of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */

    USER_FSK_READY,                     /* 14FSK启动 */
    USER_CDI_SEIZURE_ACK,               /* 15摘机响应  CDI */ 
    USER_CDI_CALLED_OFFHOOK,            /* 16被叫摘机  CDI */
    USER_CDI_CALLED_ONHOOK,             /* 17被叫挂机  CDI */ 
    USER_CDI_SEND_NUM_OK,               /* 18送号OK  CDI */
    USER_CDI_BLOCK,                     /* 阻塞  CDI */
    USER_CDI_UNBLOCK,                   /* 解阻塞  CDI */

    /* BEGIN: Added by chenzhigang, 2008/5/16   问题单号:忙音*/
    USER_SERVICEMANTONE_FINISH,
    /* END:   Added by chenzhigang, 2008/5/16 */
    USER_BUTT
} USER_PORT_EVENT_REPORT_E;


/* Start of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
/*当上报事件类型为USER_CODE_KEY时,pReport定义*/
typedef struct tag_CODE_KEY_S 
{
    VOS_UINT8 aucNumber[LENGTH_OF_CALLING_NUMBER];
}CODE_KEY_S;
/* End of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */

/******************************************************************************************/

/******************************************************************************************
TAPI_DspEventReport相关定义
******************************************************************************************/
typedef enum tagCHANNEL_EVENT_REPORT_E
{
    CHANNEL_FAULT,                      /*故障*/
    CHANNEL_FAX_START,                  /*传真开始*/
    CHANNEL_FAX_END,                    /*传真结束*/
    CHANNEL_MODEM_START,                /*MODEM开始*/
    CHANNEL_DETECT_DTMF,                /*DTMF检测*/
    CHANNEL_DETECT_FSK,                 /*FSK检测*/
    CHANNEL_DETECT_BUSY_TONE,           /*忙音检测*/
    CHANNEL_RTCP_ALARM,                 /*RTCP告警*/
    CHANNEL_OPEN_FAILED                 /*打开通道失败*/
}CHANNEL_EVENT_REPORT_E;
/******************************************************************************************/

/******************************************************************************************
TAPI_DevEventReport相关定义
******************************************************************************************/
typedef enum tagDEV_TYPE_DEFINE_E
{
    DEV_USER_BOARD,
    DEV_DSP_CHIP,
    DEV_LSW_CHIP,           /*AG需求*/
    DEV_FPGA,               /*AG需求*/
    DEV_FE,                 /*AG需求*/
    DEV_BUTT
}DEV_TYPE_DEFINE_E;

typedef enum tagDEV_EVENT_TYPE_E
{
    FAULT,
    NORMAL,
}DEV_EVENT_TYPE_E;

typedef struct tagBOARD_DATA_S
{
    VOS_UINT32 ulPortType;     /*暂不使用*/
    VOS_UINT32 ulStartUserPort;
    VOS_UINT32 ulEndUserPort;
} BOARD_DATA_S;
 	
typedef struct tagDSP_INDEX_S
{
    VOS_UINT32 ulStartDspIndex;
    VOS_UINT32 ulEndDspIndex;
}DSP_INDEX_S;
 	
typedef struct tagDEV_INFO_S
{
    VOS_UINT32 ulBoardIndex;
} DEV_INFO_S;
/******************************************************************************************/




/******************************************************************************************
TAPI_GetDecodeCapability相关定义
ulCodecCap定义见CMD_PARA_MEDIA_PROTOCAL_E定义
ulCapPara定义如下：
 	**对应于语音编解码能力
           打包时长，单位ms
 	**对应于T38、RFC2198、RFC2833  
           冗余包个数
 	**对应于V.150
           暂不定义
******************************************************************************************/
#define MAX_DEV_CAPABILITY_NUM     32
/* BEGIN: Added by chenzhigang, 2008/1/7   问题单号:VDF SDP部分详细设计*/
//G.729的最大数量
#define MAX_DEV_CAPABILITY_G729_NUM      2
/* END:   Added by chenzhigang, 2008/1/7 */

typedef struct tagCAPABILITY_ITEM_S
{
    VOS_UINT32  ulCodecCap;
    VOS_UINT32  ulCapPara;
}CAPABILITY_ITEM_S;

typedef struct tagDEV_CAPABILITY_S
{
    CAPABILITY_ITEM_S  stDevCap[MAX_DEV_CAPABILITY_NUM];
}DEV_CAPABILITY_S;

typedef enum tagGET_CAP_SELECT_CODE_E
{
    GET_DEV_CAP_ALL     = 0,        /*获取所有能力，不分优先级，包含所有支持的打包时长和编解码的组合。在同一种编解码，不同打包时长情况下，驱动必须将这些能力连续放置在数组中，并按照打包时长排序（升序）*/
    GET_DEV_CAP_CALL    = 1,        /*必须返回所有支持的编解码能力，每种能力只提供一种打包时长，并按照系统提供的编解码优先级排序（0号能力最优先）*/
    GET_DEV_CAP_FAX     = 2,        /*返回传真能力，按照优先级排序。分三种情况描述，G.711（结合打包时长，只返回一个记录）、 T.38（冗余包个数最大值）、 T.38+2（冗余包个数最大值）*/
    GET_DEV_CAP_2198    = 3,
    GET_DEV_CAP_2833    = 4,
    GET_DEV_CAP_BUFF
}GET_CAP_SELECT_CODE_E;




/******************************************************************************************
TAPI_GetPortStatus相关定义
******************************************************************************************/
typedef enum tagPORT_STATUS_E
{
    PORT_STATUS_IDLE,
    PORT_STATUS_BUSY,
    PORT_STATUS_FAULT,
    PORT_STATUS_LOCKED,
    PORT_STATUS_BLOCK_OAM,
    PORT_STATUS_BLOCK_PEER,
    PORT_STATUS_BLOCK_FAULT,
    PORT_STATUS_TESTING,
    PORT_STATUS_LOOP,                     /*环回,由于和通道使用状态处于并列的地位，终端不使用这个定义。AG有可能使用**/
    PORT_STATUS_NULL,                     /*未安装*/
    PORT_STATUS_BUFF
}PORT_STATUS_E;



/* Start of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */
/******************************************************************************************
TAPI_CIPReceiveCallstatus相关定义
******************************************************************************************/
typedef enum tagCIP_CALL_STATUS_E
{
    CIP_CALLSTATUS_IDLE = 0 ,
    /* Start of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
    CIP_CALLER_DAILING_OVER,        /* 作为主叫，拨号音结束*/
    /* End of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
    CIP_CALLER_START,               /* 作为主叫，呼叫开始,拨号成功开始呼叫*/
    CIP_CALLED_START,               /* 作为被叫，呼叫开始,开始振铃*/
    CIP_CALLER_END,                 /* 作为主叫，呼叫结束，挂机*/
    CIP_CALLED_END,                 /* 作为被叫，呼叫结束，接通前对方挂机*/    
    CIP_CALL_PEER_BUSY,             /* 作为主叫，呼叫对方忙*/ 
    CIP_CALL_PEER_NO_RES,           /* 作为主叫，呼叫无应答, 对方久叫不应*/ 
    CIP_TALK_START,                 /* 通话开始*/ 
    CIP_TALK_END,                   /* 通话结束,两方只要有一方挂机就通话结束，三方通话至少要两方结束*/
    CIP_BUTT,
}CIP_CALL_STATUS_E;
/* End of ouyangyueling+29638 2005-01-17 IAD2000V300R001B01D010 FNRD03870 */




/* Start of ouyangyueling+29638 2005-01-24 IAD2000V300R001B01D020 FNRD03949 */
/******************************************************************************************
TAPI_FXOStateChange/TAPI_FXOStateGet相关定义
******************************************************************************************/
typedef enum tag_FXO_STATE_E
{
    FXO_IDLE,
    FXO_BUSY,
    FXO_ALERT,
    FXO_NULL
}FXO_STATE_E;




/******************************************************************************************
TAPI_FXOSwitch相关定义
******************************************************************************************/
typedef enum tagSWITCH_DIRECT_E
{
    POTS_TO_FXO,   /* switch  POTS port to   FXO port */
    POTS_TO_FXS    /* switch  POTS port to   FXS port */
}SWITCH_DIRECT_E;
/* End of ouyangyueling+29638 2005-01-24 IAD2000V300R001B01D020 FNRD03949 */


/* Start of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */
/******************************************************************************************
TAPI_SetTalkForPhone相关定义
******************************************************************************************/
typedef enum tagTEL_DIRE_E
{
    TEL_HANDSET_ON,                     /*设置通道在听筒*/
    TEL_HANDFREE_ON,                    /*设置通道在喇叭*/
    TEL_DEFAULT,                        /*缺省设置*/
    TEL_BUTT,                           
}TEL_DIRECT_E;


/******************************************************************************************
TAPI_GetHardwareType相关定义
******************************************************************************************/
typedef enum tagHARDWARE_TYPE_E
{
    HARDWARE_TYPE_IAD101H,                    
    HARDWARE_TYPE_IAD101S,                     
    HARDWARE_TYPE_IAD102H,                    
    HARDWARE_TYPE_IAD104H,    
    HARDWARE_TYPE_IAD102S,    
    HARDWARE_TYPE_BUFF
}HARDWARE_TYPE_E;


/******************************************************************************************
TAPI_RegLedDisplay相关定义
******************************************************************************************/
typedef enum tagLED_STATUS_E
{
    LED_TURNOFF,                        /*灯灭 */
    LED_LIGHT,                          /*摘机指示灯，常亮*/
    LED_FALSH,                          /*留言或未接来电指示用,闪烁*/
    LED_BUTT                            /*无效参数*/
}LED_STATUS_E;

/* End of ouyangyueling+29638 2005-03-15 TS0001 V300R001B01D030 FNRD04333 */

/* Start of lidongyun+39160 2005-06-21 TS0001 TS0401V300R001B01D060 A41D00612*/
typedef struct tagRING_CHANNEL_PARA_S
{
    VOS_UINT8  ucRingParaType;  //(L/r0---L/r7) ,不用时填成0xFF
    VOS_UINT8  ucDspIndex;
    VOS_UINT8  ucBuffer[2];     //不用时添成0xFFFFFF
} RING_CHANNEL_PARA_S;

typedef enum tagRING_PARA_E
{
    RING_R0 = 0,     // 对于振铃参数(L/r0)
    RING_R1,         // 对于振铃参数(L/r1)
    RING_R2,         // 对于振铃参数(L/r2)
    RING_R3,         // 对于振铃参数(L/r3)
    RING_R4,         // 对于振铃参数(L/r4)
    RING_R5,         // 对于振铃参数(L/r5)
    RING_R6,         // 对于振铃参数(L/r6)
    RING_R7,         // 对于振铃参数(L/r7)
    RING_RG,
    RING_BUTT        // 对于振铃参数 
}RING_PARA_E;
/*End of lidongyun+39160 2005-06-21 TS0001 TS0401V300R001B01D060 A41D00612*/

/* Start of zhuweiping 30370 2005-07-28 TS0401 TS0401V300R001B01D090 A41D00849*/
/*此结构设计为MGCP MIB的逻辑端口表使用.如果当前有呼叫，则是使用当前呼叫的参数;反之，则取命令行配置的缺省值*/
/*注1:当呼叫已经建立或建立过程中，此时修改命令行配置，此种情况，仍以当前呼叫的为准.*/
/*注2:对于传真和MODEM业务，也以当前呼叫的为准，不依赖于命令行配置*/
#define CHANNEL_PARA_COMFORT_NOISE_DISABLE   0  /*表示不支持舒适噪声，目前不能设置为不支持，暂时不用,为后续保留该接口*/
#define CHANNEL_PARA_COMFORT_NOISE_ENABLE    1  /*表示支持舒适噪声*/

typedef struct tag_TAPI_MIB_DSP_CHANNEL_PARA_S
{
    VOS_UINT32  ulEcho;          /*缺省ON.采用宏定义CHANNEL_PARA_EC_DISABLE表示OFF;CHANNEL_PARA_EC_ENABLE表示ON,这两个宏都定义在TAPI_DEF.H中，可以直接使用*/
    VOS_UINT32  ulSilence;       /*缺省OFF.采用宏定义CHANNEL_PARA_SILENCE_DISABLE表示OFF;CHANNEL_PARA_SILENCE_ENABLE表示ON,这两个宏都定义在TAPI_DEF.H中，可以直接使用*/
    VOS_UINT32  ulComfortNoise;  /*缺省ON.采用宏定义CHANNEL_PARA_COMFORT_NOISE_DISABLE表示OFF;CHANNEL_PARA_COMFORT_NOISE_ENABLE表示ON,这两个宏都定义在TAPI_DEF.H中，可以直接使用*/
}TAPI_MIB_DSP_CHANNEL_PARA_S;
/* End of zhuweiping 30370 2005-07-28 TS0401 TS0401V300R001B01D090 A41D00849*/

typedef enum tagTAPICallStateE
{
    CS_CALL_IDLE = 0,           /*空闲*/
    CS_CALLER_DIAL_TONE,        /*播放主叫拨号音*/
    CS_CALLER_START,            /*主叫收齐号码，开始发起呼叫*/
    CS_CALLER_RINGBACK,         /*主叫呼叫成功，开始播放回铃音*/
    CS_CALLER_CONXT,            /*被叫摘机，呼叫成功建立*/
    CS_CALLER_NOREPLY,          /*久叫不应*/
    CS_CALLER_BUSY,             /*被叫忙*/
    CS_CALL_INI_ONHOOK,         /*本端挂机，拆除呼叫成功*/
    CS_CALL_PAS_ONHOOK,         /*对端挂机，拆除呼叫成功*/
    CS_CALLEE_ALERTING,         /*开始放振铃，此时主叫号码已经下发过了*/
    CS_CALLEE_CONXT,            /*被叫摘机上报，对端接受呼叫，呼叫开始*/
    CS_CALL_HOLD_START,         /*呼叫保持成功，开始放二次拨号音*/
    CS_CALL_HOLD_END,           /*呼叫保持结束，停止放二次拨号音*/
    CS_CALL_WAITING_START,      /*有新呼叫进入，播放呼叫等待音，呼叫等待开始*/
    CS_CALL_TWOCR_SWITCH,       /*新业务中两个用户切换*/
    CS_CALL_RELEASE_ONECR,      /*新业务中结束一个呼叫，保留另外一个呼叫*/
    CS_CALL_CONF_START,         /*三方通话开始*/
    CS_CALL_CONF_END,           /*三方通话结束*/
    CS_CALL_END,                /*呼叫结束*/
}TAPICallStateE;
typedef enum channel_ACTION_E
{
    TAIP_CHANNEL_INIT = 0,
    TAIP_CHANNEL_MDFY ,
    TAPI_CHANNEL_CLOSE
}CHANNEL_ACTION_E;

#endif  /*TAPI_DEF_H*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
