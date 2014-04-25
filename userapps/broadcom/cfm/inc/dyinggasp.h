/***********************************************************************
  版权信息: 版权所有(C) 1988-2006, 华为技术有限公司.
  文件名: dyinggasp.h
  作者: liuzhijie 00028714
  版本: V1.0
  创建日期: 2006-5-10
  完成日期: 
  功能描述: 
      临终遗言特性用户层的宏定义、数据结构和函数声明
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    
  备注: 
************************************************************************/
#ifndef _USER_DYINGGASP_H_
#define _USER_DYINGGASP_H_

#if defined(__cplusplus)
extern "C" {
#endif

/**********************************************/
/* Macro definition                           */
/*                                            */

/* 最大进程调度次数，即记录最后MAX_SCHEDLISTRECORD次进程调度过程 */
#define DG_MAX_SCHEDLISTRECORD                  50

/* 最大未用进程数，用来记录长久没有被调用的进程 */
#define DG_MAX_NOUSEPROCESS                     10

/* 记录的最大报文个数 */
#define DG_MAX_IPPACKAGENUM                     10

/* WEB操作记录最大长度 */
#define DG_MAX_WEB_OP_LEN                       256

/* WEB操作记录数 */
#define DG_MAX_WEB_OP_NUM                       50

/* 命令行操作记录最大长度 */
#define DG_MAX_CLI_OP_LEN                       128

/* 命令行操作记录数 */
#define DG_MAX_CLI_OP_NUM                       50

/* 接口状态改变记录数 */
#define DG_MAX_ITF_STATE_NUM                    100

/* 接口名最大长度 */
#define DG_MAX_ITF_NAME_LEN                     10

/* 模块名最大长度 */
#define DG_MAX_MODULE_NAME                      32

/* 记录最大模块数 */
#define DG_MAX_MODULES_NUM                      50

/* 当前进程堆栈记录长度 */
#define DG_MAX_STACK_LEN                        40

/* 进程运行轨迹记录长度 */
#define DG_MAX_TRACE_LEN                        10

/* 进程当前执行语句上下文记录长度 */
#define DG_MAX_CODE_LEN                         9

/* 记录进程信息最大数目 */
#define DG_MAX_PROCESS_NUM                      50

/* 数据校验字 */
#define DG_RESERVED_SDRAM_CHECKSUM              0x5AA5A55A

#define TAINT_PROPRIETARY_MODULE	(1<<0)
#define TAINT_FORCED_MODULE		(1<<1)
#define TAINT_UNSAFE_SMP		(1<<2)
#define TAINT_FORCED_RMMOD		(1<<3)

#define ST0_IE			0x00000001
#define ST0_EXL			0x00000002
#define ST0_ERL			0x00000004
#define ST0_KSU			0x00000018
#define KSU_USER		0x00000010
#define KSU_SUPERVISOR	0x00000008
#define KSU_KERNEL		0x00000000
#define ST0_UX			0x00000020
#define ST0_SX			0x00000040
#define ST0_KX 			0x00000080
#define ST0_DE			0x00010000
#define ST0_CE			0x00020000
#define  CAUSEB_EXCCODE		2
#define  CAUSEF_EXCCODE		((unsigned long)(31)  <<  2)

/**********************************************/
/* Structure definition                       */
/*                                            */

/* 保存每次调度发生时的进程号 */
struct tag_DG_SCHED_LIST
{
    int           iPid;
};
typedef struct tag_DG_SCHED_LIST S_DG_SCHED_LIST;

/* 保存每个进程最近一次被调度的时间 */
struct tag_DG_SCHED_TIME
{
    int           iPid;
    unsigned long   ulTicks;
    struct tag_DG_SCHED_TIME *psNext;
};
typedef struct tag_DG_SCHED_TIME S_DG_SCHED_TIME;

/* 记录进程调度部分需要记录在临终遗言中的所有内容 */
struct  tag_DG_SCHED_INFO
{
    unsigned long   ulCurNum;                           /*指向stSchededList数组中当前可记录的空闲位置*/
    unsigned long   ulCurPID;                           /*当前运行的进程*/
    unsigned long   ulAllSchedNum;                      /*系统总调度次数*/
    S_DG_SCHED_LIST sSchededList[DG_MAX_SCHEDLISTRECORD]; /*记录进程调度的顺序的数组*/
    S_DG_SCHED_TIME asSchedTime[DG_MAX_NOUSEPROCESS]; /* 记录每个进程最近一次被调度的时间的数组 */
};
typedef struct tag_DG_SCHED_INFO S_DG_SCHED_INFO;

/* 记录系统收到的报文内容 */
struct  tag_DG_IN_PACK_ENTRY
{
    unsigned char   aucDevName[16];
    unsigned char   aucMacInfo[16];
    unsigned char   aucData[1536];
    unsigned long   ulLen;
};
typedef struct  tag_DG_IN_PACK_ENTRY  S_DG_IN_PACK_ENTRY;

/* 循环链表，用来记录系统收到的最后几个报文 */
struct  tag_DG_IN_PACK_INFO
{
    unsigned long           ulCurPosition;
    S_DG_IN_PACK_ENTRY      sIpInputBuf[DG_MAX_IPPACKAGENUM];
};
typedef struct  tag_DG_IN_PACK_INFO S_DG_IN_PACK_INFO;

/* 循环链表，用来记录系统收到的最后几个http操作 */
struct  tag_DG_WEB_OP_INFO
{
    unsigned long           ulCurPosition;
    char                    achWebOp[DG_MAX_WEB_OP_NUM][DG_MAX_WEB_OP_LEN];
};
typedef struct  tag_DG_WEB_OP_INFO S_DG_WEB_OP_INFO;

/* 循环链表，用来记录系统收到的最后几个命令行操作 */
struct  tag_DG_CLI_OP_INFO
{
    unsigned long           ulCurPosition;
    char                    achCliOp[DG_MAX_CLI_OP_NUM][DG_MAX_CLI_OP_LEN];
};
typedef struct  tag_DG_CLI_OP_INFO S_DG_CLI_OP_INFO;

enum E_DG_ITF_STATE
{
    DG_ITF_UP = 0,
    DG_ITF_DOWN,
    DG_XDSL_TRAINING_G992_EXCHANGE,
    DG_XDSL_TRAINING_G992_CHANNEL_ANALYSIS,
    DG_XDSL_TRAINING_G992_STARTED,
    DG_XDSL_TRAINING_G994,
    DG_XDSL_G994_NONSTDINFO_RECEIVED,
    DG_XDSL_BERT_COMPLETE,
    DG_XDSL_ATM_IDLE,
    DG_XDSL_EVENT,
    DG_XDSL_G997_FRAME_RECEIVED,
    DG_XDSL_G997_FRAME_SENT
};

/* 端口状态记录 */
struct tag_DG_ITF_STATE
{
    char achItfName[DG_MAX_ITF_NAME_LEN];   //接口名
    int iItfState;                          //接口状态，0:up 1:down
    int iTime;                              //接口状态改变时间(相对系统启动的时间)
};
typedef struct tag_DG_ITF_STATE S_DG_ITF_STATE;

/* 循环链表，记录最后几次端口状态改变 */
struct tag_DG_ITF_STATE_INFO
{
    unsigned long   ulCurPosition;
    S_DG_ITF_STATE  asItfStat[DG_MAX_ITF_STATE_NUM];    
};
typedef struct tag_DG_ITF_STATE_INFO S_DG_ITF_STATE_INFO;

struct tag_DG_REG
{
    unsigned long cpu;
	/* Saved main processor registers. */
	unsigned long regs[32];

	/* Saved special registers. */
	unsigned long cp0_status;
	unsigned long lo;
	unsigned long hi;
	unsigned long cp0_badvaddr;
	unsigned long cp0_cause;
	unsigned long cp0_epc;
    int tainted;
    unsigned long prid;
};    
typedef struct tag_DG_REG S_DG_REG;

/* 记录进程异常信息 */
struct tag_DG_CURRENT_PROCESS_INFO
{
    //把寄存器、堆栈等结构复制一份，在用户态进行显示，可以参考内核显示的方式
	char comm[16];
	int pid;
    unsigned long threadinfo;
    unsigned long task_addr;
    S_DG_REG sReg;
    char achModules[DG_MAX_MODULES_NUM][DG_MAX_MODULE_NAME];
    long alStack[DG_MAX_STACK_LEN];
    unsigned long aulTrace[DG_MAX_TRACE_LEN];
    unsigned long aulCode[DG_MAX_CODE_LEN];
};
typedef struct tag_DG_CURRENT_PROCESS_INFO S_DG_CURRENT_PROCESS_INFO;

struct tag_DG_PROCESS_CONTEXT
{
    //保存各进程数据，包括状态，寄存器，堆栈
	char comm[16];
	int pid;
    long state;
    unsigned long threadinfo;
    unsigned long task_addr;
    S_DG_REG sReg;
    long alStack[DG_MAX_STACK_LEN];
    unsigned long aulTrace[DG_MAX_TRACE_LEN];
};
typedef struct tag_DG_PROCESS_CONTEXT S_DG_PROCESS_CONTEXT;

/* 临终遗言总结构，数据在保留内存中存放格式。方便应用层命令行或web从内核中读取 */
struct tag_DYINGGASP_INFO
{
    unsigned long           ulChecksum;    //校验字，由RESERVED_SDRAM_CHECKSUM定义
    long                    lTime;         //记录时间
    S_DG_SCHED_INFO         sSched;        //记录进程调度信息
    S_DG_IN_PACK_INFO       sInputPackage; //记录系统收到的报文内容
    S_DG_WEB_OP_INFO sWebOp;            //记录web操作
    S_DG_CLI_OP_INFO sCliOp;            //记录命令行操作
    S_DG_ITF_STATE_INFO sItfStat;       //记录端口状态改变
    S_DG_CURRENT_PROCESS_INFO sCurProc; //记录当前进程信息，在进程异常时记录
    S_DG_PROCESS_CONTEXT sProcContext[DG_MAX_PROCESS_NUM];  //记录所有进程上下文
};
typedef struct tag_DYINGGASP_INFO    S_DYINGGASP_INFO;

/**********************************************/
/* Function announcement                      */
/*                                            */
extern int dg_checkDyingGasp(S_DYINGGASP_INFO *psDyingGaspInfo);
extern void dg_outputDyingGasp(S_DYINGGASP_INFO *psDyingGaspInfo);

#if defined(__cplusplus)
}
#endif

#endif /* _USER_DYINGGASP_H_ */

