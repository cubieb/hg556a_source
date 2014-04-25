/*
 * Misc useful routines to access NIC local SROM/OTP .
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmsrom.h,v 1.1 2008/08/25 06:41:57 l65130 Exp $
 */

#ifndef	_bcmsrom_h_
#define	_bcmsrom_h_

#include <bcmsrom_fmt.h>

/* Prototypes */
extern int srom_var_init(sb_t *sbh, uint bus, void *curmap, osl_t *osh,
                         char **vars, uint *count);

extern int srom_read(sb_t *sbh, uint bus, void *curmap, osl_t *osh,
                     uint byteoff, uint nbytes, uint16 *buf);
extern int srom_write(sb_t *sbh, uint bus, void *curmap, osl_t *osh,
                      uint byteoff, uint nbytes, uint16 *buf);

extern int srom_parsecis(osl_t *osh, uint8 **pcis, uint ciscnt,
                         char **vars, uint *count);

#endif	/* _bcmsrom_h_ */
