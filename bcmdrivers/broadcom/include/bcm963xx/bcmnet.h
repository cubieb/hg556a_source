/*
<:copyright-gpl
 Copyright 2002 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcmnet.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LINKSTATE_DOWN      0
#define LINKSTATE_UP        1

#if defined(CHIP_6338) || defined(CONFIG_BCM96338)
#define NR_RX_BDS               80
#else
#define NR_RX_BDS               120 
#endif
#define NR_TX_BDS               200
/*---------------------------------------------------------------------*/
/* Ethernet Switch Type                                                */
/*---------------------------------------------------------------------*/
#define ESW_TYPE_UNDEFINED                  0
#define ESW_TYPE_BCM5325M                   1
#define ESW_TYPE_BCM5325E                   2
#define ESW_TYPE_BCM5325F                   3
#define ESW_TYPE_ADM6996M                   4


#ifdef CONFIG_LSWWAN
#define LSW_WPORT   1
#endif
/*
 * Ioctl definitions.
 */
/* reserved SIOCDEVPRIVATE */
enum {
    SIOCGLINKSTATE = SIOCDEVPRIVATE + 1,
    SIOCSCLEARMIBCNTR,
    SIOCGIFTRANSSTART,
    SIOCMIBINFO,    
    #if 0
    SIOCSDUPLEX,	/* 0: auto 1: full 2: half */
    SIOCSSPEED,		/* 0: auto 1: 100mbps 2: 10mbps */
    #endif
    SIOCSETMODE,
    SIOCCIFSTATS,
    SIOCGENABLEVLAN,
    SIOCGDISABLEVLAN,
    SIOCGQUERYNUMVLANPORTS,
    SIOCGSWITCHTYPE,
    SIOCGQUERYNUMPORTS,
    #if 0
    SIOCGMACTOPORT,
    #endif 
    SIOCREATWAN,
    #if 0
    SIOCGLINKSTATUS,
    #endif
    /* start 支持以太网口配置 by l68693 2009-02-17, Private Ioctl已经达到最大数目,要想再增加必须删除某些项*/    
    SIOCGPORTOFF,
    SIOCGPORTON,       
    /* end 支持以太网口配置 by l68693 2009-02-17*/
    SIOCGLSWLINKSTATE,
    SIOCLAST
};

#define SPEED_10MBIT        10000000
#define SPEED_100MBIT       100000000

typedef struct IoctlMibInfo
{
    unsigned long ulIfLastChange;
    unsigned long ulIfSpeed;
} IOCTL_MIB_INFO, *PIOCTL_MIB_INFO;


#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
