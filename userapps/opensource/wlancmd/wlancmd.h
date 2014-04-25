/******************************************************************************
  版权所有  : 2007-2020，华为技术有限公司
  文 件 名  : wlancmd.h
  作    者  : l69021
  版    本  : 1.0
  创建日期  : 2007-8-22
  描    述  : 固网终端软件平台WLANCMD命令相关底层接口头文件
  函数列表  :

  历史记录      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

*********************************************************************************/

#ifndef __WLANCMD_H__
#define __WLANCMD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "wlancmdadapter.h"

typedef struct cmd 
{
    char *name;
    cmd_func_t *func;
    char *help;
}wlan_cmd_t;

#define CMD_ERR -1  /* Error for command */
#define CMD_OPT 0   /* a command line option */
#define CMD_WL  1   /* the start of a wl command */

#define INT_FMT_DEC 0   /* signed integer */
#define INT_FMT_UINT    1   /* unsigned integer */
#define INT_FMT_HEX 2   /* hexdecimal */
#define TRUE    1
#define FALSE   0

#define DEV_TYPE_LEN 2 /* length for devtype 'wl'/'et' */

#if __cplusplus
#ifdef __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __WLANCMD_H__ */
