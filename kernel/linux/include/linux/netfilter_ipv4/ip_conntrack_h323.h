#ifndef _IP_CONNTRACK_H323_H
#define _IP_CONNTRACK_H323_H
/* H.323 connection tracking. */

#ifdef __KERNEL__
/* Protects H.323 related data */
#include <linux/netfilter_ipv4/lockhelp.h>
DECLARE_LOCK_EXTERN(ip_h323_lock);
#endif

/* Default H.225 port */
#define H225_PORT	1720

#define H245_REC_ADDR_NUM 8
#define Q931_REC_ADDR_NUM 16
#define RAS_REC_ADDR_NUM  8

#define H323_ADDR_LENGTH 6
#define H323_MSG_UNIT_LENGTH 16
#define H323_ADDR_OFFSET_NUM 16
#define H323_BUFFER_SIZE 10240

#define H323_ADDR_TYPE_Q931 1
#define H323_ADDR_TYPE_H245 2
#define H323_ADDR_TYPE_RTP  3



/* This structure is per expected connection */
struct ip_ct_h225_expect {
	u_int16_t port;			/* Port of the H.225 helper/RTCP/RTP channel */
	enum ip_conntrack_dir dir;	/* Direction of the original connection */
	unsigned int offset[H323_ADDR_OFFSET_NUM];
	unsigned int uOffsetCount;
};

/* This structure exists only once per master */
struct ip_ct_h225_master {
	int is_h225;				/* H.225 or H.245 connection */
#ifdef CONFIG_IP_NF_NAT_NEEDED
	enum ip_conntrack_dir dir;		/* Direction of the original connection */
	u_int32_t seq[IP_CT_DIR_MAX];		/* Exceptional packet mangling for signal addressess... */
	unsigned int offset[IP_CT_DIR_MAX];	/* ...and the offset of the addresses in the payload */
#endif
};

#endif /* _IP_CONNTRACK_H323_H */
