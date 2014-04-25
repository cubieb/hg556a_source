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
 * portMirror.h -- Internal definitions for Port Mirroring Feature.
 *
 * Description:
 *   Port Mirroring feature definitions. Initially for the BLAA driver.
 *
 *
 * Authors: Srinivasan Seshadri.
 *
 * $Revision: 1.1 $
 *
 * Initial CVS checkin.
 *
 ****************************************************************************/

#ifndef __PORT_MIRROR_H
#define __PORT_MIRROR_H

#define DIR_INVALID                  0
#define DIR_IN                       1
#define DIR_OUT                      2
#define MAX_DIRECTIONS               DIR_OUT

#define FLOW_INVALID                 0
#define RFC2684_802_3_BRIDGING_FLOW  1
#define MAX_FLOWS                    RFC2684_802_3_BRIDGING_FLOW

#define MIRROR_PORT_INVALID          0
#define MIRROR_PORT_ETH0             1
#define MIRROR_PORT_ETH1             2
#define MIRROR_PORT_USB              3
#define MAX_MIRROR_PORTS             MIRROR_PORT_USB

#define MIRROR_DISABLED              1
#define MIRROR_ENABLED               2

typedef struct _MirrorCfg {
    int   dir ;
    int   flowType ;
    int   mirrorPort ;
    int   status ;
} MirrorCfg ;

#define CMD_PORT_MIRRORING           1

#define MIRROR_NAME_MAX_LEN          16

#endif /* #ifndef __PORT_MIRROR_H */
