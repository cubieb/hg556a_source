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

/*******************************************************************
 * DiagDef.h
 * 
 *	Description:
 *		Diag definitions
 *
 * $Revision: 1.1 $
 *
 * $Id: DiagDef.h,v 1.1 2008/08/25 06:41:10 l65130 Exp $
 *
 * $Log: DiagDef.h,v $
 * Revision 1.1  2008/08/25 06:41:10  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:28  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:48:04  z30370
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
 * Revision 1.30  2007/03/05 18:33:28  tonytran
 * Added DIAG_DEBUG_CMD_SET_L2_TIMEOUT
 *
 * Revision 1.29  2007/02/22 05:00:17  ilyas
 * Added logr - log after reset command to do TEQ logging after PHY reset or download
 *
 * Revision 1.28  2006/08/08 20:04:14  dadityan
 * ^M clean up
 *
 * Revision 1.27  2006/08/08 08:01:44  dadityan
 * Diags Pass OEM Param
 *
 * Revision 1.26  2006/06/12 22:26:11  ilyas
 * Updated version number and made release 21
 *
 * Revision 1.25  2006/03/31 16:32:41  ovandewi
 * changes for PLN and NL from MIB
 *
 * Revision 1.24  2006/03/31 09:08:58  dadityan
 * Added LOG_CMD_CFG_PHY2
 *
 * Revision 1.23  2005/07/14 23:43:20  ilyas
 * Added command to start data logging
 *
 * Revision 1.22  2004/10/16 23:43:19  ilyas
 * Added playback resume command
 *
 * Revision 1.21  2004/10/16 23:24:08  ilyas
 * Improved FileRead command support for LOG file playback (RecordTest on the board)
 *
 * Revision 1.20  2004/04/28 16:52:32  ilyas
 * Added GDB frame processing
 *
 * Revision 1.19  2004/03/10 22:26:53  ilyas
 * Added command-line parameter for IP port number.
 * Added proxy remote termination
 *
 * Revision 1.18  2004/01/24 23:41:37  ilyas
 * Added DIAG_DEBUG_CMD_LOG_SAMPLES debug command
 *
 * Revision 1.17  2003/11/19 02:25:45  ilyas
 * Added definitions for LOG frame retransmission, time, ADSL2 plots
 *
 * Revision 1.16  2003/11/14 18:46:05  ilyas
 * Added G992p3 debug commands
 *
 * Revision 1.15  2003/10/02 19:50:41  ilyas
 * Added support for buffering data for AnnexI and statistical counters
 *
 * Revision 1.14  2003/09/03 19:45:11  ilyas
 * To refuse connection with older protocol versions
 *
 * Revision 1.13  2003/08/30 00:12:39  ilyas
 * Added support for running chip test regressions via DslDiags
 *
 * Revision 1.12  2003/08/12 00:19:28  ilyas
 * Improved image downloading protocol.
 * Added DEBUG command support
 *
 * Revision 1.11  2003/04/11 00:37:24  ilyas
 * Added DiagProtoFrame definition
 *
 * Revision 1.10  2003/03/25 00:10:07  ilyas
 * Added command for "long" BERT test
 *
 * Revision 1.9  2003/01/30 03:29:32  ilyas
 * Added PHY_CFG support and fixed printing showtime counters
 *
 * Revision 1.8  2002/12/16 20:56:38  ilyas
 * Added support for binary statuses
 *
 * Revision 1.7  2002/12/06 20:19:13  ilyas
 * Added support for binary statuses and scrambled status strings
 *
 * Revision 1.6  2002/11/05 00:18:27  ilyas
 * Added configuration dialog box for Eye tone selection.
 * Added Centillium CRC workaround to AnnexC config dialog
 * Bit allocation update on bit swap messages
 *
 * Revision 1.5  2002/07/30 23:23:43  ilyas
 * Implemented DIAG configuration command for AnnexA and AnnexC
 *
 * Revision 1.4  2002/07/30 22:47:15  ilyas
 * Added DIAG command for configuration
 *
 * Revision 1.3  2002/07/15 23:52:51  ilyas
 * iAdded switch RJ11 pair command
 *
 * Revision 1.2  2002/04/25 17:55:51  ilyas
 * Added mibGet command
 *
 * Revision 1.1  2002/04/02 22:56:39  ilyas
 * Support DIAG connection at any time; BERT commands
 *
 *
 ******************************************************************/

#define	LOG_PROTO_ID				"*L"

#define	DIAG_PARTY_ID_MASK			0x01
#define	LOG_PARTY_CLIENT			0x01
#define	LOG_PARTY_SERVER			0x00

#define	DIAG_DATA_MASK				0x0E
#define	DIAG_DATA_LOG				0x02
#define	DIAG_DATA_EYE				0x04
#define	DIAG_DATA_LOG_TIME			0x08
#define DIAG_DATA_GUI_ID			0x10	

#define	DIAG_DATA_EX				0x80
#define	DIAG_PARTY_ID_MASK_EX		(DIAG_DATA_EX | DIAG_PARTY_ID_MASK)
#define	LOG_PARTY_SERVER_EX			(DIAG_DATA_EX | LOG_PARTY_SERVER)

#define	DIAG_ACK_FRAME_ACK_MASK		0x000000FF
#define	DIAG_ACK_FRAME_RCV_SHIFT	8
#define	DIAG_ACK_FRAME_RCV_MASK		0x0000FF00
#define	DIAG_ACK_FRAME_RCV_PRESENT	0x00010000
#define	DIAG_ACK_TIMEOUT			-1
#define	DIAG_ACK_LEN_INDICATION		-1

#define LOG_CMD_CFG_PHY2			235
#define	LOG_CMD_GDB					236
#define	LOG_CMD_PROXY				237
#define	LOG_CMD_RETR				238
#define	LOG_CMD_DEBUG				239
#define	LOG_CMD_BERT_EX				240
#define	LOG_CMD_CFG_PHY				241
#define	LOG_CMD_RESET				242
#define	LOG_CMD_SCRAMBLED_STRING	243
#define	LOG_CMD_EYE_CFG				244
#define	LOG_CMD_CONFIG_A			245
#define	LOG_CMD_CONFIG_C			246
#define	LOG_CMD_SWITCH_RJ11_PAIR	247
#define	LOG_CMD_MIB_GET				248
#define	LOG_CMD_LOG_STOP			249
#define	LOG_CMD_PING_REQ			250
#define	LOG_CMD_PING_RSP			251
#define	LOG_CMD_DISCONNECT			252
#define	LOG_CMD_STRING_DATA			253
#define	LOG_CMD_TEST_DATA			254
#define	LOG_CMD_CONNECT				255


typedef struct _LogProtoHeader {
	unsigned char	logProtoId[2];
	unsigned char	logPartyId;
	unsigned char	logCommmand;
} LogProtoHeader;

#define	LOG_FILE_PORT			5100
#define	LOG_FILE_PORT2			5099
#define	LOG_MAX_BUF_SIZE		1400
#define	LOG_MAX_DATA_SIZE		(LOG_MAX_BUF_SIZE - sizeof(LogProtoHeader))

typedef struct {
	LogProtoHeader	diagHdr;
	unsigned char	diagData[LOG_MAX_DATA_SIZE];
} DiagProtoFrame;

#define	DIAG_PROXY_TERMINATE				1

#define	DIAG_DEBUG_CMD_READ_MEM				1
#define	DIAG_DEBUG_CMD_SET_MEM				2
#define	DIAG_DEBUG_CMD_RESET_CONNECTION		3
#define	DIAG_DEBUG_CMD_RESET_PHY			4
#define	DIAG_DEBUG_CMD_RESET_CHIP			5
#define	DIAG_DEBUG_CMD_EXEC_FUNC			6
#define	DIAG_DEBUG_CMD_EXEC_ADSL_FUNC		7
#define	DIAG_DEBUG_CMD_WRITE_FILE			8
#define	DIAG_DEBUG_CMD_G992P3_DEBUG			9
#define	DIAG_DEBUG_CMD_G992P3_DIAG_MODE		10
#define	DIAG_DEBUG_CMD_CLEAR_TIME			11
#define	DIAG_DEBUG_CMD_PRINT_TIME			12
#define	DIAG_DEBUG_CMD_LOG_SAMPLES			13

#define	DIAG_DEBUG_CMD_PLAYBACK_STOP		14
#define	DIAG_DEBUG_CMD_PLAYBACK_RESUME		15

#define	DIAG_DEBUG_CMD_LOG_DATA				16
#define	DIAG_DEBUG_CMD_LOG_AFTER_RESET		 1

#define DIAG_DEBUG_CMD_CLEAREOC_LOOPBACK	17
#define	DIAG_DEBUG_CMD_ANNEXM_CFG			18

#define	DIAG_DEBUG_CMD_PRINT_STAT			21
#define	DIAG_DEBUG_CMD_CLEAR_STAT			22
#define DIAG_DEBUG_CMD_SET_OEM				23
#define DIAG_DEBUG_CMD_SET_L2_TIMEOUT			24

typedef struct {
	unsigned short	cmd;
	unsigned short	cmdId;
	unsigned long	param1;
	unsigned long	param2;
	unsigned char	diagData[1];
} DiagDebugData;

#define	DIAG_TEST_CMD_LOAD					101
#define	DIAG_TEST_CMD_READ					102
#define	DIAG_TEST_CMD_WRITE					103
#define	DIAG_TEST_CMD_APPEND				104
#define	DIAG_TEST_CMD_TEST_COMPLETE			105

#define	DIAG_TEST_FILENAME_LEN				64

typedef struct {
	unsigned short	cmd;
	unsigned short	cmdId;
	unsigned long	offset;
	unsigned long	len;
	unsigned long	bufPtr;
	char			fileName[DIAG_TEST_FILENAME_LEN];
} DiagTestData;

typedef struct {
	unsigned long	frStart;
	unsigned long	frNum;
} DiagLogRetrData;
