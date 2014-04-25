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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "../include/ebtables_u.h"
#include "../include/ethernetdb.h"
#include <linux/netfilter_bridge/ebt_vlan_t.h>
#include <linux/if_ether.h>

static int id_supplied;
static int prio_supplied;


#define GET_BITMASK(_MASK_) vlaninfo->bitmask & _MASK_
#define SET_BITMASK(_MASK_) vlaninfo->bitmask |= _MASK_
#define CHECK_IF_MISSING_VALUE if (optind > argc) print_error ("Missing %s value", opts[c].name);
#define CHECK_RANGE(_RANGE_) if (_RANGE_) print_error ("Invalid %s range", opts[c].name);

#define NAME_VLAN_TARGET_ID    "set-id"
#define NAME_VLAN_TARGET_PRIO  "set-prio"
#define NAME_VLAN_TARGET	   "vlan-target"

#define VLAN_TARGET_ID    0
#define VLAN_TARGET_PRIO  1
#define VLAN_TARGET		  2

static struct option opts[] = {
	{NAME_VLAN_TARGET_ID, required_argument, NULL,
	 VLAN_TARGET_ID},
	{NAME_VLAN_TARGET_PRIO, required_argument, NULL,
	 VLAN_TARGET_PRIO},
	{NAME_VLAN_TARGET, required_argument, 0, VLAN_TARGET },
	
	{NULL}
};

/*
 * option inverse flags definition 
 */
#define OPT_VLAN_TARGET_ID     0x01
#define OPT_VLAN_TARGET_PRIO   0x02
#define OPT_VLAN_TARGET			0x03



/*
 * Print out local help by "ebtables -h <target name>" 
 */

static void print_help()
{
#define HELP_TITLE "802.1Q VLAN target extension"

	printf(HELP_TITLE " options:\n");

	printf(
	" --set-id value     : Remark VLAN ID in VLAN-tagged frames (12 - bits field) \n"
	"			 : VLAN-tagged frame identifier, 0,1-4094 (integer)\n"
	" --set-prio value     : Remark VLAN Priority in VLAN-tagged frames (3 - bits field) \n"
	"			 : Priority-tagged frame user_priority, 0-7 (integer)\n"
	" --vlan-target target : ACCEPT, DROP, RETURN or CONTINUE\n");

}

/*
 * Initialization function 
 */
static void init(struct ebt_entry_target *target)
{
	struct ebt_vlan_t_info *vlaninfo =
	    (struct ebt_vlan_t_info *) target->data;
	/*
	 * Set initial values 
	 */
	vlaninfo->target = EBT_CONTINUE;	 
	vlaninfo->id = 1;	/* Default VID for VLAN-tagged 802.1Q frames */
	vlaninfo->prio = 0;
	vlaninfo->bitmask = 0;
	id_supplied = 0;
	prio_supplied = 0;
}




static int find_vlan_target(char *_str, int *p_pos)
{
	int _i, _ret = 0;								  
		for (_i = 0; _i < NUM_STANDARD_TARGETS; _i++)	
			if (!strcmp(_str, standard_targets[_i])) {
				*p_pos = -_i - 1; 				  
				break;							  
			}										 
		if (_i == NUM_STANDARD_TARGETS) 				
			_ret = 1;								 
		return _ret;	 


}

/*
 * Parse passed arguments values (ranges, flags, etc...)
 * int c - parameter number from static struct option opts[]
 * int argc - total amout of arguments (std argc value)
 * int argv - arguments (std argv value)
 * const struct ebt_u_entry *entry - default ebtables entry set
 * unsigned int *flags -
 * struct ebt_entry_target **target - 
 */
static int
parse(int c,
      char **argv,
      int argc,
      const struct ebt_u_entry *entry,
      unsigned int *flags, struct ebt_entry_target **target)
{
	struct ebt_vlan_t_info *vlaninfo =
	    (struct ebt_vlan_t_info *) (*target)->data;
	char *end;
	struct ebt_vlan_t_info local;
    memset(&local, 0, sizeof(struct ebt_vlan_t_info));
	switch (c) {
	case VLAN_TARGET_ID:
		check_option(flags, OPT_VLAN_TARGET_ID);
		CHECK_IF_MISSING_VALUE;
		local.id =
		    (unsigned short)strtoul(argv[optind - 1], &end, 10);
		CHECK_RANGE(local.id > 4094 || *end != '\0');
		vlaninfo->id = local.id;
		SET_BITMASK(EBT_VLAN_TARGET_ID);
		id_supplied = 1;
		break;

	case VLAN_TARGET_PRIO:
		check_option(flags, OPT_VLAN_TARGET_PRIO);
		CHECK_IF_MISSING_VALUE;
		local.prio =
		    (unsigned char)strtoul(argv[optind - 1], &end, 10);
		CHECK_RANGE(local.prio >= 8 || *end != '\0');
		vlaninfo->prio = local.prio;
		SET_BITMASK(EBT_VLAN_TARGET_PRIO);
		prio_supplied = 1;
		break;
	case VLAN_TARGET:
		check_option(flags, OPT_VLAN_TARGET);
		//if (FILL_TARGET(optarg, vlaninfo->target))
		if(find_vlan_target(optarg, &(vlaninfo->target)))
		{	
			printf("Illegal --vlan-target target.\n");
			exit(-1);
		}
		break;

	default:
		return 0;

	}
	return 1;
}

/*
 * Final check - logical conditions
 */
static void
final_check(const struct ebt_u_entry *entry,
	    const struct ebt_entry_target *target,
	    const char *name, unsigned int hookmask, unsigned int time)
{

	struct ebt_vlan_t_info *vlaninfo =
	    (struct ebt_vlan_t_info *) target->data;
	/*
	 * Specified proto isn't 802.1Q?
	 */
	if (entry->ethproto != ETH_P_8021Q || entry->invflags & EBT_IPROTO)
	{	
		printf("For use 802.1Q extension the protocol must be specified as 802_1Q");
		exit(-1);
	}
	/*start --  w00135358 ported from vlan match extension
	 * I don't know why vlan-id must be 0 if priority used.
	 */ 
#if 0
	/*
	 * Check if specified vlan-id=0 (priority-tagged frame condition) 
	 * when vlan-prio was specified.
	 */
	if (GET_BITMASK(EBT_VLAN_TARGET_PRIO)) {
		if (vlaninfo->id && GET_BITMASK(EBT_VLAN_TARGET_ID))
			print_error
			    ("For use user_priority the specified vlan-id must be 0");
	}
#endif
	if (GET_BITMASK(EBT_VLAN_TARGET_ID)) {
		if (time == 0 && id_supplied == 0)
		{	
			printf("No VLAN ID value supplied.\n");
			exit(-1);
		}
	}

	if (GET_BITMASK(EBT_VLAN_TARGET_PRIO)) {
		if (time == 0 && prio_supplied == 0)
		{	
			printf("No VLAN Priority value supplied.\n");
			exit(-1);
		}
	}

	if (BASE_CHAIN && vlaninfo->target == EBT_RETURN)
	{	
		printf("--vlan-target RETURN not allowed on base chain.\n");
		exit(-1);
	}	

}

/*
 * Print line when listing rules by ebtables -L 
 */
static void
print(const struct ebt_u_entry *entry, const struct ebt_entry_target *target)
{
	struct ebt_vlan_t_info *vlaninfo =
	    (struct ebt_vlan_t_info *) target->data;

	/*
	 * Print VLAN ID if they are specified 
	 */
	if (GET_BITMASK(EBT_VLAN_TARGET_ID)) {
		printf("--%s %d ",
		       opts[VLAN_TARGET_ID].name,vlaninfo->id);
	}
	/*
	 * Print user priority if they are specified 
	 */
	if (GET_BITMASK(EBT_VLAN_TARGET_PRIO)) {
		printf("--%s %d ",
		       opts[VLAN_TARGET_PRIO].name,vlaninfo->prio);
	}
	/*
	 * Print Vlan Target  
	 */
	if (vlaninfo->target == EBT_ACCEPT)
		return;
	printf(" --vlan-target %s", TARGET_NAME(vlaninfo->target));
	
}


static int
compare(const struct ebt_entry_target *vlan1,
	const struct ebt_entry_target *vlan2)
{
	struct ebt_vlan_t_info *vlaninfo1 =
	    (struct ebt_vlan_t_info *) vlan1->data;
	struct ebt_vlan_t_info *vlaninfo2 =
	    (struct ebt_vlan_t_info *) vlan2->data;
	/*
	 * Compare argc 
	 */
	if (vlaninfo1->bitmask != vlaninfo2->bitmask)
		return 0;
	
	/*
	 * Compare VLAN ID if they are present 
	 */
	if (vlaninfo1->bitmask & EBT_VLAN_TARGET_ID) {
		if (vlaninfo1->id != vlaninfo2->id)
			return 0;
	};
	/*
	 * Compare VLAN Prio if they are present 
	 */
	if (vlaninfo1->bitmask & EBT_VLAN_TARGET_PRIO) {
		if (vlaninfo1->prio != vlaninfo2->prio)
			return 0;
	};

	return 1;
}

static struct ebt_u_target vlan_target = {
	EBT_VLAN_TARGET,
	sizeof(struct ebt_vlan_t_info),
	print_help,
	init,
	parse,
	final_check,
	print,
	compare,
	opts,
};

static void _init(void) __attribute__ ((constructor));
static void _init(void)
{
	register_target(&vlan_target);
}
