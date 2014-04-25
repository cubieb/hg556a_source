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
//  Filename:       sysMem.cpp
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      It provides operator new and delete in C++.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include "sysmem.h"

extern "C" void *malloc (size_t);
extern "C" void free (void *);

void * operator new (size_t sz) {
   return malloc(sz);
}

void * operator new[] (size_t sz) {
  return ::operator new(sz);
}

void operator delete (void *ptr) {
  if (ptr)
    free (ptr);
}

void operator delete[] (void *ptr) {
  ::operator delete (ptr);
}

extern "C" {
// overload function that is defined in ../gcc_3.0.1/libstdc++-v3/libsupc++/eh_personality.cc
void __gxx_personality_v0(void){};
void __Unwind_Resume(void){};
}
