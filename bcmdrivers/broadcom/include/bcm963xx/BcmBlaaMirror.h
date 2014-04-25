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
 * BcmBlaaMirror.h -- Internal definitions for BLAA Port Mirroring Feature.
 *
 * Description:
 *   Port Mirroring feature definitions for the BLAA driver.
 *
 *
 * Authors: Srinivasan Seshadri.
 *
 * $Revision: 1.1 $
 *
 * Initial CVS checkin.
 *
 ****************************************************************************/

#ifndef __BCM_BLAA_MIRROR_H
#define __BCM_BLAA_MIRROR_H

#include <portMirror.h>

/* RFC2684 definitions */

#define RFC2684_802_3_LLC_HDR1           0xAAAA0300
#define RFC2684_802_3_LLC_HDR2_NO_FCS    0x80C20001
#define RFC2684_802_3_LLC_HDR2_FCS       0x80C20007
#define RFC2684_802_3_LLC_VC_PAD         0x0000
#define RFC2684_802_3_VPN_BRIDGE_HDR2    0x005E0008

#define RFC2684_802_3_LLC_ENCAP_LEN            10  /* in bytes */
#define RFC2684_802_3_VC_ENCAP_LEN              2  /* in bytes */
#define RFC2684_802_3_VPN_BRIDGE_ENCAP_LEN     16  /* in bytes */

void blaaUtilSetMirrorPort (int mirrorPort, char *pMirrorPort) ;
int blaaMirrorWriteData (struct net_device * dbgDev, struct sk_buff *pSockBuf) ;
void blaaMirrorRfc2684Frame (struct sk_buff *pSockBuf, UINT32 ulRcvLen, UINT32 dir,
                             char *dev) ;

#endif /* __BCM_BLAA_MIRROR_H */
