/* vi: set sw=4 ts=4: */
/*
 * Utility routines.
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
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
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "busybox.h"


#if BUFSIZ < 4096
#undef BUFSIZ
#define BUFSIZ 4096
#endif

#define BUF_LEN 1024

/* If size is 0 copy until EOF */
static size_t bb_full_fd_action(int src_fd, int dst_fd, const size_t size)
{
	size_t read_total = 0;
	RESERVE_CONFIG_BUFFER(buffer,BUFSIZ);

	while ((size == 0) || (read_total < size)) {
		size_t read_try;
		ssize_t read_actual;

 		if ((size == 0) || (size - read_total > BUFSIZ)) {
			read_try = BUFSIZ;
		} else {
			read_try = size - read_total;
		}

		read_actual = safe_read(src_fd, buffer, read_try);
		if (read_actual > 0) {
			if ((dst_fd >= 0) && (bb_full_write(dst_fd, buffer, (size_t) read_actual) != read_actual)) {
				bb_perror_msg(bb_msg_write_error);	/* match Read error below */
				break;
			}
		}
		else if (read_actual == 0) {
			if (size) {
				bb_error_msg("Unable to read all data");
			}
			break;
		} else {
			/* read_actual < 0 */
			bb_perror_msg("Read error");
			break;
		}

		read_total += read_actual;
	}

	RELEASE_CONFIG_BUFFER(buffer);

	return(read_total);
}

/* start of 增加tr143 诊断功能中的即时统计数据*/
static size_t bb_full_fd_ftp_action(int src_fd, int dst_fd, const size_t size)
{
	size_t read_total = 0;
	RESERVE_CONFIG_BUFFER(buffer,BUFSIZ);

	while ((size == 0) || (read_total < size)) {
		size_t read_try;
		ssize_t read_actual;

 		if ((size == 0) || (size - read_total > BUFSIZ)) {
			read_try = BUFSIZ;
		} else {
			read_try = size - read_total;
		}

		read_actual = safe_read(src_fd, buffer, read_try);
		if (read_actual > 0) {
			if ((dst_fd >= 0) && (bb_full_write(dst_fd, buffer, (size_t) read_actual) != read_actual)) {
				bb_perror_msg(bb_msg_write_error);	/* match Read error below */
				break;
			}
		}
		else if (read_actual == 0) {
			if (size) {
				bb_error_msg("Unable to read all data");
			}
			break;
		} else {
			/* read_actual < 0 */
			bb_perror_msg("Read error");
			break;
		}

		read_total += read_actual;
	}

	RELEASE_CONFIG_BUFFER(buffer);

	return(read_total);
}

/* end of 增加tr143 诊断功能中的即时统计数据*/

extern int bb_copyfd_size(int fd1, int fd2, const off_t size)
{
	if (size) {
		return(bb_full_fd_action(fd1, fd2, size));
	}
	return(0);
}

extern int bb_copyfd_eof(int fd1, int fd2)
{
	return(bb_full_fd_action(fd1, fd2, 0));
}


/* start of HG556A 增加tr143 诊断功能 by KF20515 at 1/5/2010 */


static size_t bb_full_fd_action_tr143( size_t testLen, const int dst_fd)
{
	size_t  write_actual =BUF_LEN;
	size_t  total_actual =0;
	char buf[BUF_LEN] = {0};

	while(testLen >0) {
	      if ((testLen < BUF_LEN  ) ||(testLen == BUF_LEN))
	     {
		     write_actual = testLen;
		     memset(buf, 0, (write_actual ));
		     memset(buf, 'A', (write_actual));		     
		     testLen =0;
	      }  else {
		     write_actual = BUF_LEN;
		     testLen = testLen - write_actual;
		     memset(buf, 0, write_actual);
		     memset(buf, 'A', write_actual);		    
	      }
		if ((dst_fd >= 0) && (bb_full_write(dst_fd, buf, (size_t) write_actual) != write_actual)) {
				bb_perror_msg(bb_msg_write_error);
				break;
		}
		total_actual = total_actual + write_actual;
	}
	return total_actual;
}


extern int bb_copyfd_eof_tr143(int testLen, int fd2)
{
	return(bb_full_fd_action_tr143(testLen, fd2));
}

/*end of HG556A 增加tr143 诊断功能 by KF20515 at 1/5/2010 */

