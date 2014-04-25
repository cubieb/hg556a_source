/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/***************************************************************************
 * File Name  : AtmApiDrv.h
 *
 * Description: This file contains the definitions and structures for the
 *              Linux IOCTL interface that used between the user mode ATM
 *              API library and the kernel ATM API driver.
 *
 * Updates    : 09/15/2000  lat.  Created.
 ***************************************************************************/

#if !defined(_ATMAPIDRV_H_)
#define _ATMAPIDRV_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Incldes. */
#include <bcmatmapi.h>

/* Defines. */
#define ATMDRV_MAJOR            205 /* arbitrary unused value */

#define ATMIOCTL_INITIALIZE \
    _IOWR(ATMDRV_MAJOR, 0, ATMDRV_INITIALIZE)
#define ATMIOCTL_UNINITIALIZE \
    _IOR(ATMDRV_MAJOR, 1, ATMDRV_STATUS_ONLY)
#define ATMIOCTL_GET_INTERFACE_ID \
    _IOWR(ATMDRV_MAJOR, 2, ATMDRV_INTERFACE_ID)
#define ATMIOCTL_GET_TRAFFIC_DESCR_TABLE_SIZE \
    _IOR(ATMDRV_MAJOR, 3, ATMDRV_TRAFFIC_DESCR_TABLE_SIZE)
#define ATMIOCTL_GET_TRAFFIC_DESCR_TABLE \
    _IOWR(ATMDRV_MAJOR, 4, ATMDRV_TRAFFIC_DESCR_TABLE)
#define ATMIOCTL_SET_TRAFFIC_DESCR_TABLE \
    _IOWR(ATMDRV_MAJOR, 5, ATMDRV_TRAFFIC_DESCR_TABLE)
#define ATMIOCTL_GET_INTERFACE_CFG \
    _IOWR(ATMDRV_MAJOR, 6, ATMDRV_INTERFACE_CFG)
#define ATMIOCTL_SET_INTERFACE_CFG \
    _IOWR(ATMDRV_MAJOR, 7, ATMDRV_INTERFACE_CFG)
#define ATMIOCTL_GET_VCC_CFG \
    _IOWR(ATMDRV_MAJOR, 8, ATMDRV_VCC_CFG)
#define ATMIOCTL_SET_VCC_CFG \
    _IOWR(ATMDRV_MAJOR, 9, ATMDRV_VCC_CFG)
#define ATMIOCTL_GET_VCC_ADDRS \
    _IOWR(ATMDRV_MAJOR, 10, ATMDRV_VCC_ADDRS)
#define ATMIOCTL_GET_INTERFACE_STATISTICS \
    _IOWR(ATMDRV_MAJOR, 11, ATMDRV_INTERFACE_STATISTICS)
#define ATMIOCTL_GET_VCC_STATISTICS \
    _IOWR(ATMDRV_MAJOR, 12, ATMDRV_VCC_STATISTICS)
#define ATMIOCTL_SET_INTERFACE_LINK_INFO \
    _IOWR(ATMDRV_MAJOR, 13, ATMDRV_INTERFACE_LINK_INFO)
#define ATMIOCTL_TEST \
    _IOWR(ATMDRV_MAJOR, 14, ATMDRV_TEST)
#define ATMIOCTL_OAM_LOOPBACK_TEST \
    _IOWR(ATMDRV_MAJOR, 15, ATMDRV_OAM_LOOPBACK)
#define ATMIOCTL_PORT_MIRRORING   \
    _IOWR(ATMDRV_MAJOR, 16, MirrorCfg)
#define ATMIOCTL_GET_PRIORITY_PACKET_GROUP \
    _IOWR(ATMDRV_MAJOR, 17, ATMDRV_PRIORITY_PACKET_GROUP)
#define ATMIOCTL_SET_PRIORITY_PACKET_GROUP \
    _IOWR(ATMDRV_MAJOR, 18, ATMDRV_PRIORITY_PACKET_GROUP)
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#define ATMIOCTL_SET_TRAFFIC_TYPE \
    _IOWR(ATMDRV_MAJOR, 19, ATMDRV_TRAFFIC_TYPE)
#define ATMIOCTL_GET_TRAFFIC_TYPE \
    _IOWR(ATMDRV_MAJOR, 20, ATMDRV_TRAFFIC_TYPE)
#define ATMIOCTL_SET_CONFIG_MODE \
    _IOWR(ATMDRV_MAJOR, 21, ATMDRV_TRAFFIC_TYPE)
#define ATMIOCTL_SET_QOSWEIGHT_CFG\
    _IOWR(ATMDRV_MAJOR, 22,  ATMDRV_QOSWEIGHT_TYPE)

#define MAX_ATMDRV_IOCTL_COMMANDS   23
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* Typedefs. */
typedef struct
{
    BCMATM_STATUS baStatus;
} ATMDRV_STATUS_ONLY, *PATMDRV_STATUS_ONLY;

typedef struct
{   PATM_INITIALIZATION_PARMS pInit;
    BCMATM_STATUS baStatus;
} ATMDRV_INITIALIZE, *PATMDRV_INITIALIZE;

typedef struct
{
    UINT8 ucPhyPort;
    UINT8 ucReserved[3];
    UINT32 ulInterfaceId;
    BCMATM_STATUS baStatus;
} ATMDRV_INTERFACE_ID, *PATMDRV_INTERFACE_ID;

typedef struct
{
    UINT32 ulTrafficDescrTableSize;
    BCMATM_STATUS baStatus;
} ATMDRV_TRAFFIC_DESCR_TABLE_SIZE, *PATMDRV_TRAFFIC_DESCR_TABLE_SIZE;

typedef struct
{
    UINT32 ulGroupNumber;
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets;
    UINT32 ulPriorityPacketsSize;
    BCMATM_STATUS baStatus;
} ATMDRV_PRIORITY_PACKET_GROUP, *PATMDRV_PRIORITY_PACKET_GROUP;

typedef struct
{
    PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable;
    UINT32 ulTrafficDescrTableSize;
    BCMATM_STATUS baStatus;
} ATMDRV_TRAFFIC_DESCR_TABLE, *PATMDRV_TRAFFIC_DESCR_TABLE;

typedef struct
{
    UINT32 ulInterfaceId;
    PATM_INTERFACE_CFG pInterfaceCfg;
    BCMATM_STATUS baStatus;
} ATMDRV_INTERFACE_CFG, *PATMDRV_INTERFACE_CFG;

typedef struct
{
    ATM_VCC_ADDR VccAddr;
    PATM_VCC_CFG pVccCfg;
    BCMATM_STATUS baStatus;
} ATMDRV_VCC_CFG, *PATMDRV_VCC_CFG;

typedef struct
{
    UINT32 ulInterfaceId;
    PATM_VCC_ADDR pVccAddrs;
    UINT32 ulNumVccs;
    UINT32 ulNumReturned;
    BCMATM_STATUS baStatus;
} ATMDRV_VCC_ADDRS, *PATMDRV_VCC_ADDRS;

typedef struct
{
    UINT32 ulInterfaceId;
    PATM_INTERFACE_STATS pStatistics;
    UINT32 ulReset;
    BCMATM_STATUS baStatus;
} ATMDRV_INTERFACE_STATISTICS, *PATMDRV_INTERFACE_STATISTICS;

typedef struct
{
    ATM_VCC_ADDR VccAddr;
    PATM_VCC_STATS pVccStatistics;
    UINT32 ulReset;
    BCMATM_STATUS baStatus;
} ATMDRV_VCC_STATISTICS, *PATMDRV_VCC_STATISTICS;

typedef struct
{
    UINT32 ulInterfaceId;
    ATM_INTERFACE_LINK_INFO InterfaceCfg;
    BCMATM_STATUS baStatus;
} ATMDRV_INTERFACE_LINK_INFO, *PATMDRV_INTERFACE_LINK_INFO;

typedef struct
{
    ATM_VCC_ADDR VccAddr;
    UINT32 ulNumToSend;
    BCMATM_STATUS baStatus;
} ATMDRV_TEST, *PATMDRV_TEST;

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
typedef struct
{
    int ulTrafficType;
    BCMATM_STATUS baStatus;
}
ATMDRV_TRAFFIC_TYPE, *PATMDRV_TRAFFIC_TYPE;
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* start of protocol QoS for KPN by zhangchen z45221 2006年7月18日 */
typedef struct
{
    int ulWeightFlg;
    int ulWeight;
    BCMATM_STATUS baStatus;
}
ATMDRV_QOSWEIGHT_TYPE, *PATMDRV_QOSWEIGHT_TYPE;
/* end of protocol QoS for KPN by zhangchen z45221 2006年7月18日 */

typedef struct
{
    ATM_VCC_ADDR VccAddr;
    UINT32 type;
    BCMATM_STATUS baStatus;
    UINT32 repetition;
    UINT32 timeout;
    UINT32 sent;
    UINT32 received;
    UINT32 minResponseTime;
    UINT32 maxResponseTime;
    UINT32 avgResponseTime;
} ATMDRV_OAM_LOOPBACK, *PATMDRV_OAM_LOOPBACK;

#include "portMirror.h"

#define OAM_TYPE_FUNCTION_BYTE_OFFSET       0
#define OAM_LB_INDICATION_BYTE_OFFSET       1
#define OAM_LB_CORRELATION_TAG_BYTE_OFFSET  2
#define OAM_LB_LOCATION_ID_BYTE_OFFSET      6
#define OAM_LB_SRC_ID_BYTE_OFFSET           22
#define OAM_LB_UNUSED_BYTE_OFFSET           38
#define OAM_RDI_UNUSED_BYTE_OFFSET          1
#define OAM_LB_CRC_BYTE_OFFSET              46
#define OAM_RDI_CRC_BYTE_OFFSET             46
#define OAM_LB_CORRELATION_TAG_LEN          4
#define OAM_LB_LOCATION_ID_LEN              16
#define OAM_LB_SRC_ID_LEN                   16
#define OAM_LB_UNUSED_BYTE_LEN              8
#define OAM_RDI_UNUSED_BYTE_LEN             45
#define OAM_LB_CRC_BYTE_LEN                 2
#define OAM_RDI_CRC_BYTE_LEN                2
#define OAM_FAULT_MANAGEMENT_LB             0x18
#define OAM_FAULT_MANAGEMENT_RDI            0x11
#define OAM_FAULT_MANAGEMENT_LB_REQUEST     1
#define OAM_FAULT_MANAGEMENT_LB_RESPOND     0
#define OAM_FAULT_MANAGEMENT_CORRELATION_VAL  0xbcbcbcbc
#define OAM_FAULT_MANAGEMENT_SRC_ID_3       0xffffffff
#define OAM_FAULT_MANAGEMENT_SRC_ID_2       0xffffffff
#define OAM_FAULT_MANAGEMENT_SRC_ID_1       0xffffffff
#define OAM_FAULT_MANAGEMENT_SRC_ID_0       0xffffffff
#define OAM_FAULT_MANAGEMENT_LOCATION_ID_3  0xffffffff
#define OAM_FAULT_MANAGEMENT_LOCATION_ID_2  0xffffffff    
#define OAM_FAULT_MANAGEMENT_LOCATION_ID_1  0xffffffff
#define OAM_FAULT_MANAGEMENT_LOCATION_ID_0  0xffffffff
#define OAM_LB_UNUSED_BYTE_DEFAULT          0x6a
#define OAM_LB_SEGMENT_TYPE                 0
#define OAM_LB_END_TO_END_TYPE              1
#define OAM_F4_LB_SEGMENT_TYPE              2
#define OAM_F4_LB_END_TO_END_TYPE           3
#define RM_PROT_ID_OFFSET                   0
#define RM_MESSAGE_TYPE_OFFSET              1
#define RM_PROTOCOL_ID                      1
#define RM_TYPE_DEFAULT                     0x20 /* forward/source_generated/congested */
#define RM_UNUSED_BYTES_OFFSET              2
#define RM_UNUSED_BYTES_LEN                 46
#define OAM_LOOPBACK_DEFAULT_REPETITION     1
#define OAM_LOOPBACK_DEFAULT_TIMEOUT        1000    /* msec */
#define OAM_LOOPBACK_RX_TIMEOUT_INTERVAL    10     /* check receive of OAM response every 10 ms */

#if defined(__cplusplus)
}
#endif

#endif // _ATMAPIDRV_H_

