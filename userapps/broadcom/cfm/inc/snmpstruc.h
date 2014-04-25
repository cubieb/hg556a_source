/*****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  Filename:       snmpstruc.h
//  Author:         seanl
//  Creation Date:  5/28/03
//
******************************************************************************
//  Description:
//      Define the snmp data struct to be saved in psi
//
*****************************************************************************/

#ifndef __SNMP_STRUC_H__
#define __SNMP_STRUC_H__
#include "ifcdefs.h"
#include "syscall.h"
/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include "snmpDebug.h"

#define BCM_SNMP_STATUS_ENABLE        1
#define BCM_SNMP_STATUS_DISABLE       0
#define SNMP_CONF_FILE                "/var/snmpd.conf"

#define BCM_SNMP_RO_COMMUNITY         1
#define BCM_SNMP_RW_COMMUNITY         2
#define BCM_SNMP_SYS_NAME             3
#define BCM_SNMP_SYS_LOCATION         4
#define BCM_SNMP_SYS_CONTACT          5

typedef struct snmpStruct {
   int status;
   char roCommunity[IFC_TINY_LEN];
   char rwCommunity[IFC_TINY_LEN];
   char sysName[IFC_STRING_LEN];
   char sysLocation[IFC_STRING_LEN];
   char sysContact[IFC_STRING_LEN];
   struct in_addr trapIp; 
   unsigned char configId[SNMP_CONFIG_ID_LEN];
   SNMP_DEBUG_FLAG debug;
} BCM_SNMP_STRUCT, *PBCM_SNMP_STRUCT;

void bcmSnmpStart(PBCM_SNMP_STRUCT pSnmpConfig);
void bcmSnmpStop(void);

#if defined(__cplusplus)
}
#endif

#endif /* __SNMP_STRUC_H__ */
