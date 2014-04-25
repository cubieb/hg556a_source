#ifndef _CLIENTPACKET_H
#define _CLIENTPACKET_H

unsigned long random_xid(void);
int send_discover(unsigned long xid, unsigned long requested);
int send_selecting(unsigned long xid, unsigned long server, unsigned long requested);
int send_renew(unsigned long xid, unsigned long server, unsigned long ciaddr);
/* BEGIN: Modified by y67514, 2008/2/20   PN:AU8D00203*/
int send_renew_with_request(unsigned long xid, unsigned long server, unsigned long ciaddr);
/* END:   Modified by y67514, 2008/2/20 */
int send_release(unsigned long server, unsigned long ciaddr);
int get_raw_packet(struct dhcpMessage *payload, int fd);

#endif
