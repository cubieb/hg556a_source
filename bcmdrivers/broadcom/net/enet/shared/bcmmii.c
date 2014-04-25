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
//**************************************************************************
// File Name  : bcmmii.c
//
// Description: Broadcom PHY/Ethernet Switch Configuration
//               
//**************************************************************************

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/stddef.h>

#include "bcmenet.h"

#include <bcm_map.h>
#include "bcmenet.h"
#include "bcmmii.h"
#include "ifonmii.h"

typedef enum {
    MII_100MBIT     = 0x0001,
    MII_FULLDUPLEX  = 0x0002,
    MII_AUTONEG     = 0x0004,
}   MII_CONFIG;

#define EMAC_MDC            0x1f

/* local prototypes */
static MII_CONFIG mii_getconfig(struct net_device *dev);
static MII_CONFIG mii_autoconfigure(struct net_device *dev);
static void mii_setup(struct net_device *dev);
static void mii_soft_reset(struct net_device *dev, int PhyAddr);
#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
static int spi_read( int spi_ss, unsigned char *msg_buf, int nbytes );
static int spi_write(int spi_ss, unsigned char *msg_buf, int nbytes );
static int ethsw_spi_ss_id(struct net_device *dev);
static void ethsw_spi_select(struct net_device *dev, int page);
static void ethsw_spi_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
static void ethsw_spi_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
#endif
static void ethsw_mdio_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
static void ethsw_mdio_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
void ethsw_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
static void ethsw_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len);
static int ethsw_switch_type(struct net_device *dev);
#if defined (CONFIG_BCM96338)
static void pga_fix_enable(struct net_device *dev);
#endif
#ifdef PHY_LOOPBACK
static void mii_loopback(struct net_device *dev);
#endif


/* read a value from the MII */
int mii_read(struct net_device *dev, int phy_id, int location) 
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac = pDevCtrl->emac;

    emac->mdioData = MDIO_RD | (phy_id << MDIO_PMD_SHIFT) | (location << MDIO_REG_SHIFT);
    mdelay(1);
    while ( ! (emac->intStatus & EMAC_MDIO_INT) );
    emac->intStatus |= EMAC_MDIO_INT;
    return emac->mdioData & 0xffff;
}

/* write a value to the MII */
void mii_write(struct net_device *dev, int phy_id, int location, int val)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac = pDevCtrl->emac;
    uint32 data;

    data = MDIO_WR | (phy_id << MDIO_PMD_SHIFT) | (location << MDIO_REG_SHIFT) | val;
    emac->mdioData = data;
    mdelay(1);
    while ( ! (emac->intStatus & EMAC_MDIO_INT) );
    emac->intStatus |= EMAC_MDIO_INT;
}

#ifdef PHY_LOOPBACK
/* set the MII loopback mode */
static void mii_loopback(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint32 val;

    TRACE(("mii_loopback\n"));

    val = mii_read(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR);
    /* Disable autonegotiation */
    val &= ~BMCR_ANENABLE;
    /* Enable Loopback */
    val |= BMCR_LOOPBACK;
    mii_write(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR, val);
}
#endif

int mii_linkstatus(struct net_device *dev, int phy_id)
{
    int val;

    val = mii_read(dev, phy_id, MII_BMSR);
    /* reread: the link status bit is sticky */
    val = mii_read(dev, phy_id, MII_BMSR);

    if (val & BMSR_LSTATUS)
        return 1;
    else
        return 0;
}

void mii_linkstatus_start(struct net_device *dev, int phy_id)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac = pDevCtrl->emac;
    int location = MII_BMSR;

    emac->mdioData = MDIO_RD | (phy_id << MDIO_PMD_SHIFT) |
        (location << MDIO_REG_SHIFT);
}

int mii_linkstatus_check(struct net_device *dev, int *up)
{
    int done = 0;
    int val;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac = pDevCtrl->emac;

    if( emac->intStatus & EMAC_MDIO_INT ) {

        emac->intStatus |= EMAC_MDIO_INT;
        val = emac->mdioData & 0xffff;
        *up = ((val & BMSR_LSTATUS) == BMSR_LSTATUS) ? 1 : 0;
        done = 1;
    }

    return( done );
}

void mii_enablephyinterrupt(struct net_device *dev, int phy_id)
{
    mii_write(dev, phy_id, MII_INTERRUPT, 
        MII_INTR_ENABLE | MII_INTR_MASK_FDX | MII_INTR_MASK_LINK_SPEED);
}

void mii_clearphyinterrupt(struct net_device *dev, int phy_id)
{
    mii_read(dev, phy_id, MII_INTERRUPT);
}

/* return the current MII configuration */
static MII_CONFIG mii_getconfig(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint32 val;
    MII_CONFIG eConfig = 0;

    TRACE(("mii_getconfig\n"));

    /* Read the Link Partner Ability */
    val = mii_read(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_LPA);
    if (val & LPA_100FULL) {          /* 100 MB Full-Duplex */
        eConfig = (MII_100MBIT | MII_FULLDUPLEX);
    } else if (val & LPA_100HALF) {   /* 100 MB Half-Duplex */
        eConfig = MII_100MBIT;
    } else if (val & LPA_10FULL) {    /* 10 MB Full-Duplex */
        eConfig = MII_FULLDUPLEX;
    } 

    return eConfig;
}

/* Auto-Configure this MII interface */
static MII_CONFIG mii_autoconfigure(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int i;
    int val;
    MII_CONFIG eConfig;

    TRACE(("mii_autoconfigure\n"));

    /* enable and restart autonegotiation */
    val = mii_read(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR);
    val |= (BMCR_ANRESTART | BMCR_ANENABLE);
    mii_write(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR, val);

    /* wait for it to finish */
    for (i = 0; i < 1000; i++) {
        mdelay(1);
        val = mii_read(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMSR);
        if (val & BMSR_ANEGCOMPLETE) {
            break;
        }
    }

    eConfig = mii_getconfig(dev);
    if (val & BMSR_ANEGCOMPLETE) {
        eConfig |= MII_AUTONEG;
    } 

    return eConfig;
}

static void mii_setup(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    MII_CONFIG eMiiConfig;

    eMiiConfig = mii_autoconfigure(dev);

    if (! (eMiiConfig & MII_AUTONEG)) {
        printk(KERN_INFO CARDNAME ": Auto-negotiation timed-out\n");
    }

    if ((eMiiConfig & (MII_100MBIT | MII_FULLDUPLEX)) == (MII_100MBIT | MII_FULLDUPLEX)) {
        printk(KERN_INFO CARDNAME ": 100 MB Full-Duplex (auto-neg)\n");
    } else if (eMiiConfig & MII_100MBIT) {
        printk(KERN_INFO CARDNAME ": 100 MB Half-Duplex (auto-neg)\n");
    } else if (eMiiConfig & MII_FULLDUPLEX) {
        printk(KERN_INFO CARDNAME ": 10 MB Full-Duplex (auto-neg)\n");
    } else {
        printk(KERN_INFO CARDNAME ": 10 MB Half-Duplex (assumed)\n");
    }

#ifdef PHY_LOOPBACK
    /* Enable PHY loopback */
    mii_loopback(dev);
#endif

    /* Check for FULL/HALF duplex */
    if (eMiiConfig & MII_FULLDUPLEX) {
        pDevCtrl->emac->txControl = EMAC_FD;
    }
}

/* reset the MII */
static void mii_soft_reset(struct net_device *dev, int PhyAddr) 
{
    int val;

    mii_write(dev, PhyAddr, MII_BMCR, BMCR_RESET);
    udelay(10); /* wait ~10usec */
    do {
        val = mii_read(dev, PhyAddr, MII_BMCR);
    } while (val & BMCR_RESET);

}

#if defined (CONFIG_BCM96338)
static void pga_fix_enable(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int val;
    int phyaddr;

    phyaddr = pDevCtrl->EnetInfo.ucPhyAddress;
    val = (MII_BRCM_TEST_HARDRESET |
           MII_BRCM_TEST_10BT_SERIAL_NODRIB |
           MII_BRCM_TEST_100TX_POWERDOWN |
           MII_BRCM_TEST_10BT_POWERDOWN );
    mii_write(dev, phyaddr, MII_BRCM_TEST, val); //  reset phy
    val = mii_read(dev, phyaddr, MII_BRCM_TEST);
    val |= MII_BRCM_TEST_SHADOW_ENABLE;
    mii_write(dev, phyaddr, MII_BRCM_TEST, val); //  shadow mode
    val = mii_read(dev, phyaddr, MII_TX_CONTROL);
    val |= MII_TX_CONTROL_PGA_FIX_ENABLE;
    mii_write(dev, phyaddr, MII_TX_CONTROL, val); // pga fix enable
    val = mii_read(dev, phyaddr, MII_BRCM_TEST);
    val &= ~MII_BRCM_TEST_SHADOW_ENABLE;
    mii_write(dev, phyaddr, MII_BRCM_TEST, val); //  shadow mode
}

/* phy_tuning_mse */
void phy_tuning_mse(struct net_device *dev) 
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac = pDevCtrl->emac;
    int phyaddr;
    int val;
    int i = 0;
    int retry = 0;

    phyaddr = pDevCtrl->EnetInfo.ucPhyAddress;
    val = mii_read(dev, phyaddr, MII_BRCM_TEST);
    val |= MII_BRCM_TEST_SHADOW_ENABLE;
    mii_write(dev, phyaddr, MII_BRCM_TEST, val); // enabled shadow mode

    do {
        i++;
        val = mii_read(dev, phyaddr, MII_AUX_STATUS3); // read the MSE value
        if ((val & MII_AUX_STATUS3_MSE_MASK) >= 0x4000) {
            val = mii_read(dev, phyaddr, MII_TX_CONTROL);
            val &= ~MII_TX_CONTROL_PGA_FIX_ENABLE;
            mii_write(dev, phyaddr, MII_TX_CONTROL, val); // pga fix disable
            udelay(100); /* wait ~100usec */
            val |= MII_TX_CONTROL_PGA_FIX_ENABLE;
            mii_write(dev, phyaddr, MII_TX_CONTROL, val); // pga fix enable
            i = 0;
            retry++;
        }
        if ((i > 12) || (retry > 2)) // read twelve times to ensure good 
            break;
    } while (1);

    val = mii_read(dev, phyaddr, MII_BRCM_TEST);
    val &= ~MII_BRCM_TEST_SHADOW_ENABLE;
    mii_write(dev, phyaddr, MII_BRCM_TEST, val); // disabled shadow mode
}
#endif

#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
/* BCM5325 register access through SPI */
static int spi_read( int spi_ss, unsigned char *msg_buf, int nbytes )
{
    int i;

    SPI->spiMsgCtl = (nbytes << SPI_BYTE_CNT_SHIFT |
                            HALF_DUPLEX_R << SPI_MSG_TYPE_SHIFT);
    
    for (i = 0; i < BCM5325_SPI_PREPENDCNT; i++)
	    SPI->spiMsgData[i] = msg_buf[i];

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                    spi_ss << SPI_CMD_DEVICE_ID_SHIFT | 
                    BCM5325_SPI_PREPENDCNT << SPI_CMD_PREPEND_BYTE_CNT_SHIFT );

    while (!(SPI->spiIntStatus & SPI_INTR_CMD_DONE));

    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

    for(i = 0; i < nbytes; i++) {
	    msg_buf[i] = SPI->spiRxDataFifo[i];
    }

    return SPI_STATUS_OK;
}

static int spi_write(int spi_ss, unsigned char *msg_buf, int nbytes )
{
    int i;
    
    if (nbytes < BCM5325_SPI_PREPENDCNT) {
        return SPI_STATUS_INVALID_LEN;
    }

    SPI->spiMsgCtl = (nbytes << SPI_BYTE_CNT_SHIFT |
                    HALF_DUPLEX_W << SPI_MSG_TYPE_SHIFT);

    for (i = 0; i < nbytes; i++)
	    SPI->spiMsgData[i] = msg_buf[i];

    SPI->spiCmd = (SPI_CMD_START_IMMEDIATE << SPI_CMD_COMMAND_SHIFT | 
                    spi_ss << SPI_CMD_DEVICE_ID_SHIFT | 
                    0 << SPI_CMD_PREPEND_BYTE_CNT_SHIFT );

    while (!(SPI->spiIntStatus & SPI_INTR_CMD_DONE));

    SPI->spiIntStatus = SPI_INTR_CLEAR_ALL;

    return SPI_STATUS_OK;
}

static int ethsw_spi_ss_id(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int slave_select;

    switch(pDevCtrl->EnetInfo.usConfigType) {
        case BP_ENET_CONFIG_SPI_SSB_0:
            slave_select = SPI_DEV_ID_0;
            break;
        case BP_ENET_CONFIG_SPI_SSB_1:
            slave_select = SPI_DEV_ID_1;
            break;
        case BP_ENET_CONFIG_SPI_SSB_2:
            slave_select = SPI_DEV_ID_2;
            break;
        case BP_ENET_CONFIG_SPI_SSB_3:
            slave_select = SPI_DEV_ID_3;
            break;
        default:
            slave_select = SPI_DEV_ID_1;
            break;
    }
    return slave_select;
}

static void ethsw_spi_select(struct net_device *dev, int page)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    ethsw_info_t *ethsw = &pDevCtrl->ethSwitch;
    unsigned char buf[3];
    int spi_ss;

    if (ethsw->cid == pDevCtrl->EnetInfo.ucPhyAddress && ethsw->page == page)
        return;

    spi_ss = ethsw_spi_ss_id(dev);
    ethsw->cid = pDevCtrl->EnetInfo.ucPhyAddress;
    ethsw->page = page;
    /* Select new chip */
    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_WRITE |
        ((ethsw->cid & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);

    /* Select new page */
    buf[1] = PAGE_SELECT;
    buf[2] = (char)ethsw->page;
    spi_write(spi_ss, buf, sizeof(buf));
}

static void ethsw_spi_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    unsigned char buf[64];
    int rc;
    int i;
    int max_check_spi_sts;
    int spi_ss;

    ethsw_spi_select(dev, page);

    spi_ss = ethsw_spi_ss_id(dev);
    /* write command byte and register address */
    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
        ((pDevCtrl->ethSwitch.cid & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
    buf[1] = (unsigned char)reg;
    rc = spi_read(spi_ss, buf, 1);

    if (rc == SPI_STATUS_OK) {
        max_check_spi_sts = 0;
        do {
            /* write command byte and read spi_sts address */
            buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
                ((pDevCtrl->ethSwitch.cid & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
            buf[1] = (unsigned char)BCM5325_SPI_STS;
            rc = spi_read(spi_ss, buf, 1);
            if (rc == SPI_STATUS_OK) {
                /* check the bit 0 RACK bit is set */
                if (buf[0] & BCM5325_SPI_CMD_RACK) {
                    break;
                }
                mdelay(1);
            } else {
                break;
            }
        } while (max_check_spi_sts++ < 10);

        if (rc == SPI_STATUS_OK) {
            for (i = len-1; i >= 0; i--) {
                buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
                    ((pDevCtrl->ethSwitch.cid & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
                buf[1] = (unsigned char)0xf0;
                rc = spi_read(spi_ss, buf, 1);
                if (rc == SPI_STATUS_OK) {
                    /* Write the data out in LE format to the switch */
                    *(data + len - i - 1) = buf[0];
                }
            }
        }
   }
}

static void ethsw_spi_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    unsigned char buf[64];
    int i;
    int spi_ss;

    ethsw_spi_select(dev, page);

    spi_ss = ethsw_spi_ss_id(dev);
    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_WRITE |
        ((pDevCtrl->ethSwitch.cid & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);

    buf[1] = (char)reg;
    for (i = 0; i < len; i++) {
        /* Write the data out in LE format to the switch */
        buf[BCM5325_SPI_PREPENDCNT+i] = *(data + (len - i - 1));
    }
    spi_write( spi_ss, buf, len+BCM5325_SPI_PREPENDCNT);
}
#endif

/* BCM5325E register access through MDC/MDIO */
static void ethsw_mdio_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac; 
    int cmd, res, ret;
    int max_retry = 0;

    emac = pDevCtrl->emac;
    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        res = mii_read(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        udelay(10);
    } while ((max_retry++ < 5) &&
             ((res & (REG_PPM_REG17_OP_WRITE|REG_PPM_REG17_OP_READ)) != REG_PPM_REG17_OP_DONE));

    ret = 0;
    ret |= mii_read(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24) << 0;
    ret |= mii_read(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25) << 16;
    switch (len) {
        case 1:
            *data = (uint8)ret;
            break;
        case 2:
            *(uint16 *)data = (uint16)ret;
            break;
        case 4:
            *(uint32 *)data = ret;
            break;
    }
}

static void ethsw_mdio_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac; 
    uint32 cmd, res;
    int val = 0;
    int max_retry = 0;

    emac = pDevCtrl->emac;
    switch (len) {
        case 1:
            val = *data;
            break;
        case 2:
            val = *(uint16 *)data;
            break;
        case 4:
            val = *(uint32 *)data;
            break;
    }
    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = val>>0 & 0xffff;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, cmd);
    cmd = val>>16 & 0xffff;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, cmd);
    cmd = 0;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, cmd);
    cmd = 0;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    mii_write(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        res = mii_read(dev, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        udelay(10);
    } while ((max_retry++ < 5) &&
             ((res & (REG_PPM_REG17_OP_WRITE|REG_PPM_REG17_OP_READ)) != REG_PPM_REG17_OP_DONE));
}

void ethsw_rreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

    switch (pDevCtrl->EnetInfo.usConfigType) {
        case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
            ethsw_mdio_rreg(dev, page, reg, data, len);
            break;

        case BP_ENET_CONFIG_SPI_SSB_0:
        case BP_ENET_CONFIG_SPI_SSB_1:
        case BP_ENET_CONFIG_SPI_SSB_2:
        case BP_ENET_CONFIG_SPI_SSB_3:
#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
            ethsw_spi_rreg(dev, page, reg, data, len);
#endif
            break;
    }
}

static void ethsw_wreg(struct net_device *dev, int page, int reg, uint8 *data, int len)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

    switch (pDevCtrl->EnetInfo.usConfigType) {
        case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
            ethsw_mdio_wreg(dev, page, reg, data, len);
            break;

        case BP_ENET_CONFIG_SPI_SSB_0:
        case BP_ENET_CONFIG_SPI_SSB_1:
        case BP_ENET_CONFIG_SPI_SSB_2:
        case BP_ENET_CONFIG_SPI_SSB_3:
#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
            ethsw_spi_wreg(dev, page, reg, data, len);
#endif
            break;
    }
}

/* Configure the switch for VLAN support */
static int ethsw_switch_type(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint8 val8   = 0;
    int type;

    if ( pDevCtrl->EnetInfo.ucPhyType != BP_ENET_EXTERNAL_SWITCH ||
        ((pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_MDIO_PSEUDO_PHY) &&
         (pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_SPI_SSB_0) &&
         (pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_SPI_SSB_1) &&
         (pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_SPI_SSB_2) &&
         (pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_SPI_SSB_3)) )
         return ESW_TYPE_UNDEFINED;

    /*
     * Determine if the external PHY is connected to a 5325E, 5325F or 5325M
     * read CTRL REG 4 if the default value is 
     * 1 - 5325E
     * 3 - 5325F
     * else - 5325M
     */ 
    ethsw_rreg(dev, PAGE_VLAN, REG_VLAN_CTRL4, (char*)&val8, sizeof(val8));
    switch (val8)
    {
    case 1: /* BCM5325E */
        type = ESW_TYPE_BCM5325E;
        break;
    case 3: /* BCM5325F */
        type = ESW_TYPE_BCM5325F;
        break;
    default: /* default to BCM5325M */
        type = ESW_TYPE_BCM5325M;
    }
    return type;
}

/* Configure the switch for VLAN support */
int ethsw_config_vlan(struct net_device *dev, int enable, unsigned int vid)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint8 val8   = 0;
    uint16 val16 = 0;
    uint32 val32 = 0;
    uint port;
    int max_retry = 0;

    switch(pDevCtrl->ethSwitch.type) {
        case ESW_TYPE_BCM5325E:
        case ESW_TYPE_BCM5325F:
            break;
        default:
            return -EFAULT;
    }

    /* Enable 802.1Q VLAN and Individual VLAN learning mode */
    if (enable) {
        val8 = REG_VLAN_CTRL0_ENABLE_1Q | REG_VLAN_CTRL0_IVLM;
    }
    else
    {
        val8 = (uint8) ~REG_VLAN_CTRL0_ENABLE_1Q;
    }
    ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_CTRL0, (char *)&val8, sizeof(val8));

    /* Enable high order 8 bit VLAN table check and drop non .1Q frames for MII port */
    val8 = REG_VLAN_CTRL3_8BIT_CHECK | REG_VLAN_CTRL3_MII_DROP_NON_1Q;
    ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_CTRL3, (char *)&val8, sizeof(val8));

    /* Drop V_table miss frames */
    val8 = REG_VLAN_CTRL5_DROP_VTAB_MISS;
    val8 |= REG_VLAN_CTRL5_ENBL_MANAGE_RX_BYPASS;
    val8 |= REG_VLAN_CTRL5_ENBL_CRC_GEN;
    ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_CTRL5, (char *)&val8, sizeof(val8));

    for (port = 0; port < 5; port++) {
        /* default tag for every port */
        val16 = vid + port;
        ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_PTAG0 + port * sizeof(uint16), (char *)&val16, sizeof(val16));

        /* Program VLAN table for each VLAN */
        val32 = ((1 << MII_PORT) | (1 << port)) << REG_VLAN_GROUP_SHIFT;
        val32 |= (1 << port) << REG_VLAN_UNTAG_SHIFT;
        val32 |= REG_VLAN_WRITE_VALID;
        ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_WRITE, (char*)&val32, sizeof(val32));
        val16 = REG_VLAN_ACCESS_START_DONE | REG_VLAN_ACCESS_WRITE_STATE | (vid + port);
        ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_ACCESS, (char*)&val16, sizeof(val16));
        do {
            ethsw_rreg(dev, PAGE_VLAN, REG_VLAN_ACCESS, (char*)&val16, sizeof(val16));
            udelay(10);
        } while ((max_retry++ < 5) && (val16 & REG_VLAN_ACCESS_START_DONE));
    }

    return 0;
}

void mii_switch_power_off(void *context)
{
    struct net_device *dev = (struct net_device *)context;

    uint8 ports = 
        REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE |
        REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE |
        REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE |
        REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE |
        REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE;

    ethsw_wreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, &ports, sizeof(ports));
}

/* start 支持以太网口配置 by l68693 2009-02-17*/
void mii_port_power_off(void *context, int portnum)
{
    struct net_device *dev = (struct net_device *)context;
    uint8 ports = 0;

     printk("mii_port_power_off, port:%d\n",portnum);

    ethsw_rreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, &ports, sizeof(ports));
    switch (portnum)
    {
        case 0: 
            ports |= REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE;
            break;
        case 1:
            ports |= REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE;
            break;
        case 2:
            ports |= REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE;
            break;
        case 3:
            ports |= REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE;           
            break;
        default:
            return;
    }
    ethsw_wreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, &ports, sizeof(ports));
}

void mii_port_power_on(void *context, int portnum)
{
    struct net_device *dev = (struct net_device *)context;
    uint8 ports = 0;

     printk("mii_port_power_on, port:%d\n",portnum);

    ethsw_rreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, &ports, sizeof(ports));
    switch (portnum)
    {
        case 0: 
            ports &= (~REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE);
            break;
        case 1:
            ports &= (~REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE);
            break;
        case 2:
            ports &= (~REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE);
            break;
        case 3:
            ports &= (~REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE);           
            break;
        default:
            return;
    }
    ethsw_wreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, &ports, sizeof(ports));
}
/* end 支持以太网口配置 by l68693 2009-02-17*/


/*start of add lsw port separate by port_base vlan by 139225 2006-8-15*/
void mii_switch_port_vlan_base_mode(struct net_device *dev,unsigned short vlan)
{
    uint16 v16;
    uint8 i = 0;
    uint8 j = 0;
    uint8 uPort[5] ={0,0,0,0,0};
    int val = 0;

    BcmEnet_devctrl* pDevCtrl = NULL;

   /*add of support ADM6996M LSW by l39225 20061218*/
    if(dev == NULL)
    {
    	  printk("  dev is NULL\n");
    	  return;
    }
    
    pDevCtrl= netdev_priv(dev);
    if(pDevCtrl == NULL)
    {
    	  printk("  pDevCtrl is NULL\n");
    	  return;
    }

    for ( i= 0; i < 5; i++)
    {
        if( ((vlan >> i ) & 0x0001) > 0)
        {
             uPort[i] = 1;
        }
    }

   if ( ESW_TYPE_ADM6996M != pDevCtrl->ethSwitch.type)
   {
   	    for(i = 0; i < 5;i++)
	    {
	        for (j= i+1; j < 5; j++)
	        {
	             if ( (uPort[i] > 0) && (uPort[j] > 0) )
	             {
	                ethsw_rreg(dev, PAGE_PORT_VLAN, i*2, &v16, sizeof(v16));
	                v16 |= 0x0100; //forward only to MII port
	                v16 |= (0x0001 <<  j);
	                ethsw_wreg(dev, PAGE_PORT_VLAN, i*2, &v16, sizeof(v16));

	                ethsw_rreg(dev, PAGE_PORT_VLAN,  2*j, &v16, sizeof(v16));
	                v16 |= 0x0100;
	                v16 |= (0x0001 << i);
	                ethsw_wreg(dev, PAGE_PORT_VLAN,  2*j, &v16, sizeof(v16));
	             }
	             else if( (uPort[i] ==  0) && (uPort[j] == 0))
	             {
	                   continue;
	             }
	             else
	             {
	                  ethsw_rreg(dev, PAGE_PORT_VLAN, i*2, &v16, sizeof(v16));
	                  v16 |= 0x0100; //forward only to MII port
	                  v16 &= ~(0x0001 <<  j);
	                  ethsw_wreg(dev, PAGE_PORT_VLAN, i*2, &v16, sizeof(v16));

	                  ethsw_rreg(dev, PAGE_PORT_VLAN, 2*j, &v16, sizeof(v16));
	                  v16 |= 0x0100;
	                  v16 &= ~(0x0001 << i);
	                  ethsw_wreg(dev, PAGE_PORT_VLAN, 2*j, &v16, sizeof(v16));
	             }
	        }     
   	   }
   	    v16 = 0xffff;
           ethsw_wreg(dev, PAGE_PORT_VLAN , 0x10, &v16, sizeof(v16));
   }
   else
   {
	    for(i = 0; i < 5;i++)
	    {
	        for (j= i+1; j < 5; j++)
	        {
	             if ( (uPort[i] > 0) && (uPort[j] > 0) )
	             {
		               val = mii_read(dev,0,SWI_VLANFILTER_LOWREG0 +2*i);
		               val |= 0x20; //forward only to MII port
		               val |= (0x0001 <<  j);
		               mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*i , val);

		               val = mii_read(dev,0,SWI_VLANFILTER_LOWREG0 +2*j);
		               val |= 0x20; //forward only to MII port
		               val |= (0x0001 << i);
		               mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*j , val);
	    
	             }
	             else if( (uPort[i] ==  0) && (uPort[j] == 0))
	             {
	                   continue;
	             }
	             else
	             {
	                  val = mii_read(dev,0,SWI_VLANFILTER_LOWREG0 +2*i);
	                  val |= 0x020; //forward only to MII port
	                  val &= ~(0x0001 <<  j);
	                  mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*i , val);

	                  val = mii_read(dev,0,SWI_VLANFILTER_LOWREG0 +2*j);
	                  val |= 0x20;
	                  val &= ~(0x0001 << i);
	                  mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*j , val);
	             }
	        }
	    }
	    
	   val = 0x3f;
	   mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*5 , val); 	
   	}
}

void mii_switch_unPort_vlan_base_mode(struct net_device *dev)
{
    uint16 v16;
    uint8 i=0;

    BcmEnet_devctrl* pDevCtrl = NULL;

   /*add of support ADM6996M LSW by l39225 20061218*/
    if(dev == NULL)
    {
    	  printk("  dev is NULL\n");
    	  return;
    }
    
    pDevCtrl= netdev_priv(dev);
    if(pDevCtrl == NULL)
    {
    	  printk("  pDevCtrl is NULL\n");
    	  return;
    }


   if ( ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
   {
   	    int val;
   	    
	     val =  0x2f;
	     for (i =0;i<4;i++)
	     {
	 		mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*i , val);
	     }

	      val = 0x30;
	      mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*4 , val);

	      val = 0x3f;
	      mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*5 , val); 	
	    
   }
   /*end of support ADM6996M LSW by l39225 20061218*/
  else
  {
	    v16 = 0xffff; 
	    for (i = 0; i < 5 ; i++)
	    {
	         ethsw_wreg(dev, PAGE_PORT_VLAN, 2*i, &v16, sizeof(v16));
	    }
	    
	    v16 = 0xffff;
	    ethsw_wreg(dev, PAGE_PORT_VLAN, 0x10, &v16, sizeof(v16));
  }
}
/*end of add lsw port separate by port_base vlan by 139225 2006-8-15*/


void mii_switch_frame_manage_mode(struct net_device *dev)
{
    uint8 value;
    BcmEnet_devctrl* pDevCtrl = NULL;
    int i = 0;

   /*add of support ADM6996M LSW by l39225 20061218*/
    if(dev == NULL)
    {
    	  printk("  dev is NULL\n");
    	  return;
    }
    
    pDevCtrl= netdev_priv(dev);
    if(pDevCtrl == NULL)
    {
    	  printk("  pDevCtrl is NULL\n");
    	  return;
    }


   if ( ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
   {
   	    int val;
   	    
   	    val =  mii_read(dev,0,SWI_SYSTEM_CTRLREG3);
   	    val |= 0xe000;
   	    val &= 0xafff;
   	    val |=  0x1800;
	    mii_write(dev, 0, SWI_SYSTEM_CTRLREG3, val);

#if 0    //VALN TAG
	    val = mii_read(dev,0,SWI_ARP_CTRLREG );
	    val |=0x40;
	    mii_write(dev, 0, SWI_ARP_CTRLREG , val);
#endif

	    val = BRCM_TYPE;
	    mii_write(dev, 0, SWI_ADMTAG_ETHTYPE , val);

	    val = 0x1ff;
	    mii_write(dev, 0, SWI_SPCTAG_INSCTRL , val);


	     val =  0x2f;
	     for (i =0;i<4;i++)
	     {
	 		mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*i , val);
	     }

	      val = 0x30;
	      mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*4 , val);

	      val = 0x3f;
	      mii_write(dev, 0, SWI_VLANFILTER_LOWREG0 +2*5 , val); 
	    
   }
   /*end of support ADM6996M LSW by l39225 20061218*/
   else 
   {
	    ethsw_rreg(dev, PAGE_CONTROL, REG_SWITCH_MODE, &value, sizeof(value));
	    value |= REG_SWITCH_MODE_FRAME_MANAGE_MODE;
	    value |= REG_SWITCH_MODE_SW_FWDG_EN;
	    ethsw_wreg(dev, PAGE_CONTROL, REG_SWITCH_MODE, &value, sizeof(value));

	    ethsw_rreg(dev, PAGE_CONTROL, REG_MII_PORT_CONTROL, &value, sizeof(value));
	    value |= REG_MII_PORT_CONTROL_RX_UCST_EN;
	    value |= REG_MII_PORT_CONTROL_RX_MCST_EN;
	    value |= REG_MII_PORT_CONTROL_RX_BCST_EN;
	    ethsw_wreg(dev, PAGE_CONTROL, REG_MII_PORT_CONTROL, &value, sizeof(value));

	    value = 0x80;
	    ethsw_wreg(dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &value, sizeof(value));

	    ethsw_rreg(dev, PAGE_VLAN, REG_VLAN_CTRL5, &value, sizeof(value));
	    value |= REG_VLAN_CTRL5_ENBL_MANAGE_RX_BYPASS;
	    value |= REG_VLAN_CTRL5_ENBL_CRC_GEN;
	    ethsw_wreg(dev, PAGE_VLAN, REG_VLAN_CTRL5, &value, sizeof(value));
	    
	    /* start of y42304 20060809: 使能lanswitch接收BPDU报文 */
	    ethsw_rreg(dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &value, sizeof(value));
	    //value |= 0x82;
	    value |= 0x8a;
	    ethsw_wreg(dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &value, sizeof(value));
	    /* end of y42304 20060809: 使能lanswitch接收BPDU报文 */
   }

}

void mii_switch_unmanage_mode(struct net_device *dev)
{
    uint8 value;

    BcmEnet_devctrl* pDevCtrl = NULL;

   /*add of support ADM6996M LSW by l39225 20061218*/
    if(dev == NULL)
    {
    	  printk("  dev is NULL\n");
    	  return;
    }
    
    pDevCtrl= netdev_priv(dev);
    if(pDevCtrl == NULL)
    {
    	  printk("  pDevCtrl is NULL\n");
    	  return;
    }


   if ( ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
   {
   	    int val;
   	    
   	    val =  mii_read(dev,0,SWI_SYSTEM_CTRLREG3);
   	    val &= ~0x1800;
	    mii_write(dev, 0, SWI_SYSTEM_CTRLREG3, val);
	    
   }
   /*end of support ADM6996M LSW by l39225 20061218*/
  else 
  {
	    ethsw_rreg(dev, PAGE_CONTROL, REG_SWITCH_MODE, &value, sizeof(value));
	    value &= ~REG_SWITCH_MODE_FRAME_MANAGE_MODE;
	    ethsw_wreg(dev, PAGE_CONTROL, REG_SWITCH_MODE, &value, sizeof(value));

	    value = 0;
	    ethsw_wreg(dev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &value, sizeof(value));
  }
}

int mii_dump_page(struct net_device *dev, uint16 page, unsigned char *buf)
{
    unsigned char v;
    int len;
    int i;

    len = 0;
    len += sprintf(buf, "page %04x\n", page);

    for (i = 0; i < 256; i++)
    {
        if ((i % 16) == 0)
            len += sprintf(buf + len, "%02x: ", i);

        ethsw_rreg(dev, page, i, (char *)&v, 1);
        len += sprintf(buf + len, "%02x ", v);

        if ((i % 16) == 15)
            len += sprintf(buf + len, "\n");
    }

    return len;
}

int mii_init(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile EmacRegisters *emac;
    int data32;
    uint16 data16;
#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
    uint16 blkEnables;
#endif
    uint8 data8;
    int i;
    char *phytype = "";
    char *setup = "";

    

    switch(pDevCtrl->EnetInfo.ucPhyType) {
        case BP_ENET_INTERNAL_PHY:
            phytype = "Internal PHY";
            break;

        case BP_ENET_EXTERNAL_PHY:
            phytype = "External PHY";
            break;

        case BP_ENET_EXTERNAL_SWITCH:
            phytype = "Ethernet Switch";
            break;

        default:
            printk(KERN_INFO CARDNAME ": Unknown PHY type\n");
            return -1;
    }
    switch (pDevCtrl->EnetInfo.usConfigType) {
        case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
            setup = "MDIO Pseudo PHY Interface";
            break;

        case BP_ENET_CONFIG_SPI_SSB_0:
            setup = "SPI Slave Select 0";
            break;

        case BP_ENET_CONFIG_SPI_SSB_1:
            setup = "SPI Slave Select 1";
            break;

        case BP_ENET_CONFIG_SPI_SSB_2:
            setup = "SPI Slave Select 2";
            break;

        case BP_ENET_CONFIG_SPI_SSB_3:
            setup = "SPI Slave Select 3";
            break;

        case BP_ENET_CONFIG_MDIO:
            setup = "MDIO";
            break;

         /*start of support   infineon ADM6996M LSW by l39225 20061218*/
        case  BP_ENET_CONFIG_SMI:
        	setup = "SMI";
        	break;
         /*end of support   infineon ADM6996M LSW by l39225 20061218*/

        default:
            setup = "Undefined Interface";
            break;
    }
    printk("Config %s Through %s\n", phytype, setup);

    emac = pDevCtrl->emac;
    switch(pDevCtrl->EnetInfo.ucPhyType) {

        case BP_ENET_INTERNAL_PHY:
            /* init mii clock, do soft reset of phy, default is 10Base-T */
            emac->mdioFreq = EMAC_MII_PRE_EN | EMAC_MDC;
            /* reset phy */
            mii_soft_reset(dev, pDevCtrl->EnetInfo.ucPhyAddress);
#if defined(CONFIG_BCM96338)
            pga_fix_enable(dev);
#endif
            mii_setup(dev);
            break;

        case BP_ENET_EXTERNAL_PHY:
            emac->config |= EMAC_EXT_PHY;
            emac->mdioFreq = EMAC_MII_PRE_EN | EMAC_MDC;
            /* reset phy */
            if (pDevCtrl->EnetInfo.usGpioPhyReset != BP_NOT_DEFINED) {
                data16 = GPIO_NUM_TO_MASK(pDevCtrl->EnetInfo.usGpioPhyReset);
                GPIO->GPIODir |= data16;
                if (pDevCtrl->EnetInfo.usGpioPhyReset & BP_ACTIVE_LOW) {
                    GPIO->GPIOio &= ~data16;
                    udelay(400); /* hold > 150us */
                    GPIO->GPIOio |= data16;
                }
                else {
                    GPIO->GPIOio |= data16;
                    udelay(400); /* hold > 150us */
                    GPIO->GPIOio &= ~data16;
                }
                mdelay(1100); /* wait > 1 second */
            } else {
                mii_soft_reset(dev, pDevCtrl->EnetInfo.ucPhyAddress);
            }

            data32 = mii_read(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_ADVERTISE);
            data32 |= ADVERTISE_FDFC; /* advertise flow control capbility */
            mii_write(dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_ADVERTISE, data32);

            mii_setup(dev);
            break;

        case BP_ENET_EXTERNAL_SWITCH:
#if defined(CONFIG_BCM96358)
            GPIO->GPIOMode |= GPIO_MODE_EMAC2_MII_CLK_INV;
#endif
            emac->config |= EMAC_EXT_PHY;
            emac->mdioFreq = EMAC_MII_PRE_EN | EMAC_MDC;
            emac->txControl = EMAC_FD;
            switch (pDevCtrl->EnetInfo.usConfigType) 
            {
                case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
                    mii_soft_reset(dev, PSEUDO_PHY_ADDR);
                    break;

                case BP_ENET_CONFIG_SPI_SSB_0:
                case BP_ENET_CONFIG_SPI_SSB_1:
                case BP_ENET_CONFIG_SPI_SSB_2:
                case BP_ENET_CONFIG_SPI_SSB_3:
#if defined (CONFIG_BCM96338) || defined (CONFIG_BCM96348)
                    blkEnables = PERF->blkEnables;
                    if ((blkEnables & SPI_CLK_EN) == 0) {
                        blkEnables |= SPI_CLK_EN;
                        PERF->blkEnables = blkEnables;
                    }
                    pDevCtrl->ethSwitch.cid = 0xff;
                    pDevCtrl->ethSwitch.page = 0xff;
#endif
                    break;

                case BP_ENET_CONFIG_MDIO:
                    /* reset phy */
                    if (pDevCtrl->EnetInfo.numSwitchPorts) {
                        for (i = 0; i < pDevCtrl->EnetInfo.numSwitchPorts; i++) {
                            mii_soft_reset(dev, pDevCtrl->EnetInfo.ucPhyAddress | i);
                        }
                    }
                    return 0;

		  /*start of support   infineon ADM6996M LSW by l39225 20061218*/
		 case  BP_ENET_CONFIG_SMI:
		       if (pDevCtrl->EnetInfo.numSwitchPorts) 
		       {
                           for (i = 0; i < pDevCtrl->EnetInfo.numSwitchPorts; i++)
                           {
				    int val;
				    mii_write(dev, 0, SWI_PHYREG_START_ADDR+i*SWI_PHYREG_OFFSET, BMCR_RESET);
				    udelay(10); 
				    do 
				    {
				        val = mii_read(dev, 0, SWI_PHYREG_START_ADDR+i*SWI_PHYREG_OFFSET);
				    } while (val & BMCR_RESET);
                            }
                     }
		       break;
		   /*end of support   infineon ADM6996M LSW by l39225 20061218*/
 
                default:
                    printk(KERN_INFO CARDNAME ": Unknown PHY configuration type\n");
                    break;
            }
            if( pDevCtrl->EnetInfo.usConfigType != BP_ENET_CONFIG_SMI)
            {
	            if (pDevCtrl->EnetInfo.numSwitchPorts) {
	                data8 = 0;
	                switch (pDevCtrl->EnetInfo.numSwitchPorts) {
	                    case 5:
	                        data8 |= REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE;
	                    case 4:
	                        data8 |= REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE;
	                    case 3:
	                        data8 |= REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE;
	                    case 2:
	                        data8 |= REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE;
	                    case 1:
	                        /* 
	                         * y42304 delete: Do not set bit 0 to a 1 , Doing so 
	                         * disable pll power and lanswitch function 
	                         */
	                        //data8 |= REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE; 
	                        break;
	                    default:
	                        break;
	                }

	                /* start of enet y42304 modified 20060711: power down port 5*/
	                data8 |= REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE;
	                /* disable Switch port PHY */
	                ethsw_wreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, (uint8 *)&data8, sizeof(data8));
	                
	                /* enable Switch port 0-3 PHY */
	                data8 |= REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE;
	                data8 = ((~data8) | REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE);                           
	                ethsw_wreg(dev, PAGE_CONTROL, REG_POWER_DOWN_MODE, (uint8 *)&data8, sizeof(data8));                
	                /* end of enet y42304 modified 20060711: power down port 5*/
	            }

	            /* setup Switch MII1 port state override */
	            ethsw_rreg(dev, PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &data8, sizeof(data8));

	            if (pDevCtrl->EnetInfo.usReverseMii == BP_ENET_REVERSE_MII)
	                data8 |= REG_CONTROL_MPSO_REVERSE_MII;
	            data8 |= (REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS);
	            data8 |= (REG_CONTROL_MPSO_LP_FLOW_CONTROL|REG_CONTROL_MPSO_SPEED100|REG_CONTROL_MPSO_FDX);

	            ethsw_wreg(dev, PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &data8, sizeof(data8));

	            /* checking Switch functional */
	            data8 = 0;
	            ethsw_rreg(dev, PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &data8, sizeof(data8));
	            if ((data8 & (REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS)) !=
	                (REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS) ||
	                (data8 == 0xff)) {
	                    printk(KERN_INFO CARDNAME ": error on Ethernet Switch setup\n");
	                    return -1;
	            }
	            if (pDevCtrl->EnetInfo.usReverseMii == BP_ENET_REVERSE_MII) {
	                if ((data8 & REG_CONTROL_MPSO_REVERSE_MII) != REG_CONTROL_MPSO_REVERSE_MII) {
	                    printk(KERN_INFO CARDNAME ": error on Ethernet Switch reverse MII setup\n");
	                    return -1;
	                }
	            }
	            pDevCtrl->ethSwitch.type = ethsw_switch_type(dev);
           }
            /*start of support   infineon ADM6996M LSW by l39225 20061218*/
           else 
           {
                int i ,v;
                for( i = 0; i< 5;i++)
                {
                    v = mii_read(dev,0,0x01+2*i);
                    v |= 0x8000;
                    mii_write(dev,0,0x01+2*i,v);
                }

                v = mii_read(dev,0,0x08);
                v |= 0x8000;
                mii_write(dev,0,0x08,v);
                
           		pDevCtrl->ethSwitch.type =  ESW_TYPE_ADM6996M;
           }
          /*end of support   infineon ADM6996M LSW by l39225 20061218*/
            break;

        default:
            break;
    }

    return 0;
}

