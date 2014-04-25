/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : at_thread.c
  作者     : lichangqing 45517
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : 实现AT服务线程
      
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// A064D00358 EC506 ADD (by l45517 2005年11月2日 ) BEGIN
#include <setjmp.h>
#include <linux/termios.h>
// A064D00358 EC506 ADD (by l45517 2005年11月2日 ) END

#include "pppd.h"
#include "at_sm.h"
#include "utils_lib.h"
#include "pppd_thread.h"
#include "at_thread.h"
#include "files.h"    //add by sxg
#include "ttyUSB.h"


int               g_log_fd = -1;
//static RSSI_S     g_cm500_rssi;                          // cm500 RSSI
//static RSSI_S     g_cm500_hdrrssi;                       // cm500 evdo RSSI
//static SYS_INFO_S g_cm500_sysinfo = {0, 0, 0, 0, 0};     // cm500系统信息
//static char       g_cm500_sw_version[EVDO_VERSION_LEN];  // 软件版本号,长度不大于31的字符串 不包括\0吗？
//static char       g_cm500_hw_version[EVDO_VERSION_LEN];  // 硬件版本号。
//static char       g_cm500_esn[ESN_LEN];//A064D00348 qinzhiyuan            

extern struct stMsg_RSSI_Query			rssi_get ;
extern struct stMsg_System_Info_Query	sys_info_get;
extern struct stMsg_Syscfg_Set			cfg_set;
extern struct stMsg_PDP_Set				pdp_set;
extern struct stMsg_Service_Status		rp_service_sts;
extern struct stMsg_System_Mode			rp_sys_mode;


int               g_main_thread_killed = 0;              // 将在信号处理程序中设置；
int               g_shake_hand_failed  = 0;              // 握手失败标志
int               g_child_exited       = 0;
AT_TIMER_S        g_shankehand_timer;                    // cm500握手的定时器
//MUTEX_S           g_rcv_start_mutex;
//MUTEX_S           g_pppd_start_mutex;

extern char* nvram_get(const char *name);

//A064D00348 qinzhiyuan begin
extern char* nvram_set(const char* name,const char* value);
extern char* nvram_commit(void);
//A064D00348 qinzhiyuan end

#define MODEM_TIMEOUT 50
static int g_no_upstats_times = 0;
static int g_no_downstats_times = 0;
static int g_has_set_network = 0;

int g_modem_stub = 0;
int g_modem_restore_ldisc_stub = 0;
int g_stub = 0; // A064D00429 EC506 ADD (by L45517 2005年11月25?) 

/*保存AT模块的配置参数 add by sxg*/
AT_CONFIG_S at_config = {NULL, NULL, NULL, 0, 0, 0};

/*------------------------------------------------------------
  函数原型: int at_thread_cm500_sysmode()
  描述    : 获取cm500系统模式
  输入    : 无
  输出    : 无
  返回值  : 返回系统模式
-------------------------------------------------------------*/
int at_thread_cm500_sysmode()
{
    return sys_info_get.ucSysMode;
}


/*------------------------------------------------------------
  函数原型: int at_thread_cm500_rssi()
  描述    : 获取cm500系统模式
  输入    : 无
  输出    : 无
  返回值  : 返回系统模式
-------------------------------------------------------------*/
int at_thread_cm500_rssi()
{
    return rssi_get.ucRSSI;
}


/*------------------------------------------------------------
  函数原型: void at_thread_reset_sysmode()
  描述    : 获取cm500系统模式
  输入    : 无
  输出    : 无
  返回值  : 无
-------------------------------------------------------------*/
void at_thread_reset_sysmode()
{
    sys_info_get.ucSysMode= 0;
}

/*------------------------------------------------------------
  函数原型 : static void toggle_wan_led()
  描述     : wan 口点灯函数
  输入     : 无
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
static void toggle_wan_led()
{
	static int i_is_wan_ledon = 0;
	// A064D00429 EC506 MOD (by L45517 2005年12月1日 ) BEGIN
	static struct ppp_his_stat history_stats = {0, 0};
    struct ppp_his_stat cur_stats = {0, 0};
	// A064D00429 EC506 MOD (by L45517 2005年12月1日 ) END

	int language = 0;//CUSTOMID;  /* A64D01304: chenjindong add 2006-7-11 */
	
	if (PHASE_RUNNING != phase)
	{
		//wan_ledoff();
		return;
	}
    
    // A064D00358 EC506 ADD (by l45517 2005年11月2日 ) BEGIN
    // 添加检测dcd信号线的功能。
    if (real_ttyfd != -1)
    {
        int modembits;
        
        if (ioctl(real_ttyfd, TIOCMGET, &modembits))
        {
            ERROR("get DCD error");
        }
        else
        {
            if (!(modembits & TIOCM_CD))
            {
                kill(g_ppp_thread.thread_id, SIGHUP);
                return;
            }
            /* A64D01304: chenjindong begin 2006-7-11 */
            /* --------------------------------------- 
                村通版本检测DSR 控制线状态    
                OFF:断开ppp 连接  
                --------------------------------------- */
            else if(!(modembits & TIOCM_DSR))
            {                
                if(1 == language)
                {
                    kill(g_ppp_thread.thread_id, SIGHUP);
                    return;
                }
            }
            /* A64D01304: chenjindong end 2006-7-11 */        
            //INFO("modembits <%x>", modembits);
        }
    }
    
	if (!get_cur_link_stats(&cur_stats))
    {
		//wan_ledoff();
		return;
    }
    // A064D00358 EC506 ADD (by l45517 2005年11月2日 ) END


  	if (history_stats.bytes_out == cur_stats.bytes_out)
  	{
      	g_no_upstats_times++;
  	}
    else
    {
        history_stats.bytes_out = cur_stats.bytes_out;
        
        if (g_no_upstats_times > 20)
        {
            g_no_downstats_times = 0;
        }
            
        g_no_upstats_times = 0;
    }

	if (history_stats.bytes_in == cur_stats.bytes_in)
	{
    	g_no_downstats_times++;
        
        if( g_no_downstats_times > 10 )
        {
		   // wan_ledon();
            return;
        }
	}
    else
    {
        if (g_no_downstats_times > 20)
        {
            g_no_upstats_times = 0;
        }
        g_no_downstats_times = 0;
    }
    
	history_stats.bytes_in = cur_stats.bytes_in;
	if (i_is_wan_ledon)
	{
		i_is_wan_ledon = 0;
		//wan_ledoff();
		return;
	}
	else
	{
    	i_is_wan_ledon = 1;
    	//wan_ledon();
    	return;
	}
}


/*------------------------------------------------------------
  函数原型 : void at_thread_shakehand_callback(int arg)
  描述     : 与cm500定时握手回调函数
  输入     : arg无意义
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
static int g_shakehand_times = 0; // 定时器触发的次数
void at_thread_shakehand_callback(int arg)
{
    int at_res;
	int   errno_tmp = errno;
    int fd_tmp_pcui;
    //char* pch = "ERROR";//A064D00348 qinzhiyuan begin
    char* tmp;
    char buf[AT_CMD_PARAM_LEN];
	struct timeval cur_time;
    struct ppp_his_stat cur_stats;

    toggle_wan_led();

    // A64D01229 ADD (by l45517 2006年5月30日 ) BEGIN
    {
        
        static int block_phase_authenticate = 0;
        static int block_phase_serialconn = 0;

        /*
         *  该问题单是处理因为ppp状态机存在阻塞的问题，以前的测试发现在ppp 状态
         *  机处于PHASE_AUTHENTICATE和PHASE_SERIALCONN时会出现该现象。
         *  该问题比较难复现，不好定位，暂且通过以下的方法规避: 即当状态机处于
         *  前面的两个状态的时间超过5秒时，通过发送信号SIGHUP来将ppp状态机从阻塞
         *  中唤醒。
         *  定时器每100毫秒来一次，block_phase_authenticate和block_phase_serialconn乘以100 就是ppp在某
         *  个状态停留的时间。
         */

        if (PHASE_AUTHENTICATE == phase)
        {
            block_phase_authenticate++;
        }
        else
        {
            block_phase_authenticate = 0;
        }
        
        if (PHASE_SERIALCONN == phase)
        {
            block_phase_serialconn++;
        }
        else
        {
            block_phase_serialconn = 0;
        }

        if ((block_phase_authenticate > 1800) || (block_phase_serialconn > 1800))
        {
            kill(g_ppp_thread.thread_id, SIGHUP);
        }
    }

    /* 计算5秒间隔，到5 秒启动与CM500的握手，否则不执行任何操作。*/
    if (g_shakehand_times < 50) 
    {
        g_shakehand_times++;
        return;
    }
    g_shakehand_times = 0;

    // A64D01229 ADD (by l45517 2006年5月30日 ) END


    // 如果状态机处于活动状态
    if (at_sm_is_active())
    {
        return;
    }
    
    /* HUAWEI HGW qinzhiyuan 2005年10月9日
       设置系统模式*/

    if (1) 
    {
        if (at_sm_submit_at(AT_SYSINFO_QUERY, &at_res, NULL))
        {
            INFO("cm500 shakehand timeout");
            g_shake_hand_failed = AT_TIMEOUT;
            __stop_timer(&g_shankehand_timer);
            errno = errno_tmp;
            return;
        }
        if (at_res)
        {
            WARN("got sysinfo error msg");
        }
    }

    // 查询RSSI
    if (at_sm_submit_at(AT_RSSI_QUERY,  &at_res, NULL))
    {
        _DEBUG("cm500 shakehand timeout");
        g_shake_hand_failed = AT_TIMEOUT;
        __stop_timer(&g_shankehand_timer);
        errno = errno_tmp;
        return;
    }
    if (at_res)
    {
        WARN("got rssi error msg");
        // A064D00428 EC506 DEL (by l45517 2005年11月21?) BEGIN
        //  g_cm500_rssi = 0;
        // A064D00428 EC506 DEL (by l45517 2005年11月21?) END
    }

    INFO("hdrrssi, rssi<%d>", rssi_get.ucRSSI); 

#if 1
    // A064D00429 EC506 ADD (by L45517 2005年12月2日 ) BEGIN
    get_cur_link_time(&cur_time);
    get_cur_link_stats(&cur_stats);

    _DEBUG("timer shakehand rssi <%d>, sysmode <%d>, stubs <%d, %d, %d>\n" 

        "    srv_status      = %d\n"
        "    srv_domain      = %d\n"
        "    roam_status     = %d\n"
        "    sys_mode        = %d\n"
        "    sim_state       = %d\n"
        "    ppp link state  = %d\n"
        "    ppp ip addr     = %I\n"
        //"    evdo sw version = %s\n"
        //"    evdo hw version = %s\n"
        //"    ppp link time        cur = %d, history = %d\n"
        //"    ppp link status-up   cur = %d, history = %d\n"
        //"    ppp link status-down cur = %d, history = %d"

        , (int)rssi_get.ucRSSI, sys_info_get.ucSysMode, g_modem_stub, g_modem_restore_ldisc_stub, g_stub  

        , sys_info_get.ucSrvStatus
        , sys_info_get.ucSrvDomain
        , sys_info_get.ucRoamStatus
        , sys_info_get.ucSysMode
        , sys_info_get.ucSimState
        , phase
        , ipcp_get_ppp_addr()
        //, g_cm500_sw_version
        //, g_cm500_hw_version
        //, cur_time.tv_sec,     (cur_time.tv_sec     + g_pppd_history_time.tv_sec)
        //, cur_stats.bytes_out, (cur_stats.bytes_out + g_pppd_history_stats.bytes_out)
        //, cur_stats.bytes_in,  (cur_stats.bytes_in  + g_pppd_history_stats.bytes_in)
        );
	
    // A064D00429 EC506 ADD (by L45517 2005年12月2日 ) END

#else
    _DEBUG("timer shakehand rssi <%d>, sysmode <%d>\n"
        , (int)g_cm500_rssi, g_cm500_sysinfo.sys_mode);
#endif

   // rssi_ledshow((int)g_cm500_rssi);

    // 如果系统模式不正确，则查询系统模式
    if (SYSMODE_NO_SERVICES == sys_info_get.ucSysMode)
    {
        if (at_sm_submit_at(AT_SYSINFO_QUERY, &at_res, NULL))
        {
            _DEBUG("cm500 shakehand timeout");
            g_shake_hand_failed = AT_TIMEOUT;
            __stop_timer(&g_shankehand_timer);
            errno = errno_tmp;
            return;
        }
        if (at_res)
        {
            WARN("got sysinfo error msg");
        }
        else
        {
            _DEBUG("timer shakehand sysinfo <srv_status = %d, srv_domain = %d,"
                "roam_status = %d, sys_mode = %d, sim_state = %d>",
                sys_info_get.ucRoamStatus,
                sys_info_get.ucSimState,
                sys_info_get.ucSrvDomain,
                sys_info_get.ucSrvStatus,
                sys_info_get.ucSysMode);
        }
    }
    /* end modify */
    
    errno = errno_tmp;
    return;

}


/*------------------------------------------------------------
  函数原型: int open_log(void)
  描述    : 打开系统日志文件
  输入    : 命令行参数
  输出    : 无
  返回值  : 0 成功；1 失败
-------------------------------------------------------------*/
int open_log(void)
{
    g_log_fd = open("/var/log/ppp.log", O_CREAT | O_RDWR | O_TRUNC);
    if (g_log_fd < 0)
    {
        return FAILED;
    }
    return SUCCESS;
}


/*------------------------------------------------------------
  函数原型: void close_log(void)
  描述    : 打开系统日志文件
  输入    : 命令行参数
  输出    : 无
  返回值  : 无
-------------------------------------------------------------*/
void close_log(void)
{
    if (g_log_fd < 0)
    {
        return;
    }
    close(g_log_fd);
}

/*------------------------------------------------------------
  函数原型: int main(int args, const char** argv)
  描述    : AT服务线程也是进程的主函数
  输入    : 命令行参数
  输出    : 无
  返回值  : 无
-------------------------------------------------------------*/
int main(int args, const char** argv)
{
    int i_time; // cm500启动等待时间
    int i_ret;
    int sin_size;
    int request_num;
    int recvbytes;
    //A064D00363 yangjianping begin
    int fd_pcui = -1;
    //A064D00363 yangjianping end
    //system("ttyUSB");
	//__msleep(100000);
    int    sock_fd,     client_fd;   // sock_fd：监听socket; client_fd：数据传输socket
    struct sockaddr_in  my_addr;     // 本机地址信息
    struct sockaddr_in  remote_addr; // 客户端地址信息

    // A64D01229 ADD (by l45517 2006年5月30日 ) BEGIN
    int i_cm500_start_time[3] = {15, 30, 45}; // cm500的启动等待时间
    // A64D01229 ADD (by l45517 2006年5月30日 ) END

    int i_cm500_start_times   = 0;

    int i_args = 0;
    //g_pppd_history_time.tv_sec     = 0;
    //g_pppd_history_time.tv_usec    = 0;
    //g_pppd_history_stats.bytes_in  = 0;
    //g_pppd_history_stats.bytes_out = 0;

    // A064D00428 EC506 ADD (by l45517 2005年11月21?) BEGIN
    //g_cm500_sw_version[0] = '\0';
    //g_cm500_hw_version[0] = '\0';
    // A064D00428 EC506 ADD (by l45517 2005年11月21?) END
    
    daemon(1, 1);

    sigaction(SIGALRM, NULL, &(g_shankehand_timer.old_sigalrm));

    reopen_log();
    if (open_log())
    {
        printf("[%16s:%4d] open log failed, errno = %d <%s>\n", 
            __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    debug = 0;
//    __init_mutex(&g_rcv_start_mutex, "/usr/sbin/pppd",  3);
//    __init_mutex(&g_pppd_start_mutex, "/usr/sbin/pppd", 4);
	//__p(&g_pppd_start_mutex);
	
    /*start, modify by sxg*/
	for(i_args = 0; i_args < args; i_args++)
	{
		if(!strstr(argv[i_args], "-U"))
		{
			continue;
		}
		/*PPP Over USB--走HSPA上行*/
//		__p(&g_pppd_start_mutex);
		/*
	    if (pppd_thread_initialize(args, argv))
	    {
	        ERROR("pppd initialization failed");
	        exit(1);
	    }*/
	    //读取配置信息
		if(!read_config(ATCFG_PROFILE))
		{
			ERROR("at read config fail <%s>\n", ATCFG_PROFILE);
		}
		_DEBUG("at config: at_profile=%s, phone_number=%s, ap_name=%s, operator=%d, conn_type=%d, channel=%s\n", 
		                at_config.profile ? at_config.profile:"<NULL>", 
		                at_config.phone_number ? at_config.phone_number:"<NULL>" ,
		                at_config.ap_name ? at_config.ap_name:"<NULL>" ,
		                at_config.operator,
		                at_config.conn_type,
		                at_config.channel);

	    _DEBUG("VDF: start ppp over usb\n");
		return pppd_main(args, argv);
	    //break;
	}

	/*ppp over ADSL--走ADSL上行*/
	if(i_args == args)
	{
		_DEBUG("VDF: start ppp over ADSL\n");
		return pppd_main(args, argv);
	}
	/*end, modify by sxg*/
	printf("VDF:%s:%s:%d:erro!!! ppp can't start up***\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

/*------------------------------------------------------------
  函数原型: int at_init()
  描述    : 数据卡初始状态的查询、设置
  输入    : 无
  输出    : 无
  返回值  : 0－初始化成功；1－不成功
-------------------------------------------------------------*/

int at_init()
{
	int   at_res;
	char* pch = "ERROR";
	char* tmp;
	char buf[20];	
	struct stHspaInfo HspaInfo;
	FILE *fp = NULL;
       /* BEGIN: Added by y67514, 2008/12/1   问题单号:AU8D01263:HSPA上行PIN码未保存情况下，HSAP拨号不成功。*/
       int needToDelay = 0;
       /* END:   Added by y67514, 2008/12/1 */
	
	VDF_DBG("VDF:%s:%s:%d:开始查询***\n",__FILE__,__FUNCTION__,__LINE__);

	memset(&HspaInfo,0,sizeof(HspaInfo));
	while (1)
	{
		//通过读hspa状态文件判断数据卡是否可用
		if ( (fp = fopen(HSPA_SHARE_FILE_PATH, "r")) == NULL )
		{
			printf("Error:HspaStatus read failed.\n");
		}
		else
		{
			fread(&HspaInfo, sizeof(struct stHspaInfo), 1, fp);
			if(0 != HspaInfo.sPinPukNeed)
			{
				//pin码需验证
				fclose(fp);
				VDF_DBG("VDF:%s:%s:%d:pin码需验证***\n",__FILE__,__FUNCTION__,__LINE__);
                 __msleep(3000);         
                 /* BEGIN: Added by y67514, 2008/12/1   问题单号:AU8D01263:HSPA上行PIN码未保存情况下，HSAP拨号不成功。*/
                 needToDelay  = 1;
                 /* END:   Added by y67514, 2008/12/1 */
				continue;
			}
			if(255 == HspaInfo.sSimExist)
			{
				//sim卡不存在
				VDF_DBG("VDF:%s:%s:%d:sim卡不存在***\n",__FILE__,__FUNCTION__,__LINE__);
				fclose(fp);
                 __msleep(3000);
                 /* BEGIN: Added by y67514, 2008/12/1   问题单号:AU8D01263:HSPA上行PIN码未保存情况下，HSAP拨号不成功。*/
                 needToDelay = 1;
                 /* END:   Added by y67514, 2008/12/1 */
				continue;
			}
			//sim卡状态正常
			fclose(fp);
             /* BEGIN: Added by y67514, 2008/12/1   问题单号:AU8D01263:HSPA上行PIN码未保存情况下，HSAP拨号不成功。*/
              if ( needToDelay)
              {
                  needToDelay = 0;
                  __msleep(5000);
              }
              /* END:   Added by y67514, 2008/12/1 */
			VDF_DBG("VDF:%s:%s:%d:sSysMode=%d 初始状态ok***\n",__FILE__,__FUNCTION__,__LINE__,HspaInfo.sSysMode);
			break;
        	}
              __msleep(5000);
	}
	
	// 获取cm500 系统信息
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
		return AT_TIMEOUT;
	}
	else
	{
		VDF_DBG("***VDF:cm500 sysinfo <srv_status = %d, srv_domain = %d,"
			"roam_status = %d, sys_mode = %d, sim_state = %d>***\n",
				sys_info_get.ucSrvStatus,
				sys_info_get.ucSrvDomain,
				sys_info_get.ucRoamStatus,
				sys_info_get.ucSysMode,
				sys_info_get.ucSimState);

	}

	// 如果系统模式不正确，重新查一遍
	if (SYSMODE_NO_SERVICES == sys_info_get.ucSysMode)
	{
		__msleep(5000);
		if (at_sm_submit_at(AT_SYSINFO_QUERY,  &at_res, NULL))
		{
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			WARN("got sysinfo error msg");
			return AT_TIMEOUT;
		}
		else
		{
			VDF_DBG("***VDF:cm500 sysinfo <srv_status = %d, srv_domain = %d,"
				"roam_status = %d, sys_mode = %d, sim_state = %d>***\n",
					sys_info_get.ucSrvStatus,
					sys_info_get.ucSrvDomain,
					sys_info_get.ucRoamStatus,
					sys_info_get.ucSysMode,
					sys_info_get.ucSimState);

		}
	}
	
	// 查询cm500 RSSI值
	if (at_sm_submit_at(AT_RSSI_QUERY, &at_res, NULL))
	{
		ERROR("cm500 timeout");
		return AT_TIMEOUT;
	}
	if (at_res)
	{
		WARN("got rssi error msg");
	}
	VDF_DBG("VDF:%s:%s:%d:rssi=%d***\n",__FILE__,__FUNCTION__,__LINE__,rssi_get.ucRSSI);
	/*设置 APN,自动模式ap_nam=NULL,手动模式使用配置值*/  
	if(at_config.ap_name)
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "1,\"IP\",\"%s\"", at_config.ap_name);
		if(at_sm_submit_at(AT_PDP_SET, &at_res, buf))
		{
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			//返回失败
			WARN("set APN error msg");
			return AT_TIMEOUT;
		}
	}
	
    #if 0
	/*设置连接类型: GPRS/3G*/
	if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_FIRST
		|| at_config.conn_type == ATCFG_CONNTYPE_3G_FIRST
		|| at_config.conn_type == ATCFG_CONNTYPE_GPRS_ONLY
		|| at_config.conn_type == ATCFG_CONNTYPE_3G_ONLY
		|| at_config.conn_type == ATCFG_CONNTYPE_AUTO)
	{
		memset(buf, 0, sizeof(buf));
		if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_FIRST)
		{
			sprintf(buf, "2,1");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_3G_FIRST)
		{
			sprintf(buf,"2,2");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_GPRS_ONLY)
		{
			sprintf(buf,"13,3");
		}
		else if(at_config.conn_type == ATCFG_CONNTYPE_3G_ONLY)
		{
			sprintf(buf,"14,3");			
		}
        else if(at_config.conn_type == ATCFG_CONNTYPE_AUTO)
		{
			sprintf(buf,"2,0");			
		}

		/*设置通道限制类型*/
		/*
		if(at_config.channel == ATCFG_CHANNEL_UNLIMITED)
		{
			sprintf(buf, "%s,3FFFFFFF", buf);
		}
		else if(at_config.channel == ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100)
		{
			sprintf(buf, "%s,00400380", buf);			 
		}
		else if(at_config.channel == ATCFG_CHANNEL_GSM1900)
		{
			sprintf(buf, "%s,00200000", buf);			  
		}
		else
		{
			sprintf(buf, "%s,40000000",buf);
		}
		*/
		
		sprintf(buf, "%s,%s", buf,at_config.channel);
		sprintf(buf, "%s,2,4", buf);
		VDF_DBG("AT is %s.........\n",buf);
		if(at_sm_submit_at(AT_SYSCFG_SET, &at_res, buf))
		{
			ERROR("cm500 timeout");
			return AT_TIMEOUT;
		}
		if (at_res)
		{
			WARN("set Conn Type error msg");
			return AT_TIMEOUT;
		}
	}
    #endif
	VDF_DBG("VDF:%s:%s:%d:配置完成***\n",__FILE__,__FUNCTION__,__LINE__);
	return AT_OK;
}


