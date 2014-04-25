/* ------------------------------------------------------------------------- */
/* tftp.c                                                                    */
/*                                                                           */
/* A simple tftp client for busybox.                                         */
/* Tries to follow RFC1350.                                                  */
/* Only "octet" mode supported.                                              */
/* Optional blocksize negotiation (RFC2347 + RFC2348)                        */
/*                                                                           */
/* Copyright (C) 2001 Magnus Damm <damm@opensource.se>                       */
/*                                                                           */
/* Parts of the code based on:                                               */
/*                                                                           */
/* atftp:  Copyright (C) 2000 Jean-Pierre Lefebvre <helix@step.polymtl.ca>   */
/*                        and Remi Lefebvre <remi@debian.org>                */
/*                                                                           */
/* utftp:  Copyright (C) 1999 Uwe Ohse <uwe@ohse.de>                         */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          */
/* General Public License for more details.                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "busybox.h"

//#define CONFIG_FEATURE_TFTP_DEBUG

#define TFTP_BLOCKSIZE_DEFAULT 512 /* according to RFC 1350, don't change */
#define TFTP_TIMEOUT 5             /* seconds */

/* opcodes we support */

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5
#define TFTP_OACK  6

static const char *tftp_bb_error_msg[] = {
	"Undefined error",
	"File not found",
	"Access violation",
	"Disk full or allocation error",
	"Illegal TFTP operation",
	"Unknown transfer ID",
	"File already exists",
	"No such user"
};

const int tftp_cmd_get = 1;
const int tftp_cmd_put = 2;

// brcm begine.
#include "../../ftpd/fwsyscall.h"
#include "bcmTag.h"

// use soem globals to reduce the messiness of change the original tftp function.
int glbNeedReset = FALSE;
char *glbImagePtr = NULL;
int glbUploadSize = 0;
char *glbCurPtr = NULL;
unsigned int glbTotalAllocatedSize = 0;
char glbUploadType = 'i';
extern char glbIfName[];
char *gXmlBuf = NULL;

extern UPLOAD_RESULT verifyTag(PFILE_TAG pTag, int passNumber);

int myRead(char *outBuf, int inLen)
{
   int readLen = 0;
   static int xmlCfgLen = 0;
   static int offset = 0;
   
   if (!gXmlBuf) 
   {
      int psiSize = 0;
      char *pc = NULL;
      psiSize = sysGetPsiSize();
      if ((gXmlBuf = xmalloc(psiSize)) == NULL) 
      {
         printf("Failed to allocate %d bytes of memory.\n", psiSize);
         return -1;
      }
      sysPersistentGet(gXmlBuf, psiSize, 0);
      // if <psitree> cannot be found then return
      if (memcmp(gXmlBuf, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) == 0) 
      {
         // determine data length
         pc = strstr(gXmlBuf, XML_PSI_END_TAG);   
         if (pc != NULL)
            xmlCfgLen = pc - gXmlBuf + strlen(XML_PSI_END_TAG);
      }
      else
      {
         printf("Invalid configuration data.\n");
         return -1;
      }
   }

   if (xmlCfgLen <= inLen)
      readLen = xmlCfgLen;
   else
      readLen = inLen;

   memcpy(outBuf, (gXmlBuf + offset), readLen);

   xmlCfgLen -= readLen;
   offset += readLen;
   glbUploadSize += readLen;

   if (xmlCfgLen == 0)
   {
      free(gXmlBuf);
      offset = 0;
      gXmlBuf = NULL;
   }

   return readLen;
}


int myWrite(char *inBuf, int inBufLen)
{
   if (glbCurPtr == NULL) 
   {
	  FILE *fs;
      int socketPid = 0;
	  int telnetIfcPid = 0;
	  int sshdIfcPid = 0;
      int skipApp = SKIP_HTTPD_APP;       // save httpd for xml config. file process

      if (inBufLen < TFTP_BLOCKSIZE_DEFAULT)   // not enough data for a valid first packet and exit
         return -1;   
      // socketPid is the ifc tftp socket is on -- can't be killed
      socketPid = bcmSocketIfPid();  

      // if using telnet, should not kill telnet.
	  if ((fs=fopen("/var/run/telnetd_ifc","r")) != NULL) {
	     char tmpIfc[32];
         fscanf(fs, "%s", tmpIfc);
         fclose(fs);
         skipApp |= SKIP_TELNETD_APP;
		 if (strncmp(tmpIfc, glbIfName, IFC_NAME_LEN) != 0) {  // different ifc
			strncpy(glbIfName, tmpIfc, IFC_NAME_LEN);
			telnetIfcPid = bcmSocketIfPid(); 	 
		 }
	  }
      
      // if using sshd, should not kill sshd.
	  if ((fs=fopen("/var/run/sshd_ifc","r")) != NULL) {
	     char tmpIfc[32];
         fscanf(fs, "%s", tmpIfc);
         fclose(fs);
         skipApp |= SKIP_SSHD_APP;
		 if (strncmp(tmpIfc, glbIfName, IFC_NAME_LEN) != 0) {  // different ifc
			strncpy(glbIfName, tmpIfc, IFC_NAME_LEN);
			sshdIfcPid = bcmSocketIfPid(); 	 
		 }
	  }
  
	  killAllApps(socketPid, skipApp, telnetIfcPid, sshdIfcPid);
      printf("Done removing processes\n");
      glbNeedReset = TRUE;
      // The first data that is downloaded is the FILE_TAG structure.
      // After the entire FILE_TAG structure has been downloaded, use
      // the totalImageSize field to determine the size of inBuf to
      // allocate.
      PFILE_TAG pTag = (PFILE_TAG) inBuf;

      if (verifyTag(pTag, 1) == UPLOAD_OK) 
      {

         // if chip id mismatched.
         if (checkChipId(pTag->chipId, pTag->signiture_2) != 0)
            return -1;
         // add tag len plus some inBuf for over flow during the sending...
         glbTotalAllocatedSize =  atoi(pTag->totalImageLen) + TAG_LEN + 8;
         printf( "Allocating %d bytes for broadcom image.\n", glbTotalAllocatedSize );
      }
      // try to allocate the memory for .w image or psi, same as flash size + some overhead
      else 
      {   
         glbTotalAllocatedSize = sysFlashSizeGet() + TOKEN_LEN;
         printf("Allocating %d bytes for flash image.\n", glbTotalAllocatedSize);
      }
      glbCurPtr = (char *) malloc(glbTotalAllocatedSize);
      if (glbCurPtr == NULL) 
      {
         printf("Not enough memory error.");   
         return -1;
      }
      printf("Memory allocated\n");
      glbImagePtr = glbCurPtr;
   } // if glbCurPtr == NULL

   memcpy(glbCurPtr, inBuf, inBufLen);
   glbCurPtr += inBufLen;
   glbUploadSize += inBufLen;
   if (glbUploadSize > glbTotalAllocatedSize) // try not to over flow the inBuf
   {       
       printf("Failed on corrupted data or over sized image size.\n");
       return -1;
   }

   return inBufLen;
}


//-- from igmp
#include <bits/socket.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#define MAXCTRLSIZE						\
	(sizeof(struct cmsghdr) + sizeof(struct sockaddr_in) +	\
	sizeof(struct cmsghdr) + sizeof(int) + 32)

char *myif_indextoname(int sockfd,unsigned int ifindex,char *ifname)
{
	struct ifreq ifr;
	int status;

	memset(&ifr,0,sizeof(struct ifreq));
 	ifr.ifr_ifindex = ifindex;
	
	status = ioctl(sockfd,SIOCGIFNAME,&ifr);
	
	if (status < 0) {
		//printf("ifindex %d has no device \n",ifindex);
		return NULL;
	}
	else
		return strncpy(ifname,ifr.ifr_name,IFNAMSIZ);
}
// brmc end

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE

static int tftp_blocksize_check(int blocksize, int bufsize)
{
        /* Check if the blocksize is valid:
	 * RFC2348 says between 8 and 65464,
	 * but our implementation makes it impossible
	 * to use blocksizes smaller than 22 octets.
	 */

        if ((bufsize && (blocksize > bufsize)) ||
	    (blocksize < 8) || (blocksize > 65464)) {
	        bb_error_msg("bad blocksize");
	        return 0;
	}

	return blocksize;
}

static char *tftp_option_get(char *buf, int len, char *option)
{
        int opt_val = 0;
	int opt_found = 0;
	int k;

	while (len > 0) {

	        /* Make sure the options are terminated correctly */

	        for (k = 0; k < len; k++) {
		        if (buf[k] == '\0') {
			        break;
			}
		}

		if (k >= len) {
		        break;
		}

		if (opt_val == 0) {
			if (strcasecmp(buf, option) == 0) {
			        opt_found = 1;
			}
		}
		else {
		        if (opt_found) {
				return buf;
			}
		}

		k++;

		buf += k;
		len -= k;

		opt_val ^= 1;
	}

	return NULL;
}

#endif

static inline int tftp(const int cmd, const struct hostent *host,
	const char *remotefile, int localfd, const unsigned short port, int tftp_bufsize)
{
	const int cmd_get = cmd & tftp_cmd_get;
	const int cmd_put = cmd & tftp_cmd_put;
	const int bb_tftp_num_retries = 5;

	struct sockaddr_in sa;
	struct sockaddr_in from;
	struct timeval tv;
	socklen_t fromlen;
	fd_set rfds;
	char *cp;
	unsigned short tmp;
	int socketfd;
	int len;
	int opcode = 0;
	int finished = 0;
	int timeout = bb_tftp_num_retries;
	unsigned short block_nr = 1;
// brcm begine
	struct sockaddr_in saTmp;
	int i = 1;
// brcm end

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
	int want_option_ack = 0;
#endif

	/* Can't use RESERVE_CONFIG_BUFFER here since the allocation
	 * size varies meaning BUFFERS_GO_ON_STACK would fail */
	char *buf=xmalloc(tftp_bufsize + 4);

	tftp_bufsize += 4;

	if ((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		bb_perror_msg("socket");
		return EXIT_FAILURE;
	}

// brcm begine
    len = sizeof(saTmp);
	memset(&saTmp, 0, len);
    saTmp.sin_family = AF_INET;
    saTmp.sin_addr.s_addr = INADDR_ANY;
   
    // set option for getting the to ip address.
    setsockopt(socketfd, IPPROTO_IP, IP_PKTINFO, &i, sizeof(i));

    if (bind(socketfd, (struct sockaddr *)&saTmp, len) == -1)
    {
        printf("Cannot bind socket.\n\n");
        return EXIT_FAILURE;
    }
    memset(&sa, 0, len);
    sa.sin_family = host->h_addrtype;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr, (struct in_addr *) host->h_addr, host->h_length);
// brcm end

	/* build opcode */

	if (cmd_get) {
		opcode = TFTP_RRQ;
	}

	if (cmd_put) {
		opcode = TFTP_WRQ;
	}

	while (1) {

		cp = buf;

		/* first create the opcode part */

		*((unsigned short *) cp) = htons(opcode);

		cp += 2;

		/* add filename and mode */

		if ((cmd_get && (opcode == TFTP_RRQ)) ||
			(cmd_put && (opcode == TFTP_WRQ))) {
                        int too_long = 0;

			/* see if the filename fits into buf */
			/* and fill in packet                */

			len = strlen(remotefile) + 1;

			if ((cp + len) >= &buf[tftp_bufsize - 1]) {
			        too_long = 1;
			}
			else {
			        safe_strncpy(cp, remotefile, len);
				cp += len;
			}

			if (too_long || ((&buf[tftp_bufsize - 1] - cp) < 6)) {
				bb_error_msg("too long remote-filename");
				break;
			}

			/* add "mode" part of the package */

			memcpy(cp, "octet", 6);
			cp += 6;

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE

			len = tftp_bufsize - 4; /* data block size */

			if (len != TFTP_BLOCKSIZE_DEFAULT) {

			        if ((&buf[tftp_bufsize - 1] - cp) < 15) {
				        bb_error_msg("too long remote-filename");
					break;
				}

				/* add "blksize" + number of blocks  */

				memcpy(cp, "blksize", 8);
				cp += 8;

				cp += snprintf(cp, 6, "%d", len) + 1;

				want_option_ack = 1;
			}
#endif
		}

		/* add ack and data */

		if ((cmd_get && (opcode == TFTP_ACK)) ||
			(cmd_put && (opcode == TFTP_DATA))) {

			*((unsigned short *) cp) = htons(block_nr);

			cp += 2;

			block_nr++;

			if (cmd_put && (opcode == TFTP_DATA)) {
// brcm				len = bb_full_read(localfd, cp, tftp_bufsize - 4);
				len = myRead(cp, tftp_bufsize - 4);

				if (len < 0) {
					bb_perror_msg("read");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
				}

				cp += len;
			} 
		}


		/* send packet */


		timeout = bb_tftp_num_retries;  /* re-initialize */
		do {

			len = cp - buf;

#ifdef CONFIG_FEATURE_TFTP_DEBUG
			fprintf(stderr, "sending %u bytes\n", len);
			for (cp = buf; cp < &buf[len]; cp++)
				fprintf(stderr, "%02x ", (unsigned char)*cp);
			fprintf(stderr, "\n");
#endif
			if (sendto(socketfd, buf, len, 0,
					(struct sockaddr *) &sa, sizeof(sa)) < 0) {
				bb_perror_msg("send");
				len = -1;
				break;
			}


			if (finished && (opcode == TFTP_ACK)) {
				break;
			}

			/* receive packet */

			memset(&from, 0, sizeof(from));
			fromlen = sizeof(from);

			tv.tv_sec = TFTP_TIMEOUT;
			tv.tv_usec = 0;

			FD_ZERO(&rfds);
			FD_SET(socketfd, &rfds);

			switch (select(FD_SETSIZE, &rfds, NULL, NULL, &tv)) {
			case 1: {
// brcm begin
				static int firstTime = 1;
				// first time, use recvmsg to get ifc name
				if (firstTime) {
					struct iovec iov = {buf, tftp_bufsize + 4};
					struct cmsghdr *cmsg;
					char *ctrl = (char *)xmalloc(MAXCTRLSIZE);
					struct msghdr msg = {(void*)&from, sizeof from, &iov, 1, (void*)ctrl, MAXCTRLSIZE, 0};
					struct in_pktinfo *info = NULL;
					char *p = glbIfName;

					len = recvmsg (socketfd, &msg, MSG_WAITALL);
					if (len <= 0) {
						printf("*** error recvmsg len=%d\n", len);
						break;
					}
					for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL;cmsg =CMSG_NXTHDR(&msg,cmsg)) {
						if (cmsg->cmsg_type == IP_PKTINFO){
							info = (struct in_pktinfo *)CMSG_DATA(cmsg);
							i = info->ipi_ifindex;
							//printf("indx=%d, sepc_dst=%s, ipi_addr=%s\n", 
							//i, inet_ntoa(info->ipi_spec_dst),inet_ntoa(info->ipi_addr));
						}
						else {
							printf("BAD CMSG_HDR\n");
							break;
						}
					}
					free(ctrl);
 					p = myif_indextoname(socketfd, i, glbIfName);
					firstTime = 0;
				}
				else
					len = recvfrom(socketfd, buf, tftp_bufsize, 0, (struct sockaddr *) &from, &fromlen);

				if (len < 0) {
					bb_error_msg("recvmsg");
					break;
				}

				timeout = 0;
				//printf("sa.sin_port =%d, from.sin_port = %d\n", sa.sin_port, from.sin_port);
// brcm end
				if (sa.sin_port ==  htons(port)) {      // brcm modified.
					sa.sin_port = from.sin_port;
				}
				if (sa.sin_port == from.sin_port) {
					break;
				}

				/* fall-through for bad packets! */
				/* discard the packet - treat as timeout */
				timeout = bb_tftp_num_retries;
            }
			case 0:
				bb_error_msg("timeout");

				timeout--;
				if (timeout == 0) {
					len = -1;
					bb_error_msg("last timeout");
				}
				break;

			default:
				bb_perror_msg("select");
				len = -1;
			}

		} while (timeout && (len >= 0));

		if ((finished) || (len < 0)) {
			break;
		}

		/* process received packet */


		opcode = ntohs(*((unsigned short *) buf));
		tmp = ntohs(*((unsigned short *) &buf[2]));

#ifdef CONFIG_FEATURE_TFTP_DEBUG
		fprintf(stderr, "received %d bytes: %04x %04x\n", len, opcode, tmp);
#endif

		if (opcode == TFTP_ERROR) {
			char *msg = NULL;

			if (buf[4] != '\0') {
				msg = &buf[4];
				buf[tftp_bufsize - 1] = '\0';
			} else if (tmp < (sizeof(tftp_bb_error_msg)
					  / sizeof(char *))) {

				msg = (char *) tftp_bb_error_msg[tmp];
			}

			if (msg) {
				bb_error_msg("server says: %s", msg);
			}

			break;
		}

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
		if (want_option_ack) {

			 want_option_ack = 0;

		         if (opcode == TFTP_OACK) {

			         /* server seems to support options */

			         char *res;

				 res = tftp_option_get(&buf[2], len-2,
						       "blksize");

				 if (res) {
				         int blksize = atoi(res);
			
					 if (tftp_blocksize_check(blksize,
							   tftp_bufsize - 4)) {

					         if (cmd_put) {
				                         opcode = TFTP_DATA;
						 }
						 else {
				                         opcode = TFTP_ACK;
						 }
#ifdef CONFIG_FEATURE_TFTP_DEBUG
						 fprintf(stderr, "using blksize %u\n", blksize);
#endif
					         tftp_bufsize = blksize + 4;
						 block_nr = 0;
						 continue;
					 }
				 }
				 /* FIXME:
				  * we should send ERROR 8 */
				 bb_error_msg("bad server option");
				 break;
			 }

			 bb_error_msg("warning: blksize not supported by server"
				   " - reverting to 512");

			 tftp_bufsize = TFTP_BLOCKSIZE_DEFAULT + 4;
		}
#endif

		if (cmd_get && (opcode == TFTP_DATA)) {

			if (tmp == block_nr) {
			
//	brcm			len = bb_full_ write(localfd, &buf[4], len - 4);
				len = myWrite(&buf[4], len-4);

				if (len < 0) {
					bb_perror_msg("write");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
				}

				opcode = TFTP_ACK;
				continue;
			}
		}

		if (cmd_put && (opcode == TFTP_ACK)) {

			if (tmp == (unsigned short)(block_nr - 1)) {
				if (finished) {
					break;
				}

				opcode = TFTP_DATA;
				continue;
			}
		}
	}

#ifdef CONFIG_FEATURE_CLEAN_UP
	close(socketfd);

        free(buf);
#endif

	return finished ? EXIT_SUCCESS : EXIT_FAILURE;
}

int tftp_main(int argc, char **argv)
{
	struct hostent *host = NULL;
	// brcm begin
#ifdef CONFIG_FEATURE_TFTP_DEBUG
	const char *localfile = NULL;
#endif
	const char *remotefile = NULL;
	int fBufType = BUF_IMAGES;
	PARSE_RESULT imageType = NO_IMAGE_FORMAT;

	// brcm end
	int port;
	int cmd = 0;
	int fd = -1;
	int flags = 0;
	int opt;
	int result;
	int blocksize = TFTP_BLOCKSIZE_DEFAULT;

	/* figure out what to pass to getopt */

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
#define BS "b:"
#else
#define BS
#endif

#ifdef CONFIG_FEATURE_TFTP_GET
#define GET "g"
#else
#define GET
#endif

#ifdef CONFIG_FEATURE_TFTP_PUT
#define PUT "p"
#else
#define PUT
#endif

// brcm modify next line
    while ((opt = getopt(argc, argv, BS GET PUT "f:t:")) != -1) {
		switch (opt) {
#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
		case 'b':
			blocksize = atoi(optarg);
			if (!tftp_blocksize_check(blocksize, 0)) {
                                return EXIT_FAILURE;
			}
			break;
#endif
#ifdef CONFIG_FEATURE_TFTP_GET
		case 'g':
			cmd = tftp_cmd_get;
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;
#endif
#ifdef CONFIG_FEATURE_TFTP_PUT
		case 'p':
			cmd = tftp_cmd_put;
			flags = O_RDONLY;
			break;
#endif

// brcm begin
#if 0  
		case 'l':
			localfile = optarg;
			break;
		case 'r':
			remotefile = optarg;
			break;
#endif
		case 'f':
			remotefile = optarg;
			break;
		case 't':
			glbUploadType = optarg[0];
			break;
		}
	}
	if ((cmd == 0) || (optind == argc) || !(glbUploadType == 'i' || glbUploadType == 'c')) {
		bb_show_usage();
	}
	if (glbUploadType == 'c')
		fBufType = BUF_CONFIG_DATA;

#if 0 // brcm
	if(localfile && strcmp(localfile, "-") == 0) {
	    fd = fileno((cmd==tftp_cmd_get)? stdout : stdin);
	}
	if(localfile == NULL)
	    localfile = remotefile;
	if(remotefile == NULL)
	    remotefile = localfile;
	if (fd==-1) {
	    fd = open(localfile, flags, 0644);
	}
	if (fd < 0) {
		bb_perror_msg_and_die("local file");
	}
#endif //brcm

	host = xgethostbyname(argv[optind]);
	port = 69;      // brcm port = bb_lookup_port(argv[optind + 1], "udp", 69);

#ifdef CONFIG_FEATURE_TFTP_DEBUG
	fprintf(stderr, "using server \"%s\", remotefile \"%s\", "
		"localfile \"%s\".\n",
		inet_ntoa(*((struct in_addr *) host->h_addr)),
		remotefile, localfile);
#endif

	result = tftp(cmd, host, remotefile, fd, port, blocksize);
// brcm begin
   printf("Total image size: %d\n", glbUploadSize);
   //printf("****result = %d\n", result);
  
   if (result == EXIT_SUCCESS )
   {
      if  (cmd == tftp_cmd_get)
      {
         if ((glbImagePtr != NULL) && 
            ((imageType = parseImageData(glbImagePtr, glbUploadSize, fBufType)) != NO_IMAGE_FORMAT)) 
         {
            printf("Tftp image done.\n");
            flashImage(glbImagePtr, imageType, glbUploadSize);
            if (glbImagePtr)
	            free(glbImagePtr);
         }
         else 
            printf("Tftp Image failed: Illegal image.\n");
      }
      else
         printf("Tftp image done.\n");
   }
   else
      printf("Tftp Image failed: tftp server OR file name not found.\n");
      
   if (glbNeedReset && imageType != PSI_IMAGE_FORMAT)
       sysMipsSoftReset();
// brcm end

#ifdef CONFIG_FEATURE_CLEAN_UP
	if (!(fd == STDOUT_FILENO || fd == STDIN_FILENO)) {
	    close(fd);
	}
#endif
	return(result);
}
