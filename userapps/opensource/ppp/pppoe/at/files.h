/* files.h */
#ifndef _AT_FILES_H_
#define _AT_FILES_H_

#define ATCFG_BUF_LEN                          300
#define ATCFG_PROFILE                          "/var/at_profile"

/*运营商选择方式*/
enum
{
	ATCFG_OPTOR_AUTO   = 0,                     //手自动选择运营商
	ATCFG_OPTOR_MANUAL                          //动选择运营商
};

/*链接类型*/
enum
{
    ATCFG_CONNTYPE_GPRS_FIRST = 0,              //GPRS优先
    ATCFG_CONNTYPE_3G_FIRST,                    // 3G优先
    ATCFG_CONNTYPE_GPRS_ONLY,                   //仅仅使用GPRS
    ATCFG_CONNTYPE_3G_ONLY,                       //仅仅使用3G
    ATCFG_CONNTYPE_AUTO,                            //自动
};

/*通道选择类型*/
/*
enum
{
    ATCFG_CHANNEL_UNLIMITED = 0,                //通道不限制
    ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100,   //GSM900/GSM1800/WCDMA2100
    ATCFG_CHANNEL_GSM1900                        //GSM1900
};
*/


struct config_keyword {
	char *keyword;
	int (*handler)(char *line, void *var);
	void *var;
	char *def;
};

int read_config(char *file);
#endif
