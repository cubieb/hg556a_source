#ifndef _IP_CONNTRACK_PT_H
#define _IP_CONNTRACK_PT_H
/* PT tracking. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

#include <linux/netfilter_ipv4/lockhelp.h>

/* Protects pt part of conntracks */
DECLARE_LOCK_EXTERN(ip_pt_lock);

enum ip_ct_pt_type
{
	/* from client */
	IP_CT_OUTGOING_PORT,
};

/* We record pt ip/port here: all in
   host order. */
struct ip_ct_pt
{
	/* This tells NAT that this is an pt connection */
	int is_pt;
	/* 0 means not found yet */
	u_int32_t len;
	enum ip_ct_pt_type pttype;
	/* Port that was to be used */
	u_int16_t port;
};

#endif /* _IP_CONNTRACK_PT_H */
