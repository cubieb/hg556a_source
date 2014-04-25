/***********************************************************************
  版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名: at_sm.h
  作者: lichangqing 45517
  版本: V500R003
  创建日期: 2005-8-8
  完成日期: 2005-8-10
  功能描述: 头文件
      
  主要函数列表: 无
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#ifndef __AT_SM_H
#define __AT_SM_H

// 为每条AT命令分配的序号
#define AT_RSSI_QUERY    			 0	 // RSSI查询功能命令			 RSSI_QUERY
#define AT_SYSINFO_QUERY			 1	 // 系统信息查询				 SYSTEM_INFO_QUERY
#define AT_SYSCFG_SET  				 2 	//设置连接类型				SYSCFG_SET
#define AT_PDP_SET					 3	//定义PDP关联					PDP_SET
#define AT_AUTO_SERVICE_REPORT 	 4	//服务状态变化指示			AUTO_SERVICE_STATUS_REPORT
#define AT_AUTO_SYSTEM_REPORT		 5	//系统模式变化事件指示 	AUTO_SYSTEM_MODE_STATUS_REPORT
#if 0
#define AT_0         0 // 测试命令         AT       <CR><LF>OK<CR><LF>*/
#define AT_GMR_1     1 // 软件版本号，     +GMR     <CR><LF>+GMR:<softversion><CR><LF>   有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_HWVER_2   2 // 硬件版本号查询   ^HWVER   <CR><LF>^HWVER:<hardversion><CR><LF> 有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_ESN_3     3 // ESN查询命令      +GSN     <CR><LF>+GSN: <ESN><CR><LF>          有MS相关错误时：<CR><LF>ERROR<CR><LF>
#define AT_SYSINFO_4 4 // 系统信息查询     ^SYSINFO <CR><LF>^SYSINFO:< srv_status >,< srv_domain >,< roam_status >,< sys_mode >,< sim_state ><CR><LF>
#define AT_CSQ_5     5 // RSSI查询功能命令 +CSQ     <CR><LF>+CSQ: <rssi>,<ber><CR><LF><CR><LF>OK<CR><LF> 有MS相关错误时：<CR><LF>+CME ERROR: <err><CR><LF>
#define AT_HDRCSQ_6  6 // evdo RSSI 查询   ^HDRCSQ  <CR><LF>^HDRRSSI: <rssi><CR><LF>
#define AT_NETMODE   7//  网络模式设置   

/*add by sxg*/
#define AT_PDP_CONTEXT 8//定义PDP关联
#define AT_OPERATOR    9 //设置运营商选择模式: 自动/手动
#define AT_SYSCFG   10 //设置连接类型: GPRS/3G,设置通道限制类型
#endif



int at_sm_is_active();
int at_sm_submit_at(int at_num,  int* is_report_err, char* param);
//int at_sm_initialize();
//int at_sm_destroy();
int at_sm_modem_submit_at(int modem_fd, char* at_dial_str);

#define SYSMODE_WCDMA       5
#define SYSMODE_CDMA        2
#define SYSMODE_NO_SERVICES 0

#define LOOP_TIMES 5

#define MAX_CMD_LEN    9  // 上报命令的最大匹配长度
#define MAX_REPORT_NUM 7  // 允许上报的数目
#define READ_SIZE      64 // 每此试图从串口读64个字符
#define AT_REPORT_OK   1
#define AT_REPORT_ERR  0
#define PARSE_OK		1
#define PARSE_ERROR		0
// A064D00428 EC506 ADD (by l45517 2005年11月21?) BEGIN
#define AT_REPORT_BUSY AT_REPORT_OK
// A064D00428 EC506 ADD (by l45517 2005年11月21?) END

#if 0
typedef struct AT_HANDLE_S
{
    char* at_cmd_str;
    char* at_report_str;
    char* at_report_err_str;
    void  (*create_at_cmd)(char* buf, char* param);
    int  (*at_sm_report_handler)(char* report_buf);
}AT_HANDLE_T;
#endif
typedef struct AT_HANDLE_S
{
	int at_cmd_id;
	void* at_cmd_struct;
	int  (*at_cmd_report_handler)(struct stAtRcvMsg *pstAtRcvMsg);
	char* at_cmd_str;
	void  (*create_at_cmd)(char* buf, char* param);
}AT_HANDLE_T;

 
#endif // __AT_SM_H

