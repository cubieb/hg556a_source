/*
 *  SDIO spec header file
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: sdio.h,v 1.1 2008/08/25 06:42:10 l65130 Exp $
 */

#ifndef	_SDIO_H
#define	_SDIO_H

/* CCCR structure for function 0 */
typedef volatile struct {
	uint8	cccr_sdio_rev;	/* RO, cccr and sdio revision */
	uint8	sd_rev;		/* RO, sd spec revision */
	uint8	io_en;		/* I/O enable */
	uint8	io_rdy;		/* I/O ready reg */
	uint8	intr_ctl;	/* Master and per function interrupt enable control */
	uint8	intr_status;	/* RO, interrupt status */
	uint8	io_abort;	/* read/write abort or reset all functions */
	uint8	bus_inter;	/* bus interface control */
	uint8	capability;	/* RO, card capability */

        uint8	cis_base_low;	/* 0x9 RO, common CIS base address, LSB */
	uint8	cis_base_mid;	
	uint8	cis_base_high;	/* 0xB RO, common CIS base address, MSB */

	/* suspend/resume registers */	
	uint8	bus_suspend;	/* 0xC */
	uint8	func_select;	/* 0xD */
	uint8	exec_flag;	/* 0xE */
	uint8	ready_flag;	/* 0xF */

	uint8	fn0_blk_size[2];/* 0x10, 0x11 */
#ifdef DSLCPE /*for 4312*/
        uint8   reserved;
        uint8   hs_ctl;         /*highspeed control*/
#endif	
} sdio_regs_t;

#define SDIO_CCCR_REV			0x0
#define SDIO_SDREV			0x1
#define SDIO_IO_ENABLE			0x2
#define SDIO_IO_READY			0x3
#define SDIO_INT_ENABLE			0x4
#define SDIO_INT_PENDING		0x5
#define SDIO_IO_ABORT			0x6
#define SDIO_IO_BUS_INTERFACE		0x7
#define SDIO_CARD_CAPABILITY		0x8
#define SDIO_CIS_COMMON_BYTE1		0x9
#define SDIO_CIS_COMMON_BYTE2		0xA
#define SDIO_CIS_COMMON_BYTE3		0xB
#define SDIO_BUS_SUSPEND		0xC
#define SDIO_FUNC_SELECT		0xD
#define SDIO_EXEC_FLAGS			0xE
#define SDIO_READY_FLAGS		0xF
#define SDIO_FN0_BLK_SIZE_BYTE1		0x10
#define SDIO_FN0_BLK_SIZE_BYTE2		0x11

/* cccr_sdio_rev */
#define SDIO_REV_SDIOID_MASK			0xf0	/* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK			0x0f	/* CCCR format version number */

/* sd_rev */
#define SD_REV_PHY_MASK				0x0f	/* SD format version number */

/* io_en */
#define SDIO_FUNC_ENABLE_1			0x02	/* funcion 1 I/O enable */

/* io_rdys */
#define SDIO_FUNC_READY_1			0x02	/* funcion 1 I/O ready */

/* intr_ctl */
#define INTR_CTL_FUNC1_EN			0x2	/* interrupt enable for function 1 */
#define INTR_CTL_MASTER_EN			0x1	/* interrutp enable master */

/* intr_status */
#define INTR_STATUS_FUNC1			0x2	/* interrupt pending for function 1 */

/* io_abort */
#define IO_ABORT_RESET_ALL			0x08	/* I/O card reset */
#define IO_ABORT_FUNC_MASK			0x07	/* abort selction: function x */

/* bus_inter */
#define BUS_CARD_DETECT_DIS			0x80	/* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP			0x40	/* support continuous SPI interrtup */
#define BUS_SPI_CONT_INTR_EN			0x20	/* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK			0x03	/* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT			0x02	/* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT			0x00	/* bus width 1-bit mode */

/* capability */
#define SDIO_CAP_4BLS				0x80	/* 4-bit support for low speed card */
#define SDIO_CAP_LSC				0x40	/* low speed card */
#define SDIO_CAP_E4MI				0x20	/* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI				0x10	/* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS				0x08	/* support suspend/resume */
#define SDIO_CAP_SRW				0x04	/* support read wait */
#define SDIO_CAP_SMB				0x02	/* support multi-block transfer */
#define SDIO_CAP_SDC				0x01	/* Support Direct commands during multi-byte transfer */

/* CIA FBR1 registers */
#define SDIO_FUNC1_INFO				0x100	/* basic info for function 1 */
#define SDIO_FUNC1_EXT				0x101	/* extension of standard I/O device code */
#define SDIO_CIS_FUNC1_BASE_LOW			0x109	/* function 1 cis base address bit 0-7 */
#define SDIO_CIS_FUNC1_BASE_MID			0x10A   /* function 1 cis base address bit 8-15 */
#define SDIO_CIS_FUNC1_BASE_HIGH		0x10B	/* function 1 cis base address bit 16 */
#define SDIO_CSA_BASE_LOW			0x10C	/* CSA base address byte 0 */
#define SDIO_CSA_BASE_MID			0x10D	/* CSA base address byte 1 */
#define SDIO_CSA_BASE_HIGH			0x10E	/* CSA base address byte 2 */
#define SDIO_CSA_DATA_OFFSET			0x10F	/* CSA data register */
#define SDIO_IO_BLK_SIZE_LOW			0x110	/* I/O block size byte 0 */
#define SDIO_IO_BLK_SIZE_HIGH			0x111	/* I/O block size byte 1 */

/* SDIO_FUNC1_INFO bits */
#define SDIO_FUNC1_INFO_DIC			0x0f	/* device interface code */
#define SDIO_FUNC1_INFO_CSA			0x40	/* CSA support flag */
#define SDIO_FUNC1_INFO_CSA_EN			0x80	/* CSA enabled */

/* SDIO_FUNC1_EXT bits */
#define SDIO_FUNC1_EXT_SHP			0x03	/* support high power */
#define SDIO_FUNC1_EXT_EHP			0x04	/* enable high power */

/* FBR structure for function 1-7 */
typedef volatile struct {
	uint8	devctr;		/* device interface, CSA control */
	uint8	ext_dev;	/* extended standard I/O device type code*/
	uint8	high_power;	/* support high power */
	uint8	PAD[6];		/* reserved */

	uint8	cis_low;	/* CIS LSB */
	uint8	cis_mid;	
	uint8	cis_high;	/* CIS MSB */
	uint8	csa_low;	/* code storage area, LSB */
	uint8	csa_mid;	
	uint8	csa_high;	/* code storage area, MSB */
	uint8	csa_dat_win;	/* data access window to function */

	uint8	fnx_blk_size[2];/* block size, little endian */
} sdio_fbr_t;

/* Maximum number of I/O funcs */
#define SDIOD_MAX_IOFUNCS		7

/* SDIO Device FBR Start Address  */
#define SDIOD_FBR_STARTADDR		0x100

/* SDIO Device FBR Size */
#define SDIOD_FBR_SIZE			0x100

/* Macro to calculate FBR register base */
#define FBR_REG_BASE(n)			(n * 0x100)

/* Function register offsets */
#define SDIOD_FBR_DEVCTR		0x00	/* basic info for function */
#define SDIOD_FBR_EXT_DEV		0x01	/* extended I/O device code */
#define SDIOD_FBR_PWR_SEL		0x02	/* power selection bits */

/* SDIO Function CIS ptr offset */
#define SDIOD_FBR_CISPTR_0		0x09
#define SDIOD_FBR_CISPTR_1		0x0A
#define SDIOD_FBR_CISPTR_2		0x0B

/* Code Storage Area pointer */
#define SDIOD_FBR_CSA_ADDR_0		0x0C
#define SDIOD_FBR_CSA_ADDR_1		0x0D
#define SDIOD_FBR_CSA_ADDR_2		0x0E
#define SDIOD_FBR_CSA_DATA		0x0F

/* XXX At the moment the FBR BLKSIZE is not used; drivers using offset */
/* XXX defines use the CCCR_F0 ones.  Should change that, or add an */
/* XXX #error if they don't match (or something). */

/* SDIO Function I/O Block Size */
#define SDIOD_FBR_BLKSIZE_0		0x10
#define SDIOD_FBR_BLKSIZE_1		0x11

/* devctr */
#define SD_DEVCTR_DEVINTER	0xf	/* I/O device interface code */
#define SD_DEVCTR_CSA_SUP	0x40	/* support CSA */
#define SD_DEVCTR_CSA_EN	0x80	/* enable CSA */
#define SD_INTER_NONE		0	/* SDIO standard interface is not supported */
#define SD_INTER_UART		1
#define SD_INTER_BLUETOOTH_A	2
#define SD_INTER_BLUETOOTH_B	3
#define SD_INTER_GPS		4
#define SD_INTER_CAMERA		5
#define SD_INTER_PHS		6
#define SD_INTER_WLAN		7
#define SD_INTER_EXT		0xf	/* extended device interface, read ext_dev register */

/* ext_dev */
#define SD_HIGHPWR_SUPPORT_M	0x3	/* supports high-power mask */
#define SD_HIGHPWR_EN		0x4	/* enable high power */
#define SD_HP_STD		0	/* standard power function(up to 200mA */
#define SD_HP_REQUIRED		0x2	/* need high power to operate */
#define SD_HP_DESIRED		0x3	/* can work with standard power, but prefer high power */

/* misc define */
#define FBR_REG_BASE(n)		(n*0x100)	/* macro to calculate fbr register base */
#define SDIO_FUNC_0			0
#define SDIO_FUNC_1			1
#define SDIO_FUNC_2			2
#define SDIO_FUNC_3			3
#define SDIO_FUNC_4			4
#define SDIO_FUNC_5			5
#define SDIO_FUNC_6			6
#define SDIO_FUNC_7			7

#define SD_CARD_TYPE_UNKNOWN		0	/* bad type or unrecognized */
#define SD_CARD_TYPE_IO			1	/* IO only card */
#define SD_CARD_TYPE_MEMORY		2	/* memory only card */
#define SD_CARD_TYPE_COMBO		3	/* IO and memory combo card */

#define SDIO_MAX_BLOCK_SIZE		2048	/* maximum block size for block mode operation */
#define SDIO_MIN_BLOCK_SIZE		1	/* minimum block size for block mode operation */

/* Card registers: status bit position */
#define CARDREG_STATUS_BIT_OUTOFRANGE		31
#define CARDREG_STATUS_BIT_COMCRCERROR		23
#define CARDREG_STATUS_BIT_ILLEGALCOMMAND	22
#define CARDREG_STATUS_BIT_ERROR		19
#define CARDREG_STATUS_BIT_IOCURRENTSTATE3	12
#define CARDREG_STATUS_BIT_IOCURRENTSTATE2	11
#define CARDREG_STATUS_BIT_IOCURRENTSTATE1	10
#define CARDREG_STATUS_BIT_IOCURRENTSTATE0	9
#define CARDREG_STATUS_BIT_FUN_NUM_ERROR	4

/* SDIO command parameters */

#define SD_CMD_GO_IDLE_STATE		0	/* mandatory for SDIO */
#define SD_CMD_SEND_OPCOND		1 
#define SD_CMD_MMC_SET_RCA		3 
#define SD_CMD_IO_SEND_OP_COND		5	/* mandatory for SDIO */
#define SD_CMD_SELECT_DESELECT_CARD	7
#define SD_CMD_SEND_CSD			9 
#define SD_CMD_SEND_CID			10
#define SD_CMD_STOP_TRANSMISSION	12
#define SD_CMD_SEND_STATUS		13
#define SD_CMD_GO_INACTIVE_STATE	15
#define SD_CMD_SET_BLOCKLEN		16
#define SD_CMD_READ_SINGLE_BLOCK	17
#define SD_CMD_READ_MULTIPLE_BLOCK	18
#define SD_CMD_WRITE_BLOCK		24
#define SD_CMD_WRITE_MULTIPLE_BLOCK	25
#define SD_CMD_PROGRAM_CSD		27
#define SD_CMD_SET_WRITE_PROT		28
#define SD_CMD_CLR_WRITE_PROT		29
#define SD_CMD_SEND_WRITE_PROT		30
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_ERASE			38
#define SD_CMD_LOCK_UNLOCK		42
#define SD_CMD_IO_RW_DIRECT		52	/* mandatory for SDIO */
#define SD_CMD_IO_RW_EXTENDED		53	/* mandatory for SDIO */
#define SD_CMD_APP_CMD			55
#define SD_CMD_GEN_CMD			56
#define SD_CMD_READ_OCR			58
#define SD_CMD_CRC_ON_OFF		59	/* mandatory for SDIO */
#define SD_ACMD_SD_STATUS		13
#define SD_ACMD_SEND_NUM_WR_BLOCKS	22
#define SD_ACMD_SET_WR_BLOCK_ERASE_CNT	23
#define SD_ACMD_SD_SEND_OP_COND		41
#define SD_ACMD_SET_CLR_CARD_DETECT	42
#define SD_ACMD_SEND_SCR		51

	/* argument for SD_CMD_IO_RW_DIRECT and SD_CMD_IO_RW_EXTENDED */
#define SD_IO_OP_READ		0   /* Read_Write */
#define SD_IO_OP_WRITE		1   /* Read_Write */
#define SD_IO_RW_NORMAL		0   /* no RAW */
#define SD_IO_RW_RAW		1   /* RAW */
#define SD_IO_BYTE_MODE		0   /* Byte Mode */
#define SD_IO_BLOCK_MODE	1   /* BlockMode */
#define SD_IO_FIXED_ADDRESS	0   /* fix Address */
#define SD_IO_INCREMENT_ADDRESS	1   /* IncrementAddress */

	/* build SD_CMD_IO_RW_DIRECT Argument */
#define SDIO_IO_RW_DIRECT_ARG(rw, raw, func, addr, data) \
	(((rw & 1) << 31) | ((func & 0x7) << 28) | ((raw & 1) << 27) | ((addr & 0x1FFFF) << 9) | (data & 0xFF))

	/* build SD_CMD_IO_RW_EXTENDED Argument */
#define SDIO_IO_RW_EXTENDED_ARG(rw, blk, func, addr, inc_addr, count) \
	(((rw & 1) << 31) | ((func & 0x7) << 28) | ((blk & 1) << 27) | ((inc_addr & 1) << 26) | ((addr & 0x1FFFF) << 9) | (count & 0x1FF))

/* SDIO response parameters */
#define SD_RSP_NO_NONE			0
#define SD_RSP_NO_1			1
#define SD_RSP_NO_2			2
#define SD_RSP_NO_3			3
#define SD_RSP_NO_4			4
#define SD_RSP_NO_5			5
#define SD_RSP_NO_6			6

	/* Modified R6 response (to CMD3) */
#define SD_RSP_MR6_COM_CRC_ERROR	0x8000
#define SD_RSP_MR6_ILLEGAL_COMMAND	0x4000
#define SD_RSP_MR6_ERROR		0x2000

	/* Modified R1 in R4 Response (to CMD5) */
#define SD_RSP_MR1_SBIT			0x80
#define SD_RSP_MR1_PARAMETER_ERROR	0x40
#define SD_RSP_MR1_RFU5			0x20
#define SD_RSP_MR1_FUNC_NUM_ERROR	0x10
#define SD_RSP_MR1_COM_CRC_ERROR	0x80
#define SD_RSP_MR1_ILLEGAL_COMMAND	0x40
#define SD_RSP_MR1_RFU1			0x20
#define SD_RSP_MR1_IDLE_STATE		0x01

	/* R5 response (to CMD52 and CMD53) */
#define SD_RSP_R5_COM_CRC_ERROR		0x80
#define SD_RSP_R5_ILLEGAL_COMMAND	0x40
#define SD_RSP_R5_IO_CURRENTSTATE1	0x20
#define SD_RSP_R5_IO_CURRENTSTATE0	0x10
#define SD_RSP_R5_ERROR			0x80
#define SD_RSP_R5_RFU			0x40
#define SD_RSP_R5_FUNC_NUM_ERROR	0x20
#define SD_RSP_R5_OUT_OF_RANGE		0x01

#endif /* _SDIO_H */
