/*****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  Filename:       systemlog.h
//  Author:         Yen Tran
//  Creation Date:  5/2/02
//
******************************************************************************
//  Description:
//      Define the global constants, and functions for sysDiag utility.
//
*****************************************************************************/

#ifndef __SYSTEM_LOG_H__
#define __SYSTEM_LOG_H__

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* option is masked bits of BCM_SYSLOG_LOCAL, BCM_SYSLOG_REMOTE,
   BCM_SYSLOG_FILE and BCM_SYSLOG_CIRCULAR */
/* localDisplayLevel, localLogLevel and remoteLogLevel are essentially
   the priority of message which ranges from LOG_EMERG to LOG_DEBUG as
   defined in /sys/syslog.h. */
typedef struct bcmSyslogCb {
  int status;              /* enable, disable */
  /* start of maintain syslog sever 可以通过dhcp option7 by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
  int CfgServerEnable;   /* enable, disable */
#endif
  /* end of maintain syslog sever 可以通过dhcp option7 by xujunxia 43813 2006年6月29日 */
  int option;              /* log local, log remote, log local+remote */
  int serverPort;          /* if remote logging, syslog server's UDP port number */
  int localDisplayLevel;   /* messages get displayed if their priority is higher than this level */
  int localLogLevel;       /* messages get logged if their priority is higher than this level */
  int remoteLogLevel;      /* messages get logged if their priority is higher than this level */
  /* start of maintain log server允许配置域名 by liuzhijie 00028714 2006年7月11日
  struct in_addr serverIP; // if remote logging, syslog server's IP address
  */
  char serverAddr[64]; /* if remote logging, syslog server's IP address */
  /* end of maintain log server允许配置域名 by liuzhijie 00028714 2006年7月11日 */
  /*add by z67625 增加防火墙日志写flash开关 start*/
  int fwlog;         /*enable,disable*/
  /*add by z67625 增加防火墙日志写flash开关 end*/

  /*start of 增加系统日志保存FLASH开关by c00131380 at 080926*/
  int Syslog;        /*enable,disable*/
  /*end of 增加系统日志保存FLASH开关by c00131380 at 080926*/
  
} BCM_SYSLOG_CB, *pBCM_SYSLOG_CB;

#define BCM_SYSLOG_TASK             1
#define BCM_KLOG_TASK               2

/* bit fields for syslogd options */
#define BCM_SYSLOG_LOCAL                1
#define BCM_SYSLOG_REMOTE               2
#define BCM_SYSLOG_FILE                 4
#define BCM_SYSLOG_CIRCULAR             8
#define BCM_SYSLOG_LOCAL_REMOTE         3

#define BCM_SYSLOG_LOCAL_MASK           1
#define BCM_SYSLOG_REMOTE_MASK          2
#define BCM_SYSLOG_LOCAL_REMOTE_MASK    3
#define BCM_SYSLOG_CIRCULAR_MASK        8

/* default values */
#define BCM_SYSLOG_DEF_DISPLAY_LEVEL     LOG_ERR
#define BCM_SYSLOG_DEF_LOCAL_LOG_LEVEL   LOG_DEBUG
#define BCM_SYSLOG_DEF_REMOTE_LOG_LEVEL  LOG_ERR

#define BCM_SYSLOG_FIRST_READ           -2
#define BCM_SYSLOG_READ_BUFFER_ERROR    -1
#define BCM_SYSLOG_READ_BUFFER_END      -3
#define BCM_SYSLOG_MAX_LINE_SIZE        255
#define BCM_SYSLOG_MESSAGE_LEN_BYTES    4
#define BCM_SYSLOG_KILL_SIGKILL         9
#define BCM_SYSLOG_KILL_SIGTERM         15

#define BCM_SYSLOG_STATUS_ENABLE        1
#define BCM_SYSLOG_STATUS_DISABLE       0

/*modified by z67625 AU8D01025 开启IGMP log功能后，造成下挂PC无法访问web start*/
#define CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE 16
/*modified by z67625 AU8D01025 开启IGMP log功能后，造成下挂PC无法访问web end*/

/***************************************************
 This function is called to start syslogd & klogd
****************************************************/
void bcmSyslogStop(void);
void bcmSyslogStart(pBCM_SYSLOG_CB pSyslogConfig);

/***********************************************************************
 This function is called to read circular buffer
 ptr is the ptr of line to be read; buffer is the ptr of buffer where
 message line is to be stored and return; this function returns new
 ptr value, this ptr value is essentially the next line to be read. 
***********************************************************************/
int bcmLogreadPartial(int ptr, char* buffer);

void bcmLogClear();

int bcmSyslogIsRunning(int task);
void initSyslog(void);
/* start of maintain syslog server通过DHCP Option获取 by liuzhijie 00028714 2006年7月10日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
void bcmSyslogSvFrmDHCPStart(void);
#endif
/* end of maintain syslog server通过DHCP Option获取 by liuzhijie 00028714 2006年7月10日 */

#if defined(__cplusplus)
}
#endif

#endif /* __SYSTEM_LOG_H__ */
