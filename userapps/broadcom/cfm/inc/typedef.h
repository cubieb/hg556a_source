/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2004, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: v_typdef.h                                                      */
/*                                                                           */
/* Author:                                                                   */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 1999-12-21                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification: Create this file                                         */
/*                                                                           */
/* 2. Date: 2000-03_06                                                       */
/*    Author: Qin Peifeng                                                    */
/*    Modification: Standardize code                                         */
/*                                                                           */
/* 3. Date: 2004-09-21                                                       */
/*    Author: Zheng Wenqin                                                   */
/*    Modification: Move CPU_TICK, DATE_T, TIME_T, SYS_T to this file.       */
/*                                                                           */
/*****************************************************************************/

#ifndef _TYPDEF_H
#define _TYPDEF_H


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


/************************* MACRO & TYPE DEFINITION ***************************/
#define VOS_NULL               (0)
#define VOS_NULL_PTR           (0L)
#define VOS_NULL_BYTE          (0XFF)
#define VOS_NULL_WORD          (0xFFFF)
#define VOS_NULL_DWORD         (0xFFFFFFFF)
#define VOS_NULL_LONG          (VOS_NULL_DWORD)
#define VOS_NULL_OBJ           (-1)
#define VOS_NULL_PARA          (-1)

#define VOS_OK                 (0)
#define VOS_ERR                (1)                 /* For VRP VOS adaptation */
#define VOS_ERROR              (-1)


typedef void                   VOS_VOID;


typedef unsigned char          VOS_UINT8;

typedef signed char            VOS_INT8;

typedef char                   VOS_CHAR;           /* 8位ASCII值 */

typedef unsigned char          VOS_UCHAR;


typedef unsigned short         VOS_UINT16;

typedef signed   short         VOS_INT16;

typedef signed   short         VOS_WCHAR;          /* 16位Unicode值 */


typedef unsigned long          VOS_UINT32;

typedef signed   long          VOS_INT32;

typedef unsigned long          VOS_BOOL;


#if( VOS_HARDWARE_PLATFORM == VOS_MIPS )

typedef unsigned long  long    VOS_UINT64;

typedef signed   long  long    VOS_INT64;

#endif


typedef int                    VOS_INT;

typedef unsigned int           VOS_UINT;


typedef float                  VOS_FLOAT;          /* 32位符点数 */

typedef double                 VOS_DOUBLE;         /* 64位符点数 */



typedef unsigned long          VOS_PID;

typedef unsigned long          VOS_FID;

typedef unsigned long          SIZE_T;

typedef VOS_UINT32             VOS_SOCKET;

typedef VOS_UINT32             HTIMER;

/* yuqinyun for IAD added (adapt) FID,PID*/
typedef unsigned long           PID;

typedef unsigned long           FID;


typedef struct tagCpuTick
{
    VOS_UINT32  ulLow;
    VOS_UINT32  ulHigh;
}CPU_TICK;


typedef struct DATE_STRU
{
    VOS_UINT16  uwYear;    /* year  */
    VOS_UINT8   ucMonth;   /* month */
    VOS_UINT8   ucDate;    /* day   */
} DATE_T;


typedef struct TIME_STRU
{
    VOS_UINT8   ucHour;    /* hour   */
    VOS_UINT8   ucMinute;  /* minute */
    VOS_UINT8   ucSecond;  /* second */
    VOS_UINT8   ucPadding;
} TIME_T;


typedef struct tagSysTime
{
    VOS_UINT16  uwYear;
    VOS_UINT8   ucMonth;   /* scope is 1 - 12 */
    VOS_UINT8   ucDate;    /* scope is 1 - 31 */
    VOS_UINT8   ucHour;    /* scope is 0 - 23 */
    VOS_UINT8   ucMinute;  /* scope is 0 - 59 */
    VOS_UINT8   ucSecond;  /* scope is 0 - 59 */
    VOS_UINT8   ucWeek;    /* scope is 1 - 7  */
} SYS_T;


enum _BOOL_DEFINE
{
    VOS_FALSE = 0,
    VOS_TRUE = 1
};


#if ( VOS_OS_VER == VOS_WIN32 )

#define far

#ifdef FAR
#undef FAR
#define FAR
#else
#define FAR
#endif

#define near

#ifdef NEAR
#undef NEAR
#define NEAR
#else
#define NEAR
#endif

#elif ( VOS_OS_VER == VOS_PSOS )

#define far
#define FAR
#define near
#define NEAR

#elif ( VOS_OS_VER == VOS_VXWORKS )

#define FAR 
#define far
#define near
#define NEAR

typedef unsigned int pthread_t;

#elif ( VOS_OS_VER == VOS_LINUX )

#define FAR 
#define far
#define near
#define NEAR
#define LINUX_KEY_BASE    1024 
 
#define TRUE 1
#define FALSE 0 
typedef VOS_INT32             LINUX_MSGQUEUE;
typedef void *( * _LINUX_TASK_ENTRY )( void * );

#elif  (VOS_OS_VER == VOS_SOLARIS)
#define FAR 
#define far
#define near
#define NEAR
#define SOLARIS_KEY_BASE    1024 
 
#define TRUE 1
#define FALSE 0 
typedef VOS_INT32             SOLARIS_MSGQUEUE;
typedef void *( * _SOLARIS_TASK_ENTRY )( void * );

#elif  (VOS_OS_VER == VOS_HPUNIX)
#define FAR 
#define far
#define near
#define NEAR
#define HPUX_KEY_BASE    1024 
 
#define TRUE 1
#define FALSE 0 
typedef VOS_INT32             HPUX_MSGQUEUE;
typedef void *( * _HPUX_TASK_ENTRY )( void * );

#elif  (VOS_OS_VER == VOS_IBMUNIX)
#define FAR 
#define far
#define near
#define NEAR
#define IBMUX_KEY_BASE    1024 
 
#define TRUE 1
#define FALSE 0 
typedef VOS_INT32             IBMUX_MSGQUEUE;
typedef void *( * _IBMUX_TASK_ENTRY )( void * );

#elif ( VOS_OS_VER == VOS_UNIX )

#define FAR 
#define far
#define near
#define NEAR
 
#define TRUE 1
#define FALSE 0 

#elif ( VOS_OS_VER == VOS_TLINUX )

#define FAR 
#define far
#define near
#define NEAR
 
#define TRUE 1
#define FALSE 0 

typedef void *( * _TLINUX_TASK_ENTRY )( void * );

#endif /* VOS_OS_VER */




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _TYPDEF_H */




