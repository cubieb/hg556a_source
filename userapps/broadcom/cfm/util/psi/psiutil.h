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
//  Filename:       psiutil.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the PsiUtil class and functions.
//
*****************************************************************************/

#ifndef __PSIUTIL_H__
#define __PSIUTIL_H__

/********************** Global Constants ************************************/

#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */

/********************** Class Declaration ***********************************/

class PsiUtil {
public:
   ~PsiUtil() {};
   static void   octprint(char* octets, int size);
   static UINT32 getCrc32(UINT8 *pData, UINT32 nBytes, UINT32 crc);
private:
   PsiUtil() {};
};

#endif
