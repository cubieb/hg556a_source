#
# Makefile for the netfilter modules on top of IPv4.
#

# objects for the conntrack and NAT core (used by standalone and backw. compat)
# CONFIG_MIPS_BRCM Begin Broadcom changed code. Add ESP
ip_nf_conntrack-objs	:= ip_conntrack_core.o ip_conntrack_proto_generic.o ip_conntrack_proto_tcp.o ip_conntrack_proto_udp.o ip_conntrack_proto_icmp.o ip_conntrack_proto_esp.o
ip_nf_nat-objs		:= ip_nat_core.o ip_nat_helper.o ip_nat_proto_unknown.o ip_nat_proto_tcp.o ip_nat_proto_udp.o ip_nat_proto_icmp.o ip_nat_proto_esp.o
# CONFIG_MIPS_BRCM End Broadcom changed code.

# objects for the standalone - connection tracking / NAT
ip_conntrack-objs	:= ip_conntrack_standalone.o $(ip_nf_conntrack-objs)
iptable_nat-objs	:= ip_nat_standalone.o ip_nat_rule.o $(ip_nf_nat-objs)

# objects for backwards compatibility mode
ip_nf_compat-objs	:= ip_fw_compat.o ip_fw_compat_redir.o ip_fw_compat_masq.o $(ip_nf_conntrack-objs) $(ip_nf_nat-objs)

ipfwadm-objs		:= $(ip_nf_compat-objs) ipfwadm_core.o
ipchains-objs		:= $(ip_nf_compat-objs) ipchains_core.o

# connection tracking
obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

# connection tracking helpers
obj-$(CONFIG_IP_NF_AMANDA) += ip_conntrack_amanda.o
obj-$(CONFIG_IP_NF_TFTP) += ip_conntrack_tftp.o
obj-$(CONFIG_IP_NF_FTP) += ip_conntrack_ftp.o
obj-$(CONFIG_IP_NF_IRC) += ip_conntrack_irc.o
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o
obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o ip_conntrack_gre.o
#obj-$(CONFIG_IP_NF_RTSP) += ip_conntrack_rtsp.o

# NAT helpers 
obj-$(CONFIG_IP_NF_NAT_AMANDA) += ip_nat_amanda.o
obj-$(CONFIG_IP_NF_NAT_TFTP) += ip_nat_tftp.o
obj-$(CONFIG_IP_NF_NAT_FTP) += ip_nat_ftp.o
obj-$(CONFIG_IP_NF_NAT_IRC) += ip_nat_irc.o
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o
obj-$(CONFIG_IP_NF_NAT_PPTP) += ip_nat_pptp.o ip_nat_gre.o
#obj-$(CONFIG_IP_NF_NAT_RTSP) += ip_nat_rtsp.o

# CONFIG_MIPS_BRCM Begin Broadcom changed code.
BCMALG_DIR := ./broadcom
# DirectX8  support
obj-$(CONFIG_IP_NF_DTX8) += $(BCMALG_DIR)/ip_conntrack_dtx8.o $(BCMALG_DIR)/ip_nat_dtx8.o

# MicroSoft Media Player UDP  support
obj-$(CONFIG_IP_NF_WM) += $(BCMALG_DIR)/ip_conntrack_wm.o $(BCMALG_DIR)/ip_nat_wm.o

# Port Triggerng  support
obj-$(CONFIG_IP_NF_PT) += $(BCMALG_DIR)/ip_conntrack_pt.o $(BCMALG_DIR)/ip_nat_pt.o

# IPSec support
obj-$(CONFIG_IP_NF_IPSEC) += $(BCMALG_DIR)/ip_conntrack_ipsec.o
obj-$(CONFIG_IP_NF_NAT_IPSEC) += $(BCMALG_DIR)/ip_nat_ipsec.o

# CONFIG_MIPS_BRCM End Broadcom changed code.

# generic IP tables 
obj-$(CONFIG_IP_NF_IPTABLES) += ip_tables.o

# the three instances of ip_tables
obj-$(CONFIG_IP_NF_FILTER) += iptable_filter.o
obj-$(CONFIG_IP_NF_MANGLE) += iptable_mangle.o
obj-$(CONFIG_IP_NF_NAT) += iptable_nat.o
obj-$(CONFIG_IP_NF_RAW) += iptable_raw.o

# matches
obj-$(CONFIG_IP_NF_MATCH_HELPER) += ipt_helper.o
obj-$(CONFIG_IP_NF_MATCH_LIMIT) += ipt_limit.o
obj-$(CONFIG_IP_NF_MATCH_MARK) += ipt_mark.o
obj-$(CONFIG_IP_NF_MATCH_MAC) += ipt_mac.o
obj-$(CONFIG_IP_NF_MATCH_IPRANGE) += ipt_iprange.o

obj-$(CONFIG_IP_NF_MATCH_PKTTYPE) += ipt_pkttype.o
obj-$(CONFIG_IP_NF_MATCH_MULTIPORT) += ipt_multiport.o
obj-$(CONFIG_IP_NF_MATCH_OWNER) += ipt_owner.o
obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o

obj-$(CONFIG_IP_NF_MATCH_RECENT) += ipt_recent.o

obj-$(CONFIG_IP_NF_MATCH_ECN) += ipt_ecn.o
obj-$(CONFIG_IP_NF_MATCH_DSCP) += ipt_dscp.o
obj-$(CONFIG_IP_NF_MATCH_AH_ESP) += ipt_ah.o ipt_esp.o

obj-$(CONFIG_IP_NF_MATCH_LENGTH) += ipt_length.o

obj-$(CONFIG_IP_NF_MATCH_TTL) += ipt_ttl.o
obj-$(CONFIG_IP_NF_MATCH_STATE) += ipt_state.o
obj-$(CONFIG_IP_NF_MATCH_CONNLIMIT) += ipt_connlimit.o
obj-$(CONFIG_IP_NF_MATCH_CONNTRACK) += ipt_conntrack.o
obj-$(CONFIG_IP_NF_MATCH_TCPMSS) += ipt_tcpmss.o
obj-$(CONFIG_IP_NF_MATCH_REALM) += ipt_realm.o
obj-$(CONFIG_IP_NF_MATCH_ADDRTYPE) += ipt_addrtype.o

obj-$(CONFIG_IP_NF_MATCH_PHYSDEV) += ipt_physdev.o
obj-$(CONFIG_IP_NF_MATCH_TIME) += ipt_time.o
obj-$(CONFIG_IP_NF_MATCH_MISC) += ipt_misc.o

# targets
obj-$(CONFIG_IP_NF_TARGET_REJECT) += ipt_REJECT.o
obj-$(CONFIG_IP_NF_TARGET_TOS) += ipt_tos_j.o
# CONFIG_MIPS_BRCM Begin Broadcom changed code.
obj-$(CONFIG_IP_NF_TARGET_FTOS) += ipt_FTOS.o
# CONFIG_MIPS_BRCM End Broadcom changed code.
obj-$(CONFIG_IP_NF_TARGET_ECN) += ipt_ecn_j.o
obj-$(CONFIG_IP_NF_TARGET_DSCP) += ipt_dscp_j.o
obj-$(CONFIG_IP_NF_TARGET_MARK) += ipt_mark_j.o
obj-$(CONFIG_IP_NF_TARGET_MASQUERADE) += ipt_MASQUERADE.o
obj-$(CONFIG_IP_NF_TARGET_REDIRECT) += ipt_REDIRECT.o
obj-$(CONFIG_IP_NF_TARGET_NETMAP) += ipt_NETMAP.o
obj-$(CONFIG_IP_NF_TARGET_SAME) += ipt_SAME.o
obj-$(CONFIG_IP_NF_TARGET_CLASSIFY) += ipt_CLASSIFY.o
obj-$(CONFIG_IP_NF_NAT_SNMP_BASIC) += ip_nat_snmp_basic.o
obj-$(CONFIG_IP_NF_TARGET_LOG) += ipt_LOG.o
obj-$(CONFIG_IP_NF_TARGET_ULOG) += ipt_ULOG.o
obj-$(CONFIG_IP_NF_TARGET_TCPMSS) += ipt_tcpmss_j.o
obj-$(CONFIG_IP_NF_TARGET_NOTRACK) += ipt_NOTRACK.o

# generic ARP tables
obj-$(CONFIG_IP_NF_ARPTABLES) += arp_tables.o
obj-$(CONFIG_IP_NF_ARP_MANGLE) += arpt_mangle.o

# just filtering instance of ARP tables for now
obj-$(CONFIG_IP_NF_ARPFILTER) += arptable_filter.o

# backwards compatibility 
obj-$(CONFIG_IP_NF_COMPAT_IPCHAINS) += ipchains.o
obj-$(CONFIG_IP_NF_COMPAT_IPFWADM) += ipfwadm.o

obj-$(CONFIG_IP_NF_QUEUE) += ip_queue.o
obj-$(CONFIG_IP_NF_MATCH_HASHLIMIT) += ipt_hashlimit.o
obj-$(CONFIG_IP_NF_MATCH_STRING) += ipt_string.o
