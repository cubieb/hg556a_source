/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2000 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : dspdrv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the dsp driver.
 *
 ***************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/timex.h>
#include <asm/uaccess.h>

/*--------------------------------------------------------------------------
** IMPORTANT: The following symbol represents the entry point of the DSP 
** code. This symbol will be resolved when this particular file is linked 
** with the partially linked DSP binary.
*/
extern void TP0halResetEntry(void); 
EXPORT_SYMBOL(TP0halResetEntry);

/* Prototypes. */
static int __init dsp_init( void );
static void __exit dsp_cleanup( void );


#if defined(MODULE)
/***************************************************************************
 * Function Name: init_module
 * Description  : Initial function that is called if this driver is compiled
 *                as a module.  If it is not, endpoint_init is called in
 *                chr_dev_init() in drivers/char/mem.c.
 * Returns      : None.
 ***************************************************************************/
int init_module(void)
{
    return( dsp_init() );
}

/***************************************************************************
 * Function Name: cleanup_module
 * Description  : Final function that is called if this driver is compiled
 *                as a module.
 * Returns      : None.
 ***************************************************************************/
void cleanup_module(void)
{
    dsp_cleanup();
}
#endif //MODULE


/***************************************************************************
 * Function Name: dsp_init
 * Description  : Initial function that is called at system startup. This 
 *                is only a stub since we do not have much to initialize.
 * Returns      : None.
 ***************************************************************************/
static int __init dsp_init( void )
{
   printk( "DSP Driver: DSP init stub\n" );
   return( 0 );
} /* endpoint_init */


/***************************************************************************
 * Function Name: dsp_cleanup
 * Description  : Final function that is called when the module is unloaded. 
 *                This is only a stub since we do not have much to cleanup.
 * Returns      : None.
 ***************************************************************************/
static void __exit dsp_cleanup( void )
{
    printk( "DSP Driver: DSP cleanup stub\n" );
} /* dsp_cleanup */

#if !defined(MODULE)
/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( dsp_init );
module_exit( dsp_cleanup );
#endif