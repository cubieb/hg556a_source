/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 by Ralf Baechle
 */
#ifndef __ASM_MACH_GENERIC_PARAM_H
#define __ASM_MACH_GENERIC_PARAM_H

/* Header file config.h, which defines CONFIG_MIPS_BRCM, is not included by
 * all files that include this header file.  Therefore, unconditionally
 * define HZ to 200.
 */
#if 1 /* defined(CONFIG_MIPS_BRCM) */
#define HZ		200		    /* Internal kernel timer frequency */
#else
#define HZ		1000		/* Internal kernel timer frequency */
#endif

#endif /* __ASM_MACH_GENERIC_PARAM_H */
