/*
 * Linux OS Independent Layer For SKB Packet Handling
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_osl_pkt.c,v 1.1 2008/08/25 06:41:27 l65130 Exp $
 */

#define LINUX_OSL_PKT

#include <typedefs.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 */

#ifndef osl_t 
#define osl_t void
#endif

void
osl_pktsetcookie(void *skb, void *x)
{
	((struct sk_buff*)skb)->csum = (uint)x;
}

void*
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct sk_buff *skb;

	if ((skb = dev_alloc_skb(len)) == NULL)
		return (NULL);

	skb_put(skb, len);

	/* ensure the cookie field is cleared */ 
	osl_pktsetcookie(skb, NULL);

	return ((void*) skb);
}

void
osl_pktfree(void *p)
{
	struct sk_buff *skb, *nskb;

	skb = (struct sk_buff*) p;

	/* perversion: we use skb->next to chain multi-skb packets */
	while (skb) {
		nskb = skb->next;
		skb->next = NULL;
#ifdef DSLCPE
		dev_kfree_skb_any(skb);
#else		
		if (skb->destructor) {
			/* cannot kfree_skb() on hard IRQ (net/core/skbuff.c) if destructor exists */
			dev_kfree_skb_any(skb);
		} else {
			/* can free immediately (even in_irq()) if destructor does not exist */
			dev_kfree_skb(skb);
		}
#endif		
		skb = nskb;
	}
}

uchar*
osl_pktdata(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->data);
}

uint
osl_pktlen(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->len);
}

uint
osl_pktheadroom(osl_t *osh, void *skb)
{
	return (uint) skb_headroom((struct sk_buff *) skb);
}

uint
osl_pkttailroom(osl_t *osh, void *skb)
{
	return (uint) skb_tailroom((struct sk_buff *) skb);
}

void*
osl_pktnext(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->next);
}

void
osl_pktsetnext(void *skb, void *x)
{
	((struct sk_buff*)skb)->next = (struct sk_buff*)x;
}

void
osl_pktsetlen(osl_t *osh, void *skb, uint len)
{
	__skb_trim((struct sk_buff*)skb, len);
}

uchar*
osl_pktpush(osl_t *osh, void *skb, int bytes)
{
	return (skb_push((struct sk_buff*)skb, bytes));
}

uchar*
osl_pktpull(osl_t *osh, void *skb, int bytes)
{
	return (skb_pull((struct sk_buff*)skb, bytes));
}

void*
osl_pktdup(osl_t *osh, void *skb)
{
	return (skb_clone((struct sk_buff*)skb, GFP_ATOMIC));
}

void*
osl_pktcookie(void *skb)
{
	return ((void*)((struct sk_buff*)skb)->csum);
}

void*
osl_pktlink(void *skb)
{
	return (((struct sk_buff*)skb)->prev);
}

void
osl_pktsetlink(void *skb, void *x)
{
	((struct sk_buff*)skb)->prev = (struct sk_buff*)x;
}

#ifdef DSLCPE
#define PRIO_LOC_NFMARK 16
#endif

uint
osl_pktprio(void *skb)
{
#ifdef DSLCPE
	((struct sk_buff*)skb)->priority = ((struct sk_buff*)skb)->nfmark>>PRIO_LOC_NFMARK & 0x7;
#endif
	return (((struct sk_buff*)skb)->priority);
}

void
osl_pktsetprio(void *skb, uint x)
{
	((struct sk_buff*)skb)->priority = x;

 #ifdef DSLCPE
    ((struct sk_buff*)skb)->nfmark &= ~(0xf << PRIO_LOC_NFMARK);
    ((struct sk_buff*)skb)->nfmark |= (x & 0x7) << PRIO_LOC_NFMARK;
 #endif
}


EXPORT_SYMBOL(osl_pktsetcookie);
EXPORT_SYMBOL(osl_pktget);
EXPORT_SYMBOL(osl_pktfree);
EXPORT_SYMBOL(osl_pktdata);
EXPORT_SYMBOL(osl_pktlen);
EXPORT_SYMBOL(osl_pktheadroom);
EXPORT_SYMBOL(osl_pkttailroom);
EXPORT_SYMBOL(osl_pktnext);
EXPORT_SYMBOL(osl_pktsetnext);
EXPORT_SYMBOL(osl_pktsetlen);
EXPORT_SYMBOL(osl_pktpush);
EXPORT_SYMBOL(osl_pktpull);
EXPORT_SYMBOL(osl_pktdup);
EXPORT_SYMBOL(osl_pktcookie);
EXPORT_SYMBOL(osl_pktlink);
EXPORT_SYMBOL(osl_pktsetlink);
EXPORT_SYMBOL(osl_pktprio);
EXPORT_SYMBOL(osl_pktsetprio);
