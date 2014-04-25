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
//  Filename:       psishm.cpp
//  Author:         Peter T. Tran
//  Creation Date:  01/13/05
//
******************************************************************************
//  Description:
//      Implement the PsiShm class and functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include "psishm.h"
#include "bget.h"
#include "stdlib.h"

/********************** Class Method Implementations ************************/

/***************************************************************************
// Function Name: PsiShm constructor.
// Description  : create default PsiShm.
// Returns      : n/a.
****************************************************************************/
PsiShm::PsiShm() {
}

/***************************************************************************
// Function Name: PsiShm destructor.
// Description  : free memory of object.
// Returns      : n/a.
****************************************************************************/
PsiShm::~PsiShm() {
}

/***************************************************************************
// Function Name: PsiShm new operator.
// Description  : new operator overloading for creating shared memory.
// Returns      : n/a.
****************************************************************************/
void* PsiShm::operator new(size_t size) {
   return bget(size);
}

/***************************************************************************
// Function Name: PsiShm delete operator.
// Description  : delete operator overloading for free shared memory.
// Returns      : n/a.
****************************************************************************/
void PsiShm::operator delete(void* obj) {
   brel(obj);
}
