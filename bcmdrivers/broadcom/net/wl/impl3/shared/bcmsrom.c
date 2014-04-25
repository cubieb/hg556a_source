/*
 *  Routines to access SPROM and to parse SROM/CIS variables.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmsrom.c,v 1.1 2008/08/25 06:42:15 l65130 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <stdarg.h>
#include <bcmutils.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <sbpcmcia.h>
#include <pcicfg.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <bcmsrom.h>
#ifdef BCMSDIO
#include <bcmsdh.h>
#include <sbsdio.h>
#include <sdio.h>
#endif /* BCMSDIO */

#include <bcmnvram.h>
#include <bcmotp.h>

#if defined(BCMUSBDEV)
#include <sbsdio.h>
#include <sbhnddma.h>
#include <sbsdpcmdev.h>
#endif /* defined(BCMUSBDEV) || defined(BCMSDIO) */

#ifndef DSLCPE
#ifdef WLTEST
#include <sbsprom.h>
#endif /* WLTEST */
#endif /* DSLCPE */

#include <proto/ethernet.h>	/* for sprom content groking */
#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
extern int read_sromfile(uint chipId, void *buf, uint offset, uint nbytes);
#endif

/* debug/trace */
#if defined(BCMDBG_ERR) || defined(WLTEST)
#define	BS_ERROR(args)	printf args
#else
#define	BS_ERROR(args)
#endif	/* BCMDBG_ERR || WLTEST */

#define WRITE_ENABLE_DELAY	500	/* 500 ms after write enable/disable toggle */
#define WRITE_WORD_DELAY	20	/* 20 ms between each word write */

typedef struct varbuf {
	char *buf;		/* pointer to current position */
	unsigned int size;	/* current (residual) size in bytes */
} varbuf_t;

static int initvars_srom_sb(sb_t *sbh, osl_t *osh, void *curmap, char **vars, uint *count);
static void _initvars_srom_pci(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b);
static int initvars_srom_pci(sb_t *sbh, void *curmap, char **vars, uint *count);
static int initvars_cis_pcmcia(sb_t *sbh, osl_t *osh, char **vars, uint *count);
#if !defined(BCMUSBDEV)
static int initvars_flash_sb(sb_t *sbh, char **vars, uint *count);
#endif	
#ifdef BCMSDIO
static int initvars_cis_sdio(osl_t *osh, char **vars, uint *count);
static int sprom_cmd_sdio(osl_t *osh, uint8 cmd);
static int sprom_read_sdio(osl_t *osh, uint16 addr, uint16 *data);
static int sprom_write_sdio(osl_t *osh, uint16 addr, uint16 data);
#endif /* BCMSDIO */
static int sprom_cmd_pcmcia(osl_t *osh, uint8 cmd);
static int sprom_read_pcmcia(osl_t *osh, uint16 addr, uint16 *data);
static int sprom_write_pcmcia(osl_t *osh, uint16 addr, uint16 data);
static int sprom_read_pci(osl_t *osh, sb_t *sbh, uint16 *sprom, uint wordoff, uint16 *buf,
	uint nwords, bool check_crc);

static int initvars_table(osl_t *osh, char *start, char *end, char **vars, uint *count);
static int initvars_flash(sb_t *sbh, osl_t *osh, char **vp, uint len);

#ifdef BCMUSBDEV
static int get_sb_pcmcia_srom(sb_t *sbh, osl_t *osh, uint8 *pcmregs,
                              uint boff, uint16 *srom, uint bsz, bool check_crc);
static int set_sb_pcmcia_srom(sb_t *sbh, osl_t *osh, uint8 *pcmregs,
                              uint boff, uint16 *srom, uint bsz);
static uint srom_size(sb_t *sbh, osl_t *osh);
#endif /* def BCMUSBDEV */

#ifdef	USB4322
char *defaultsromvars1 = "il0macaddr=00:11:22:33:44:51\0"
	"et0macaddr=00:11:22:33:44:52\0"
	"et1macaddr=00:11:22:33:44:53\0"
	"macaddr=00:90:4c:d3:04:03\0"
	"boardrev=0x1203\0"
	"boardflags=0xa00\0"
	"boardflags2=0x0\0"
	"boardtype=0x4a3\0"
	"boardnum=0x403\0"
	"machi=0x90\0"
	"macmid=0x4cd3\0"
	"maclo=0x403\0"
	"ccode=0x0\0"
	"regrev=0x0\0"
	"ledbh0=0xff\0"
	"ledbh1=0xff\0"
	"ledbh2=0xff\0"
	"ledbh3=0xff\0"
	"leddc=0xffff\0"
	"pa0b0=0xfeb1\0"
	"pa0b1=0x1696\0"
	"pa0b2=0xfac7\0"
	"pa0itssit=0x20\0"
	"pa0maxpwr=0x4a\0"
	"opo=0x0\0"
	"aa2g=0x3\0"
	"aa5g=0x3\0"
	"ag0=0x2\0"
	"ag1=0x2\0"
	"ag2=0xff\0"
	"ag3=0xff\0"
	"pa1b0=0xfe8c\0"
	"pa1b1=0x145e\0"
	"pa1b2=0xfabd\0"
	"pa1lob0=0xfe8b\0"
	"pa1lob1=0x1432\0"
	"pa1lob2=0xfacc\0"
	"pa1hib0=0xfe82\0"
	"pa1hib1=0x1389\0"
	"pa1hib2=0xfadd\0"
	"pa1itssit=0x3e\0"
	"pa1maxpwr=0x3c\0"
	"pa1lomaxpwr=0x3c\0"
	"pa1himaxpwr=0x3c\0"
	"bxa2g=0x3\0"
	"rssisav2g=0x7\0"
	"rssismc2g=0xf\0"
	"rssismf2g=0xf\0"
	"bxa5g=0x3\0"
	"rssisav5g=0x7\0"
	"rssismc5g=0xf\0"
	"rssismf5g=0xf\0"
	"tri2g=0xff\0"
	"tri5g=0xff\0"
	"tri5gl=0xff\0"
	"tri5gh=0xff\0"
	"rxpo2g=0xff\0"
	"rxpo5g=0xff\0"
	"txchain=0x3\0"
	"rxchain=0x3\0"
	"antswitch=0x0\0"
	"tssipos2g=0x1\0"
	"extpagain2g=0x0\0"
	"pdetrange2g=0x0\0"
	"triso2g=0x3\0"
	"antswctl2g=0x0\0"
	"tssipos5g=0x1\0"
	"extpagain5g=0x0\0"
	"pdetrange5g=0x0\0"
	"triso5g=0x3\0"
	"antswctl5g=0x0\0"
	"cck2gpo=0x0\0"
	"ofdm2gpo=0x0\0"
	"ofdm5gpo=0x0\0"
	"ofdm5glpo=0x0\0"
	"ofdm5ghpo=0x0\0"
	"mcs2gpo0=0x0\0"
	"mcs2gpo1=0x0\0"
	"mcs2gpo2=0x0\0"
	"mcs2gpo3=0x0\0"
	"mcs2gpo4=0x0\0"
	"mcs2gpo5=0x0\0"
	"mcs2gpo6=0x0\0"
	"mcs2gpo7=0x0\0"
	"mcs5gpo0=0x0\0"
	"mcs5gpo1=0x0\0"
	"mcs5gpo2=0x0\0"
	"mcs5gpo3=0x0\0"
	"mcs5gpo4=0x0\0"
	"mcs5gpo5=0x0\0"
	"mcs5gpo6=0x0\0"
	"mcs5gpo7=0x0\0"
	"mcs5glpo0=0x0\0"
	"mcs5glpo1=0x0\0"
	"mcs5glpo2=0x0\0"
	"mcs5glpo3=0x0\0"
	"mcs5glpo4=0x0\0"
	"mcs5glpo5=0x0\0"
	"mcs5glpo6=0x0\0"
	"mcs5glpo7=0x0\0"
	"mcs5ghpo0=0x0\0"
	"mcs5ghpo1=0x0\0"
	"mcs5ghpo2=0x0\0"
	"mcs5ghpo3=0x0\0"
	"mcs5ghpo4=0x0\0"
	"mcs5ghpo5=0x0\0"
	"mcs5ghpo6=0x0\0"
	"mcs5ghpo7=0x0\0"
	"cddpo=0x0\0"
	"stbcpo=0x0\0"
	"bw40po=0x4\0"
	"bwduppo=0x2\0"
	"maxp2ga0=0x4a\0"
	"itt2ga0=0x20\0"
	"itt5ga0=0x3e\0"
	"pa2gw0a0=0xfeb1\0"
	"pa2gw1a0=0x1696\0"
	"pa2gw2a0=0xfac7\0"
	"maxp5ga0=0x3c\0"
	"maxp5gha0=0x3c\0"
	"maxp5gla0=0x3c\0"
	"pa5gw0a0=0xfe8c\0"
	"pa5gw1a0=0x145e\0"
	"pa5gw2a0=0xfabd\0"
	"pa5glw0a0=0xfe8b\0"
	"pa5glw1a0=0x1432\0"
	"pa5glw2a0=0xfacc\0"
	"pa5ghw0a0=0xfe82\0"
	"pa5ghw1a0=0x1389\0"
	"pa5ghw2a0=0xfadd\0"
	"maxp2ga1=0x4a\0"
	"itt2ga1=0x20\0"
	"itt5ga1=0x3e\0"
	"pa2gw0a1=0xfe93\0"
	"pa2gw1a1=0x165d\0"
	"pa2gw2a1=0xfabe\0"
	"maxp5ga1=0x3c\0"
	"maxp5gha1=0x3c\0"
	"maxp5gla1=0x3c\0"
	"pa5gw0a1=0xfe83\0"
	"pa5gw1a1=0x1417\0"
	"pa5gw2a1=0xfac9\0"
	"pa5glw0a1=0xfe87\0"
	"pa5glw1a1=0x1530\0"
	"pa5glw2a1=0xfa91\0"
	"pa5ghw0a1=0xfe84\0"
	"pa5ghw1a1=0x1532\0"
	"pa5ghw2a1=0xfa88\0"
	"END\0";
#endif	/* USB4322 */

#if defined(WLTEST) || defined(USB4322)
char mfgsromvars[256];
char *defaultsromvars = "il0macaddr=00:11:22:33:44:51\0"
	"et0macaddr=00:11:22:33:44:52\0"
	"et1macaddr=00:11:22:33:44:53\0"
	"boardtype=0x04a2\0"
	"boardrev=0x10\0"
	"boardflags=8\0"
	"sromrev=2\0"
	"aa2g=3\0"
	"\0";
#define	MFGSROM_DEFVARSLEN	149 /* default srom len */
#endif /* WL_TEST */


#ifdef USB4322
/* It must end with pattern of "END" */
static uint
srom_vars_len(char *vars)
{
	uint pos = 0;
	uint len;
	char *s;

	for (s = vars; s && *s;) {

		if (strcmp(s, "END") == 0)
			break;

		len = strlen(s);
		s += strlen(s) + 1;
		pos += len + 1;
		/* BS_ERROR(("len %d vars[pos] %s\n", pos, s)); */
		if (pos > 4000) {
			return 0;
		}
	}

	return pos + 2;	/* include the "END\0" */
}
#endif	/* USB4322 */

/* Initialization of varbuf structure */
static void
BCMINITFN(varbuf_init)(varbuf_t *b, char *buf, uint size)
{
	b->size = size;
	b->buf = buf;
}

/* append a null terminated var=value string */
static int
BCMINITFN(varbuf_append)(varbuf_t *b, const char *fmt, ...)
{
	va_list ap;
	int r;

	if (b->size < 2)
	  return 0;

	va_start(ap, fmt);
	r = vsnprintf(b->buf, b->size, fmt, ap);
	va_end(ap);

	/* C99 snprintf behavior returns r >= size on overflow,
	 * others return -1 on overflow.
	 * All return -1 on format error.
	 * We need to leave room for 2 null terminations, one for the current var
	 * string, and one for final null of the var table. So check that the
	 * strlen written, r, leaves room for 2 chars.
	 */
	if ((r == -1) || (r > (int)(b->size - 2))) {
		b->size = 0;
		return 0;
	}

	/* skip over this string's null termination */
	r++;
	b->size -= r; 
	b->buf += r;

	return r;
}

#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
char wl_srom_present = 1; /* default is present, only wombo board don't have srom */

#define SROM_SCANAREA 12
int wl_probe_srom(osl_t *osh, void *curmap)
{
#ifdef BCMSDIO
	return 0; /* 6338W, 6338W2 doesn't have eeprom/srom */
/*	
	uint i, off, nw, checked_word=0;
	char buf[SROM_SCANAREA]={0};
	off = 0;

	nw = SROM_SCANAREA / 2;	
	for (i = 0; i < nw; i++) {
		if (sprom_read_sdio(osh, (uint16)(off + i), (uint16 *)(buf + i)))
			return 0; //assuming srom is not present
		if (*((uint16 *)(buf+i)) == 0xff || *((uint16 *)(buf+i)) == 0xffff) {
			// 4312/6338w2 returns 0xff, 4318/6338w returns 0xffff
			checked_word++;	
		}
	}
	return (checked_word != nw);
*/	
#else
	int i;
	uint16 tmp;
	uint16 *srom;
		
	if (!curmap) {
		return 1;
	}
	
	srom = (uint16 *)((uint)curmap + PCI_BAR0_SPROM_OFFSET);

	for (i = 0; i < SROM_SCANAREA; i++) {
		tmp = R_REG(osh, &srom[i]);
		if (tmp != 0x0000 && tmp !=0xffff) {
			return 1; /* assuming srom is present */
		}
	}
	return 0;
#endif	
}
#endif
/*
 * Initialize local vars from the right source for this platform.
 * Return 0 on success, nonzero on error.
 */
int
BCMINITFN(srom_var_init)(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
	char **vars, uint *count)
{
	uint len;

	len = 0;

	ASSERT(bustype == BUSTYPE(bustype));
	if (vars == NULL || count == NULL)
		return (0);

	*vars = NULL;
	*count = 0;

	switch (BUSTYPE(bustype)) {
	case SB_BUS:
	case JTAG_BUS:
#ifndef USB4322
		return initvars_srom_sb(sbh, osh, curmap, vars, count);
#else
		len = srom_vars_len(defaultsromvars1);
		*vars = MALLOC(osh, len);
		bcopy(defaultsromvars1, *vars, len);
		*count = len;
		return 0;
		return initvars_srom_sb(sbh, osh, curmap, vars, count);
#endif	/* USB4322 */

	case PCI_BUS:
		ASSERT(curmap);	/* can not be NULL */
		return initvars_srom_pci(sbh, curmap, vars, count);

	case PCMCIA_BUS:
		return initvars_cis_pcmcia(sbh, osh, vars, count);

#ifdef BCMSDIO
	case SDIO_BUS:
		return initvars_cis_sdio(osh, vars, count);
#endif /* BCMSDIO */

	default:
		ASSERT(0);
	}
	return (-1);
}

/* support only 16-bit word read from srom */
int
srom_read(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
          uint byteoff, uint nbytes, uint16 *buf)
{
	void *srom;
	uint i, off, nw;

	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > (SPROM_SIZE * 2))
		return 1;

	off = byteoff / 2;
	nw = nbytes / 2;

	if (BUSTYPE(bustype) == PCI_BUS) {
		if (!curmap)
			return 1;
		srom = (uchar*)curmap + PCI_BAR0_SPROM_OFFSET;
		if (sprom_read_pci(osh, sbh, srom, off, buf, nw, FALSE))
			return 1;
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		for (i = 0; i < nw; i++) {
			if (sprom_read_pcmcia(osh, (uint16)(off + i), (uint16 *)(buf + i)))
				return 1;
		}
#ifdef BCMSDIO
	} else if (BUSTYPE(bustype) == SDIO_BUS) {
		off = byteoff / 2;
		nw = nbytes / 2;
		for (i = 0; i < nw; i++) {
			if (sprom_read_sdio(osh, (uint16)(off + i), (uint16 *)(buf + i)))
				return 1;
		}
#endif /* BCMSDIO */
	} else if (BUSTYPE(bustype) == SB_BUS) {
#ifdef BCMUSBDEV
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx;
			void *regs;
			int rc;
			bool wasup;

			origidx = sb_coreidx(sbh);
			regs = sb_setcore(sbh, SB_PCMCIA, 0);
			ASSERT(regs != NULL);

			if (!(wasup = sb_iscoreup(sbh)))
				sb_core_reset(sbh, 0, 0);

			rc = get_sb_pcmcia_srom(sbh, osh, regs, byteoff, buf, nbytes, TRUE);

			if (!wasup)
				sb_core_disable(sbh, 0);

			sb_setcoreidx(sbh, origidx);
			return rc;
		}
#endif /* def BCMUSBDEV */

		return 1;
	} else {
		return 1;
	}

	return 0;
}

/* support only 16-bit word write into srom */
int
srom_write(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
           uint byteoff, uint nbytes, uint16 *buf)
{
	uint16 *srom;
	uint i, nw, crc_range;
	uint16 old[SPROM_SIZE], new[SPROM_SIZE];
	uint8 crc;
	volatile uint32 val32;

	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only */
	if ((byteoff & 1) || (nbytes & 1))
		return 1;

	if ((byteoff + nbytes) > (SPROM_SIZE * 2))
		return 1;

	if (BUSTYPE(bustype) == PCMCIA_BUS) {
		crc_range = SPROM_SIZE * 2;
	}
#ifdef BCMSDIO
	else if (BUSTYPE(bustype) == SDIO_BUS) {
		crc_range = SPROM_SIZE * 2;
	}
#endif /* def BCMSDIO */
	else {
		crc_range = SPROM_CRC_RANGE * 2 * 2;	/* must big enough for SROM8 */
	}

	nw = crc_range / 2;
	/* read first small number words from srom, then adjust the length, read all */
	if (srom_read(sbh, bustype, curmap, osh, 0, crc_range, old))
		return 1;

	BS_ERROR(("srom_write: old[SROM4_SIGN] 0x%x, old[SROM8_SIGN] 0x%x\n",
	          old[SROM4_SIGN], old[SROM8_SIGN]));
	if ((old[SROM4_SIGN] == SROM4_SIGNATURE) ||
	    (old[SROM8_SIGN] == SROM4_SIGNATURE)) {
		nw = SROM4_WORDS;
		crc_range = nw * 2;
		if (srom_read(sbh, bustype, curmap, osh, 0, crc_range, old))
			return 1;
	}

	if (byteoff == 0x55aa) {
		/* Erase request */
		crc_range = 0;
		memset((void *)new, 0xff, nw * 2);
	} else {
		/* Copy old contents */
		bcopy((void *)old, (void *)new, nw * 2);
		/* make changes */
		bcopy((void *)buf, (void *)&new[byteoff / 2], nbytes);
	}

	if (crc_range) {
		/* calculate crc */
		htol16_buf(new, crc_range);
		crc = ~hndcrc8((uint8 *)new, crc_range - 1, CRC8_INIT_VALUE);
		ltoh16_buf(new, crc_range);
		new[nw - 1] = (crc << 8) | (new[nw - 1] & 0xff);
	}

	if (BUSTYPE(bustype) == PCI_BUS) {
#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
		if (wl_srom_present == 0) {
			printk("wl: write to main memory mapped srom not supported\n");			
		} else {
		
#endif		
		srom = (uint16 *)((uchar*)curmap + PCI_BAR0_SPROM_OFFSET);
		/* enable writes to the SPROM */
		val32 = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		val32 |= SPROM_WRITEEN;
		OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32);
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				W_REG(osh, &srom[i], new[i]);
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}
		/* disable writes to the SPROM */
		OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32 &
		                     ~SPROM_WRITEEN);
#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
		}
#endif		
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		/* enable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WEN))
			return 1;
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				sprom_write_pcmcia(osh, (uint16)(i), new[i]);
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}
		/* disable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WDS))
			return 1;
#ifdef BCMSDIO
	} else if (BUSTYPE(bustype) == SDIO_BUS) {
		/* enable writes to the SPROM */
		if (sprom_cmd_sdio(osh, SBSDIO_SPROM_WEN))
			return 1;
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				sprom_write_sdio(osh, (uint16)(i), new[i]);
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}
		/* disable writes to the SPROM */
		if (sprom_cmd_sdio(osh, SBSDIO_SPROM_WDS))
			return 1;
#endif /* BCMSDIO */
	} else if (BUSTYPE(bustype) == SB_BUS) {
#ifdef BCMUSBDEV
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx;
			void *regs;
			int rc;
			bool wasup;

			origidx = sb_coreidx(sbh);
			regs = sb_setcore(sbh, SB_PCMCIA, 0);
			ASSERT(regs != NULL);

			if (!(wasup = sb_iscoreup(sbh)))
				sb_core_reset(sbh, 0, 0);

			rc = set_sb_pcmcia_srom(sbh, osh, regs, byteoff, buf, nbytes);

			if (!wasup)
				sb_core_disable(sbh, 0);

			sb_setcoreidx(sbh, origidx);
			return rc;
		}
#endif /* def BCMUSBDEV */
		return 1;
	} else {
		return 1;
	}

	bcm_mdelay(WRITE_ENABLE_DELAY);
	return 0;
}

#ifdef BCMUSBDEV
#define SB_PCMCIA_READ(osh, regs, fcr) \
		R_REG(osh, (volatile uint8 *)(regs) + 0x600 + (fcr) - 0x700 / 2)
#define SB_PCMCIA_WRITE(osh, regs, fcr, v) \
		W_REG(osh, (volatile uint8 *)(regs) + 0x600 + (fcr) - 0x700 / 2, v)

/* set PCMCIA srom command register */
static int
srom_cmd_sb_pcmcia(osl_t *osh, uint8 *pcmregs, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 0;

	/* write srom command register */
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_CS, cmd);

	/* wait status */
	while (++wait_cnt < 1000000) {
		status = SB_PCMCIA_READ(osh, pcmregs, SROM_CS);
		if (status & SROM_DONE)
			return 0;
		OSL_DELAY(1);
	}

	BS_ERROR(("sr_cmd: Give up after %d tries, stat = 0x%x\n", wait_cnt, status));
	return 1;
}

/* read a word from the PCMCIA srom over SB */
static int
srom_read_sb_pcmcia(osl_t *osh, uint8 *pcmregs, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h,  data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRH, addr_h);
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRL, addr_l);

	/* do read */
	if (srom_cmd_sb_pcmcia(osh, pcmregs, SROM_READ))
		return 1;

	/* read data */
	data_h = SB_PCMCIA_READ(osh, pcmregs, SROM_DATAH);
	data_l = SB_PCMCIA_READ(osh, pcmregs, SROM_DATAL);
	*data = ((uint16)data_h << 8) | data_l;

	return 0;
}

/* write a word to the PCMCIA srom over SB */
static int
srom_write_sb_pcmcia(osl_t *osh, uint8 *pcmregs, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;
	int rc;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRH, addr_h);
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRL, addr_l);

	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* write data */
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_DATAH, data_h);
	SB_PCMCIA_WRITE(osh, pcmregs, SROM_DATAL, data_l);

	/* do write */
	rc = srom_cmd_sb_pcmcia(osh, pcmregs, SROM_WRITE);
	OSL_DELAY(20000);
	return rc;
}

/*
 * Read the srom for the pcmcia-srom over sb case.
 * Return 0 on success, nonzero on error.
 */
static int
get_sb_pcmcia_srom(sb_t *sbh, osl_t *osh, uint8 *pcmregs,
                   uint boff, uint16 *srom, uint bsz, bool check_crc)
{
	uint i, nw, woff, wsz;
	int err = 0;

	/* read must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);

	/* read sprom size and validate the parms */
	if ((nw = srom_size(sbh, osh)) == 0) {
		BS_ERROR(("get_sb_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("get_sb_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}

	/* read in sprom contents */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_read_sb_pcmcia(osh, pcmregs, (uint16)woff, &srom[i])) {
			BS_ERROR(("get_sb_pcmcia_srom: sprom read failed\n"));
			err = -3;
			goto out;
		}
	}

#ifdef USB4322
	check_crc = FALSE;
#endif
	if (check_crc) {
		if (srom[0] == 0xffff) {
			/* The hardware thinks that an srom that starts with 0xffff
			 * is blank, regardless of the rest of the content, so declare
			 * it bad.
			 */
			BS_ERROR(("%s: srom[0] == 0xffff, assuming unprogrammed srom\n",
			          __FUNCTION__));
			err = -4;
			goto out;
		}

		/* fixup the endianness so crc8 will pass */
		htol16_buf(srom, nwords * 2);
		if (hndcrc8((uint8 *)srom, nw * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
			BS_ERROR(("%s: bad crc\n", __FUNCTION__));
			err = -5;
		}
		/* now correct the endianness of the byte array */
		ltoh16_buf(srom, nw * 2);
	}

out:
	return err;
}

/*
 * Write the srom for the pcmcia-srom over sb case.
 * Return 0 on success, nonzero on error.
 */
static int
set_sb_pcmcia_srom(sb_t *sbh, osl_t *osh, uint8 *pcmregs,
                   uint boff, uint16 *srom, uint bsz)
{
	uint i, nw, woff, wsz;
	uint16 word;
	uint8 crc;
	int err = 0;

	/* write must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);

	/* read sprom size and validate the parms */
	if ((nw = srom_size(sbh, osh)) == 0) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}

	/* enable write */
	if (srom_cmd_sb_pcmcia(osh, pcmregs, SROM_WEN)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom wen failed\n"));
		err = -3;
		goto out;
	}

	/* write buffer to sprom */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_write_sb_pcmcia(osh, pcmregs, (uint16)woff, srom[i])) {
			BS_ERROR(("set_sb_pcmcia_srom: sprom write failed\n"));
			err = -4;
			goto out;
		}
	}

	/* fix crc */
	crc = CRC8_INIT_VALUE;
	for (woff = 0; woff < nw; woff ++) {
		if (srom_read_sb_pcmcia(osh, pcmregs, (uint16)woff, &word)) {
			BS_ERROR(("set_sb_pcmcia_srom: sprom fix crc read failed\n"));
			err = -5;
			goto out;
		}
		word = htol16(word);
		crc = hndcrc8((uint8 *)&word, woff != nw - 1 ? 2 : 1, crc);
	}
	word = (~crc << 8) + (ltoh16(word) & 0xff);
	if (srom_write_sb_pcmcia(osh, pcmregs, (uint16)(woff - 1), word)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom fix crc write failed\n"));
		err = -6;
		goto out;
	}

	/* disable write */
	if (srom_cmd_sb_pcmcia(osh, pcmregs, SROM_WDS)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom wds failed\n"));
		err = -7;
		goto out;
	}

out:
	return err;
}
#endif /* def BCMUSBDEV */

static char BCMINITDATA(vstr_manf)[] = "manf=%s";
static char BCMINITDATA(vstr_productname)[] = "productname=%s";
static char BCMINITDATA(vstr_manfid)[] = "manfid=0x%x";
static char BCMINITDATA(vstr_prodid)[] = "prodid=0x%x";
#ifdef BCMSDIO
static char BCMINITDATA(vstr_sdmaxspeed)[] = "sdmaxspeed=%d";
static char BCMINITDATA(vstr_sdmaxblk)[][13] = { "sdmaxblk0=%d", "sdmaxblk1=%d", "sdmaxblk2=%d" };
#endif
static char BCMINITDATA(vstr_regwindowsz)[] = "regwindowsz=%d";
static char BCMINITDATA(vstr_sromrev)[] = "sromrev=%d";
static char BCMINITDATA(vstr_chiprev)[] = "chiprev=%d";
static char BCMINITDATA(vstr_subvendid)[] = "subvendid=0x%x";
static char BCMINITDATA(vstr_subdevid)[] = "subdevid=0x%x";
static char BCMINITDATA(vstr_boardrev)[] = "boardrev=0x%x";
static char BCMINITDATA(vstr_aa2g)[] = "aa2g=%d";
static char BCMINITDATA(vstr_aa5g)[] = "aa5g=%d";
static char BCMINITDATA(vstr_ag1)[] = "ag1=%d";
static char BCMINITDATA(vstr_cc)[] = "cc=%d";
static char BCMINITDATA(vstr_opo)[] = "opo=%d";
static char BCMINITDATA(vstr_pa0b)[][9] = { "pa0b0=%d", "pa0b1=%d", "pa0b2=%d" };
static char BCMINITDATA(vstr_pa0itssit)[] = "pa0itssit=%d";
static char BCMINITDATA(vstr_pa0maxpwr)[] = "pa0maxpwr=%d";
static char BCMINITDATA(vstr_pa1b)[][9] = { "pa1b0=%d", "pa1b1=%d", "pa1b2=%d" };
static char BCMINITDATA(vstr_pa1lob)[][11] = { "pa1lob0=%d", "pa1lob1=%d", "pa1lob2=%d" };
static char BCMINITDATA(vstr_pa1hib)[][11] = { "pa1hib0=%d", "pa1hib1=%d", "pa1hib2=%d" };
static char BCMINITDATA(vstr_pa1itssit)[] = "pa1itssit=%d";
static char BCMINITDATA(vstr_pa1maxpwr)[] = "pa1maxpwr=%d";
static char BCMINITDATA(vstr_pa1lomaxpwr)[] = "pa1lomaxpwr=%d";
static char BCMINITDATA(vstr_pa1himaxpwr)[] = "pa1himaxpwr=%d";
static char BCMINITDATA(vstr_oem)[] = "oem=%02x%02x%02x%02x%02x%02x%02x%02x";
static char BCMINITDATA(vstr_boardflags)[] = "boardflags=0x%x";
static char BCMINITDATA(vstr_ledbh0)[] = "ledbh0=%d";
static char BCMINITDATA(vstr_ledbh1)[] = "ledbh1=%d";
static char BCMINITDATA(vstr_ledbh2)[] = "ledbh2=%d";
static char BCMINITDATA(vstr_ledbh3)[] = "ledbh3=%d";
static char BCMINITDATA(vstr_noccode)[] = "ccode=";
static char BCMINITDATA(vstr_ccode)[] = "ccode=%c%c";
static char BCMINITDATA(vstr_cctl)[] = "cctl=0x%x";
static char BCMINITDATA(vstr_cckpo)[] = "cckpo=0x%x";
static char BCMINITDATA(vstr_ofdmpo)[] = "ofdmpo=0x%x";
static char BCMINITDATA(vstr_rdlid)[] = "rdlid=0x%x";
static char BCMINITDATA(vstr_rdlrndis)[] = "rdlrndis=%d";
static char BCMINITDATA(vstr_rdlrwu)[] = "rdlrwu=%d";
static char BCMINITDATA(vstr_rdlsn)[] = "rdlsn=%d";
static char BCMINITDATA(vstr_rssismf2g)[] = "rssismf2g=%d";
static char BCMINITDATA(vstr_rssismc2g)[] = "rssismc2g=%d";
static char BCMINITDATA(vstr_rssisav2g)[] = "rssisav2g=%d";
static char BCMINITDATA(vstr_bxa2g)[] = "bxa2g=%d";
static char BCMINITDATA(vstr_rssismf5g)[] = "rssismf5g=%d";
static char BCMINITDATA(vstr_rssismc5g)[] = "rssismc5g=%d";
static char BCMINITDATA(vstr_rssisav5g)[] = "rssisav5g=%d";
static char BCMINITDATA(vstr_bxa5g)[] = "bxa5g=%d";
static char BCMINITDATA(vstr_tri2g)[] = "tri2g=%d";
static char BCMINITDATA(vstr_tri5gl)[] = "tri5gl=%d";
static char BCMINITDATA(vstr_tri5g)[] = "tri5g=%d";
static char BCMINITDATA(vstr_tri5gh)[] = "tri5gh=%d";
static char BCMINITDATA(vstr_rxpo2g)[] = "rxpo2g=%d";
static char BCMINITDATA(vstr_rxpo5g)[] = "rxpo5g=%d";
static char BCMINITDATA(vstr_boardtype)[] = "boardtype=0x%x";
static char BCMINITDATA(vstr_leddc)[] = "leddc=0x%x%04x";
static char BCMINITDATA(vstr_vendid)[] = "vendid=0x%x";
static char BCMINITDATA(vstr_devid)[] = "devid=0x%x";
static char BCMINITDATA(vstr_xtalfreq)[] = "xtalfreq=%d";
static char BCMINITDATA(vstr_ag0)[] = "ag0=%d";

#define FROMHOST()
static char BCMINITDATA(vstr_boardnum)[] = "boardnum=%d";
static char BCMINITDATA(vstr_macaddr)[] = "macaddr=%s";

int
BCMINITFN(srom_parsecis)(osl_t *osh, uint8 *pcis[], uint ciscnt, char **vars, uint *count)
{
	char eabuf[32];
	char *base;
	varbuf_t b;
	uint8 *cis, tup, tlen, sromrev = 1;
	int i, j;
	bool ag_init = FALSE;
	uint32 w32;
	uint funcid;
	uint cisnum;
	int32 boardnum;
	int err;

	ASSERT(vars);
	ASSERT(count);

	boardnum = -1;

	base = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(base);
	if (!base)
		return -2;

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);

	eabuf[0] = '\0';
	for (cisnum = 0; cisnum < ciscnt; cisnum++) {
		cis = *pcis++;
		i = 0;
		funcid = 0;
		do {
			tup = cis[i++];
			tlen = cis[i++];
			if ((i + tlen) >= CIS_SIZE)
				break;

			switch (tup) {
			case CISTPL_VERS_1: FROMHOST();
				/* assume the strings are good if the version field checks out */
				if (((cis[i + 1] << 8) + cis[i]) >= 0x0008) {
					varbuf_append(&b, vstr_manf, &cis[i + 2]);
					varbuf_append(&b, vstr_productname,
					              &cis[i + 3 + strlen((char *)&cis[i + 2])]);
					break;
				}

			case CISTPL_MANFID: FROMHOST();
				varbuf_append(&b, vstr_manfid, (cis[i + 1] << 8) + cis[i]);
				varbuf_append(&b, vstr_prodid, (cis[i + 3] << 8) + cis[i + 2]);
				break;

			case CISTPL_FUNCID: FROMHOST();
				funcid = cis[i];
				break;

			case CISTPL_FUNCE: FROMHOST();
				switch (funcid) {
#ifdef BCMSDIO
				case CISTPL_FID_SDIO: FROMHOST();
					if (cis[i] == 0) {
						uint8 spd = cis[i + 3];
						static int base[] = {
							-1, 10, 12, 13, 15, 20, 25, 30,
							35, 40, 45, 50, 55, 60, 70, 80
						};
						static int mult[] = {
							10, 100, 1000, 10000,
							-1, -1, -1, -1
						};
						ASSERT((mult[spd & 0x7] != -1) &&
						       (base[(spd >> 3) & 0x0f]));
						varbuf_append(&b, vstr_sdmaxblk[0],
						              (cis[i + 2] << 8) + cis[i + 1]);
						varbuf_append(&b, vstr_sdmaxspeed,
						              (mult[spd & 0x7] *
						               base[(spd >> 3) & 0x0f]));
					} else if (cis[i] == 1) {
						varbuf_append(&b, vstr_sdmaxblk[cisnum],
						              (cis[i + 13] << 8) | cis[i + 12]);
					}
					funcid = 0;
					break;
#endif /* BCMSDIO */
				default: FROMHOST();
					/* set macaddr if HNBU_MACADDR not seen yet */
					if (eabuf[0] == '\0' && cis[i] == LAN_NID) {
						ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 2],
						               eabuf);
					}
					/* set boardnum if HNBU_BOARDNUM not seen yet */
					if (boardnum == -1)
						boardnum = (cis[i + 6] << 8) + cis[i + 7];
					break;
				}
				break;

			case CISTPL_CFTABLE: FROMHOST();
				varbuf_append(&b, vstr_regwindowsz, (cis[i + 7] << 8) | cis[i + 6]);
				break;

			case CISTPL_BRCM_HNBU:
				switch (cis[i]) {
				case HNBU_SROMREV:
					sromrev = cis[i + 1];
					varbuf_append(&b, vstr_sromrev, sromrev);
					break;

				case HNBU_CHIPID: FROMHOST();
					varbuf_append(&b, vstr_vendid, (cis[i + 2] << 8) +
					              cis[i + 1]);
					varbuf_append(&b, vstr_devid, (cis[i + 4] << 8) +
					              cis[i + 3]);
					if (tlen >= 7) {
						varbuf_append(&b, vstr_chiprev,
						              (cis[i + 6] << 8) + cis[i + 5]);
					}
					if (tlen >= 9) {
						varbuf_append(&b, vstr_subvendid,
						              (cis[i + 8] << 8) + cis[i + 7]);
					}
					if (tlen >= 11) {
						varbuf_append(&b, vstr_subdevid,
						              (cis[i + 10] << 8) + cis[i + 9]);
						/* subdevid doubles for boardtype */
						varbuf_append(&b, vstr_boardtype,
						              (cis[i + 10] << 8) + cis[i + 9]);
					}
					break;

				case HNBU_BOARDREV: FROMHOST();
					varbuf_append(&b, vstr_boardrev, cis[i + 1]);
					break;

				case HNBU_AA: FROMHOST();
					varbuf_append(&b, vstr_aa2g, cis[i + 1]);
					break;

				case HNBU_AG:
					varbuf_append(&b, vstr_ag0, cis[i + 1]);
					ag_init = TRUE;
					break;

				case HNBU_ANT5G:
					varbuf_append(&b, vstr_aa5g, cis[i + 1]);
					varbuf_append(&b, vstr_ag1, cis[i + 2]);
					break;

				case HNBU_CC:
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_cc, cis[i + 1]);
					break;

				case HNBU_PAPARMS:
					if (tlen == 2) {
						ASSERT(sromrev == 1);
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 1]);
					} else if (tlen >= 9) {
						if (tlen == 10) {
							ASSERT(sromrev >= 2);
							varbuf_append(&b, vstr_opo, cis[i + 9]);
						} else
							ASSERT(tlen == 9);

						for (j = 0; j < 3; j++) {
							varbuf_append(&b, vstr_pa0b[j],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}
						varbuf_append(&b, vstr_pa0itssit, cis[i + 7]);
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 8]);
					} else
						ASSERT(tlen >= 9);
					break;

				case HNBU_PAPARMS5G:
					ASSERT((sromrev == 2) || (sromrev == 3));
					for (j = 0; j < 3; j++) {
						varbuf_append(&b, vstr_pa1b[j],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					for (j = 3; j < 6; j++) {
						varbuf_append(&b, vstr_pa1lob[j - 3],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					for (j = 6; j < 9; j++) {
						varbuf_append(&b, vstr_pa1hib[j - 6],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					varbuf_append(&b, vstr_pa1itssit, cis[i + 19]);
					varbuf_append(&b, vstr_pa1maxpwr, cis[i + 20]);
					varbuf_append(&b, vstr_pa1lomaxpwr, cis[i + 21]);
					varbuf_append(&b, vstr_pa1himaxpwr, cis[i + 22]);
					break;

				case HNBU_OEM: FROMHOST();
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_oem,
					              cis[i + 1], cis[i + 2],
					              cis[i + 3], cis[i + 4],
					              cis[i + 5], cis[i + 6],
					              cis[i + 7], cis[i + 8]);
					break;

				case HNBU_BOARDFLAGS: FROMHOST();
					w32 = (cis[i + 2] << 8) + cis[i + 1];
					if (tlen == 5)
						w32 |= (cis[i + 4] << 24) + (cis[i + 3] << 16);
					varbuf_append(&b, vstr_boardflags, w32);
					break;

				case HNBU_LEDS: FROMHOST();
					if (cis[i + 1] != 0xff) {
						varbuf_append(&b, vstr_ledbh0, cis[i + 1]);
					}
					if (cis[i + 2] != 0xff) {
						varbuf_append(&b, vstr_ledbh1, cis[i + 2]);
					}
					if (cis[i + 3] != 0xff) {
						varbuf_append(&b, vstr_ledbh2, cis[i + 3]);
					}
					if (cis[i + 4] != 0xff) {
						varbuf_append(&b, vstr_ledbh3, cis[i + 4]);
					}
					break;

				case HNBU_CCODE:
					ASSERT(sromrev > 1);
					if ((cis[i + 1] == 0) || (cis[i + 2] == 0))
						varbuf_append(&b, vstr_noccode);
					else
						varbuf_append(&b, vstr_ccode,
						              cis[i + 1], cis[i + 2]);
					varbuf_append(&b, vstr_cctl, cis[i + 3]);
					break;

				case HNBU_CCKPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_cckpo,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_OFDMPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_ofdmpo,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_RDLID: FROMHOST();
					varbuf_append(&b, vstr_rdlid,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_RDLRNDIS: FROMHOST();
					varbuf_append(&b, vstr_rdlrndis, cis[i + 1]);
					break;

				case HNBU_RDLRWU: FROMHOST();
					varbuf_append(&b, vstr_rdlrwu, cis[i + 1]);
					break;

				case HNBU_RDLSN: FROMHOST();
					varbuf_append(&b, vstr_rdlsn,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_XTALFREQ: FROMHOST();
					varbuf_append(&b, vstr_xtalfreq,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_RSSISMBXA2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf2g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc2g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav2g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa2g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_RSSISMBXA5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf5g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc5g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav5g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa5g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_TRI2G: FROMHOST();
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri2g, cis[i + 1]);
					break;

				case HNBU_TRI5G: FROMHOST();
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri5gl, cis[i + 1]);
					varbuf_append(&b, vstr_tri5g, cis[i + 2]);
					varbuf_append(&b, vstr_tri5gh, cis[i + 3]);
					break;

				case HNBU_RXPO2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo2g, cis[i + 1]);
					break;

				case HNBU_RXPO5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo5g, cis[i + 1]);
					break;

				case HNBU_BOARDNUM: FROMHOST();
					boardnum = (cis[i + 2] << 8) + cis[i + 1];
					break;

				case HNBU_MACADDR: FROMHOST();
					bcm_ether_ntoa((struct ether_addr *)&cis[i + 1],
					               eabuf);
					break;

				case HNBU_BOARDTYPE: FROMHOST();
					varbuf_append(&b, vstr_boardtype,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_LEDDC: FROMHOST();
					varbuf_append(&b, vstr_leddc,
					              cis[i + 2], cis[i + 1]);
					break;

#if defined(BCMSDIO) || defined(BCMCCISSR3)
				case HNBU_SROM3SWRGN: FROMHOST();
					if (tlen >= 73) {
						uint16 srom[35];
						uint8 srev = cis[i + 1 + 70];
						ASSERT(srev == 3);
						/* make tuple value 16-bit aligned and parse it */
						bcopy(&cis[i + 1], srom, sizeof(srom));
						_initvars_srom_pci(srev, srom, SROM3_SWRGN_OFF, &b);
						/* 2.4G antenna gain is included in SROM */
						ag_init = TRUE;
						/* Ethernet MAC address is included in SROM */
						eabuf[0] = 0;
						boardnum = -1;
					}
					/* create extra variables */
					if (tlen >= 75)
						varbuf_append(&b, vstr_vendid,
						              (cis[i + 1 + 73] << 8) +
						              cis[i + 1 + 72]);
					if (tlen >= 77)
						varbuf_append(&b, vstr_devid,
						              (cis[i + 1 + 75] << 8) +
						              cis[i + 1 + 74]);
					if (tlen >= 79)
						varbuf_append(&b, vstr_xtalfreq,
						              (cis[i + 1 + 77] << 8) +
						              cis[i + 1 + 76]);
					break;
#endif	/* BCMSDIO || BCMCCISSR3 */
				}
				break;
			}
			i += tlen;
		} while (tup != CISTPL_END);
	}

	if (boardnum != -1) {
		varbuf_append(&b, vstr_boardnum, boardnum);
	}

	if (eabuf[0]) {
		varbuf_append(&b, vstr_macaddr, eabuf);
	}

	/* if there is no antenna gain field, set default */
	if (ag_init == FALSE) {
		varbuf_append(&b, vstr_ag0, 0xff);
	}

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
	*b.buf++ = '\0';

	ASSERT(b.buf - base <= MAXSZ_NVRAM_VARS);

	err = initvars_table(osh, base, b.buf, vars, count);

	MFREE(osh, base, MAXSZ_NVRAM_VARS);
	return err;
}


/* set PCMCIA sprom command register */
static int
sprom_cmd_pcmcia(osl_t *osh, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 1000;

	/* write sprom command register */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_CS, &cmd, 1);

	/* wait status */
	while (wait_cnt--) {
		OSL_PCMCIA_READ_ATTR(osh, SROM_CS, &status, 1);
		if (status & SROM_DONE)
			return 0;
	}

	return 1;
}

/* read a word from the PCMCIA srom */
static int
sprom_read_pcmcia(osl_t *osh, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRL, &addr_l, 1);

	/* do read */
	if (sprom_cmd_pcmcia(osh, SROM_READ))
		return 1;

	/* read data */
	data_h = data_l = 0;
	OSL_PCMCIA_READ_ATTR(osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_READ_ATTR(osh, SROM_DATAL, &data_l, 1);

	*data = (data_h << 8) | data_l;
	return 0;
}

/* write a word to the PCMCIA srom */
static int
sprom_write_pcmcia(osl_t *osh, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);
	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRL, &addr_l, 1);

	/* write data */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_DATAL, &data_l, 1);

	/* do write */
	return sprom_cmd_pcmcia(osh, SROM_WRITE);
}

/*
 * Read in and validate sprom.
 * Return 0 on success, nonzero on error.
 */
static int
sprom_read_pci(osl_t *osh, sb_t *sbh, uint16 *sprom, uint wordoff, uint16 *buf, uint nwords,
	bool check_crc)
{
	int err = 0;
	uint i;

#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
	if (wl_srom_present == 0) {
		int bytes_read;
		/* no srom, use memory map */
		read_sromfile(sb_chip(sbh), buf, wordoff*2, nwords*2);

		if(check_crc) {
			/* hack to work with rev4 srom on wombo*/
			bytes_read = read_sromfile(sb_chip(sbh), buf, 0, SROM4_WORDS*2);
			if ((bytes_read == SROM4_WORDS*2) && (SROM4_WORDS > nwords)) {
				/* not reading enough */
				err = 1;
			}						
		}
		return err;		
	}	
#endif

	/* read the sprom */
	for (i = 0; i < nwords; i++) {
		if (ISSIM_ENAB(sbh)) {
			buf[i] = R_REG(osh, &sprom[wordoff + i]);
		}
		buf[i] = R_REG(osh, &sprom[wordoff + i]);
	}

	if (check_crc) {
		if (buf[0] == 0xffff) {
			/* The hardware thinks that an srom that starts with 0xffff
			 * is blank, regardless of the rest of the content, so declare
			 * it bad.
			 */
			BS_ERROR(("%s: buf[0] = 0x%x, returning bad-crc\n", __FUNCTION__, buf[0]));
			return 1;
		}

		/* fixup the endianness so crc8 will pass */
		htol16_buf(buf, nwords * 2);
		if (hndcrc8((uint8 *)buf, nwords * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE)
			err = 1;
		/* now correct the endianness of the byte array */
		ltoh16_buf(buf, nwords * 2);
	}

	return err;
}

/*
* Create variable table from memory.
* Return 0 on success, nonzero on error.
*/
static int
BCMINITFN(initvars_table)(osl_t *osh, char *start, char *end, char **vars, uint *count)
{
	int c = (int)(end - start);

	/* do it only when there is more than just the null string */
	if (c > 1) {
		char *vp = MALLOC(osh, c);
		ASSERT(vp);
		if (!vp)
			return BCME_NOMEM;
		bcopy(start, vp, c);
		*vars = vp;
		*count = c;
	}
	else {
		*vars = NULL;
		*count = 0;
	}

	return 0;
}

/*
 * Find variables with <devpath> from flash. 'base' points to the beginning
 * of the table upon enter and to the end of the table upon exit when success.
 * Return 0 on success, nonzero on error.
 */
static int
initvars_flash(sb_t *sbh, osl_t *osh, char **base, uint len)
{
	char *vp = *base;
	char *flash;
	int err;
	char *s;
	uint l, dl, copy_len;
	char devpath[SB_DEVPATH_BUFSZ];

	/* allocate memory and read in flash */
	if (!(flash = MALLOC(osh, NVRAM_SPACE)))
		return BCME_NOMEM;
	if ((err = nvram_getall(flash, NVRAM_SPACE)))
		goto exit;

	sb_devpath(sbh, devpath, sizeof(devpath));

	/* grab vars with the <devpath> prefix in name */
	dl = strlen(devpath);
	for (s = flash; s && *s; s += l + 1) {
		l = strlen(s);

		/* skip non-matching variable */
		if (strncmp(s, devpath, dl))
			continue;

		/* is there enough room to copy? */
		copy_len = l - dl + 1;
		if (len < copy_len) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}

		/* no prefix, just the name=value */
		strncpy(vp, &s[dl], copy_len);
		vp += copy_len;
		len -= copy_len;
	}

	/* add null string as terminator */
	if (len < 1) {
		err = BCME_BUFTOOSHORT;
		goto exit;
	}
	*vp++ = '\0';

	*base = vp;

exit:	MFREE(osh, flash, NVRAM_SPACE);
	return err;
}

#if !defined(BCMUSBDEV)
/*
 * Initialize nonvolatile variable table from flash.
 * Return 0 on success, nonzero on error.
 */
static int
initvars_flash_sb(sb_t *sbh, char **vars, uint *count)
{
	osl_t *osh = sb_osh(sbh);
	char *vp, *base;
	int err;

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp);
	if (!vp)
		return BCME_NOMEM;

	if ((err = initvars_flash(sbh, osh, &vp, MAXSZ_NVRAM_VARS)) == 0)
		err = initvars_table(osh, base, vp, vars, count);

	MFREE(osh, base, MAXSZ_NVRAM_VARS);

	return err;
}
#endif	


/*
 * Initialize nonvolatile variable table from sprom.
 * Return 0 on success, nonzero on error.
 */

typedef struct {
	const char *name;
	uint32	revmask;
	uint32	flags;
	uint16	off;
	uint16	mask;
} sromvar_t;

#define SRFL_MORE	1		/* value continues as described by the next entry */
#define	SRFL_NOFFS	2		/* value bits can't be all one's */
#define	SRFL_PRHEX	4		/* value is in hexdecimal format */
#define	SRFL_PRSIGN	8		/* value is in signed decimal format */
#define	SRFL_CCODE	0x10		/* value is in country code format */
#define	SRFL_ETHADDR	0x20		/* value is an Ethernet address */
#define SRFL_LEDDC	0x40		/* value is an LED duty cycle */

/* Assumptions:
 * - Ethernet address spins across 3 consective words
 *
 * Table rules:
 * - Add multiple entries next to each other if a value spins across multiple words
 *   (even multiple fields in the same word) with each entry except the last having
 *   it's SRFL_MORE bit set.
 * - Ethernet address entry does not follow above rule and must not have SRFL_MORE
 *   bit set. Its SRFL_ETHADDR bit implies it takes multiple words.
 * - The last entry's name field must be NULL to indicate the end of the table. Other
 *   entries must have non-NULL name.
 */

static const sromvar_t pci_sromvars[] = {
	{"boardrev",	0x0000000e,	SRFL_PRHEX,	SROM_AABREV, SROM_BR_MASK},
	{"boardrev",	0x000000f0,	SRFL_PRHEX,	SROM4_BREV, 0xffff},
	{"boardrev",	0xffffff00,	SRFL_PRHEX,	SROM8_BREV, 0xffff},
	{"boardflags",	0x00000002,	SRFL_PRHEX,	SROM_BFL, 0xffff},
	{"boardflags",	0x00000004,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL, 0xffff},
	{"",		0,		0,			SROM_BFL2, 0xffff},
	{"boardflags",	0x00000008,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL, 0xffff},
	{"",		0,		0,			SROM3_BFL2, 0xffff},
	{"boardflags",	0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL0, 0xffff},
	{"",		0,		0,			SROM4_BFL1, 0xffff},
	{"boardflags",	0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL0, 0xffff},
	{"",		0,		0,			SROM5_BFL1, 0xffff},
	{"boardflags",	0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL0, 0xffff},
	{"",		0,		0,			SROM8_BFL1, 0xffff},
	{"boardflags2", 0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL2, 0xffff},
	{"",		0,		0,			SROM4_BFL3, 0xffff},
	{"boardflags2", 0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL2, 0xffff},
	{"",		0,		0,			SROM5_BFL3, 0xffff},
	{"boardflags2", 0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL2, 0xffff},
	{"",		0,		0,			SROM8_BFL3, 0xffff},
	{"boardtype",	0xfffffffc,	SRFL_PRHEX,	SROM_SSID, 0xffff},
	{"boardnum",	0x00000006,	0,		SROM_MACLO_IL0, 0xffff},
	{"boardnum",	0x00000008,	0,		SROM3_MACLO, 0xffff},
	{"boardnum",	0x00000010,	0,		SROM4_MACLO, 0xffff},
	{"boardnum",	0x000000e0,	0,		SROM5_MACLO, 0xffff},
	{"boardnum",	0xffffff00,	0,		SROM8_MACLO, 0xffff},
	{"cc",		0x00000002,	0,		SROM_AABREV, SROM_CC_MASK},
	{"regrev",	0x00000008,	0,		SROM_OPO, 0xff00},
	{"regrev",	0x00000010,	0,		SROM4_REGREV, 0xff},
	{"regrev",	0x000000e0,	0,		SROM5_REGREV, 0xff},
	{"regrev",	0xffffff00,	0,		SROM8_REGREV, 0xff},
	{"ledbh0",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10, 0xff},
	{"ledbh1",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10, 0xff00},
	{"ledbh2",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32, 0xff},
	{"ledbh3",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32, 0xff00},
	{"ledbh0",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10, 0xff},
	{"ledbh1",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10, 0xff00},
	{"ledbh2",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32, 0xff},
	{"ledbh3",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32, 0xff00},
	{"ledbh0",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10, 0xff},
	{"ledbh1",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10, 0xff00},
	{"ledbh2",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32, 0xff},
	{"ledbh3",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32, 0xff00},
	{"ledbh0",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH10, 0xff},
	{"ledbh1",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH10, 0xff00},
	{"ledbh2",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH32, 0xff},
	{"ledbh3",	0xffffff00,	SRFL_NOFFS,	SROM8_LEDBH32, 0xff00},
	{"pa0b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB0, 0xffff},
	{"pa0b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB1, 0xffff},
	{"pa0b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB2, 0xffff},
	{"pa0itssit",	0x0000000e,	0,		SROM_ITT, 0xff},
	{"pa0maxpwr",	0x0000000e,	0,		SROM_WL10MAXP, 0xff},
	{"pa0b0",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB0, 0xffff},
	{"pa0b1",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB1, 0xffff},
	{"pa0b2",	0xffffff00,	SRFL_PRHEX,	SROM8_W0_PAB2, 0xffff},
	{"pa0itssit",	0xffffff00,	0,		SROM8_W0_ITTMAXP, 0xff00},
	{"pa0maxpwr",	0xffffff00,	0,		SROM8_W0_ITTMAXP, 0xff},
	{"opo",		0x0000000c,	0,		SROM_OPO, 0xff},
	{"opo",		0xffffff00,	0,		SROM8_2G_OFDMPO, 0xff},
	{"aa2g",	0x0000000e,	0,		SROM_AABREV, SROM_AA0_MASK},
	{"aa2g",	0x000000f0,	0,		SROM4_AA, 0xff},
	{"aa2g",	0xffffff00,	0,		SROM8_AA, 0xff},
	{"aa5g",	0x0000000e,	0,		SROM_AABREV, SROM_AA1_MASK},
	{"aa5g",	0x000000f0,	0,		SROM4_AA, 0xff00},
	{"aa5g",	0xffffff00,	0,		SROM8_AA, 0xff00},
	{"ag0",		0x0000000e,	0,		SROM_AG10, 0xff},
	{"ag1",		0x0000000e,	0,		SROM_AG10, 0xff00},
	{"ag0",		0x000000f0,	0,		SROM4_AG10, 0xff},
	{"ag1",		0x000000f0,	0,		SROM4_AG10, 0xff00},
	{"ag2",		0x000000f0,	0,		SROM4_AG32, 0xff},
	{"ag3",		0x000000f0,	0,		SROM4_AG32, 0xff00},
	{"ag0",		0xffffff00,	0,		SROM8_AG10, 0xff},
	{"ag1",		0xffffff00,	0,		SROM8_AG10, 0xff00},
	{"ag2",		0xffffff00,	0,		SROM8_AG32, 0xff},
	{"ag3",		0xffffff00,	0,		SROM8_AG32, 0xff00},
	{"pa1b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB0, 0xffff},
	{"pa1b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB1, 0xffff},
	{"pa1b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB2, 0xffff},
	{"pa1lob0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB0, 0xffff},
	{"pa1lob1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB1, 0xffff},
	{"pa1lob2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB2, 0xffff},
	{"pa1hib0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB0, 0xffff},
	{"pa1hib1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB1, 0xffff},
	{"pa1hib2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB2, 0xffff},
	{"pa1itssit",	0x0000000e,	0,		SROM_ITT, 0xff00},
	{"pa1maxpwr",	0x0000000e,	0,		SROM_WL10MAXP, 0xff00},
	{"pa1lomaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP, 0xff00},
	{"pa1himaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP, 0xff},
	{"pa1b0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0, 0xffff},
	{"pa1b1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1, 0xffff},
	{"pa1b2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2, 0xffff},
	{"pa1lob0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0_LC, 0xffff},
	{"pa1lob1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1_LC, 0xffff},
	{"pa1lob2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2_LC, 0xffff},
	{"pa1hib0",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB0_HC, 0xffff},
	{"pa1hib1",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB1_HC, 0xffff},
	{"pa1hib2",	0xffffff00,	SRFL_PRHEX,	SROM8_W1_PAB2_HC, 0xffff},
	{"pa1itssit",	0xffffff00,	0,		SROM8_W1_ITTMAXP, 0xff00},
	{"pa1maxpwr",	0xffffff00,	0,		SROM8_W1_ITTMAXP, 0xff},
	{"pa1lomaxpwr",	0xffffff00,	0,		SROM8_W1_MAXP_LCHC, 0xff00},
	{"pa1himaxpwr",	0xffffff00,	0,		SROM8_W1_MAXP_LCHC, 0xff},
	{"bxa2g",	0x00000008,	0,		SROM_BXARSSI2G, 0x1800},
	{"rssisav2g",	0x00000008,	0,		SROM_BXARSSI2G, 0x0700},
	{"rssismc2g",	0x00000008,	0,		SROM_BXARSSI2G, 0x00f0},
	{"rssismf2g",	0x00000008,	0,		SROM_BXARSSI2G, 0x000f},
	{"bxa2g",	0xffffff00,	0,		SROM8_BXARSSI2G, 0x1800},
	{"rssisav2g",	0xffffff00,	0,		SROM8_BXARSSI2G, 0x0700},
	{"rssismc2g",	0xffffff00,	0,		SROM8_BXARSSI2G, 0x00f0},
	{"rssismf2g",	0xffffff00,	0,		SROM8_BXARSSI2G, 0x000f},
	{"bxa5g",	0x00000008,	0,		SROM_BXARSSI5G, 0x1800},
	{"rssisav5g",	0x00000008,	0,		SROM_BXARSSI5G, 0x0700},
	{"rssismc5g",	0x00000008,	0,		SROM_BXARSSI5G, 0x00f0},
	{"rssismf5g",	0x00000008,	0,		SROM_BXARSSI5G, 0x000f},
	{"bxa5g",	0xffffff00,	0,		SROM8_BXARSSI5G, 0x1800},
	{"rssisav5g",	0xffffff00,	0,		SROM8_BXARSSI5G, 0x0700},
	{"rssismc5g",	0xffffff00,	0,		SROM8_BXARSSI5G, 0x00f0},
	{"rssismf5g",	0xffffff00,	0,		SROM8_BXARSSI5G, 0x000f},
	{"tri2g",	0x00000008,	0,		SROM_TRI52G, 0xff},
	{"tri5g",	0x00000008,	0,		SROM_TRI52G, 0xff00},
	{"tri5gl",	0x00000008,	0,		SROM_TRI5GHL, 0xff},
	{"tri5gh",	0x00000008,	0,		SROM_TRI5GHL, 0xff00},
	{"tri2g",	0xffffff00,	0,		SROM8_TRI52G, 0xff},
	{"tri5g",	0xffffff00,	0,		SROM8_TRI52G, 0xff00},
	{"tri5gl",	0xffffff00,	0,		SROM8_TRI5GHL, 0xff},
	{"tri5gh",	0xffffff00,	0,		SROM8_TRI5GHL, 0xff00},
	{"rxpo2g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G, 0xff},
	{"rxpo5g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G, 0xff00},
	{"rxpo2g",	0xffffff00,	SRFL_PRSIGN,	SROM8_RXPO52G, 0xff},
	{"rxpo5g",	0xffffff00,	SRFL_PRSIGN,	SROM8_RXPO52G, 0xff00},
	{"txchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC, SROM4_TXCHAIN_MASK},
	{"rxchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC, SROM4_RXCHAIN_MASK},
	{"antswitch",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC, SROM4_SWITCH_MASK},
	{"txchain",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC, SROM4_TXCHAIN_MASK},
	{"rxchain",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC, SROM4_RXCHAIN_MASK},
	{"antswitch",	0xffffff00,	SRFL_NOFFS,	SROM8_TXRXC, SROM4_SWITCH_MASK},
	{"tssipos2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl2g",	0xffffff00,	0,		SROM8_FEM2G,	SROM8_FEM_ANTSWLUT_MASK},
	{"tssipos5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl5g",	0xffffff00,	0,		SROM8_FEM5G,	SROM8_FEM_ANTSWLUT_MASK},
	{"txpid2ga0",	0x000000f0,	0,		SROM4_TXPID2G, 0xff},
	{"txpid2ga1",	0x000000f0,	0,		SROM4_TXPID2G, 0xff00},
	{"txpid2ga2",	0x000000f0,	0,		SROM4_TXPID2G + 1, 0xff},
	{"txpid2ga3",	0x000000f0,	0,		SROM4_TXPID2G + 1, 0xff00},
	{"txpid5ga0",	0x000000f0,	0,		SROM4_TXPID5G, 0xff},
	{"txpid5ga1",	0x000000f0,	0,		SROM4_TXPID5G, 0xff00},
	{"txpid5ga2",	0x000000f0,	0,		SROM4_TXPID5G + 1, 0xff},
	{"txpid5ga3",	0x000000f0,	0,		SROM4_TXPID5G + 1, 0xff00},
	{"txpid5gla0",	0x000000f0,	0,		SROM4_TXPID5GL, 0xff},
	{"txpid5gla1",	0x000000f0,	0,		SROM4_TXPID5GL, 0xff00},
	{"txpid5gla2",	0x000000f0,	0,		SROM4_TXPID5GL + 1, 0xff},
	{"txpid5gla3",	0x000000f0,	0,		SROM4_TXPID5GL + 1, 0xff00},
	{"txpid5gha0",	0x000000f0,	0,		SROM4_TXPID5GH, 0xff},
	{"txpid5gha1",	0x000000f0,	0,		SROM4_TXPID5GH, 0xff00},
	{"txpid5gha2",	0x000000f0,	0,		SROM4_TXPID5GH + 1, 0xff},
	{"txpid5gha3",	0x000000f0,	0,		SROM4_TXPID5GH + 1, 0xff00},
	{"cck2gpo",	0x000000f0,	0,		SROM4_2G_CCKPO, 0xffff},
	{"cck2gpo",	0xffffff00,	0,		SROM8_2G_CCKPO, 0xffff},
	{"ofdm2gpo",	0x000000f0,	SRFL_MORE,	SROM4_2G_OFDMPO, 0xffff},
	{"",		0,		0,		SROM4_2G_OFDMPO + 1, 0xffff},
	{"ofdm5gpo",	0x000000f0,	SRFL_MORE,	SROM4_5G_OFDMPO, 0xffff},
	{"",		0,		0,		SROM4_5G_OFDMPO + 1, 0xffff},
	{"ofdm5glpo",	0x000000f0,	SRFL_MORE,	SROM4_5GL_OFDMPO, 0xffff},
	{"",		0,		0,		SROM4_5GL_OFDMPO + 1, 0xffff},
	{"ofdm5ghpo",	0x000000f0,	SRFL_MORE,	SROM4_5GH_OFDMPO, 0xffff},
	{"",		0,		0,		SROM4_5GH_OFDMPO + 1, 0xffff},
	{"ofdm2gpo",	0xffffff00,	SRFL_MORE,	SROM8_2G_OFDMPO, 0xffff},
	{"",		0,		0,		SROM8_2G_OFDMPO + 1, 0xffff},
	{"ofdm5gpo",	0xffffff00,	SRFL_MORE,	SROM8_5G_OFDMPO, 0xffff},
	{"",		0,		0,		SROM8_5G_OFDMPO + 1, 0xffff},
	{"ofdm5glpo",	0xffffff00,	SRFL_MORE,	SROM8_5GL_OFDMPO, 0xffff},
	{"",		0,		0,		SROM8_5GL_OFDMPO + 1, 0xffff},
	{"ofdm5ghpo",	0xffffff00,	SRFL_MORE,	SROM8_5GH_OFDMPO, 0xffff},
	{"",		0,		0,		SROM8_5GH_OFDMPO + 1, 0xffff},
	{"mcs2gpo0",	0x000000f0,	0,		SROM4_2G_MCSPO, 0xffff},
	{"mcs2gpo1",	0x000000f0,	0,		SROM4_2G_MCSPO + 1, 0xffff},
	{"mcs2gpo2",	0x000000f0,	0,		SROM4_2G_MCSPO + 2, 0xffff},
	{"mcs2gpo3",	0x000000f0,	0,		SROM4_2G_MCSPO + 3, 0xffff},
	{"mcs2gpo4",	0x000000f0,	0,		SROM4_2G_MCSPO + 4, 0xffff},
	{"mcs2gpo5",	0x000000f0,	0,		SROM4_2G_MCSPO + 5, 0xffff},
	{"mcs2gpo6",	0x000000f0,	0,		SROM4_2G_MCSPO + 6, 0xffff},
	{"mcs2gpo7",	0x000000f0,	0,		SROM4_2G_MCSPO + 7, 0xffff},
	{"mcs5gpo0",	0x000000f0,	0,		SROM4_5G_MCSPO, 0xffff},
	{"mcs5gpo1",	0x000000f0,	0,		SROM4_5G_MCSPO + 1, 0xffff},
	{"mcs5gpo2",	0x000000f0,	0,		SROM4_5G_MCSPO + 2, 0xffff},
	{"mcs5gpo3",	0x000000f0,	0,		SROM4_5G_MCSPO + 3, 0xffff},
	{"mcs5gpo4",	0x000000f0,	0,		SROM4_5G_MCSPO + 4, 0xffff},
	{"mcs5gpo5",	0x000000f0,	0,		SROM4_5G_MCSPO + 5, 0xffff},
	{"mcs5gpo6",	0x000000f0,	0,		SROM4_5G_MCSPO + 6, 0xffff},
	{"mcs5gpo7",	0x000000f0,	0,		SROM4_5G_MCSPO + 7, 0xffff},
	{"mcs5glpo0",	0x000000f0,	0,		SROM4_5GL_MCSPO, 0xffff},
	{"mcs5glpo1",	0x000000f0,	0,		SROM4_5GL_MCSPO + 1, 0xffff},
	{"mcs5glpo2",	0x000000f0,	0,		SROM4_5GL_MCSPO + 2, 0xffff},
	{"mcs5glpo3",	0x000000f0,	0,		SROM4_5GL_MCSPO + 3, 0xffff},
	{"mcs5glpo4",	0x000000f0,	0,		SROM4_5GL_MCSPO + 4, 0xffff},
	{"mcs5glpo5",	0x000000f0,	0,		SROM4_5GL_MCSPO + 5, 0xffff},
	{"mcs5glpo6",	0x000000f0,	0,		SROM4_5GL_MCSPO + 6, 0xffff},
	{"mcs5glpo7",	0x000000f0,	0,		SROM4_5GL_MCSPO + 7, 0xffff},
	{"mcs5ghpo0",	0x000000f0,	0,		SROM4_5GH_MCSPO, 0xffff},
	{"mcs5ghpo1",	0x000000f0,	0,		SROM4_5GH_MCSPO + 1, 0xffff},
	{"mcs5ghpo2",	0x000000f0,	0,		SROM4_5GH_MCSPO + 2, 0xffff},
	{"mcs5ghpo3",	0x000000f0,	0,		SROM4_5GH_MCSPO + 3, 0xffff},
	{"mcs5ghpo4",	0x000000f0,	0,		SROM4_5GH_MCSPO + 4, 0xffff},
	{"mcs5ghpo5",	0x000000f0,	0,		SROM4_5GH_MCSPO + 5, 0xffff},
	{"mcs5ghpo6",	0x000000f0,	0,		SROM4_5GH_MCSPO + 6, 0xffff},
	{"mcs5ghpo7",	0x000000f0,	0,		SROM4_5GH_MCSPO + 7, 0xffff},
	{"mcs2gpo0",	0xffffff00,	0,		SROM8_2G_MCSPO, 0xffff},
	{"mcs2gpo1",	0xffffff00,	0,		SROM8_2G_MCSPO + 1, 0xffff},
	{"mcs2gpo2",	0xffffff00,	0,		SROM8_2G_MCSPO + 2, 0xffff},
	{"mcs2gpo3",	0xffffff00,	0,		SROM8_2G_MCSPO + 3, 0xffff},
	{"mcs2gpo4",	0xffffff00,	0,		SROM8_2G_MCSPO + 4, 0xffff},
	{"mcs2gpo5",	0xffffff00,	0,		SROM8_2G_MCSPO + 5, 0xffff},
	{"mcs2gpo6",	0xffffff00,	0,		SROM8_2G_MCSPO + 6, 0xffff},
	{"mcs2gpo7",	0xffffff00,	0,		SROM8_2G_MCSPO + 7, 0xffff},
	{"mcs5gpo0",	0xffffff00,	0,		SROM8_5G_MCSPO, 0xffff},
	{"mcs5gpo1",	0xffffff00,	0,		SROM8_5G_MCSPO + 1, 0xffff},
	{"mcs5gpo2",	0xffffff00,	0,		SROM8_5G_MCSPO + 2, 0xffff},
	{"mcs5gpo3",	0xffffff00,	0,		SROM8_5G_MCSPO + 3, 0xffff},
	{"mcs5gpo4",	0xffffff00,	0,		SROM8_5G_MCSPO + 4, 0xffff},
	{"mcs5gpo5",	0xffffff00,	0,		SROM8_5G_MCSPO + 5, 0xffff},
	{"mcs5gpo6",	0xffffff00,	0,		SROM8_5G_MCSPO + 6, 0xffff},
	{"mcs5gpo7",	0xffffff00,	0,		SROM8_5G_MCSPO + 7, 0xffff},
	{"mcs5glpo0",	0xffffff00,	0,		SROM8_5GL_MCSPO, 0xffff},
	{"mcs5glpo1",	0xffffff00,	0,		SROM8_5GL_MCSPO + 1, 0xffff},
	{"mcs5glpo2",	0xffffff00,	0,		SROM8_5GL_MCSPO + 2, 0xffff},
	{"mcs5glpo3",	0xffffff00,	0,		SROM8_5GL_MCSPO + 3, 0xffff},
	{"mcs5glpo4",	0xffffff00,	0,		SROM8_5GL_MCSPO + 4, 0xffff},
	{"mcs5glpo5",	0xffffff00,	0,		SROM8_5GL_MCSPO + 5, 0xffff},
	{"mcs5glpo6",	0xffffff00,	0,		SROM8_5GL_MCSPO + 6, 0xffff},
	{"mcs5glpo7",	0xffffff00,	0,		SROM8_5GL_MCSPO + 7, 0xffff},
	{"mcs5ghpo0",	0xffffff00,	0,		SROM8_5GH_MCSPO, 0xffff},
	{"mcs5ghpo1",	0xffffff00,	0,		SROM8_5GH_MCSPO + 1, 0xffff},
	{"mcs5ghpo2",	0xffffff00,	0,		SROM8_5GH_MCSPO + 2, 0xffff},
	{"mcs5ghpo3",	0xffffff00,	0,		SROM8_5GH_MCSPO + 3, 0xffff},
	{"mcs5ghpo4",	0xffffff00,	0,		SROM8_5GH_MCSPO + 4, 0xffff},
	{"mcs5ghpo5",	0xffffff00,	0,		SROM8_5GH_MCSPO + 5, 0xffff},
	{"mcs5ghpo6",	0xffffff00,	0,		SROM8_5GH_MCSPO + 6, 0xffff},
	{"mcs5ghpo7",	0xffffff00,	0,		SROM8_5GH_MCSPO + 7, 0xffff},
	{"cddpo",	0x000000f0,	0,		SROM4_CDDPO, 0xffff},
	{"stbcpo",	0x000000f0,	0,		SROM4_STBCPO, 0xffff},
	{"bw40po",	0x000000f0,	0,		SROM4_BW40PO, 0xffff},
	{"bwduppo",	0x000000f0,	0,		SROM4_BWDUPPO, 0xffff},
	{"cddpo",	0xffffff00,	0,		SROM8_CDDPO, 0xffff},
	{"stbcpo",	0xffffff00,	0,		SROM8_STBCPO, 0xffff},
	{"bw40po",	0xffffff00,	0,		SROM8_BW40PO, 0xffff},
	{"bwduppo",	0xffffff00,	0,		SROM8_BWDUPPO, 0xffff},
	{"ccode",	0x0000000f,	SRFL_CCODE,	SROM_CCODE, 0xffff},
	{"ccode",	0x00000010,	SRFL_CCODE,	SROM4_CCODE, 0xffff},
	{"ccode",	0x000000e0,	SRFL_CCODE,	SROM5_CCODE, 0xffff},
	{"ccode",	0xffffff00,	SRFL_CCODE,	SROM8_CCODE, 0xffff},
	{"macaddr",	0xffffff00,	SRFL_ETHADDR,	SROM8_MACHI, 0xffff},
	{"macaddr",	0x000000e0,	SRFL_ETHADDR,	SROM5_MACHI, 0xffff},
	{"macaddr",	0x00000010,	SRFL_ETHADDR,	SROM4_MACHI, 0xffff},
	{"macaddr",	0x00000008,	SRFL_ETHADDR,	SROM3_MACHI, 0xffff},
	{"il0macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_IL0, 0xffff},
	{"et1macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_ET1, 0xffff},
	{"leddc",	0xffffff00,	SRFL_NOFFS|SRFL_LEDDC,	SROM8_LEDDC, 0xffff},
	{"leddc",	0x000000e0,	SRFL_NOFFS|SRFL_LEDDC,	SROM5_LEDDC, 0xffff},
	{"leddc",	0x00000010,	SRFL_NOFFS|SRFL_LEDDC,	SROM4_LEDDC, 0xffff},
	{"leddc",	0x00000008,	SRFL_NOFFS|SRFL_LEDDC,	SROM3_LEDDC, 0xffff},
	{NULL,		0,		0,		0, 0}
};

static const sromvar_t perpath_pci_sromvars[] = {
	{"maxp2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP, 0xff},
	{"itt2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP, 0xff00},
	{"itt5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP, 0xff00},
	{"pa2gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA, 0xffff},
	{"pa2gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 1, 0xffff},
	{"pa2gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 2, 0xffff},
	{"pa2gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 3, 0xffff},
	{"maxp5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP, 0xff},
	{"maxp5gha",	0x000000f0,	0,		SROM4_5GLH_MAXP, 0xff},
	{"maxp5gla",	0x000000f0,	0,		SROM4_5GLH_MAXP, 0xff00},
	{"pa5gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA, 0xffff},
	{"pa5gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 1, 0xffff},
	{"pa5gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 2, 0xffff},
	{"pa5gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 3, 0xffff},
	{"pa5glw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA, 0xffff},
	{"pa5glw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 1, 0xffff},
	{"pa5glw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 2, 0xffff},
	{"pa5glw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 3, 0xffff},
	{"pa5ghw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA, 0xffff},
	{"pa5ghw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 1, 0xffff},
	{"pa5ghw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 2, 0xffff},
	{"pa5ghw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 3, 0xffff},
	{"maxp2ga",	0xffffff00,	0,		SROM8_2G_ITT_MAXP, 0xff},
	{"itt2ga",	0xffffff00,	0,		SROM8_2G_ITT_MAXP, 0xff00},
	{"itt5ga",	0xffffff00,	0,		SROM8_5G_ITT_MAXP, 0xff00},
	{"pa2gw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA, 0xffff},
	{"pa2gw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA + 1, 0xffff},
	{"pa2gw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_2G_PA + 2, 0xffff},
	{"maxp5ga",	0xffffff00,	0,		SROM8_5G_ITT_MAXP, 0xff},
	{"maxp5gha",	0xffffff00,	0,		SROM8_5GLH_MAXP, 0xff},
	{"maxp5gla",	0xffffff00,	0,		SROM8_5GLH_MAXP, 0xff00},
	{"pa5gw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA, 0xffff},
	{"pa5gw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA + 1, 0xffff},
	{"pa5gw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5G_PA + 2, 0xffff},
	{"pa5glw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA, 0xffff},
	{"pa5glw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA + 1, 0xffff},
	{"pa5glw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GL_PA + 2, 0xffff},
	{"pa5ghw0a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA, 0xffff},
	{"pa5ghw1a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA + 1, 0xffff},
	{"pa5ghw2a",	0xffffff00,	SRFL_PRHEX,	SROM8_5GH_PA + 2, 0xffff},
	{NULL,		0,		0,		0, 0}
};

/* Parse SROM and create name=value pairs. 'srom' points to
 * the SROM word array. 'off' specifies the offset of the
 * first word 'srom' points to, which should be either 0 or
 * SROM3_SWRG_OFF (full SROM or software region).
 */

static uint
mask_shift(uint16 mask)
{
	uint i;
	for (i = 0; i < (sizeof(mask) << 3); i ++) {
		if (mask & (1 << i))
			return i;
	}
	ASSERT(mask);
	return 0;
}

static uint
mask_width(uint16 mask)
{
	int i;
	for (i = (sizeof(mask) << 3) - 1; i >= 0; i --) {
		if (mask & (1 << i))
			return (uint)(i - mask_shift(mask) + 1);
	}
	ASSERT(mask);
	return 0;
}

#ifdef BCMDBG_ASSERT
static bool
mask_valid(uint16 mask)
{
	uint shift = mask_shift(mask);
	uint width = mask_width(mask);
	return mask == ((~0 << shift) & ~(~0 << (shift + width)));
}
#endif

static void
_initvars_srom_pci(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b)
{
	uint16 w;
	uint32 val;
	const sromvar_t *srv;
	uint width;
	uint flags;
	uint32 sr = (1 << sromrev);

	varbuf_append(b, "sromrev=%d", sromrev);

	for (srv = pci_sromvars; srv->name != NULL; srv ++) {
		const char *name;

		if ((srv->revmask & sr) == 0)
			continue;

		if (srv->off < off)
			continue;

		flags = srv->flags;
		name = srv->name;

		if (flags & SRFL_ETHADDR) {
			char eabuf[ETHER_ADDR_STR_LEN];
			struct ether_addr ea;

			ea.octet[0] = (srom[srv->off - off] >> 8) & 0xff;
			ea.octet[1] = srom[srv->off - off] & 0xff;
			ea.octet[2] = (srom[srv->off + 1 - off] >> 8) & 0xff;
			ea.octet[3] = srom[srv->off + 1 - off] & 0xff;
			ea.octet[4] = (srom[srv->off + 2 - off] >> 8) & 0xff;
			ea.octet[5] = srom[srv->off + 2 - off] & 0xff;
			bcm_ether_ntoa(&ea, eabuf);

			varbuf_append(b, "%s=%s", name, eabuf);
		}
		else {
			ASSERT(mask_valid(srv->mask));
			ASSERT(mask_width(srv->mask));

			w = srom[srv->off - off];
			val = (w & srv->mask) >> mask_shift(srv->mask);
			width = mask_width(srv->mask);

			while (srv->flags & SRFL_MORE) {
				srv ++;
				ASSERT(srv->name);

				if (srv->off == 0 || srv->off < off)
					continue;

				ASSERT(mask_valid(srv->mask));
				ASSERT(mask_width(srv->mask));

				w = srom[srv->off - off];
				val += ((w & srv->mask) >> mask_shift(srv->mask)) << width;
				width += mask_width(srv->mask);
			}

			if ((flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
				continue;

			if (flags & SRFL_CCODE) {
				if (val == 0)
					varbuf_append(b, "ccode=");
				else
					varbuf_append(b, "ccode=%c%c", (val >> 8), (val & 0xff));
			}
			/* LED Powersave duty cycle has to be scaled:
			 *(oncount >> 24) (offcount >> 8)
			 */
			else if (flags & SRFL_LEDDC) {
				uint32 w32 = (((val >> 8) & 0xff) << 24) | /* oncount */
					     (((val & 0xff)) << 8); /* offcount */
				varbuf_append(b, "leddc=%d", w32);
			}
			else if (flags & SRFL_PRHEX)
				varbuf_append(b, "%s=0x%x", name, val);
			else if ((flags & SRFL_PRSIGN) && (val & (1 << (width - 1))))
				varbuf_append(b, "%s=%d", name, (int)(val | (~0 << width)));
			else
				varbuf_append(b, "%s=%u", name, val);
		}
	}

	if (sromrev >= 4) {
		/* Do per-path variables */
		uint p, pb, psz;

		if (sromrev >= 8) {
			pb = SROM8_PATH0;
			psz = SROM8_PATH1 - SROM8_PATH0;
		} else {
			pb = SROM4_PATH0;
			psz = SROM4_PATH1 - SROM4_PATH0;
		}

		for (p = 0; p < MAX_PATH_SROM; p++) {
			for (srv = perpath_pci_sromvars; srv->name != NULL; srv ++) {
				if ((srv->revmask & sr) == 0)
					continue;

				if (pb + srv->off < off)
					continue;

				w = srom[pb + srv->off - off];
				ASSERT(mask_valid(srv->mask));
				val = (w & srv->mask) >> mask_shift(srv->mask);
				width = mask_width(srv->mask);

				/* Cheating: no per-path var is more than 1 word */

				if ((srv->flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
					continue;

				if (srv->flags & SRFL_PRHEX)
					varbuf_append(b, "%s%d=0x%x", srv->name, p, val);
				else
					varbuf_append(b, "%s%d=%d", srv->name, p, val);
			}
			pb += psz;
		}
	}
}

static int
initvars_srom_pci(sb_t *sbh, void *curmap, char **vars, uint *count)
{
	uint16 *srom;
	uint8 sromrev = 0;
	uint32 sr;
	varbuf_t b;
	char *vp, *base = NULL;
	osl_t *osh = sb_osh(sbh);
	bool flash = FALSE;
	char *value;
	int err;

	/*
	 * Apply CRC over SROM content regardless SROM is present or not,
	 * and use variable <devpath>sromrev's existance in flash to decide
	 * if we should return an error when CRC fails or read SROM variables
	 * from flash.
	 */
	srom = MALLOC(osh, SROM_MAX);
	ASSERT(srom);
	if (!srom)
		return -2;

	err = sprom_read_pci(osh, sbh, (void *)((int8 *)curmap + PCI_BAR0_SPROM_OFFSET), 0, srom,
	                     SROM_WORDS, TRUE);

	if ((srom[SROM4_SIGN] == SROM4_SIGNATURE) ||
	    (((sbh->buscoretype == SB_PCIE) && (sbh->buscorerev >= 6)) ||
	     ((sbh->buscoretype == SB_PCI) && (sbh->buscorerev >= 0xe)))) {
		/* sromrev >= 4, read more */
		err = sprom_read_pci(osh, sbh, (void *)((int8 *)curmap + PCI_BAR0_SPROM_OFFSET), 0,
		                     srom, SROM4_WORDS, TRUE);
		sromrev = srom[SROM4_CRCREV] & 0xff;

	} else if (err == 0) {
		/* srom is good and is rev < 4 */
		/* top word of sprom contains version and crc8 */
		sromrev = srom[SROM_CRCREV] & 0xff;
		/* bcm4401 sroms misprogrammed */
		if (sromrev == 0x10)
			sromrev = 1;
	}

	if (err) {
#ifdef WLTEST
		uint32 val;

		BS_ERROR(("SROM Crc Error, so see if we could use a default\n"));
		val = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		if (val & SPROM_OTPIN_USE) {
			BS_ERROR(("srom crc failed with OTP, use default vars....\n"));
			vp = base = mfgsromvars;
			if (sb_chip(sbh) == BCM4311_CHIP_ID) {
				const char *devid = "devid=0x4311";
				const size_t devid_strlen = strlen(devid);
				BS_ERROR(("setting the devid to be 4311\n"));
				bcopy(devid, vp, devid_strlen + 1);
				vp += devid_strlen + 1;
			}
			bcopy(defaultsromvars, vp, MFGSROM_DEFVARSLEN);
			vp += MFGSROM_DEFVARSLEN;
			goto varsdone;
		} else {
#endif /* WLTEST */
			BS_ERROR(("srom crc failed with SPROM....\n"));
			if (!(value = sb_getdevpathvar(sbh, "sromrev"))) {
				err = -1;
				goto errout;
			}
			sromrev = (uint8)bcm_strtoul(value, NULL, 0);
			flash = TRUE;
#ifdef WLTEST
		}
#endif /* WLTEST */
	}

	/* Bitmask for the sromrev */
	sr = 1 << sromrev;

	/* srom version check
	 * Current valid versions: 1, 2, 3, 4, 5, 8
	 */
	if ((sr & 0x13e) == 0) {
		err = -2;
		goto errout;
	}

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp);
	if (!vp) {
		err = -2;
		goto errout;
	}

	/* read variables from flash */
	if (flash) {
		if ((err = initvars_flash(sbh, osh, &vp, MAXSZ_NVRAM_VARS)))
			goto errout;
		goto varsdone;
	}

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);

	/* parse SROM into name=value pairs. */
	_initvars_srom_pci(sromrev, srom, 0, &b);

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
	vp = b.buf;
	*vp++ = '\0';

	ASSERT((vp - base) <= MAXSZ_NVRAM_VARS);

varsdone:
	err = initvars_table(osh, base, vp, vars, count);

errout:
#ifdef WLTEST
	if (base && (base != mfgsromvars))
#else
	if (base)
#endif
		MFREE(osh, base, MAXSZ_NVRAM_VARS);

	MFREE(osh, srom, SROM_MAX);
	return err;
}

/*
 * Read the cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
initvars_cis_pcmcia(sb_t *sbh, osl_t *osh, char **vars, uint *count)
{
	uint8 *cis = NULL;
	int rc;
	uint data_sz;

	data_sz = (sb_pcmciarev(sbh) == 1) ? (SPROM_SIZE * 2) : CIS_SIZE;

	if ((cis = MALLOC(osh, data_sz)) == NULL)
		return (-2);

	if (sb_pcmciarev(sbh) == 1) {
		if (srom_read(sbh, PCMCIA_BUS, (void *)NULL, osh, 0, data_sz, (uint16 *)cis)) {
			MFREE(osh, cis, data_sz);
			return (-1);
		}
		/* fix up endianess for 16-bit data vs 8-bit parsing */
		htol16_buf((uint16 *)cis, data_sz);
	} else
		OSL_PCMCIA_READ_ATTR(osh, 0, cis, data_sz);

	rc = srom_parsecis(osh, &cis, 1, vars, count);

	MFREE(osh, cis, data_sz);

	return (rc);
}

#ifdef BCMSDIO
/*
 * Read the SDIO cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
initvars_cis_sdio(osl_t *osh, char **vars, uint *count)
{
	uint8 *cis[SBSDIO_NUM_FUNCTION + 1];
	uint fn, numfn;
	int rc = 0;


	numfn = bcmsdh_query_iofnum(NULL);
	ASSERT(numfn <= SDIOD_MAX_IOFUNCS);

	for (fn = 0; fn <= numfn; fn++) {
		if ((cis[fn] = MALLOC(osh, SBSDIO_CIS_SIZE_LIMIT)) == NULL) {
			rc = -1;
			break;
		}
		
		bzero(cis[fn], SBSDIO_CIS_SIZE_LIMIT);
		if (bcmsdh_cis_read(NULL, fn, cis[fn], SBSDIO_CIS_SIZE_LIMIT) != 0) {
			MFREE(osh, cis[fn], SBSDIO_CIS_SIZE_LIMIT);
			rc = -2;
			break;
		}
	}

	if (!rc)
		rc = srom_parsecis(osh, cis, fn, vars, count);

	while (fn-- > 0)
		MFREE(osh, cis[fn], SBSDIO_CIS_SIZE_LIMIT);

	return (rc);
}

/* set SDIO sprom command register */
static int
sprom_cmd_sdio(osl_t *osh, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 1000;

	/* write sprom command register */
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_CS, cmd, NULL);

	/* wait status */
	while (wait_cnt--) {
		status = bcmsdh_cfg_read(NULL, SDIO_FUNC_1, SBSDIO_SPROM_CS, NULL);
		if (status & SBSDIO_SPROM_DONE)
			return 0;
	}

	return 1;
}

/* read a word from the SDIO srom */
static int
sprom_read_sdio(osl_t *osh, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_ADDR_HIGH, addr_h, NULL);
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_ADDR_LOW, addr_l, NULL);

	/* do read */
	if (sprom_cmd_sdio(osh, SBSDIO_SPROM_READ))
		return 1;

	/* read data */
	data_h = bcmsdh_cfg_read(NULL, SDIO_FUNC_1, SBSDIO_SPROM_DATA_HIGH, NULL);
	data_l = bcmsdh_cfg_read(NULL, SDIO_FUNC_1, SBSDIO_SPROM_DATA_LOW, NULL);

	*data = (data_h << 8) | data_l;
	return 0;
}

/* write a word to the SDIO srom */
static int
sprom_write_sdio(osl_t *osh, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);
	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* set address */
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_ADDR_HIGH, addr_h, NULL);
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_ADDR_LOW, addr_l, NULL);

	/* write data */
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_DATA_HIGH, data_h, NULL);
	bcmsdh_cfg_write(NULL, SDIO_FUNC_1, SBSDIO_SPROM_DATA_LOW, data_l, NULL);

	/* do write */
	return sprom_cmd_sdio(osh, SBSDIO_SPROM_WRITE);
}
#endif /* BCMSDIO */

#if defined(BCMUSBDEV)
static int
BCMINITFN(initvars_srom_sb)(sb_t *sbh, osl_t *osh, void *curmap, char **vars, uint *varsz)
{
	static bool srvars = FALSE;	/* Use OTP/SPROM as global variables */

	int sel = 0;	/* where to read srom/cis: 0 - none, 1 - otp, 2 - sprom */
	uint sz = 0;	/* srom size in bytes */
	void *oh = NULL;
	int rc = BCME_OK;

	/* Bail out if we've dealt with OTP/SPROM before! */
	if (srvars)
		return 0;

#if defined(BCM4328)
	if (sbh->chip == BCM4328_CHIP_ID) {
		/* Access the SPROM if it is present */
		if ((sz = srom_size(sbh, osh)) != 0) {
			sz <<= 1;
			sel = 2;
		}
	}
#endif
#if defined(BCM4325)
	if (sbh->chip == BCM4325_CHIP_ID) {
		uint32 cst = sbh->chipst & CST4325_SPROM_OTP_SEL_MASK;

		/* Access OTP if it is present, powered on, and programmed */
		if ((oh = otp_init(sbh)) != NULL && (otp_status(oh) & OTPS_GUP_SW)) {
			sz = otp_size(oh);
			sel = 1;
		}
		/* Access the SPROM if it is present and allow to be accessed */
		else if ((cst == CST4325_OTP_PWRDN || cst == CST4325_SPROM_SEL) &&
		         (sz = srom_size(sbh, osh)) != 0) {
			sz <<= 1;
			sel = 2;
		}
	}
#endif	/* BCM4325 */
#ifdef BCM4322
	if (sbh->chip == BCM4322_CHIP_ID) {
		/* Access OTP if it is present and programmed */
		if ((oh = otp_init(sbh)) != NULL && (otp_status(oh) & OTPS_GUP_SW)) {
			sz = otp_size(oh);
			sel = 1;
		}
	}
#endif	/* BCM4322 */

	/* Read CIS in OTP/SPROM */
	if (sel != 0) {
		uint16 *srom;
		uint8 *body = NULL;

		ASSERT(sz);

		/* Allocate memory */
		if ((srom = (uint16 *)MALLOC(osh, sz)) == NULL)
			return BCME_NOMEM;

		/* Read CIS */
		switch (sel) {
		case 1:
			rc = otp_read_region(oh, OTP_SW_RGN, srom, sz);
			body = (uint8 *)srom;
			break;
		case 2:
			rc = srom_read(sbh, SB_BUS, curmap, osh, 0, sz, srom);
			/* sprom has 8 byte h/w header */
			body = (uint8 *)srom + SBSDIO_SPROM_CIS_OFFSET;
			break;
		default:
			/* impossible to come here */
			ASSERT(0);
			break;
		}

		/* Parse CIS */
		if (rc == BCME_OK) {
			uint i, tpls = 0xffffffff;
			/* # sdiod fns + common + extra */
			uint8 *cis[SBSDIO_NUM_FUNCTION + 2];
			uint ciss = 0;

			/* each word is in host endian */
			htol16_buf((uint8 *)srom, sz);

			ASSERT(body);

			/* count cis tuple chains */
			for (i = 0; i < sz && ciss < ARRAYSIZE(cis) && tpls != 0; i ++) {
				cis[ciss++] = &body[i];
				for (tpls = 0; i < sz - 1; tpls ++) {
					if (body[i++] == CISTPL_END)
						break;
					i += body[i] + 1;
				}
			}

			/* call parser routine only when there are tuple chains */
			if (ciss > 1)
				rc = srom_parsecis(osh, cis, ciss, vars, varsz);
		}

		/* Clean up */
		MFREE(osh, srom, sz);

		/* Make SROM variables global */
		if (rc == BCME_OK) {
			rc = nvram_append((void *)sbh, *vars, *varsz);
			srvars = TRUE;

			/* Tell the caller there is no individual SROM variables */
			*vars = NULL;
			*varsz = 0;
		}
	}

	return rc;
}

/* Return sprom size in 16-bit words */
static uint
srom_size(sb_t *sbh, osl_t *osh)
{
	uint size = 0;
	if (SPROMBUS == PCMCIA_BUS) {
		uint32 origidx;
		sdpcmd_regs_t *pcmregs;
		bool wasup;

		origidx = sb_coreidx(sbh);
		pcmregs = sb_setcore(sbh, SB_PCMCIA, 0);
		ASSERT(pcmregs);

		if (!(wasup = sb_iscoreup(sbh)))
			sb_core_reset(sbh, 0, 0);

		/* not worry about earlier core revs */
		if (sb_corerev(sbh) < 8)
			goto done;

		/* SPROM is accessible only in PCMCIA mode unless there is SDIO clock */
		if (!(R_REG(osh, &pcmregs->corestatus) & CS_PCMCIAMODE))
			goto done;

		switch (SB_PCMCIA_READ(osh, pcmregs, SROM_INFO) & SRI_SZ_MASK) {
		case 1:
			size = 256;	/* SROM_INFO == 1 means 4kbit */
			break;
		case 2:
			size = 1024;	/* SROM_INFO == 2 means 16kbit */
			break;
		default:
			break;
		}

	done:
		if (!wasup)
			sb_core_disable(sbh, 0);

		sb_setcoreidx(sbh, origidx);
	}
	return size;
}
#else /* !BCMUSBDEV && !BCMSDIODEV */
static int
BCMINITFN(initvars_srom_sb)(sb_t *sbh, osl_t *osh, void *curmap, char **vars, uint *varsz)
{
	/* Search flash nvram section for srom variables */
	return initvars_flash_sb(sbh, vars, varsz);
}
#endif	
