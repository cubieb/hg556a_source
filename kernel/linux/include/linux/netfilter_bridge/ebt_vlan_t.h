#ifndef __LINUX_BRIDGE_EBT_VLAN_T_H
#define __LINUX_BRIDGE_EBT_VLAN_T_H

#define EBT_VLAN_TARGET_ID	0x01
#define EBT_VLAN_TARGET_PRIO	0x02

#define EBT_VLAN_TARGET_MASK (EBT_VLAN_TARGET_ID | EBT_VLAN_TARGET_PRIO | 0x00)
struct ebt_vlan_t_info
{
	uint16_t id;		/* VLAN ID {1-4095} */
	uint8_t prio;		/* VLAN User Priority {0-7} */
	uint8_t bitmask;		/* Args bitmask bit 1=1 - ID arg,
				   bit 2=1 User-Priority arg*/
	// EBT_ACCEPT, EBT_DROP or EBT_CONTINUE or EBT_RETURN
	int target;
};
#define EBT_VLAN_TARGET "vlan_t"

#endif
