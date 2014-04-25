/*
 * linux/arch/arm/mach-sa1100/leds-system3.c
 *
 * Copyright (C) 2001 Stefan Eletzhofer <stefan.eletzhofer@gmx.de>
 *
 * Original (leds-footbridge.c) by Russell King
 *
 * $Id: leds-system3.c,v 1.1 2008/08/26 01:33:21 l65130 Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * $Log: leds-system3.c,v $
 * Revision 1.1  2008/08/26 01:33:21  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 09:36:46  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 05:31:20  z30370
 * *** empty log message ***
 *
 * Revision 1.1  2006/12/20 08:37:40  d55909
 * *** empty log message ***
 *
 * Revision 1.1  2006/11/14 08:12:57  d55909
 * 新建模块
 *
 * Revision 1.1  2006/04/19 05:14:55  z60003055
 * z60003055：新增BCM306 版本代码基线
 *
 * Revision 1.1.6.1  2001/12/04 15:19:26  seletz
 * - merged from linux_2_4_13_ac5_rmk2
 *
 * Revision 1.1.4.2  2001/11/19 17:58:53  seletz
 * - cleanup
 *
 * Revision 1.1.4.1  2001/11/16 13:49:54  seletz
 * - dummy LED support for PT Digital Board
 *
 * Revision 1.1.2.1  2001/10/15 16:03:39  seletz
 * - dummy function
 *
 *
 */
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>

#include "leds.h"


#define LED_STATE_ENABLED	1
#define LED_STATE_CLAIMED	2

static unsigned int led_state;
static unsigned int hw_led_state;

void system3_leds_event(led_event_t evt)
{

	/* TODO: support LEDs */
}
