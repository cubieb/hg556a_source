/*
<:copyright-gpl
 Copyright 2002 Broadcom Corp. All Rights Reserved.

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
/*
 ***************************************************************************
 * File Name  : bcm63xx_flash.c
 *
 * Description: This file contains the flash device driver APIs for bcm63xx board. 
 *
 * Created on :  8/10/2002  seanl:  use cfiflash.c, cfliflash.h (AMD specific)
 *1、增加FLASH VA分区        by l39225 2006-05-08
   2、增加PSI的清空接口by 139225 2006-05-08
 ***************************************************************************/


/* Includes. */
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <asm/uaccess.h>

#include <bcm_map_part.h>
#include <board.h>
#define  BCMTAG_EXE_USE
#include <bcmTag.h>
#include "flash_api.h"
#include "boardparms.h"

//#define DEBUG_FLASH

extern PFILE_TAG kerSysImageTagGet(void);

static FLASH_ADDR_INFO fInfo;
static int flashInitialized = 0;

void *retriedKmalloc(size_t size)
{
    void *pBuf;
    int tryCount = 0;


    // try 1000 times before quit
    while (((pBuf = kmalloc(size, GFP_KERNEL)) == NULL) && (tryCount++ < 1000))
    {
		current->state   = TASK_INTERRUPTIBLE;
		schedule_timeout(HZ/10);
	}
    if (tryCount >= 1000)
        pBuf = NULL;
    else
	    memset(pBuf, 0, size);

    return pBuf;
}

void retriedKfree(void *pBuf)
{
	kfree(pBuf);
}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc) 
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

// get the nvram start addr
//
unsigned long get_nvram_start_addr(void)
{
    return ((unsigned long) 
        (flash_get_memptr(fInfo.flash_nvram_start_blk) + fInfo.flash_nvram_blk_offset));
}

// get the scratch_pad start addr
//
unsigned long get_scratch_pad_start_addr(void)
{
    return ((unsigned long) 
        (flash_get_memptr(fInfo.flash_scratch_pad_start_blk) + fInfo.flash_scratch_pad_blk_offset));
}


/**************************************************************************
 * 函数名  :   GetBoardVersion
 * 功能    :   GPIO获取Board版本号
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    正确  :  Board版本号 
 *              错误  :  NOT_SUPPROT_GET_VERSION
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-07-07创建  
 * HG556 修改获取版本号为GPIO5 GPIO4
 *  0        0          --> HG55VDFA
 *  1        1          --> HG56BZRB
 *
 *
 ***************************************************************************/
unsigned char GetBoardVersion(void)
{        
    unsigned char ucTmpVersion = 0;

    ucTmpVersion = kerSysGetGPIO(BP_GPIO_5_AL);
    ucTmpVersion = ((ucTmpVersion << 1) | kerSysGetGPIO(BP_GPIO_4_AL));

    return ucTmpVersion;        
}

/**************************************************************************
 * 函数名  :   GetHarewareType
 * 功能    :  
 * 输入参数:   无           
 * 输出参数:   无 
 *             
 * 返回值  : 
 *
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-07-10创建      
 ***************************************************************************/
unsigned char GetHarewareType(void)
{      
    return GetBoardVersion();        
}


// Initialize the flash and fill out the fInfo structure
void kerSysFlashInit( void )
{
    int i = 0;
    int totalBlks = 0;
    int totalSize = 0;
    int startAddr = 0;
    int usedBlkSize = 0;
    int usedVaSize  = 0;
    int usedVaBlkSize = 0;
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    int usedPsiBakBlkSize = 0;
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
    NVRAM_DATA nvramData;
    UINT32 crc = CRC32_INIT_VALUE, savedCrc;
    PFILE_TAG pTag = NULL;
    unsigned long kernelEndAddr = 0;
    unsigned long spAddr = 0;
    unsigned char ucHardwareType = 0;

    if (flashInitialized)
        return;

    flashInitialized = 1;
    flash_init();

    totalBlks = flash_get_numsectors();
    totalSize = flash_get_total_size();

    printk("Total Flash size: %dK with %d sectors\n", totalSize/1024, totalBlks);

    /* nvram is always at the end of flash */
    fInfo.flash_nvram_length = NVRAM_LENGTH;
    fInfo.flash_nvram_start_blk = 0;  /* always the first block */
    fInfo.flash_nvram_number_blk = 1; /*always fits in the first block */
    fInfo.flash_nvram_blk_offset = NVRAM_DATA_OFFSET;
 
    // check nvram CRC
    memcpy((char *)&nvramData, (char *)get_nvram_start_addr(), sizeof(NVRAM_DATA));
    savedCrc = nvramData.ulCheckSum;
    nvramData.ulCheckSum = 0;
    crc = getCrc32((char *)&nvramData, (UINT32) sizeof(NVRAM_DATA), crc);   

    ucHardwareType = GetHarewareType();
    printk("\nBoard id is set %s, ucHardwareType %d\n", nvramData.szBoardId, ucHardwareType);
    //BpSetBoardId( nvramData.szBoardId );   
    BpSetBoardIdByHardwareType(ucHardwareType);

    fInfo.flash_persistent_length = NVRAM_PSI_DEFAULT;
    if (savedCrc != crc)
    {
        printk("***Board is not initialized****: Using the default PSI size: %d\n",
            fInfo.flash_persistent_length);
    }
    else
    {
        unsigned long ulPsiSize;
        if( BpGetPsiSize( &ulPsiSize ) == BP_SUCCESS )
            fInfo.flash_persistent_length = ulPsiSize;
        else
        {
            printk("***Board id is not set****: Using the default PSI size: %d\n",
                fInfo.flash_persistent_length);
        }
    }

    fInfo.flash_persistent_length *= ONEK;
    startAddr = totalSize - fInfo.flash_persistent_length;
    fInfo.flash_persistent_start_blk = flash_get_blk(startAddr+FLASH_BASE);
    fInfo.flash_persistent_number_blk = totalBlks - fInfo.flash_persistent_start_blk;
    // find out the offset in the start_blk
    usedBlkSize = 0;
    for (i = fInfo.flash_persistent_start_blk; 
        i < (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk); i++)
    {
        usedBlkSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_persistent_blk_offset =  usedBlkSize - fInfo.flash_persistent_length;

    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    fInfo.flash_psi_backup_length = fInfo.flash_persistent_length;
    startAddr = totalSize - usedBlkSize - fInfo.flash_psi_backup_length;
    fInfo.flash_psi_backup_start_blk = flash_get_blk(startAddr + FLASH_BASE);
    fInfo.flash_psi_backup_number_blk = fInfo.flash_persistent_start_blk\
        - fInfo.flash_psi_backup_start_blk;
    for (i = fInfo.flash_psi_backup_start_blk;
        i < (fInfo.flash_psi_backup_start_blk + fInfo.flash_psi_backup_number_blk); i++)
    {
        usedPsiBakBlkSize += flash_get_sector_size((unsigned short)i);
    }
    fInfo.flash_psi_backup_blk_offset = usedPsiBakBlkSize - fInfo.flash_psi_backup_length;
    usedBlkSize += usedPsiBakBlkSize;
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

    /*start l65130 20080328 增加2个分区 Fix Avail, 将原有 VA 及 SP 分别使用1 sector */

    /*FIX*/
    fInfo.flash_fix_length = FIX_MAX_LEN;
    startAddr = totalSize - usedBlkSize - fInfo.flash_fix_length;
    fInfo.flash_fix_start_blk = flash_get_blk(startAddr + FLASH_BASE);
    fInfo.flash_fix_number_blk = fInfo.flash_psi_backup_start_blk \
        - fInfo.flash_fix_start_blk;

    usedPsiBakBlkSize = 0;
    for (i = fInfo.flash_fix_start_blk;
        i < (fInfo.flash_fix_start_blk + fInfo.flash_fix_number_blk); i++)
    {
        usedPsiBakBlkSize += flash_get_sector_size((unsigned short)i);
    }
    fInfo.flash_fix_blk_offset = usedPsiBakBlkSize - fInfo.flash_fix_length;
    usedBlkSize += usedPsiBakBlkSize;


    /*AVAIL*/
    fInfo.flash_avail_length = AVAIL_MAX_LEN;
    startAddr = totalSize - usedBlkSize - fInfo.flash_avail_length;
    fInfo.flash_avail_start_blk = flash_get_blk(startAddr + FLASH_BASE);
    fInfo.flash_avail_number_blk = fInfo.flash_fix_start_blk \
        - fInfo.flash_avail_start_blk;

    usedPsiBakBlkSize = 0;
    for (i = fInfo.flash_avail_start_blk;
        i < (fInfo.flash_avail_start_blk + fInfo.flash_avail_number_blk); i++)
    {
        usedPsiBakBlkSize += flash_get_sector_size((unsigned short)i);
    }
    fInfo.flash_avail_blk_offset = usedPsiBakBlkSize - fInfo.flash_avail_length;
    usedBlkSize += usedPsiBakBlkSize;

    /*VA*/
    fInfo.flash_variable_length = VA_MAX_LEN;
    startAddr = totalSize -  usedBlkSize - fInfo.flash_variable_length;
    fInfo.flash_variable_start_blk = flash_get_blk(startAddr+FLASH_BASE);
    fInfo.flash_variable_number_blk = fInfo.flash_avail_start_blk\
        - fInfo.flash_variable_start_blk;
     for ( i = fInfo.flash_variable_start_blk; 
        i < (fInfo.flash_variable_start_blk +  fInfo.flash_variable_number_blk); i++)
    {
         usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_variable_blk_offset =  usedVaSize - fInfo.flash_variable_length;
    usedBlkSize += usedVaSize;
    
    /*SP*/
    /*SP shared kernel sectors wouldn't be considered. Make sure Flash sectors are 
      enough for the kernel.*/
    fInfo.flash_scratch_pad_length = SP_MAX_LEN;
    startAddr = totalSize -  usedBlkSize - fInfo.flash_scratch_pad_length;
    fInfo.flash_scratch_pad_start_blk = flash_get_blk(startAddr+FLASH_BASE);
    fInfo.flash_scratch_pad_number_blk = fInfo.flash_variable_start_blk\
        - fInfo.flash_scratch_pad_start_blk;
    usedVaSize = 0;
    for ( i = fInfo.flash_scratch_pad_start_blk;
        i < (fInfo.flash_scratch_pad_start_blk +  fInfo.flash_scratch_pad_number_blk); i++)
    {
         usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_scratch_pad_blk_offset = usedVaSize - fInfo.flash_scratch_pad_length;
    usedBlkSize += usedVaSize;
    
    /*FIREWALL_LOG*/
    /*start of 增加防火墙日志保存接口 by c00131380 at 080911*/
    fInfo.flash_firewalllog_length = FIREWALL_LOG_MAX_LEN;
    startAddr = totalSize - usedBlkSize - fInfo.flash_firewalllog_length;
    fInfo.flash_firewalllog_start_blk  = flash_get_blk(startAddr+FLASH_BASE);
    fInfo.flash_firewalllog_number_blk = fInfo.flash_scratch_pad_start_blk\
        - fInfo.flash_firewalllog_start_blk;
    usedVaSize = 0; //借用临时统计变量
    for (i = fInfo.flash_firewalllog_start_blk;
        i < (fInfo.flash_firewalllog_start_blk + fInfo.flash_firewalllog_number_blk); i++)
    {
        usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_firewalllog_blk_offset = usedVaSize - fInfo.flash_firewalllog_length;         
    usedBlkSize += usedVaSize;
    /*end of 增加防火墙日志保存分区 by c00131380 at 080911*/

    /*SYSTEM_LOG*/
    /*start of 增加系统日志保存分区 by c00131380 at 090926*/
    fInfo.flash_systemlog_length = SYSTEM_LOG_MAX_LEN;
    startAddr = totalSize - usedBlkSize - fInfo.flash_systemlog_length;
    fInfo.flash_systemlog_start_blk  = flash_get_blk(startAddr + FLASH_BASE);
    fInfo.flash_systemlog_number_blk = fInfo.flash_firewalllog_start_blk\
        - fInfo.flash_systemlog_start_blk;
    usedVaSize = 0;//借用临时统计变量
    for (i = fInfo.flash_systemlog_start_blk;
         i < (fInfo.flash_systemlog_start_blk + fInfo.flash_systemlog_number_blk); i++)
    {
        usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_systemlog_blk_offset = usedVaSize - fInfo.flash_systemlog_length;
    usedBlkSize += usedVaSize;
    /*end of 增加系统日志保存分区 by c00131380 at 090926*/

    /*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
    /* WLAN PARAM, 因为涉及已经生产的单板, 因此必须与生产时的分区以前保持一致 */
    if (HG55VDFA == GetHarewareType())
    {
        fInfo.flash_wlanparam_length = ATHEROS_WLAN_PARAM_MAX_LEN;
    }
    else
    {
        fInfo.flash_wlanparam_length = RALINK_WLAN_PARAM_MAX_LEN;
    }
    startAddr = totalSize - usedBlkSize - fInfo.flash_wlanparam_length;
    fInfo.flash_wlanparam_start_blk  = flash_get_blk(startAddr + FLASH_BASE);
    fInfo.flash_wlanparam_number_blk = fInfo.flash_systemlog_start_blk\
        - fInfo.flash_wlanparam_start_blk;
    usedVaSize = 0;//借用临时统计变量
    for (i = fInfo.flash_wlanparam_start_blk;
         i < (fInfo.flash_wlanparam_start_blk + fInfo.flash_wlanparam_number_blk); i++)
    {
        usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_wlanparam_blk_offset = usedVaSize - fInfo.flash_wlanparam_length;
    usedBlkSize += usedVaSize;
    /*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/

#if 0

    /*start of 增加FLASH VA的分区 porting by l39225 20060504*/
    fInfo.flash_variable_length = VA_MAX_LEN;
    startAddr = totalSize -  usedBlkSize - VA_MAX_LEN;
    fInfo.flash_variable_start_blk = flash_get_blk(startAddr+FLASH_BASE);
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    fInfo.flash_variable_number_blk = fInfo.flash_psi_backup_start_blk\
        - fInfo.flash_variable_start_blk;
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
     for ( i = fInfo.flash_variable_start_blk; 
        i < (fInfo.flash_variable_start_blk +  fInfo.flash_variable_number_blk); i++)
    {
         usedVaSize += flash_get_sector_size((unsigned short) i);
    }
    fInfo.flash_variable_blk_offset =  usedVaSize - fInfo.flash_variable_length;
    // save abs SP address (Scratch Pad). it is before PSI 
    /*start of 把VA与SP放在一个区间，不太安全，但为了省FLASH，忍了2007-2-3*/
    //spAddr =  totalSize -  usedBlkSize - usedVaSize - SP_MAX_LEN;
    spAddr =  totalSize -  usedBlkSize - VA_MAX_LEN - SP_MAX_LEN;
    /*start of 把VA与SP放在一个区间，不太安全，但为了省FLASH，忍了2007-2-3*/
    /*end of 增加FLASH VA的分区 porting by l39225 20060504*/
 
    // get the info for sp
    if (!(pTag = kerSysImageTagGet()))
    {
        printk("Failed to read image tag from flash\n");
        return;
    }
    kernelEndAddr = (unsigned long)simple_strtoul(pTag->kernelAddress,NULL,10)+ \
        (unsigned long) simple_strtoul(pTag->kernelLen, NULL, 10) + BOOT_OFFSET;

    // make suer sp does not share kernel block
    fInfo.flash_scratch_pad_start_blk = flash_get_blk(spAddr+FLASH_BASE);
    if (fInfo.flash_scratch_pad_start_blk != flash_get_blk(kernelEndAddr))
    {
        fInfo.flash_scratch_pad_length = SP_MAX_LEN;
        //这种情况不会发生
        if (fInfo.flash_variable_start_blk == fInfo.flash_scratch_pad_start_blk)  // share blk
        {
#if 0 /* do not used scratch pad unless it's in its own sector */
            printk("Scratch pad is not used for this flash part.\n");  
            fInfo.flash_scratch_pad_length = 0;     // no sp
#else /* allow scratch pad to share a sector with another section such as PSI */
            fInfo.flash_scratch_pad_number_blk = 1;
            fInfo.flash_scratch_pad_blk_offset = fInfo.flash_variable_blk_offset - fInfo.flash_scratch_pad_length;
#endif
        }
        else // on different blk
        {
            fInfo.flash_scratch_pad_number_blk = fInfo.flash_variable_start_blk\
                - fInfo.flash_scratch_pad_start_blk;
            // find out the offset in the start_blk
            usedBlkSize = 0;
            for (i = fInfo.flash_scratch_pad_start_blk; 
                i < (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk); i++)
                usedBlkSize += flash_get_sector_size((unsigned short) i);
                fInfo.flash_scratch_pad_blk_offset =  usedBlkSize - fInfo.flash_scratch_pad_length;
        }
    }
    else
    {
        printk("No flash for scratch pad!\n");  
        fInfo.flash_scratch_pad_length = 0;     // no sp
    }

#endif


#if defined(DEBUG_FLASH)
    printk("fInfo.flash_nvram_start_blk = %d\n", fInfo.flash_nvram_start_blk);
    printk("fInfo.flash_nvram_number_blk = %d\n", fInfo.flash_nvram_number_blk);
    printk("fInfo.flash_nvram_length = 0x%x\n", (unsigned int)fInfo.flash_nvram_length);
    printk("fInfo.flash_nvram_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_nvram_blk_offset);

    printk("fInfo.flash_persistent_start_blk = %d\n", fInfo.flash_persistent_start_blk);
    printk("fInfo.flash_persistent_number_blk = %d\n", fInfo.flash_persistent_number_blk);
    printk("fInfo.flash_persistent_length = 0x%x\n", fInfo.flash_persistent_length);
    printk("fInfo.flash_persistent_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_persistent_blk_offset);

    printk("fInfo.flash_psi_backup_start_blk = %d\n", fInfo.flash_psi_backup_start_blk);
    printk("fInfo.flash_psi_backup_number_blk = %d\n", fInfo.flash_psi_backup_number_blk);
    printk("fInfo.flash_psi_backup_length = 0x%x\n", fInfo.flash_psi_backup_length);
    printk("fInfo.flash_psi_backup_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_psi_backup_blk_offset);

    printk("fInfo.flash_fix_start_blk = %d\n", fInfo.flash_fix_start_blk);
    printk("fInfo.flash_fix_number_blk = %d\n", fInfo.flash_fix_number_blk);
    printk("fInfo.flash_fix_length = 0x%x\n", fInfo.flash_fix_length);
    printk("fInfo.flash_fix_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_fix_blk_offset);

    printk("fInfo.flash_avail_start_blk = %d\n", fInfo.flash_avail_start_blk);
    printk("fInfo.flash_avail_number_blk = %d\n", fInfo.flash_avail_number_blk);
    printk("fInfo.flash_avail_length = 0x%x\n", fInfo.flash_avail_length);
    printk("fInfo.flash_avail_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_avail_blk_offset);

    printk("fInfo.flash_variable_start_blk = %d\n", fInfo.flash_variable_start_blk);
    printk("fInfo.flash_variable_number_blk = %d\n", fInfo.flash_variable_number_blk);
    printk("fInfo.flash_variable_length = 0x%x\n", fInfo.flash_variable_length);
    printk("fInfo.flash_variable_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_variable_blk_offset);

    printk("fInfo.flash_scratch_pad_start_blk = %d\n", fInfo.flash_scratch_pad_start_blk);
    printk("fInfo.flash_scratch_pad_number_blk = %d\n", fInfo.flash_scratch_pad_number_blk);
    printk("fInfo.flash_scratch_pad_length = 0x%x\n", fInfo.flash_scratch_pad_length);
    printk("fInfo.flash_scratch_pad_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_scratch_pad_blk_offset);
#endif

    /*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
    printk("fInfo.flash_wlanparam_start_blk = %d\n", fInfo.flash_wlanparam_start_blk);
    printk("fInfo.flash_wlanparam_number_blk = %d\n", fInfo.flash_wlanparam_number_blk);
    printk("fInfo.flash_wlanparam_length = 0x%x\n", fInfo.flash_wlanparam_length);
    printk("fInfo.flash_wlanparam_blk_offset = 0x%x\n\n", (unsigned int)fInfo.flash_wlanparam_blk_offset);        
    /*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/

}



/***********************************************************************
 * Function Name: kerSysFlashAddrInfoGet
 * Description  : Fills in a structure with information about the NVRAM
 *                and persistent storage sections of flash memory.  
 *                Fro physmap.c to mount the fs vol.
 * Returns      : None.
 ***********************************************************************/
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info)
{
    pflash_addr_info->flash_nvram_blk_offset = fInfo.flash_nvram_blk_offset;
    pflash_addr_info->flash_nvram_length = fInfo.flash_nvram_length;
    pflash_addr_info->flash_nvram_number_blk = fInfo.flash_nvram_number_blk;
    pflash_addr_info->flash_nvram_start_blk = fInfo.flash_nvram_start_blk;
    pflash_addr_info->flash_persistent_blk_offset = fInfo.flash_persistent_blk_offset;
    pflash_addr_info->flash_persistent_length = fInfo.flash_persistent_length;
    pflash_addr_info->flash_persistent_number_blk = fInfo.flash_persistent_number_blk;
    pflash_addr_info->flash_persistent_start_blk = fInfo.flash_persistent_start_blk;
}


// get shared blks into *** pTempBuf *** which has to be released bye the caller!
// return: if pTempBuf != NULL, poits to the data with the dataSize of the buffer
// !NULL -- ok
// NULL  -- fail
static char *getSharedBlks(int start_blk, int end_blk)
{
    int i = 0;
    int usedBlkSize = 0;
    int sect_size = 0;
    char *pTempBuf = NULL;
    char *pBuf = NULL;

    for (i = start_blk; i < end_blk; i++)
        usedBlkSize += flash_get_sector_size((unsigned short) i);

#if defined(DEBUG_FLASH)
    printk("usedBlkSize = %d\n", usedBlkSize);
#endif

    if ((pTempBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
        printk("failed to allocate memory with size: %d\n", usedBlkSize);
        return pTempBuf;
    }
    
    pBuf = pTempBuf;
    for (i = start_blk; i < end_blk; i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);

#if defined(DEBUG_FLASH)
        printk("i = %d, sect_size = %d, end_blk = %d\n", i, sect_size, end_blk);
#endif
        flash_read_buf((unsigned short)i, 0, pBuf, sect_size);
        pBuf += sect_size;
    }
    
    return pTempBuf;
}



// Set the pTempBuf to flash from start_blk to end_blk
// return:
// 0 -- ok
// -1 -- fail
static int setSharedBlks(int start_blk, int end_blk, char *pTempBuf)
{
    int i = 0;
    int sect_size = 0;
    int sts = 0;
    char *pBuf = pTempBuf;

    for (i = start_blk; i < end_blk; i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);
        flash_sector_erase_int(i);
        if (flash_write_buf(i, 0, pBuf, sect_size) != sect_size)
        {
            printk("Error writing flash sector %d.", i);
            sts = -1;
            break;
        }
        pBuf += sect_size;
    }

    return sts;
}



/*******************************************************************************
 * NVRAM functions
 *******************************************************************************/

// get nvram data
// return:
//  0 - ok
//  -1 - fail
int kerSysNvRamGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (!flashInitialized)
        kerSysFlashInit();

    if (strLen > NVRAM_LENGTH)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_nvram_start_blk,
        (fInfo.flash_nvram_start_blk + fInfo.flash_nvram_number_blk))) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_nvram_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


// set nvram 
// return:
//  0 - ok
//  -1 - fail
int kerSysNvRamSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > NVRAM_LENGTH)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_nvram_start_blk,
        (fInfo.flash_nvram_start_blk + fInfo.flash_nvram_number_blk))) == NULL)
        return -1;

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_nvram_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_nvram_start_blk, 
        (fInfo.flash_nvram_number_blk + fInfo.flash_nvram_start_blk), pBuf) != 0)
        sts = -1;
    
    retriedKfree(pBuf);

    return sts;
}


/***********************************************************************
 * Function Name: kerSysEraseNvRam
 * Description  : Erase the NVRAM storage section of flash memory.
 * Returns      : 1 -- ok, 0 -- fail
 ***********************************************************************/
int kerSysEraseNvRam(void)
{
    int sts = 1;
    char *tempStorage = retriedKmalloc(NVRAM_LENGTH);
    
    // just write the whole buf with '0xff' to the flash
    if (!tempStorage)
        sts = 0;
    else
    {
        memset(tempStorage, 0xff, NVRAM_LENGTH);
        if (kerSysNvRamSet(tempStorage, NVRAM_LENGTH, 0) != 0)
            sts = 0;
        retriedKfree(tempStorage);
    }

    return sts;
}


/*******************************************************************************
 * PSI functions
 *******************************************************************************/
// get psi data
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_persistent_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk))) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_persistent_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


// set psi 
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_persistent_length)
        return -1;

#if 0        
    /* y42304 added: 判断FLASH的最后64是否由8个8K小块组成: 为了提供在web页面按save按钮时写FLASH的速度 */
    if (JudgeFlashIsTopDevice8K() == 1 )
    { 
        int iStartBlock = fInfo.flash_persistent_start_blk + (offset+fInfo.flash_persistent_blk_offset)%(8*ONEK);
        int iEndBlock = iStartBlock+ (strLen+8*ONEK-1)/(8*ONEK);      

        printk("iStartBlock %d iEndBlock %d \r\n", iStartBlock, iEndBlock);
	  
        if ((pBuf = getSharedBlks(iStartBlock, iEndBlock)) == NULL)           
        {
            return -1;
        }

        // set string to the memory buffer
        memcpy((pBuf + fInfo.flash_persistent_blk_offset + offset), string, strLen);

        if (setSharedBlks(iStartBlock, iEndBlock, pBuf) == NULL)
        {
            sts = -1;
        }        
    }
    else
    {
#endif
        if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
            (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk))) == NULL)
        {
            return -1;
        }

        // set string to the memory buffer
        memcpy((pBuf + fInfo.flash_persistent_blk_offset + offset), string, strLen);

        if (setSharedBlks(fInfo.flash_persistent_start_blk, 
           (fInfo.flash_persistent_number_blk + fInfo.flash_persistent_start_blk), pBuf) != 0)
        {
            sts = -1;
        }
//    }
 
    retriedKfree(pBuf);

    return sts;
}

 /*start of  增加PSI 清空接口by l39225 20060504*/      
 /*******************************************************************************
  Function:        kerSysPersistentClear
  Description:    清除PSI内容
  Input:            无
  Output:          无
  Return:          0:成功
                       -1:失败
  Others:           无
 *******************************************************************************/
int kerSysPersistentClear(void)
{
    int iSts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;

    if (0 == fInfo.flash_persistent_length )
    {
        return iSts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        (fInfo.flash_persistent_start_blk + fInfo.flash_persistent_number_blk))) == NULL)
    {
        return iSts;
    }

    pBuf = pShareBuf + fInfo.flash_persistent_blk_offset;  
    memset(pBuf, 0x00,  fInfo.flash_persistent_length);

    /* begin --- modified by w69233: Fixed bug erase too much data when shared one sector */
    iSts = setSharedBlks(fInfo.flash_persistent_start_blk, 
        (fInfo.flash_persistent_number_blk + fInfo.flash_persistent_start_blk), pShareBuf); 
    
    retriedKfree(pShareBuf);
    /* end ----- modified by w69233: Fixed bug erase too much data when shared one sector */

    return iSts;
}
 /*end of  增加PSI 清空接口by l39225 20060504*/   

/* begin --- Add Persistent storage interface backup flash partition by w69233 */
/*******************************************************************************
 * PSI Backup functions
 *******************************************************************************/
// get psi backup data
// return:
//  0 - ok
//  -1 - fail
int kerSysPsiBackupGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_psi_backup_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_psi_backup_start_blk,
        (fInfo.flash_psi_backup_start_blk + fInfo.flash_psi_backup_number_blk))) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_psi_backup_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


// set psi backup data
// return:
//  0 - ok
//  -1 - fail
int kerSysPsiBackupSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_psi_backup_length)
        return -1;
#if 0        
    /* y42304 added: 判断FLASH的最后64是否由8个8K小块组成: 为了提供在web页面按save按钮时写FLASH的速度 */
    if (JudgeFlashIsTopDevice8K() == 1 )
    { 
        int iStartBlock = fInfo.flash_psi_backup_start_blk + (offset+fInfo.flash_psi_backup_blk_offset)%(8*ONEK);
        int iEndBlock = iStartBlock+ (strLen+8*ONEK-1)/(8*ONEK);      
      
        if ((pBuf = getSharedBlks(iStartBlock, iEndBlock)) == NULL)           
        {
            return -1;
        }

        // set string to the memory buffer
        memcpy((pBuf + fInfo.flash_psi_backup_blk_offset + offset), string, strLen);

        if (setSharedBlks(iStartBlock, iEndBlock, pBuf) == NULL)
        {
            sts = -1;
        }        
    }
    else
    {
#endif
        if ((pBuf = getSharedBlks(fInfo.flash_psi_backup_start_blk,
            (fInfo.flash_psi_backup_start_blk + fInfo.flash_psi_backup_number_blk))) == NULL)
        {
            return -1;
        }

        // set string to the memory buffer
        memcpy((pBuf + fInfo.flash_psi_backup_blk_offset + offset), string, strLen);

        if (setSharedBlks(fInfo.flash_psi_backup_start_blk, 
           (fInfo.flash_psi_backup_number_blk + fInfo.flash_psi_backup_start_blk), pBuf) != 0)
        {
            sts = -1;
        }
//    }
 
    retriedKfree(pBuf);

    return sts;
}


// clear psi backup data
// return:
//  0 - ok
//  -1 - failint kerSysPsiBackupClear(void)
int kerSysPsiBackupClear(void)
{
    int iSts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;

    if (0 == fInfo.flash_psi_backup_length )
    {
        return iSts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_psi_backup_start_blk,
        (fInfo.flash_psi_backup_start_blk + fInfo.flash_psi_backup_number_blk))) == NULL)
    {
        return iSts;
    }

    pBuf = pShareBuf + fInfo.flash_psi_backup_blk_offset;  
    memset(pBuf, 0x00,  fInfo.flash_psi_backup_length);

    /* begin --- modified by w69233: Fixed bug erase too much data when shared one sector */
    iSts = setSharedBlks(fInfo.flash_psi_backup_start_blk, 
        (fInfo.flash_psi_backup_number_blk + fInfo.flash_psi_backup_start_blk), pShareBuf); 
    
    retriedKfree(pShareBuf);
    /* end ----- modified by w69233: Fixed bug erase too much data when shared one sector */


    return iSts;
} 
/*end ----- Add Persistent storage interface backup flash partition by w69233 */

 
 /*start of  增加FLASH VA分区porting by l39225 20060504*/                
/*******************************************************************************
  Function:        kerSysVariableGet
  Description:    get va data
  Input:            string  :存放数据的内存区的指针           
                       strLen: 所取数据的长度，以BYTE为单位
                       offset:  基址的偏移量
  Output:          无
  Return:          0:成功
                       -1:失败
  Others:          无
 *******************************************************************************/

int kerSysVariableGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_variable_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_variable_start_blk,
        (fInfo.flash_variable_start_blk + fInfo.flash_variable_number_blk))) == NULL)
    {
        return -1;
    }

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_variable_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

EXPORT_SYMBOL(kerSysVariableGet);

/*******************************************************************************
  Function:        kerSysVariableSet
  Description:    set va data
  Input:            string  :存放数据的内存区的指针           
                       strLen: 所取数据的长度，以BYTE为单位
                       offset:  基址的偏移量
  Output:          无
  Return:          0:成功
                       -1:失败
  Others:          无
 *******************************************************************************/
int kerSysVariableSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_variable_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_variable_start_blk,
        (fInfo.flash_variable_start_blk + fInfo.flash_variable_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_variable_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_variable_start_blk, 
        (fInfo.flash_variable_number_blk + fInfo.flash_variable_start_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;
}
EXPORT_SYMBOL(kerSysVariableSet);
/*end of  增加FLASH VA分区porting by l39225 20060504*/ 

// flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
int kerSysBcmImageSet( int flash_start_addr, char *string, int size)
{
    int sts;
    int sect_size;
    int blk_start;
    int i;
    char *pTempBuf = NULL;
    int whole_image = 0;

    blk_start = flash_get_blk(flash_start_addr);
    if( blk_start < 0 )
        return( -1 );

    if (flash_start_addr == FLASH_BASE && size > FLASH_LENGTH_BOOT_ROM)
        whole_image = 1;

   /* write image to flash memory */
    do 
    {
        sect_size = flash_get_sector_size(blk_start);
// NOTE: for memory problem in multiple PVC configuration, temporary get rid of kmalloc this 64K for now.
//        if ((pTempBuf = (char *)retriedKmalloc(sect_size)) == NULL)
//        {
//            printk("Failed to allocate memory with size: %d.  Reset the router...\n", sect_size);
//            kerSysMipsSoftReset();     // reset the board right away.
//        }
        // for whole image, no check on psi
         /*start of  增加FLASH VA分区porting by l39225 20060504*/      
        if (!whole_image && blk_start == fInfo.flash_variable_start_blk)  // share the blk with psi
        {
            if (size > (sect_size - fInfo.flash_variable_length))
            {
        /*end of  增加FLASH VA分区porting by l39225 20060504*/      
                printk("Image is too big\n");
                break;          // image is too big. Can not overwrite to nvram
            }
            if ((pTempBuf = (char *)retriedKmalloc(sect_size)) == NULL)
            {
               printk("Failed to allocate memory with size: %d.  Reset the router...\n", sect_size);
               kerSysMipsSoftReset();     // reset the board right away.
            }
            flash_read_buf((unsigned short)blk_start, 0, pTempBuf, sect_size);
            if (copy_from_user((void *)pTempBuf,(void *)string, size) != 0)
            {
                break;  // failed ?
            }
            flash_sector_erase_int(blk_start);     // erase blk before flash
            if (flash_write_buf(blk_start, 0, pTempBuf, sect_size) == sect_size) 
            	{
                size = 0;   // break out and say all is ok
            	}
	      retriedKfree(pTempBuf);
            break;
        }
#if 1//for vdf-GL l65130 2008.09.12 
/* BEGIN: Modified by c106292, 2009/3/13   PN:Led light method changes*/
  //      kerSysLedCtrl(kLedAdsl, kLedStateOn);
/* END:   Modified by c106292, 2009/3/13 */
#endif
        
        flash_sector_erase_int(blk_start);     // erase blk before flash

        if (sect_size > size) 
        {
            if (size & 1) 
                size++;
            sect_size = size;
        }
#if 1//for vdf-GL l65130 2008.09.12 
/* BEGIN: Modified by c106292, 2009/3/13   PN:Led light method changes*/
//        kerSysLedCtrl(kLedAdsl, kLedStateOff);
/* END:   Modified by c106292, 2009/3/13 */
#endif

        if ((i = flash_write_buf(blk_start, 0, string, sect_size)) != sect_size) {
            break;
        }
        blk_start++;
        string += sect_size;
        size -= sect_size; 
    } while (size > 0);
/* BEGIN: Modified by c106292, 2009/3/13   PN:Led light method changes*/
#if 1//for vdf-GL l65130 2008.09.12 
    /* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
 //   kerSysLedCtrl(kLedAdsl, kLedStateOff);
    /* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/
#endif
/* END:   Modified by c106292, 2009/3/13 */
    if (whole_image)  
    {
        // If flashing a whole image, erase to end of flash.
        int total_blks = flash_get_numsectors();
        while( blk_start < total_blks )
        {
            flash_sector_erase_int(blk_start);
            blk_start++;
        }
    }
    if (pTempBuf)
        retriedKfree(pTempBuf);

    if( size == 0 ) 
        sts = 0;  // ok
    else  
        sts = blk_start;    // failed to flash this sector

    return sts;
}

/*******************************************************************************
 * SP functions
 *******************************************************************************/
// get sp data.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  >0 - number of bytes copied to tokBuf
//  -1 - fail
int kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int sts = -1;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( bufLen >= (fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN)) ) 
    {
        printk("Exceed scratch pad space by %d\n", bufLen -
            fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
            sizeof(SP_TOKEN));
        return sts;
    }

    if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk +
        fInfo.flash_scratch_pad_number_blk))) == NULL )
    {
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scratch pad is not initialized.\n");
        /* start of board y42304 modified 20060720: 解决D012与D020版本切换进入shell的问题*/
        kerSysScratchPadClearAll();
        /* end of board y42304 modified 20060720: 解决D012与D020版本切换进入shell的问题*/
        return sts;
    }

    // search for the token
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;
    while( pToken->tokenName[0] != '\0' && pToken->tokenLen > 0 &&
        pToken->tokenLen < fInfo.flash_scratch_pad_length &&
        usedLen < fInfo.flash_scratch_pad_length )
    {

        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            sts = pToken->tokenLen;
            memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), sts);
            break;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    retriedKfree(pShareBuf);

    return sts;
}


// set sp.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    int sts = -1;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( bufLen >= fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN) )
    {
        printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
            sizeof(SP_TOKEN));
        return sts;
    }

    if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk +
        fInfo.flash_scratch_pad_number_blk) )) == NULL )
    {
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, TOKEN_NAME_LEN - 1);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        printk("No scratch pad found.  Initialize scratch pad...\n");
        /* start of board y42304 modified 20060720: 解决D012与D020版本切换进入shell的问题*/
        kerSysScratchPadClearAll();
        /* end of board y42304 modified 20060720: 解决D012与D020版本切换进入shell的问题*/
        memcpy(pBuf, (char *)&SPHead, sizeof(SP_HEADER));
        curPtr = pBuf + sizeof(SP_HEADER);
        memcpy(curPtr, (char *)&SPToken, sizeof(SP_TOKEN));
        curPtr += sizeof(SP_TOKEN);
        if( tokBuf )
            memcpy(curPtr, tokBuf, bufLen);
    }
    else  
    {
        int putAtEnd = 1;
        int curLen;
        int usedLen;
        int skipLen;

        /* Calculate the used length. */
        usedLen = sizeof(SP_HEADER);
        curPtr = pBuf + sizeof(SP_HEADER);
        pToken = (PSP_TOKEN) curPtr;
        skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        while( pToken->tokenName[0] >= 'A' && pToken->tokenName[0] <= 'z' &&
            strlen(pToken->tokenName) < TOKEN_NAME_LEN &&
            pToken->tokenLen > 0 &&
            pToken->tokenLen < fInfo.flash_scratch_pad_length &&
            usedLen < fInfo.flash_scratch_pad_length )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            fInfo.flash_scratch_pad_length )
        {
            printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + sizeof(SP_TOKEN)) -
                fInfo.flash_scratch_pad_length);
            return sts;
        }

        curPtr = pBuf + sizeof(SP_HEADER);
        curLen = sizeof(SP_HEADER);
        while( curLen < usedLen )
        {
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
            if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
            {
                // The token id already exists.
                if( tokBuf && pToken->tokenLen == bufLen )
                {
                    // The length of the new data and the existing data is the
                    // same.  Overwrite the existing data.
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);
                    putAtEnd = 0;
                }
                else
                {
                    // The length of the new data and the existing data is
                    // different.  Shift the rest of the scratch pad to this
                    // token's location and put this token's data at the end.
                    char *nextPtr = curPtr + sizeof(SP_TOKEN) + skipLen;
                    int copyLen = usedLen - (curLen+sizeof(SP_TOKEN) + skipLen);
                    memcpy( curPtr, nextPtr, copyLen );
                    memset( curPtr + copyLen, 0x00, 
                        fInfo.flash_scratch_pad_length - (curLen + copyLen) );
                    usedLen -= sizeof(SP_TOKEN) + skipLen;
                }
                break;
            }

            // get next token
            curPtr += sizeof(SP_TOKEN) + skipLen;
            curLen += sizeof(SP_TOKEN) + skipLen;
        } // end while

        if( putAtEnd )
        {
            if( tokBuf )
            {
                memcpy( pBuf + usedLen, &SPToken, sizeof(SP_TOKEN) );
                memcpy( pBuf + usedLen + sizeof(SP_TOKEN), tokBuf, bufLen );
            }
            memcpy( pBuf, &SPHead, sizeof(SP_HEADER) );
        }

    } // else if not new sp

    sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
        (fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk),
        pShareBuf);
    
    retriedKfree(pShareBuf);

    return sts;

    
}

// wipe out the scratchPad
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadClearAll(void)
{ 
    int sts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
	

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk) )) == NULL )
        return sts;
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
    memset(pBuf, 0x00,  fInfo.flash_scratch_pad_length);

    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
    sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
		(fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk),  pBuf);
    */
        sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
            (fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk),  pShareBuf);
    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */

   retriedKfree(pShareBuf);
	
   return sts;
}

int kerSysFlashSizeGet(void)
{
   return flash_get_total_size();
}

int kerSysMemoryMappedFlashSizeGet(void)
{
    return( flash_get_total_memory_mapped_size() );
}

unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    int sect = flash_get_blk((int) fromaddr);
    unsigned char *start = flash_get_memptr(sect);
    flash_read_buf( sect, (int) fromaddr - (int) start, toaddr, len );

    return( len );
}


/*****************************************************************************
 函 数 名  : kerSysFixSet
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysFixSet(char *string, int strLen, int offset)
{
	
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_fix_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_fix_start_blk,
        (fInfo.flash_fix_start_blk + fInfo.flash_fix_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_fix_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_fix_start_blk, 
        (fInfo.flash_fix_start_blk + fInfo.flash_fix_number_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;
}


/*****************************************************************************
 函 数 名  : kerSysFixSet
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysAvailSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_avail_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_avail_start_blk,
        (fInfo.flash_avail_start_blk + fInfo.flash_avail_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_avail_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_avail_start_blk, 
        (fInfo.flash_avail_start_blk + fInfo.flash_avail_number_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;
}
EXPORT_SYMBOL(kerSysAvailSet);

/*****************************************************************************
 函 数 名  : kerSysFixGet
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysFixGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_fix_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_fix_start_blk,
        (fInfo.flash_fix_start_blk + fInfo.flash_fix_number_blk))) == NULL)
    {
        return -1;
    }

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_fix_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}


/*****************************************************************************
 函 数 名  : kerSysAvailGet
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysAvailGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_avail_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_avail_start_blk,
        (fInfo.flash_avail_start_blk + fInfo.flash_avail_number_blk))) == NULL)
    {
        return -1;
    }

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_avail_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}
EXPORT_SYMBOL(kerSysAvailGet);

/*****************************************************************************
 函 数 名  : kerSysFirewalllogGet
 功能描述  : 获得防火墙日志
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月11日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysFirewalllogGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_firewalllog_length)
    {
        return -1;
    }
 
    if ((pBuf = getSharedBlks(fInfo.flash_firewalllog_start_blk,
        (fInfo.flash_firewalllog_start_blk + fInfo.flash_firewalllog_number_blk))) == NULL)
    {
        return -1;
    }
    
    // get string off the memory buffer 一次读取10K数据返回
    memcpy(string, (pBuf + fInfo.flash_firewalllog_blk_offset), fInfo.flash_firewalllog_length);
    
    retriedKfree(pBuf);
    return 0;
}

/*****************************************************************************
 函 数 名  : kerSysFirewalllogSet
 功能描述  : 写防火墙日志到FLASH
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月11日
    作    者   : C00131380
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysFirewalllogSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_firewalllog_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_firewalllog_start_blk,
        (fInfo.flash_firewalllog_start_blk + fInfo.flash_firewalllog_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_firewalllog_blk_offset), string, fInfo.flash_firewalllog_length);

    if (setSharedBlks(fInfo.flash_firewalllog_start_blk, 
        (fInfo.flash_firewalllog_start_blk + fInfo.flash_firewalllog_number_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;
}

/*****************************************************************************
 函 数 名  : kerSysSystemLogGet
 功能描述  : 增加读取系统日志功能,大小16k
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月26日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysSystemLogGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if (strLen > fInfo.flash_systemlog_length)
    {
        return -1;
    }
 
    if ((pBuf = getSharedBlks(fInfo.flash_systemlog_start_blk,
        (fInfo.flash_systemlog_start_blk + fInfo.flash_systemlog_number_blk))) == NULL)
    {
        return -1;
    }
    
    // get string off the memory buffer 一次读取16K数据返回
    memcpy(string, (pBuf + fInfo.flash_systemlog_blk_offset), fInfo.flash_systemlog_length);
    
    retriedKfree(pBuf);
    return 0;

}

/*****************************************************************************
 函 数 名  : kerSysSystemLogSet
 功能描述  : 增加系统日志保存功能,大小16k
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月26日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysSystemLogSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_systemlog_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_systemlog_start_blk,
        (fInfo.flash_systemlog_start_blk + fInfo.flash_systemlog_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_systemlog_blk_offset), string, fInfo.flash_systemlog_length);

    if (setSharedBlks(fInfo.flash_systemlog_start_blk, 
        (fInfo.flash_systemlog_start_blk + fInfo.flash_systemlog_number_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;

}


/*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
/*****************************************************************************
 函 数 名  : kerSysWlanParamGet
 功能描述  :增加WLAN ART参数读取功能,大小?k
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年10月15日
    作    者   : l68693
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysWlanParamGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

   //printk("==strLen=%d, offset=%d, \n", strLen,offset);

    if (strLen > fInfo.flash_wlanparam_length)
    {
        return -1;
    }
 
    if ((pBuf = getSharedBlks(fInfo.flash_wlanparam_start_blk,
        (fInfo.flash_wlanparam_start_blk + fInfo.flash_wlanparam_number_blk))) == NULL)
    {
        return -1;
    }

    //printk("==flash_wlanparam_length=%d, flash_wlanparam_blk_offset=%d, \n", fInfo.flash_wlanparam_length,fInfo.flash_wlanparam_blk_offset);

    // get string off the memory buffer 
    memcpy(string, (pBuf + fInfo.flash_wlanparam_blk_offset), strLen);
    
    retriedKfree(pBuf);
    return 0;

}
EXPORT_SYMBOL(kerSysWlanParamGet);
/*****************************************************************************
 函 数 名  : kerSysWlanParamSet
 功能描述  : 增加WLAN ART参数保存功能,大小?k
 输入参数  : char *string  
             int strLen    
             int offset    
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年10月15日
    作    者   : l68693
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysWlanParamSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if (strLen > fInfo.flash_wlanparam_length)
    {
        return -1;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_wlanparam_start_blk,
        (fInfo.flash_wlanparam_start_blk + fInfo.flash_wlanparam_number_blk))) == NULL)
    {
        return -1;
    }

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_wlanparam_blk_offset), string, strLen);

    if (setSharedBlks(fInfo.flash_wlanparam_start_blk, 
        (fInfo.flash_wlanparam_start_blk + fInfo.flash_wlanparam_number_blk), pBuf) != 0)
    {
        sts = -1;
    }
    
    retriedKfree(pBuf);

    return sts;

}
EXPORT_SYMBOL(kerSysWlanParamSet);
/*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/



/*****************************************************************************
 函 数 名  : kerSysFixClear
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysFixClear(void)
{
    int iSts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;

    if (0 == fInfo.flash_fix_length )
    {
        return iSts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_fix_start_blk,
        (fInfo.flash_fix_start_blk + fInfo.flash_fix_number_blk))) == NULL)
    {
        return iSts;
    }

    pBuf = pShareBuf + fInfo.flash_fix_blk_offset;  
    memset(pBuf, 0x00,  fInfo.flash_fix_length);

    iSts = setSharedBlks(fInfo.flash_fix_start_blk, 
        (fInfo.flash_fix_number_blk + fInfo.flash_fix_start_blk), pShareBuf); 
    
    retriedKfree(pShareBuf);

    return iSts;
}  


/*****************************************************************************
 函 数 名  : kerSysAvailClear
 功能描述  : kerSysAvailClear
 输入参数  : void  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年3月28日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
int kerSysAvailClear(void)
{
    int iSts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;

    if (0 == fInfo.flash_avail_length )
    {
        return iSts;
    }

    if ((pShareBuf = getSharedBlks(fInfo.flash_avail_start_blk,
        (fInfo.flash_avail_start_blk + fInfo.flash_avail_number_blk))) == NULL)
    {
        return iSts;
    }

    pBuf = pShareBuf + fInfo.flash_avail_blk_offset;  
    memset(pBuf, 0x00,  fInfo.flash_avail_length);

    iSts = setSharedBlks(fInfo.flash_avail_start_blk, 
        (fInfo.flash_avail_number_blk + fInfo.flash_avail_start_blk), pShareBuf); 
    
    retriedKfree(pShareBuf);

    return iSts;
}

