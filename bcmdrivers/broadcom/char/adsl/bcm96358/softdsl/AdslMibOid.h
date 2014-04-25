/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * AdslMibOid.h 
 *
 * Description:
 *	SNMP object identifiers for ADSL MIB and other related MIBs
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslMibOid.h,v 1.1.2.1 2009/11/19 06:39:13 l43571 Exp $
 *
 * $Log: AdslMibOid.h,v $
 * Revision 1.1.2.1  2009/11/19 06:39:13  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:41:01  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:21  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:16  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:55  z45221
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
 * Revision 1.5  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.4  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.3  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.2  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#ifndef	AdslMibOidHeader
#define	AdslMibOidHeader

#define kOidAdsl							94
#define kOidAdslInterleave					124
#define kOidAdslFast						125
#define kOidAtm								37
#define kOidAdslPhyCfg						95

#define kOidAdslLine						1
#define kOidAdslMibObjects					1

#define kOidAdslLineTable					1
#define kOidAdslLineEntry					1
#define kOidAdslLineCoding					1
#define kOidAdslLineType					2
#define kOidAdslLineSpecific			    3
#define kOidAdslLineConfProfile				4
#define kOidAdslLineAlarmConfProfile		5

#define kOidAdslAtucPhysTable				2
#define kOidAdslAturPhysTable				3
#define kOidAdslPhysEntry					1
#define kOidAdslPhysInvSerialNumber     	1
#define kOidAdslPhysInvVendorID             2
#define kOidAdslPhysInvVersionNumber    	3
#define kOidAdslPhysCurrSnrMgn          	4
#define kOidAdslPhysCurrAtn             	5
#define kOidAdslPhysCurrStatus          	6
#define kOidAdslPhysCurrOutputPwr       	7
#define kOidAdslPhysCurrAttainableRate  	8

#define kOidAdslAtucChanTable				4
#define kOidAdslAturChanTable				5
#define kOidAdslChanEntry					1
#define kOidAdslChanInterleaveDelay			1
#define kOidAdslChanCurrTxRate				2
#define kOidAdslChanPrevTxRate          	3
#define kOidAdslChanCrcBlockLength      	4

#define kOidAdslAtucPerfDataTable			6
#define kOidAdslAturPerfDataTable			7
#define kOidAdslPerfDataEntry				1
#define kOidAdslPerfLofs                 	1
#define kOidAdslPerfLoss                 	2
#define kOidAdslPerfLprs                 	3
#define kOidAdslPerfESs                  	4
#define kOidAdslPerfValidIntervals          5
#define kOidAdslPerfInvalidIntervals     	6
#define kOidAdslPerfCurr15MinTimeElapsed 	7
#define kOidAdslPerfCurr15MinLofs        	8
#define kOidAdslPerfCurr15MinLoss        	9
#define kOidAdslPerfCurr15MinLprs        	10
#define kOidAdslPerfCurr15MinESs         	11
#define kOidAdslPerfCurr1DayTimeElapsed     12
#define kOidAdslPerfCurr1DayLofs         	13
#define kOidAdslPerfCurr1DayLoss         	14
#define kOidAdslPerfCurr1DayLprs         	15
#define kOidAdslPerfCurr1DayESs          	16
#define kOidAdslPerfPrev1DayMoniSecs     	17
#define kOidAdslPerfPrev1DayLofs         	18
#define kOidAdslPerfPrev1DayLoss            19
#define kOidAdslPerfPrev1DayLprs         	20
#define kOidAdslPerfPrev1DayESs          	21

#define kOidAdslAtucPerfIntervalTable		8
#define kOidAdslAturPerfIntervalTable		9
#define kOidAdslPerfIntervalEntry			1
#define kOidAdslIntervalNumber				1
#define kOidAdslIntervalLofs				2
#define kOidAdslIntervalLoss				3
#define kOidAdslIntervalLprs				4
#define kOidAdslIntervalESs					5
#define kOidAdslIntervalValidData			6

#define kOidAdslAtucChanPerfTable					10
#define kOidAdslAturChanPerfTable					11
#define kOidAdslChanPerfEntry						1
#define kOidAdslChanReceivedBlks                 	1
#define kOidAdslChanTransmittedBlks              	2
#define kOidAdslChanCorrectedBlks                	3
#define kOidAdslChanUncorrectBlks                	4
#define kOidAdslChanPerfValidIntervals           	5
#define kOidAdslChanPerfInvalidIntervals         	6
#define kOidAdslChanPerfCurr15MinTimeElapsed     	7
#define kOidAdslChanPerfCurr15MinReceivedBlks    	8
#define kOidAdslChanPerfCurr15MinTransmittedBlks 	9
#define kOidAdslChanPerfCurr15MinCorrectedBlks   	10
#define kOidAdslChanPerfCurr15MinUncorrectBlks   	11
#define kOidAdslChanPerfCurr1DayTimeElapsed      	12
#define kOidAdslChanPerfCurr1DayReceivedBlks     	13
#define kOidAdslChanPerfCurr1DayTransmittedBlks  	14
#define kOidAdslChanPerfCurr1DayCorrectedBlks    	15
#define kOidAdslChanPerfCurr1DayUncorrectBlks    	16
#define kOidAdslChanPerfPrev1DayMoniSecs         	17
#define kOidAdslChanPerfPrev1DayReceivedBlks     	18
#define kOidAdslChanPerfPrev1DayTransmittedBlks  	19
#define kOidAdslChanPerfPrev1DayCorrectedBlks    	20
#define kOidAdslChanPerfPrev1DayUncorrectBlks    	21

#define kOidAdslAtucChanIntervalTable				12
#define kOidAdslAturChanIntervalTable				13
#define kOidAdslChanIntervalEntry					1
#define kOidAdslChanIntervalNumber					1
#define kOidAdslChanIntervalReceivedBlks        	2
#define kOidAdslChanIntervalTransmittedBlks     	3
#define kOidAdslChanIntervalCorrectedBlks       	4
#define kOidAdslChanIntervalUncorrectBlks       	5
#define kOidAdslChanIntervalValidData           	6

/* AdslExtra OIDs for kOidAdslPrivate, kOidAdslPrivExtraInfo (defined in AdslMibDef.h) */

#define kOidAdslExtraConnectionInfo					1
#define kOidAdslExtraConnectionStat					2
#define kOidAdslExtraFramingMode					3
#define kOidAdslExtraTrainingState					4
#define kOidAdslExtraNonStdFramingAdjustK			5
#define kOidAdslExtraAtmStat						6
#define kOidAdslExtraDiagModeData					7
#define kOidAdslExtraAdsl2Info						8
#define kOidAdslExtraTxPerfCounterInfo				9

#define kOidAtmMibObjects		1
#define kOidAtmTcTable			4
#define kOidAtmTcEntry			1
#define kOidAtmOcdEvents		1
#define kOidAtmAlarmState		2

#endif	/* AdslMibOidHeader */
