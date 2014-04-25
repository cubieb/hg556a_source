/*****************************************************************************/
/*                                                                           */
/*                Copyright 2004 - 2050, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileNmae: os_def.h                                                    */
/* Version : 1.0                                                             */
/*                                                                           */
/* Description:  公共数据类型定义                                                            */
/*                                                                           */
/* History:                                                                  */
/*****************************************************************************/

#include "os_types_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifndef __OS_DEF_INTERFACE_H__
#define __OS_DEF_INTERFACE_H__


#define VTOP_OS_TYPE VTOP_OS_LINUX
/*#define VTOP_OSADP_DEBUG_VERSION */
/*公共类型定义*/

typedef VOS_INT32   VTOP_TIME_T;
typedef VOS_UINT32 VTOP_SIZE_T;
typedef VOS_UINT32 VTOP_MODE_T;

typedef VOS_INT32   VTOP_SSIZE_T;
typedef VOS_INT       VTOP_OFF_T;

typedef VOS_UINT32 VTOP_UID_T;
typedef VOS_UINT32 VTOP_GID_T;

typedef VOS_UINT32 VTOP_MSGQNUM_T;
typedef VOS_UINT32 VTOP_MSGLEN_T;

typedef VOS_INT      VTOP_IntPtr_T;
typedef VOS_UINT32 VTOP_SockLen_T;
typedef VOS_INT32 VTOP_PID_T;

typedef VOS_UINT32 VTOP_InAddr_T;
typedef VOS_UINT32 VTOP_Pthread_T;

/*******************************************************
|														|
|      各模块公共宏定义                                              |
|														|
*******************************************************/
#define  VTOP_INADDR_ANY     ((VTOP_InAddr_T) 0x00000000)
#define VTOP_INADDR_NONE     0xFFFFFFFFUL 
#define VTOP_INVALID_SOCKET 0xFFFFFFFFUL
#define VTOP_NULL_LONG 0xFFFFFFFFUL
/* Infinite timeout*/
#define VTOP_INFINITE            0xFFFFFFFF  
#define VTOP_SEM_VALUE_MAX 0xFFFF

#ifdef VTOP_OSADP_DEBUG_VERSION

#define VTOP_RETURN_VALUE_IFTRUE(expr, val)	\
     if (expr) { return (val);}

#define VTOP_RETURN_NULL_IFTRUE(expr)	\
     if (expr) {  return (VOS_NULL);}
     
#define  VTOP_RETURN_IFTRUE(expr)\
    if (expr) {  return ;}

#define VTOP_RETURN_VALUE_IFNULL(expr, val)	\
     if (VOS_NULL_PTR==(expr)) {  return (val);}

#define VTOP_RETURN_NULL_IFNULL(expr)	\
     if (VOS_NULL_PTR==(expr)){ return (VOS_NULL);}
     
#define VTOP_RETURN_NULLPTR_IFNULL(expr)	\
     if (VOS_NULL_PTR==(expr)){ return ((VOS_VOID *)VOS_NULL_PTR);}
     
#define  VTOP_RETURN_IFNULL(expr)\
    if (VOS_NULL_PTR==(expr)) { return ;}

#define VTOP_RETURN_VALUE_IFNULLSTR(expr, val)	\
     if (0 ==  VTOP_StrLen(expr)) {  return (val);}
     
/*VTOP_ASSERT*/
#define  VTOP_ASSERT(expr)\
    if (expr) { } else	{ return ;}

#define VTOP_ASSERT_RETURN_NOK(expr) \
    if (expr) { } else { return VTOP_NOK;}

#else
#define VTOP_RETURN_VALUE_IFTRUE(expr, val)	

#define VTOP_RETURN_NULL_IFTRUE(expr)	
     
#define  VTOP_RETURN_IFTRUE(expr)

#define VTOP_RETURN_VALUE_IFNULL(expr, val)	

#define VTOP_RETURN_NULL_IFNULL(expr)
     
#define VTOP_RETURN_NULLPTR_IFNULL(expr)	
     
#define  VTOP_RETURN_IFNULL(expr)

#define VTOP_RETURN_VALUE_IFNULLSTR(expr, val)	
     
#define  VTOP_ASSERT(expr)

#define VTOP_ASSERT_RETURN_NOK(expr) 

#endif
/*errno*/

/*
 * digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 */
#define VTOP_ISDIGIT(x) (((x) >= '0') && ((x) <= '9'))

/*文件名最大长度*/
#define VTOP_MAX_FILENAME_LENGTH  255



/********************************************/
/*     打开文件权限及模式定义  */
/*     同样适用与共享内存操作  */
/********************************************/
/* Open shared memory segment.  */
#define VTOP_O_RDONLY  00
#define VTOP_O_WRONLY	     01
#define VTOP_O_RDWR     02
#define VTOP_O_CREAT   0100    /* not fcntl */

#if VTOP_OS_TYPE == VTOP_OS_LINUX
#define VTOP_O_EXCL      0200    /* not fcntl */
#define VTOP_O_TRUNC    01000 /* not fcntl */
#define VTOP_O_APPEND          02000

#define VTOP_GENERIC_READ                 VTOP_O_RDONLY
#define VTOP_GENERIC_WRITE               VTOP_O_WRONLY
#define VTOP_GENERIC_EXECUTE           VTOP_O_EXCL
#define VTOP_GENERIC_ALL                    VTOP_O_RDWR

/*!!! 以下取值不具有可移植性,只在linux下有意义*/

#define VTOP_O_NOCTTY           0400 
#define VTOP_O_ACCMODE          0003
#define VTOP_O_NONBLOCK        04000
#define VTOP_O_NDELAY        VTOP_O_NONBLOCK
#define VTOP_O_SYNC           010000
#define VTOP_O_FSYNC          VTOP_O_SYNC
#define VTOP_O_ASYNC          020000


#elif VTOP_OS_TYPE == VTOP_OS_WIN32

#define VTOP_O_APPEND       0x0008  /* writes done at eof */
#define VTOP_O_TRUNC        0x0200  /* open and truncate */
#define VTOP_O_EXCL         0x0400  /* open only if file doesn't already exist */

/*for VTOP_ShmOpen 's oflag argument ,use this value, can trans the program to windows freely*/
#define VTOP_GENERIC_READ                     (0x80000000L)
#define VTOP_GENERIC_WRITE                    (0x40000000L)
#define VTOP_GENERIC_EXECUTE                  (0x20000000L)
#define VTOP_GENERIC_ALL                      (0x10000000L)

#define VTOP_O_TEXT         0x4000  /* file mode is text (translated) */
#define VTOP_O_BINARY       0x8000  /* file mode is binary (untranslated) */

/*!!! 以下取值不具有可移植性,只在windows下有意义*/
/* macro to translate the C 2.0 name used to force binary mode for files */
#define VTOP_O_RAW  VTOP_O_BINARY

/* Open handle inherit bit */

#define VTOP_O_NOINHERIT    0x0080  /* child process doesn't inherit file */

/* Temporary file bit - file is deleted when last handle is closed */

#define VTOP_O_TEMPORARY    0x0040  /* temporary file bit */

/* temporary access hint */

#define VTOP_O_SHORT_LIVED  0x1000  /* temporary storage file, try not to flush */

/* sequential/random access hints */

#define VTOP_O_SEQUENTIAL   0x0020  /* file access is primarily sequential */
#define VTOP_O_RANDOM       0x0010  /* file access is primarily random */

#else
#error YOU MUST DEFINE VTOP OS TYPE VTOP_OS_TYPE == VTOP_OS_WIN32 OR VTOP_OS_LINUX !  
#endif  

/*mode ,for open file or other*/
#define VTOP_S_IREAD        0000400         /* read permission, owner */
#define VTOP_S_IWRITE       0000200         /* write permission, owner */
#define VTOP_S_IEXEC        0000100         /* execute/search permission, owner */
#define	VTOP_S_IRWXU	(VTOP_S_IREAD|VTOP_S_IWRITE|VTOP_S_IEXEC)

#if VTOP_OS_TYPE == VTOP_OS_LINUX
/* Priority limits.  */
#define VTOP_PRIO_MIN        (-20)     /* Minimum priority a process can have.  */
#define VTOP_PRIO_MAX        20      /* Maximum priority a process can have.  */

#define	VTOP__S_ISUID	04000	/* Set user ID on execution.  */
#define	VTOP__S_ISGID	02000	/* Set group ID on execution.  */
#define	VTOP__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	VTOP__S_IREAD	0400	/* Read by owner.  */
#define	VTOP__S_IWRITE	0200	/* Write by owner.  */
#define	VTOP__S_IEXEC	0100	/* Execute by owner.  */


#define	VTOP_S_ISUID  VTOP__S_ISUID	/* Set user ID on execution.  */
#define	VTOP_S_ISGID	VTOP__S_ISGID	/* Set group ID on execution.  */
#define   VTOP_S_ISVTX  VTOP__S_ISVTX


#define	VTOP_S_IRUSR	 VTOP__S_IREAD	/* Read by owner.  */
#define	VTOP_S_IWUSR	VTOP__S_IWRITE	/* Write by owner.  */
#define	VTOP_S_IXUSR  VTOP__S_IEXEC	/* Execute by owner.  */
/* Read, write, and execute by owner.  */

#define	VTOP_S_IRGRP	(VTOP_S_IRUSR >> 3)	/* Read by group.  */
#define	VTOP_S_IWGRP	(VTOP_S_IWUSR >> 3)	/* Write by group.  */
#define	VTOP_S_IXGRP	(VTOP_S_IXUSR >> 3)	/* Execute by group.  */
/* Read, write, and execute by group.  */
#define	VTOP_S_IRWXG	(VTOP_S_IRWXU >> 3)

#define	VTOP_S_IROTH	(VTOP_S_IRGRP >> 3)	/* Read by others.  */
#define	VTOP_S_IWOTH	(VTOP_S_IWGRP >> 3)	/* Write by others.  */
#define	VTOP_S_IXOTH	(VTOP_S_IXGRP >> 3)	/* Execute by others.  */
/* Read, write, and execute by others.  */
#define	VTOP_S_IRWXO	(VTOP_S_IRWXG >> 3)

#elif VTOP_OS_TYPE == VTOP_OS_WIN32
#define VTOP_FILE_SHARE_READ                 0x00000001  
#define VTOP_FILE_SHARE_WRITE                0x00000002  
#define VTOP_FILE_SHARE_DELETE               0x00000004  

#define	VTOP_S_IRUSR	 VTOP_FILE_SHARE_READ	/* Read by owner.  */
#define	VTOP_S_IWUSR	VTOP_FILE_SHARE_WRITE	/* Write by owner.  */
#define	VTOP_S_IXUSR  VTOP_FILE_SHARE_DELETE	/* Execute by owner.  */
/* Read, write, and execute by owner.  */

#define	VTOP_S_IRGRP	VTOP_FILE_SHARE_READ	/* Read by group.  */
#define	VTOP_S_IWGRP	VTOP_FILE_SHARE_WRITE	/* Write by group.  */
#define	VTOP_S_IXGRP	VTOP_FILE_SHARE_DELETE	/* Execute by group.  */
/* Read, write, and execute by group.  */

#define	VTOP_S_IROTH	VTOP_FILE_SHARE_READ	/* Read by others.  */
#define	VTOP_S_IWOTH	VTOP_FILE_SHARE_WRITE	/* Write by others.  */
#define	VTOP_S_IXOTH	VTOP_FILE_SHARE_DELETE	/* Execute by others.  */
#if 0
#define VTOP_S_IFMT         0170000         /* file type mask */
#define VTOP_S_IFDIR        0040000         /* directory */
#define VTOP_S_IFCHR        0020000         /* character special */
#define VTOP_S_IFIFO        0010000         /* pipe */
#define VTOP_S_IFREG        0100000         /* regular */
#endif
#else
#error YOU MUST DEFINE VTOP OS TYPE WIN32 OR LINUX ! 
#endif

/*///////////////////////  MMap port define /////////////////////////////////////*/

#if VTOP_OS_TYPE == VTOP_OS_LINUX

#define  VTOP_PROT_READ     0x1               /* Page can be read.  */
#define  VTOP_PROT_WRITE   0x2               /* Page can be written.  */
#define  VTOP_PROT_EXEC     0x4     /* Page can be executed.  */
#define VTOP_PROT_NONE       0x0             /* Page can not be accessed.  */

#elif VTOP_OS_TYPE == VTOP_OS_WIN32
#define VTOP_PAGE_NOACCESS          0x01     
#define VTOP_PAGE_READONLY          0x02     
#define VTOP_PAGE_READWRITE         0x04     
#define VTOP_PAGE_EXECUTE           0x10     
/*
#define VTOP_PAGE_EXECUTE_READ      0x20     
#define VTOP_PAGE_EXECUTE_READWRITE 0x40     
#define VTOP_PAGE_EXECUTE_WRITECOPY 0x80     
#define VTOP_PAGE_GUARD            0x100     
#define VTOP_PAGE_NOCACHE          0x200     
#define VTOP_PAGE_WRITECOMBINE     0x400    
*/
#define  VTOP_PROT_READ     VTOP_PAGE_READONLY               /* Page can be read.  */
#define  VTOP_PROT_WRITE   VTOP_PAGE_READWRITE               /* Page can be written.  */
#define  VTOP_PROT_EXEC     VTOP_PAGE_EXECUTE     /* Page can be executed.  */
#define VTOP_PROT_NONE  VTOP_PAGE_NOACCESS
#else
#error YOU MUST DEFINE VTOP OS TYPE VTOP_OS_TYPE == VTOP_OS_WIN32 OR VTOP_OS_LINUX !  
#endif  





#endif /* __OS_DEF_INTERFACE_H__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

