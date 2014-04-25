/* vi: set sw=4 ts=4: */
/*
 * Mini syslogd implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Copyright (C) 2000 by Karl M. Hegbloom <karlheg@debian.org>
 *
 * "circular buffer" Copyright (C) 2001 by Gennady Feldman <gfeldman@gena01.com>
 *
 * Maintainer: Gennady Feldman <gfeldman@gena01.com> as of Mar 12, 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/param.h>

#include "busybox.h"

/* SYSLOG_NAMES defined to pull some extra junk from syslog.h */
#define SYSLOG_NAMES
#include <sys/syslog.h>
#include <sys/uio.h>
/*add by z67625 增加支持防火墙日志 start*/
#include <sys/stat.h>
#include <fcntl.h>
#include "board_api.h"
/*add by z67625 增加支持防火墙日志 end*/

/*BRCM begin*/
int localLogLevel=-1;
int remoteLogLevel=-1;
/*BRCM end*/

/* Path for the file where all log messages are written */
#define __LOG_FILE "/var/log/messages"

/*start of 增加从flash读日志到内存系统日志中by c00131380 at 080927*/
#define SYSLOG_BUF_SIZE_MAX 16*1024
/*end of 增加从flash读日志到内存系统日志中by c00131380 at 080927*/

/* Path to the unix socket */
static char lfile[MAXPATHLEN];

static const char *logFilePath = __LOG_FILE;

#ifdef CONFIG_FEATURE_ROTATE_LOGFILE
/* max size of message file before being rotated */
static int logFileSize = 200 * 1024;

/* number of rotated message files */
static int logFileRotate = 1;
#endif

/* interval between marks in seconds */
/*BRCM begin*/
/*static int MarkInterval = 20 * 60; */
static int MarkInterval = 60 * 60;
/*BRCM end*/

/* localhost's name */
static char LocalHostName[64];

#ifdef CONFIG_FEATURE_REMOTE_LOG
#include <netinet/in.h>
/* udp socket for logging to remote host */
static int remotefd = -1;
static struct sockaddr_in remoteaddr;

/* where do we log? */
static char *RemoteHost;

/* what port to log to? */
static int RemotePort = 514;

/* To remote log or not to remote log, that is the question. */
static int doRemoteLog = FALSE;
static int local_logging = FALSE;
#endif

/* Make loging output smaller. */
static bool small = false;

#define MAXLINE         1024	/* maximum line length */

/*add by z67625 增加防火墙日志级别 start*/
#define FW_LOG_LV 8
/*add by z67625 增加防火墙日志级别 end*/
/* circular buffer variables/structures */
#ifdef CONFIG_FEATURE_IPC_SYSLOG

#if CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE < 4
#error Sorry, you must set the syslogd buffer size to at least 4KB.
#error Please check CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE
#endif

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/* our shared key */
static const long KEY_ID = 0x414e4547;	/*"GENA" */

// Semaphore operation structures
static struct shbuf_ds {
	int size;			// size of data written
	int head;			// start of message list
	int tail;			// end of message list
	char data[1];		// data/messages
} *buf = NULL;			// shared memory pointer

static struct sembuf SMwup[1] = { {1, -1, IPC_NOWAIT} };	// set SMwup
static struct sembuf SMwdn[3] = { {0, 0}, {1, 0}, {1, +1} };	// set SMwdn

static int shmid = -1;	// ipc shared memory id
static int s_semid = -1;	// ipc semaphore id
static int shm_size = ((CONFIG_FEATURE_IPC_SYSLOG_BUFFER_SIZE)*1024);	// default shm size
static int circular_logging = FALSE;

static void init_RemoteLog(void);

/*add by z67625 防火墙日志结构定义 start*/
static struct st_Fwlog
{
    struct st_Fwlog *next;
    int log_size;    //记录大小
    int log_num;     //记录数目
    char *data;      //记录数据
} st_Fwlog;
/*add by z67625 防火墙日志结构定义 end*/


/*
 * sem_up - up()'s a semaphore.
 */
static inline void sem_up(int semid)
{
	if (semop(semid, SMwup, 1) == -1) {
		bb_perror_msg_and_die("semop[SMwup]");
	}
}

/*
 * sem_down - down()'s a semaphore
 */
static inline void sem_down(int semid)
{
	if (semop(semid, SMwdn, 3) == -1) {
		bb_perror_msg_and_die("semop[SMwdn]");
	}
}


void ipcsyslog_cleanup(void)
{
	printf("Exiting Syslogd!\n");
	if (shmid != -1) {
		shmdt(buf);
	}

	if (shmid != -1) {
		shmctl(shmid, IPC_RMID, NULL);
	}
	if (s_semid != -1) {
		semctl(s_semid, 0, IPC_RMID, 0);
	}
}

void ipcsyslog_init(void)
{
	if (buf == NULL) 
    {
	    if ((shmid = shmget(KEY_ID, shm_size, IPC_CREAT | IPC_EXCL | 0666)) == -1) 
        {
		    bb_perror_msg_and_die("shmget");
	    }

		if ((buf = shmat(shmid, (void*)0x58800000, 0)) == NULL) 
        	{
			bb_perror_msg_and_die("shmat");
		}


		// we'll trust the OS to set initial semval to 0 (let's hope)
		if ((s_semid = semget(KEY_ID, 2, IPC_CREAT | IPC_EXCL | 1023)) == -1) 
        	{
			if (errno == EEXIST) 
            		{
				if ((s_semid = semget(KEY_ID, 2, 0)) == -1) 
                		{
					bb_perror_msg_and_die("semget");
				}
			} 
	            else 
	            {
					bb_perror_msg_and_die("semget");
		      }
		}

		sem_down(s_semid);

		if (sysSystemLogGet((char*)buf, shm_size, 0) < 0)
	       {
	            printf("read buf is failure\n");
	       }

		 if (buf->head > shm_size || buf->head < 0)
	 	 {
	 	 	 buf->head = buf->tail = 0;
	 	 }

		 if (buf->tail > shm_size || buf->tail < 0)
	 	 {
	 	 	 buf->head = buf->tail = 0;
	 	 }

		 buf->size = shm_size - sizeof(*buf);

		 sem_up(s_semid);

        	 printf("init syslog is end\n");
	} 
    else 
    {
		printf("Buffer already allocated just grab the semaphore?");
	}
}

/* write message to buffer */
void circ_message(const char *msg)
{
	int l = strlen(msg) + 1;	/* count the whole message w/ '\0' included */

	sem_down(s_semid);

	/*
	 * Circular Buffer Algorithm:
	 * --------------------------
	 *
	 * Start-off w/ empty buffer of specific size SHM_SIZ
	 * Start filling it up w/ messages. I use '\0' as separator to break up messages.
	 * This is also very handy since we can do printf on message.
	 *
	 * Once the buffer is full we need to get rid of the first message in buffer and
	 * insert the new message. (Note: if the message being added is >1 message then
	 * we will need to "remove" >1 old message from the buffer). The way this is done
	 * is the following:
	 *      When we reach the end of the buffer we set a mark and start from the beginning.
	 *      Now what about the beginning and end of the buffer? Well we have the "head"
	 *      index/pointer which is the starting point for the messages and we have "tail"
	 *      index/pointer which is the ending point for the messages. When we "display" the
	 *      messages we start from the beginning and continue until we reach "tail". If we
	 *      reach end of buffer, then we just start from the beginning (offset 0). "head" and
	 *      "tail" are actually offsets from the beginning of the buffer.
	 *
	 * Note: This algorithm uses Linux IPC mechanism w/ shared memory and semaphores to provide
	 *       a threasafe way of handling shared memory operations.
	 */
	if ((buf->tail + l) < buf->size) {
		/* before we append the message we need to check the HEAD so that we won't
		   overwrite any of the message that we still need and adjust HEAD to point
		   to the next message! */
		if (buf->tail < buf->head) {
			if ((buf->tail + l) >= buf->head) {
				/* we need to move the HEAD to point to the next message
				 * Theoretically we have enough room to add the whole message to the
				 * buffer, because of the first outer IF statement, so we don't have
				 * to worry about overflows here!
				 */
				int k = buf->tail + l - buf->head;	/* we need to know how many bytes
													   we are overwriting to make
													   enough room */
				char *c =
					memchr(buf->data + buf->head + k, '\0',
						   buf->size - (buf->head + k));
				if (c != NULL) {	/* do a sanity check just in case! */
					buf->head = c - buf->data + 1;	/* we need to convert pointer to
													   offset + skip the '\0' since
													   we need to point to the beginning
													   of the next message */
					/* Note: HEAD is only used to "retrieve" messages, it's not used
					   when writing messages into our buffer */
				} else {	/* show an error message to know we messed up? */
					printf("Weird! Can't find the terminator token??? \n");
					buf->head = 0;
				}
			}
		}

		/* in other cases no overflows have been done yet, so we don't care! */
		/* we should be ok to append the message now */
		strncpy(buf->data + buf->tail, msg, l);	/* append our message */
		buf->tail += l;	/* count full message w/ '\0' terminating char */
	} else {
		/* we need to break up the message and "circle" it around */
		char *c;
		int k = buf->tail + l - buf->size;	/* count # of bytes we don't fit */

		/* We need to move HEAD! This is always the case since we are going
		 * to "circle" the message.
		 */
		c = memchr(buf->data + k, '\0', buf->size - k);

		if (c != NULL) {	/* if we don't have '\0'??? weird!!! */
			/* move head pointer */
			buf->head = c - buf->data + 1;

			/* now write the first part of the message */
			strncpy(buf->data + buf->tail, msg, l - k - 1);

			/* ALWAYS terminate end of buffer w/ '\0' */
			buf->data[buf->size - 1] = '\0';

			/* now write out the rest of the string to the beginning of the buffer */
			strcpy(buf->data, &msg[l - k - 1]);

			/* we need to place the TAIL at the end of the message */
			buf->tail = k + 1;
		} else {
			printf
				("Weird! Can't find the terminator token from the beginning??? \n");
			buf->head = buf->tail = 0;	/* reset buffer, since it's probably corrupted */
		}

	}
	sem_up(s_semid);
}
#endif							/* CONFIG_FEATURE_IPC_SYSLOG */

/* Note: There is also a function called "message()" in init.c */
/* Print a message to the log file. */
static void message(char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
static void message(char *fmt, ...)
{
	int fd;
	struct flock fl;
	va_list arguments;

	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;

#ifdef CONFIG_FEATURE_IPC_SYSLOG
	if ((circular_logging == TRUE) && (buf != NULL)) {
		char b[1024];

		va_start(arguments, fmt);
		vsnprintf(b, sizeof(b) - 1, fmt, arguments);
		va_end(arguments);
		circ_message(b);

	} else
#endif
	if ((fd =
			 device_open(logFilePath,
							 O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND |
							 O_NONBLOCK)) >= 0) {
		fl.l_type = F_WRLCK;
		fcntl(fd, F_SETLKW, &fl);
#ifdef CONFIG_FEATURE_ROTATE_LOGFILE
		if ( logFileSize > 0 ) {
			struct stat statf;
			int r = fstat(fd, &statf);
			if( !r && (statf.st_mode & S_IFREG)
				&& (lseek(fd,0,SEEK_END) > logFileSize) ) {
				if(logFileRotate > 0) {
					int i;
					char oldFile[(strlen(logFilePath)+3)], newFile[(strlen(logFilePath)+3)];
					for(i=logFileRotate-1;i>0;i--) {
						sprintf(oldFile, "%s.%d", logFilePath, i-1);
						sprintf(newFile, "%s.%d", logFilePath, i);
						rename(oldFile, newFile);
					}
					sprintf(newFile, "%s.%d", logFilePath, 0);
					fl.l_type = F_UNLCK;
					fcntl (fd, F_SETLKW, &fl);
					close(fd);
					rename(logFilePath, newFile);
					fd = device_open (logFilePath,
						   O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND |
						   O_NONBLOCK);
					fl.l_type = F_WRLCK;
					fcntl (fd, F_SETLKW, &fl);
				} else {
					ftruncate( fd, 0 );
				}
			}
		}
#endif
		va_start(arguments, fmt);
		vdprintf(fd, fmt, arguments);
		va_end(arguments);
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLKW, &fl);
		close(fd);
	} else {
		/* Always send console messages to /dev/console so people will see them. */
		if ((fd =
			 device_open(_PATH_CONSOLE,
						 O_WRONLY | O_NOCTTY | O_NONBLOCK)) >= 0) {
			va_start(arguments, fmt);
			vdprintf(fd, fmt, arguments);
			va_end(arguments);
			close(fd);
		} else {
			fprintf(stderr, "Bummer, can't print: ");
			va_start(arguments, fmt);
			vfprintf(stderr, fmt, arguments);
			fflush(stderr);
			va_end(arguments);
		}
	}
}

/*add by z67625 增加支持防火墙日志 start*/
#define FW_LOG_PATH "/var/fw_log"  //日志保存路径

#define FW_LOG_MAXSIZE 10*1024   //日志最大为10k
#define FW_LOG_MAXNUM  100    //日志记录最多为100条

struct st_Fwlog *g_phead = NULL;
struct st_Fwlog *g_ptail = NULL;
int IsInited = FALSE;

/*增加新记录*/
struct st_Fwlog* MallocNewNode(char *msg)
{
    struct st_Fwlog *ptmp = NULL;
    if(NULL == msg)
    {
        printf("msg pointer is null\n");
        return NULL;
    }
    
    ptmp = malloc(sizeof(struct st_Fwlog));
    if(ptmp == NULL)
    {
        printf("syslogd.c: malloc failed\n");
        return NULL;
    }
    ptmp->data = malloc(strlen(msg)+1);
    if(ptmp->data == NULL)
    {
        printf("syslogd.c: malloc data failed\n");
        free(ptmp);
        return NULL;
    }
    memset(ptmp->data, 0 , strlen(msg) + 1);
    strcpy(ptmp->data, msg);
    ptmp->log_size = strlen(msg);
    ptmp->next = NULL;

    return ptmp;
}

/*从flash获取防火墙日志并形成链表*/
struct st_Fwlog* GetLogfromFILE(void)
{
    FILE *fp = NULL;
    char acLine[1024];
    struct st_Fwlog *phead = NULL;
    struct st_Fwlog *ptmp = NULL;
    struct st_Fwlog *pnew = NULL;
    char acbuf[FW_LOG_MAXSIZE];

    memset(acbuf, 0, FW_LOG_MAXSIZE);
    
    if(NULL == (fp = fopen(FW_LOG_PATH,"w")))
    {
        printf("create file %s failed",FW_LOG_PATH);
        return NULL;
    }
    
    /*从flash读取log文件写入文件*/
    sysFirewallLogGet(acbuf, FW_LOG_MAXSIZE, 0);
    /*防止没有日志时，打印出空日志*/
    if(NULL != strstr(acbuf,"fwlog"))
    {
        fwrite(acbuf, sizeof(char), FW_LOG_MAXSIZE, fp);
    }
    else
    {
        printf("acbuf is null\n");
    }
    fclose(fp);

    /*创建链表*/
    if(NULL == (fp = fopen(FW_LOG_PATH,"r")))
    {
        printf("read file %s failed",FW_LOG_PATH);
        return NULL;
    }
    
    phead = malloc(sizeof(struct st_Fwlog));
    if(NULL == phead)
    {
        printf("malloc failed\n");
        fclose(fp);
        return NULL;
    }

    memset(phead, 0, sizeof(struct st_Fwlog));
    phead->next = NULL;
    ptmp = phead;
    
    while(NULL != (fgets(acLine,1024,fp)))
    {
        pnew = MallocNewNode(acLine);
        if(pnew == NULL)
        {
            continue;
        }
        ptmp->next = pnew;
        ptmp = pnew;
        phead->log_num++;
        phead->log_size += ptmp->log_size;
    }
    
    fclose(fp);

    return phead;
}

/*防火墙日志初始化*/
static int FwLog_Init(void)
{
    if(IsInited == FALSE)
    {
        g_phead = GetLogfromFILE();
        g_ptail = g_phead;
        while(g_ptail->next != NULL)
        {
            g_ptail = g_ptail->next;
        }
        IsInited = TRUE;
    }
    return TRUE;
}

/*新增记录写入日志文件*/
static void WriteFwlog(char *msg)
{
    FILE *fp = NULL;
    struct st_Fwlog *ptmp = NULL;

    if(NULL == msg)
    {
        printf("pointer to msg is null\n");
        return;
    } 

    if(IsInited == FALSE)
    {
        printf("fwlog is still not inited\n");
        return;
    }

    /*如果超过最大值，需要循环记录*/
    if(g_phead->log_size + strlen(msg) >= FW_LOG_MAXSIZE 
        || g_phead->log_num >= FW_LOG_MAXNUM)
    {
        printf("The file size will beyond the maxsize,so circulating record it \n");
        ptmp = g_phead->next;
        g_phead->log_size -= ptmp->log_size;
        g_phead->log_num--;
        g_phead->next = ptmp->next;
        free(ptmp->data);
        free(ptmp);        
    }
    else
    {
        
    }

    ptmp = MallocNewNode(msg);

    if(ptmp == NULL)
    {
        printf("malloc new node failed\n");
        return;
    }
    g_ptail->next = ptmp;
    g_ptail = ptmp;
    g_phead->log_size += ptmp->log_size;
    g_phead->log_num++;

    if(NULL == (fp = fopen(FW_LOG_PATH, "w")))
    {
        printf("open file %s failed\n",FW_LOG_PATH);
        return;
    }
    ptmp = g_phead->next;
    while(NULL != ptmp)
    {
        fputs(ptmp->data,fp);
        ptmp = ptmp->next;
    }
    fclose(fp);
    
}

/*记录防火墙日志*/
static void logFirewallMessage(char *msg)
{
    char *p = NULL;
    printf("{%s}\n",msg);
    if(NULL == strstr(msg,"\n"))
    {
        if(NULL == (p = malloc(strlen(msg) + 2)))
        {
            printf("malloc failed\n");
            return;
        }        
        sprintf(p, "%s\n",msg);
        WriteFwlog(p);
        free(p);
    }
    else
    {
        WriteFwlog(msg);
    }
}
/*add by z67625 增加支持防火墙日志 end*/

static void logMessage(int pri, char *msg)
{
	time_t now;
	char *timestamp;
	static char res[20] = "";
#ifdef CONFIG_FEATURE_REMOTE_LOG	
	static char line[MAXLINE + 1];
#endif
	CODE *c_pri, *c_fac;
/*BRCM begin*/
	int localLog=1;
	int remoteLog=1;
	int len;
/*BRCM end*/
    /*add by z67625 如果为防火墙日志级别，记录防火期日志 start*/
    /*modified by z67625 根据特征字符串[fwlog]判断为防火墙日志*/
    if(pri == FW_LOG_LV && NULL != strstr(msg, "[fwlog]"))
    {
        logFirewallMessage(msg);
        return;
    }
    /*add by z67625 如果为防火墙日志级别，记录防火期日志 end*/

	if (pri != 0) {
		for (c_fac = facilitynames;
			 c_fac->c_name && !(c_fac->c_val == LOG_FAC(pri) << 3); c_fac++);
		for (c_pri = prioritynames;
			 c_pri->c_name && !(c_pri->c_val == LOG_PRI(pri)); c_pri++);
		if (c_fac->c_name == NULL || c_pri->c_name == NULL) {
			snprintf(res, sizeof(res), "<%d>", pri);
		} else {
			snprintf(res, sizeof(res), "%s.%s", c_fac->c_name, c_pri->c_name);
		}
	}

/*BRCM begin*/
	if ((pri != 0) && (c_pri->c_name != NULL)) {
		if (c_pri->c_val > localLogLevel) 
			localLog = 0;
		if (c_pri->c_val > remoteLogLevel)
			remoteLog = 0;
	}
	if (!localLog && !remoteLog)
		return;
/*BRCM end*/

	if (strlen(msg) < 16 || msg[3] != ' ' || msg[6] != ' ' ||
		msg[9] != ':' || msg[12] != ':' || msg[15] != ' ') {
		time(&now);
		timestamp = ctime(&now) + 4;
		timestamp[15] = '\0';
	} else {
		timestamp = msg;
		timestamp[15] = '\0';
		msg += 16;
	}

	/* todo: supress duplicates */

#ifdef CONFIG_FEATURE_REMOTE_LOG
	if (doRemoteLog == TRUE) {
		/* trying connect the socket */
		if (-1 == remotefd) {
			init_RemoteLog();
		}

		/* if we have a valid socket, send the message */
		if (-1 != remotefd) {
			now = 1;
			/* start of maintain 增加log信息内容 by liuzhijie 00028714 2006年7月11日 */
#ifdef SUPPORT_KPN_SYSLOG
			snprintf(line, sizeof(line), "%s %s %s %s", "KPN", "VTMOS", res, msg);
#else
			snprintf(line, sizeof(line), "<%d> %s", pri, msg);
#endif
			/* end of maintain 增加log信息内容 by liuzhijie 00028714 2006年7月11日 */

		retry:
			/* send message to remote logger */
			if(( -1 == sendto(remotefd, line, strlen(line), 0,
							(struct sockaddr *) &remoteaddr,
							sizeof(remoteaddr))) && (errno == EINTR)) {
				/* sleep now seconds and retry (with now * 2) */
				sleep(now);
				now *= 2;
				goto retry;
			}
		}
	}

/*BRCM begin*/
//	if (local_logging == TRUE)
        /* bug fix ; circular_logging should do too; it only checked
		 *            -L local logging.   */
	if (((local_logging == TRUE) || (circular_logging)) && localLog) {
#endif
        //		/* now spew out the message to wherever it is supposed to go */
        //		message("%s %s %s %s\n", timestamp, LocalHostName, res, msg);
        /* brcm, add len of string to do log display with latestest event displayed first */
        /* now spew out the message to wherever it is supposed to go; 4 spaces+ \0 + \n + len(3bytes) */
		len = (strlen(timestamp)+strlen(LocalHostName)+strlen(res)+strlen(msg)+9);
		message("%s %s %s %s %3i\n", timestamp, LocalHostName, res, msg, len);
#ifdef CONFIG_FEATURE_REMOTE_LOG
        }
#endif
/*BRCM end*/
}

static void quit_signal(int sig)
{
/*BRCM begin */
	char pidfilename[30];
	/* change to priority emerg to be consistent with klogd */
	logMessage(LOG_SYSLOG | LOG_EMERG, "System log daemon exiting.");
        // logMessage(LOG_SYSLOG | LOG_INFO, "System log daemon exiting.");

	unlink(lfile);
	sprintf (pidfilename, "%s%s.pid", _PATH_VARRUN, "syslogd");
	unlink (pidfilename);
/*BRCM end*/

	unlink(lfile);
#ifdef CONFIG_FEATURE_IPC_SYSLOG
	ipcsyslog_cleanup();
#endif

	exit(TRUE);
}

static void domark(int sig)
{
	if (MarkInterval > 0) {
		logMessage(LOG_SYSLOG | LOG_INFO, "-- MARK --");
		alarm(MarkInterval);
	}
}

/* This must be a #define, since when CONFIG_DEBUG and BUFFERS_GO_IN_BSS are
 * enabled, we otherwise get a "storage size isn't constant error. */
static int serveConnection(char *tmpbuf, int n_read)
{
	char *p = tmpbuf;

	while (p < tmpbuf + n_read) {

		int pri = (LOG_USER | LOG_NOTICE);
		int num_lt = 0;
		char line[MAXLINE + 1];
		unsigned char c;
		char *q = line;

		while ((c = *p) && q < &line[sizeof(line) - 1]) {
			if (c == '<' && num_lt == 0) {
				/* Parse the magic priority number. */
				num_lt++;
				pri = 0;
				while (isdigit(*(++p))) {
					pri = 10 * pri + (*p - '0');
				}
				if (pri & ~(LOG_FACMASK | LOG_PRIMASK)) {
					pri = (LOG_USER | LOG_NOTICE);
				}
			} else if (c == '\n') {
				*q++ = ' ';
			} else if (iscntrl(c) && (c < 0177)) {
				*q++ = '^';
				*q++ = c ^ 0100;
			} else {
				*q++ = c;
			}
			p++;
		}
		*q = '\0';
		p++;
		/* Now log it */
		logMessage(pri, line);
	}
	return n_read;
}


#ifdef CONFIG_FEATURE_REMOTE_LOG
static void init_RemoteLog(void)
{
	memset(&remoteaddr, 0, sizeof(remoteaddr));
	remotefd = socket(AF_INET, SOCK_DGRAM, 0);

	if (remotefd < 0) {
          /* BRCM begin */
          if ((local_logging == FALSE) && (circular_logging == FALSE))
		bb_error_msg_and_die("cannot create socket");
          else
                bb_perror_msg("cannot create socket");
          /* BRCM end */
	}

	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr = *(struct in_addr *) *(xgethostbyname(RemoteHost))->h_addr_list;
	remoteaddr.sin_port = htons(RemotePort);
}
#endif

static void doSyslogd(void) __attribute__ ((noreturn));
static void doSyslogd(void)
{
	struct sockaddr_un sunx;
	socklen_t addrLength;
/*BRCM begin*/
	FILE *pidfile;
	char pidfilename[30];
/* All the access to /dev/log will be redirected to /var/log/log
 *  * which is TMPFS, memory file system.
 **/
#define BRCM_PATH_LOG "/var/log/log"
/*BRCM end*/

	int sock_fd;
	fd_set fds;

	/* Set up signal handlers. */
	signal(SIGINT, quit_signal);
	signal(SIGTERM, quit_signal);
	signal(SIGQUIT, quit_signal);
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
#ifdef SIGCLD
	signal(SIGCLD, SIG_IGN);
#endif
	signal(SIGALRM, domark);
	alarm(MarkInterval);

	/* Create the syslog file so realpath() can work. */
/*BRCM begin*/
	/*
	if (realpath (_PATH_LOG, lfile) != NULL) {
	*/
	if (realpath (BRCM_PATH_LOG, lfile) != NULL) {
/*BRCM end*/
		unlink (lfile);
        }

	memset(&sunx, 0, sizeof(sunx));
	sunx.sun_family = AF_UNIX;
	strncpy(sunx.sun_path, lfile, sizeof(sunx.sun_path));
	if ((sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
/*BRCM begin*/
/*		bb_perror_msg_and_die("Couldn't get file descriptor for socket "
						   _PATH_LOG);
*/
		bb_error_msg_and_die ("Couldn't get file descriptor for socket " BRCM_PATH_LOG);
/*BRCM end*/
	}

	addrLength = sizeof(sunx.sun_family) + strlen(sunx.sun_path);
	if (bind(sock_fd, (struct sockaddr *) &sunx, addrLength) < 0) {
/*BRCM begin*/
/*		bb_perror_msg_and_die("Could not connect to socket " _PATH_LOG);
 */
		bb_error_msg_and_die ("Could not connect to socket " BRCM_PATH_LOG);
/*BRCM end*/
	}

	if (chmod(lfile, 0666) < 0) {
/*BRCM begin*/
/*		bb_perror_msg_and_die("Could not set permission on " _PATH_LOG);
 */
		bb_error_msg_and_die ("Could not set permission on " BRCM_PATH_LOG);
/*BRCM end*/
	}
#ifdef CONFIG_FEATURE_IPC_SYSLOG
	if (circular_logging == TRUE) {
		ipcsyslog_init();
	}
#endif

#ifdef CONFIG_FEATURE_REMOTE_LOG
	if (doRemoteLog == TRUE) {
		init_RemoteLog();
	}
#endif

/*BRCM begin*/
	if (localLogLevel < LOG_EMERG)
		localLogLevel = LOG_DEBUG;
	if (remoteLogLevel < LOG_EMERG)
		remoteLogLevel = LOG_ERR;

	/* change to priority emerg to be consistent with klogd */
        /* logMessage(LOG_SYSLOG | LOG_INFO, "syslogd started: " BB_BANNER); */
	logMessage (LOG_SYSLOG | LOG_EMERG, "HG556  started: " BB_BANNER);

	sprintf (pidfilename, "%s%s.pid", _PATH_VARRUN, "syslogd");
	if ((pidfile = fopen (pidfilename, "w")) != NULL) {
		fprintf (pidfile, "%d\n", getpid ());
		(void) fclose (pidfile);
	}
	else {
		logMessage ((LOG_SYSLOG | LOG_ERR),("Failed to create pid file %s", pidfilename));
	}
/*BRCM end*/

	for (;;) {

		FD_ZERO(&fds);
		FD_SET(sock_fd, &fds);

		if (select(sock_fd + 1, &fds, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				/* alarm may have happened. */
				continue;
			}
			bb_perror_msg_and_die("select error");
		}

		if (FD_ISSET(sock_fd, &fds)) {
			int i;

			RESERVE_CONFIG_BUFFER(tmpbuf, MAXLINE + 1);

			memset(tmpbuf, '\0', MAXLINE + 1);
			if ((i = recv(sock_fd, tmpbuf, MAXLINE, 0)) > 0) {
				serveConnection(tmpbuf, i);
			} else {
				bb_perror_msg_and_die("UNIX socket error");
			}
			RELEASE_CONFIG_BUFFER(tmpbuf);
		}				/* FD_ISSET() */
	}					/* for main loop */
}

extern int syslogd_main(int argc, char **argv)
{
	int opt;

	int doFork = TRUE;

	char *p;

	/* do normal option parsing */
/*BRCM begin*/
	/*
	while ((opt = getopt(argc, argv, "m:nO:s:b:R:LC::")) > 0) {
	*/
	/* brcm, l - local log level, r - remote log level */
	while ((opt = getopt(argc, argv, "m:nO:s:Sb:R:l:r:LC")) > 0) {
/*BRCM end*/
		switch (opt) {
		case 'm':
			MarkInterval = atoi(optarg) * 60;
			break;
		case 'n':
			doFork = FALSE;
			break;
		case 'O':
			logFilePath = optarg;
			break;
#ifdef CONFIG_FEATURE_ROTATE_LOGFILE
		case 's':
			logFileSize = atoi(optarg) * 1024;
			break;
		case 'b':
			logFileRotate = atoi(optarg);
			if( logFileRotate > 99 ) logFileRotate = 99;
			break;
#endif
#ifdef CONFIG_FEATURE_REMOTE_LOG
		case 'R':
			RemoteHost = bb_xstrdup(optarg);
			if ((p = strchr(RemoteHost, ':'))) {
				RemotePort = atoi(p + 1);
				*p = '\0';
			}
			doRemoteLog = TRUE;
			break;
		case 'L':
			local_logging = TRUE;
			break;
#endif
#ifdef CONFIG_FEATURE_IPC_SYSLOG
		case 'C':
			if (optarg) {
				int buf_size = atoi(optarg);
				if (buf_size >= 4) {
					shm_size = buf_size * 1024;
				}
			}
			circular_logging = TRUE;
			break;
#endif
		case 'S':
		        small = true;
			break;
/*BRCM begin*/
		case 'l':
			localLogLevel = atoi(optarg);
			break;
		case 'r':
			remoteLogLevel = atoi(optarg);
			break; 
/*BRCM end*/
		default:
			bb_show_usage();
		}
	}

    /*add by z67625 防火墙日志初始化 start*/
    FwLog_Init();
    /*add by z67625 防火墙日志初始化 end*/
    
#ifdef CONFIG_FEATURE_REMOTE_LOG
	/* If they have not specified remote logging, then log locally */
	if (doRemoteLog == FALSE)
		local_logging = TRUE;
#endif


	/* Store away localhost's name before the fork */
	gethostname(LocalHostName, sizeof(LocalHostName));
	if ((p = strchr(LocalHostName, '.'))) {
		*p = '\0';
	}

	umask(0);

	if (doFork == TRUE) {
#if defined(__uClinux__)
		vfork_daemon_rexec(0, 1, argc, argv, "-n");
#else /* __uClinux__ */
		if(daemon(0, 1) < 0)
			bb_perror_msg_and_die("daemon");
#endif /* __uClinux__ */
	}
	doSyslogd();

	return EXIT_SUCCESS;
}

/*
Local Variables
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
