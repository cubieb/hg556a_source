/*                                                                           */
/*                Copyright 2004 - 2050, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileNmae: os_tmr_interface.h                                              */
/* Version : 1.0                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* History:                                                                  */
/* Data            Author            Modification                            */
/*****************************************************************************/
#ifndef __OS_TMR_INTERFACE_H__
#define __OS_TMR_INTERFACE_H__

#include <time.h>
#include "os_def.h"
#include "os_tim_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

typedef VOS_VOID * VTOP_TIMER_ID;

/*timer type*/
typedef enum _VTOP_E_TimerAttr
{
    VTOP_TIMER_LOOP  = 0,       /* the timer start loop */
    VTOP_TIMER_ONE_SHOOT ,      /* the timer start once and then stop */
    VTOP_TIMER_AUTO_FREE        /* the timer start once and then free */
}VTOP_E_TimerAttr;

/* 启动定时器API*/
extern VOS_INT VTOP_StartTimerTask(void * pthread, const void * pattr);
#ifdef VTOP_OS_WIN32
#define VTOP_InitTimer() VTOP_StartTimerTask(VOS_NULL_PTR, VOS_NULL_PTR)
#endif

/*创建定时器API*/
extern VTOP_TIMER_ID VTOP_CreateAbsTimerM(VOS_VOID (*timeout)(void * arg), VTOP_E_TimerAttr attr, VOS_UINT32 ulLineNo, const VOS_CHAR *strFileName);
#define VTOP_CreateAbsTimer(timeout, attr) VTOP_CreateAbsTimerM(timeout, attr, __LINE__, __FILE__) 
extern VTOP_TIMER_ID VTOP_CreateRelTimerM(VOS_VOID (*timeout)(void * arg), VTOP_E_TimerAttr attr, VOS_UINT32 ulLineNo, const VOS_CHAR *strFileName);
#define VTOP_CreateRelTimer(timeout, attr) VTOP_CreateRelTimerM(timeout, attr, __LINE__, __FILE__) 

/*启动定时器API*/
extern VOS_INT VTOP_StartAbsTimer(VTOP_TIMER_ID timer, const VTOP_S_TM * tm, VOS_VOID * arg);
extern VOS_INT VTOP_StartRelTimer(VTOP_TIMER_ID timer, VOS_UINT32 msec, VOS_VOID * arg);
extern VOS_INT VTOP_StartRelTimer_Simple(VOS_UINT32 msec, VOS_VOID (*timeout)(void * arg),VOS_VOID * arg);

/*停止定时器API*/
extern VOS_INT VTOP_StopAbsTimer_Safe(VTOP_TIMER_ID timer, VOS_VOID(* finallize)(VOS_VOID *), VOS_VOID * arg);
#define VTOP_StopAbsTimer(timer)  VTOP_StopAbsTimer_Safe(timer, VOS_NULL_PTR,VOS_NULL_PTR)

extern VOS_INT VTOP_StopRelTimer_Safe(VTOP_TIMER_ID timer, VOS_VOID(* finallize)(VOS_VOID *), VOS_VOID * arg);
#define VTOP_StopRelTimer(timer)  VTOP_StopRelTimer_Safe(timer, VOS_NULL_PTR, VOS_NULL_PTR)

extern VOS_INT VTOP_StopAbsTimer_SafeEx(VTOP_TIMER_ID timer, VOS_VOID(* finallize)(VOS_VOID *), VOS_VOID * parg, VOS_VOID ** pparg);
#define VTOP_StopAbsTimerEx(timer, pparg)  VTOP_StopAbsTimer_SafeEx(timer, VOS_NULL_PTR,VOS_NULL_PTR, pparg)

extern VOS_INT VTOP_StopRelTimer_SafeEx(VTOP_TIMER_ID timer, VOS_VOID(* finallize)(VOS_VOID *), VOS_VOID * parg, VOS_VOID ** pparg);
#define VTOP_StopRelTimerEx(timer, pparg)  VTOP_StopRelTimer_SafeEx(timer, VOS_NULL_PTR, VOS_NULL_PTR, pparg)

/*释放定时器API*/
extern VOS_INT VTOP_FreeAbsTimer(VTOP_TIMER_ID timer);
extern VOS_INT VTOP_FreeRelTimer(VTOP_TIMER_ID timer);

/*修改系统时间接口*/
extern VOS_INT VTOP_SetSysTime(const VTOP_S_TM * pstime);

/*定时器调试信息*/
/*定时器类型*/
#define VTOP_ABS_TIMER  0x1
#define VTOP_REL_TIMER  0x2

/*定时器运行状态*/
#define VTOP_TIMER_RUN  0x1
#define VTOP_TIMER_STOP 0x2

/*定时器维护信息数据结构*/
typedef struct VTOP_TmInfo
{
    VOS_UINT32 ulTmFlag;     /*相对/绝对定时器标志，VTOP_ABS_TIMER为绝对定时器，VTOP_REL_TIMER为相对定时器 */
    VOS_VOID * pulTimerID;   /*定时器指针*/
    VOS_UINT32 ulTmAttr;     /*定时器类型，周期：VTOP_TIMER_LOOP，自动释放：VTOP_TIMER_AUTO_FREE，停止但不释放：VTOP_TIMER_ONE_SHOOT */
    VOS_UINT32 ulTmRunInfo; /*定时器运行状态，运行：VTOP_TIMER_RUN，停止：VTOP_TIMER_STOP*/
    union
    {
        struct tm  stAbsTime; /*绝对定时器超时时刻*/
        VOS_UINT32 ulRelTmLen;/*相对定时器时长*/
    }unTime;
    VOS_UINT32 ulLineNo;      /*申请定时器的文件行号*/
    const VOS_CHAR * pstrFileName;  /*申请定时器的文件名的指针*/
}VTOP_stTmInfo;

typedef struct VTOP_TmNum
{
    VOS_UINT32 ulTotalAbsTm;  /*总共申请的绝对定时器总数，包括已经释放的*/
    VOS_UINT32 ulTotalRelTm;  /*总共申请的相对定时器总数，包括已经释放的*/
    VOS_UINT32 ulCurAbsTm;    /*当前申请的绝对定时器数，不包括已经释放的*/
    VOS_UINT32 ulCurRelTm;    /*当前申请的相对定时器数，不包括已经释放的*/
    VOS_UINT32 ulRunAbsTm;    /*当前正在运行的绝对定时器数*/   
    VOS_UINT32 ulRunRelTm;    /*当前正在运行的绝对定时器数*/
} VTOP_stTmNum;

/*定时器信息数组*/
extern VTOP_stTmInfo astTmInfo[];

/* 定时器调试API*/
/*获得定时器信息数组维护信息接口*/
extern VTOP_stTmInfo * VTOP_GetTmInfo(VTOP_stTmNum * pstTmNum);
/*打印定时器信息数组维护信息接口*/
extern VOS_VOID VTOP_PrintTmInfo(const VTOP_stTmInfo* astTimerInfo, const VTOP_stTmNum * pstTmNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __TMR_INTERFACE_H__ */

