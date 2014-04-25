#ifndef _IPT_MAC_H
#define _IPT_MAC_H

#define IPTMAC_SRC 0x01
#define IPTMAC_DST 0x02

struct ipt_mac_info {
    unsigned char srcaddr[ETH_ALEN];
    unsigned char dstaddr[ETH_ALEN];
    int invert;
    unsigned int flag;
};
#endif /*_IPT_MAC_H*/
