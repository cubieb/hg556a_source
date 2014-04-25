/***********************************************************************
  版权信息: 版权所有(C) 1988-2006, 华为技术有限公司.
  文件名: syswatch.h
  作者: l28714
  版本: V1.0
  创建日期: 2006-4-21
  完成日期: 
  功能描述: 
      
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    
  备注: 
************************************************************************/
#ifndef __SYS_MON__
#define __SYS_MON__

#if defined(__cplusplus)
extern "C" {
#endif

/***********************************************************************
*                             宏定义                                   *
************************************************************************/
#define NO_ERROR            0
#define ERR_OPEN_FILE       1
#define ERR_NULL_POINTER    2
#define ERR_INVALID_VALUE   3
#define ERR_OPEN_DIR        4
#define MAX_ERRNO           5

#define NEED_DISPLAY        1
#define NO_DISPLAY          0
/***********************************************************************
*                            数据结构                                  *
************************************************************************/
typedef struct tag_ERROR_NUMBER
{
    int iErrNo;
    char *pchErrInfo;
}S_ERROR_NUMBER;

/* 保存进程信息，包括PID，进程状态，内存占用大小，CPU占用率，进程命令字 */
typedef struct tag_SYSMON_PROCESS_STATUS
{
    int		iPid;       /* 进程ID */
    char	achState[4];    /* 进程状态 */
    long	lMem;       /* 进程内存占用大小(单位KB) */
    int		iCpu;       /* CPU占用率(单位千分之一) */
    char	achCmd[16]; /* 进程执行的命令(不包括参数) */
    int     iDispFlag;  /* 数据是否需要显示的标志 */
    struct tag_SYSMON_PROCESS_STATUS *psNext;
} S_SYSMON_PROCESS_STATUS, *PS_SYSMON_PROCESS_STATUS; 

/* 保存系统资源占用情况 */
typedef struct tag_SYSMON_TOTAL_USAGE
{
    int iTotal;     /* 总内存(单位KB) */
    int iUsed;      /* 使用的内存(单位KB) */
    int iFree;      /* 空闲内存(单位KB) */
    int iBuffers;   /* 缓冲区占用的内存(单位KB) */
    int iUCpu;      /* 用户态CPU占用率(单位千分之一) */
    int iSCpu;      /* 内核态CPU占用率(单位千分之一) */
}S_SYSMON_TOTAL_USAGE, *PS_SYSMON_TOTAL_USAGE;

/***********************************************************************
*                            函数声明                                  *
************************************************************************/
extern void sysmon_main(void);
extern void init_Hertz_value(void);
extern S_SYSMON_PROCESS_STATUS * procps_scan(void);

#if defined(__cplusplus)
}
#endif

#endif /* __SYS_MON__ */
