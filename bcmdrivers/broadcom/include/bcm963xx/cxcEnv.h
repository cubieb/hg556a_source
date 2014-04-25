/***************************************************************************
*    Copyright 2000  Broadcom Corporation
*    All Rights Reserved
*    No portions of this material may be reproduced in any form without the
*    written permission of:
*             Broadcom Corporation
*             16251 Laguna Canyon Road
*             Irvine, California  92618
*    All information contained in this document is Broadcom Corporation
*    company private, proprietary, and trade secret.
*
****************************************************************************
*
*    Filename: cxcEnv.h
*    Creation Date:
*    VSS Info:
*        $Revision: 1.1 $
*        $Date: 2008/08/25 06:41:10 $
*
****************************************************************************
*    Description:
*
*     This file is used in CableX to provide system-wide types and definitions. Howewever.
*     since iit tries to include various C run-time library dcefintions (eh stdio.h, it is not
*     appropriate for use "as is" in a Linux kernel environment. So the CableX endpoint source 
*     does not need to change, we use this file to recursively include bcmtypes.h
*
*    NOTE:  This file is based upon LDX's hausenv.h, but has been renamed to
*           avoid project dependencies, and allow for CablexChange customizations.
*
****************************************************************************/

#ifndef CXC_ENV_H
#define CXC_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
** Just include definitions from bcmtypes.h
*/
#include <bcmtypes.h>

#undef EPT_TEST

#if !defined( HAUS_SINT16_DEFINED )
#define HAUS_SINT16_DEFINED
typedef signed short    SINT16;     /* SINT16 is platform independant */
#endif


#if !defined( HAUS_SINT32_DEFINED )
#define HAUS_SINT32_DEFINED
typedef signed long   SINT32;
#endif

#define SUCCESS   0
#define FAILURE   1


// GNU definition imported from cxc_compiler.h
#define CXC_INLINE __inline__	  

// Endian-ness imported from cxc_cpu.h
#define CXC_CPU_BIG_ENDIAN     1
#define CXC_CPU_LITTLE_ENDIAN  0

#if defined(__KERNEL__)
#define CXC_OS_LINUX 1
#define PSOS 0
#define VXWORKS 0
#endif

#ifdef __cplusplus
}
#endif

#endif   /* CXC_ENV_H */
