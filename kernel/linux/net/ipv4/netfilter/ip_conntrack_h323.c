/***********************************************************
  Copyright (C), 2005-2006, Huawei Tech. Co., Ltd.
  FileName: ip_conntrack_h323.c
  Author: caiyanfeng  Version: v1.0  Date: 2006-06-09
  Description: Connection tracking support for H.323
  History:
      <author>  <time>   <version >   <desc>
***********************************************************/


#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack_h323.h>


MODULE_AUTHOR("Cai Yanfeng <caiyanfeng@huawei.com>");
MODULE_DESCRIPTION("H.323 connection tracking module");
MODULE_LICENSE("GPL");

static char h323Buffer[H323_BUFFER_SIZE];
DECLARE_LOCK(ip_h323_lock);
struct module *ip_conntrack_h323 = THIS_MODULE;

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif


struct stRecordAddr 
{    
    u_int32_t iAddrIP;
    u_int16_t nAddrPort;
    u_int8_t  cAddrType;
    u_int8_t  cAddrNum;
    u_int16_t nAddrOffset[H323_ADDR_OFFSET_NUM];    
};


static int h245_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo);
static int q931_expect(struct ip_conntrack *ct);
static int q931_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo);
static int ras_expect(struct ip_conntrack *ct);
static int ras_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo);


/* H.245 structure: helper is not registered! */
static struct ip_conntrack_helper h245 = 
{ 
    .list = {NULL, NULL},
    .name = "H.245",
    .flags = IP_CT_HELPER_F_REUSE_EXPECT,
    .me = THIS_MODULE,
    .max_expected = 32,
    .timeout = 100,
    .tuple = 
    {
        {0, {.tcp = {0}}},
        {0, {0}, IPPROTO_TCP} 
    },
    .mask =
    {
        {0, {.tcp = {0xFFFF}}},
        {0, {0}, 0xFFFF} 
    },   
    .help = h245_help
};

/* H.225.0: Q.931 structure */
static struct ip_conntrack_helper q931 = 
{
    .list = {NULL, NULL},
    .name = "Q.931",
    .flags = IP_CT_HELPER_F_REUSE_EXPECT,
    .me = THIS_MODULE,
    .max_expected = 16,
    .timeout = 100,
    .tuple = 
    { 
        {0, {.tcp = { __constant_htons(H225_PORT)}}},
        {0, {0}, IPPROTO_TCP} 
    },
    .mask = 
    { 
        {0, {.tcp = {0xFFFF}}},
        {0, {0}, 0xFFFF} 
    },
    .help = q931_help
};

/* H.225.0: RAS structure */
static struct ip_conntrack_helper ras = 
{
    .list = {NULL, NULL},
    .name = "RAS",
    .flags = IP_CT_HELPER_F_REUSE_EXPECT,
    .me = THIS_MODULE,
    .max_expected = 16,
    .timeout = 100,  
    .tuple = 
    {
        {0, {.udp = {__constant_htons(H225_PORT-1)}}},
        {0, {0}, IPPROTO_UDP}
    },
    .mask = 
    { 
        {0, {.udp = {0xFFFF}}},
        {0, {0}, 0xFFFF} 
    },
    .help = ras_help 
};

static unsigned short udp_check(struct udphdr *uh, int len, unsigned long saddr, unsigned long daddr, unsigned long base)
{
	return(csum_tcpudp_magic(saddr, daddr, len, 17, base));
};

static void find_ip_port(char *data, 
                         u_int32_t *dataIp, 
                         u_int16_t *dataPort)
{
    char dataAddr[H323_ADDR_LENGTH];
    memcpy(dataAddr, data, H323_ADDR_LENGTH);
    *dataIp = *((u_int32_t *)dataAddr);
    *dataPort = *((u_int16_t *)(dataAddr + 4));
}

static int find_h245_ip_port(char *data, 
                             u_int32_t *dataIp,
                             u_int16_t *dataPort)
{
    int ret = -1;
    char dataAddr[H323_ADDR_LENGTH];
    memcpy(dataAddr, data + 2, H323_ADDR_LENGTH);
    /*start of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
    // if ((*((u_int32_t *)dataAddr) == __constant_htonl(0x0008914a))
    /*start of HG553 2008.05.17 V100R001C02B015 AU8D00641 by c65985 */
    if ((*((u_int32_t *)dataAddr) == 0x0008914a)
    /*end of HG553 2008.05.17 V100R001C02B015 AU8D00641 by c65985 */
        && ((*data) & 0x40) 
        && (*(data + 1) == 6) 
        && (*(data + 6) == 0) 
        && (*(data + 8) == 0))
    {
        memcpy(dataAddr, data + 9, H323_ADDR_LENGTH);
        *dataIp = *((u_int32_t *)dataAddr);
        *dataPort = *((u_int16_t *)(dataAddr + 4));
        ret = 0;
    }
    /*e n d of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
    return ret;
}

static int fill_record(struct stRecordAddr *pstRecIpPort, 
             u_int16_t nRecNum,
             u_int32_t dataIp, 
             u_int16_t dataPort,
             u_int16_t nOffset,
             int addrType)
{
    int ret = -1;
    int i;
    
    for (i = 0; i < nRecNum; i ++)
    {
        if ((pstRecIpPort[i].iAddrIP == dataIp) 
            && (pstRecIpPort[i].nAddrPort == dataPort))
        {
            pstRecIpPort[i].cAddrType = addrType;
            pstRecIpPort[i].nAddrOffset[pstRecIpPort[i].cAddrNum] = nOffset;            
            if (pstRecIpPort[i].cAddrNum < (H323_ADDR_OFFSET_NUM -1))
            {
                pstRecIpPort[i].cAddrNum ++;
            }
            ret = 0;
            break;
        }
        else if((pstRecIpPort[i].iAddrIP == 0) 
            && (pstRecIpPort[i].nAddrPort == 0))
        {
            pstRecIpPort[i].iAddrIP = dataIp;
            pstRecIpPort[i].nAddrPort = dataPort;
            pstRecIpPort[i].cAddrType = addrType;
            pstRecIpPort[i].nAddrOffset[pstRecIpPort[i].cAddrNum] = nOffset;
            if (pstRecIpPort[i].cAddrNum < (H323_ADDR_OFFSET_NUM -1))
            {
                pstRecIpPort[i].cAddrNum ++;
            }
            ret = 0;
            break;
        }
    }
    
    return ret;
}

static int expect_record(struct ip_conntrack *ct,
             struct stRecordAddr *pstRecIpPort, 
             u_int16_t nRecNum,
             int dir,
             u_int32_t seq)
{
    int ret = 0;
    struct ip_conntrack_expect *exp = NULL;
    int i, j;
    
    for (i = 0; i < nRecNum; i ++)
    {
        if ((pstRecIpPort[i].iAddrIP != 0)
            && (pstRecIpPort[i].nAddrPort != 0))
        {
            exp = ip_conntrack_expect_alloc();
            if (exp)
            {
                exp->help.exp_h225_info.port = pstRecIpPort[i].nAddrPort;
                exp->help.exp_h225_info.dir = dir;
                exp->help.exp_h225_info.uOffsetCount = pstRecIpPort[i].cAddrNum;
                for (j = 0; j < pstRecIpPort[i].cAddrNum; j++)
                {		
                    exp->help.exp_h225_info.offset[j] = pstRecIpPort[i].nAddrOffset[j];
                }
                exp->seq = seq + pstRecIpPort[i].nAddrOffset[0];
                exp->tuple.src.ip = ct->tuplehash[!dir].tuple.src.ip;
                exp->tuple.dst.ip = pstRecIpPort[i].iAddrIP;
                //exp->tuple.dst.u.tcp.port = pstRecIpPort[i].nAddrPort;
                /*start of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
                exp->mask.src.ip = 0;//0xFFFFFFFF;
                /*e n d of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
                exp->mask.dst.ip = 0xFFFFFFFF;
                //exp->mask.dst.u.tcp.port = 0xFFFF;
                exp->mask.dst.protonum = 0;                
                if (pstRecIpPort[i].cAddrType == H323_ADDR_TYPE_Q931)
                {
                    exp->tuple.dst.u.tcp.port = pstRecIpPort[i].nAddrPort;
                    exp->mask.dst.u.tcp.port = 0xFFFF;
                    exp->tuple.dst.protonum = IPPROTO_TCP;
                    exp->expectfn = ras_expect;
                }
                else if (pstRecIpPort[i].cAddrType == H323_ADDR_TYPE_H245)
                {
                    exp->tuple.dst.u.tcp.port = pstRecIpPort[i].nAddrPort;
                    exp->mask.dst.u.tcp.port = 0xFFFF;
                    exp->tuple.dst.protonum = IPPROTO_TCP;
                    exp->expectfn = q931_expect;
                    //WRITE_LOCK(&ip_conntrack_lock);
                    //ct->tuplehash[!dir].tuple.dst.ip = pstRecIpPort[i].iAddrIP;
                    //ct->tuplehash[dir].tuple.src.ip = pstRecIpPort[i].iAddrIP;
                    //WRITE_UNLOCK(&ip_conntrack_lock);
                }
                else //H323_ADDR_TYPE_RTP
                {
                    exp->tuple.dst.u.udp.port = pstRecIpPort[i].nAddrPort;
                    exp->mask.dst.u.udp.port = 0xFFFF;
                    exp->tuple.dst.protonum = IPPROTO_UDP;
                }

                ret = ip_conntrack_expect_related(exp, ct);
                DEBUGP("H.323 expect record ct[%p] protocol[%d] port[%d] ret[%d]\n",
                       ct, exp->tuple.dst.protonum, exp->tuple.dst.u.tcp.port, ret);
            }
            else
            {
                ret = -1;
                DEBUGP("H.323 expect alloc failed.\n");
            } 
        }
    }
    return ret;
}

static int h245_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo)
{
    struct tcphdr _tcph, *tcph = &_tcph;
    unsigned char *data = h323Buffer;
    unsigned char *dataStart, *dataEnd;
    u_int32_t tcplen = skb->len - skb->nh.iph->ihl * 4;
    u_int32_t datalen;
    int dir = CTINFO2DIR(ctinfo);
    struct ip_ct_h225_master *info = &ct->help.ct_h225_info;
    u_int16_t dataPort;
    u_int32_t dataIp;
    struct stRecordAddr stRecIpPort[H245_REC_ADDR_NUM];

    memset(stRecIpPort, 0 , sizeof(stRecIpPort));

    /* Can't track connections formed before we registered */
    if (!info)
    {
        return NF_ACCEPT;
    }

    tcph = skb_header_pointer(skb, skb->nh.iph->ihl*4, sizeof(_tcph), &_tcph);
	if (!tcph)
	{
		return NF_ACCEPT;
	}  

    DEBUGP("H.245 help: help entered %u.%u.%u.%u:%u->%u.%u.%u.%u:%u\n",
        NIPQUAD(skb->nh.iph->saddr), ntohs(tcph->source),
        NIPQUAD(skb->nh.iph->daddr), ntohs(tcph->dest));
    
    /* Until there's been traffic both ways, don't look in packets. */
    if ((ctinfo != IP_CT_ESTABLISHED)
        && (ctinfo != (IP_CT_ESTABLISHED + IP_CT_IS_REPLY)))
    {
        DEBUGP("H.245 help: Conntrackinfo = %u\n", ctinfo);
        return NF_ACCEPT;
    }
   
    /* Not whole TCP header or too short packet? */
    if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4 + 5)
    {
        DEBUGP("H.245 help: tcplen = %u\n", (unsigned)tcplen);
        return NF_ACCEPT;
    }

    /* Checksum invalid?  Ignore. */
    /* FIXME: Source route IP option packets --RR */
    if (tcp_v4_check(tcph, tcplen, skb->nh.iph->saddr, skb->nh.iph->daddr,
                  csum_partial((char *)tcph, tcplen, 0))) 
    {
        DEBUGP("H.245 help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
               tcph, tcplen, NIPQUAD(skb->nh.iph->saddr),
               NIPQUAD(skb->nh.iph->daddr));
        return NF_ACCEPT;
    }

    LOCK_BH(&ip_h323_lock);
    datalen = tcplen - tcph->doff * 4;
    if (datalen > H323_BUFFER_SIZE)
    {
        datalen = H323_BUFFER_SIZE;
    }
    skb_copy_bits(skb, skb->nh.iph->ihl*4 + tcph->doff*4, 
                  h323Buffer, datalen);          
    dataStart = (unsigned char *)data;
    dataEnd = (unsigned char *)data + datalen - 1;

    /* bytes[xxxx xx]: ipaddr port */
    while (data <= (dataEnd - 5))
    {
        find_ip_port(data, &dataIp, &dataPort);
        if (dataIp == skb->nh.iph->saddr)
        {
            DEBUGP("H.245 help: Rtp Address %u.%u.%u.%u:%u\n",
                    NIPQUAD(dataIp), dataPort);
            fill_record(stRecIpPort, H245_REC_ADDR_NUM, dataIp, dataPort,
                        data - dataStart, H323_ADDR_TYPE_RTP);
            data += 6;
        }
        else
        {
            data ++;
        }
    }
    
    info->is_h225 = __constant_htons(H225_PORT + 1);
    expect_record(ct, stRecIpPort, H245_REC_ADDR_NUM, dir, ntohl(tcph->seq));
    
    UNLOCK_BH(&ip_h323_lock);
    return NF_ACCEPT;
}

static int q931_expect(struct ip_conntrack *ct)
{
    WRITE_LOCK(&ip_conntrack_lock);
    ct->helper = &h245;
    DEBUGP("H.225.0 Q.931 expect: helper for %p added\n", ct);
    WRITE_UNLOCK(&ip_conntrack_lock);
    
    return NF_ACCEPT;
}

static int q931_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo)
{
    struct tcphdr _tcph, *tcph = &_tcph; 
    unsigned char *data = h323Buffer;
    unsigned char *dataStart, *dataEnd;
    u_int32_t tcplen = skb->len - skb->nh.iph->ihl * 4;
    u_int32_t datalen;
    int dir = CTINFO2DIR(ctinfo);
    struct ip_ct_h225_master *info = &ct->help.ct_h225_info;
    u_int16_t dataPort;
    u_int32_t dataIp;
    struct stRecordAddr stRecIpPort[Q931_REC_ADDR_NUM];
    
    memset(stRecIpPort, 0 , sizeof(stRecIpPort));    

    /* Can't track connections formed before we registered */
    if (!info)
    {
        DEBUGP("Q.931: help has not registered.\n");
        return NF_ACCEPT;
    }

    tcph = skb_header_pointer(skb, skb->nh.iph->ihl*4, sizeof(_tcph), &_tcph);
	if (!tcph)
	{
		return NF_ACCEPT;
	}

    DEBUGP("Q.931: help entered %u.%u.%u.%u:%u->%u.%u.%u.%u:%u\n",
        NIPQUAD(skb->nh.iph->saddr), ntohs(tcph->source),
        NIPQUAD(skb->nh.iph->daddr), ntohs(tcph->dest));
    
    /* Until there's been traffic both ways, don't look in packets. */
    if ((ctinfo != IP_CT_ESTABLISHED)
        && (ctinfo != (IP_CT_ESTABLISHED + IP_CT_IS_REPLY)))
    {
        DEBUGP("Q.931 help: Conntrackinfo = %u\n", ctinfo);
        return NF_ACCEPT;
    }
    
    /* Not whole TCP header or too short packet? */
    if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4 + 5)
    {
        DEBUGP("Q.931 help: tcplen = %u\n", (unsigned)tcplen);
        return NF_ACCEPT;
    }

    /* Checksum invalid?  Ignore. */
    /* FIXME: Source route IP option packets --RR */
    if (tcp_v4_check(tcph, tcplen, skb->nh.iph->saddr, skb->nh.iph->daddr,
                  csum_partial((char *)tcph, tcplen, 0)))
    {
        DEBUGP("Q.931 help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
               tcph, tcplen, NIPQUAD(skb->nh.iph->saddr),
               NIPQUAD(skb->nh.iph->daddr));
        return NF_ACCEPT;
    }

    LOCK_BH(&ip_h323_lock);
    datalen = tcplen - tcph->doff * 4;
    if (datalen > H323_BUFFER_SIZE)
    {
        datalen = H323_BUFFER_SIZE;
    }    
    skb_copy_bits(skb, skb->nh.iph->ihl*4 + tcph->doff*4,
                  h323Buffer, datalen);
    dataStart = (unsigned char *)data;
    dataEnd = (unsigned char *)data + datalen - 1;

    /* bytes[xxxx xx]: ipaddr port */
    while (data <= (dataEnd - 5))
    {
        find_ip_port(data, &dataIp, &dataPort);    
        

        if (dataIp == skb->nh.iph->saddr)
        {
            if (dataPort == tcph->source) 
            {
                /* Signal address */
                DEBUGP("Q.931 help: sourceCallSignalAddress from %u.%u.%u.%u\n",
                    NIPQUAD(skb->nh.iph->saddr));
                /* Update the Q.931 info so that NAT can mangle the address/port
                   even when we have no expected connection! */
#ifdef CONFIG_IP_NF_NAT_NEEDED
                //LOCK_BH(&ip_h323_lock);
                info->dir = dir;
                info->seq[IP_CT_DIR_ORIGINAL] = ntohl(tcph->seq) + (int)(data - dataStart);
                info->offset[IP_CT_DIR_ORIGINAL] = (int)(data - dataStart);
                //UNLOCK_BH(&ip_h323_lock);
#endif
            } 
            else 
            {
                DEBUGP("Q.931 help: Rtp Address %u.%u.%u.%u:%u\n",
                    NIPQUAD(dataIp), dataPort);
                fill_record(stRecIpPort, Q931_REC_ADDR_NUM, dataIp, dataPort,
                            data - dataStart, H323_ADDR_TYPE_RTP);
            }
            data += 6;
        }
        else if (dataIp == skb->nh.iph->daddr)
        {
#ifdef CONFIG_IP_NF_NAT_NEEDED        
            if (dataPort == tcph->dest)
            {
                /* Signal address */
                DEBUGP("Q.931 help: destCallSignalAddress %u.%u.%u.%u\n",
                    NIPQUAD(skb->nh.iph->daddr));
                /* Update the H.225 info so that NAT can mangle the address/port
                   even when we have no expected connection! */
                //LOCK_BH(&ip_h323_lock);
                info->dir = dir;
                info->seq[IP_CT_DIR_REPLY] = ntohl(tcph->seq) + (int)(data - dataStart);
                info->offset[IP_CT_DIR_REPLY] = (int)(data - dataStart);
                //UNLOCK_BH(&ip_h323_lock);
            }
#endif
            data += 4;
        }
        else
        {
            if ((dataEnd - data) >= 14)
            {
                if (!find_h245_ip_port(data, &dataIp, &dataPort))
                {
                    DEBUGP("Q.931 help: H.245 Address %u.%u.%u.%u:%u\n",
                    NIPQUAD(dataIp), dataPort);
                    /*start of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
                    if (dataIp == skb->nh.iph->daddr || dataIp == skb->nh.iph->saddr)
                    {
                        fill_record(stRecIpPort, Q931_REC_ADDR_NUM, dataIp, dataPort,
                                data + 9 - dataStart, H323_ADDR_TYPE_H245);
                    }
                    /*e n d of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
                    data += 15;
                }
                else
                {
                    data ++;
                }
            }
            else
            {
                data ++;
            }
        }
        
    }

    info->is_h225 = __constant_htons(H225_PORT);
    expect_record(ct, stRecIpPort, Q931_REC_ADDR_NUM, dir, ntohl(tcph->seq));
    UNLOCK_BH(&ip_h323_lock);

    return NF_ACCEPT;
}

static int ras_expect(struct ip_conntrack *ct)
{
    WRITE_LOCK(&ip_conntrack_lock);
    ct->helper = &q931;
    DEBUGP("H.225.0 RAS expect: helper for %p added\n", ct);
    WRITE_UNLOCK(&ip_conntrack_lock);
    
    return NF_ACCEPT;
}


static int ras_help(struct sk_buff *skb,
             struct ip_conntrack *ct,
             enum ip_conntrack_info ctinfo)
{
    struct udphdr _udph, *udph = &_udph;
    unsigned char *data = h323Buffer;
    unsigned char *dataStart, *dataEnd;
    u_int32_t udplen = skb->len - skb->nh.iph->ihl * 4;
    u_int32_t datalen = udplen - 8;
    int dir = CTINFO2DIR(ctinfo);
    struct ip_ct_h225_master *info = &ct->help.ct_h225_info;
    u_int16_t dataPort;
    u_int32_t dataIp;
    struct stRecordAddr stRecIpPort[RAS_REC_ADDR_NUM];

    memset(stRecIpPort, 0 , sizeof(stRecIpPort));

    /* Can't track connections formed before we registered */
    if (!info)
    {
        DEBUGP("RAS: help has not registered.\n");
        return NF_ACCEPT;
    }

    udph = skb_header_pointer(skb, skb->nh.iph->ihl*4, sizeof(_udph), &_udph);
	if (!udph)
	{
		return NF_ACCEPT;
	}

    DEBUGP("RAS: help entered %u.%u.%u.%u:%u->%u.%u.%u.%u:%u\n",
        NIPQUAD(skb->nh.iph->saddr), ntohs(udph->source),
        NIPQUAD(skb->nh.iph->daddr), ntohs(udph->dest));
	
/*start of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/
#if 0
    /* Until there's been traffic both ways, don't look in packets. */
    if ((ctinfo != IP_CT_ESTABLISHED)
        && (ctinfo != (IP_CT_ESTABLISHED + IP_CT_IS_REPLY)))
    {
        DEBUGP("RAS: Conntrackinfo = %u\n", ctinfo);
        return NF_ACCEPT;
    }
#endif
/*e n d of HG_Protocol 2007.08.13 HG550V100R002C01B013 A36D03815 by d37981*/

    /* Not whole TCP header or too short packet? */
    if (udplen < sizeof(struct udphdr))
    {
        DEBUGP("RAS: help udplen = %u\n", (unsigned)udplen);
        return NF_ACCEPT;
    }

    /* Checksum invalid?  Ignore. */
    /* FIXME: Source route IP option packets --RR */
    if (udp_check(udph, udplen, skb->nh.iph->saddr, skb->nh.iph->daddr,
                  csum_partial((char *)udph, udplen, 0))) 
    {
        DEBUGP("RAS help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
               udph, udplen, NIPQUAD(skb->nh.iph->saddr),
               NIPQUAD(skb->nh.iph->daddr));
        return NF_ACCEPT;
    }

    LOCK_BH(&ip_h323_lock);
    if (datalen > H323_BUFFER_SIZE)
    {
        datalen = H323_BUFFER_SIZE;
    }    
    skb_copy_bits(skb, skb->nh.iph->ihl*4 + 8, h323Buffer, datalen);
    dataStart = (unsigned char *)data;
    dataEnd = (unsigned char *)data + datalen - 1;
    
    /* bytes[xxxx xx]: ipaddr port */
    while (data <= (dataEnd - 5))
    {
        find_ip_port(data, &dataIp, &dataPort);
        if (dataIp == skb->nh.iph->saddr)
        {         
            if (dataPort == udph->source)
            {
#ifdef CONFIG_IP_NF_NAT_NEEDED
                /* Signal address */
                DEBUGP("RAS: sourceCallSignalAddress from %u.%u.%u.%u\n",
                    NIPQUAD(skb->nh.iph->saddr));
                /* Update the H.225 info so that NAT can mangle the address/port
                   even when we have no expected connection! */
                //LOCK_BH(&ip_h323_lock);
                info->dir = dir;
                //info->seq[IP_CT_DIR_ORIGINAL] = ntohl(tcph->seq) + i;
                info->offset[IP_CT_DIR_ORIGINAL] = (int)(data - dataStart);
                //UNLOCK_BH(&ip_h323_lock);
#endif
            }
            else
            {
                fill_record(stRecIpPort, RAS_REC_ADDR_NUM, dataIp, dataPort,
                            data - dataStart, H323_ADDR_TYPE_Q931);
            }  
            data += 6;
        }
        else if (dataIp == skb->nh.iph->daddr)
        {
#ifdef CONFIG_IP_NF_NAT_NEEDED
            if (dataPort == udph->dest)
            {
                /* Signal address */
                DEBUGP("RAS: destCallSignalAddress %u.%u.%u.%u\n",
                    NIPQUAD(skb->nh.iph->daddr));
                /* Update the H.225 info so that NAT can mangle the address/port
                   even when we have no expected connection! */
                //LOCK_BH(&ip_h323_lock);
                info->dir = dir;
                //info->seq[IP_CT_DIR_REPLY] = ntohl(tcph->seq) + i;
                info->offset[IP_CT_DIR_REPLY] = (int)(data - dataStart);
                //UNLOCK_BH(&ip_h323_lock);
            }
#endif
            data += 4;
        }
        else
        {
            data ++;
        }
    }

    info->is_h225 = __constant_htons(H225_PORT);
    expect_record(ct, stRecIpPort, RAS_REC_ADDR_NUM, dir, 0);
    UNLOCK_BH(&ip_h323_lock);

    return NF_ACCEPT;
}

static int __init init(void)
{
    ip_conntrack_helper_register(&ras);
    ip_conntrack_helper_register(&q931);

    return 0;
}

static void __exit fini(void)
{

    ip_conntrack_helper_unregister(&ras);
    ip_conntrack_helper_unregister(&q931);
}

EXPORT_SYMBOL(ip_h323_lock);

module_init(init);
module_exit(fini);
