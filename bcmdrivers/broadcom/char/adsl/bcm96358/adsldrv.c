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
/***************************************************************************
 * File Name  : AdslDrv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the ATM API driver.
 *
 * Updates    : 08/24/2001  lat.  Created.
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/if_arp.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>

#include <bcmtypes.h>
#include <adsldrv.h>
#include <bcmadsl.h>
#include <bcmatmapi.h>
#include <board.h>

#include <DiagDef.h>
#include "BcmOs.h"

#ifdef HMI_QA_SUPPORT
#include "BcmXdslHmi.h"
#endif

#ifndef DSL_IFNAME
#define	DSL_IFNAME "dsl0"
#endif

#if defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)
extern int atmbonding_enabled;
extern BCMADSL_STATUS BcmAdsl_SetGfc2VcMapping(int bOn) ;
#endif

/* typedefs. */
typedef void (*ADSL_FN_IOCTL) (unsigned long arg);

/* Externs. */
BCMADSL_STATUS BcmAdsl_SetVcEntry (int gfc, int port, int vpi, int vci);
BCMADSL_STATUS BcmAdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci, int pti);
void BcmAdsl_AtmSetPortId(int path, int portId);
void BcmAdsl_AtmClearVcTable(void);
void BcmAdsl_AtmAddVc(int vpi, int vci);
void BcmAdsl_AtmDeleteVc(int vpi, int vci);
void BcmAdsl_AtmSetMaxSdu(unsigned short maxsdu);

extern void *g_pfnAdslSetVcEntry;
extern void *g_pfnAdslSetAtmLoopbackMode;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
extern void *g_pfnAdslSetVcEntryEx;
#ifdef CONFIG_ATM_EOP_MONITORING
extern void *g_pfnAdslAtmClearVcTable;
extern void *g_pfnAdslAtmAddVc;
extern void *g_pfnAdslAtmDeleteVc;
extern void *g_pfnAdslAtmSetMaxSdu;
#endif
#ifdef BCM_ATM_BONDING
extern void *g_pfnAdslAtmbSetPortId;
#endif
#if defined(BCM_ATM_BONDING) || defined(BCM_ATM_BONDING_ETH)
extern void *g_pfnAdslAtmbGetConnInfo;
#endif
#endif


#ifdef VDF_SPAIN
extern int g_ulAdslOnLineStart;
extern int g_ulAdslCuts;

extern ADSL_LINK_STATE g_LinkState;

#endif


extern void BcmAdslCoreDiagWriteStatusString(char *fmt, ...);





/* Prototypes. */
static int __init adsl_init( void );
static void __exit adsl_cleanup( void );
static int adsl_open( struct inode *inode, struct file *filp );
static int adsl_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg );
static void DoCheck( unsigned long arg );
static void DoInitialize( unsigned long arg );
static void DoUninitialize( unsigned long arg );
static void DoConnectionStart( unsigned long arg );
static void DoConnectionStop( unsigned long arg );
static void DoGetPhyAddresses( unsigned long arg );
static void DoSetPhyAddresses( unsigned long arg );
static void DoMapATMPortIDs( unsigned long arg );
static void DoGetConnectionInfo( unsigned long arg );
static void DoDiagCommand( unsigned long arg );
static void DoGetObjValue( unsigned long arg );
static void DoSetObjValue( unsigned long arg );
static void DoStartBERT( unsigned long arg );
static void DoStopBERT( unsigned long arg );
static void DoConfigure( unsigned long arg );
static void DoTest( unsigned long arg );
static void DoGetConstelPoints( unsigned long arg );
static void DoGetVersion( unsigned long arg );
static void DoSetSdramBase( unsigned long arg );
static void DoResetStatCounters( unsigned long arg );
static void DoSetOemParameter( unsigned long arg );
static void DoBertStartEx( unsigned long arg );
static void DoBertStopEx( unsigned long arg );
static void AdslConnectCb( ADSL_LINK_STATE AdslLinkState, UINT32 ulParm );
#ifdef HMI_QA_SUPPORT
static void DoHmiCommand( unsigned long arg );
#endif

#ifdef VDF_SPAIN
static void DoGetCuts(unsigned long arg);
static void DoGetOnlineTime(unsigned long arg);
#endif



#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
static ssize_t adsl_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos);
static ssize_t adsl_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos);
static unsigned int adsl_poll(struct file *file, poll_table *wait);
extern void dumpaddr( unsigned char *pAddr, int nLen );
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */

typedef struct
{
  int readEventMask;    
  int writeEventMask;    
  char *buffer;
} ADSL_IO, *pADSL_IO;

/* Globals. */
static struct file_operations adsl_fops =
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    .ioctl      = adsl_ioctl,
    .open       = adsl_open,
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
    .read       = adsl_read,
    .write      = adsl_write,
    .poll       = adsl_poll,
#endif 
#else
    ioctl:      adsl_ioctl,
    open:       adsl_open,
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
    read:       adsl_read,
    write:      adsl_write,
    poll:       adsl_poll,
#endif 
#endif
};

static UINT16 g_usAtmFastPortId;
static UINT16 g_usAtmInterleavedPortId;
static struct net_device_stats g_dsl_if_stats;
static struct net_device *g_dslNetDev;
static int g_eoc_hdr_offset = 0;
static int g_eoc_hdr_len = 0;



#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#if defined(MODULE)
/***************************************************************************
 * Function Name: init_module
 * Description  : Initial function that is called if this driver is compiled
 *                as a module.  If it is not, adsl_init is called in
 *                chr_dev_init() in drivers/char/mem.c.
 * Returns      : None.
 ***************************************************************************/
int init_module(void)
{
    return( adsl_init() );              
}


/***************************************************************************
 * Function Name: cleanup_module
 * Description  : Final function that is called if this driver is compiled
 *                as a module.
 * Returns      : None.
 ***************************************************************************/
void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("adsl: cleanup_module failed because module is in use\n");
    else
        adsl_cleanup();
}
#endif //MODULE 
#endif //LINUX_VERSION_CODE

/***************************************************************************
 * Function Name: dsl_config_netdev
 * Description  : Configure a network device for DSL interface
 * Returns      : None.
 ***************************************************************************/
struct net_device_stats *dsl_get_stats(struct net_device *dev)
{
/*
        UINT32 interfaceId;
        ATM_INTERFACE_CFG Cfg;
        ATM_INTERFACE_STATS KStats;
        PATM_INTF_ATM_STATS pAtmStats = &KStats.AtmIntfStats;

        BcmAtm_GetInterfaceId(0, &interfaceId);
        Cfg.ulStructureId = ID_ATM_INTERFACE_CFG;
        KStats.ulStructureId = ID_ATM_INTERFACE_STATS;
        if ( BcmAtm_GetInterfaceCfg(interfaceId, &Cfg)==STS_SUCCESS &&
            BcmAtm_GetInterfaceStatistics(interfaceId, &KStats, 0)==STS_SUCCESS)
        {
            atm_if_stats.rx_bytes = (unsigned long) pAtmStats->ulIfInOctets;
            atm_if_stats.tx_bytes = (unsigned long) pAtmStats->ulIfOutOctets;
            atm_if_stats.rx_errors = (unsigned long) pAtmStats->ulIfInErrors;
            atm_if_stats.tx_errors = (unsigned long) pAtmStats->ulIfOutErrors;  
        }
*/
        return &g_dsl_if_stats;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static void dsl_setup(struct net_device *dev) {
        
    dev->type = ARPHRD_DSL;
    dev->mtu = 0;
    dev->get_stats = dsl_get_stats;        
}

static int dsl_config_netdev(void) {

    g_dslNetDev = alloc_netdev(0, DSL_IFNAME, dsl_setup);
    if ( g_dslNetDev )
         return register_netdev(g_dslNetDev);
    else    
         return -ENOMEM;
}
#else
static int dsl_config_netdev(void) {
    int status = 0;
    char name[16]="";

    sprintf(name, "dsl0");
    g_dslNetDev = dev_alloc(name,  &status);
    g_dslNetDev->type = ARPHRD_DSL;
    g_dslNetDev->mtu = 0;
    g_dslNetDev->get_stats = dsl_get_stats;
    register_netdev(g_dslNetDev);    
    return status;
}
#endif

/***************************************************************************
 * Function Name: adsl_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init adsl_init( void )
{
    printk( "adsl: adsl_init entry\n" );
    if ( register_chrdev( ADSLDRV_MAJOR, "adsl", &adsl_fops ) )
    {
        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting--VDF ADSL error led add begin:*/
        kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);    
        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting--VDF ADSL error led add end.*/
         printk("adsldd: failed to create adsl\n");
         return( -1 );
    }            
    if ( dsl_config_netdev() )
    {
        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting--VDF ADSL error led add begin:*/
        kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);    
        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting--VDF ADSL error led add end.*/
         printk("adsldd: failed to create %s\n", DSL_IFNAME);
         return( -1 );
    }
    else
         return( 0 );
} /* adsl_init */


/***************************************************************************
 * Function Name: adsl_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit adsl_cleanup( void )
{
    printk( "adsl: adsl_cleanup entry\n" );
    unregister_chrdev( ADSLDRV_MAJOR, "adsl" );
    if(g_dslNetDev) {
        unregister_netdev(g_dslNetDev);
	 free_netdev(g_dslNetDev);
	 g_dslNetDev = NULL;
    }
} /* adsl_cleanup */


/***************************************************************************
 * Function Name: adsl_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int adsl_open( struct inode *inode, struct file *filp )
{
    return( 0 );
} /* adsl_open */


/***************************************************************************
 * Function Name: adsl_ioctl
 * Description  : Main entry point for an application send issue ATM API
 *                requests.
 * Returns      : 0 - success or error
 ***************************************************************************/
static int adsl_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg )
{
    int ret = 0;
    unsigned int cmdnr = _IOC_NR(command);

    ADSL_FN_IOCTL IoctlFuncs[] = {DoCheck, DoInitialize, DoUninitialize,
        DoConnectionStart, DoConnectionStop, DoGetPhyAddresses, DoSetPhyAddresses,
                DoMapATMPortIDs,DoGetConnectionInfo, DoDiagCommand, DoGetObjValue, 
                DoStartBERT, DoStopBERT, DoConfigure, DoTest, DoGetConstelPoints,
                DoGetVersion, DoSetSdramBase, DoResetStatCounters, DoSetOemParameter, 
                DoBertStartEx, DoBertStopEx , DoSetObjValue
#ifdef HMI_QA_SUPPORT
		  ,DoHmiCommand
#endif

#ifdef VDF_SPAIN
		, DoGetCuts, DoGetOnlineTime
#endif

			};

    if( cmdnr >= 0 && cmdnr < MAX_ADSLDRV_IOCTL_COMMANDS )
        (*IoctlFuncs[cmdnr]) (arg);
    else
        ret = -EINVAL;

    return( ret );
} /* adsl_ioctl */


/***************************************************************************
 * Function Name: DoCheck
 * Description  : Calls BcmAdsl_Check on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoCheck( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_Check();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoCheck */


/***************************************************************************
 * Function Name: DoInitialize
 * Description  : Calls BcmAdsl_Initialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoInitialize( unsigned long arg )
{
        ADSLDRV_INITIALIZE      KArg;
        adslCfgProfile          adslCfg, *pAdslCfg;

    /* For now, the user callback function is not used.  Rather,
     * this module handles the connection up/down callback.  The
     * user application will need to call BcmAdsl_GetConnectionStatus
     * in order to determine the state of the connection.
     */

        pAdslCfg = NULL;
        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                if (NULL != KArg.pAdslCfg) {
                        if( copy_from_user( &adslCfg, KArg.pAdslCfg, sizeof(adslCfg)) != 0 )
                                break;
                        pAdslCfg = &adslCfg;
                }
        } while (0);

    KArg.bvStatus = BcmAdsl_Initialize( AdslConnectCb, 0, pAdslCfg);
#ifdef DYING_GASP_API
    kerSysRegisterDyingGaspHandler(DSL_IFNAME, &BcmAdsl_DyingGaspHandler, 0);
#endif
    put_user( KArg.bvStatus, &((PADSLDRV_INITIALIZE) arg)->bvStatus );

    /* This ADSL module is loaded after the ATM API module so the ATM API
     * cannot call the functions below directly.  This modules sets the
     * function address to a global variable which is used by the ATM API
     * module.
     */
    g_pfnAdslSetVcEntry = (void *) BcmAdsl_SetVcEntry;
    g_pfnAdslSetAtmLoopbackMode = (void *) BcmAdsl_SetAtmLoopbackMode;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    g_pfnAdslSetVcEntryEx = (void *) BcmAdsl_SetVcEntryEx;
#ifdef CONFIG_ATM_EOP_MONITORING
	
    g_pfnAdslAtmClearVcTable = BcmAdsl_AtmClearVcTable;
    g_pfnAdslAtmAddVc = BcmAdsl_AtmAddVc;
    g_pfnAdslAtmDeleteVc = BcmAdsl_AtmDeleteVc;
    g_pfnAdslAtmSetMaxSdu = BcmAdsl_AtmSetMaxSdu;
#endif
#ifdef BCM_ATM_BONDING
	 g_pfnAdslAtmbSetPortId = BcmAdsl_AtmSetPortId;
#endif
#if defined(BCM_ATM_BONDING) || defined(BCM_ATM_BONDING_ETH)
    g_pfnAdslAtmbGetConnInfo = BcmAdsl_GetConnectionInfo;
#endif

#if defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)
    if (atmbonding_enabled == 1)
    {
      printk("Disabling Gfc2VcMapping\n");
      BcmAdsl_SetGfc2VcMapping(0);
    }
#endif
#endif

    /* Set LED into mode that is looking for carrier. */
    /* kerSysLedCtrl(kLedAdsl, kLedStateFail); */
    /* Jiajun 12/18/2007 LED off */
/* BEGIN: Modified by c106292, 2009/4/13   PN: */
//kerSysLedCtrl(kLedAdsl, kLedStateOff);
    kerSysLedCtrl(kLedAdsl, kLedStateVerySlowBlinkContiunes);
/* END:   Modified by c106292, 2009/4/13 */
} /* DoInitialize */


/***************************************************************************
 * Function Name: DoUninitialize
 * Description  : Calls BcmAdsl_Uninitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoUninitialize( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_Uninitialize();
#ifdef DYING_GASP_API
    kerSysDeregisterDyingGaspHandler(DSL_IFNAME);
#endif
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoUninitialize */


/***************************************************************************
 * Function Name: DoConnectionStart
 * Description  : Calls BcmAdsl_ConnectionStart on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConnectionStart( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ConnectionStart();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoConnectionStart */


/***************************************************************************
 * Function Name: DoConnectionStop
 * Description  : Calls BcmAdsl_ConnectionStop on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConnectionStop( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ConnectionStop();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
} /* DoConnectionStop */


/***************************************************************************
 * Function Name: DoGetPhyAddresses
 * Description  : Calls BcmAdsl_GetPhyAddresses on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetPhyAddresses( unsigned long arg )
{
    ADSLDRV_PHY_ADDR KArg;

    KArg.bvStatus = BcmAdsl_GetPhyAddresses( &KArg.ChannelAddr );
    copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
} /* DoGetPhyAddresses */


/***************************************************************************
 * Function Name: DoSetPhyAddresses
 * Description  : Calls BcmAdsl_SetPhyAddresses on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetPhyAddresses( unsigned long arg )
{
    ADSLDRV_PHY_ADDR KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.bvStatus = BcmAdsl_SetPhyAddresses( &KArg.ChannelAddr );
        put_user( KArg.bvStatus, &((PADSLDRV_PHY_ADDR) arg)->bvStatus );
    }
} /* DoSetPhyAddresses */


/***************************************************************************
 * Function Name: DoMapATMPortIDs
 * Description  : Maps ATM Port IDs to DSL PHY Utopia Addresses.
 * Returns      : None.
 ***************************************************************************/
static void DoMapATMPortIDs( unsigned long arg )
{
    ADSLDRV_MAP_ATM_PORT KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
                g_usAtmFastPortId = KArg.usAtmFastPortId;
                g_usAtmInterleavedPortId = KArg.usAtmInterleavedPortId;
            put_user( BCMADSL_STATUS_SUCCESS, &((PADSLDRV_MAP_ATM_PORT) arg)->bvStatus );
        }
} /* DoMapATMPortIDs */


/***************************************************************************
 * Function Name: DoGetConnectionInfo
 * Description  : Calls BcmAdsl_GetConnectionInfo on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetConnectionInfo( unsigned long arg )
{
    ADSLDRV_CONNECTION_INFO KArg;

    KArg.bvStatus = BcmAdsl_GetConnectionInfo( &KArg.ConnectionInfo );
    copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
} /* DoGetConnectionInfo */


/***************************************************************************
 * Function Name: DoDiagCommand
 * Description  : Calls BcmAdsl_DiagCommand on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoDiagCommand ( unsigned long arg )
{
    ADSLDRV_DIAG        KArg;
        char                    cmdData[LOG_MAX_DATA_SIZE];

        KArg.bvStatus = BCMADSL_STATUS_ERROR;
        do {
                if (copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0)
                        break;

                if ((LOG_CMD_CONNECT != KArg.diagCmd) && (0 != KArg.diagMap)) {
                        if (KArg.logTime > sizeof(cmdData))
                                break;
                        if (copy_from_user(cmdData, (char*)KArg.diagMap, KArg.logTime) != 0)
                                break;

                        KArg.diagMap = (int) cmdData;
                }

        KArg.bvStatus = BcmAdsl_DiagCommand ( (PADSL_DIAG) &KArg );
        } while (0);
        put_user( KArg.bvStatus, &((PADSLDRV_DIAG) arg)->bvStatus );
} /* DoGetConnectionInfo */

/***************************************************************************
 * Function Name: DoGetObjValue
 * Description  : Calls BcmAdsl_GetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetObjValue( unsigned long arg )
{
        ADSLDRV_GET_OBJ KArg;
        char                    objId[kOidMaxObjLen];
        int                             retObj;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                if ((NULL != KArg.objId) && (KArg.objIdLen)) {
                        if (KArg.objIdLen > kOidMaxObjLen)
                                break;
                        if( copy_from_user( objId, KArg.objId, KArg.objIdLen ) != 0 )
                                break;
                        retObj = BcmAdsl_GetObjectValue (objId, KArg.objIdLen, NULL, &KArg.dataBufLen);
                }
                else
                        retObj = BcmAdsl_GetObjectValue (NULL, 0, NULL, &KArg.dataBufLen);

                if ((retObj >= kAdslMibStatusLastError) && (retObj < 0))
                        break;

                copy_to_user( KArg.dataBuf, (void *) retObj, KArg.dataBufLen );
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
                KArg.dataBufLen = 0;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
}


/***************************************************************************
 * Function Name: DoSetObjValue
 * Description  : Calls BcmAdsl_SetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetObjValue( unsigned long arg )
{
        ADSLDRV_GET_OBJ KArg;
        char                    objId[kOidMaxObjLen];
        char					dataBuf[100];
		int                     retObj;

        do {
				
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;
				if ((NULL != KArg.objId) && (KArg.objIdLen)) {
                        if (KArg.objIdLen > kOidMaxObjLen)
                                break;
                        if( copy_from_user( objId, KArg.objId, KArg.objIdLen ) != 0 )
                                break;
						else if (copy_from_user( dataBuf, KArg.dataBuf, KArg.dataBufLen ) != 0 )
								break;
						retObj = BcmAdsl_SetObjectValue (objId, KArg.objIdLen, dataBuf, &KArg.dataBufLen);
                }
                
                if ((retObj >= kAdslMibStatusLastError) && (retObj < 0))
                        break;
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
                KArg.dataBufLen = 0;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
}

/***************************************************************************
 * Function Name: DoStartBERT
 * Description  : Calls BcmAdsl_StartBERT on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoStartBERT( unsigned long arg )
{
        ADSLDRV_BERT    KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
        KArg.bvStatus = BcmAdsl_StartBERT (KArg.totalBits);
        put_user( KArg.bvStatus, &((PADSLDRV_BERT) arg)->bvStatus );
    }
}

/***************************************************************************
 * Function Name: DoStopBERT
 * Description  : Calls BcmAdsl_StopBERT on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoStopBERT( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_StopBERT();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoConfigure
 * Description  : Calls BcmAdsl_Configureon behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoConfigure( unsigned long arg )
{
        ADSLDRV_CONFIGURE       KArg;
        adslCfgProfile          adslCfg;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                if (NULL != KArg.pAdslCfg) {
                        if( copy_from_user( &adslCfg, KArg.pAdslCfg, sizeof(adslCfg)) != 0 )
                                break;
                        KArg.bvStatus = BcmAdsl_Configure(&adslCfg);
                }
                else
                        KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_CONFIGURE) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoTest
 * Description  : Controls ADSl driver/PHY special test modes
 * Returns      : None.
 ***************************************************************************/
static void DoTest( unsigned long arg )
{
        ADSLDRV_TEST            KArg;
        char                            rcvToneMap[512/8];
        char                            xmtToneMap[512/8];
        int                                     nToneBytes;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                if ((NULL != KArg.xmtToneMap) && (0 != KArg.xmtNumTones)) {
                        nToneBytes = (KArg.xmtNumTones + 7) >> 3;
                        if (nToneBytes > sizeof(xmtToneMap)) {
                                nToneBytes = sizeof(xmtToneMap);
                                KArg.xmtNumTones = nToneBytes << 3;
                        }
                        if( copy_from_user( xmtToneMap, KArg.xmtToneMap, nToneBytes) != 0 ) {
                                KArg.xmtNumTones = 0;
                                break;
                        }
                        KArg.xmtToneMap = xmtToneMap;
                }

                if ((NULL != KArg.rcvToneMap) && (0 != KArg.rcvNumTones)) {
                        nToneBytes = (KArg.rcvNumTones + 7) >> 3;
                        if (nToneBytes > sizeof(rcvToneMap)) {
                                nToneBytes = sizeof(rcvToneMap);
                                KArg.rcvNumTones = nToneBytes << 3;
                        }
                        if( copy_from_user( rcvToneMap, KArg.rcvToneMap, nToneBytes) != 0 ) {
                                KArg.rcvNumTones = 0;
                                break;
                        }
                        KArg.rcvToneMap = rcvToneMap;
                }
                
                if (ADSL_TEST_SELECT_TONES != KArg.testCmd)
                        KArg.bvStatus = BcmAdsl_SetTestMode(KArg.testCmd);
                else
                        KArg.bvStatus = BcmAdsl_SelectTones(
                                KArg.xmtStartTone, KArg.xmtNumTones,
                                KArg.rcvStartTone, KArg.rcvNumTones,
                                xmtToneMap, rcvToneMap);
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_TEST) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoGetConstelPoints
 * Description  : Calls BcmAdsl_GetObjectValue on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetConstelPoints( unsigned long arg )
{
        ADSLDRV_GET_CONSTEL_POINTS      KArg;
        ADSL_CONSTELLATION_POINT        pointBuf[64];
        int                                                     numPoints;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                numPoints = (KArg.numPoints > (sizeof(pointBuf)/sizeof(pointBuf[0])) ? sizeof(pointBuf)/sizeof(pointBuf[0]) : KArg.numPoints);
                numPoints = BcmAdsl_GetConstellationPoints (KArg.toneId, pointBuf, KArg.numPoints);
                if (numPoints > 0)
                        copy_to_user( KArg.pointBuf, (void *) pointBuf, numPoints * sizeof(ADSL_CONSTELLATION_POINT) );
                KArg.numPoints = numPoints;
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        if (BCMADSL_STATUS_ERROR == KArg.bvStatus)
                KArg.numPoints = 0;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
}

static void DoGetVersion( unsigned long arg )
{
        ADSLDRV_GET_VERSION             KArg;
        adslVersionInfo                 adslVer;

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }

                BcmAdsl_GetVersion(&adslVer);
                copy_to_user( KArg.pAdslVer, (void *) &adslVer, sizeof(adslVersionInfo) );
                KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus , &((PADSLDRV_GET_VERSION) arg)->bvStatus );
}

static void DoSetSdramBase( unsigned long arg )
{
        ADSLDRV_SET_SDRAM_BASE          KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
        KArg.bvStatus = BcmAdsl_SetSDRAMBaseAddr((void *)KArg.sdramBaseAddr);
        put_user( KArg.bvStatus, &((PADSLDRV_SET_SDRAM_BASE) arg)->bvStatus );
    }
}

static void DoResetStatCounters( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_ResetStatCounters();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}

static void DoSetOemParameter( unsigned long arg )
{
        ADSLDRV_SET_OEM_PARAM   KArg;
        char                                    dataBuf[256];

        do {
                if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) != 0 ) {
                        KArg.bvStatus = BCMADSL_STATUS_ERROR;
                        break;
                }
                KArg.bvStatus = BCMADSL_STATUS_ERROR;

                if ((NULL != KArg.buf) && (KArg.len > 0)) {
                        if (KArg.len > sizeof(dataBuf))
                                KArg.len = sizeof(dataBuf);
                        if( copy_from_user( dataBuf, (void *)KArg.buf, KArg.len) != 0 )
                                break;
                        KArg.bvStatus = BcmAdsl_SetOemParameter (KArg.paramId, dataBuf, KArg.len);
                }
                else
                        KArg.bvStatus = BCMADSL_STATUS_SUCCESS;
        } while (0);

        put_user( KArg.bvStatus, &((PADSLDRV_SET_OEM_PARAM) arg)->bvStatus );
}

/***************************************************************************
 * Function Name: DoBertStartEx
 * Description  : Calls BcmAdsl_BertStartEx on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoBertStartEx( unsigned long arg )
{
        ADSLDRV_BERT_EX         KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
        KArg.bvStatus = BcmAdsl_BertStartEx (KArg.totalSec);
        put_user( KArg.bvStatus, &((PADSLDRV_BERT_EX) arg)->bvStatus );
    }
}

/***************************************************************************
 * Function Name: DoBertStopEx
 * Description  : Calls BcmAdsl_BertStopEx on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoBertStopEx( unsigned long arg )
{
    BCMADSL_STATUS bvStatus = BcmAdsl_BertStopEx();
    put_user( bvStatus, &((PADSLDRV_STATUS_ONLY) arg)->bvStatus );
}
#ifdef HMI_QA_SUPPORT

/***************************************************************************
 * Function Name: DoHmiCommand
 * Description  : Calls BcmAdslHmiMsgProcess on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoHmiCommand ( unsigned long arg )
{
	BcmAdslHmiMsgProcess ( (void *)arg );	
}
#endif

/*****************************************************************************
 函 数 名  : DoGetCuts
 功能描述  : 获取掉线次数
 输入参数  : unsigned long arg
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年7月15日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
static void DoGetCuts( unsigned long arg )
{
	put_user(g_ulAdslCuts, (unsigned long *)arg);
}

/*****************************************************************************
 函 数 名  : DoGetOnlineTime
 功能描述  : 获取ADSL上线时长
 输入参数  : void
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年7月15日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
static void DoGetOnlineTime( unsigned long arg )
{
	long ldiff = 0;

	if (g_LinkState == BCM_ADSL_LINK_UP)
	{
		ldiff = (long)jiffies - (long)g_ulAdslOnLineStart;
		ldiff /= HZ;
	}

	put_user(ldiff, (unsigned long *)arg);
}


/***************************************************************************
 * Function Name: AdslConnectCb
 * Description  : Callback function that is called when by the ADSL driver
 *                when there is a change in status.
 * Returns      : None.
 ***************************************************************************/
static void AdslConnectCb( ADSL_LINK_STATE AdslLinkState, UINT32 ulParm )
{
    UINT32 ulInterfaceId;
    ADSL_CONNECTION_INFO AdslConnInfo;
    ATM_INTERFACE_LINK_INFO AtmLinkInfo;

#ifdef DYING_GASP_API
    /* Wake up the user mode application that monitors link status. */
    kerSysWakeupMonitorTask();
#endif

    AtmLinkInfo.ulLinkState = LINK_DOWN;
    switch (AdslLinkState)
      {
      case ADSL_LINK_UP:
        AtmLinkInfo.ulLinkState = LINK_UP;
	  /*l65130 2008-09-20 start*/		
        kerSysLedCtrl(kLedAdsl, kLedStateOn);
	  /*l65130 2008-09-20 end*/
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
        snmp_adsl_eoc_event();
#endif
        break;
      case ADSL_LINK_DOWN:
      case BCM_ADSL_ATM_IDLE:
	  	/* BEGIN: Modified by c106292, 2009/4/13   PN: */
	  	 kerSysLedCtrl(kLedAdsl, kLedStateVerySlowBlinkContiunes);//c106292
      //  if (AdslLinkState == ADSL_LINK_DOWN) {

		/*l65130 2008-09-20 start*/	
        //	kerSysLedCtrl(kLedAdsl, kLedStateOff);
		/*l65130 2008-09-20 end*/
      //  }
/* END:   Modified by c106292, 2009/4/13 */
        break;
      /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
      case BCM_ADSL_TRAINING_G994:
      /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
      case BCM_ADSL_TRAINING_G992_EXCHANGE:
      case BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS:
      case BCM_ADSL_TRAINING_G992_STARTED:
        /* kerSysLedCtrl(kLedAdsl, kLedStateFastBlinkContinues); */
    	/* Jiajun 12/18/2007 change to 500ms blink */
    	/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
    	kerSysLedCtrl(kLedAdsl, kLedStateVerySlowBlinkContiunes);  
    	*/
	/* BEGIN: Modified by c106292, 2009/3/13   PN:Led light method changes*/
    	/*l65130 2008-09-20 start*/
    	//kerSysLedCtrl(kLedAdsl, kLedStateSlowBlinkContinues);
	/*l65130 2008-09-20 end*/
    	kerSysLedCtrl(kLedAdsl, kLedStateFastBlinkContinues);
	/* END:   Modified by c106292, 2009/3/13 */
    	/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
        break;
#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
      case BCM_ADSL_G997_FRAME_RECEIVED:
        return;
      case BCM_ADSL_G997_FRAME_SENT:
 #ifdef BUILD_SNMP_TRANSPORT_DEBUG
        printk("BCM_ADSL_G997_FRAME_SENT \n");
 #endif
        return;
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */
      default:
        return;
      }
    /* record time stamp of link state change (in 1/100ths of a second unit per MIB spec.) */
    if (g_dslNetDev != NULL) 
      g_dslNetDev->trans_start = jiffies * 100 / HZ;

    BcmAdsl_GetConnectionInfo( &AdslConnInfo );
    AtmLinkInfo.ulStructureId = ID_ATM_INTERFACE_LINK_INFO;

    /* For now, only ATM port 0 is supported */
    BcmAtm_GetInterfaceId( 0, &ulInterfaceId );
    AtmLinkInfo.ulLineRate = (AdslConnInfo.ulFastUpStreamRate) ? AdslConnInfo.ulFastUpStreamRate
        : AdslConnInfo.ulInterleavedUpStreamRate;
    BcmAtm_SetInterfaceLinkInfo( ulInterfaceId, &AtmLinkInfo );

    if (AdslLinkState == BCM_ADSL_ATM_IDLE) {
        AtmLinkInfo.ulLinkState = LINK_UP;
                BcmAtm_SetInterfaceLinkInfo( ulInterfaceId, &AtmLinkInfo );
    }
} /* AdslConnectCb */


#if defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO)
/* routine copied from tiny bridge written by lt */
void snmp_adsl_eoc_event(void)
{
  adslMibInfo adslMib;
  long size = sizeof(adslMib);
  
  if( BcmAdsl_GetObjectValue(NULL, 0, (char *)&adslMib, &size) ==
      BCMADSL_STATUS_SUCCESS )
    {
      if( kAdslModAdsl2p == adslMib.adslConnection.modType ||
          kAdslModAdsl2  == adslMib.adslConnection.modType )
        {
          printk("ADSL2/ADSL2+ connection\n");
          g_eoc_hdr_offset = ADSL_2P_HDR_OFFSET;
          g_eoc_hdr_len    = ADSL_2P_EOC_HDR_LEN; 
        }
      else
        {
          printk("ADSL1 connection\n");
          g_eoc_hdr_offset = ADSL_HDR_OFFSET;
          g_eoc_hdr_len    = ADSL_EOC_HDR_LEN; 
        }
    }
}

/***************************************************************************
 * Function Name: adsl_read
 * The EOC read code (slightly modified) is copied from cfeBridge written by lat
 ***************************************************************************/
static ssize_t adsl_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
	int ret = 0, len, request_length=0;
	char *p;
	unsigned char adsl_eoc_hdr[] = ADSL_EOC_HDR;
	unsigned char adsl_eoc_enable[] = ADSL_EOC_ENABLE;
	char inputBuffer[520];
	
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
	printk("adsl_read entry\n");
#endif

	/* Receive SNMP request. */
	p = BcmAdsl_G997FrameGet( &len );
	if( p && len > 0 )
	{
		if( memcmp(p, adsl_eoc_enable, sizeof(adsl_eoc_enable)) == 0 ) {
			/* return now and let the upper layer know that link up is received */
			/* don't clear the FRAME_RCVD bit, just in case there are things to read */
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
			printk("*** Got message up on ADSL EOC connection, len %d ***\n",len);
			dumpaddr(p,len);
#endif
			BcmAdsl_G997FrameFinished();
			return len;
		}

		if (memcmp(p, adsl_eoc_hdr+g_eoc_hdr_offset, g_eoc_hdr_len) == 0) {
			  p += g_eoc_hdr_len;
			  len -= g_eoc_hdr_len;
		}
		else {
			  printk("something is wrong, len %d, no adsl_eoc_hdr found\n",len);
			  dumpaddr(p,len);
		}

		do
		{
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
			printk("***count %d, processing rx from EOC link, len %d/requestLen %d ***\n",
			count,len,request_length);
			dumpaddr(p,len);
#endif
			if( len + request_length < count ) {
				memcpy(inputBuffer + request_length, p, len);
				request_length += len;
			}
			p = BcmAdsl_G997FrameGetNext( &len );
		  } while( p && len > 0 );

		BcmAdsl_G997FrameFinished();
		if (!(copy_to_user(buf, inputBuffer, request_length)))
			ret = request_length;

#ifdef BUILD_SNMP_TRANSPORT_DEBUG
		printk("adsl_read(): end, request_length %d\n", request_length);
		printk("inputBuffer:\n");
		dumpaddr(inputBuffer,request_length);
#endif
	} /* (p  & len > 0) */
	
	return( ret );
} /* adsl_read */

/***************************************************************************
 * Function Name: adsl_write
 * The EOC write code is copied from cfeBridge written by lat
 ***************************************************************************/
static ssize_t adsl_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos)
{
	unsigned char adsl_eoc_hdr[] = ADSL_EOC_HDR;
	int ret = 0, len;
	char *pBuf;

#ifdef BUILD_SNMP_TRANSPORT_DEBUG
	printk("adsl_write(entry): count %d\n",count);
#endif

	/* Need to include adsl_eoc_hdr */
	len = count + g_eoc_hdr_len;

	if( (pBuf = calloc(len, 1)) == NULL ) {
		BcmAdslCoreDiagWriteStatusString("ADSL send failed, out of memory");
		return ret;
	}
	
	memcpy(pBuf, adsl_eoc_hdr+g_eoc_hdr_offset, g_eoc_hdr_len);

	if (copy_from_user((pBuf+g_eoc_hdr_len), buf, count) == 0)	{
#ifdef BUILD_SNMP_TRANSPORT_DEBUG
		printk("adsl_write ready to send data over EOC, len %d\n", len);
		dumpaddr(pBuf, len);
#endif
		
		if( BCMADSL_STATUS_SUCCESS == BcmAdsl_G997SendData(pBuf, len) ) {
			ret = len;
		}
		else {
			free(pBuf);
			BcmAdslCoreDiagWriteStatusString("ADSL send failed, len %d",   len);
		}
	}
	else {
		free(pBuf);
		BcmAdslCoreDiagWriteStatusString("ADSL send copy_from_user failed, len %d",   len);
	}		
  
	return ret;
	
} 

static unsigned int adsl_poll(struct file *file, poll_table *wait)
{
	return AdslCoreG997FrameReceived();
}
#endif /* defined(BUILD_SNMP_EOC) || defined(BUILD_SNMP_AUTO) */

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) || !defined(MODULE)
module_init( adsl_init );
module_exit( adsl_cleanup );
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
MODULE_LICENSE("Proprietary");
#endif

