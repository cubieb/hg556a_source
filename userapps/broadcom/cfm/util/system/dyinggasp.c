/***********************************************************************
  版权信息: 版权所有(C) 1988-2006, 华为技术有限公司.
  文件名: dyinggasp.cpp
  作者: l28714
  版本: V1.0
  创建日期: 2006-2-16
  完成日期: 
  功能描述: 
      临终遗言用户层的函数实现      
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    
  备注: 
************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dyinggasp.h"

/*------------------------------------------------------------
  函数原型: void dg_outputSchedInfo(ST_SCHEDINFO *pstSched)
  描述: 输出进程调度信息
  输入: pstSched    保存进程调度信息的结构
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void dg_outputSchedInfo(S_DG_SCHED_INFO *psSched)
{
    int i, iOutputNum;

    if ((psSched->ulCurNum < DG_MAX_SCHEDLISTRECORD) && (psSched->ulCurNum >= 0))
    {
        printf("\n==============================[Schedule] INFO BEGIN================================");
        printf("\nSystem Processed Scheduled Whole Times before Reboot : [%lu]", psSched->ulAllSchedNum);
        printf("\nSystem Last Current Process ID before Reboot : [%lu]", psSched->ulCurPID);
        printf("\n-----------------------------------------------------------------------------------");
        printf("\nSystem Last %d Invoked Processes List before Reboot:", DG_MAX_SCHEDLISTRECORD);

        for (iOutputNum = 0; iOutputNum < DG_MAX_SCHEDLISTRECORD; iOutputNum++)
        {
            if (0 == iOutputNum % 8)
            {
                printf("\n\t");
            }

            i = (psSched->ulCurNum - 1 - iOutputNum + DG_MAX_SCHEDLISTRECORD) % DG_MAX_SCHEDLISTRECORD;

            printf("<%4d>  ", psSched->sSchededList[i].iPid);
        }

        printf("\nTop ten of not be scheduled:");
        for (i = 0; i < DG_MAX_NOUSEPROCESS; i++)
        {
            printf("\n[process %d]\t%lu s", 
                   psSched->asSchedTime[i].iPid,
                   psSched->asSchedTime[i].ulTicks);
        }
        printf("\n==============================[Schedule] INFO END==================================\n");
    }
    else
    {
        printf("\nDyingGasp info list number == %lu\n", psSched->ulCurNum);
        printf("\nDyingGasp info list number Error!\n");
    }
}

/*------------------------------------------------------------
  函数原型: void dg_outputPackageInfo(ST_INPUTPACKAGEINFO *psInputPackage)
  描述: 输出系统收到的最后几个报文的内容
  输入: psInputPackage     保存报文内容的结构
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void dg_outputPackageInfo(S_DG_IN_PACK_INFO *psInputPackage)
{
    int i, j, count;

    if ((psInputPackage->ulCurPosition < DG_MAX_IPPACKAGENUM) && (psInputPackage->ulCurPosition >= 0))
    {
        printf("\n==============================[IP Packet] INFO BEGIN===============================");
        printf("\n<DevName>        <DstMac>             <SrcMac>             [Type]");
        printf("\n-----------------------------------------------------------------------------------");

        for (count = 0; count < DG_MAX_IPPACKAGENUM; count++)
        {
            i = (psInputPackage->ulCurPosition - 1 - count + DG_MAX_IPPACKAGENUM) % DG_MAX_IPPACKAGENUM;
            printf("\n%-15s  <%02X:%02X:%02X:%02X:%02X:%02X>  <%02X:%02X:%02X:%02X:%02X:%02X>  [0x%x]",
                psInputPackage->sIpInputBuf[i].aucDevName,
                psInputPackage->sIpInputBuf[i].aucMacInfo[0],
                psInputPackage->sIpInputBuf[i].aucMacInfo[1],
                psInputPackage->sIpInputBuf[i].aucMacInfo[2],
                psInputPackage->sIpInputBuf[i].aucMacInfo[3],
                psInputPackage->sIpInputBuf[i].aucMacInfo[4],
                psInputPackage->sIpInputBuf[i].aucMacInfo[5],
                psInputPackage->sIpInputBuf[i].aucMacInfo[6],
                psInputPackage->sIpInputBuf[i].aucMacInfo[7],
                psInputPackage->sIpInputBuf[i].aucMacInfo[8],
                psInputPackage->sIpInputBuf[i].aucMacInfo[9],
                psInputPackage->sIpInputBuf[i].aucMacInfo[10],
                psInputPackage->sIpInputBuf[i].aucMacInfo[11],
                *(unsigned short *)&(psInputPackage->sIpInputBuf[i].aucMacInfo[12]));
            printf("\nPacket Len == %lu", psInputPackage->sIpInputBuf[i].ulLen);
            printf("\nPacket [%d]", i);
            for (j = 0; 
                 j < (psInputPackage->sIpInputBuf[i].ulLen > 1536 ? 1536 : psInputPackage->sIpInputBuf[i].ulLen); 
                 j++)
            {
                if (0 == j % 16)
                {
                    printf("\n  0x%04x: ", j);
                }
                printf("%02x ", psInputPackage->sIpInputBuf[i].aucData[j]);
            }
            printf("\n");
        }

        printf("\n==============================[IP Packet] INFO END=================================\n");
    }
    else
    {
        printf("\nInput package list number == %lu\n", psInputPackage->ulCurPosition);
        printf("\nInput package list number Error!\n");
    }

}

void dg_outputCLIOp(S_DG_CLI_OP_INFO *psCliOp)
{
    int i, count;

    if ((psCliOp->ulCurPosition < DG_MAX_CLI_OP_NUM) && (psCliOp->ulCurPosition >= 0))
    {
        printf("User command line operation:\n");

        for (count = 0; count < DG_MAX_CLI_OP_NUM; count++)
        {
            i = (psCliOp->ulCurPosition - 1 - count + DG_MAX_CLI_OP_NUM) % DG_MAX_CLI_OP_NUM;
            if (0 != strlen(psCliOp->achCliOp[i]))
            {
                printf("%s\n", psCliOp->achCliOp[i]);
            }
        }
        printf("\n");
    }
    else
    {
        printf("\nUser command line operation list number == %lu\n", psCliOp->ulCurPosition);
        printf("\nUser command line operation list number Error!\n");
    }

}

void dg_outputWebOp(S_DG_WEB_OP_INFO *psWebOp)
{
    int i, count;

    if ((psWebOp->ulCurPosition < DG_MAX_WEB_OP_NUM) && (psWebOp->ulCurPosition >= 0))
    {
        printf("User web operation:\n");

        for (count = 0; count < DG_MAX_WEB_OP_NUM; count++)
        {
            i = (psWebOp->ulCurPosition - 1 - count + DG_MAX_WEB_OP_NUM) % DG_MAX_WEB_OP_NUM;
            if (0 != strlen(psWebOp->achWebOp[i]))
            {
                printf("%s\n", psWebOp->achWebOp[i]);
            }
        }
        printf("\n");
    }
    else
    {
        printf("\nUser web operation list number == %lu\n", psWebOp->ulCurPosition);
        printf("\nUser web operation list number Error!\n");
    }

}

void dg_outputItfState(S_DG_ITF_STATE_INFO *psItfState)
{
    int i, count;
    char achState[80];

    if ((psItfState->ulCurPosition < DG_MAX_WEB_OP_NUM) && (psItfState->ulCurPosition >= 0))
    {
        printf("Interface state:\n");

        for (count = 0; count < DG_MAX_WEB_OP_NUM; count++)
        {
            i = (psItfState->ulCurPosition - 1 - count + DG_MAX_WEB_OP_NUM) % DG_MAX_WEB_OP_NUM;
            if (0 != strlen(psItfState->asItfStat[i].achItfName))
            {
                switch (psItfState->asItfStat[i].iItfState)
                {
                    case DG_ITF_UP:
                        strcpy(achState, "up");
                        break;
                        
                    case DG_ITF_DOWN:
                        strcpy(achState, "down");
                        break;

                    case DG_XDSL_TRAINING_G992_EXCHANGE:
                        strcpy(achState, "xDSL G.992 exchange");
                        break;
                        
                    case DG_XDSL_TRAINING_G992_CHANNEL_ANALYSIS:
                        strcpy(achState, "xDSL G.992 channel analysis");
                        break;
                        
                    case DG_XDSL_TRAINING_G992_STARTED:
                        strcpy(achState, "xDSL G.992 start");
                        break;
                        
                    case DG_XDSL_TRAINING_G994:
                        strcpy(achState, "xDSL G.994 training");
                        break;
                        
                    case DG_XDSL_G994_NONSTDINFO_RECEIVED:
                        strcpy(achState, "xDSL G.994 nonstdinfo received");
                        break;
                        
                    case DG_XDSL_BERT_COMPLETE:
                        strcpy(achState, "xDSL bert complete");
                        break;
                        
                    case DG_XDSL_ATM_IDLE:
                        strcpy(achState, "xDSL atm idle");
                        break;
                        
                    case DG_XDSL_EVENT:
                        strcpy(achState, "xDSL event");
                        break;
                        
                    case DG_XDSL_G997_FRAME_RECEIVED:
                        strcpy(achState, "xDSL G.997 frame received");
                        break;
                        
                    case DG_XDSL_G997_FRAME_SENT:
                        strcpy(achState, "xDSL G.997 frame sent");
                        break;

                    default:
                        strcpy(achState, "Unknown state");
                }

                printf("%s %s %s\n", ctime((time_t *)&(psItfState->asItfStat[i].iTime)),
                       psItfState->asItfStat[i].achItfName,
                       achState);
            }
        }
        printf("\n");
    }
    else
    {
        printf("\nInterface state list number == %lu\n", psItfState->ulCurPosition);
        printf("\nInterface state list number Error!\n");
    }

}

static const char *print_tainted(int tainted)
{
    static char buf[20];
    if (tainted) {
        snprintf(buf, sizeof(buf), "Tainted: %c%c%c",
            tainted & TAINT_PROPRIETARY_MODULE ? 'P' : 'G',
            tainted & TAINT_FORCED_MODULE ? 'F' : ' ',
            tainted & TAINT_UNSAFE_SMP ? 'S' : ' ');
    }
    else
        snprintf(buf, sizeof(buf), "Not tainted");
    return(buf);
}

void dg_currProcShowRegs(S_DG_REG *regs)
{
    unsigned int cause = regs->cp0_cause;
    int i;

    printf("Cpu %lu\n", regs->cpu);

    /*
     * Saved main processor registers
     */
    for (i = 0; i < 32; ) {
        if ((i % 4) == 0)
            printf("$%2d   :", i);
        if (i == 0)
            printf(" %08lx", 0UL);
        else if (i == 26 || i == 27)
            printf(" %s", "");
        else
            printf(" %08lx", regs->regs[i]);

        i++;
        if ((i % 4) == 0)
            printf("\n");
    }

    printf("Hi    : %08lx\n", regs->hi);
    printf("Lo    : %08lx\n", regs->lo);

    /*
     * Saved cp0 registers
     */
    printf("epc   : %08lx ", regs->cp0_epc);
    printf("    %s\n", print_tainted(regs->tainted));
    printf("ra    : %08lx ", regs->regs[31]);

    printf("Status: %08lx    ", regs->cp0_status);

    if (regs->cp0_status & ST0_KX)
        printf("KX ");
    if (regs->cp0_status & ST0_SX)
        printf("SX ");
    if (regs->cp0_status & ST0_UX)
        printf("UX ");
    switch (regs->cp0_status & ST0_KSU) {
    case KSU_USER:
        printf("USER ");
        break;
    case KSU_SUPERVISOR:
        printf("SUPERVISOR ");
        break;
    case KSU_KERNEL:
        printf("KERNEL ");
        break;
    default:
        printf("BAD_MODE ");
        break;
    }
    if (regs->cp0_status & ST0_ERL)
        printf("ERL ");
    if (regs->cp0_status & ST0_EXL)
        printf("EXL ");
    if (regs->cp0_status & ST0_IE)
        printf("IE ");
    printf("\n");

    printf("Cause : %08x\n", cause);

    cause = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;
    if (1 <= cause && cause <= 5)
        printf("BadVA : %08lx\n", regs->cp0_badvaddr);

    printf("PrId  : %08lx\n", regs->prid);
}

void dg_outputCurProc(S_DG_CURRENT_PROCESS_INFO *psCurProc)
{
    int i;

    dg_currProcShowRegs(&(psCurProc->sReg));
    printf("Modules linked in:");
    for (i = 0; i < DG_MAX_MODULES_NUM; i++)
    {
        printf(" %s", psCurProc->achModules[i]);
    }
    printf("\n");
    printf("Process %s (pid: %d, threadinfo=%0lx, task=%0lx)\n",
            psCurProc->comm, psCurProc->pid, psCurProc->threadinfo, psCurProc->task_addr);
    
    printf("Stack :");
    printf("\n       ");
    for (i = 0; i < DG_MAX_STACK_LEN; i++)
    {
        printf(" %08lx", psCurProc->alStack[i]);

        if (i % 8 == 7)
            printf("\n       ");

    }
    printf(" ...");
    printf("\n");

    printf("Call Trace:");
    for (i = 0; i < DG_MAX_TRACE_LEN; i++)
    {
        printf(" [<%08lx>] ", psCurProc->aulTrace[i]);
    }
    printf("\n");

    printf("\nCode:");

    for(i = -3 ; i < 6 ; i++) 
    {
        printf("%c%08lx%c", (i?' ':'<'), psCurProc->aulCode[i + 3], (i?' ':'>'));
    }
}

void dg_ProcContextShowRegs(S_DG_REG *regs)
{
    unsigned int cause = regs->cp0_cause;
    int i;
    /*
     * Saved main processor registers
     */
    /* 只能记录reg16-reg23, reg29-reg31 */
    for (i = 16; i < 32; ) {
        if ((i % 4) == 0)
            printf("$%2d   :", i);
        if (i == 28 )
            printf(" %08lx", 0UL);
        else if (i == 24 || i == 25 || i == 26 || i == 27)
            printf(" %s", "");
        else
            printf(" %08lx", regs->regs[i]);

        i++;
        if ((i % 4) == 0)
            printf("\n");
    }
}
void dg_outputProcContext(S_DG_PROCESS_CONTEXT *psProcContext)
{
    int i;

/* -1 unrunnable, 0 runnable, >0 stopped */
    printf("\nProcess %s (pid: %d, threadinfo=%0lx, task=%0lx ",
           psProcContext->comm, psProcContext->pid, psProcContext->threadinfo, psProcContext->task_addr);
    if (-1 == psProcContext->state)
    {
        printf("unrunnable)\n");
    }
    else if (0 == psProcContext->state)
    {
        printf("runnable)\n");
    }
    else
    {
        printf("stopped)\n");
    }

    dg_ProcContextShowRegs(&(psProcContext->sReg));
    
    printf("Stack :");
    printf("\n       ");
    for (i = 0; i < DG_MAX_STACK_LEN; i++)
    {
        printf(" %08lx", psProcContext->alStack[i]);

        if (i % 8 == 7)
            printf("\n       ");

    }
    printf(" ...");
    printf("\n");

    printf("Call Trace:");
    for (i = 0; i < DG_MAX_TRACE_LEN; i++)
    {
        printf(" [<%08lx>] ", psProcContext->aulTrace[i]);
    }
    printf("\n");
}

/*------------------------------------------------------------
  函数原型: int dg_checkDyingGasp(ST_DYINGGASP_INFO *psDyingGaspInfo)
  描述: 校验临终遗言数据有效性
  输入: psDyingGaspInfo     保存临终遗言数据的结构
  输出: 无
  返回值: 0     数据有效
          -1    数据无效
-------------------------------------------------------------*/
int dg_checkDyingGasp(S_DYINGGASP_INFO *psDyingGaspInfo)
{
    if (DG_RESERVED_SDRAM_CHECKSUM == psDyingGaspInfo->ulChecksum)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/*------------------------------------------------------------
  函数原型: void dg_outputDyingGasp(S_DYINGGASP_INFO *psDyingGaspInfo)
  描述: 输出临终遗言信息
  输入: psDyingGaspInfo     保存临终遗言数据的结构
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void dg_outputDyingGasp(S_DYINGGASP_INFO *psDyingGaspInfo)
{
    S_DG_SCHED_INFO     *psSched;
    S_DG_IN_PACK_INFO *psInputPackage;
    S_DG_CLI_OP_INFO *psCliOp;
    S_DG_WEB_OP_INFO *psWebOp;
    S_DG_ITF_STATE_INFO *psItfState;
    S_DG_CURRENT_PROCESS_INFO *psCurProc;
    S_DG_PROCESS_CONTEXT *psProcContext;
    int i;

    printf("\nRecord time: %s", ctime((time_t *)&(psDyingGaspInfo->lTime)));

    /* 输出进程调度信息 */
    psSched = &(psDyingGaspInfo->sSched);
    dg_outputSchedInfo(psSched);

    /* 输出系统收到的报文信息 */
    psInputPackage = &(psDyingGaspInfo->sInputPackage);
    dg_outputPackageInfo(psInputPackage);

    /* 输出用户命令行操作日志 */
    psCliOp = &(psDyingGaspInfo->sCliOp);
    dg_outputCLIOp(psCliOp);

    /* 输出用户WEB操作日志 */
    psWebOp = &(psDyingGaspInfo->sWebOp);
    dg_outputWebOp(psWebOp);
    
    /* 输出端口状态改变记录 */
    psItfState = &(psDyingGaspInfo->sItfStat);
    dg_outputItfState(psItfState);

    /* 输出进程异常信息 */
    if (0 != psDyingGaspInfo->sCurProc.pid)
    {
        psCurProc = &(psDyingGaspInfo->sCurProc);
        dg_outputCurProc(psCurProc);
    }

    /* 输出各进程上下文 */
    for (i = 0; i < DG_MAX_PROCESS_NUM; i++)
    {
        if (0 != psDyingGaspInfo->sProcContext[i].pid)
        {
            dg_outputProcContext(&(psDyingGaspInfo->sProcContext[i]));
        }
    }
}

