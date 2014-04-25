/* Shared library add-on to iptables to add MARK target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
/* For 64bit kernel / 32bit userspace */
#include "../include/linux/netfilter_ipv4/ipt_mark_j.h"

/* Function which prints out usage message. */
/* HUAWEI HGW c40163 2005-8-2 modify begin
static void
help(void)
{
	printf(
"MARK target v%s options:\n"
"  --set-mark value                   Set nfmark value\n"
"\n",
IPTABLES_VERSION);
}
*/
static void help(void)
{
	printf("MARK target v%s options:\n"
		"  --set-mark value[/mask]         Set nfmark value\n"
		"\n",
		IPTABLES_VERSION);
}
/* HUAWEI HGW c40163 2005-8-2 modify end */

static struct option opts[] = {
	{ "set-mark", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_mark_target_info *markinfo
		= (struct ipt_mark_target_info *)(*target)->data;
	char *end;	/*HUAWEI HGW c40163 2005-8-2 add */

	switch (c) {
	case '1':
#ifdef KERNEL_64_USERSPACE_32
		/*HUAWEI HGW c40163 2005-8-2 add begin */
		markinfo->mark = strtoull(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoull(end+1, &end, 0);
		} else {
			markifno->mask = 0xffffffffffffffffULL;
		}
		/*HUAWEI HGW c40163 2005-8-2 add begin */
#else
		/*HUAWEI HGW c40163 2005-8-2 add begin */
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoul(end+1, &end, 0);
		} else {
			markinfo->mask = 0xffffffff;
		}
		/*HUAWEI HGW c40163 2005-8-2 add begin */
#endif
		if (*end != '\0' || end == optarg) /*HUAWEI HGW c40163 2005-8-2 add */
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "MARK target: Can't specify --set-mark twice");
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
		           "MARK target: Parameter --set-mark is required");
}

/* HUAWEI HGW c40163 2005-8-2 modify begin
#ifdef KERNEL_64_USERSPACE_32
static void
print_mark(unsigned long long mark)
{
	printf("0x%llx ", mark);
}
#else
static void
print_mark(unsigned long mark)
{
	printf("0x%lx ", mark);
}
#endif
*/
#ifdef KERNEL_64_USERSPACE_32
static void
print_mark(unsigned long long mark, unsigned long long mask)
{
	if (mask != 0xffffffffffffffffULL) {
		printf("0x%llx/0x%llx ", mark, mask);
	}
	else {
		printf("0x%llx ", mark);
	}
}
#else
static void
print_mark(unsigned long mark, unsigned long mask)
{
	if (mask != 0xffffffff) {
		printf("0x%lx/0x%lx ", mark, mask);
	}
	else {
		printf("0x%lx ", mark);
	}
}
#endif
/* HUAWEI HGW c40163 2005-8-2 modify end */

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;
	printf("MARK set ");
	/* HUAWEI HGW c40163 2005-8-2 modify begin
	print_mark(markinfo->mark);
	*/
	print_mark(markinfo->mark, markinfo->mask);
	/* HUAWEI HGW c40163 2005-8-2 modify end */
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;

	printf("--set-mark ");
	/* HUAWEI HGW c40163 2005-8-2 modify begin
	print_mark(markinfo->mark);
	*/
	print_mark(markinfo->mark, markinfo->mask);
	/* HUAWEI HGW c40163 2005-8-2 modify end */
}

static
struct iptables_target mark
= { NULL,
    "MARK",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_mark_target_info)),
    IPT_ALIGN(sizeof(struct ipt_mark_target_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};

void _init(void)
{
	register_target(&mark);
}
