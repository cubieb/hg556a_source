/* 
 * $Id: tftpd.c,v 1.1 2008/08/25 13:28:57 l65130 Exp $
 * A simple tftpd server for busybox
 *
 * Copyright (C) 2001 Steven Carr <Steven_Carr@yahoo.com>
 *
 * Tries to follow RFC1350 and RFC2347.
 * Only "octet" mode supported.
 * tsize option is supported on sending files only (pxelinux support).
 * chroot jail for security.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-137 USA
 */

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
#include <arpa/tftp.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include "busybox.h"

#ifndef OACK
#define OACK 6
#endif
#ifndef EOPTNEG
#define EOPTNEG 8
#endif

#define ENOPUT -2
#define ENOGET -3

#if !defined(__UCLIBC__) || defined(__UCLIBC_HAS_MMU__)
#define FORK() fork()
#else
#define FORK() vfork()
#endif

#define TFTP_BLOCKSIZE_DEFAULT 512	/* according to RFC 1350, don't change */
#define TFTP_TIMEOUT 5		/* seconds */
// brcm begine.  Enable put only
#include "../../ftpd/fwsyscall.h"
#include "bcmTag.h"
extern UPLOAD_RESULT verifyTag(PFILE_TAG pTag, int passNumber);
#define DUP_PKT_CHECK_COUNT      3
//Enable put only
//#define CONFIG_FEATURE_TFTPD_GET
#define CONFIG_FEATURE_TFTPD_PUT
extern char glbIfName[];
void perror_msg_and_die(char *);

void perror_msg_and_die(char * msg)
{
    printf("%s\n", msg);
    exit(-1);
}

// if applications are killed, reset the modem.
int myExit(int fNeedReset, int peer)
{
    close(peer);
    if (fNeedReset)
        sysMipsSoftReset();
    exit(0);
}

// brcm end
/*
 * Handle initial connection protocol.
 *     +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+-->  >-------+---+---~~---+---+
 *     |  opc  |filename| 0 |  mode  | 0 |  opt1  | 0 | value1 | 0 | <  <  optN  | 0 | valueN | 0 |
 *     +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+-->  >-------+---+---~~---+---+
 *             ^--->
 */

/* The options are zero terminated, retrieve a list of pointers to the first character of each option */
int tftpd_options (char *options, int opt_len, char **argv, int max_arg)
{
  int x;
  int y;
  argv[0] = options;
  for (y = 1, x = 0; (y < max_arg) && (x < (opt_len - 1)); x++)
    {
      if (options[x] == 0)
	{
	  if (options[x + 1] == 0)
	    return y;
	  argv[y] = &options[x + 1];
	  y++;
	}
    }
  return y;
}


/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
void tftpd_nak (int peer, int error)
{
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;

  pkt = (struct tftphdr *) buf;
  pkt->th_opcode = htons ((u_short) ERROR);
  pkt->th_code = htons ((u_short) error);

  switch (error)
    {
    case ENOPUT:
      strcpy (pkt->th_msg, "Put not supported");
      pkt->th_code = htons (EUNDEF);
      break;
    case ENOGET:
      strcpy (pkt->th_msg, "Get not supported");
      pkt->th_code = htons (EUNDEF);
      break;
    case EUNDEF:
      strcpy (pkt->th_msg, "Undefined error code");
      break;
    case ENOTFOUND:
      strcpy (pkt->th_msg, "File not found");
      break;
    case EACCESS:
      strcpy (pkt->th_msg, "Access violation");
      break;
    case ENOSPACE:
      strcpy (pkt->th_msg, "Disk full or allocation exceeded");
      break;
    case EBADOP:
      strcpy (pkt->th_msg, "Illegal TFTP operation");
      break;
    case EBADID:
      strcpy (pkt->th_msg, "Unknown transfer ID");
      break;
    case EEXISTS:
      strcpy (pkt->th_msg, "File already exists");
      break;
    case ENOUSER:
      strcpy (pkt->th_msg, "No such user");
      break;
    case EOPTNEG:
      strcpy (pkt->th_msg, "Failure to negotiate RFC2347 options");
      break;
    default:
      strcpy (pkt->th_msg, strerror (error - 100));
      pkt->th_code = htons (EUNDEF);
      break;
    }

  send (peer, buf, strlen (pkt->th_msg) + 5, 0);
}

/*
 * Send a ack packet 
 */
void tftpd_ack (int peer, int block)
{
  struct tftphdr pkt;

  pkt.th_opcode = htons (ACK);
  pkt.th_block = htons (block);

  if (send (peer, &pkt, sizeof(pkt), 0)!=sizeof(pkt))
  	perror_msg_and_die("tftpd_ack send");
}


/*
 * send an oack
 */
void tftpd_oack (int peer, int count, char **list)
{
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;
  int x;
  char *ptr;

  pkt=(struct tftphdr *)buf;
  pkt->th_opcode = htons (OACK);
  ptr=pkt->th_stuff;
  
  for (x=0;x<count;x++)
	 ptr=strrchr (strcpy (ptr, list[x]), '\0') + 1;

 if ( send (peer, buf, (ptr-buf), 0)!=(ptr-buf))
  	perror_msg_and_die("tftpd_oack send");
}


/*
 * send data
 */
void tftpd_data (int peer, int block, char *data, int size)
{
  struct tftphdr *pkt;
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];

  pkt=(struct tftphdr *)buf;
  pkt->th_opcode = htons (DATA);
  pkt->th_block  = htons(block);
  
  memcpy(pkt->th_data,data,size);

  if (send (peer, &buf, size+4, 0)!=(size+4))
  	perror_msg_and_die("tftpd_data send");
}

int tftpd_getdata(int peer, int block, char *data, int size, int fNeedReset)
{
  struct tftphdr *pkt;
  struct timeval tv;
  fd_set rfds;
  int len=-1;
  int timeout_counter = 4;
  
  pkt=(struct tftphdr *)data;
  
  do {	
      tv.tv_sec = TFTP_TIMEOUT;
      tv.tv_usec = 0;
      FD_ZERO (&rfds);
      FD_SET (peer, &rfds);
      switch (select (FD_SETSIZE, &rfds, NULL, NULL, &tv))
	{
	case 1:		/* data ready */
	  len = recv (peer, data, size, 0);
	  if (len < 0)
	    perror_msg_and_die ("failed to read (data)");

	  pkt->th_opcode = ntohs (pkt->th_opcode);
	  pkt->th_block = ntohs (pkt->th_block);
	  if (pkt->th_opcode == ERROR)
	    {
	      bb_error_msg (pkt->th_data);
	      myExit(fNeedReset, peer);
	    }
	  if ((pkt->th_opcode == DATA) && (pkt->th_block != block))
	    {
	      //synchronize (peer);
	    }
	  break;
	case 0:		/* timeout */
	  timeout_counter--;
	  if (timeout_counter == 0)
	    {
	      bb_error_msg ("last timeout");
	      myExit(fNeedReset, peer);
	    }
	  break;
	default:		/* error */
	  perror_msg_and_die ("select failed");
	  break;
	}
   }while (!(pkt->th_opcode == DATA) && (pkt->th_block == block));
   return len;
}



int tftpd_getack(int peer, int block)
{
  char data[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;
  struct timeval tv;
  fd_set rfds;
  int timeout_counter = 4;
  int len;
  
  pkt=(struct tftphdr *)data;

  do {	
      tv.tv_sec = TFTP_TIMEOUT;
      tv.tv_usec = 0;
      FD_ZERO (&rfds);
      FD_SET (peer, &rfds);
      switch (select (FD_SETSIZE, &rfds, NULL, NULL, &tv))
	{
	case 1:		/* data ready */

	  len = recv (peer, data, TFTP_BLOCKSIZE_DEFAULT + 4, 0);
	  if (len < 0)
	    perror_msg_and_die ("failed to read (data)");

	  pkt->th_opcode = ntohs (pkt->th_opcode);
	  pkt->th_block = ntohs (pkt->th_block);

	  if (pkt->th_opcode == ERROR)
	    {
	      bb_error_msg (pkt->th_data);
	      exit (0);
	    }

	  if ((pkt->th_opcode == ACK) && (pkt->th_block != block))
	    {
	      //synchronize (peer);
	    }
	  break;
	case 0:		/* timeout */
	  timeout_counter--;
	  if (timeout_counter == 0)
	    {
	      bb_error_msg ("last timeout");
	      exit (0);
	    }
	  break;
	default:		/* error */
	  perror_msg_and_die ("select failed");
	  break;
	}
   }while (! ((pkt->th_opcode == ACK) && (pkt->th_block == block)) );

  return (1==1);
}





#ifndef CONFIG_FEATURE_TFTPD_GET
void
tftpd_send (int peer, struct tftphdr *tp, int n, int buffersize)
{
	/* we aren't configured for sending files */
	tftpd_nak (peer, ENOGET);
	close (peer);
	exit(0);
}

#else
void
tftpd_send (int peer, struct tftphdr *first_pkt, int pkt_len, int buffersize)
{
	FILE *file=NULL;
	char buffer[TFTP_BLOCKSIZE_DEFAULT+4];
	char *list[64]; /* list of pointers to options and values */
	int listcount;
	char *reply_list[64];
	int reply_listcount=0;
	char tsize_ret[32];
	int block, inbytes, x;
	
	listcount = tftpd_options (first_pkt->th_stuff, pkt_len, list ,64);

	/* get the size of the file (remember, chroot() supposed to point us in the right directory) */

	if (strcasecmp(list[1],"octet")!=0)
	{
		tftpd_nak(peer,EBADOP);
		close(peer);
		exit(0);
	}


	file = fopen (list[0], "r");
	if (file == NULL)
	{
	  tftpd_nak (peer, ENOTFOUND);
	  close(peer);
	  exit(0);
	}
	fseek (file, 0, SEEK_END);
	sprintf(tsize_ret,"%lu", ftell (file));
	fseek (file, 0, SEEK_SET);


	/* 0=filename, 1=mode, 2=option, 3=option_value ... */
	block = 1;
	reply_listcount=0;

	/* look through the options for the ones we support */
	for (x=2;x<listcount;x++)
	{
		if (strcasecmp(list[x],"tsize")==0) /* only one option supported so far */
		{
			reply_list[reply_listcount]=list[x];		
			reply_listcount++;
			reply_list[reply_listcount]=tsize_ret; /* point to the real value */
			reply_listcount++;
		}
	}

	/* if there are any options, send an OACK instead of an ACK */
	if (reply_listcount>0)
	{
		do
		{
			tftpd_oack(peer,reply_listcount,reply_list);
		}
		while (!tftpd_getack(peer,0));
	}	


	/* Send the file! */
	while ((inbytes = fread(buffer,1,TFTP_BLOCKSIZE_DEFAULT,file))>0)
	{
		do
		{
			tftpd_data(peer,block,buffer,inbytes);
		}
		while (!tftpd_getack(peer,block));
		block++;
	}
	fclose(file);
	close(peer);
	exit (0);
}

#endif


#ifndef CONFIG_FEATURE_TFTPD_PUT
void
tftpd_receive (int peer, struct tftphdr *tp, int n, int buffersize)
{
	/* we aren't configured for receiving files */
	tftpd_nak (peer, ENOPUT);
	close (peer);
	exit(0);
}

#else
void
//brcm begin
tftpd_receive (int peer, struct tftphdr *first_pkt, int pkt_len, int buffersize)
{
// brcm	FILE *file=NULL;
	char buffer[TFTP_BLOCKSIZE_DEFAULT+4];
	struct tftphdr *pkt;
	int block, inbytes;
	char *list[64];
	int listcount;

    //brcm begin
    PARSE_RESULT imageType = NO_IMAGE_FORMAT;
    int byteRd = 0;
    char tag[TAG_LEN];
    int tagSize = 0;
    int size = 0;
    char *curPtr = NULL;
    unsigned int totalAllocatedSize = 0;
    int socketPid = 0;
    int fNeedReset = FALSE;
	 char *imagePtr = NULL;
    int uploadSize = 0;
    int i = 0;   

//printf("tftpd_receive, peer = %d, pkt_len = %d, buffersize=%d\n", peer, pkt_len, buffersize);

	pkt=(struct tftphdr *)buffer;
	listcount = tftpd_options (first_pkt->th_stuff, pkt_len, list ,64);

	/* get the size of the file (remember, chroot() supposed to point us in the right directory) */

//printf ("mode= %s, file= %s\n", list[1], list[0]);	
    if (strcasecmp(list[1],"octet")!=0)
	{
        printf("Only support 'bin' mode. Type 'bin' at tftp client\n");
		tftpd_nak(peer,EBADOP);
		close(peer);
		exit(0);
	}

#if 0//brcm
    file = fopen (list[0], "w");
	if (file == NULL)
	{
	  tftpd_nak (peer, EACCESS);
	  close(peer);
	  exit(0);
	}
#endif //brcm

    block=0;

    socketPid = bcmSocketIfPid();   
    // kill the app first so that when there are heavy traffic, the UDP packet will not be lost.
    killAllApps(socketPid, SKIP_HTTPD_APP, 0, 0);     // skip kill httpd for xml config. update
    printf("Done removing processes\n");
    fNeedReset = TRUE;

	do
	{
		tftpd_ack(peer,block);
		block++;

      // if duplicate pkt, (for slow ack on 38R board) discard it.
      for (i = 0; i < DUP_PKT_CHECK_COUNT; i++)
      {
         inbytes=tftpd_getdata(peer,block,buffer,TFTP_BLOCKSIZE_DEFAULT+4, fNeedReset);
         if (block == (int) (*(short*)(buffer+2)))
            break;
      }
		// brcm fwrite(pkt->th_msg,1,inbytes-4,file);
        byteRd=inbytes-4;
        // brcm begin
        if (curPtr == NULL) 
        {
            if (byteRd < TFTP_BLOCKSIZE_DEFAULT)   // not enough data for a valid first packet and exit
            {
                uploadSize = byteRd;
                break;
            }
            // The first data that is downloaded is the FILE_TAG structure.
            // After the entire FILE_TAG structure has been downloaded, use
            // the totalImageSize field to determine the size of pkt->th_msg to
            // allocate.
            if( tagSize + byteRd > TAG_LEN)
                size = TAG_LEN - tagSize;
            else 
                size = byteRd;

            memcpy(tag + tagSize, pkt->th_msg, size);
            tagSize += size;
            byteRd -= size;
            if( tagSize == TAG_LEN ) 
            {
                PFILE_TAG pTag = (PFILE_TAG) tag;

                if (verifyTag(pTag, 1) == UPLOAD_OK) 
                {
                    // if chip id mismatched.
                    if (checkChipId(pTag->chipId, pTag->signiture_2) != 0)
                        myExit(fNeedReset, peer);
                    int totalImageSize = atoi(pTag->totalImageLen);
                    // add tag len plus some pkt->th_msg for over flow during the sending...
                    totalAllocatedSize = totalImageSize + TAG_LEN + 8;
                    printf( "Allocating %d bytes for broadcom image.\n", totalAllocatedSize );
                    curPtr = (char *) malloc(totalImageSize + TAG_LEN + 8);
                    if (curPtr == NULL) 
                    {
                        printf("Not enough memory error.");       
                        myExit(fNeedReset, peer);
                    }
                    else 
                    {
                        printf("Memory allocated\n");
                        imagePtr = curPtr;
                        memcpy(curPtr, tag, TAG_LEN);
                        curPtr += TAG_LEN;
                        uploadSize += TAG_LEN;
                        memcpy(curPtr, pkt->th_msg + size, byteRd);
                        curPtr += byteRd;
                    }
                }
                // try to allocate the memory for .w image, same as flash size + some overhead
                else 
                {   
                    totalAllocatedSize = sysFlashSizeGet() + TOKEN_LEN;
                    printf("Allocating %d bytes for flash image.\n", totalAllocatedSize);
                    curPtr = (char *) malloc(totalAllocatedSize);
                    if(curPtr == NULL) 
                    {
                        printf("Not enough memory error.");       
                        myExit(fNeedReset, peer);
                    }
                    else 
                    {
                        printf("Memory allocated\n");
                        imagePtr = curPtr;
                        byteRd += TAG_LEN;          // add back the tag size
                        memcpy(curPtr, pkt->th_msg, byteRd);
                        curPtr += byteRd;
                    }
                }  // else alloc memory for .w image
            } //tagSize == TAG_LEN
        } // if curPtr == NULL
        else 
        { // not first block of data
            memcpy(curPtr, pkt->th_msg, byteRd);
            curPtr += byteRd;
        }
        
        uploadSize += byteRd;
//printf("Current uploadSize= %d\n", uploadSize);
        if (uploadSize > totalAllocatedSize) // try not to over flow the pkt->th_msg
        {       
            printf("Failed on data corruption during file transfer or over sized image.\n");
			printf("Upload size = %d, size allowed = %d\n", uploadSize, totalAllocatedSize);
            myExit(fNeedReset, peer);
        }
        // brcm end
	}
	while (inbytes==(TFTP_BLOCKSIZE_DEFAULT+4));

    tftpd_ack(peer,block); /* final acknowledge */

// brcm	fclose(file);

    printf("Total size: %d\n", uploadSize);

    if ((imagePtr != NULL) && 
        ((imageType = parseImageData(imagePtr, uploadSize, BUF_ALL_TYPES)) != NO_IMAGE_FORMAT)) 
    {
        printf("Tftp image done.\n");
        flashImage(imagePtr, imageType, uploadSize);
        if (imagePtr)
	        free(imagePtr);
    }
    else 
        printf("Tftp Image failed: Illegal image.\n");

    // no reset if psi update (need httpd to process xml file)
    if (imageType == PSI_IMAGE_FORMAT)
        fNeedReset=FALSE;
    myExit(fNeedReset, peer);
}
// brcm end
#endif

static struct in_addr getLanIp(void) //struct in_addr *lan_ip)
{
   int socketfd;
   struct ifreq lan;

   memset(&lan, 0, sizeof(lan));
   if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error openning socket when getting LAN info\n");
   }
   else  {
        strcpy(lan.ifr_name, "br0");
        if (ioctl(socketfd,SIOCGIFADDR,&lan) < 0) {
            printf("Error getting LAN IP address\n");
        }
   }
   close(socketfd);
   return ((struct sockaddr_in *)&(lan.ifr_addr))->sin_addr;
}

// brcm -- from igmp
#include <bits/socket.h>
#include <sys/uio.h>
#define MAXCTRLSIZE						\
	(sizeof(struct cmsghdr) + sizeof(struct sockaddr_in) +	\
	sizeof(struct cmsghdr) + sizeof(int) + 32)
// brmc end

static int
tftpd_daemon (char *directory, char *address, int port)
{
  struct tftphdr *tp;
  struct sockaddr_in from;
  struct sockaddr_in myaddr;
  struct sockaddr_in bindaddr;
  int fd = -1;
  int peer;
  int rv;
  int n;
  pid_t pid;
  int i = 1;

  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct iovec iov = { buf, sizeof buf };
  struct cmsghdr *cmsg;
  char *ctrl = (char *)xmalloc(MAXCTRLSIZE);
  struct msghdr msg = { (void*)&from, sizeof from, &iov, 1, (void*)ctrl, MAXCTRLSIZE, 0};
  struct in_pktinfo *info = NULL;

  daemon(0,1);

  if ((fd = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
    perror_msg_and_die ("socket");
  memset (&bindaddr, 0, sizeof (bindaddr));
  bindaddr.sin_family = AF_INET;
  bindaddr.sin_addr.s_addr = INADDR_ANY;
  bindaddr.sin_port = htons (port);
  if (address != NULL)
    {
      struct hostent *hostent;
      hostent = xgethostbyname (address);
      if (!hostent || hostent->h_addrtype != AF_INET)
	perror_msg_and_die ("cannot resolve local bind address");
      memcpy (&bindaddr.sin_addr, hostent->h_addr, hostent->h_length);
    }
  // set option for getting the to ip address.
  setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &i, sizeof(i));

  if (bind (fd, (struct sockaddr *) &bindaddr, sizeof (bindaddr)) < 0)
    perror_msg_and_die ("daemon bind failed");
  /* This means we don't want to wait() for children */
  signal (SIGCHLD, SIG_IGN);

  // get pid for web access function to start/stop tftpd.
  {
    FILE *pid_fp;
    if (!(pid_fp = fopen("/var/run/tftpd_pid", "w"))) 
    {
        printf("Error open /var/run/tftpd_pid");
        exit(0);
    }
    fprintf(pid_fp, "%d\n", getpid());
    fclose(pid_fp);
  }

  while (1)
    {
      fd_set readset;
      memset(buf,0,TFTP_BLOCKSIZE_DEFAULT + 4);
      memset (&myaddr, 0, sizeof (myaddr));
      FD_ZERO (&readset);
      FD_SET (fd, &readset);
      /* Never time out, we're in standalone mode */
      rv = select (fd + 1, &readset, NULL, NULL, NULL);
      if (rv == -1 && errno == EINTR)
	continue;		/* Signal caught, reloop */
      if (rv == -1)
	perror_msg_and_die ("select loop");
      if (rv == 0)
	{
	  bb_error_msg ("We shouldn't be timeing out!");
	  exit (0);		/* Timeout, return to inetd */
	}

      n = recvmsg (fd, &msg, MSG_WAITALL);
      if (n <= 0)
      {
          if (n == 0)  /* Keven -- Received message with zero length, reloop */
            continue;
          else {      	
            printf("*** error recvmsg n=%d\n", n);
            break;
          }
      }
//printf("incoming_ip=%s, n=%d\n", inet_ntoa(from.sin_addr), n);
	for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL;cmsg =CMSG_NXTHDR(&msg,cmsg)) {
		if (cmsg->cmsg_type == IP_PKTINFO){
		    info = (struct in_pktinfo *)CMSG_DATA(cmsg);
//            printf("sepc_dst=%s, ipi_addr=%s\n", inet_ntoa(info->ipi_spec_dst),inet_ntoa(info->ipi_addr));
            break;
		}
    }
  /* Process the request */
  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons (0);	/* we want a new local port */
  myaddr.sin_addr = getLanIp();

  if (myaddr.sin_addr.s_addr != info->ipi_spec_dst.s_addr)
    memcpy (&myaddr.sin_addr, &bindaddr.sin_addr,sizeof bindaddr.sin_addr);

      /* Now that we have read the request packet from the UDP
         socket, we fork and go back to listening to the socket. */
    pid = FORK ();

    if (pid < 0)
	    perror_msg_and_die ("cannot fork");
    if (pid == 0)
	    break;			/* Child exits the while(1), parent continues to loop */
    }

  /* Close file descriptors we don't need */
  // brcm close (fd);

  /* Get a socket.  This has to be done before the chroot() (/dev goes away) */
  peer = socket (AF_INET, SOCK_DGRAM, 0);
  if (peer < 0)
    perror_msg_and_die ("socket");
  if (chroot ("."))
    perror_msg_and_die ("chroot");
  from.sin_family = AF_INET;

  /* Process the request */
  if (bind (peer, (struct sockaddr *) &myaddr, sizeof myaddr) < 0)
    perror_msg_and_die ("daemon-child bind");

//printf("after bind. my_ip=%s*****\n", inet_ntoa(myaddr.sin_addr));

  if (connect (peer, (struct sockaddr *) &from, sizeof from) < 0)
    perror_msg_and_die ("daemon-child connect");
  tp = (struct tftphdr *) buf;

//printf("after connect \n");

  int accessMode = bcmCheckEnable("tftp", from.sin_addr);
//printf("accessmode = %d\n", accessMode);
  if (accessMode == CLI_ACCESS_DISABLED) 
  {
    close(peer);
    exit(0);
  }
  if (accessMode == CLI_ACCESS_REMOTE) 
  {
    glbIfName[0] = '\0';
    if ((bcmGetIntfNameSocket(peer, glbIfName) != 0) || glbIfName[0] == '\0') 
    {
      printf("Failed to get remote ifc name!\n");
      close(peer);
      exit(0);
    }
  }
//printf("socket ifname = %s\n", glbIfName);
  
  tp->th_opcode = ntohs (tp->th_opcode);

  switch(tp->th_opcode)
  {
  	case RRQ:
	    tftpd_send (peer, tp, n, TFTP_BLOCKSIZE_DEFAULT);
	    break;
	case WRQ:
	    tftpd_receive (peer, tp, n, TFTP_BLOCKSIZE_DEFAULT);
	    break;
  }
  exit (0);
}



int
tftpd_main (int argc, char **argv)
{
  int result;
  char *address = NULL;		/* address to listen to */
  int port = 69;
  int daemonize = 1;            // brcm (1 == 0);

#if 0 //brcm  
  int on = 1;
  int fd = 0;
  int opt;  
  char directory[256];		/* default directory "/tftpboot/" */
  memset (directory, 0, sizeof (directory));
  strcpy (directory, "/tftpboot/");

  while ((opt = getopt (argc, argv, "sp:a:d:h")) != -1)
    {
      switch (opt)
	{
	case 'p':
	  port = atoi (optarg);
	  break;
	case 'a':
	  address = optarg;
	  break;
	case 's':
	  daemonize = (1 == 1);
	  break;
	case 'd':
	  safe_strncpy (directory, optarg, sizeof (directory));
	  break;
	case 'h':
	  show_usage ();
	  break;
	}
    }
  if (chdir (directory))
    perror_msg_and_die ("Invalid Directory");

  if (ioctl (fd, FIONBIO, &on) < 0)
    perror_msg_and_die ("ioctl(FIONBIO)");

  /* daemonize this process */
  if (daemonize)
    {
      pid_t f = FORK ();
      if (f > 0) {
// brcm save the pid in the file for kill.
        FILE *pid_fp;
        if (!(pid_fp = fopen("/var/run/tftpd_pid", "w"))) 
        {
            printf("Error open /var/run/tftpd_pid");
            exit(0);
        }
        fprintf(pid_fp, "%d\n", f);
        fclose(pid_fp);
    	exit (0);
      }
      if (f < 0)
    	perror_msg_and_die ("cannot fork");
      close (0);
      close (1);
      close (2);
    }
#endif //brcm

  result = tftpd_daemon ("", address, port);
  return (result);
}

