/*

<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92618 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/

/****************************************************************************
 *
 * bcmBlaaMirrorLinux.c -- Bcm BLAA core Diagnostics for the incoming and
   outgoing packets.
 *
 * Description:
 *   This file contains BCM BLAA core port mirroring feature for the WAN side packets.
   Selective thro compilation for the initial version. Plans exist to
   make it a CLI/Web UI control driven.
 *
 * Authors: Srinivasan Seshadri
 *
 * Revision 1.0
 * Inial CVS checkin
 *
 ****************************************************************************/

/* Includes. */
#include <linux/types.h>
#define   _SYS_TYPES_H

#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>

#include <net/ip.h>
#include <net/route.h>
#include <net/arp.h>
#include <linux/version.h>

#include "bcm_common.h"
#include "bcm_map.h"
#include "bcm_intr.h"
#include "board.h"

#include <BcmBlaaMirror.h>

#include <linux/etherdevice.h>

//#define PORT_MIRROR_DBG   1

UINT32  blaaNoMirrorPktCount, blaaMirrorPktCount ;

int blaaMirrorWriteData (struct net_device * dbgDev, struct sk_buff *pSockBuf)
{
   int n ;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   ulong            flags;
#endif
#if (defined(PORT_MIRROR_DBG))
   printk ("BLAA_MIRROR: Before Eth type trans Data Bytes  %lx \t %lx \t %lx \t %lx \t %lx \n",
      *((UINT16 *) pSockBuf->data),
      *((UINT32 *) (pSockBuf->data+2)),
      *((UINT32 *) (pSockBuf->data+6)),
      *((UINT32 *) (pSockBuf->data+10)),
      *((UINT32 *) (pSockBuf->data+14))) ;
#endif

   pSockBuf->dev = dbgDev;
#if 0
#if (defined(PORT_MIRROR_DBG))
   /* The following routine seems to remove the ethernet header out. */
   pSockBuf->protocol = eth_type_trans (pSockBuf, dbgDev) ;
#endif
#endif
pSockBuf->protocol = htons(ETH_P_802_3); /* Need to find a better way instead of hardcoding. */
pSockBuf->nfmark  |= BCM_MIRROR_PACKETS;

#if (defined(PORT_MIRROR_DBG))
   printk ("BLAA_MIRROR: After eth type trans. Data Bytes  %lx \t %lx \t %lx \t %lx \t %lx \n",
      *((UINT16 *) pSockBuf->data),
      *((UINT32 *) (pSockBuf->data+2)),
      *((UINT32 *) (pSockBuf->data+6)),
      *((UINT32 *) (pSockBuf->data+10)),
      *((UINT32 *) (pSockBuf->data+14))) ;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   local_irq_save(flags);
   local_irq_enable();
#endif

   n = dev_queue_xmit(pSockBuf) ;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   local_irq_restore(flags);
#endif
   return n;
}

void blaaMirrorRfc2684Frame (struct sk_buff *pSockBuf, UINT32 ulRcvLen, UINT32 dir,
                             char *dev)
{
   UINT8            *pData ;
   UINT32           hdr1, hdr2 ;
   UINT32           adjustLen = 0 ;
   struct sk_buff   *pCloneBuf ;
   struct           net_device  *netDev ;

   /* Analyse the data on the Socket Buffer->data for the following frame
   ** types.
   ** 1) RFC 2684 - 802.3 Bridged Frames including the MER frames.
   ** 2) RFC 2684 - VPN Based Bridge Frames.
   ** If the frame is of any other type, it will not be sent for tracing and it
   ** will be silently ignored.
   **/

   pData = pSockBuf->data ;

   hdr1 = *((UINT32 *) pData) ;
   hdr2 = *((UINT32 *) (pData+4) ) ;

   if (hdr1 == RFC2684_802_3_LLC_HDR1) {

      if ((hdr2 == RFC2684_802_3_LLC_HDR2_NO_FCS) ||
     (hdr2 == RFC2684_802_3_LLC_HDR2_FCS)) {

    adjustLen += RFC2684_802_3_LLC_ENCAP_LEN ;
      }
      else if (hdr2 == RFC2684_802_3_VPN_BRIDGE_HDR2) {

    /* VPN based bridging. Check for the inner bridge encapsulations */
    adjustLen += RFC2684_802_3_VPN_BRIDGE_ENCAP_LEN ;
    hdr1 = *((UINT32 *) (pData+RFC2684_802_3_VPN_BRIDGE_ENCAP_LEN)) ;

    if (hdr1 == RFC2684_802_3_LLC_HDR1) {
       adjustLen += RFC2684_802_3_LLC_ENCAP_LEN ;
    }
    else {
       adjustLen += RFC2684_802_3_VC_ENCAP_LEN ;
    }
      }
      else {
    /* It could be a IP routed frame. Since it does not have an ethernet
    ** header, we can not trace it. Ignore it.
    **/
    blaaNoMirrorPktCount++ ;

#if defined(PORT_MIRROR_DBG)
    printk ("BLAA_MIRROR: blaaNoMirrorPktCount = %ld \n", blaaNoMirrorPktCount) ;
#endif
    goto _EndProcessing ;
      }
   }
   else {

      hdr1 = *((UINT16 *) (pData)) ;
      if (hdr1 == RFC2684_802_3_LLC_VC_PAD) {
    /* A Vc Muxed bridge frame */
    adjustLen += RFC2684_802_3_VC_ENCAP_LEN ;
      }
      else {
    /* It is a possibly a routed frame. Ignore tracing */
    blaaNoMirrorPktCount++ ;

#if defined(PORT_MIRROR_DBG)
    printk ("BLAA_MIRROR: blaaNoMirrorPktCount = %ld \n", blaaNoMirrorPktCount) ;
#endif
    goto _EndProcessing ;
      }
   }

   /* If we are here, we need to send the frame for Mirroring. Let's clone
   ** the sk_buff hdr sharing the data with adjusted len information and send
   ** it onto Ethernet If 0.
   **/

   pCloneBuf = skb_clone (pSockBuf, GFP_ATOMIC) ;
   if (pCloneBuf == NULL) {
      printk ("sk_buff cloning failed. fatal error. No Mirroring \n") ;
      goto _EndProcessing ;
   }

#if defined(PORT_MIRROR_DBG)
   printk ("BLAA_MIRROR: Data Bytes  %lx \t %lx \t %lx \t %lx \t %lx \n",
      *((UINT32 *) pCloneBuf->data),
      *((UINT32 *) (pCloneBuf->data+4)),
      *((UINT32 *) (pCloneBuf->data+8)),
      *((UINT32 *) (pCloneBuf->data+12)),
      *((UINT32 *) (pCloneBuf->data+16))) ;
#endif

#if defined(PORT_MIRROR_DBG)
   printk ("LLC_802_3_SNAP - data addr before adj- %lx \n", (UINT32) pCloneBuf->data) ;
#endif

   pCloneBuf->data     = ((UINT8 *) pCloneBuf->data + adjustLen) ;

#if defined(PORT_MIRROR_DBG)
   printk ("LLC_802_3_SNAP - data addr after adj- %lx \n", (UINT32) pCloneBuf->data) ;
#endif

   pCloneBuf->data_len -= adjustLen ;
   pCloneBuf->len      -= adjustLen ;

#if defined(PORT_MIRROR_DBG)
   printk ("BLAA_MIRROR: Before __dev_get_by_name Data Bytes  %lx \t %lx \t %lx \t %lx \t %lx \n",
      *((UINT16 *) pCloneBuf->data),
      *((UINT32 *) (pCloneBuf->data+2)),
      *((UINT32 *) (pCloneBuf->data+6)),
      *((UINT32 *) (pCloneBuf->data+10)),
      *((UINT32 *) (pCloneBuf->data+14))) ;
#endif

   /* "dir" parameter to this function can be used here, if we want to output
   ** differently for each of the direction of the traffic. Currently, assumed
   ** to be eth0. So, "dir" is ignored.
   **/
   netDev = __dev_get_by_name (dev) ;


  

#if defined(PORT_MIRROR_DBG)
   printk ("BLAA_MIRROR: After __dev_get_by_name Data Bytes  %lx \t %lx \t %lx \t %lx \t %lx \n",
      *((UINT16 *) pCloneBuf->data),
      *((UINT32 *) (pCloneBuf->data+2)),
      *((UINT32 *) (pCloneBuf->data+6)),
      *((UINT32 *) (pCloneBuf->data+10)),
      *((UINT32 *) (pCloneBuf->data+14))) ;
#endif

   if (netDev == NULL) {
      printk ("Net Device for %s is not found. Fatal Error.      \n", dev) ;
      goto _EndProcessing ;
   }

   if (blaaMirrorWriteData (netDev, pCloneBuf) >= 0) {
      /* Success */
      blaaMirrorPktCount++ ;
#if defined(PORT_MIRROR_DBG)
   printk ("BLAA_MIRROR: blaaMirrorPktCount = %ld \n", blaaMirrorPktCount) ;
#endif
   }
   else {
      printk ("BLAA_MIRROR: Mirror transmission failed. Fatal \n") ;
   }

_EndProcessing :
      return ;
}
