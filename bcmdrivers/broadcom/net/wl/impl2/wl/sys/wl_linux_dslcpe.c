/*
 * DSLCPE specific functions
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl_linux_dslcpe.c,v 1.1 2008/08/25 06:41:49 l65130 Exp $
 */

#include <typedefs.h>

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linuxver.h>
#include <osl.h>

//#include <wl_dbg.h>
#include <wlioctl.h>

#include <wl_linux_dslcpe.h>
#include <wl_linux.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#ifdef BCMSDIO
#include <bcmsdh.h>
extern wl_info_t *sdio_dev;
void *bcmsdh_osh = NULL;
#endif /* BCMSDIO */

#include "bcm_map.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcmnet.h"
#include "boardparms.h"

#ifdef BCMDBG
extern int msglevel;
#endif
extern struct pci_device_id wl_id_table[];

extern bool
wlc_chipmatch(uint16 vendor, uint16 device);

#define PCI2_INT_BUS_RD_PREFETCH	0x1f0

static void 
mpi_setup(void)
{
	u32 chipID;
	u32 modesel;
	MpiRegisters *mpi = (MpiRegisters *)(MPI_BASE);
		
	chipID = ((PERF->RevID & 0xFFFF0000) >> 16);
	
	/* start- setup pci read prefetch */
	modesel = mpi->pcimodesel;		
	switch (chipID) {
		case BCM96348_BOARD:
			modesel &= ~PCI2_INT_BUS_RD_PREFETCH;
			modesel |= 0x80;
			break;
		case BCM96358_BOARD:			
		case BCM96368_BOARD:			
			modesel &= ~PCI2_INT_BUS_RD_PREFETCH;
			modesel |= 0x100;
			break;
		default:
			break;					
	}
	mpi->pcimodesel = modesel;
	/* end- setup pci read prefetch */	
}

/*
 * wl_dslcpe_open:
 * extended hook for device open for DSLCPE.
 */
int wl_dslcpe_open(struct net_device *dev)
{
	return 0;	
}

/*
 * wl_dslcpe_close:
 * extended hook for device close for DSLCPE.
 */
int wl_dslcpe_close(struct net_device *dev)
{
	return 0;
}
/*
 * wlc_dslcpe_boardflags:
 * extended hook for modifying boardflags for DSLCPE.
 */
void wlc_dslcpe_boardflags(uint32 *boardflags, uint32 *boardflags2 )
{
	return;
}

/*
 * wlc_dslcpe_led_attach:
 * extended hook for when led is to be initialized for DSLCPE.
 */
void wlc_dslcpe_led_attach(void)
{
	return;
}

/*
 * wlc_dslcpe_led_detach:
 * extended hook for when led is to be de-initialized for DSLCPE.
 */
void wlc_dslcpe_led_detach(void)
{
	return;
}
/*
 * wlc_dslcpe_timer_led_blink_timer:
 * extended hook for when periodical(200ms) led timer is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_timer_led_blink_timer(void)
{
	return;
}
/*
 * wlc_dslcpe_led_timer:
 * extended hook for when led blink timer(10ms) is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_led_timer(void)
{
	return;
}

/*
 * wl_dslcpe_ioctl:
 * extended ioctl support on BCM63XX.
 */
int
wl_dslcpe_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int isup = 0;
	int error =-1;

	if (cmd >= SIOCGLINKSTATE && cmd < SIOCLAST) {
		error = 0;
		/* we can add sub-command in ifr_data if we need to in the future */
		switch (cmd) {
			case SIOCGLINKSTATE:
				if (dev->flags&IFF_UP) isup = 1;
				if (copy_to_user((void*)(int*)ifr->ifr_data, (void*)&isup,
					sizeof(int))) {
					return -EFAULT;
				}
				break;
			case SIOCSCLEARMIBCNTR:
				wl_reset_cnt(dev);
				break;
		}
	} else {
		error = wl_ioctl(dev, ifr, cmd);
	}
	return error;
}

/*
 * wl_dslcpe_detect:
 * returns FALSE if id not found.
 */
uint
wl_dslcpe_detect(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	u32 chipID;
	int Found = FALSE;

	chipID = ((PERF->RevID & 0xFFFF0000) >> 16);

	if (chipID == BCM96338_BOARD || chipID == BCM96348_BOARD || 
        chipID == BCM96358_BOARD || chipID == BCM96368_BOARD)
    {
		Found = TRUE;
	}

	return Found;
}

/*
 * wl_dslcpe_isr:
 */
irqreturn_t
wl_dslcpe_isr(int irq, void *dev_id, struct pt_regs *ptregs)
{
	bool ours;
	
#ifdef BCMSDIO
	ours = bcmsdh_intr_handler(wl_sdh_get((wl_info_t *)dev_id));
#else
	/* ignore wl_isr return value due to dedicated interrupt line */
	ours = wl_isr(irq, dev_id, ptregs);
#endif
	
	BcmHalInterruptEnable(irq);

	return IRQ_RETVAL(ours);
}

/* special deal for dslcpe */
int __devinit
wl_dslcpe_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	wl_info_t *wl;
	struct net_device *dev;
	int irq;
	
#ifdef BCMSDIO
	wl = (wl_info_t*)sdio_dev;
#else

	if (!wl_dslcpe_detect(pdev, ent))
		return -ENODEV;

	if (wl_pci_probe(pdev, ent))
		return -ENODEV;

	wl = pci_get_drvdata(pdev);
#endif
	ASSERT(wl);

	/* additional setup for mpi */
	mpi_setup();

	/* hook ioctl */
	dev = wl_netdev_get(wl);
	ASSERT(dev);
	ASSERT(dev->do_ioctl);
	dev->do_ioctl = wl_dslcpe_ioctl;

#ifdef DSLCPE_DGASP	
    	kerSysRegisterDyingGaspHandler(dev->name, &wl_shutdown_handler, wl);
#endif

#ifdef BCMSDIO
	irq = dev->irq;
#else
	irq = pdev->irq;
#endif
	if (irq && wl) {
		free_irq(irq, wl);
		if (BcmHalMapInterrupt((FN_ISR)wl_dslcpe_isr, (unsigned int)wl, irq)) {
			printk("wl_dslcpe: request_irq() failed\n");
			goto fail;
		}
		BcmHalInterruptEnable(irq);
	}
	
	return 0;

fail:
	wl_free(wl);
	return -ENODEV;
}

#ifndef BCMSDIO 
static void __devexit
wl_remove(struct pci_dev *pdev) 
{
	wl_info_t *wl;
	
	if (!wlc_chipmatch(pdev->vendor, pdev->device))
		return;

	if ((wl = (wl_info_t *) pci_get_drvdata(pdev))) {
		wl_suspend(pdev, 0);
#ifdef DSLCPE_DGASP		
		kerSysDeregisterDyingGaspHandler(wl_netdev_get(wl)->name);
#endif		
		wl_free(wl);
		pci_set_drvdata(pdev, NULL);
	}
}


static struct pci_driver wl_pci_driver = {
	name:		"wl",
	probe:		wl_dslcpe_probe,
	remove:		__devexit_p(wl_remove),
	id_table:	wl_id_table,
	};
#endif

static int __init
wl_module_init(void)
{

	int error;

#ifdef BCMDBG
	if (msglevel != 0xdeadbeef) {
		//wl_msg_level = msglevel;
		printf("%s: msglevel set to 0x%x\n", __FUNCTION__, msglevel);
	}
#endif /* BCMDBG */

#ifdef BCMSDIO
	bcmsdh_osh = osl_attach(NULL); 
	if (!(error = wl_sdio_register(VENDOR_BROADCOM, BCM4318_DEVICE_ID, (void *)0xfffe2300, bcmsdh_osh, INTERRUPT_ID_SDIO))) {
		if((error = wl_dslcpe_probe(0, 0)) != 0) {	/* to hookup entry points or misc */
			osl_detach(bcmsdh_osh);
			return error;
		}		
	} else {
		osl_detach(bcmsdh_osh);
	}
	
#else	/* BCMSDIO */

	if (!(error = pci_module_init(&wl_pci_driver)))
		return (0);
#endif

	return (error);	
}

static void __exit
wl_module_exit(void)
{
#ifdef BCMSDIO
	wl_sdio_unregister();
	osl_detach(bcmsdh_osh);
#else
	pci_unregister_driver(&wl_pci_driver);
#endif	

}

module_init(wl_module_init);
module_exit(wl_module_exit);


int wl_chipid(void) {

	int devid = 0;
#ifdef BCMSDIO
	return BCM4318_DEVICE_ID;
#else	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)		
	struct pci_driver *drv;
	struct bus_type * bus;
	struct list_head * entry;
	struct device * dev;
	struct pci_dev * pci_dev;
	
	drv = &wl_pci_driver;
	bus = drv->driver.bus;
	list_for_each(entry, &bus->devices.list) {
		dev = container_of(entry, struct device, bus_list);
		if (dev) {
			pci_dev = to_pci_dev(dev);
			if (pci_dev) {
				devid = pci_dev->device;
				break;
			}
		}
	}
#else
	struct pci_dev *pci_dev;
	int devid = 0;

	pci_for_each_dev(pci_dev) {
		devid = pci_dev->device;
		break;
	}
	
#endif	/* LINUX_VERSION_CODE */
#endif  /* BCMSDIO */
	return devid;
}

