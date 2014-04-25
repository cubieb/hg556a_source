/*
<:copyright-broadcom 
 
 Copyright (c) 2004 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/

#ifndef __BCM_COMMON_H
#define __BCM_COMMON_H

#if defined(CONFIG_BCM96338)
#include <6338_common.h>
#endif
#if defined(CONFIG_BCM96348)
#include <6348_common.h>
#endif
#if defined(CONFIG_BCM96358)
#include <6358_common.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,5)      /* starting from 2.4.5 */
#define skb_dataref(x)   (&skb_shinfo(x)->dataref)
#else
#define skb_dataref(x)   skb_datarefp(x)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,19) /* starting from 2.4.19 */
#define VIRT_TO_PHY(a)  (((unsigned long)(a)) & 0x1fffffff)
#else
#define VIRT_TO_PHY     virt_to_phys
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define __save_and_cli  save_and_cli
#define __restore_flags restore_flags
#endif

#endif

