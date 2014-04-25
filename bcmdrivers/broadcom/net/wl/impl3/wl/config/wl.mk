# Helper makefile for building Broadcom wl device driver
# This file maps wl driver feature flags (import) to WLFLAGS and WLFILES (export).
#
# Copyright 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
# $Id: wl.mk,v 1.1 2008/08/25 06:42:23 l65130 Exp $

# os-independent config flag -> WLFLAGS and WLFILES mapping

# debug/internal
ifeq ($(DEBUG),1)
	WLFLAGS += -DBCMDBG -DWLTEST
else
	# This is true for mfgtest builds.
	ifeq ($(WLTEST),1)
	WLFLAGS += -DWLTEST -DBCMNVRAMW
	BCMNVRAMW=1
	endif
endif


ifeq ($(BCMDBG_MEM),1)
	WLFLAGS += -DBCMDBG_MEM
endif

ifeq ($(BCMDBG_PKT),1)
	WLFLAGS += -DBCMDBG_PKT
endif

ifeq ($(WLLMAC),1)
	WLFLAGS += -DWLLMAC -DEXTENDED_SCAN
endif

ifeq ($(WLNOKIA),1)
	WLFLAGS += -DWLNOKIA
	WLFILES += wlc_nokia.c
endif

## wl driver common
#w/wpa
ifeq ($(WL),1)
	WLFILES += wlc.c d11ucode.c wlc_phy.c qmath.c wlc_rate.c wlc_security.c
	WLFILES += wlc_key.c wlc_scb.c wlc_rate_sel.c
	ifeq ($(WLLMAC),1)
		WLFILES += wlc_lmac.c
		ifeq ($(STA), 1)
			WLFILES += wlc_lmac_sta.c
		endif
		ifeq ($(WLLMACPROTO), 1)
			WLFLAGS += -DWLLMACPROTO
			WLFILES += wlc_lmac_proto.c
		endif
	else
		WLFILES += wlc_event.c wlc_channel.c
	endif
	WLFILES += wlc_bsscfg.c mimophytbls.c lpphytbls.c wlc_scan.c
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES += bcmwpa.c rc4.c tkhash.c tkmic.c wep.c
	endif
endif

## wl OSL
ifeq ($(WLVX),1)
	WLFILES += wl_vx.c
	WLFILES += bcmstdlib.c
endif

ifeq ($(WLBSD),1)
	WLFILES += wl_bsd.c
endif

ifeq ($(WLLX),1)
	WLFILES += wl_linux.c
endif

ifeq ($(WLLXIW),1)
	WLFILES += wl_iw.c
	WLFILES += bcmwifi.c
endif

ifeq ($(WLNDIS),1)
	WLFILES += wl_ndis.c
  ifeq ($(WLDHD),)
	WLFILES += nhd_ndis.c
  endif
	WLFILES += wl_ndconfig.c	
	WLFILES += bcmstdlib.c
	WLFILES += bcmwifi.c
endif

ifeq ($(WLCFE),1)
	WLFILES += wl_cfe.c
endif

ifeq ($(WLRTE),1)
	WLFILES += wl_rte.c
endif


## wl special
# oids

#ifdef BINOSL
	ifeq ($(BINOSL),1)
		WLFLAGS += -DBINOSL
	endif
#endif

## wl features
# ap
ifeq ($(AP),1)
	WLFILES += wlc_ap.c
	WLFILES += wlc_apps.c
	WLFILES += wlc_apcs.c
	WLFLAGS += -DAP

	ifeq ($(MBSS),1)
		WLFLAGS += -DMBSS
	endif

	ifeq ($(WME_PER_AC_TX_PARAMS),1)
		WLFLAGS += -DWME_PER_AC_TX_PARAMS
	endif

	ifeq ($(WME_PER_AC_TUNING),1)
		WLFLAGS += -DWME_PER_AC_TUNING
	endif

endif

# sta
ifeq ($(STA),1)
	WLFLAGS += -DSTA
	ifeq ($(WLVISTA),1)
		WLFLAGS += -DEXT_STA
		WLFLAGS += -DWLNOEIND
		WLFLAGS += -DWL_MONITOR
		WLFLAGS += -DIBSS_PEER_GROUP_KEY
		WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
		WLFLAGS += -DIBSS_PEER_MGMT
	endif
	ifeq ($(WLXP),1)
		WLFLAGS += -DWLNOEIND
	endif
	ifeq ($(WLVISTA_USBDHD),1)
		WLFLAGS += -DEXT_STA
	endif
	ifeq ($(WLVISTA_DONGLE),1)
		WLFLAGS += -DEXT_STA
		WLFLAGS += -DWL_MONITOR
	endif
endif

ifeq ($(WLDHD),1)
	WLFLAGS += -DBCMEMBEDIMAGE
	WLFLAGS += -DSHOW_EVENTS -DBCMDHDUSB -DBCMDONGLEHOST
	WLFLAGS += -DBCMPERFSTATS -DBDC
	WLFLAGS += -DBCM4325 -DBCM4328

	WLFILES += dhd_cdc.c
	WLFILES += dhd_common.c
	WLFILES += dhd_ndis.c
	WLFILES += dhd_usb_ndis.c

	ifeq ($(WLVISTA_USBDHD),1)
		WLFLAGS += -DVISTA_DONGLE
		WLFILES += wlc_rate.c
	endif

	ifeq ($(WLXP_USBDHD),1)
		WLFLAGS += -DNDIS_DMAWAR -DBINARY_COMPATIBLE -DWIN32_LEAN_AND_MEAN=1 -DMEMORY_TAG="'432x'"
	endif
endif

# apsta
ifeq ($(APSTA),1)
	WLFLAGS += -DAPSTA
endif
# apsta

# wet
ifeq ($(WET),1)
	WLFLAGS += -DWET
	WLFILES += wlc_wet.c
endif

# mac spoof
ifeq ($(MAC_SPOOF),1)
	WLFLAGS += -DMAC_SPOOF
endif

# Router IBSS Security Support
ifeq ($(ROUTER_SECURE_IBSS),1)
	WLFLAGS += -DIBSS_PEER_GROUP_KEY
	WLFLAGS += -DIBSS_PSK
	WLFLAGS += -DIBSS_PEER_MGMT
	WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
endif

# led
ifeq ($(WLLED),1)
	WLFLAGS += -DWLLED
	WLFILES += wlc_led.c
endif

# WME
ifeq ($(WME),1)
	WLFLAGS += -DWME
endif

# WLBA
ifeq ($(WLBA),1)
	WLFLAGS += -DWLBA
	WLFILES += wlc_ba.c
endif

# WLPIO 
ifeq ($(WLPIO),1)
	WLFLAGS += -DWLPIO
	WLFILES += wlc_pio.c
endif

# CRAM
ifeq ($(CRAM),1)
	WLFLAGS += -DCRAM
	WLFILES += wlc_cram.c
endif

# 11H 
ifeq ($(WL11H),1)
	WLFLAGS += -DWL11H
endif

# 11D 
ifeq ($(WL11D),1)
	WLFLAGS += -DWL11D
endif

# DBAND
ifeq ($(DBAND),1)
	WLFLAGS += -DDBAND
endif

# WLRM
ifeq ($(WLRM),1)
	WLFLAGS += -DWLRM
endif

# WLCQ
ifeq ($(WLCQ),1)
	WLFLAGS += -DWLCQ
endif

# WLCNT
ifeq ($(WLCNT),1)
	WLFLAGS += -DWLCNT
endif

# WLCNTSCB
ifeq ($(WLCNTSCB),1)
	WLFLAGS += -DWLCNTSCB
endif

# WLMOTOROLALJ
ifeq ($(WLMOTOROLALJ),1)
	WLFLAGS += -DWLMOTOROLALJ
endif

# WLCOEX
ifeq ($(WLCOEX),1)
	WLFLAGS += -DWLCOEX
endif

## wl security
# in-driver supplicant
ifeq ($(BCMSUP_PSK),1)
	WLFLAGS += -DBCMSUP_PSK
	WLFILES += wlc_sup.c
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES += aes.c aeskeywrap.c hmac.c prf.c sha1.c
		##NetBSD 2.0 has MD5 and AES built in
		ifneq ($(OSLBSD),1)
			WLFILES += md5.c rijndael-alg-fst.c
		endif
	endif
	WLFILES += passhash.c
endif

# bcmccx

# BCMWPA2
ifeq ($(BCMWPA2),1)
	WLFLAGS += -DBCMWPA2
endif

# Soft AES CCMP
ifeq ($(BCMCCMP),1)
	WLFLAGS += -DBCMCCMP
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES += aes.c
		##BSD has  AES built in
		ifneq ($(BSD),1)
			WLFILES +=rijndael-alg-fst.c
		endif
	endif
endif


# BCMDMA64
ifeq ($(BCMDMA64),1)
	WLFLAGS += -DBCMDMA64
endif

## wl over jtag
#ifdef BCMJTAG
	ifeq ($(BCMJTAG),1)
		WLFLAGS += -DBCMJTAG -DBCMSLTGT
		WLFILES += bcmjtag.c bcmjtag_linux.c ejtag.c jtagm.c
	endif
#endif

ifeq ($(WLAMSDU),1)
	WLFLAGS += -DWLAMSDU
	WLFILES += wlc_amsdu.c
endif

ifeq ($(WLAMSDU_SWDEAGG),1)
	WLFLAGS += -DWLAMSDU_SWDEAGG
endif

ifeq ($(WLAMPDU),1)
	WLFLAGS += -DWLAMPDU
	WLFILES += wlc_ampdu.c
endif

ifeq ($(WOWL),1)
	WLFLAGS += -DWOWL
	WLFILES += d11wakeucode.c wlc_wowl.c wowlaestbls.c
endif

ifeq ($(BTC2WIRE),1)
	WLFLAGS += -DBTC2WIRE
	WLFILES += d11ucode_2w.c
endif


ifeq ($(WL_ASSOC_RECREATE),1)
	ifeq ($(STA),1)
		WLFLAGS += -DWL_ASSOC_RECREATE
		ifneq ($(BCMDBG),1)
			WLFILES += bcmwifi.c
		endif
	endif
endif

ifeq ($(WLDPT),1)
	WLFLAGS += -DWLDPT
	WLFILES += wlc_dpt.c
endif

ifeq ($(WLPLT),1)
	WLFLAGS += -DWLPLT
	WLFILES += wlc_plt.c
endif


## --- which buses

# silicon backplane

ifeq ($(BCMSBBUS),1)
	WLFLAGS += -DBCMBUSTYPE=SB_BUS
endif


# sdio
#ifdef BCMSDIO
	ifeq ($(BCMSDIO),1)
		WLFLAGS += -DBCMSDIO -DBCM4328
#ifdef BCM4328
		WLFLAGS += -DBCM4328
#endif
#ifdef BCM4325
		WLFLAGS += -DBCM4325
#endif
		# Non-PCI WL SDIO Driver
		ifeq ($(BCMSDIONP),1)
			WLFLAGS += -DBCMSDIONP
		else
			# SDNOW! SDIO stack for WindowsCE
			ifeq ($(BCMSDIO_SDNOW),1)
				WLFILES += sdnow.c
			else
				# BRCM SDIO stack for Standard SDIO host
				ifeq ($(BCMSDIOH_STD),1)
					WLFLAGS += -DBCMSDIOH_STD
					WLFILES += bcmsdh.c bcmsdstd.c
					ifeq ($(WLIOPOS), 1) # Our Arasan stack in IOP OS.
						WLFILES += bcmsdstd_iopos.c bcmsdh_iopos.c
					endif
					ifeq ($(OSLLX),1)
						WLFILES += bcmsdstd_linux.c bcmsdh_linux.c
					else 
						ifeq ($(OSLNDIS),1)
							WLFILES += bcmsdstd_ndis.c bcmsdh_ndis.c
						endif
					endif
				endif
				# BRCM SDIO stack for BRCM SDIO host
				ifeq ($(BCMSDIOH_BCM),1)
					WLFLAGS += -DBCMSDIOH_BCM
					WLFILES += bcmsdh.c bcmsdbrcm.c bcmsdh_linux.c \
						bcmsdbrcm_linux.c
				endif
				# BRCM SDIO stack for BRCM PciSpiHost host
				ifeq ($(BCMSDIOH_SPI),1)
					WLFLAGS += -DBCMSDIOH_SPI
					WLFILES += bcmsdh.c bcmsdh_linux.c bcmsdspi.c bcmsdspi_linux.c \
						bcmpcispi.c
				endif
				# BCRM SDIO stub functions
				ifeq ($(BCMSDIOH_STUB),1)
					WLFILES += bcmsdh_stubs.c
				endif
			endif
		endif
	endif
#endif

# AP/ROUTER with SDSTD
ifeq ($(WLAPSDSTD),1)
	WLFILES += nvramstubs.c bcmsrom.c
endif

## --- basic shared files

ifeq ($(HNDDMA),1)
	WLFILES += hnddma.c
endif

ifeq ($(BCMUTILS),1)
	WLFILES += bcmutils.c
endif

ifeq ($(BCMSROM),1)
	WLFILES += bcmsrom.c bcmotp.c
endif

ifeq ($(SBUTILS),1)
	WLFILES += sbutils.c hndpmu.c
endif

ifeq ($(SBMIPS),1)
	WLFILES += hndmips.c hndchipc.c
endif

ifeq ($(SBSDRAM),1)
	WLFILES += sbsdram.c
endif

ifeq ($(SBPCI),1)
	WLFILES += hndpci.c
endif

ifeq ($(SFLASH),1)
	WLFILES += sflash.c
endif

ifeq ($(FLASHUTL),1)
	WLFILES += flashutl.c
endif


## --- shared OSL
# linux osl
ifeq ($(OSLLX),1)
	WLFILES += linux_osl.c
endif

ifeq ($(OSLLXPCI),1)
	WLFILES += linux_pci.c
endif

# vx osl
ifeq ($(OSLVX),1)
	WLFILES += vx_osl.c
	WLFILES += bcmallocache.c
endif

# bsd osl
ifeq ($(OSLBSD),1)
	WLFILES += bsd_osl.c nvramstubs.c
endif

ifeq ($(OSLCFE),1)
	WLFILES += cfe_osl.c
endif

ifeq ($(OSLRTE),1)
	WLFILES += hndrte_osl.c
endif

ifeq ($(OSLNDIS),1)
	WLFILES += ndshared.c ndis_osl.c
endif

ifeq ($(CONFIG_USBRNDIS_RETAIL),1)
	WLFLAGS += -DCONFIG_USBRNDIS_RETAIL
	WLFILES += wl_ndconfig.c
	ifneq ($(WLLXIW),1)
		WLFILES += bcmwifi.c
	endif
endif

ifeq ($(NVRAM),1)
	WLFILES += nvram.c
endif

ifeq ($(NVRAMVX),1)
	WLFILES += nvram_rw.c
endif

ifeq ($(BCMNVRAMR),1)
	WLFILES += nvram_ro.c sflash.c bcmotp.c
	WLFLAGS += -DBCMNVRAMR
else
	ifeq ($(BCMNVRAMW),1)
		WLFILES += bcmotp.c
	endif
endif

## --- DSLCPE
ifeq ($(DSLCPE),1)
	WLFILES += wl_linux_dslcpe.c
	WLFLAGS += -DDSLCPE
endif

ifeq ($(WLDIAG),1)
	WLFLAGS += -DWLDIAG
	WLFILES += wlc_diag.c
endif

ifeq ($(WLTIMER),1)
	WLFLAGS += -DWLTIMER
endif

ifneq ($(BCMDBG),1)
	ifeq ($(WLTINYDUMP),1)
		WLFLAGS += -DWLTINYDUMP
	endif
endif

ifeq ($(BCMQT),1)
  # Set flag to indicate emulated chip
  WLFLAGS += -DBCMSLTGT -DBCMQT
  ifeq ($(WLRTE),1)
    # Use of RTE implies embedded (CPU emulated)
    WLFLAGS += -DBCMQT_CPU
  endif
endif

ifeq ($(BCMPMU),1)
  WLFLAGS += -DBCMPMU
endif

#ifdef WLPFN
ifeq ($(WLPFN),1)
	WLFLAGS += -DWLPFN
	WLFILES += wl_pfn.c
	ifeq ($(WLPFN_AUTO_CONNECT),1)
		WLFLAGS += -DWLPFN_AUTO_CONNECT
	endif
endif
#endif

#ifdef TOE
ifeq ($(TOE),1)
	WLFLAGS += -DTOE
	WLFILES += wl_toe.c
endif
#endif

#ifdef ARPOE
ifeq ($(ARPOE),1)
	WLFLAGS += -DARPOE
	WLFILES += wl_arpoe.c
endif
#endif

#ifdef PCOEM_LINUXBIN
ifeq ($(PCOEM_LINUXBIN),1)
	WLFLAGS += -DPCOEM_LINUXBIN
endif
#endif

#wlinfo:
#	@echo "WLFLAGS=\"$(WLFLAGS)\""
#	@echo "WLFILES=\"$(WLFILES)\""
