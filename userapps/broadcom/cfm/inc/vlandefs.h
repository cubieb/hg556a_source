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
//  Filename:       pmapdefs.h
//  Author:         Pavan Kumar
//  Creation Date:  09/03/04
//
******************************************************************************
//  Description:
//      Port Mapping Configuration Data Structures
//
*****************************************************************************/

#ifndef __VLAN_DEFS_H__
#define __VLAN_DEFS_H__

#include <netinet/in.h>
#include <ifcdefs.h>

/********************** Global Types ****************************************/

typedef enum {
   VLAN_STS_OK = 0,
   VLAN_STS_ERR_GENERAL,
   VLAN_STS_ERR_MEMORY,
   VLAN_STS_ERR_INVALID_OBJECT,
   VLAN_STS_ERR_OBJECT_NOT_FOUND,
   VLAN_STS_ERR_OBJECT_EXIST,
   VLAN_STS_ERR_OPEN_FILE
} VLAN_STATUS;

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/********************** Global Constants ************************************/
#define DEBUG_VBR 0

#define VLAN_APPID              "VlanCfg"
#define VLCM_VLAN_CFG_ID         1
#define VLCM_VLAN_NUM_ID         2
#define VLCM_VLAN_CFG_TBL_ID     3
#define VLCM_PORT_NUM_ID         4
#define VLCM_PORT_CFG_TBL_ID     5

#define VLAN_ENTRY_NOTINUSE 0
#define VLAN_ENTRY_INUSE 1
#define VLAN_8021Q_PORT_MAX 16
#define VLAN_8021Q_NUM_MAX  16
#define VLAN_INTF_LIST_SIZE 273 /* 16 chars ifname * 16 ifs/vlan + 16 delim + '\0'*/

#define VLAN_ISVALID_PRIO(prio) (((prio) >= 0) && ((prio) <= 7))
#define VLAN_GMRP_MAC_STR             "01:80:C2:00:00:20"
#define VLAN_GVRP_MAC_STR             "01:80:C2:00:00:21"
#define VLAN_BPDU_MAC_STR             "01:80:c2:00:00:00"

enum 
{
    BR_PORT_MODE_NORMAL = 0,
    BR_PORT_MODE_ACCESS,
    BR_PORT_MODE_TRUNK,
    BR_PORT_MODE_HYBRID,
    /*more add here*/
    BR_PORT_MODE_UNKNOWN
};//refer to br_private.h

enum
{
    VLAN_SKB_DISCARD = 0,
    VLAN_SKB_FORWARD
};

typedef struct Vlan8021qPort{
    char name[IFC_TINY_LEN];
    unsigned short mode;
    unsigned short pvid;
    unsigned short prio;
    unsigned short useFlag;    
}VLAN_8021Q_PORT_ENTRY;

typedef struct Vlan8021qItem{
    int            no;
    unsigned short vlanid;
    unsigned char  macln;
    unsigned char  useFlag;
    char  grpIf[IFC_TINY_LEN];
    char  ifList[VLAN_INTF_LIST_SIZE];
}VLAN_8021Q_ITEM_ENTRY;

typedef struct Vlan8021qCfg{
    unsigned short mngrid;
    unsigned char  garpAct;
    unsigned char  bpduAct;
    unsigned char  enabled;
    unsigned char  reserved1;
    unsigned short  reserved2;
    char garpFDIF[IFC_TINY_LEN];
    char bpduFDIF[IFC_TINY_LEN];
    VLAN_8021Q_ITEM_ENTRY *vlanItem;
}VLAN_8021Q_CFG_ENTRY;

#if defined(__cplusplus)
}
#endif

#endif /* __PMAP_DEFS_H_ */
