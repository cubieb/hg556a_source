/*
<:copyright-gpl
 Copyright 2004 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/

/****************************************************************************
 *
 * AdslMibOid.h 
 *
 * Description:
 *  SNMP object identifiers for ADSL MIB and other related MIBs
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslMibDef.h,v 1.1 2008/08/25 06:41:10 l65130 Exp $
 *
 * $Log: AdslMibDef.h,v $
 * Revision 1.1  2008/08/25 06:41:10  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:28  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:47  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:10:47  z45221
 * 合入人:
 * s48571
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
 * Revision 1.26  2007/02/15 23:27:19  tonytran
 * Fixed output log file access problem after log=0 or completed. Added profstart/profstop commands and display FIRE counters for US
 *
 * Revision 1.24  2007/01/11 09:14:04  tonytran
 * Fixed the set phy cfg and bitswap counter problems; Removed FAST_DEC_DISABLE from phy cfg
 *
 * Revision 1.22  2006/04/03 15:30:27  ovandewi
 * fix structure name and types
 *
 * Revision 1.20  2006/03/31 20:26:36  dadityan
 * PLN and NL Data
 *
 * Revision 1.19  2005/08/11 15:08:38  ilyas
 * Added string scrambling checking and AnnexL/M submodes display
 *
 * Revision 1.18  2005/04/01 21:57:42  ilyas
 * Added definition for PwmClock (ADSL driver)
 *
 * Revision 1.17  2004/07/27 19:24:40  ilyas
 * Added AnnexM configuration option
 *
 * Revision 1.16  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.15  2004/05/25 16:15:04  ilyas
 * Added ADSL2 framing status
 *
 * Revision 1.14  2004/03/31 19:09:48  ilyas
 * Added ADSL2+ modulation control
 *
 * Revision 1.13  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.12  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.11  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.10  2003/09/29 18:39:51  ilyas
 * Added new definitions for AnnexI
 *
 * Revision 1.9  2003/07/18 19:14:34  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.8  2003/07/08 18:34:16  ilyas
 * Added fields to adsl configuration structure
 *
 * Revision 1.7  2003/03/25 00:07:00  ilyas
 * Added "long" BERT supprt
 *
 * Revision 1.6  2003/02/27 07:10:52  ilyas
 * Added more configuration and status parameters (for EFNT)
 *
 * Revision 1.5  2003/01/23 20:29:37  ilyas
 * Added structure for ADSL PHY configuration command
 *
 * Revision 1.4  2002/11/13 21:32:49  ilyas
 * Added adjustK support for Centillium non-standard framing mode
 *
 * Revision 1.3  2002/10/31 01:35:50  ilyas
 * Fixed size of K for S=1/2
 *
 * Revision 1.2  2002/10/05 03:28:31  ilyas
 * Added extra definitions for Linux and VxWorks drivers.
 * Added definitions for SelfTest support
 *
 * Revision 1.1  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#ifndef AdslMibDefHeader
#define AdslMibDefHeader

#if defined(__cplusplus)
extern "C" {
#endif

/* 
**
**      ADSL configuration parameters 
**
*/

#define kAdslCfgModMask                     (0x00000007 | 0x0000F000)
#define kAdslCfgModAny                      0x00000000

#define kAdslCfgModGdmtOnly                 0x00000001
#define kAdslCfgModGliteOnly                0x00000002
#define kAdslCfgModT1413Only                0x00000004
#define kAdslCfgModAnnexIOnly               0x00000004
#define kAdslCfgModAdsl2Only                0x00001000
#define kAdslCfgModAdsl2pOnly               0x00002000

#define kAdslCfgBitmapMask                  0x00000018
#define kAdslCfgDBM                         0x00000000
#define kAdslCfgFBM                         0x00000008
#define kAdslCfgFBMSoL                      0x00000010

#define kAdslCfgLinePairMask                0x00000020
#define kAdslCfgLineInnerPair               0x00000000
#define kAdslCfgLineOuterPair               0x00000020

#define kAdslCfgCentilliumCRCWorkAroundMask         0x00000040
#define kAdslCfgCentilliumCRCWorkAroundDisabled     0x00000000
#define kAdslCfgCentilliumCRCWorkAroundEnabled      0x00000040

#define kAdslCfgExtraData                   0x00000080
#define kAdslCfgTrellisMask                 (0x00000100 | kAdslCfgExtraData)
#define kAdslCfgTrellisOn                   (0x00000100 | kAdslCfgExtraData)
#define kAdslCfgTrellisOff                  (0 | kAdslCfgExtraData)
#define kAdslCfgExtraMask                   0xFFFFFF80

#define kAdslCfgLOSMonitoringMask           0x00000200
#define kAdslCfgLOSMonitoringOff            0x00000200
#define kAdslCfgLOSMonitoringOn             0x00000000

#define kAdslCfgMarginMonitoringMask        0x00000400
#define kAdslCfgMarginMonitoringOn          0x00000400
#define kAdslCfgMarginMonitoringOff         0x00000000

#define kAdslCfgDemodCapMask                0x00000800
#define kAdslCfgDemodCapOn                  0x00000800
#define kAdslCfgDemodCapOff                 0x00000000

/* Flags 0x00001000 - 0x00008000 are reserved for modulation (see above) */

/* Upstream mode flags 0x00010000 - 0x00030000 */

#define kAdslCfgUpstreamModeMask            0x00030000
#define kAdslCfgUpstreamMax                 0x00000000
#define kAdslCfgUpstreamSingle              0x00010000
#define kAdslCfgUpstreamDouble              0x00020000
#define kAdslCfgUpstreamTriple              0x00030000

#define kAdslCfgNoSpectrumOverlap           0x00040000

/* Pwm sync clock configuration */

#define kAdslCfgPwmSyncClockMask            0x00080000
#define kAdslCfgPwmSyncClockOn              0x00080000
#define kAdslCfgPwmSyncClockOff             0x00000000

#define kAdslCfgDemodCap2Mask               0x00100000
#define kAdslCfgDemodCap2On                 0x00100000
#define kAdslCfgDemodCap2Off                0x00000000

#define kAdslCfgDefaultTrainingMargin       -1
#define kAdslCfgDefaultShowtimeMargin       -1
#define kAdslCfgDefaultLOMTimeThld          -1

/* ADSL2 parameters */

#define kAdsl2CfgReachExOn                  0x00000001
#define kAdsl2CfgAnnexMEnabled              0x00000002

#define kAdsl2CfgAnnexMPsdShift             2
#define kAdsl2CfgAnnexMPsdBits              12
#define kAdsl2CfgAnnexMPsdMask              (0xFFF << kAdsl2CfgAnnexMPsdShift)

#if (((1 << kAdsl2CfgAnnexMPsdBits) - 1) != (kAdsl2CfgAnnexMPsdMask >> kAdsl2CfgAnnexMPsdShift))
#error Inconsistent kAdsl2CfgAnnexM definitions
#endif

#define kAdsl2CfgAnnexMUp32                 (0x00000001 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp36                 (0x00000002 << kAdsl2CfgAnnexMPsdShift) 
#define kAdsl2CfgAnnexMUp40                 (0x00000004 << kAdsl2CfgAnnexMPsdShift) 
#define kAdsl2CfgAnnexMUp44                 (0x00000008 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp48                 (0x00000010 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp52                 (0x00000020 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp56                 (0x00000040 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp60                 (0x00000080 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMUp64                 (0x00000100 << kAdsl2CfgAnnexMPsdShift)
#define kAdsl2CfgAnnexMCustomPsd            (0x00000200 << kAdsl2CfgAnnexMPsdShift)

#define kAdsl2CfgAnnexLShift                (2 + kAdsl2CfgAnnexMPsdBits)
#define kAdsl2CfgAnnexLMask                 (0x7 << kAdsl2CfgAnnexLShift)

#define kAdsl2CfgAnnexLUpWide               (0x00000001 << kAdsl2CfgAnnexLShift)
#define kAdsl2CfgAnnexLUpNarrow             (0x00000002 << kAdsl2CfgAnnexLShift)
#define kAdsl2CfgAnnexLDnOvlap              (0x00000004 << kAdsl2CfgAnnexLShift)

typedef struct _adslCfgProfile {
    long        adslAnnexCParam;
    long        adslAnnexAParam;
    long        adslTrainingMarginQ4;
    long        adslShowtimeMarginQ4;
    long        adslLOMTimeThldSec;
    long        adslDemodCapMask;
    long        adslDemodCapValue;
    long        adsl2Param;
    long        adslPwmSyncClockFreq;
    long        adslHsModeSwitchTime;
    long        adslDemodCap2Mask;
    long        adslDemodCap2Value;
} adslCfgProfile;

/* 
**
**      ADSL PHY configuration
**
*/

typedef struct _adslPhyCfg {
    long        demodCapMask;
    long        demodCap;
} adslPhyCfg;

/* 
**
**      ADSL version info parameters 
**
*/

#define kAdslVersionStringSize              32

#define kAdslTypeUnknown                    0
#define kAdslTypeAnnexA                     1
#define kAdslTypeAnnexB                     2
#define kAdslTypeAnnexC                     3
#define kAdslTypeSADSL                      4

typedef struct _adslVersionInfo {
    unsigned short  phyType;
    unsigned short  phyMjVerNum;
    unsigned short  phyMnVerNum;
    char            phyVerStr[kAdslVersionStringSize];
    unsigned short  drvMjVerNum;
    unsigned short  drvMnVerNum;
    char            drvVerStr[kAdslVersionStringSize];
} adslVersionInfo;

/* 
**
**      ADSL self-test parameters 
**
*/

#define kAdslSelfTestLMEM                   0x00000001
#define kAdslSelfTestSDRAM                  0x00000002
#define kAdslSelfTestAFE                    0x00000004
#define kAdslSelfTestQproc                  0x00000008
#define kAdslSelfTestRS                     0x00000010
#define kAdslSelfTestHostDma                0x00000020

#define kAdslSelfTestAll                    ((kAdslSelfTestHostDma - 1) | kAdslSelfTestHostDma)

#define kAdslSelfTestInProgress             0x40000000
#define kAdslSelfTestCompleted              0x80000000

/* MIB OID's for ADSL objects */

#define kOidMaxObjLen                       80

#define kOidAdsl                            94
#define kOidAdslInterleave                  124
#define kOidAdslFast                        125
#define kOidAtm                             37
#define kOidAdslPrivate                     255
#define kOidAdslPrivatePartial              254

#define kAdslMibAnnexAToneNum               256
#define kAdslMibToneNum                     kAdslMibAnnexAToneNum
#define kAdslMibMaxToneNum                  kAdslMibAnnexAToneNum*2*2

#define kOidAdslPrivSNR                     1
#define kOidAdslPrivBitAlloc                2
#define kOidAdslPrivGain                    3
#define kOidAdslPrivShowtimeMargin          4
#define kOidAdslPrivChanCharLin             5
#define kOidAdslPrivChanCharLog             6
#define kOidAdslPrivQuietLineNoise          7
#ifdef ADSL_MIBOBJ_PLN
#define kOidAdslExtraPLNInfo						11
#define kOidAdslExtraPLNData						12

#define kOidAdslExtraPLNDataThldBB						1
#define kOidAdslExtraPLNDataThldPerTone					2
#define kOidAdslExtraPLNDataPLNState					3
#define kOidAdslExtraPLNDataNbDurBins					4
#define kOidAdslExtraPLNDataNbIntArrBins				5
#define kOidAdslExtraPLNDataUpdate						6
#define kOidAdslPrivPLNDurationBins         8
#define kOidAdslPrivPLNIntrArvlBins         9
#define kOidAdslPrivPLNValueps              10
#define kOidAdslPrivPLNThldCntps            11
#define kOidAdslPrivPLNDurationHist         12
#define kOidAdslPrivPLNIntrArvlHist         13
#endif
#define kOidAdslPrivNLDistNoise             14
#define kOidAdslPrivExtraInfo               255
#define kOidAdslExtraNLInfo							10

#define kOidAdslLine                        1
#define kOidAdslMibObjects                  1

#define kOidAdslLineTable                   1
#define kOidAdslLineEntry                   1
#define kOidAdslLineCoding                  1
#define kOidAdslLineType                    2
#define kOidAdslLineSpecific                3
#define kOidAdslLineConfProfile             4
#define kOidAdslLineAlarmConfProfile        5

#define kOidAdslAtucPhysTable               2
#define kOidAdslAturPhysTable               3
#define kOidAdslPhysEntry                   1
#define kOidAdslPhysInvSerialNumber         1
#define kOidAdslPhysInvVendorID             2
#define kOidAdslPhysInvVersionNumber        3
#define kOidAdslPhysCurrSnrMgn              4
#define kOidAdslPhysCurrAtn                 5
#define kOidAdslPhysCurrStatus              6
#define kOidAdslPhysCurrOutputPwr           7
#define kOidAdslPhysCurrAttainableRate      8

#define kOidAdslAtucChanTable               4
#define kOidAdslAturChanTable               5
#define kOidAdslChanEntry                   1
#define kOidAdslChanInterleaveDelay         1
#define kOidAdslChanCurrTxRate              2
#define kOidAdslChanPrevTxRate              3
#define kOidAdslChanCrcBlockLength          4

#define kOidAdslAtucPerfDataTable           6
#define kOidAdslAturPerfDataTable           7
#define kOidAdslPerfDataEntry               1
#define kOidAdslPerfLofs                    1
#define kOidAdslPerfLoss                    2
#define kOidAdslPerfLprs                    3
#define kOidAdslPerfESs                     4
#define kOidAdslPerfValidIntervals          5
#define kOidAdslPerfInvalidIntervals        6
#define kOidAdslPerfCurr15MinTimeElapsed    7
#define kOidAdslPerfCurr15MinLofs           8
#define kOidAdslPerfCurr15MinLoss           9
#define kOidAdslPerfCurr15MinLprs           10
#define kOidAdslPerfCurr15MinESs            11
#define kOidAdslPerfCurr1DayTimeElapsed     12
#define kOidAdslPerfCurr1DayLofs            13
#define kOidAdslPerfCurr1DayLoss            14
#define kOidAdslPerfCurr1DayLprs            15
#define kOidAdslPerfCurr1DayESs             16
#define kOidAdslPerfPrev1DayMoniSecs        17
#define kOidAdslPerfPrev1DayLofs            18
#define kOidAdslPerfPrev1DayLoss            19
#define kOidAdslPerfPrev1DayLprs            20
#define kOidAdslPerfPrev1DayESs             21

#define kOidAdslAtucPerfIntervalTable       8
#define kOidAdslAturPerfIntervalTable       9
#define kOidAdslPerfIntervalEntry           1
#define kOidAdslIntervalNumber              1
#define kOidAdslIntervalLofs                2
#define kOidAdslIntervalLoss                3
#define kOidAdslIntervalLprs                4
#define kOidAdslIntervalESs                 5
#define kOidAdslIntervalValidData           6

#define kOidAdslAtucChanPerfTable                   10
#define kOidAdslAturChanPerfTable                   11
#define kOidAdslChanPerfEntry                       1
#define kOidAdslChanReceivedBlks                    1
#define kOidAdslChanTransmittedBlks                 2
#define kOidAdslChanCorrectedBlks                   3
#define kOidAdslChanUncorrectBlks                   4
#define kOidAdslChanPerfValidIntervals              5
#define kOidAdslChanPerfInvalidIntervals            6
#define kOidAdslChanPerfCurr15MinTimeElapsed        7
#define kOidAdslChanPerfCurr15MinReceivedBlks       8
#define kOidAdslChanPerfCurr15MinTransmittedBlks    9
#define kOidAdslChanPerfCurr15MinCorrectedBlks      10
#define kOidAdslChanPerfCurr15MinUncorrectBlks      11
#define kOidAdslChanPerfCurr1DayTimeElapsed         12
#define kOidAdslChanPerfCurr1DayReceivedBlks        13
#define kOidAdslChanPerfCurr1DayTransmittedBlks     14
#define kOidAdslChanPerfCurr1DayCorrectedBlks       15
#define kOidAdslChanPerfCurr1DayUncorrectBlks       16
#define kOidAdslChanPerfPrev1DayMoniSecs            17
#define kOidAdslChanPerfPrev1DayReceivedBlks        18
#define kOidAdslChanPerfPrev1DayTransmittedBlks     19
#define kOidAdslChanPerfPrev1DayCorrectedBlks       20
#define kOidAdslChanPerfPrev1DayUncorrectBlks       21

#define kOidAdslAtucChanIntervalTable               12
#define kOidAdslAturChanIntervalTable               13
#define kOidAdslChanIntervalEntry                   1
#define kOidAdslChanIntervalNumber                  1
#define kOidAdslChanIntervalReceivedBlks            2
#define kOidAdslChanIntervalTransmittedBlks         3
#define kOidAdslChanIntervalCorrectedBlks           4
#define kOidAdslChanIntervalUncorrectBlks           5
#define kOidAdslChanIntervalValidData               6

#define kOidAtmMibObjects       1
#define kOidAtmTcTable          4
#define kOidAtmTcEntry          1
#define kOidAtmOcdEvents        1
#define kOidAtmAlarmState       2

/* Adsl Channel coding */

#define kAdslRcvDir         0
#define kAdslXmtDir         1

#define kAdslRcvActive      (1 << kAdslRcvDir)
#define kAdslXmtActive      (1 << kAdslXmtDir)

#define kAdslIntlChannel    0
#define kAdslFastChannel    1

#define kAdslTrellisOff     0
#define kAdslTrellisOn      1
#define kAdsl2TrellisRxEnabled        0x1
#define kAdsl2TrellisTxEnabled        0x2

/* AnnexC modulation and bitmap types for the field (adslConnection.modType) */

#define kAdslModMask        0x7

#define kAdslModGdmt        0
#define kAdslModT1413       1
#define kAdslModGlite       2
#define kAdslModAnnexI      3
#define kAdslModAdsl2       4
#define kAdslModAdsl2p      5
#define kAdslModReAdsl2     6

/* HUAWEI HGW songliyuan 48571 2006年8月4日" tr069--VDSL add begin:*/
#define kAdslModVdsl2      0x10
/* HUAWEI HGW songliyuan 48571 2006年8月4日" tr069--VDSL add end.*/



#define kAdslBitmapShift    3
#define kAdslBitmapMask     kAdslCfgBitmapMask
#define kAdslDBM            (0 << kAdslBitmapShift)
#define kAdslFBM            (1 << kAdslBitmapShift)
#define kAdslFBMSoL         (2 << kAdslBitmapShift)

#define kAdslUpstreamModeShift      5
#define kAdslUpstreamModeMask       (3 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeSingle     (0 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeDouble     (1 << kAdslUpstreamModeShift)
#define kAdslUpstreamModeTriple     (2 << kAdslUpstreamModeShift)

/* AdslLineCodingType definitions */

#define kAdslLineCodingOther        1
#define kAdslLineCodingDMT          2
#define kAdslLineCodingCAP          3
#define kAdslLineCodingQAM          4

/* AdslLineType definitions */

#define kAdslLineTypeNoChannel      1
#define kAdslLineTypeFastOnly       2
#define kAdslLineTypeIntlOnly       3
#define kAdslLineTypeFastOrIntl     4
#define kAdslLineTypeFastAndIntl    5

typedef struct _adslLineEntry {
    unsigned char   adslLineCoding;
    unsigned char   adslLineType;
} adslLineEntry;


/* AdslPhys status definitions */

#define kAdslPhysStatusNoDefect     (1 << 0)
#define kAdslPhysStatusLOF          (1 << 1)    /* lossOfFraming (not receiving valid frame) */
#define kAdslPhysStatusLOS          (1 << 2)    /* lossOfSignal (not receiving signal) */
#define kAdslPhysStatusLPR          (1 << 3)    /* lossOfPower */
#define kAdslPhysStatusLOSQ         (1 << 4)    /* lossOfSignalQuality */
#define kAdslPhysStatusLOM          (1 << 5)    /* lossOfMargin */

#define kAdslPhysVendorIdLen        8
#define kAdslPhysSerialNumLen       32
#define kAdslPhysVersionNumLen      32

typedef struct _adslPhysEntry {
    long        adslCurrSnrMgn;
    long        adslCurrAtn;
    long        adslCurrStatus;
    long        adslCurrOutputPwr;
    long        adslCurrAttainableRate;
    long        adslSignalAttn;
    long        adslHlinScaleFactor;
    long        adslLDCompleted;
    char        adslVendorID[kAdslPhysVendorIdLen];
} adslPhysEntry;

typedef struct _adslFullPhysEntry {
    char        adslSerialNumber[kAdslPhysSerialNumLen];
    char        adslVendorID[kAdslPhysVendorIdLen];
    char        adslVersionNumber[kAdslPhysVersionNumLen];
    long        adslCurrSnrMgn;
    long        adslCurrAtn;
    long        adslCurrStatus;
    long        adslCurrOutputPwr;
    long        adslCurrAttainableRate;
    long        adslSignalAttn;
    long        adslHlinScaleFactor;
} adslFullPhysEntry;

/* Adsl channel entry definitions */

typedef struct _adslChanEntry {
    unsigned long       adslChanIntlDelay;
    unsigned long       adslChanCurrTxRate;
    unsigned long       adslChanPrevTxRate;
    unsigned long       adslChanCrcBlockLength;
} adslChanEntry;

/* Adsl performance data definitions */

typedef struct _adslPerfCounters {
    unsigned long       adslLofs;
    unsigned long       adslLoss;
    unsigned long       adslLols;   /* Loss of Link failures (ATUC only) */
    unsigned long       adslLprs;
    unsigned long       adslESs;    /* Count of Errored Seconds */
    unsigned long       adslInits;  /* Count of Line initialization attempts (ATUC only) */
    unsigned long       adslUAS;    /* Count of Unavailable Seconds */
    unsigned long       adslSES;    /* Count of Severely Errored Seconds */
    unsigned long       adslLOSS;   /* Count of LOS seconds */
    unsigned long       adslFECs;   /* Count of FEC seconds  */
    unsigned long       adslLCDS;   /* Count of LCD Errored Seconds */
    unsigned long       adslAS;         /* Count of Availabe in second */ 
} adslPerfCounters;

typedef struct _adslFailureCounters {
    unsigned long       adslRetr;       /* Count of total retrains */
    unsigned long       adslRetrLof;    /* Count of retrains due to LOF */
    unsigned long       adslRetrLos;    /* Count of retrains due to LOS */
    unsigned long       adslRetrLpr;    /* Count of retrains due to LPR */
    unsigned long       adslRetrLom;    /* Count of retrains due to LOM */
    unsigned long       adslInitErr;    /* Count of training failures */
    unsigned long       adslInitTo;     /* Count of training timeouts */
    unsigned long       adslLineSearch; /* Count of line search inits */
} adslFailureCounters;

typedef struct _adslPerfDataEntry {
    adslPerfCounters    perfTotal;
    adslPerfCounters    perfSinceShowTime;
    unsigned long               adslPerfValidIntervals;
    unsigned long               adslPerfInvalidIntervals;
    adslPerfCounters    perfCurr15Min;
    unsigned long               adslPerfCurr15MinTimeElapsed;
    adslPerfCounters    perfCurr1Day;
    unsigned long               adslPerfCurr1DayTimeElapsed;
    adslPerfCounters    perfPrev1Day;
    unsigned long               adslAturPerfPrev1DayMoniSecs;
    adslFailureCounters failTotal;
    adslFailureCounters failSinceShowTime;
	long				lastRetrainReason;
	long				lastShowtimeDropReason;
} adslPerfDataEntry;

#define kAdslMibPerfIntervals       4

/* Adsl channel performance data definitions */

typedef struct _adslChanCounters {
    unsigned long       adslChanReceivedBlks;
    unsigned long       adslChanTransmittedBlks;
    unsigned long       adslChanCorrectedBlks;
    unsigned long       adslChanUncorrectBlks;
} adslChanCounters;

typedef struct _adslChanPerfDataEntry {
    adslChanCounters    perfTotal;
    unsigned long               adslChanPerfValidIntervals;
    unsigned long               adslChanPerfInvalidIntervals;
    adslChanCounters    perfCurr15Min;
    unsigned long               adslPerfCurr15MinTimeElapsed;
    adslChanCounters    perfCurr1Day;
    unsigned long               adslPerfCurr1DayTimeElapsed;
    adslChanCounters    perfPrev1Day;
    unsigned long               adslAturPerfPrev1DayMoniSecs;
} adslChanPerfDataEntry;


typedef struct _adslNonLinearityData{
  unsigned short                maxCriticalDistNoise;
  unsigned short                distAffectedBits;
} adslNonLinearityData;

#ifdef ADSL_MIBOBJ_PLN
typedef struct _adslPLNDataEntry {
    char PLNState;
    unsigned long PLNThldBB;
    unsigned long PLNThldPerTone;
    char PLNNbDurBins;
    char PLNNbIntArrBins;
    unsigned long PLNPerToneCounter;
    unsigned long PLNBBCounter;
	char  PLNUpdateData;
} adslPLNDataEntry;

#endif

#define kAdslMibChanPerfIntervals   4

/* Adsl trap threshold definitions */

#define kAdslEventLinkChange        0x001
#define kAdslEventRateChange        0x002
#define kAdslEventLofThresh         0x004
#define kAdslEventLosThresh         0x008
#define kAdslEventLprThresh         0x010
#define kAdslEventESThresh          0x020
#define kAdslEventFastUpThresh      0x040
#define kAdslEventIntlUpThresh      0x080
#define kAdslEventFastDownThresh    0x100
#define kAdslEventIntlDwonThresh    0x200

typedef struct _adslThreshCounters {
    unsigned long       adslThreshLofs;
    unsigned long       adslThreshLoss;
    unsigned long       adslThreshLols; /* Loss of Link failures (ATUC only) */
    unsigned long       adslThreshLprs;
    unsigned long       adslThreshESs;
    unsigned long       adslThreshFastRateUp;
    unsigned long       adslThreshIntlRateUp;
    unsigned long       adslThreshFastRateDown;
    unsigned long       adslThreshIntlRateDown;
} adslThreshCounters;


/* Atm PHY performance data definitions */

#define kAtmPhyStateNoAlarm         1
#define kAtmPhyStateLcdFailure      2

typedef struct _atmPhyDataEntrty {
    unsigned long       atmInterfaceOCDEvents;
    unsigned long       atmInterfaceTCAlarmState;
} atmPhyDataEntrty;

typedef struct _adslBertResults {
    unsigned long       bertTotalBits;
    unsigned long       bertErrBits;
} adslBertResults;

typedef struct {
    unsigned long       cntHi;
    unsigned long       cntLo;
} cnt64;

typedef struct _adslBertStatusEx {
    unsigned long       bertSecTotal;
    unsigned long       bertSecElapsed;
    unsigned long       bertSecCur;
    cnt64               bertTotalBits;
    cnt64               bertErrBits;
} adslBertStatusEx;

typedef struct _adslDataConnectionInfo {
    unsigned short      K;
    unsigned char       S, R, D;
} adslDataConnectionInfo;

typedef struct _adslConnectionInfo {
    unsigned char           chType;             /* fast or interleaved */
    unsigned char           modType;            /* modulation type: G.DMT or T1.413 */
    unsigned char           trellisCoding;      /* off(0) or on(1) */
    adslDataConnectionInfo  rcvInfo;
    adslDataConnectionInfo  xmtInfo;
	unsigned char			trellisCoding2;
} adslConnectionInfo;

typedef struct _adsl2DataConnectionInfo {
    unsigned char       Nlp;
    unsigned char       Nbc;
    unsigned char       MSGlp;
    unsigned short      MSGc;

    unsigned long       L;
    unsigned short      M;
    unsigned short      T;
    unsigned short      D;
    unsigned short      R;
    unsigned short      B;
} adsl2DataConnectionInfo;

/* ADSL2 connection parameters */

#define kAdsl2ModeAnnexMask                 0xF

#define kAdsl2ModeAnnexMUp32                1
#define kAdsl2ModeAnnexMUp36                2 
#define kAdsl2ModeAnnexMUp40                3 
#define kAdsl2ModeAnnexMUp44                4
#define kAdsl2ModeAnnexMUp48                5
#define kAdsl2ModeAnnexMUp52                6
#define kAdsl2ModeAnnexMUp56                7
#define kAdsl2ModeAnnexMUp60                8
#define kAdsl2ModeAnnexMUp64                9

#define kAdsl2ModeAnnexLShift               4
#define kAdsl2ModeAnnexLMask                (0x3 << kAdsl2ModeAnnexLShift)

#define kAdsl2ModeAnnexLUpMask              (0x00000001 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLUpWide              (0x00000000 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLUpNarrow            (0x00000001 << kAdsl2ModeAnnexLShift)

#define kAdsl2ModeAnnexLDnMask              (0x00000002 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLDnNonOvlap          (0x00000000 << kAdsl2ModeAnnexLShift)
#define kAdsl2ModeAnnexLDnOvlap             (0x00000002 << kAdsl2ModeAnnexLShift)

typedef struct _adsl2DelayInp{
    unsigned short delay;
    unsigned short inp;
} adsl2DelayInp;

typedef struct _adsl2ConnectionInfo {
    long                    adsl2Mode;
    long                    rcvRate;
    long                    xmtRate;
    unsigned char           pwrState;           /* Lx state: x = 0..3 */
    adsl2DataConnectionInfo rcv2Info;
    adsl2DataConnectionInfo xmt2Info;
    adsl2DelayInp	rcv2DelayInp;
    adsl2DelayInp	xmt2DelayInp;	
} adsl2ConnectionInfo;

typedef struct _adslConnectionDataStat {
    unsigned long           cntRS;  
    unsigned long           cntRSCor;   
    unsigned long           cntRSUncor; 
    unsigned long           cntSF;  
    unsigned long           cntSFErr;
} adslConnectionDataStat;

typedef struct _adslConnectionBitswapStat {
    unsigned long	rcvCnt;
    unsigned long	xmtCnt;
    unsigned long	reserved[6];
}adslConnectionBitswapStat;

typedef struct _adslFireStat {
    unsigned long	status;
    unsigned long	reXmtRSCodewordsRcved;
    unsigned long	reXmtUncorrectedRSCodewords;
    unsigned long	reXmtCorrectedRSCodewords;
    unsigned long	reXmtRSCodewordsRcvedUS;
    unsigned long	reXmtUncorrectedRSCodewordsUS;
    unsigned long	reXmtCorrectedRSCodewordsUS;
} adslFireStat;

typedef struct _adslConnectionStat {
    adslConnectionDataStat  rcvStat;
    adslConnectionDataStat  xmtStat;
    adslConnectionBitswapStat bitswapStat;
    adslFireStat                          fireStat;
} adslConnectionStat;

typedef struct _atmConnectionDataStat {
    unsigned long           cntHEC;
    unsigned long           cntOCD;
    unsigned long           cntLCD;
    unsigned long           cntES;
    unsigned long           cntCellTotal;
    unsigned long           cntCellData;
    unsigned long           cntCellDrop;
    unsigned long           cntBitErrs;
} atmConnectionDataStat;

typedef struct _atmConnectionStat {
    atmConnectionDataStat   rcvStat;
    atmConnectionDataStat   xmtStat;
} atmConnectionStat;

#define kAdslFramingModeMask            0x0F
#define kAtmFramingModeMask             0xF0
#define kAtmHeaderCompression           0x80

/* ADSL2 data */

typedef struct _adslDiagModeData {
    long                    loopAttn;
    long                    signalAttn;
    long                    snrMargin;
    long                    attnDataRate;
    long                    actXmtPower;
    long                    hlinScaleFactor;
} adslDiagModeData;

/* AdslMibGetObjectValue return codes */

#define kAdslMibStatusSuccess           0
#define kAdslMibStatusFailure           -1
#define kAdslMibStatusNoObject          -2
#define kAdslMibStatusObjectInvalid     -3
#define kAdslMibStatusBufferTooSmall    -4
#define kAdslMibStatusLastError         -4

/* Adsl training codes */

#define kAdslTrainingIdle               0
#define kAdslTrainingG994               1
#define kAdslTrainingG992Started        2
#define kAdslTrainingG992ChanAnalysis   3
#define kAdslTrainingG992Exchange       4
#define kAdslTrainingConnected          5

/* Global info structure */

typedef struct _adslMibInfo {
    adslLineEntry           adslLine;
    adslPhysEntry           adslPhys;
    adslChanEntry           adslChanIntl;
    adslChanEntry           adslChanFast;
    adslPerfDataEntry       adslPerfData;
    adslPerfCounters        adslPerfIntervals[kAdslMibPerfIntervals];
    adslChanPerfDataEntry   adslChanIntlPerfData;
    adslChanPerfDataEntry   adslChanFastPerfData;
    adslChanCounters        adslChanIntlPerfIntervals[kAdslMibChanPerfIntervals];
    adslChanCounters        adslChanFastPerfIntervals[kAdslMibChanPerfIntervals];

    adslThreshCounters      adslAlarm;

    atmPhyDataEntrty        adslChanIntlAtmPhyData;
    atmPhyDataEntrty        adslChanFastAtmPhyData;

    adslBertResults         adslBertRes;

    adslConnectionInfo      adslConnection;
    adslConnectionStat      adslStat;
    adslConnectionStat      adslStatSincePowerOn;
    unsigned char           adslTrainingState;
    atmConnectionStat       atmStat;
    atmConnectionStat       atmStatSincePowerOn;

    adslFullPhysEntry       adslAtucPhys;
    unsigned char           adslRxNonStdFramingAdjustK;
    unsigned char           adslFramingMode;
    adslBertStatusEx        adslBertStatus;
    long                    afeRxPgaGainQ1;

    adslDiagModeData        adslDiag;
    adsl2ConnectionInfo     adsl2Info;
    adslPerfCounters        adslTxPerfTotal;
    adslPerfCounters        adslTxPerfLast15Min;
    adslPerfCounters        adslTxPerfLast1Day;
    adslNonLinearityData    adslNonLinData;
	#ifdef ADSL_MIBOBJ_PLN
    adslPLNDataEntry        adslPLNData;
	#endif
} adslMibInfo;

#if defined(__cplusplus)
}
#endif

#endif  /* AdslMibDefHeader */
