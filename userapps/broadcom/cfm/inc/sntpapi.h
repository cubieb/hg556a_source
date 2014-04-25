/*****************************************************************************
//
//  Copyright (c) 2000-2004  Broadcom Corporation
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
//  Filename:       sntpapi.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  02/26/04
//
******************************************************************************
//  Description:
//      Define the BcmSntp functions.
//
*****************************************************************************/

#ifndef __SNTP_API_H__
#define __SNTP_API_H__

#include "ifcdefs.h"

// Return codes for the various functions

typedef enum _sntpStatus {
   SNTP_OK,
   SNTP_STORE_FAILED,
   SNTP_NOT_INIT,
   SNTP_BAD_RULE,
   SNTP_RULE_EXISTS,
   SNTP_ADD_FAIL,
   SNTP_INIT_FAILED
} SNTP_STATUS;

typedef struct dstInfo {
   unsigned char month;
   unsigned char day;
   unsigned char day_type; // 0 = day, 1-7 = Mon - Fri
   unsigned int timeofday;
} DST_INFO, *PDST_INFO;

typedef struct ntpConfig {
   char ntp_enabled;
   /* start of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
#ifdef SUPPORT_SNTP_AUTOCONF
   char CfgServerEnable;
#endif
   /* end of maintain sntp sever 可以通过dhcp option42获取 by xujunxia 43813 2006年6月29日 */
   char ntpServer1[IFC_DOMAIN_LEN];
/*start of HG_Protocol 2007.01.16 HG550 V100R002C02B010 A36D03268 */
   char ntpServer2[IFC_DOMAIN_LEN];
  // char ntpServer2[IFC_MEDIUM_LEN];
/*end of HG_Protocol 2007.01.16 HG550 V100R002C02B010 A36D03268 */
//BEGIN:add by zhourongfei to config 3rd ntpserver
   char ntpServer3[IFC_DOMAIN_LEN];
//END:add by zhourongfei to config 3rd ntpserver
   char timezone[64];
   int timezone_offset;
   /*start of 增加sntp 刷新时间配置 by s53329 at  20080908*/
   int  iFailTimes;
   int  iSucTimes;
   int  iEnableSum;
    /*end of 增加sntp 刷新时间配置 by s53329 at  20080908*/
   char use_dst;
   DST_INFO dst_start;
   DST_INFO dst_end;
} NTP_CONFIG, *PNTP_CONFIG;

#define SNTP_APP_ID         "SNTPCfg"
#define SNTP_CONFIG         1

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern SNTP_STATUS BcmSntp_init(void);
extern int BcmSntp_isInitialized(void);
extern SNTP_STATUS BcmSntp_GetVar( char *var, char *value );
extern SNTP_STATUS BcmSntp_SetVar( char *var, char *value );
extern void BcmSntp_serverRestart(void);
extern SNTP_STATUS BcmSntp_Store(void);
extern void BcmSntp_close(void);
/* start of maintain sntp server通过DHCP获取 by liuzhijie 00028714 2006年7月4日" */
#ifdef SUPPORT_SNTP_AUTOCONF
void bcmSNTPSvFrmDHCPStart(void);
#endif
SNTP_STATUS Sntp_changeLog(void);
/* end of maintain sntp server通过DHCP获取 by liuzhijie 00028714 2006年7月4日" */

void BcmSntp_unInit(void);

#if defined(__cplusplus)
}
#endif

#endif
