#ifndef _SCRIPT_H
#define _SCRIPT_H

void run_script(struct dhcpMessage *packet, const char *name);

/*start of  2008.05.04 HG553V100R001C02B013 AU8D00566 */
void run_staticRoute(); //for option121
/*end of  2008.05.04 HG553V100R001C02B013 AU8D00566 */

#endif
