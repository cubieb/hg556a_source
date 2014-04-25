/***********************************************************************
  版权信息: 版权所有(C) 1988-2006, 华为技术有限公司.
  文件名: dyinggasp.c
  作者: l28714
  版本: V1.0
  创建日期: 2006-2-16
  完成日期: 
  功能描述: 
      临终遗言内核部分的函数实现
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    
  备注: 
************************************************************************/
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/time.h>
#include "../../../../../bcmdrivers/opensource/include/bcm963xx/board.h"
#include <linux/dyinggasp.h>

extern void show_trace(struct task_struct *task, unsigned long *stack);

S_DG_NORMAL_INFO sDyinggasp;
/* 
 * 调度正常标志
 *
 * 针对HGW来说，写flash时关闭所有中断，进程不会被调度，
 * 所以需要增加此标志来通知定时检测进程调度的程序
 * 其他产品可以根据自己的需要来设置此标志
 */
S_DG_SCHED_STATE sSchedulerState;

/*------------------------------------------------------------
  函数原型:  void dg_init(void)
  描述: 初始化保存临终遗言的结构
  输入: 无
  输出: stDyinggasp
        stSchedulerState
  返回值: 无
-------------------------------------------------------------*/
void dg_init(void)
{
    memset(&sDyinggasp, 0, sizeof(S_DG_NORMAL_INFO));
    sDyinggasp.ulChecksum = DG_RESERVED_SDRAM_CHECKSUM;

    /* 调度进程默认状态为正常 */
    sSchedulerState.ulState = 1;
    sSchedulerState.ulCount = 0;

    /* 申请表头 */
    sDyinggasp.sSched.psSchedTime = kmalloc(sizeof(S_DG_SCHED_TIME), GFP_ATOMIC);
    if (NULL == sDyinggasp.sSched.psSchedTime)
    {
        printk("Malloc memory for schedule record failed.\n");
    }
    memset(sDyinggasp.sSched.psSchedTime, 0, sizeof(S_DG_SCHED_TIME));
}

/***********************************************************
Function:       void dg_recordSchedInfo(task_t *task)
Description:    每次进程调度时，记录进程调度信息
Calls:          NULL
Called By:      schedule
Input:          将要被调度的进程控制块指针
Output:         stDyinggasp.stSched    记录进程调度信息
Return:         NULL
Others:         NULL

----------------------------------------------------------------------------
实现：
记录传入进程的进程ID到保留内存空间（循环存放，只保留最后N组数据）
记录进程调度的时间（准备计算长时间未调度的进程信息）
************************************************************/
void dg_recordSchedInfo(task_t *task)
{
    S_DG_SCHED_INFO_DYNAMIC *psSchedInfo;
    S_DG_SCHED_TIME *p, *psHead;
    int iPid = (int)task->pid;

    /* 如果数据没有初始化，先初始化 */
    if (DG_RESERVED_SDRAM_CHECKSUM != sDyinggasp.ulChecksum)
    {
        dg_init();
    }

    psSchedInfo = &(sDyinggasp.sSched);

    /* 只有非0进程才需要记录调度信息 */
    if (0 != iPid)
    {
        psSchedInfo->sSchededList[psSchedInfo->ulCurNum].iPid = iPid;
        psSchedInfo->ulCurNum = (psSchedInfo->ulCurNum + 1) % DG_MAX_SCHEDLISTRECORD;
        psSchedInfo->ulCurPID = iPid;

        psHead = psSchedInfo->psSchedTime;
        /* 表为空，插入表头 */
        if (NULL == psHead->psNext)
        {
            psHead->psNext = kmalloc(sizeof(S_DG_SCHED_TIME), GFP_KERNEL);
            if (NULL == psHead->psNext)
            {
                printk("Malloc memory for schedule record failed.\n");
            }
            else
            {
                p = psHead->psNext;
                p->iPid = iPid;
                p->ulTicks = jiffies;
                p->psNext = NULL;
            }        
        }
        else
        {
            p = psHead;
            while (NULL != p->psNext)
            {
                p = p->psNext;
                
                if (iPid == p->iPid)
                {
                    p->ulTicks = jiffies;
                    break;
                }
            }

            if (NULL == p->psNext && p->iPid != iPid)
            {
                p->psNext = kmalloc(sizeof(S_DG_SCHED_TIME), GFP_KERNEL);
                if (NULL == p->psNext)
                {
                    printk("Malloc memory for schedule record failed.\n");
                }
                else
                {
                    p->psNext->iPid = iPid;
                    p->psNext->ulTicks = jiffies;
                    p->psNext->psNext = NULL;
                }        
            }
        }
    }

    /* 记录系统总调度次数，作为判断系统是否正常运行的依据 */
    psSchedInfo->ulAllSchedNum++;

}

/*------------------------------------------------------------
  函数原型: void dg_recordInputPackage(struct sk_buff *skb)
  描述: 记录系统收到的最后MAX_IPPACKAGENUM个报文
  输入: skb     报文
  输出: sDyinggasp.sInputPackage     记录报文信息
  返回值: 无
-------------------------------------------------------------*/
void dg_recordInputPackage(struct sk_buff *skb)
{
    S_DG_IN_PACK_INFO *psInPackInfo = &(sDyinggasp.sInPack);

    memcpy(psInPackInfo->sIpInputBuf[psInPackInfo->ulCurPosition].aucDevName, 
           skb->dev->name, 
           16);
    memcpy(psInPackInfo->sIpInputBuf[psInPackInfo->ulCurPosition].aucMacInfo, 
           skb->mac.ethernet, 
           14);
    memcpy(psInPackInfo->sIpInputBuf[psInPackInfo->ulCurPosition].aucData, 
           skb->data, 
           (skb->len > 1536 ? 1536 : skb->len));
    psInPackInfo->sIpInputBuf[psInPackInfo->ulCurPosition].ulLen = skb->len;
    psInPackInfo->ulCurPosition = (psInPackInfo->ulCurPosition + 1) % DG_MAX_IPPACKAGENUM;
}

/***************************************************************************
Function:       void dg_calcNoUseProcess(ST_SCHEDTIME *, ST_SCHEDTIME **)
Description:    计算最久未调度的进程并记录
Calls:          NULL
Called By:      dg_storeDyingGaspInfo
Input:          记录的所有进程调度时间链表
Output:         最长未调度的10个进程数组
Return:         NULL
Others:         NULL

----------------------------------------------------------------------------
实现：
根据各进程上次被调度到现在的时间得出最长未调度的10个进程并记录
***************************************************************************/
void dg_calcNoUseProcess(S_DG_SCHED_TIME *psBefore, S_DG_SCHED_TIME *psAfter)
{
    S_DG_SCHED_TIME *psMin, *p;
    int i, j;

    i = 0;

    /* 只记录前10条数据 */
    for (i = 0; i < DG_MAX_NOUSEPROCESS; i++)
    {
        /* 查找ticks最短的数据 */

        /* 先取得表中未记录的一条数据作为比较的参考值 */
        p = psBefore;
        psMin = NULL;
        while (NULL != p->psNext)
        {
            p = p->psNext;
            for (j = 0; j < i; j++)
            {
                if (psAfter[j].iPid == p->iPid)
                {
                    break;
                }
            }

            /* 当前结点未记录 */
            if (j == i)
            {
                psMin = p;
                break;
            }
        }

        /* 所有数据全部记录了 */
        if (NULL == psMin)
        {
            break;
        }
            
        p = psBefore;
        while (NULL != p->psNext)
        {
            p = p->psNext;
            
            if (time_before(p->ulTicks, psMin->ulTicks))
            {
                /* 去除掉已经记录的数据 */
                for (j = 0; j < i; j++)
                {
                    if (psAfter[j].iPid == p->iPid)
                    {
                        break;
                    }
                }

                /* 在已记录数据中未找到该数据 */
                if (j == i)
                {
                    psMin = p;
                }
            }
        }

        /* 记录数据 */
        psAfter[i].iPid = psMin->iPid;
        psAfter[i].ulTicks = (jiffies - psMin->ulTicks) / HZ;
        psAfter[i].psNext = NULL;
    }
}

/***************************************************************************
Function:       void dg_recordWebOp(char *pchStr, int iStrLen)
Description:    保存用户web操作
Calls:          NULL
Called By:      board_ioctl
Input:          NULL
Output:         NULL
Return:         NULL
Others:         NULL

***************************************************************************/
void dg_recordWebOp(char *pchStr, int iStrLen)
{
    memset(sDyinggasp.sWebOp.achWebOp[sDyinggasp.sWebOp.ulCurPosition],
           0,
           DG_MAX_WEB_OP_LEN);

    /* 若输入字符串超长，则只保存前N个字符 */
    if (iStrLen >= DG_MAX_WEB_OP_LEN)
    {
        memcpy(sDyinggasp.sWebOp.achWebOp[sDyinggasp.sWebOp.ulCurPosition],
               pchStr,
               DG_MAX_WEB_OP_LEN - 1);
    }
    else
    {
        strcpy(sDyinggasp.sWebOp.achWebOp[sDyinggasp.sWebOp.ulCurPosition],
               pchStr);
    }
    
    sDyinggasp.sWebOp.ulCurPosition = (sDyinggasp.sWebOp.ulCurPosition + 1) % DG_MAX_WEB_OP_NUM;
}

/***************************************************************************
Function:       void dg_recordCliOp(char *pchStr, int iStrLen)
Description:    保存用户命令行操作
Calls:          NULL
Called By:      board_ioctl
Input:          NULL
Output:         NULL
Return:         NULL
Others:         NULL

***************************************************************************/
void dg_recordCliOp(char *pchStr, int iStrLen)
{
    memset(sDyinggasp.sCliOp.achCliOp[sDyinggasp.sCliOp.ulCurPosition],
           0,
           DG_MAX_CLI_OP_LEN);

    /* 若输入字符串超长，则只保存前N个字符 */
    if (iStrLen >= DG_MAX_CLI_OP_LEN)
    {
        memcpy(sDyinggasp.sCliOp.achCliOp[sDyinggasp.sCliOp.ulCurPosition],
               pchStr,
               DG_MAX_CLI_OP_LEN - 1);
    }
    else
    {
        strcpy(sDyinggasp.sCliOp.achCliOp[sDyinggasp.sCliOp.ulCurPosition],
               pchStr);
    }
    
    sDyinggasp.sCliOp.ulCurPosition = (sDyinggasp.sCliOp.ulCurPosition + 1) % DG_MAX_CLI_OP_NUM;
}

/***************************************************************************
Function:       void dg_recordIntfState(char *dev_name, int state)
Description:    记录各接口UP/DOWN信息
Calls:          NULL
Called By:      bcm63xx_enet_poll_timer
                xDSL接口状态检测函数
Input:          接口名、接口状态
Output:         NULL
Return:         NULL
Others:         NULL

----------------------------------------------------------------------------
实现：
记录对应接口的状态和当前系统时间
***************************************************************************/
void dg_recordIntfState(char *dev_name, int state)
{
    int i;
    /* 如果端口状态和上次状态一致，不再记录 */
    for (i = (sDyinggasp.sItfStat.ulCurPosition + DG_MAX_ITF_STATE_NUM - 1) % DG_MAX_ITF_STATE_NUM;
         i != sDyinggasp.sItfStat.ulCurPosition;
         i = (i + DG_MAX_ITF_STATE_NUM - 1) % DG_MAX_ITF_STATE_NUM)
    {
        if (0 == strcmp(dev_name, sDyinggasp.sItfStat.asItfStat[i].achItfName))
        {
            if (sDyinggasp.sItfStat.asItfStat[i].iItfState == state)
            {
                return;
            }
            else
            {
                break;
            }
        }
    }
    memset(&(sDyinggasp.sItfStat.asItfStat[sDyinggasp.sItfStat.ulCurPosition]),
           0,
           sizeof(S_DG_ITF_STATE));

    strncpy(sDyinggasp.sItfStat.asItfStat[sDyinggasp.sItfStat.ulCurPosition].achItfName,
            dev_name,
            DG_MAX_ITF_NAME_LEN - 1);

    sDyinggasp.sItfStat.asItfStat[sDyinggasp.sItfStat.ulCurPosition].iItfState = state;

    sDyinggasp.sItfStat.asItfStat[sDyinggasp.sItfStat.ulCurPosition].iTime = xtime.tv_sec;

    sDyinggasp.sItfStat.ulCurPosition = (sDyinggasp.sItfStat.ulCurPosition + 1) % DG_MAX_ITF_STATE_NUM;
}
EXPORT_SYMBOL(dg_recordIntfState);

/*------------------------------------------------------------
  函数原型: void dg_recordRegs(struct pt_regs *regs)
  描述: 记录当前进程寄存器信息
  输入: regs    寄存器数据
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordRegs(struct pt_regs *regs)
{
    int i;
    S_DG_REG *psReg = &(sDyinggasp.sCurProc.sReg);

    /*
     * Saved processor id
     */
    psReg->cpu = smp_processor_id();

    /*
     * Saved main processor registers
     */
    for (i = 0; i < 32; ) {
        psReg->regs[i] = regs->regs[i];
    }

    psReg->hi = regs->hi;
    psReg->lo = regs->lo;

    /*
     * Saved cp0 registers
     */
    psReg->cp0_epc = regs->cp0_epc;
    psReg->tainted = tainted;
    psReg->cp0_status = regs->cp0_status;
    psReg->cp0_cause = regs->cp0_cause;
    psReg->prid = read_c0_prid();
}

/*------------------------------------------------------------
  函数原型: void dg_recordModules(void)
  描述: 记录当前加载的模块信息。由于内核模块链表指针是静态变量，只好把记录函数放在module.c中调用
  输入: NULL
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordModules(struct list_head *modules)
{
	struct module *mod;
    int i = 0;
 
	list_for_each_entry(mod, modules, list)
	{
		strcpy(sDyinggasp.sCurProc.achModules[i], mod->name);
        i++;
        if (i >= DG_MAX_MODULES_NUM)
        {
            break;
        }
	}
}

/*------------------------------------------------------------
  函数原型: void dg_recordStack(struct task_struct *task, unsigned long *sp)
  描述: 记录当前进程堆栈
  输入: task    当前进程控制块
        sp      sp寄存器
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordStack(struct task_struct *task, unsigned long *sp, long *plStack)
{
	long stackdata;
	int i;

	sp = sp ? sp : (unsigned long *) &sp;

	i = 0;
	while ((unsigned long) sp & (PAGE_SIZE - 1)) {
		if (__get_user(stackdata, sp++)) {
			break;
		}

        plStack[i] = stackdata;
		i++;
	}
}

/*------------------------------------------------------------
  函数原型: void dg_recordTrace(struct task_struct *task, unsigned long *stack)
  描述: 记录当前进程运行轨迹
  输入: task    进程控制块
        stack   sp寄存器
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordTrace(struct task_struct *task, unsigned long *stack, unsigned long *pulTrace)
{
	unsigned long addr;
    int i;

	if (!stack)
		stack = (unsigned long*)&stack;

    i = 0;
	while (!kstack_end(stack)) {
		addr = *stack++;
		if (__kernel_text_address(addr)) {
            pulTrace[i] = addr;
		}
        i++;
	}
}

/*------------------------------------------------------------
  函数原型: void dg_recordCode(unsigned int *pc)
  描述: 记录当前执行语句前后的代码
  输入: pc  当前运行代码指针
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordCode(unsigned int *pc, unsigned long *pulCode)
{
	long i;
	unsigned int insn;

	for(i = -3 ; i < 6 ; i++) {
		if (__get_user(insn, pc + i)) {
			break;
		}
		pulCode[i + 3] = insn;
	}
}

/*------------------------------------------------------------
  函数原型: void show_registers(struct pt_regs *regs)
  描述: 记录当前进程数据
  输入: 当前进程寄存器
  输出: NULL
  返回值: NULL
-------------------------------------------------------------*/
void dg_recordRegisters(struct pt_regs *regs)
{
	dg_recordRegs(regs);
    strcpy(sDyinggasp.sCurProc.comm ,current->comm);
    sDyinggasp.sCurProc.pid = current->pid;
    sDyinggasp.sCurProc.threadinfo = (unsigned long)current_thread_info();
    sDyinggasp.sCurProc.task_addr = (unsigned long)current;
	dg_recordStack(current, (long *) regs->regs[29], sDyinggasp.sCurProc.alStack);
	dg_recordTrace(current, (long *) regs->regs[29], sDyinggasp.sCurProc.aulTrace);
	dg_recordCode((unsigned int *) regs->cp0_epc, sDyinggasp.sCurProc.aulCode);
}

/***************************************************************************
Function:       void dg_recordProcessInfo(void)
Description:    保存当前所有进程上下文
Calls:          NULL
Called By:      dg_storeDyingGaspInfo
Input:          NULL
Output:         NULL
Return:         NULL
Others:         NULL

----------------------------------------------------------------------------
实现：
保存当前所有进程的进程控制块，进程堆栈信息，当前进程状态
***************************************************************************/
void dg_recordProcessInfo(S_DYINGGASP_INFO *psInfo)
{
    struct task_struct *task;
    int i;
    
    i = 0;
    for_each_process(task) {
        strcpy(psInfo->sProcContext[i].comm ,task->comm);
        psInfo->sProcContext[i].pid = task->pid;
        psInfo->sProcContext[i].state = task->state;
        psInfo->sProcContext[i].sReg.regs[16] = task->thread.reg16;
        psInfo->sProcContext[i].sReg.regs[17] = task->thread.reg17;
        psInfo->sProcContext[i].sReg.regs[18] = task->thread.reg18;
        psInfo->sProcContext[i].sReg.regs[19] = task->thread.reg19;
        psInfo->sProcContext[i].sReg.regs[20] = task->thread.reg20;
        psInfo->sProcContext[i].sReg.regs[21] = task->thread.reg21;
        psInfo->sProcContext[i].sReg.regs[22] = task->thread.reg22;
        psInfo->sProcContext[i].sReg.regs[23] = task->thread.reg23;
        psInfo->sProcContext[i].sReg.regs[29] = task->thread.reg29;
        psInfo->sProcContext[i].sReg.regs[30] = task->thread.reg30;
        psInfo->sProcContext[i].sReg.regs[31] = task->thread.reg31;
        
        psInfo->sProcContext[i].threadinfo = (unsigned long)(task->thread_info);
        psInfo->sProcContext[i].task_addr = (unsigned long)task;

    	dg_recordStack(task, (long *) task->thread.reg29, psInfo->sProcContext[i].alStack);
    	dg_recordTrace(task, (long *) task->thread.reg29, psInfo->sProcContext[i].aulTrace);

        i++;
        if (i >= DG_MAX_PROCESS_NUM)
        {
            break;
        }
    }
    /* end of maintain dying gasp debug by liuzhijie 00028714 2006年5月11日 */
}

/*------------------------------------------------------------
  函数原型: void dg_storeDyingGaspInfo(void)
  描述: 系统异常时，保存临终遗言
  输入: 无
  输出: sDyinggasp
  返回值: 无
-------------------------------------------------------------*/
void dg_storeDyingGaspInfo(void)
{
    S_DYINGGASP_INFO *psSdram;
    int i;

    psSdram = (S_DYINGGASP_INFO *)DG_RESERVED_SDRAM_START_ADDR;
    memset(psSdram, 0, sizeof(S_DYINGGASP_INFO));

    psSdram->ulChecksum = DG_RESERVED_SDRAM_CHECKSUM;
    psSdram->lTime = xtime.tv_sec;

    psSdram->sSched.ulCurNum = sDyinggasp.sSched.ulCurNum;
    psSdram->sSched.ulCurPID = sDyinggasp.sSched.ulCurPID;
    psSdram->sSched.ulAllSchedNum = sDyinggasp.sSched.ulAllSchedNum;
    memcpy(&(psSdram->sSched.sSchededList), 
           &(sDyinggasp.sSched.sSchededList), 
           sizeof(S_DG_SCHED_LIST) * DG_MAX_SCHEDLISTRECORD);

    dg_calcNoUseProcess(sDyinggasp.sSched.psSchedTime,
                        psSdram->sSched.asSchedTime);

    /* 记录当前所有进程上下文 */
    dg_recordProcessInfo(psSdram);

    /* 记录异常进程寄存器 */
    memcpy(&(psSdram->sCurProc), 
           &(sDyinggasp.sCurProc), 
           sizeof(S_DG_CURRENT_PROCESS_INFO));

    /* 保存系统收到的报文 */
    memcpy(&(psSdram->sInputPackage), 
           &(sDyinggasp.sInPack), 
           sizeof(S_DG_IN_PACK_INFO));

    /* 保存web操作日志 */
    memcpy(&(psSdram->sWebOp), &(sDyinggasp.sWebOp),
           sizeof(S_DG_WEB_OP_INFO));

    /* 保存命令行操作日志 */
    memcpy(&(psSdram->sCliOp), &(sDyinggasp.sCliOp),
           sizeof(S_DG_CLI_OP_INFO));

    /* 保存端口状态改变记录 */
    memcpy(&(psSdram->sItfStat), &(sDyinggasp.sItfStat),
           sizeof(S_DG_ITF_STATE_INFO));

}

/*------------------------------------------------------------
  函数原型: void dg_setScheduleState(int iState)
  描述: 设置调度状态
  输入: iState      调度状态
  输出: sSchedulerState
  返回值: 无
-------------------------------------------------------------*/
void dg_setScheduleState(int iState)
{
    if (0 == iState)
    {
        if (1 == sSchedulerState.ulState)
        {
            sSchedulerState.ulState = 0;
        }
        sSchedulerState.ulCount++;
    }
    else if (1 == iState)
    {
        if (0 == sSchedulerState.ulState)
        {
            sSchedulerState.ulCount--;

            if (0 == sSchedulerState.ulCount)
            {
                sSchedulerState.ulState = 1;
            }
        }
    }
    else
    {
        return;
    }
}

/*------------------------------------------------------------
  函数原型: void dg_checkSchedulerAlive(unsigned long psecs)
  描述: 定时检测调度进程是否仍在调度，若没有进行调度，表明系统异常，保存现场信息，复位
  输入: psecs       当前tick数
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void dg_checkSchedulerAlive(unsigned long psecs)
{
    static unsigned long lastMonTime = 0;  //上次检测时间
    static unsigned long lastMonCount = 0; //上次检测时进程的总调度数
    S_DG_SCHED_TIME *p, *q;
    struct task_struct *task;

    if (0 == sSchedulerState.ulState)
    {
        lastMonTime = psecs;
        return;
    }

    if (psecs - lastMonTime >= CIRCLE_MONITOR_INTERVAL) {
        if (lastMonCount != sDyinggasp.sSched.ulAllSchedNum) {
            lastMonCount = sDyinggasp.sSched.ulAllSchedNum;
        }
        else {
            printk("Schedule process dead. Current %s Time %d \n", current->comm, psecs - lastMonTime);
            dg_storeDyingGaspInfo();
            printk("\nReset Board!\n");
            kerSysMipsSoftReset();            
        }
        lastMonTime = psecs;

        /* 定时把记录进程最后一次调用时间的结构中已经退出的进程数据删除掉 */
        p = sDyinggasp.sSched.psSchedTime;
        while (NULL != p->psNext)// || NULL != p)//如果最后一个数据被删除，则p有可能为空
        {
            for_each_process(task) 
            {
                if (task->pid == p->psNext->iPid)
                {
                    break;
                }
            }
            if (task == &init_task && 1 != p->psNext->iPid)
            {
                q = p->psNext;
                p->psNext = p->psNext->psNext;
                kfree(q);
            }
            else
            {
                p = p->psNext;
            }
        }
    }
}    
