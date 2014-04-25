/* vi: set sw=4 ts=4: */
/*
 * ftpget
 *
 * Mini implementation of FTP to retrieve a remote file.
 *
 * Copyright (C) 2002 Jeff Angielski, The PTR Group <jeff@theptrgroup.com>
 * Copyright (C) 2002 Glenn McGrath <bug1@iinet.net.au>
 *
 * Based on wget.c by Chip Rosenthal Covad Communications
 * <chip@laserlink.net>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "busybox.h"

typedef struct ftp_host_info_s {
	char *user;
	char *password;
	struct sockaddr_in *s_in;
} ftp_host_info_t;

static char verbose_flag = 0;
static char do_continue = 0;

static char *m_pcLocalIP = NULL;
static int ftpcmd(const char *s1, const char *s2, int stream, char *buf)
{
    char acSendBuf[512];
    int len;
    int lBytesSent;

    len = 0;
    if (NULL != s1)
    {
        if (NULL != s2)
        {
            len = snprintf(acSendBuf, sizeof(acSendBuf), "%s%s\r\n", s1, s2);
        }
        else
        {
            len = snprintf(acSendBuf, sizeof(acSendBuf), "%s\r\n", s1);
        }
    }

	if (verbose_flag) {
	}

    lBytesSent = 0;
	while (len > 0)
    {
        lBytesSent = send(stream, (acSendBuf + lBytesSent), len, 0);
        if (lBytesSent < 0)
        {
            bb_perror_msg("ftpcmd send failed");
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            return 0;
        }
        else if (lBytesSent == 0)
        {
            bb_perror_msg("ftpcmd send failed due to server closed");
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            return 0;
        }

        len -= lBytesSent;
    }

    char *buf_ptr;
    char *next_line;
    char *last_line;
	do {

        lBytesSent = recv(stream, buf, 500, 0);
		if (lBytesSent < 0) {
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            break;
		}
		else if (lBytesSent == 0)
		{
            bb_perror_msg("ftpcmd recv failed due to server closed");
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            break;
		}

        buf[lBytesSent] = '\0';
        last_line = buf;
        while (1)
        {
            buf_ptr = strstr(last_line, "\r\n");
    		if (NULL == buf_ptr)
    		{
    		    break;
    		}
            *buf_ptr = '\0';
            buf_ptr += 2;
            next_line = strstr(buf_ptr, "\r\n");
            if (NULL == next_line)
            {
                break;
            }
            last_line = buf_ptr;
        }
	} while (! isdigit(last_line[0]) || last_line[3] != ' ');

	return atoi(buf);
}

static int xconnect_ftpdata(ftp_host_info_t *server, const char *buf)
{
	char *buf_ptr;
	unsigned short port_num;
#if 0
    struct sockaddr_in stBindAddr;
    int retFd;
#endif

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = atoi(buf_ptr + 1);

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += atoi(buf_ptr + 1) * 256;

	server->s_in->sin_port=htons(port_num);
    bb_default_error_retval = ATP_TRANS_TIMEOUT;

    return xbind_connect((struct sockaddr_in *)(server->s_in), m_pcLocalIP);

#if 0
    retFd = socket(AF_INET, SOCK_STREAM, 0);
    if (NULL != m_pcLocalIP)
    {
        memset((void *)(&stBindAddr), 0, sizeof(stBindAddr));
        stBindAddr.sin_family = AF_INET;
        stBindAddr.sin_addr.s_addr = inet_addr(m_pcLocalIP);
        //stBindAddr.sin_port = htons(9092);
        bind(retFd, (struct sockaddr *)(&stBindAddr), sizeof(stBindAddr));
    }

    if (connect(retFd, (struct sockaddr_in *)(server->s_in), sizeof(struct sockaddr_in)) < 0)
	{
		bb_perror_msg_and_die("Unable to connect to remote host (%s)",
				inet_ntoa(server->s_in->sin_addr));
	}
	return retFd;
	//return(xconnect(server->s_in));
#endif
}

static int ftp_login(ftp_host_info_t *server)
{
	int control_stream;
	char buf[512];
    struct sockaddr_in stBindAddr;
    struct timeval tv;
    
	/* Connect to the command socket */
    bb_default_error_retval = ATP_TRANS_TIMEOUT;
    control_stream = socket(AF_INET, SOCK_STREAM, 0);
     /*start of 增加超时处理函数 by s53329  at  20080107*/
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(control_stream,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    setsockopt(control_stream,SOL_SOCKET,SO_SNDTIMEO ,&tv,sizeof(tv));
    /*end  of 增加超时处理函数 by s53329  at  20080107*/

    if (NULL != m_pcLocalIP)
    {
        memset((void *)(&stBindAddr), 0, sizeof(stBindAddr));
        stBindAddr.sin_family = AF_INET;
        stBindAddr.sin_addr.s_addr = inet_addr(m_pcLocalIP);
        //stBindAddr.sin_port = htons(9091);
        bind(control_stream, (struct sockaddr *)(&stBindAddr), sizeof(stBindAddr));
    }

    if (connect(control_stream, (struct sockaddr_in *)server->s_in, sizeof(struct sockaddr_in)) < 0)
	{
		bb_perror_msg_and_die("Unable to connect to remote host (%s)",
				inet_ntoa(server->s_in->sin_addr));
	}
    //control_stream = xconnect(server->s_in);
    bb_default_error_retval = ATP_TRANS_OK;

	if (ftpcmd(NULL, NULL, control_stream, buf) != 220) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(ATP_TRANS_SYS_ERR);
	}

	/*  Login to the server */
	switch (ftpcmd("USER ", server->user, control_stream, buf)) {
	case 230:
		break;
	case 331:
		if (ftpcmd("PASS ", server->password, control_stream, buf) != 230) {
            bb_default_error_retval = ATP_TRANS_AUTH_ERR;
            exit(bb_default_error_retval);
		}
		break;
	default:
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(bb_default_error_retval);
	}

	ftpcmd("TYPE I", NULL, control_stream, buf);

	return(control_stream);
}

#ifdef CONFIG_FTPGET
static int ftp_recieve(ftp_host_info_t *server, int control_stream,
		const char *local_path, char *server_path)
{
	char buf[512];
	off_t filesize = 0;
	int fd_data;
	int fd_local = -1;
	off_t beg_range = 0;
    int lBytesSent;
    int lBytesToSend;
    char * pcLocalTemp;
    int readTotal = 0;
    FILE *fLocal;
    char pcServerPath[1024] = {0};
    struct timeval tv;
    
/*d104343 修改路径多一个/ start*/
    pcLocalTemp = server_path;
    strcpy(pcServerPath, server_path);
    if (pcLocalTemp[0] == '/'&& NULL != pcLocalTemp[1])
    {
        pcLocalTemp++;
        strcpy(pcServerPath, pcLocalTemp);
    }
/*d104343 修改路径多一个/ end*/

	/* Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(bb_default_error_retval);
	}
	fd_data = xconnect_ftpdata(server, buf);
    
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd_data,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    setsockopt(fd_data,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));

	if (ftpcmd("SIZE ", pcServerPath, control_stream, buf) == 213) {
		unsigned long value=filesize;
		if (safe_strtoul(buf + 4, &value))
		{
		    bb_default_error_retval = ATP_TRANS_SYS_ERR;
            exit(bb_default_error_retval);
        }
		filesize = value;
	}
	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
		fd_local = STDOUT_FILENO;
		do_continue = 0;
	}

	if (do_continue) {
		struct stat sbuf;
		if (lstat(local_path, &sbuf) < 0) {
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            exit(bb_default_error_retval);
		}
		if (sbuf.st_size > 0) {
			beg_range = sbuf.st_size;
		} else {
			do_continue = 0;
		}
	}

	if (do_continue) {
		sprintf(buf, "REST %ld", (long)beg_range);
		if (ftpcmd(buf, NULL, control_stream, buf) != 350) {
			do_continue = 0;
		} else {
			filesize -= beg_range;
		}
	}

#if 0
    if (NULL != server_path)
    {
        lBytesToSend = snprintf(buf, sizeof(buf), "RETR %s\r\n", server_path);
    }
    else
    {
        lBytesToSend = snprintf(buf, sizeof(buf), "RETR \r\n");
    }
    lBytesSent = 0;
	while (lBytesToSend > 0)
    {
        lBytesSent = send(control_stream, (buf+lBytesSent), lBytesToSend, 0);
        if (lBytesSent < 0)
        {
            bb_default_error_retval = ATP_TRANS_SYS_ERR;
            exit(ATP_TRANS_SYS_ERR);
        }

        lBytesToSend -= lBytesSent;
    }
#else
	if (ftpcmd("RETR ", pcServerPath, control_stream, buf) > 150) {
        bb_default_error_retval = ATP_TRANS_FILE_ERR;
        exit(ATP_TRANS_FILE_ERR);
	}
#endif

	/* only make a local file if we know that one exists on the remote server */
	if (fd_local == -1) {
		if (do_continue) {
			fd_local = bb_xopen(local_path, O_APPEND | O_WRONLY);
		} else {
			fd_local = bb_xopen(local_path, O_CREAT | O_TRUNC | O_WRONLY );
		}
	}

	/* Copy the file */
	if (bb_copyfd_size(fd_data, fd_local, filesize) < filesize) {
        bb_perror_msg("FTP get file failed");
        bb_default_error_retval = ATP_TRANS_FILE_ERR;
        exit(ATP_TRANS_FILE_ERR);
	}


	/* close it all down */
	close(fd_data);
	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
	}
	ftpcmd("QUIT", NULL, control_stream, buf);

	return(ATP_TRANS_OK);
}
#endif

#ifdef CONFIG_FTPPUT
static int ftp_send(ftp_host_info_t *server, int control_stream,
		const char *server_path, char *local_path)
{
	struct stat sbuf;
	char *pcLocal;
	char buf[512];
	int fd_data;
	int fd_local;
	int response;
    struct sockaddr_in stBindAddr;

	/*  Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(bb_default_error_retval);
	}
	fd_data = xconnect_ftpdata(server, buf);

	if (ftpcmd("CWD ", server_path, control_stream, buf) != 250) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(bb_default_error_retval);
	}

	/* get the local file */
	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
		fd_local = STDIN_FILENO;
	} 
    else 
	{
	    bb_default_error_retval = ATP_TRANS_SYS_ERR;
		fd_local = bb_xopen(local_path, O_RDONLY);
		fstat(fd_local, &sbuf);
        /*start of HG556a V100R001C01B021 AU8D01213 by c00131380 at 081129*/
        /*避免FTP服务端不支持ALLO命令而返回错误码终止传输的问题*/
		sprintf(buf, "ALLO %lu", (unsigned long)sbuf.st_size);
		response = ftpcmd(buf, NULL, control_stream, buf);
		switch (response) 
        {
    		case 200:
    		case 202:
            case 500:/*FTP服务器命令不支持*/
            case 550:
    			break;
    		default:
     			close(fd_local);
                bb_default_error_retval = ATP_TRANS_SYS_ERR;
                exit(bb_default_error_retval);
    			break;
		}
        /*end of HG556a V100R001C01B021 AU8D01213 by c00131380 at 081129*/
	}
    pcLocal = strrchr(local_path, '/');
    if (NULL == pcLocal)
    {
        pcLocal = "test";
    }
    else
    {
        pcLocal++;
    }
	//response = ftpcmd("STOR ", server_path, control_stream, buf);
	response = ftpcmd("STOR ", pcLocal, control_stream, buf);
	switch (response) {
	case 125:
	case 150:
		break;
	default:
		close(fd_local);
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
        exit(ATP_TRANS_SYS_ERR);
	}

	/* transfer the file  */
	if (bb_copyfd_eof(fd_local, fd_data) == -1) {
		exit(ATP_TRANS_SYS_ERR);
	}

	/* close it all down */
	close(fd_data);
	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
        bb_default_error_retval = ATP_TRANS_SYS_ERR;
	}
	ftpcmd("QUIT", NULL, control_stream, buf);

	return(ATP_TRANS_OK);
}
#endif

#define FTPGETPUT_OPT_GET	1
#define FTPGETPUT_OPT_SEND	2
#define FTPGETPUT_OPT_USER	4
#define FTPGETPUT_OPT_PASSWORD	8
#define FTPGETPUT_OPT_PORT	16

int ftpgetput_main(int argc, char **argv)
{
	/* content-length of the file */
	unsigned long opt = 0;
    char *pcLocalFile;
    char *pcRemoteFile;
    char *pcTransDirec;
    char *pcRemoteIP;
	char *port = "ftp";
    int  iIsrev;
	/* socket to ftp server */
	int control_stream;
	struct sockaddr_in s_in;
	/* continue a prev transfer (-c) */
	ftp_host_info_t *server;

	int (*ftp_action)(ftp_host_info_t *, int, const char *, char *) = NULL;

	/* Check to see if the command is ftpget or ftput */

    /*
     * Default values
     */
    bb_default_error_retval = ATP_TRANS_SYS_ERR;

	/* Set default values */
	server = xmalloc(sizeof(ftp_host_info_t));
	server->user = "anonymous";
	server->password = "busybox@";
	verbose_flag = 0;

	/*
	 * Decipher the command line
	 */
	pcRemoteFile = NULL;
    pcLocalFile = NULL;

    /*下载的两边路径都是文件名，上传得时候，
    本地是文件名，那边是路径名，例如 test/handy/ 或者 test/handy/*/
    pcRemoteIP = NULL;
	bb_applet_long_options = bb_transtool_long_options;
	opt = bb_getopt_ulflags(argc, argv, "gsu:p:r:l:P:B:A:",
                            &server->user,
                            &server->password,
                            &pcRemoteFile,
                            &pcLocalFile,
                            &port,
                            &m_pcLocalIP,
                            &pcRemoteIP);

    if ((0 == opt) || (NULL == argv[optind]) ||
        (NULL == pcRemoteFile) || (NULL == pcLocalFile))
    {
        bb_show_usage();
    }

	/* Process the non-option command line arguments */
    /* Check to see if the command is ftpget or ftput */
    iIsrev = 3;

	if (opt & FTPGETPUT_OPT_GET) {
		 ftp_action = ftp_recieve;
         iIsrev = 0;
	}

	if (opt & FTPGETPUT_OPT_SEND) {
		  ftp_action = ftp_send;
          iIsrev = 1;
	}

	/* We want to do exactly _one_ DNS lookup, since some
	 * sites (i.e. ftp.us.debian.org) use round-robin DNS
	 * and we want to connect to only one IP... */
	server->s_in = &s_in;
    if (NULL == pcRemoteIP)
	{
	    bb_lookup_host(&s_in, argv[optind]);    // May do DNS look up
    }
    else
    {
        bb_lookup_host(&s_in, pcRemoteIP);      // No DNS look up
    }
	s_in.sin_port = bb_lookup_port(port, "tcp", 21);

	control_stream = ftp_login(server);

    if (0 == iIsrev)
    {
	    return (ftp_action(server, control_stream, pcLocalFile, pcRemoteFile));
    }
    else if (1 == iIsrev)
    {
        return (ftp_action(server, control_stream, pcRemoteFile, pcLocalFile));
    }

    return ATP_TRANS_SYS_ERR;
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
