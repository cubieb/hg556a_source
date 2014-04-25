/*****************************************************************************/
/*                Copyright 2004 - 2004, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/* FileName: vtoptypes.h                                                     */
/* Version: 01a                                                              */
/* Description:  										                     */
/*                                                                           */
/* History:                                                                  */
/* 1. 01a,2004-08-31,y19511 Create this file.                                */
/*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifndef __VTOP_TYPES_H__
#define __VTOP_TYPES_H__

/*平台类型*/
#define VTOP_OS_WIN32 1
#define VTOP_OS_LINUX  2
#define VTOP_OS_TYPE VTOP_OS_LINUX

#define VTOP_LIBC_GLIBC 1
#define VTOP_LIBC_UCLIBC 2
/*define VTOP_LIBC_TYPE VTOP_LIBC_GLIBC*/

typedef const char				VOS_CONST_CHAR;
#define VOS_NOK                 1L
#define VTOP_OK                 0L
#define VTOP_NOK                1L


/* if vos.h is not included */
#ifndef  _TYPDEF_H

/************************* MACRO & TYPE DEFINITION ***************************/
#define VOS_NULL              (0)
#define VOS_NULL_PTR           (0L)

#define VOS_OK                 (0)
#define VOS_ERROR              -1

#define VOS_TRUE               1L 
#define VOS_FALSE              0L


typedef void                    VOS_VOID;

typedef unsigned char           VOS_UINT8;
typedef char             VOS_INT8;
typedef char                    VOS_CHAR;

typedef unsigned short          VOS_UINT16;
typedef signed short            VOS_INT16;

typedef unsigned int           VOS_UINT32;
typedef signed int             VOS_INT32;

typedef unsigned int            VOS_UINT;
typedef int                     VOS_INT;

typedef unsigned int           VOS_BOOL;

typedef float                   VOS_FLOAT;
typedef double                  VOS_DOUBLE;

typedef unsigned int           VOS_SOCKET;

#if VTOP_OS_TYPE == VTOP_OS_LINUX
typedef unsigned long long           VOS_UINT64;
typedef long long             VOS_INT64;
#endif
#endif /* _V_TYPDEF_H */

#define    VTOP_LITTLE_ENDIAN        1234 /* least-significant byte first (vax) */
#define    VTOP_BIG_ENDIAN           4321 /* most-significant byte first (IBM, net) */

#ifndef VTOP_BYTE_ORDER
#define    VTOP_BYTE_ORDER          VTOP_BIG_ENDIAN  
#endif

#ifdef VTOP_BYTE_ORDER
#if	VTOP_BYTE_ORDER==VTOP_BIG_ENDIAN
#define	VTOP_NTOHL(x)	(x)
#define	VTOP_HTONL(x)	(x)
#define	VTOP_NTOHS(x)	(x)
#define	VTOP_HTONS(x)	(x)
#else
#define VTOP_NTOHL(x)	((((x) & 0x000000ff) << 24) | \
			 (((x) & 0x0000ff00) <<  8) | \
			 (((x) & 0x00ff0000) >>  8) | \
			 (((x) & 0xff000000) >> 24))

#define VTOP_HTONL(x)	((((x) & 0x000000ff) << 24) | \
			 (((x) & 0x0000ff00) <<  8) | \
			 (((x) & 0x00ff0000) >>  8) | \
			 (((x) & 0xff000000) >> 24))

#define VTOP_NTOHS(x)	((((x) & 0x00ff) << 8) | \
			 (((x) & 0xff00) >> 8))

#define VTOP_HTONS(x)	((((x) & 0x00ff) << 8) | \
			 (((x) & 0xff00) >> 8))

#endif	/* VTOP_BYTE_ORDER==VTOP_BIG_ENDIAN */

#else
#error "Byte order macro (VTOP_BYTE_ORDER) not defined!"
#endif  /* VTOP_BYTE_ORDER */


#endif /* __VTOP_TYPES_H__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
