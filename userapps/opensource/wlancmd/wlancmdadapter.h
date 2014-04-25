/******************************************************************************
  版权所有  : 2007-2020，华为技术有限公司
  文 件 名  : wlancmdadapter.h
  作    者  : l69021
  版    本  : 1.0
  创建日期  : 2007-8-22
  描    述  : WLANCMD命令适配头文件
  函数列表  :

  历史记录      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

*********************************************************************************/

#ifndef __WLANCMDADAPTER_H__
#define __WLANCMDADAPTER_H__

typedef int (cmd_func_t)(void *wl, char **argv);

extern int wlan_cmdverison(void *wl, char **argv);
extern int wlan_cmdverison(void *wl, char **argv);
extern int wlan_driververison(void *wl, char **argv);
extern int wlan_firmwareverison(void *wl, char **argv);
extern int wlan_up(void *wl, char **argv);
extern int wlan_down(void *wl, char **argv);
extern int wlan_mode(void *wl, char **argv);
extern int wlan_rate(void *wl, char **argv);
extern int wlan_rateset(void *wl, char **argv);
extern int wlan_channel(void *wl, char **argv); 
extern int wlan_region(void *wl, char **argv);
extern int wlan_phytype(void *wl, char **argv);
extern int wlan_txpwr(void *wl, char **argv);
extern int wlan_bssid(void *wl, char **argv);
extern int wlan_ssid(void *wl, char **argv);
extern int wlan_status(void *wl, char **argv);
extern int wlan_hide(void *wl, char **argv);
extern int wlan_assoclist(void *wl, char **argv);
extern int wlan_secmode(void *wl, char **argv);
extern int wlan_wepkey(void *wl, char **argv);
extern int wlan_wepkeyindex(void *wl, char **argv);
extern int wlan_pskkey(void *wl, char **argv);
extern int wlan_wpaencryption(void *wl, char **argv);
extern int wlan_wlanqos(void *wl, char **argv);

extern int wlan_regulatory(void *wl, char **argv);
extern int wlan_radar(void *wl, char **argv);
extern int wlan_spect(void *wl, char **argv);

/*******************************Begin to add for V100R001*****************/
extern int wlan_fltmacctl(void *wl, char **argv);
extern int wlan_fltmacmode(void *wl, char **argv);
extern int wlan_fltmac(void *wl, char **argv);
extern int wlan_assocctl(void *wl, char **argv);
/*******************************End of add ********************************/
extern int wlan_cnxt(void *wl, char **argv);
extern int wlan_bss(void *wl, char **argv);
#ifdef SUPPORT_WPS
extern int wlan_wpsnvram(void *wl, char **argv);
extern int wlan_wpsctl(void *wl, char **argv);
extern int wlan_wpsmode(void *wl, char **argv);
extern int wlan_wpspinval(void *wl, char **argv);
extern int wlan_wpswsc(void *wl, char **argv);
extern int wlan_wpspara(void *wl, char **argv);
extern int wlan_wpsgetstat(void *wl, char **argv);
/*start change wps of minpenwei*/
extern int wps_set_confstat(void *wl, char **argv);
/*end change wps of minpenwei*/

#endif
extern int wlan_mbss(void *wl, char **argv);
extern int wlan_curetheraddr(void *wl, char **argv);
extern int wlan_isolate (void *wl, char **argv);
extern int wlan_maxassoc (void *wl, char **argv);
extern int wl_check(void *wl);
extern int wlanoption(char ***pargv, char **pifname, int *phelp);
extern int wl_get_dev_type(char *name, void *buf, int len);
extern int wlan_Qosctl(void *wl, char **argv);
extern int wlan_Stactl(void *wl, char **argv);

#ifdef SUPPORT_MIMO
extern int wlan_HT_GI(void *wl, char **argv);
extern int wlan_HT_AutoBA(void *wl, char **argv);
extern int wlan_HT_BW(void *wl, char **argv);
extern int wlan_HT_RDG(void *wl, char **argv);
extern int wlan_HT_BADecline(void *wl, char **argv);
extern int wlan_TxBurst(void *wl, char **argv);
#endif

extern int wlan_site_survey(void *wl, char **argv);
extern int WlanCmdWdsEnable(void *wl, char **argv);
extern int WlanWdsList(void *wl, char **argv);
extern int WlanWdsEncrypType(void *wl, char **argv);
extern int WlanWdsKey(void *wl, char **argv);
extern int WlanWds0Key(void *wl, char **argv);
extern int WlanWds1Key(void *wl, char **argv);
extern int WlanWds2Key(void *wl, char **argv);
extern int WlanWds3Key(void *wl, char **argv);
extern int WlanWdsDefaultKeyID(void *wl, char **argv);
extern int WlanWdsPhyMode(void *wl, char **argv);
extern int WlanIEEE8021X(void *wl, char **argv);
extern int WlanEAPifname(void *wl, char **argv);
extern int WlanPreAuthifname(void *wl, char **argv);
extern int WlanRADIUS_Server(void *wl, char **argv);
extern int WlanRADIUS_Port(void *wl, char **argv);
extern int WlanRADIUS_Key(void *wl, char **argv);
extern int Wlanown_ip_addr(void *wl, char **argv);
extern int Wlansession_timeout_interval(void *wl, char **argv);
extern int wlan_HtExtcha(void *wl, char **argv);

#endif /* __WLANCMDADAPTER_H__ */
