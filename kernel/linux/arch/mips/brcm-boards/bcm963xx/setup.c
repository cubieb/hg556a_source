/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/*
 * Generic setup routines for Broadcom 963xx MIPS boards
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <asm/addrspace.h>
#include <asm/bcache.h>
#include <asm/irq.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/gdb-stub.h>

extern void brcm_timer_setup(struct irqaction *irq);
extern unsigned long getMemorySize(void);

#include <bcm_map_part.h>

#if defined(CONFIG_PCI)
#include <linux/pci.h>
#include <linux/delay.h>
#include <bcmpci.h>

static volatile MpiRegisters * mpi = (MpiRegisters *)(MPI_BASE);
#endif

/* This function should be in a board specific directory.  For now,
 * assume that all boards that include this file use a Broadcom chip
 * with a soft reset bit in the PLL control register.
 */
static void brcm_machine_restart(char *command)
{
    PERF->pll_control |= SOFT_RESET;
}

static void brcm_machine_halt(void)
{
    printk("System halted\n");
    while (1);
}

#if defined(CONFIG_PCI)

static void mpi_SetLocalPciConfigReg(uint32 reg, uint32 value)
{
    /* write index then value */
    mpi->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    mpi->pcicfgdata = value;
}

static uint32 mpi_GetLocalPciConfigReg(uint32 reg)
{
    /* write index then get value */
    mpi->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    return mpi->pcicfgdata;
}

/*
 * mpi_ResetPcCard: Set/Reset the PcCard
 */
static void mpi_ResetPcCard(int cardtype, BOOL bReset)
{
    if (cardtype == MPI_CARDTYPE_NONE) {
        return;
    }

    if (cardtype == MPI_CARDTYPE_CARDBUS) {
        bReset = ! bReset;
    }

    if (bReset) {
        mpi->pcmcia_cntl1 = (mpi->pcmcia_cntl1 & ~PCCARD_CARD_RESET);
    } else {
        mpi->pcmcia_cntl1 = (mpi->pcmcia_cntl1 | PCCARD_CARD_RESET);
    }
}

/*
 * mpi_ConfigCs: Configure an MPI/EBI chip select
 */
static void mpi_ConfigCs(uint32 cs, uint32 base, uint32 size, uint32 flags)
{
    mpi->cs[cs].base = ((base & 0x1FFFFFFF) | size);
    mpi->cs[cs].config = flags;
}

/*
 * mpi_InitPcmciaSpace
 */
static void mpi_InitPcmciaSpace(void)
{
    // ChipSelect 4 controls PCMCIA Memory accesses
    mpi_ConfigCs(PCMCIA_COMMON_BASE, pcmciaMem, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 5 controls PCMCIA Attribute accesses
    mpi_ConfigCs(PCMCIA_ATTRIBUTE_BASE, pcmciaAttr, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 6 controls PCMCIA I/O accesses
    mpi_ConfigCs(PCMCIA_IO_BASE, pcmciaIo, EBI_SIZE_64K, (EBI_WORD_WIDE|EBI_ENABLE));

    mpi->pcmcia_cntl2 = ((PCMCIA_ATTR_ACTIVE << RW_ACTIVE_CNT_BIT) | 
                         (PCMCIA_ATTR_INACTIVE << INACTIVE_CNT_BIT) | 
                         (PCMCIA_ATTR_CE_SETUP << CE_SETUP_CNT_BIT) | 
                         (PCMCIA_ATTR_CE_HOLD << CE_HOLD_CNT_BIT));

    mpi->pcmcia_cntl2 |= (PCMCIA_HALFWORD_EN | PCMCIA_BYTESWAP_DIS);
}

/*
 * cardtype_vcc_detect: PC Card's card detect and voltage sense connection
 * 
 *   CD1#/      CD2#/     VS1#/     VS2#/    Card       Initial Vcc
 *  CCD1#      CCD2#     CVS1      CVS2      Type
 *
 *   GND        GND       open      open     16-bit     5 vdc
 *
 *   GND        GND       GND       open     16-bit     3.3 vdc
 *
 *   GND        GND       open      GND      16-bit     x.x vdc
 *
 *   GND        GND       GND       GND      16-bit     3.3 & x.x vdc
 *
 *====================================================================
 *
 *   CVS1       GND       CCD1#     open     CardBus    3.3 vdc
 *
 *   GND        CVS2      open      CCD2#    CardBus    x.x vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    y.y vdc
 *
 *   GND        CVS2      GND       CCD2#    CardBus    3.3 & x.x vdc
 *
 *   CVS2       GND       open      CCD1#    CardBus    x.x & y.y vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    3.3, x.x & y.y vdc
 *
 */
static int cardtype_vcc_detect(void)
{
    uint32 data32;
    int cardtype;

    cardtype = MPI_CARDTYPE_NONE;
    /* HUAWEI HGW s48571 2008年1月19日 Wireless led error modify begin:
    mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE); // Turn on the output enables and drive
                                        // the CVS pins to 0.
    */
    mpi->pcmcia_cntl1 = CARDBUS_ENABLE; 
    /*Because all the GPIO pin here detected is used as other purpose, so these four pin
    could not work as the detection function. We get rid of the method of detect by GPIO
    in this version---VDF HG553*/
    return cardtype;
    /* HUAWEI HGW s48571 2008年1月19日 Wireless led error modify end. */
                                        
    data32 = mpi->pcmcia_cntl1;
    switch (data32 & (CD2_IN|CD1_IN))  // Test CD1# and CD2#, see if card is plugged in.
    {
    case (CD2_IN|CD1_IN):  // No Card is in the slot.
        printk("mpi: No Card is in the PCMCIA slot\n");
        break;

    case CD2_IN:  // Partial insertion, No CD2#.
        printk("mpi: Card in the PCMCIA slot partial insertion, no CD2 signal\n");
        break;

    case CD1_IN:  // Partial insertion, No CD1#.
        printk("mpi: Card in the PCMCIA slot partial insertion, no CD1 signal\n");
        break;

    case 0x00000000:
        mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN|VS1_OEN); 
                                        // Turn off the CVS output enables and
                                        // float the CVS pins.
        mdelay(1);
        data32 = mpi->pcmcia_cntl1;
        // Read the Register.
        switch (data32 & (VS2_IN|VS1_IN))  // See what is on the CVS pins.
        {
        case 0x00000000: // CVS1 and CVS2 are tied to ground, only 1 option.
            printk("mpi: Detected 3.3 & x.x 16-bit PCMCIA card\n");
            cardtype = MPI_CARDTYPE_PCMCIA;
            break;
          
        case VS1_IN: // CVS1 is open or tied to CCD1/CCD2 and CVS2 is tied to ground.
                         // 2 valid voltage options.
        switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
        {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("mpi: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2. 
                mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN); // Drive CVS1 to a 0.
                mdelay(1);
                data32 = mpi->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS2, not valid.
                    printk("mpi: Unknown card plugged into slot\n"); 
                } else {                   // CCD2 is tied to CVS1.
                    printk("mpi: Detected 3.3, x.x and y.y Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;
                
            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("mpi: Unknown card plugged into slot\n"); 
                break;
                
            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                printk("mpi: Detected x.x vdc 16-bit PCMCIA card\n");
                cardtype = MPI_CARDTYPE_PCMCIA;
                break;
            }
            break;
          
        case VS2_IN: // CVS2 is open or tied to CCD1/CCD2 and CVS1 is tied to ground.
                         // 2 valid voltage options.
            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("mpi: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = mpi->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS1, not valid.
                    printk("mpi: Unknown card plugged into slot\n"); 
                } else {// CCD2 is tied to CVS2.
                    printk("mpi: Detected 3.3 and x.x Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;

            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("mpi: Unknown card plugged into slot\n"); 
                break;

            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("mpi: Detected 3.3 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;
          
        case (VS2_IN|VS1_IN):  // CVS1 and CVS2 are open or tied to CCD1/CCD2.
                          // 5 valid voltage options.
      
            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("mpi: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                              // CCD1 is tied to ground.
                mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = mpi->pcmcia_cntl1;
                if (data32 & CD2_IN) {  // CCD2 is tied to CVS1.
                    printk("mpi: Detected y.y vdc Cardbus card\n");
                } else {                    // CCD2 is tied to CVS2.
                    printk("mpi: Detected x.x vdc Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;
      
            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // CCD2 is tied to ground.
      
                mpi->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = mpi->pcmcia_cntl1;
                if (data32 & CD1_IN) {// CCD1 is tied to CVS1.
                    printk("mpi: Detected 3.3 vdc Cardbus card\n");
                } else {                    // CCD1 is tied to CVS2.
                    printk("mpi: Detected x.x and y.y Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;
      
            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("mpi: Detected 5 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;
      
        default:
            printk("mpi: Unknown card plugged into slot\n"); 
            break;
        
        }
    }
    return cardtype;
}

/*
 * mpi_DetectPcCard: Detect the plugged in PC-Card
 * Return: < 0 => Unknown card detected
 *         0 => No card detected
 *         1 => 16-bit card detected
 *         2 => 32-bit CardBus card detected
 */
static int mpi_DetectPcCard(void)
{
    int cardtype;

    cardtype = cardtype_vcc_detect();
    switch(cardtype) {
        case MPI_CARDTYPE_PCMCIA:
            mpi->pcmcia_cntl1 &= ~(CARDBUS_ENABLE|PCMCIA_ENABLE|PCMCIA_GPIO_ENABLE); // disable enable bits
            mpi->pcmcia_cntl1 |= (PCMCIA_ENABLE | PCMCIA_GPIO_ENABLE);
            mpi_InitPcmciaSpace();
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        case MPI_CARDTYPE_CARDBUS:
            // 8 => CardBus Enable
            // 1 => PCI Slot Number
            // C => Float VS1 & VS2
            mpi->pcmcia_cntl1 = (mpi->pcmcia_cntl1 & 0xFFFF0000) | 
                                CARDBUS_ENABLE | 
                                (CARDBUS_SLOT << 8)| 
                                VS2_OEN |
                                VS1_OEN | PCMCIA_GPIO_ENABLE;
            /* access to this memory window will be to/from CardBus */
            mpi->l2pmremap1 |= CARDBUS_MEM;

            // Need to reset the Cardbus Card. There's no CardManager to do this, 
            // and we need to be ready for PCI configuration. 
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        default:
            break;
    }
    return cardtype;
}

static int mpi_init(void)
{
    unsigned long data;
    unsigned int chipid;
    unsigned int chiprev;
    unsigned int sdramsize;

    chipid  = (PERF->RevID & 0xFFFF0000) >> 16;
    chiprev = (PERF->RevID & 0xFF);
    sdramsize = getMemorySize();

#if defined(CONFIG_BCM96348)
    /*
     * Init the pci interface 
     */
    data = GPIO->GPIOMode; // GPIO mode register
    data |= GROUP2_PCI | GROUP1_MII_PCCARD; // PCI internal arbiter + Cardbus
    GPIO->GPIOMode = data; // PCI internal arbiter
#endif
    /*
     * CardBus support is defaulted to Slot 0
     * because there is no external IDSEL for CardBus.  To disable
     * the CardBus and allow a standard PCI card in Slot 0 
     * set the cbus_idsel field to 0x1f.
    */
    /*
    uData = mpi->pcmcia_cntl1;
    uData |= CARDBUS_IDSEL;
    mpi->pcmcia_cntl1 = uData;
    */

    // UBUS to PCI address range
    // Memory Window 1. Used for devices in slot 0. Potentially can be CardBus
    mpi->l2pmrange1 = ~(BCM_PCI_MEM_SIZE_16MB-1);
    // UBUS to PCI Memory base address. This is akin to the ChipSelect base
    // register. 
    mpi->l2pmbase1 = BCM_CB_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address. Replaces the masked address bits in the
    // range register with this setting. 
    // Also, enable direct I/O and direct Memory accesses
    mpi->l2pmremap1 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Memory Window 2. Used for devices in other slots
    mpi->l2pmrange2 = ~(BCM_PCI_MEM_SIZE_16MB-1);
    // UBUS to PCI Memory base address. 
    mpi->l2pmbase2 = BCM_PCI_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address
    mpi->l2pmremap2 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Setup PCI I/O Window range. Give 64K to PCI I/O
    mpi->l2piorange = ~(BCM_PCI_IO_SIZE_64KB-1);
    // UBUS to PCI I/O base address 
    mpi->l2piobase = BCM_PCI_IO_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI I/O Window remap
    mpi->l2pioremap = (BCM_PCI_IO_BASE | MEM_WINDOW_EN);

    // enable PCI related GPIO pins and data swap between system and PCI bus
    mpi->locbuscntrl = (EN_PCI_GPIO | DIR_U2P_NOSWAP);

    /* Enable BusMaster and Memory access mode */
    data = mpi_GetLocalPciConfigReg(PCI_COMMAND);
    data |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
    mpi_SetLocalPciConfigReg(PCI_COMMAND, data);

    /* Configure two 16 MByte PCI to System memory regions. */
    /* These memory regions are used when PCI device is a bus master */
    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_3, BCM_HOST_MEM_SPACE1);

#if defined(CONFIG_BCM96348)
    mpi->sp0remap = 0x0;    
#else    
    mpi->sp0remap = MEM_WINDOW_EN;
#endif    

    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_4, BCM_HOST_MEM_SPACE2);
    
#if defined(CONFIG_BCM96348)
    mpi->sp1remap = 0x0;
#else
    mpi->sp1remap = MEM_WINDOW_EN;
#endif    
    
    mpi->pcimodesel |= 0x40;

    if (!((chipid == 0x6348) && ((chiprev & 0xF0) == 0xa0))) {
        mpi->sp0range = ~(sdramsize-1);
        mpi->sp1range = ~(sdramsize-1);
    }
    /*
     * Change PCI Cfg Reg. offset 0x40 to PCI memory read retry count infinity
     * by set 0 in bit 8~15.  This resolve read Bcm4306 srom return 0xffff in
     * first read.
     */
    data = mpi_GetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER);
    data &= ~BRCM_PCI_CONFIG_TIMER_RETRY_MASK;
    data |= 0x00000080;
    mpi_SetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER, data);

    /* enable pci interrupt */
    mpi->locintstat |= (EXT_PCI_INT << 16);

    mpi_DetectPcCard();

    ioport_resource.start = BCM_PCI_IO_BASE;
    ioport_resource.end = BCM_PCI_IO_BASE + BCM_PCI_IO_SIZE_64KB;

#if defined(CONFIG_USB)
#if defined(CONFIG_BCM96348)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    *USBH = USBH_BYTE_SWAP;
#endif
#if defined(CONFIG_BCM96358)
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->TestPortControl = 0x001c0020;
#endif
#endif

    return 0;
}
#endif

static int __init brcm63xx_setup(void)
{
    extern int panic_timeout;

    _machine_restart = brcm_machine_restart;
    _machine_halt = brcm_machine_halt;
    _machine_power_off = brcm_machine_halt;

    board_timer_setup = brcm_timer_setup;

    panic_timeout = 1;

#if defined(CONFIG_PCI)
    /* mpi initialization */
    mpi_init();
#endif
    return 0;
}

early_initcall(brcm63xx_setup);

/***************************************************************************
 * C++ New and delete operator functions
 ***************************************************************************/

/* void *operator new(unsigned int sz) */
void *_Znwj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* void *operator new[](unsigned int sz)*/
void *_Znaj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* placement new operator */
/* void *operator new (unsigned int size, void *ptr) */
void *ZnwjPv(unsigned int size, void *ptr)
{
    return ptr;
}

/* void operator delete(void *m) */
void _ZdlPv(void *m)
{
    kfree(m);
}

/* void operator delete[](void *m) */
void _ZdaPv(void *m)
{
    kfree(m);
}

EXPORT_SYMBOL(_Znwj);
EXPORT_SYMBOL(_Znaj);
EXPORT_SYMBOL(ZnwjPv);
EXPORT_SYMBOL(_ZdlPv);
EXPORT_SYMBOL(_ZdaPv);

