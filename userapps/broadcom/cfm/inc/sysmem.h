/*****************************************************************************
//
//  Copyright (c) 2000-2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       sysmem.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      It provides declaration for operator new and delete in C++.
//
*****************************************************************************/

#ifndef __SYS_MEM__
#define __SYS_MEM__

//#include <cstddef>
#include <stddef.h>

extern "C++" {

void *operator new(size_t);
void *operator new[](size_t);
void operator delete(void *);
void operator delete[](void *);

// Default placement versions of operator new.
inline void *operator new(size_t, void *place) { return place; }
inline void *operator new[](size_t, void *place) { return place; }
//@}
} // extern "C++"

#endif
