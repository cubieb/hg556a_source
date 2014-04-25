/*****************************************************************************
;
;   (C) Unpublished Work of ADMtek Incorporated.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF ADMTEK INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) ADMTEK EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN ADMTEK WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ADMTEK.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : ADM6996M
;    Creator : l39225
;    File    :     ifonmii.h
;    Abstract: 
;
;Modification History:
; 
;
;*****************************************************************************/


#ifndef __SWFCT_H__
#define __SWFCT_H__


typedef struct _REGRW_
{
  unsigned int regAddr;
  unsigned int regVal;
} REGRW, *PREGRW; // add - 06/27/05,bolo

// #define EEPROM                              // Define this symbol to enable EEPROM read/write function
#define EEPROM_ACCESS_DELAY         5       // Delay between EEPROM access, add - 06/27/05,bolo
#define SWI_MAX_PORT_NUMBER         6
#define SWI_ACCESS_ERROR            0x55aa  // add - 06/24/05,bolo

#define SWI_SIGNATURE_STATREG       0x00
#define SWI_PORT0_BASIC_CTRLREG     0x01
#define SWI_PORT1_BASIC_CTRLREG     0x03
#define SWI_PORT2_BASIC_CTRLREG     0x05
#define SWI_PORT3_BASIC_CTRLREG     0x07
#define SWI_PORT4_BASIC_CTRLREG     0x08
#define SWI_PORT5_BASIC_CTRLREG     0x09
#define SWI_SYSTEM_CTRLREG0         0x0a
#define SWI_SYSTEM_CTRLREG1         0x0b
#define SWI_MCSTSNOOP_CTRLREG       0x0c
#define  SWI_ARP_CTRLREG                   0x0d
#define SWI_VLANPRI_MAPREG          0x0e
#define SWI_TOSPRI_MAPREG           0x0f
#define SWI_SYSTEM_CTRLREG2         0x10
#define SWI_SYSTEM_CTRLREG3         0x11
#define SWI_SYSTEM_CTRLREG4         0x12
#define SWI_FWDGROUP0_PORTMAP       0x13 // VLAN0(0x13)..VLAN15(0x22), 0x13 also for port security option & spanning tree state
#define SWI_FWDGROUP1_PORTMAP       0x14
#define SWI_FWDGROUP2_PORTMAP       0x15
#define SWI_FWDGROUP3_PORTMAP       0x16
#define SWI_FWDGROUP4_PORTMAP       0x17
#define SWI_FWDGROUP5_PORTMAP       0x18
#define SWI_VLANPRI_ENABLEMAP       0x1e
#define SWI_SERVPRI_ENABLEMAP       0x1f
#define SWI_INPUT_FORCE_NOTAG       0x20
#define SWI_INGRESS_FILTER_REG      0x21
                                         // 0x1e also for VLAN priority enable in b14..b9
                                         // 0x1f also for IPV4-TOS/IPV6 priority enable in b14..b9
                                         // 0x20 also for Input force no TAG in b14..b9
                                         // 0x21 also for Ingress filter in b14..b9
#define SWI_FWDGROUP15_PORTMAP      0x22 // 0x22 also for VLAN security disable in b14..b9
#define SWI_BUFFER_THRESHOLD0       0x23
#define SWI_QW1INJAM_THRESHOLD      0x25
#define SWI_QW2VIDCHK_PPPOEPORT     0x26
#define SWI_QW3BKVLAN_VLTAGONLY     0x27
#define SWI_PORT0_VIDREG            0x28 // P0VID(0x28)..P5VID(0x2c) for the port VID b11..b4
#define SWI_PORT1_VIDREG            0x29 // add - 08/12/05 for the port VID
#define SWI_OUTPUT_TAGPASS_REG      0x2a 
#define SWI_PORT2_VIDREG            0x2a // add - 08/12/05 for the port VID
#define SWI_PORT3P4_VIDREG          0x2b // add - 08/12/05 for the port VID
#define SWI_PORT5_VIDREG            0x2c
#define SWI_PHY_CTRLREG             0x2d
#define SWI_ADMTAG_ETHTYPE          0x2e
#define SWI_MISCEL_CTRLREG          0x30
#define SWI_BASIC_BANDWIDTH0        0x31
#define SWI_BASIC_BANDWIDTH1        0x32
#define SWI_BANDWIDTH_ENABLE        0x33
#define SWI_EXTEND_BANDWIDTH0       0x34
#define SWI_EXTEND_BANDWIDTH1       0x35
#define SWI_EXTEND_BANDWIDTH2       0x36
#define SWI_EXTEND_BANDWIDTH3       0x37
#define SWI_EXTEND_BANDWIDTH4       0x38 // also for MDIX control for port0
#define SWI_EXTEND_BANDWIDTH5       0x39 // also for MDIX control for port1
#define SWI_EXTEND_BANDWIDTH6       0x3a // also for default VLAN member
#define SWI_NEWSTORM_CTRLREG0       0x3b
#define SWI_NEWSTORM_CTRLREG1       0x3c // also for MDIX control for port2..port4
#define SWI_HWIGMP_CTRLREG          0x3f
#define SWI_VLANFILTER_LOWREG0      0x40
#define SWI_VLANFILTER_HIGHREG0     0x41
#define SWI_VLANFILTER_LOWREG15     0x5e
#define SWI_VLANFILTER_HIGHREG15    0x5f
#define SWI_SERVICE_PRI_MAPREG0     0x6c
#define SWI_SERVICE_PRI_MAPREG3F    0x73
#define SWI_TUF_REG                 0x94
#define SWI_UDPIP_REG               0x98
#define SWI_SPCTAG_INSCTRL          0x99 // also for extended IGMP control
#define SWI_INTERRUPT_ENABLE        0x9a
#define SWI_INTERRUPT_STATUS        0x9b

/***********************************************************************
 * The address definition of the switch internal configration registers 
 ***********************************************************************/
 
#define SWI_PORT_STATUS0            0xa2 // for the link status report about port0 & port1
#define SWI_PORT_STATUS1            0xa3 // for the link status report about port2 & port3
#define SWI_PORT_STATUS2            0xa4 // for the link status report about port4 & port5
#define SWI_PORT0_RXPKTCL           0xa8
#define SWI_PORT1_RXPKTCL           0xac
#define SWI_PORT2_RXPKTCL           0xb0
#define SWI_PORT3_RXPKTCL           0xb4
#define SWI_PORT4_RXPKTCL           0xb6
#define SWI_PORT5_RXPKTCL           0xb8
#define SWI_PORT0_RXBYTECL          0xba
#define SWI_PORT1_RXBYTECL          0xbe
#define SWI_PORT2_RXBYTECL          0xc2
#define SWI_PORT3_RXBYTECL          0xc6
#define SWI_PORT4_RXBYTECL          0xc8
#define SWI_PORT5_RXBYTECL          0xca
#define SWI_PORT0_TXPKTCL           0xcc
#define SWI_PORT1_TXPKTCL           0xd0
#define SWI_PORT2_TXPKTCL           0xd4
#define SWI_PORT3_TXPKTCL           0xd8
#define SWI_PORT4_TXPKTCL           0xda
#define SWI_PORT5_TXPKTCL           0xdc
#define SWI_PORT0_TXBYTECL          0xde
#define SWI_PORT1_TXBYTECL          0xe2
#define SWI_PORT2_TXBYTECL          0xe6
#define SWI_PORT3_TXBYTECL          0xea
#define SWI_PORT4_TXBYTECL          0xec
#define SWI_PORT5_TXBYTECL          0xee
#define SWI_PORT0_COLISNCL          0xf0
#define SWI_PORT1_COLISNCL          0xf4
#define SWI_PORT2_COLISNCL          0xf8
#define SWI_PORT3_COLISNCL          0xfc
#define SWI_PORT4_COLISNCL          0xfe
#define SWI_PORT5_COLISNCL          0x100
#define SWI_PORT0_ERRORCL           0x102
#define SWI_PORT1_ERRORCL           0x106
#define SWI_PORT2_ERRORCL           0x10a
#define SWI_PORT3_ERRORCL           0x10e
#define SWI_PORT4_ERRORCL           0x110
#define SWI_PORT5_ERRORCL           0x112
#define SWI_ADDRTBL_CTRLREG0        0x11a
#define SWI_ADDRTBL_CTRLREG1        0x11b
#define SWI_ADDRTBL_CTRLREG2        0x11c
#define SWI_ADDRTBL_CTRLREG3        0x11d
#define SWI_ADDRTBL_CTRLREG4        0x11e
#define SWI_ADDRTBL_CTRLREG5        0x11f
#define SWI_ADDRTBL_STATREG0        0x120
#define SWI_ADDRTBL_STATREG1        0x121
#define SWI_ADDRTBL_STATREG2        0x122
#define SWI_ADDRTBL_STATREG3        0x123
#define SWI_ADDRTBL_STATREG4        0x124
#define SWI_ADDRTBL_STATREG5        0x125
#define SWI_HWSET_STATLOW           0x130
#define SWI_ASSIGN_ADDRLOW          0x132
#define SWI_ASSIGN_ADDRMIDDLE       0x133
#define SWI_ASSIGN_ADDRHIGH         0x134
#define SWI_ASSIGN_OPTION           0x135
#define SWI_MIRROR_CTRLREG0         0x136
#define SWI_MIRROR_CTRLREG1         0x137 
#define SWI_SECURITY_VIOLPORT       0x138
#define SWI_SECURITY_STATREG0       0x139
#define SWI_SECURITY_STATREG1       0x13a
#define SWI_FIRSTLOCK_SEARCH        0x13b
#define SWI_FIRSTLOCK_ADDRLOW       0x13c
#define SWI_FIRSTLOCK_ADDRMIDDLE    0x13d
#define SWI_FIRSTLOCK_ADDRHIGH      0x13e
#define SWI_FIRSTLOCK_FID           0x13f
#define SWI_COUNTER_CTRLLOW         0x140
#define SWI_COUNTER_CTRLHIGH        0x141
#define SWI_COUNTER_STATLOW         0x142
#define SWI_COUNTER_STATHIGH        0x143
#define SWI_PHYREG_START_ADDR       0x200

#define TRUE                        1
#define FALSE                       0                                       

#define READ_MODE                   0
#define WRITE_MODE                  1

#define SWI_MACADDR_LENGTH          6
#define SWI_IGMP_MACADDR_LENGTH     3


#define SWI_MIRROR0_NOT_MIRROR      0x0
#define SWI_MIRROR0_TXPKT_PORT      0x1
#define SWI_MIRROR0_TXDA_PORT       0x2
#define SWI_MIRROR0_TXSA_PORT       0x3

#define SWI_MIRROR0_RXPKT_PORT      0x1
#define SWI_MIRROR0_RXDA_PORT       0x2
#define SWI_MIRROR0_RXSA_PORT       0x3

#define SWI_MIRROR1_ENABLE_BIT      0x8000
#define SWI_MIRROR1_CRC_BIT         0x4000
#define SWI_MIRROR1_RXER_BIT        0x2000
#define SWI_MIRROR1_PAUSE_BIT       0x1000
#define SWI_MIRROR1_LONG_BIT        0x0800
#define SWI_MIRROR1_SHORT_BIT       0x0400
#define SWI_MIRROR1_TXPKT_UMON_BIT  0x0100
#define SWI_MIRROR1_OPT_CRC         1
#define SWI_MIRROR1_OPT_RXER        2
#define SWI_MIRROR1_OPT_PAUSE       3
#define SWI_MIRROR1_OPT_LONG        4
#define SWI_MIRROR1_OPT_SHORT       5
#define SWI_MIRROR1_OPT_TXPKT_UMON  6
#define SWI_MIRROR1_OPT_MIRRORDIS   7
#define SWI_MIRROR1_KEEP_TYPEOPT    8

#define SWI_PORT_VLAN_MODE          0
#define SWI_TAGGED_VLAN_MODE        0x20

#define SWI_PORT0_FWDGRP_BIT        0x0001
#define SWI_PORT0_FWDGRP_OFFSET     0x1
#define SWI_PORT1_FWDGRP_BIT        0x0004
#define SWI_PORT1_FWDGRP_OFFSET     0x2
#define SWI_PORT2_FWDGRP_BIT        0x0010
#define SWI_PORT2_FWDGRP_OFFSET     0x4
#define SWI_PORT3_FWDGRP_BIT        0x0040
#define SWI_PORT3_FWDGRP_OFFSET     0x8
#define SWI_PORT4_FWDGRP_BIT        0x0080
#define SWI_PORT4_FWDGRP_OFFSET     0x10
#define SWI_PORT5_FWDGRP_BIT        0x0100
#define SWI_PORT5_FWDGRP_OFFSET     0x20
#define SWI_ALLPORT_FWDGRP_BIT      0x01d5

// The bits definition for the port security, the source intrusion and the source violation
#define SWI_PORT_SECURE_OPT_OFFSET  11
#define SWI_CLOSE_PORT_OFFSET       14
#define SWI_ALL_SECURE_BIT          0x7800
#define SWI_SOURCE_INTRUDE_MAXOPT   3
#define SWI_SOURCE_VIOLATE_MAXOPT   4
#define SWI_SOURCE_INTRUDE_OFFSET   9
#define SWI_SOURCE_VIOLATE_OFFSET   1
#define SWI_INTRUDE_VIOLATE_BIT     0x0e1e

#define SWI_PHYREG_OFFSET           0x20

#define SWI_COUNTER_OFFSET          0x12
#define SWI_COUNTER_HIGH_REG        0x01

#define SWI_SYSCTRL1_TRUNK_ENABLE   0x80
#define SWI_SYSCTRL1_TRUNK_SHIFT    7



#define SWI_PORT_NWAY_ABILITY       1
#define SWI_PORT_SPEED_ABILITY      2
#define SWI_PORT_DUPLEX_ABILITY     3

#define SWI_PORT_LINK_STATUS        0x01
#define SWI_PORT_SPEED_STATUS       0x02
#define SWI_PORT_SPEED_OFFSET       1
#define SWI_PORT_DUPLEX_STATUS      0x04
#define SWI_PORT_DUPLEX_OFFSET      2
#define SWI_PORT_FLOWCTRL_STATUS    0x08
#define SWI_PORT_FLOWCTRL_OFFSET    3

#define SWI_VLFILTER_SHIFT          2
#define SWI_VLFILTER_FID_OFFSET     12
#define SWI_TAGGED_MEMBER_OFFSET    6
#define SWI_VLFILTER_VAILD_OFFSET   15
#define SWI_VLFILTER_PRI_OFFSET     12

// The bits definition for the port priority, the port VID, VLAN priority and TOS priority etc
#define SWI_PORT_VID0B3_BIT         0x3c00  // add - 08/12/05 for the port VID
#define SWI_PORT_VID0B3_MASK        0x0f    // add - 08/12/05 for the port VID
#define SWI_PORT_VID4B11_BIT        0xff    // add - 08/12/05 for the port VID
#define SWI_PORT4_VID4B11_BIT       0xff00  // add - 08/12/05 for the port VID
#define SWI_PORT_VID4B11_MASK       0xff0   // add - 08/12/05 for the port VID
#define SWI_PORT_PRI_BIT            0x0380
#define SWI_IPOVLAN_PRI_BIT         0x0040
#define SWI_VLAN_PRIEN_BIT          0x7e00
#define SWI_SERVICE_PRIEN_BIT       0x7e00
#define SWI_TOS_USING_3BITS         0x0002
#define SWI_PRIQ_OPTION_VALUE       0x0003
#define SWI_VID_CHECK_BIT           0x0fc0
#define SWI_DFTVLAN_MEMBER_BIT      0x0fc0
#define SWI_DEFAULT_FID_BIT         0x0078
#define SWI_BACK_VLAN_BIT           0x0fc0
#define SWI_FORCE_NOTAG_BIT         0x7e00
#define SWI_INGRESS_FILTER_BIT      0x7e00
#define SWI_ADMIT_VLANTAG_BIT       0x003f
#define SWI_OUTPUT_TAGPASS_BIT      0x3f00
#define SWI_VLAN_SECURITY_BIT       0x7e00
#define SWI_NEW_STORM_ENABLE        0x0040
#define SWI_STORMDROP_THRES_BIT     0x1fff
#define SWI_STORM_DROP_ENABLE_BIT   0x6000
#define SWI_MULPKT_STORM_BIT        0x0020
#define SWI_NEW_BWCTRL_BIT          0x1000
#define SWI_BWT_TYPEL0_4BIT         0x000f
#define SWI_BWT_TYPEL1_4BIT         0x00f0
#define SWI_BWT_TYPEH0_4BIT         0x0f00
#define SWI_BWT_TYPEH1_4BIT         0xf000
#define SWI_BWT_TYPEL0_3BIT         0x0007
#define SWI_BWT_TYPEL1_3BIT         0x0038
#define SWI_BWT_TYPEH0_3BIT         0x01c0
#define SWI_BWT_TYPEH1_3BIT         0x0e00
#define SWI_BWT_TYPEH2_3BIT         0x7000
#define SWI_BWT_TYPEH3_3BIT         0x0700

// The offset definition for the port priority, the port VID, VLAN priority and TOS priority etc
#define SWI_PRIQ0_MAP_OFFSET        0
#define SWI_PRIQ1_MAP_OFFSET        2
#define SWI_PRIQ2_MAP_OFFSET        4
#define SWI_PRIQ3_MAP_OFFSET        6
#define SWI_PRIQ4_MAP_OFFSET        8
#define SWI_PRIQ5_MAP_OFFSET        10
#define SWI_PRIQ6_MAP_OFFSET        12
#define SWI_PRIQ7_MAP_OFFSET        14
#define SWI_PORT_VID0B3_OFFSET      10  // add - 08/12/05 for the port VID
#define SWI_PORT_VID0B3_SHIFT       4   // add - 08/12/05 for the port VID
#define SWI_PORT4_VID4B11_OFFSET    8   // add - 08/12/05 for the port VID
#define SWI_PORT_PRI_OFFSET         8
#define SWI_PORT_PRIEN_OFFSET       7
#define SWI_IPOVLAN_PRI_OFFSET      6
#define SWI_VLAN_PRIEN_OFFSET       9
#define SWI_SERVICE_PRIEN_OFFSET    9
#define SWI_VID_CHECK_OFFSET        6
#define SWI_DFTVLAN_MEMBER_OFFSET   6
#define SWI_DEFAULT_FID_OFFSET      3
#define SWI_BACK_VLAN_OFFSET        6
#define SWI_FORCE_NOTAG_OFFSET      9
#define SWI_INGRESS_FILTER_OFFSET   9
#define SWI_ADMIT_VLANTAG_OFFSET    0
#define SWI_OUTPUT_TAGPASS_OFFSET   8
#define SWI_VLAN_SECURITY_OFFSET    9
#define SWI_STORM_ENABLE_OFFSET     13
#define SWI_DROP_ENABLE_OFFSET      14
#define SWI_MULPKT_STORM_OFFSET     5
#define SWI_TX0BWC_ENABLE_OFFSET    1
#define SWI_TX1BWC_ENABLE_OFFSET    3
#define SWI_TX2BWC_ENABLE_OFFSET    5
#define SWI_TX3BWC_ENABLE_OFFSET    9
#define SWI_TX4BWC_ENABLE_OFFSET    10
#define SWI_TX5BWC_ENABLE_OFFSET    11
#define SWI_RX0BWC_ENABLE_OFFSET    0
#define SWI_RX1BWC_ENABLE_OFFSET    2
#define SWI_RX2BWC_ENABLE_OFFSET    4
#define SWI_RX3BWC_ENABLE_OFFSET    6
#define SWI_RX4BWC_ENABLE_OFFSET    7
#define SWI_RX5BWC_ENABLE_OFFSET    8
#define SWI_BWT_TYPEL0_OFFSET0      0
#define SWI_BWT_TYPEL1_OFFSET0      4
#define SWI_BWT_TYPEH0_OFFSET0      8
#define SWI_BWT_TYPEH1_OFFSET0      12
#define SWI_BWT_TYPEL0_OFFSET1      0
#define SWI_BWT_TYPEL1_OFFSET1      3
#define SWI_BWT_TYPEH0_OFFSET1      6
#define SWI_BWT_TYPEH1_OFFSET1      9
#define SWI_BWT_TYPEH2_OFFSET1      12

// The MAC address type for the learning table
#define SWI_DYNAMIC_MACADDR         0
#define SWI_STATIC_MACADDR          1
#define SWI_LRNTBL_INFOTYPE_BIT     0x1000
#define SWI_LRNTBL_INFOTYPE_OFFSET  12
#define SWI_LRNTBL_PORTMAP_BIT      0x03f0
#define SWI_LRNTBL_PORTMAP_OFFSET   4
#define SWI_LRNTBL_FID_BIT          0x000f
#define SWI_LRNTBL_FID_OFFSET       0
#define SWI_LRNTBL_INFOCTRL_BIT     0x01ff
#define SWI_LRNTBL_INFOCTRL_OFFSET  0
#define SWI_LRNTBL_ACCTRL_OFFSET    0
#define SWI_LRNTBL_COMMAND_OFFSET   4
#define SWI_LRNTBL_RESULT_BIT       0x7000
#define SWI_LRNTBL_RESULT_OFFSET    12

// The access control code for the learning table
#define SWI_ACCESS_CREATE_NEWMAC    0x7
#define SWI_ACCESS_OVERWRITE_MAC    0xf
#define SWI_ACCESS_ERASE_MAC        0xf
#define SWI_ACCESS_SEARCH_EMPTYMAC  0x0
#define SWI_ACCESS_SEARCH_OUTPORT   0x9
#define SWI_ACCESS_SEARCH_FWDGRP    0xa
#define SWI_ACCESS_SEARCH_MACADDR   0xc
#define SWI_ACCESS_SEARCH_FWGPMAC   0xe
#define SWI_ACCESS_SEARCH_MACOUTPT  0xd
#define SWI_ACCESS_SEARCH_FWGOUTPT  0xb
#define SWI_ACCESS_SEARCH_FWMACOUT  0xf

// The command code for the learning table
#define SWI_COMMAND_WRITE_MAC       0x0
#define SWI_COMMAND_ERASE_MAC       0x1
#define SWI_COMMAND_SEARCH_MAC      0x2
#define SWI_COMMAND_INITIAL_MAC     0x3
#define SWI_COMMAND_WRITE_IGMPTBL   0x4
#define SWI_COMMAND_READ_IGMPTBL    0x5
#define SWI_LRNTABL_ACCESS_BUSY     0x8000

// The command code for the learning table
#define SWI_COMMAND_ENTRY_FOUND     0
#define SWI_ACCESS_ALLENTRY_USED    1
#define SWI_COMMAND_ENTRY_MISSED    5
#define SWI_COMMAND_ENTRY_BUSY      7

// The status bits for the IGMP table
#define SWI_IGMPTBL_ACCESS_MAXNUM   10
#define SWI_IGMPTBL_ENTRY_BIT       0x01f
#define SWI_IGMPTBL_OCCUPY_BIT      0x0300
#define SWI_IGMPTBL_OCCUPY_OFFSET   8
#define SWI_IGMPTBL_PORTMAP_BIT     0x1f80
#define SWI_IGMPTBL_PORTMAP_OFFSET  7

// The register bits for the hardware IGMP snooping
#define SWI_IGMP_ROUTER_ENABLE_BIT  0x01
#define SWI_IGMP_SNOOP_ENABLE_BIT   0x02
#define SWI_IGMP_IGNORE_CPUPORT_BIT 0x04
#define SWI_IGMP_QRYINTERVAL_OFFSET 8
#define SWI_IGMP_ROBUSTVAR_OFFSET   6

// The option bits for the hardware IGMP snooping
#define SWI_TRAP_IGMP_PACKET_BIT    0x08
#define SWI_TRAP_IGMP_PACKET_OFFSET 3
#define SWI_IGMP_CROSS_VLAN_BIT     0x100
#define SWI_IGMP_CROSS_VLAN_OFFSET  8
#define SWI_IGMP_PACKET_PRI_BIT     0x1800
#define SWI_IGMP_PKT_TXTAG_OFFSET   9
#define SWI_IGMP_PKT_TXTAG_BIT      0x0600
#define SWI_IGMP_PACKET_PRI_OFFSET  11
#define SWI_IGMP_PKT_PRIEN_BIT      0x2000
#define SWI_IGMP_PKT_PRIEN_OFFSET   13
#define SWI_IGMP_CTRL_ACTION_BIT    0xc000
#define SWI_IGMP_CTRL_ACTION_OFFSET 14
#define SWI_IGMP_EXTRA_CTRL_BIT     0x3000
#define SWI_IGMP_EXTRA_CTRL_OFFSET  12
#define SWI_SRCVIO_OVER_IGMP_BIT    0x04
#define SWI_SRCVIO_OVER_IGMP_OFFSET 2
#define SWI_SRCVIO_OVER_DFLT_BIT    0x02
#define SWI_SRCVIO_OVER_DFLT_OFFSET 1

// The register bits for the counter control registers, add - 06/16/05,bolo
#define SWI_COUNTER_BUSY_BIT        0x80
#define SWI_RENEW_PORT_COUNTER      0xc0
#define SWI_ACCESS_COUNTER_FREE     0
#define SWI_ACCESS_COUNTER_BUSY     1


#endif
