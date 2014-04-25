/*
 * Copyright 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: epivers.h,v 1.1 2008/08/25 06:42:07 l65130 Exp $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	4

#define	EPI_MINOR_VERSION	174

#define	EPI_RC_NUMBER		64

#define	EPI_INCREMENTAL_NUMBER	12

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		4, 174, 64, 12

#define	EPI_VERSION_NUM		0x04ae400c

/* Driver Version String, ASCII, 32 chars max */
#ifdef DSLCPE
#if defined(BCMSDIO)
#define	EPI_VERSION_STR		"4.174.64.12.cpe1.1sd"
#else
#define	EPI_VERSION_STR		"4.174.64.12.cpe1.1"
#endif
#else
#define	EPI_VERSION_STR		"4.174.64.12"
#endif

#endif /* _epivers_h_ */
