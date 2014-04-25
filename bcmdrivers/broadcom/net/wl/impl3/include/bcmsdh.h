/*
 * SDIO host client driver interface of Broadcom HNBU
 *     export functions to client driver(wl)
 *     abstract OS and BUS specific details of SDIO
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 * $Id: bcmsdh.h,v 1.1 2008/08/25 06:41:57 l65130 Exp $
 */

#ifndef	_bcmsdh_h_
#define	_bcmsdh_h_

#define BCMSDH_SUCCESS	0
#define BCMSDH_FAIL	99

/* forward declaration */
typedef struct bcmsdh_info bcmsdh_info_t;

/* function callback takes just one void arg */
typedef void (*bcmsdh_cb_fn_t)(void *);

/* attach the underline SD host driver, 
 * return the bcmsdh handler and virtual address of register access 
 *   The bcmsdh handler should be used in subsequent calls whenever possible, in the case that 
 *     the handler can't be provided like bcmsdh_reg_read/write(due to current wl driver design), 
 *     SD host driver/glue code shall save a local copy of the handler.
 *   The va of register access should be used for all subsequent register access
 */
extern bcmsdh_info_t *bcmsdh_attach(osl_t *osh, void *cfghdl, void **regsva);

/* detach the underline SD host driver, release bcmsdh handler */
extern int bcmsdh_detach(osl_t *osh, void *sdh);

/* query if SD interrupt is enabled */
extern bool bcmsdh_intr_query(void *sdh);

/* enable/disable SD interrupt */
extern bool bcmsdh_intr_enable(void *sdh);
extern bool bcmsdh_intr_disable(void *sdh);

#ifdef DSLCPE
extern int bcmsdh_intr_handler(void *sdh);
extern void bcmsdh_intr_mask(void *sdh);
extern void bcmsdh_intr_unmask(void *sdh);
extern void bcmsdh_set_sdclk(void *sdh);
#endif
/* register/deregister interrupt handler IF SD host driver handles the interrupt */
extern int bcmsdh_intr_reg(void *sdh, bcmsdh_cb_fn_t fn, void *argh);
extern int bcmsdh_intr_dereg(void *sdh);

/* register device remove handler IF SD host driver handles the device removal */
extern int bcmsdh_devremove_reg(void *sdh, bcmsdh_cb_fn_t fn, void *argh); 

/* get/set SB register base for core switching */
extern void bcmsdh_regbase_set(void *sdh, uint32 addr);
extern uint32 bcmsdh_regbase_get(void *sdh);

/* cfg register(CCCR etc.) read/write */
//extern uint8 bcmsdh_cfg_read(void *sdh, uint fuc, uint32 addr);
//extern uint bcmsdh_cfg_write(void *sdh, uint fuc, uint32 addr, uint8 data);
extern uint8 bcmsdh_cfg_read(void *sdh, uint fnc_num, uint32 addr, int *err);
extern void bcmsdh_cfg_write(void *sdh, uint fnc_num, uint32 addr, uint8 data, int *err);

/* read SD function x's cis */
extern uint8 bcmsdh_cis_read(void *sdh, uint fuc, uint8 *cis, uint32 length);

/* access Silicon Backplane register in 2 or 4 bytes */
extern uint32 bcmsdh_reg_read(void *sdh, uint32 addr, uint size);
extern uint32 bcmsdh_reg_write(void *sdh, uint32 addr, uint size, uint32 data);

/* transfer whole buffer to/from SB registers: 
 *    addr is the SB register access, e.g. d11 memory or PIO
 *    size is the register width, either 2 or 4 bytes
 *    isfifo means after each register access, whether to increase address automatically or not
 *  return code: TRUE on success, FALSE on fail 
 */
extern bool bcmsdh_send_buf(void *sdh, uint32 addr, uint size, bool isfifo, uint8 *buf, uint32 len);
extern bool bcmsdh_recv_buf(void *sdh, uint32 addr, uint size, bool isfifo, uint8 *buf, uint32 len);

#ifdef BCMSDIO_EBIDMA
/* dma commands */
extern int  bcmsdh_ebidma_tx_start(void *sdh, uint32 addr, void *p0, uint len, uint32 coreflags, void *cb, void *param);
extern bool bcmsdh_ebidma_tx_end(void *sdh);
extern int  bcmsdh_ebidma_rx_start(void *sdh, uint32 addr, void *p0, uint len, uint32 coreflags, void *cb, void *param);
extern int  bcmsdh_ebidma_rx_end(void *sdh);
/* sdio command */
extern bool bcmsdh_send_dmabuf(void *sdh, uint32 addr, uint size, bool isfifo, uint8 *buf, uint32 len);
extern bool bcmsdh_recv_dmabuf(void *sdh, uint32 addr, uint size, bool isfifo, uint8 *buf, uint32 len);
#endif

extern uint bcmsdh_query_iofnum(void *sdh);

#endif	/* _bcmsdh_h_ */
