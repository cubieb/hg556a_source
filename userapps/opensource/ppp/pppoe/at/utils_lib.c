/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : utils_lib.c
  作者     : lichangqing
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 
  功能描述 : 
      
  主要函数列表: 
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include "pppd.h"
#include "utils_lib.h"

// A64D01145 ADD (by l45517 2006年5月12日 ) BEGIN
#include "at_thread.h"
extern int g_main_thread_killed;
// A64D01145 ADD (by l45517 2006年5月12日 ) END

/*------------------------------------------------------------
  函数原型 : int __msleep(int millisecond )
  描述     : 实现以毫秒位单位的不被唤醒的等待
  输入     : millisecond(睡眠时间)
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
int __msleep(int millisecond) 
{
    int ret;
    struct timeval tvSelect;
    
    tvSelect.tv_sec  = millisecond / 1000;
    tvSelect.tv_usec = (millisecond % 1000) * 1000;

#ifdef DEBUG_UTILS
    struct timeval tm;
    gettimeofday(&tm, (struct timezone *)NULL);
#endif
    UTILS_DEBUG("__usleep begin time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));

    while (1)
    {
        ret = select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0,
        	(struct timeval*)&tvSelect);
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            FATAL("msleep : select error \n");
        }
        break;
    }
    
#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
#endif
    UTILS_DEBUG("__usleep end   time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));

    return 0;
}


/*------------------------------------------------------------
  函数原型 : int __init_wait(WAIT_S *wait, const char* key)
  描述     : 初始化一个wait_t结构
  输入     : wait(初始化的对象); key(wait的关键字)
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
int __init_wait(WAIT_S *wait, const char* key)
{
    int ret;

    // 创建fifo
    unlink(key);
    if ((ret = mkfifo(key, 
    	(O_CREAT | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))) < 0)
    {
        if (ret != EEXIST)
        {
            FATAL("cannot create fifoserver key = %s \n", key);
        }
    }

    // 拷贝fifo关键字
    ret = strlen(key) + 1;
    wait->key = (char*)malloc(ret);
    if (!wait->key)
    {
        FATAL("malloc failed \n");
    }
    
    strcpy(wait->key, key);
    return 0;
}


/*------------------------------------------------------------
  函数原型 : int __destroy_wait(WAIT_S *wait)
  描述     : 注销一个wait_t结构
  输入     : wait(被注销的指针)
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
int __destroy_wait(WAIT_S *wait)
{
    // 取消fifo
    unlink(wait->key);
    free(wait->key);
    return 0;
}


/*------------------------------------------------------------
  函数原型 : int __sleep_wait(WAIT_S* wait, int millisecond)
  描述     : 在wait上实现等待唤醒
  输入     : wait(等待结构); millisecond(超时时间, 单位毫秒)
  输出     : 无
  返回值   : 1 成功；0 超时
-------------------------------------------------------------*/
int __sleep_wait(WAIT_S* wait, int millisecond)
{
    int    i_ret;
    char   ch[4];
    int    fifo_fd;
    fd_set read_set;
    struct timeval tvSelect;

#ifdef DEBUG_UTILS
	struct timeval tm;
#endif

    // 打开fifo
    fifo_fd = open(wait->key, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0)
    {
        ERROR("fifo : open failed<%s>", wait->key);
        return 1;
    }

    // 设置等待时间
    tvSelect.tv_sec  = millisecond / 1000;
    tvSelect.tv_usec = (millisecond % 1000) * 1000;

    // select fifo 等待唤醒
#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
#endif

    UTILS_DEBUG("sleep time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
    
    while (1) {
        FD_ZERO(&read_set);
        FD_SET(fifo_fd, &read_set);
        i_ret = select(fifo_fd + 1, &read_set, NULL, NULL, &tvSelect);
        if (i_ret < 0)
        {
            // A64D01038 ADD (by l45517 2006年4月10日 ) BEGIN
            if (EINTR == errno) 
            {
                extern int g_main_thread_killed;
                if (g_main_thread_killed)
                {
                    return i_ret;
                }
                UTILS_DEBUG("fifo : select EINTR");
                continue;
            }
            // A64D01038 ADD (by l45517 2006年4月10日 ) END
            ERROR("fifo : select fifo error  \n");
            close(fifo_fd);
            return i_ret;
        }

        if (!i_ret)
        {
#ifdef DEBUG_UTILS
            gettimeofday(&tm, (struct timezone *)NULL);
            UTILS_DEBUG("timeout time is : sec = %d, ms = %d, us = %d ", 
                tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
#endif
            close(fifo_fd);
            return 0; //超时返回
        }
        break;
    }

#ifdef DEBUG_UTILS
    gettimeofday(&tm, (struct timezone *)NULL);
    UTILS_DEBUG("wakeup time is : sec = %d, ms = %d, us = %d ", 
        tm.tv_sec, (tm.tv_usec / 1000), (tm.tv_usec % 1000));
#endif

    // i_ret大于0，表示线程被唤醒
    i_ret = read(fifo_fd, ch, 4);
    if (4 != i_ret)
    {
        WARN("fifo : have not read enougn bytes \n");
    }

    close(fifo_fd);
    return 1;
}


/*------------------------------------------------------------
  函数原型 : int __wakeup(WAIT_S *wait)
  描述     : 唤醒在wait上等待的线程
  输入     : wait(等待的结构)
  输出     : 无
  返回值   : 0 成功
-------------------------------------------------------------*/
int __wakeup(WAIT_S *wait)
{
    int i_ret;
    int fifo_fd;
    char tmp[4] = {'A','T','S','B'};
    fd_set  write_set;
    struct  timeval tvSelect;

    // 打开fifo
    fifo_fd = open(wait->key, (O_WRONLY | O_NONBLOCK), 0);

    if (fifo_fd < 0)
    {
        if (ENXIO == errno)
        {
            WARN("fifo : no thread waiting");
        }
        else
        {
            ERROR("fifo : open failed<%s>", wait->key);
        }
        return 0;
    }
    
    tvSelect.tv_sec  = 1; // 等待1秒超时
    tvSelect.tv_usec = 0;
    UTILS_DEBUG("fifo : opened key = <%s>", wait->key);


    // 等待fifo可写
    /*while (1) {
        FD_ZERO(&write_set);
        FD_SET(fifo_fd, &write_set);
        i_ret = select(fifo_fd + 1, NULL, &write_set, NULL, &tvSelect);
        if (i_ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            error("[%s : %d] select fifo error \n", __FILE__, __LINE__);
            close(fifo_fd);
            return 0;
        }
        
        if (0 == i_ret) // 若一秒钟FIFO还不能写，说明超时
        {
            warn("[%s : %d] select fifo timeout \n", __FILE__, __LINE__);
            close(fifo_fd);
            return 0;
        }
    }*/

    // 唤醒等待线程
    UTILS_DEBUG("fifo wake up waiting thread");

    while(1) {
        i_ret = write(fifo_fd, tmp, sizeof(tmp));
        if (i_ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            
            if (EPIPE == errno)
            {
                WARN("fifo : no thread waiting \n");
            }
            else 
            {
                ERROR("fifo : write fifo failed \n");
            }
            
            close(fifo_fd);
            return 0;
        }

        // 写入为0，则重新写
        if (0 == i_ret)
        {
            UTILS_DEBUG("fifo write failed, retrying");
            continue; 
        }
        
        break;
    }
    
    UTILS_DEBUG("fifo wake up waiting thread successfully");

    if (sizeof(tmp) != i_ret)
    {
        WARN("fifo : do not write enough bytes\n");
    }

    close(fifo_fd);
    return 1;
}

#if 0
/*------------------------------------------------------------
  函数原型 : void __init_mutex(MUTEX_S * mutex)
  描述     : 初始化一个mutex结构
  输入     : mutex(被初始化的指针); key_str(信号量的字符串关键); key2(关键字2)
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void __init_mutex(MUTEX_S * mutex, char* key, int key2)
{
    key_t key_val;
    union semun un;

    UTILS_DEBUG("key = %s, key = %d", key, key2);
    key_val = ftok((const char*)key, key2);
    UTILS_DEBUG("ftok key = %d", key_val);

    mutex->mutexid = semget(key_val, 1,	IPC_CREAT |	00666);
    if (mutex->mutexid == -1)
    {
        ERROR("mutex : get semaphore error");
        // A64D01145 ADD (by l45517 2006年5月10日 ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006年5月10日 ) END
    }

    un.val = 1;
    semctl(mutex->mutexid, 0, SETVAL, un);

    mutex->p_sem.sem_num = 0;
    mutex->p_sem.sem_op  = -1;
    mutex->p_sem.sem_flg = 0;

    mutex->v_sem.sem_num = 0;
    mutex->v_sem.sem_op  = 1;
    mutex->v_sem.sem_flg = 0;
    return;
}


/*------------------------------------------------------------
  函数原型 : __destroy_mutex(MUTEX_S* mutex)
  描述     : 注销一个mutex
  输入     : mutex(注销的结构的指针)
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void  __destroy_mutex(MUTEX_S* mutex)
{
    if (semctl(mutex->mutexid, 0, IPC_RMID) < 0)
    {
        ERROR("mutex : destory semaphore error \n");
    }
}


/*------------------------------------------------------------
  函数原型 : void __p(MUTEX_S *mutex)
  描述     : mutex的p原语
  输入     : mutex(操作的结构)
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void __p(MUTEX_S *mutex)
{
    if(-1 == semop(mutex->mutexid, &mutex->p_sem, 1))
    {
        if (EINVAL == errno)
        {
            ERROR("mutex : v semaphore error(semaphore do not exit) \n");
            // A64D01145 ADD (by l45517 2006年5月10日 ) BEGIN
            g_main_thread_killed = AT_MAIN_EXITED;
            return;
            // A64D01145 ADD (by l45517 2006年5月10日 ) END
        }
		ERROR("mutex : p semaphore error \n");
        if (-1 == semctl(mutex->mutexid, 0, IPC_RMID))
        {
            ERROR("mutex : rm semaphore error \n");
        }
        // A64D01145 ADD (by l45517 2006年5月10日 ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006年5月10日 ) END
    }
}


/*------------------------------------------------------------
  函数原型 : void __v(MUTEX_S *mutex)
  描述     : mutex的v原语
  输入     : mutex(操作的结构)
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void __v(MUTEX_S *mutex)
{
    if(-1 == semop(mutex->mutexid, &mutex->v_sem, 1))
    {
        if (EINVAL == errno)
        {
            ERROR("mutex : v semaphore error(semaphore do not exit) \n");
            // A64D01145 ADD (by l45517 2006年5月10日 ) BEGIN
            g_main_thread_killed = AT_MAIN_EXITED;
            return;
            // A64D01145 ADD (by l45517 2006年5月10日 ) END
        }
		ERROR("mutex : v semaphore error \n");
        if (-1 == semctl(mutex->mutexid, 0, IPC_RMID))
        {
            ERROR("mutex : rm semaphore error \n");
        }
        // A64D01145 ADD (by l45517 2006年5月10日 ) BEGIN
        g_main_thread_killed = AT_MAIN_EXITED;
        return;
        // A64D01145 ADD (by l45517 2006年5月10日 ) END
    }
}
#endif

/*------------------------------------------------------------
  函数原型 : int __start_timer(AT_TIMER_S* timer, int millisecond,  void (*timer_handler)(int))
  描述     : 启动一个定时器
  输入     : timer(定时器结构); millisecond(定时器时间); timer_handler(定时器回调函数)
  输出     : 无
  返回值   : 0(成功)
-------------------------------------------------------------*/
static int g_is_timer_started = 0;

int __start_timer(AT_TIMER_S* timer, int millisecond,  
	void (*timer_handler)(int))
{
    sigset_t mask;
    
    if (g_is_timer_started)
    {
        WARN("timer : started \n");
        return SUCCESS;
    }

    g_is_timer_started = 1;

    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGUSR2);

    timer->new_sigalrm.sa_handler = timer_handler;
    // sigemptyset(&timer->new_sigalrm.sa_mask);
    timer->new_sigalrm.sa_mask  = mask;
    timer->new_sigalrm.sa_flags = 0;

    // 设置SIGALRM信号的信号处理函数
    sigaction(SIGALRM, &timer->new_sigalrm, &timer->old_sigalrm);

    // 设置定时器时间
    timer->new_timer.it_interval.tv_sec  = millisecond / 1000;
    timer->new_timer.it_value   .tv_sec  = millisecond / 1000;
    timer->new_timer.it_interval.tv_usec = millisecond % 1000 * 1000;
    timer->new_timer.it_value   .tv_usec = millisecond % 1000 * 1000;

    // 启动定时器
    setitimer(ITIMER_REAL, &timer->new_timer, &timer->old_timer);

    UTILS_DEBUG("cm500 shakehand timer started");
    return SUCCESS;
}


/*------------------------------------------------------------
  函数原型 : int __stop_timer(AT_TIMER_S *timer)
  描述     : 取消定时器
  输入     : timer(定时器结构)
  输出     : 无
  返回值   : 0(成功)
-------------------------------------------------------------*/
int __stop_timer(AT_TIMER_S *timer)
{
    if (!g_is_timer_started)
    {
        // INFO("timer : has not started\n");
        return 0;
    }

    g_is_timer_started = 0;
    setitimer(ITIMER_REAL, &timer->old_timer,   NULL);
    sigaction(SIGALRM,     &timer->old_sigalrm, NULL);
    INFO("timer : has stopped\n");
    return 0;
}


/*------------------------------------------------------------
  函数原型 : int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
  描述     : 初始化线程结构
  输入     : 无
  输出     : 无
  返回值   : 1(成功)；0(失败)
-------------------------------------------------------------*/
int __init_thread_t(THREAD_S * pthread, int (*thread_fn)(void *arg), void* arg
#if 0
, void* stack, unsigned int stack_size
#endif
)
{
    int * t;
     //printf("Caoxiang :%s : %s()进程号:%d\r\n",__FILE__,__FUNCTION__,  getpid());
    // assert(pthread, "the pthread is null.");
    // assert(thread_fn,    "the thread_fn is   null.");
    pthread->thread_fn = thread_fn;
    pthread->arg = arg;
    t = malloc(sizeof(int) * 1024   * 16);
    if (!t)
    {
        FATAL("thread : molloc THREAD_S failed \n");
        return FAILED;
    }
    pthread->stack_p = t;
    pthread->stack = &t[1024 * 16   - 1];   
    return SUCCESS;
}


/*------------------------------------------------------------
  函数原型 : int __start_thread(THREAD_S *pthread)
  描述     : 启动线程
  输入     : pthread(要启动的线程的结构)
  输出     : 无
  返回值   : 线程号
-------------------------------------------------------------*/
int __start_thread(THREAD_S *pthread)
{
    int flag = CLONE_VM | CLONE_FS | CLONE_FILES | SIGCHLD;
     //printf("Caoxiang :%s : %s()进程号:%d\r\n",__FILE__,__FUNCTION__,  getpid());
    return pthread->thread_id = 
    	clone(pthread->thread_fn, pthread->stack, flag, pthread->arg);  
}


/*------------------------------------------------------------
  函数原型 : int __destroy_thread_t(THREAD_S *pthread_t)
  描述     : 线程结构注销
  输入     : pthread(注销的线程的结构)
  输出     : 无
  返回值   : 0 (成功)
-------------------------------------------------------------*/
int __destroy_thread_t(THREAD_S *pthread)
{
    if (pthread->stack_p)
    {
        free(pthread->stack_p);
        pthread->stack = NULL;
    }
    return 0;
}


/*------------------------------------------------------------
  函数原型 : int get_thread_esp(int* esp_val)
  描述     : 获取线程的当前堆栈指针
  输入     : esp_val : local变量指针
  输出     : 无
  返回值   : 线程堆栈指针
-------------------------------------------------------------*/
int* __get_thread_esp(int* esp_val)
{

// 汇编处理待完善
#if 0
    void* p_esp;

    __asm__ __volatile__(
        ".set push \n"
    	".set noat \n"
    	"mov %0, $29"
        ".set pop \n"
        :"=r" (p_esp)
    );

    return p_esp;
#else

    return esp_val;

#endif
}


/*------------------------------------------------------------
  函数原型 : int* __errno_location()
  描述     : 重载c库函数以支持多线程的出错处理号errno,该函数必须放到文件的结尾
  输入     : esp_val : local变量指针
  输出     : 无
  返回值   : 线程堆栈指针
-------------------------------------------------------------*/
#undef errno

int g_main_errno = 0;
int g_ppp_errno  = 0;
int g_at_errno   = 0;

int* __errno_location()
{

    int esp_test;
    void* p_esp_val;
   
    p_esp_val = __get_thread_esp(&esp_test);

    if ((p_esp_val <= g_ppp_thread.stack  ) &&
        (p_esp_val >= g_ppp_thread.stack_p))
    {
        return &g_ppp_errno;
    }

    if ((p_esp_val <= g_st_at_report_thread.stack  ) &&
        (p_esp_val >= g_st_at_report_thread.stack_p))
    {
        return &g_at_errno;
    }

    return &g_main_errno;	
}
