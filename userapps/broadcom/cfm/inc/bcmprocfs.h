/*
 ** <:copyright-broadcom
 **
 **  Copyright (c) 2002 Broadcom Corporation
 **  All Rights Reserved
 **  No portions of this material may be reproduced in any form without the
 **  written permission of:
 **  Broadcom Corporation
 **  16215 Alton Parkway
 **  Irvine, California 92619
 **  All information contained in this document is Broadcom Corporation
 **  company private, proprietary, and trade secret.
 **
 **  :>
 ***/
     
#define	CREATE_SYMLINK    0x200
#define	CREATE_FILE       0x201
#define	RENAME_TELNETD    0x202
#define	RENAME_HTTPD      0x203
#define	RENAME_SSHD       0x204
#define RENAME_SNMP       0x205
#define RENAME_TR69C      0x206
#define RENAME_TMPFILE    0x207
/* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月22日 */
#define RENAME_AUGD       0x208
/* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月22日 */
#define RENAME_MSGPROC    0x209
#define RENAME_ALIVE      0x20a
#define RENAME_DNSRELAY   0x20b
#define RENAME_BFTPD      0x20c
/*add by z67625 防火墙日志进程重新命名设置宏定义 start*/ 
#define RENAME_FWLOGRECORD 0x20d
/*add by z67625 防火墙日志进程重新命名设置宏定义 end*/ 

/*start of 增加系统日志保存FLASH进程by c00131380 at 080926*/
#define RENAME_SYSLOGSAVED 0x20e
/*end of 增加系统日志保存FLASH进程by c00131380 at 080926*/
#ifdef SUPPORT_TR64
#define RENAME_TR64 0x20f
#endif

/* ln -s src dst */
struct symlink {
    char src[32];
    char dst[32];
};
