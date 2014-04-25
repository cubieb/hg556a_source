/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : utils_lib.h
  作者     : lichangqing
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : 头文件
      
  主要函数列表: 无
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#ifndef __UTILS_H
#define __UTILS_H

//#include <linux/sem.h>
#include <sched.h>

#include <sys/time.h> 
#include <signal.h> 

// 毫秒为单位的等待声明
int __msleep(int millisecond);


// wait声明
typedef struct
{
    char* key;
} WAIT_S;

int __init_wait(WAIT_S *wait, const char* key);
int __destroy_wait(WAIT_S *wait);
int __sleep_wait(WAIT_S* wait,int millisecond);
int __wakeup(WAIT_S *wait);


// mutex 声明

/*
typedef pthread_mutex_t MUTEX_S;
void __init_mutex(MUTEX_S *mutex);
void __destroy_mutex(MUTEX_S*);
void __p(MUTEX_S *mutex);
void __v(MUTEX_S *mutex);
*/
#if 0

typedef struct mutex {
    int mutexid;
    struct sembuf p_sem;
    struct sembuf v_sem;
} MUTEX_S;


void __init_mutex(MUTEX_S *mutex, char* key, int key2);
void __destroy_mutex(MUTEX_S*);
void __p(MUTEX_S *mutex);
void __v(MUTEX_S *mutex);
#endif
// 定时器声明
typedef struct 
{
    struct sigaction new_sigalrm;
    struct sigaction old_sigalrm;
    struct itimerval new_timer;
    struct itimerval old_timer;
} AT_TIMER_S;
typedef void (*timer_callback_t)(int k);

int __start_timer(AT_TIMER_S* timer, int millisecond,  
	void (*timer_handler)(int));
int __stop_timer(AT_TIMER_S *timer);


// 线程声明
typedef struct
{
    int thread_id;
    int (*thread_fn)(void *arg);
    void* arg;
    void* stack_p;
    void * stack;
#if 0
    unsigned int stack_size;   // 默认64k
#endif
} THREAD_S;

int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
#if 0
, void* stack, unsigned int stack_size
#endif
);
int __start_thread(THREAD_S *pthread_t);
int __destroy_thread_t(THREAD_S *pthread_t);

#if 0//del by sxg, no interfaces
// zhouxuezhong 提供的库函数声明
extern void cm500_poweron();
extern void cm500_poweroff();
extern void rssi_ledshow (unsigned int value);
extern unsigned int bcm_powerstatus();
extern void wan_ledon(void);
extern void wan_ledoff(void);
#endif//0
// 全局变量
extern THREAD_S g_ppp_thread;          // PPP线程描述结构
extern THREAD_S g_st_at_report_thread; // AT上报接收线程描述结构

#endif // __UTILS_H
