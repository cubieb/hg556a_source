/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   :ttyUSB.h
  版 本 号   : 初稿
  作    者   : 曹祥
  生成日期   : 2007年12月29日
  最近修改   :
  功能描述   : cmdTest.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2007年12月29日
    作    者   : 曹祥
    修改内容   : 创建文件

******************************************************************************/


#ifndef __TTYUSB_H__
#define __TTYUSB_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include <linux/types.h>
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
 /* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 
   for 只有识别到2或3或4口才拨号,Usb-Serial Port Num增加一个口变成4个口, 增加这个口为了QXDM透传抓log信息.
	   原来port的作用还是不变:0用来数据,1用来语音,2用来AT指令,3用来QXDM. 
	   E180从11.104.18.00.00版本改成是4口,E169从11.314.17.31.00版本改成4口.*/
#define HSPA_MAX_AVALABLE       4         //HSPA最多可用port num
#define HSPA_MIN_AVALABLE       2         //HSPA至少可用port num
#define HSPA_MIN_VOICE_AVALABLE 3         //HSPA支持语音时至少需要的port num
#define HSPA_MAX_LINE_LEN       64        //最大行长度
#define HSPA_PORT_NUM_STR       "Usb-Serial Port Num:%d"
/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 */

/*测试使能宏*/
//#define CX_DEBUG_VERSION

/*表示不同的模块*/
/*
#define AT_MODULE 0
#define VOICE_MODULE 1
#define MODEM_MODULE 2
#define TR069_MODULE 3
#define HTTP_MODULE 4
#define LED_MODULE 5
#define CMD_MODULE 6
*/
#define AT_MODULE 1
#define VOICE_MODULE 2
#define MODEM_MODULE 3
#define TR069_MODULE 4
#define HTTP_MODULE 5
#define LED_MODULE 6
#define CMD_MODULE 7
#define PINMNGR_MODULE 8

/*表示不同的模块对应的消息队列*/
#define MSG_AT_QUEUE  100
#define MSG_VOICE_QUEUE  101
#define MSG_MODEM_QUEUE  102
#define MSG_TR069_QUEUE  103
#define MSG_HTTP_QUEUE  104
#define MSG_LED_QUEUE  105
#define MSG_CMD_QUEUE  106
#define MSG_PINMNGR_QUEUE 107

/*发送往HSPA控制进程的消息结构体的缓冲区长度*/
#define	AT_SND_MSG_LENGTH 128

/*接收来自HSPA控制进程的消息结构体的缓冲区长度*/
#define	AT_RCV_MSG_LENGTH 252

/************************************************************/
#define	CMEE_SET	1
#define	CMEE_QUERY	2
#define	E_SET	3
#define	S3_SET	4
#define	S3_QUERY	5
#define	S4_SET	6
#define	S4_QUERY	7
#define	S5_SET	8
#define	S5_QUERY	9
#define	V_SET	10
#define	FACTORY_SET	11
#define	REPORT_PORT_SET	12
#define	REPORT_PORT_QUERY	13
#define	CFUN_SET	14
#define	CFUN_QUERY	15
#define	PRODUCT_NAME_QUERY	16
#define	FACTORY_INFO_QUERY	17
#define	FACTORY_INFO_QUERY_C	18
#define	SOFTWARE_VERSION_QUERY	19
#define	SOFTWARE_VERSION_QUERY_C	20
#define	HARDWARE_VERSION_QUERY	21
#define	HARDWARE_VERSION_QUERY_W	22
#define	EARPHONE_VOLUME_SET	23
#define	EARPHONE_VOLUME_QUERY	24
#define	EARPHONE_SILENCE_SET	25
#define	EARPHONE_SILENCE_QUERY	26
#define	IMSI_QUERY	27
#define	IMEI_QUERY	28
#define	SPN_QUERY	29
#define	SIM_USIM_MODE_QUERY	30
#define	RSSI_QUERY	31
#define	AUTO_REPORT_SET	32
#define	AUTO_REPORT_QUERY	33
#define	SYSTEM_INFO_QUERY	34
#define	NETWORK_REG	35
#define	NETWORK_REG_QUERY	36
#define	GPRS_NETWORK_REG	37
#define	GPRE_NETWORK_REG_QUERY	38
#define	SYSCFG_SET	39
#define	SYSCFG_QUERY	40
#define	SYSCONFIG_SET	41
#define	SYSCONFIG_QUERY	42
#define	FREQLOCK_SET	43
#define	FREQLOCK_QUERY	44
#define	PIN_PASSWORD_MODIFY	45
#define	PIN_MANAGE_PLUSS	46
#define	PIN_QUERY_PLUSS	47
#define	PIN_ENABLE	48
#define	PIN_MANAGE	49
#define	PIN_QUERY	50
#define	PDP_SET	51
#define	PDP_QUERY	52
#define	DSFLOW_CLEAR	53
#define	DSFLOW_QUERY	54
#define	DIAL	55
#define	DIAG_PORT_SET	56
#define	ACCEPT	57
#define	CHUP	58
#define	TELNUM_DISPLAY_SET	59
#define	TELNUM_DISPLAY_SET_QUERY	60
#define	CALL_STATUS_QUERY	61
#define	CALL_TIME_QUERY	62
#define	AUTO_SIM_STATUS_REPORT	63
#define	RING_REPORT	64
#define	CALL_NUM_REPORT	65
#define	CALL_END_REPORT	66
#define	AUTO_SERVICE_STATUS_REPORT	67
#define	AUTO_SYSTEM_MODE_STATUS_REPORT	68
#define 	PIN_PUK_TIMES_QUERY 69
#define 	PIN_ENABLE_QUERY	70
#define  	CONN_REPORT 71
#define 	OPERATION_QUERY 72
#define	CARDMODE_QUERY 73
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add begin:*/
#define	SN_QUERY	74
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add end.*/
#define DEX_SET     75
#define   CGACT_QUERY  76
/* HUAWEI HGW s48571 2008年8月16日 关闭语音需求 add begin:*/
#define VOICE_SET   77
/* HUAWEI HGW s48571 2008年8月16日 关闭语音需求 add end.*/

#define DISABLE_VOICE 0
#define ENABLE_VOICE 1

/* l65130 hspa配置信息文件 */
#define HSPA_SYSCFG_FILE   "/var/hspasys.cfg"
#define HSPA_SYS_CFG_FORMAT_W "ConType:%d\nbands:%s\nVoiceEnable:%d\nOperator:%s\n"
#define HSPA_SYS_CFG_FORMAT_R "ConType:%d\nbands:%s\nVoiceEnable:%d\nOperator:%31[^\n]\n"

/*<CR><LF>^CARDMODE:<sim_type><CR><LF><CR><LF>OK <CR><LF>*/
struct stMsg_Card_Mode
{
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
	/*<sim_type>：SIM卡的类型，取值如下：
	0:  Unknown mode
	1:  SIM卡
	2:  USIM卡*/
	unsigned char ucCardMode;
	unsigned char ucExtern1;
};


/*#define 	OPERATION_QUERY 72
AT+COPS?

<CR><LF>+COPS: <mode>[,<format>,<oper>[,<rat>]]
<CR><LF>OK<CR><LF>
有MS相关错误时：
<CR><LF>+CME ERROR: <err><CR><LF>
<mode>:   网络选择模式
0:     自动搜网，设置命令中，mode取值为0时，后面参数均不要带。
1:     手动搜网
2:     去注册网络
3:     仅为+COPS?读命令设置返回的格式<format>
4:     手动与自动的联合，如果手动搜网失败，则自动转入自动搜网模式
<format>:  运营商信息<oper>的格式   
0:	长字符串格式的运营商信息<oper>
1:	短字符串格式的运营商信息<oper>
2:	数字格式的运营商信息<oper>
<oper>: 运营商的信息。
<stat>:  网络的状态标识，取值如下：  
0:	未知的
1:	可用的
2:	当前注册
3:	禁止
<rat>：无线接入技术，取值如下：
 0:	GSM/GPRS 制式
 2:       WCDMA制式*/

/*                                                                  
+COPS: 0,2,"46000",0                                                            
                                                                                
OK*/

/*+COPS: (2,"CHINA  MOBILE","CMCC","46000",0),
(1,"","","46019",2 ),
(3,"CHN-CUGSM","CU-GSM","46001",0),

,(0,1,3,4),(0,1,2)  */
struct stMsg_Operation_Info_Query
{
	#define HSPA_OPERATION_INFO_LENGTH	23
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
	/*<mode>:   网络选择模式
	0:     自动搜网，设置命令中，mode取值为0时，后面参数均不要带。
	1:     手动搜网
	2:     去注册网络
	3:     仅为+COPS?读命令设置返回的格式<format>
	4:     手动与自动的联合，如果手动搜网失败，则自动转入自动搜网模式	*/
	unsigned char ucChooseMode;
	/*<format>:  运营商信息<oper>的格式   
	0:	长字符串格式的运营商信息<oper>
	1:	短字符串格式的运营商信息<oper>
	2:	数字格式的运营商信息<oper>	*/
	unsigned char ucOperationFormat;
	/*<oper>: 运营商的信息。最多不超过20字符*/
	char acOperationInfo[HSPA_OPERATION_INFO_LENGTH];
	/*	<rat>：无线接入技术，取值如下：
	 0:	GSM/GPRS 制式
 	2:       WCDMA制式*/
	unsigned char ucWifiTech;
};


/*通用字符数组长度*/
#define	COMMON_LONG_STRING_LENGTH	128
#define	COMMON_STRING_LENGTH	64
#define	COMMON_SHORT_STRING_LENGTH	32
#define	COMMON_VERY_SHORT_STRING_LENGTH	16
/*结构体中字符串长度*/
#define	HSPA_FACINFO_LENGTH	60	
#define	HSPA_SOFTWARE_LENGTH	60
#define	HSPA_HARDWARE_LENGTH	60
#define	HSPA_PRODUCT_NAME_LENGTH	60

#define	HSPA_IMSI_LENGTH	16
#define	HSPA_IMEI_LENGTH	16
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add begin:*/
#define	HSPA_SN_LENGTH 32
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add end.*/
#define	HSPA_BAND_LENGTH	12
#define	HSPA_PROTOCOL_KIND_LENGTH	8
#define	HSPA_APN_NAME_LENGTH	112
#define	HSPA_DOMAIN_ADDR_LENGTH	20
#define	HSPA_TELNUM_LENGTH	24

/*执行结果是正确的*/
#define	HSPA_OK	0
#define	HSPA_ERROR_NO_E172	1
#define	HSPA_ERROR_NO_SIM		2
#define	HSPA_BAD_COMMAND 	3
#define	HSPA_ERROR_PIN_NEED	4
#define	HSPA_ERROR_PUK_NEED	5
#define	HSPA_ERROR_OPERATION_REFUSED	6
#define	HSPA_ERROR_COMMAND_REFUSED	7
#define	HSPA_ERROR_INCORRECT_PASSWORD	8
#define	HSPA_ERROR_PIN_NEED	9
#define	HSPA_ERROR_PUK_NEED  10

#define HSPA_PIN_PUK_NEEDED 101


/*预留到8~31*/
#define	HSPA_UNKNOW	(-1)
#define	HSPA_ERROR	(-1)
#define	HSPA_UNKNOW_VALUE	250
#define	HSPA_OFFSET	1
#define	HSPA_EXIT	125
#define	HSPA_WRITE_ENABLE		1
#define	HSPA_WRITE_DISABLE	0
#define	HSPA_ERROR_UNKNOW	100

/*消息来源对应数组下标宏*/
#define	HSPA_FROM_POSITION 0
#define	HSPA_RESULT_POSITION 1

/*数据卡的有无宏*/
#define	HSPA_NO	 0
#define	HSPA_EXIST	1
#define	HSPA_UNKNOWN	2


/*SIM卡不在宏*/
#define  HSPA_NO_SIM	255

/*SIM卡无效宏*/
#define  HSPA_INVALID_SIM	 0
/*SIM卡有效宏*/
#define  HSPA_VALID_SIM	 1
/*需要PIN码校验宏*/

#define	HSPA_PIN_NEED	1
/*需要PUK码校验宏*/
#define	HSPA_PUK_NEED	2
/*不需要PIN/PUK码校验宏*/
#define	HSPA_PIN_PUK_OK	0

/*命令是否写入设备宏
其中前两个宏表示从消
息队列中收取了消息*/
#define 	HSPA_WRITE_OK	1
#define	HSPA_WRITE_NO	0
#define	HSPA_WRITE_UNKNOW (-1)


/*
<curr_ds_time>：
00000000－FFFFFFFF，8位16进制数，表示当前这次DS的连接时间，单位为秒。
<ltx_rate>：
		00000000－FFFFFFFF，8位16进制数，表示当前的发送速率，单位为字节每秒。
<rx_rate>：
		00000000－FFFFFFFF，8位16进制数，表示当前的接收速率，单位为字节每秒。
<curr_tx_flow>：
0000000000000000－FFFFFFFFFFFFFFFF ，16位16进制数，表示当前这次DS的发送流量，单位为字节。
<curr_rx_flow>：
0000000000000000－FFFFFFFFFFFFFFFF ，16位16进制数，表示当前这次DS的接收流量，单位为字节。
<qos_tx_rate>：
		00000000－FFFFFFFF，8位16进制数，与网络侧协商后确定的PDP连接发送速率，单位为字节每秒。
<qos_rx_rate>：
		00000000－FFFFFFFF，8位16进制数，与网络侧协商后确定的PDP连接接收速率，单位为字节每秒。

*/
struct stDataFlow
{
	unsigned long int ulCurrDsTime;
	unsigned long int ulTxRate;
	unsigned long int ulRxRate;
	unsigned long int ulQosTxRate;
	unsigned long int ulQosRxRate;
	/*char acCurrDsTime[COMMON_SHORT_STRING_LENGTH];
	char acTxRate[COMMON_SHORT_STRING_LENGTH];
	char acRxRate[COMMON_SHORT_STRING_LENGTH];
	char acCurrTxFlow[COMMON_SHORT_STRING_LENGTH];
	char acCurrRxFlow[COMMON_SHORT_STRING_LENGTH];
	char acQosTxRate[COMMON_SHORT_STRING_LENGTH];
	char acQosRxRate[COMMON_SHORT_STRING_LENGTH];*/
};
#define HSPA_SHARE_FILE_PATH "/var/HspaStatus"
struct stHspaInfo
{
	/*HSPA控制进程pid*/
	int lPid;
	/*USB-SERIAL设备接口数目
	-1：状态未知
	0：无设备
	3：正常	*/
	short int sDeviceCount;
	/*	-1：状态未知
	0：设置为LPM（低功耗）模式（MS之前的设置必须为非offline模式）
	1：设置为online模式（默认值）（MS之前的设置必须为非offline模式）
	4：设置为offline模式（MS之前的设置必须为非FTM模式）
	5：设置为FTM模式（MS之前的设置必须为online模式）
	6：重启MS（MS之前的设置必须为offline模式）	*/
	short int sDeviceStatus;
	/*SIM卡状态，取值如下：
	-1：状态未知
	0： USIM卡状态无效
	1:  USIM卡状态有效  
	2： USIM在CS下无效（EW）
	3： USIM在PS下无效(EW)
	4： USIM在PS+CS下均无效(EW)
	240：ROMSIM版本
	255： USIM卡不存在 	
	250:表示未获取到数值，无意义*/
	short int sSimExist;
	/*	表示SIM卡状态
	-1：状态未知；
	0：	   SIM卡未被SIMLOCK功能锁定
	1：	   SIM卡被SIMLOCK功能锁定*/
	short int sSimStatus;
	/*系统模式，取值如下：
	-1：状态未知；
	0：无服务
	1：AMPS模式（暂不使用）
	2：CDMA模式（暂不使用）
	3：GSM/GPRS 模式
	4：HDR模式
	5：WCDMA模式
	6：GPS模式*/
	short int sSysMode;
	/*系统模式的子模式，取值如下：
	-1：状态未知；
	0：无服务
	1：GSM模式
	2：GPRS模式
	3：EDGE模式
	4：WCDMA模式
	5：HSDPA模式
	6：HSUPA模式
	7：HSDPA&HSUPA模式*/
	short int sSubSysMode;
	/*	-1：状态未知；
	0:  无服务
	1:  有限制服务
	2:  服务有效	
	3:  有限制的区域服务
	4:  省电和深睡状态	*/
	short int sSrvStatus;
	/*-1：状态未知；
	0:不需要任何验证
	1:PIN验证
	2:PUK验证
	*/
	short int sPinPukNeed;
	char acIMEI[HSPA_IMEI_LENGTH]; //存储在卡中的IMSI值，为一个0～9的十进制数构成的字符串
	char acPinCode[HSPA_IMEI_LENGTH];
	char acOldIMEI[HSPA_IMEI_LENGTH];
	struct stDataFlow stDataFlowInfo;

 };
//#define	FACTORY_INFO_QUERY 17
//#define	FACTORY_INFO_QUERY_C 18
/*厂商信息查询 
命令格式 AT+GMI 
响应格式：<CR><LF>+GMI: <manufacturer><CR><LF><CR><LF>OK<CR><LF> 有MS相关错误时：<CR><LF>ERROR<CR><LF>*/
/*厂商信息查询 
命令格式：AT+CGMI 
响应格式：CR><LF>+GMI: <manufacturer><CR><LF><CR><LF>OK<CR><LF> 
有MS相关错误时：<CR><LF>ERROR<CR><LF>*/
struct stMsg_Factory_Info_Query
{ 
      unsigned char ucCmdFrom;      
      unsigned char ucResult;
      unsigned char ucExtern1;
      unsigned char ucExtern2;
      char acFactoryInfo[HSPA_FACINFO_LENGTH];//厂商信息，取值为字符串。     
};


//#define	SOFTWARE_VERSION_QUERY	19
//#define	SOFTWARE_VERSION_QUERY_C 20
/*软件版本号查询 
命令格式：AT+GMR <CR><LF>+GMR:<softversion><CR><LF><CR><LF>OK<CR><LF>
有MS相关错误时：<CR><LF>ERROR<CR><LF>*/
/*软件版本号查询 
命令格式：AT+CGMR <CR><LF>+GMR:<softversion><CR><LF><CR><LF>OK<CR><LF>
有MS相关错误时：<CR><LF>ERROR<CR><LF>*/
struct stMsg_Software_Version_Query
{
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	char acSoftWareVersion[HSPA_SOFTWARE_LENGTH]; //软件版本号    
};


//#define	HARDWARE_VERSION_QUERY	21
//#define	HARDWARE_VERSION_QUERY_W  22
/*硬件版本号查询 
命令格式：AT^HVER 
<CR><LF>^HWVER:<hardversion><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
/*硬件版本号查询 
命令格式：AT^HWVER 
响应格式：<CR><LF>^HWVER:<hardversion><CR><LF><CR><LF>OK<CR><LF>
有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_Hardware_Version_Query
{
	unsigned char ucCmdFrom; 
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	char acHardWareVersion[HSPA_HARDWARE_LENGTH];  //硬件版本号   
};

	
//#define	IMSI_QUERY	27
/*IMSI查询 
命令格式：AT+CIMI 
响应格式：<CR><LF><IMSI><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_IMSI_Query
{
	unsigned char ucCmdFrom; 
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	char acIMSI[HSPA_IMSI_LENGTH]; //存储在卡中的IMSI值，为一个0～9的十进制数构成的字符串,  
};

/*#define	IMEI_QUERY	28
IMEI查询 
命令格式：AT+CGSN 
响应格式：<CR><LF><IMEI><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_IMEI_Query
{ 
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	char acIMEI[HSPA_IMEI_LENGTH];//单板的IMEI值,为一字符串   
};

/*#define	RSSI_QUERY	31
RSSI查询 
命令格式：AT+CSQ 
响应格式：<CR><LF>+CSQ: <rssi>,<ber><CR><LF><CR><LF>OK<CR><LF> */
struct stMsg_RSSI_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucRSSI;/*接收信号强度指示
	                        0   等于或小于 113 dBm
	                        1   111 dBm
	                        2   ...30  109...  53 dBm
	                        31 等于或大于 51 dBm
	                        99 未知或不可测*/
	unsigned char ucBer;//暂时不支持 
};

/*#define	SYSTEM_INFO_QUERY	 34
系统的信息查询 
命令格式：AT^SYSINFO 
响应格式：<CR><LF>^SYSINFO:< srv_status >,< srv_domain >,< roam_status >,< sys_mode >,< sim_state >[,<lock_state>,<sys_submode>]<CR><LF><CR><LF>OK<CR><LF> <CR><LF>^SYSINFO:< srv_status >,< srv_domain >,< roam_status >,< sys_mode >,< sim_state >[,<lock_state>,<sys_submode>]<CR><LF><CR><LF>OK<CR><LF>*/
struct stMsg_System_Info_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucSrvStatus;/*系统服务状态，取值如下：
	                              0:  无服务
	                              1:  有限制服务
	                              2:  服务有效 
	                              3:  有限制的区域服务
	                              4:  省电和深睡状态   */
	unsigned char ucSrvDomain;/*系统服务域，取值如下：
	                              0：    无服务
	                              1：    仅CS服务
	                              2：    仅PS服务
	                              3：    PS+CS服务
	                              4：    CS、PS均未注册，并处于搜索状态
	                              255：CDMA不支持。

					250:表示未获取到数值，无意义
	                              */
	unsigned char ucRoamStatus;/*漫游状态，取值如下：
	                              0：  非漫游状态
	                              1：  漫游状态 */
	unsigned char ucSysMode;/*系统模式，取值如下：
							0：无服务
							1：AMPS模式（暂不使用）
							2：CDMA模式（暂不使用）
							3：GSM/GPRS 模式
							4：HDR模式
							5：WCDMA模式
							6：GPS模式
							*/	
	unsigned char ucSimState;  /*SIM卡状态，取值如下：
	                              0： USIM卡状态无效
	                              1:  USIM卡状态有效  
	                              2： USIM在CS下无效（EW）
	                              3： USIM在PS下无效(EW)
	                              4： USIM在PS+CS下均无效(EW)
	                              240：ROMSIM版本
	                              255： USIM卡不存在   

					250:表示未获取到数值，无意义

	                              */
	unsigned char ucLockState;/*SIM卡的LOCK状态，取值如下：
	                               0：    SIM卡未被SIMLOCK功能锁定
	                               1：    SIM卡被SIMLOCK功能锁定   */
	unsigned char ucSysSubMode;/*    0：无服务
							              1：GSM模式
							              2：GPRS模式
							              3：EDGE模式
							              4：WCDMA模式
							              5：HSDPA模式
							              6：HSUPA模式
							7：HSDPA&HSUPA模式
							 */
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	unsigned char ucExtern3;
};

//#define	SYSCFG_SET	39
/*系统配置参考设置 
命令格式：AT^SYSCFG<mode>,<acqorder>,<band>,<roam>,<srvdomain> 
响应格式：<CR><LF>OK<CR><LF> */
struct stMsg_Syscfg_Set
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	SYSCFG_QUERY	40
/*系统配置参考设置查询 
//命令格式：AT^SYSCFG? 
//响应格式：
<CR><LF>^SYSCFG:<mode>,<acqorder>,<band>,<roam>,<srvdomain><CR><LF><CR><LF>OK<CR><LF>*/
struct stMsg_Syscfg_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucMode;/*系统模式参考： 
	                      2 自动选择
	                      13     GSM ONLY
	                      14     WCDMA ONLY
	                      16     无变化*/
	unsigned char ucAcqorder;/*网络接入次序参考：
	                         0    自动
	                         1    先GSM后WCDMA
	                         2    先WCDMA后GSM
	                         3     无变化*/
	char acBand[HSPA_BAND_LENGTH];/*频带选择，和模式的选择是相关的，具体取决于单板性能，其参数为16进制，
	                      取值为下列各参数或者是0x3FFFFFFF和0x40000000以外的各参数叠加值：
	                        80（CM_BAND_PREF_GSM_DCS_1800）               GSM DCS systems
	                        100（CM_BAND_PREF_GSM_EGSM_900）             Extended GSM 900
	                        200（CM_BAND_PREF_GSM_PGSM_900）             Primary GSM 900
	                        200000（CM_BAND_PREF_GSM_PCS_1900）            GSM PCS
	                        400000（CM_BAND_PREF_WCDMA_I_IMT_2000）   WCDMA IMT 2000
	                        3FFFFFFF（CM_BAND_PREF_ANY）           任何频带
	                        40000000（CM_BAND_PREF_NO_CHANGE）     频带不变化     */
	                         
	unsigned char ucRoam;/*漫游支持：
	                         0 不支持
	                         1 可以漫游
	                         2   无变化*/
	unsigned char usSrvDomain;/*域设置：
	                         0 CS_ONLY
	                         1 PS_ONLY
	                         2         CS_PS
	                         3         ANY
	                         4         无变化*/
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	SYSCONFIG_SET	41
/*系统配置参考设置 
命令格式：AT^SYSCONFIG =<mode>,<acqorder>,<roam>,<srvdomain> 
响应格式：<CR><LF>OK<CR><LF>*/ 
struct stMsg_Sysconfig_Set
{
	unsigned char ucCmdFrom;  
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	SYSCONFIG_QUERY	42
/*系统配置参考设置查询 
命令格式：AT^SYSCONFIG? 
响应格式：
<CR><LF>^SYSCONFIG:<mode>,<acqorder>,<roam>,<srvdomain><CR><LF><CR><LF>OK<CR><LF>*/
struct stMsg_Sysconfig_Query
{   
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucMode;/*系统模式参考： 
	                       <2>    自动选择
	                       <13>   GSM ONLY
	                       <14>   WCDMA ONLY
	                       <16 >  无变化*/
	unsigned char ucAcqorder;/*网络接入次序参考：
	                        <0>   自动
	                        <1>    先GSM后WCDMA
	                        <2>  先WCDMA后GSM
	                        <3>    无变化 */
	unsigned char ucRoam;/*漫游支持：
	                       <0> 不支持
	                       <1> 可以漫游
	                       <2>  无变化*/
	unsigned char usSrvDomain;/*域设置：
	                       <0> CS_ONLY
	                       <1> PS_ONLY
	                       <2>   CS_PS
	                       <3>   ANY
	                       <4>   无变化 */
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	PDP_SET	51
/*PDP设置环境 
命令格式：
AT+CGDCONT=<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>]]]]] 
响应格式：<CR><LF>OK<CR><LF> */
struct stMsg_PDP_Set
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	PDP_QUERY	52
/*PDP设置环境查询
命令格式：AT+CGDCONT? 
响应格式：<CR><LF>+CGDCONT: <cid>, <PDP_type>, <APN>,<PDP_addr>, <d_comp>, <h_comp>[<CR><LF>+CGDCONT: <cid>, <PDP_type>, <APN>,<PDP_addr>, <d_comp>, <h_comp>[...]]<CR><LF><CR><LF>OK<CR><LF> */
struct stMsg_PDP_Query
{      
	unsigned char ucCmdFrom;      
	unsigned char ucResult;//0-成功，其它－失败，反馈信息结构体都是如此
	unsigned char ucCidIndex;//1－16：PDP设置环境的索引值。其他PDP相关的命令可以通过此索引值来调用保存的设置。
	unsigned char ucDComp;//数字值，控制PDP数据的压缩。  0：不压缩。  1：压缩。
	unsigned char ucHComp;//数字值，控制PDP头的压缩。 0：不压缩。  1：压缩。
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	unsigned char ucExtern3;
	char acProtocolKind[HSPA_PROTOCOL_KIND_LENGTH];//字符串值，表示包交换协议类型。
	char acAPN[HSPA_APN_NAME_LENGTH];//字符串值，表示连接GGSN或外部网的接入点域名。
	char acDomainName[HSPA_DOMAIN_ADDR_LENGTH];//字符串值，表示MS的地址。
};

//#define	PIN_PASSWORD_MODIFY	45
/*PIN修改密码 
命令格式：AT+CPWD=<fac>,<oldpwd>,<newpwd>
响应格式：<CR><LF>OK<CR><LF>
有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_Pin_Password_Modify
{
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	PIN_ENABLE	48
/*PIN码使能 
命令格式：AT+ CLCK =<fac>,<mode>[,<passwd>[,<class>]]
响应格式：当 <mode>=2且命令成功执行时：<CR><LF>+CLCK: <status><CR><LF><CR><LF>OK<CR><LF>
当<mode>≠2且执行成功时：<CR><LF>OK<CR><LF>
有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_Pin_Password_Verification
{
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
	unsigned char ucStatus;// 0：未激活 1：激活 other:无效数值，不理会
	unsigned char ucExtern1;

};

/*add by w00135351 20080908*/
/*PIN码使能 
命令格式：AT+ CLCK =<fac>,<mode>[,<passwd>[,<class>]]
当<mode>≠2且执行成功时：
<CR><LF>OK<CR><LF>
有MT相关错误时：
<CR><LF>+CME ERROR: <err><CR><LF>
mode=0 解锁;mode=1 锁
*/
struct stMsg_Pin_Enable   /*暂时只定义两个元素，以后根据需要可以扩充*/
{
	unsigned char ucCmdFrom;      
	unsigned char ucResult;
};
/*end by w00135351 20080908*/

//#define	PIN_MANAGE	49
/*PIN管理
命令格式：
AT+CPIN=<pin>[,<newpin>] 
AT^CPIN=<pin>[,<newpin>] 
响应格式：<CR><LF>OK<CR><LF>
有MS相关错误时：
<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stPin_Verfication_Modify
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	DIAL	55
/*呼叫发起 
命令格式：ATD[digits][I/i][;] 
响应格式：<CR><LF>OK<CR><LF>执行错误<CR><LF>NO CARRIER<CR><LF>*/
struct stVoiceMsg_Dial
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	DIAG_PORT_SET	56
/*DIAG口切换设置 
命令格式：AT^DDSETEX=2     
响应格式：<CR><LF>OK<CR><LF>*/
struct stVoice_Msg_Diag_Port_Exchange
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};
//#define	ACCEPT	57
/*接听 
命令格式：ATA 
响应格式：<CR><LF>OK<CR><LF>*/ 
struct stVoiceMsg_Accept
{
	unsigned char ucCmdFrom; 
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	CHUP	58
/*呼叫挂断 
命令格式：AT+CHUP 
响应格式：<CR><LF>OK<CR><LF>*/ 
struct stVoiceMsg_Hung
{   
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};

//#define	CALL_TIME_QUERY	62
/*通话时长查询 
命令格式：AT^CDUR=<idx> 
响应格式：<CR<LF>^cdur:<idx>,<duration><CR<LF><CR<LF>OK<CR<LF>*/
struct stVoiceMsg_C
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	unsigned long ulDuration;/*通话时长，以s为单位*/
};

//#define	AUTO_SIM_STATUS_REPORT	63
/*SIM 状态变化指示  
响应格式：<CR><LF>^SIMST:<sim_state>[,<lock_state>]<CR><LF>*/
struct stMsg_Sim_State
{
	unsigned char ucCmdFrom;
	unsigned char ucResult; 
	unsigned char ucSimState;/*USIM卡状态，取值如下：
	                             0：     USIM卡状态无效
	                             1:      USIM卡状态有效  
	                             2：     USIM在CS下无效
	                             3：     USIM在PS下无效
	                             4：     USIM在PS+CS下均无效
	                             255： USIM卡不存在，包括USIM卡确实不存在和因为被SIMLOCK锁住而不存在两种情况。
	                                    此时，应通过ucLockState的取值判定USIM卡的真实状态。*/
	unsigned char ucLockState;/*SIM卡的LOCK状态，取值如下：
	                               0：    SIM卡未被SIMLOCK功能锁定
	                               1：    SIM卡被SIMLOCK功能锁定 */
};

//#define	RING_REPORT	 64
/*来电响铃指示  
响应格式：<CR><LF>RING<CR><LF>*/
struct stVoiceMsg_Ring
{  
	unsigned char ucCmdFrom;
	unsigned char ucResult;
};

//#define	CALL_NUM_REPORT	65
/*来电号码显示  
响应格式：<CR><LF>+CLIP:<number>,<type>,,,,<CLI validity><CR><LF> */
struct stVoiceMsg_Call_Num
{
	unsigned char ucCmdFrom;
	unsigned char ucResult; 
	unsigned char ucTelType;/*号码类型，其中145表示国际号码，具体取值可参见“短信发送命令 +CMGS”一节中，
	                           SC号码中的type_addr参数的具体定义。255,CDMA不支持*/
	unsigned char ucCliFlag;/*0 CLI 有效
	                            1 CLI 被呼叫发起方拒绝.
	                            2 由于发起网络的限制或网络问题造成CLI不可用。
	                            <type>与<CLI validity>之间保留了三个字段。*/
	char acTelNum[HSPA_TELNUM_LENGTH];/*来电号码，ASCII 字符，合法的字符仅包括：’0’ – ‘9’, ‘*’, ‘//#’*/
};

//#define	CALL_END_REPORT	66
/*通话结束指示  
//响应格式：
<CR><LF>^CEND:<call_x>,< duration >,<end_status>[,<cc_cause>]<CR><LF>*/
struct stVoice_Msg_Call_End
{
      unsigned char ucCmdFrom;
      unsigned char ucResult;
      unsigned char ucCallId;/*呼叫ID，EC取值0－6*/
      unsigned char ucEndCause;/*呼叫结束原因，EC取值255*/
      unsigned long ulDuration;/*通话时长，以s为单位*/
      unsigned char ucCallCtlCause;/*呼叫控制信息 EC取值255
                                     注：如果为网侧引发的呼叫结束才会有字段<cc_cause>的上报；
                                     如果本地发出的呼叫，还没有得到网侧的响应，
                                     呼叫就结束，此时就不会有<cc_cause>上报。*/
};

//#define	CFUN_SET	14
/*操作模式设置命令
命令格式 AT+CFUN=<fun> 
响应格式：<CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>
*/

struct stMsg_Operation_Mode_Set
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};
//#define	CFUN_QUERY	15
/*操作模式查询命令
命令格式 AT+CFUN？ 
响应格式：<CR><LF>+CFUN: <fun><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/

struct stMsg_Operation_Mode_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucOperationMode;
	unsigned char ucExtern1;
};
//#define	PRODUCT_NAME_QUERY	16
/*产品名称查询
命令格式：AT+CGMM
响应格式：<CR><LF> < production_ name><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_Product_Name_Query
{
	unsigned char ucCmdFrom; 
	unsigned char ucResult;
	char acProductName[HSPA_PRODUCT_NAME_LENGTH];  //产品名   
	unsigned char ucExtern1;
	unsigned char ucExtern2;
};
//#define	AUTO_SERVICE_STATUS_REPORT	67
/*服务状态变化指示 ^SRVST（EW 新增）
自动上报格式： <CR><LF>^SRVST:<srv_status><CR><LF>
当服务状态发生变化时，MS主动上报新的服务状态给TE。*/
struct stMsg_Service_Status
{ 
	unsigned char ucCmdFrom;
	//消息内容
	unsigned char ucResult;
	unsigned char ucSrvStatus;/*系统服务状态，取值如下：
	                       0:  无服务
	                       1:  有限制服务
	                       2:  服务有效 
	                       3:  有限制的区域服务
	                       4:  省电和深睡状态*/
	unsigned char ucExtern1;
};
//#define	AUTO_SYSTEM_MODE_STATUS_REPORT	68
/*系统模式变化事件指示 ^MODE
响应格式：^MODE:<sys_mode>[,<sys_submode>]*/
struct stMsg_System_Mode
{ 
	unsigned char ucCmdFrom;
	//消息内容
	unsigned char ucResult;
	unsigned char ucSysMode;/*系统模式
		                       0：无服务
		                       1：AMPS模式（暂不使用）
		                       2：CDMA模式（暂不使用）
		                       3：GSM/GPRS 模式
		                       4：HDR模式
		                       5：WCDMA模式
		                       6：GPS模式*/
	unsigned char ucSubSysMode;/*系统模式的子模式，取值如下：
				      0：无服务
				      1：GSM模式
				      2：GPRS模式
				      3：EDGE模式
				      4：WCDMA模式
				      5：HSDPA模式
				      6：HSUPA模式
				      7：HSDPA&HSUPA模式*/
};
/*PIN管理命令AT^CPIN?
响应格式：^CPIN: <code>,[<times>],<puk_times>,<pin_times>,<puk2_times>,<pin2_times><CR><LF><CR><LF>OK<CR><LF>*/
struct stMsg_Pin_Time_Query
{ 
      unsigned char ucCmdFrom;
      unsigned char ucResult;
    unsigned char ucType;
    unsigned char ucTimes;/*剩余的输入次数，对于PIN和PIN2，最大输入次数是3次，对于PUK和PUK2最大输入次数是10次。
说明：如果存在密码输入请求，则<times>字段会给出当前需要校验的密码的剩余校验次数，如果不存在密码输入请求则该字段为空。
*/
    unsigned char ucPukTimes; /*<puk_times>：剩余的PUK输入次数，最大输入次数是10次。*/
    unsigned char ucPinTimes; /*<pin_times>：剩余的PIN输入次数，最大输入次数是3次*/
    unsigned char ucPuk2Times; /*<puk2_times>：剩余的PUK2输入次数，最大输入次数是10次。*/
    unsigned char ucPin2Times;  /*<pin2_times>：剩余的PIN2输入次数，最大输入次数是3次*/
};
/*
<CR><LF>^CARDMODE:<sim_type><CR><LF><CR><LF>OK <CR><LF>
<sim_type>：SIM卡的类型，取值如下：
0:  Unknown mode
1:  SIM卡
2:  USIM卡*/
struct stMsg_Sim_Type
{
      unsigned char ucCmdFrom;
      unsigned char ucResult;
      unsigned char ucSimType;
	unsigned char ucExtern1;
};

/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add begin:*/
/*
SN查询 
命令格式：AT^SN 
响应格式：<CR><LF><SN><CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>*/
struct stMsg_SN_Query
{ 
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucExtern1;
	unsigned char ucExtern2;
	char acSN[HSPA_SN_LENGTH];//单板的SN值,为一字符串   
};

/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add end.*/
struct stMsg_CGACT_Query
{
	unsigned char ucCmdFrom;
	unsigned char ucResult;
	unsigned char ucCid;
	unsigned char ucState;
};


/*发送往HSPA控制进程的消息结构体*/
struct stAtSndMsg
{
	long int lMsgType;//命令所对应的宏	
	char acParam[AT_SND_MSG_LENGTH];
};
/*接收来自HSPA控制进程的消息结构体*/
struct stAtRcvMsg
{
	long int lMsgType; 
	char acText[AT_RCV_MSG_LENGTH];
};

int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType);
int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TTYUSB_H__ */




