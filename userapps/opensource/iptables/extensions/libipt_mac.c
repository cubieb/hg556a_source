/* Shared library add-on to iptables to add MAC address support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_mac.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MAC v%s options:\n"
" --mac-source [!] XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n"
" --mac-dest   [!] XX:XX:XX:XX:XX:XX\n"
"				Match destination MAC address\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "mac-source", 1, 0, '1' },
	{ "mac-dest", 1, 0, '2' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
    struct ipt_mac_info *macinfo = (struct ipt_mac_info *)m->data;
    
	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;

	memset(macinfo, 0, sizeof(struct ipt_mac_info));
}

static void
parse_mac(const char *mac, struct ipt_mac_info *info, int flag)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		exit_error(PARAMETER_PROBLEM, "Bad mac address `%s'", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
	    {
	        if(flag == IPTMAC_SRC)
    			info->srcaddr[i] = number;
	        else if(flag == IPTMAC_DST)
    			info->dstaddr[i] = number;
	    }
		else
			exit_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_mac_info *macinfo = (struct ipt_mac_info *)(*match)->data;

	switch (c) {
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		parse_mac(argv[optind-1], macinfo, IPTMAC_SRC);
		if (invert)
			macinfo->invert |= IPTMAC_SRC;
		macinfo->flag |= IPTMAC_SRC;
		*flags |= IPTMAC_SRC;
		break;
    case '2':
        check_inverse(optarg, &invert, &optind, 0);
		parse_mac(argv[optind-1], macinfo, IPTMAC_DST);
		if (invert)
			macinfo->invert |= IPTMAC_DST;
		macinfo->flag |= IPTMAC_DST;
		*flags |= IPTMAC_DST;
		break;
	default:
		return 0;
	}

	return 1;
}

static void print_mac(unsigned char macaddress[ETH_ALEN])
{
	unsigned int i;

	printf("%02X", macaddress[0]);
	for (i = 1; i < ETH_ALEN; i++)
		printf(":%02X", macaddress[i]);
	printf(" ");
}

/* Final check; must have specified --mac. */
static void final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source' or '--mac-dest'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("MAC ");

    if(((struct ipt_mac_info *)match->data)->flag & IPTMAC_SRC){
    	printf("source ");
    	if (((struct ipt_mac_info *)match->data)->invert & IPTMAC_SRC)
    		printf("! ");
    	
    	print_mac(((struct ipt_mac_info *)match->data)->srcaddr);
    }
    if(((struct ipt_mac_info *)match->data)->flag & IPTMAC_DST){
    	printf("dest ");
    	if (((struct ipt_mac_info *)match->data)->invert & IPTMAC_DST)
    		printf("! ");
    	
    	print_mac(((struct ipt_mac_info *)match->data)->dstaddr);
    }
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
    if(((struct ipt_mac_info *)match->data)->flag & IPTMAC_SRC){
    	if (((struct ipt_mac_info *)match->data)->invert & IPTMAC_SRC)
    		printf("! ");

    	printf("--mac-source ");
    	print_mac(((struct ipt_mac_info *)match->data)->srcaddr);
    }

    if(((struct ipt_mac_info *)match->data)->flag & IPTMAC_DST){
    	if (((struct ipt_mac_info *)match->data)->invert & IPTMAC_DST)
    		printf("! ");

    	printf("--mac-dest ");
    	print_mac(((struct ipt_mac_info *)match->data)->dstaddr);
    }
}

static
struct iptables_match mac
= { NULL,
    "mac",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_mac_info)),
    IPT_ALIGN(sizeof(struct ipt_mac_info)),
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
	register_match(&mac);
}
