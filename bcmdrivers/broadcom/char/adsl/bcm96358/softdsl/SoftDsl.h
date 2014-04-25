/****************************************************************************
 *
 * SoftDsl.h 
 *
 *
 * Description:
 *	This file contains the exported interface for SoftDsl.c
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: SoftDsl.h,v 1.1.2.1 2009/11/19 06:39:17 l43571 Exp $
 *
 * $Log: SoftDsl.h,v $
 * Revision 1.1.2.1  2009/11/19 06:39:17  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:41:06  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:25  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:41  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:57  z45221
 * 合入人:
 * s48571
 *
 * 合入内容：
 * ADSL驱动更新版本至A2x020h
 * ADSLfirmware版本更换版本至a2pb023k/b2pb023k
 *
 * 修改文件列表：
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCore.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCore.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreFrame.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreFrame.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreMap.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslDrvVersion.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslFile.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslFile.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslSelfTest.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslSelfTest.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslCore.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslCore.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiag.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagLinux.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagNone.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagRtems.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmOs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/Makefile
 * /bcmdrivers/broadcom/char/adsl/impl1/adsl.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adsldrv.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_defs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_lmem.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_lmem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_sdram.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_sdram.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_defs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_lmem.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_lmem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_sdram.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_sdram.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslCoreDefs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMibObj.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMibOid.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslXfaceData.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/BlockUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/CircBuf.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/DslFramer.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/DslFramer.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/Flatten.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/Flatten.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcByte.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcFramer.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcFramer.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MathUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MipsAsm.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MiscUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftAtmVc.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDsl.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDsl.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDslFrame.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftModem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftModemTypes.h
 *
 * /bcmdrivers/broadcom/includeAdslMibDef.h
 * /bcmdrivers/broadcom/includeDiagDef.h
 * /bcmdrivers/broadcom/includeadsldrv.h
 * /bcmdrivers/broadcom/includebcmadsl.h
 *
 * /userapps/broadcom/adslctl/adslctl.c
 * /userapps/broadcom/adslctl/adsluser.c
 *
 * /userapps/broadcom/cfm/html/statsadsl.html
 * /userapps/broadcom/cfm/html/countrysupport/italy_vdf/info.html
 * /userapps/broadcom/cfm/inc/syscall.h
 * /userapps/broadcom/cfm/main/adslcfgmngr/adslctlapi.c
 * /userapps/broadcom/cfm/util/system/syscall.c
 * /userapps/broadcom/cfm/web/cgimain.c
 * /userapps/broadcom/cfm/web/cgimain.h
 *
 * Revision 1.337  2007/04/10 19:44:10  tonytran
 * Merged with release_A2pB024_rc4; Adsl Driver/Dsldiags need the head version+ changes in _A2pB024_rc4
 *
 * Revision 1.336  2007/04/09 20:46:21  mprahlad
 * add VDSL codepoints - initial rev
 *
 * Revision 1.335  2007/03/16 15:43:35  jboxho
 * Remove useless NEW_FIRE_PARAMS compilation flag
 *
 * Revision 1.334  2007/02/26 13:22:34  jboxho
 * Fire new parameters implementation (INPmax and minReXmtRate) (PR30818)
 *
 * Revision 1.333  2007/02/14 22:33:59  ilyas
 * Commented out DSLVARS_GLOBAL_REG
 *
 * Revision 1.332  2007/02/14 22:31:49  ilyas
 * Redefine gDslVars to use persistent  register. Added macros to 'remove' gDslVars - first function parameter
 *
 * Revision 1.331  2007/02/01 20:15:23  tonytran
 * Updated Fire counter define
 *
 * Revision 1.330  2007/01/25 01:25:43  tonytran
 * Added binary status output filtering; display Fire, AS and Bitswap counters in the State Summary Window
 *
 * Revision 1.329  2007/01/20 05:00:27  yongbing
 * PR30908: Create another control status when part of the band have margins below -1dB
 *
 * Revision 1.328  2007/01/19 21:15:37  ilyas
 * Merged vdsl-sjc tree
 *
 * Revision 1.327  2007/01/16 00:48:54  ovandewi
 * add BB pln reset indication
 *
 * Revision 1.326  2007/01/15 15:41:02  jboxho
 * Fire state reporting (PR30818)
 *
 * Revision 1.325  2006/12/22 14:46:41  jboxho
 * FIRE feature implementation: handshake, counters, block interleaving, new retransmit request format
 *
 * Revision 1.324  2006/12/08 17:46:13  ovandewi
 * bit is reserved regardless of FIRE being in the PHY
 *
 * Revision 1.323  2006/12/08 17:20:51  jboxho
 * DS Fire capability foreseen in demodCapabilities2 (PR30818)
 *
 * Revision 1.322  2006/10/27 00:16:51  ilyas
 * Added definitions for time based profiling
 *
 * Revision 1.321  2006/10/11 00:00:28  yongbing
 * PR 30854: Reduce SDRAM size by eliminating debug printf
 *
 * Revision 1.320  2006/09/04 14:25:21  ovandewi
 * means to retrain on min margin
 *
 * Revision 1.319  2006/07/06 20:59:41  mding
 * add status to update Gi only in the mib
 *
 * Revision 1.318  2006/06/15 17:10:48  ovandewi
 * add a bit to control 24 kbytes option
 *
 * Revision 1.317  2006/05/19 23:45:34  yongbing
 * Move some non-time-critial codes/data from Lmem to SDRAM for 24K, PR 30468
 *
 * Revision 1.316  2006/05/18 22:42:52  yongbing
 * Reverse last check in, uchar for D in G.DMT is sufficient, PR 30753
 *
 * Revision 1.315  2006/04/20 16:43:44  ovandewi
 * support D multiple of 32
 *
 * Revision 1.314  2006/04/18 06:56:39  ilyas
 * Added commands for setting ATM cells portID
 *
 * Revision 1.313  2006/04/15 11:53:20  ovandewi
 * annex M custom mode definition
 *
 * Revision 1.312  2006/03/24 18:01:38  ovandewi
 * add SES retrain defs
 *
 * Revision 1.311  2006/03/22 08:53:44  jboxho
 * Non-linear detection tool upgrade
 *
 * Revision 1.310  2006/02/24 19:38:55  ilyas
 * Added I432 command to chnage header handler callback for more efficient EOP workaround implementation
 *
 * Revision 1.309  2006/02/16 19:39:05  ilyas
 * Added AAL5 cell EOP monitoring
 *
 * Revision 1.308  2006/02/16 11:26:54  ovandewi
 * use status to report drop reason
 *
 * Revision 1.307  2006/02/06 21:11:10  ilyas
 * Implemented using  to address dslSlowVars (conditional compile)
 *
 * Revision 1.306  2006/02/04 02:32:48  linyin
 * Add AFE sample loss status
 *
 * Revision 1.305  2006/02/04 02:27:53  linyin
 * Reduce the possiblity to AFE sample lose
 *
 * Revision 1.304  2006/01/31 16:35:41  jboxho
 * PLN : Cycle optimization
 *
 * Revision 1.303  2006/01/30 17:03:46  jboxho
 * PLN: Cycle issue solved for short loop, Nitro off and interleave mode
 *
 * Revision 1.302  2006/01/26 16:21:36  jboxho
 * PLN tool: PLN message base re-initialized at each G994.1 start
 *
 * Revision 1.301  2006/01/16 15:15:06  jboxho
 * PLN Fix for Bin Table Initialization
 *
 * Revision 1.300  2006/01/13 17:14:06  ovandewi
 * PLN clean-up and add constants for diags compilation
 *
 * Revision 1.299  2006/01/12 00:12:35  ilyas
 * Defined macros and functions for checking SDRAM write completion and cache writeback
 *
 * Revision 1.298  2006/01/05 04:05:34  dadityan
 * PLN Status
 *
 * Revision 1.297  2006/01/04 16:38:00  jboxho
 * PLN command update: Impulse duration and inter-arrival bin definition tables & PLN status command
 *
 * Revision 1.296  2005/12/24 00:01:33  ilyas
 * Added definitions for PLN's programmable bins extensions
 *
 * Revision 1.295  2005/12/13 02:42:22  dslsjtst
 * PLN Control for Counters Added
 *
 * Revision 1.294  2005/11/22 18:45:22  ilyas
 * Added definitions for EC update in showtime (cold start), loop diagnostic (LD), SNR
 *
 * Revision 1.293  2005/11/02 18:17:20  ilyas
 * Added status for PLN message base (negotiated in NSIF)
 *
 * Revision 1.292  2005/10/31 18:28:53  ovandewi
 * add L2 disable flag + command if for pln margins
 *
 * Revision 1.291  2005/10/25 15:48:54  ovandewi
 * PLN bit mask for adaptive status
 *
 * Revision 1.290  2005/10/21 21:13:49  ilyas
 * Implemented queueing of background functions SoftDslBgScheduleXxx API
 *
 * Revision 1.289  2005/10/19 18:53:05  ilyas
 * Added margin level parameters (spec) to PLNStart command
 *
 * Revision 1.288  2005/10/19 06:05:50  ilyas
 * Enabled PLN and fix compile problems
 *
 * Revision 1.287  2005/10/13 22:14:53  ilyas
 * Pass G992p3 capability structure to DslDiags for printing
 *
 * Revision 1.286  2005/10/11 03:55:45  ilyas
 * Added more commands and statuses for PLN data
 *
 * Revision 1.285  2005/09/27 02:02:08  ilyas
 * Added commands and statuses for PLN measurements
 *
 * Revision 1.284  2005/09/25 05:27:37  ilyas
 * Added statuses for sample buffer reporting. Ifdef'ed out unused code
 *
 * Revision 1.283  2005/09/15 21:28:08  ilyas
 * Made unaligned DebugData work by marking offset and DMAing from word aligned address
 *
 * Revision 1.282  2005/07/29 02:50:39  kdu
 * PR30498: Report special value to driver for SNR margin in DELT mode
 *
 * Revision 1.281  2005/07/27 14:08:07  ovandewi
 * annexM PSD mask G992.[35] info
 *
 * Revision 1.280  2005/07/19 23:01:55  ovandewi
 * annex M EU 56 def
 *
 * Revision 1.279  2005/07/14 19:28:29  ilyas
 * Added macros to print data buffers and printf with string id
 *
 * Revision 1.278  2005/06/24 21:51:39  ilyas
 * Added definitions for debug data logging
 *
 * Revision 1.277  2005/06/10 17:37:25  yongbing
 * Add status definition for RDI recovery
 *
 * Revision 1.276  2005/06/08 00:42:48  ilyas
 * Merged G.994/T1.413 switch time change from 18c_branch
 *
 * Revision 1.275  2005/04/28 22:55:36  ilyas
 * Cleaned up kDslG992RunAnnexaP3ModeInAnnexaP5, kG992EnableAnnexM and kDslAtuChangeTxFilterRequest definitions
 *
 * Revision 1.274  2005/04/27 20:57:32  yongbing
 * Implement 32 frequency break points for TSSI, PR 30211
 *
 * Revision 1.273  2005/04/02 03:27:52  kdu
 * PR30236: Define kDslEnableRoundUpDSLoopAttn, this is shared with kDslCentilliumCRCWorkAroundEnabled.
 *
 * Revision 1.272  2005/04/01 21:56:39  ilyas
 * Added more test commands definitions
 *
 * Revision 1.271  2005/02/11 05:03:57  ilyas
 * Added support for DslOs
 *
 * Revision 1.270  2005/02/11 03:33:22  lke
 * Support 2X, 4X, and 8X spectrum in ANNEX_I DS
 *
 * Revision 1.269  2005/01/08 00:11:58  ilyas
 * Added definition for AnnexL status
 *
 * Revision 1.268  2004/12/18 00:52:35  mprahlad
 * Add Dig US Pwr cutback status
 *
 * Revision 1.267  2004/11/08 22:21:38  ytan
 * init swap state after retrain
 *
 * Revision 1.266  2004/11/05 21:16:50  ilyas
 * Added support for pwmSyncClock
 *
 * Revision 1.265  2004/10/28 20:05:17  gsyu
 * Fixed compilation errors for simulation targets
 *
 * Revision 1.264  2004/10/23 00:16:35  nino
 * Added kDslHardwareSetRcvAGC status to set absolute rcv agc gain.
 *
 * Revision 1.263  2004/10/22 21:21:06  ilyas
 * Fixed bit definition overlap in demodCapabilities
 *
 * Revision 1.262  2004/10/20 00:43:20  gsyu
 * Added constants to support new xmt sample buffer control scheme
 *
 * Revision 1.261  2004/10/12 01:09:28  nino
 * Remove kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSyncClkFreq
 * status definitions. Add kDslEnablePwmSyncClk and kDslSetPwmSyncClkFreq
 * command definitions.
 *
 * Revision 1.260  2004/10/11 20:21:26  nino
 * Added kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSynClkFreq hardware statuses.
 *
 * Revision 1.259  2004/10/07 19:17:29  nino
 * Added kDslHardwareGetRcvAGC status.
 *
 * Revision 1.258  2004/10/02 00:17:14  nino
 * Added kDslHardwareAGCSetPga2 and kDslSetPilotEyeDisplay status definitions.
 *
 * Revision 1.257  2004/08/27 01:00:30  mprahlad
 *
 * Keep kDslAtuChangeTxFilterRequest defined by default so ADSL1 only targets can
 * build
 *
 * Revision 1.256  2004/08/20 19:00:34  ilyas
 * Added power management code for 2+
 *
 * Revision 1.255  2004/08/17 23:18:25  kdu
 * Merged interop changes for TDC lab from a023e9.
 *
 * Revision 1.254  2004/07/22 00:56:03  yongbing
 * Add ADSL2 Annex B modulation definition
 *
 * Revision 1.253  2004/07/16 22:23:28  nino
 * - Defined macros to extract subcarrier and supported set information
 *   for tssi. Subcarrier and suported set indicator is packed into
 *   dsSubcarrier index array.
 *
 * Revision 1.252  2004/07/01 00:11:22  nino
 * Added preliminary code for debugDataHandler (inside of #if DEBUG_DATA_HANDLER).
 *
 * Revision 1.251  2004/06/24 03:08:39  ilyas
 * Added GFC mapping control for ATM bonding
 *
 * Revision 1.250  2004/06/23 00:03:20  khp
 * - shorten self test result register length to 1 (satisfied requirement
 *   at DT, no known requirement anywhere else)
 *
 * Revision 1.249  2004/06/15 20:18:33  ilyas
 * Made D uchar again for compatibility with older ADSl drivers that use this structure. ADSL driver will rely on G992p3 parameters for large D
 *
 * Revision 1.248  2004/06/12 00:26:03  gsyu
 * Added constants for AnnexM
 *
 * Revision 1.247  2004/06/10 18:53:24  yjchen
 * add large D support
 *
 * Revision 1.246  2004/06/04 01:55:00  linyin
 * Add a constant for SRA enable/disable
 *
 * Revision 1.245  2004/05/19 23:22:23  linyin
 * Support L2
 *
 * Revision 1.244  2004/05/15 03:04:58  ilyas
 * Added L3 test definition
 *
 * Revision 1.243  2004/05/14 03:04:38  ilyas
 * Fixed structure name typo
 *
 * Revision 1.242  2004/05/14 02:01:01  ilyas
 * Fixed structure name typo
 *
 * Revision 1.241  2004/05/14 01:21:49  nino
 * Added kDslSignalAttenuation, kDslAttainableNetDataRate kDslHLinScale constant definitions.
 *
 * Revision 1.240  2004/05/13 19:07:58  ilyas
 * Added new statuses for ADSL2
 *
 * Revision 1.239  2004/05/01 01:09:51  ilyas
 * Added power management command and statuses
 *
 * Revision 1.238  2004/04/23 22:50:38  ilyas
 * Implemented double buffering to ensure G.997 HDLC frame (OvhMsg) is continuous
 *
 * Revision 1.237  2004/03/31 18:57:39  ilyas
 * Added drop on data error capability control
 *
 * Revision 1.236  2004/03/30 03:11:30  ilyas
 * Added #ifdef for CFE build
 *
 * Revision 1.235  2004/03/29 23:06:39  ilyas
 * Added status for BG table update
 *
 * Revision 1.234  2004/03/17 02:49:49  ilyas
 * Turn off ATM bit reversal for Alcatel DSLAM only
 *
 * Revision 1.233  2004/03/11 03:09:48  mprahlad
 * Add test mode for afeloopback test
 *
 * Revision 1.232  2004/03/10 23:15:53  ilyas
 * Added ETSI modem support
 *
 * Revision 1.231  2004/03/04 19:28:14  linyin
 * Support adsl2plus
 *
 * Revision 1.230  2004/02/28 00:06:21  ilyas
 * Added OLR message definitions for ADSL2+
 *
 * Revision 1.229  2004/02/13 03:21:15  mprahlad
 * define kDslAturHwAgcMaxGain correctly for 6348
 *
 * Revision 1.228  2004/02/09 05:06:17  yongbing
 * Add ADSL2 bit swap function
 *
 * Revision 1.227  2004/02/04 02:08:19  linyin
 * remove the redefined kG992p5AnnexA
 *
 * Revision 1.226  2004/02/04 01:41:48  linyin
 * Add some variables for G992P5
 *
 * Revision 1.225  2004/02/03 19:12:22  gsyu
 * Added a dedicate structure and constants for G992P5
 *
 * Revision 1.224  2004/01/24 01:18:34  ytan
 * add multi-section swapping flag
 *
 * Revision 1.223  2004/01/17 00:21:48  ilyas
 * Added commands and statuses for OLR
 *
 * Revision 1.222  2004/01/13 19:12:37  gsyu
 * Added more constants for Double upstream
 *
 * Revision 1.221  2003/12/23 21:19:04  mprahlad
 * Define BCM6348_TEMP_MOVE_TO_LMEM to FAST_TEXT for 6348 targets - this is for
 * ADSL2/AnnexA multimode builds - move a few functions to Lmem for now to avoid
 * changes for swap on 6348.
 *
 * Revision 1.220  2003/12/19 21:21:53  ilyas
 * Added dying gasp support for ADSL2
 *
 * Revision 1.219  2003/12/05 02:09:51  mprahlad
 * Leave the AnalogEC defs in - saves ifdef-ing all uses of these defines.
 * Include Bcm6345_To_Bcm6348.h - to be able to pick up macros for the
 * transition
 *
 * Revision 1.218  2003/12/04 02:10:58  linyin
 * Redefine some constants for supporting different pilot and TTR
 *
 * Revision 1.217  2003/12/03 02:24:39  gsyu
 * Reverse previous check in for Double Upstream demo
 *
 * Revision 1.215  2003/11/20 00:58:47  yongbing
 * Merge ADSL2 functionalities into Annex A branch
 *
 * Revision 1.214  2003/11/06 00:35:06  nino
 * Added kDslWriteAfeRegCmd and kDslReadAfeRegCmd commands.
 *
 * Revision 1.213  2003/11/05 21:04:23  ilyas
 * Added more codes for LOG data
 *
 * Revision 1.212  2003/10/22 00:51:52  yjchen
 * define constant for quiet line noise
 *
 * Revision 1.211  2003/10/20 22:08:57  nino
 * Added kDslSetRcvGainCmd and kDslBypassRcvHpfCmd debug commands.
 *
 * Revision 1.210  2003/10/18 00:04:59  yjchen
 * define constants for G992P3 diagnostic mode channel response
 *
 * Revision 1.209  2003/10/17 22:41:29  yongbing
 * Add INP message support
 *
 * Revision 1.208  2003/10/16 00:06:09  uid1249
 * Moved G.994 definitions from G.994p1MainTypes.h
 *
 * Revision 1.207  2003/10/15 20:45:11  linyin
 * Add some constants for support Revision 2
 *
 * Revision 1.206  2003/10/14 22:04:02  ilyas
 * Added Nino's AFE statuses for 6348
 *
 * Revision 1.205  2003/10/10 18:49:26  gsyu
 * Added test modes to workaround the clock domain crossing bug, PR18038
 *
 * Revision 1.204  2003/09/30 19:27:46  mprahlad
 * ifdef AnalogEC definies with #ifndef BCM6348_SRC
 *
 * Revision 1.203  2003/09/26 19:36:34  linyin
 * Add annexi constant and vars
 *
 * Revision 1.202  2003/09/25 20:16:13  yjchen
 * remove featureNTR definition
 *
 * Revision 1.201  2003/09/08 20:29:51  ilyas
 * Added test commands for chip regression tests
 *
 * Revision 1.200  2003/08/26 00:58:14  ilyas
 * Added I432 reset command (for header compression)
 * Fixed SoftDsl time (for I432 header compression)
 *
 * Revision 1.199  2003/08/26 00:37:29  ilyas
 * #ifdef'ed DslFrameFunctions in dslCommand structure to save space
 *
 * Revision 1.198  2003/08/22 22:45:00  liang
 * Change the NF field in G992CodingParams from uchar to ushort to support K=256 (dataRate=255*32kbps) in fast path.
 *
 * Revision 1.197  2003/08/21 21:19:05  ilyas
 * Changed dataPumpCapabilities structure for G992P3
 *
 * Revision 1.196  2003/08/12 22:44:28  khp
 * - for Haixiang: added kDslTestMarginTweak command and marginTweakSpec
 *
 * Revision 1.195  2003/07/24 17:28:16  ovandewi
 * added Tx filter change request code
 *
 * Revision 1.194  2003/07/24 15:48:55  yongbing
 * Reduce TSSI buffer size to avoid crash at the beginning of G.994.1. Need to find out why
 *
 * Revision 1.193  2003/07/19 07:11:47  nino
 * Revert back to version 1.191.
 *
 * Revision 1.191  2003/07/17 21:25:25  yongbing
 * Add support for READSL2 and TSSI
 *
 * Revision 1.190  2003/07/14 19:42:33  yjchen
 * add constants for G992P3
 *
 * Revision 1.189  2003/07/10 23:07:11  liang
 * Add demodCapability bit to minimize showtime ATUC xmt power through b&g table.
 *
 * Revision 1.188  2003/07/08 22:18:50  liang
 * Added demodCapability bit for G.994.1 Annex A multimode operation.
 *
 * Revision 1.187  2003/07/07 23:24:43  ilyas
 * Added G.dmt.bis definitions
 *
 * Revision 1.186  2003/06/25 02:44:02  liang
 * Added demod capability bit kDslUE9000ADI918FECFixEnabled.
 * Added back kDslHWEnableAnalogECUpdate & kDslHWEnableAnalogEC for backward compatibility (annex A).
 *
 * Revision 1.185  2003/06/18 01:39:19  ilyas
 * Added AFE test commands. Add #defines for driver's builds
 *
 * Revision 1.184  2003/06/06 23:58:09  ilyas
 * Added command and status for standalone AFE tests
 *
 * Revision 1.183  2003/05/29 21:09:32  nino
 * - kDslHWEnableAnalogECUpdate define replaced with kDslHWSetDigitalEcUpdateMode
 * - kDslHWEnableAnalogEC       define replaced with kDslHWDisableDigitalECUpdate
 *
 * Revision 1.182  2003/04/15 22:08:15  liang
 * Changed one of the demodCapability bit name from last checkin.
 *
 * Revision 1.181  2003/04/13 19:25:54  liang
 * Added three more demodCapability bits.
 *
 * Revision 1.180  2003/04/02 02:09:17  liang
 * Added demodCapability bit for ADI low rate option fix disable.
 *
 * Revision 1.179  2003/03/18 18:22:06  yongbing
 * Use 32 tap TEQ for Annex I
 *
 * Revision 1.178  2003/03/06 00:58:07  ilyas
 * Added SetStausBuffer command
 *
 * Revision 1.177  2003/02/25 00:46:26  ilyas
 * Added T1.413 EOC vendor ID
 *
 * Revision 1.176  2003/02/21 23:30:54  ilyas
 * Added Xmtgain command framing mode status and T1413VendorId parameters
 *
 * Revision 1.175  2003/02/07 22:13:55  liang
 * Add demodCapabilities bits for sub-sample alignment and higher T1.413 level (used internally only).
 *
 * Revision 1.174  2003/01/23 02:54:07  liang
 * Added demod capability bit for bitswap enable.
 *
 * Revision 1.173  2002/12/13 18:36:33  yongbing
 * Add support for G.992.2 Annex C
 *
 * Revision 1.172  2002/12/10 23:27:12  ilyas
 * Extended dslException parameter structure to allow printout from DslDiags
 *
 * Revision 1.171  2002/12/06 02:10:19  liang
 * Moved the T1.413 RAck1/RAck2 switching variables to connection setup structure.
 * Added/Modified the training progress codes for T1.413 RAck1/RAck2 and upstream 2x IFFT disable.
 *
 * Revision 1.170  2002/11/11 00:20:05  liang
 * Add demod capability constant for internally disabling upstream 2x IFFT in T1.413 mode.
 *
 * Revision 1.169  2002/11/06 03:46:19  liang
 * Add training progress code for upstream 2x IFFT disable.
 *
 * Revision 1.168  2002/11/01 01:41:06  ilyas
 * Added flags for Centillium 4103 workarround
 *
 * Revision 1.167  2002/10/26 01:26:11  gsyu
 * Move SoftDslLineHandler from SDRAM to LMEM
 *
 * Revision 1.166  2002/10/20 18:56:20  khp
 * - for linyin
 *   - #ifdef NEC_NSIF_WORKAROUND:
 *     - add macros to extract NSIF status and fail counter vars
 *
 * Revision 1.165  2002/10/14 05:24:35  liang
 * Add training status code to request alternate xmt filter (for Samsung 6-port ADI918 DSLAMs) to meet KT 2km spec.
 *
 * Revision 1.164  2002/10/08 21:44:50  ilyas
 * Fixed EOC stuffing byte to indicate "no synchronization" action
 *
 * Revision 1.163  2002/10/03 19:34:24  ilyas
 * Added size for EOC serial number register
 *
 * Revision 1.162  2002/09/28 02:42:27  yongbing
 * Add retrain in T1.413 with R-Ack1 tone
 *
 * Revision 1.161  2002/09/28 01:23:35  gsyu
 * Reverse us2xifft change so that we can install new us2xifft on the tree
 *
 * Revision 1.160  2002/09/26 23:30:48  yongbing
 * Add synch symbol detection in Showtime
 *
 * Revision 1.159  2002/09/20 23:47:52  khp
 * - for gsyu: enable 2X IFFT for Annex A (XMT_FFT_SIZE_2X)
 *
 * Revision 1.158  2002/09/14 03:26:39  ilyas
 * Changed far-end RDI reporting
 *
 * Revision 1.157  2002/09/13 21:10:54  ilyas
 * Added reporting of remote modem LOS and RDI.
 * Moved G992CodingParams definition to SoftDsl.h
 *
 * Revision 1.156  2002/09/12 21:07:19  ilyas
 * Added HEC, OCD and LCD counters
 *
 * Revision 1.155  2002/09/09 21:31:30  linyin
 * Add two constant to support long reach
 *
 * Revision 1.154  2002/09/07 01:31:51  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.153  2002/09/04 22:36:14  mprahlad
 * defines for non standard info added
 *
 * Revision 1.152  2002/08/02 21:59:09  liang
 * Enable G.992.2 carrierInfo in capabitilities when G.992.1 annex A is used for G.992.2.
 *
 * Revision 1.151  2002/07/29 20:01:03  ilyas
 * Added command for Atm VC map table change
 *
 * Revision 1.150  2002/07/18 22:30:47  liang
 * Add xmt power and power cutback related constants.
 *
 * Revision 1.149  2002/07/11 01:30:58  ilyas
 * Changed status for ShowtimeMargin reporting
 *
 * Revision 1.148  2002/07/09 19:19:09  ilyas
 * Added status parameters for ShowtimeSNRMargin info and command to filter
 * out SNR margin data
 *
 * Revision 1.147  2002/06/27 21:50:24  liang
 * Added test command related demodCapabilities bits.
 *
 * Revision 1.146  2002/06/26 21:29:00  liang
 * Added dsl test cmd structure and showtime margin connection info status.
 *
 * Revision 1.145  2002/06/15 05:15:51  ilyas
 * Added definitions for Ping, Dying Gasp and other test commands
 *
 * Revision 1.144  2002/05/30 19:55:15  ilyas
 * Added status for ADSL PHY MIPS exception
 * Changed conflicting definition for higher rates (S=1/2)
 *
 * Revision 1.143  2002/05/21 23:41:07  yongbing
 * First check-in of Annex C S=1/2 codes
 *
 * Revision 1.142  2002/04/29 22:25:09  georgep
 * Merge from branch annexC_demo - add status message constants
 *
 * Revision 1.141  2002/04/18 19:00:35  ilyas
 * Added include file for builds in CommEngine environment
 *
 * Revision 1.140  2002/04/18 00:18:36  yongbing
 * Add detailed timeout error messages
 *
 * Revision 1.139  2002/04/02 10:03:18  ilyas
 * Merged BERT from AnnexA branch
 *
 * Revision 1.138  2002/03/26 01:42:29  ilyas
 * Added timeout message constants for annex C
 *
 * Revision 1.137  2002/03/22 19:39:22  yongbing
 * Modify for co-exist of G994P1 and T1P413
 *
 * Revision 1.136  2002/03/22 01:19:40  ilyas
 * Add status message constants for total FEXT Bits, NEXT bits
 *
 * Revision 1.135  2002/03/10 22:32:24  liang
 * Added report constants for LOS recovery and timing tone index.
 *
 * Revision 1.134  2002/03/07 22:06:32  georgep
 * Replace ifdef G992P1 with G992P1_ANNEX_A for annex A variables
 *
 * Revision 1.133  2002/02/16 01:08:18  georgep
 * Add log constant for showtime mse
 *
 * Revision 1.132  2002/02/08 04:36:27  ilyas
 * Added commands for LOG file and fixed Idle mode pointer update
 *
 * Revision 1.131  2002/01/24 20:21:30  georgep
 * Add logging defines, remove fast retrain defines
 *
 * Revision 1.130  2002/01/19 23:59:17  ilyas
 * Added support for LOG and eye data to ADSL core target
 *
 * Revision 1.129  2002/01/16 23:43:54  liang
 * Remove the carriage return character from last checkin.
 *
 * Revision 1.128  2002/01/15 22:27:13  ilyas
 * Added command for ADSL loopback
 *
 * Revision 1.127  2002/01/10 07:18:22  ilyas
 * Added status for printf (mainly for ADSL core debugging)
 *
 * Revision 1.126  2001/12/21 22:45:34  ilyas
 * Added support for ADSL MIB data object
 *
 * Revision 1.125  2001/12/13 02:24:22  ilyas
 * Added G997 (Clear EOC and G997 framer) support
 *
 * Revision 1.124  2001/11/30 05:56:31  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.123  2001/11/15 19:01:07  yongbing
 * Modify only T1.413 part to the top of tree based on AnnexBDevelopment branch
 *
 * Revision 1.122  2001/10/19 00:12:07  ilyas
 * Added support for frame oriented (no ATM) data link layer
 *
 * Revision 1.121  2001/10/09 22:35:13  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.105.2.20  2001/11/27 02:32:03  liang
 * Combine vendor ID, serial #, and version number into SoftModemVersionNumber.c.
 *
 * Revision 1.105.2.19  2001/11/21 01:29:14  georgep
 * Add a status message define for annexC
 *
 * Revision 1.105.2.18  2001/11/08 23:26:28  yongbing
 * Add carrier selection function for Annex A and B
 *
 * Revision 1.105.2.17  2001/11/07 22:55:30  liang
 * Report G992 rcv msg CRC error as what it is instead of time out.
 *
 * Revision 1.105.2.16  2001/11/05 19:56:21  liang
 * Add DC offset info code.
 *
 * Revision 1.105.2.15  2001/10/16 00:47:16  yongbing
 * Add return-to-T1p413 starting point if in error
 *
 * Revision 1.105.2.14  2001/10/15 23:14:01  yjchen
 * remove ADSL_SINGLE_SYMBOL_BLOCK
 *
 * Revision 1.105.2.13  2001/10/12 18:07:16  yongbing
 * Add support for T1.413
 *
 * Revision 1.105.2.12  2001/10/04 00:23:52  liang
 * Add connection info constants for TEQ coef and PSD.
 *
 * Revision 1.105.2.11  2001/10/03 01:44:01  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.105.2.10  2001/09/28 22:10:04  liang
 * Add G994 exchange message status reports.
 *
 * Revision 1.105.2.9  2001/09/26 18:08:21  georgep
 * Send status error message in case features field is not setup properly
 *
 * Revision 1.105.2.8  2001/09/05 01:58:13  georgep
 * Added status message for annexC measured delay
 *
 * Revision 1.105.2.7  2001/08/29 00:37:52  georgep
 * Add log constants for annexC
 *
 * Revision 1.105.2.6  2001/08/18 00:01:34  georgep
 * Add constants for annexC
 *
 * Revision 1.105.2.5  2001/08/08 17:33:28  yongbing
 * Merge with tag SoftDsl_2_17
 *
 * Revision 1.120  2001/08/29 02:56:01  ilyas
 * Added tests for flattening/unflatenning command and statuses (dual mode)
 *
 * Revision 1.119  2001/08/28 03:26:32  ilyas
 * Added support for running host and adsl core parts separately ("dual" mode)
 *
 * Revision 1.118  2001/08/16 02:16:10  khp
 * - mark functions with FAST_TEXT to reduce cycle counts for QPROC targets
 *   (replaces use of LMEM_INSN)
 *
 * Revision 1.117  2001/06/18 20:06:35  ilyas
 * Added forward declaration of dslCommandStruc to avoid gcc warnings
 *
 * Revision 1.116  2001/06/18 19:49:36  ilyas
 * Changes to include support for HOST_ONLY mode
 *
 * Revision 1.115  2001/06/01 22:00:33  ilyas
 * Changed ATM PHY interface to accomodate UTOPIA needs
 *
 * Revision 1.114  2001/05/16 06:22:24  liang
 * Added status reports for xmt & rcv prefix enable position.
 *
 * Revision 1.113  2001/05/02 20:34:32  georgep
 * Added log constants for snr1 calculation
 *
 * Revision 1.112  2001/04/25 01:20:11  ilyas
 *
 * Don't use DSL frame functions if ATM_LAYER is not defined
 *
 * Revision 1.111  2001/04/17 21:13:00  georgep
 * Define status constant kDslHWSetDigitalEcUpdateShift
 *
 * Revision 1.110  2001/04/16 23:38:36  georgep
 * Add HW AGC constants for ATUR
 *
 * Revision 1.109  2001/04/06 23:44:53  georgep
 * Added status constant for setting up digitalEcGainShift
 *
 * Revision 1.108  2001/03/29 05:58:34  liang
 * Replaced the Aware compatibility codes with automatic detection codes.
 *
 * Revision 1.107  2001/03/25 06:11:22  liang
 * Combined separate loop attenuation status for ATUR & ATUC into one status.
 * Replace separate hardware AGC info status for ATUR & ATUC into hardware AGC
 * request status and hardware AGC obtained status.
 * Use store AGC command to save hardware AGC value instead of returning value
 * from status report.
 *
 * Revision 1.106  2001/03/24 00:43:22  liang
 * Report more checksum results (NumOfCalls, txSignal, rxSignal & eyeData).
 *
 * Revision 1.105  2001/03/16 23:57:31  georgep
 * Added more loop attenuation reporting status constants
 *
 * Revision 1.104  2001/03/15 00:22:07  liang
 * Back to version 1.101.
 *
 * Revision 1.103  2001/03/15 00:03:44  yjchen
 * use kDslATURHardwareAGCInfo for AltoE14 AGC as well
 *
 * Revision 1.102  2001/03/14 23:10:56  yjchen
 * add defns for AltoE14 AGC
 *
 * Revision 1.101  2001/03/08 23:31:34  georgep
 * Added R, S, D, coding parameters to dslDataPumpCapabilities
 *
 * Revision 1.100  2001/02/10 03:03:09  ilyas
 * Added one more DslFrame function
 *
 * Revision 1.99  2001/02/09 01:55:27  ilyas
 * Added status codes and macros to support printing of AAL packets
 *
 * Revision 1.98  2001/01/30 23:28:10  georgep
 * Added kDslDspControlStatus for handling changes to dsp params
 *
 * Revision 1.97  2001/01/12 01:17:18  georgep
 * Added bit in demodCapabilities for analog echo cancellor
 *
 * Revision 1.96  2001/01/04 05:51:03  ilyas
 * Added more dslStatuses
 *
 * Revision 1.95  2000/12/21 05:46:07  ilyas
 * Added name for struct _dslFrame
 *
 * Revision 1.94  2000/12/13 22:04:39  liang
 * Add Reed-Solomon coding enable bit in demodCapabilities.
 *
 * Revision 1.93  2000/11/29 20:42:02  liang
 * Add defines for SNR & max achivable rate status and DEC enable demodCapabilities bit.
 *
 * Revision 1.92  2000/09/22 21:55:13  ilyas
 * Added support for DSL + Atm physical layer only (I.432) simulations
 *
 * Revision 1.91  2000/09/10 09:20:53  lkaplan
 * Improve interface for sending Eoc messages
 *
 * Revision 1.90  2000/09/08 19:37:58  lkaplan
 * Added code for handling EOC messages
 *
 * Revision 1.89  2000/09/07 23:02:27  georgep
 * Add HarwareAGC Bit to demod Capabilities
 *
 * Revision 1.88  2000/09/01 00:57:34  georgep
 * Added Hardware AGC status defines
 *
 * Revision 1.87  2000/08/31 19:04:26  liang
 * Added external reference for stack size requirement test functions.
 *
 * Revision 1.86  2000/08/24 23:16:46  liang
 * Increased sample block size for noBlock.
 *
 * Revision 1.85  2000/08/23 18:34:39  ilyas
 * Added XxxVcConfigure function
 *
 * Revision 1.84  2000/08/05 00:25:04  georgep
 * Redefine sampling freq constants
 *
 * Revision 1.83  2000/08/03 14:04:00  liang
 * Add hardware time tracking clock error reset code.
 *
 * Revision 1.82  2000/07/23 20:52:52  ilyas
 * Added xxxFrameBufSetAddress() function for ATM framer layers
 * Rearranged linkLayer functions in one structure which is passed as a
 * parameter to xxxLinkLayerInit() function to be set there
 *
 * Revision 1.81  2000/07/18 20:03:24  ilyas
 * Changed DslFrame functions definitions to macros,
 * Removed gDslVars from their parameter list
 *
 * Revision 1.80  2000/07/17 21:08:15  lkaplan
 * removed global pointer
 *
 * Revision 1.79  2000/06/21 20:38:44  georgep
 * Added bit to demodCapabilities for HW_TIME_TRACKING
 *
 * Revision 1.78  2000/06/19 19:57:55  georgep
 * Added constants for logging of HWResampler data
 *
 * Revision 1.77  2000/06/02 18:57:21  ilyas
 * Added support for DSL buffers consisting of many ATM cells
 *
 * Revision 1.76  2000/05/27 02:19:28  liang
 * G992MonitorParams structure is moved here, and Tx/Rx data handler type definitions changed.
 *
 * Revision 1.75  2000/05/15 18:17:21  liang
 * Added statuses for sent and received frames
 *
 * Revision 1.74  2000/05/14 01:56:38  ilyas
 * Added ATM cell printouts
 *
 * Revision 1.73  2000/05/09 23:00:26  ilyas
 * Added ATM status messages, ATM timer, Tx frames flush on timeout
 * Fixed a bug - adding flushed Tx frames to the list of free Rx frames
 *
 * Revision 1.72  2000/05/03 18:01:18  georgep
 * Removed old function declarations for Eoc/Aoc
 *
 * Revision 1.71  2000/05/03 03:57:04  ilyas
 * Added LOG file support for writing ATM data
 *
 * Revision 1.70  2000/05/02 00:04:36  liang
 * Add showtime monitoring and message exchange info constants.
 *
 * Revision 1.69  2000/04/28 23:34:20  yongbing
 * Add constants for reporting error events in performance monitoring
 *
 * Revision 1.68  2000/04/21 23:09:04  liang
 * Added G992 time out training progress constant.
 *
 * Revision 1.67  2000/04/19 00:31:47  ilyas
 * Added global SoftDsl functions for Vc, added OOB info functions
 *
 * Revision 1.66  2000/04/18 00:45:31  yongbing
 * Add G.DMT new frame structure, define G992P1_NEWFRAME to enable, need ATM layer to work
 *
 * Revision 1.65  2000/04/15 01:48:34  georgep
 * Added T1p413 status constants
 *
 * Revision 1.64  2000/04/13 08:36:22  yura
 * Added SoftDslSetRefData, SoftDslGetRefData functions
 *
 * Revision 1.63  2000/04/13 05:42:35  georgep
 * Added constant for T1p413
 *
 * Revision 1.62  2000/04/05 21:49:54  liang
 * minor change.
 *
 * Revision 1.61  2000/04/04 04:16:06  liang
 * Merged with SoftDsl_0_03 from old tree.
 *
 * Revision 1.65  2000/04/04 01:47:21  ilyas
 * Implemented abstract dslFrame and dslFrameBuffer objects
 *
 * Revision 1.64  2000/04/01 08:12:10  yura
 * Added preliminary revision of the SoftDsl driver architecture
 *
 * Revision 1.63  2000/04/01 02:55:33  georgep
 * New defines for G992p2Profile Structure
 *
 * Revision 1.62  2000/04/01 00:50:36  yongbing
 * Add initial version of new frame structure for full-rate
 *
 * Revision 1.61  2000/03/24 03:30:45  georgep
 * Define new constant kDslUpstreamSamplingFreq
 *
 * Revision 1.60  2000/03/23 19:51:30  georgep
 * Define new features bits for G992p1
 *
 * Revision 1.59  2000/03/18 01:28:41  georgep
 * Changed connectionSetup to include G992p1 Capabilities
 *
 * Revision 1.58  2000/02/29 01:40:03  georgep
 * Changed modulationtype defines to be the same as SPAR1 in G994p1
 *
 * Revision 1.57  1999/11/19 01:03:19  george
 * Use Block Size 256 for single symbol Mode
 *
 * Revision 1.56  1999/11/18 02:37:43  george
 * Porting to 16Bit
 *
 * Revision 1.55  1999/11/12 02:12:55  george
 * Added status constant for reporting of profile channel matching calculation
 *
 * Revision 1.54  1999/11/11 19:19:42  george
 * Porting to 16Bit Compiler
 *
 * Revision 1.53  1999/11/05 01:27:06  liang
 * Add recovery-from-inpulse-noise progress report.
 *
 * Revision 1.52  1999/11/02 02:06:27  george
 * Added SNRMargin training status value
 *
 * Revision 1.51  1999/10/27 23:02:03  wan
 * Add G.994.1 setup in dslConnectionSetupStruct for setting up Initiation side
 *
 * Revision 1.50  1999/10/25 21:55:36  liang
 * Renamed the constant for FEQ output error.
 *
 * Revision 1.49  1999/10/23 02:20:55  george
 * Add debug data codes
 *
 * Revision 1.48  1999/10/19 23:59:06  liang
 * Change line handler interface to work with nonsymmetric sampling freq.
 *
 * Revision 1.47  1999/10/09 01:38:04  george
 * Define maxProfileNumber
 *
 * Revision 1.46  1999/10/07 23:30:51  wan
 * Add G.994.1 Tone and Fast Retrain Recov detections in G.992p2 SHOWTIME and Fast Retrain
 *
 * Revision 1.45  1999/10/06 13:59:27  liang
 * Escape to G994.1 should be done through status instead of command.
 *
 * Revision 1.44  1999/10/06 02:01:28  george
 * Add kDslReturnToG994p1Cmd
 *
 * Revision 1.43  1999/09/30 19:29:58  george
 * Add reporting constant for Fast Retrain
 *
 * Revision 1.42  1999/09/16 23:41:56  liang
 * Added command for host forced retrain.
 *
 * Revision 1.41  1999/08/20 00:47:25  wan
 * Add constants for Fast Retrain progress status
 *
 * Revision 1.40  1999/08/16 18:06:01  wan
 * Add more reporting constants for Fast Retrain
 *
 * Revision 1.39  1999/08/12 00:18:10  wan
 * Add several Fast Retrain Status constants
 *
 * Revision 1.38  1999/08/10 18:25:38  george
 * Define constants used for Fast Retrain
 *
 * Revision 1.37  1999/07/31 01:47:43  george
 * Add status constants for eoc/aoc
 *
 * Revision 1.36  1999/07/27 18:19:52  george
 * declare aoc/eoc functions
 *
 * Revision 1.35  1999/07/19 22:44:47  george
 * Add constants for G994p1 Message Exchange
 *
 * Revision 1.34  1999/07/16 02:03:03  liang
 * Modified Dsl link layer command spec structure.
 *
 * Revision 1.33  1999/07/14 22:53:16  george
 * Add Constants for G994p1
 *
 * Revision 1.32  1999/07/13 00:02:26  liang
 * Added more feature bits.
 *
 * Revision 1.31  1999/07/09 01:58:14  wan
 * Added more constants G.994.1 testing reports
 *
 * Revision 1.30  1999/07/07 23:51:04  liang
 * Added rcv power and loop attenuation reports.
 *
 * Revision 1.29  1999/07/06 21:32:01  liang
 * Added some aux. feature bits, and field performanceMargin was changed to noiseMargin in Capabilities.
 *
 * Revision 1.28  1999/07/03 01:40:17  liang
 * Redefined dsl command parameter list and added connection setup struct.
 *
 * Revision 1.27  1999/07/02 00:41:18  liang
 * Add bit and gain logging as well as rcv carrier range status.
 *
 * Revision 1.26  1999/06/25 21:37:10  wan
 * Work in progress for G994.1.
 *
 * Revision 1.25  1999/06/16 00:54:36  liang
 * Added Tx/Rx SHOWTIME active training progress codes.
 *
 * Revision 1.24  1999/06/11 21:59:37  wan
 * Added G994.1 fail status constant.
 *
 * Revision 1.23  1999/06/11 21:29:01  liang
 * Constants for C/R-Msgs was changed to C/R-Msg.
 *
 * Revision 1.22  1999/06/08 02:49:42  liang
 * Added SNR data logging.
 *
 * Revision 1.21  1999/06/07 21:05:08  liang
 * Added more training status values.
 *
 * Revision 1.20  1999/05/22 02:18:26  liang
 * More constant defines.
 *
 * Revision 1.19  1999/05/14 22:49:35  liang
 * Added more status codes and debug data codes.
 *
 * Revision 1.18  1999/04/12 22:41:39  liang
 * Work in progress.
 *
 * Revision 1.17  1999/04/01 20:28:07  liang
 * Added RReverb detect event status.
 *
 * Revision 1.16  1999/03/26 03:29:54  liang
 * Add DSL debug data constants.
 *
 * Revision 1.15  1999/03/08 21:58:00  liang
 * Added more constant definitions.
 *
 * Revision 1.14  1999/03/02 01:49:36  liang
 * Added more connection info codes.
 *
 * Revision 1.13  1999/03/02 00:25:55  liang
 * Added DSL tx and rx data handler type definitions.
 *
 * Revision 1.12  1999/02/27 01:16:55  liang
 * Increase allowable static memory size to a VERY large number for now.
 *
 * Revision 1.11  1999/02/25 00:24:06  liang
 * Increased symbol block size to 16.
 *
 * Revision 1.10  1999/02/23 22:03:26  liang
 * Increased maximal static memory size allowed.
 *
 * Revision 1.9  1999/02/17 02:39:21  ilyas
 * Changes for NDIS
 *
 * Revision 1.8  1999/02/11 22:44:30  ilyas
 * More definitions for ATM
 *
 * Revision 1.7  1999/02/10 01:56:38  liang
 * Added hooks for G994.1 and G992.2.
 *
 *
 *****************************************************************************/

#ifndef	SoftDslHeader
#define	SoftDslHeader

/* #define DSLVARS_GLOBAL_REG */

/* for builds in Linux/VxWorks CommEngine environment */
#if (defined(__KERNEL__) && !defined(LINUX_DRIVER)) || defined(VXWORKS) || defined(_WIN32_WCE) || defined(TARG_OS_RTEMS) || defined(_CFE_)
#include "AdslCoreDefs.h"
#else
#include "Bcm6345_To_Bcm6348.h"		/* File for 45->48 changes */
#endif

#ifndef	SoftModemPh
#include "SoftModem.h"
#endif

#ifdef DSL_OS
#include "DslOs.h"
#endif
#include "CircBuf.h"

#ifdef  VDSL_MODEM
#include "SoftVdsl.h"
#endif

/*
**
**		Type definitions 
**
*/

#if defined(ATM) || defined(DSL_PACKET)
#define DSL_LINKLAYER
#endif

#if defined(ATM_LAYER) || defined(DSL_PACKET_LAYER) || defined(G997_1_FRAMER)
#define DSL_FRAME_FUNCTIONS
#endif

#define FLD_OFFSET(type,fld)	((int)(void *)&(((type *)0)->fld))

#include "Que.h"
#include "SoftAtmVc.h"

typedef struct _dslFrameBuffer
	{
	struct _dslFrameBuffer *next;	/* link to the next buffer in the frame */
	void				   *pData;	/* pointer to data */
	ulong				   length;	/* size (in bytes) of data */
	} dslFrameBuffer;

typedef struct _dslFrame
	{
	ulong			Reserved[3];

	ulong			totalLength;	/* total amount of data in the packet */
	int				bufCnt;			/* buffer counter */
	struct _dslFrameBuffer *head;	/* first buffer in the chain */
	struct _dslFrameBuffer *tail;	/* last buffer in the chain  */
	} dslFrame;


/* VC types and parameters */

#define	kDslVcAtm		1

typedef	struct
	{
	ulong	vcType;
	union
		{
		atmVcParams	atmParams;
		} params;
	} dslVcParams;

/*
**	Assuming that dslVcParams.params is the first field in VC
**	and RefData is the first field in dslVcParams.params
*/

#define	DslVcGetRefData(pVc)	(*(void **) (pVc))

/* Frame OOB types */

#define	kDslFrameAtm	1

typedef	struct
	{
	ulong	frameType;		
	union
		{
		atmOobPacketInfo	atmInfo;
		} param;
	} dslOobFrameInfo;


typedef struct
	{
	ulong (SM_DECL *__DslFrameBufferGetLength) (dslFrameBuffer *fb);
	void * (SM_DECL *__DslFrameBufferGetAddress) (dslFrameBuffer *fb);
	void (SM_DECL *__DslFrameBufferSetLength) (dslFrameBuffer *fb, ulong l);
	void (SM_DECL *__DslFrameBufferSetAddress) (dslFrameBuffer *fb, void *p);

	void (SM_DECL *__DslFrameInit) (dslFrame *f);
	ulong (SM_DECL *__DslFrameGetLength) (dslFrame *pFrame);
	ulong (SM_DECL *__DslFrameGetBufCnt) (dslFrame *pFrame);
	dslFrameBuffer * (SM_DECL *__DslFrameGetFirstBuffer) (dslFrame *pFrame);
	dslFrameBuffer * (SM_DECL *__DslFrameGetNextBuffer) (dslFrameBuffer *pFrBuffer);
	void  (SM_DECL *__DslFrameSetNextBuffer) (dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext);
	dslFrameBuffer * (SM_DECL *__DslFrameGetLastBuffer) (dslFrame *pFrame);
	void * (SM_DECL *__DslFrameGetLinkFieldAddress) (dslFrame *f);
	dslFrame * (SM_DECL *__DslFrameGetFrameAddressFromLink) (void *lnk);

	Boolean (SM_DECL *__DslFrameGetOobInfo) (dslFrame *f, dslOobFrameInfo	*pOobInfo);
	Boolean (SM_DECL *__DslFrameSetOobInfo) (dslFrame *f, dslOobFrameInfo	*pOobInfo);

	void (SM_DECL *__DslFrameEnqueBufferAtBack) (dslFrame *f, dslFrameBuffer *b);
	void (SM_DECL *__DslFrameEnqueFrameAtBack) (dslFrame *fMain, dslFrame *f);
	void (SM_DECL *__DslFrameEnqueBufferAtFront) (dslFrame *f, dslFrameBuffer *b);
	void (SM_DECL *__DslFrameEnqueFrameAtFront) (dslFrame *fMain, dslFrame *f);
	dslFrameBuffer * (SM_DECL *__DslFrameDequeBuffer) (dslFrame *pFrame);

	void * (SM_DECL *__DslFrameAllocMemForFrames) (ulong frameNum);
	void (SM_DECL *__DslFrameFreeMemForFrames) (void *hMem);
	dslFrame * (SM_DECL *__DslFrameAllocFrame) (void *handle);
	void (SM_DECL *__DslFrameFreeFrame) (void *handle, dslFrame *pFrame);
	void * (SM_DECL *__DslFrameAllocMemForBuffers) (void **ppMemPool, ulong bufNum, ulong memSize);
	void (SM_DECL *__DslFrameFreeMemForBuffers) (void *hMem, ulong memSize, void *pMemPool);
	dslFrameBuffer * (SM_DECL *__DslFrameAllocBuffer) (void *handle, void *pMem, ulong length);
	void (SM_DECL *__DslFrameFreeBuffer) (void *handle, dslFrameBuffer *pBuf);

	/* for LOG file support */

	ulong (SM_DECL *__DslFrame2Id)(void *handle, dslFrame *pFrame);
	void * (SM_DECL *__DslFrameId2Frame)(void *handle, ulong frameId);
	} dslFrameFunctions;

#define	 DslFrameDeclareFunctions( name_prefix )								\
extern ulong SM_DECL name_prefix##BufferGetLength(dslFrameBuffer *fb);			\
extern void * SM_DECL name_prefix##BufferGetAddress(dslFrameBuffer *fb);		\
extern void SM_DECL name_prefix##BufferSetLength(dslFrameBuffer *fb, ulong l);	\
extern void SM_DECL name_prefix##BufferSetAddress(dslFrameBuffer *fb, void *p); \
																				\
extern void SM_DECL name_prefix##Init(dslFrame *f);								\
extern ulong SM_DECL name_prefix##GetLength (dslFrame *pFrame);					\
extern ulong SM_DECL name_prefix##GetBufCnt(dslFrame *pFrame);					\
extern dslFrameBuffer * SM_DECL name_prefix##GetFirstBuffer(dslFrame *pFrame);	\
extern dslFrameBuffer * SM_DECL name_prefix##GetNextBuffer(dslFrameBuffer *pFrBuffer);	\
extern void SM_DECL name_prefix##SetNextBuffer(dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext);	\
extern dslFrameBuffer * SM_DECL name_prefix##GetLastBuffer(dslFrame *pFrame);			\
extern void * SM_DECL name_prefix##GetLinkFieldAddress(dslFrame *f);					\
extern Boolean SM_DECL name_prefix##GetOobInfo(dslFrame *f, dslOobFrameInfo *pOobInfo);	\
extern Boolean SM_DECL name_prefix##SetOobInfo(dslFrame *f, dslOobFrameInfo *pOobInfo);	\
extern dslFrame* SM_DECL name_prefix##GetFrameAddressFromLink(void *lnk);				\
extern void SM_DECL name_prefix##EnqueBufferAtBack(dslFrame *f, dslFrameBuffer *b);		\
extern void SM_DECL name_prefix##EnqueFrameAtBack(dslFrame *fMain, dslFrame *f);		\
extern void SM_DECL name_prefix##EnqueBufferAtFront(dslFrame *f, dslFrameBuffer *b);	\
extern void SM_DECL name_prefix##EnqueFrameAtFront(dslFrame *fMain, dslFrame *f);		\
extern dslFrameBuffer * SM_DECL name_prefix##DequeBuffer(dslFrame *pFrame);				\
																						\
extern void * SM_DECL name_prefix##AllocMemForFrames(ulong frameNum);					\
extern void SM_DECL name_prefix##FreeMemForFrames(void *hMem);							\
extern dslFrame * SM_DECL name_prefix##AllocFrame(void *handle);						\
extern void SM_DECL name_prefix##FreeFrame(void *handle, dslFrame *pFrame);				\
extern void * SM_DECL name_prefix##AllocMemForBuffers(void **ppMemPool, ulong bufNum, ulong memSize);	\
extern void SM_DECL name_prefix##FreeMemForBuffers(void *hMem, ulong memSize, void *pMemPool);			\
extern dslFrameBuffer * SM_DECL name_prefix##AllocBuffer(void *handle, void *pMem, ulong length);		\
extern void SM_DECL name_prefix##FreeBuffer(void *handle, dslFrameBuffer *pBuf);		\
extern ulong SM_DECL name_prefix##2Id(void *handle, dslFrame *pFrame);					\
extern void * SM_DECL name_prefix##Id2Frame(void *handle, ulong frameId);


#define	 DslFrameAssignFunctions( var, name_prefix )	do {			\
	(var).__DslFrameBufferGetLength	= name_prefix##BufferGetLength;		\
	(var).__DslFrameBufferGetAddress= name_prefix##BufferGetAddress;	\
	(var).__DslFrameBufferSetLength	= name_prefix##BufferSetLength;		\
	(var).__DslFrameBufferSetAddress= name_prefix##BufferSetAddress;	\
																		\
	(var).__DslFrameInit			= name_prefix##Init;				\
	(var).__DslFrameGetLength		= name_prefix##GetLength;			\
	(var).__DslFrameGetBufCnt		= name_prefix##GetBufCnt;			\
	(var).__DslFrameGetFirstBuffer	= name_prefix##GetFirstBuffer;		\
	(var).__DslFrameGetNextBuffer	= name_prefix##GetNextBuffer;		\
	(var).__DslFrameSetNextBuffer	= name_prefix##SetNextBuffer;		\
	(var).__DslFrameGetLastBuffer	= name_prefix##GetLastBuffer;		\
	(var).__DslFrameGetLinkFieldAddress		= name_prefix##GetLinkFieldAddress;		\
	(var).__DslFrameGetFrameAddressFromLink	= name_prefix##GetFrameAddressFromLink; \
																		\
	(var).__DslFrameGetOobInfo		= name_prefix##GetOobInfo;			\
	(var).__DslFrameSetOobInfo		= name_prefix##SetOobInfo;			\
																		\
	(var).__DslFrameEnqueBufferAtBack	= name_prefix##EnqueBufferAtBack;	\
	(var).__DslFrameEnqueFrameAtBack	= name_prefix##EnqueFrameAtBack;	\
	(var).__DslFrameEnqueBufferAtFront= name_prefix##EnqueBufferAtFront;	\
	(var).__DslFrameEnqueFrameAtFront	= name_prefix##EnqueFrameAtFront;	\
	(var).__DslFrameDequeBuffer		= name_prefix##DequeBuffer;			\
																		\
	(var).__DslFrameAllocMemForFrames	= name_prefix##AllocMemForFrames;	\
	(var).__DslFrameFreeMemForFrames	= name_prefix##FreeMemForFrames;	\
	(var).__DslFrameAllocFrame			= name_prefix##AllocFrame;			\
	(var).__DslFrameFreeFrame			= name_prefix##FreeFrame;			\
	(var).__DslFrameAllocMemForBuffers= name_prefix##AllocMemForBuffers;	\
	(var).__DslFrameFreeMemForBuffers = name_prefix##FreeMemForBuffers;	\
	(var).__DslFrameAllocBuffer		= name_prefix##AllocBuffer;			\
	(var).__DslFrameFreeBuffer		= name_prefix##FreeBuffer;			\
																		\
	(var).__DslFrame2Id				= name_prefix##2Id;					\
	(var).__DslFrameId2Frame		= name_prefix##Id2Frame;			\
} while (0)

typedef	struct
	{
	Boolean		febe_I;
	Boolean		fecc_I;
	Boolean		los, rdi;
	Boolean		ncd_I;
	Boolean		hec_I;
#ifdef G992P3 
	Boolean		lpr;
#endif

#ifdef	G992P1_NEWFRAME

	Boolean		febe_F;
	Boolean		fecc_F;
	Boolean		ncd_F;
	Boolean		hec_F;
	
#endif
	} G992MonitorParams;

typedef struct
	{
	ushort	K;
	uchar	S, R;
	uchar   D;
#ifdef G992P3 
	uchar	T, SEQ;
#endif
	directionType	direction;

#ifdef		G992P1_NEWFRAME

	ushort	N;
	ushort	NF;
	uchar	RSF;

	uchar	AS0BF, AS1BF, AS2BF, AS3BF, AEXAF;
	ushort	AS0BI;
	uchar	AS1BI, AS2BI, AS3BI, AEXAI;

	uchar	LS0CF, LS1BF, LS2BF, LEXLF;
	uchar	LS0CI, LS1BI, LS2BI, LEXLI;
	
	uchar	mergedModeEnabled;
	
#endif

	} G992CodingParams;

typedef struct
	{
	uchar	Nlp;
	uchar	Nbc;
	uchar	MSGlp;
	ushort	MSGc;

	ulong	L;
	ushort	M;
	ushort	T;
	ushort	D;
	ushort	R;
	ushort	B;
	} G992p3CodingParams;

/* Power Management Message definitions (used in command and status) */

typedef struct
	{
	long			msgType;
	union
		{
		long		value;
		struct
			{
			long	msgLen;
			void	*msgData;
			} msg;
		} param;
	} dslPwrMessage;

/* Power Management commands and responses */

#define	kPwrSimpleRequest							1
#define	kPwrL2Request								2
#define	kPwrL2TrimRequest							3

#define	kPwrGrant									0x80
#define	kPwrReject									0x81
#define	kPwrL2Grant									0x82
#define	kPwrL2Reject								0x83
#define	kPwrL2TrimGrant								0x84
#define	kPwrL2TrimReject							0x85
#define	kPwrL2Grant2p								0x86

#define	kPwrBusy									0x01
#define	kPwrInvalid									0x02
#define	kPwrNotDesired								0x03
#define	kPwrInfeasibleParam							0x04

/* Power Management reason codes */

/* OLR definitions (used in command and status) */

typedef struct
	{
	ushort	msgType;
	ushort	nCarrs;
	ushort	L[4];
	uchar	B[4];
	void	*carrParamPtr;
	} dslOLRMessage;

typedef struct
	{
	uchar	ind;
	uchar	gain;
	uchar	gb;
	} dslOLRCarrParam;

typedef struct
	{
	ushort	ind;
	uchar	gain;
	uchar	gb;
	} dslOLRCarrParam2p;

/* OLR messages */

#define	kOLRRequestType1							1
#define	kOLRRequestType2							2
#define	kOLRRequestType3							3
#define	kOLRRequestType4							4
#define	kOLRRequestType5							5
#define	kOLRRequestType6							6

#define	kOLRDeferType1								0x81
#define	kOLRRejectType2								0x82
#define	kOLRRejectType3								0x83

/* OLR reason codes */

#define	kOLRBusy									1
#define	kOLRInvalidParam							2
#define	kOLRNotEnabled								3
#define	kOLRNotSupported							4

/* common EOC definitions  */
#define	kG992EocStuffingByte						0x0C

/* showtime monitor counters */
#define	kG992ShowtimeRSCodewordsRcved				0	/* number of Reed-Solomon codewords received */
#define kG992ShowtimeRSCodewordsRcvedOK				1	/* number of Reed-Solomon codewords received with all symdromes zero */
#define	kG992ShowtimeRSCodewordsRcvedCorrectable	2	/* number of Reed-Solomon codewords received with correctable errors */
#define	kG992ShowtimeRSCodewordsRcvedUncorrectable	3	/* number of Reed-Solomon codewords received with un-correctable errors */
#define	kG992ShowtimeSuperFramesRcvd				4	/* number of super frames received */
#define	kG992ShowtimeSuperFramesRcvdWrong			5	/* number of super frames received with CRC error */
#define	kG992ShowtimeLastUncorrectableRSCount		6	/* last recorded value for kG992ShowtimeRSCodewordsRcvedUncorrectable */
#define	kG992ShowtimeLastWrongSuperFrameCount		7	/* last recorded value for kG992ShowtimeSuperFramesRcvdWrong */
#define	kG992ShowtimeNumOfShortResync				8	/* number of short interrupt recoveries by FEQ */

#define	kG992ShowtimeNumOfFEBE						9	/* number of other side superframe errors */
#define	kG992ShowtimeNumOfFECC						10	/* number of other side superframe FEC errors */
#define	kG992ShowtimeNumOfFHEC						11	/* number of far-end ATM header CRC errors */
#define	kG992ShowtimeNumOfFOCD						12	/* number of far-end OCD events */
#define	kG992ShowtimeNumOfFLCD						13	/* number of far-end LCD events */
#define	kG992ShowtimeNumOfHEC						14	/* number of ATM header CRC errors */
#define	kG992ShowtimeNumOfOCD						15	/* number of OCD events */
#define	kG992ShowtimeNumOfLCD						16	/* number of LCD events */

#define	kG992ShowtimeNumOfMonitorCounters			(kG992ShowtimeNumOfLCD+1)	/* always last number + 1 */
#define	kG992ShowtimeMonitorReportNumber			9

#define	kG992ShowtimeLCDNumShift					1
#define	kG992ShowtimeLCDFlag						1

/* Fire monitor counters */
#define kFireReXmtRSCodewordsRcved                      0
#define kFireReXmtUncorrectedRSCodewords                1
#define kFireReXmtCorrectedRSCodewords                  2

#define kFireNumOfCounters                              3

/* Fire status bitmap */
#define kFireDsEnabled                                  0x1
#define kFireUsEnabled                                  0x2             /* Unused in the PHY for the moment as Fire US is not implemented */

/*  line-drop reason code */
#define kRetrainReasonLosDetector                   0
#define kRetrainReasonRdiDetector                   1
#define kRetrainReasonNegativeMargin                2
#define kRetrainReasonTooManyUsFEC                  3
#define kRetrainReasonCReverb1Misdetection          4
#define kRetrainReasonTeqDsp                        5
#define kRetrainReasonAnsiTonePowerChange           6
#define kRetrainReasonIfftSizeChange                7
#define kRetrainReasonRackChange                    8
#define kRetrainReasonVendorIdSync                  9
#define kRetrainReasonTargetMarginSync             10
#define kRetrainReasonToneOrderingException        11
#define kRetrainReasonCommandHandler               12
#define kRetrainReasonDslStartPhysicalLayerCmd     13
#define kRetrainReasonUnknown                      14
#define kRetrainReasonG992Failure                  15
#define kRetrainReasonSes                          16
#define kRetrainReasonCoMinMargin                  17

typedef	int		(SM_DECL *dslFrameHandlerType)	(void *gDslVars, void *pVc, ulong mid, dslFrame *);

typedef	void*   (SM_DECL *dslHeaderHandlerType) (void *gDslVars, ulong hdr, uchar hdrHec);
typedef	void*	(SM_DECL *dslTxFrameBufferHandlerType)	(void *gDslVars, int*,	void*);
typedef	void*	(SM_DECL *dslRxFrameBufferHandlerType)	(void *gDslVars, int,	void*);

typedef	void*	(SM_DECL *dslVcAllocateHandlerType)	(void *gDslVars, void *);
typedef	void	(SM_DECL *dslVcFreeHandlerType)	(void *gDslVars, void *);
typedef	Boolean	(SM_DECL *dslVcActivateHandlerType)	(void *gDslVars, void *);
typedef	void	(SM_DECL *dslVcDeactivateHandlerType) (void *gDslVars, void *);
typedef	Boolean	(SM_DECL *dslVcConfigureHandlerType) (void *gDslVars, void *pVc, ulong mid, void *);

typedef	ulong	(SM_DECL *dslLinkVc2IdHandlerType) (void *gDslVars, void *);
typedef	void*	(SM_DECL *dslLinkVcId2VcHandlerType) (void *gDslVars, ulong);
typedef void*	(SM_DECL *dslGetFramePoolHandlerType) (void *gDslVars);

typedef	void	(SM_DECL *dslLinkCloseHandlerType) (void *gDslVars);
typedef	int		(SM_DECL *dslTxDataHandlerType)(void *gDslVars, int, int, uchar*, G992MonitorParams*);
typedef	int		(SM_DECL *dslRxDataHandlerType)(void *gDslVars, int, uchar*, G992MonitorParams*);

typedef	void	(SM_DECL *dslLinkStatusHandler) (void *gDslVars, ulong statusCode, ...);

typedef Boolean (SM_DECL *dslPhyInitType) (
	void						*gDslVars, 
	bitMap						setupMap,
	dslHeaderHandlerType		rxCellHeaderHandlerPtr,
	dslRxFrameBufferHandlerType	rxFrameHandlerPtr,  
	dslTxFrameBufferHandlerType txFrameHandlerPtr,
	atmStatusHandler			statusHandlerPtr);

struct _dslFramerBufDesc;

typedef	Boolean (SM_DECL *dslFramerDataGetPtrHandlerType) (void *gDslVars, struct _dslFramerBufDesc *pBufDesc);
typedef	void	(SM_DECL *dslFramerDataDoneHandlerType) (void *gDslVars, struct _dslFramerBufDesc *pBufDesc);

typedef	void	(SM_DECL *dslDriverCallbackType) (void *gDslVars);

#ifdef DSL_PACKET

typedef	struct 
	{
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;
	} dslPacketPhyFunctions;

typedef Boolean (SM_DECL *dslPacketPhyInitType) (
	void						*gDslVars, 
	bitMap						setupMap,
	dslPacketPhyFunctions		dslPhyFunctions,
	dslLinkStatusHandler		statusHandlerPtr);


#endif /* DSL_PACKET */


typedef	int			dslDirectionType;
typedef	bitMap		dslModulationType;
typedef	bitMap		dslLinkLayerType;

/*
**
**		Log data codes 
**
*/

#define	kDslEyeData					eyeData

#define	kDslLogComplete				(inputSignalData - 1)
#define	kDslLogInputData			inputSignalData
#define	kDslLogInputData1			(inputSignalData + 1)
#define	kDslLogInputData2			(inputSignalData + 2)
#define	kDslLogInputData3			(inputSignalData + 3)

#define	kDslDebugData				(inputSignalData + 8)
#define	kDslDebugDataValueMask		0x3
#define	kDslDebugDataAlignShift		2
#define	kDslDebugDataAlignMask		(0x3 << kDslDebugDataAlignShift)
#define	kDslDebugDataMask			(kDslDebugDataValueMask | kDslDebugDataAlignMask)

/*
**
**		Status codes 
**
*/

typedef long						dslStatusCode;
#define	kFirstDslStatusCode			256
#define	kDslError					(kFirstDslStatusCode + 0)
#define	kAtmStatus					(kFirstDslStatusCode + 1)
#define	kDslTrainingStatus			(kFirstDslStatusCode + 2)
#define	kDslConnectInfoStatus		(kFirstDslStatusCode + 3)
#define	kDslEscapeToG994p1Status	(kFirstDslStatusCode + 4)
#define	kDslFrameStatus				(kFirstDslStatusCode + 5)
#define kDslReceivedEocCommand		(kFirstDslStatusCode + 6)
#define kDslSendEocCommandDone		(kFirstDslStatusCode + 7)
#define kDslSendEocCommandFailed	(kFirstDslStatusCode + 8)
#define kDslWriteRemoteRegisterDone	(kFirstDslStatusCode + 9)
#define kDslReadRemoteRegisterDone	(kFirstDslStatusCode + 10)
#define	kDslExternalError			(kFirstDslStatusCode + 11)
#define kDslDspControlStatus		(kFirstDslStatusCode + 12)
#define kDslATUHardwareAGCRequest	(kFirstDslStatusCode + 13)
#define	kDslPacketStatus			(kFirstDslStatusCode + 14)
#define	kDslG997Status				(kFirstDslStatusCode + 15)
#define	kDslPrintfStatus			(kFirstDslStatusCode + 16)
#define	kDslPrintfStatus1			(kFirstDslStatusCode + 17)
#define	kDslExceptionStatus			(kFirstDslStatusCode + 18)
#define	kDslPingResponse			(kFirstDslStatusCode + 19)
#define	kDslShowtimeSNRMarginInfo	(kFirstDslStatusCode + 20)
#define	kDslGetOemParameter			(kFirstDslStatusCode + 21)
#define	kDslOemDataAddrStatus		(kFirstDslStatusCode + 22)
#define	kDslDataAvailStatus			(kFirstDslStatusCode + 23)
/* #define kDslAtuChangeTxFilterRequest (kFirstDslStatusCode + 24) */
#define kDslTestPllPhaseResult      (kFirstDslStatusCode + 25)
#ifdef BCM6348_SRC
#define kDslHardwareAGCSetPga1      (kFirstDslStatusCode + 26)
#define kDslHardwareAGCDecPga1      (kFirstDslStatusCode + 27)
#define kDslHardwareAGCIncPga1      (kFirstDslStatusCode + 28)
#define kDslHardwareAGCSetPga2Delta (kFirstDslStatusCode + 29)
#endif
#define	kDslOLRRequestStatus		(kFirstDslStatusCode + 30)
#define	kDslOLRResponseStatus		(kFirstDslStatusCode + 31)
#define	kDslOLRBitGainUpdateStatus	(kFirstDslStatusCode + 32)
#define	kDslPwrMgrStatus		    (kFirstDslStatusCode + 33)
#define	kDslEscapeToT1p413Status	(kFirstDslStatusCode + 34)
#ifdef BCM6348_SRC
#define kDslHardwareAGCSetPga2      (kFirstDslStatusCode + 35)
#define kDslHardwareGetRcvAGC       (kFirstDslStatusCode + 36)
#endif
#define kDslUpdateXmtReadPtr        (kFirstDslStatusCode + 37)
#define kDslHardwareSetRcvAGC       (kFirstDslStatusCode + 38)
#ifdef BCM6348_SRC
#define kDslSetDigUsPwrCutback      (kFirstDslStatusCode + 39)
#endif

#define	kClientSideInitiation		0
#define	kClientSideRespond			1
#define	kCentralSideInitiation		2
#define	kCentralSideRespond			3

/* OEM parameter ID definition */

#define	kDslOemG994VendorId			1
#define	kDslOemG994XmtNSInfo		2
#define	kDslOemG994RcvNSInfo		3
#define	kDslOemEocVendorId			4
#define	kDslOemEocVersion			5
#define	kDslOemEocSerNum			6
#define	kDslOemT1413VendorId		7
#define	kDslOemT1413EocVendorId		8

typedef long	dslErrorCode;

typedef long	atmStatusCode;
typedef long	dslFramerStatusCode;

typedef long	atmErrorCode;

typedef long	dslTrainingStatusCode;

#define kDslStartedG994p1					0
#define kDslStartedT1p413HS					1

/* reserved for G.994.1: 1 ~ 8 */

#define	kDslG994p1MessageDet				100
#define	kDslG994p1ToneDet					101
#define	kDslG994p1RToneDet					102
#define	kDslG994p1FlagDet					103
#define	kDslG994p1GalfDet					104
#define	kDslG994p1ErrorFrameDet				105
#define	kDslG994p1BadFrameDet				106
#define	kDslG994p1SilenceDet				107
#define	kDslG994p1RcvTimeout				108
#define	kDslG994p1XmtFinished				109
#define	kDslG994p1ReturntoStartup			110
#define	kDslG994p1InitiateCleardown			111
#define	kDslG994p1StartupFinished			112
#define	kDslG994p1RcvNonStandardInfo		113
#define	kDslG994p1XmtNonStandardInfo		114

#define	kG994p1MaxNonstdMessageLength		64

#define kDslFinishedT1p413					1100
#define kDslT1p413DetectedCTone				1101
#define kDslT1p413DetectedCAct				1102
#define kDslT1p413DetectedCReveille			1103
#define kDslT1p413DetectedRActReq			1104
#define kDslT1p413DetectedRQuiet1			1105
#define kDslT1p413DetectedRAct				1106
#define kDslT1p413TimeoutCReveille			1107
#define	kDslT1p413ReturntoStartup			1108

#define	kDslG994p1Timeout					8
#define kDslFinishedG994p1					9
#define kDslStartedG992p2Training			10
#define	kDslG992p2DetectedPilotSymbol		11
#define	kDslG992p2DetectedReverbSymbol		12
#define	kDslG992p2TEQCalculationDone		13
#define	kDslG992p2TrainingFEQ				14
#define	kDslG992p2Phase3Started				15
#define	kDslG992p2ReceivedRates1			16
#define	kDslG992p2ReceivedMsg1				17
#define	kDslG992p2Phase4Started				18
#define	kDslG992p2ReceivedRatesRA			19
#define	kDslG992p2ReceivedMsgRA				20
#define	kDslG992p2ReceivedRates2			21
#define	kDslG992p2ReceivedMsg2				22
#define	kDslG992p2ReceivedBitGainTable		23
#define	kDslG992p2TxShowtimeActive			24
#define	kDslG992p2RxShowtimeActive			25
#define	kDslG992p2TxAocMessage				26
#define	kDslG992p2RxAocMessage				27
#define	kDslG992p2TxEocMessage				28
#define	kDslG992p2RxEocMessage				29
#define kDslFinishedG992p2Training			30
#define	kDslRecoveredFromImpulseNoise		31
#define	kDslG992Timeout						32
#define	kDslT1p413Isu1SglByteSymDetected	33	/* detected T1.413 Issue 1 single byte per symbol mode */
#define	kDslG992RxPrefixOnInAFewSymbols		34
#define	kDslG992TxPrefixOnInAFewSymbols		35
#define	kDslAnnexCXmtCPilot1Starting		36
#define	kDslXmtToRcvPathDelay				37
#define kDslFeaturesUnsupported				38
#define	kDslG992RcvMsgCrcError				39
#define	kDslAnnexCDetectedStartHyperframe	40

#define kDslG992AnnexCTimeoutCPilot1Detection	41
#define kDslG992AnnexCTimeoutCReverb1Detection	42
#define kDslG992AnnexCTimeoutECTraining			43
#define kDslG992AnnexCTimeoutHyperframeDetector	44
#define kDslG992AnnexCTimeoutSendRSegue2		45
#define kDslG992AnnexCTimeoutDetectCSegue1		46
#define kDslG992AnnexCAlignmentErrDetected		47
#define kDslG992AnnexCTimeoutSendRSegueRA		48
#define kDslG992AnnexCTimeoutSendRSegue4		49
#define kDslG992AnnexCTimeoutCSegue2Detection	50
#define kDslG992AnnexCTimeoutCSegue3Detection	51
/* Progress report for fast retrain */

#define	kG994p1EventToneDetected				54
#define	kDslG992p2RcvVerifiedBitAndGain         55
#define	kDslG992p2ProfileChannelResponseCalc    56
#define kDslG992AnnexCTotalFEXTBits				57
#define kDslG992AnnexCTotalNEXTBits				58
#define kDslG992AnnexCTotalFEXTCarrs			59
#define kDslG992AnnexCTotalNEXTCarrs			60

#define	kDslG992p3ReceivedMsgFmt				61
#define	kDslG992p3ReceivedMsgPcb				62

#define	kDslG992p3AnnexLMode					63

#define kDslG992p2ReceivedMsgLD                         64

/* performance monitoring report */

#define	kG992DataRcvDetectFastRSCorrection				70
#define	kG992DataRcvDetectInterleaveRSCorrection		71
#define	kG992DataRcvDetectFastCRCError					72
#define	kG992DataRcvDetectInterleaveCRCError			73
#define	kG992DataRcvDetectFastRSError					74
#define	kG992DataRcvDetectInterleaveRSError				75
#define	kG992DataRcvDetectLOS							76
#define	kG992DecoderDetectRDI							77
#define	kG992DataRcvDetectLOSRecovery					78
#define	kG992AtmDetectHEC								79
#define	kG992DataRcvDetectPartialNegativeMargin			80
#define	kG992DataRcvDetectPartialNegativeMarginRecovered	81
#define	kG992DecoderDetectRDIRecovery					179
#define	kG992AtmDetectOCD								180
#define	kG992AtmDetectCD								181
#define	kG992DecoderDetectRemoteLOS						182
#define	kG992DecoderDetectRemoteLOSRecovery				183
#define	kG992DecoderDetectRemoteRDI						184
#define	kG992DecoderDetectRemoteRDIRecovery				185
#define	kG992RcvDetectSyncSymbolOffset					186
#define	kG992Upstream2xIfftDisabled						187
#if defined(G992P5)
#define	kDslG992RunAnnexaP3ModeInAnnexaP5	   	        188	 /* run Annex C mode in Annex I compiled codes */
#else
#define	kDslG992RunAnnexCModeInAnnexI			        188	 /* run Annex C mode in Annex I compiled codes */
#endif

/* OLR PHY status */

#define	kG992EventSynchSymbolDetected					189
#define	kG992EventReverseSynchSymbolDetected			190
#define	kG992EventL2CReverbSymbolDetected				191
#define	kG992EventL2CSegueSymbolDetected				192

/* ANNEX_M */
#define kG992EnableAnnexM                               191

#define kDslAtuChangeTxFilterRequest					192

/* sample buffer conditions */

#define kDslMarkerCheckFailed							193
#define kDslMarkerCheckRecovered						194

#define kDslRcvBufferOverflowDetected					195
#define kDslRcvBufferOverflowRecovered					196

#define kDslXmtBufferOverflowDetected					197
#define kDslXmtBufferOverflowRecovered					198

#define kDslXmtBufferUnderflowDetected					199
#define kDslXmtBufferUnderflowRecovered					200

/* NSIF PLN statuses */

#define kG992SetPLNMessageBase							201
#define kG992DefaultPLNMessageBase						0x10

/* Loop Diagnostic (LD) statuses */

#define kG992LDStartMode								202
#define kG992LDCompleted								203

#define kDslAfeSampleLoss					   		    204

#define kDslRetrainReason                               205

/* Fire status US & DS */
#define kG992FireState                                  206
#define kG992RcvDelay                               208
#define kG992RcvInp                                 209

/* More PLN statuses */
#define kG992PlnBroadbandCounterReset                   207

/* detailed error messages reports */

#define	kDslG992XmtRReverbRAOver4000			80
#define	kDslG992XmtRReverb5Over4000				81
#define	kDslG992RcvCSegue2Failed				82
#define	kDslG992RcvCSegueRAFailed				83
#define	kDslG992RcvCSegue3Failed				84
#define	kDslG992RcvShowtimeStartedTooLate		85
#define	kDslG992XmtRReverb3Over4000				86
#define	kDslG992RcvFailDetCSegue1InWindow		87
#define	kDslG992RcvCPilot1Failed				88
#define	kDslG992RcvCReverb1Failed				89
#define	kG992ControlAllRateOptionsFailedErr		90
#define	kG992ControlInvalidRateOptionErr		91
#define	kDslG992XmtInvalidXmtDErr				92
#define	kDslG992BitAndGainCalcFailed			93
#define	kDslG992BitAndGainVerifyFailed			94

#define	kDslT1p413RetrainToUseCorrectRAck		95
#define	kDslUseAlternateTxFilter				96
#define	kDslT1p413RetrainToUseCorrectIFFT		97

typedef	long	dslConnectInfoStatusCode;
#define	kG992p2XmtToneOrderingInfo			0
#define	kG992p2RcvToneOrderingInfo			1
#define	kG992p2XmtCodingParamsInfo			2
#define	kG992p2RcvCodingParamsInfo			3
#define	kG992p2TrainingRcvCarrEdgeInfo		4
#define	kG992ShowtimeMonitoringStatus		5
#define	kG992MessageExchangeRcvInfo			6
#define	kG992MessageExchangeXmtInfo			7
#define	kG994MessageExchangeRcvInfo			8
#define	kG994MessageExchangeXmtInfo			9

#define	kDslATURClockErrorInfo				10
#define	kDslATURcvPowerInfo					11
#define	kDslATUAvgLoopAttenuationInfo		12
#define	kDslHWTimeTrackingResetClockError	13
#define	kDslHWTimeTrackingClockTweak		14
#define kDslATUHardwareAGCObtained			15
#define	kDslTEQCoefInfo						16
#define	kDslRcvCarrierSNRInfo				17
#define	kDslMaxReceivableBitRateInfo		18
#define kDslHWSetDigitalEcUpdateMode		19
#define kDslHWEnableDigitalECUpdate			20
#define kDslHWDisableDigitalECUpdate 		21
#define kDslHWEnableDigitalEC				22
#define kDslHWSetDigitalEcGainShift			23
#define kDslHWSetDigitalEcUpdateShift		24
#define	kDslRcvPsdInfo						25
#define	kDslHWdcOffsetInfo					26
#define	kG994SelectedG994p1CarrierIndex		27
#define	kDslSelectedTimingTone				28

#define	kDslHWEnableAnalogECUpdate			kDslHWSetDigitalEcUpdateMode	
#define	kDslHWEnableAnalogEC				kDslHWDisableDigitalECUpdate

#define	kG992AocMessageExchangeRcvInfo		29
#define	kG992AocMessageExchangeXmtInfo		30
#define	kG992AocBitswapTxStarted			31
#define	kG992AocBitswapRxStarted			32
#define	kG992AocBitswapTxCompleted			33
#define	kG992AocBitswapRxCompleted			34
#define kDslChannelResponseLog				35
#define kDslChannelResponseLinear			36
#define kDslChannelQuietLineNoise			37

#define	kDslATUCXmtPowerCutbackInfo			40
#define	kDslATURXmtPowerCutbackInfo			41
#define	kDslATUCXmtPowerInfo				42
#define	kDslATURXmtPowerInfo				43

#define	kDslFramingModeInfo					50
#define	kDslG992VendorID					51

#ifdef BCM6348_SRC
#define kDslHWSetRcvFir2OutputScale			52
#endif

#define kDslSignalAttenuation               53
#define kDslAttainableNetDataRate           54
#define kDslHLinScale                       55

#define	kG992p3XmtCodingParamsInfo			60
#define	kG992p3RcvCodingParamsInfo			61
#define	kG992p3PwrStateInfo					62
#define	kG992PilotToneInfo					63

#define kDslSetPilotEyeDisplay              64

/* PLN control */
#define	kDslPLNPeakNoiseTablePtr			65
#define kDslPerBinThldViolationTablePtr		66
#define	kDslImpulseNoiseDurationTablePtr	67
#define	kDslImpulseNoiseTimeTablePtr		68
#define kDslPLNMarginPerBin                 69
#define kDslPLNMarginBroadband              70
#define kDslPerBinMsrCounter                71
#define kDslBroadbandMsrCounter             72
#define	kDslInpBinTablePtr					73
#define	kDslItaBinTablePtr					74
#define	kDslStatusBufferInfo				75
#define	kDslRcvCarrierSNRInfo1				76
#define kDslPlnState                        77

/* Non-linear detection info */
#define kDslNLNoise                             78
#define kDslNLMaxCritNoise                      79
#define kDslNLAffectedBits                      80
#define	kDslInitializationSNRMarginInfo         81

#define	kDslAtmVcTablePtr					90

#define kDslG992RcvShowtimeUpdateGainPtr   91
#define kFireMonitoringCounters                         92
#define kDslRxOvhMsg                                         93
#define kDslTxOvhMsg                                         94

#define kDslAturHwAgcResolutionMask			(0xFFFFFFF8)
#define kDslAturHwAgcMinGain				((-12)<<4)
#ifndef BCM6348_SRC
#define kDslAturHwAgcMaxGain				(30<<4)
#else
#define kDslAturHwAgcMaxGain				(36<<4)
#endif

#define	kDslFrameStatusSend					1
#define	kDslFrameStatusSendComplete			2
#define	kDslFrameStatusRcv					3
#define	kDslFrameStatusReturn				4

typedef struct _dslFramerStatus
	{
	dslFramerStatusCode		code;
	union
		{
		long				value;
		dslErrorCode		error;
		struct
			{
			long	length;
			uchar	*framePtr;
			} frame;
		struct
			{
			long	nRxFrameTotal;
			long	nRxFrameError;
			long	nTxFrameTotal;
			} statistic;
		} param;
	} dslFramerStatus;

typedef	struct
	{
	dslStatusCode					code;
	union
		{
		long						value;
		dslErrorCode				error;
		struct
			{
			atmStatusCode			code;
			union
				{
				long				value;
				dslErrorCode		error;
				struct
					{
					long	vci;
					long	mid;
					long	aalType;
					long	length;
					uchar	*framePtr;
					} frame;
				struct
					{
					long	nFrames;
					long	nFrameErrors;
					} statistic;
				struct
					{
					long	vcId;
					long	vci;
					long	aalType;
					ulong   fwdPeakCellTime;
					ulong   backPeakCellTime;
					} vcInfo;
				struct
					{
					long	cellHdr;
					long	oamCmd;
					} oamInfo;
				struct 
					{
					void	*pVc;
					char	*pHdr;
					void	*cellHdr;
					void	*cellData;
					} cellInfo;
				struct 
					{
					long	totalBits;
					long	errBits;
					} bertInfo;
				} param;			
			} atmStatus;
#ifdef DSL_PACKET
		dslFramerStatus		dslPacketStatus;
#endif
#ifdef G997_1_FRAMER
		dslFramerStatus		g997Status;
#endif
		struct
			{
			dslTrainingStatusCode		code;
			long						value;
			} dslTrainingInfo;
		struct
			{
			dslConnectInfoStatusCode	code;
			long						value;
			void						*buffPtr;
			} dslConnectInfo;
		struct
			{
			long						maxMarginCarrier;
			long						maxSNRMargin;
			long						minMarginCarrier;
			long						minSNRMargin;
			long						avgSNRMargin;
			long						nCarriers;
			void						*buffPtr;
			} dslShowtimeSNRMarginInfo;
		struct 
			{
			long						code;
			long						vcId;
			long						timeStamp;
			} dslFrameInfo;
#ifdef G997_1
		struct
			{
			long	msgId;
			long	msgType;
			char	*dataPtr;
			} dslClearEocMsg;
#endif
		struct
			{
			char	*fmt;
			long	argNum;
			void	*argPtr;
			} dslPrintfMsg;
		struct
			{
			ulong	nBits;
			ulong	nBlocks;
			ulong	nBitErrors;
			ulong	nBlockErrors;

			ulong	nAudioBits;
			ulong	nAudioBlocks;
			ulong	nAudioSyncErrors;
			ulong	nAudioBlockErrors;
			} testResults;
		struct
			{
			ulong	code;
			uchar	*valuePtr;
			ulong	length;
			} dslDataRegister;
		struct
			{
			ulong	code;
			char	*desc;
			} dslExternalError;
		struct
			{
			ulong	numberOfCalls;
			ulong	txSignalChecksum;
			ulong	rxSignalChecksum;
			ulong	eyeDataChecksum;
			} checksums;
		struct
			{
			int		sp;
			int		argc;
			int		*argv;
			int		*stackPtr;
			int		stackLen;
			} dslException;
		struct
			{
			ulong	paramId;
			void	*dataPtr;
			ulong	dataLen;
			} dslOemParameter;
		struct
			{
			void	*dataPtr;
			ulong	dataLen;
			} dslDataAvail;
		dslOLRMessage		dslOLRRequest;
		dslPwrMessage		dslPwrMsg;
		} param;
	} dslStatusStruct;
	
typedef	void	(SM_DECL *dslStatusHandlerType)		(void *gDslVars, dslStatusStruct*);


/*
**
**		Command codes 
**
*/

typedef long						dslCommandCode;
#define	kFirstDslCommandCode		256
#define kDslIdleCmd					(kFirstDslCommandCode + 0)
#define kDslIdleRcvCmd				(kFirstDslCommandCode + 1)
#define kDslIdleXmtCmd				(kFirstDslCommandCode + 2)
#define	kDslStartPhysicalLayerCmd	(kFirstDslCommandCode + 3)
#define	kDslStartRetrainCmd			(kFirstDslCommandCode + 4)
#define	kDslSetFrameFunctions		(kFirstDslCommandCode + 5)
#define kDslSendEocCommand			(kFirstDslCommandCode + 6)
#define kDslWriteRemoteRegister		(kFirstDslCommandCode + 7)
#define kDslReadRemoteRegister		(kFirstDslCommandCode + 8)
#define kDslWriteLocalRegister		(kFirstDslCommandCode + 9)
#define kDslReadLocalRegister		(kFirstDslCommandCode + 10)
#define	kDslStoreHardwareAGCCmd		(kFirstDslCommandCode + 11)
#define kDslSetCommandHandlerCmd	(kFirstDslCommandCode + 12)
#define kSetLinkLayerStatusHandlerCmd (kFirstDslCommandCode + 13)
#define kDslSetG997Cmd				(kFirstDslCommandCode + 14)
#define kDslLoopbackCmd				(kFirstDslCommandCode + 15)
#define kDslDiagSetupCmd			(kFirstDslCommandCode + 16)
#define kDslSetDriverCallbackCmd	(kFirstDslCommandCode + 17)
#define kDslDiagStopLogCmd			(kFirstDslCommandCode + 18)
#define kDslDiagStartBERT			(kFirstDslCommandCode + 19)
#define kDslDiagStopBERT			(kFirstDslCommandCode + 20)
#define kDslPingCmd					(kFirstDslCommandCode + 21)
#define kDslDyingGaspCmd			(kFirstDslCommandCode + 22)
#define kDslTestCmd					(kFirstDslCommandCode + 23)
#define kDslFilterSNRMarginCmd		(kFirstDslCommandCode + 24)
#define kDslAtmVcMapTableChanged	(kFirstDslCommandCode + 25)
#define	kDslGetOemDataAddrCmd		(kFirstDslCommandCode + 26)
#define kDslAtmReportHEC			(kFirstDslCommandCode + 27)
#define kDslAtmReportCD				(kFirstDslCommandCode + 28)
#define kDslSetXmtGainCmd			(kFirstDslCommandCode + 29)
#define kDslSetStatusBufferCmd		(kFirstDslCommandCode + 30)
#define kDslAfeTestCmd				(kFirstDslCommandCode + 31)
#define kDslI432ResetCmd			(kFirstDslCommandCode + 32)
#define kDslSetRcvGainCmd           (kFirstDslCommandCode + 33)
#define kDslBypassRcvHpfCmd         (kFirstDslCommandCode + 34)
#define kDslWriteAfeRegCmd          (kFirstDslCommandCode + 35)
#define kDslReadAfeRegCmd           (kFirstDslCommandCode + 36)
#define	kDslOLRRequestCmd			(kFirstDslCommandCode + 37)
#define	kDslOLRResponseCmd			(kFirstDslCommandCode + 38)
#define kDslI432SetScrambleCmd		(kFirstDslCommandCode + 39)
#define	kDslPwrMgrCmd				(kFirstDslCommandCode + 40)
#define kDslAtmGfcMappingCmd		(kFirstDslCommandCode + 41)

#ifdef BCM6348_SRC
#define kDslEnablePwmSyncClk        (kFirstDslCommandCode + 42)
#define kDslSetPwmSyncClkFreq       (kFirstDslCommandCode + 43)
#endif

#define	kDslSetG994p1T1p413SwitchTimerCmd	(kFirstDslCommandCode + 44)
#define	kDslPLNControlCmd			(kFirstDslCommandCode + 45)

#define kDslSetDigEcShowtimeUpdateModeFast  (0)
#define kDslSetDigEcShowtimeUpdateModeSlow  (1)
#define kDslSetDigEcShowtimeUpdateModeCmd   (kFirstDslCommandCode + 46)

#define	kDslAtmVcControlCmd			(kFirstDslCommandCode + 47)
  #define kDslAtmVcClear			0
  #define kDslAtmVcAddEntry			(1 << 24)
  #define kDslAtmVcDeleteEntry		(2 << 24)
  #define kDslAtmSetMaxSDU			(3 << 24)
  #define kDslAtmEopMonitorEnable	(4 << 24)
  #define kDslAtmEopMonitorDisable	(5 << 24)
  #define kDslAtmReportVcTable		(6 << 24)
  #define kDslAtmSetFastPortId		(10 << 24)
  #define kDslAtmSetIntlPortId		(11 << 24)
#define kDslI432SetRxHeaderHandler	(kFirstDslCommandCode + 48)
#define kDslProfileControlCmd		(kFirstDslCommandCode + 49)
  #define kDslProfileEnable			1
  #define kDslProfileDisable		0
#define kDslAfelbTestCmd            (kFirstDslCommandCode + 50)
#define kDslTestQuietCmd            (kFirstDslCommandCode + 51)
#define kDslTestQLNTimeCmd          (kFirstDslCommandCode + 52)
#define	kG994p1Duplex								1
#define	kG994p1HalfDuplex  							2

/* Eoc Messages from ATU-C to ATU-R */
#define kDslEocHoldStateCmd						1
#define kDslEocReturnToNormalCmd				2
#define kDslEocPerformSelfTestCmd				3
#define kDslEocRequestCorruptCRCCmd				4
#define kDslEocRequestEndCorruptCRCCmd			5
#define kDslEocNotifyCorruptCRCCmd				6
#define kDslEocNotifyEndCorruptCRCCmd			7
#define kDslEocRequestTestParametersUpdateCmd	8
#define kDslEocGrantPowerDownCmd				9
#define kDslEocRejectPowerDownCmd				10

/* Eoc Messages  from ATU-R to ATU-C */
#define kDslEocRequestPowerDownCmd				11
#define kDslEocDyingGaspCmd						12

/* Clear Eoc Messages  */
#define kDslClearEocFirstCmd					100
#define kDslClearEocSendFrame					100
#define kDslClearEocSendComplete				101
#define kDslClearEocRcvedFrame					102
#define kDslClearEocSendComplete2				103

#define kDslClearEocMsgLengthMask				0x0000FFFF
#define kDslClearEocMsgNumMask					0x00FF0000
#define kDslClearEocMsgDataVolatileMask			0x01000000
#define kDslClearEocMsgDataVolatile				kDslClearEocMsgDataVolatileMask
#define kDslClearEocMsgExtraSendComplete		0x02000000

/* General status messages (using clearEOC message structure) */
#define kDslGeneralMsgStart						300
#define kDslGeneralMsgDbgDataPrint				300
#define kDslGeneralMsgDbgPrintf					301
#define kDslGeneralMsgDbgPrintG992p3Cap			303
#define kDslGeneralMsgDbgProfData				304

/* General kDslGeneralMsgDbgDataPrint flags */
#define kDslDbgDataSizeMask						0x00030000
#define kDslDbgDataSize8						0x00000000
#define kDslDbgDataSize16						0x00010000
#define kDslDbgDataSize32						0x00020000
#define kDslDbgDataSize64						0x00030000

#define kDslDbgDataSignMask						0x00040000
#define kDslDbgDataSigned						0x00040000
#define kDslDbgDataUnsigned						0x00000000

#define kDslDbgDataFormatMask					0x00080000
#define kDslDbgDataFormatHex					0x00080000
#define kDslDbgDataFormatDec					0x00000000

#define kDslDbgDataQxShift						20
#define kDslDbgDataQxMask						(0xF << kDslDbgDataQxShift)
#define kDslDbgDataQ0							0x00000000
#define kDslDbgDataQ1							(1 << kDslDbgDataQxShift)
#define kDslDbgDataQ4							(4 << kDslDbgDataQxShift)
#define kDslDbgDataQ8							(8 << kDslDbgDataQxShift)
#define kDslDbgDataQ15							(0xF << kDslDbgDataQxShift)

/* General kDslGeneralMsgDbgPrintf flags */
#define kDslDbgDataPrintfIdMask					0x00FF0000
#define kDslDbgDataPrintfIdShift				16


/* ADSL Link Power States */
#define kDslPowerFullOn						0
#define kDslPowerLow						1
#define kDslPowerIdle						3

/* ATU-R Data Registers */
#define kDslVendorIDRegister				1
#define kDslRevisionNumberRegister			2
#define kDslSerialNumberRegister			3
#define kDslSelfTestResultsRegister			4
#define kDslLineAttenuationRegister			5
#define kDslSnrMarginRegister				6
#define kDslAturConfigurationRegister		7
#define kDslLinkStateRegister				8

#define kDslVendorIDRegisterLength			8
#define kDslRevisionNumberRegisterLength	32
#define kDslSerialNumberRegisterLength		32
#define kDslSelfTestResultsRegisterLength	1
#define kDslLineAttenuationRegisterLength	1
#define kDslSnrMarginRegisterLength			1
#define kDslAturConfigurationRegisterLength	30
#define kDslLinkStateRegisterLength			1

/* Dsl Diags setup flags */
#define kDslDiagEnableEyeData				1
#define kDslDiagEnableLogData				2
#define kDslDiagEnableDebugData				4

/* Dsl test commands */
typedef	long								dslTestCmdType;
#define	kDslTestBackToNormal				0
#define kDslTestReverb						1
#define kDslTestMedley						2
#define kDslTestToneSelection				3
#define	kDslTestNoAutoRetrain				4
#define	kDslTestMarginTweak					5
#define kDslTestEstimatePllPhase            6
#define kDslTestReportPllPhaseStatus        7
#define kDslTestAfeLoopback					8
#define kDslTestL3							9
#define kDslTestAdsl2DiagMode				10
#define kDslTestRetL0						11
#define kDslTestExecuteDelay                              12
#define kDslTestQuiet   					13

/* Xmt gain default setting */
#define	kDslXmtGainAuto						0x80000000

/* Unit (AFE) test commands */
#define	kDslAfeTestLoadImage				0
#define	kDslAfeTestPatternSend				1
#define	kDslAfeTestLoadImageOnly			2
#define	kDslAfeTestPhyRun					3
#define	kDslAfeTestLoadBuffer				4

/* kDslPLNControlCmd sub-commands */

#define	kDslPLNControlStart					1
#define	kDslPLNControlStop					2
#define	kDslPLNControlClear					3
#define	kDslPLNControlPeakNoiseGetPtr			4
#define kDslPLNControlThldViolationGetPtr		5
#define	kDslPLNControlImpulseNoiseEventGetPtr	6
#define	kDslPLNControlImpulseNoiseTimeGetPtr	7
#define kDslPLNControlGetStatus                         8
#define kDslPLNControlDefineInpBinTable                 9
#define kDslPLNControlDefineItaBinTable                 10
#define kDslPLNControlDefineDefaultBinTables            11

/* PLN constants */
#define kPlnNumberOfDurationBins       32
#define kPlnNumberOfInterArrivalBins   16
#define	kPlnNumberOfRcvErrMonitorCarrPerSymb  12	/* Number of monitored carriers per symbol to calculate error power (PLN) */

typedef struct
	{
#if defined(G992P1_ANNEX_I) || defined(G992P5) || defined(G993) 
	ushort 				downstreamMinCarr, downstreamMaxCarr;
#else
	uchar 				downstreamMinCarr, downstreamMaxCarr;
#endif
#if !defined(G993)
	uchar           	upstreamMinCarr, upstreamMaxCarr;
#else
	ushort           	upstreamMinCarr, upstreamMaxCarr;
#endif
	}carrierInfo;

#if defined(G992P3) && !defined(BCM6348_SRC) 
#define	FAST_TEXT_TYPE
#else
#define	FAST_TEXT_TYPE			FAST_TEXT
#endif

#if defined(BCM6348_SRC) 
#define BCM6348_TEMP_MOVE_TO_LMEM 
#else
#define BCM6348_TEMP_MOVE_TO_LMEM
#endif

#ifdef	G992P3
#undef	PRINT_DEBUG_INFO
#else
#define	PRINT_DEBUG_INFO
#endif

#ifdef G992P3

#define		kG992p3MaxSpectBoundsUpSize		16
#define		kG992p3MaxSpectBoundsDownSize	16

/* G.994 definitions */

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 1 ***/

#define	kG994p1G992p3AnnexASpectrumBoundsUpstream	0x01
#define	kG994p1G992p3AnnexASpectrumShapingUpstream	0x02
#define	kG994p1G992p3AnnexASpectrumBoundsDownstream	0x04
#define	kG994p1G992p3AnnexASpectrumShapingDownstream	0x08
#define	kG994p1G992p3AnnexATxImageAboveNyquistFreq	0x10
#define	kG994p1G992p3AnnexLReachExtended			0x20
#define kG994p1G992p3AnnexMSubModePSDMasks          0x20

#define	kG994p1G992p3AnnexLUpNarrowband				0x02
#define	kG994p1G992p3AnnexLUpWideband				0x01
#define	kG994p1G992p3AnnexLDownNonoverlap			0x01

#define kG994p1G992pNAnnexMUpAdlu32                             0x001
#define kG994p1G992pNAnnexMUpAdlu36                             0x002
#define kG994p1G992pNAnnexMUpAdlu40                             0x004
#define kG994p1G992pNAnnexMUpAdlu44                             0x008
#define kG994p1G992pNAnnexMUpAdlu48                             0x010
#define kG994p1G992pNAnnexMUpAdlu52                             0x020
#define kG994p1G992pNAnnexMUpAdlu56                             0x040
#define kG994p1G992pNAnnexMUpAdlu60                             0x080
#define kG994p1G992pNAnnexMUpAdlu64                             0x100

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 2 ***/

#define	kG994p1G992p3AnnexADownOverheadDataRate		0x01
#define	kG994p1G992p3AnnexAUpOverheadDataRate		0x02
#define	kG994p1G992p3AnnexAMaxNumberDownTPSTC		0x04
#define	kG994p1G992p3AnnexAMaxNumberUpTPSTC			0x08

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 3,5,7,9 ***/

#define	kG994p1G992p3AnnexADownSTM_TPS_TC			0x01
#define	kG994p1G992p3AnnexAUpSTM_TPS_TC				0x02
#define	kG994p1G992p3AnnexADownATM_TPS_TC			0x04
#define	kG994p1G992p3AnnexAUpATM_TPS_TC				0x08
#define	kG994p1G992p3AnnexADownPTM_TPS_TC			0x10
#define	kG994p1G992p3AnnexAUpPTM_TPS_TC				0x20

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 4,6,8,10 ***/

#define	kG994p1G992p3AnnexADownPMS_TC_Latency		0x01
#define	kG994p1G992p3AnnexAUpPMS_TC_Latency			0x02

/*** 
 *   TSSI
 *
 *   TSSI information is specified in 2 parts: subcarrier index,
 *   tssi value, and an indication of whether or no the tone specified
 *   is part of the supported set.
 *
 *   The subcarrier index information is currently stored in the
 *   dsSubcarrierIndex array defined below. The tssi value are stored
 *   in the dsLog_tss array.
 *
 *   The subcarrier index information only occupies the lower 12 bits
 *   of the available 16 bits (short type). Therefore, we will pack the
 *   supported set information in bit 15.
 */
#define kG992DsSubCarrierIndexMask          (0x0fff)   /* AND mask to ectract ds subcarrier index */
#define kG992DsSubCarrierSuppSetMask        (0x8000)   /* AND mask to extract supported set indication */

#define G992GetDsSubCarrierIndex(arg)               ((arg)  & kG992DsSubCarrierIndexMask)
#define G992GetDsSubCarrierSuppSetIndication(arg)   (((arg) & kG992DsSubCarrierSuppSetMask) >> 15)

/* Caution: Do not change anything in this structure definition, including associated constant */
/* This structure definition is used only by the driver and any change impose incompatibility issue in driver */
/* The structure following this structure (g992p3PhyDataPumpCapabilities) can be changed in PHY application */

typedef struct
	{
	Boolean				rcvNTREnabled, shortInitEnabled, diagnosticsModeEnabled;
	
	char				featureSpectrum, featureOverhead;
	char				featureTPS_TC[4], featurePMS_TC[4];
	
	short				rcvNOMPSDus, rcvMAXNOMPSDus, rcvMAXNOMATPus;
	short				usSubcarrierIndex[kG992p3MaxSpectBoundsUpSize],
						usLog_tss[kG992p3MaxSpectBoundsUpSize];
	short				numUsSubcarrier;
	short				rcvNOMPSDds, rcvMAXNOMPSDds, rcvMAXNOMATPds;
	short				dsSubcarrierIndex[kG992p3MaxSpectBoundsDownSize],
						dsLog_tss[kG992p3MaxSpectBoundsDownSize];
	short				numDsSubcarrier;
	uchar				sizeIDFT, fillIFFT;
	uchar				readsl2Upstream, readsl2Downstream;
	uchar				minDownOverheadDataRate, minUpOverheadDataRate;
	uchar				maxDownSTM_TPSTC, maxDownATM_TPSTC, maxDownPTM_TPSTC;
	uchar				maxUpSTM_TPSTC, maxUpATM_TPSTC, maxUpPTM_TPSTC;

	short				minDownSTM_TPS_TC[4], maxDownSTM_TPS_TC[4],
						minRevDownSTM_TPS_TC[4], maxDelayDownSTM_TPS_TC[4];
	uchar				maxErrorDownSTM_TPS_TC[4], minINPDownSTM_TPS_TC[4];
	short				minUpSTM_TPS_TC[4], maxUpSTM_TPS_TC[4],
						minRevUpSTM_TPS_TC[4], maxDelayUpSTM_TPS_TC[4];
	uchar				maxErrorUpSTM_TPS_TC[4], minINPUpSTM_TPS_TC[4];

	short				maxDownPMS_TC_Latency[4], maxUpPMS_TC_Latency[4];
	short				maxDownR_PMS_TC_Latency[4], maxDownD_PMS_TC_Latency[4];
	short				maxUpR_PMS_TC_Latency[4], maxUpD_PMS_TC_Latency[4];

	short				minDownATM_TPS_TC[4], maxDownATM_TPS_TC[4],
						minRevDownATM_TPS_TC[4], maxDelayDownATM_TPS_TC[4];
	uchar				maxErrorDownATM_TPS_TC[4], minINPDownATM_TPS_TC[4];
	short				minUpATM_TPS_TC[4], maxUpATM_TPS_TC[4],
						minRevUpATM_TPS_TC[4], maxDelayUpATM_TPS_TC[4];
	uchar				maxErrorUpATM_TPS_TC[4], minINPUpATM_TPS_TC[4];

	short				minDownPTM_TPS_TC[4], maxDownPTM_TPS_TC[4],
						minRevDownPTM_TPS_TC[4], maxDelayDownPTM_TPS_TC[4];
	uchar				maxErrorDownPTM_TPS_TC[4], minINPDownPTM_TPS_TC[4];
	short				minUpPTM_TPS_TC[4], maxUpPTM_TPS_TC[4],
						minRevUpPTM_TPS_TC[4], maxDelayUpPTM_TPS_TC[4];
	uchar				maxErrorUpPTM_TPS_TC[4], minINPUpPTM_TPS_TC[4];

    ushort              subModePSDMasks;
	} g992p3DataPumpCapabilities;

#define		kG992p3p5MaxSpectBoundsUpSize		16
#define		kG992p3p5MaxSpectBoundsDownSize		32

typedef struct
	{
	Boolean				rcvNTREnabled, shortInitEnabled, diagnosticsModeEnabled;
	
	char				featureSpectrum, featureOverhead;
	char				featureTPS_TC[4], featurePMS_TC[4];
	
	short				rcvNOMPSDus, rcvMAXNOMPSDus, rcvMAXNOMATPus;
	short				usSubcarrierIndex[kG992p3p5MaxSpectBoundsUpSize],
						usLog_tss[kG992p3p5MaxSpectBoundsUpSize];
	short				numUsSubcarrier;
	short				rcvNOMPSDds, rcvMAXNOMPSDds, rcvMAXNOMATPds;
	short				dsSubcarrierIndex[kG992p3p5MaxSpectBoundsDownSize],
						dsLog_tss[kG992p3p5MaxSpectBoundsDownSize];
	short				numDsSubcarrier;
	uchar				sizeIDFT, fillIFFT;
	uchar				readsl2Upstream, readsl2Downstream;
	uchar				minDownOverheadDataRate, minUpOverheadDataRate;
	uchar				maxDownSTM_TPSTC, maxDownATM_TPSTC, maxDownPTM_TPSTC;
	uchar				maxUpSTM_TPSTC, maxUpATM_TPSTC, maxUpPTM_TPSTC;

	short				minDownSTM_TPS_TC[4], maxDownSTM_TPS_TC[4],
						minRevDownSTM_TPS_TC[4], maxDelayDownSTM_TPS_TC[4];
	uchar				maxErrorDownSTM_TPS_TC[4], minINPDownSTM_TPS_TC[4];
	short				minUpSTM_TPS_TC[4], maxUpSTM_TPS_TC[4],
						minRevUpSTM_TPS_TC[4], maxDelayUpSTM_TPS_TC[4];
	uchar				maxErrorUpSTM_TPS_TC[4], minINPUpSTM_TPS_TC[4];

	short				maxDownPMS_TC_Latency[4], maxUpPMS_TC_Latency[4];
	short				maxDownR_PMS_TC_Latency[4], maxDownD_PMS_TC_Latency[4];
	short				maxUpR_PMS_TC_Latency[4], maxUpD_PMS_TC_Latency[4];

	short				minDownATM_TPS_TC[4], maxDownATM_TPS_TC[4],
						minRevDownATM_TPS_TC[4], maxDelayDownATM_TPS_TC[4];
	uchar				maxErrorDownATM_TPS_TC[4], minINPDownATM_TPS_TC[4];
	short				minUpATM_TPS_TC[4], maxUpATM_TPS_TC[4],
						minRevUpATM_TPS_TC[4], maxDelayUpATM_TPS_TC[4];
	uchar				maxErrorUpATM_TPS_TC[4], minINPUpATM_TPS_TC[4];

	short				minDownPTM_TPS_TC[4], maxDownPTM_TPS_TC[4],
						minRevDownPTM_TPS_TC[4], maxDelayDownPTM_TPS_TC[4];
	uchar				maxErrorDownPTM_TPS_TC[4], minINPDownPTM_TPS_TC[4];
	short				minUpPTM_TPS_TC[4], maxUpPTM_TPS_TC[4],
						minRevUpPTM_TPS_TC[4], maxDelayUpPTM_TPS_TC[4];
	uchar				maxErrorUpPTM_TPS_TC[4], minINPUpPTM_TPS_TC[4];

    ushort              subModePSDMasks;
	} g992p3PhyDataPumpCapabilities;

#ifdef FIRE_RETRANSMISSION
typedef struct
	{
        uchar                           version;
        uchar                           setting;
        uchar                           halfRoundTripAtur, halfRoundTripAtuc;
#if 0
        uchar                           availableXmtBufferUS;
        uchar                           INPmaxUS;
        uchar                           minReXmtRateUS;
        uchar                           minRsOverheadUS;
#endif
        uchar                           availableXmtBufferDS;
        uchar                           INPmaxDS;
        uchar                           minReXmtRateDS;
        uchar                           minRsOverheadDS;
        } g992FireSpecifications;
#endif /*FIRE_REXmt */

#endif /* G992p3/G992p5 */

/* ADD - VDSL2 code ---- */

#ifdef G993

/* Caution: Do not change anything in this structure definition, including associated constant */
/* This structure definition is used only by the driver and any change impose incompatibility issue in driver */
/* The structure following this structure (g992p3PhyDataPumpCapabilities) can be changed in PHY application */

#define NbandsSupport 5 /* FIXME - This is not the right place for the define - move to correct place */
#define		kG993p2MaxSpectBoundsUpSize		16 /* FIXME - WHat is this value? */
#define		kG993p2MaxSpectBoundsDownSize		32

typedef struct
	{
	Boolean				ADL, dsVirtualNoise, lineProbe, ldMode;
	
	Boolean				profiles, bandsUs, bandsDs, bandsRFI, sizeIDFT, CElen, A_US0, B_US0, C_US0;
	Boolean 			profile8a, profile8b, profile8c, profile8d, profile12a, profile12b, profile17a, profile30a ; 
	ushort				usSubCarrierIndex[2*NbandsSupport], dsSubCarrierIndex[2*NbandsSupport], rfiSubCarrierIndex[2*NbandsSupport];
	/*
	short				usSubcarrierIndex[kG993p2MaxSpectBoundsUpSize],
	short				dsSubcarrierIndex[kG993p2MaxSpectBoundsDownSize],
	short				dsSubcarrierIndex[kG993p2MaxSpectBoundsDownSize],
	*/

	short				numUsSubcarrier;
	short				numDsSubcarrier;
	
	uchar				IDFTsize ; 
	ushort 				initialIDFTsize , fillIFFT; 

	uchar 				ceLen_m[15] ; 

	Boolean 			EU32, EU36, EU40, EU44, EU48, EU52, EU56, EU60, EU64 ; 
	Boolean 			ADLU32, ADLU36, ADLU40, ADLU44, ADLU48, ADLU52, ADLU56, ADLU60, ADLU64 ; 
	Boolean 			A_US012b, A_US017a ; 
	Boolean 			B_US0_Amask, B_US0_Bmask, B_US0_Mmask , B_US012b, B_US017a ; 

    ushort              subModePSDMasks;
	} g993p2DataPumpCapabilities;


typedef struct
	{
	Boolean				ADL, dsVirtualNoise, lineProbe, ldMode;
	
	Boolean				profiles, bandsUs, bandsDs, bandsRFI, sizeIDFT, CElen, A_US0, B_US0, C_US0;
	Boolean 			profile8a, profile8b, profile8c, profile8d, profile12a, profile12b, profile17a, profile30a ; 
	ushort				usSubcarrierIndex[2*NbandsSupport], dsSubcarrierIndex[2*NbandsSupport], rfiSubcarrierIndex[2*NbandsSupport];
	/*
	short				usSubcarrierIndex[kG993p2MaxSpectBoundsUpSize],
	short				dsSubcarrierIndex[kG993p2MaxSpectBoundsDownSize],
	short				dsSubcarrierIndex[kG993p2MaxSpectBoundsDownSize],
	*/

	short				numUsSubcarrier;
	short				numDsSubcarrier;
	
	uchar 				IDFTsize ; 
	ushort				initialIDFTsize, fillIFFT;

	uchar 				ceLen_m[15] ; 

	Boolean 			EU32, EU36, EU40, EU44, EU48, EU52, EU56, EU60, EU64 ; 
	Boolean 			ADLU32, ADLU36, ADLU40, ADLU44, ADLU48, ADLU52, ADLU56, ADLU60, ADLU64 ; 
	Boolean 			A_US012b, A_US017a ; 
	Boolean 			B_US0_Amask, B_US0_Bmask, B_US0_Mmask , B_US012b, B_US017a ; 

    ushort              subModePSDMasks;
	} g993p2PhyDataPumpCapabilities;
#endif

/* end ADD for VDSL2 code */

typedef struct
	{
	dslModulationType	modulations;
	bitMap				auxFeatures;
	bitMap          	features;	
	bitMap				demodCapabilities;
	bitMap				demodCapabilities2;
	ushort				noiseMargin;		/* Q4 dB */
#ifdef G992_ATUC
	short				xmtRSf, xmtRS, xmtS, xmtD;
	short				rcvRSf, rcvRS, rcvS, rcvD;
#endif	
#if defined(G993)	 /* VDSL2 */
	carrierInfo     	carrierInfoG993p2;
#endif /* END VDSL2 */
#ifdef G992P1_ANNEX_A	
	bitMap          	subChannelInfo;
	carrierInfo     	carrierInfoG992p1;
#endif
#ifdef G992P1_ANNEX_B	
	bitMap          	subChannelInfoAnnexB;
	carrierInfo     	carrierInfoG992p1AnnexB;
#endif
#ifdef G992_ANNEXC	
	bitMap          	subChannelInfoAnnexC;
	carrierInfo     	carrierInfoG992p1AnnexC;
#endif
#if defined(G992P1_ANNEX_I)
	bitMap          	subChannelInfoAnnexI;
	carrierInfo     	carrierInfoG992p1AnnexI;
#endif
#ifdef G992P5
	bitMap          	subChannelInfop5;
	carrierInfo     	carrierInfoG992p5;
#endif
#if defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
	carrierInfo     	carrierInfoG992p2;
#endif
	ushort           	maxDataRate;
	uchar           	minDataRate;	     
#ifdef G992P3
	g992p3DataPumpCapabilities	*carrierInfoG992p3AnnexA;
#endif
#ifdef G992P5
	g992p3DataPumpCapabilities	*carrierInfoG992p5AnnexA;
#endif
#ifdef G993 
	g993p2DataPumpCapabilities	*carrierInfoG993p2AnnexA;
#endif
	} dslDataPumpCapabilities;
		
struct __dslCommandStruct;
typedef	Boolean	(*dslCommandHandlerType)	(void *gDslVars, struct __dslCommandStruct*);
typedef	struct __dslCommandStruct
	{
	dslCommandCode						command;
	union
		{
		long							value;
		Boolean							flag;
		struct
			{
			dslTestCmdType				type;
			union
				{
				struct
					{
					ulong				xmtStartTone, xmtNumOfTones;
					ulong				rcvStartTone, rcvNumOfTones;
					uchar				*xmtMap, *rcvMap;
					} toneSelectSpec;
				struct
					{
					long				extraPowerRequestQ4dB;
					long				numOfCarriers;
					char				*marginTweakTableQ4dB;
					} marginTweakSpec;
				ulong                                   value;
				} param;
			} dslTestSpec;
		struct
			{
			dslDirectionType			direction;			
			dslDataPumpCapabilities		capabilities;
			} dslModeSpec;
		struct
			{
			bitMap						setup;			
			ulong						eyeConstIndex1;
			ulong						eyeConstIndex2;
			ulong						logTime;
			} dslDiagSpec;
		struct
			{
			void						*pBuf;			
			ulong						bufSize;
			} dslStatusBufSpec;
		struct
			{
			ulong						type;
			void						*afeParamPtr;
			ulong						afeParamSize;
			void						*imagePtr;
			ulong						imageSize;
			} dslAfeTestSpec;
		struct
			{
			ulong						plnCmd;
			ulong						mgnDescreaseLevelPerBin;			
			ulong						mgnDescreaseLevelBand;
			ulong						nInpBin;
			ushort						*inpBinPtr;
			ulong						nItaBin;
			ushort						*itaBinPtr;
			} dslPlnSpec;
		struct
			{
			dslLinkLayerType			type;
			bitMap						setup;
			union
				{
				struct
					{
					dataRateMap					rxDataRate;
					dataRateMap					txDataRate;
					long						rtDelayQ4ms;				
					ulong						rxBufNum;
					ulong						rxCellsInBuf;
					ulong						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPhyInitType				atmPhyInitPtr;
					} atmLinkSpec;
				struct
					{
					dslHeaderHandlerType		rxHeaderHandlerPtr;
					dslRxFrameBufferHandlerType	rxDataHandlerPtr;
					dslTxFrameBufferHandlerType txHandlerPtr;
					} atmPhyLinkSpec;
#ifdef DSL_PACKET
				struct
					{
					ulong						rxBufNum;
					ulong						rxBufSize;
					ulong						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPacketPhyInitType		dslPhyInitPtr;
					} dslPacketLinkSpec;
				dslPacketPhyFunctions			dslPacketPhyLinkSpec;
#endif
				struct
					{
					txDataHandlerType			txDataHandlerPtr;
					rxDataHandlerType			rxDataHandlerPtr;
					} nullLinkSpec;
				} param;
			} dslLinkLayerSpec;
#ifdef G997_1
#ifdef G997_1_FRAMER
		struct
			{
			bitMap						setup;
			ulong						rxBufNum;
			ulong						rxBufSize;
			ulong						rxPacketNum;
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType 		txCompleteHandlerPtr;
			} dslG997Cmd;
#endif
		struct
			{
			long	msgId;
			long	msgType;
			char	*dataPtr;
			} dslClearEocMsg;
#endif
		struct
			{
			ulong						code;
			uchar						*valuePtr;
			ulong						length;
			} dslDataRegister;
		union
			{
			dslStatusHandlerType		statusHandlerPtr;
			dslCommandHandlerType		commandHandlerPtr;
			eyeHandlerType				eyeHandlerPtr;
			logHandlerType				logHandlerPtr;
#if defined(DEBUG_DATA_HANDLER)
            debugDataHandlerType        debugDataHandlerPtr;
#endif
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType			txCompleteHandlerPtr;
			dslDriverCallbackType		driverCallback;
			} handlerSpec;
#if !defined(CHIP_SRC) || defined(DSL_FRAME_FUNCTIONS)
		dslFrameFunctions				DslFunctions;
#endif
		dslOLRMessage					dslOLRRequest;
		dslPwrMessage					dslPwrMsg;
		} param;
	} dslCommandStruct;
	



typedef struct
	{
	dslCommandHandlerType			linkCommandHandlerPtr;
	timerHandlerType				linkTimerHandlerPtr;
	dslLinkCloseHandlerType			linkCloseHandlerPtr;

	dslFrameHandlerType				linkSendHandlerPtr;
	dslFrameHandlerType				linkReturnHandlerPtr;

	dslVcAllocateHandlerType		linkVcAllocateHandlerPtr;
	dslVcFreeHandlerType			linkVcFreeHandlerPtr;
	dslVcActivateHandlerType		linkVcActivateHandlerPtr;
	dslVcDeactivateHandlerType		linkVcDeactivateHandlerPtr;
	dslVcConfigureHandlerType		linkVcConfigureHandlerPtr;

	dslLinkVc2IdHandlerType			linkVc2IdHandlerPtr;
	dslLinkVcId2VcHandlerType		linkVcId2VcHandlerPtr;
	dslGetFramePoolHandlerType		linkGetFramePoolHandlerPtr;

#ifndef ADSLCORE_ONLY
	dslHeaderHandlerType			linkRxCellHeaderHandlerPtr;
	dslRxFrameBufferHandlerType		linkRxCellDataHandlerPtr;
	dslTxFrameBufferHandlerType		linkTxCellHandlerPtr;
#endif

	txDataHandlerType				linkTxDataHandlerPtr;
	rxDataHandlerType				linkRxDataHandlerPtr;
	} linkLayerFunctions;

#ifndef ADSLCORE_ONLY

#define	 LinkLayerAssignFunctions( var, name_prefix )	do {					\
	(var).linkCommandHandlerPtr = name_prefix##CommandHandler;					\
	(var).linkTimerHandlerPtr	= name_prefix##TimerHandler;					\
	(var).linkCloseHandlerPtr	= name_prefix##CloseHandler;					\
																				\
	(var).linkSendHandlerPtr	= name_prefix##SendFrameHandler;				\
	(var).linkReturnHandlerPtr	= name_prefix##ReturnFrameHandler;				\
																				\
	(var).linkVcAllocateHandlerPtr		= name_prefix##VcAllocateHandler;		\
	(var).linkVcFreeHandlerPtr			= name_prefix##VcFreeHandler;			\
	(var).linkVcActivateHandlerPtr		= name_prefix##VcActivateHandler;		\
	(var).linkVcDeactivateHandlerPtr	= name_prefix##VcDeactivateHandler;		\
	(var).linkVcConfigureHandlerPtr		= name_prefix##VcConfigureHandler;		\
																				\
	(var).linkVc2IdHandlerPtr			= name_prefix##Vc2IdHandler;			\
	(var).linkVcId2VcHandlerPtr			= name_prefix##VcId2VcHandler;			\
	(var).linkGetFramePoolHandlerPtr	= name_prefix##GetFramePoolHandler;		\
																				\
	(var).linkRxCellHeaderHandlerPtr	= name_prefix##RxCellHeaderHandler;		\
	(var).linkRxCellDataHandlerPtr		= name_prefix##RxCellDataHandler;		\
	(var).linkTxCellHandlerPtr			= name_prefix##TxCellHandler;			\
																				\
	(var).linkTxDataHandlerPtr	= name_prefix##TxDataHandler;					\
	(var).linkRxDataHandlerPtr	= name_prefix##RxDataHandler;					\
} while (0)

#else

#define	 LinkLayerAssignFunctions( var, name_prefix )	do {					\
	(var).linkCommandHandlerPtr = name_prefix##CommandHandler;					\
	(var).linkTimerHandlerPtr	= name_prefix##TimerHandler;					\
	(var).linkCloseHandlerPtr	= name_prefix##CloseHandler;					\
																				\
	(var).linkSendHandlerPtr	= name_prefix##SendFrameHandler;				\
	(var).linkReturnHandlerPtr	= name_prefix##ReturnFrameHandler;				\
																				\
	(var).linkVcAllocateHandlerPtr		= name_prefix##VcAllocateHandler;		\
	(var).linkVcFreeHandlerPtr			= name_prefix##VcFreeHandler;			\
	(var).linkVcActivateHandlerPtr		= name_prefix##VcActivateHandler;		\
	(var).linkVcDeactivateHandlerPtr	= name_prefix##VcDeactivateHandler;		\
	(var).linkVcConfigureHandlerPtr		= name_prefix##VcConfigureHandler;		\
																				\
	(var).linkVc2IdHandlerPtr			= name_prefix##Vc2IdHandler;			\
	(var).linkVcId2VcHandlerPtr			= name_prefix##VcId2VcHandler;			\
	(var).linkGetFramePoolHandlerPtr	= name_prefix##GetFramePoolHandler;		\
																				\
	(var).linkTxDataHandlerPtr	= name_prefix##TxDataHandler;					\
	(var).linkRxDataHandlerPtr	= name_prefix##RxDataHandler;					\
} while (0)

#endif

typedef struct
	{
	dslFrameHandlerType				rxIndicateHandlerPtr;
	dslFrameHandlerType				txCompleteHandlerPtr;
	dslStatusHandlerType			statusHandlerPtr;
	} upperLayerFunctions;


/*
 * Debug data
 */
#define	kDslFirstDebugData					1000
#define	kDslXmtPerSymTimeCompData			(kDslFirstDebugData + 0)
#define	kDslRcvPerSymTimeCompData			(kDslFirstDebugData + 1)
#define	kDslXmtAccTimeCompData				(kDslFirstDebugData + 2)
#define	kDslRcvAccTimeCompData				(kDslFirstDebugData + 3)
#define	kDslRcvPilotToneData				(kDslFirstDebugData + 4)
#define	kDslTEQCoefData						(kDslFirstDebugData + 5)
#define	kDslTEQInputData					(kDslFirstDebugData + 6)
#define	kDslTEQOutputData					(kDslFirstDebugData + 7)
#define	kDslRcvFFTInputData					(kDslFirstDebugData + 8)
#define	kDslRcvFFTOutputData				(kDslFirstDebugData + 9)
#define	kDslRcvCarrierSNRData				(kDslFirstDebugData + 10)
#define	kDslXmtToneOrderingData				(kDslFirstDebugData + 11)
#define	kDslRcvToneOrderingData				(kDslFirstDebugData + 12)
#define	kDslXmtGainData						(kDslFirstDebugData + 13)
#define	kDslRcvGainData						(kDslFirstDebugData + 14)
#define	kDslMseData							(kDslFirstDebugData + 15)
#define	kDslFEQOutErrData					(kDslFirstDebugData + 16)
#define kDslFEQCoefData 					(kDslFirstDebugData + 17)
#define kDslShowtimeMseData					(kDslFirstDebugData + 18)
#define kDslTimeEstimationHWPhaseTweak		(kDslFirstDebugData + 24)
#define	kDslSlicerInput						(kDslFirstDebugData + 40)
#define	kDslXmtConstellations				(kDslFirstDebugData + 41)
#define kDslSnr1ShiftData					(kDslFirstDebugData + 50)
#define kDslSnr1InputData					(kDslFirstDebugData + 51)
#define kDslSnr1ReverbAvgData				(kDslFirstDebugData + 52)
#define kDslAnnexCFextSnrData				(kDslFirstDebugData + 53)
#define kDslAnnexCNextSnrData				(kDslFirstDebugData + 54)
#define	kG994p1OutputXmtSample				(kDslFirstDebugData + 100)
#define	kG994p1OutputMicroBit				(kDslFirstDebugData + 101)
#define	kG994p1OutputBit					(kDslFirstDebugData + 102)
#define	kG994p1OutputTimer					(kDslFirstDebugData + 103)

/****************************************************************************/
/*	2.	Constant definitions.												*/
/*																			*/
/*	2.1	Defininitive constants												*/
/****************************************************************************/

/* dslDirectionType */

#define	kATU_C		0
#define	kATU_R		1

/* ATM setup maps	*/

#define	kAtmCallMgrEnabled			0x00000001		/* Bit 0  */
#define	kAtmAAL1FecEnabledMask		0x00000006		/* Bit 1  */
#define	kAtmAAL1HiDelayFecEnabled	0x00000002		/* Bit 2  */
#define	kAtmAAL1LoDelayFecEnabled	0x00000004		/* Bit 3  */

/* dslLinkLayerType */

#define kNoDataLink			0
#define kAtmLink			0x00000001
#define kAtmPhyLink			0x00000002
#define kDslPacketLink		0x00000003
#define kDslPacketPhyLink	0x00000004

/* dslModulationType */
#define	kNoCommonModulation	0x00000000				
#define	kG994p1				0x00000020				/* G.994.1 or G.hs */
#define	kT1p413				0x00000040				/* T1.413 handshaking */
#define	kG992p1AnnexA		0x00000001				/* G.992.1 or G.dmt Annex A */
#define	kG992p1AnnexB		0x00000002				/* G.992.1 or G.dmt Annex B */
#define	kG992p1AnnexC		0x00000004				/* G.992.1 or G.dmt Annex C */
#define	kG992p2AnnexAB		0x00000008				/* G.992.2 or G.lite Annex A/B */
#define	kG992p2AnnexC		0x00000010				/* G.992.2 or G.lite Annex C */
#define	kG992p3AnnexA		0x00000100				/* G.992.3 or G.DMTbis Annex A */
#define	kG992p3AnnexB		0x00000200				/* G.992.3 or G.DMTbis Annex A */
#define	kG992p1AnnexI		0x00000400				/* G.992.1 Annex I */
#define kG992p5AnnexA       0x00010000              /* G.992.5 Annex A */
#define kG992p5AnnexB       0x00020000              /* G.992.5 Annex B */
#define kG992p5AnnexI       0x00040000              /* G.992.5 Annex I */
#define kG992p3AnnexM       0x00080000              /* G.992.3 Annex M */
#define kG992p5AnnexM       0x01000000              /* G.992.5 Annex M */
#if defined(G993)
#define kG993p2AnnexA       0x02000000              /* G.993.2 Annex A */
#endif /* END G993 */

/* demodCapabilities bitmap */
#define	kEchoCancellorEnabled					0x00000001
#define	kSoftwareTimeErrorDetectionEnabled		0x00000002
#define	kSoftwareTimeTrackingEnabled			0x00000004
#define kDslTrellisEnabled			            0x00000008
#define	kHardwareTimeTrackingEnabled			0x00000010
#define kHardwareAGCEnabled						0x00000020
#define kDigitalEchoCancellorEnabled			0x00000040
#define kReedSolomonCodingEnabled				0x00000080
#define kAnalogEchoCancellorEnabled				0x00000100
#define	kT1p413Issue1SingleByteSymMode			0x00000200
#define	kDslAturXmtPowerCutbackEnabled			0x00000400
#ifdef G992_ANNEXC_LONG_REACH
#define kDslAnnexCPilot48                       0x00000800
#define kDslAnnexCReverb33_63                   0x00001000
#endif
#ifdef G992_ANNEXC
#define kDslCentilliumCRCWorkAroundEnabled		0x00002000
#else
#define kDslEnableRoundUpDSLoopAttn		        0x00002000
#endif
#define	kDslBitSwapEnabled						0x00004000
#define	kDslADILowRateOptionFixDisabled			0x00008000
#define	kDslAnymediaGSPNCrcFixEnabled			0x00010000
#define	kDslMultiModesPreferT1p413				0x00020000
#define	kDslT1p413UseRAck1Only					0x00040000
#define	kDslUE9000ADI918FECFixEnabled			0x00080000
#define	kDslG994AnnexAMultimodeEnabled			0x00100000
#define	kDslATUCXmtPowerMinimizeEnabled			0x00200000
#define	kDropOnDataErrorsDisabled			    0x00400000
#define	kDslSRAEnabled						    0x00800000

#define	kDslT1p413HigherToneLevelNeeded			0x01000000
#define	kDslT1p413SubsampleAlignmentEnabled		0x02000000
#define	kDslT1p413DisableUpstream2xIfftMode		0x04000000

/* test mode related demodCapabilities, for internal use only */
#define	kDslTestDemodCapMask					0xF8000000
#define	kDslSendReverbModeEnabled				0x10000000
#define	kDslSendMedleyModeEnabled				0x20000000
#define	kDslAutoRetrainDisabled					0x40000000
#define kDslPllWorkaroundEnabled                0x80000000
#define kDslAfeLoopbackModeEnabled              0x08000000

/* demodCapabilities bitmap2 */

/* only in Annex C */
#define kDslAnnexCProfile1	    			    0x00000001
#define kDslAnnexCProfile2	    			    0x00000002
#define kDslAnnexCProfile3	    			    0x00000004
#define kDslAnnexCProfile4	    			    0x00000008
#define kDslAnnexCProfile5	    			    0x00000010
#define kDslAnnexCProfile6	    			    0x00000020
#define kDslAnnexCPilot64			   	        0x00000040
#define kDslAnnexCPilot48                       0x00000080
#define kDslAnnexCPilot32			   	        0x00000100
#define kDslAnnexCPilot16			   	        0x00000200
#define kDslAnnexCA48B48			   		    0x00000400
#define kDslAnnexCA24B24			    	    0x00000800
#define kDslAnnexCReverb33_63                   0x00001000
#define kDslAnnexCCReverb6_31	  		        0x00002000

#define kDslAnnexIShapedSSVI                    0x00004000
#define kDslAnnexIFlatSSVI                      0x00008000

#define kDslAnnexIPilot64			   	        0x00010000
#define kDslAnnexIA48B48			   		    0x00020000
#define kDslAnnexIPilot128			   	        0x00040000
#define kDslAnnexIPilot96			   	        0x00080000

/* Only in Annex A */
/* Bits 0 to 8 : Annex M submask control */
/* bit 9 : enable custom mode            */
#define kDslAnnexMcustomModeShift               9
#define kDslAnnexMcustomMode                    (1<<kDslAnnexMcustomModeShift)
#define kDslDisableL2                           0x00010000
#define kDigEcShowtimeUpdateDisabled            0x00020000
#define kDigEcShowtimeFastUpdateDisabled        0x00040000
#define kDslRetrainOnSesEnabled                 0x00080000
#define kDsl24kbyteInterleavingEnabled          0x00100000
#define kDslRetrainOnDslamMinMargin             0x00200000
#define kDslFireDsSupported                     0x00400000

/* Features bitmap */
#define	kG992p2RACK1   						    0x00000001
#define	kG992p2RACK2							0x00000002
#define	kG992p2DBM								0x00000004
#define	kG992p2FastRetrain						0x00000008
#define	kG992p2RS16								0x00000010
#define	kG992p2ClearEOCOAM						0x00000020
#define	kG992NTREnabled							0x00000040
#define	kG992p2EraseAllStoredProfiles			0x00000080
#define kG992p2FeaturesNPar2Mask                0x0000003B
#define kG992p2FeaturesNPar2Shift                        0

#define kG992p1RACK1                            0x00000100
#define kG992p1RACK2                            0x00000200
#define kG992p1STM                              0x00000800
#define kG992p1ATM                              0x00001000
#define	kG992p1ClearEOCOAM						0x00002000
#define kG992p1FeaturesNPar2Mask                0x00003B00
#define kG992p1FeaturesNPar2Shift                        8	
#define kG992p1DualLatencyUpstream				0x00004000
#define kG992p1DualLatencyDownstream			0x00008000
#define kG992p1HigherBitRates					0x40000000

#if defined(G992P1_ANNEX_I)
#define kG992p1HigherBitRates1over3				0x80000000
#define kG992p1AnnexIShapedSSVI                 0x00000001
#define kG992p1AnnexIFlatSSVI                   0x00000002
#define kG992p1AnnexIPilotFlag			   		0x00000008
#define kG992p1AnnexIPilot64			   		0x00000001
#define kG992p1AnnexIPilot128			   		0x00000004
#define kG992p1AnnexIPilot96			   		0x00000008
#define kG992p1AnnexIPilotA48B48                0x00000010
#endif

#define kG992p1AnnexBRACK1                      0x00010000
#define kG992p1AnnexBRACK2                      0x00020000
#define kG992p1AnnexBUpstreamTones1to32			0x00040000
#define kG992p1AnnexBSTM                        0x00080000
#define kG992p1AnnexBATM                        0x00100000
#define	kG992p1AnnexBClearEOCOAM				0x00200000
#define kG992p1AnnexBFeaturesNPar2Mask          0x003F0000
#define kG992p1AnnexBFeaturesNPar2Shift                 16	

#define kG992p1AnnexCRACK1                      0x01000000
#define kG992p1AnnexCRACK2                      0x02000000
#define kG992p1AnnexCDBM						0x04000000
#define kG992p1AnnexCSTM                        0x08000000
#define kG992p1AnnexCATM                        0x10000000
#define	kG992p1AnnexCClearEOCOAM				0x20000000
#define kG992p1AnnexCFeaturesNPar2Mask          0x3F000000
#define kG992p1AnnexCFeaturesNPar2Shift                 24	

#define kG992p1HigherBitRates1over3				0x80000000

/* auxFeatures bitmap */
#define	kG994p1PreferToExchangeCaps				0x00000001
#define	kG994p1PreferToDecideMode				0x00000002
#define	kG994p1PreferToMPMode				    0x00000004
#define	kAfePwmSyncClockShift					3
#define	kAfePwmSyncClockMask					(0xF << kAfePwmSyncClockShift)
#define	AfePwmSyncClockEnabled(val)				(((val) & kAfePwmSyncClockMask) != 0)
#define	AfePwmGetSyncClockFreq(val)				((((val) & kAfePwmSyncClockMask) >> kAfePwmSyncClockShift) - 1)
#define	AfePwmSetSyncClockFreq(val,freq)		((val) |= ((((freq)+1) << kAfePwmSyncClockShift) & kAfePwmSyncClockMask))

/* SubChannel Info bitMap for G992p1 */
#define kSubChannelASODownstream                0x00000001
#define kSubChannelAS1Downstream                0x00000002
#define kSubChannelAS2Downstream                0x00000004
#define kSubChannelAS3Downstream                0x00000008
#define kSubChannelLSODownstream                0x00000010
#define kSubChannelLS1Downstream                0x00000020
#define kSubChannelLS2Downstream                0x00000040
#define kSubChannelLS0Upstream                  0x00000080
#define kSubChannelLS1Upstream                  0x00000100
#define kSubChannelLS2Upstream                  0x00000200
#define kSubChannelInfoOctet1Mask               0x0000001F
#define kSubChannelInfoOctet2Mask               0x000003E0
#define kSubChannelInfoOctet1Shift              		 0	
#define kSubChannelInfoOctet2Shift              		 5	

/****************************************************************************/
/*	3.	Interface functions.												*/
/*																			*/
/****************************************************************************/

#ifdef G992P1
#if defined(G992P1_ANNEX_I2X) || defined(G992P5) 
/* lke */
#define	kDslSamplingFreq			4416000
#define	kDslMaxFFTSize			 	1024	
#define	kDslMaxFFTSizeShift			10
#elif defined(G992P1_ANNEX_I4X)
#define	kDslSamplingFreq			8832000
#define	kDslMaxFFTSize			 	2048	
#define	kDslMaxFFTSizeShift			11
#elif defined(G992P1_ANNEX_I8X)
#define	kDslSamplingFreq			17664000
#define	kDslMaxFFTSize			 	4096	
#define	kDslMaxFFTSizeShift			12
#else
#define	kDslSamplingFreq			2208000
#define	kDslMaxFFTSize				512
#define	kDslMaxFFTSizeShift			9
#endif
#else
#define	kDslSamplingFreq			1104000
#define	kDslMaxFFTSize				256
#define	kDslMaxFFTSizeShift			8
#endif

#if defined(G992_ATUR_UPSTREAM_SAMPLING_FREQ_276KHZ)
#define kDslATURUpstreamSamplingFreq    276000
#define	kDslATURFFTSizeShiftUpstream	6
#elif defined(G992_ATUR_UPSTREAM_SAMPLING_FREQ_552KHZ)
#define kDslATURUpstreamSamplingFreq    552000
#define	kDslATURFFTSizeShiftUpstream	7
#else
#define kDslATURUpstreamSamplingFreq    kDslSamplingFreq
#define	kDslATURFFTSizeShiftUpstream	kDslMaxFFTSizeShift
#endif 

#if defined(G992_ATUC_UPSTREAM_SAMPLING_FREQ_276KHZ)
#define kDslATUCUpstreamSamplingFreq    276000
#define	kDslATUCFFTSizeShiftUpstream	6
#elif defined(G992_ATUC_UPSTREAM_SAMPLING_FREQ_552KHZ)
#define kDslATUCUpstreamSamplingFreq    552000
#define	kDslATUCFFTSizeShiftUpstream	7
#else
#define kDslATUCUpstreamSamplingFreq    kDslSamplingFreq
#define	kDslATUCFFTSizeShiftUpstream	kDslMaxFFTSizeShift
#endif 

#define	kDslMaxSamplesPerSymbol		(kDslMaxFFTSize+kDslMaxFFTSize/16)

#if defined(G992P1_ANNEX_I) || defined(G992P5)
#define kDslMaxTEQLength	        32
#else
#define kDslMaxTEQLength	        16
#endif

#define	kDslMaxSymbolBlockSize		1
#define	kDslMaxSampleBlockSize		(kDslMaxSymbolBlockSize*kDslMaxSamplesPerSymbol)

#ifdef G992_ANNEXC
#define	kG992AnnexCXmtToRcvPathDelay	512   /* In samples at kDslSamplingFreq */
#endif

/*** For compatibility with existing test codes ***/
#if !defined(TARG_OS_RTEMS)
typedef dslStatusCode				modemStatusCode;
typedef	dslStatusStruct				modemStatusStruct;
typedef	dslStatusHandlerType		statusHandlerType;
typedef dslCommandCode				modemCommandCode;
typedef	dslCommandStruct			modemCommandStruct;
typedef	dslCommandHandlerType		commandHandlerType;
#endif

extern void		SM_DECL SoftDslSetRefData	(void *gDslVars, ulong refData);
extern ulong	SM_DECL SoftDslGetRefData	(void *gDslVars);
extern int		SM_DECL SoftDslGetMemorySize(void);
extern void		SM_DECL SoftDslInit			(void *gDslVars);
extern void		SM_DECL SoftDslReset		(void *gDslVars);
extern void		SM_DECL SoftDslLineHandler	(void *gDslVars, int rxNSamps, int txNSamps, short *rcvPtr, short *xmtPtr) FAST_TEXT;
extern Boolean	SM_DECL SoftDslCommandHandler (void *gDslVars, dslCommandStruct *cmdPtr);

/* swap Lmem functions */
#if defined(bcm47xx) && defined(SWAP_LMEM)
extern int SoftDslSwapLmem(void *gDslVars, int sectionN, int imageN);
extern void init_SoftDslSwapLmem(void);
#endif

/* SoftDsl time functions	*/

extern ulong	SM_DECL SoftDslGetTime(void *gDslVars);
#define			__SoftDslGetTime(gv)		gDslGlobalVarPtr->execTime

extern void		SM_DECL SoftDslTimer(void *gDslVars, ulong timeMs);

/* SoftDsl IO functions	*/

extern void		SM_DECL SoftDslClose (void *gDslVars);
extern int		SM_DECL SoftDslSendFrame (void *gDslVars, void *pVc, ulong mid, dslFrame * pFrame);
extern int		SM_DECL SoftDslReturnFrame (void *gDslVars, void *pVc, ulong mid, dslFrame * pFrame);

/* SoftDsl connection functions	*/

extern	void*	SM_DECL SoftDslVcAllocate(void *gDslVars, dslVcParams *pVcParams);
extern	void	SM_DECL SoftDslVcFree(void *gDslVars, void *pVc);
extern	Boolean SM_DECL SoftDslVcActivate(void *gDslVars, void *pVc);
extern  void	SM_DECL SoftDslVcDeactivate(void *gDslVars, void *pVc);
extern  Boolean SM_DECL SoftDslVcConfigure(void *gDslVars, void *pVc, ulong mid, dslVcParams *pVcParams);

/* Special functions for LOG support */

extern  ulong	SM_DECL SoftDslVc2Id(void *gDslVars, void *pVc);
extern  void*	SM_DECL SoftDslVcId2Vc(void *gDslVars, ulong vcId);
extern	void*	SM_DECL SoftDslGetFramePool(void *gDslVars);

/* Functions for host mode execution */

extern  void*   SM_DECL SoftDslRxCellHeaderHandler (void *gDslVars, ulong hdr, uchar hdrHec);
extern	void*	SM_DECL SoftDslRxCellDataHandler (void *gDslVars, int,	void*);
extern  void*	SM_DECL SoftDslTxCellHandler	(void *gDslVars, int*,	void*);
extern  Boolean	SM_DECL SoftDslPhyCommandHandler (void *gDslVars, dslCommandStruct *cmdPtr);

/* Functions getting OEM parameters including G994 non standard info management */

extern	char*	SM_DECL SoftDslGetTrainingVendorIDString(void *gDslVars);
extern	char*	SM_DECL SoftDslGetVendorIDString(void *gDslVars);
extern	char*	SM_DECL SoftDslGetSerialNumberString(void *gDslVars);
extern	char*	SM_DECL SoftDslGetRevString(void *gDslVars);
extern	int		SM_DECL SoftDslRevStringSize(void *gDslVars);
extern	int		SM_DECL SoftDslSerNumStringSize(void *gDslVars);
							   
extern  void*	SM_DECL SoftDslGetG994p1RcvNonStdInfo(void *gDslVars, ulong *pLen);
extern  void*	SM_DECL SoftDslGetG994p1XmtNonStdInfo(void *gDslVars, ulong *pLen);

#ifdef G997_1_FRAMER

/* G997 functions */

extern int		SM_DECL SoftDslG997SendFrame (void *gDslVars, void *pVc, ulong mid, dslFrame * pFrame);
extern int		SM_DECL SoftDslG997ReturnFrame (void *gDslVars, void *pVc, ulong mid, dslFrame * pFrame);

#endif

#ifdef ADSL_MIB
extern void	 *	SM_DECL	SoftDslMibGetData (void *gDslVars, int dataId, void *pAdslMibData);
#endif

#define	SoftDsl					SoftDslLineHandler
#define	kSoftDslMaxMemorySize	(32768*16384)

/*
 * Internal functions
 */

#ifdef	EXTENDED_INTERLEAVE_DEPTH_24K
extern  void	SoftDslStatusHandler	(void *gDslVars, dslStatusStruct *status);
#else
extern  void	SoftDslStatusHandler	(void *gDslVars, dslStatusStruct *status) FAST_TEXT;
#endif
extern  void	SoftDslInternalStatusHandler (void *gDslVars, dslStatusStruct *status);

/*
 *		DSL OS functions
 */

#define	BG_TASK_IDLE						0
#define	BG_TASK_SCHEDULED					1
#define	BG_TASK_RUNNING						2

#ifdef DSL_OS

#define	SoftDslIsBgAvailable(gDslVars)		(DSLOS_THREAD_INACTIVE == DslOsGetThreadState(&(gDslGlobalVarPtr->tcbDslBg)))
#define	SoftDslGetBgThread(gDslVars)		\
	((DSLOS_THREAD_INACTIVE != DslOsGetThreadState(&(gDslGlobalVarPtr->tcbDslBg))) ? &gDslGlobalVarPtr->tcbDslBg : NULL)
#define	SoftDslBgStart(gDslVars, pFunc)		\
	DslOsCreateThread(&gDslGlobalVarPtr->tcbDslBg, DSLOS_PRIO_HIGHEST - 10, pFunc, gDslVars,	\
	WB_ADDR(gDslGlobalVarPtr->bgStack), sizeof(gDslGlobalVarPtr->bgStack))
#define	SoftDslBgStop(gDslVars)				DslOsDeleteThread(&gDslGlobalVarPtr->tcbDslBg)

extern Boolean SoftDslBgScheduleTask(void *gDslVars, void *pFunc);
extern Boolean SoftDslBgScheduleTaskFirst(void *gDslVars, void *pFunc);
extern void	*  SoftDslBgScheduleGetTask(void *gDslVars);
extern Boolean SoftDslBgScheduleIsTaskAvail(void *gDslVars);
extern void	   SoftDslBgScheduleClear(void *gDslVars);
extern void	   SoftDslBgScheduleInit(void *gDslVars);

#define	SoftDslEnterCritical()				DslOsEnterCritical()
#define	SoftDslLeaveCritical(id)			DslOsLeaveCritical(id)

#else

#define	SoftDslIsBgAvailable(gDslVars)		1
#define	SoftDslGetBgThread(gDslVars)		1
#define	SoftDslBgStart(gDslVars, pFunc)		(*pFunc)(gDslVars)
#define	SoftDslBgStop(gDslVars)

#define	SoftDslBgScheduleTask(gDslVars,pFunc)		(*pFunc)(gDslVars)
#define	SoftDslBgScheduleTaskFirst(gDslVars,pFunc)	(*pFunc)(gDslVars)
#define	SoftDslBgScheduleGetTask(gDslVars)			NULL
#define	SoftDslBgScheduleIsTaskAvail(gDslVars)		0
#define	SoftDslBgScheduleClear(gDslVars)
#define	SoftDslBgScheduleInit(gDslVars)

#define	SoftDslEnterCritical()				0
#define	SoftDslLeaveCritical(id)

#endif

/* SDRAM write control */
extern void DslCoreXfaceWrCheck(void) FAST_TEXT; 
#define	CHECK_WR_COMPLETE()					DslCoreXfaceWrCheck()

/* Fast dslSlowVarsStruct access */
#if !(defined(bcm47xx) && defined(MIPS_SRC))
#undef SLOW_VAR_GLOBAL_REG
#endif

#ifdef SLOW_VAR_GLOBAL_REG
register  struct __dslSlowVarsStruct *gDslSlowVars asm ("$28");
#endif

#ifdef DSLVARS_GLOBAL_REG
register  struct __dslVarsStruct	 *gDslVarsReg  asm ("$27");
#define gDslVars	gDslVarsReg
#endif

/*
 *		DSL frames and native frame functions
 */

DslFrameDeclareFunctions (DslFrameNative)

/*
 * These functions are for testing purpose, they are defined outside.
 */
#ifdef STACK_SIZE_REQUIREMENT_TEST
extern	void		StackSizeTestInitializeStackBeforeEntry(void);
extern	void		StackSizeTestCheckStackAfterExit(void);
extern	void		StackSizeTestBackupStack(void);
extern	void		StackSizeTestRestoreStack(void);
#endif /* STACK_SIZE_REQUIREMENT_TEST */

#ifdef NEC_NSIF_WORKAROUND
#define	SoftDslGetG994NsStatus(gDslVars)		        (gDslGlobalVarPtr->G994NsStatus)
#define	SoftDslGetG994NsFailCounter(gDslVars)		    (gDslGlobalVarPtr->G994NsFailCounter)
#endif

#endif	/* SoftDslHeader */
