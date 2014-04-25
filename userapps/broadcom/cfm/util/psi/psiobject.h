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
//  Filename:       psiobject.h
//  Author:         Peter T. Tran
//  Creation Date:  26/12/01
//
******************************************************************************
//  Description:
//      Define the PsiObject class and functions.
//
*****************************************************************************/

#ifndef __PSIOBJECT_H__
#define __PSIOBJECT_H__

/********************** Include Files ***************************************/

#include "psidefs.h"
#include "psishm.h"

/********************** Class Declaration ***********************************/

class PsiObject : public PsiShm {
public:
   PsiObject():m_id(0),m_length(0),m_value(NULL) {};
   PsiObject(const UINT16 id, const PSI_VALUE value, const UINT16 len);
   ~PsiObject();
   UINT16     getId() { return m_id; }
   UINT16     getLength() { return m_length; }
   char       *getValue() { return m_value; }
   UINT16     toOctets(char* data, UINT32 pos);
   PSI_STATUS set(const PSI_VALUE value, const UINT16 len);
   PSI_STATUS get(PSI_VALUE value, const UINT16 len, UINT16* retLen);
   UINT16     toXml(FILE *file, char *appName);
private:
   UINT16     m_id;
   UINT16     m_length;
   char*      m_value;
};

#endif
