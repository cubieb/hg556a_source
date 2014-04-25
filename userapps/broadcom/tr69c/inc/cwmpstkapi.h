/******************************************************************************
  版权所有  : 2007-2020，华为技术有限公司
  文 件 名  : cwmpstkapi.h
  作    者  : handy
  版    本  : V1.0
  创建日期  : 2007-12-30
  描    述  : CWMP协议栈对外头文件
  函数列表  :

  历史记录      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

*********************************************************************************/

#ifndef __CWMP_STK_API_H__
#define __CWMP_STK_API_H__
#if 0
#include "atperrdef.h"
#include "atputil.h"
#endif
#include "xml_interface.h"

#ifdef ATP_CWMP_ENABLE_SSL
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#endif
#ifdef USE_CWMP_TRACE
VOS_UINT32 ATP_CWMP_Outputf(const CHAR *app, ULONG level,
                                     ULONG type, ULONG logno,
                                     const CHAR *file, ULONG line,
                                     const CHAR *fmt, ...);

#define ATP_CWMP_TRACE              ATP_CWMP_Outputf

#ifdef DEBUG
#define ATP_CWMP_DEBUG              ATP_CWMP_Outputf
#else
#define ATP_CWMP_DEBUG(x...)
#endif

#ifdef DEBUG
VOS_UINT32 ATP_CWMP_Outputf(const CHAR *app, ULONG level,
                                     ULONG type, ULONG logno,
                                     const CHAR *file, ULONG line,
                                     const CHAR *fmt, ...);

#define ATP_CWMP_DEBUG              ATP_CWMP_Outputf
#else
#define ATP_CWMP_DEBUG(x...)
#endif

#endif


#define ATP_CWMP_ENABLE_HTTP_AUTH

/* log level */
typedef enum vtopLOG_E_LEVEL
{
    LOG_LEVEL_FATAL = 0,    /* just for compatibility with previous version */
    LOG_LEVEL_ALERT,        /* action must be taken immediately             */
    LOG_LEVEL_CRIT,         /* critical conditions                          */
    LOG_LEVEL_ERROR,        /* error conditions                             */
    LOG_LEVEL_WARNING,      /* warning conditions                           */
    LOG_LEVEL_NOTICE,       /* normal but significant condition             */
    LOG_LEVEL_INFO,         /* informational                                */
    LOG_LEVEL_DEBUG,        /* debug-level                                  */
    LOG_LEVEL_BUTT
}LOG_E_LEVEL;

/* log type */
typedef enum vtopLOG_E_TYPE
{
    LOG_TYPE_OPERATION = 1,
    LOG_TYPE_SECURATY  = 3,
    LOG_TYPE_SYSTEM    = 4,
    LOG_TYPE_DEBUG     = 23,
    LOG_TYPE_BUTT
}LOG_E_TYPE;

/*
 *  协议栈错误码
 */
#if 0 
typedef enum tagATP_CWMP_STK_ERR_CODE
{
    ATP_ERR_CWMP_STK_START= 0; 
    ATP_ERR_CWMP_STK_PARA_INVALID,
    ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH,
    ATP_ERR_CWMP_STK_INVALID_FORMAT,
    ATP_ERR_CWMP_STK_TR069_NOT_INITED,
    ATP_ERR_CWMP_STK_ALREADY_STARTED,
    ATP_ERR_CWMP_STK_NOT_STARTED,
    ATP_ERR_CWMP_STK_UNRESOLVED_HOST,

    ATP_ERR_CWMP_STK_RPC_NOT_FOUND,
    ATP_ERR_CWMP_STK_INVALID_EVT,

    ATP_ERR_CWMP_STK_NET_SOCK 
    ATP_ERR_CWMP_STK_NET_CONN,
    ATP_ERR_CWMP_STK_NET_DNS,
    ATP_ERR_CWMP_STK_NET_RECV,
    ATP_ERR_CWMP_STK_NET_SEND,
    ATP_ERR_CWMP_STK_NET_HEADER,
    ATP_ERR_CWMP_STK_NET_HTTP,
    ATP_ERR_CWMP_STK_NET_SEC,
    ATP_ERR_CWMP_STK_NET_SSL,
    ATP_ERR_CWMP_STK_NET_AUTH,
    ATP_ERR_CWMP_STK_SOAP_XML,
    ATP_ERR_CWMP_STK_SOAP_ID,
    ATP_ERR_CWMP_STK_SOAP_FAULT
} ATP_CWMP_STK_ERR_CODE_EN;
#endif
/*
 *  CWMP定义的每种参数类型
 */
typedef enum
{
    ATP_CWMP_VALUE_TYPE_ULONG       = 0,
    ATP_CWMP_VALUE_TYPE_LONG        = 1,
    ATP_CWMP_VALUE_TYPE_BOOL        = 2,
    ATP_CWMP_VALUE_TYPE_DATE        = 3,
    ATP_CWMP_VALUE_TYPE_BASE64      = 4,
    ATP_CWMP_VALUE_TYPE_CHAR        = 5
} ATP_CWMP_ATTR_VALUE_TYPE_E;

/*
typedef enum
{
    VALUE_TYPE_MASK        = 0xFF00,
    VALUE_TYPE_ULONG       = 0x100,
    VALUE_TYPE_LONG        = 0x200,
    VALUE_TYPE_BOOL        = 0x300,
    VALUE_TYPE_DATE        = 0x400, //字符串类型的枚举值必须放在VALUE_TYPE_DATE之后
    VALUE_TYPE_BASE64      = 0x500,
    VALUE_TYPE_CHAR        = 0x600
}ATTR_VALUE_TYPE_E;
 */
/* TSP CFM的数据类型定义和CWMP数据类型之间的转换宏 */
#define TSP_VALUETYPE_TO_CWMP(x)    (((x)>>8) - 1)


/*
 *  CWMP协议栈可注册的外部适配函数接口
 */

/* 从Flash读取协议栈状态信息的外部函数，
   协议栈分配了ulLength长度的字符串，
   pulUpgRes指明上次升级是(VOS_TRUE)否(VOS_FALSE)成功 */
typedef VOS_VOID (*PFAtpCwmpReadStkState)(VOS_CHAR *pcState,
                                                        VOS_UINT32 ulLength,
                                                        VOS_UINT32 *pulUpgRes);

/* 把协议栈状态信息写入到Flash的外部函数，协议栈构造好了字符串和长度 */
typedef VOS_VOID (*PFAtpCwmpWriteStkState)(VOS_CHAR *pcState,
                                                        VOS_UINT32 ulLength);

/* Inform扩展处理函数，协议栈根据TR069协议构造好Inform报文后，
   产品注册此函数可以对Inform进行扩展 */
typedef VOS_UINT32 (* PFAtpCwmpInformHook)(NODEPTR hInformSoap);

/*Start of 维护管理组 2008-3-20 15:31 for C02增强Session控制功能 by handy*/
#if 0
/* 会话结束处理函数，协议栈不判断该处理函数的返回值，可带两个参数 */
typedef VOS_UINT32 (*PFCwmpOnSessionEnd)(VOS_VOID *pvArg1,
                                                      VOS_VOID *pvArg2);
#else
typedef enum tagATP_CWMP_SESS_CTRL_TYPE_EN
{
	ATP_CWMP_SESS_START,		// 会话开始回调函数
	ATP_CWMP_SESS_END,		    // 会话结束时回调函数(无论成功还是失败)
	ATP_CWMP_SESS_MAX
} ATP_CWMP_SESS_CTRL_TYPE_EN;

/* 会话控制函数，协议栈不判断该处理函数的返回值 */
typedef VOS_UINT32 (*PFCwmpSessionCtrlFunc)(
                                            VOS_UINT32  ulErrCode,
                                            VOS_VOID    *pvArg1,
                                            VOS_VOID    *pvArg2);
#endif
/*End of 维护管理组 2008-3-20 15:31 for C02增强Session控制功能 by handy*/

/* 产品定制的域名解析函数，返回值表示是(VOS_TRUE)否(VOS_FALSE)解析成功 */
typedef VOS_BOOL (*PFCwmpNameResolveFunc)(
                    const VOS_CHAR  *pcName,        // 协议栈待解析的域名
                    VOS_UINT32      ulLocalIP,      // 协议栈所绑定的IP
                    VOS_UINT8       **ppucIPs,      // 解析后的IP地址
                    VOS_UINT16      *pusIPCnt);     // 解析后的IP地址个数

/*
 *  协议栈基本操作函数
 */
/******************************************************************************
  函数名称  : ATP_CWMP_StkInit
  功能描述  : 协议栈初始化接口
  输入参数  :
              1. pfReadState    :   外部实现的读取Flash接口
              2. pfWriteState   :   外部实现的写Flash接口
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_StkInit(PFAtpCwmpReadStkState           pfReadState,
                                   PFAtpCwmpWriteStkState    pfWriteState);

/******************************************************************************
  函数名称  : ATP_CWMP_StkStart
  功能描述  : 启动CWMP协议栈，启动前，需设置协议栈绑定的IP
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_ALREADY_STARTED

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_StkStart();

/******************************************************************************
  函数名称  : ATP_CWMP_StkStop
  功能描述  : 停止CWMP协议栈
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_NOT_STARTED

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_StkStop();

/*
 *  协议栈Session控制接口
 */
/******************************************************************************
  函数名称  : ATP_CWMP_StkIsBusy
  功能描述  : 判断协议栈当前是否有会话
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_TRUE:  当前会话还没有结束
              VOS_FALSE: 当前没有会话

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_BOOL  ATP_CWMP_StkIsBusy();

/******************************************************************************
  函数名称  : ATP_CWMP_StkStartSession
  功能描述  : 启动一次会话，该函数一定会成功；如果协议栈忙或者没有启动时，
              在协议栈不忙或者启动后，会启动一次会话
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : VOS_OK

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_StkStartSession();

/******************************************************************************
  函数名称  : ATP_CWMP_PauseSession
  功能描述  : 暂停协议栈的Session，调用该接口后，协议栈在完成当前会话后，
              不会再发起任何会话。除非理解，请不要随意使用本接口。
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-1-31
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_VOID ATP_CWMP_PauseSession();

/******************************************************************************
  函数名称  : ATP_CWMP_ResumeSession
  功能描述  : 恢复协议栈的Session，调用该接口后，如果有会话被暂停，
              协议栈会立即启动新的会话。请和ATP_CWMP_PauseSession配套使用。
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-1-31
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_VOID ATP_CWMP_ResumeSession();

/*
 *  协议栈IP地址操作接口
 */
/******************************************************************************
  函数名称  : ATP_CWMP_SetLocalIP
  功能描述  : 设置协议栈绑定的本地IP地址，当注册了外部解析函数时，
              本接口会首先调用外部注册的解析函数解析IP
              如果没有注册外部解析函数，或者外部解析函数解析失败，
              则会使用默认路由解析域名，如果仍解析失败，
              则返回ATP_ERR_CWMP_STK_UNRESOLVED_HOST
  输入参数  :
              1. pcIP:  点分格式的IP地址
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_UNRESOLVED_HOST

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_SetLocalIP(const VOS_CHAR *pcIP);

/******************************************************************************
  函数名称  : ATP_CWMP_GetLocalIP
  功能描述  : 获取本地的IP，IP地址以网络字节序存放
  输入参数  :
              1. pulLocalIP :   存放解析后本地IP地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetLocalIP(VOS_UINT32           *pulLocalIP);

/******************************************************************************
  函数名称  : ATP_CWMP_SetConReqPort
  功能描述  : 获取本地的回连端口
  输入参数  :
              1. usLocalPort    :   待设置的回连端口
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2008-5-29
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetConReqPort(VOS_UINT16           usLocalPort);

/******************************************************************************
  函数名称  : ATP_CWMP_GetRemoteIP
  功能描述  : 获取ACS服务器解析后的IP地址，网络字节序
  输入参数  :
              1. pulRemoteIP    :   存放ACS服务器解析后IP地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetRemoteIP(VOS_UINT32           *pulRemoteIP);

/*
 *  注册回调函数接口
 */
/*Start of 维护管理组 2008-3-20 15:32 for C02增强会话控制功能 by handy*/
#if 0
/******************************************************************************
  函数名称  : ATP_CWMP_RegEndSessionFunc
  功能描述  : 注册会话结束处理函数，注册的函数只被调用一次
  输入参数  :
              1. pfFunc :   会话结束回调函数指针，不能为空。
              2. pvArg1 :   回调函数的参数，需注册者自己分配内存，
                            并在回调函数中释放内存。
              3. pvArg2 :   回调函数的参数，需注册者自己分配内存,
                            并在回调函数中释放内存。
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_RegEndSessionFunc(PFCwmpOnSessionEnd pfFunc,
                                                   VOS_VOID           *pvArg1,
                                                   VOS_VOID           *pvArg2);
#else
/******************************************************************************
  函数名称  : ATP_CWMP_RegSessionCtrlFunc
  功能描述  : 注册会话控制函数，包括会话开始和会话结束处理函数
  输入参数  :
              1. pfFunc             :   会话控制回调函数指针，不能为空。
              2. enFuncType         :   会话控制函数的类型
              3. bDeleteByCurSess   :   该会话控制函数是否需要被当前会话删除
              2. pvArg1             :   回调函数的参数，需注册者自己分配内存，
                                        并在回调函数中释放内存。
              3. pvArg2             :   回调函数的参数，需注册者自己分配内存,
                                        并在回调函数中释放内存。
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_RegSessionCtrlFunc(
                            PFCwmpSessionCtrlFunc           pfFunc,
                            ATP_CWMP_SESS_CTRL_TYPE_EN      enFuncType,
                            VOS_BOOL                        bDeleteByCurSess,
                            VOS_VOID                        *pvArg1,
                            VOS_VOID                        *pvArg2);

/******************************************************************************
  函数名称  : ATP_CWMP_RegEndSessionFunc
  功能描述  : 注册会话结束处理函数，注册的函数只被调用一次
  输入参数  :
              1. pfFunc :   会话结束回调函数指针，不能为空。
              2. pvArg1 :   回调函数的参数，需注册者自己分配内存，
                            并在回调函数中释放内存。
              3. pvArg2 :   回调函数的参数，需注册者自己分配内存,
                            并在回调函数中释放内存。
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
#define ATP_CWMP_RegEndSessionFunc(pfFunc, pvArg1, pvArg2)  ATP_CWMP_RegSessionCtrlFunc((pfFunc), ATP_CWMP_SESS_END, VOS_TRUE, (pvArg1), (pvArg2))
#endif
/*End of 维护管理组 2008-3-20 15:32 for C02增强会话控制功能 by handy*/

/******************************************************************************
  函数名称  : ATP_CWMP_RegNameResolveFunc
  功能描述  : 注册解析域名的回调函数，多次调用以最后一次为准
  输入参数  :
              1. pfFunc :   可以为空，为空时，使用默认路由进行域名解析
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_RegNameResolveFunc(PFCwmpNameResolveFunc pfFunc);

/******************************************************************************
  函数名称  : ATP_CWMP_RegInformHooks
  功能描述  : 注册Inform报文处理的回调函数，多次调用以最后一次为准
  输入参数  :
              1. pfInformHook   :   报文处理函数指针，可以为空
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : VOS_OK

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_RegInformHooks(
                                PFAtpCwmpInformHook     pfInformHook);

/*
 *  设置配置项
 */
/******************************************************************************
  函数名称  : ATP_CWMP_StkSetAcsUrl
  功能描述  : 设置ACS地址
  输入参数  :
              1. pcAcsUrl   :   ACS地址，必须为http或https方式，不能超过255字节
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_NET_SSL
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_StkSetAcsUrl(const VOS_CHAR *pcAcsUrl);

/******************************************************************************
  函数名称  : ATP_CWMP_SetUsername
  功能描述  : 设置ACS认证CPE时，CPE使用的认证用户名
  输入参数  :
              1. pcUsername :   设置的用户名，不允许为空，不允许超过256字节
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetUsername(const VOS_CHAR *pcUsername);

/******************************************************************************
  函数名称  : ATP_CWMP_SetPassword
  功能描述  : 设置ACS认证CPE时，CPE使用的认证密码
  输入参数  :
              1. pcPassword :   设置的密码，不允许为空，不允许超过256字节
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetPassword(const VOS_CHAR *pcPassword);

/******************************************************************************
  函数名称  : ATP_CWMP_SetConReqUsername
  功能描述  : 设置CPE认证ACS时使用的认证用户名
  输入参数  :
              1. pcUsername :   设置的用户名，不允许为空，不允许超过256字节
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetConReqUsername(const VOS_CHAR *pcUsername);

/******************************************************************************
  函数名称  : ATP_CWMP_SetConReqPassword
  功能描述  : 设置CPE认证ACS时使用的认证密码
  输入参数  :
              1. pcPassword :   设置的密码，不允许为空，不允许超过256字节
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetConReqPassword(const VOS_CHAR *pcPassword);

#ifdef ATP_CWMP_ENABLE_SSL
/******************************************************************************
  函数名称  : ATP_CWMP_SetCertEnable
  功能描述  : 设置是否是能SSL客户端证书认证
  输入参数  :
              1. bCertEnable    :   是否是能
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetCertEnable(VOS_BOOL bCertEnable);

/******************************************************************************
  函数名称  : ATP_CWMP_SetCertificate
  功能描述  : 设置客户端认证的证书路径
  输入参数  :
              1. pcCertPath :   设置的证书路径，不允许为空
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetCertificate(const VOS_CHAR *pcCertPath);
#endif

/******************************************************************************
  函数名称  : ATP_CWMP_SetInformEnable
  功能描述  : 设置使能周期Inform
  输入参数  :
              1. bInformEnable  :   是否是能周期Inform
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : VOS_OK

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetInformEnable(VOS_BOOL bInformEnable);

/******************************************************************************
  函数名称  : ATP_CWMP_SetInformPeriod
  功能描述  : 设置Inform的周期
  输入参数  :
              1. ulInformPeriod :   设置的周期
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : VOS_OK

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetInformPeriod(VOS_UINT32 ulInformPeriod);

/*
 *  获取配置项
 */
/******************************************************************************
  函数名称  : ATP_CWMP_GetAcsUrl
  功能描述  : 获取Acs地址，执行后ppcAcsUrl指向协议栈中的内存，使用者不得释放
  输入参数  :
              1. ppcAcsUrl  :   指向ACS地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetAcsUrl(const VOS_CHAR **ppcAcsUrl);

/******************************************************************************
  函数名称  : ATP_CWMP_GetUsername
  功能描述  : 获取认证用户名，执行后ppcUsername指向协议栈中的内存，使用者不得释放
  输入参数  :
              1. ppcUsername    :   指向认证用户名地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetUsername(const VOS_CHAR **ppcUsername);

/******************************************************************************
  函数名称  : ATP_CWMP_GetPassword
  功能描述  : 获取认证密码，执行后ppcPassword指向协议栈中的内存，使用者不得释放
  输入参数  :
              1. ppcPassword    :   指向认证密码地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetPassword(const VOS_CHAR **ppcPassword);

/******************************************************************************
  函数名称  : ATP_CWMP_GetConReqURL
  功能描述  : 返回ConnectionRequestURL的值
  输入参数  : 无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : ConnectionRequestURL的值，使用者不得释放该值

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
const VOS_CHAR *ATP_CWMP_GetConReqURL(VOS_VOID);

/******************************************************************************
  函数名称  : ATP_CWMP_GetConReqUsername
  功能描述  : 获取认证用户名，执行后ppcUsername指向协议栈中的内存，
              使用者不得释放
  输入参数  :
              1. ppcUsername    :   指向认证用户名地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetConReqUsername(const VOS_CHAR **ppcUsername);

/******************************************************************************
  函数名称  : ATP_CWMP_GetConReqPassword
  功能描述  : 获取认证密码，执行后ppcPassword指向协议栈中的内存，使用者不得释放
  输入参数  :
              1. ppcPassword    :   指向认证密码地址的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetConReqPassword(const VOS_CHAR **ppcPassword);

#ifdef ATP_CWMP_ENABLE_SSL
/******************************************************************************
  函数名称  : ATP_CWMP_GetCertEnable
  功能描述  : 获取SSL证书认证是否使能标志
  输入参数  :
              1. pbCertEnable   :   存放是否使能SSL证书认证的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetCertEnable(VOS_BOOL *pbCertEnable);
#endif

/******************************************************************************
  函数名称  : ATP_CWMP_GetInformEnable
  功能描述  : 获取是否使能周期Inform标志
  输入参数  :
              1. pbInformEnable :   存放是否使能周期Inform的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetInformEnable(VOS_BOOL *pbInformEnable);

/******************************************************************************
  函数名称  : ATP_CWMP_GetInformPeriod
  功能描述  : 获取Inform的周期
  输入参数  :
              1. pulInformPeriod    :   存放Inform周期的指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_GetInformPeriod(VOS_UINT32 *pulInformPeriod);


/*
 *  协议栈名字空间接口
 */

/*封套名字空间结构*/
#define ATP_CWMP_NAMESPACE_LEN_D    (32)

typedef struct tagATP_CWMP_NAME_SAPCE_ST
{
    VOS_CHAR      acSoapEnv[ATP_CWMP_NAMESPACE_LEN_D];
    VOS_CHAR      acSoapEnc[ATP_CWMP_NAMESPACE_LEN_D];
    VOS_CHAR      acSoapXsd[ATP_CWMP_NAMESPACE_LEN_D];
    VOS_CHAR      acSoapXsi[ATP_CWMP_NAMESPACE_LEN_D];
    VOS_CHAR      acCwmp[ATP_CWMP_NAMESPACE_LEN_D];
} ATP_CWMP_NAME_SAPCE_ST;

/******************************************************************************
  函数名称  : ATP_CWMP_SetNs
  功能描述  : 设置协议栈的SOAP的名字空间
  输入参数  :
              1. pstNs  :   名字空间结构体指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetNs(const ATP_CWMP_NAME_SAPCE_ST *pstNs);

/******************************************************************************
  函数名称  : ATP_CWMP_GetNs
  功能描述  : 获取协议栈当前所使用的名字空间结构题指针
  输入参数  : 无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 协议栈当前所使用的名字空间结构题指针

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
const ATP_CWMP_NAME_SAPCE_ST *ATP_CWMP_GetNs(VOS_VOID);


/*
 *  RPC注册接口
 */

/* RPC方法处理函数指针 */
typedef VOS_UINT32 (*PFATPCWMPRpcMethod)(
                        const ATP_CWMP_NAME_SAPCE_ST  *pstNs,
                        NODEPTR                       hRpcRequest,
                        NODEPTR                       *phRpcResponse);

/*Start of ATP 2008-12-3 for AU4D01174 by z65940: CPE的GetRPCMethod response的返回值不符合协议标准*/
#if 1
/******************************************************************************
  函数名称  : ATP_CWMP_RegRPC
  功能描述  : 注册外部RPC方法，外部函数
  输入参数  :
              1. pcRPCName  :   RPC方法名称，不能为空，不能超过256字节
              2. pfMethod   :   RPC方法处理函数
              3. bDirection :   RPC方法方向，VOS_TRUE表示CPE提供的RPC；
              				VOS_FALSE表示ACS提供的RPC
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2008-12-3
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_RegRPCEx(const VOS_CHAR				*pcRPCName,
									   PFATPCWMPRpcMethod   		pfMethod,
									   VOS_BOOL						bDirection);
#endif
/*End of ATP 2008-12-3 for AU4D01174 by z65940*/

/******************************************************************************
  函数名称  : ATP_CWMP_RegRPC
  功能描述  : 注册外部RPC方法，外部函数
  输入参数  :
              1. pcRPCName  :   RPC方法名称，不能为空，不能超过256字节
              2. pfMethod   :   RPC方法处理函数
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-7-16
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
#define ATP_CWMP_RegRPC(pcRPCName, pfMethod) ATP_CWMP_RegRPCEx((pcRPCName), (pfMethod), VOS_TRUE)

/*
 *  Inform相对接口
 */

/*
 *  设置DeviceID接口
 */
typedef struct tagATP_CWMP_DEVICEID_ST
{
    VOS_CHAR            *pcManufacturer;
    VOS_CHAR            *pcOUI;
    VOS_CHAR            *pcProductClass;
    VOS_CHAR            *pcSerialNumber;
} ATP_CWMP_DEVICEID_ST;

/******************************************************************************
  函数名称  : ATP_CWMP_SetDeviceID
  功能描述  : 设置Inform上报的DeviceID结构值
  输入参数  :
              1. pstDeviceID    :   设置的值
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetDeviceID(const ATP_CWMP_DEVICEID_ST *pstDeviceID);

/*
 *  Inform上报参数列表接口
 */

/******************************************************************************
  函数名称  : ATP_CWMP_AddParameterEx
  功能描述  : 初始化时添加上报参数，每次会话都会上报该参数。只能在初始化时使用
  输入参数  :
              1. pcParaName     :   上报的参数名称
              2. pcParaValue    :   上报的参数值
              3. enValueType    :   上报参数值的类型
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-20
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_AddParameterEx(
                                const VOS_CHAR              *pcParaName,
                                const VOS_CHAR              *pcParaValue,
                                ATP_CWMP_ATTR_VALUE_TYPE_E  enValueType);

/******************************************************************************
  函数名称  : ATP_CWMP_AddParameterEx
  功能描述  : 初始化时添加上报参数，每次会话都会上报该参数。只能在初始化时使用
  输入参数  :
              1. pcParaName     :   上报的参数名称
              2. pcParaValue    :   上报的参数值
              3. enValueType    :   上报参数值的类型
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-20
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
#define ATP_CWMP_AddParameter(pcParaName, pcParaValue) \
            ATP_CWMP_AddParameterEx((pcParaName), (pcParaValue), ATP_CWMP_VALUE_TYPE_CHAR)

/******************************************************************************
  函数名称  : ATP_CWMP_AddParaValueChgEx
  功能描述  : 参数改变时，添加上报参数，只在该会话中上报，一旦上报完成，则销毁
  输入参数  :
              1. pcParaName     :   上报的参数名称
              2. pcParaValue    :   上报的参数值
              3. enValueType    :   上报参数值的类型
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-20
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_AddParaValueChgEx(
                                const VOS_CHAR              *pcParaName,
                                const VOS_CHAR              *pcParaValue,
                                ATP_CWMP_ATTR_VALUE_TYPE_E  enValueType);

/******************************************************************************
  函数名称  : ATP_CWMP_AddParaValueChg
  功能描述  : 添加Inform上报参数，添加后只会上报一次，上报完后自动被删除
              默认上报的参数值类型为string
  输入参数  :
              1. pcParaName     :   上报的参数名称，不能为空，不要超过256字节
              2. pcParaValue    :   上报的参数值，可以为空
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
#define ATP_CWMP_AddParaValueChg(pcParaName, pcParaValue) \
            ATP_CWMP_AddParaValueChgEx((pcParaName), (pcParaValue), ATP_CWMP_VALUE_TYPE_CHAR)

/******************************************************************************
  函数名称  : ATP_CWMP_SetConnectionIP
  功能描述  : 添加TR069所绑定的IPConnection或PPPConnection的名称和IP值，
              提供本接口是为了保证上报时，每次TR069绑定的IP在上报参数列表中都是
              最后一个，弥补SupportSoft ACS的Bug
  输入参数  :
              1. pcParaName     :   TR069所绑定的名称，不要超过256字节
              2. pcParaValue    :   IP值
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetConnectionIP(const VOS_CHAR *pcParaName,
                                              const VOS_CHAR *pcParaValue);

/******************************************************************************
  函数名称  : ATP_CWMP_DelParameter
  功能描述  : 从每次都需要上报的参数列表中删除上报的参数
  输入参数  :
              1. pcParaName :   待删除的参数名称
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_VOID ATP_CWMP_DelParameter(const VOS_CHAR   *pcParaName);

/*
 *  Inform上报事件接口
 */

/* 事件名称 */
#define ATP_CWMP_EVT_BOOTSTRAP_STR_D     "0 BOOTSTRAP"
#define ATP_CWMP_EVT_BOOT_STR_D          "1 BOOT"
#define ATP_CWMP_EVT_PERIODIC_STR_D      "2 PERIODIC"
#define ATP_CWMP_EVT_SCHEDULED_STR_D     "3 SCHEDULED"
#define ATP_CWMP_EVT_VALUECHANGE_STR_D   "4 VALUE CHANGE"
#define ATP_CWMP_EVT_KICKED_STR_D        "5 KICKED"
#define ATP_CWMP_EVT_CONNREQ_STR_D       "6 CONNECTION REQUEST"
#define ATP_CWMP_EVT_TRANSCOMPLETE_STR_D "7 TRANSFER COMPLETE"
#define ATP_CWMP_EVT_DIAGCOMPLETE_STR_D  "8 DIAGNOSTICS COMPLETE"
#define ATP_CWMP_EVT_REQDOWNLOAD_STR_D   "9 REQUEST DOWNLOAD"
#define ATP_CWMP_EVT_MREBOOT_STR_D       "M Reboot"
#define ATP_CWMP_EVT_MSCHEDULE_STR_D     "M ScheduleInform"
#define ATP_CWMP_EVT_MDOWNLOAD_STR_D     "M Download"
#define ATP_CWMP_EVT_MUPLOAD_STR_D       "M Upload"

/* 会话重试策略 */
typedef enum tagATP_CWMP_EVT_RETRY_POLICY_EN
{
    ATP_CWMP_EVT_RETRY_EXCLUSIVE,       // 必须重试，且不和其它事件共存，
                                        // 如O BOOTSTRAP事件
    ATP_CWMP_EVT_RETRY_FOREVER,         // 即使掉电重启也需要重试的事件
    ATP_CWMP_EVT_RETRY_ALIVE,           // 掉电后不需要重试的事件
    ATP_CWMP_EVT_RETRY_NO,              // 不需要重试的事件
    ATP_CWMP_EVT_RETRY_ARBITRARY        // 没有定义的事件
} ATP_CWMP_EVT_RETRY_POLICY_EN;

/******************************************************************************
  函数名称  : ATP_CWMP_RegEvent
  功能描述  : 向协议栈新增一个用户扩展事件
  输入参数  :
              1. pcEvtName          :   事件名称，不允许为空，不要超过256字节
              2. pcConfirmRPC       :   事件确认被ACS接收的RPC方法名称
              3. pcRequestRPC       :   事件发送完后需要发送给ACS的请求名称
              4. pcRelEvt           :   相关事件的名称
              5. eRetryPolicy       :   事件重试策略
              6. ucSingleOrMultiple :   事件是否可以累积
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              1. VOS_OK                             : 注册成功
              2. ATP_ERR_CWMP_STK_PARA_INVALID      : 参数为空指针
              3. ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH    : 内存错误
              4. ATP_ERR_CWMP_STK_RPC_NOT_FOUND     : RPC没有找到
              5. ATP_ERR_CWMP_STK_INVALID_EVT       : 非法的关联事件

  修改历史      :
   1.日    期   : 2007-7-4
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_RegEvent(
                                const VOS_CHAR          *pcEvtName,
                                const VOS_CHAR          *pcConfirmRPC,
                                const VOS_CHAR          *pcRequestRPC,
                                const VOS_CHAR          *pcRelEvt,
                                ATP_CWMP_EVT_RETRY_POLICY_EN    eRetryPolicy,
                                VOS_BOOL                bSingleOrMultiple);

/******************************************************************************
  函数名称  : ATP_CWMP_AddEvt
  功能描述  : TR069协议栈在运行时添加事件，这些事件会上报给ACS，
              如果该事件有请求RPC，则还会发送RPC请求。
  输入参数  :
              1. pcEvtName      :   事件名称
              2. pcCmdKey       :   该事件的CommandKey，可以为空，不超过32字节
              3. hstRPCArg      :   对应的RPC方法的输入DOM树
              4. bActiveInform  :   是否立即发起会话
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              1. VOS_OK                             : 添加成功
              2. ATP_ERR_CWMP_STK_PARA_INVALID      : 参数非法
              3. ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH    : 内存错误
              4. ATP_ERR_CWMP_STK_INVALID_EVT       : 事件没有注册

  修改历史      :
   1.日    期   : 2007-7-4
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_AddEvent(
                                const VOS_CHAR          *pcEvtName,
                                const VOS_CHAR          *pcCmdKey,
                                NODEPTR                 hstRPCArg,
                                VOS_BOOL                bActiveInform);

/*
 *  RPC DOM树构造函数
 */
typedef enum tagATP_CWMP_CPE_FAULT_CODES_EN
{
    ATP_CWMP_RPC_CPE_OK                                 = 0,
    ATP_CWMP_RPC_CPE_METHOD_NOT_SPT                     = 9000,
    ATP_CWMP_RPC_CPE_REQ_DENIED                         = 9001,
    ATP_CWMP_RPC_CPE_INTER_ERROR                        = 9002,
    ATP_CWMP_RPC_CPE_INVALID_ARG                        = 9003,
    ATP_CWMP_RPC_CPE_RES_EXCD                           = 9004,
    ATP_CWMP_RPC_CPE_INVALID_PARAM_NAME                 = 9005,
    ATP_CWMP_RPC_CPE_INVALID_PARAM_TYPE                 = 9006,
    ATP_CWMP_RPC_CPE_INVALID_PARAM_VALUE                = 9007,
    ATP_CWMP_RPC_CPE_ATMPT_SET_NON_WRITE_PARAM          = 9008,
    ATP_CWMP_RPC_CPE_NOTFY_REQ_REJ                      = 9009,
    ATP_CWMP_RPC_CPE_DOWNLOAD_FAIL                      = 9010,
    ATP_CWMP_RPC_CPE_UPLOAD_FAIL                        = 9011,
    ATP_CWMP_RPC_CPE_FILE_TRANF_AUTH_FAIL               = 9012,
    ATP_CWMP_RPC_CPE_UNSPTED_PROT_FOR_FILE_TRAN         = 9013,

    ATP_CWMP_RPC_CPE_INVALID_SCHEMA                     = 9037,     // Extended
    ATP_CWMP_RPC_CPE_MAX_MASK                           = 0XFFFF
} ATP_CWMP_CPE_FAULT_CODES_EN;

/******************************************************************************
  函数名称  : ATP_CWMP_GetErrStringByCode
  功能描述  : 根据错误码获取错误描述字符串
  输入参数  :
              1. ulErrCode  :   错误码
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 错误字符串，没有找到时，返回"Unknown Error"

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
const VOS_CHAR *ATP_CWMP_GetErrStringByCode(
                                VOS_UINT32             ulErrCode);

/******************************************************************************
  函数名称  : ATP_CWMP_BuildErrorNode
  功能描述  : 构造SOAP错误DOM树，格式如下:
      <soap:Fault>
          <faultcode>Server</faultcode>
          <faultstring>CWMP fault</faultstring>
          <detail>
              <cwmp:Fault>
                  <FaultCode>8003</FaultCode>
                  <FaultString>Invalid arguments</FaultString>
              </cwmp:Fault>
          </detail>
      </soap:Fault>
  输入参数  :
              1. ulErrCode  :   错误码
              2. pstNs      :   名字空间
  调用函数  :
  被调函数  :
  输出参数  :
              1. phError    :   存放构造的DOM树的指针
  返 回 值  :
              VOS_OK
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_BuildErrorNode(
                                   NODEPTR                        *phError,
                                   VOS_UINT32                     ulErrCode,
                                   const ATP_CWMP_NAME_SAPCE_ST   *pstNs);

/******************************************************************************
  函数名称  : ATP_CWMP_ErrorAddPara
  功能描述  : 往构造好的SOAP错误DOM树中添加错误参数
            <cwmp:Fault>
                      <FaultCode>9003</FaultCode>
                      <FaultString>Invalid arguments</FaultString>
                      <SetParameterValuesFault>
                          <ParameterName>
                              InternetGatewayDevice.Time.LocalTimeZone
                          </ParameterName>
                          <FaultCode>9012</FaultCode>
                          <FaultString>...</FaultString>
                      </SetParameterValuesFault>
             _________<SetParameterValuesFault>
            |             <ParameterName>
          添|  pcErrPara:     InternetGatewayDevice.Time.LocalTimeZoneName
          加|             </ParameterName>
          部|  ulErrCode: <FaultCode>9012</FaultCode>
          分|             <FaultString>...</FaultString>
            |_________</SetParameterValuesFault>
                  </cwmp:Fault>
          </detail>
          </soap:Fault>

  输入参数  :
              1. hError     :   已经构造好的错误DOM树
              2. pcErrPara  :   错误参数，不能为空，不要超过256字节
              3. ulErrCode  :   错误码
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_ErrorAddPara(NODEPTR             hError,
                            const VOS_CHAR           *pcErrPara,
                            VOS_UINT32               ulErrCode);

typedef enum tagCWMP_NEXT_LEVEL_TYPE_EN
{
    CWMP_NEXT_LEVEL_TYPE_NONE,
    CWMP_NEXT_LEVEL_TYPE_STATUS_0,
    CWMP_NEXT_LEVEL_TYPE_STATUS_1,
    CWMP_NEXT_LEVEL_TYPE_PARALIST
} CWMP_NEXT_LEVEL_TYPE_EN;

/******************************************************************************
  函数名称  : ATP_CWMP_CreateRpcTopNode
  功能描述  : 构造外层DOM树，其格式如下:
CWMP_NEXT_LEVEL_TYPE_STATUS_0/CWMP_NEXT_LEVEL_TYPE_STATUS_1:
    <cwmp:SetParameterValuesResponse>       // phTopNode
          <Status>0</Status>                // phNextLevelNode
    </cwmp:SetParameterValuesResponse>

CWMP_NEXT_LEVEL_TYPE_NONE:
    <cwmp:SetParameterAttributesResponse>   // phTopNode
    </cwmp:SetParameterAttributesResponse>

CWMP_NEXT_LEVEL_TYPE_PARALIST:
    <cwmp:GetParameterValuesResponse>       // phTopNode
        <ParameterList>                     // phNextLevelNode
        </ParameterList>
    </cwmp:GetParameterValuesResponse>

  输入参数  :
              1. pcRpcName      :   RPC名称，不能为空，不要超过256字节
              2. bResponse      :   是否为Response
              3. ulNextLevelType:   构造类型，见描述
              4. pstNs          :   名字空间，不能超过256字节
  调用函数  :
  被调函数  :
  输出参数  :
              1. phTopNode      :   存放外层DOM树指针的指针，不能为空
              2. phNextLevelNode:   存放内层DOM树指针的指针，可以为空
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_CreateRpcTopNode(
                        const VOS_CHAR                  *pcRpcName,
                        VOS_BOOL                        bResponse,
                        VOS_UINT32                      ulNextLevelType,
                        const ATP_CWMP_NAME_SAPCE_ST    *pstNs,
                        NODEPTR                         *phTopNode,
                        NODEPTR                         *phNextLevelNode);

/******************************************************************************
  函数名称  : ATP_CWMP_SetArrayTypeAttr
  功能描述  : 设置ParameterList属性
    <cwmp:GetParameterValuesResponse>
                      |pcTypeNs|                             |-ulArrayNum-|
        <ParameterList soapenc:arrayType="cwmp:ParameterValueStruct[1]">
                       |------添加部分---------|                  |---|
            <ParameterValueStruct>             |----pcArrayType---|
                <Name>InternetGatewayDevice.NumberOfWANDevices</Name>
                <Value></Value>
            </ParameterValueStruct>
        </ParameterList>
    </cwmp:GetParameterValuesResponse>

  输入参数  :
              1. hParaList      :   已经构造好的ParameterList树DOM指针
              2. pcTypeNs       :   ParameterList中每个参数类型的名字空间，
                                    不要超过256字节
              3. pcArrayType    :   ParameterList中每个参数的类型
              4. ulArrayNum     :   ParameterList中参数的个数
              5. pstNs          :   名字空间
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetArrayTypeAttr(
                        NODEPTR                         hParaList,
                        const VOS_CHAR                  *pcTypeNs,
                        const VOS_CHAR                  *pcArrayType,
                        VOS_UINT32                      ulArrayNum,
                        const ATP_CWMP_NAME_SAPCE_ST    *pstNs);

/******************************************************************************
  函数名称  : ATP_CWMP_AppendParaInfoNode
  功能描述  : 往ParameterList中添加参数
    <cwmp:GetParameterNamesResponse>
        <ParameterList SOAP-ENC:arrayType="cwmp:ParameterInfoStruct[1]">
        _
     添 |   <ParameterInfoStruct>
     加 |       <Name>InternetGatewayDevice.WANDevice</Name>    // pcParaName
     部 |       <Writable>0</Writable>                          // bWritable
     分 |   </ParameterInfoStruct>
        -
        </ParameterList>
    </cwmp:GetParameterNamesResponse>

  输入参数  :
              1. hParaList  :   已经构造好的ParameterList树DOM指针
              2. pcParaName :   待添加参数的名称，不能为空，不要超过256字节
              3. bWritable  :   待添加参数是否可写
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_AppendParaInfoNode(
                                        NODEPTR         hParaList,
                                        const VOS_CHAR  *pcParaName,
                                        VOS_BOOL        bWritable);

/******************************************************************************
  函数名称  : ATP_CWMP_AppendParaValueStructNode
  功能描述  : 添加ParameterValueStruct结构
  <cwmp:GetParameterValuesResponse>
      <ParameterList soapenc:arrayType="cwmp:ParameterValueStruct[1]">
      _
   添 | <ParameterValueStruct>
   加 |   <Name>InternetGatewayDevice.NumberOfWANDevices</Name> // pcParaName
   部 |   <Value>1</Value>                                      // pcParaValue
   分 | </ParameterValueStruct>
      -
      </ParameterList>
  </cwmp:GetParameterValuesResponse>

  输入参数  :
              1. hParaList      :   已经构造好的ParameterList树DOM指针
              2. pcParaName     :   待添加的参数名称，不能为空，不要超过256字节
              3. pcParaValue    :   待添加的参数值
              4. enValueType    :   参数值的类型
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_AppendParaValueStructNode(
                                NODEPTR                     hParaList,
                                const VOS_CHAR              *pcParaName,
                                const VOS_CHAR              *pcParaValue,
                                ATP_CWMP_ATTR_VALUE_TYPE_E   enValueType);

/******************************************************************************
  函数名称  : ATP_CWMP_BuildTransCompleteNodes
  功能描述  : 创建TransferComplete DOM树
    <cwmp:TransferComplete>
        <CommandKey></CommandKey>           // pcCmdKey
        <FaultStruct>
            <FaultCode></FaultCode>         // ulErrCode
            <FaultString></FaultString>
        </FaultStruct>
        <StartTime></StartTime>             // pcStartTime
        <CompleteTime></CompleteTime>       // pcEndTime
    </cwmp:TransferComplete>

  输入参数  :
              1. pcCmdKey       :   RPC的CommandKey
              2. pcStartTime    :   传输开始时间，可以为空
                                    为空时，使用默认时间"0001-01-01T00:00:00Z"
              3. pcEndTime      :   传输结束时间，可以为空
                                    为空时，使用默认时间"0001-01-01T00:00:00Z"
              4. ulErrCode      :   传输错误码
              5. pstNs          :   名字空间
  调用函数  :
  被调函数  :
  输出参数  :
              1. hOutNode       :   构造好的DOM树指针
  返 回 值  :
              VOS_OK
              ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_MEM_NOT_ENOUGH
              TSP XML组件错误码

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_BuildTransCompleteNodes(
                                    const VOS_CHAR  *pcCmdKey,
                                    const VOS_CHAR  *pcStartTime,
                                    const VOS_CHAR  *pcEndTime,
                                    VOS_UINT32      ulErrCode,
                                    NODEPTR         *hOutNode,
                                    const ATP_CWMP_NAME_SAPCE_ST  *pstNs);

/*
 *  CWMP调试信息接口
 */

/******************************************************************************
  函数名称  : ATP_CWMP_StkGetVersion
  功能描述  : 获取CWMP协议栈组件的版本号
  输入参数  :
              1. pszVersion :   存放版本号字符串的指针，不允许为空
              2. ulLen      :   缓冲区长度
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-1-3
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_StkGetVersion(
                            VOS_INT8        *pszVersion,
                            VOS_UINT32      ulLen);

#define USE_CWMP_TRACE

#ifdef USE_CWMP_TRACE
/******************************************************************************
  函数名称  : ATP_CWMP_Outputf
  功能描述  : 调试信息打印函数，可通过网络重定向
  输入参数  :
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-12-30
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_Outputf(const VOS_INT8 *app, VOS_UINT32 level,
                                     VOS_UINT32 type, VOS_UINT32 logno,
                                     const VOS_INT8 *file, VOS_UINT32 line,
                                     const VOS_INT8 *fmt, ...);

#define ATP_CWMP_TRACE              ATP_CWMP_Outputf

#ifdef DEBUG
#define ATP_CWMP_DEBUG              ATP_CWMP_Outputf
#else
#define ATP_CWMP_DEBUG(x...)
#endif

#else

#define ATP_CWMP_TRACE              VTOP_LOG_Outputf

#ifdef DEBUG
ULONG ATP_CWMP_Outputf(const VOS_INT8 *app, VOS_UINT32 level,
                                     VOS_UINT32 type, VOS_UINT32 logno,
                                     const VOS_INT8 *file, VOS_UINT32 line,
                                     const VOS_INT8 *fmt, ...);

#define ATP_CWMP_DEBUG              ATP_CWMP_Outputf
#else
#define ATP_CWMP_DEBUG(x...)
#endif

#endif

#define ATP_CWMP_ENABLE_HTTP_AUTH

/*
 *  C02 新增特性
 */
#ifdef ATP_CWMP_ENABLE_SSL

/* 证书认证模式 */
typedef enum tagATP_CWMP_SSL_MODE
{
    ATP_CWMP_SSL_CERT_AUTH_NONE,        // 不进行证书认证
    ATP_CWMP_SSL_CERT_AUTH_PEER,        // 需要对对方进行证书认证，
                                        // 必须提供本地信任的根证书
    ATP_CWMP_SSL_CERT_AUTH_BY_PEER, // 需要被对端认证，
                                        // 需要提供本地的证书以及私钥
    ATP_CWMP_SSL_CERT_AUTH_BOTH,        // 双向认证，需要提供上述两个证书
    ATP_CWMP_SSL_CERT_AUTH_MAX
} ATP_CWMP_SSL_MODE;

/* SSL版本 */
typedef enum tagATP_CWMP_SSL_VERSION
{
    ATP_CWMP_SSL_ANY_EXCLUDE_V2,        // 排除V2，自适应使用V3或者TLS，安全性更好
    ATP_CWMP_SSL_ANY,                   // 自适应使用V2、V3或者TSL
    ATP_CWMP_SSL_TLS,                   // TLS
    ATP_CWMP_SSL_V3,
    ATP_CWMP_SSL_MAX
} ATP_CWMP_SSL_VERSION;


/* SSL证书结构体 */
typedef struct tagATP_CWMP_SSL_CERTS_ST
{
	VOS_CHAR		*pcRootCertPath;		// 本地根证书路径
	VOS_CHAR		*pcCertPath;			// 本地证书路径
	VOS_CHAR		*pcKeyPath;				// 本地证书对应的私钥路径
	VOS_CHAR		*pcKeyPassword;			// 本地私钥的加密密码，如果私钥有加密，
	                                        // 则必须提供此项
} ATP_CWMP_SSL_CERTS_ST;

/* SSL配置结构体 */
typedef struct tagATP_CWMP_SSL_CONFIG_ST
{
	VOS_UINT8	ucClientAuthMode;	// CWMP作为客户端的认证模式，取ATP_CWMP_SSL_MODE枚举值
	VOS_UINT8	ucServerAuthMode;	// CWMP作为服务器的认证模式，取ATP_CWMP_SSL_MODE枚举值
	VOS_UINT8	ucClientSSLVersion;		// 客户端SSL版本，取ATP_CWMP_SSL_VERSION枚举值
	VOS_UINT8	ucServerSSLVersion;		// 服务器SSL版本号
	ATP_CWMP_SSL_CERTS_ST	*pstClientCerts;	// 客户端的证书路径
	ATP_CWMP_SSL_CERTS_ST	*pstServerCerts;	// 服务器的证书路径，如果和客户端相同，则可以为空
} ATP_CWMP_SSL_CONFIG_ST;

/* 回连模式 */
typedef enum tagATP_CWMP_CON_REQ_MODE
{
    ATP_CWMP_CON_REQ_MODE_HTTP,     // HTTP方式回连
    ATP_CWMP_CON_REQ_MODE_HTTPS,        // HTTPS方式回连
    ATP_CWMP_CON_REQ_MODE_BOTH,     // 两者都支持，但上报的是HTTP方式的URL
    ATP_CWMP_CON_REQ_MODE_MAX
} ATP_CWMP_CON_REQ_MODE;

/******************************************************************************
  函数名称  : ATP_CWMP_RefreshSSLContext
  功能描述  : 更新SSL的上下文，当需要更新证书时，需要使用此接口来更新证书
  输入参数  :
              无
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : ATP_ERR_CWMP_STK_NET_SSL
              ATP_ERR_CWMP_STK_PARA_INVALID
              VOS_OK

  修改历史      :
   1.日    期   : 2008-4-15
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_RefreshSSLContext();

/******************************************************************************
  函数名称  : ATP_CWMP_ConfigSSL
  功能描述  : 配置CWMP组件的客户端和服务器的SSL特性
  输入参数  :
              1. pstConfig  :   SSL特性结构体
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : ATP_ERR_CWMP_STK_NET_SSL
              ATP_ERR_CWMP_STK_PARA_INVALID
              VOS_OK

  修改历史      :
   1.日    期   : 2008-4-15
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_ConfigSSL(ATP_CWMP_SSL_CONFIG_ST *pstConfig);

#ifdef ATP_CWMP_ENABLE_SERVER_SSL
/******************************************************************************
  函数名称  : ATP_CWMP_SetConReqMode
  功能描述  : 配置CWMP组件的服务器的回连方式
  输入参数  :
              1. enMode :   回连方式值
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : ATP_ERR_CWMP_STK_PARA_INVALID
              ATP_ERR_CWMP_STK_NET_SOCK
              ATP_ERR_CWMP_STK_NET_SSL

  修改历史      :
   1.日    期   : 2008-4-15
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_SetConReqMode(ATP_CWMP_CON_REQ_MODE enMode);
#endif

/*
 * 证书认证函数，返回VOS_TRUE表示证书认证通过；返回VOS_FALSE表示证书认证失败
 * bClient表示调用者是客户端还是服务器，VOS_TRUE表示客户端、VOS_FALSE表示服务器
 * ok表示OpenSSL内置的证书校验是否已经通过
 * store是SSL证书结构，可以根据该结果获取对端证书，并用OpenSSL接口做校验。
 */
typedef VOS_INT32 (*PFATPCWMPSSLCertCheck)(VOS_BOOL bClient, VOS_INT32 ok,
                                                         X509_STORE_CTX *store);

/******************************************************************************
  函数名称  : ATP_CWMP_RegSSLCertCheckFunc
  功能描述  : 注册证书认证扩展函数，多次调用时，以最后一次的为准
  输入参数  :
              1. pfCheckCert    :   扩展的认证函数指针
  调用函数  :
  被调函数  :
  输出参数  : 无
  返 回 值  : VOS_OK

  修改历史      :
   1.日    期   : 2008-4-15
     作    者   : handy
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32 ATP_CWMP_RegSSLCertCheckFunc(PFATPCWMPSSLCertCheck pfCheckCert);

#endif

extern VOS_UINT16 g_usLocalPortMin;
extern VOS_UINT16 g_usLocalPortRng;

/*
 * TR069协议栈创建socket时，额外的socket操作函数
 *      可以在此回调函数中加入QoS标签
 * lSockDesc : 新创建的socket描述符
 * usPort       : 新创建的socket所绑定的端口，没有绑定时为0
 */
typedef VOS_VOID (*PFCwmpExtraSocketOp)(
                                VOS_INT32       lSockDesc,
                                VOS_UINT16      usPort);

/******************************************************************************
  函数名称  : ATP_CWMP_RegExtraSockOp
  功能描述  : 注册TR069协议栈创建socket时的额外处理函数
  输入参数  : 
              1.  pfFunc : 待处理的回调函数
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-9-26
     作    者   : z65940
     修改内容   : 完成初稿

******************************************************************************/
VOS_UINT32  ATP_CWMP_RegExtraSockOp(PFCwmpExtraSocketOp pfFunc);
/*End of MNT 2008-10-25 10:52 for A36D05726 by z65940*/

#endif

