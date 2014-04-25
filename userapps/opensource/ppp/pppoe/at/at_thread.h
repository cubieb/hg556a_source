/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : at_thread.h
  作者     : lichangqing 45517
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : 头文件
      
  主要函数列表: 无
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#ifndef __AT_THREAD_H
#define __AT_THREAD_H

typedef int RSSI_S;
typedef struct
{
    int srv_status;
    int srv_domain;
    int roam_status;
    int sys_mode;
    int sim_state;
} SYS_INFO_S;

#define USIM_1           1        // 获取usim卡状态
#define RSSI_2           2        // 获取RSSI
#define EVDO_VERSION_3   3        // 获取EVDO版本
#define PPP_IP_STATE_4   4        // 获取PPP连接状态和IP地址
#define PPP_LINKTIME_5   5        // 获取PPP连接时长
#define PPP_STATS_6      6        // 获取PPP累计流量和当前流量
#define ERROR_11         11       // 接口取消
#define ERROR_12         12       // 接口取消
#define CM500_NETTYPE_13 13       // 获取网络类型

#define RESPONSE_SUCCESSE 1       // 响应用户获取信息成功
#define RESPONSE_FAILED   0       // 响应用户获取信息失败
#define BASE_SIZE         8       // 返回的数据中ret_val和cmd的长度

#define EVDO_VERSION_LEN    32    // EVDO版本字符串的长度
#define ESN_LEN             32    // ESN字符串的长度
#define CM500_START_TIME_15 15    // 20秒

#define AT_TIMEOUT          1     // AT命令查询失败
#define AT_OK               0     // AT命令查询成功
#define AT_CHLD_EXITED      1     // 没有子线程退出
#define AT_CHLD_NOT_EXITED  0     // 有子线程退出
#define AT_MAIN_EXITED      1     // 主线程退出
#define AT_SM_RCV_EXITED    1     // AT上报线程退出


#define  SERVPORT           8765  //服务器监听端口号
#define  BACKLOG            10    // 最大同时连接请求数
#define  REQUEST_SIZE       4     // 提交请求的最大长度

#define AT_CMD_PARAM_LEN    8     //AT命令参数长度

struct evdo_version_3
{
    char soft[EVDO_VERSION_LEN];  // 长度不大于31的字符串
    char hard[EVDO_VERSION_LEN];  // 长度不大于31的字符串
};

// ppp连接状态
#define CONNECTED     1
#define CONNECTING    0
#define DISCONNECTED -1

// ppp IP地址和连接状态
struct ppp_info_4
{
    unsigned int ip_addr;
    unsigned int ppp_state;
};

// ppp 当前连接时长和累计连接时长
struct ppp_time_stat_5 
{
    unsigned int cur_time_len;
    unsigned int total_time_len;
};

// ppp 当前流量和累计流量
// A064D00313 EC506 MOD (by l45517 2005年10月25?) BEGIN
/*
struct ppp_stat_6 
{
    unsigned int cur_ppp_ibytes;
    unsigned int cur_ppp_obytes;
    unsigned int total_ppp_ibytes;
    unsigned int total_ppp_obytes;
};
*/
struct ppp_stat_6
{
    long long cur_ppp_ibytes;
    long long cur_ppp_obytes;
    long long total_ppp_ibytes;
    long long total_ppp_obytes;
};
// A064D00313 EC506 MOD (by l45517 2005年10月25?) END


//返回给client结果的结构体
struct request_result 
{
    int ret_val;     // 表示请求执行是否成功
    int cmd;         // 处理后的请求号
    union 
    {
        int    uim_1;
        int    evdo_rssi_2;
        struct evdo_version_3   evdo_version_3;
        struct ppp_info_4       ppp_info_4;
        struct ppp_time_stat_5  ppp_time_5;

        struct ppp_stat_6       ppp_stat_6;
        int    net_type_13;
    } ret_data;
};

/*AT配置参数结构体, add by sxg*/
typedef struct tagATConfig
{
	char *profile;
	char *phone_number;
	char *ap_name;
	unsigned int operator;
	unsigned int conn_type;
	char * channel;
}AT_CONFIG_S;

#define SIGNAL(s, handler)	do { \
	sa.sa_handler = handler; \
	if (sigaction(s, &sa, NULL) < 0) \
	    FATAL("Couldn't establish signal handler (%d): %m", s); \
    } while (0);

#endif //AT_THREAD_H

