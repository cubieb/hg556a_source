#ifndef __LINUX_BRIDGE_EBT_MARK_T_H
#define __LINUX_BRIDGE_EBT_MARK_T_H

struct ebt_mark_t_info
{
	unsigned long mark;
	/*HUAWEI HGW c40163 2005-8-2 add begin */
	unsigned long mask;
	/*HUAWEI HGW c40163 2005-8-2 add end */
	/* EBT_ACCEPT, EBT_DROP, EBT_CONTINUE or EBT_RETURN */
	int target;
};
#define EBT_MARK_TARGET "mark"

#endif
