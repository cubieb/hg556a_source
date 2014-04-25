/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : at_sm.c
  作者     : lichangqing 45517
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : at状态机的实现
      
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>

#include <termios.h>
#include <unistd.h>
#include <setjmp.h>

#include "pppd.h"
#include "at_sm.h"
#include "at_thread.h"
#include "utils_lib.h"

#include "ttyUSB.h"
#include <sys/ipc.h>
#include <sys/msg.h>

static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg);
static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg);

static void  at_create_pdp_context_cmd(char* buf, char* param);
static void  at_create_syscfg_cmd(char* buf, char* param);


struct stMsg_RSSI_Query			rssi_get = {0,0,0,0};
struct stMsg_System_Info_Query	sys_info_get = {0,0,0,0,0,0,0,0,0};
struct stMsg_Syscfg_Set			cfg_set = {0,0};
struct stMsg_PDP_Set				pdp_set = {0,0};
struct stMsg_Service_Status		rp_service_sts = {0,0,0};
struct stMsg_System_Mode			rp_sys_mode = {0,0,0,0};

int AtSndMsgId = -1;
int AtRcvMsgId = -1;

AT_HANDLE_T  g_at_handles[] =
{
	{RSSI_QUERY,&rssi_get,at_report_handler_rssi,"AT+CSQ",NULL},
	{SYSTEM_INFO_QUERY,&sys_info_get,at_report_handler_sysinfo,"AT^SYSINFO",NULL},
	{SYSCFG_SET,&cfg_set,at_report_handler_cfg_set,"AT^SYSCFG=",at_create_syscfg_cmd},
	{PDP_SET,&pdp_set,at_report_handler_pdp_set,"AT+CGDCONT=",at_create_pdp_context_cmd},
	{AUTO_SERVICE_STATUS_REPORT,&rp_service_sts,at_report_handler_service_sts,NULL,NULL},
	{AUTO_SYSTEM_MODE_STATUS_REPORT,&rp_sys_mode,at_report_handler_sys_mode,NULL,NULL},
	{0,NULL,NULL,NULL,NULL},
};

/*
// 为每条AT命令分配的序号
#define AT_RSSI_QUERY    				 0	 // RSSI查询功能命令			RSSI_QUERY
#define AT_SYSINFO_QUERY			 1	 // 系统信息查询				SYSTEM_INFO_QUERY
#define AT_SYSCFG_SET  				 2 	//设置连接类型				SYSCFG_SET
#define AT_PDP_SET					 3	//定义PDP关联					PDP_SET
#define AT_AUTO_SERVICE_REPORT 		 4	//服务状态变化指示		AUTO_SERVICE_STATUS_REPORT
#define AT_AUTO_SYSTEM_REPORT		 5	//系统模式变化事件指示 	AUTO_SYSTEM_MODE_STATUS_REPORT

*/
int g_i_at_rcv_thread_killed = 0; // 该变量目前未实现，将在signal处理程序中实现
//int g_pcui_fd  = -1;

THREAD_S         g_st_at_report_thread;  // 线程描述结构
static WAIT_S    g_st_submit_at_wait;    // 等待上报接收线程唤醒的事件
static int       g_i_is_sm_active;       // 是否有线程下正在下发AT命令;
//MUTEX_S          g_st_cur_at_num_mutex;  // 与上报线程同步的mutex;
//MUTEX_S          g_st_pcui_wr_mutex;     // 与上报线程同步的mutex;

//static int       g_i_cur_at_num        = -1;    // 暂存 at_sm_submit_at提交的at命令序号
void*            g_at_result           = NULL;  // 暂村 at_sm_submit_at提交的返回结果指针
int              g_i_is_report_ok;              // 若AT上报error或上报格式错误值为0，否则为1
//static           sigjmp_buf g_sigjmp;
static int       rcv_waiting = 0;               // 上报接收线程处于等待上报状态

extern int g_main_thread_killed;

/*------------------------------------------------------------
  函数原型: static int  at_create_pdp_context_cmd(char* buf, char* parameters)
  描述: 用来创建AT命令,
  输入: AT命令需要的参数
  输出: 无
  返回值:生成的AT命令
-------------------------------------------------------------*/
static void  at_create_pdp_context_cmd(char* buf, char* param)
{
    sprintf(buf,"AT+CGDCONT=%s", param);
}


/*------------------------------------------------------------
  函数原型: static int  at_create_syscfg_cmd(char* buf, char* parameters)
  描述: 用来创建AT命令,
  输入: AT命令需要的参数
  输出: 无
  返回值:生成的AT命令
-------------------------------------------------------------*/
static void  at_create_syscfg_cmd(char* buf, char* param)
{
    sprintf(buf,"AT^SYSCFG=%s", param);
}

#if 1
/*------------------------------------------------------------
  函数原型: static inline int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)  
  描述: 接收HSPA接口模块的消息
  输入: lMsgId:消息ID；pstAtRcvMsg :消息结构体指针;lMsgType:消息类型
  输出: 无
  返回值:
-------------------------------------------------------------*/

int AtRcv(int lMsgId, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    /* BEGIN: Modified by y67514, 2008/6/20   问题单号:优化AT收发机制*/
    int iRetryTimes = 0;
    struct stAtRcvMsg AtRcvMsg;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
    memset(&AtRcvMsg, 0, sizeof(AtRcvMsg));
  
    /* 若接收消息失败，重试10次 */
    while (-1 == msgrcv(lMsgId,(void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (15 <= iRetryTimes)
        {
            printf("\n! ! !NO RESPONSE FOR AT TYPE %d ! ! !\n",lMsgType);
            return -1;
        }
        __msleep(1000);
    }
    
    while (-1 != msgrcv(lMsgId,(void*)&AtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        memcpy(pstAtRcvMsg,&AtRcvMsg,sizeof(*pstAtRcvMsg));
        __msleep(1000);
    }
    return 0;
    /* END:   Modified by y67514, 2008/6/20 */
}


/*------------------------------------------------------------
  函数原型:static inline int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
  描述: 向HSPA接口模块发送消息
  输入: lMsgId:消息ID；pstAtSndMsg :消息结构体指针;cFrom:消息来源;lMsgType:消息类型;pszParam:AT指令参数
  输出: 无
  返回值:
-------------------------------------------------------------*/

int AtSend(int lMsgId, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
{
	memset(pstAtSndMsg, 0, sizeof(*pstAtSndMsg));	
	pstAtSndMsg->lMsgType = lMsgType;
	pstAtSndMsg->acParam[0] = cFrom;
	//strcpy(pstAtSndMsg->acText+1, pszParam);
	if(NULL!= pszParam)
		strcpy(pstAtSndMsg->acParam+1, pszParam);
       //发送消息时使用非阻塞式的
	return msgsnd(lMsgId, (void*)pstAtSndMsg, AT_SND_MSG_LENGTH, IPC_NOWAIT);
}
#endif

/*------------------------------------------------------------
  函数原型: static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的rssi参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_rssi (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(RSSI_QUERY != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}


	rssi_get.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rssi_get.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rssi_get.ucResult)
	{
		return PARSE_ERROR;
	}
	rssi_get.ucRSSI=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型: static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的sysinfo参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_sysinfo (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(SYSTEM_INFO_QUERY != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	sys_info_get.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(sys_info_get.ucResult)
	{
		return PARSE_ERROR;
	}
	sys_info_get.ucSrvStatus = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSrvDomain = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucRoamStatus = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSysMode = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSimState = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucLockState = (unsigned char)pstAtRcvMsg->acText[i++];
	sys_info_get.ucSysSubMode = (unsigned char)pstAtRcvMsg->acText[i++];
	VDF_DBG("VDF:%s:%s:%d:sysmode＝%d***\n",__FILE__,__FUNCTION__,__LINE__,sys_info_get.ucSysMode);
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型: static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的cfg_set参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_cfg_set (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(SYSCFG_SET != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	cfg_set.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	cfg_set.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(cfg_set.ucResult)
	{
		return PARSE_ERROR;
	}
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型: static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的pdp_set参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_pdp_set (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(PDP_SET != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	pdp_set.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	pdp_set.ucResult= (unsigned char)pstAtRcvMsg->acText[i++];
	if(pdp_set.ucResult)
	{
		return PARSE_ERROR;
	}
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型: static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的service_sts参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_service_sts (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(AUTO_SERVICE_STATUS_REPORT != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}


	rp_service_sts.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rp_service_sts.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rp_service_sts.ucResult)
	{
		return PARSE_ERROR;
	}
	rp_service_sts.ucSrvStatus=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型: static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg)
  描述: 用来解析返回的rssi参数
  输入: 接收到的消息结构
  输出: 无
  返回值:PARSE_OK 解析成功，PARSE_ERROR解析失败
-------------------------------------------------------------*/

static int at_report_handler_sys_mode (struct stAtRcvMsg *pstAtRcvMsg)
{
	int i = 0;
	if(AUTO_SYSTEM_MODE_STATUS_REPORT != pstAtRcvMsg->lMsgType)
	{
		return PARSE_ERROR;
	}

	rp_sys_mode.ucCmdFrom = (unsigned char)pstAtRcvMsg->acText[i++];
	
	rp_sys_mode.ucResult=  (unsigned char)pstAtRcvMsg->acText[i++];
	if(rp_sys_mode.ucResult)
	{
		return PARSE_ERROR;
	}
	rp_sys_mode.ucSysMode=  (unsigned char)pstAtRcvMsg->acText[i++];
	rp_sys_mode.ucSubSysMode=  (unsigned char)pstAtRcvMsg->acText[i++];
	
	return PARSE_OK;

}

/*------------------------------------------------------------
  函数原型 : int at_sm_error_handler(char* report_buf, int len, int err_num)
  描述     : 对CM500端上报错误消息的处理
  输入     : report_buf、len同report_handler_0, err_num为出错类型号
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
void at_sm_error_handler()
{
    
    g_i_is_report_ok = AT_REPORT_ERR;
    __wakeup(&g_st_submit_at_wait);
}


// A064D00428 EC506 ADD (by l45517 2005年11月21?) BEGIN
void at_sm_modem_busy_handler()
{
    
    g_i_is_report_ok = AT_REPORT_BUSY;
    __wakeup(&g_st_submit_at_wait);
}
// A064D00428 EC506 ADD (by l45517 2005年11月21?) END


/*------------------------------------------------------------
  函数原型 : int at_sm_is_active()
  描述     : 是否有线程在下发AT命令
  输入     : 无
  输出     : 无
  返回值   : 0(表示AT状态机空闲); 1(有线程在下发AT命令)
-------------------------------------------------------------*/
int at_sm_is_active()
{
    return g_i_is_sm_active;
}

/*------------------------------------------------------------
  函数原型 : int at_sm_submit_at(int at_num, void* res, int* is_report_err,char* parameters)
  描述     : 通过pcui口下发一条AT命令
  输入     : at_num AT命令号;parameters AT命令需要用到的参数
  输出     : res : 接受AT上报的结果; is_report_err: 返回cm500端是否上报了error消息或上报格式错误
  返回值   : 1 : 表示超时;　0 : AT命令下发成功
-------------------------------------------------------------*/
int at_sm_submit_at(int at_num, int* is_report_err,char* param)
{
	struct stAtSndMsg AtSndMsg;
	struct stAtRcvMsg AtRcvMsg;
	char atbuf[AT_SND_MSG_LENGTH];
	int ret = 1;
	int iRetryTimes = 0;
	//char buf[64];//存放AT命令

	g_i_is_sm_active = 1;
	g_i_is_report_ok = AT_REPORT_OK;
	
	memset(atbuf,0,AT_SND_MSG_LENGTH);
	if (g_at_handles[at_num].create_at_cmd == NULL)
	{
		/*AT生成函数为空，表明AT命令为一般形式*/
		strcpy(atbuf, g_at_handles[at_num].at_cmd_str); //, strlen(g_at_handles[at_num].at_cmd_str));   
	}
	else
	{ 
		/*AT生成函数不为空，表明AT命令格式特殊，需要特殊函数生成*/
		g_at_handles[at_num].create_at_cmd(atbuf, param);
	}
	
	AtSndMsgId = msgget(MSG_AT_QUEUE, 0666);
	while(-1 == AtSndMsgId)
	{
		VDF_DBG("VDF:%s:%s:%d:can't get the AtSndMsgId***\n",__FILE__,__FUNCTION__,__LINE__);
		__msleep(5*1000);
		AtSndMsgId = msgget(MSG_AT_QUEUE, 0666);
	}

	while(-1 ==AtSend(AtSndMsgId,&AtSndMsg,MODEM_MODULE,g_at_handles[at_num].at_cmd_id,atbuf))
	{
		VDF_DBG("VDF:%s:%s:%d:AtSend ERRO:msgid=%d,atnum=%d,param=%s***\n",__FILE__,__FUNCTION__,__LINE__,AtSndMsgId,g_at_handles[at_num].at_cmd_id,atbuf);
		//g_i_is_sm_active = 0;
		//return AT_TIMEOUT;
		__msleep(5*1000);
	}

	//g_i_cur_at_num = at_num;
	//ret = __sleep_wait(&g_st_submit_at_wait, 7 * 1000);
	if (g_main_thread_killed) 
	{
		ret = 0;
	}

	AtRcvMsgId = msgget(MSG_MODEM_QUEUE, 0666);
	while(-1 == AtRcvMsgId)
	{
		VDF_DBG("VDF:%s:%s:%d:Can't get the AtRcvMsgId***\n",__FILE__,__FUNCTION__,__LINE__);
		__msleep(5*1000);
		AtRcvMsgId = msgget(MSG_MODEM_QUEUE, 0666);
	}
	VDF_DBG("VDF:%s:%s:%d:AtRcvMsgId=%d***\n",__FILE__,__FUNCTION__,__LINE__,AtRcvMsgId);
        __msleep(200);
        /* BEGIN: Modified by y67514, 2008/6/20   问题单号:优化AT收发机制*/
        if(-1 == AtRcv(AtRcvMsgId,&AtRcvMsg,g_at_handles[at_num].at_cmd_id))
        {
            g_i_is_sm_active = 0;
            return AT_TIMEOUT;
        }   
        /* END:   Modified by y67514, 2008/6/20 */

	if(PARSE_ERROR == g_at_handles[at_num].at_cmd_report_handler(&AtRcvMsg))
	{
		*is_report_err = 1;
	}
	else
	{
		*is_report_err = 0;
	}
	

	// 与上报接收线程同步，避免当前线程返回后，上报线程使用g_at_result，造成堆栈错误；
	//__p(&g_st_cur_at_num_mutex);
	//g_i_cur_at_num = -1;    
	//g_at_result  = NULL;
	//__v(&g_st_cur_at_num_mutex);

	g_i_is_sm_active = 0;
	return (ret > 0) ? AT_OK : AT_TIMEOUT;
}


/*------------------------------------------------------------
  函数原型 : int at_sm_modem_submit_at(char* at_dial_str)
  描述     : 通过modem口拨号
  输入     : at_dial_str为下发的拨号字符串, modem_fd modem口文件句柄
  输出     : 无
  返回值   : 1 : 表示失败; 0 : 表示成功
-------------------------------------------------------------*/
int at_sm_modem_submit_at(int modem_fd, char* at_dial_str)
{
    int    ii = LOOP_TIMES;
    int    i_len;
    int    ret;
	    // A064D00411 EC506 ADD (by l45517 2005年11月18?) BEGIN
    char   ch[512], *pch = NULL;    // 用来接收数据的缓冲区，大小为512字节
    const char *report_msg;  // 上报批匹配消息
    const char *report_no_carrier = NULL;
    fd_set read_set;
    struct timeval tvSelect;
	memset(ch,0,512);
	//printf("***YP:%s:%s:%d:pid=%d,at_dial_str=%s***\n",__FILE__,__FUNCTION__,__LINE__,getpid(),at_dial_str);
    
    if (strcmp(at_dial_str, "ATH\r"))
    {
        //report_msg = "\r\nCONNECT\r\n";
        report_msg = "CONNECT"; /* less strict. <tanyin 2009.4.1> */
        report_no_carrier = "NO CARRIER";
    }
    else
    {
        report_msg = "\r\nOK\r\n";
    }
    // A064D00411 EC506 ADD (by l45517 2005年11月18?) END

    i_len = strlen(at_dial_str);
    while (ii)
    {
        ret = write(modem_fd, at_dial_str, i_len);
        if (-1 == ret)
        {
            if (EINTR == errno)
            {
                continue;
            }
            ERROR("modem : write AT dial command(%s) error", at_dial_str);
            VDF_DBG("AT:%s:%s:%d:write AT erro***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        ii--;
        if (ret != i_len)
        {
            if (0 == ii) // 如果下发5次都失败
            {
                ERROR("modem : submit AT dial command(%s) failed\n", at_dial_str);
                VDF_DBG("AT:%s:%s:%d:submit AT dial  failed***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            WARN("modem : have not submit whole at command for %d times", ii);
            continue;
        }
        break;
    }
    AT_SM_DEBUG("modem dial : write at cmd successful");

    // A064D00300 EC506 MOD (by l45517 2005年11月3日 ) BEGIN
    tvSelect.tv_sec  = 20; // 最常等待20秒
    // A064D00300 EC506 MOD (by l45517 2005年11月3日 ) END

    tvSelect.tv_usec = 0;

    pch = &ch[0];

    // A064D00411 EC506 ADD (by l45517 2005年11月18?) BEGIN
    while (1)
    {
        if (hungup)
        {
            VDF_DBG("AT:%s:%s:%d:hungup***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        
        while (1)
        {
            FD_ZERO(&read_set);
            FD_SET(modem_fd, &read_set);

            ret = select(modem_fd + 1, &read_set, NULL, NULL, &tvSelect);
            if (-1 == ret)
            {
                if (EINTR == errno)
                {
                    // A64D01038 ADD (by l45517 2006年4月11日 ) BEGIN
                    if (!persist)
                    {
                        break;
                    }
                    // A64D01038 ADD (by l45517 2006年4月11日 ) END
                    continue;
                }
                ERROR("modem : select modem error");
                VDF_DBG("AT:%s:%s:%d:select modem error***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            if (!ret)
            {
                //A64D01427 yangjianping begin
                WARN("modem : wait for report msg timeout \n");
                //A64D01427 yangjianping end
                VDF_DBG("AT:%s:%s:%d:wait for report msg timeout***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            break;
        }
        // A064D00503 EC506 ADD (by L45517 2005年12月2日 ) BEGIN
        INFO("select time is : sec = %d, ms = %d, us = %d , hungup <%d>", 
            tvSelect.tv_sec, (tvSelect.tv_usec / 1000), (tvSelect.tv_usec % 1000), hungup);
        // A064D00503 EC506 ADD (by L45517 2005年12月2日 ) END

        while (1)
        {
            ret = read(modem_fd, pch, 64); // 每次最多读出64字节
            if (-1 == ret)
            {
                if (EINTR == errno)
                {
                    // A64D01038 ADD (by l45517 2006年4月11日 ) BEGIN
                    if (!persist)
                    {
                        break;
                    }
                    // A64D01038 ADD (by l45517 2006年4月11日 ) END
                    continue;
                }
                ERROR("read modem_fd error\n");
                VDF_DBG("AT:%s:%s:%d:read modem_fd error***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            /* BEGIN: Added by y67514, 2008/11/29   PN:AU8D01263:HSPA上行PIN码未保存情况下，HSAP拨号不成功。*/
            if ( 0 == ret )
            {
                /*读到内容为空或读到文件尾*/
                VDF_DBG("AT:%s:%s:%d:modem_fd read nothing!!!\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
            /* END:   Added by y67514, 2008/11/29 */
            break;
        }
        // A64D01038 ADD (by l45517 2006年4月11日 ) BEGIN
        if (!persist)
        {
            VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
        // A64D01038 ADD (by l45517 2006年4月11日 ) END

        pch[ret] = '\0';
        pch += ret;

	//printf("***YP:%s:%s:%d:MODEM REPORT=%s***\n",__FILE__,__FUNCTION__,__LINE__,ch);
        if (strstr(ch, report_msg)) 
        {
            return AT_OK;
        }
        
       // printf("(sxg)%s %d, %s %s %s\n", __FILE__, __LINE__, ch, report_no_carrier, pch);
        
        if (report_no_carrier)
        {
            if (strstr(ch, report_no_carrier))
            {
                VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
                return AT_TIMEOUT;
            }
        }

        if (pch > &ch[512-64]) 
        {
            VDF_DBG("AT:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
            return AT_TIMEOUT;
        }
    }
    // A064D00411 EC506 ADD (by l45517 2005年11月18?) END

}


