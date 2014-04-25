/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : pppd_thread.c
  作者     : lichangqing 45517
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : 

  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

#include "pppd.h"
#include "utils_lib.h"
#include "at_sm.h"
#include "at_thread.h"

extern struct tagATConfig at_config;
/*------------------------------------------------------------
  函数原型 : int pppd_thread_at_dail(int modem_fd)
  描述     : AT拨号建立连接
  输入     : 无
  输出     : 无
  返回值   : 1:拨号失败；0:拨号成功
-------------------------------------------------------------*/
int pppd_thread_at_dail(int modem_fd)
{
    int sys_mode;
    int sys_rssi;
    int tmp;
    char buf[64];//存放AT命令


    memset(buf, 0, sizeof(buf));
    sprintf(buf, "ATD%s\r", at_config.phone_number ? at_config.phone_number:"*99#" );
    
    sys_mode = at_thread_cm500_sysmode();
    sys_rssi = at_thread_cm500_rssi();

    if (SYSMODE_WCDMA == sys_mode)
    {
		//printf("VDF:%s:%s:%d:SYSMODE_WCDMA == sys_mode***\n",__FILE__,__FUNCTION__,__LINE__);
        //if (at_sm_modem_submit_at(modem_fd, "ATD*99#\r"))
        if (at_sm_modem_submit_at(modem_fd, buf))		/*向Modem口下发AT指令*/
        {
            __msleep(500);
            WARN("at dial : retrying ...");
            //if (at_sm_modem_submit_at(modem_fd, "ATD*99#\r"))
            if (at_sm_modem_submit_at(modem_fd, buf))
            {
                WARN("at dial : failed");
                return AT_TIMEOUT;
            }
        }
    }
    else if (SYSMODE_NO_SERVICES != sys_mode)
    {
		//printf("VDF:%s:%s:%d:SYSMODE_NO_SERVICES != sys_mode***\n",__FILE__,__FUNCTION__,__LINE__);
        WARN("AT dial %s ... ...", buf);
        if (!sys_rssi)
        {
            WARN("at dial failed rssi<%d>", sys_rssi);
            //return AT_TIMEOUT; //modify by sxg, ignore rssi value
        }
        //if (at_sm_modem_submit_at(modem_fd, "ATD#777\r"))
        if (at_sm_modem_submit_at(modem_fd, buf))
        {
            __msleep(500);
            // A064D00300 EC506 ADD (by l45517 2005年11月3日 ) BEGIN
            write(modem_fd, "\r", 1);
            // A064D00300 EC506 ADD (by l45517 2005年11月3日 ) END
            WARN("at dial failed, writing abort");
            
            // A064D00300 EC506 MOD (by l45517 2005年11月3日 ) BEGIN
            return AT_TIMEOUT;
            // A064D00300 EC506 MOD (by l45517 2005年11月3日 ) END
        }
    }
    else
    {
    		printf("VDF:%s:%s:%d:SYSMODE_NO_SERVICES***\n",__FILE__,__FUNCTION__,__LINE__);
		int at_res = 0;
		if (at_sm_submit_at(AT_SYSINFO_QUERY,  &at_res, NULL))
		{
			printf("VDF:%s:%s:%d:SYSINFO超时***\n",__FILE__,__FUNCTION__,__LINE__);
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			printf("VDF:%s:%s:%d:SYSINFO解析出错***\n",__FILE__,__FUNCTION__,__LINE__);
			WARN("got sysinfo error msg");
		}
	//printf("VDF:%s:%s:%d:sys_mode == %d***\n",__FILE__,__FUNCTION__,__LINE__,sys_mode);
        WARN("illegal sysmode type : NO SERVICES\n");
        return AT_TIMEOUT;
    }
    PPPD_THREAD_DEBUG("at dial successful");
    return AT_OK;
}


/*------------------------------------------------------------
  函数原型 : int pppd_thread_ath(int modem_fd)
  描述     : AT拨号挂起连接
  输入     : 无
  输出     : 无
  返回值   : 1:拨号失败；0:拨号成功
-------------------------------------------------------------*/
int pppd_thread_ath(int modem_fd)
{
    int sys_mode;
    int sys_rssi;
    int tmp;
    if (at_sm_modem_submit_at(modem_fd, "ATH\r")) 
    {
        // A064D00428 EC506 MOD (by l45517 2005年11月21?) BEGIN
        /*
        if (at_sm_modem_submit_at(modem_fd, "ATH\r")) 
        {
            return AT_TIMEOUT;
        }
        */
        return AT_TIMEOUT;
        // A064D00428 EC506 MOD (by l45517 2005年11月21?) END
    }
    return AT_OK;
}


/*------------------------------------------------------------
  函数原型 : static int pppd_thread_entry(void* args)
  描述     : pppd线程启动入口
  输入     : args无意义
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
static int     g_thread_args = 0;
static char**  g_thread_argv = NULL;

static int pppd_thread_entry(void* args)
{
 
    pppd_main(g_thread_args, g_thread_argv);
    return SUCCESS;
}


/*------------------------------------------------------------
  函数原型 : int pppd_thread_initialize(int args, char** argv)
  描述     : pppd线程初始化
  输入     : args和argv为命令行参数
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
THREAD_S g_ppp_thread;

//A064D00348 qinzhiyuan begin
//int pppd_thread_initialize(int args,  char** argv)
int pppd_thread_initialize(int args, const char** argv)
//A064D00348 qinzhiyuan end
{

    g_thread_args = args;
    g_thread_argv = argv;
	printf("***YP:%s:%s:%d:pppd thread initialized***\n",__FILE__,__FUNCTION__,__LINE__);
    /// pppd_main(g_thread_args, g_thread_argv);
    
    if (__init_thread_t(&g_ppp_thread, pppd_thread_entry, NULL))
    {
        return FAILED;
    }
    
    PPPD_DEBUG("pppd thread initialized");
    if (__start_thread(&g_ppp_thread) < 0)
    {
        ERROR("pppd thread start failed");
        return FAILED;
    }
    
    PPPD_DEBUG("pppd thread started thread_id = %d", g_ppp_thread.thread_id);
    return SUCCESS;
}


/*------------------------------------------------------------
  函数原型 : int pppd_thread_destroy()
  描述     : pppd线程注销
  输入     : 无
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
int pppd_thread_destroy()
{
    int i_pid;
    int tmp;
    int status;

    // A64D01038 ADD (by l45517 2006年4月10日 ) BEGIN
    // 如果ppp 存在则将其推出。
    INFO("kill : pppd thread not exist");
    if (g_ppp_thread.thread_id > 0)
    {
        tmp = kill(g_ppp_thread.thread_id, SIGTERM);
        if (tmp)
        {
            if (ESRCH == errno)
            {
                INFO("kill : pppd thread not exist");
            }
            else
            {
                ERROR("kill : pppd error");
            }
        }
        else
        {
            INFO("pppd thread killed, pid = %d", g_ppp_thread.thread_id);
            while(1)
            {
                i_pid = waitpid(g_ppp_thread.thread_id, &status, __WALL);
                if (i_pid > 0)
                {
                    INFO("waitpid : thread %d exited\n", i_pid);
                }
                else if (i_pid == 0)
                {
                    INFO("waitpid : no child was available\n");
                }
                else
                {
                    if (EINTR == errno)
                    {
                        continue;
                    }
                    ERROR("waitpid : ");
                }
                break;
            }
        }
    }
    // A64D01038 ADD (by l45517 2006年4月10日 ) END

    __destroy_thread_t(&g_ppp_thread);
    return 0;
}
