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
//  Filename:       bcm96345util.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the bcm96345util class and functions.
//
*****************************************************************************/

#ifndef __BCM_96345_UTIL_H__
#define __BCM_96345_UTIL_H__

/********************** Include Files ***************************************/

// Includes.
#include "cliapi.h"
#include "psiapi.h"

#if defined(__cplusplus)
extern "C" {
#endif
void init(void);
void destroy(void);
void initDnldHttp( void );
void initSysDiag(void);
int BcmUtil_macStrToNum(char *macAddr, char *str); 
int BcmUtil_macNumToStr(char *macAddr, char *str);

void resetRunningConfig(void);

#if defined(__cplusplus)
}
#endif  // __cplusplus

/********************** Class Declaration ***********************************/

class Bcm96345Util {
public:
   ~Bcm96345Util() {};
   static void createTopLevelMenu(void);
   static void saveProfile(void);
   static void reset(void);
   static void confirmReboot(char *msg);
   static void printBroadcomLogo(void);
   static void subnetMaskHexToDec(char *in, char *out);
   static void subnetMaskDecToHex(char *in, char *out);
   static void getSoftwareVersion (char *swVersion, UINT16 size);
   static CLI_STATUS rebootSystem( char *cmdLine, int mode, void *pParm );
   static CLI_STATUS dumpAddr( char *cmdName, int help, void *pParm );
   static CLI_STATUS setMem( char *cmdName, int help, void *pParm );
   static CLI_STATUS dumpAtmRegs( char *cmdName, int help, void *pParm );
   static CLI_STATUS displayFreePages( char *cmdName, int help, void *pParm );
   static CLI_STATUS displayAtmStats( char *cmdName, int help, void *pParm );
   static CLI_STATUS ipStrToNum(char *ipAddr, char *str);
   static CLI_STATUS ipNumToStr(char *ipAddr, char *str);
   static CLI_STATUS macStrToNum(char *macAddr, char *str);
   static CLI_STATUS macNumToStr(char *macAddr, char *str);
private:
   Bcm96345Util() {};
};


#endif

