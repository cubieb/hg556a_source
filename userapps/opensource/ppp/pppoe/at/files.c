/* 
 * files.c -- AT file manipulation *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "files.h"
#include "at_thread.h"

static int read_str(char *line, void *arg)
{
	char **dest = arg;

	if(NULL == line)
	{
		return	0;
	}
	if (*dest) free(*dest);

	*dest = strdup(line);

    return 1;
}

static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	if(NULL == line)
	{
		return	0;
	}
	*dest = strtoul(line, NULL, 0);
	return 1;
}


static int read_operator(char *line, void *arg)
{
	unsigned int *dest = (unsigned int*)arg;
	
	if (!strcasecmp("auto", line)|| !strcasecmp("automatic", line) || !strcasecmp("0", line))
		*dest = ATCFG_OPTOR_AUTO;
	else if (!strcasecmp("manual", line) || !strcasecmp("1", line))
		*dest = ATCFG_OPTOR_MANUAL;
	else return 0;
	
	return 1;
}

static int read_conntype(char *line, void *arg)
{
	unsigned int *dest = (unsigned int*)arg;
	
	if (!strcasecmp("gprsfirst", line)|| !strcasecmp("gprs_first", line) || !strcasecmp("0", line))
		*dest = ATCFG_CONNTYPE_GPRS_FIRST;
	else if (!strcasecmp("3gfirst", line) || !strcasecmp("3g_first", line) || !strcasecmp("1", line))
		*dest = ATCFG_CONNTYPE_3G_FIRST;
	else if (!strcasecmp("gprsonly", line) || !strcasecmp("gprs_only", line) || !strcasecmp("2", line))
		*dest = ATCFG_CONNTYPE_GPRS_ONLY;
	else if (!strcasecmp("3gonly", line) || !strcasecmp("3g_only", line) || !strcasecmp("3", line))
		*dest = ATCFG_CONNTYPE_3G_ONLY;
        else if ( !strcasecmp("auto", line) || !strcasecmp("4", line) )
        {
            *dest = ATCFG_CONNTYPE_AUTO;
        }
	else return 0;
	
	return 1;
}
/*
static int read_channel(char *line, void *arg)
{
	unsigned int *dest = (unsigned int*)arg;
	if (!strcasecmp("unlimited", line)|| !strcasecmp("unlimit", line) || !strcasecmp("unrestricted", line) || !strcasecmp("0", line))
		*dest = ATCFG_CHANNEL_UNLIMITED;
	else if (!strcasecmp("gsm900/1800/wcdma2100", line) || !strcasecmp("gsm900/gsm1800/wcdma2100", line)
		      || !strcasecmp("gsm900_gsm1800_wcdma2100", line) || !strcasecmp("1", line))
		*dest = ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100;
	else if (!strcasecmp("gsm1900", line) || !strcasecmp("2", line))
		*dest = ATCFG_CHANNEL_GSM1900;
	else return 0;

	
	return 1;
}
*/
extern struct tagATConfig at_config;

static struct config_keyword keywords[] = {
	/* keyword	       handler    variable address		     default */
	{"profile",	      read_str,  &(at_config.profile),	      "Default"},
	{"phone_number",  read_str,  &(at_config.phone_number),    "*99#"},
	{"ap_name",       read_str,  &(at_config.ap_name),         ""},
	{"operator",	  read_operator,  &(at_config.operator),  "0"},
	{"conn_type",	  read_conntype,  &(at_config.conn_type), "0"},
	{"channel",		  read_str,  &(at_config.channel),	  "3FFFFFFF"},
	{"",		      NULL, 	  NULL,				              ""}
};

int read_config(char *file)
{
	FILE *in;
    char buffer[ATCFG_BUF_LEN], *token, *line;

	int i;

	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		return 0;
	}

	memset(buffer,0,ATCFG_BUF_LEN);
	while (fgets(buffer, ATCFG_BUF_LEN, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		//if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		line = line + strspn(line, " \t=");
		if (*line == '\0') continue;
				
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				keywords[i].handler(line, keywords[i].var);
	}
	fclose(in);
	return 1;
}

