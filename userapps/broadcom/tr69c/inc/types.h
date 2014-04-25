/*----------------------------------------------------------------------*
<:copyright-broadcom 
 
 Copyright (c) 2005 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
 *----------------------------------------------------------------------*/

#ifndef _MAIN_TYPES_H_
#define _MAIN_TYPES_H_

#if 0
#define GC_DEBUG
#include "/usr/local/include/gc.h"
#if 1
#define malloc(n) GC_MALLOC(n)
#define calloc(m, n) GC_MALLOC((m)*(n))
#define free(p) GC_FREE(p)
#define realloc(p,n) GC_REALLOC((p),(n))
#endif
#define CHECK_LEAKS() GC_gcollect()
#else
#define CHECK_LEAKS() 
#endif


/*----------------------------------------------------------------------
 * types
 */

typedef unsigned int tIpAddr;

#define LOG_ERROR LOG_ERR

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#endif
