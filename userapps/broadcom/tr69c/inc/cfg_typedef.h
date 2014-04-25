/***********************************************************************
    版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
    文件名:
    作者: Diao.Meng
    版本: HG550V200R001B02
    创建日期: 2005-4-18
    完成日期:
    功能描述:

    主要函数列表:

    修改历史记录列表:
    <作  者>    <修改时间>  <版本>  <修改描述>

    备注:
************************************************************************/


#ifndef _CFG_TYPEDEF_H_
#define _CFG_TYPEDEF_H_

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifdef WIN32
    #ifndef XINLINE
    #define XINLINE
    #endif
#else
    #define XINLINE inline
#endif

typedef enum {
    TRX_OK=0,
    TRX_REBOOT,
    TRX_ERR
}TRX_STATUS;

#ifndef VOID
#define VOID void
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef SHORT
#define SHORT short
#endif

#ifndef USHORT
#define USHORT  unsigned short
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef BOOL
#define BOOL unsigned int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef NULL
#undef NULL
#endif

#define NULL    0
/*Some systems define NULL as (-1), in our system, NULL must be (0). */

typedef CHAR  DATE_TIME;

#define MAX_DATE_TIME_LEN   20      /* YYYY-MM-DDTHH:MM:SS */


#define STRING_LEN_6                     6
#define STRING_LEN_16                   16
#define STRING_LEN_32                   32
#define STRING_LEN_64                   64
#define STRING_LEN_128                 128
#define STRING_LEN_256                 256

/* HUAWEI HGW 张小田 2005年6月30日
  */
/*
#ifdef _DEBUG
#define DBGONLY(x)  x
#else
#define DBGONLY(x)
#endif
*/

/*
 *  如果y > 0, x  取x  和y 之间的最小值
 */
#define MIN(x, y)   ( ((x) < (y)) ? (x) : (y) )
#define MAX(x, y)   ( ((x) < (y)) ? (y) : (x) )

#ifdef  __cplusplus
}
#endif

#endif /*End of the file.*/


