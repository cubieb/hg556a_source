/* 
 * options.c -- DHCP server option packet tools 
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#ifdef VDF_OPTION
//add for option125
#include "../../broadcom/tr69c/inc/tr69cdefs.h"
#include "ifcdefs.h"
#include "../../broadcom/cfm/inc/board_api.h"
#endif


/* supported options are easily added here */
struct dhcp_option options[] = {
	/* name[10]	flags				code */
	{"subnet",	OPTION_IP,			0x01},
	{"timezone",	OPTION_S32,			0x02},
	#ifdef VDF_OPTION
	{"option121",	OPTION_STRING ,		0x79},
	#endif
	{"router",	OPTION_IP | OPTION_LIST,	0x03},
	{"timesvr",	OPTION_IP | OPTION_LIST,	0x04},
	{"namesvr",	OPTION_IP | OPTION_LIST,	0x05},
	{"dns",		OPTION_IP | OPTION_LIST,	0x06},
	{"logsvr",	OPTION_IP_STRING,	0x07},//w44771 modify for more dhcp options
	{"cookiesvr",	OPTION_IP | OPTION_LIST,	0x08},
	{"lprsvr",	OPTION_IP | OPTION_LIST,	0x09},
	{"hostname",	OPTION_STRING,			0x0c},
	{"bootsize",	OPTION_U16,			0x0d},
	{"domain",	OPTION_STRING,			0x0f},
	{"swapsvr",	OPTION_IP,			0x10},
	{"rootpath",	OPTION_STRING,			0x11},
	{"ipttl",	OPTION_U8,			0x17},
	{"mtu",		OPTION_U16,			0x1a},
	{"broadcast",	OPTION_IP,			0x1c},
/* j00100803 Add Begin 2008-04-16 */
#ifdef SUPPORT_VDF_DHCP
    {"ntpsvr",  OPTION_IP | OPTION_LIST, 0x2a},
/* j00100803 Add Begin 2008-05-20 for option43 */
    {"option43", OPTION_STRING, 0x2b},
/* j00100803 Add End 2008-05-20 for option43 */
#else
	{"ntpsvr",	OPTION_IP_STRING,	0x2a},//w44771 modify for more dhcp options
#endif
/* j00100803 Add End 2008-04-16 */
	{"tr069acs",   OPTION_STRING,          0x2b},//w44771 add for option43
	{"wins",	OPTION_IP | OPTION_LIST,	0x2c},
	{"requestip",	OPTION_IP,			0x32},
	{"lease",	OPTION_U32,			0x33},
	{"dhcptype",	OPTION_U8,			0x35},
	{"serverid",	OPTION_IP,			0x36},
/* j00100803 Add Begin 2008-04-15 */
#ifdef SUPPORT_VDF_DHCP
    {"renewal", OPTION_U32, 0x3a},
    {"rebinding", OPTION_U32, 0x3b},
#endif
/* j00100803 Add End 2008-04-15 */
#ifdef SUPPORT_OPTION61
       {"clientid", OPTION_STRING,               0x3d},
#endif
	{"tftp",	OPTION_IP_STRING,			0x42},//w44771 modify for more dhcp options
	{"bootfile",	OPTION_STRING,			0x43},
#ifdef SUPPORT_DHCPOPTIONS
        {"option240",	OPTION_STRING,			0xF0},
        {"option241",	OPTION_STRING,			0xF1},//w44771 modify for option 241 and 242
        {"option242",	OPTION_STRING,			0xF2},
        {"option243",	OPTION_STRING,			0xF3},
        {"option244",	OPTION_STRING,			0xF4},
        {"option245",	OPTION_STRING,			0xF5},
#endif
	{"",		0x00,				0x00}
};

/* Lengths of the different option types */
int option_lengths[] = {
	[OPTION_IP] =		4,
	[OPTION_IP_PAIR] =	8,
	[OPTION_BOOLEAN] =	1,
	[OPTION_STRING] =	0,
	[OPTION_U8] =		1,
	[OPTION_U16] =		2,
	[OPTION_S16] =		2,
	[OPTION_U32] =		4,
	[OPTION_S32] =		4,
	[OPTION_IP_STRING]=128 //w44771 add for more dhcp options
};


/* get an option with bounds checking (warning, not aligned). */
unsigned char *get_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	static char err[] = "bogus packet, option fields too long."; /* save a few bytes */
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			LOG(LOG_WARNING, err);
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, err);
				return NULL;
			}
			return optionptr + i + 2;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, err);
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}


/* return the position of the 'end' option (no bounds checking) */
int end_option(unsigned char *optionptr) 
{
	int i = 0;
	
	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}


/* add an option string to the options (an option string contains an option code,
 * length, then data) */
int add_option_string(unsigned char *optionptr, unsigned char *string)
{
	int i, end = end_option(optionptr);
	
	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= 1024) {
		for (i = 0; options[i].code && options[i].code != string[OPT_CODE]; i++);
		LOG(LOG_ERR, "Option %s (0x%02x) did not fit into the packet!", 
			options[i].code ? options[i].name : "unknown", string[OPT_CODE]);
		return 0;
	}
	DEBUG(LOG_INFO, "adding option 0x%02x", string[OPT_CODE]);
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}


/* add a one to four byte option to a packet */
int add_simple_option(unsigned char *optionptr, unsigned char code, u_int32_t data)
{
	char length = 0;
	int i, end;
	char buffer[4]; /* Cant copy straight to optionptr, it might not be aligned */

	for (i = 0; options[i].code; i++)
		if (options[i].code == code) {
			length = option_lengths[options[i].flags & TYPE_MASK];
			break;
		}
		
	if (!length) {
		DEBUG(LOG_ERR, "Could not add option 0x%02x", code);
		return 0;
	}
	
	DEBUG(LOG_INFO, "adding option 0x%02x", code);
	end = end_option(optionptr);
	optionptr[end + OPT_CODE] = code;
	optionptr[end + OPT_LEN] = length;

	switch (length) {
		case 1: buffer[0] = (char) data; break;
		case 2: *((u_int16_t *) buffer) = htons(data); break;
		case 4: *((u_int32_t *) buffer) = htonl(data); break;
	}
	memcpy(&optionptr[end + 2], buffer, length);
	optionptr[end + length + 2] = DHCP_END;
	return length;
}


/* find option 'code' in opt_list */
struct option_set *find_option(struct option_set *opt_list, char code)
{
	while (opt_list && opt_list->data[OPT_CODE] != (unsigned char)code)//w44771 modify
		opt_list = opt_list->next;

	if (opt_list && opt_list->data[OPT_CODE] == (unsigned char)code) return opt_list;
	else return NULL;
}


/* add an option to the opt_list */
void attach_option(struct option_set **opt_list, struct dhcp_option *option, char *buffer, int length)
{
	struct option_set *existing, *new, **curr;

	/* add it to an existing option */
	if ((existing = find_option(*opt_list, option->code))) {
		DEBUG(LOG_INFO, "Attaching option %s to existing member of list", option->name);
		if ((option->flags & OPTION_LIST) ||
		     (((unsigned char)(option->code) >= 240) && ((unsigned char)(option->code) <= 245))) { //w44771 add
			if (existing->data[OPT_LEN] + length <= 255) {
				existing->data = realloc(existing->data, 
						existing->data[OPT_LEN] + length + 2);
				memcpy(existing->data + existing->data[OPT_LEN] + 2, buffer, length);
				existing->data[OPT_LEN] += length;
			} /* else, ignore the data, we could put this in a second option in the future */
		} /* else, ignore the new data */
	} else {
		DEBUG(LOG_INFO, "Attaching option %s to list", option->name);
		
		/* make a new option */
		new = malloc(sizeof(struct option_set));
		new->data = malloc(length + 2);
		new->data[OPT_CODE] = option->code;
		new->data[OPT_LEN] = length;
		memcpy(new->data + 2, buffer, length);
		
		curr = opt_list;
		while (*curr && (*curr)->data[OPT_CODE] < option->code)
			curr = &(*curr)->next;
			
		new->next = *curr;
		*curr = new;		
	}
}
#ifdef VDF_OPTION
//add for option125
void viInfoFree(pVI_OPTION_INFO pInfo)
{
  if (pInfo) {
    free(pInfo->oui);
    free(pInfo->serialNumber);
    free(pInfo->productClass);
    free(pInfo);
  }
}

void addViToList(pVI_OPTION_INFO pNew) {
  pVI_OPTION_INFO pPtr= NULL;

  /* if VI exists already, don't add, just update the info */
  if (viList->count > 0) {
    pPtr = viList->pHead;
    while (pPtr) {
      if (pPtr->ipAddr == pNew->ipAddr) {
	/* found it, just copy the info over */
	memcpy(pPtr,pNew,sizeof(VI_OPTION_INFO));
	viInfoFree(pNew);
	return;
      }
      pPtr = pPtr->next;
    } /* while */
  } /* list has something */

  if (viList->pHead == NULL) {
    viList->pHead = pNew;
    viList->pTail = pNew;
  }
  else {
    viList->pTail->next = pNew;
    viList->pTail = pNew;
  }
  viList->count++;
}

void viListFree(void)
{
  pVI_OPTION_INFO pInfo;

  while (viList->pHead) {
    pInfo = viList->pHead;
    viList->pHead = viList->pHead->next;
    free(pInfo->oui);
    free(pInfo->serialNumber);
    free(pInfo->productClass);
    free(pInfo);
  }
  viList->count = 0;
}


/* this function generates VI info based on TR111 part I.
   If -1 is return, there is no VI info found.  Otherwise, 0 is returned.
   Type specifies gateway vendor info or device vendor info.   
   VIinfo is where the option string is stored */
int createVIoption(int type, char *VIinfo)
{
  FILE* fs;
  char optionData[VENDOR_IDENTIFYING_INFO_LEN], *dataPtr;
  int len, totalLen = 0;
  char line[VENDOR_IDENTIFYING_INFO_LEN];
  char oui[7];
  char serialNum[64];
  char productClass[64];
  char buf[7];
  memset(oui,0,7);
  memset(serialNum,0,64);
  memset(productClass,0,64);
	boardIoctl(BOARD_IOCTL_EQUIPMENT_TEST, GET_SERIAL_NUMBER, serialNum, NVRAM_SERIAL_NUMBER_LEN+1, 0, "");
    	boardIoctl(BOARD_IOCTL_EQUIPMENT_TEST, GET_BASE_MAC_ADDRESS, buf, 6, 0, "");

	sprintf(oui, "%02x%02x%02x", (unsigned char)buf[0], (unsigned char)buf[1], (unsigned char)buf[2]);
	strcpy(productClass,"EchoLife");
	
  optionData[VENDOR_OPTION_CODE_OFFSET] = (char)VENDOR_IDENTIFYING_OPTION_CODE;
  *(unsigned int*)(optionData+VENDOR_OPTION_ENTERPRISE_OFFSET) = (unsigned int)VENDOR_ADSL_FORUM_ENTERPRISE_NUMBER;
  dataPtr = optionData + VENDOR_OPTION_DATA_OFFSET;
  totalLen = VENDOR_ENTERPRISE_LEN + 1;/*1是data-len占用的字节数*/
  /* read system information and add it to option data */
  /* OUI */
  if (type == VENDOR_IDENTIFYING_FOR_DEVICE)
    *dataPtr++ = (char)VENDOR_DEVICE_OUI_SUBCODE;
  else 
    *dataPtr++ = (char)VENDOR_GATEWAY_OUI_SUBCODE;
  len = strlen(oui);
  *dataPtr++ = len;
  strncpy(dataPtr,oui,len);
  dataPtr += len;
  totalLen += (len + VENDOR_SUBCODE_AND_LEN_BYTES);
  
  /* Serial Number */
  if (type == VENDOR_IDENTIFYING_FOR_DEVICE)
    *dataPtr++ = (char)VENDOR_DEVICE_SERIAL_NUMBER_SUBCODE;
  else
    *dataPtr++ = (char)VENDOR_GATEWAY_SERIAL_NUMBER_SUBCODE;
  len = strlen(serialNum);
  *dataPtr++ = len;
  strncpy(dataPtr,(const char*)serialNum,len);
  dataPtr += len;
  totalLen += (len + VENDOR_SUBCODE_AND_LEN_BYTES);

  /* Product Class */
  if (type == VENDOR_IDENTIFYING_FOR_DEVICE)
    *dataPtr++ = VENDOR_DEVICE_PRODUCT_CLASS_SUBCODE;
  else 
    *dataPtr++ = VENDOR_GATEWAY_PRODUCT_CLASS_SUBCODE;
  len = strlen(productClass);
  *dataPtr++ = len;
  strncpy(dataPtr,(const char*)productClass,len);
  dataPtr += len;
  totalLen += (len + VENDOR_SUBCODE_AND_LEN_BYTES);

  optionData[VENDOR_OPTION_LEN_OFFSET] = totalLen;
  optionData[VENDOR_OPTION_DATA_LENTH_OFFSET] = totalLen - (VENDOR_OPTION_DATA_LENTH_OFFSET - VENDOR_OPTION_LEN_OFFSET);

  /* also copy the option code and option len which is not counted in total len */
  memcpy((void*)VIinfo,(void*)optionData,(totalLen+VENDOR_OPTION_DATA_OFFSET));

  return 0;
}
/* udp_send and readIp from voice */
static int readIp(const char* ip)
{
   int n = 0;
   int res = 0;
   
   while (n < 4 && *ip)
   {
      if (isdigit(*ip)) 
      {
         res = (res << 8) | atoi(ip);
         n++;
         while (isdigit(*ip)) 
         {
            ip++;
         }
      } 
      else 
      {
         ip++;
		}
   }
   return res;
}

static int notifyApp()
{
	return TR069_SendMessage(CWMP_MSG_ASSOC_DEVICE, NULL, 0);
	#if 0
   int sockfd;
   struct sockaddr_in serv_addr;
   struct sockaddr_in cli_addr;
   
   /* fill in server address */
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family      = AF_INET;
   serv_addr.sin_addr.s_addr = readIp("127.0.0.1");
   serv_addr.sin_port        = htons(port);

   /* open udp socket */
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   {
      printf("WT-104: Could not open socket for send\n");
      return -1; /* could not open socket */
   }

   /* bind any local address for us */ 
   memset(&cli_addr, 0, sizeof(cli_addr));
   cli_addr.sin_family      = AF_INET;
   cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   cli_addr.sin_port        = htons(0);

   if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) 
   {
      printf("dhcpd: Could not bind client socket\n");
      return -2; /* could not bind client socket */
   }
 
   /* send the data */
   if (sendto(sockfd, data, len, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != len) 
   {
      printf( "dhcpd: Could not sendto\n");
      return -3; /* could not sendto */
   }
   close(sockfd);
   return 0;
   #endif
}

/* this function parses received VI info, and save it.
   If -1 is return, there is no VI info found; invalid option.  Otherwise, 0 is returned.
   *option is the received option string to parse; it points to optionData.
   ipAddr is the device's offered address or device's ip address */
int saveVIoption(char *option, u_int32_t ipAddr)
{
  char *optionPtr;

  int maxSubcode = 3;
  int subcodeParsed = 0;
  int subcode, sublen;
  pVI_OPTION_INFO pInfo = NULL;
  int ret = 0;
  int msg = 0;

	if (option == NULL) 
	{
		DEBUG(LOG_ERR, "saveVIoption(): option is NULL.");
		printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
		return -1;
	}

	optionPtr = option;
	pInfo = malloc(sizeof(VI_OPTION_INFO));
	memset(pInfo,0,sizeof(VI_OPTION_INFO));
	pInfo->enterprise = *(unsigned int*)(optionPtr);
	//pInfo->enterprise = 3561;
	pInfo->ipAddr = ipAddr;
	optionPtr = optionPtr + VENDOR_ENTERPRISE_LEN + 1;

	while (subcodeParsed < maxSubcode) 
	{
		/* subcode, len, data */
		subcode = *optionPtr++;
		sublen = *optionPtr++;
		subcodeParsed++;
		switch (subcode)
		{
			case VENDOR_DEVICE_OUI_SUBCODE:
			case VENDOR_GATEWAY_OUI_SUBCODE:
				if (sublen <= VENDOR_GATEWAY_OUI_MAX_LEN) 
				{
					if ((pInfo->oui = malloc(sublen+1)) != NULL) 
					{
						memcpy(pInfo->oui,optionPtr,sublen);
						pInfo->oui[sublen] = '\0';
					}
					else	
					{
						printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
						goto viError;
					}
				}
				else 
				{
					printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
					DEBUG(LOG_ERR, "saveVIoption(): subcode OUI, OUI len %d is too long.",sublen);
					goto viError;
				}
				break;
			case VENDOR_DEVICE_SERIAL_NUMBER_SUBCODE:
			case VENDOR_GATEWAY_SERIAL_NUMBER_SUBCODE:
				if (sublen <= VENDOR_GATEWAY_SERIAL_NUMBER_MAX_LEN) 
				{
					if ((pInfo->serialNumber = malloc(sublen+1)) != NULL) 
					{
						memcpy(pInfo->serialNumber,optionPtr,sublen);
						pInfo->serialNumber[sublen] = '\0';
					}
					else
					{
						printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
						goto viError;
					}
				}
				else 
				{
					printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
					DEBUG(LOG_ERR, "saveVIoption(): subcode SerialNumber, Serial Number len %d is too long.",sublen);
					goto viError;
				}
				break;
			case VENDOR_DEVICE_PRODUCT_CLASS_SUBCODE:
			case VENDOR_GATEWAY_PRODUCT_CLASS_SUBCODE:
				if (sublen <= VENDOR_GATEWAY_PRODUCT_CLASS_MAX_LEN) 
				{
					if ((pInfo->productClass = malloc(sublen+1)) != NULL) 
					{
						memcpy(pInfo->productClass,optionPtr,sublen);
						pInfo->productClass[sublen] = '\0';
					}
					else
					{
						printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
						goto viError;
					}
				}
				else 
				{
					printf("VDF:%s:%s:%d:***\n",__FILE__,__FUNCTION__,__LINE__);
					DEBUG(LOG_ERR, "saveVIoption(): subcode ProductClass, Class len %d is too long.",sublen);
					goto viError;
				}
				break;
			default:
				printf("VDF:%s:%s:%d:subcode=%d***\n",__FILE__,__FUNCTION__,__LINE__,subcode);
				DEBUG(LOG_ERR, "saveVIoption(): subcode %d, not supported.",subcode);
				goto viError;
		}
		optionPtr += sublen;
	} /* while subcodeParsed < maxSubcode */
	/* add info to the manageable device link list */
	addViToList(pInfo);
	write_viTable();
	//notifyApp();
#ifdef SUPPORT_TR69C
	TR069_SendMessage(CWMP_MSG_ASSOC_DEVICE, NULL, 0);
#endif	  

  return ret;

 viError:
  ret = -1;
  if (pInfo) {
    if (pInfo->oui)
      free(pInfo->oui);
    if (pInfo->serialNumber)
      free(pInfo->serialNumber);
    if (pInfo->productClass)
      free(pInfo->productClass);
    free(pInfo);
  }
  return ret;
}
#endif

#ifdef  SUPPORT_PORTMAPING
    int check_option60(unsigned char *optionstr, unsigned char *vendorstr)
    {
        char r1, r2;
        char *l;
        if (optionstr == NULL || vendorstr == NULL)
        {
            return 0;
        }
        char *end = optionstr + strlen(optionstr)-1;
        l = strstr(optionstr, vendorstr);

        if (!strlen(vendorstr))
    	  return 0;
        if(!strlen(optionstr))
    	  return 0;
       
        for (;;) {
            r1 =0; r2=0;
            if (l != NULL) {
                if (l == optionstr) {
                    r1 = 1;
                }
                else { // need to see | if not the first one
                    if (*(l-1) == '|') {
                        r1 = 1;
                    }
                }
                
                if (*(l + strlen(vendorstr)) == 0 || *(l + strlen(vendorstr)) == '|' ) { //need to see |if not last one
                    r2 = 1;
                }

                if (r1 && r2) {
                    /*if (loc != NULL) {
                        *loc = l;
                    }*/
                    return 1;
                }
                else {
                    if (l < end) {
                        l = strstr(l+1, vendorstr);
                    }
                }
            }
            else {
                return 0;
            }
        }
    }
#endif
