/***************************************************************************
 *     Copyright (c) 2004, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: nanoxml.c $
 * $brcm_Revision: Irvine_BSEAVSW_Devel/1 $
 * $brcm_Date: 1/6/04 11:15p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: /vobs/BSEAV/linux/lib/nanoxml/nanoxml.c $
 * 
 * Irvine_BSEAVSW_Devel/1   1/6/04 11:15p erickson
 * PR9211: created nanoxml library and testapp
 * 
 * 7/2/04 ytran -- initialize data with zero; garbage content returned by malloc
 ****************************************************************/
#include "nanoxml.h"
#include <stdlib.h>
#include <string.h>

int nxml_open(nxml_t *handle, const nxml_settings *settings)
{
        if ((*handle = (nxml_t)malloc(sizeof(**handle))) != NULL) {
	  (*handle)->settings = *settings;
	  (*handle)->namecachesize = 0;
	  (*handle)->skipwhitespace = 1;
	  (*handle)->state = state_begin_tag;
	}
	return 0;
}

void nxml_close(nxml_t handle)
{
	free(handle);
}

#define WHITESPACE " \t\r\n"
#define strskip(DATA,CHARS) ((const char *)(strspn(DATA,CHARS) + (DATA)))

/* Append data to the namecache. This is used for tag and attribute names.
If the nocopy parameters are specified, then the caller regards the name
as complete, and if there's nothing already in the cache, then there's no
need for a copy. */
static void nxml_add_namecache(nxml_t handle, const char *data, int len,
	const char **data_nocopy, int *len_nocopy)
{
	/* if the nocopy parameters are supplied, and there's nothing in the cache, 
	then don't copy. just pass them back. */
	if (data_nocopy && !handle->namecachesize) {
		*data_nocopy = data;
		*len_nocopy = len;
	}
	else {
		if (len > NXML_MAX_NAME_SIZE - handle->namecachesize)
			len = NXML_MAX_NAME_SIZE - handle->namecachesize;
		if (len) {
			strncpy(&handle->namecache[handle->namecachesize], data, len);
			handle->namecachesize += len;
		}
		if (data_nocopy) {
			*data_nocopy = handle->namecache;
			*len_nocopy = handle->namecachesize;
		}
	}
}

int nxml_write(nxml_t handle, const char *data, unsigned len)
{
DEBUG((" ---- nxml_write(): len %d\n",len));
        int psiEnd=0; 
	const char *enddata = data + len;
	while ((data < enddata) && (!psiEnd)) {
		const char *s; /* temp value for capturing search results */
		
		/* skip whitespace */
		if (handle->skipwhitespace) {
			s = strskip(data, WHITESPACE);
			if (s != data) {
				data = s;
				continue;
			}
		}

DEBUG((" ---- state %d, %c, %d\n", handle->state, *data, handle->namecachesize));
		switch (handle->state) {
		case state_begin_tag:
			s = (const char *)strchr(data, '<');
			if (!s) {
				/* it's all data */
				(*handle->settings.data)(handle, data, enddata-data);
				return 0;
			}
			else if (data != s) {
				/* we have some data, then a tag */
				(*handle->settings.data)(handle, data, s-data);
			}
			/* skip over the tag begin and process the tag name */
			data = s+1;
			handle->state = state_tag_name;
			handle->namecachesize = 0;
			break;

		case state_finish_tag:
			/* we don't care about anything but the end of a tag */
			s = (const char *)strchr(data, '>');
			if (!s) return 0;

			/* we found it, so start looking for the next tag */
			data = s+1;
			handle->state = state_begin_tag;
			break;

		case state_end_tag_name:
			s = (const char *)strpbrk(data, WHITESPACE ">");
			if (!s) {
				/* it's all name, and we're not done */
				nxml_add_namecache(handle, data, enddata-data, NULL, NULL);
				handle->skipwhitespace = 0;
				return 0;
			}
			else {
				const char *name;
				unsigned len;
				nxml_add_namecache(handle, data, s-data, &name, &len);

				if (strncmp(name,"psitree",len) == 0)
				  psiEnd = 1;

				(*handle->settings.tag_end)(handle, name, len);
				handle->state = state_finish_tag;
				data = s;
			}
			break;

		case state_tag_name:
		case state_attr_name:
			if (*data == '/') {
				/* this tag is done */
				if (handle->state == state_tag_name && !handle->namecachesize) {
					/* we can still parse the end tag name so that the uppperlevel app
					can validate if it cares */
					handle->state = state_end_tag_name;
					data++;
					break;
				}
				else if (handle->state == state_attr_name) {
					/* we had an attribute, so this tag is just done */
					(*handle->settings.tag_end)(handle, handle->namecache, handle->namecachesize);
					handle->state = state_finish_tag;
					data++;
					break;
				}
			}
			else if (*data == '>') {
				handle->state = state_begin_tag;
				data++;
				break;
			}

			/* TODO: is = a valid in a tag? I don't think so. */
			s = (const char *)strpbrk(data, WHITESPACE "=/>");
			if (!s) {
				/* it's all name, and we're not done */
				nxml_add_namecache(handle, data, enddata-data, NULL, NULL);
				handle->skipwhitespace = 0;
				return 0;
			}
			else {
				/* we have the entire name */
				const char *name;
				int len;
				nxml_add_namecache(handle, data, s-data, &name, &len);

				if (handle->state == state_tag_name) {
					(*handle->settings.tag_begin)(handle, name, len);
					handle->state = state_attr_name;
				}
				else {
					(*handle->settings.attribute_begin)(handle, name, len);
					handle->state = state_attr_value_equals;
				}
				handle->namecachesize = 0;
				data = s;
			}
			break;

		case state_attr_value:
			s = (const char *)strchr(data, '"');
			if (!s) {
				/* it's all attribute_value, and we're not done */
				(*handle->settings.attribute_value)(handle, data, enddata-data);
				handle->skipwhitespace = 0;
				return 0;
			}
			else {
				/* we have some data, then a tag */
				(*handle->settings.attribute_value)(handle, data, s-data);
			}
			/* skip over the quote and look for more attributes */
			data = s+1;
			handle->state = state_attr_name;
			handle->namecachesize = 0;
			break;


		case state_attr_value_equals:
			if (*data == '>') {
				handle->state = state_begin_tag;
				data++;
			}
			else if (*data == '=') {
				handle->state = state_attr_value_quote;
				data++;
			}
			else
				handle->state = state_attr_name;
			break;

		case state_attr_value_quote:
			if (*data == '"')
				data++;
			handle->state = state_attr_value;
			break;
		}
		/* start of maintain 修改解析器会自动删除属性值的前导空格问题 by xujunxia 43813 2006年5月12日
		handle->skipwhitespace = 1;
		*/
		if (state_attr_value == handle->state)
	       {
	            handle->skipwhitespace = 0;
	       }
		else
	       {
		    handle->skipwhitespace = 1;
	       }
		/* end of maintain 修改解析器会自动删除属性值的前导空格问题 by xujunxia 43813 2006年5月12日 */
	}
	return 0;
}


