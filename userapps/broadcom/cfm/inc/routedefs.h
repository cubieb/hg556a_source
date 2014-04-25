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
//  Filename:       routedefs.h
//  Author:         Peter Tran
//  Creation Date:  12/17/02
//
******************************************************************************
//  Description:
//      Route Configuration Data Structures
//
*****************************************************************************/

#ifndef __ROUTE_DEFS_H__
#define __ROUTE_DEFS_H__

#include <netinet/in.h>
#include <ifcdefs.h>
#include "bcmcfm.h"

/********************** Global Types ****************************************/

typedef enum {
   ROUTE_STS_OK = 0,
   ROUTE_STS_ERR_GENERAL,
   ROUTE_STS_ERR_MEMORY,
   ROUTE_STS_ERR_INVALID_OBJECT,
   ROUTE_STS_ERR_OBJECT_NOT_FOUND,
   ROUTE_STS_ERR_OBJECT_EXIST,
   ROUTE_STS_ERR_OPEN_FILE
} ROUTE_STATUS;

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/********************** Global Constants ************************************/

#define ROUTE_APPID              "RouteCfg"
#define RCM_RIP_CFG_ID           1
#define RCM_RIP_IFC_TBL_ID       2
#define RCM_ROUTE_NUM_ID         3
#define RCM_ROUTE_CFG_TBL_ID     4
/* j00100803 Add Begin 2008-02-28 */
#ifdef SUPPORT_POLICY
#define POLICY_ROUTE_NUM_ID     5
#define POLICY_ROUTE_CFG_TBL_ID 6
#endif
/* j00100803 Add Begin 2008-02-28 */
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
#define RIP_IFC_NAME_MAX_LEN     16
#define RIP_CMD_LEN              64

/* j00100803 Add Begin 2008-02-23 */
#ifdef SUPPORT_POLICY
#define VODAFONE_ROUTEPOLICY_APPID "RoutePolicyCfg"
#define VODAFONE_ROUTEPOLICY_CFG_ID 1
#define POLICY_MAX_SIZE     8
/*begin add by p44980 2008.10.16*/
#define RT_MAX_DSCP 64
/*begin add by p44980 2008.10.16*/
#endif
/* j00100803 Add Begin 2008-02-23 */
// bit 0-1 bit : 2 bits,  mode:  0 = off, 1 = on
// bit 2-5     : 4 bits,  version: 1, 2 version and 3 = Both
// bit 5-6     : 2 bits,  operation: 0 = active, 1 = passive
// bit 7-15    : reserved
typedef struct {
   unsigned short
   mode:2,
   version:4,
   operation:2,
   reserved:8;
} RIP_FLAG;

typedef struct RipIfcCfg {
   RIP_FLAG flag;                      // rip flags 2 bytes defined above
   char name[RIP_IFC_NAME_MAX_LEN];    /* interface’s name: i.e. eth0 */
} RT_RIP_IFC_ENTRY, *PRT_RIP_IFC_ENTRY;

typedef struct RipCfg {
  int globalMode; 
  int logEnable;
  int debugFlag;
  int distributeFlag;
  int ifcCount;
} RT_RIP_CFG, *PRT_RIP_CFG;

typedef struct cliMenuRipCfg {
  RT_RIP_IFC_ENTRY ripCfg;
  char vpiVci[IFC_TINY_LEN];
} CLI_MENU_RIP_CFG, *pCLI_MENU_RIP_CFG;

typedef struct RouteCfg{
   uint32 id;
   struct in_addr ipAddress;
   struct in_addr subnetMask;
   struct in_addr gateway;
   char ifName[IFC_TINY_LEN];
   /*start of Enable or disable the forwarding entry by l129990 ,2009-11-5*/
   uint32 entryControl;
   /*end of Enable or disable the forwarding entry by l129990 ,2009-11-5*/
} RT_ROUTE_CFG_ENTRY, *PRT_ROUTE_CFG_ENTRY;


/* j00100803 Add End 2008-02-23 */
#ifdef SUPPORT_POLICY
#define POLICY_MAX_NUM 8
#define MAC_ADDR_MAX_LEN 6
typedef struct tagIFC_RoutePolicy_Info {
    BcmCfm_RoutingPolicyType   enType;  // the type of Policy of routing
    //BcmCfm_WanInterfaceType enWanIfcType;
    //BcmCfm_WanInterfaceMod enWanIfcMod;
    uint32 uId;
    uint32 uWanIndex;
    char   szSrcMacAddress[MAC_ADDR_MAX_LEN + 1];    // the source mac address
    char   szIPSrcAddress[IFC_SMALL_LEN];      // the source IP address
    char   szSrcIfcName[IFC_TINY_LEN];          // the source interface index 
    char   szWanIfcName[IFC_TINY_LEN];          // the wan interface name
    char   szProtocolType[IFC_TINY_LEN];       //the source protocol
    char   szIPDestAddress[IFC_SMALL_LEN];
    /* BEGIN: Added by y67514, 2009/2/16   PN:GLB WEB:DSCP FOR POLICYROUTE*/
    int8    nDscpValue;
    /* END:   Added by y67514, 2009/2/16 */
} IFC_RoutePolicy_Info, *PIFC_RoutePolicy_Info;
#endif
/* j00100803 Add Begin 2008-02-23 */

#if defined(__cplusplus)
}
#endif

#endif /* __RIP_DEFS_H_ */
