/*****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
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
//  Filename:       bcmrip.h
//  Author:         Yen Tran
//  Creation Date:  10/21/02
//
******************************************************************************
//  Description:
//      RIP Configuration Data Structures
//
*****************************************************************************/

#ifndef __BCM_RIP_H__
#define __BCM_RIP_H__

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_RIP_INTERFACES      16
#define RIP_ENABLE               1
#define RIP_DISABLE              0
#define RIP_VERSION_1            1
#define RIP_VERSION_2            2
#define RIP_VERSION_1_2          3
#define RIP_OP_RX_ONLY           1
#define RIP_OP_TX_ONLY           2
#define RIP_OP_RX_TX             3
#define RIP_DISTRIBUTE_KERNEL    1
#define RIP_DISTRIBUTE_STATIC    2
#define RIP_DISTRIBUTE_CONNECTED 4
#define RIP_DEBUG_PACKET_RX      1
#define RIP_DEBUG_PACKET_TX      2
#define RIP_DEBUG_EVENTS         4
#define RIP_DEBUG_DISABLE        0
#define RIP_LOG_FILE             1
#define RIP_LOG_STDOUT           2
#define RIP_LOG_DISABLE          0
#define RIP_INTF_NAME_MAX_LEN    20
#define RIP_CMD_LEN              40
#define RIP_CFG_ERROR            -1
#define RIP_CFG_SUCCESS          0
#define RIP_INDEX_NOT_FOUND      -1

typedef struct rip_intf_cfg
{
  int mode;      
  int version;   
  int op;        
  char name[RIP_INTF_NAME_MAX_LEN];  
} RIP_INTF_CFG, *pRIP_INTF_CFG;

typedef struct rip_cfg
{
  int globalMode; 
  int logEnable;
  int debugFlag;
  int distributeFlag;
  int intfCount;
  pRIP_INTF_CFG pIntf[MAX_RIP_INTERFACES]; 
}RIP_CFG, *pRIP_CFG;

int bcmRipStart(pRIP_INTF_CFG pInputCfg, int start);

#if defined(__cplusplus)
}
#endif

#endif /* BCM_RIP_H_ */
