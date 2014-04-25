#ifndef _IPT_MARK_H_target
#define _IPT_MARK_H_target

/* HUAWEI HGW c40163 2005-8-2 modify begin
struct ipt_mark_target_info {
#ifdef KERNEL_64_USERSPACE_32
	unsigned long long mark;
#else
	unsigned long mark;
#endif
};
*/
struct ipt_mark_target_info {
#ifdef KERNEL_64_USERSPACE_32
	unsigned long long mark;
	unsigned long long mask;
#else
	unsigned long mark;
	unsigned long mask;
#endif
};
/* HUAWEI HGW c40163 2005-8-2 modify end */

#endif /*_IPT_MARK_H_target*/
