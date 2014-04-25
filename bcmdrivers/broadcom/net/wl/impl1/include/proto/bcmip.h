/*
 * Copyright 2005, Broadcom Corporation      
 * All Rights Reserved.                      
 *                                           
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;         
 * the contents of this file may not be disclosed to third parties, copied      
 * or duplicated in any form, in whole or in part, without the prior            
 * written permission of Broadcom Corporation.                                  
 *
 * Fundamental constants relating to IP Protocol
 *
 * $Id: bcmip.h,v 1.1 2008/08/25 06:41:26 l65130 Exp $
 */

#ifndef _bcmip_h_
#define _bcmip_h_

/* IP header */
#define IPV4_VERIHL_OFFSET	0	/* version and ihl byte offset */
#define IPV4_TOS_OFFSET		1	/* TOS offset */
#define IPV4_PROT_OFFSET	9	/* protocol type offset */
#define IPV4_CHKSUM_OFFSET	10	/* IP header checksum offset */
#define IPV4_SRC_IP_OFFSET	12	/* src IP addr offset */
#define IPV4_DEST_IP_OFFSET	16	/* dest IP addr offset */

#define IPV4_VER_MASK	0xf0
#define IPV4_IHL_MASK	0x0f

#define IPV4_PROT_UDP	17	/* UDP protocol type */

#define IPV4_ADDR_LEN	4	/* IP v4 address length */

#define IPV4_VER_NUM	0x40	/* IP v4 version number */

/* NULL IP address check */
#define IPV4_ISNULLADDR(a)	((((uint8 *)(a))[0] + ((uint8 *)(a))[1] + \
				((uint8 *)(a))[2] + ((uint8 *)(a))[3]) == 0)

#define IPV4_ADDR_STR_LEN	16

/* IP TOS byte */
#define	IPV4_TOS_DSCP_MASK	0xFC		/* DiffServ codepoint */
#define	IPV4_TOS_DSCP_SHIFT	2

#define	IPV4_TOS_PREC_MASK	0xE0		/* Historical precedence */
#define	IPV4_TOS_PREC_SHIFT	5
#define IPV4_TOS_LOWDELAY	0x10
#define IPV4_TOS_THROUGHPUT	0x8
#define IPV4_TOS_RELIABILITY	0x4

#endif	/* _bcmip_h_ */
