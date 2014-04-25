/* String matching match for iptables
 * 
 * (C) 2005 Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_string.h>
#include <linux/textsearch.h>

MODULE_AUTHOR("Pablo Neira Ayuso <pablo@eurodev.net>");
MODULE_DESCRIPTION("IP tables string match module");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_string");
MODULE_ALIAS("ip6t_string");

static int match/*(const struct sk_buff *skb,
		 const struct net_device *in,
		 const struct net_device *out,
		 const struct xt_match *match,
		 const void *matchinfo,
		 int offset,
		 unsigned int protoff,
		 int *hotdrop)*/
                (const struct sk_buff *skb,
                const struct net_device *in,
                const struct net_device *out,
                const void *matchinfo,
                int offset,
                int *hotdrop)
{
	const struct xt_string_info *conf = matchinfo;
	struct ts_state state;

	memset(&state, 0, sizeof(struct ts_state));

	return (skb_find_text((struct sk_buff *)skb, conf->from_offset, 
			     conf->to_offset, conf->config, &state) 
			     != UINT_MAX) ^ conf->invert;
}

#define STRING_TEXT_PRIV(m) ((struct xt_string_info *) m)

static int checkentry/*(const char *tablename,
		      const void *ip,
		      const struct xt_match *match,
		      void *matchinfo,
		      unsigned int hook_mask)*/
		      (
                const char *tablename,
                const struct ipt_ip *ip,
                void *matchinfo,
                unsigned int matchsize,
                unsigned int hook_mask)
{
	struct xt_string_info *conf = matchinfo;
	struct ts_config *ts_conf;

	/* Damn, can't handle this case properly with iptables... */
	if (conf->from_offset > conf->to_offset)
		return 0;
	if (conf->algo[XT_STRING_MAX_ALGO_NAME_SIZE - 1] != '\0')
	    	return 0;
	if (conf->patlen > XT_STRING_MAX_PATTERN_SIZE)
		return 0;
	ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
				     GFP_KERNEL, TS_AUTOLOAD);
	if (IS_ERR(ts_conf))
		return 0;

	conf->config = ts_conf;

	return 1;
}

static void destroy(/*const struct xt_match *match,*/ void *matchinfo, unsigned int matchinfosize)
{
	textsearch_destroy(STRING_TEXT_PRIV(matchinfo)->config);
}

static struct ipt_match xt_string_match = {
		.name 		= "string",
//		.family		= AF_INET,
		.checkentry	= checkentry,
		.match 		= match,
		.destroy 	= destroy,
//		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
};

static int __init xt_string_init(void)
{
	return ipt_register_match(&xt_string_match);
}

static void __exit xt_string_fini(void)
{
	ipt_unregister_match(&xt_string_match);
}

module_init(xt_string_init);
module_exit(xt_string_fini);
