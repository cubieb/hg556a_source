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
//**************************************************************************
// File Name  : AtmProcRegs.h for BCM63xx
//
// Description: This file contains definitions for the ATM processor registers.
//
// Updates    : 12/21/2001  lat.  Created from BCM6352.
//**************************************************************************

#if !defined(_ATMPROCREGS_H_)
#define _ATMPROCREGS_H_

#if defined(_CFE_)
#if defined(_BCM96348_)
#define CONFIG_BCM96348
#elif defined(_BCM96338_)
#define CONFIG_BCM96338
#elif defined(_BCM96358_)
#define CONFIG_BCM96358
#endif
#endif

// Maximum values.
#define AP_MAX_QUEUES                       8
#define AP_MAX_SHAPERS                      AP_MAX_QUEUES
#define AP_MAX_MULTI_PRIORITY_VCIDS         2

#if defined(CONFIG_BCM96358)
#define AP_MAX_PORTS                        4
#define AP_MAX_VCCS                         8
#define AP_MAX_VCIDS                        16
#define AP_MAX_RX_VCIDS                     16
#elif defined(CONFIG_BCM96348)
#define AP_MAX_PORTS                        2
#define AP_MAX_VCCS                         8
#define AP_MAX_VCIDS                        16
#define AP_MAX_RX_VCIDS                     16
#define AP_RESERVED                         32
#elif defined(CONFIG_BCM96338)
#define AP_MAX_PORTS                        2
#define AP_MAX_VCCS                         8
#define AP_MAX_VCIDS                        8
#define AP_MAX_RX_VCIDS                     4
#define AP_RX_HW_VCIDS                      4
#endif

// Definitions for ulTxQHeadTailLen field.
#define DMAR_TX_LEN_MASK                    0x000000ff
#define DMAR_TX_LEN_SHIFT                   0
#define DMAR_TX_TAIL_MASK                   0x0000ff00
#define DMAR_TX_TAIL_SHIFT                  8
#define DMAR_TX_HEAD_MASK                   0x00ff0000
#define DMAR_TX_HEAD_SHIFT                  16
#define DMAR_TX_WM_MASK                     0xff000000
#define DMAR_TX_WM_SHIFT                    24

// Offsets for free and receive queue DMA registers.
#define RX_FREE_ADDR_OFFSET                 0
#define RX_FREE_HEAD_OFFSET                 1
#define RX_FREE_TAIL_OFFSET                 2
#define RX_FREE_LEN_OFFSET                  3

// Definitions for ulIsbCfg field.
#define DMAR_MS4BITS_MASK                   0x0000000f
#define DMAR_MS4BITS_SHIFT                  0
#define DMAR_SLAVE_ACK_TO_MASK              0x000003f0
#define DMAR_SLAVE_ACK_TO_SHIFT             4

// Definitions for ulAdslLastDescCfg.
#define ALDC_TX_DESC_8                      0x0000
#define ALDC_TX_DESC_16                     0x0001
#define ALDC_RX_DESC_8                      0x0000
#define ALDC_RX_DESC_16                     0x0004
#define ALDC_TX_ADSL_ADSL                   0x0000
#define ALDC_TX_UTOPIA_UTOPIA               0x0010
#define ALDC_TX_UTOPIA_ADSL                 0x0020
#define ALDC_RX_TX_UTOPIA_ADSL_OVERLAY      0x0080  /* For Bonding */
#define ALDC_RX_ADSL_ADSL                   0x0000
#define ALDC_RX_UTOPIA_UTOPIA               0x0100
#define ALDC_RX_UTOPIA_ADSL                 0x0200

// Definitions for ulDmaCfg.
#define RBC_RX_BUFFER_SEPARATION            0x01
#define RBC_RX_THRESHOLD                    0x02
#define RBC_RX_ARB_PORT0_HIGH               0x00
#define RBC_RX_ARB_PORT1_HIGH               0x04
#define RBC_RX_ARB_ROUND_ROBIN              0x08
#define RBC_RX_ARB_NOT_USED                 0x0c
#define RBC_TX_ABORT_ON_EMPTY               0x10

// Definitions for ulIrqStatus and ulIrqMask field.
#define INTR_TX_QUEUE                       0x00000001
#define INTR_RCQ_ALMOST_FULL                0x00000002
#define INTR_FCQ_ALMOST_EMPTY               0x00000004
#define INTR_RPQ_ALMOST_FULL                0x00000008
#define INTR_FPQ_ALMOST_EMPTY               0x00000010
#define INTR_MIB_COUNTER_HALF_FULL          0x00000020
#define INTR_RCQ_WD_TIMER                   0x00000040
#define INTR_RPQ_WD_TIMER                   0x00000080
#define INTR_RCQ_IMMED_RSP                  0x00000100
#define INTR_RPQ_IMMED_RSP                  0x00000200
#define INTR_ERROR_RX_RTR_DROPPED_CELL      0x00000400
#define INTR_ERROR_VCAM_MULT_MATCH          0x00000800
#define INTR_TX_QUEUE_ENABLE_ALL            0x00ff0000

// Definitions for ulRxFreeIrqCqWatermark and ulRxFreeIrqPqWatermark.
#define INTR_RX_WM_MASK                     0x0000ffff
#define INTR_RX_WM_SHIFT                    0
#define INTR_FREE_WM_MASK                   0xffff0000
#define INTR_FREE_WM_SHIFT                  16

// Definitions for ulRxQWatchdog.
#define INTR_RCQ_WATCHDOG_MASK              0x0000ffff
#define INTR_RCQ_WATCHDOG_SHIFT             0
#define INTR_RPQ_WATCHDOG_MASK              0xffff0000
#define INTR_RPQ_WATCHDOG_SHIFT             16

// Definitions for ulTxAtmHdrCfg field.
#if defined(CONFIG_BCM96358)
#define ATMR_TX_PORT_EN_MASK                0x0000000f
#define ATMR_TX_PORT_EN_SHIFT               0
#define ATMR_OAM_CRC10_EN_MASK              0x000000f0
#define ATMR_OAM_CRC10_EN_SHIFT             4
#define ATMR_OUT_FIFO_RESET_MASK            0x00000f00
#define ATMR_OUT_FIFO_RESET_SHIFT           8
#define ATMR_USE_ALT_GFC_MASK               0x0000f000
#define ATMR_USE_ALT_GFC_SHIFT              12
#define ATMR_ALT_GFC_MASK                   0x000f0000
#define ATMR_ALT_GFC_SHIFT                  16
#define ATMR_TX_HALT_SHAPERS_MASK           0x00f00000
#define ATMR_TX_HALT_SHAPERS_SHIFT          20
#define ATMR_SCHED_RESET_VALUE_MASK         0x0f000000
#define ATMR_SCHED_RESET_SHIFT              24
#define ATMR_CRC_INIT                       0x10000000
#define ATMR_AAL5_SW_TRAILER_EN             0x20000000
#else
#define ATMR_TX_PORT_EN_MASK                0x00000003
#define ATMR_TX_PORT_EN_SHIFT               0
#define ATMR_OAM_CRC10_EN_MASK              0x0000000c
#define ATMR_OAM_CRC10_EN_SHIFT             2
#define ATMR_OUT_FIFO_RESET_MASK            0x00000030
#define ATMR_OUT_FIFO_RESET_SHIFT           4
#define ATMR_USE_ALT_GFC_MASK               0x000000c0
#define ATMR_USE_ALT_GFC_SHIFT              6
#define ATMR_ALT_GFC_MASK                   0x00000f00
#define ATMR_ALT_GFC_SHIFT                  8
#define ATMR_TX_HALT_SHAPERS_MASK           0x00003000
#define ATMR_TX_HALT_SHAPERS_SHIFT          12
#define ATMR_SCHED_RESET_VALUE_MASK         0x00018000
#define ATMR_SCHED_RESET_SHIFT              15
#define ATMR_AAL5_SW_TRAILER_EN             0x00020000
#endif

// Definitions for ulTxAtmSchedCfg field.
#if defined(CONFIG_BCM96358)
#define ATMR_SCHED_EN                       0x00000001
#define ATMR_SCHED_EN_MASK                  0x0000000f
#define ATMR_SCHED_EN_SHIFT                 0
#define ATMR_PORT_MODE_MASK                 0x000000f0
#define ATMR_PORT_MODE_SHIFT                4
#define ATMR_SIT_MASK                       0x00ffff00
#define ATMR_SIT_SHIFT                      8
#define ATMR_SIT_MODE                       0x01000000
#define ATMR_ENE_INIT                       0x02000000
#define ATMR_PHY_SYNC_EN                    0x04000000
#define ATMR_EARLY_SCHED_EN                 0x08000000

#else
#define ATMR_SCHED_EN                       0x00000001
#define ATMR_SCHED_EN_MASK                  0x00000003
#define ATMR_SCHED_EN_SHIFT                 0
#define ATMR_PORT_MODE_MASK                 0x0000000c
#define ATMR_PORT_MODE_SHIFT                2
#define ATMR_SIT_MODE                       0x00000010
#define ATMR_SIT_MASK                       0x00003fe0
#define ATMR_SIT_SHIFT                      5
#define ATMR_EARLY_SCHED_EN                 0x00800000 // BCM6348B0
#endif

// Definitions for ulBondingCfg field [BCM6358 Only].
#define ATMR_BOND_ASM_VPI_VCI_MASK          0x00ffffff
#define ATMR_BOND_AUTO_SID                  0x20000000
#define ATMR_BOND_12_BIT_SID                0x40000000
#define ATMR_BOND_EN                        0x80000000

// Definitions for ulTxAtmMpAalCfg field.
#define ATMR_MP_VC_EN                       0x01
#define ATMR_MP_VCID_SHIFT                  0x01

#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96348)
#define ATMR_MP_SHIFT                       8
#define ATMR_MP_MASK                        0xff

#endif

// Definitions for ulRxAtmCfg field.
#define ATMR_RX_PORT_EN                     0x00000001
#define ATMR_DROP_ON_ERROR_MASK             0x000001fe
#define ATMR_DROP_ON_ERROR_SHIFT            0x0001
#define ATMR_DROP_GFC_ERRORS                (0x01 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_CRC_ERRORS                (0x02 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_NOT_USED1                 (0x04 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_CAM_LOOKUP_ERRORS         (0x08 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_IDLE_CELLS                (0x10 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_PTI_ERRORS                (0x20 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_NOT_USED2                 (0x40 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_DROP_PORT_NOT_ENABLED_ERRORS   (0x80 << ATMR_DROP_ON_ERROR_SHIFT)
#define ATMR_OAM_CRC10_IGNORE               0x00000200
#define ATMR_GFC_ERROR_IGNORE               0x00000400
#define ATMR_ERROR_CELLS_TO_PRQ             0x00000800
#define ATMR_TSP_CELLS_TO_PRQ               0x00001000
#define ATMR_TSP_IMMED_RSP                  0x00002000
#define ATMR_UTOPIA_RX_FLOW_CTRL            0x00004000 // [BCM6348 Only]
#define ATMR_MASK_ALL_VCI                   0x00008000
#define ATMR_VPI_VCI_MASK_MASK              0xffff0000
#define ATMR_VPI_VCI_MASK_SHIFT             16


// Definitions for ulRxAalCfg field.
#define ATMR_RX_AAL5_SW_TRAILER_EN          0x00000001
#define ATMR_EN                             0x00000002 // [BCM6338 Only]
#define ATMR_LINK                           0x00000004 // [BCM6338 Only]
#define ATMR_MODE_LINK_ONLY                 0x00000000 // [BCM6338 Only]
#define ATMR_MODE_CELL_ACTIVITY             0x00000008 // [BCM6338 Only]
#define ATMR_MODE_MELODY_LINK               0x00000010 // [BCM6338 Only]
#define ATMR_MODE_LINK_CELL_ACTIVITY        0x00000018 // [BCM6338 Only]
#define ATMR_LED_SPEED_30MS                 0x00000000 // [BCM6338 Only]
#define ATMR_LED_SPEED_50MS                 0x00000020 // [BCM6338 Only]
#define ATMR_LED_SPEED_125MS                0x00000040 // [BCM6338 Only]
#define ATMR_LED_SPEED_250MS                0x00000060 // [BCM6338 Only]
#define ATMR_LED_SPEED_MASK                 0x00000060 // [BCM6338 Only]

// Definitions for ulRxAalMaxSdu field.
#define ATMR_MAX_AAL5_SDU_MASK              0x0000ffff

// Definitions for ulUtopiaCfg.
#define ATMR_TX_UTOPIA_EN                   0x00000001
#define ATMR_TX_UTOPIA_LVL2_EN              0x00000002
#define ATMR_TX_UTOPIA_NEG_EDGE_SEL         0x00000004 // [BCM6358 Only]
#define ATMR_RX_UTOPIA_EN                   0x00000010
#define ATMR_RX_UTOPIA_LVL2_EN              0x00000020
#define ATMR_RX_UTOPIA_NEG_EDGE_SEL         0x00000040 // [BCM6358 Only]

// Definitions for ulAdslLedCtrl. [BCM6348 Only]
#define ALC_EN                              0x00000001
#define ALC_MODE_LINK_ONLY                  0x00000000
#define ALC_MODE_CELL_ACTIVITY              0x00000002
#define ALC_MODE_MELODY_LINK                0x00000004
#define ALC_MODE_LINK_CELL_ACTIVITY         0x00000006
#define ALC_LINK                            0x00000010
#define ALC_LED_SPEED_30MS                  0x00000000
#define ALC_LED_SPEED_50MS                  0x00000020
#define ALC_LED_SPEED_125MS                 0x00000040
#define ALC_LED_SPEED_250MS                 0x00000060
#define ALC_LED_SPEED_MASK                  0x00000060

// Definitions for ulTxVpiVciTable.
#define TXTBL_VCI_MASK                      0x0000ffff
#define TXTBL_VCI_SHIFT                     0
#define TXTBL_VPI_MASK                      0x00ff0000
#define TXTBL_VPI_SHIFT                     16
#define TXTBL_UD_CRC10_EN                   0x01000000
#define TXTBL_SW_DEFINED                    0x02000000

// Definitions for ulRxVpiVciCam - CAM Side.
#if defined(CONFIG_BCM96358)
#define RXCAM_PORT_MASK                     0x00000003
#define RXCAM_PORT_SHIFT                    0
#define RXCAM_VCI_MASK                      0x0003fffc
#define RXCAM_VCI_SHIFT                     2
#define RXCAM_VPI_MASK                      0x03fc0000
#define RXCAM_VPI_SHIFT                     18
#define RXCAM_VALID                         0x04000000
#else
#define RXCAM_PORT_MASK                     0x00000001
#define RXCAM_PORT_SHIFT                    0
#define RXCAM_VCI_MASK                      0x0001fffe
#define RXCAM_VCI_SHIFT                     1
#define RXCAM_VPI_MASK                      0x01fe0000
#define RXCAM_VPI_SHIFT                     17
#define RXCAM_VALID                         0x02000000
#endif

// Definitions for ulRxVpiVciCam - RAM Side.
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96348)
#define RXCAM_CT_AAL5                       0x00000000
#define RXCAM_CT_TRANSPARENT                0x00000001
#define RXCAM_CT_AAL0_PACKET                0x00000002
#define RXCAM_CT_AAL0_CELL                  0x00000003
#define RXCAM_UD_RC_RCQ                     0x00000000
#define RXCAM_UD_RC_RPQ                     0x00000004
#define RXCAM_OAM_RC_RCQ                    0x00000000
#define RXCAM_OAM_RC_RPQ                    0x00000008
#define RXCAM_UD_CRC10_EN                   0x00000010
#define RXCAM_VCID_MASK                     0x000001e0
#define RXCAM_VCID_SHIFT                    5
#define RXCAM_RM_IMMED_RSP                  0x00000200
#define RXCAM_OAM_IMMED_RSP                 0x00000400
#define RXCAM_NON_OAM_RM_IMMED_RSP          0x00000800
#define RXCAM_HDLC_EN                       0x00001000

#elif defined(CONFIG_BCM96338)
#define RXCAM_CT_AAL5                       0x00000000
#define RXCAM_CT_TRANSPARENT                0x00000001
#define RXCAM_CT_AAL0_PACKET                0x00000002
#define RXCAM_CT_AAL0_CELL                  0x00000003
#define RXCAM_UD_RC_RCQ                     0x00000000
#define RXCAM_UD_RC_RPQ                     0x00000004
#define RXCAM_OAM_RC_RCQ                    0x00000000
#define RXCAM_OAM_RC_RPQ                    0x00000008
#define RXCAM_UD_CRC10_EN                   0x00000010
#define RXCAM_VCID_MASK                     0x00000060
#define RXCAM_VCID_SHIFT                    5
#define RXCAM_RM_IMMED_RSP                  0x00000080
#define RXCAM_OAM_IMMED_RSP                 0x00000100
#define RXCAM_NON_OAM_RM_IMMED_RSP          0x00000200
#endif

// Definitions for ulSstCtrl.
#if defined(CONFIG_BCM96358)
#define SST_EN                              0x00000001
#define SST_VCID_MASK                       0x0000001e
#define SST_VCID_SHIFT                      1
#define SST_PORT_ID_MASK                    0x00000060
#define SST_PORT_ID_SHIFT                   5
#define SST_UBR_NO_PCR                      0x00000000
#define SST_UBR_PCR                         0x00000080
#define SST_VBR_1                           0x00000100
#define SST_CBR                             0x00000180
#define SST_MCR_EN                          0x00000200
#define SST_SRC_PRI_MASK                    0x00000c00
#define SST_SRC_PRI_SHIFT                   10
#define SST_MP_EN                           0x00001000
#define SST_RATE_PCR_MASK                   0x01ffe000
#define SST_RATE_PCR_SHIFT                  13
#define SST_SHAPER_RESET                    0x02000000

#elif defined(CONFIG_BCM96348)
#define SST_EN                              0x00000001
#define SST_VCID_MASK                       0x0000001e
#define SST_VCID_SHIFT                      1
#define SST_PORT_ID_MASK                    0x00000020
#define SST_PORT_ID_SHIFT                   5
#define SST_UBR_NO_PCR                      0x00000000
#define SST_UBR_PCR                         0x00000040
#define SST_VBR_1                           0x00000080
#define SST_CBR                             0x000000c0
#define SST_MCR_EN                          0x00000100
#define SST_SRC_PRI_MASK                    0x00000600
#define SST_SRC_PRI_SHIFT                   9
#define SST_MP_EN                           0x00000800
#define SST_RATE_PCR_MASK                   0x00fff000
#define SST_RATE_PCR_SHIFT                  12
#define SST_SHAPER_RESET                    0x01000000

#endif

// Definitions for ulSstVbr.
#define SST_RATE_SCR_MASK                   0x00000fff
#define SST_RATE_SCR_SHIFT                  0
#define SST_RATE_BT_MASK                    0x7ffff000
#define SST_RATE_BT_SHIFT                   12


#if defined(CONFIG_BCM96358)

#define ATM_PROCESSOR_BASE                                       0xfffe2000
typedef struct AtmProcessorRegisters
{
    UINT32 ulTxQAddr[AP_MAX_QUEUES];                                // 0000
    UINT32 ulTxQHeadTailLen[AP_MAX_QUEUES];                         // 0020
    UINT32 ulTxSdramChStatus;                                       // 0040
    UINT32 ulTxSdramChEnable;                                       // 0044
    UINT32 ulIsbCfg;                                                // 0048
    UINT32 ulReserved1;                                             // 004c
    UINT32 ulAdslLastDescBaseAddr;                                  // 0050
    UINT32 ulAdslLastDescCfg;                                       // 0054
    UINT32 ulAdslLastDescCellCnts;                                  // 0058
    UINT32 ulReserved2[9];                                          // 005c
    UINT32 ulFreePktQAddr;                                          // 0080
    UINT32 ulFreePktQHead;                                          // 0084
    UINT32 ulFreePktQTail;                                          // 0088
    UINT32 ulFreePktQLen;                                           // 008c
    UINT32 ulRxPktQAddr;                                            // 0090
    UINT32 ulRxPktQHead;                                            // 0094
    UINT32 ulRxPktQTail;                                            // 0098
    UINT32 ulRxPktQLen;                                             // 009c
    UINT32 ulFreeCellQAddr;                                         // 00a0
    UINT32 ulFreeCellQHead;                                         // 00a4
    UINT32 ulFreeCellQTail;                                         // 00a8
    UINT32 ulFreeCellQLen;                                          // 00ac
    UINT32 ulRxCellQAddr;                                           // 00b0
    UINT32 ulRxCellQHead;                                           // 00b4
    UINT32 ulRxCellQTail;                                           // 00b8
    UINT32 ulRxCellQLen;                                            // 00bc
    UINT32 ulReserved3[4];                                          // 00c0
    UINT32 ulRxChBusyStatus;                                        // 00d0
    UINT32 ulRxChActiveStatus;                                      // 00d4
    UINT32 ulRxFlushBd;                                             // 00d8
    UINT32 ulDmaCfg;                                                // 00dc
    UINT32 ulReserved4[8];                                          // 00e0
    UINT32 ulTxAdslDmaDescrTableP0[32];                             // 0100
    UINT32 ulTxAdslDmaDescrTableP1[32];                             // 0180
    UINT32 ulRxAdslDmaDescrTableP0[32];                             // 0200
    UINT32 ulRxAdslDmaDescrTableP1[32];                             // 0280
    UINT32 ulIrqStatus;                                             // 0300
    UINT32 ulIrqMask;                                               // 0304
    UINT32 ulIrqTxQWatchdog;                                        // 0308
    UINT32 ulIrqRxFreeCellQWatermark;                               // 030c
    UINT32 ulIrqRxFreePktQWatermark;                                // 0310
    UINT32 ulIrqRxQWatchdog;                                        // 0314
    UINT32 ulReserved5[58];                                         // 0318
    UINT32 ulTxAtmHdrCfg;                                           // 0400
    UINT32 ulTxAtmSchedCfg;                                         // 0404
    UINT32 ulBondingCfg;                                            // 0408
    UINT32 ulTxAtmStatus;                                           // 040c
    UINT32 ulTxAtmStatus2;                                          // 0410
    UINT32 ulTxAtmMpAalCfg;                                         // 0414
    UINT32 ulRxAtmCfg[AP_MAX_PORTS];                                // 0418
    UINT32 ulRxAtmStatus;                                           // 0428
    UINT32 ulUtopiaCfg;                                             // 042c
    UINT32 ulRxAalCfg;                                              // 0430
    UINT32 ulRxAalMaxSdu;                                           // 0434
    UINT32 ulRxAalStatus;                                           // 0438
    UINT32 ulAdslLedCtrl;                                           // 043c
    UINT32 ulTxAal5CellCnt[AP_MAX_PORTS];                           // 0440
    UINT32 ulTxAal0CellCnt[AP_MAX_PORTS];                           // 0450
    UINT32 ulRxAal5CellCnt[AP_MAX_PORTS];                           // 0460
    UINT32 ulRxAal0CellCnt[AP_MAX_PORTS];                           // 0470
    UINT32 ulReserved6[32];                                         // 0480
    UINT32 ulTxVpiVciTable[AP_MAX_VCIDS];                           // 0500
    UINT32 ulReserved7[48];                                         // 0540
    UINT32 ulRxVpiVciCam[AP_MAX_VCIDS * 2];                         // 0600
    UINT32 ulReserved8[32];                                         // 0680
    UINT32 ulSstCtrl[AP_MAX_SHAPERS];                               // 0700
    UINT32 ulSstVbr[AP_MAX_SHAPERS];                                // 0720
    UINT32 ulSstMcr[AP_MAX_SHAPERS];                                // 0740
    UINT32 ulReserved13[40];                                        // 0760
} ATM_PROCESSOR_REGISTERS, *PATM_PROCESSOR_REGISTERS;

#elif defined(CONFIG_BCM96348)
#define ATM_PROCESSOR_BASE                                       0xfffe4000
typedef struct AtmProcessorRegisters
{
    UINT32 ulTxQAddr[AP_MAX_QUEUES];                                // 0000
    UINT32 ulReserved1[24];                                         // 0020
    UINT32 ulTxQHeadTailLen[AP_MAX_QUEUES];                         // 0080
    UINT32 ulReserved2[40];                                         // 00a0
    UINT32 ulTxSdramChStatus;                                       // 0140
    UINT32 ulTxSdramChEnable;                                       // 0144
    UINT32 ulReserved3[3];                                          // 0148
    UINT32 ulIsbCfg;                                                // 0154
    UINT32 ulAdslLastDescBaseAddr;                                  // 0158
    UINT32 ulAdslLastDescCfg;                                       // 015c
    UINT32 ulAdslLastDescCellCnts;                                  // 0160
    UINT32 ulReserved4[39];                                         // 0164
    UINT32 ulFreePktQAddr;                                          // 0200
    UINT32 ulFreePktQHead;                                          // 0204
    UINT32 ulFreePktQTail;                                          // 0208
    UINT32 ulFreePktQLen;                                           // 020c
    UINT32 ulRxPktQAddr;                                            // 0210
    UINT32 ulRxPktQHead;                                            // 0214
    UINT32 ulRxPktQTail;                                            // 0218
    UINT32 ulRxPktQLen;                                             // 021c
    UINT32 ulFreeCellQAddr;                                         // 0220
    UINT32 ulFreeCellQHead;                                         // 0224
    UINT32 ulFreeCellQTail;                                         // 0228
    UINT32 ulFreeCellQLen;                                          // 022c
    UINT32 ulRxCellQAddr;                                           // 0230
    UINT32 ulRxCellQHead;                                           // 0234
    UINT32 ulRxCellQTail;                                           // 0238
    UINT32 ulRxCellQLen;                                            // 023c
    UINT32 ulReserved5[18];                                         // 0240
    UINT32 ulRxChBusyStatus;                                        // 0288
    UINT32 ulRxChActiveStatus;                                      // 028c
    UINT32 ulRxFlushBd;                                             // 0290
    UINT32 ulDmaCfg;                                                // 0294
    UINT32 ulReserved6[26];                                         // 0298
    UINT32 ulIrqStatus;                                             // 0300
    UINT32 ulIrqMask;                                               // 0304
    UINT32 ulIrqTxQWatchdog;                                        // 0308
    UINT32 ulIrqRxFreeCellQWatermark;                               // 030c
    UINT32 ulIrqRxFreePktQWatermark;                                // 0310
    UINT32 ulIrqRxQWatchdog;                                        // 0314
    UINT32 ulReserved7[58];                                         // 0318
    UINT32 ulTxAtmHdrCfg;                                           // 0400
    UINT32 ulTxAtmSchedCfg;                                         // 0404
    UINT32 ulReserved8;                                             // 0408
    UINT32 ulTxAtmStatus;                                           // 040c
    UINT32 ulTxAtmMpAalCfg;                                         // 0410
    UINT32 ulRxAtmCfg[AP_MAX_PORTS];                                // 0414
    UINT32 ulRxAtmStatus;                                           // 041c
    UINT32 ulRxAalCfg;                                              // 0420
    UINT32 ulRxAalMaxSdu;                                           // 0424
    UINT32 ulRxAalStatus;                                           // 0428
    UINT32 ulUtopiaCfg;                                             // 042c
    UINT32 ulAdslLedCtrl; // [BCM6348 Only]                         // 0430
    UINT32 ulReserved9[3];                                          // 0434
    UINT32 ulTxAal5CellCnt[AP_MAX_PORTS];                           // 0440
    UINT32 ulTxAal0CellCnt[AP_MAX_PORTS];                           // 0448
    UINT32 ulRxAal5CellCnt[AP_MAX_PORTS];                           // 0450
    UINT32 ulRxAal0CellCnt[AP_MAX_PORTS];                           // 0458
    UINT32 ulReserved10[40];                                        // 0460
    UINT32 ulTxVpiVciTable[AP_MAX_SHAPERS];                         // 0500
    UINT32 ulReserved11[56];                                        // 0520
    UINT32 ulRxVpiVciCam[AP_MAX_VCIDS * 2];                         // 0600
    UINT32 ulReserved12[AP_RESERVED];                               // 0640
    UINT32 ulSstCtrl[AP_MAX_SHAPERS];                               // 0700
    UINT32 ulSstVbr[AP_MAX_SHAPERS];                                // 0720
    UINT32 ulSstMcr[AP_MAX_SHAPERS]; // [BCM6348 Only]              // 0740
    UINT32 ulReserved13[40];                                        // 0760
    UINT32 ulTxAdslDmaDescrTableP0[64];                             // 0800
    UINT32 ulTxAdslDmaDescrTableP1[64];                             // 0900
    UINT32 ulRxAdslDmaDescrTableP0[64];                             // 0a00
    UINT32 ulRxAdslDmaDescrTableP1[64];                             // 0b00
} ATM_PROCESSOR_REGISTERS, *PATM_PROCESSOR_REGISTERS;

#elif defined(CONFIG_BCM96338)
#define ATM_PROCESSOR_BASE                                       0xfffe2000
typedef struct AtmProcessorRegisters
{
    UINT32 ulAdslLastDescBaseAddr;                                  // 0000
    UINT32 ulAdslLastDescCfg;                                       // 0004
    UINT32 ulAdslLastDescCellCnts;                                  // 0008
    UINT32 ulIrqStatus;                                             // 000c
    UINT32 ulIrqMask;                                               // 0010
    UINT32 ulIrqRxFreeCellQWatermark;                               // 0014
    UINT32 ulIrqRxFreePktQWatermark;                                // 0018
    UINT32 ulIrqRxQWatchdog;                                        // 001c
    UINT32 ulRxAtmCfg[AP_MAX_PORTS];                                // 0020
    UINT32 ulRxAtmStatus;                                           // 0028
    UINT32 ulReserved1;                                             // 002c
    UINT32 ulRxAalCfg;                                              // 0030
    UINT32 ulRxAalMaxSdu;                                           // 0034
    UINT32 ulRxAalStatus;                                           // 0038
    UINT32 ulReserved2;                                             // 003c
    UINT32 ulRxVpiVciCam[AP_RX_HW_VCIDS * 2];                       // 0040
    UINT32 ulReserved3[8];                                          // 0060
    UINT32 ulFreePktQAddr;                                          // 0080
    UINT32 ulFreePktQHead;                                          // 0084
    UINT32 ulFreePktQTail;                                          // 0088
    UINT32 ulFreePktQLen;                                           // 008c
    UINT32 ulRxPktQAddr;                                            // 0090
    UINT32 ulRxPktQHead;                                            // 0094
    UINT32 ulRxPktQTail;                                            // 0098
    UINT32 ulRxPktQLen;                                             // 009c
    UINT32 ulFreeCellQAddr;                                         // 00a0
    UINT32 ulFreeCellQHead;                                         // 00a4
    UINT32 ulFreeCellQTail;                                         // 00a8
    UINT32 ulFreeCellQLen;                                          // 00ac
    UINT32 ulRxCellQAddr;                                           // 00b0
    UINT32 ulRxCellQHead;                                           // 00b4
    UINT32 ulRxCellQTail;                                           // 00b8
    UINT32 ulRxCellQLen;                                            // 00bc
    UINT32 ulRxChBusyStatus;                                        // 00c0
    UINT32 ulRxChActiveStatus;                                      // 00c4
    UINT32 ulRxFlushBd;                                             // 00c8
    UINT32 ulDmaCfg;                                                // 00cc
    UINT32 ulReserved4[12];                                         // 00d0
    UINT32 ulRxAdslDmaDescrTableP0[64];                             // 0100
    UINT32 ulRxAdslDmaDescrTableP1[64];                             // 0200
} ATM_PROCESSOR_REGISTERS, *PATM_PROCESSOR_REGISTERS;
#endif

#define AP_REGS ((volatile PATM_PROCESSOR_REGISTERS const) ATM_PROCESSOR_BASE)

// ADSL register that is used to determine if the ADSL core has enabled one or
// two DMA ports.
#define ADSL_INTERNAL_PORTS_REG         ((UINT32 *) 0xfff55000)
#define ADSL_PORT1_EN                   0x30

#endif

