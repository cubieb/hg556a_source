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
//  Filename:       psishm.h
//  Author:         Peter T. Tran
//  Creation Date:  01/13/05
//
******************************************************************************
//  Description:
//      Define the PsiShm class and functions.
//
*****************************************************************************/

#ifndef __PSISHM_H__
#define __PSISHM_H__

/********************** Include Files ***************************************/

#include <stdio.h>
#include "psidefs.h"

/********************** Class Declaration ***********************************/

class PsiShm {
public:
   PsiShm();
   virtual ~PsiShm();
   void* operator new(size_t);
   void operator delete(void*);
};

#endif
