#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <linux/kernel.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include "sysmon.h"

S_ERROR_NUMBER sErrTable[MAX_ERRNO + 1] =
{
    {NO_ERROR,          "No error."},
    {ERR_OPEN_FILE,     "Open file error."},
    {ERR_NULL_POINTER,  "NULL pointer."},
    {ERR_INVALID_VALUE, "Invalid value."},
    {ERR_OPEN_DIR,      "Open directory error."},
    {MAX_ERRNO,         NULL},
};
unsigned long Hertz;

/***********************************************************
Function:       void printErr(int)
Description:    打印错误信息
Calls:          NULL
Called By:      需要输出错误信息的函数
Input:          错误码
Output:         NULL
Return:         NULL
Others:         NULL
************************************************************/
void printErr(int iErrNo)
{
    int i;

    for (i = 0; i < MAX_ERRNO; i++)
    {
        if (iErrNo == sErrTable[i].iErrNo)
        {
            printf("Error: %s\n", sErrTable[i].pchErrInfo);
            return;
        }
    }
    printf("Error: Unknown error\n");
}

/***********************************************************************
 * Some values in /proc are expressed in units of 1/HZ seconds, where HZ
 * is the kernel clock tick rate. One of these units is called a jiffy.
 * The HZ value used in the kernel may vary according to hacker desire.
 * According to Linus Torvalds, this is not true. He considers the values
 * in /proc as being in architecture-dependent units that have no relation
 * to the kernel clock tick rate. Examination of the kernel source code
 * reveals that opinion as wishful thinking.
 *
 * In any case, we need the HZ constant as used in /proc. (the real HZ value
 * may differ, but we don't care) There are several ways we could get HZ:
 *
 * 1. Include the kernel header file. If it changes, recompile this library.
 * 2. Use the sysconf() function. When HZ changes, recompile the C library!
 * 3. Ask the kernel. This is obviously correct...
 *
 * Linus Torvalds won't let us ask the kernel, because he thinks we should
 * not know the HZ value. Oh well, we don't have to listen to him.
 * Someone smuggled out the HZ value. :-)
 *
 * This code should work fine, even if Linus fixes the kernel to match his
 * stated behavior. The code only fails in case of a partial conversion.
 *
 */
#define FILE_TO_BUF(filename, fd) do{                           \
    if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) {    \
	printf("/proc not be mounted?");                            \
	return;                                                    \
    }                                                           \
    lseek(fd, 0L, SEEK_SET);                                    \
    if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {        \
	printf("%s", filename);                                     \
	return;                                                   \
    }                                                           \
    buf[local_n] = '\0';                                        \
}while(0)

#define FILE_TO_BUF2(filename, fd) do{                          \
    lseek(fd, 0L, SEEK_SET);                                    \
    if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {        \
	printf("%s", filename);                                     \
	return;                                                   \
    }                                                           \
    buf[local_n] = '\0';                                        \
}while(0)

void init_Hertz_value(void) {
  unsigned long user_j, nice_j, sys_j, other_j;  /* jiffies (clock ticks) */
  double up_1, up_2, seconds;
  unsigned long jiffies, h;
  char buf[80];
  int uptime_fd = -1;
  int stat_fd = -1;

  long smp_num_cpus = sysconf(_SC_NPROCESSORS_CONF);

  if(smp_num_cpus<1) smp_num_cpus=1;

  do {
    int local_n;

    FILE_TO_BUF("/proc/uptime", uptime_fd);
    up_1 = strtod(buf, 0);
    FILE_TO_BUF("/proc/stat", stat_fd);
    sscanf(buf, "cpu %lu %lu %lu %lu", &user_j, &nice_j, &sys_j, &other_j);
    FILE_TO_BUF2("/proc/uptime", uptime_fd);
    up_2 = strtod(buf, 0);
  } while((long)( (up_2-up_1)*1000.0/up_1 )); /* want under 0.1% error */

  close(uptime_fd);
  close(stat_fd);

  jiffies = user_j + nice_j + sys_j + other_j;
  seconds = (up_1 + up_2) / 2;
  h = (unsigned long)( (double)jiffies/seconds/smp_num_cpus );
  /* actual values used by 2.4 kernels: 32 64 100 128 1000 1024 1200 */
  switch(h){
  case   30 ...   34 :  Hertz =   32; break; /* ia64 emulator */
  case   48 ...   52 :  Hertz =   50; break;
  case   58 ...   62 :  Hertz =   60; break;
  case   63 ...   65 :  Hertz =   64; break; /* StrongARM /Shark */
  case   95 ...  105 :  Hertz =  100; break; /* normal Linux */
  case  124 ...  132 :  Hertz =  128; break; /* MIPS, ARM */
  case  195 ...  204 :  Hertz =  200; break; /* normal << 1 */
  case  253 ...  260 :  Hertz =  256; break;
  case  295 ...  304 :  Hertz =  300; break; /* 3 cpus */
  case  393 ...  408 :  Hertz =  400; break; /* normal << 2 */
  case  495 ...  504 :  Hertz =  500; break; /* 5 cpus */
  case  595 ...  604 :  Hertz =  600; break; /* 6 cpus */
  case  695 ...  704 :  Hertz =  700; break; /* 7 cpus */
  case  790 ...  808 :  Hertz =  800; break; /* normal << 3 */
  case  895 ...  904 :  Hertz =  900; break; /* 9 cpus */
  case  990 ... 1010 :  Hertz = 1000; break; /* ARM */
  case 1015 ... 1035 :  Hertz = 1024; break; /* Alpha, ia64 */
  case 1095 ... 1104 :  Hertz = 1100; break; /* 11 cpus */
  case 1180 ... 1220 :  Hertz = 1200; break; /* Alpha */
  default:
    /* If 32-bit or big-endian (not Alpha or ia64), assume HZ is 100. */
    Hertz = (sizeof(long)==sizeof(int) || htons(999)==999) ? 100UL : 1024UL;
  }
}


S_SYSMON_PROCESS_STATUS * procps_scan(void)
{
	static DIR *dir;
	struct dirent *entry;
	static S_SYSMON_PROCESS_STATUS ret_status;
	char *name;
	int n;
	char status[32];
	char buf[1024];
	FILE *fp;
	S_SYSMON_PROCESS_STATUS curstatus;
	int pid;
	long tasknice;
    unsigned long ulUserTime,ulSysTime;

	if (!dir) {
		dir = opendir("/proc");
		if(!dir)
		{
			printf("Can't open /proc");
            return 0;
		}
	}
	for(;;) {
		if((entry = readdir(dir)) == NULL) {
			closedir(dir);
			dir = 0;
			return 0;
		}
		name = entry->d_name;
		if (!(*name >= '0' && *name <= '9'))
			continue;

		memset(&curstatus, 0, sizeof(S_SYSMON_PROCESS_STATUS));
		pid = atoi(name);
		curstatus.iPid = pid;

		sprintf(status, "/proc/%d/stat", pid);
		if((fp = fopen(status, "r")) == NULL)
			continue;
		name = fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(name == NULL)
			continue;
		name = strrchr(buf, ')'); /* split into "PID (cmd" and "<rest>" */
		if(name == 0 || name[1] != ' ')
			continue;
		*name = 0;
		sscanf(buf, "%*s (%15c", curstatus.achCmd);
		n = sscanf(name+2,
		"%c %*d "
		"%*s %*s %*s %*s "     /* pgrp, session, tty, tpgid */
		"%*s %*s %*s %*s %*s " /* flags, min_flt, cmin_flt, maj_flt, cmaj_flt */
		"%lu %lu "
		"%*s %*s %*s "         /* cutime, cstime, priority */
		"%ld "
		"%*s %*s %*s "         /* timeout, it_real_value, start_time */
		"%*s "                 /* vsize */
		"%ld",
		curstatus.achState,
		&ulUserTime, &ulSysTime,
		&tasknice,
		&curstatus.lMem);
		if(n != 5)
			continue;

        curstatus.iCpu = ulUserTime + ulSysTime;
        
		if (curstatus.lMem == 0 && curstatus.achState[0] != 'Z')
			curstatus.achState[1] = 'W';
		else
			curstatus.achState[1] = ' ';
		if (tasknice < 0)
			curstatus.achState[2] = '<';
		else if (tasknice > 0)
			curstatus.achState[2] = 'N';
		else
			curstatus.achState[2] = ' ';

#ifdef PAGE_SHIFT
		curstatus.lMem <<= (PAGE_SHIFT - 10);     /* 2**10 = 1kb */
#else
		curstatus.lMem *= (getpagesize() >> 10);     /* 2**10 = 1kb */
#endif

		return memcpy(&ret_status, &curstatus, sizeof(S_SYSMON_PROCESS_STATUS));
	}
}

/***********************************************************
Function:       int sysmon_getsysinfo(PS_SYSMON_PROCESS_STATUS *, PS_SYSMON_TOTAL_USAGE)
Description:    获取系统CPU和内存使用信息
Calls:          NULL
Called By:      sysmon_main
Input:          NULL
Output:         保存各进程信息的链表
                保存系统总CPU/内存使用信息的结构
Return:         0       成功
                其他    失败
Others:         NULL
************************************************************/
int sysmon_getsysinfo(PS_SYSMON_PROCESS_STATUS *ppsProc, PS_SYSMON_TOTAL_USAGE psTotal)
{
    float fProcCpu;
	float fUserUsage, fSysUsage;
    FILE *fp;
    DIR *dir;
    struct dirent *entry;
    int iPid;
    char achProcFile[32];
	unsigned long ulUserPrev,ulNicePrev,ulSysPrev,ulIdlePrev;
	unsigned long ulUserAft,ulNiceAft,ulSysAft,ulIdleAft;
    unsigned long ulUserTime,ulSysTime;
    unsigned long ulTotalPrev, ulTotalAft;
    char *pchProcName;
    char chStatus;
    char buf[128];
    char bufbk[256];
    char achCmd[16];
    PS_SYSMON_PROCESS_STATUS psSysInfo, psCurrPosition, psNext;
    struct timeval t;
    struct timeval oldtime;
    struct timezone timez;
    float elapsed_time;

    /* Calculate total cpu time */
    fp = fopen("/proc/stat","r");
    if (NULL == fp)
    {
        return ERR_OPEN_FILE;
    }
    fscanf(fp, "cpu %lu %lu %lu %lu", &ulUserPrev, &ulNicePrev, &ulSysPrev, &ulIdlePrev);
    fclose(fp);

//    ulTotalPrev = ulUserPrev + ulNicePrev + ulSysPrev + ulIdlePrev;

    gettimeofday(&t, &timez);
    oldtime.tv_sec  = t.tv_sec;
    oldtime.tv_usec = t.tv_usec;

    /* Here I use a chain table store the data, 
     * for I cannot know how many process there is now
     */

    /* alloc chain header
     * psSysInfo: chain table header
     * psCurrPosition: current store position
     */
    psSysInfo = malloc(sizeof(S_SYSMON_PROCESS_STATUS));
    if (NULL == psSysInfo) {
        closedir(dir);
        return ERR_NULL_POINTER;
    }
    memset(psSysInfo, 0, sizeof(S_SYSMON_PROCESS_STATUS));
    psCurrPosition = psSysInfo;

    PS_SYSMON_PROCESS_STATUS p;
	/* read every "/proc/[pid]/stat" file, 
     * and calculate the cpu time current process spent 
     */
	while ((p = procps_scan()) != 0) {
        /* alloc chain table entry
         * if failure, we should free memery alloced before */
        psCurrPosition->psNext = malloc(sizeof(S_SYSMON_PROCESS_STATUS));
        if (NULL == psCurrPosition->psNext) 
        {
            psCurrPosition = psSysInfo;
            while(NULL != psCurrPosition->psNext) 
            {
                psNext = psCurrPosition->psNext;
                free(psCurrPosition);
                psCurrPosition = psNext;
            }
            free(psCurrPosition);

            return ERR_NULL_POINTER;
        }
        psCurrPosition = psCurrPosition->psNext;
    	memcpy(psCurrPosition, p, sizeof(S_SYSMON_PROCESS_STATUS));
        psCurrPosition->psNext = NULL;

	}
    
    sleep(5);

    /* calculate cpu total time again */
    fp = fopen("/proc/stat","r");
    if (NULL == fp)
    {
        psCurrPosition = psSysInfo;
        while(NULL == psCurrPosition->psNext) 
        {
            psNext = psCurrPosition->psNext;
            free(psCurrPosition);
            psCurrPosition = psNext;
        }
        free(psCurrPosition);
        
        return ERR_OPEN_FILE;
    }
    fscanf(fp, "cpu %lu %lu %lu %lu", &ulUserAft, &ulNiceAft, &ulSysAft, &ulIdleAft);
    fclose(fp);

    ulTotalAft = ulUserAft + ulNiceAft + ulSysAft + ulIdleAft;

    gettimeofday(&t, &timez);
    elapsed_time = (t.tv_sec - oldtime.tv_sec)
	+ (float) (t.tv_usec - oldtime.tv_usec) / 1000000.0;

    /* calculate process cpu time again */
	/* read every "/proc/[pid]/stat" file, 
     * and calculate the cpu time current process spent 
     */
	while ((p = procps_scan()) != 0) {
        psCurrPosition = psSysInfo;
        while(NULL != psCurrPosition->psNext) 
        {
            psCurrPosition = psCurrPosition->psNext;
            if (psCurrPosition->iPid == p->iPid) {
                psCurrPosition->iCpu = p->iCpu - psCurrPosition->iCpu;
                psCurrPosition->iDispFlag = NEED_DISPLAY;

                fProcCpu =(float)(psCurrPosition->iCpu) * 10 * 100 / Hertz
            					/ elapsed_time;

                psCurrPosition->iCpu = (int)fProcCpu;
                if(psCurrPosition->iCpu > 999)
            	{
                    psCurrPosition->iCpu = 999;
            	}
                break;
            }
        }
	}

    *ppsProc = psSysInfo;

#if 0
    /* 从/proc/stat中得出系统总CPU占用率 */
	fUserUsage =(((float)(ulUserAft + ulNiceAft) 
				- (float)(ulUserPrev + ulNicePrev)) 
				/ (float)(ulTotalAft - ulTotalPrev)) * 1000;

	fSysUsage =(((float)ulSysAft - (float)ulSysPrev) 
				/ (float)(ulTotalAft - ulTotalPrev)) * 1000;

    psTotal->iUCpu = (int)fUserUsage;
    psTotal->iSCpu = (int)fSysUsage;
#endif    
    /* 从/proc/meminfo中得出系统总内存使用情况 */
	fp = fopen("/proc/meminfo","r");
	if (NULL == fp)
	{
        return ERR_OPEN_FILE;
	}	
	fscanf(fp, "MemTotal: %lu %*s\n", (long *)&psTotal->iTotal);
	fscanf(fp, "MemFree: %lu %*s\n", (long *)&psTotal->iFree);
	fscanf(fp, "Buffers: %lu %*s\n", (long *)&psTotal->iBuffers);
    psTotal->iUsed = psTotal->iTotal - psTotal->iFree;

    fclose(fp);

    return NO_ERROR;
}

/***********************************************************
Function:       void sysmon_main(void)
Description:    系统监控命令行入口，在命令行显示查询信息
Calls:          sysmon_getsysinfo
Called By:      processSystemMonitorCmd
Input:          NULL
Output:         NULL
Return:         NULL
Others:         NULL
************************************************************/
void sysmon_main(void)
{
    PS_SYSMON_PROCESS_STATUS p, psProcStat;
    S_SYSMON_TOTAL_USAGE sTotalStat;
    int iResult;

    init_Hertz_value();
    /* call sysmon_getsysinfo取得CPU和内存使用情况 */
    iResult = sysmon_getsysinfo(&psProcStat, &sTotalStat);
    if (0 != iResult) 
    {
        printErr(iResult);
        return;
    }

    /* 输出信息 */
    /*
    PID    CPU    MEM(kB) STATUS   COMMAND
    x      x.x    xxxx       S     xxxxxx
    y      y.y    yyyy       R     yyyyyy

    CPU:   user mode 5.6%   system mode 2.4%   idle 92%
    Mem:    16000k total,   13500k used,     2500k free,    1432k buffers
    */
    printf("    PID    CPU    MEM(kB)  STATUS  COMMAND\n");
    p = psProcStat;
    while (NULL != p->psNext) 
    {
        p = p->psNext;
        if (NEED_DISPLAY == p->iDispFlag)
        {
            printf("  %5d   %2d.%1d%%    %5d     %s     %s\n",
                   p->iPid, p->iCpu / 10, p->iCpu % 10, p->lMem * 4, p->achState, p->achCmd);
        }
    }

#if 0
    printf("\n  CPU:   user mode %d.%d%%   system mode %d.%d%%   idle %d.%d%%"
           "\n  Mem:    %dk total,   %dk used,   %dk free,   %dk buffers\n",
           sTotalStat.iUCpu / 10, sTotalStat.iUCpu % 10,
           sTotalStat.iSCpu / 10, sTotalStat.iSCpu % 10,
           (1000 - sTotalStat.iUCpu - sTotalStat.iSCpu) / 10,
           (1000 - sTotalStat.iUCpu - sTotalStat.iSCpu) % 10,
           sTotalStat.iTotal, sTotalStat.iUsed,
           sTotalStat.iFree, sTotalStat.iBuffers);
#else
    printf("\n  Mem:    %dk total,   %dk used,   %dk free,   %dk buffers\n",
           sTotalStat.iTotal, sTotalStat.iUsed,
           sTotalStat.iFree, sTotalStat.iBuffers);
#endif

    
    /* 释放sysmon_getprocessinfo中申请的内存空间 */
    while (NULL != psProcStat->psNext) 
    {
        p = psProcStat->psNext;
        free(psProcStat);
        psProcStat = p;
    }
    free(psProcStat);
}
