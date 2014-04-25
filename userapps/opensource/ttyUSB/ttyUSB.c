/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : ttyUSB.c
  版 本 号   : 初稿
  作    者   : 曹祥
  生成日期   : 2008年1月11日
  最近修改   :
  功能描述   : 完成HSPA控制模
  函数列表   :
 
  修改历史   :
  1.日    期   : 2008年1月11日
    作    者   : 曹祥
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <memory.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/msg.h>
#include<stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include"ttyUSB.h"
#include<signal.h>
#include "board_api.h"
#include <semaphore.h>
#include<sys/ioctl.h>
#include <stdarg.h>
#include "syscall.h"

/*add by w00135351*/
#include "tr69cdefs.h"
#include "app/cwmp_init.h"
/*end by w00135351*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void*  WritePcui(void* arg);/*向HSPA写指令函数*/
static void* SendSysinfo(void* arg);
static   int ReadPcui();
static  int MsgIdCompute(unsigned char ucCmdFrom);

/*HSPA反馈信息解析函数*/

static void FacInfoAnlysis(char *pszInfoBack); //FACTORY_INFO_QUERY 17--AT+GMI\r
static void FacInfoAnlysis_C(char *pszInfoBack); //FACTORY_INFO_QUERY_C 18--AT+CGMI\r
static void SoftwareVersionAnlysis(char *pszInfoBack) ; //SOFTWARE_VERSION_QUERY 19--AT+GMR\r
static void SoftwareVersionAnlysis_C(char *pszInfoBack) ; //SOFTWARE_VERSION_QUERY_C	20--AT+CGMR\r
static void HardwareVersionAnlysis(char *pszInfoBack) ;//HARDWARE_VERSION_QUERY  21--AT^HVER\r
static void HardwareVersionAnlysis_W(char *pszInfoBack) ;//HARDWARE_VERSION_QUERY  22--AT^HWVER\r
static void ImsiInfoAnlysis(char *pszInfoBack) ;//IMSI_QUERY 27--AT+CIMI\r
static void ImeiInfoAnlysis(char *pszInfoBack) ;//IMEI_QUERY 28--AT+CGSN\r
static void RssiInfoAnlysis(char *pszInfoBack) ;//RSSI_QUERY 31----AT+CSQ\r
static void SystemInfoQuery(char *pszInfoBack);
static void CfgInfoSetBack(char *pszInfoBack);
static void CfgInfoQuery(char *pszInfoBack);
static void ConfigInfoSet(char *pszInfoBack);
static void ConfigInfoQuery(char *pszInfoBack);
static void CgdcontSet(char *pszInfoBack);
static void CgdcontQuery(char *pszInfoBack);
static void OperationInfoQuery(char *pszInfoBack);
static void CardModeQuery(char *pszInfoBack);

static void PinNumModify(char *pszInfoBack);
static void PinEnableSet(char *pszInfoBack);
static void PinEnableQuery(char *pszInfoBack);
static void PinNumVerfication(char *pszInfoBack);
static void PinNumLeftQuery(char *pszInfoBack);

static void TelNumReport(char *pszInfoBack);
static void CallDial(char *pszInfoBack);
static void AcceptCall(char *pszInfoBack);
static void EndCallReport(char *pszInfoBack);
static void ChupBack(char *pszInfoBack);
static void AutoConf(char *pszInfoBack);

/* HUAWEI HGW l65130 2008-08-07 begin*/
static void CgactQuery(char *pszInfoBack);
/* HUAWEI HGW l65130 2008-08-07 begin*/

static void CfunSet(char *pszInfoBack);
static void CfunQuery(char *pszInfoBack);
static void AutoModeReport(char *pszInfoBack);
static void AutoServerStatus(char *pszInfoBack);
static void AutoRing(char *pszInfoBack);
static void AutoConn(char *pszInfoBack);
static void AutoDataFlow(char *pszInfoBack);
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add begin:*/
static void SnQuery(char *pszInfoBack);
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add end.*/
static int  ImeiInfoGet();
static void DExSet(char *pszInfoBack);
/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add begin:*/
static void VoiceEnableSet(char *pszInfoBack);
/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add end.*/

static int  PinPukProcess(char *pszInitHspa);
void HspaDeviceSignalProcess(int x);
void HspaDeviceDetect(void);
void HspaSwtichDelay(void);
//add by z67625
int TTY_Output(const char* fmt,...);
int debugswitch(void);
void TTY_WriteDebug(char *pcTmp);

 unsigned long int HexToNum( char cChar);
unsigned long int  aHexToUl(char * pszChar);

static int RegisterNtwk(void);

/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add begin:*/
static int VoiceJudge(void);
/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add end.*/

/*Begin:Added by luokunling 00192527 for imsi inform requirement,2011/9/19*/
int ImsiNumIsOldImsiNum(char *newImsiNum); 
void DelayTimeInformIMSI(void* arg);
/*End:Added by luokunling 00192527,2011/9/19*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
struct stAt_Fun_Com
{
	char acAtCmdPart[COMMON_SHORT_STRING_LENGTH];
	void (*pfAtBackInfoAnalysis)(char *pszChar);
};

static struct stAt_Fun_Com 	g_stAtFunCom[] =
{
	{"AT+GMI",	FacInfoAnlysis},
	{"AT+CGMI", FacInfoAnlysis_C},
	{"AT+GMR",	SoftwareVersionAnlysis},
	{"AT+CGMR", SoftwareVersionAnlysis_C},
	{"AT^HVER",	HardwareVersionAnlysis},
	{"AT^HWVER",	HardwareVersionAnlysis_W},
	{"AT+CIMI",	ImsiInfoAnlysis},
	{"AT+CGSN",	ImeiInfoAnlysis},
	{"AT+CSQ",	RssiInfoAnlysis},
	{"AT^SYSINFO",	SystemInfoQuery},
	{"AT^SYSCFG=",	CfgInfoSetBack},
	{"AT^SYSCFG?",	CfgInfoQuery},
	{"AT^SYSCONFIG=",	ConfigInfoSet},
	{"AT^SYSCONFIG?",	ConfigInfoQuery},
	{"AT+CGDCONT=",	CgdcontSet},
	{"AT+CGDCONT?",	CgdcontQuery},
	{"AT+CPWD=\"SC\",",	PinNumModify},
	{"AT+CLCK=\"SC\",2",	PinEnableQuery},
	{"AT+CLCK=\"SC\",0,",	PinEnableSet},
	{"AT+CLCK=\"SC\",1,",	PinEnableSet},
	{"AT+CPIN=",	PinNumVerfication},
	{"AT^CPIN=",	PinNumVerfication},
	{"AT^CPIN?",	PinNumLeftQuery},
	{"AT+CPIN?",	PinNumLeftQuery},
	{"ATD",	CallDial},
	{"ATA",	AcceptCall},
	{"\r\nRING\r\n",	AutoRing},
	{"\r\n+CLIP:",	TelNumReport},
	{"\r\n^CEND:",	EndCallReport},
	{"AT+CFUN=",	CfunSet},
	{"AT+CFUN?",	CfunQuery},
	{"\r\n^SRVST:",	AutoServerStatus},
	{"\r\n^MODE:",	AutoModeReport},
    	{"\r\n^CONN",  AutoConn}  ,
	{"AT+COPS?", OperationInfoQuery},
	{"AT^CARDMODE", CardModeQuery},
	{"AT+CHUP", ChupBack},
	{"\r\n^DSFLOWRPT:", AutoDataFlow},
	{"\r\n^CONF", AutoConf},
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add begin:*/
    {"AT^SN",   SnQuery},
/* HUAWEI HGW s48571 2008年2月15日 获取SN命令 add end.*/
    {"AT^DDSETEX=2",DExSet},
    {"AT+CGACT?", CgactQuery},
/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add begin:*/
    {"AT^CVOICE=",VoiceEnableSet}
/* HUAWEI HGW s48571 2008年8月16日 语音关闭需求 add end:*/
};

/*Begin:Added by luokunling 00192527 for imsi inform requirement,2011/9/19*/
static int g_iDelaytime_flg    = 0;                /*网关插上数据卡，重启时延迟主动上报TR069 标记*/
int g_iHspaWeatherAutoPin = 0;                /*hspa 是否自动解  pin   ，保存pin码，1 - 自动解聘*/
/*End:Added by luokunling 00192527 ,2011/9/19*/

static struct stAtSndMsg g_stAtFromOther;/*业务消息队列发给HSPA控制模块的消息*/
static struct stAtRcvMsg g_stAtToOther;/*HSPA控制模块发给业务模块的消息*/
/*是否需要加锁，有待确认*/
static int g_lAtMsgId = -1;	/*HSPA控制模块消息队列ID*/
static int g_lModemMsgId = -1;/*数据业务消息队列ID*/
static int g_lVoiceMsgId = -1;/*语音业务消息队列ID*/
static int g_lTr069MsgId = -1;/*tr069消息队列ID*/
static int g_lHttpMsgId = -1;/*WEB配置查询消息队列ID*/
static int g_lLedMsgId = -1;/*LED指示灯消息队列ID*/
#ifdef CX_DEBUG_VERSION	
static int g_lCmdMsgId = -1;/*桩测试消息队列ID*/
#endif

#ifdef POTS_PINMNGR
static int g_lPinMngrMsgId = -1; /*PIN管理业务消息队列ID*/
#endif
static int g_iPcuiFd = -1;	/*PCUI口对应的串口设备文件fd*/
static int g_ShareiFd = -1;/*数据共享文件fd*/
static int g_HspaExistFlag = HSPA_UNKNOWN;
static int g_iWriteFlag = -1;
static int g_SignalRcv = 0;
static pthread_mutex_t work_mutex;
static sem_t sem;
static struct stHspaInfo g_stHspa;/*存储数据卡的常用状态，
							 存在/var/HspaStatus文件中，供
							 	各个业务模块查询*/
static int g_sndflg = 0;

/*Begin:Added by luokunling 00192527 for imsi inform requirement,2011/9/19*/
#define TR69_PID_FILE  "/var/tr69/tr69pid"
/*End:Added by luokunling 00192527,2011/9/19*/
#define HSPA_OFF 0
#define HSPA_ON 1
#define MID_STATE_MASK 0x00f0
#define HSPA_STATE_MASK 0x000f
#define HSPA_LINE_MASK 0x00ff

static int g_HspaLine = HSPA_ON;

#define VOICE_CALLOUT_IDLE 0
#define VOICE_CALLOUT_DAIL 1
#define VOICE_CALLOUT_AUTOCONF 2
static int g_iCallInFlag = VOICE_CALLOUT_IDLE;

#define VOICE_CALLIN_IDLE 0
#define VOICE_CALLIN_ATA 1
#define VOICE_CALLIN_CONN 2
#define VOICE_CALLIN_SETD 3
static int g_iCallOutFlag = VOICE_CALLIN_IDLE;
/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/

#define UNKNOW_ON_HSPA  0x10
#define NO_CALL_DATA  0x0
#define CALL_ON_HSPA  0x2
#define DATA_ON_HSPA  0x4
static unsigned int  g_ulTrafficFlag = NO_CALL_DATA;
/* END:   Added by c106292, 2009/3/16 */

/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin:*/
static struct stHspaInfo  g_stHspaBackup;

/*add by z67625*/
int debug = 0;//调试开关
int OutputTerm = 2;//2//2输出到串口，1输出到串口和telnet

#define TTY_DEBUG TTY_Output("\r\nfunction: [%s] ,Line [%u] ,",__FUNCTION__, __LINE__);    TTY_Output
#define TTY_LOG  TTY_Output
#define TTYDEBUGPATH "/var/ttydebuginfo"
#define TTYDEBUG  "/var/ttydebug"
#define TTYPID    "/var/ttypid"
//add by z67625

#define MAX_PIN_TEST 5

#define VOICE_UNAVAILABLE 1
#define VOICE_AVAILABLE 0

#define MAX_ATCMD_CNT  3

static void BackupHspaInfo(struct stHspaInfo *pstHspa )
{
	memcpy(&g_stHspaBackup, pstHspa, sizeof(g_stHspaBackup));
}

static int CheckHspaInfo( struct stHspaInfo *pstHspa)
{
	static int voiceStatusBackup = VOICE_UNAVAILABLE ;
	int voiceStatus =  VOICE_UNAVAILABLE;
	voiceStatus = ( 0 == pstHspa->sDeviceCount ) ||( -1 == pstHspa->sDeviceCount )
				|| ( 0 == pstHspa->sSimExist ) || ( 255 == pstHspa->sSimExist )
				|| ( 1 == pstHspa->sPinPukNeed ) ||( 2 == pstHspa->sPinPukNeed )
				|| ( 0 == pstHspa->sSysMode ) || ( -1 == pstHspa->sSysMode );
	
	if ( ( g_stHspaBackup.sDeviceCount != pstHspa->sDeviceCount )
		|| ( g_stHspaBackup.sSimExist != pstHspa->sSimExist ) 
		||( g_stHspaBackup.sPinPukNeed != pstHspa->sPinPukNeed ) 
		|| ( g_stHspaBackup.sSysMode != pstHspa->sSysMode ))
	{
		if ( voiceStatus != voiceStatusBackup )
		{
			voiceStatusBackup = voiceStatus;
			return HSPA_OK;
		}
	}
	
	return -1;
}
/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

/*begin:Added by luokunling 00192527 for imsi info requirement,2011/9/19*/
//*********************************************************************************
// Function Name: DelayTimeInformIMSI
// Description  : 网关插上数据卡重启，并自动解pin后延迟上报IMSI值
// Returns      : void
// 修改历史:
//                     新生成函数
//                     修改人:luokunling
//                     修改日期:2011/9/19
//*********************************************************************************
void DelayTimeInformIMSI(void* arg)
{
   int DelayTime = 60 ;
   int Number = 0;
   FILE *tr69cpidflie = NULL;
   int tr69cpid = 0;
   struct stAtSndMsg stAtFromOther;
   
   while(Number < DelayTime)
   {
        tr69cpidflie = fopen(TR69_PID_FILE,"r");
	 if(NULL != tr69cpidflie)   /*tr69c进程已启动，向数据卡发送IMSI请求*/
	 {
	    if(1 == g_iHspaWeatherAutoPin)
	    {
        	    AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, IMSI_QUERY, "AT+CIMI");
        	    usleep(500);
	    }
	    else
	    {
	           TR069_SendMessage(TTYUSB_INFORM_TR069,NULL,0);
	    }
		   
	    g_iDelaytime_flg = 0;   
		fclose(tr69cpidflie);	
		break;  
	 }
	 
	 Number = Number + 3;
	 sleep(3);
   }
   
   g_iDelaytime_flg = 0;
   
}
/*End:Added by luokunling 00192527 for imsi info requirement,2011/9/19*/



int HspaMatchRead(char *pszInfoBack)
{
    int lLen = 0;
	if ( NULL == strstr(pszInfoBack, "\r\n") )
	{
		memset(pszInfoBack, 0, AT_RCV_MSG_LENGTH);
		lLen= read(g_iPcuiFd, pszInfoBack, AT_RCV_MSG_LENGTH);
		//APN ZHOURONGFEI KF34753
		if(NULL==strstr(pszInfoBack,"CGDCONT"))
		{
        		TTY_LOG("[ %s ]\n",pszInfoBack);
		}
		//APN ZHOURONGFEI KF34753
	}
    return lLen;

}

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HSPA_DEVICE_COUNT "Usb-Serial Port Num:3\r\nOpen:1\r\n"
#define HSPA_ERR_LIMIT      10

#ifdef CX_DEBUG_VERSION	
#define cx_printf(format, args...) printf(format, ## args)
#else
#define cx_printf(format, args...)
#endif

int glbSpecStick = 0;   /*是否是特殊的数据卡（如e180），不能发cfun＝0，只能
                          发cfun＝7*/
char glbStick[]="K3715";
                          
int main()
{
	pid_t pid;
	pid =  fork();
	if( 0==pid)
	{
		cx_printf("%s,%d,%s():child process going!\n",__FILE__, __LINE__, __FUNCTION__);
        	TTY_LOG("child process going!\n");
		ttyusb_main();
	}
	else
	if(pid!=0)
	{
		cx_printf("%s,%d,%s():parent process exit!\n",__FILE__, __LINE__, __FUNCTION__);
        	TTY_LOG("parent process exit!\n");
		exit(0);
	}
	return 0;
}
/*****************************************************************************
 函 数 名  : main
 功能描述  : HSPA控制模块主函数，其中含有两个线程，
 				一个线程读取指令反馈信息，一个线程下发
 				AT指令
 输入参数  : void  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

 执行流程:
 1、邦定消息处理函数，
 2、创建消息队列；
 3、打开"/proc/UsbModem"文件，创建共享文件，获取进程PID,
 	判断设备在否；在－>4，否－>5。
 4、打开PCUI口，设备存在标志置1；初始化串口属性，初始化HSPA设备
 	发送获取共享文件中参数的AT指令。
 5、设备存在标志置0；
 6、写入共享文件。PCUI口写使能置1。
 8、创建读取PCUI口线程。运行写PCUI口函数。

*****************************************************************************/

int ttyusb_main( void )
{
	pthread_t thread_Write;/*写PCUI口的线程标志*/
	pthread_t thread_SendSysinfo;/*发AT^SYSINFO线程标志*/
	int fdProc = -1;
    int fd = -1;
	char acPid[16];
    char acCmd[64];
	/*设定信号处理函数*/
	signal(SIGUSR1, HspaDeviceSignalProcess);
    //add by z67625
    signal(SIGUSR2,debugswitch);
    fd = open(TTYPID, O_RDWR|O_CREAT);
    if (fd < 0)
    {
        TTY_DEBUG("open /var/ttypid failed\n");
        return 0;
    }
    else
    {
        sprintf(acPid, "%d", getpid());
        write(fd, acPid, sizeof(acPid));
        fsync(fd);
        close(fd);
    }
    //add by z67625
	
	if (0 != pthread_mutex_init(&work_mutex,NULL))
	{
	    TTY_DEBUG("init work_mutex failed\n");
		return 0;
	}
	if( 0 != sem_init(&sem, 0, 1) )
	{
	    TTY_DEBUG("init sem failed\n");
		return 0;
	}

	memset(acPid, 0, sizeof(acPid));
	memset(&g_stHspa, 0, sizeof(struct stHspaInfo));

	/*创建共享文件*/
	while(g_ShareiFd<0)
		g_ShareiFd = open(HSPA_SHARE_FILE_PATH, O_RDWR|O_CREAT);
	cx_printf("%s, %d, %s, pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    TTY_LOG("pid = %d\n",getpid());
	g_stHspa.lPid =getpid();
	/*将设备共享文件中的数据设置为未知	*/
	g_stHspa.sDeviceCount = HSPA_UNKNOW;
	g_stHspa.sDeviceStatus = HSPA_UNKNOW;
	g_stHspa.sSimExist = HSPA_UNKNOW;
	g_stHspa.sSimStatus = HSPA_UNKNOW;
	g_stHspa.sSysMode = HSPA_UNKNOW;
	g_stHspa.sSubSysMode = HSPA_UNKNOW;
	g_stHspa.sSrvStatus = HSPA_UNKNOW;
	g_stHspa.sPinPukNeed= HSPA_UNKNOW;

	/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin:*/
	BackupHspaInfo(&g_stHspa);
	/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

    fdProc = open("/proc/UsbModem", O_RDWR);
    if (fdProc < 0)
    {
        TTY_DEBUG("open /proc/UsbModem failed\n");
        return 0;
    }
    sprintf(acPid, "%d", g_stHspa.lPid);
    write(fdProc, acPid, sizeof(acPid));
    fsync(fdProc);
    close(fdProc);

	/*更新*/
	lseek(g_ShareiFd, 0, SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);


	/*创建消息队列*/
	while ( g_lAtMsgId < 0 )
		g_lAtMsgId = msgget((key_t)MSG_AT_QUEUE, IPC_CREAT |0666);/*HSPA控制模块消息队列ID*/

	while ( g_lModemMsgId < 0 )
		g_lModemMsgId = msgget((key_t)MSG_MODEM_QUEUE,  IPC_CREAT |0666);/*数据业务消息队列ID*/

	while ( g_lVoiceMsgId < 0 )
		g_lVoiceMsgId = msgget((key_t)MSG_VOICE_QUEUE,  IPC_CREAT |0666);/*语音业务消息队列ID*/

	while ( g_lTr069MsgId < 0 )
		g_lTr069MsgId = msgget((key_t)MSG_TR069_QUEUE,  IPC_CREAT |0666);/*tr069消息队列ID*/

	while ( g_lHttpMsgId < 0 )
		g_lHttpMsgId = msgget((key_t)MSG_HTTP_QUEUE,  IPC_CREAT |0666);/*WEB配置查询消息队列ID*/

	while ( g_lLedMsgId < 0 )
		g_lLedMsgId = msgget((key_t)MSG_LED_QUEUE,  IPC_CREAT |0666);/*LED指示灯消息队列ID*/

#ifdef CX_DEBUG_VERSION	
	while ( g_lCmdMsgId < 0 )
		g_lCmdMsgId = msgget((key_t)MSG_CMD_QUEUE,  IPC_CREAT |0666);/*桩测试消息队列ID*/
#endif
#ifdef POTS_PINMNGR
    while ( g_lPinMngrMsgId < 0 )
        g_lPinMngrMsgId = msgget((key_t)MSG_PINMNGR_QUEUE,  IPC_CREAT |0666);/*语音PIN管理消息队列ID*/
#endif

#ifdef CX_DEBUG_VERSION	
	cx_printf( "g_lAtMsgId = %d\n", g_lAtMsgId);
	cx_printf( "g_lModemMsgId = %d\n", g_lModemMsgId);
	cx_printf( "g_lVoiceMsgId = %d\n", g_lVoiceMsgId);
	cx_printf( "g_lTr069MsgId = %d\n", g_lTr069MsgId);
	cx_printf( "g_lHttpMsgId = %d\n", g_lHttpMsgId);
	cx_printf( "lCmdMsgId = %d\n", g_lCmdMsgId);
#ifdef POTS_PINMNGR    
    cx_printf( "g_lPinMngrMsgId = %d\n", g_lPinMngrMsgId);
#endif
#endif
    TTY_LOG( "g_lAtMsgId = %d\n", g_lAtMsgId);
	TTY_LOG( "g_lModemMsgId = %d\n", g_lModemMsgId);
	TTY_LOG( "g_lVoiceMsgId = %d\n", g_lVoiceMsgId);
	TTY_LOG( "g_lTr069MsgId = %d\n", g_lTr069MsgId);
	TTY_LOG( "g_lHttpMsgId = %d\n", g_lHttpMsgId);
		//HspaDeviceSignalProcess(1);
        HspaDeviceDetect();

	/*将USB-SERIAL的IFD传给子进程收发消息循环
	问题:目前还没考虑的是设备在循环中设备断开如何处理?	*/	
	cx_printf("%s, %d, %s(),pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	while ( 0 != pthread_create(&thread_Write,  NULL, WritePcui, NULL) )
		;
	/* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/9 
	   for 解决3G环境里,插上E180数据卡一直亮红灯问题.因为E180搜网时间较长,插上数据卡或系统启动时发送AT^SYSINFO,返回的SysMode还未变化
	   而后续只有收到数据卡主动发过来^MODE指令我们才会发送AT^SYSINFO. 所以ttyUSB启动时启动一线程每隔10发一次AT^SYSINFO,其它代码都不修改,保证不影响原有的流程.*/
	usleep(100);
	while ( 0 != pthread_create(&thread_SendSysinfo,  NULL, SendSysinfo, NULL) )
		;
	/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/9 */
	cx_printf("%s, %d, %s(),pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	ReadPcui();
	cx_printf("%s, %d, %s(),pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	pthread_mutex_destroy(&work_mutex);
	sem_destroy(&sem);
	return 0;
	
}

/*****************************************************************************
 函 数 名  : ImsiInfoGet
 功能描述  : 
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月16日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static int  ImeiInfoGet()
{
	char acInitHspa[COMMON_LONG_STRING_LENGTH];
	char *pChar = NULL;
	int len = 0;
	int i = 0;
	len = write(g_iPcuiFd, "AT+CGSN\r", strlen("AT+CGSN\r"));
	if (len <0 )
	{
	    TTY_DEBUG("write AT+CGSN failed\n");
		return HSPA_ERROR;
	}
	
	usleep(20);
	len = read(g_iPcuiFd, acInitHspa, sizeof(acInitHspa));
	if (len <0 )
	{
	    TTY_DEBUG("read pcuifd failed\n");
		return HSPA_ERROR;
	}	
	cx_printf("%s, %d, %s(), acInitHspa = %s\n",__FILE__, __LINE__, __FUNCTION__, acInitHspa);
	
	if ( NULL == strstr(acInitHspa, "AT+CGSN") )/*判断反馈信息*/
	{
		memset(acInitHspa, 0, sizeof(acInitHspa));
		len = read(g_iPcuiFd, acInitHspa, sizeof(acInitHspa));
		if (len <0 )
		{
		    TTY_DEBUG("read pcuifd failed\n");
			return HSPA_ERROR;
		}
	}
	cx_printf("%s, %d, %s(), acInitHspa = %s\n",__FILE__, __LINE__, __FUNCTION__, acInitHspa);
	
	if ( NULL != strstr(acInitHspa, "AT+CGSN") )/*判断反馈信息*/
	{
		if(  NULL == strstr(acInitHspa, "\r\n") )
		{
			memset(acInitHspa, 0, sizeof(acInitHspa));
			len = read(g_iPcuiFd, acInitHspa, sizeof(acInitHspa));
			if (len <0 )
			{
			    TTY_DEBUG("read pcuifd failed\n");
				return HSPA_ERROR;
			}
		}
		cx_printf("%s, %d, %s(), acInitHspa = %s\n",__FILE__, __LINE__, __FUNCTION__, acInitHspa);
		
		if ( NULL != strstr(acInitHspa, "OK") )/*判断反馈信息是否OK*/
		{
			pChar = strstr(acInitHspa, "\r\n");
			if ( NULL == pChar)
			{
			    TTY_DEBUG("acInitHspa string error\n");
				return;
			}
			pChar += 2;

            len = strlen(pChar);
			while( i <= len )
			{
				if ( isdigit(*pChar) )
				{
					g_stHspa.acIMEI[i] = *pChar;
				}
				else
				{
					g_stHspa.acIMEI[i] = 0;
					break;
				}
				++pChar;
				++i;				
			}
		}
	}
	cx_printf("%s, %d, %s(), acIMEI = %s\n",__FILE__, __LINE__, __FUNCTION__, g_stHspa.acIMEI);
    TTY_LOG("acIMEI = %s\n",g_stHspa.acIMEI);
	lseek(g_ShareiFd, 0,SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);
	return HSPA_OK;
	
}

/*****************************************************************************
 函 数 名  : PinPukProcess
 功能描述  : 
 输入参数  : char *pszInitHspa  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月15日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static int PinPukProcess(char *pszInitHspa)
{
	
#define IFC_IMEI_LEN         16          //IMEI缓冲区大小
#define IMEI_OFFSET          96          //IMEI相对变量存储区首地址的偏移量
#define IFC_PIN_PASSWORD_LEN   16          //PIN码的缓冲区大小
#define PINCODE_OFFSET         80          //相对变量存储区首地址的偏移量  	  
 	char acPincode[IFC_PIN_PASSWORD_LEN] = {0};
	char acInitHspa[COMMON_LONG_STRING_LENGTH];
	int len = 0;
    int i = 0;
    /* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） add begin. */
    int iRet = HSPA_OK;
    /* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） add end. */
    
    memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
    TTY_DEBUG("pszInitHspa = %s\n", pszInitHspa);
	memcpy(acInitHspa, pszInitHspa, COMMON_LONG_STRING_LENGTH);

    for (i = 0; i < MAX_ATCMD_CNT; i++)
    {
        if ( NULL == strstr(acInitHspa,"AT+CLCK=\"SC\",2") )
    	{
    		memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
    		len = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
    		if ( len < 0 )
            {   
                TTY_DEBUG("read pcuifd failed\n");
    			return HSPA_ERROR;
            }
            TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
    	}
        else
        {
            break;
        }
    }

    for (i = 0; i < MAX_ATCMD_CNT; i++)
    {
        if ( (NULL == strstr(acInitHspa,"+CME ERROR"))&&(NULL == strstr(acInitHspa,"OK"))&& (NULL == strstr(acInitHspa,"ERROR")) )
    	{
    		memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
    		len = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
    		if ( len < 0 )
            {   
                TTY_DEBUG("read pcuifd failed\n");
    			return HSPA_ERROR;
            }
            TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
    	}
        else
        {
            break;
        }
    }

    
	/*判断是否是无卡出错*/
	if ( NULL != strstr(acInitHspa, "+CME ERROR: 13") ) /*SIM卡不在*/
	{
        TTY_DEBUG("No SIM, blink HSPA LED\n");

#if 0
        if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
        {
    		/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
    		//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
    		sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
    		/* END:   Modified by c106292, 2009/3/17 */
        }
#endif
        sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
		g_stHspa.sSimExist = HSPA_NO_SIM;
	}
	else
	{
		g_stHspa.sSimExist = HSPA_EXIST;
		cx_printf("%s, %d, %s(), pszInitHspa = %s\n",__FILE__, __LINE__, __FUNCTION__, acInitHspa);
		/*判断卡是否需要PIN码，或PUK码*/
		if (( NULL == strstr(acInitHspa, "ERROR")) && ( NULL != strstr(acInitHspa, "OK") )  ) /*不需要PIN校验*/
		{
			cx_printf("%s, %d, %s(), Don't need Pin!\n",__FILE__, __LINE__, __FUNCTION__);
			g_stHspa.sPinPukNeed = HSPA_PIN_PUK_OK;
		}		
		else
		if ( NULL != strstr(acInitHspa, "+CME ERROR: 11") ) /*要PIN校验*/
		{
			/*获取PIN码和数据卡IMEI号码*/
			memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
			memset(acPincode, 0, IFC_PIN_PASSWORD_LEN);
			/* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
			sysVariableGet(acPincode, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
			*/

			//sysAvailGet(acPincode, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);

			
			
			strcpy(acPincode, g_stHspa.acPinCode);
			
			/* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
			/* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
			sysVariableGet(acInitHspa, IFC_IMEI_LEN, IMEI_OFFSET);
			*/

			//sysAvailGet(acInitHspa, IFC_IMEI_LEN, IMEI_OFFSET);

			//strcpy(acInitHspa, g_stHspa.acOldIMEI);
			
			/* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            TTY_LOG("PIN CODE = %s\n IMEI in Flash = %s",acPincode, acInitHspa);

			if ( HSPA_ERROR == ImeiInfoGet() )
			{
			    TTY_DEBUG("ImeiInfoGet failed");
				return HSPA_ERROR;
			}

			/*获取数据卡号码*/
			if ( (NULL == strstr(g_stHspa.acOldIMEI, g_stHspa.acIMEI))  && (g_stHspa.acOldIMEI[0] != '\0'))
			{
				g_stHspa.sPinPukNeed = HSPA_PIN_NEED;
                TTY_DEBUG("Need PIN Code,blink HSPA LED\n");
#if 0
                if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
                {
					/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
					//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
					sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
                }
#endif
                sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);

			}
			else
			{
				/*更新共享文件中数据卡IMEI号码*/
			//	memcpy(g_stHspa.acIMEI, acInitHspa, HSPA_IMEI_LENGTH);
				/*下发AT指令验证PIN吗*/
				memset(acInitHspa, 0, sizeof(acInitHspa));
				sprintf(acInitHspa, "AT^CPIN=\"%s\"\r", acPincode);
				write(g_iPcuiFd, acInitHspa, strlen(acInitHspa));

				/*判断是否成功*/
				memset(acInitHspa, 0, sizeof(acInitHspa));
				len = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
				if ( len < 0 )
				{
				    TTY_DEBUG("read pcuifd failed\n");
					return HSPA_ERROR;
				}
				
                TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
				if ( NULL != strstr(acInitHspa, "ERROR") )
				{
					g_stHspa.sPinPukNeed = HSPA_PIN_NEED;
                    TTY_DEBUG("Need PIN, blink HSPA LED!");
                    sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);

#if 0
                    if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
                    {

					/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
						//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
					  sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
					/* END:   Modified by c106292, 2009/3/17 */
                    }
#endif
				}
				else
				{
				
                    for(i = 0; i < MAX_ATCMD_CNT; i++ )
                    {
    					if  (( NULL == strstr(acInitHspa, "OK") ) && ( NULL == strstr(acInitHspa, "ERROR") ))
    					{
    						memset(acInitHspa, 0, sizeof(acInitHspa));
    						len = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
    						if ( len < 0 )
    						{
    						    TTY_DEBUG("read pcuifd failed\n");
    							return HSPA_ERROR;
    						}
                            TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
    					}
                        else
                        {
                            break;
                        }
                    }
					if( NULL != strstr(acInitHspa, "OK") )
					{/*验证通过*/
						g_stHspa.sSimStatus = HSPA_VALID_SIM;
						g_stHspa.sPinPukNeed = HSPA_PIN_PUK_OK;
                        TTY_DEBUG("PIN Verificate OK!");
						/*更新老的IMEI信息*/
						strcpy(g_stHspa.acOldIMEI, g_stHspa.acIMEI);

                        /* stop blinking led when pin verified correct. <2009.1.5 tanyin> */
                        TTY_DEBUG("PIN verified, turn on HSPA LED!");
                        sysLedCtrl(kLedHspa, kLedStateOn);

#if 0
                        if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
                        {
                            TTY_DEBUG("PIN verified, turn on RED LED!");
			                sysLedCtrl(kLedHspa, kLedStateOn);
                        }
#endif
					}
					else
					{/*PIN码被改，需要用户输入PIN码*/
						g_stHspa.sPinPukNeed = HSPA_PIN_NEED;
                        TTY_DEBUG("Need PIN, blink HSPA LED!");
                        sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);

#if 0
                        if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
                        {
                            TTY_DEBUG("PIN is needed, fast blink RED LED!");
						/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
							//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
						  sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
						/* END:   Modified by c106292, 2009/3/17 */
                        }
#endif
					}				
				}
			}
		}
		else
		if ( NULL != strstr(acInitHspa, "+CME ERROR: 12")) /*PUK校验*/
		{/*需要用户输入PUK码*/
			g_stHspa.sPinPukNeed = HSPA_PUK_NEED;
			/*红灯*/

            TTY_DEBUG("PUK is needed, blink HSPA LED!");
            sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);

#if 0
	            if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
	            {
	                TTY_DEBUG("PUK is needed, turn on RED LED!");
        			/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
      				//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
       			    sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
	            }
#endif
		}
		else/**/
		{
		    TTY_DEBUG("System can't know whether PIN/PUK need or not! Error!!!!!\n");
            /* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） add begin. */
            iRet = HSPA_ERROR_OPERATION_REFUSED;
            /* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） add end. */
		}	
	}
	lseek(g_ShareiFd, 0,SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);

	/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin:*/
	if ( HSPA_OK == CheckHspaInfo(&g_stHspa))
	{
		sysWakeupMonitorTask();
        TTY_DEBUG("sysWakeupMonitorTask\n");
	}
    
	BackupHspaInfo(&g_stHspa);
	/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

	/* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） modify begin:
	return HSPA_OK;
	*/
	return iRet;
	/* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） modify end. */
	
}
/*****************************************************************************
 函 数 名  : AtRcv
 功能描述  : 业务进程
 输入参数  : int lMsgKey                     
             struct stAtRcvMsg *pstAtRcvMsg  
             int lMsgType                    
 输出参数  : 无
 返 回 值  : static inline int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
	memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
	return  msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, 0);
}
/*****************************************************************************
 函 数 名  : AtSend
 功能描述  : 
 输入参数  : int lMsgKey                     
             struct stAtSndMsg *pstAtSndMsg  
             char cFrom                      
             int lMsgType                    
             const char *pszParam            
 输出参数  : 无
 返 回 值  : static inline int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
{
	int flag = -1;
	memset(pstAtSndMsg, 0, sizeof(*pstAtSndMsg));	
	pstAtSndMsg->lMsgType = lMsgType;
	pstAtSndMsg->acParam[HSPA_FROM_POSITION] = cFrom;
	if(NULL!= pszParam)
		strcpy(pstAtSndMsg->acParam+1, pszParam);
	while ( flag < 0 )
	{
		flag = msgsnd(lMsgKey, (void*)pstAtSndMsg, AT_SND_MSG_LENGTH, IPC_NOWAIT);
		if ( flag < 0 )
		{
		    TTY_DEBUG("Snd msg failed\n");
			usleep(100);
		}
	}
	return flag;
}

void HspaSwtichDelay(void)
{
    int i = 0, success_flag = 0;
    int iRetryTimes = 0;
	struct stAtSndMsg stAtFromOther;

    g_HspaLine = g_HspaLine & HSPA_STATE_MASK;
    AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, CFUN_QUERY, "AT+CFUN?");
    AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, SYSTEM_INFO_QUERY, "AT^SYSINFO");
}
/*****************************************************************************
 函 数 名  : HspaDeviceSignalProcess
 功能描述  : 
		 信号处理函数处理两种情况:
		一、突然断开数据卡:
			1、读取USBMODEM文件判断设备在否。
			2、关闭PCUI口
			3、处理消息队列中没有处理的消息，告诉业务模块，现在设备断了。
			疑问:进程会不会阻塞在读写函数的地方。
		二、HG正常运行时候接入数据卡:
			1、读取USBMODEM文件判断设备在否。
			2、重新打开PCUI口；
			3、重新初始化PCUI口
			4、正常处理业务模块发送来的消息			

 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月10日
    作    者   : 曹祥
    修改内容   : 新生成函数
执行流程:
 3、打开"/proc/UsbModem"文件，创建共享文件，获取进程PID,
 	判断设备在否；在－>4，否－>5。
 4、打开PCUI口，设备存在标志置1；初始化串口属性，初始化HSPA设备
 	发送获取共享文件中参数的AT指令。
 5、设备存在标志置0；
 6、写入共享文件。PCUI口写使能置1。
*****************************************************************************/
void HspaDeviceSignalProcess(int x)
{
	  g_SignalRcv = 1;

	 
}

/*add by w00135351 20080916*/
int TR069_SendMessage(long msg_type, void *msg_content, int content_len)
{
    int msgid;
    int iKey;
    int iPid;
    S_TR069_MSG sMsg;
  
    if (content_len > TR069_MAX_CONTENT)
    {
        return -1;
    }
    /* start of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日*/
    FILE *pidfile;
    int  tr69c_pid = 0;
    pidfile = fopen("/var/tr69/tr69pid", "r");
    if (pidfile == NULL) 
    {
        return -1;
    }  
    else
    {
        if (fscanf(pidfile, "%d", &tr69c_pid) != 1) 
        {
           fclose(pidfile);
    	    return -1;
        }
        fclose(pidfile);
    }
    /* end of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日 */
    
    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    iPid = getpid();
    
    memset(&sMsg, 0, sizeof(S_TR069_MSG));
    sMsg.lMsgType = msg_type;
    if (msg_content != NULL)
    {
        memcpy(sMsg.achContent, msg_content, content_len);
    }
    if (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0))
    {
        printf("TR069_SendMessage send error  %d\n");
        return -1;
    }

    return 0;
}
/*end by w00135351 20080916*/

getCgmmInfo()
{
    char bufTem[COMMON_LONG_STRING_LENGTH] = {0};
    fd_set read_set;
    struct timeval tvSelect;
    tvSelect.tv_sec  = 5; // 最长等待10秒
    tvSelect.tv_usec = 0;
    int ret = 0;
    glbSpecStick = 0;
    int i = 0;
    
    write(g_iPcuiFd, "AT+CGMM\r", strlen("AT+CGMM\r"));
    
    while (1)
    {
        memset(bufTem, 0, sizeof(bufTem));
        FD_ZERO(&read_set);
        FD_SET(g_iPcuiFd, &read_set);
        ret = select(g_iPcuiFd + 1, &read_set, NULL, NULL, &tvSelect);
        if (ret > 0)
        {
            ret = read(g_iPcuiFd, bufTem, COMMON_LONG_STRING_LENGTH);
            if (0 < strlen(bufTem))
            {
                TTY_DEBUG("==%s:%s:%d:%s:VHG==\n",__FILE__,__FUNCTION__,__LINE__,bufTem);
                if ( strstr(bufTem,"OK")||strstr(bufTem,"CME ERROR") )
                {
                    if ( strstr(bufTem,glbStick))
                    {
                        TTY_DEBUG("Stick is %s",glbStick);
                        glbSpecStick = 1;
                    }
                    else
                    {
                        TTY_DEBUG("Stick is not %s",glbStick);
                        glbSpecStick = 0;
                    }
                    break;
                }
            }
            else if ( HSPA_NO == g_stHspa.sDeviceCount)
            {
                TTY_DEBUG("No HSPA Card\n");
                 break;
            }

        }
        else if ( HSPA_NO == g_stHspa.sDeviceCount)
        {
            TTY_DEBUG("No HSPA Card\n");
             break;
        }
        if ( i++ > 10 )
        {
            TTY_DEBUG("==%s:%s:%d:get the response of cgmm erro!==\n",__FILE__,__FUNCTION__,__LINE__);
            return;
        }
    }


}

void HspaDeviceDetect(void)
{
      /*Begin:Added by luokunling 00192527 for new requirement,IMSI report,2011/9/29*/
       FILE *ftr69cpid_file = NULL;
	pthread_t delaytimethread;
	int itr69c_pid = 0;
      /*End:Added by luokunling 00192527,2011/9/29*/
	struct termios stTerm;
	struct stAtSndMsg stAtFromOther;
	int lDeviceFd = -1;
	char acInitHspa[COMMON_LONG_STRING_LENGTH];
    char* pChar = NULL;
    char* pTmp = NULL;
	int fd;
	int lLen = 0;
    char CendCmd[64] = "";
    int lFlag = 0;
    FILE* fs = NULL;

    /*add by w00135351*/
    int iLen = 0;
    /*end by w00135351*/
    /*Begin:Added by luokunling 00192527 for new requirement,IMSI report,2011/9/29*/
     ftr69cpid_file = fopen(TR69_PID_FILE,"r");
     if(NULL == ftr69cpid_file)
     {
          g_iDelaytime_flg = 1;
     }
     else
     {
         fclose(ftr69cpid_file);
     }     
    /*End:Added by luokunling 00192527,2011/9/29*/
    fs = fopen(HSPA_SYSCFG_FILE, "r");
    if (!fs)
    {
        return;
    }

    fclose(fs);
    fs = NULL;
    
	cx_printf( "Signal process function begin!\n" );		 	
    TTY_LOG("Signal process function begin!\n");
	/*打开"/proc/UsbModem"文件*/
	while ( lDeviceFd<0 )
	{
	    sleep(3);
		lDeviceFd = open("/proc/UsbModem",O_RDONLY);
	}
	memset(acInitHspa, 0, sizeof(acInitHspa));
	/* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 
	   for 用宏替换*/
	read(lDeviceFd,acInitHspa, HSPA_MAX_LINE_LEN);/*"Usb-Serial Port Num:4\r\n"*/
	/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 */
	close(lDeviceFd);

	/*临时解决两种数据卡适配问题*/
	pChar = strstr(acInitHspa, ":");
	cx_printf("%s \n", acInitHspa);

	if (NULL == pChar)
	{
        TTY_LOG("HSPA device count parse Error!\n");
	    return;
	}

	/*判断设备在否*/
	if ( atoi(pChar+1) > 1 )
	{/*将设备存在标志设为1，将PCUI口打开*/
        TTY_LOG("HSPA device exist!\n");
		//sysLedCtrl(kLedHspa, kLedStateOff);
		    
            if(g_HspaExistFlag == HSPA_EXIST)/*之前已经检测过了HSPA数据卡存在*/
            {
                   
            	   pTmp = strstr(pChar+1, ":");
                
		   if (!pTmp)
		   {
		   	  return;
		   }

		   if (atoi(pTmp+1) == 0)
	   	   {
	   	   	 g_SignalRcv = 1;
	   	   	 goto OpenTtyChange;
		   }
                return;
            }

            g_iPcuiFd = -1;

		while( g_iPcuiFd < 0 )
		{
			g_stHspa.sDeviceCount = 3;
			g_iPcuiFd = open("/dev/ttyUSB2",O_RDWR);
			if (g_iPcuiFd < 0)
			{			    
                TTY_DEBUG("TRYING to Open ttyUSB1 \n");
			    g_stHspa.sDeviceCount = 2;
			    g_iPcuiFd = open("/dev/ttyUSB1",O_RDWR);
			}
			if ( g_iPcuiFd < 0 )
			{
			    TTY_DEBUG("Open ttyUsb failed");
				goto OpenTtyChange;
			}
		}

        /*更新共享数据*/
		g_stHspa.sDeviceStatus = HSPA_UNKNOW;
		g_stHspa.sSimExist = HSPA_UNKNOW;
		g_stHspa.sSimStatus = HSPA_UNKNOW;
		g_stHspa.sSysMode = HSPA_UNKNOW;
		g_stHspa.sSubSysMode = HSPA_UNKNOW;
		g_stHspa.sSrvStatus = HSPA_UNKNOW;
		g_stHspa.sPinPukNeed = HSPA_UNKNOW;
		memset(g_stHspa.acIMEI, 0, HSPA_IMEI_LENGTH);
		memset(&g_stHspa.stDataFlowInfo, 0, sizeof(g_stHspa.stDataFlowInfo));

		/*w00135358 初始化全局变量里保存的pin码，若以后支持语音此处还需修改*/
   	    memset(g_stHspa.acPinCode, 0, HSPA_IMEI_LENGTH);
	
		/*更新共享文件*/
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);

		/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin:*/
		BackupHspaInfo(&g_stHspa);
		/* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

		/*初始化PCUI口*/
		if ( tcgetattr(g_iPcuiFd, &stTerm) < 0 )
		{
		    TTY_DEBUG("Get Usb-Serial port's parameter failure!\n");
			goto OpenTtyChange;
		}
		stTerm.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
		stTerm.c_iflag &= ~( INLCR | ICRNL | IGNCR );
		stTerm.c_iflag &= ~( IXON | IXOFF | IXANY );
		stTerm.c_oflag  = 0;
/*S48571 ttyUSB 阻塞问题 */
        stTerm.c_cc[VTIME] = 50; /* 设置超时5 seconds*/ 
        stTerm.c_cc[VMIN] = 0; /* Update the options and do it NOW */


		if ( tcsetattr(g_iPcuiFd, TCSAFLUSH, &stTerm) < 0 )
		{
		    TTY_DEBUG("Set Usb-Serial port's parameter failure!\n");
			goto OpenTtyChange;
		}

        if( HSPA_OFF == (g_HspaLine & HSPA_STATE_MASK))
        {
            lFlag = 1;
        }

		lLen = write(g_iPcuiFd, "AT+CFUN=1\r", strlen("AT+CFUN=1\r"));
		usleep(300);
		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
        cx_printf("%s,%d----%s\n",__FILE__, __LINE__,acInitHspa);
        //s48571 modified for the wrong led state when plug in a dongle.
        //g_HspaLine = HSPA_ON;

		/*初始化HSPA设备*/
		lLen = write(g_iPcuiFd, "AT^CURC=0\r", strlen("AT^CURC=0\r"));
		usleep(100);
		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
        /* HUAWEI HGW s48571 2008年8月16日 语音关闭需求  add begin:*/
        VoiceJudge();
        /* HUAWEI HGW s48571 2008年8月16日 语音关闭需求  add end.*/
		lLen = write(g_iPcuiFd, "AT+CMEE=1\r", strlen("AT+CMEE=1\r"));
		usleep(300);
		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);

        /* turn on HSPA led */
        sysLedCtrl(kLedHspa, kLedStateOn);

#if 0 //按照VDF要求，打开数据卡的指示灯
		lLen = write(g_iPcuiFd, "AT^LIGHT=0\r", strlen("AT^LIGHT=0\r"));
		usleep(300);
		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
#endif
/* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） modify begin:*/

        int lPinCounter = 0;

        for( lPinCounter = 0; lPinCounter < MAX_PIN_TEST; lPinCounter++)
        {

		/*************检验是否需要PIN校验,并处理*************/
		if (strlen(g_stHspa.acPinCode) < 4)
		{
			sysAvailGet(g_stHspa.acPinCode, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
		}
		
   		sysAvailGet(g_stHspa.acOldIMEI, IFC_IMEI_LEN, IMEI_OFFSET);
   		lLen = strlen(g_stHspa.acOldIMEI);

		cx_printf("%s, %d, acOldIMEI in Flash = %s\n",__FILE__, __LINE__, g_stHspa.acOldIMEI);
		
   		while(lLen > 0)
   		{
   			if (isdigit(g_stHspa.acOldIMEI[lLen-1]))
   			{
   				lLen--;
   			}
   			else
   			{
   				memset(g_stHspa.acOldIMEI, 0, sizeof(g_stHspa.acOldIMEI));
   				break;
   			}
   		}

    		
    	
    		lLen = write(g_iPcuiFd, "AT+CLCK=\"SC\",2\r", strlen("AT+CLCK=\"SC\",2\r"));
    		memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
    		usleep(500);
    		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
            TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
            
    		if ( lLen < 0 )
    		{
    		    TTY_DEBUG("len =%d\n",lLen);
    			cx_printf("%s,%d----lLen = %d\n",__FILE__, __LINE__, lLen);
    			goto OpenTtyChange;
    		}
            
    		if  (HSPA_OK != PinPukProcess(acInitHspa) )
    		{
    		    TTY_DEBUG("HSPA_OK != PinPukProcess(acInitHspa)\n");
                if ( lPinCounter == (MAX_PIN_TEST - 1))
                {
                    goto OpenTtyChange;
                }
                sleep(5);
    		}
            else
            {
                break;
            }
        }
        /* HUAWEI HGW s48571 2008年6月28日 和数据卡E160对接问题（初始化时间较长，不能反馈是否需要PIN码） modify end. */

        if ( HSPA_PIN_PUK_OK == g_stHspa.sPinPukNeed )
        {
            /*Begin:Added by luokunling 00192527 for imsi inform requirement,2011/9/19*/
            g_iHspaWeatherAutoPin = 1;
	     /*Begin:Added by luokunling 00192527 for imsi inform requirement*/	
            lLen = write(g_iPcuiFd, "AT+CLIP=1\r", strlen("AT+CLIP=1\r"));
            memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
            usleep(500);
            lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
            
            if ( NULL == strstr(acInitHspa, "OK") )
            {
                cx_printf("%s,%d----CLIP set failed.\n",__FILE__, __LINE__);
            }
            else
            {
                cx_printf("%s,%d----CLIP set ok.\n",__FILE__, __LINE__);
            }
        }
  
		/*****************************************************************************/
		g_HspaExistFlag = HSPA_EXIST;

        /*add by w00135351 20080918*/
            //iLen = sizeof(TR069_CONFIG_TTYUSBFO);
        printf("%s %d the g_iHspaWeatherAutoPin is :%d#########\n",__FILE__, __LINE__,g_iHspaWeatherAutoPin);
        if(0 == g_iHspaWeatherAutoPin)
        {
            TR069_SendMessage(TTYUSB_INFORM_TR069,NULL,0);
        }
        /*end by w00135351 20080918*/
        getCgmmInfo();

        /*读取hspa配置，设置注册网络类型，恢复数据卡正常模式*/    
        RegisterNtwk();
        
        if (lFlag)
        {
            /*modified by w00135351 20081231 避免E180在低功耗下掉电*/
            if ( glbSpecStick )
            {
                lLen = write(g_iPcuiFd, "AT+CFUN=7\r", strlen("AT+CFUN=7\r"));            
            }
            else
            {
                lLen = write(g_iPcuiFd, "AT+CFUN=0\r", strlen("AT+CFUN=0\r"));            
            }
            usleep(500);
            lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
            cx_printf("%s,%d----%s\n", __FILE__, __LINE__, acInitHspa);
            sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
            //lDeviceFd = AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, CFUN_SET, "AT+CFUN=0");
            //s48571 modified for the wrong led state when plug in a dongle.            
            //g_HspaLine = HSPA_OFF;
        }
        
		lDeviceFd = AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, CFUN_QUERY, "AT+CFUN?");
		cx_printf("%s,%d----SEND1---%d\n",__FILE__, __LINE__, lDeviceFd);
		lDeviceFd = AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, PIN_ENABLE_QUERY, "AT+CLCK=\"SC\",2");
		cx_printf("%s,%d----SEND2---%d\n",__FILE__, __LINE__, lDeviceFd);
        sleep(5);// s48571 add to prevent short red led perion after pluging into a dongle, 2008.06.21
		lDeviceFd = AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, SYSTEM_INFO_QUERY, "AT^SYSINFO");
		cx_printf("%s,%d----SEND3---%d\n",__FILE__, __LINE__, lDeviceFd);  
		
		/*Begin:Added by luokunling 00192527 for new requirement,IMSI report,2011/9/16*/	
		if(0 == g_iDelaytime_flg)
		{
		    usleep(500);
		    lDeviceFd = AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, IMSI_QUERY, "AT+CIMI");
	           usleep(500);
		}
		else
		{
		   while(0 != pthread_create(&delaytimethread,NULL,DelayTimeInformIMSI,NULL))
		   {
		        ;
		   }
		}
	      /*End:Added by luokunling 00192527 for new requirement.*/
		
	}
	else/*设备不在了*/
	{/*将设备存在标志设为0，
		将PCUI口文件标志关闭*/
OpenTtyChange:
        //通知hspa语音
        strcpy(CendCmd,"\r\n^CEND:1,0,255,255\r\n");
        EndCallReport(CendCmd);
        
        TTY_DEBUG("There isn't any HSPA device!\n");
        if(g_HspaExistFlag == HSPA_NO)
        {
            cx_printf("%s, %d, %s() HSPA device already pulled out! return! \n", __FILE__, __LINE__, __FUNCTION__);
            return;
        }

        /*关灯*/
		sysLedCtrl(kLedHspa, kLedStateOff);

		close(g_iPcuiFd);
		g_iPcuiFd = -1;
		/*更新共享数据*/
		g_stHspa.sDeviceCount = 0;
		g_stHspa.sDeviceStatus = HSPA_UNKNOW;
		g_stHspa.sSimExist = HSPA_UNKNOW;
		g_stHspa.sSimStatus = HSPA_UNKNOW;
		g_stHspa.sSysMode = HSPA_UNKNOW;
		g_stHspa.sSubSysMode = HSPA_UNKNOW;
		g_stHspa.sSrvStatus = HSPA_UNKNOW;
		g_stHspa.sPinPukNeed = HSPA_UNKNOW;
		
		memset(g_stHspa.acIMEI, 0, HSPA_IMSI_LENGTH);
		memset(&g_stHspa.stDataFlowInfo, 0, sizeof(g_stHspa.stDataFlowInfo));
		/*更新共享文件*/
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);	

		g_HspaExistFlag = HSPA_NO;
        
        /*add by w00135351 20080918 发送消息到TR069进程*/	
	     g_iHspaWeatherAutoPin = 0;
        //TR069_SendMessage(TTYUSB_INFORM_TR069,NULL,0);
       TR069_SendMessage( TTYUSB_PLUGOUT_INFORM_TR069,NULL,0);
        /*end by w00135351 20080918*/
        
		//通知wan监控程序
		if( (fd = open( "/dev/brcmboard", O_RDWR )) != -1 )
		{
			ioctl( fd, BOARD_IOCTL_WAKEUP_MONITOR_TASK, NULL);
			close(fd);
		}

            FILE *fp_pppou = NULL;
            char acCmd[10];
            int pppoupid = 0;
            memset(acCmd, 0x00, 10);
            if(NULL != (fp_pppou = fopen("/var/pppoupid", "r")))
            {
                fgets(acCmd, 10, fp_pppou);
                fclose(fp_pppou);
                pppoupid = atoi(acCmd);
            }
                
            if( pppoupid > 0)
            {
                kill(pppoupid, SIGHUP);
            }
	}
    sleep(2);
    sysWakeupMonitorTask();
    TTY_LOG("Signal process function is over!\n");
	return;
	
}

/*****************************************************************************
 函 数 名  : SendSysinfo
 功能描述  : 当数据卡存在并不是LPM时,每隔10s发送指令AT^SYSINFO,查询系统信息
 输入参数  : void* arg  
 输出参数  : 无
 返 回 值  : static void*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月9日
    作    者   : pengqiuming(48166)
    修改内容   : 新生成函数

*****************************************************************************/

static void* SendSysinfo(void* arg)
{
	struct stAtSndMsg stAtFromOther;
	memset(&stAtFromOther, 0, sizeof(stAtFromOther));
	
	while(1)
	{
		sleep(15);
		if ((HSPA_EXIST == g_HspaExistFlag) && ((g_HspaLine & HSPA_LINE_MASK) == HSPA_ON))
		{
			AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, SYSTEM_INFO_QUERY, "AT^SYSINFO");
		}
	}
}

/*****************************************************************************
 函 数 名  : WritePcui
 功能描述  : 
 输入参数  : void* arg  
 输出参数  : 无
 返 回 值  : static void*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void*  WritePcui(void* arg)
{
	int wflag = 0;
	int lLen =0;
	static int iErrNum = 0;

	
    signal(SIGALRM,HspaSwtichDelay);
	while(1)
	{
		sem_wait(&sem);
        sem.__sem_value = 0;
		memset(&g_stAtFromOther, 0, sizeof(g_stAtFromOther));
#if 0
cx_printf("sDeviceCount = %d\n", g_stHspa.sDeviceCount);
cx_printf("sDeviceStatus = %d\n", g_stHspa.sDeviceStatus);
cx_printf("sSimExist = %d\n", g_stHspa.sSimExist);
cx_printf("sSimStatus = %d\n", g_stHspa.sSimStatus);
cx_printf("sSysMode = %d\n", g_stHspa.sSysMode);
cx_printf("sSubSysMode = %d\n", g_stHspa.sSubSysMode);
cx_printf("sSrvStatus = %d\n", g_stHspa.sSrvStatus);
cx_printf("sPinPukNeed = %d\n", g_stHspa.sPinPukNeed);
cx_printf("acIMEI = %s\n", g_stHspa.acIMEI);	
#endif
		/*从lAtMsgId中收取业务进程发来的命令消息到stAtSnd
		wflag存取接收消息是否成功标志*/
        while (-1 == ( wflag = msgrcv(g_lAtMsgId, (void*)&g_stAtFromOther, AT_SND_MSG_LENGTH, 0, 0)))
        {      
            printf("%s,%d,%s(),msgrcv is interrupted, try again\n", __FILE__,  __LINE__, __FUNCTION__);
        }
                 if ( PDP_SET != g_stAtFromOther.lMsgType)
                 {
                            cx_printf("%s,%d,%s(),AT =%d, %s, sem value is %d\n", __FILE__,  __LINE__, __FUNCTION__, g_stAtFromOther.lMsgType, g_stAtFromOther.acParam+1, sem.__sem_value);
		            TTY_LOG("%s,%d,%s(),AT =%d, %s, sem value is %d\n", __FILE__,  __LINE__, __FUNCTION__, g_stAtFromOther.lMsgType, g_stAtFromOther.acParam+1, sem.__sem_value);
                 }
		/*根据wflag判断释放收到了命令消息，
		如果成功将指令写入HSPA，否则跳过，
		直接读取HSPA反馈的消息*/
		if ( -1 != wflag )
		{
			/*对互斥量加锁,保证在此期间不被read打断*/
			pthread_mutex_lock(&work_mutex);
	
			strcat(g_stAtFromOther.acParam+HSPA_OFFSET, "\r");
			lLen = strlen(g_stAtFromOther.acParam+HSPA_OFFSET);
			cx_printf("%s, %d, %s(),len = %d\n", __FILE__, __LINE__, __FUNCTION__,  lLen);
            if ( CFUN_SET == g_stAtFromOther.lMsgType )
            {
                /*modified by w00135351 20081231 避免180在低功耗下掉电*/
                //if( NULL != strstr(g_stAtFromOther.acParam, "AT+CFUN=0"))
                if( NULL != strstr(g_stAtFromOther.acParam, "AT+CFUN=0"))
                {
                    if ( glbSpecStick)
                    {
                        g_stAtFromOther.acParam[9] = '7';
                    }
                    TTY_DEBUG("hspa off!");
                    TTY_DEBUG("\nCFUN FROM %d\n",g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
                    g_HspaLine = HSPA_OFF|MID_STATE_MASK;
                    alarm(10);
                }
                else if( NULL != strstr(g_stAtFromOther.acParam, "AT+CFUN=1"))
                {
                    TTY_DEBUG("hspa on!");
                    TTY_DEBUG("\nCFUN FROM %d\n",g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
                    g_HspaLine = HSPA_ON|MID_STATE_MASK;
                    alarm(10);
                }
            }
			/*将命令消息stAtSnd写入HSPA*/
			if( HSPA_EXIST == g_HspaExistFlag )/*设备在的时候才能进行写操作*/
			{		
				wflag = write(g_iPcuiFd, g_stAtFromOther.acParam+HSPA_OFFSET,	lLen);		
				if( lLen == wflag )
				{
					g_iWriteFlag = HSPA_WRITE_OK;
					iErrNum = 0;
				}
				else
				{
					g_iWriteFlag = HSPA_WRITE_NO;
					if (-1 == wflag)
					{

						iErrNum++;
						if (iErrNum > HSPA_ERR_LIMIT)
						{
							iErrNum = 0;
							g_SignalRcv = 1;
						}
					}
					
				}
				cx_printf("%s, %d, g_stAtFromOther.lMsgType = %d, g_iWriteFlag = %d\n", __FILE__,  __LINE__,g_stAtFromOther.lMsgType,g_iWriteFlag );
				cx_printf("%s, %d, wflag = %d\n", __FILE__, __LINE__,wflag);
			}
			else
			{
				g_iWriteFlag = HSPA_WRITE_NO;	
				cx_printf("%s, %d, g_stAtFromOther.lMsgType = %d\n", __FILE__, __LINE__, g_stAtFromOther.lMsgType);
			}

            if ( PDP_SET != g_stAtFromOther.lMsgType)
            {
                    TTY_DEBUG("g_stAtFromOther.lMsgType = %d, g_iWriteFlag = %d, wflag = %d\n", g_stAtFromOther.lMsgType, g_iWriteFlag,wflag);
            }
			/*对互斥量解锁*/
			pthread_mutex_unlock(&work_mutex);

		}	

	}
	msgctl(g_lAtMsgId, IPC_RMID,0);
    #ifdef CX_DEBUG_VERSION	
	msgctl(g_lCmdMsgId, IPC_RMID,0);
    #endif
	msgctl(g_lModemMsgId, IPC_RMID,0);
	msgctl(g_lVoiceMsgId, IPC_RMID,0);
	msgctl(g_lTr069MsgId, IPC_RMID,0);
	msgctl(g_lHttpMsgId, IPC_RMID,0);
	msgctl(g_lLedMsgId, IPC_RMID,0);
#ifdef POTS_PINMNGR    
	msgctl(g_lPinMngrMsgId, IPC_RMID,0);
#endif    
	cx_printf("%s, %d, %s()\n", __FILE__, __LINE__, __FUNCTION__);
	close(g_iPcuiFd);	
	close(g_ShareiFd);
	exit(0);
}
/*****************************************************************************
 函 数 名  : ReadPcui
 功能描述  : 
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static int   ReadPcui()
{
	char acInfoBack[AT_RCV_MSG_LENGTH];
	char acInfoBackAnalysis[AT_RCV_MSG_LENGTH];
	char *pChar = NULL;
	int lPairNum = sizeof(g_stAtFunCom)/(sizeof(struct stAt_Fun_Com));/*结构体数组中元素个数*/
	int i = 0;
	int lMsgId = 0;
	int len = 0;

	static int iErrReadNum = 0;

	
	/*读取USB-SERIAL发送消息*/
	while(1)
	{
//        cx_printf("%s, %d\n", __FILE__, __LINE__);

         memset(&g_stAtToOther, 0, sizeof(g_stAtToOther));
         g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;

         /*对互斥量加锁*/
         pthread_mutex_lock(&work_mutex);

	  if (iErrReadNum > HSPA_ERR_LIMIT)
	  {
			g_SignalRcv = 1;
			iErrReadNum = 0;
	  }

		 
         if( 1 == g_SignalRcv)
         {
             g_SignalRcv = 0;
             TTY_DEBUG("connect/disconnect hspa CARD \n");
             HspaDeviceDetect();
        }
         
		/*如果没有写入数据卡，直接将该消息反馈，告诉
		业务当时没有设备*/
		if  ( (HSPA_WRITE_NO  == g_iWriteFlag) && ( 0 != g_stAtFromOther.acParam[HSPA_FROM_POSITION]) ) 
		{
			g_iWriteFlag = HSPA_WRITE_UNKNOW;
            /*对互斥量解锁*/
            pthread_mutex_unlock(&work_mutex);
			lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
			g_stAtToOther.lMsgType = g_stAtFromOther.lMsgType;
			g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
			g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_NO_E172;
			if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
			{
                TTY_DEBUG("lMsgId is %d\n", lMsgId);
                msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
			}
            g_sndflg = 0;
			sem_post(&sem);
            TTY_DEBUG("lMsgId is %d && msg is sent\n", lMsgId);
			continue;
		}
		/*	1.设备不在不能读
			2.没有收到消息不能读*/
	//	if (( HSPA_NO == g_HspaExistFlag ) || (HSPA_WRITE_UNKNOW == g_iWriteFlag) ) 
		if ( HSPA_NO == g_HspaExistFlag ) 
		{
			usleep(500);
            if  ( (HSPA_WRITE_OK == g_iWriteFlag) && ( 0 != g_stAtFromOther.acParam[HSPA_FROM_POSITION])) 
            {
                g_iWriteFlag = HSPA_WRITE_UNKNOW;
                /*对互斥量解锁*/
                pthread_mutex_unlock(&work_mutex);
                lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
                g_stAtToOther.lMsgType = g_stAtFromOther.lMsgType;
                g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
                g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_NO_E172;
                if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
                {
                    TTY_DEBUG("lMsgId is %d\n", lMsgId);
                    msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
                }
                g_sndflg = 0;
                sem_post(&sem);
                TTY_DEBUG("lMsgId is %d && msg is sent\n", lMsgId);
                continue;
            }
            
            /*对互斥量解锁*/
            pthread_mutex_unlock(&work_mutex);
			continue;
		}
#if 0		
cx_printf("sDeviceCount = %d\n", g_stHspa.sDeviceCount);
cx_printf("sDeviceStatus = %d\n", g_stHspa.sDeviceStatus);
cx_printf("sSimExist = %d\n", g_stHspa.sSimExist);
cx_printf("sSimStatus = %d\n", g_stHspa.sSimStatus);
cx_printf("sSysMode = %d\n", g_stHspa.sSysMode);
cx_printf("sSubSysMode = %d\n", g_stHspa.sSubSysMode);
cx_printf("sSrvStatus = %d\n", g_stHspa.sSrvStatus);
cx_printf("sPinPukNeed = %d\n", g_stHspa.sPinPukNeed);
cx_printf("acIMEI = %s\n", g_stHspa.acIMEI);
#endif
        /*对互斥量解锁*/
        pthread_mutex_unlock(&work_mutex);

		memset(acInfoBack, 0, AT_RCV_MSG_LENGTH);
		len = read(g_iPcuiFd, acInfoBack, AT_RCV_MSG_LENGTH); //从HSPA获得回应信息
		if ( PDP_SET != g_stAtFromOther.lMsgType)
	        {
		TTY_LOG("%s, %d, %s, len = %d\n", __FILE__, __LINE__, acInfoBack,len);
		}
		
		/*处理异常:命令写入了设备，但是在读取前，设备移除
			读取失败*/
        //if( ( len < 0 ) ||( len == 0 )) 
        if(( len < 0 )||(( len == 0 )&&( 1 == g_sndflg )))
        {
		iErrReadNum++;
				
            if ((0 != g_stAtFromOther.acParam[HSPA_FROM_POSITION]))
            {
                pthread_mutex_lock(&work_mutex);
        		if  ( HSPA_WRITE_OK  == g_iWriteFlag  )
        		{
        			g_iWriteFlag = HSPA_WRITE_UNKNOW;
                    /*对互斥量解锁*/
                    pthread_mutex_unlock(&work_mutex);
        			lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
        			g_stAtToOther.lMsgType = g_stAtFromOther.lMsgType;
        			g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
        			g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_NO_E172;
        			if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
        			{
                        TTY_DEBUG("lMsgId is %d\n", lMsgId);
                        msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
        			}
                    g_sndflg = 0;
        			sem_post(&sem);
                    TTY_DEBUG("lMsgId is %d && msg is sent\n", lMsgId);
        			continue;
        		}
                TTY_DEBUG("len=%d, g_sndflg=%d\n", len, g_sndflg);
                /*对互斥量解锁*/
                pthread_mutex_unlock(&work_mutex);
            }
            continue;            
        }
        else if(( len == 0 )&&( 0 == g_sndflg ))
        {

		iErrReadNum++;
            if ((0 != g_stAtFromOther.acParam[HSPA_FROM_POSITION]))
            {
                g_sndflg = 1;  /*modified by w00135351 09.4.1*/
                TTY_DEBUG("set g_sndflg...\n");
            }
        }
	  else 
  	 {
  	 	iErrReadNum = 0;
  	 }

	   if ( PDP_SET != g_stAtFromOther.lMsgType)
	   {
        
        TTY_LOG("TTYUSB: {%s}\n",acInfoBack);
		/*^BOOT自动上报信息不处理*/		
		//if ( NULL != strstr(acInfoBack, "BOOT") )		
		//	continue;
		
		cx_printf("%s, %d, %s(),len = %d ,acInfoBack= %s\n", __FILE__, __LINE__, __FUNCTION__, len, acInfoBack);
	   }
		for ( i=0;  i<lPairNum;  ++i)
		{
			pChar = strstr(acInfoBack, g_stAtFunCom[i].acAtCmdPart);
			if ( NULL == pChar )
			{
				continue;
			}
			else
			{	
				memset (acInfoBackAnalysis, 0, AT_RCV_MSG_LENGTH);
				strcpy(acInfoBackAnalysis, pChar);
				g_stAtFunCom[i].pfAtBackInfoAnalysis(acInfoBackAnalysis);
			}
		}
	}
}

/*****************************************************************************
 函 数 名  : MsgIdCompute
 功能描述  : 根据消息来自的模块，计算消息发往的 MsgId
 输入参数  : unsigned char ucCmdFrom  
 输出参数  : 无
 返 回 值  : static inline int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static  int MsgIdCompute(unsigned char ucCmdFrom)
{
	if ( LED_MODULE == ucCmdFrom )
		return g_lLedMsgId;
	else
	if ( HTTP_MODULE == ucCmdFrom )
		return g_lHttpMsgId;
	else
	if ( TR069_MODULE == ucCmdFrom )
		return g_lTr069MsgId;
	else
	if ( MODEM_MODULE == ucCmdFrom )
		return g_lModemMsgId;
	else
	if ( VOICE_MODULE == ucCmdFrom )
		return g_lVoiceMsgId;
#ifdef CX_DEBUG_VERSION
	else
	if ( CMD_MODULE == ucCmdFrom )
		return g_lCmdMsgId;
#endif
#ifdef POTS_PINMNGR
	else
	if ( PINMNGR_MODULE	== ucCmdFrom )
		return g_lPinMngrMsgId;
#endif    
	else
		return -1;
}


/*****************************************************************************
 函 数 名  : StrStrToUnChar
 功能描述  : 将acCharParam中的第一个数字字符串解析为数值
 输入参数  : char **pChar       
             char *acCharParam  
 输出参数  : 无
 返 回 值  : static unsigned char
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月14日
    作    者   : 曹祥
    修改内容   : 新生成函数
  2.日    期   : 2008年2月2日
    作    者   : s48571
    修改内容   : 错误处理，防止字符串中没有数字，导致字符越界

*****************************************************************************/
static unsigned char StrStrToUnChar(char **pChar, char *acCharParam)
{
	int i = 0;
	int iLen = strlen(*pChar);
	
	while(1)
	{
		++*pChar;
		if ( isdigit(**pChar) ||(' ' == **pChar) )
		{
			acCharParam[i] = **pChar;
		}
		else
		{
			acCharParam[i] = 0;
			break;
		}
		++i;
		if ( i == iLen)
			break;
	}
	
	if (( 0 == i )||( iLen == i ))
	{
		return HSPA_UNKNOW_VALUE;
	}
	return ((unsigned char) atoi(acCharParam));
}
//add by z67625 20080330 for cend, start
/*****************************************************************************
 函 数 名  : StrStrToInt
 功能描述  : 将acCharParam中的第一个数字字符串解析为数值
 输入参数  : char **pChar       
             char *acCharParam  
 输出参数  : 无
 返 回 值  : static unsigned char
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月14日
    作    者   : 曹祥
    修改内容   : 新生成函数
  2.日    期   : 2008年2月2日
    作    者   : s48571
    修改内容   : 错误处理，防止字符串中没有数字，导致字符越界

*****************************************************************************/
static int StrStrToInt(char **pChar, char *acCharParam)
{
	int i = 0;
	int iLen = strlen(*pChar);
	
	while(1)
	{
		++*pChar;
		if ( isdigit(**pChar) ||(' ' == **pChar) )
		{
			acCharParam[i] = **pChar;
		}
		else
		{
			acCharParam[i] = 0;
			break;
		}
		++i;
		if ( i == iLen)
			break;
	}
	
	if (( 0 == i )||( iLen == i ))
	{
		return HSPA_UNKNOW_VALUE;
	}
	return atoi(acCharParam);
}
//add by z67625 20080330 for cend, end
/*****************************************************************************
 函 数 名  : FacInfoAnlysis
 功能描述  : 完成指令AT+GMI\r反馈信息的解析与分发
 输入参数  : char *pszInfoBack －反馈信息字符串 
 输出参数  : 无
 返 回 值  : static void
指令和反馈信息格式:
*FACTORY_INFO_QUERY	17----AT+GMI\r
*NORMAL:7----AT+GMI<CR>, 16----<CR><LF>huawei<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:7----AT+GMI<CR>, 5----<CR><LF>ERROR<CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void FacInfoAnlysis(char *pszInfoBack) 
{
	struct stMsg_Factory_Info_Query stFacInfo;/*反馈信息对应的结构体*/
	int lMsgId = 0;
	int lLen = 0;
	memset(&stFacInfo, 0, sizeof(stFacInfo));
	g_stAtToOther.lMsgType = FACTORY_INFO_QUERY;/*设置消息对应的命令*/
	stFacInfo.ucCmdFrom = AT_MODULE;	/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack) && (NULL!=strstr(pszInfoBack, "\r\nOK\r\n")) )
	{
		lLen = strlen(pszInfoBack);
		stFacInfo.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stFacInfo.acFactoryInfo) )
		{
			lLen = sizeof(stFacInfo.acFactoryInfo) -1;
		}
		memcpy(stFacInfo.acFactoryInfo, pszInfoBack+strlen("\r\n"), lLen);
	}	
	else
	{
		stFacInfo.ucResult = HSPA_ERROR_UNKNOW;
	}
	
	memcpy(g_stAtToOther.acText, (void*)&stFacInfo, sizeof(stFacInfo));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);

	cx_printf("%s,%d,%s(),acFactoryInfo = %s\n", __FILE__,  __LINE__, __FUNCTION__,  stFacInfo.acFactoryInfo);
	return;
}
/*****************************************************************************
 函 数 名  : FacInfoAnlysis_C
 功能描述  : 完成指令AT+CGMI\r反馈信息的解析与分发
 输入参数  : char *pszInfoBack －反馈信息字符串 
 输出参数  : 无
 返 回 值  : static void
指令和反馈信息格式:
*FACTORY_INFO_QUERY_C	18----AT+CGMI\r
*NORMAL:8----AT+CGMI<CR>, 16----<CR><LF>huawei<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:8----AT+CGMI<CR>, 5----<CR><LF>ERROR<CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void FacInfoAnlysis_C(char *pszInfoBack) 
{
	struct stMsg_Factory_Info_Query stFacInfo;/*反馈信息对应的结构体*/
	int lMsgId;
	int lLen;
	memset(&stFacInfo, 0, sizeof(stFacInfo));
	g_stAtToOther.lMsgType = FACTORY_INFO_QUERY_C;/*设置消息对应的命令*/
	stFacInfo.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
        
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stFacInfo.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stFacInfo.acFactoryInfo) )
		{
			lLen = sizeof(stFacInfo.acFactoryInfo) -1;
		}
		memcpy(stFacInfo.acFactoryInfo, pszInfoBack+strlen("\r\n"), lLen);
	}	
	else
	{
		stFacInfo.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stFacInfo, sizeof(stFacInfo));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	cx_printf("%s, %d, %s(), lMsgId = %dn",__FILE__, __LINE__, __FUNCTION__, lMsgId);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s, %d, %s(), acFactoryInfo = %s\n",__FILE__, __LINE__, __FUNCTION__, stFacInfo.acFactoryInfo);
	return;
}

/*****************************************************************************
 函 数 名  : SoftwareVersionAnlysis
 功能描述  : 完成AT+GMR\r的反馈信息解析和分发
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
指令和反馈信息格式:
*SOFTWARE_VERSION_QUERY	19----AT+GMR\r
*NORMAL:7----AT+GMR<CR>,25----<CR><LF>11.310.09.00.00<CR><LF><CR><LF>OK<CF><LF>
*UNNORMAL:7----AT+GMR<CR>,9----<CR><LF>ERROR<CR><LF>
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void SoftwareVersionAnlysis(char *pszInfoBack) 
{
	struct stMsg_Software_Version_Query stSoftwareVer;
	int lMsgId;
	int lLen;
	memset(&stSoftwareVer, 0, sizeof(stSoftwareVer));
	g_stAtToOther.lMsgType = SOFTWARE_VERSION_QUERY;/*设置消息对应的命令*/
	stSoftwareVer.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
            
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack) && (NULL!=strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stSoftwareVer.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stSoftwareVer.acSoftWareVersion) )
		{
			lLen = sizeof(stSoftwareVer.acSoftWareVersion) -1;
		}
		memcpy(stSoftwareVer.acSoftWareVersion, pszInfoBack+strlen("\r\n"), lLen);
	}	
	else
	{
		stSoftwareVer.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stSoftwareVer, sizeof(stSoftwareVer));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	cx_printf("%s, %d, %s(), lMsgId = %d\n",__FILE__, __LINE__, __FUNCTION__, lMsgId);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d, %s(), acSoftWareVersion = %s\n",__FILE__, __LINE__, __FUNCTION__ , stSoftwareVer.acSoftWareVersion );
	return;
}


/*****************************************************************************
 函 数 名  : SoftwareVersionAnlysis_C
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
指令和反馈信息格式:
*SOFTWARE_VERSION_QUERY_C	20----AT+CGMR\r
*NORMAL:8----AT+CGMR<CR>,25----<CR><LF>11.310.09.00.00<CR><LF><CR><LF>OK<CF><LF>
*UNNORMAL:8----AT+CGMR<CR>,9----<CR><LF>ERROR<CR><LF>
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void SoftwareVersionAnlysis_C(char *pszInfoBack) 
 {
	struct stMsg_Software_Version_Query stSoftwareVer;
	int lMsgId;
	int lLen;
	memset(&stSoftwareVer, 0, sizeof(stSoftwareVer));
	g_stAtToOther.lMsgType = SOFTWARE_VERSION_QUERY_C;/*设置消息对应的命令*/
	stSoftwareVer.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack) && (NULL!=strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stSoftwareVer.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stSoftwareVer.acSoftWareVersion) )
		{
			lLen = sizeof(stSoftwareVer.acSoftWareVersion) -1;
		}
		memcpy(stSoftwareVer.acSoftWareVersion, pszInfoBack+strlen("\r\n"), lLen);
	}	
	else
	{		
		stSoftwareVer.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stSoftwareVer, sizeof(stSoftwareVer));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s, %d, %s(), acSoftWareVersion = %s\n",__FILE__, __LINE__, __FUNCTION__, stSoftwareVer.acSoftWareVersion );
	return;

 }

/*****************************************************************************
 函 数 名  : HardwareVersionAnlysis
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*HARDWARE_VERSION_QUERY	21----AT^HVER\r
*NORMAL:8----AT^HVER<CR>,26----<CR><LF>^HVER:"CD33TCPW"<CR><LF><CR><LF>OK<CF><LF>
*UNNORMAL:8----AT^HVER<CR>,<CR><LF>+CME ERROR: <err><CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void HardwareVersionAnlysis(char *pszInfoBack) 
 {
 	struct stMsg_Hardware_Version_Query stHardwareVer;
	int lMsgId;
	int lLen;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	memset(&stHardwareVer, 0, sizeof(stHardwareVer));
	g_stAtToOther.lMsgType = HARDWARE_VERSION_QUERY;/*设置消息对应的命令*/
	stHardwareVer.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack) && (NULL!=strstr(pszInfoBack, "\r\nOK\r\n")))/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stHardwareVer.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n^HVER:\"\"\r\n\r\nOK\r\n");
		if(lLen>=sizeof(stHardwareVer.acHardWareVersion))
			lLen = sizeof(stHardwareVer.acHardWareVersion) -1;
		memcpy(stHardwareVer.acHardWareVersion, pszInfoBack+strlen("\r\n^HVER:\""), lLen);
	}	
	else
	{
		stHardwareVer.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stHardwareVer, sizeof(stHardwareVer));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d, %s(), acHardWareVersion = %s\n",__FILE__, __LINE__,  __FUNCTION__, stHardwareVer.acHardWareVersion);
	return;	
 }
 
/*****************************************************************************
 函 数 名  : HardwareVersionAnlysis_W
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*HARDWARE_VERSION_QUERY_W	22----AT^HWVER\r
*NORMAL:9----AT^HWVER<CR>,27----<CR><LF>^HWVER:"CD33TCPW"<CR><LF><CR><LF>OK<CF><LF>
*UNNORMAL:<CR><LF>+CME ERROR: <err><CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void HardwareVersionAnlysis_W(char *pszInfoBack) 
{
 	struct stMsg_Hardware_Version_Query stHardwareVer;
	int lMsgId;
	int lLen;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	memset(&stHardwareVer, 0, sizeof(stHardwareVer));
	g_stAtToOther.lMsgType = HARDWARE_VERSION_QUERY_W;/*设置消息对应的命令*/
	stHardwareVer.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack) && (NULL!=strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stHardwareVer.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n^HWVER:\"\"\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stHardwareVer.acHardWareVersion) )
			lLen = sizeof(stHardwareVer.acHardWareVersion) -1;
		memcpy(stHardwareVer.acHardWareVersion, pszInfoBack+strlen("\r\n^HWVER:\""), lLen);
	}	
	else
	{
		stHardwareVer.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stHardwareVer, sizeof(stHardwareVer));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d, %s(), acHardWareVersion = %s\n",__FILE__, __LINE__, __FUNCTION__, stHardwareVer.acHardWareVersion);
	return;	

}

/*****************************************************************************
 函 数 名  : ImsiInfoAnlysis
 功能描述  : 
 输入参数  : char *pszInfoBack  
             int lLen           
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*IMSI_QUERY	27----AT+CIMI\r
*NORMAL:8----AT+CIMI<CR>, 25----<CR><LF>460028174600118<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:<CR><LF>+CME ERROR: <err><CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void ImsiInfoAnlysis(char *pszInfoBack) 
{
 	struct stMsg_IMSI_Query stImsi;
	int lMsgId;
	int lLen;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	memset(&stImsi, 0, sizeof(stImsi));
	g_stAtToOther.lMsgType = IMSI_QUERY;/*设置消息对应的命令*/
	stImsi.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	
	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stImsi.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stImsi.acIMSI) )
		{
			lLen = sizeof(stImsi.acIMSI) -1;
		}
		memcpy(stImsi.acIMSI, pszInfoBack+strlen("\r\n"), lLen);
		
		sysAvailSet(stImsi.acIMSI, IFC_IMSI_LEN, IMSI_OFFSET);
		/*begin:Added by luokunling 00192527 for imsi info requirement,2011/9/19*/
		if(0 == g_iHspaWeatherAutoPin)
		{
	          //ImsiNumIsOldImsiNum(stImsi.acIMSI);
	          TR069_SendMessage(CWMP_MSG_IMSI, (void*)(stImsi.acIMSI), strlen(stImsi.acIMSI)+1);
		}
		else
		{
		    TR069_SendMessage(CWMP_MSG_CONNECTED_IMSI_INFORM, (void*)(stImsi.acIMSI), strlen(stImsi.acIMSI)+1);
		    g_iHspaWeatherAutoPin = 0;		
		}
		/*End:Added by luokunling 00192527,2011/9/19*/
	}	
	else
	{
		stImsi.ucResult = HSPA_ERROR_UNKNOW;
	}	
	memcpy(g_stAtToOther.acText, (void*)&stImsi, sizeof(stImsi));

	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d,%s(), acIMSI = %s", __FILE__, __LINE__, __FUNCTION__, stImsi.acIMSI);
	return;

}
/*****************************************************************************
 函 数 名  : ImeiInfoAnlysis
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*IMEI_QUERY	28----AT+CGSN?\r
*NORMAL:8----AT+CGSN<CR>,  25----<CR><LF>358192015569919<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:<CR><LF>+CME ERROR: <err><CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void ImeiInfoAnlysis(char *pszInfoBack) 
{
 	struct stMsg_IMEI_Query stImei;
	int lMsgId;
	int lLen;
	memset(&stImei, 0, sizeof(stImei));
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	g_stAtToOther.lMsgType = IMEI_QUERY;/*设置消息对应的命令*/
	stImei.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stImei.ucResult = HSPA_OK;
		lLen = lLen-strlen("\r\n\r\n\r\nOK\r\n");
		if ( lLen >= sizeof(stImei.acIMEI) )
			lLen = sizeof(stImei.acIMEI) -1;
		memcpy(stImei.acIMEI, pszInfoBack+strlen("\r\n"), lLen);
	}	
	else
	{
		stImei.ucResult = HSPA_ERROR_UNKNOW;
	}	
	if ( HSPA_OK == g_stAtToOther.acText[HSPA_RESULT_POSITION] )
	{
		strcpy(g_stHspa.acIMEI, stImei.acIMEI);
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);	
	}	
	memcpy(g_stAtToOther.acText, (void*)&stImei, sizeof(stImei));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d,%s(), acIMEI = %s", __FILE__, __LINE__, __FUNCTION__,  stImei.acIMEI);
	return;

}
/*****************************************************************************
 函 数 名  : RssiInfoAnlysis
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*RSSI_QUERY	31----AT+CSQ\r
*NORMAL:7----AT+CSQ<CR>, 21----<CR><LF>+CSQ: 25,99<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void RssiInfoAnlysis(char *pszInfoBack) 
{
 	struct stMsg_RSSI_Query stRssi;
	int lMsgId;
	char *pChar;
	char acRssi[COMMON_VERY_SHORT_STRING_LENGTH];
	char acBer[COMMON_VERY_SHORT_STRING_LENGTH];
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	memset(&stRssi, 0, sizeof(stRssi));
	g_stAtToOther.lMsgType = RSSI_QUERY;/*设置消息对应的命令*/
	stRssi.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	TTY_LOG("TTYUSB: {%s}\n", pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
	if ( (NULL!=pszInfoBack)  &&  (NULL!=strstr(pszInfoBack, "\r\nOK\r\n") ) 
		&&  (NULL != (pChar = strstr(pszInfoBack, "+CSQ:")))  )/*判断反馈信息是否成功*/
	{
		stRssi.ucResult = HSPA_OK;
		pChar += strlen("+CSQ:");	

		stRssi.ucRSSI =StrStrToUnChar(&pChar, acRssi);
		stRssi.ucBer= StrStrToUnChar(&pChar, acBer);
	}	
	else
	{
		if ( (NULL != pszInfoBack) || (0 == strcmp(pszInfoBack, "\r\nERROR\r\n")) )
		{
			stRssi.ucResult = HSPA_ERROR_UNKNOW;
		}
	}	
	memcpy(g_stAtToOther.acText, (void*)&stRssi, sizeof(stRssi));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
	cx_printf("%s,%d,%s(), ucRSSI = %d, ucBer = %d\n", __FILE__, __LINE__, __FUNCTION__, stRssi.ucRSSI, stRssi.ucBer);
    g_sndflg = 0;
	sem_post(&sem);
	return;	
}
static int getWanStatus()
{
    FILE *fp = NULL;
    char acCmd[10] = "";
    int wanStatus = -1;

    if(NULL == (fp = fopen("/var/HspaDialFlag", "r")))
    {
        return -1;
    }
    fgets(acCmd, 10, fp);
    fclose(fp);
    wanStatus = atoi(acCmd);
    return wanStatus;
}
/*****************************************************************************
 函 数 名  : SystemInfoQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*SYSTEM_INFO_QUERY	34----AT^SYSINFO\r
*NORMAL:11---AT^SYSINFO<CR>, 31----<CR><LF>^SYSINFO:2,3,1,3,1,,3<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void SystemInfoQuery(char *pszInfoBack) 
{
	struct stMsg_System_Info_Query stSysInfo;
	int lMsgId;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	int lLightFlag = 0;
	struct stAtSndMsg stAtFromOther;
	char acInitHspa[COMMON_LONG_STRING_LENGTH];
	int lLen = 0;
	int lPinCounter = 0;
	
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	g_stAtToOther.lMsgType = SYSTEM_INFO_QUERY;/*设置消息对应的命令*/
	stSysInfo.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	if ( NULL != strstr(pszInfoBack, "AT^SYSINFO") )
	{
		pszInfoBack = strstr(pszInfoBack, "\r\n");
	}	
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		stSysInfo.ucResult = HSPA_OK;
		pChar = index(pszInfoBack, ':');
		if ( NULL == pChar )
		{
			goto Exit1;
		}
		/*计算第一个参数值*/
		stSysInfo.ucSrvStatus = StrStrToUnChar(&pChar, acCharParam);
		if ( ',' != *pChar )
			goto Exit2;
		/*计算第二个参数值*/
		stSysInfo.ucSrvDomain = StrStrToUnChar(&pChar, acCharParam);
		if ( ',' != *pChar )
			goto Exit3;
		/*计算第三个参数值*/
		stSysInfo.ucRoamStatus = StrStrToUnChar(&pChar, acCharParam);

		if ( ',' != *pChar )
			goto Exit4;
		/*计算第四个参数值*/	
		stSysInfo.ucSysMode = StrStrToUnChar(&pChar, acCharParam);

		if ( ',' != *pChar )
			goto Exit5;
		/*计算第五个参数值*/
		stSysInfo.ucSimState = StrStrToUnChar(&pChar, acCharParam);

		if ( ',' != *pChar )
			goto Exit6;
		/*计算第六个参数值*/
		stSysInfo.ucLockState = StrStrToUnChar(&pChar, acCharParam);

		if ( ',' != *pChar )
			goto Exit7;
		/*计算第七个参数值*/
		stSysInfo.ucSysSubMode = StrStrToUnChar(&pChar, acCharParam);

		memcpy(g_stAtToOther.acText, (void*)&stSysInfo, sizeof(stSysInfo));
		goto Exit8;
		
	}
	else
	{
		stSysInfo.ucResult = HSPA_ERROR_UNKNOW;
		goto Exit1;
	}
Exit1:
	stSysInfo.ucSrvStatus = 250;
Exit2:
	stSysInfo.ucSrvDomain = 250;	/*255表示不支持CDMA，所以错误码要修改250*/
Exit3:
	stSysInfo.ucRoamStatus = 250;
Exit4:
	stSysInfo.ucSysMode = 250;
Exit5:
	 stSysInfo.ucSimState = 250;/*255表示不支持SIM卡不存在，所以错误码要修改250*/
Exit6:
	stSysInfo.ucLockState = 250;
Exit7:
	stSysInfo.ucSysSubMode = 250;
Exit8:
	memcpy(g_stAtToOther.acText, (void*)&stSysInfo, sizeof(stSysInfo));

	/*更新共享数据*/
	g_stHspa.sSimExist = stSysInfo.ucSimState;
	g_stHspa.sSysMode = stSysInfo.ucSysMode;
	g_stHspa.sSubSysMode = stSysInfo.ucSysSubMode;
    g_stHspa.sSrvStatus = stSysInfo.ucSrvStatus;/*add by w00135351 20081107 AU8D01187*/
	
	if ( HSPA_UNKNOW_VALUE != stSysInfo.ucLockState)
	{
		g_stHspa.sSimStatus = stSysInfo.ucLockState;	
	}
    /*start of hspa with no sim minpengwei 20101208*/
	lLen = write(g_iPcuiFd, "AT+CLCK=\"SC\",2\r", strlen("AT+CLCK=\"SC\",2\r"));
	memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
	usleep(500);
	lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
	TTY_DEBUG("acInitHspa = %s\n", acInitHspa);
	if ( lLen < 0 )
	{
		TTY_DEBUG("len =%d\n",lLen);
		cx_printf("%s,%d----lLen = %d\n",__FILE__, __LINE__, lLen);
		g_stHspa.sSimExist = UNKNOW_ON_HSPA;
	}
	else if ( NULL != strstr(acInitHspa, "+CME ERROR: 13") ) /*SIM卡不在*/
	{
		TTY_DEBUG("No SIM, blink HSPA LED\n");
		sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
		g_stHspa.sSimExist = HSPA_NO_SIM;
	}
       /*end of hspa with no sim minpengwei 20101208*/
	TTY_DEBUG("g_stHspa.sSimExist = %d\n", g_stHspa.sSimExist);
	TTY_DEBUG("g_stHspa.sSimStatus = %d\n", g_stHspa.sSimStatus);
    TTY_DEBUG("g_stHspa.sSysMode = %d\n", g_stHspa.sSysMode);
    TTY_DEBUG("g_stHspa.sPinPukNeed = %d\n", g_stHspa.sPinPukNeed);
    TTY_DEBUG("g_stHspa.sDeviceStatus = %d\n", g_stHspa.sDeviceStatus);
    

	/* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/5 
	   for 解决E180V数据卡 HSPA切换到ADSL再切回HSPA, 灯一直显示红色问题. SYSINFO数据卡上报的第5个数字为0,表示USIM卡状态无效,
	   可能是需要pin码,先主动询问下是否需要,因为E180V在LPM模式丢失了已保存的pin码,而我们网关在flash里是一直保存的*/
    /*start of hspa with no sim minpengwei 20101208*/
    //if (0 == stSysInfo.ucSimState)
    if(0 == g_stHspa.sSimExist)
    /*end of hspa with no sim minpengwei 20101208*/
    {
		for( lPinCounter = 0; lPinCounter < MAX_PIN_TEST; lPinCounter++)
        {

			/*************检验是否需要PIN校验,并处理*************/
			if (strlen(g_stHspa.acPinCode) < 4)
			{
				sysAvailGet(g_stHspa.acPinCode, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
			}
		
   			sysAvailGet(g_stHspa.acOldIMEI, IFC_IMEI_LEN, IMEI_OFFSET);
   			lLen = strlen(g_stHspa.acOldIMEI);

			TTY_DEBUG("In Flash acPinCode = %s, acOldIMEI = %s\n", g_stHspa.acPinCode, g_stHspa.acOldIMEI);
		
   			while(lLen > 0)
   			{
   				if (isdigit(g_stHspa.acOldIMEI[lLen-1]))
   				{
   					lLen--;
   				}
   				else
   				{
   					memset(g_stHspa.acOldIMEI, 0, sizeof(g_stHspa.acOldIMEI));
   					break;
   				}
   			}

    		lLen = write(g_iPcuiFd, "AT+CLCK=\"SC\",2\r", strlen("AT+CLCK=\"SC\",2\r"));
    		memset(acInitHspa, 0, COMMON_LONG_STRING_LENGTH);
    		usleep(500);
    		lLen = read(g_iPcuiFd, acInitHspa, COMMON_LONG_STRING_LENGTH);
                TTY_DEBUG("len = %d, acInitHspa = %s\n", lLen, acInitHspa);
            
    		if ( lLen < 0 )
    		{
    			break;
    		}
			if ( (HSPA_PIN_NEED == g_stHspa.sPinPukNeed) 
			  || (HSPA_PUK_NEED == g_stHspa.sPinPukNeed))			  
			{
				break;
			}
			else
			{
        		if  (HSPA_OK != PinPukProcess(acInitHspa) )
        		{
        		    TTY_DEBUG("HSPA_OK != PinPukProcess(acInitHspa)\n");
                    sleep(5);
        		}
                else
                {
                    break;
                }
            }
        }
    }
	/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/5 */

    /*modified by w00135351 20081231*/
    /*star:检查数据卡共享文件中的设备状态和数据卡是否与g_HspaLine变量一致,不一致时g_HspaLine为准下发AT^CFUN设置命令使一致*/
    if ((HSPA_OFF == g_HspaLine & HSPA_LINE_MASK) && (g_stHspa.sDeviceStatus == 1))
    {
        TTY_DEBUG("( HSPA_OFF ==  g_HspaLine  ) && (g_stHspa.sDeviceStatus == 1)\n");
        AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, CFUN_SET, "AT+CFUN=0");
    }

    if ((HSPA_ON == g_HspaLine & HSPA_LINE_MASK) && (g_stHspa.sDeviceStatus == 0))
    {
        TTY_DEBUG("( HSPA_ON ==  g_HspaLine  ) && (g_stHspa.sDeviceStatus == 0)\n");
        AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, CFUN_SET, "AT+CFUN=1");
    }

    //printf("%d, %s() : g_HspaLine = %x\n", __LINE__, __FUNCTION__, g_HspaLine);
    /*end 状态一致性检查*/
    /*end by w00135351 20081231*/

    int wanStatus = getWanStatus();
	/*HSPA灯亮灭处理*/
        if ( 0 == g_stHspa.sDeviceStatus || 7 == g_stHspa.sDeviceStatus)
        {
            TTY_DEBUG("HSPA in LPM mode\n");
            sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
        }
    /*start of hspa with no sim minpengwei 20101208*/
       //else if ( ( 0 < g_stHspa.sSysMode)  && ( HSPA_NO_SIM != stSysInfo.ucSimState ) 	&& ( HSPA_INVALID_SIM != stSysInfo.ucSimState ))
	else if ( ( 0 < g_stHspa.sSysMode)  && ( HSPA_NO_SIM != g_stHspa.sSimExist) 	&& ( HSPA_INVALID_SIM != g_stHspa.sSimExist ))
    /*end of hspa with no sim minpengwei 20101208*/
	{

		/*start of when hspa call in the hspa led should be blink minpengwei 20101013*/
		
		FILE *pInCall = fopen("/var/incomingcall", "r");		
        char InCall = 0;
		if ( pInCall != NULL)
		{
		   //ringing
           fread(&InCall, 1, 1, pInCall);
           fclose(pInCall);
		}
	    if(InCall == 49)
	    {
	    	TTY_DEBUG("HSPA on Call In.\n");
	        sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
	    }else		
	   /*end of when hspa call in the hspa led should be blink minpengwei 20101013*/		
		/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/
		if (  CALL_ON_HSPA & g_ulTrafficFlag )
		{
		    //电话中
            TTY_DEBUG("HSPA on voice\n");
    	    sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
		}
		else if (2 == wanStatus)
		{

            TTY_DEBUG("HSPA on data\n");
            sysLedCtrl(kLedHspa, kLedStateVerySlowBlinkContiunes);
		}
		else/*网络已经通了，但是没有承载数据和语音业务*/
		{
            TTY_DEBUG("HSPA has no data or voice\n");
			//sysLedCtrl(kLedHspa, kLedStateOff);
			sysLedCtrl(kLedHspa, kLedStateOn);

		}
		/* END:   Added by c106292, 2009/3/16 */
	}
    else if ( ( HSPA_NO_SIM == g_stHspa.sSimExist ) ||( HSPA_INVALID_SIM == g_stHspa.sSimExist ) 
		|| ( 0 == stSysInfo.ucSysMode ) || (HSPA_PIN_PUK_OK != g_stHspa.sPinPukNeed))/*反馈信息255表示SIM卡不存在*/
	{
		memset(g_stHspa.acIMEI, 0, HSPA_IMSI_LENGTH);

/* BEGIN: Modified by y67514, 2009/12/8   问题单号:在未解pin码状态，紧急呼叫后，hspa灯常量7～8秒*/
            if (  CALL_ON_HSPA & g_ulTrafficFlag )
            {
                //电话中
                TTY_DEBUG("HSPA on voice\n");
                sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
            }
            else if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON)
            {
                TTY_DEBUG("HSPA abnormal, Blink HSPA Led!\n");
            	/* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
           	    sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
            	/* END:   Modified by c106292, 2009/3/17 */	   
            }
/* END:   Modified by y67514, 2009/12/8 */
	}
	
	/*没有设备，关灯
	防止在上面开灯时候，被信号处理函数
	打断*/
	if ( HSPA_NO == g_stHspa.sDeviceCount)
	{
        TTY_DEBUG("No HSPA Card: Turn off Led Led!\n");
		sysLedCtrl(kLedHspa, kLedStateOff);
	}

	/*更新共享文件*/
	lseek(g_ShareiFd, 0,SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);
    
    /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin:*/
	if ( HSPA_OK == CheckHspaInfo(&g_stHspa))
	{
		sysWakeupMonitorTask();
        TTY_DEBUG("sysWakeupMonitorTask\n");
	}
	BackupHspaInfo(&g_stHspa);
    /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);

	return;
}

/*****************************************************************************
 函 数 名  : CfgInfoSetBack
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*SYSCFG_SET	39---AT^SYSCFG=.....
*NORMAL:4----<CR><LF>OK<CR><LF>
*UNNORMAL:31---<CR><LF>INPUT VALUE IS OUT OF RANGE<CR><LF>
*9----<CR><LF>ERROR<CR><LF>
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CfgInfoSetBack(char *pszInfoBack)
{
	struct stMsg_Syscfg_Set	stSyscfgSetBack;
	int lMsgId;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	memset(&stSyscfgSetBack, 0, sizeof(stSyscfgSetBack));
	g_stAtToOther.lMsgType = SYSCFG_SET;/*设置消息对应的命令*/
	stSyscfgSetBack.ucCmdFrom = AT_MODULE;/*设置消息来源*/

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
	{
	  	stSyscfgSetBack.ucResult = HSPA_OK;
	}
	else
	{
		stSyscfgSetBack.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, (void*)&stSyscfgSetBack, sizeof(stSyscfgSetBack));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;
}

/*****************************************************************************
 函 数 名  : CfgInfoQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*SYSCFG_QUERY	40----AT^SYSCFG?\r
*NORMAL:34:  <CR><LF>^SYSCFG:2,1,3FFFFFFF,1,2<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CfgInfoQuery(char *pszInfoBack)
{
	struct stMsg_Syscfg_Query	stSyscfgQuery;
	int lMsgId;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	int i = 0;
	memset(&stSyscfgQuery, 0, sizeof(stSyscfgQuery));
	g_stAtToOther.lMsgType = SYSCFG_QUERY;/*设置消息对应的命令*/
	stSyscfgQuery.ucCmdFrom = AT_MODULE;/*设置消息来源*/

	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	
	if ( (NULL != strstr(pszInfoBack, "\r\nOK\r\n"))  && ( NULL != (pChar = strstr(pszInfoBack, ":")) ) )
	{		
	  	stSyscfgQuery.ucResult = HSPA_OK;
		//pChar = strstr(pszInfoBack, ":");
		/*计算第一个参数*/
		stSyscfgQuery.ucMode =StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%s, %d,pChar = %s, ucMode = %d\n",__FILE__,  __LINE__, pChar, stSyscfgQuery.ucMode);
		/*计算第二个参数*/
		stSyscfgQuery.ucAcqorder = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%s %d,acCharParam = %s, ucAcqorder = %d\n",  __FILE__, __LINE__, acCharParam, stSyscfgQuery.ucAcqorder);

		/*计算第三个参数*/
		while(1)
		{
			++pChar;
			if ( isxdigit(*pChar) )
			{
				stSyscfgQuery.acBand[i] = *pChar;
			}
			else
			{
				stSyscfgQuery.acBand[i] = 0;
				i=0;
				break;
			}
			++i;	
		}
		/*计算第四个参数*/	

		stSyscfgQuery.ucRoam = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%s %d,acCharParam = %s, ucRoam = %d\n",  __FILE__, __LINE__, acCharParam, stSyscfgQuery.ucRoam);

		/*计算第五个参数*/
		stSyscfgQuery.usSrvDomain = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("55--%d,acCharParam = %s, usSrvDomain = %d\n",  __LINE__, acCharParam, stSyscfgQuery.usSrvDomain);
		
		
	}
	else
	{
		stSyscfgQuery.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, (void*)&stSyscfgQuery, sizeof(stSyscfgQuery));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	
	return;
}
/*****************************************************************************
 函 数 名  : ConfigInfoSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*SYSCONFIG_SET	41---AT^SYSCONFIG =<mode>,<acqorder>,<roam>,<srvdomain> 
*NORMAL:
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void ConfigInfoSet(char *pszInfoBack)
{
	struct stMsg_Sysconfig_Set stSysconfigSetBack;
	int lMsgId;
	memset(&stSysconfigSetBack, 0, sizeof(stSysconfigSetBack));
	g_stAtToOther.lMsgType = SYSCONFIG_SET;/*设置消息对应的命令*/
	stSysconfigSetBack.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
	{
	  	stSysconfigSetBack.ucResult = HSPA_OK;
	}
	else
	{
		stSysconfigSetBack.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, (void*)&stSysconfigSetBack, sizeof(stSysconfigSetBack));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : ConfigInfoQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*SYSCONFIG_QUERY	42----AT^SYSCONFIG?\r
*NORMAL:28--<CR><LF>^SYSCONFIG:2,1,1,2<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void ConfigInfoQuery(char *pszInfoBack)
{
	struct stMsg_Sysconfig_Query stSysConfigQuery;
	int lMsgId;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	memset(&stSysConfigQuery, 0, sizeof(stSysConfigQuery));
	g_stAtToOther.lMsgType = SYSCONFIG_QUERY;/*设置消息对应的命令*/
	stSysConfigQuery.ucCmdFrom = AT_MODULE;/*设置消息来源*/

	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	pChar = strstr(pszInfoBack, ":");
	if ( (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) && (NULL != pChar) )
	{
		stSysConfigQuery.ucResult = HSPA_OK;
		/*计算第一个参数*/
		stSysConfigQuery.ucMode = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucMode = %d\n",  __LINE__, stSysConfigQuery.ucMode);
		/*计算第二个参数*/
		stSysConfigQuery.ucAcqorder= StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucAcqorder = %d\n",  __LINE__, stSysConfigQuery.ucAcqorder);
		/*计算第三个参数*/
		stSysConfigQuery.ucRoam = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucRoam = %d\n",  __LINE__, stSysConfigQuery.ucRoam);
		/*计算第四个参数*/
		stSysConfigQuery.usSrvDomain = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,usSrvDomain = %d\n",  __LINE__, stSysConfigQuery.usSrvDomain);
	}
	else
	{
		stSysConfigQuery.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, (void*)&stSysConfigQuery, sizeof(stSysConfigQuery));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : CgdcontSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*PDP_SET	51--+CGDCONT=<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>]]]]]
*NORMAL:
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CgdcontSet(char *pszInfoBack)
{
	struct  stMsg_PDP_Set stPdpSet;
	int lMsgId;
	memset(&stPdpSet, 0, sizeof(stPdpSet));
	g_stAtToOther.lMsgType = PDP_SET;/*设置消息对应的命令*/
	stPdpSet.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
	{
	  	stPdpSet.ucResult = HSPA_OK;
	}
	else
	{
		stPdpSet.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, (void*)&stPdpSet, sizeof(stPdpSet));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}

void ParsePdpParam(char *pDst, char *pSrc, int len)
{
    int i = 0;
    for (i = 0; i < len; i++ )
    {
        *pDst = *pSrc;
        ++pSrc;
        ++i;
        ++pDst;
        if ( ',' == *pSrc )
        {
            *pDst = 0;
            break;
        }
    }
}
/*****************************************************************************
 函 数 名  : CgdcontQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*PDP_QUERY	52----AT+CGDCONT?\r
*NORMAL:48----<CR><LF>+CGDCONT: 1,"IP","CMNET","0.0.0.0",0,0 <CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CgdcontQuery(char *pszInfoBack)
{
	struct stMsg_PDP_Query stPdpQuery;
	int lMsgId;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	int i = 0;
	memset(&stPdpQuery, 0, sizeof(stPdpQuery));
	g_stAtToOther.lMsgType = PDP_QUERY;/*设置消息对应的命令*/
	stPdpQuery.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )	
	{
		stPdpQuery.ucResult = HSPA_OK;
		pChar = index(pszInfoBack, ':' );/*表明当前没有保存任何设置*/
		if ( NULL == pChar)
			goto Exit1;
		/*解析第一个参数1,"IP","CMNET","0.0.0.0",0,0*/
		stPdpQuery.ucCidIndex = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,acCharParam = %s, ucCidIndex = %u\n",  __LINE__, acCharParam, stPdpQuery.ucCidIndex);
	
		/*解析第二个参数1,"IP","CMNET","0.0.0.0",0,0*/
		pChar = index(pChar, '"' );
		ParsePdpParam(stPdpQuery.acProtocolKind, pChar, HSPA_PROTOCOL_KIND_LENGTH);
		cx_printf("%d,acProtocolKind = %s\n",  __LINE__,stPdpQuery.acProtocolKind);
		/*解析第三个参数1,"IP","CMNET","0.0.0.0",0,0*/
		pChar = index(pChar, '"' );
		ParsePdpParam(stPdpQuery.acAPN, pChar, HSPA_APN_NAME_LENGTH);
		cx_printf("%d, acAPN = %s\n",  __LINE__,stPdpQuery.acAPN);
		/*解析第四个参数1,"IP","CMNET","0.0.0.0",0,0*/
		pChar = index(pChar, '"' );
		ParsePdpParam(stPdpQuery.acDomainName, pChar, HSPA_DOMAIN_ADDR_LENGTH);
		cx_printf("%d, acDomainName = %s\n",  __LINE__,  stPdpQuery.acDomainName);
		/*解析最后两个参数*/
		++pChar;
		stPdpQuery.ucDComp = (unsigned char)(*pChar - '0');/*将字符转化为对应的数值*/
		pChar +=2;
		stPdpQuery.ucHComp = (unsigned char)(*pChar - '0');/*将字符转化为对应的数值*/
		cx_printf("%d, ucDComp = %d, ucHComp = %d\n",  __LINE__,  stPdpQuery.ucDComp, stPdpQuery.ucHComp);
	}
	else
	{
		stPdpQuery.ucResult = HSPA_ERROR_UNKNOW;
	}
Exit1:
	memcpy(g_stAtToOther.acText, (void*)&stPdpQuery, sizeof(stPdpQuery));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : PinNumModify
 功能描述  : 
 输入参数  : char *pszInfoBack  
             int lLen           
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*PIN码使能激活的情况下才可使用
*PIN_PASSWORD_MODIFY	45--AT+CPWD=<fac>,<oldpwd>,<newpwd>
*NORMAL:
*UNNORMAL:	+CME ERROR: incorrect password
*				+CME ERROR: operation not allowed
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void PinNumModify(char *pszInfoBack)
{
	int lMsgId;
	g_stAtToOther.lMsgType = PIN_PASSWORD_MODIFY;/*设置消息对应的命令*/
	g_stAtToOther.acText[HSPA_FROM_POSITION]= AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL != strstr(pszInfoBack, "OK") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
	}
	else
	if ( NULL != strstr(pszInfoBack, "incorrect password") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_INCORRECT_PASSWORD;
	}
	else
	if ( NULL != strstr(pszInfoBack, "operation not allowed") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_OPERATION_REFUSED;
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : PinEnableSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 

指令和反馈信息格式:
*PIN_ENABLE	48---AT+CLCK=
*NORMAL:		
*UNNORMAL:		+CME ERROR: incorrect password

 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void PinEnableSet(char *pszInfoBack)
{
	int lMsgId;
	g_stAtToOther.lMsgType = PIN_ENABLE;/*设置消息对应的命令*/
	g_stAtToOther.acText[HSPA_FROM_POSITION]= AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
    /*add by w00135351 20080908*/
	if ( NULL != strstr(pszInfoBack, "OK") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
	}
	else
	if ( NULL != strstr(pszInfoBack, "+CME ERROR: 16") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_INCORRECT_PASSWORD;
	}
	else
	if ( NULL != strstr(pszInfoBack, "+CME ERROR: 3") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_OPERATION_REFUSED;
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
    /*end by w00135351 20080908*/
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : PinNumVerfication
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 

*用于PIN码验证
*PIN_MANAGE	49---AT+CPIN=<pin>[,<newpin>] 
*NORMAL:
*UNNORMAL:
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void PinNumVerfication(char *pszInfoBack)
{
	int lMsgId;
	struct stAtSndMsg stAtFromOther;
	g_stAtToOther.lMsgType = PIN_MANAGE;/*设置消息对应的命令*/
	g_stAtToOther.acText[HSPA_FROM_POSITION]= AT_MODULE;/*设置消息来源*/
	char* pPinCode = NULL;
	char* pPinEnd = NULL;
    char buf[COMMON_LONG_STRING_LENGTH];
	

	TTY_LOG("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    usleep(500);
    HspaMatchRead(pszInfoBack);
    
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL!=strstr(pszInfoBack, "OK") )
	{
        /* turn on led. <2009.1.5 tanyin> */
        TTY_DEBUG("pin verified ok,turn on led\n");
        sysLedCtrl(kLedHspa, kLedStateOn);

		g_stAtToOther.acText[HSPA_RESULT_POSITION]= HSPA_OK;
		
		g_stHspa.sPinPukNeed = HSPA_PIN_PUK_OK;
		/*更新共享文件*/
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);

		
		if ( pPinCode = strstr(g_stAtFromOther.acParam, "AT+CPIN=\""))
		{
            if(NULL != strstr(g_stAtFromOther.acParam, ",\""))
            {
                pPinCode = strstr(g_stAtFromOther.acParam, ",\"");
                strcpy(g_stHspa.acPinCode, pPinCode+2);
            }
            else
            {
                strcpy(g_stHspa.acPinCode, pPinCode+strlen("AT+CPIN=\""));
            }
                
			TTY_DEBUG("pincode command: %s\n", pPinCode);
			//strcpy(g_stHspa.acPinCode, pPinCode+strlen("AT+CPIN=\""));
			pPinEnd = strstr(g_stHspa.acPinCode, "\"");
			if (pPinEnd)
			{
				*pPinEnd = '\0';
			}
			TTY_DEBUG("pincode: %s \n",g_stHspa.acPinCode);
		}


		strcpy(g_stHspa.acOldIMEI, g_stHspa.acIMEI);
		
        /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin.*/
		if ( HSPA_OK == CheckHspaInfo(&g_stHspa))
		{
			sysWakeupMonitorTask();
            TTY_DEBUG("sysWakeupMonitorTask\n");
			cx_printf("%s, %d, %s(),       sysWakeupMonitorTask\n", __FILE__, __LINE__, __FUNCTION__);
		}
		BackupHspaInfo(&g_stHspa);
        /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION]= HSPA_ERROR_UNKNOW;
	}
	/*判断是否关红灯，判断是否开蓝灯*/
	if ( ( HSPA_PIN_PUK_OK==  g_stHspa.sPinPukNeed) &&  ( g_stHspa.sSysMode > 0)  
		&& (HSPA_NO_SIM != g_stHspa.sSimStatus) && (0 != g_stHspa.sSimStatus)
		&&(0 != g_stHspa.sDeviceStatus)&&(7 != g_stHspa.sDeviceStatus))	
	{
		
		sysLedCtrl(kLedHspa, kLedStateOn); 
	}


	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId !=  lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}

    g_sndflg = 0;
	sem_post(&sem);

	if( HSPA_OK == g_stAtToOther.acText[HSPA_RESULT_POSITION]  )
	{
		write(g_iPcuiFd, "AT+CLIP=1\r",	strlen("AT+CLIP=1\r"));	
        usleep(500);
        read(g_iPcuiFd, buf, COMMON_LONG_STRING_LENGTH);
		#if 0
		write(g_iPcuiFd, "AT^LIGHT=0\r", strlen("AT^LIGHT=0\r"));
		#endif
		AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, PIN_ENABLE_QUERY, "AT+CLCK=\"SC\",2");
		sleep(1);
             /*Begin:Added by luokunling 00192527 for IMSI inform requirement,2011/9/20*/
		AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, IMSI_QUERY,"AT+CIMI");
	      /*end:Added by luokunling 00192527,2011/9/20*/
	}
	return;
}
/*****************************************************************************
 函 数 名  : PinNumLeftQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 

指令和反馈信息格式:
*PIN_PUK_TIMES_QUERY	69---AT^CPIN?\r
*NORMAL:		^CPIN: READY,,10,3,10,3
*UNNORMAL:

 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void PinNumLeftQuery(char *pszInfoBack)
{
	struct stMsg_Pin_Time_Query stPinTimeLeft;
	int lMsgId;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	memset(&stPinTimeLeft, 0, sizeof(stPinTimeLeft));
	g_stAtToOther.lMsgType = PIN_PUK_TIMES_QUERY;/*设置消息对应的命令*/
	stPinTimeLeft.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);

    pChar=strstr(pszInfoBack, ",");
	if ( NULL != strstr(pszInfoBack, "^CPIN: READY") )
	{
		stPinTimeLeft.ucResult = HSPA_OK;
		/*stPinTimeLeft.ucTimes = 3;
		stPinTimeLeft.ucPukTimes= 10;
		stPinTimeLeft.ucPinTimes= 3;
		stPinTimeLeft.ucPuk2Times= 10;
		stPinTimeLeft.ucPin2Times= 3;*/ /*modified by w00135351 20081110*/
        //stPinTimeLeft.ucTimes = StrStrToUnChar(&pChar, acCharParam);
        stPinTimeLeft.ucTimes = 3;
        TTY_DEBUG("CPIN is ready\n");
        pChar++;
        stPinTimeLeft.ucPukTimes= StrStrToUnChar(&pChar, acCharParam);
        stPinTimeLeft.ucPinTimes= StrStrToUnChar(&pChar, acCharParam);
        stPinTimeLeft.ucPuk2Times= StrStrToUnChar(&pChar, acCharParam);
        stPinTimeLeft.ucPin2Times= StrStrToUnChar(&pChar, acCharParam);

        g_stHspa.sPinPukNeed = 0;  /*add by w00135351 20081226 规避偶现状态文件中该值未更新的问题*/
		/*更新共享文件*/
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);
        
		goto Exit1;
	}
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	pChar=strstr(pszInfoBack, ",");
	if( ( NULL != strstr(pszInfoBack, "^CPIN:") ) && ( NULL != pChar ) )
	{
		stPinTimeLeft.ucResult = HSPA_OK;
		/*1*/
		stPinTimeLeft.ucTimes = StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucTimes = %d\n",__LINE__, stPinTimeLeft.ucTimes);
		/*2*/
		stPinTimeLeft.ucPukTimes= StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucPukTimes = %d\n",__LINE__, stPinTimeLeft.ucPukTimes);
		/*3*/
		stPinTimeLeft.ucPinTimes= StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucPinTimes = %d\n",__LINE__, stPinTimeLeft.ucPinTimes);
		/*4*/
		stPinTimeLeft.ucPuk2Times= StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucPuk2Times = %d\n",__LINE__, stPinTimeLeft.ucPuk2Times);
		/*5*/
		stPinTimeLeft.ucPin2Times= StrStrToUnChar(&pChar, acCharParam);
		cx_printf("%d,ucPin2Times = %d\n",__LINE__, stPinTimeLeft.ucPin2Times);
		if( 0 == stPinTimeLeft.ucPinTimes)
		{
			g_stHspa.sPinPukNeed = 2;
			/*更新共享文件*/
			lseek(g_ShareiFd, 0,SEEK_SET);
			write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
			fsync(g_ShareiFd);
            
            /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin.*/
			if ( HSPA_OK == CheckHspaInfo(&g_stHspa))
			{
				sysWakeupMonitorTask();
                TTY_DEBUG("sysWakeupMonitorTask\n");
				cx_printf("%s, %d, %s(),       sysWakeupMonitorTask\n", __FILE__, __LINE__, __FUNCTION__);
			}
			BackupHspaInfo(&g_stHspa);
            /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/

		}
	}
	else
	{
		stPinTimeLeft.ucResult = HSPA_ERROR_UNKNOW;
	}
Exit1:
	memcpy(g_stAtToOther.acText, &stPinTimeLeft, sizeof(stPinTimeLeft));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId != lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : PinEnableQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 

指令和反馈信息格式:
*PIN_ENABLE_QUERY	70---AT+CLCK="SC",2
*NORMAL:		
*UNNORMAL:		+CME ERROR: incorrect password
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void PinEnableQuery(char *pszInfoBack)
{
	int lMsgId;
	struct stMsg_Pin_Password_Verification stPinEnableSet;
	char *pChar = NULL;
	memset(&stPinEnableSet, 0, sizeof(stPinEnableSet));
	g_stAtToOther.lMsgType = PIN_ENABLE_QUERY;/*设置消息对应的命令*/
	stPinEnableSet.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( (NULL != strstr(pszInfoBack, "OK")) ||  (NULL != strstr(pszInfoBack, "+CLCK:")) )
	{
		stPinEnableSet.ucResult = HSPA_OK;
		pChar = index(pszInfoBack, ':');
		if ( NULL != pChar )
		{
			++pChar;
			if ( isdigit(*pChar) )
			{
				cx_printf("%s,%d,pChar = %c\n", __FILE__, __LINE__, *pChar);
				g_stAtToOther.acText[2] =  (unsigned char)(*pChar - '0');
			}
			else
			{
				++pChar;
				cx_printf("%s,%d,pChar = %c\n", __FILE__, __LINE__, *pChar);
				g_stAtToOther.acText[2] = (unsigned char)(*pChar - '0');
			}
            stPinEnableSet.ucStatus = g_stAtToOther.acText[2]; /*add by 
            w00135351 20081030*/
		}
	
	}
    else if((NULL != strstr(pszInfoBack,"+CME ERROR: 11"))/*modified by 
    w00135351 20081030   need pin or need puk*/
    ||(NULL != strstr(pszInfoBack,"+CME ERROR: 12")))
    {
        stPinEnableSet.ucResult = HSPA_PIN_PUK_NEEDED;
    }
	else
	{
		stPinEnableSet.ucResult = HSPA_ERROR_UNKNOW;
	}

	if(HSPA_OK == g_stAtToOther.acText[HSPA_RESULT_POSITION])
	{
		g_stHspa.sSimStatus = stPinEnableSet.ucStatus;
		lseek(g_ShareiFd, 0,SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);
        
        /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add begin.*/
		if ( HSPA_OK == CheckHspaInfo(&g_stHspa))
		{
			sysWakeupMonitorTask();
            TTY_DEBUG("sysWakeupMonitorTask\n");
			cx_printf("%s, %d, %s(),       sysWakeupMonitorTask\n", __FILE__, __LINE__, __FUNCTION__);
		}
		BackupHspaInfo(&g_stHspa);
        /* HUAWEI HGW s48571 2008年2月2日" Httpd requirement for voice state monitor add end.*/
	}	
	if ( ( 0 < g_stHspa.sPinPukNeed) || ( HSPA_NO_SIM ==  g_stHspa.sSimExist)
		|| ( 0 ==  g_stHspa.sSimExist) )
	{
		/*红灯*/
        if( (g_HspaLine & HSPA_LINE_MASK) == HSPA_ON )
        {
            /* BEGIN: Modified by c106292, 2009/3/17   PN:Led light method changes*/
        	//	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
         	 sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
            /* END:   Modified by c106292, 2009/3/17 */
    	}
	}
	else
	{
		//sysLedCtrl(kLedHspa, kLedStateOn); 
	}
	
	cx_printf("%s,%d,%s\n",__FILE__, __LINE__, __FUNCTION__);
	memcpy(g_stAtToOther.acText, (void*)&stPinEnableSet, sizeof(stPinEnableSet));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
	cx_printf("%s,%d,%s\n",__FILE__, __LINE__, __FUNCTION__);
	write(g_iPcuiFd, "AT^CURC=1\r", strlen("AT^CURC=1\r"));
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : TelNumReport
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 

*CALL_NUM_REPORT	65	+CLIP: "075526747084",129,,,,0
*NORMAL:
*UNNORMAL:
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void TelNumReport(char *pszInfoBack)
{
	struct stVoiceMsg_Call_Num stTelNum;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	int i = 0;
	memset(&stTelNum, 0, sizeof(stTelNum));
	g_stAtToOther.lMsgType = CALL_NUM_REPORT;/*设置消息对应的命令*/
	stTelNum.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	stTelNum.ucResult = HSPA_ERROR_UNKNOW;
	pChar = index(pszInfoBack, '"');
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	while(1)
	{
		++pChar;
		if('"'!=*pChar)
			stTelNum.acTelNum[i] = *pChar;
		else
		{
			stTelNum.acTelNum[i] = 0;
			break;
		}
		++i;			
	}
	cx_printf("%d,acTelNum = %s\n",__LINE__, stTelNum.acTelNum);
	++pChar;
	stTelNum.ucTelType = StrStrToUnChar(&pChar,  acCharParam);
	cx_printf("%d,ucTelType = %d\n",__LINE__, stTelNum.ucTelType);
	pChar += strlen(",,,,");
	stTelNum.ucCliFlag = (unsigned char)(*pChar -'0');
	cx_printf("%d,ucCliFlag = %d\n",__LINE__, stTelNum.ucCliFlag);

	memcpy(g_stAtToOther.acText, (void*)&stTelNum, sizeof(stTelNum));
	/*要上报业务进程.*/
#ifdef CX_DEBUG_VERSION	
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#else
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#endif
	cx_printf("%s,%d,%s\n",__FILE__, __LINE__, __FUNCTION__);
	return;
}

/*****************************************************************************
 函 数 名  : CallDial
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CallDial(char *pszInfoBack)
{
	int lMsgId;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	if ( NULL == strstr(pszInfoBack,  "\r\nOK\r\n") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;

		g_stAtToOther.lMsgType = DIAL;
		lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
		cx_printf("%s,%d,%s(),lMsgId = %d\n", __FILE__,  __LINE__, __FUNCTION__, lMsgId);
		/*若是控制模块自身的消息，不予反馈*/
		if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
		{
            TTY_DEBUG("lMsgId is %d\n", lMsgId);
			msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
		}	
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}


/*****************************************************************************
 函 数 名  : AutoConf
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年2月2日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AutoConf(char *pszInfoBack)
{
    struct stAtSndMsg stAtFromOther;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	//write(g_iPcuiFd, "AT^DDSETEX=2\r",strlen("AT^DDSETEX=2\r"));
    AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, DEX_SET,"AT^DDSETEX=2" );
    g_iCallOutFlag = VOICE_CALLOUT_AUTOCONF;
	/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/
	g_ulTrafficFlag	|= CALL_ON_HSPA;
	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
	/* END:   Added by c106292, 2009/3/16 */
#if 0
	g_stAtToOther.lMsgType = DIAL;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
	g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#endif

}

/*****************************************************************************
 函 数 名  : AcceptCall
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AcceptCall(char *pszInfoBack)
{
	int lMsgId;
    int i = 0;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    for( i = 0; i < 3; i++ )
    {
        if ( NULL == strstr(pszInfoBack, "\r\nOK\r\n") )
        {
        	memset(pszInfoBack, 0, AT_RCV_MSG_LENGTH);
		TTY_LOG("ATA: Find OK error,read pcui again\n");
        	read(g_iPcuiFd, pszInfoBack, AT_RCV_MSG_LENGTH);
            cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
        }
        else
        {
	/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/
	g_ulTrafficFlag	|= CALL_ON_HSPA;
	sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
	/* END:   Added by c106292, 2009/3/16 */
            break;
        }
    }

	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
	{	
        g_iCallInFlag = VOICE_CALLIN_ATA;
        TTY_LOG("ATA: Find OK error,read pcui again\n");
	}
	else
	{
        g_stAtToOther.lMsgType = ACCEPT;
        g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
        g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
        TTY_LOG("ATA: Find OK error,Send HSPA_ERROR_UNKNOW\n");
        msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
        g_iCallInFlag = VOICE_CALLIN_IDLE;
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	return;
}

/*****************************************************************************
 函 数 名  : DExSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void DExSet(char *pszInfoBack)
{

    if( VOICE_CALLIN_CONN == g_iCallInFlag )
    {
    	g_stAtToOther.lMsgType = ACCEPT;
    	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
        
    	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    	if ( NULL == strstr(pszInfoBack, "\r\nOK\r\n") )
    	{
    		memset(pszInfoBack, 0, AT_RCV_MSG_LENGTH);
    		read(g_iPcuiFd, pszInfoBack, AT_RCV_MSG_LENGTH);
    	}
    	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
    	{	
    		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
    		TTY_LOG("DEXSET:SEND OK for ATA");
    	}
    	else
    	{
    		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
    		TTY_LOG("DEXSET:SEND NOK for ATA");
    	}
        
    	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
        g_iCallInFlag = VOICE_CALLIN_IDLE;
    }
#if 1
    else if (VOICE_CALLOUT_AUTOCONF == g_iCallOutFlag)
    {
        g_stAtToOther.lMsgType = DIAL;
        g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;

    	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    	if ( NULL == strstr(pszInfoBack, "\r\nOK\r\n") )
    	{
    		memset(pszInfoBack, 0, AT_RCV_MSG_LENGTH);
    		read(g_iPcuiFd, pszInfoBack, AT_RCV_MSG_LENGTH);
    	}
    	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

    	if ( NULL != strstr(pszInfoBack, "\r\nOK\r\n") )
    	{	
    		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
    		TTY_LOG("DEXSET:SEND OK for ATD");
    	}
    	else
    	{
    		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
    		TTY_LOG("DEXSET:SEND NOK for ATD");
    	}

    	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
        g_iCallOutFlag = VOICE_CALLOUT_IDLE;

        
    }
#endif
    g_sndflg = 0;
	sem_post(&sem);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	return;
}


/*****************************************************************************
 函 数 名  : EndCallReport
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  :  ^CEND:1,0,104,31--31可能没有，上报，就复值0。
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void EndCallReport(char *pszInfoBack)
{
	struct stVoice_Msg_Call_End stCallEnd;
	char *pChar;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	g_stAtToOther.lMsgType = CALL_END_REPORT;
	stCallEnd.ucCmdFrom = AT_MODULE;
	stCallEnd.ucResult = HSPA_OK;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    g_iCallInFlag = VOICE_CALLIN_IDLE;

	/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/
	g_ulTrafficFlag &=  ~CALL_ON_HSPA;
	if( 0  !=  g_stHspa.sDeviceCount)
	{
            /* BEGIN: Modified by y67514, 2009/12/8   PN:在未解pin码状态，紧急呼叫后，hspa灯常量7～8秒*/
            if ( ( HSPA_NO_SIM == g_stHspa.sSimExist ) ||( HSPA_INVALID_SIM == g_stHspa.sSimExist ) 
                || (HSPA_PIN_PUK_OK != g_stHspa.sPinPukNeed))
            {
                TTY_DEBUG("HSPA is invalid! \n");
                sysLedCtrl(kLedHspa, kLedStateSlowFlickerContinues);
            }
            else if( 2 == getWanStatus())
            {
                TTY_DEBUG("Data is on \n");
                sysLedCtrl(kLedHspa, kLedStateOff);
                sysLedCtrl(kLedHspa, kLedStateVerySlowBlinkContiunes);
            }
            else
            {
                TTY_DEBUG("Data is not ready \n");
                sysLedCtrl(kLedHspa, kLedStateOff);	  
                sysLedCtrl(kLedHspa, kLedStateOn);
            }
    	/* END:   Modified by y67514, 2009/12/8 */
	}
	/* END:   Added by c106292, 2009/3/16 */

	pChar = index(pszInfoBack, ':');
	if ( NULL == pChar )
		goto Exit1;
	++pChar;
	stCallEnd.ucCallId = *pChar -'0';
	cx_printf("%s, %d, ucCallId = %u\n",__FILE__, __LINE__, stCallEnd.ucCallId );
	/*解析第2个参数*/
	pChar = index(pChar, ',');
	if ( NULL == pChar )
		goto Exit1;
    //add by z67625 20080330 for cend, start
	stCallEnd.ulDuration = StrStrToInt(&pChar, acCharParam);
	cx_printf("%s, %d, ucEndCause = %u\n",__FILE__, __LINE__, stCallEnd.ucEndCause );
	/*解析第3个参数66  ^CEND:1,0,104,31 */
	stCallEnd.ucEndCause = StrStrToUnChar(&pChar, acCharParam);
    //add by z67625 20080330 for cend, end
	cx_printf("%s,%d, ulDuration = %u\n",__FILE__, __LINE__, stCallEnd.ulDuration );
	/*解析第4个参数66  ^CEND:1,0,104,31 */
	stCallEnd.ucCallCtlCause= StrStrToUnChar(&pChar, acCharParam);
	cx_printf("%s, %d, ucCallCtlCause = %u\n",__FILE__, __LINE__, stCallEnd.ucCallCtlCause );
Exit1:	
	memcpy(g_stAtToOther.acText, (void*)&stCallEnd, sizeof(stCallEnd));
#ifndef CX_DEBUG_VERSION	
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#else
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#endif
	return;
}

/*****************************************************************************
 函 数 名  : CfunSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数
*****************************************************************************/
static void CfunSet(char *pszInfoBack)
{
	int lMsgId;
	cx_printf("%s,%d,pszInfoBack = %s", __FILE__,  __LINE__, pszInfoBack);

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,pszInfoBack = %s", __FILE__,  __LINE__, pszInfoBack);
	if ( NULL != strstr(pszInfoBack,  "\r\nOK\r\n") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
	g_stAtToOther.lMsgType = CFUN_SET;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;

}
/*****************************************************************************
 函 数 名  : CfunQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
操作模式查询命令
命令格式CFUN_QUERY ---- AT+CFUN？ 
响应格式：<CR><LF>+CFUN: 1<CR><LF><CR><LF>OK<CR><LF>有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF> 
 修改历史      :
  1.日    期   : 2008年1月8日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CfunQuery(char *pszInfoBack)
{
//	struct stMsg_Operation_Mode_Query stOpModeQuery;
	int lMsgId;
	char *pChar;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	if ( NULL == strstr(pszInfoBack, "+CFUN:") )
	{
		memset(pszInfoBack, 0, AT_RCV_MSG_LENGTH);
		read(g_iPcuiFd, pszInfoBack, AT_RCV_MSG_LENGTH);
	}
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	pChar = strstr(pszInfoBack,  "+CFUN:");
	if ( (NULL != strstr(pszInfoBack,  "\r\nOK\r\n")) && (NULL != strstr(pszInfoBack,  "+CFUN:"))   )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
		pChar = index(pChar, ':');
		if ( NULL != pChar )
		{
			++pChar;
			if(isdigit(*pChar))
			{
				g_stAtToOther.acText[2] = *pChar - '0';
			}
			else
			{
				++pChar;
				g_stAtToOther.acText[2] = *pChar - '0';
			}			
		}
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
	g_stAtToOther.lMsgType = CFUN_QUERY;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;

	/*更新共享数据*/
	if ( HSPA_OK == g_stAtToOther.acText[HSPA_RESULT_POSITION] )
	{
		g_stHspa.sDeviceStatus = g_stAtToOther.acText[2];
        #if 0 //AU8D00583 SIP user先注销然后重新注册，hspa灯显示不正确。必现 -- 宋立媛
        if( (HSPA_PIN_PUK_OK != g_stHspa.sPinPukNeed) || (g_stHspa.sSimExist == HSPA_NO_SIM) )
            sysLedCtrl(kLedHspa, kLedStateFail);
        #endif

	}
	else
	{
		g_stHspa.sDeviceStatus = HSPA_UNKNOW;
	}
	/*更新共享文件*/
	lseek(g_ShareiFd, 0,SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);	

	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	
	return;
}
/*****************************************************************************
 函 数 名  : AutoModeReport
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AutoModeReport(char *pszInfoBack)
{
	struct stMsg_System_Mode stSysMode;
	char *pChar;
	int lLightFlag = 0;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	g_stAtToOther.lMsgType = AUTO_SYSTEM_MODE_STATUS_REPORT;
	stSysMode.ucCmdFrom = AT_MODULE;
	stSysMode.ucResult = HSPA_OK;

	pChar = index(pszInfoBack, ':');
	++pChar;
	stSysMode.ucSysMode = *pChar -'0';
	pChar +=2;
	stSysMode.ucSubSysMode= *pChar -'0';
#if 0
	/*判断是否更改亮灯状态*/
	if ( ( g_stHspa.sSysMode < 4) && (stSysMode.ucSysMode >= 4 ) )
	{
		lLightFlag = 1;
	}
	else
	if ( (-1 ==  g_stHspa.sSysMode ) && (stSysMode.ucSysMode > 0 ) )
	{
		lLightFlag = 1;
	}
#endif

	/*更新共享文件*/
	g_stHspa.sSysMode = (short int)stSysMode.ucSysMode;
	g_stHspa.sSubSysMode = (short int)stSysMode.ucSubSysMode;
	lseek(g_ShareiFd, 0, SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);
	
	memcpy(&g_stAtToOther.acText, &stSysMode, sizeof(stSysMode));
#ifndef CX_DEBUG_VERSION	
//	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lModemMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lTr069MsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lLedMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#else
	//msgsnd(g_lCmdMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#endif

#if 0
	/*HSPA灯亮灭处理*/
	if (( 1 == lLightFlag) &&  ( 0 <  g_stHspa.sSysMode) 
		&& (HSPA_INVALID_SIM!= g_stHspa.sSimExist) && (HSPA_NO_SIM != g_stHspa.sSimExist))
	{
		if ( g_stHspa.sSysMode < 4)
		{
			cx_printf("%d, 2G蓝灯\n", __LINE__);
			sysLedCtrl(kLedHspa, kLedStateOn); 
			sysSetHspaTrafficMode(MODE_2G);
		}
		else
		if ( ( g_stHspa.sSysMode < 8) )	
		{
			cx_printf("%d, 3G蓝灯\n", __LINE__);
			sysLedCtrl(kLedHspa, kLedStateOn); 
			sysSetHspaTrafficMode(MODE_3G);
		}
	}
	
	if ( 0 == stSysMode.ucSysMode )
	{
		/*红灯*/
		cx_printf("%d,没有连上网，开红灯，关蓝灯\n",__LINE__);
		sysSetHspaTrafficMode(MODE_NONE);
		sysLedCtrl(kLedHspa, kLedStateOff);
		sysLedCtrl(kLedHspa, kLedStateFail);
	}
	else
	if ( ( HSPA_INVALID_SIM ==  g_stHspa.sSimExist) ||  ( HSPA_NO_SIM ==  g_stHspa.sSimExist) )
	{
		/*红灯*/
		cx_printf("%d,没有SIM卡或SIM卡无效，开红灯，关蓝灯\n",__LINE__);
		sysSetHspaTrafficMode(MODE_NONE);
		sysLedCtrl(kLedHspa, kLedStateOff);
		sysLedCtrl(kLedHspa, kLedStateFail);
	}
	/*没有设备，关灯
	防止在上面开灯时候，被信号处
	理函数打断*/
	if ( HSPA_NO == g_stHspa.sDeviceCount)
	{
		cx_printf("%d,没有数据卡关灯\n",__LINE__);
		sysLedCtrl(kLedHspa, kLedStateOff);
		sysSetHspaTrafficMode(MODE_NONE); 	
	}
#endif
	/* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/9 
	   for 在ttyUSB启动时已启动一线程每隔10s发送AT^SYSINFO,所以这里就不需要了*/
	/*
	struct stAtSndMsg stAtFromOther;
	AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, SYSTEM_INFO_QUERY, "AT^SYSINFO");
	*/
	/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/9/9 */
	return;

}

/*****************************************************************************
 函 数 名  : AutoServerStatus
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AutoServerStatus(char *pszInfoBack)
{
	struct stMsg_Service_Status stSerStatus;
	char *pChar;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	
	g_stAtToOther.lMsgType = AUTO_SERVICE_STATUS_REPORT;
	stSerStatus.ucCmdFrom = AT_MODULE;
	stSerStatus.ucResult = HSPA_OK;
	
	pChar = index(pszInfoBack, ':');
	++pChar;
	stSerStatus.ucSrvStatus = *pChar - '0';
	
	memcpy(&g_stAtToOther.acText, &stSerStatus, sizeof(stSerStatus));

	/*更新共享文件*/
	g_stHspa.sSrvStatus = (short int)stSerStatus.ucSrvStatus;
	lseek(g_ShareiFd, 0, SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);
	
#ifndef CX_DEBUG_VERSION	
//	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lModemMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lTr069MsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
//	msgsnd(g_lLedMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#else
	//msgsnd(g_lCmdMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, 0);
#endif

	return;
}

/*****************************************************************************
 函 数 名  : AutoRing
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月14日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AutoRing(char *pszInfoBack)
{
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	g_stAtToOther.lMsgType = RING_REPORT;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
	g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;

#ifndef CX_DEBUG_VERSION	
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#else
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#endif
}
/*****************************************************************************
 函 数 名  : AutoConn
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月17日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void AutoConn(char *pszInfoBack)
{
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    struct stAtSndMsg stAtFromOther;

    if( VOICE_CALLIN_ATA == g_iCallInFlag )
    {
        g_iCallInFlag = VOICE_CALLIN_CONN;
        AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, DEX_SET,"AT^DDSETEX=2" );
    }
    else 
    {
    	g_stAtToOther.lMsgType = CONN_REPORT;
    	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
    	g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;

#ifndef CX_DEBUG_VERSION	
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#else
	msgsnd(g_lVoiceMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
#endif
    }
}

/*****************************************************************************
 函 数 名  : OperationInfoQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
#define 	OPERATION_QUERY 72
AT+COPS?

<CR><LF>+COPS: <mode>[,<format>,<oper>[,<rat>]]
<CR><LF>OK<CR><LF>
有MS相关错误时：
<CR><LF>+CME ERROR: <err><CR><LF>
/*                                                                  
+COPS: 0,2,"46000",0                                                            
                                                                                
OK 修改历史      :
  1.日    期   : 2008年1月17日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void OperationInfoQuery(char *pszInfoBack)
{
	int lMsgId;
	char *pChar = NULL;
	int i = 0;
	struct stMsg_Operation_Info_Query stOperInfo;
	char acCharParam[COMMON_VERY_SHORT_STRING_LENGTH];
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	g_stAtToOther.lMsgType = OPERATION_QUERY;
	memset(&stOperInfo, 0, sizeof(stOperInfo));
	stOperInfo.ucCmdFrom = AT_MODULE;

    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	
	pChar = strstr(pszInfoBack, "+COPS:");
	if ( NULL != pChar )
	{
		stOperInfo.ucResult = HSPA_OK;
		pChar += strlen("+COPS:");
		cx_printf("%s,%d,%s(),pChar = %s\n", __FILE__,  __LINE__, __FUNCTION__, pChar);
		stOperInfo.ucChooseMode = StrStrToUnChar(&pChar,  acCharParam);
		if ( 0 != strncmp(pChar, "\r\n", 2 ) )
		{
			cx_printf("%s,%d,%s(),pChar = %s\n", __FILE__,  __LINE__, __FUNCTION__, pChar);
			stOperInfo.ucOperationFormat = StrStrToUnChar(&pChar,  acCharParam);
			pChar += strlen(",\"");
			cx_printf("%s,%d,%s(),pChar = %s\n", __FILE__,  __LINE__, __FUNCTION__, pChar);
			while(1)
			{
				if ( '"' == *pChar )
				{
					stOperInfo.acOperationInfo[i] = 0;
					break;
				}
				stOperInfo.acOperationInfo[i] = *pChar;
				++pChar;
				++i;
			}
			cx_printf("%s,%d,%s(),pChar = %s\n", __FILE__,  __LINE__, __FUNCTION__, pChar);
			++pChar;
			stOperInfo.ucWifiTech = StrStrToUnChar(&pChar,  acCharParam);
			cx_printf("%s,%d,%s(),pChar = %s\n", __FILE__,  __LINE__, __FUNCTION__, pChar);
		}
		
	}
	else
	{
		stOperInfo.ucResult = HSPA_ERROR_UNKNOW;
	}
	memcpy(g_stAtToOther.acText, &stOperInfo, sizeof(stOperInfo));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : CardModeQuery
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
	<CR><LF>^CARDMODE:<sim_type><CR><LF><CR><LF>OK <CR><LF> 修改历史      :
  1.日    期   : 2008年1月17日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void CardModeQuery(char *pszInfoBack)
{
	int lMsgId;
	char *pChar;
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	g_stAtToOther.lMsgType = CARDMODE_QUERY;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
	
    HspaMatchRead(pszInfoBack);

	pChar =  strstr(pszInfoBack, "^CARDMODE:");
	if ( NULL != pChar )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
		pChar += strlen("^CARDMODE:");
		if ( isdigit(*pChar))
		{
			g_stAtToOther.acText[2] = *pChar - '0';			
		}
		else
		{
			++pChar;
			g_stAtToOther.acText[2] = *pChar - '0';			
		}
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    
    g_sndflg = 0;
	sem_post(&sem);
	return;
}
/*****************************************************************************
 函 数 名  : ChupBack
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月21日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void ChupBack(char *pszInfoBack)
{
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    g_iCallInFlag = VOICE_CALLIN_IDLE;
    g_sndflg = 0;
	/* BEGIN: Added by c106292, 2009/3/16   PN:Led light method changes*/
	g_ulTrafficFlag	&= ~CALL_ON_HSPA;

	/*c106292*/

	if( 2 == getWanStatus())
	{
        sysLedCtrl(kLedHspa, kLedStateOff);
        sysLedCtrl(kLedHspa, kLedStateVerySlowBlinkContiunes);
	}
	else
	{
	    sysLedCtrl(kLedHspa, kLedStateOff);
	    sysLedCtrl(kLedHspa, kLedStateOn);
	}
	/* END:   Added by c106292, 2009/3/16 */
	sem_post(&sem);
	return;
}

/*****************************************************************************
 函 数 名  : HexToNum
 功能描述  : 将一个16进制的字符转换为对应的无符号
 				数值
 输出参数  : 无
 返 回 值  : unsigned long int 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月25日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
 unsigned long int HexToNum( char cChar)
 {
 	switch(cChar)
 	{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'a':
			return 10;
		case 'A':
			return 10;
		case 'b':
			return 11;
		case 'B':
			return 11;
		case 'c':
			return 12;
		case 'C':
			return 12;
		case 'd':
			return 13;
		case 'D':
			return 13;
		case 'e':
			return 14;
		case 'E':
			return 14;
		case 'f':
			return 15;
		case 'F':
			return 15;
		default:
			return 0;
 	}
 }
/*****************************************************************************
 函 数 名  : aHexToUl
 功能描述  : 将一个16进制字符串转换为对应的无符号整数
 输入参数  : char * pszChar  
 输出参数  : 无
 返 回 值  : unsigned long int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月25日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
unsigned long int  aHexToUl(char * pszChar)
 {
 	#define HEX_NUM_LENGHT 20
	#define LONG_UP	16
 	char *p = NULL;
	char acHex[HEX_NUM_LENGHT] = {0};
	int i ;
	int lLen = strlen(pszChar);
	unsigned long ulResult = 0;
	unsigned long ulCom = 0;
	if ( 0 == lLen)
	{
		return 0;
	}
	//判断是否是16进制数字符串
	p = pszChar;
	for(i=0; i<lLen; ++i)
	{
		if ( 0 == isxdigit(*p))
			return 0;
		++p;
	}

	//找到非零的最高位
	p = pszChar;
	for(i=0; i<lLen; ++i)
	{
		if ( '0' == *p)
		{
			++p;
			continue;
		}
		else
		{
			break;
		}
	}
	strcpy(acHex, p);
	lLen = strlen(acHex);
	if ( LONG_UP < lLen )
	{
		return 0;
	}
	for ( i=0; i < lLen; ++i)
	{
		ulCom = HexToNum( acHex[i] );
		ulCom <<= ((lLen-i-1)<<2);
		ulResult += ulCom;
	}
 	return ulResult;
 }

/*****************************************************************************
 函 数 名  : AutoDataFlow
 功能描述  : 计算流量上报，两个16位的没有计算
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
格式:^DSFLOWRPT:00000004,00000000,00000000,0000000000000080,000000000000017C,0003E800,10106803
 修改历史      :
  1.日    期   : 2008年1月25日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/

static void AutoDataFlow(char *pszInfoBack)
{
	unsigned long  ulQosTxRate = 0;
    unsigned long  ulQosRxRate = 0;
    unsigned long  ulTxRate = 0;
    unsigned long  ulRxRate = 0;
	char *pChar;
	int fd;
	char acCharParam[COMMON_SHORT_STRING_LENGTH] = {0};

	memset(acCharParam, 0, COMMON_SHORT_STRING_LENGTH);
	pChar = index(pszInfoBack, ':');
	if( NULL == pChar )
	{
		return;
	}
#if 0
	++pChar;
	strncpy(acCharParam, pChar, 8);
	g_stHspa.stDataFlowInfo.ulCurrDsTime = aHexToUl(acCharParam);

	pChar += 9;
	strncpy(acCharParam, pChar, 8);
	g_stHspa.stDataFlowInfo.ulTxRate = aHexToUl(acCharParam);

	pChar += 9;
	strncpy(acCharParam, pChar, 8);
	g_stHspa.stDataFlowInfo.ulRxRate = aHexToUl(acCharParam);
	pChar += 43;
#endif

/*start of HG_Support 2008.10.21 HG556AV100R001C01B020 AU8D00943 */
#if 1
        pChar += 10;
        strncpy(acCharParam, pChar, 8);
        ulTxRate = aHexToUl(acCharParam);
        //printf("TX:acCharParam = %s\t ulTxRate = %d\n",acCharParam,ulTxRate);
        if(ulTxRate != g_stHspa.stDataFlowInfo.ulTxRate)
        {
            g_stHspa.stDataFlowInfo.ulTxRate = ulTxRate;
            /*更新共享文件*/
            lseek(g_ShareiFd, 0, SEEK_SET);
            write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
            fsync(g_ShareiFd);
        }
        
        pChar += 9;
        strncpy(acCharParam, pChar, 8);
        ulRxRate = aHexToUl(acCharParam);
        //printf("RX:acCharParam = %s\t ulRxRate = %d\n",acCharParam,ulRxRate);
        if(ulRxRate != g_stHspa.stDataFlowInfo.ulRxRate)
        {
            g_stHspa.stDataFlowInfo.ulRxRate = ulRxRate;
            /*更新共享文件*/
            lseek(g_ShareiFd, 0, SEEK_SET);
            write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
            fsync(g_ShareiFd);
        }
#endif
/*end of HG_Support 2008.10.21 HG556AV100R001C01B020 AU8D00943 */


    pChar = index(pszInfoBack, ':');
	pChar += 62;
	strncpy(acCharParam, pChar, 8);
	ulQosTxRate = aHexToUl(acCharParam);
	//g_stHspa.stDataFlowInfo.ulQosTxRate = aHexToUl(acCharParam);
	if ( ulQosTxRate != g_stHspa.stDataFlowInfo.ulQosTxRate)
	{
		g_stHspa.stDataFlowInfo.ulQosTxRate = ulQosTxRate;
		lseek(g_ShareiFd, 0, SEEK_SET);
		write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
		fsync(g_ShareiFd);

		if( (fd = open( "/dev/brcmboard", O_RDWR )) != -1 )
	    {
	        ioctl( fd, BOARD_IOCTL_WAKEUP_MONITOR_TASK, NULL);
	        close(fd);
	    }
	}
	/*if ( g_stHspa.stDataFlowInfo.ulQosTxRate != ulQosTxRate )
	{
		int fd = -1;
		g_stHspa.stDataFlowInfo.ulQosTxRate = ulQosTxRate;
		if( (fd = open( "/dev/brcmboard", O_RDWR )) != -1 )
		{
		    ioctl( fd, BOARD_IOCTL_WAKEUP_MONITOR_TASK, NULL);
		    close(fd);
		}
	}*/

/* commented by tanyin 2009.5.20*/
#if 0
	/* BEGIN: Modified by c106292, 2009/4/15   PN: */
	if (  CALL_ON_HSPA & g_ulTrafficFlag )
	{   
        //电话中
	    sysLedCtrl(kLedHspa, kLedStateFastBlinkContinues);
	}
	else if( (ulRxRate!=0) ||( ulTxRate!=0 ) ) 
	{
	    sysLedCtrl(kLedHspa, kLedStateVerySlowBlinkContiunes);
	}
	else
	{
	    sysLedCtrl(kLedHspa, kLedStateOff);
	    sysLedCtrl(kLedHspa, kLedStateOn);
	}
/* END:   Modified by c106292, 2009/4/15 */
#endif

#if 0
	pChar += 9;
	strncpy(acCharParam, pChar, 8);
	g_stHspa.stDataFlowInfo.ulQosRxRate = aHexToUl(acCharParam);
	/*更新共享文件*/
	lseek(g_ShareiFd, 0, SEEK_SET);
	write(g_ShareiFd, &g_stHspa, sizeof(g_stHspa));
	fsync(g_ShareiFd);
#endif
}
/*****************************************************************************
 函 数 名  : ImeiInfoAnlysis
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
指令和反馈信息格式:
*IMEI_QUERY	74----AT^SN\r
*NORMAL:8----AT^SN<CR>,  25----<CR><LF>^SN: EV2AB10810301426<CR><LF><CR><LF>OK<CR><LF>
*UNNORMAL:<CR><LF>+CME ERROR: <err><CR><LF>
 修改历史      :
  1.日    期   : 2008年1月7日
    作    者   : 曹祥
    修改内容   : 新生成函数

*****************************************************************************/
static void SnQuery(char *pszInfoBack) 
{
 	struct stMsg_SN_Query stSn;
	int lMsgId;
	int lLen;
	memset(&stSn, 0, sizeof(stSn));
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	g_stAtToOther.lMsgType = SN_QUERY;/*设置消息对应的命令*/
	stSn.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
       pszInfoBack = strstr(pszInfoBack, "^SN:"); /*兼容K3772 HSPA,去掉"^SN: "中的空格*/
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stSn.ucResult = HSPA_OK;
        /*Begin:Modefid by luokunling l00192527,2012/2/2*/
		lLen = lLen-strlen("^SN:")-strlen("\r\n\r\nOK\r\n");  /*为获取序列号长度，需减去字符串头"^SN:"长度
                                                                                 之前减去末尾"\r\n\r\n\r\nOK\r\n"长度 有误, 修改为减去"\r\n\r\nOK\r\n"长度*/
        /*End:Modefid by luokunling l00192527,2012/2/2*/
		if ( lLen >= sizeof(stSn.acSN) )
			lLen = sizeof(stSn.acSN);
        /*Begin:Modefid by luokunling l00192527,2012/2/2*/
		memcpy(stSn.acSN, pszInfoBack+strlen("^SN:"), lLen); /*兼容K3772 HSPA,去掉"^SN: "中的空格*/
        /*End:Modefid by luokunling l00192527,2012/2/2*/
	}	
	else
	{
		stSn.ucResult = HSPA_ERROR_UNKNOW;
	}	

	memcpy(g_stAtToOther.acText, (void*)&stSn, sizeof(stSn));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;

}

static void CgactQuery(char *pszInfoBack) 
{
	struct stMsg_CGACT_Query stSn;
	int lMsgId;
	int lLen;
	char* pcTmp = 0;
	memset(&stSn, 0, sizeof(stSn));
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
	g_stAtToOther.lMsgType = CGACT_QUERY;/*设置消息对应的命令*/
	stSn.ucCmdFrom = AT_MODULE;/*设置消息来源*/
	/*判断命令回显和反馈信息是否同时读出
	若不是在一起，需要读取反馈信息*/
    	HspaMatchRead(pszInfoBack);
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);

	pszInfoBack = strstr(pszInfoBack, "\r\n");
    	pszInfoBack = strstr(pszInfoBack, "+CGACT: ");
	cx_printf("%s,%d,%s(),pszInfoBack = %s\n", __FILE__,  __LINE__, __FUNCTION__, pszInfoBack);
    
	if ( (NULL != pszInfoBack) && (NULL != strstr(pszInfoBack, "\r\nOK\r\n")) )/*判断反馈信息是否成功*/
	{
		lLen = strlen(pszInfoBack);
		stSn.ucResult = HSPA_OK;

/*
+CGACT: 1,0
OK
*/
		/* not support query cid */
		stSn.ucCid = 0;

		pcTmp = strstr(pszInfoBack, "\r\n");
		*pcTmp = '\0';
		pszInfoBack = strstr(pszInfoBack, ",");
        TTY_DEBUG("...........pszInfoBack is %s\n", pszInfoBack);
		
		stSn.ucState = atoi(pszInfoBack+1);

        TTY_DEBUG("...........ucCmdFrom %d ucResult %d ucCid %d ucState %d\n", stSn.ucCmdFrom, stSn.ucResult, stSn.ucCid, stSn.ucState);


	}	
	else
	{
		stSn.ucResult = HSPA_ERROR_UNKNOW;
	}	

	memcpy(g_stAtToOther.acText, (void*)&stSn, sizeof(stSn));
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;

}

/*****************************************************************************
 函 数 名  : VoiceEnableSet
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年8月15日
    作    者   : s48571
    修改内容   : 新生成函数
*****************************************************************************/
static void VoiceEnableSet(char *pszInfoBack)
{
	int lMsgId;

    HspaMatchRead(pszInfoBack);
	if ( NULL != strstr(pszInfoBack,  "\r\nOK\r\n") )
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_OK;
	}
	else
	{
		g_stAtToOther.acText[HSPA_RESULT_POSITION] = HSPA_ERROR_UNKNOW;
	}
	g_stAtToOther.lMsgType = VOICE_SET;
	g_stAtToOther.acText[HSPA_FROM_POSITION] = AT_MODULE;
	lMsgId = MsgIdCompute(g_stAtFromOther.acParam[HSPA_FROM_POSITION]);
	/*若是控制模块自身的消息，不予反馈*/
	if ( (-1 != lMsgId) && (g_lAtMsgId!= lMsgId) )
	{
        TTY_DEBUG("lMsgId is %d\n", lMsgId);
		msgsnd(lMsgId,  (void*)&g_stAtToOther,AT_RCV_MSG_LENGTH, IPC_NOWAIT);
	}
    g_sndflg = 0;
	sem_post(&sem);
	return;

}


void TTY_WriteDebug(char *pcTmp)
{
    FILE *fs = NULL;

    if(NULL == pcTmp)
    {
        return;
    }
    
    if(NULL == (fs = fopen(TTYDEBUGPATH, "a+")))
    {
        fprintf(stdout, "%s", pcTmp);
    }
    else
    {
        fprintf(fs, "%s", pcTmp);
        fclose(fs);
    }
    return ;
}

int TTY_Output(const char* fmt,...)
{
    va_list argptr;
    char pcTmp[2048];
    int fd = 0;
    
    memset(pcTmp, 0, sizeof(pcTmp));
    if(!debug)
    {
        return 0;
    }
    
    va_start(argptr, fmt);
    vsprintf(pcTmp, fmt, argptr);
    va_end(argptr);
  
    if(OutputTerm == 2)
    {
        printf("%s",pcTmp);
        return 1;
    }
    else if(OutputTerm == 1)
    {        
        TTY_WriteDebug(pcTmp);
        return 1;
    }
    else
    {
        printf("%s",pcTmp);
        return 1;
    }

}

int debugswitch(void)
{
    FILE *fp = NULL;
    char acCmd[16] = "";
    struct stAtSndMsg stAtFromOther;

    if(NULL == (fp = fopen(TTYDEBUG, "r")))
    {
        return;
    }
    fgets(acCmd, 16, fp);
    fclose(fp);

    if (debug == atoi(acCmd))
    {
    		AtSend(g_lAtMsgId, &stAtFromOther, AT_MODULE, RSSI_QUERY, "AT+CSQ");
		return;
    }

    debug = atoi(acCmd);
    if(debug)
    {
        OutputTerm = debug;
        debug = 1;
        printf("\r\TTY debug switch turn on!\n");
        TTY_LOG( "g_lAtMsgId = %d\n", g_lAtMsgId);
        TTY_LOG( "g_lModemMsgId = %d\n", g_lModemMsgId);
        TTY_LOG( "g_lVoiceMsgId = %d\n", g_lVoiceMsgId);
        TTY_LOG( "g_lTr069MsgId = %d\n", g_lTr069MsgId);
        TTY_LOG( "g_lHttpMsgId = %d\n", g_lHttpMsgId);
    }
    else
    {
        printf("\r\TTY debug switch turn off!\n");
    }
}

static int RegisterNtwk(void)
{
    FILE* fs = NULL;
    unsigned long ulConType = 0;
    int lLen = 0;
    char acChannel[AT_SND_MSG_LENGTH];
    char acSysCfgCmd[AT_SND_MSG_LENGTH];
    char acOperator[COMMON_SHORT_STRING_LENGTH];
    unsigned long ulVoiceEnable = ENABLE_VOICE;//默认voice功能开启。

    memset(acChannel, 0, sizeof(acChannel));
    memset(acSysCfgCmd, 0, sizeof(acSysCfgCmd));
    memset(acOperator, 0,  sizeof(acOperator));

    /*读取网络注册信息*/
    fs = fopen(HSPA_SYSCFG_FILE, "r");

    if (!fs)
    {
        printf("FILE %s : open hspasys.cfg failed. \r\n", __FILE__);
        return -1;
    }

    fscanf(fs, HSPA_SYS_CFG_FORMAT_R, &ulConType, acChannel, &ulVoiceEnable, acOperator);

    fclose(fs);

    printf("[TTYUSB]ConType:%d bands:%s Operator:%s\n", ulConType, acChannel, acOperator);

    if (acChannel[0] == 0)
    {
        strcpy(acChannel, "3FFFFFFF");
    }
    memset(acSysCfgCmd, 0, sizeof(acSysCfgCmd));
    /*校验配置信息正确性*/
    if (ulConType == ATCFG_CONNTYPE_GPRS_FIRST)
    {
        sprintf(acSysCfgCmd, "AT^SYSCFG=2,1,%s,2,4\r", acChannel);
    }
    else if (ulConType == ATCFG_CONNTYPE_3G_FIRST)
    {
        sprintf(acSysCfgCmd, "AT^SYSCFG=2,2,%s,2,4\r", acChannel);
    }
    else if (ulConType == ATCFG_CONNTYPE_GPRS_ONLY)
    {
        sprintf(acSysCfgCmd, "AT^SYSCFG=13,3,%s,2,4\r", acChannel);
    }
    else if (ulConType == ATCFG_CONNTYPE_3G_ONLY)
    {
        sprintf(acSysCfgCmd, "AT^SYSCFG=14,3,%s,2,4\r", acChannel);
    }
    else if (ulConType == ATCFG_CONNTYPE_AUTO)
    {
        sprintf(acSysCfgCmd, "AT^SYSCFG=2,0,%s,2,4\r", acChannel);
    }

    /* 注册网络 */
    if (g_iPcuiFd && (acSysCfgCmd[0] != 0))
    {
        printf("[TTYUSB]Register network:%s\n", acSysCfgCmd);
        lLen = write(g_iPcuiFd, acSysCfgCmd, strlen(acSysCfgCmd));
        usleep(100);
		lLen = read(g_iPcuiFd, acSysCfgCmd, COMMON_LONG_STRING_LENGTH);
    }

    /* add operator selection. <tanyin 2009.4.9> */
    if (*acOperator != 0 && strcmp(acOperator, "auto") != 0)
    {
        if (atoi(acOperator)) {
            strcpy(acSysCfgCmd,"AT+COPS=1,2,\""); /* MCC/MNC */
        } else {
            strcpy(acSysCfgCmd,"AT+COPS=1,1,\""); /* Short network name */
        }
        strcat(acSysCfgCmd,acOperator);
        strcat(acSysCfgCmd,"\"");
    }
    else /* auto search and register */
    {
        strcpy(acSysCfgCmd,"AT+COPS=0");       
    }
    /* 选择运营商 */
    if (g_iPcuiFd)
    {
        printf("[TTYUSB]Select operator:%s\n", acSysCfgCmd);
        lLen = write(g_iPcuiFd, acSysCfgCmd, strlen(acSysCfgCmd));
        sleep(5);
		lLen = read(g_iPcuiFd, acSysCfgCmd, COMMON_LONG_STRING_LENGTH);
        printf("[TTYUSB]result:%s\n", acSysCfgCmd);
        return 0;
    }

    return -1;
}

/*****************************************************************************
 函 数 名  : VoiceJudge
 功能描述  : 
 输入参数  : char *pszInfoBack  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年8月15日
    作    者   : s48571
    修改内容   : 新生成函数
*****************************************************************************/
static int VoiceJudge(void)
{
    FILE* fs = NULL;
    unsigned long ulConType = 0;
    unsigned long ulVoiceEnable = ENABLE_VOICE;//默认voice功能开启。
    int lLen = 0;
    char acChannel[AT_SND_MSG_LENGTH];
    char acCmd[COMMON_LONG_STRING_LENGTH];
    char acOperator[COMMON_SHORT_STRING_LENGTH];

    memset(acChannel, 0, sizeof(acChannel));
    memset(acCmd, 0, sizeof(acCmd));

    /*读取网络注册信息*/
    fs = fopen(HSPA_SYSCFG_FILE, "r");

    if (!fs)
    {
        printf("FILE %s : open hspasys.cfg failed. \r\n", __FILE__);
        return -1;
    }

    fscanf(fs, HSPA_SYS_CFG_FORMAT_R, &ulConType, acChannel,&ulVoiceEnable,acOperator);

    fclose(fs);

    printf("\nTTYUSB:VoiceEnable[%d]\n", ulVoiceEnable);

    if( -1 == g_iPcuiFd)
    {
        printf("\nTTYUSB:PCUI devie fd error\n");
        return -1;
    }
    /*校验配置信息正确性*/
    if ( ENABLE_VOICE == ulVoiceEnable )
    {
        lLen = write(g_iPcuiFd, "AT^CVOICE=0\r", strlen("AT^CVOICE=0\r"));
        usleep(100);
        lLen = read(g_iPcuiFd, acCmd, COMMON_LONG_STRING_LENGTH);
    }
    else if (DISABLE_VOICE == ulVoiceEnable )
    {
        lLen = write(g_iPcuiFd, "AT^CVOICE=1\r", strlen("AT^CVOICE=1\r"));
        usleep(100);
        lLen = read(g_iPcuiFd, acCmd, COMMON_LONG_STRING_LENGTH);
    }

    return 0;
}



