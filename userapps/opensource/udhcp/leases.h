/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H


struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;	/* network order */
	u_int32_t expires;	/* host order */
	char hostname[64];
//w44771 add for test
#ifdef SUPPORT_CHINATELECOM_DHCP
      unsigned int port;
#endif
	// BRCM
	char vendorid[64];
};

/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
struct domainTab 
{
    u_int8_t chaddr[16];   /*MAC地址*/
    char domain[40];          /*域名*/
    u_int32_t yiaddr;	/* IP地址，network order */
};
/* END:   Added by y67514, 2008/9/18 */

void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr);
void expire_lease(u_int8_t *chaddr, u_int32_t yiaddr);

#ifdef SUPPORT_CHINATELECOM_DHCP
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, unsigned int port);
#else
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease);
#endif
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr);
// 061208 add by c60023298
struct dhcpOfferedAddr *find_lease_two_by_chaddr(u_int8_t *chaddr);
//end

struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr);
u_int32_t find_address(int check_expired);
int check_ip(u_int32_t addr);
#ifdef SUPPORT_DHCP_FRAG
u_int32_t find_address2(int check_expired); 
#endif

void update_lease(int dumy);

#endif
