/***************************************************************************
 *     Copyright (c) 2004, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: nanoxml.h $
 * $brcm_Revision: Irvine_BSEAVSW_Devel/1 $
 * $brcm_Date: 1/6/04 11:15p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: /vobs/BSEAV/linux/lib/nanoxml/nanoxml.h $
 * 
 * Irvine_BSEAVSW_Devel/1   1/6/04 11:15p erickson
 * PR9211: created nanoxml library and testapp
 *
 ****************************************************************/
#ifndef NANOXML_H__
#define NANOXML_H__

/*=*******************
NanoXML is a minimal streaming SAX XML parser. As a SAX parser, it parses XML and fires events
back to the caller using callbacks. It does not build data structures like a DOM parser. As
a streaming parser, it can process XML in incremental buffers, even 1 byte at a time. It makes
no assumptions as to where memory boundaries are.

Features:
1) Tiny. It compiles to 1650 bytes on i386 with gcc (-O2, stripping symbols). 
2) Portable. It is written in cross-platform ANSI C. 
3) Fast. It minimizes memory copying to one instance: when a tag or attribute name 
   spans a nxml_write() call, otherwise there are no memory copies.
   Reading 4K disk reads, I measured 80 MB parsed in 1.3 seconds on stb-irva-01.broadcom.com.

Terminology:
	<tag attribute_name="attribute_value">data</tag>

Callbacks:
	tag_begin - called when a new tag is encountered. Passes the complete tag name.
	attribute_begin - called when a new attribute is encountered. Passes the complete attribute name.
	attribute_value - called with attribute value data. It could require multiple callbacks
		to complete the attribute value. 
	data - called with element data. It could require multiple callbacks to complete the
		element data.
	tag_end - called when a tag is closed. Even self closing tags (e.g. <tag/> receive 
		both a tag_begin and a tag_end.

Restrictions:
1) Tag and attribute names cannot be greater than NXML_MAX_NAME_SIZE. Larger
   names will be truncated without any warning.
2) You will receive one tag_begin callback at the beginning of each tag, but before any attribute
   callbacks for that tag.
3) You may receive zero or more attribute_begin callbacks after the tag_begin but before
   the first data callback. After you receive a data callback, you cannot receive
   an attribute_begin callback.
4) You can receive many data or attribute_value callbacks for each tag or attribute.
   This is because the size of the data or attribute_value is not bounded.
5) If you have a tag that has attributes and that tag ends with a "/" (e.g. <tag attr="value" />),
   the tag_end callback will not send you the tag_name.
   It will be a non-NULL pointer but len == 0.
   This could be changed by increasing the storage to two name buffers instead of one.
   
TODO:
1) support attribute values with single quotes or no quotes
2) supports double double quotes in attribute values
3) separate buffers for tag and attribute names
4) support <?xml version="1.0"?> tag.
*/

/**
Summary:
Opaque handle returned by nxml_open.
**/
typedef struct nxml *nxml_t;

/**
Summary:
Maximum size of tag or attribute name that can be guaranteed to
be returned whole.
**/
#define NXML_MAX_NAME_SIZE 128

/**
Summary:
Settings structure which must be passed to nxml_open.

Description:
Every callback MUST be specified.
**/
typedef struct {
	void (*tag_begin)(nxml_t handle, const char *tag_name, unsigned len);
	void (*attribute_begin)(nxml_t handle, const char *attr_name, unsigned len);
	void (*attribute_value)(nxml_t handle, const char *attr_value, unsigned len);
	void (*data)(nxml_t handle, const char *data, unsigned len);
	void (*tag_end)(nxml_t handle, const char *tag_name, unsigned len);
} nxml_settings;

#define min(A,B) ((A)<(B)?(A):(B))
#if 0
#define DEBUG(X) printf X
#else
#define DEBUG(X) 
#endif

typedef enum {
	state_begin_tag, /* look for < or data */
	state_tag_name, /* found <, looking for whole name */
	state_end_tag_name, /* found </, looking for whole name */
	state_attr_name, /* tag begun, looking for attr */
	state_attr_value_equals, /* attr_name found, looking for = */
	state_attr_value_quote, /* attr_name and = found, looking for quote */
	state_attr_value, /* attr name found, sending attr_value */
	state_finish_tag /* look for the >, ignoring everything else */
} nxml_state;

struct nxml {
	nxml_settings settings;
	nxml_state state;
	char namecache[NXML_MAX_NAME_SIZE];
	int namecachesize;
	int skipwhitespace;
};

#if defined(__cplusplus)
extern "C" {
#endif

/**
Summary:
Open a nanoxml parsing handle. 

Description:
The handle is required to maintain state between nxml_write calls.
You can open multiple handles and use them concurrently (there are no global variables).
*/
int nxml_open(nxml_t *handle, const nxml_settings *settings);

/*
Summary:
Close a nanoxml parsing handle when you are done.

Description:
The handle becomes invalid.
*/
void nxml_close(nxml_t handle);

/*
Summary:
Parse xml data.

Description:
You can write data in any amount that you want.
You will get 0 or more callbacks in response to a write call.
*/
int nxml_write(nxml_t handle, const char *data, unsigned len);

#if defined(__cplusplus)
}
#endif

#endif /* NANOXML_H__ */

