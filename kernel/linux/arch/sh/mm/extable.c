/* $Id: extable.c,v 1.1 2008/08/26 01:47:04 l65130 Exp $
 *
 * linux/arch/sh/mm/extable.c
 *  Taken from:
 *   linux/arch/i386/mm/extable.c
 */

#include <linux/config.h>
#include <linux/module.h>
#include <asm/uaccess.h>

int fixup_exception(struct pt_regs *regs)
{
	const struct exception_table_entry *fixup;

	fixup = search_exception_tables(regs->pc);
	if (fixup) {
		regs->pc = fixup->fixup;
		return 1;
	}

	return 0;
}
