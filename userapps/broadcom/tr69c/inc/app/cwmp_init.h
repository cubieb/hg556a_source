/***********************************************************************
  版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名:
  作者: Diao.Meng
  版本: HG550V200R001B02
  创建日期: 2005-5-6
  完成日期:
  功能描述:

  主要函数列表:

  修改历史记录列表:
    <作  者>    <修改时间>  <版本>  <修改描述>

  备注:
************************************************************************/

#ifndef _CWMP_INIT_H_
#define _CWMP_INIT_H_

#include "cfg_typedef.h"
#ifdef __cplusplus
extern "C" {
#endif

#define    TR069_ACSURL_LEN                            (256)
#define    TR069_ACSUSERNAME_LEN                       (256)
typedef  struct   tagTR069_WEBINFO
{
    CHAR     pcAcsURL[TR069_ACSURL_LEN];

    CHAR     pcAcsUseName[TR069_ACSUSERNAME_LEN];
    CHAR     pcAcsPassword[TR069_ACSUSERNAME_LEN];
    CHAR     pcConnectUserName[TR069_ACSUSERNAME_LEN];
    CHAR     pcConnectPassword[TR069_ACSUSERNAME_LEN];
    BOOL     bEnableInform;
    int         iInformInterval;
}TR069_CONFIG_INFO , *PTR069_CONFIG_INFO;

typedef  struct   tagTR069_CLIINFO
{
    int         iDbgType;
    int         iDbgLevel ;
    int         ulPortNum;
    char        acIP[32];
}TR069_CONFIG_CLIINFO ,      *PTR069_CONFIG_CLIINFO;

typedef enum tagCWMP_CLI_DBG_TYPE
{
    CWMP_DBG_TYPE_SWITCH          = 0,
    CWMP_DBG_TYPE_LEVEL           = 1,
    CWMP_DBG_TYPE_TRACER          = 2,
    CWMP_DBG_TYPE_RESTORE         = 3,
    CWMP_DBG_TYPE_SSLCERT         = 4,
    CWMP_DBG_TYPE_NONE            = 5,
} CWMP_CLI_DBG_TYPE_EN;


VOID CWMP_Init(VOID);

#ifdef __cplusplus
}
#endif

#endif  /* _CWMP_INIT_H_ */


