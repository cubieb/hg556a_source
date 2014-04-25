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

#ifndef __PMAP_DEFS_H__
#define __PMAP_DEFS_H__

#include <netinet/in.h>
#include <ifcdefs.h>

/********************** Global Types ****************************************/

typedef enum {
   PORT_MAP_STS_OK = 0,
   PORT_MAP_STS_ERR_GENERAL,
   PORT_MAP_STS_ERR_MEMORY,
   PORT_MAP_STS_ERR_INVALID_OBJECT,
   PORT_MAP_STS_ERR_OBJECT_NOT_FOUND,
   PORT_MAP_STS_ERR_OBJECT_EXIST,
   PORT_MAP_STS_ERR_OPEN_FILE
} PORT_MAP_STATUS;

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/********************** Global Constants ************************************/
#define PMAP_APPID              "PMapCfg"
#define PMCM_PMAP_NUM_ID         1
#define PMCM_PMAP_CFG_TBL_ID     2
#define PMCM_PMAP_IFC_STS_ID     3

#define MAX_PORT_MAP_INTF 32
#define PORT_MAP_INTF_LIST_SIZE 273 /* 16 chars ifname * 16 ifs/vlan + 16 delim + '\0'*/
#define PORT_MAP_MAX_ENTRY 16

#define PMAP_IFC_STS_ENABLE     1   /* Interface is enabled for port mapping       */
#define PMAP_IFC_STS_DISABLE    0   /* Interface is disabled for port mapping      */
#define PMAP_CFG_ENTRY_NOTINUSE 0
#define PMAP_CFG_ENTRY_INUSE    1
#define PMAP_IFC_STS_ERROR      -1
#define DHCP_VENDOR_ID_LEN      64
#define DHCP_MAX_VENDOR_IDS     5
#define DHCPD_VENDORID_CONF_FILE "/var/udhcpd_vendorid.conf"

typedef struct PortMapDhcpVendorId {
   char vendorid[DHCP_VENDOR_ID_LEN];
} PORT_MAP_DHCP_VENDORID_ENTRY, *PPORT_MAP_VENDORID_CFG_ENTRY;

typedef struct PortMapCfg{
   char groupName[IFC_TINY_LEN];
   char ifName[IFC_TINY_LEN];
   char grpedIntf[PORT_MAP_INTF_LIST_SIZE];
   char dynIntf[PORT_MAP_INTF_LIST_SIZE];
   PORT_MAP_DHCP_VENDORID_ENTRY vid[DHCP_MAX_VENDOR_IDS];
   int  useFlag;
} PORT_MAP_CFG_ENTRY, *PPORT_MAP_CFG_ENTRY;

typedef struct PMapIfcStatus {
   char status;
   char ifName[IFC_TINY_LEN];
} PORT_MAP_IFC_STATUS_INFO, *PPORT_MAP_IFC_STATUS_INFO;

#if defined(__cplusplus)
}
#endif

#endif /* __PMAP_DEFS_H_ */
