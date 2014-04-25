/******************************************************************************
  版权所有  : 2007-2020，华为技术有限公司
  文 件 名  : wlancmd.c
  作    者  : l69021
  版    本  : 1.0
  创建日期  : 2007-8-22
  描    述  : 提供WLAN组织模块调用的WLAN配置模块命令接口文件
  函数列表  :
                main
                wlancmdHelp
                wlancmdUsage

  历史记录      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "wlancmd.h"
#include <net/if.h>
static void wlancmdUsage(wlan_cmd_t *cmd);
static int wlancmdHelp(void);
static void wlanFind(struct ifreq *ifr);
/* wlancmd 命令 */
wlan_cmd_t wlan_cmds[] = 
{
    /* WLANCMD的版本信息 */
    {"cmdver", wlan_cmdverison, "show wlancmd version.\n"
                                "\t\"wlancmd cmdver display\""},
    /* WLAN驱动的版本信息 */
    {"driverver", wlan_driververison, "show wlan driver version.\n"
                                      "\t\"wlancmd driverver display\""},
    /* WLAN芯片firmware的版本信息 */
    {"firmwarever", wlan_firmwareverison, "show wlan fireware version.\n"
                                          "\t\"wlancmd firmwarever display\""},
    /* 启动WLAN */
    {"up", wlan_up, "enable wlan.\n"
                    "\t\"wlancmd up\""},
    /* 去使能WLAN */
    {"down", wlan_down, "disable wlan.\n"
                        "\t\"wlancmd down\""},    
    /* 工作模式 */
    {"mode", wlan_mode, "set or display wlan mode.\n"
                        "\t\"wlancmd mode set {bonly|gonly|mixed}\""},    
    /* 传输速率 */
    {"rate", wlan_rate, "set or display a fixed rate.\n"
                        "\t\"wlancmd rate set {rate value}\"\n"
                        "\t802.11a :(6, 9, 12, 18, 24, 36, 48, 54)\n"
                        "\t802.11b :(1, 2, 5.5, 11)\n"
                        "\t802.11g :(1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54)\n"
                        "\t0 default: automatically select the best rate"},
    /* 传输速率集 */
    {"rateset", wlan_rateset, "set or display the basic rateset, (b) indicates basic.\n"                              
                              "\trateset \"default\" | \"all\" | <arbitrary rateset>\n"
                              "\t\tdefault - driver defaults\n"
                              "\t\tarbitrary rateset - list of rates\n"
                              "\t\tall - all rates are basic rates\n"
                              "\tList of rates are in Mbps and each rate is optionally followed\n"
                              "\tby \"(b)\" or \"b\" for a Basic rate. Example: 1(b) 2b 5.5 11\n"
                              "\tAt least one rate must be Basic for a legal rateset."},
      
    /* 工作信道 */
    {"channel", wlan_channel, "set or display wlan channel.\n"
                              "\tvalid channels in 802.11b/g: 1-14\n"
                              "\tvalid channels in 802.11a:\n"
                              "\t\t36, 40, 44, 48, 52, 56, 60, 64,\n"
                              "\t\t100, 104, 108, 112, 116,120, 124, 128, 132, 136, 140,\n"
                              "\t\t149, 153, 157, 161,\n"
                              "\t\t184, 188, 192, 196, 200, 204, 208, 212, 216\n"
                              "\t wlancmd channel set 0, for auto channel mode"},
    /* 国家码 */
    {"region", wlan_region, "set or display wlan Country code.\n"
                            "\tUse either long name or abbreviation from ISO 3166."},
    /* 物理层类型 */
    {"phytype", wlan_phytype, "display wlan phy type."},
    /* 射频功率 */
    {"txpwr", wlan_txpwr, "set or display tx power in various units.\n"     
                          "\t-d dbm units\n"
                          "\t-m milliwatt units"},
#ifdef SUPPORT_MBSS
    {"mbss", wlan_mbss, "set wlan multi-ssid."},
    {"cur_etheraddr", wlan_curetheraddr, "set or display the current hw address."},
#endif
    /* BSSID */
    {"bssid", wlan_bssid, "display wlan bssid."},
    /* SSID */
    {"ssid", wlan_ssid, "set or display a configuration's SSID.\n"
                        "\t\"wlancmd ssid set [-i SsidIndex] [<ssid>]\"\n"
                        "\tIf the configuration index 'SsidIndex' is not given,\n" 
                        "\tconfiguraion #0 is assumed"},
    /* 使能或禁止SSID */
    {"status", wlan_status, "set or get BSS enabled status: up/down."},
    
    /* 隐藏或显示SSID */
    {"hide", wlan_hide, "hides the network from active scans, enable or disable.\n"
                        "\t0 is open, 1 is hide."},                        
    /* 显示连接到无线接口的STA的MAC地址 */
    {"assoclist", wlan_assoclist, "Get the list of linked MACaddr."},
    /* SSID的安全方式 */
    {"secmode", wlan_secmode, "set or display one SSID's secure mode.\n"
                              "\t\"wlancmd secmode set [-i SsidIndex]\n"
                              "\t{open|share|wpa|wpa-psk|wpa2|wpa2-psk}\""},
    /* SSID的WEPKEY */
    {"wepkey", wlan_wepkey, "set or display one SSID wepkey.\n"
                            "\t\"wlancmd wepkey set [-i SsidIndex] [-k KeyIndex] {KeyValue}\""},
    /* WEPKEY索引号 */
    {"wepkeyindex", wlan_wepkeyindex, "set or display one SSID wepkey index number.\n"
                                      "\t\"wlancmd wepkeyindex set [-i SsidIndex] {KeyNumber}\"\n"},
    /* SSID的PSKKEY */
    {"pskkey", wlan_pskkey, "set or display one SSID pskkey.\n"
                            "\t\"wlancmd pskkey set [-i SsidIndex] {Keyvalue}\n"},
    /* WPA/WPA2加密方式 */
    {"wpaencryption", wlan_wpaencryption, "set or display one SSID WPA encryption Mode.\n"
                                          "\t\"wlancmd wpaencryption set [-i SsidIndex]\n"
                                          "\t{aes|tkip|aes+tkip}\""},
    /* QOS */
    {"wlanqos", wlan_wlanqos, "set or display wlan QoS enable/disable.\n"},
    
    /* CNXT Need */
    {"gprtctrl", wlan_cnxt, "cnxt : Get/Set 11g protection status control alg.\n"
                            "\t(0=always off, 1=monitor local association, 2=monitor overlapping BSS)"},
    {"setplcphdr", wlan_cnxt, "cnxt : Set plcp header\n"
                           "\t\"long\" or \"auto\" or \"debug\"" },
    {"rtsthresh", wlan_cnxt, "cnxt : Deprecated. Use rtsthresh."},
    {"fragthresh", wlan_cnxt, "cnxt : Deprecated. Use fragthresh."},    
    {"dtim", wlan_cnxt, "cnxt : Get/Set DTIM."},
    {"beaconinterval", wlan_cnxt, "cnxt : Get/Set the beacon period."},
    {"frameburst", wlan_cnxt, "cnxt : Disable/Enable frameburst mode."},
    {"setwme", wlan_cnxt, "cnxt : Set WME (Wireless Multimedia Extensions) status\n" 
                          "\t(0=off, 1=on, -1=auto)."},
    {"setwmestr", wlan_cnxt, "cnxt : Set WME ACK strategy status" 
                             "(0 = ack , 1 = no ack, 2 = dynamic)."},
    {"reg_domain", wlan_cnxt, "cnxt : Get/Set domain mode under 802.11d."},
    {"regulatory", wlan_regulatory, "Get/Set regulatory domain mode (802.11d). Driver must be down."},
    {"radar", wlan_radar, "Enable/Disable radar"},
    {"spect", wlan_spect, "Get/Set 802.11h Spectrum Management mode.\n"},
      
    {"help", wlancmdHelp, "show wlancmd help"},
/***************************************Begin to add for V100R001 ****************************/
    {"fltmacctl",wlan_fltmacctl,"set or display the mac filter enable or disable.\n"
                                   "\t\"wlancmd fltmacctl set {enable | disable}\""},
    {"fltmacmode",wlan_fltmacmode,"set or display the format of mac filter allow or deny .\n"
                                     "\t\"wlancmd fltmacmode set {allow |deny}\""},
    {"fltmac",wlan_fltmac,"set or display the soure macaddress.\n"
                             "\t\"wlancmd fltmac set [-i SsidIndex]{macadd value}\""},
    {"assocctl",wlan_assocctl,"set or display the assoc device ctl.\n"
                               "\t\"wlancmd assocctl set [-i ssididex]{value}\""},
    {"bss",wlan_bss,"set or display the assoc device ctl.\n"
                               "\t\"wlancmd assocctl set [-i ssididex]{value}\""},
#ifdef SUPPORT_WPS
    {"wps_nvram",wlan_wpsnvram,"init wlan nvram .\n"
                                "\t\"wlancmd wps_nvram init\""},
    {"wps_ctl",wlan_wpsctl,"set or display wps status,enable or disable.\n"
                                "\t\"wlancmd wps_ctl set {enabled|disabled}\""},
    {"wps_mode",wlan_wpsmode,"set wps mode,ap-pin or ap-pbc.\n"
                                "\t\"wlancmd wps_mode set {ap-pin|ap-pbc}\""},
    {"wps_pinval",wlan_wpspinval,"set the pin value as wps under the pin mode.\n"
                                "\t\"wlancmd wps_pinval set {pin value}\""},
    {"wps_wsc",wlan_wpswsc,"start the wsc course.\n"
                                "\t\"wlancmd wps_wsc startwps\""},
    {"wps_para",wlan_wpspara,"set wps parameter.\n"
                                "\t\"wlancmd wps_para set {wps parameter}.\""},
    {"wps_get_stat",wlan_wpsgetstat,"display wps negotiate status.\n"
                                "\t\"wlancmd wps_get_stat.\""},                                
/*start of minpengwei*/                                
    {"wps_set_confstat",wps_set_confstat,"display wps pbc enable.\n"
                                "\t\"wlancmd wps_set_confstat.\""},
/*end of minpengwei*/                         
#endif
#ifdef SUPPORT_MBSS
    {"ap_isolate",wlan_isolate,"set or display AP isolation.\n"},
    {"maxassoc",wlan_maxassoc,"set or display AP max association.\n"},
#endif
    {"Qosctl",wlan_Qosctl,"set or display the Qos enable or disable.\n"},
    {"Stactl",wlan_Stactl,"set or display the Sta enable or disable.\n"},
#ifdef SUPPORT_MIMO
    {"HT_GI",wlan_HT_GI,"set or display long or short Guard Interval.\n"},
    {"HT_AutoBA",wlan_HT_AutoBA,"set or display the function of setuping BA session automatically after connected enable or disable.\n"},
    {"HT_BW",wlan_HT_BW,"set or display 11n channel width.\n"},
/* HUAWEI VHG WangXinfeng 69233 2009-04-13 Modify for Arcor WLAN requirement begin */
    {"HtExtcha",wlan_HtExtcha,"Set extension channel\n"},

    {"HT_RDG", wlan_HT_RDG, "Set HT Reverse Direction Grant to enabled/disabled"},
    
    {"HT_BADecline", wlan_HT_BADecline, "Set Reject peer BA-Reques to true/false"},

    {"TxBurst", wlan_TxBurst, "Set TxBurst to 1/0"},
#endif

    { "site_survey" , wlan_site_survey, "set do site survey or not\n"
                                "\t\"wlancmd SiteSurvey set <0|1>.\""},

//#ifdef SUPPORT_WDS
    { "WdsEnable" , WlanCmdWdsEnable, "set Wds mode Bridge/Repeater/LAZY\n"},       
    { "WdsList" , WlanWdsList, "set Wds MAC address\n"},    
    { "WdsEncrypType" , WlanWdsEncrypType, "The encrptytion of wds is OPEN/TKIP/WEP/AES\n"},    
    { "WdsKey" , WlanWdsKey, "The key material of WDS link\n"},    
    { "Wds0Key" , WlanWds0Key, "The key material of wds0 link\n"},    
    { "Wds1Key" , WlanWds1Key, "The key material of wds1 link\n"},    
    { "Wds2Key" , WlanWds2Key, "The key material of wds2 link\n"},    
    { "Wds3Key" , WlanWds3Key, "The key material of wds3 link\n"},    
    { "WdsDefaultKeyID" , WlanWdsDefaultKeyID, "The default key index setting.\n"},    
    { "WdsPhyMode" , WlanWdsPhyMode, "set WDS phy mode\n"},
//#endif
//#ifdef SUPPORT_RADIUS
    { "IEEE8021X" , WlanIEEE8021X, "This field is enable only when Radius-WEP mode on, otherwise must disable\n"},
    { "EAPifname" , WlanEAPifname, "The binding interface for EAP negotiation\n"},
    { "PreAuthifname" , WlanPreAuthifname, "The binding interface for WPA2 Pre-authentication\n"},
    { "RADIUS_Server" , WlanRADIUS_Server, "IP for Radius server\n"},
    { "RADIUS_Port" , WlanRADIUS_Port, "This is port number for IAS service in Authentication Server\n"},
    { "RADIUS_Key" , WlanRADIUS_Key, "This is Radius Secret shared with Authenticator and AS\n"},
    { "own_ip_addr" , Wlanown_ip_addr, "set the connected stations ip address.\n"},
    { "RadiusSessionTimeout" , Wlansession_timeout_interval, "set or disable reauthenticaion interval with unit of second\n"},
//#endif
/* HUAWEI VHG WangXinfeng 69233 2009-04-13 Modify for Arcor WLAN requirement end */
/***************************************End of add********************************************/
    {NULL, NULL, NULL }
};

/******************************************************************************
  函数名称  : wlancmdUsage
  功能描述  : 打印相应命令的帮助信息。
  输入参数  : 
              1.  *cmd: 需要显示帮助信息的命令
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
static void wlancmdUsage(wlan_cmd_t *cmd)
{
    if (strlen(cmd->name) >= 8)
    {
        fprintf(stderr, "%s\n\t%s\n\n", cmd->name, cmd->help);
    }
    else
    {
        fprintf(stderr, "%s\t%s\n\n", cmd->name, cmd->help);
    }
}

/******************************************************************************
  函数名称  : wlancmdHelp
  功能描述  : 打印wlancmd所有命令的帮助信息
  输入参数  : 
              1. void:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
static int wlancmdHelp(void)
{
    wlan_cmd_t *cmd = NULL;

    for (cmd = wlan_cmds; cmd->name; cmd++) 
    {
        wlancmdUsage(cmd);
    }
    
    return 0;
}

/******************************************************************************
  函数名称  : main
  功能描述  : wlancmd 命令入口点
  输入参数  : 
              1.  argc:
              2.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 0: 成功
              非0 : 失败

  修改历史      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int main(int argc, char **argv)
{
    wlan_cmd_t *cmd = NULL;
    int err = 0;
    struct ifreq ifr;
    char *ifname = NULL;
    int help = 0;
    int status = CMD_WL;
    char *av0 = argv[0];
    
    memset(&ifr, 0, sizeof(ifr));
    for (++argv; *argv; ) 
    {                
        /* search for command */
        for (cmd = wlan_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);
        if (cmd->name)
        {
            /* do command */
            status = wlanoption(&argv, &ifname, &help);
            if (status == CMD_OPT) 
            {
                if (help)
                {
                    break;
                }
                
                if (ifname)
                {
                    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
                }
            }
            /* parse error */
            else if (status == CMD_ERR)
            {
                break;
            }

            /* use default interface */
            if (!*ifr.ifr_name)
            {
                wlanFind(&ifr);
            }
            /* validate the interface */
            if (!*ifr.ifr_name || wl_check((void *)&ifr)) 
            {
#ifndef SUPPORT_MIMO               
                fprintf(stderr, "%s: wlan driver adapter not found\n", av0);
                exit(1);
#endif
            }
            err = (*cmd->func)((void *) &ifr, argv);
        }
        else 
        {
            fprintf(stderr, "unrecognized name \"%s\", type \"wlancmd help\" for help\n",
                argv[0]);            
        }

        break;
    }

    if (err != 0)
    {
        wlancmdUsage(cmd);
    }
    
    return err;
}
static void wlanFind(struct ifreq *ifr)
{
    char proc_net_dev[] = "/proc/net/dev";
    char buf[1000], *c, *name;
    char dev_type[3];
    FILE *fp;

    ifr->ifr_name[0] = '\0';

    /* eat first two lines */
    if (!(fp = fopen(proc_net_dev, "r")) ||
        !fgets(buf, sizeof(buf), fp) ||
        !fgets(buf, sizeof(buf), fp))
    {
        return;
    }
    
    while (fgets(buf, sizeof(buf), fp)) 
    {
        c = buf;
        while (isspace(*c))
        {
            c++;
        }
        if (!(name = strsep(&c, ":")))
        {
            continue;
        }

        strncpy(ifr->ifr_name, name, IFNAMSIZ);
        
        /* The driver doesn't support get_dev_type,
        * check if "wl0" is existing.
        * thats the default interface.
        */
 
        if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 && (!strncmp(dev_type, "wl", 2)))
        {
            if (wl_check((void *) ifr) == 0)
            {      
                break;
            }
        }
        ifr->ifr_name[0] = '\0';
    }
    
    fclose(fp);
}

