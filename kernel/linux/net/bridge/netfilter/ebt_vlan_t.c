/*
 * Summary: ebt_vlan_t - IEEE 802.1Q target extension module for userspace
 *
 * Description: 802.1Q Virtual LAN remark support module for ebtables project. 
 * Enables to remark 802.1Q:
 * 1) Remark VLAN ID in VLAN-tagged frames (12 - bits field)
 * 2) Remark VLAN Priority in VLAN-tagged frames (3 - bits field)
 * 
 * Authors:
 * WangKe(w00135358) <hustwangke@huawei.com>
 * Sept. 11st, 2008
 *
 *
 */

#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/module.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_vlan_t.h>

static unsigned char debug;
#define MODULE_VERS "1.0"

/*//lint -save -e40 -e84 -e115 -e18 -e63 -e10 -e2 -e26 -e24 -e43 -e35 -e133 -e86  -e64 -e19 */

#define DEBUG_MSG(args...) if (debug) printk (KERN_DEBUG "ebt_vlan_T: " args)
#define INV_FLAG(_inv_flag_) (info->invflags & _inv_flag_) ? "!" : ""
#define GET_BITMASK(_BIT_MASK_) info->bitmask & _BIT_MASK_
#define SET_BITMASK(_BIT_MASK_) info->bitmask |= _BIT_MASK_


#define VLAN_TARGET_VID_MASK 0xfff
#define VLAN_TARGET_PRI_MASK 0xe000


static int
ebt_target_vlan(struct sk_buff **pskb, unsigned int hooknr,
		const struct net_device *in,
		const struct net_device *out,
		const void *data, unsigned int datalen)
{
	struct ebt_vlan_t_info *info = (struct ebt_vlan_t_info *) data;
	struct vlan_hdr *vh, frame;

	unsigned short TCITmp;	

	
	
	/* raw socket (tcpdump) may have clone of incoming
			   skb: don't disturb it --RR */
	if (skb_shared(*pskb)||(skb_cloned(*pskb) && !(*pskb)->sk)) {

		
		struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
		if (!nskb)
			return NF_DROP;
		kfree_skb(*pskb);
		*pskb = nskb;
	}



	
	if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q)) {
		vh = skb_header_pointer(*pskb, 0, sizeof(frame), &frame);

		if (GET_BITMASK(EBT_VLAN_TARGET_ID))
		{
			TCITmp = ntohs (vh->h_vlan_TCI) & ~VLAN_TARGET_VID_MASK;
			vh->h_vlan_TCI = htons(TCITmp | info->id);
		}
		if (GET_BITMASK(EBT_VLAN_TARGET_PRIO))
		{
			TCITmp = ntohs (vh->h_vlan_TCI) & ~VLAN_TARGET_PRI_MASK;
			vh->h_vlan_TCI = htons(TCITmp | (info->prio << 13));
		}
	}
	(*pskb)->nfcache |= NFC_ALTERED;

	
	return info->target;
}


static int
ebt_target_vlan_check(const char *tablename,
	       unsigned int hooknr,
	       const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_vlan_t_info *info = (struct ebt_vlan_t_info *) data;

	/* Parameters buffer overflow check */
	if (datalen != EBT_ALIGN(sizeof(struct ebt_vlan_t_info))) {
		DEBUG_MSG
		    ("passed size %d is not eq to ebt_vlan_info (%Zd)\n",
		     datalen, sizeof(struct ebt_vlan_t_info));
		return -EINVAL;
	}

	/* Is it 802.1Q frame checked? */
	if (e->ethproto != __constant_htons(ETH_P_8021Q)) {
		DEBUG_MSG
		    ("passed entry proto %2.4X is not 802.1Q (8100)\n",
		     (unsigned short) ntohs(e->ethproto));
		return -EINVAL;
	}

	/* Check for bitmask range
	 * True if even one bit is out of mask */
	if (info->bitmask & ~EBT_VLAN_TARGET_MASK) {
		DEBUG_MSG("bitmask %2X is out of mask (%2X)\n",
			  info->bitmask, EBT_VLAN_TARGET_MASK);
		return -EINVAL;
	}


	/* Reserved VLAN ID (VID) values
	 * -----------------------------
	 * 0 - The null VLAN ID. 
	 * 1 - The default Port VID (PVID)
	 * 0x0FFF - Reserved for implementation use. 
	 * if_vlan.h: VLAN_GROUP_ARRAY_LEN 4096. */
	if (GET_BITMASK(EBT_VLAN_TARGET_ID)) {
		if (!!info->id) { /* if id!=0 => check vid range */
			if (info->id > VLAN_GROUP_ARRAY_LEN) {
				DEBUG_MSG
				    ("id %d is out of range (1-4094)\n",
				     info->id);
				return -EINVAL;
			}


	#if 0
			/* Note: This is valid VLAN-tagged frame point.
			 * Any value of user_priority are acceptable, 
			 * but should be ignored according to 802.1Q Std.
			 * So we just drop the prio flag. */
			info->bitmask &= ~EBT_VLAN_TARGET_PRIO;
	#endif
		}
	#if 0 /*w00135358 -- VLAN ID == 0 is OK for VLAN ID Remark -- 20080928 */	
		else
		{
			return -EINVAL;
		}
	#endif	
	}

	if (GET_BITMASK(EBT_VLAN_TARGET_PRIO)) {
		if ((unsigned char) info->prio > 7) {
			DEBUG_MSG("prio %d is out of range (0-7)\n",
			     info->prio);
			return -EINVAL;
		}
	}

	return 0;
}

static struct ebt_target vlan_target = {
	.name 			= EBT_VLAN_TARGET,
	.target 		= ebt_target_vlan,
	.check          = ebt_target_vlan_check,
	.me 			= THIS_MODULE,
};

static int __init init(void)
{
	DEBUG_MSG("ebtables 802.1Q remark extension module v"
		  MODULE_VERS "\n");
	DEBUG_MSG("module debug=%d\n", !!debug);
	return ebt_register_target(&vlan_target);
}

static void __exit fini(void)
{
	ebt_unregister_target(&vlan_target);
}

module_init(init);
module_exit(fini);
/*//lint -restore*/


