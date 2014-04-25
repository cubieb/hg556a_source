/*
 * wl_linux.c exported functions and definitions
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: wl_linux.h,v 1.1 2008/08/25 06:42:23 l65130 Exp $
 */

#ifndef _wl_linux_h_
#define _wl_linux_h_

/* handle forward declaration */
typedef struct wl_info wl_info_t;

/* exported functions */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
extern irqreturn_t wl_isr(int irq, void *dev_id);
#else
extern irqreturn_t wl_isr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif	/* < 2.6.20 */

extern int __devinit wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern void wl_free(wl_info_t *wl);
extern int  wl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
extern struct net_device * wl_netdev_get(wl_info_t *wl);
#ifdef DSLCPE
extern void wl_shutdown_handler(wl_info_t *wl);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11)
extern int wl_suspend(struct pci_dev *pdev, DRV_SUSPEND_STATE_TYPE  state);
#else
extern int wl_suspend(struct pci_dev *pdev, u32 state);
#endif	/* < 2.6.11 */
extern void wl_reset_cnt(struct net_device *dev);
#ifdef BCMSDIO
extern void * wl_sdh_get(wl_info_t *wl);
extern int wl_sdio_register(uint16 venid, uint16 devid, void *regsva, void *param, int irq);
extern int wl_sdio_unregister(void);
#endif /* BCMSDIO */
#endif /* DSLCPE */
#ifdef BCM_WL_EMULATOR
extern wl_info_t *  wl_wlcreate(osl_t *osh, void *pdev);
#endif /* BCM_WL_EMULATOR */
#endif /* _wl_linux_h_ */
