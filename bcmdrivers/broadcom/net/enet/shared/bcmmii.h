/*
<:copyright-broadcom 
 
 Copyright (c) 2004 Broadcom Corporation 
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
#ifndef _BCMMII_H_
#define _BCMMII_H_

/*start of ÒÔÌ«Íø 3.1.5 porting by l39225 20060504*/
/* MII Auxiliary control/status register */
#define SP100_INDICATOR     0x0002  /* Speed indication */

#define FULLDUPLEX_INDICATOR 0x0001
#define FORCESPEED_INDICATOR 0x0004
#define AN_INDICATOR         0x0008

/* MII Control register */
#define MII_CR_FD_MODE       0x0100
#define MII_CR_SPEED_100M    0x2000
/*start of ÒÔÌ«Íø 3.1.5 porting by l39225 20060504*/

/*---------------------------------------------------------------------*/
/* Broadcom PHY MII register address                                   */
/* use when PhyType is BP_ENET_INTERNAL_PHY                            */
/*---------------------------------------------------------------------*/
#define MII_AUX_CTRL_STATUS                 0x18
#define MII_TX_CONTROL                      0x19
#define MII_INTERRUPT                       0x1A
#define MII_AUX_STATUS3                     0x1C
#define MII_BRCM_TEST                       0x1f

/* MII Auxiliary control/status register */
#define MII_AUX_CTRL_STATUS_FULL_DUPLEX     0x0001
#define MII_AUX_CTRL_STATUS_SP100_INDICATOR 0x0002  /* Speed indication */

/* MII TX Control register. */
#define MII_TX_CONTROL_PGA_FIX_ENABLE       0x0100

/* MII Interrupt register. */
#define MII_INTR_ENABLE                     0x4000
#define MII_INTR_MASK_FDX                   0x0800
#define MII_INTR_MASK_LINK_SPEED            0x0400

/* MII Auxilliary Status 3 register. */
#define MII_AUX_STATUS3_MSE_MASK            0xFF00

/* Broadcom Test register. */
#define MII_BRCM_TEST_HARDRESET             0x0200
#define MII_BRCM_TEST_SHADOW_ENABLE         0x0080
#define MII_BRCM_TEST_10BT_SERIAL_NODRIB    0x0008
#define MII_BRCM_TEST_100TX_POWERDOWN       0x0002
#define MII_BRCM_TEST_10BT_POWERDOWN        0x0001

/* Advertisement control register. */
#define ADVERTISE_FDFC                      0x0400  /* MII advertisement register Flow Control */


/*---------------------------------------------------------------------*/
/* 5325 Switch Page Register                                           */
/* use when PhyType is BP_ENET_EXTERNAL_SWITCH                         */
/*---------------------------------------------------------------------*/
#define PAGE_CONTROL                        0x00
#define PAGE_MANAGEMENT                     0x02
#define PAGE_SELECT                         0xff
#define PAGE_PORT_VLAN                 0x31

/* Control page registers */
#define REG_MII_PORT_CONTROL                0x08
#define REG_SWITCH_MODE                     0x0b
#define REG_CONTROL_MII1_PORT_STATE_OVERRIDE 0x0e
#define REG_POWER_DOWN_MODE                 0x0f

/* MII Port Control Register Page 0x00 Address 0x08 */
#define REG_MII_PORT_CONTROL_RX_UCST_EN     0x10
#define REG_MII_PORT_CONTROL_RX_MCST_EN     0x08
#define REG_MII_PORT_CONTROL_RX_BCST_EN     0x04

/* Switch Mode Register Page 0x00 Address 0x0b */
#define REG_SWITCH_MODE_FRAME_MANAGE_MODE   0x01
#define REG_SWITCH_MODE_SW_FWDG_EN          0x02

/* MII1 Port State Override Register Page 0x00 Address 0x0e */
#define REG_CONTROL_MPSO_MII_SW_OVERRIDE    0x80
#define REG_CONTROL_MPSO_REVERSE_MII        0x10
#define REG_CONTROL_MPSO_LP_FLOW_CONTROL    0x08
#define REG_CONTROL_MPSO_SPEED100           0x04
#define REG_CONTROL_MPSO_FDX                0x02
#define REG_CONTROL_MPSO_LINKPASS           0x01

/* Power down mode register Page 0x00 Address 0x0f */
#define REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE     0x01
#define REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE     0x02
#define REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE     0x04
#define REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE     0x08
#define REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE     0x10

/* Protected Mode Register Page 0x00, Address 0x26 */
#define REG_PROTECTED_MAP                   0x26

/* Global Management Register Page 0x02 Address 0x00 */
#define REG_GLOBAL_CONFIG                   0x00

/*---------------------------------------------------------------------*/
/* 5325 Switch SPI Interface                                           */
/* use when configuration type is BP_ENET_CONFIG_SPI_SSB_x             */
/*---------------------------------------------------------------------*/
#define BCM5325_SPI_CMD_LEN                 1
#define BCM5325_SPI_ADDR_LEN                1
#define BCM5325_SPI_PREPENDCNT              (BCM5325_SPI_CMD_LEN+BCM5325_SPI_ADDR_LEN)

/* 5325 SPI Status Register */
#define BCM5325_SPI_STS                     0xfe

/* 5325 SPI Status Register definition */
#define BCM5325_SPI_CMD_RACK                0x20

/* 5325 Command Byte definition */
#define BCM5325_SPI_CMD_READ                0x00    /* bit 0 - Read/Write */
#define BCM5325_SPI_CMD_WRITE               0x01    /* bit 0 - Read/Write */
#define BCM5325_SPI_CHIPID_MASK             0x7     /* bit 3:1 - Chip ID */
#define BCM5325_SPI_CHIPID_SHIFT            1
#define BCM5325_SPI_CMD_NORMAL              0x60    /* bit 7:4 - Mode */
#define BCM5325_SPI_CMD_FAST                0x10    /* bit 4 - Mode */

/*---------------------------------------------------------------------*/
/* 5325 Switch Pseudo PHY MII Register                                 */
/* use when configuration type is BP_ENET_CONFIG_MDIO_PSEUDO_PHY       */
/*---------------------------------------------------------------------*/
#define PSEUDO_PHY_ADDR             0x1e    /* Pseduo PHY address */

/* Pseudo PHY MII registers */
#define REG_PSEUDO_PHY_MII_REG16    0x10    /* register 16 - Switch Register Set Access Control Register */
#define REG_PSEUDO_PHY_MII_REG17    0x11    /* register 17 - Switch Register Set Read/Write Control Register */
#define REG_PSEUDO_PHY_MII_REG24    0x18    /* register 24 - Switch Accesss Register bit 15:0 */
#define REG_PSEUDO_PHY_MII_REG25    0x19    /* register 25 - Switch Accesss Register bit 31:16 */
#define REG_PSEUDO_PHY_MII_REG26    0x20    /* register 26 - Switch Accesss Register bit 47:32 */
#define REG_PSEUDO_PHY_MII_REG27    0x21    /* register 27 - Switch Accesss Register bit 63:48 */

/*Pseudo PHY MII register 16 Switch Register Set Access Control Register */
#define REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT  8       /* bit 8..15 - switch page number */
#define REG_PPM_REG16_MDIO_ENABLE               0x01    /* bit 0 - set MDC/MDIO access enable */

/*Pseudo PHY MII register 17 Switch Register Set Read/Write Control Register */
#define REG_PPM_REG17_REG_NUMBER_SHIFT          8       /* bit 8..15 - switch register number */
#define REG_PPM_REG17_OP_DONE       0x00    /* bit 0..1 - no operation */
#define REG_PPM_REG17_OP_WRITE      0x01    /* bit 0..1 - write operation */
#define REG_PPM_REG17_OP_READ       0x02    /* bit 0..1 - read operation */

/*---------------------------------------------------------------------*/
/* 5325 Switch ARL Access Register Configuration                       */
/*---------------------------------------------------------------------*/
/* Page numbers */
#define PAGE_ARL_ACCESS             0x05    /* ARL Access page */
/* ARL Access registers */
#define REG_ARL_ACCESS_CONTROL      0x00    /* ARL Read/Write Control register */
#define REG_ARL_ACCESS_ADDR_INDEX   0x02    /* MAC Address Index register */
#define REG_ARL_VID_TABLE_INDEX     0x08    /* ARL Entry 1 register */
#define REG_ARL_ACCESS_ARL_ENTRY0   0x10    /* ARL Entry 0 register */
#define REG_ARL_ACCESS_ARL_ENTRY1   0x18    /* ARL Entry 1 register */

/* ARL Read/Write Control Register Page 0x05 Address 0x00 */
#define REG_ARL_CONTROL_READ        0x01
#define REG_ARL_CONTROL_START       0x80

/* ARL Entry 0/1 registers */
#define REG_ARL_ENTRY_VALID         0x80
#define REG_ARL_ENTRY_STATIC        0x40
#define REG_ARL_ENTRY_PORTID_MASK   0x0f
#define REG_ARL_ENTRY_PORTID_0      0x00
#define REG_ARL_ENTRY_PORTID_1      0x01
#define REG_ARL_ENTRY_PORTID_2      0x02
#define REG_ARL_ENTRY_PORTID_3      0x03
#define REG_ARL_ENTRY_PORTID_4      0x04
#define REG_ARL_ENTRY_PORTID_MII    0x08

/*---------------------------------------------------------------------*/
/* 5325 Switch Link Status Summary                                     */
/*---------------------------------------------------------------------*/
#define REG_LINK_SUMMARY 0x00

/*---------------------------------------------------------------------*/
/* 5325e Switch VLAN Register Configuration                            */
/*---------------------------------------------------------------------*/
/* Page numbers */
#define PAGE_VLAN       0x34    /* VLAN page */
/* VLAN page registers */
#define REG_VLAN_CTRL0  0x00    /* VLAN Control 0 register */
#define REG_VLAN_CTRL1  0x01    /* VLAN Control 1 register */
#define REG_VLAN_CTRL2  0x02    /* VLAN Control 0 register */
#define REG_VLAN_CTRL3  0x03    /* VLAN Control 0 register */
#define REG_VLAN_CTRL4  0x04    /* VLAN Control 4 register */
#define REG_VLAN_CTRL5  0x05    /* VLAN Control 5 register */
#define REG_VLAN_ACCESS 0x06    /* VLAN Table Access register */
#define REG_VLAN_WRITE  0x08    /* VLAN Write register */
#define REG_VLAN_READ   0x0C    /* VLAN Read register */
#define REG_VLAN_PTAG0  0x10    /* VLAN Default Port Tag register - port 0 */
#define REG_VLAN_PTAG1  0x12    /* VLAN Default Port Tag register - port 1 */
#define REG_VLAN_PTAG2  0x14    /* VLAN Default Port Tag register - port 2 */
#define REG_VLAN_PTAG3  0x16    /* VLAN Default Port Tag register - port 3 */
#define REG_VLAN_PTAG4  0x18    /* VLAN Default Port Tag register - port 4 */
#define REG_VLAN_PTAG5  0x1A    /* VLAN Default Port Tag register - MII port */
#define REG_VLAN_PMAP   0x20    /* VLAN Priority Re-map register */
#define MII_PORT            5

/* VLAN Control 0 Register Page 0x34 Address 0x00 */
#define REG_VLAN_CTRL0_ENABLE_1Q          (1 << 7) /* 802.1Q VLAN Enable            */
#define REG_VLAN_CTRL0_SVLM               (0 << 5) /* Shared VLAN Learning mode     */
#define REG_VLAN_CTRL0_IVLM               (3 << 5) /* Individual VLAN Learning mode */
#define REG_VLAN_CTRL0_FR_CTRL_CHG_PRI    (1 << 2) /* 01 Change priority            */
#define REG_VLAN_CTRL0_FR_CTRL_CHG_VID    (2 << 2) /* 10 Change VID                 */

/* VLAN Control 3 Register Page 0x34 Address 0x03 */
#define REG_VLAN_CTRL3_8BIT_CHECK         (1 << 7) /* Check high order 8 bits in VID */
#define REG_VLAN_CTRL3_MAXSIZE_1532       (1 << 6)
#define REG_VLAN_CTRL3_MII_DROP_NON_1Q    (0 << 5)
#define REG_VLAN_CTRL3_DROP_NON_1Q_SHIFT  0

/* VLAN Control 5 Register Page 0x34 Address 0x05 */
#define REG_VLAN_CTRL5_VID_HIGH_8BIT_NOT_CHECKED (1 << 5)
#define REG_VLAN_CTRL5_APPLY_BYPASS_VLAN         (1 << 4)
#define REG_VLAN_CTRL5_DROP_VTAB_MISS            (1 << 3)   /* Drop frame when V_table miss  */
#define REG_VLAN_CTRL5_ENBL_MANAGE_RX_BYPASS     (1 << 1)
#define REG_VLAN_CTRL5_ENBL_CRC_GEN              (1 << 0)

/* VLAN Table Access Register Page 0x34 Address 0x06-0x07 */
#define REG_VLAN_ACCESS_START_DONE      (1 << 13)    /* Start/Done COmmand */
#define REG_VLAN_ACCESS_WRITE_STATE     (1 << 12)    /* Write State        */
#define REG_VALN_ACCESS_HIGH8_VID_SHIFT 4
#define REG_VALN_ACCESS_LOW4_VID_SHIFT  0

/* VLAN Write Register Page 0x34 Address 0x08-0x0B */
#define REG_VLAN_WRITE_VALID         (1 << 20) /* Valid write */
#define REG_VLAN_HIGH_8BIT_VID_SHIFT 12
#define REG_VLAN_UNTAG_SHIFT         6
#define REG_VLAN_GROUP_SHIFT         0

#define VLAN_NUMVLANS   16      /* # of VLANs */
#define VLAN_MAXVID     5       /* Max VLAN ID */

#define SPI_STATUS_OK                       0
#define SPI_STATUS_INVALID_LEN              -1

extern int mii_init(struct net_device *dev);
extern int mii_read(struct net_device *dev, int phy_id, int location);
extern void mii_write(struct net_device *dev, int phy_id, int location, int data);
extern int mii_linkstatus(struct net_device *dev, int phy_id);
void mii_linkstatus_start(struct net_device *dev, int phy_id);
int mii_linkstatus_check(struct net_device *dev, int *up);
extern void mii_enablephyinterrupt(struct net_device *dev, int phy_id);
extern void mii_clearphyinterrupt(struct net_device *dev, int phy_id);
extern int ethsw_config_vlan(struct net_device *dev, int enable, unsigned int vid);
extern void mii_switch_power_off(void* context);
extern void mii_switch_frame_manage_mode(struct net_device *dev);
extern void mii_switch_unmanage_mode(struct net_device *dev);
extern void mii_switch_port_vlan_base_mode(struct net_device *dev,unsigned short vlan);
extern void mii_switch_unPort_vlan_base_mode(struct net_device *dev);
extern int mii_dump_page(struct net_device *dev, uint16 page, unsigned char *buf);

#if defined (CONFIG_BCM96338)
extern void phy_tuning_mse(struct net_device *dev);
#endif
extern int ethsw_macaddr_to_portid(struct net_device *dev, uint8 *data, int len);
extern int ethsw_link_status(struct net_device *dev);

#endif /* _BCMMII_H_ */
