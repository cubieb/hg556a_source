/*****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  Filename:       sysdiag.h
//  Author:         Yen Tran
//  Creation Date:  2/26/02
//
******************************************************************************
//  Description:
//      Define the global constants, and functions for sysDiag utility.
//
*****************************************************************************/

#ifndef __SYS_DIAG_H__
#define __SYS_DIAG_H__

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include "bcmatmapi.h"

int bcmDiagOamSegmentTest(PATM_VCC_ADDR pVccAddr);
int bcmDiagOamLoopbackTest(int type, PATM_VCC_ADDR pVccAddr);
int bcmDiagPppOeTest(int type,char *name);
int bcmDiagDoPing(int type);

int bcmDiagTest(int type, char *device, PATM_VCC_ADDR pVccAddr);

int bcmGetIfDestAddr(char *devName, char* ipAddr);
int bcmGetIfMask(char *devName, char* ipAddr);
int bcmGetIfAddr(char *devName, char* ipAddr);
int bcmGetIfSubnet(char *devName, char* ipSubnet);
#define BCM_DIAG_ETHERNET_CONN      1
#define BCM_DIAG_ADSL_CONN          2
#define BCM_DIAG_OAM_LOOPBACK_SEG   3
#define BCM_DIAG_OAM_LOOPBACK_END   4
#define BCM_DIAG_PPPOE_CONN         5
#define BCM_DIAG_PPPOE_SESSION      6
#define BCM_DIAG_PPPOE_AUTH         7
#define BCM_DIAG_PPPOE_IP_ADDR      8
#define BCM_DIAG_PING_DEF_GW        9
#define BCM_DIAG_PING_DNS           10
#define BCM_DIAG_PING_BRCM          11
#define BCM_DIAG_ADSL_BERT          12
#define BCM_DIAG_USB_CONN           13
#define BCM_DIAG_TEST_ALL           14
#define BCM_DIAG_OAM_F4_LOOPBACK_SEG   15
#define BCM_DIAG_OAM_F4_LOOPBACK_END   16
#define BCM_DIAG_WIRELESS_CONN      17
#define BCM_DIAG_ETHERNET_CONN_2    18

#define BCM_DIAG_OAM_SEGMENT        0
#define BCM_DIAG_OAM_END2END        1
#define BCM_DIAG_OAM_F4_SEGMENT     2
#define BCM_DIAG_OAM_F4_END2END     3
#define BCM_DIAG_FAIL               0
#define BCM_DIAG_PASS               1
#define BCM_DIAG_NOT_TEST           2
#define BCM_DIAG_IN_PROGRESS        3
#define BCM_DIAG_BERT_TIME          20       /* seconds */
#define BCM_DIAG_BERT_TIME_MIN      1        /* seconds */
#define BCM_DIAG_BERT_TIME_MAX      400     /* seconds */

/* bit definition for test status */
#define BCM_DIAG_STATUS_DEF_PASS    0
#define BCM_DIAG_ETHER_CONN_FAIL    1
#define BCM_DIAG_USB_CONN_FAIL      2
#define BCM_DIAG_ADSL_CONN_FAIL     4
#define BCM_DIAG_OAM_LOOPBACK_FAIL  8
#define BCM_DIAG_PPPOE_FAIL         10
#define BCM_DIAG_PING_DEF_GW_FAIL   20
#define BCM_DIAG_PING_DNS_FAIL      40
#define BCM_DIAG_STATUS_LAN_PASS    0
#define BCM_DIAG_STATUS_LAN_FAIL    1
#define BCM_DIAG_OTHER_TESTS_FAIL_MASK   0xfffffffc

typedef enum _bcmDiagSeg {
   BCM_DIAG_SEG_PASS = 0,
   BCM_DIAG_SEG_FAIL,
   BCM_DIAG_F4_SEG_FAIL,
   BCM_DIAG_F5_SEG_FAIL
} BCM_DIAG_SEG;

typedef enum _bcmDiagEnd {
   BCM_DIAG_END_PASS = 0,
   BCM_DIAG_END_FAIL,
   BCM_DIAG_F4_END_FAIL,
   BCM_DIAG_F5_END_FAIL
} BCM_DIAG_END;

#if defined(__cplusplus)
}
#endif

#endif /* __SYS_DIAG_H__ */
