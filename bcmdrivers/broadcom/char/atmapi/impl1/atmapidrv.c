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
 * File Name  : AtmApiDrv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the ATM API driver.
 *
 * Updates    : 08/24/2001  lat.  Created.
 ***************************************************************************/


/* Includes. */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <bcmtypes.h>
#include <atmapidrv.h>
#include <atmosservices.h>
#include <board.h>
#include "atmprocregs.h"
#include "boardparms.h"

//#define PORT_MIRROR_DBG   1

/* typedefs. */
typedef void (*FN_IOCTL) (unsigned long arg);

typedef struct StuffToAlloc
{
    ATM_VCC_DATA_PARMS DataParms;
    ATM_BUFFER AtmBuffers[2];
    UINT8 ucBuffer1[48];
    UINT8 ucBuffer2[1500];
} STUFF_TO_ALLOC, *PSTUFF_TO_ALLOC;


/* Prototypes. */
static int __init atmapi_init( void );
static void __exit atmapi_cleanup( void );
static int atmapi_open( struct inode *inode, struct file *filp );
static int atmapi_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg );
static void DoInitialize( unsigned long arg );
static void DoUninitialize( unsigned long arg );
static void DoGetInterfaceId( unsigned long arg );
static void DoGetPriorityPacketGroup( unsigned long arg );
static void DoSetPriorityPacketGroup( unsigned long arg );
static void DoGetTrafficDescrTableSize( unsigned long arg );
static void DoGetTrafficDescrTable( unsigned long arg );
static void DoSetTrafficDescrTable( unsigned long arg );
static void DoGetInterfaceCfg( unsigned long arg );
static void DoSetInterfaceCfg( unsigned long arg );
static void DoGetVccCfg( unsigned long arg );
static void DoSetVccCfg( unsigned long arg );
static void DoGetVccAddrs( unsigned long arg );
static void DoGetInterfaceStatistics( unsigned long arg );
static void DoGetVccStatistics( unsigned long arg );
static void DoSetInterfaceLinkInfo( unsigned long arg );
static void DoTest( unsigned long arg );
static void DoOamLoopbackTest( unsigned long arg );
static void DoPortMirrorCfg ( unsigned long arg );
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
static void DoSetTrafficType( unsigned long arg );
static void DoGetTrafficType( unsigned long arg );
static void DoSetConfigMode( unsigned long arg );
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* start of protocol QoS for KPN by zhangchen z45221 2006年7月18日 */
static void DoSetQosWeight( unsigned long arg );
/* end of protocol QoS for KPN by zhangchen z45221 2006年7月18日 */

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338) || defined(CONFIG_BCM96358)
static void AdslLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState);
static void WanDataLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState);
#endif

int rfc2684InMirrorStatus  = MIRROR_DISABLED ;
int rfc2684OutMirrorStatus = MIRROR_DISABLED ;
char mirrorInPort  [MIRROR_NAME_MAX_LEN]  = "\n" ;
char mirrorOutPort [MIRROR_NAME_MAX_LEN]  = "\n" ;

/* start of protocol QoS for KPN在内核中获取vdsl上行激活速率 by zhangchen z45221 2006年7月17日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS     // macro QoS for KPN
unsigned int g_ulLinkUpStreamRate = 0;
unsigned int g_ulQosHWeight = 1;
unsigned int g_ulQosLWeight = 1;
#endif
#endif
/* end of protocol QoS for KPN在内核中获取vdsl上行激活速率 by zhangchen z45221 2006年7月17日 */

extern int blaa_ioctl(void *arg) ;  /* Called by ATMAPI driver for port mirroring appln. */
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
extern int g_nPtmTraffic;
extern int g_nPtmConfig;
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
/* Globals. */
static struct file_operations atmapi_fops =
{
    .ioctl  = atmapi_ioctl,
    .open   = atmapi_open,
};

/* This ATM API module is loaded before the ADSL module.  The ADSL module, once
 * it is loaded, sets the global function pointers below to the correct
 * address.
 */
void *g_pfnAdslSetVcEntry = NULL;
void *g_pfnAdslSetAtmLoopbackMode = NULL;
void *g_pfnAdslSetVcEntryEx = NULL;
void *g_pfnAdslAtmClearVcTable = NULL;
void *g_pfnAdslAtmAddVc = NULL;
void *g_pfnAdslAtmDeleteVc = NULL;
void *g_pfnAdslAtmSetMaxSdu = NULL;

void blaaUtilSetMirrorPort (int mirrorPort, char *pMirrorPort) ;


char    *blaaMirrorDevNames [] = { "eth0", "eth1", "usb0" } ;

/***************************************************************************
 * Function Name: atmapi_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init atmapi_init( void )
{
    register_chrdev( ATMDRV_MAJOR, "atmapi", &atmapi_fops );
    return( 0 );
} /* atmapi_init */


/***************************************************************************
 * Function Name: atmapi_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit atmapi_cleanup( void )
{
    printk( "atmapi: atmapi_cleanup entry\n" );
    unregister_chrdev( ATMDRV_MAJOR, "atmapi" );
    BcmAtm_Uninitialize();
} /* atmapi_cleanup */


/***************************************************************************
 * Function Name: atmapi_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int atmapi_open( struct inode *inode, struct file *filp )
{
    return( 0 );
} /* atmapi_open */


/***************************************************************************
 * Function Name: atmapi_ioctl
 * Description  : Main entry point for an application send issue ATM API
 *                requests.
 * Returns      : 0 - success or error
 ***************************************************************************/
static int atmapi_ioctl( struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg )
{
    int ret = 0;
    unsigned int cmdnr = _IOC_NR(command);

    FN_IOCTL IoctlFuncs[] = { DoInitialize, DoUninitialize, DoGetInterfaceId,
        DoGetTrafficDescrTableSize, DoGetTrafficDescrTable,
        DoSetTrafficDescrTable, DoGetInterfaceCfg, DoSetInterfaceCfg,
        DoGetVccCfg, DoSetVccCfg, DoGetVccAddrs, DoGetInterfaceStatistics,
        DoGetVccStatistics, DoSetInterfaceLinkInfo, DoTest, DoOamLoopbackTest,
        DoPortMirrorCfg, DoGetPriorityPacketGroup, DoSetPriorityPacketGroup, 
        DoSetTrafficType, DoGetTrafficType, DoSetConfigMode,DoSetQosWeight};

    if( cmdnr >= 0 && cmdnr < MAX_ATMDRV_IOCTL_COMMANDS )
        (*IoctlFuncs[cmdnr]) (arg);
    else
        ret = -EINVAL;

    return( ret );
} /* atmapi_ioctl */


/***************************************************************************
 * Function Name: DoInitialize
 * Description  : Calls BcmAtm_Initialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoInitialize( unsigned long arg )
{
    PATMDRV_INITIALIZE pUArg = (PATMDRV_INITIALIZE) arg;
    PATM_INITIALIZATION_PARMS pUInit;
    ATM_INITIALIZATION_PARMS Init;

    if( get_user( pUInit, &pUArg->pInit ) == 0 )
    {
        if( copy_from_user( &Init, pUInit, sizeof(Init) ) == 0 )
        {
/*Start modify : s48571 2006-10-26 for adsl porting*/      	
#ifdef CONFIG_BCM_VDSL
            /*Start Add:l37298 2006-05-30 for deleting adsl feature*/
        	Init.PortCfg[0].ucPortType = PT_UTOPIA;
	        Init.PortCfg[1].ucPortType = PT_UTOPIA;
	        Init.PortCfg[2].ucPortType = PT_UTOPIA;
	        Init.PortCfg[3].ucPortType = PT_UTOPIA;
	        /*End Add:l37298 2006-05-30 for deleting adsl feature*/
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
            /* printk ("\n ************** Atm Api Drv - BcmAtm_Initialize ************* \n") ; */
            BCMATM_STATUS baStatus = BcmAtm_Initialize( &Init );
            put_user( baStatus, &pUArg->baStatus );
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338) || defined(CONFIG_BCM96358)
            if( baStatus == STS_SUCCESS )
            {
                unsigned short usValue;

                if( BpGetWanDataLedGpio( &usValue ) != BP_SUCCESS )
                {
                    /* A software controlled, GPIO enabled LED is not defined
                     * for WAN data so use the ADSL hardware LED for WAN data
                     * and PPP connection.
                     */
                    kerSysLedRegisterHwHandler( kLedWanData, WanDataLedCtrl, 0 );
                    if( BpGetPppLedGpio( &usValue ) != BP_SUCCESS )
                        kerSysLedRegisterHwHandler( kLedPPP, WanDataLedCtrl, 0 );
                }
                else
                {
                    if( BpGetAdslLedGpio( &usValue ) != BP_SUCCESS )
                    {
                        /* A software controlled, GPIO enabled LED is not
                         * defined for the ADSL connection LED so use the
                         * ADSL hardware LED for the ADSL connection LED.
                         */
                        kerSysLedRegisterHwHandler( kLedAdsl, AdslLedCtrl, 0 );
                    }
                }
            }
#endif
        }
    }
} /* DoInitialize */


/***************************************************************************
 * Function Name: DoUninitialize
 * Description  : Calls BcmAtm_Uninitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoUninitialize( unsigned long arg )
{
    BCMATM_STATUS baStatus = BcmAtm_Uninitialize();
    put_user( baStatus, &((PATMDRV_STATUS_ONLY) arg)->baStatus );
} /* DoUninitialize */


/***************************************************************************
 * Function Name: DoGetInterfaceId
 * Description  : Calls BcmAtm_GetInterfaceId on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetInterfaceId( unsigned long arg )
{
    ATMDRV_INTERFACE_ID KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.baStatus = BcmAtm_GetInterfaceId( KArg.ucPhyPort,
            &KArg.ulInterfaceId );
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
    }
} /* DoGetInterfaceId */


/***************************************************************************
 * Function Name: DoGetPriorityPacketGroup
 * Description  : Calls BcmAtm_GetPriorityPacketGroup on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetPriorityPacketGroup( unsigned long arg )
{
    ATMDRV_PRIORITY_PACKET_GROUP KArg;
    ATM_PRIORITY_PACKET_ENTRY KGroup[NUM_PRI_PKT_GROUPS];

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        UINT32 ulSize = (KArg.ulPriorityPacketsSize <= NUM_PRI_PKT_GROUPS)
            ?  KArg.ulPriorityPacketsSize : NUM_PRI_PKT_GROUPS;
        if( copy_from_user( KGroup, KArg.pPriorityPackets,
            sizeof(ATM_PRIORITY_PACKET_ENTRY) * ulSize ) == 0 )
        {
            KArg.baStatus = BcmAtm_GetPriorityPacketGroup( KArg.ulGroupNumber,
                KGroup, &KArg.ulPriorityPacketsSize );

            if( KArg.baStatus == STS_SUCCESS )
            {
                copy_to_user( KArg.pPriorityPackets, KGroup,
                    sizeof(ATM_PRIORITY_PACKET_ENTRY) * ulSize );
            }

            put_user( KArg.baStatus,
                &((PATMDRV_PRIORITY_PACKET_GROUP) arg)->baStatus );
            put_user( KArg.ulPriorityPacketsSize,
                &((PATMDRV_PRIORITY_PACKET_GROUP) arg)->ulPriorityPacketsSize );
        }
    }
} /* DoGetPriorityPacketGroup */


/***************************************************************************
 * Function Name: DoSetPriorityPacketGroup
 * Description  : Calls BcmAtm_SetPriorityPacketGroup on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetPriorityPacketGroup( unsigned long arg )
{
    ATMDRV_PRIORITY_PACKET_GROUP KArg;
    ATM_PRIORITY_PACKET_ENTRY KGroup[NUM_PRI_PKT_GROUPS];

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        UINT32 ulSize = (KArg.ulPriorityPacketsSize <= NUM_PRI_PKT_GROUPS)
            ?  KArg.ulPriorityPacketsSize : NUM_PRI_PKT_GROUPS;
        if( copy_from_user( KGroup, KArg.pPriorityPackets,
            sizeof(ATM_PRIORITY_PACKET_ENTRY) * ulSize ) == 0 )
        {
            KArg.baStatus = BcmAtm_SetPriorityPacketGroup( KArg.ulGroupNumber,
                KGroup, KArg.ulPriorityPacketsSize );

            put_user( KArg.baStatus,
                &((PATMDRV_PRIORITY_PACKET_GROUP) arg)->baStatus );
        }
    }
} /* DoSetPriorityPacketGroup */


/***************************************************************************
 * Function Name: DoGetTrafficDescrTableSize
 * Description  : Calls BcmAtm_GetTrafficDescrTableSize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetTrafficDescrTableSize( unsigned long arg )
{
    ATMDRV_TRAFFIC_DESCR_TABLE_SIZE KArg;

    KArg.baStatus = BcmAtm_GetTrafficDescrTableSize(
        &KArg.ulTrafficDescrTableSize );
    copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
} /* DoGetTrafficDescrTableSize */


/***************************************************************************
 * Function Name: DoGetTrafficDescrTable
 * Description  : Calls BcmAtm_GetTrafficDescrTable on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetTrafficDescrTable( unsigned long arg )
{
    ATMDRV_TRAFFIC_DESCR_TABLE KArg;
    PATM_TRAFFIC_DESCR_PARM_ENTRY pKTbl;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        ulSize = KArg.ulTrafficDescrTableSize *
            sizeof(ATM_TRAFFIC_DESCR_PARM_ENTRY);
        pKTbl = (PATM_TRAFFIC_DESCR_PARM_ENTRY) kmalloc( ulSize, GFP_KERNEL );

        if( pKTbl )
        {
            if( get_user( pKTbl->ulStructureId, \
                &KArg.pTrafficDescrTable->ulStructureId ) == 0 )

            {
                KArg.baStatus = BcmAtm_GetTrafficDescrTable( pKTbl,
                    KArg.ulTrafficDescrTableSize );

                if( KArg.baStatus == STS_SUCCESS )
                    copy_to_user((void *) KArg.pTrafficDescrTable,pKTbl,ulSize);

                put_user( KArg.baStatus,
                    &((PATMDRV_TRAFFIC_DESCR_TABLE) arg)->baStatus );

                kfree( pKTbl );
            }
        }
    }
} /* DoGetTrafficDescrTable */


/***************************************************************************
 * Function Name: DoSetTrafficDescrTable
 * Description  : Calls BcmAtm_SetTrafficDescrTable on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetTrafficDescrTable( unsigned long arg )
{
    ATMDRV_TRAFFIC_DESCR_TABLE KArg;
    PATM_TRAFFIC_DESCR_PARM_ENTRY pKTbl;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        ulSize = KArg.ulTrafficDescrTableSize *
            sizeof(ATM_TRAFFIC_DESCR_PARM_ENTRY);
        pKTbl = (PATM_TRAFFIC_DESCR_PARM_ENTRY) kmalloc( ulSize, GFP_KERNEL );

        if( pKTbl )
        {
            if( copy_from_user( pKTbl, KArg.pTrafficDescrTable, ulSize ) == 0 )
            {
                KArg.baStatus = BcmAtm_SetTrafficDescrTable( pKTbl,
                    KArg.ulTrafficDescrTableSize );

                put_user( KArg.baStatus,
                    &((PATMDRV_TRAFFIC_DESCR_TABLE) arg)->baStatus );

            }

            kfree( pKTbl );
        }
    }
} /* DoSetTrafficDescrTable */


/***************************************************************************
 * Function Name: DoGetInterfaceCfg
 * Description  : Calls BcmAtm_GetInterfaceCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetInterfaceCfg( unsigned long arg )
{
    ATMDRV_INTERFACE_CFG KArg;
    ATM_INTERFACE_CFG KCfg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if( get_user(KCfg.ulStructureId,&KArg.pInterfaceCfg->ulStructureId)==0 )
        {
            KArg.baStatus = BcmAtm_GetInterfaceCfg( KArg.ulInterfaceId, &KCfg );

            if( KArg.baStatus == STS_SUCCESS )
                copy_to_user( KArg.pInterfaceCfg, &KCfg, sizeof(KCfg) );

            put_user( KArg.baStatus, &((PATMDRV_INTERFACE_CFG) arg)->baStatus );
        }
    }
} /* DoGetInterfaceCfg */


/***************************************************************************
 * Function Name: DoSetInterfaceCfg
 * Description  : Calls BcmAtm_SetInterfaceCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetInterfaceCfg( unsigned long arg )
{
    ATMDRV_INTERFACE_CFG KArg;
    ATM_INTERFACE_CFG KCfg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if( copy_from_user( &KCfg, KArg.pInterfaceCfg, sizeof(KCfg) ) == 0 )
        {
            KArg.baStatus = BcmAtm_SetInterfaceCfg( KArg.ulInterfaceId,
                &KCfg );

            put_user( KArg.baStatus, &((PATMDRV_INTERFACE_CFG) arg)->baStatus );
        }
    }
} /* DoSetInterfaceCfg */


/***************************************************************************
 * Function Name: DoGetVccCfg
 * Description  : Calls BcmAtm_GetVccCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetVccCfg( unsigned long arg )
{
    ATMDRV_VCC_CFG KArg;
    ATM_VCC_CFG KCfg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if( get_user( KCfg.ulStructureId, &KArg.pVccCfg->ulStructureId )==0 )
        {
            KArg.baStatus = BcmAtm_GetVccCfg( &KArg.VccAddr, &KCfg );

            if( KArg.baStatus == STS_SUCCESS )
                copy_to_user( KArg.pVccCfg, &KCfg, sizeof(KCfg) );

            put_user( KArg.baStatus, &((PATMDRV_VCC_CFG) arg)->baStatus );
        }
    }
} /* DoGetVccCfg */


/***************************************************************************
 * Function Name: DoSetVccCfg
 * Description  : Calls BcmAtm_SetVccCfg on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetVccCfg( unsigned long arg )
{
    ATMDRV_VCC_CFG KArg;
    ATM_VCC_CFG KCfg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if( KArg.pVccCfg == NULL )
        {
            KArg.baStatus = BcmAtm_SetVccCfg( &KArg.VccAddr, NULL );
            put_user( KArg.baStatus, &((PATMDRV_VCC_CFG) arg)->baStatus );
        }
        else
            if( copy_from_user( &KCfg, KArg.pVccCfg, sizeof(KCfg) ) == 0 )
            {
                KArg.baStatus = BcmAtm_SetVccCfg( &KArg.VccAddr, &KCfg );
                put_user( KArg.baStatus, &((PATMDRV_VCC_CFG) arg)->baStatus );
            }
    }
} /* DoSetVccCfg */


/***************************************************************************
 * Function Name: DoGetVccAddrs
 * Description  : Calls BcmAtm_GetVccAddrs on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetVccAddrs( unsigned long arg )
{
    ATMDRV_VCC_ADDRS KArg;
    PATM_VCC_ADDR pKAddrs;
    UINT32 ulSize;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        ulSize = KArg.ulNumVccs * sizeof(ATM_VCC_ADDR);
        pKAddrs = (PATM_VCC_ADDR) kmalloc( ulSize, GFP_KERNEL );

        if( pKAddrs || ulSize == 0 )
        {
            KArg.baStatus = BcmAtm_GetVccAddrs( KArg.ulInterfaceId, pKAddrs, 
                KArg.ulNumVccs, &KArg.ulNumReturned );

            copy_to_user( KArg.pVccAddrs, pKAddrs, ulSize );
            put_user( KArg.ulNumReturned,
                &((PATMDRV_VCC_ADDRS) arg)->ulNumReturned );

            put_user( KArg.baStatus, &((PATMDRV_VCC_ADDRS) arg)->baStatus );

            if( pKAddrs )
                kfree( pKAddrs );
        }
    }
} /* DoGetVccAddrs */


/***************************************************************************
 * Function Name: DoGetInterfaceStatistics
 * Description  : Calls BcmAtm_GetInterfaceStatistics on behalf of a user
 *                program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetInterfaceStatistics( unsigned long arg )
{
    ATMDRV_INTERFACE_STATISTICS KArg;
    ATM_INTERFACE_STATS KStats;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if( get_user(KStats.ulStructureId,&KArg.pStatistics->ulStructureId)==0 )
        {
            KArg.baStatus = BcmAtm_GetInterfaceStatistics( KArg.ulInterfaceId,
                &KStats, KArg.ulReset );

            if( KArg.baStatus == STS_SUCCESS )
                copy_to_user( KArg.pStatistics, &KStats, sizeof(KStats) );

            put_user( KArg.baStatus,
                &((PATMDRV_INTERFACE_STATISTICS)arg)->baStatus );
        }
    }
} /* DoGetInterfaceStatistics */

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
/***************************************************************************
 * Function Name: DoGetTrafficType
 * Description  : Calls BcmAtm_GetTrafficType on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetTrafficType( unsigned long arg )
{
#ifdef CONFIG_BCM_VDSL
    ATMDRV_TRAFFIC_TYPE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        KArg.ulTrafficType = g_nPtmTraffic;
        copy_to_user( (void *) arg, &KArg, sizeof(KArg) );
    }
#else
    return;
#endif
} /* DoGetTrafficType */

/***************************************************************************
 * Function Name: DoSetConfigMode
 * Description  : Sets ATM/PTM config mode parameter. For non-VDSL builds it
 *                always returns an error, and exist only to keep consistent
 *                function count in atmapidrv.h
 * Returns      : None.
 ***************************************************************************/
static void DoSetConfigMode( unsigned long arg )
{
#ifdef CONFIG_BCM_VDSL
    ATMDRV_TRAFFIC_TYPE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        /* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
        if ( KArg.ulTrafficType == ATM_TRAFFIC )
        {
            g_nPtmConfig = ATM_TRAFFIC;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulTrafficType == PTM_TRAFFIC )
        {
            g_nPtmConfig = PTM_TRAFFIC;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulTrafficType == PTM_TRAFFIC_PRTCL )
        {
            g_nPtmConfig = PTM_TRAFFIC_PRTCL;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulTrafficType == ATM_TRAFFIC_BR )
        {
            g_nPtmConfig = ATM_TRAFFIC_BR;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulTrafficType == PTM_TRAFFIC_BR )
        {
            g_nPtmConfig = PTM_TRAFFIC_BR;
            KArg.baStatus = STS_SUCCESS;
        }
        else
            KArg.baStatus = STS_ERROR;
        /* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
    }
#else
    return(STS_ERROR);
#endif
} /* DoSetTrafficType */

/* start of protocol add of  qos weight  by zhangchen z45221 2006年7月18日 */
/***************************************************************************
 * Function Name: DoSetQosWeight
 * Description  : Sets KPN QOS Weight config parameter. 
 * Returns      : None.
 ***************************************************************************/
static void DoSetQosWeight( unsigned long arg )
{
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS
    ATMDRV_QOSWEIGHT_TYPE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if ( KArg.ulWeightFlg ==  QOS_HWEIGHT)
        {
            g_ulQosHWeight = KArg.ulWeight;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulWeightFlg ==  QOS_LWEIGHT )
        {
            g_ulQosLWeight = KArg.ulWeight;
            KArg.baStatus = STS_SUCCESS;
        }
        else
        {
            KArg.baStatus = STS_ERROR;
        }
    }
#else
    return;
#endif
#else
    return;
#endif
} /* DoSetTrafficType */

/* end of protocol add of  qos weight  by zhangchen z45221 2006年7月18日 */


/***************************************************************************
 * Function Name: DoSetTrafficType
 * Description  : Sets ATM/PTM traffic type parameter. For non-VDSL builds it
 *                always returns an error, and exist only to keep consistent
 *                function count in atmapidrv.h
 * Returns      : None.
 ***************************************************************************/
static void DoSetTrafficType( unsigned long arg )
{
#ifdef CONFIG_BCM_VDSL
    ATMDRV_TRAFFIC_TYPE KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if ( KArg.ulTrafficType == ATM_TRAFFIC )
        {
            g_nPtmTraffic = 0;
            KArg.baStatus = STS_SUCCESS;
        }
        else if ( KArg.ulTrafficType == PTM_TRAFFIC )
        {
            g_nPtmTraffic = 1;
            KArg.baStatus = STS_SUCCESS;
        }
        else
            KArg.baStatus = STS_ERROR;
    }
#else
    return(STS_ERROR);
#endif
} /* DoSetTrafficType */
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/***************************************************************************
 * Function Name: DoGetVccStatistics
 * Description  : Calls BcmAtm_GetVccStatistics on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoGetVccStatistics( unsigned long arg )
{
    ATMDRV_VCC_STATISTICS KArg;
    ATM_VCC_STATS KStats;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        if(!get_user(KStats.ulStructureId,&KArg.pVccStatistics->ulStructureId))
        {
            KArg.baStatus = BcmAtm_GetVccStatistics( &KArg.VccAddr, &KStats,
                KArg.ulReset );

            if( KArg.baStatus == STS_SUCCESS )
                copy_to_user( KArg.pVccStatistics, &KStats, sizeof(KStats) );

            put_user( KArg.baStatus, &((PATMDRV_VCC_STATISTICS) arg)->baStatus );
        }
    }
} /* DoGetVccStatistics */


/***************************************************************************
 * Function Name: DoSetInterfaceLinkInfo
 * Description  : Calls BcmAtm_SetInterfaceLinkInfo on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoSetInterfaceLinkInfo( unsigned long arg )
{
    ATMDRV_INTERFACE_LINK_INFO KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        /* start of protocol QoS for KPN获取vdsl上行激活速率 by zhangchen z45221 2006年7月17日 */
        #ifdef CONFIG_BCM_VDSL
        #ifdef CONFIG_KPN_QOS   // macro QoS for KPN
        if ((g_nPtmConfig == ATM_TRAFFIC_BR) || (g_nPtmConfig == PTM_TRAFFIC_BR) )
        {
            g_ulLinkUpStreamRate = KArg.InterfaceCfg.ulLineRate;
        }
        #endif
        #endif
        /* end of protocol QoS for KPN获取vdsl上行激活速率 by zhangchen z45221 2006年7月17日 */
        KArg.baStatus = BcmAtm_SetInterfaceLinkInfo( KArg.ulInterfaceId,
            &KArg.InterfaceCfg );
        put_user(KArg.baStatus, &((PATMDRV_INTERFACE_LINK_INFO) arg)->baStatus);
    }
} /* DoSetInterfaceLinkInfo */

static void oamReceiveLoopback( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, PATM_VCC_DATA_PARMS *ppDpHT )
{
  if( ppDpHT[1] == NULL )
    {
      ppDpHT[0] = ppDpHT[1] = pDp;
    }
  else
    {
      ppDpHT[1]->pApplicationLink = pDp;
      ppDpHT[1] = pDp;
    }
  pDp->pApplicationLink = 0;
} /* ReceiveOamLoopback */

static void oamTestFreeDataParmsCb( PATM_VCC_DATA_PARMS pDataParms )
{
  if (pDataParms != NULL) {
    if (pDataParms->pAtmBuffer != NULL) {
      if (pDataParms->pAtmBuffer->pDataBuf != NULL) {
        kfree(pDataParms->pAtmBuffer->pDataBuf);
      }
      kfree(pDataParms->pAtmBuffer);
    }
    kfree(pDataParms);
  }
} /* OamTestFreeDataParmsCb */

static BCMATM_STATUS oamTestAttach(UINT32 *handle,PATM_VCC_ADDR pAddr, UINT32 type,
                                   PATM_VCC_DATA_PARMS *ppDpHT, UINT32 *managment)
{
    BCMATM_STATUS baStatus=STS_ERROR;
    ATM_VCC_ATTACH_PARMS AttachParms;
    /* Create Attach parameter structure and call BcmAtm_AttachVcc. */
    ATM_VCC_TRANSMIT_QUEUE_PARMS TxQueue =
        {ID_ATM_VCC_TRANSMIT_QUEUE_PARMS, 64, 1, 0};
    ATM_VCC_CFG vccCfg;
    int nRet=-1;

    memset( &vccCfg, 0x00, sizeof(vccCfg) );
    vccCfg.ulStructureId = ID_ATM_VCC_CFG;

    /* Read the current vcc configuration. */
    nRet = (int) BcmAtm_GetVccCfg(pAddr, &vccCfg);

    if( nRet != 0 )
      {
        /* Add the new queue parameters entry. */
        vccCfg.ulAalType = AAL_0_CELL_CRC;
        vccCfg.u.Aal5Cfg.ulAtmVccCpcsAcceptCorruptedPdus = 1;
        vccCfg.ulStructureId = ID_ATM_VCC_CFG;
        vccCfg.ulAtmVclAdminStatus = ADMSTS_UP;
        vccCfg.ulAtmVclOperStatus = OPRSTS_UP;
        vccCfg.ulAtmVclReceiveTrafficDescrIndex = 0;
        vccCfg.ulAtmVclTransmitTrafficDescrIndex = 1;
        vccCfg.ulAtmVclLastChange = 0;

        vccCfg.ulTransmitQParmsSize = 1;
        memcpy(&vccCfg.TransmitQParms[0],&TxQueue,sizeof(TxQueue));
        baStatus = BcmAtm_SetVccCfg(pAddr,&vccCfg);
        
        if( baStatus != STS_SUCCESS ) 
          {
            printk("oamTestAttach: BcmAtm_SetVccCfg() error %d!\n",baStatus);
            return baStatus;
          }
      }

    /* init attach parameters */
    AttachParms.ulStructureId = ID_ATM_VCC_ATTACH_PARMS;
    switch (type)
      {
      case OAM_LB_SEGMENT_TYPE:
        AttachParms.ulFlags = AVAP_ALLOW_OAM_F5_SEGMENT_CELLS;
        break;
      case OAM_LB_END_TO_END_TYPE:
        AttachParms.ulFlags = AVAP_ALLOW_OAM_F5_END_TO_END_CELLS;
        break;
      case OAM_F4_LB_SEGMENT_TYPE:
        AttachParms.ulFlags = (AVAP_ALLOW_OAM_F4_SEGMENT_CELLS | AVAP_ADD_AAL0_CRC10);
        break;
      case OAM_F4_LB_END_TO_END_TYPE:
        AttachParms.ulFlags = (AVAP_ALLOW_OAM_F4_END_TO_END_CELLS | AVAP_ADD_AAL0_CRC10);
        break;
      } 
    AttachParms.pFnReceiveDataCb = (FN_RECEIVE_CB)oamReceiveLoopback;
    AttachParms.ulParmReceiveData = (UINT32)ppDpHT;
    AttachParms.pTransmitQParms = NULL;
    AttachParms.ulTransmitQParmsSize = 1;
    AttachParms.ulHandle = 0;

    baStatus = BcmAtm_AttachVcc( pAddr, &AttachParms );

    if( baStatus == STS_SUCCESS ) {
        *handle = AttachParms.ulHandle;
        *managment=0;
    }
    else {
      if (baStatus == STS_IN_USE || baStatus == STS_RESOURCE_ERROR) {
          AttachParms.pFnReceiveDataCb = (FN_RECEIVE_CB)oamReceiveLoopback;
          AttachParms.pTransmitQParms = NULL;
          AttachParms.ulTransmitQParmsSize = 0;
          AttachParms.ulHandle = 0;
          baStatus = BcmAtm_AttachMgmtCells(pAddr->ulInterfaceId,&AttachParms);
          if (baStatus == STS_SUCCESS) {
            *handle = AttachParms.ulHandle;
            *managment=1;
          }
          else
            printk((KERN_CRIT "Unable attach management VCC vpi/vci %d/%d on interface %d to send OAM cell (error %d).\n"),
                   (int)pAddr->usVpi,(int)pAddr->usVci,
                   (int)(pAddr->ulInterfaceId),(int)baStatus);
      }
      else {
        printk((KERN_CRIT "Unable attach VCC vpi/vci %d/%d on interface %d to send OAM cell (error %d).\n"),
               (int)pAddr->usVpi,(int)pAddr->usVci,
               (int)(pAddr->ulInterfaceId),(int)baStatus);
      }
    }
  return baStatus;
} /* oamTestAttach */

static BCMATM_STATUS oamTestSend(unsigned long handle, PATM_VCC_ADDR pVccAddr,
                                 int type, UINT32 management)
{
  BCMATM_STATUS baStatus=STS_ERROR;
  PATM_VCC_DATA_PARMS pParms=NULL;
  PATM_BUFFER pAtmBuf=NULL;
  UINT8 *pDataBuf=NULL, *ptr;

  pAtmBuf = (PATM_BUFFER)(kmalloc(sizeof(ATM_BUFFER),GFP_KERNEL));
  pDataBuf = (UINT8*)(kmalloc(48,GFP_KERNEL));
  pParms = (PATM_VCC_DATA_PARMS)(kmalloc(sizeof(ATM_VCC_DATA_PARMS),GFP_KERNEL));
  
  if ((pAtmBuf != NULL) && (pDataBuf != NULL) && (pParms != NULL)) {
    memset(pParms,0,sizeof(ATM_VCC_DATA_PARMS));
    ptr = pDataBuf;
    ptr[OAM_TYPE_FUNCTION_BYTE_OFFSET] = OAM_FAULT_MANAGEMENT_LB;
    ptr[OAM_LB_INDICATION_BYTE_OFFSET] = OAM_FAULT_MANAGEMENT_LB_REQUEST;
    *((int*)(ptr+OAM_LB_CORRELATION_TAG_BYTE_OFFSET)) = OAM_FAULT_MANAGEMENT_CORRELATION_VAL;
    if ((type == OAM_LB_END_TO_END_TYPE) || (type == OAM_F4_LB_END_TO_END_TYPE)) {
      if (type == OAM_F4_LB_END_TO_END_TYPE) 
        pParms->ucCircuitType = CT_OAM_F4_ANY;
      else 
        pParms->ucCircuitType = CT_OAM_F5_END_TO_END;
      /* 16 bytes location ID and 16 bytes source ID */
      memset(ptr+OAM_LB_LOCATION_ID_BYTE_OFFSET,0xff,
             (OAM_LB_LOCATION_ID_LEN+OAM_LB_SRC_ID_LEN)); 
    }
    else {
      /* for segment one, we need to have NODE ID  agreement between
         our modem and the network nodes; for now, I just assume
         we are node src_id_3-0 now and the other end of the segment is locationId_3-0 */
      if (type == OAM_F4_LB_SEGMENT_TYPE) 
        pParms->ucCircuitType = CT_OAM_F4_ANY;
      else 
        pParms->ucCircuitType = CT_OAM_F5_SEGMENT;
      *((int*)(ptr+OAM_LB_LOCATION_ID_BYTE_OFFSET)) = OAM_FAULT_MANAGEMENT_LOCATION_ID_3;
      *((int*)(ptr+OAM_LB_LOCATION_ID_BYTE_OFFSET+4)) = OAM_FAULT_MANAGEMENT_LOCATION_ID_2;
      *((int*)(ptr+OAM_LB_LOCATION_ID_BYTE_OFFSET+8)) = OAM_FAULT_MANAGEMENT_LOCATION_ID_1;
      *((int*)(ptr+OAM_LB_LOCATION_ID_BYTE_OFFSET+12)) = OAM_FAULT_MANAGEMENT_LOCATION_ID_0;
      *((int*)(ptr+OAM_LB_SRC_ID_BYTE_OFFSET)) = OAM_FAULT_MANAGEMENT_SRC_ID_3;
      *((int*)(ptr+OAM_LB_SRC_ID_BYTE_OFFSET+4)) = OAM_FAULT_MANAGEMENT_SRC_ID_2;
      *((int*)(ptr+OAM_LB_SRC_ID_BYTE_OFFSET+8)) = OAM_FAULT_MANAGEMENT_SRC_ID_1;
      *((int*)(ptr+OAM_LB_SRC_ID_BYTE_OFFSET+12)) = OAM_FAULT_MANAGEMENT_SRC_ID_0;
    }
    memset((ptr+OAM_LB_UNUSED_BYTE_OFFSET),OAM_LB_UNUSED_BYTE_DEFAULT,OAM_LB_UNUSED_BYTE_LEN);
    memset((ptr+OAM_LB_CRC_BYTE_OFFSET),0,OAM_LB_CRC_BYTE_LEN);

    pParms->ulStructureId = ID_ATM_VCC_DATA_PARMS;
    pParms->pFnFreeDataParms = (FN_FREE_DATA_PARMS)oamTestFreeDataParmsCb;
    pParms->pAtmBuffer = pAtmBuf;       
    pAtmBuf->pNextAtmBuf = NULL;
    pAtmBuf->pDataBuf = pDataBuf;
    pAtmBuf->ulDataLen = 48;
        
    /* send the request out */
    if (management) 
      baStatus = BcmAtm_SendMgmtData(handle,pVccAddr,pParms);
    else {
      if ((type == OAM_F4_LB_SEGMENT_TYPE) || (type == OAM_F4_LB_END_TO_END_TYPE))
        pParms->ucCircuitType = CT_AAL0_CELL_CRC;
      pParms->ucSendPriority = (UINT8) ANY_PRIORITY;
      baStatus = BcmAtm_SendVccData(handle,pParms);
    }
    if (baStatus != STS_SUCCESS) {
      printk((KERN_CRIT "Unable to send OAM cell over VPI/VCI %d/%d (error %d).\n"),
             pVccAddr->usVpi,pVccAddr->usVci,(int)baStatus);
    }
  }
  else {
    baStatus = STS_ERROR;
  }

  if (baStatus == STS_ERROR) {
    if (pAtmBuf != NULL)
      kfree(pAtmBuf);
    if (pDataBuf != NULL)
      kfree(pDataBuf);
    if (pParms != NULL)
      kfree(pParms);
  }
  return baStatus;
} /* oamTestSend */

static BCMATM_STATUS oamTestReceive(PATM_VCC_DATA_PARMS *ppDpHT, UINT32 type)
{
  BCMATM_STATUS baStatus = STS_ERROR;
  PATM_VCC_DATA_PARMS pDp;
  PATM_BUFFER pAtmBuf=NULL;
  UINT8 *pDataBuf=NULL;

  while( ppDpHT[0] )
    {
      pDp = (PATM_VCC_DATA_PARMS) ppDpHT[0];

      ppDpHT[0] = pDp->pApplicationLink;
      if( ppDpHT[0] == NULL )
        ppDpHT[1] = NULL;

      if (baStatus != STS_SUCCESS) {
        pAtmBuf = pDp->pAtmBuffer;
        if (pAtmBuf != NULL) {
          pDataBuf = pAtmBuf->pDataBuf;
          if (pDataBuf != NULL) {
            if ((pDataBuf[OAM_TYPE_FUNCTION_BYTE_OFFSET] == OAM_FAULT_MANAGEMENT_LB) &&
                (pDataBuf[OAM_LB_INDICATION_BYTE_OFFSET] == OAM_FAULT_MANAGEMENT_LB_RESPOND)) {
              if (*((int*)(pDataBuf+OAM_LB_CORRELATION_TAG_BYTE_OFFSET)) ==
                  OAM_FAULT_MANAGEMENT_CORRELATION_VAL) {
                baStatus = STS_SUCCESS;
              }
            } /* fault management/loopback */
          } /* pDataBuf != NULL */
        }
      } /* !found */
      (*pDp->pFnFreeDataParms) (pDp);
    }
  return (baStatus);
} /* ProcessReceivePkts */

/***************************************************************************
 * Function Name: DoOamLoopbackTest
 * Description  : Send OAM loopback request and expect a response back.
 * Returns      : None; status and statistics are copied to arg.
 ***************************************************************************/
static void DoOamLoopbackTest( unsigned long arg )
{
    ATMDRV_OAM_LOOPBACK KArg;
    BCMATM_STATUS baStatus=STS_ERROR;
    /* the head and tail pointer of rx data */
    PATM_VCC_DATA_PARMS pDpHT[2] = {NULL, NULL};
    UINT32 handle=0, i=0, repetition, type, responseTime = 0;
    UINT32 management=0, sent=0, received=0, min=0, max=0, timeout, total=0;
    ATM_VCC_ADDR vccAddr;
    
    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 ) {
      if ((baStatus = oamTestAttach(&handle,&KArg.VccAddr,KArg.type,
        pDpHT,&management)) != STS_SUCCESS) 
    goto OamLoopbackTestEnd;

      min = KArg.timeout;
      repetition = KArg.repetition;
      type = KArg.type;
      memcpy(&vccAddr,&KArg.VccAddr,sizeof(ATM_VCC_ADDR));
      timeout = KArg.timeout/OAM_LOOPBACK_RX_TIMEOUT_INTERVAL;
      if (timeout == 0)
        timeout = 1;

      for (sent = 0; sent < repetition; sent++) {

    if ((baStatus = oamTestSend(handle,&vccAddr,type,management))
        != STS_SUCCESS)
        continue;
    for (i = 0; i < timeout; i++) {
        if ((baStatus = oamTestReceive(pDpHT,KArg.type)) == STS_SUCCESS)
            break;
    set_current_state(TASK_INTERRUPTIBLE);
    /* sleeps TIMEOUT_INTERVAL msec */
    schedule_timeout(1 + ((HZ*OAM_LOOPBACK_RX_TIMEOUT_INTERVAL)/1000));  
    }

    if ((i == timeout) && (baStatus != STS_SUCCESS)) {
        printk((KERN_CRIT "OAM loopback response not received on VPI/VCI %d/%d.\n"),
            KArg.VccAddr.usVpi,KArg.VccAddr.usVci);
    }
    else {
        received++;
        responseTime = (i*OAM_LOOPBACK_RX_TIMEOUT_INTERVAL);
        total += responseTime;
        if (responseTime < min)
            min = responseTime;
        if (responseTime > max)
            max = responseTime;
    }
    } 
    }
 OamLoopbackTestEnd:
    /* okay, all done now, clean up*/
    if (handle != 0) {
      BcmAtm_Detach(handle);
    }
    //put_user(baStatus, &((PATMDRV_OAM_LOOPBACK) arg)->baStatus);
    /* average response time is (total/received) */
    KArg.baStatus = baStatus;
    KArg.received = received;
    KArg.sent = sent;
    KArg.maxResponseTime = max;
    if (received != 0) {
      KArg.avgResponseTime = total/received;
      KArg.minResponseTime = min;
    }
    else {
      KArg.avgResponseTime = 0;
      KArg.minResponseTime = 0;
    }
    copy_to_user((void*)arg, (void *) &KArg, sizeof(KArg));
} /* DoOamLoopbackTest */

/***************************************************************************
 * Function Name: DoPortMirrorCfg
 * Description  : Send OAM loopback request and expect a response back.
 * Returns      : None; status is copied to arg.
 ***************************************************************************/
static void DoPortMirrorCfg( unsigned long arg )
{
#if 0
    blaa_ioctl ((void *) arg) ;
#else
    MirrorCfg  *pPortMirrorCfg ;
    pPortMirrorCfg = (MirrorCfg *) arg ;

    printk ("blaa_dd: port mirroring config. \n") ;

#if (defined(PORT_MIRROR_DBG))
    printk ("BLAA_DD: Port Mirroring Command - Dir - %d \t Status = %d \t"
            "MirrorPort = %d \n", pPortMirrorCfg->dir,
            pPortMirrorCfg->status, pPortMirrorCfg->mirrorPort) ;
#endif

    if (pPortMirrorCfg->dir == DIR_IN) {
        if (pPortMirrorCfg->status == MIRROR_DISABLED)
            strcpy (mirrorInPort, "") ;
        else {
            blaaUtilSetMirrorPort (pPortMirrorCfg->mirrorPort, mirrorInPort) ;
#if (defined(PORT_MIRROR_DBG))
            printk ("BLAA_DD: mirrorInPort = %s \n", mirrorInPort) ;
#endif
        }
        rfc2684InMirrorStatus = pPortMirrorCfg->status ;
    }
    else if (pPortMirrorCfg->dir == DIR_OUT) {
        if (pPortMirrorCfg->status == MIRROR_DISABLED)
            strcpy (mirrorOutPort, "") ;
        else {
            blaaUtilSetMirrorPort (pPortMirrorCfg->mirrorPort, mirrorOutPort) ;
#if (defined(PORT_MIRROR_DBG))
            printk ("BLAA_DD: mirrorOutPort = %s \n", mirrorOutPort) ;
#endif
        }
        rfc2684OutMirrorStatus = pPortMirrorCfg->status ;
    }
    else {
        printk ("BLAA_DD:- Port mirroring Invalid Direction \n") ;
    }

#if (defined(PORT_MIRROR_DBG))
   printk ("BLAA_DD :- rfc2684InMirrorStatus = %d \n", rfc2684InMirrorStatus) ;
   printk ("BLAA_DD :- rfc2684OutMirrorStatus = %d \n", rfc2684OutMirrorStatus) ;
#endif
#endif
} /* DoPortMirrorCfg */

/***************************************************************************
 * Function that implements a simple loopback test.
 ***************************************************************************/

static BCMATM_STATUS AtmDrv_DoAttach( PATM_VCC_ADDR pAddr,
    PATM_VCC_DATA_PARMS *ppDpHT, UINT32 *pulHandle );
static BCMATM_STATUS AtmDrv_DoSend(UINT32 ulHandle, PATM_VCC_DATA_PARMS *ppDpHT,
    UINT32 ulNumToSend );
static void ProcessReceivePkts( PATM_VCC_DATA_PARMS *ppDpHT,
    UINT32 *pulNumReceived );
static void ReceiveDataCb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, PATM_VCC_DATA_PARMS *ppDpHT );
static void DisplayDataParmsInfo( PATM_VCC_DATA_PARMS pDp, char *pszDirection );
static void FreeDataParmsCb( PATM_VCC_DATA_PARMS pDataParms );

/***************************************************************************
 * Function Name: DoTest
 * Description  : Creates a simple loopback test.
 * Returns      : None.
 ***************************************************************************/
static void DoTest( unsigned long arg )
{
    ATMDRV_TEST KArg;

    if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
    {
        PATM_VCC_DATA_PARMS pDpHT[2] = {NULL, NULL};
        UINT32 ulHandle;
        KArg.baStatus = AtmDrv_DoAttach( &KArg.VccAddr, pDpHT, &ulHandle );
        if( KArg.baStatus == STS_SUCCESS )
        {
            KArg.baStatus = AtmDrv_DoSend( ulHandle, pDpHT, KArg.ulNumToSend );
            BcmAtm_Detach( ulHandle );
        }
        put_user( KArg.baStatus, &((PATMDRV_TEST) arg)->baStatus );
    }
} /* DoTest */


/***************************************************************************
 * Function Name: AtmDrv_DoAttach
 * Description  : Calls BcmAtm_AttachVcc for a loopback test.
 * Returns      : None.
 ***************************************************************************/
static BCMATM_STATUS AtmDrv_DoAttach( PATM_VCC_ADDR pAddr,
    PATM_VCC_DATA_PARMS *ppDpHT, UINT32 *pulHandle )
{
    BCMATM_STATUS baStatus;

    /* Create Attach parameter structure and call BcmAtm_AttachVcc. */
    ATM_VCC_TRANSMIT_QUEUE_PARMS TxQueue =
        {ID_ATM_VCC_TRANSMIT_QUEUE_PARMS, 128, 1, 0};

    ATM_VCC_ATTACH_PARMS AttachParms =
        {ID_ATM_VCC_ATTACH_PARMS, 0, (FN_RECEIVE_CB) ReceiveDataCb, 
         (UINT32) ppDpHT, &TxQueue, 1, 0, 0};

    baStatus = BcmAtm_AttachVcc( pAddr, &AttachParms );

    if( baStatus == STS_SUCCESS )
    {
        *pulHandle = AttachParms.ulHandle;
    }
    else
    {
        printk( "atmapi: ERROR.  BcmAtm_AttachVcc returned " \
            "%lu\n.", (UINT32) baStatus );
    }

    return( baStatus );
} /* AtmDrv_DoAttach */


/***************************************************************************
 * Function Name: AtmDrv_DoSend
 * Description  : Calls BcmAtm_AttachVcc for a loopback test.
 * Returns      : None.
 ***************************************************************************/
static BCMATM_STATUS AtmDrv_DoSend(UINT32 ulHandle, PATM_VCC_DATA_PARMS *ppDpHT,
    UINT32 ulNumToSend )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 i, ulWaitTime;

    for( i = 0; i < ulNumToSend; i++ )
    {
        /* Create Send parameter structure and call BcmAtm_SendVcc. */
        PSTUFF_TO_ALLOC pStuff;

        pStuff = (PSTUFF_TO_ALLOC) kmalloc(sizeof(STUFF_TO_ALLOC), GFP_KERNEL);
        if( pStuff )
        {
            PATM_VCC_DATA_PARMS pParms = &pStuff->DataParms;
            PATM_BUFFER pAtmBuf1 = &pStuff->AtmBuffers[0];
            PATM_BUFFER pAtmBuf2 = &pStuff->AtmBuffers[1];
            UINT8 *pBuf1 = pStuff->ucBuffer1;
            UINT8 *pBuf2 = pStuff->ucBuffer2;
            UINT32 i;

            memset( pStuff, 0x00, sizeof(STUFF_TO_ALLOC) );

            for( i = 0; i < sizeof(pStuff->ucBuffer1); i++ )
                pStuff->ucBuffer1[i] = (UINT8) i & 0xff;
            for( i = 0; i < sizeof(pStuff->ucBuffer2); i++ )
                pStuff->ucBuffer2[i] = (UINT8) i & 0xff;

            pParms->ulStructureId = ID_ATM_VCC_DATA_PARMS;
            pParms->ucFlags = 0;
            pParms->ucSendPriority = (UINT8) ANY_PRIORITY;
            pParms->pFnFreeDataParms = (FN_FREE_DATA_PARMS) FreeDataParmsCb;
            pParms->ulParmFreeDataParms = 0;
            pParms->ucCircuitType = CT_AAL5;
            pParms->ucUuData8 = (UINT8) 0xab;
            pParms->pAtmBuffer = pAtmBuf1;
            pAtmBuf1->pNextAtmBuf = pAtmBuf2;
            pAtmBuf1->pDataBuf = pBuf1;
            pAtmBuf1->ulDataLen = sizeof(pStuff->ucBuffer1);
            pAtmBuf2->pNextAtmBuf = NULL;
            pAtmBuf2->pDataBuf = pBuf2;
            pAtmBuf2->ulDataLen = sizeof(pStuff->ucBuffer2);

            baStatus = BcmAtm_SendVccData( ulHandle, pParms );

            if( baStatus != STS_SUCCESS )
            {
                printk("atmapi: ERROR.  BcmAtm_SendVcc returned %lu\n.",
                    (UINT32) baStatus);
                kfree( pParms );
            }
        }
    }

    /* For up to 5 seconds, try to receive that frames that were just sent. */
    ulWaitTime = AtmOsTickGet();
    i = 0;
    while( i < ulNumToSend )
    {
        AtmOsDelay( 200 );

        if( AtmOsTickCheck( ulWaitTime, 5 * 1000 ) == 1 )
        {
            printk("atmapi: timeout trying to receive loopback frames\n");
            break;
        }

        ProcessReceivePkts( ppDpHT, &i );
    }

    return( baStatus );
} /* AtmDrv_DoSend */


/***************************************************************************
 * Function Name: ProcessReceivePkts
 * Description  : Task that processes receive data.
 * Returns      : None.
 ***************************************************************************/
static void ProcessReceivePkts( PATM_VCC_DATA_PARMS *ppDpHT,
    UINT32 *pulNumReceived )
{
    PATM_VCC_DATA_PARMS pDp;

    while( ppDpHT[0] )
    {
        pDp = (PATM_VCC_DATA_PARMS) ppDpHT[0];

        ppDpHT[0] = pDp->pApplicationLink;
        if( ppDpHT[0] == NULL )
            ppDpHT[1] = NULL;

        (*pulNumReceived)++;

        /* Process the received data which is to display information. */
        DisplayDataParmsInfo( pDp, "Received" );

        /* Return the ATM_VCC_DATA_PARMS structure to the ATM API module. */
        (*pDp->pFnFreeDataParms) (pDp);
    }
} /* ProcessReceivePkts */


/***************************************************************************
 * Function Name: ReceiveDataCb
 * Description  : Receives data from the ATM API module.
 * Returns      : None.
 ***************************************************************************/
static void ReceiveDataCb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, PATM_VCC_DATA_PARMS *ppDpHT )
{
    if( ppDpHT[1] == NULL )
    {
        ppDpHT[0] = ppDpHT[1] = pDp;
    }
    else
    {
        ppDpHT[1]->pApplicationLink = pDp;
        ppDpHT[1] = pDp;
    }
    pDp->pApplicationLink = 0;
} /* ReceiveDataCb */


/***************************************************************************
 * Function Name: DisplayDataParmsInfo
 * Description  : Displays information from a receieved ATM_VCC_DATA_PARMS
 *                structure.
 * Returns      : None.
 ***************************************************************************/
static void DisplayDataParmsInfo( PATM_VCC_DATA_PARMS pDp, char *pszDirection )
{
    static char *pszCircuitTypes[] = { "NOT_DEFINED", "", "CT_AAL0_PACKET",
        "CT_AAL0_CELL_CRC", "CT_OAM_F5_SEGMENT", "CT_OAM_F5_END_TO_END",
        "CT_RM", "CT_AAL5", "CT_AAL2_ALARM", "CT_AAL2_TYPE_3", "CT_AAL2_TYPE_1",
        "CT_AAL2_FRAME", "", "", "", "", "CT_TRANSPARENT" };

    UINT32 i, j, ulLen;
    PATM_BUFFER pAb;

    if( pDp->ucCircuitType<CT_AAL0_PACKET || pDp->ucCircuitType>CT_TRANSPARENT )
        pDp->ucCircuitType = 0; /* NOT_DEFINED  */

    printk( "atmapi: Data %s - Circuit Type: %s\n",
        pszDirection, pszCircuitTypes[pDp->ucCircuitType] );

    printk( "atmapi: Status %lu, AAL2 CID: %lu, UU data 8: %2.2lx,"\
        " UU data 5: %2.2lx, Flags: %2.2lx\n",  (UINT32) pDp->baReceiveStatus,
        (UINT32) pDp->ucAal2ChannelId, (UINT32) pDp->ucUuData8,
        (UINT32) pDp->ucUuData5, (UINT32) pDp->ucFlags );

    i = 1;
    ulLen = 0;
    pAb = pDp->pAtmBuffer;
    while( pAb && pAb->pDataBuf )
    {
        ulLen = (pAb->ulDataLen > 16) ? 16: pAb->ulDataLen;
        printk( "atmapi: Buf%lu len=%lu, data=", i++,
            pAb->ulDataLen );
        for( j = 0; j < ulLen; j++ )
            printk( "%2.2x ", pAb->pDataBuf[j] );
        printk( "\n" );
        pAb = pAb->pNextAtmBuf;
    }

    printk( "\n" );
} /* DisplayDataParmsInfo */


/***************************************************************************
 * Function Name: FreeDataParmsCb
 * Description  : Called by ATM API module after it has processed a send
 *                data buffer that contained a packet.
 * Returns      : None.
 ***************************************************************************/
static void FreeDataParmsCb( PATM_VCC_DATA_PARMS pDataParms )
{
    /* The ATM_VCC_DATA_PARMS structure and associated buffers were all
     * allocated in one block pointed by pDataParms.  Thus, freeing the
     * pDataParms address actually frees all memory that is assocated with it.
     */
    kfree( pDataParms );
} /* StaticFreeDataParmsCb */

/***************************************************************************
 * Function Name: AdslLedCtrl
 * Description  : Callback function that is called when by the LED driver
 *                to handle the ADSL hardware LED.
 * Returns      : None.
 ***************************************************************************/
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358)
/* ALC_LINK is active high */
static void AdslLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    switch (ledState)
    {
        case kLedStateOn:
            AP_REGS->ulAdslLedCtrl = ALC_EN | ALC_LINK;
            break;

        case kLedStateOff: 
            AP_REGS->ulAdslLedCtrl = 0;
            break;

        case kLedStateSlowBlinkContinues:
        case kLedStateFastBlinkContinues:
            AP_REGS->ulAdslLedCtrl = ALC_EN | ALC_MODE_MELODY_LINK;
            break;

        default:
            break;
    }
}
#elif defined(CONFIG_BCM96338)
/* ATMR_LINK is active low */
static void AdslLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    AP_REGS->ulRxAalCfg &= ATMR_RX_AAL5_SW_TRAILER_EN;
    switch (ledState)
    {
        case kLedStateOn:
            AP_REGS->ulRxAalCfg |= ATMR_EN;
            AP_REGS->ulRxAalCfg &= ~(ATMR_MODE_MELODY_LINK |
                ATMR_LED_SPEED_125MS | ATMR_LINK);
            break;

        case kLedStateOff: 
            AP_REGS->ulRxAalCfg |= ATMR_EN | ATMR_LINK;
            break;

        case kLedStateSlowBlinkContinues:
        case kLedStateFastBlinkContinues:
            AP_REGS->ulRxAalCfg |= ATMR_EN | ATMR_MODE_MELODY_LINK |
                ATMR_LED_SPEED_125MS;
            break;

        default:
            break;
    }
}
#endif

/***************************************************************************
 * Function Name: WanDataLedCtrl
 * Description  : Callback function that is called when by the LED driver
 *                to handle the ADSL hardware LED.
 * Returns      : None.
 ***************************************************************************/
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358)
/* ALC_LINK is active high */
static void WanDataLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    switch (ledState)
    {
        case kLedStateOn:
            AP_REGS->ulAdslLedCtrl |= ALC_EN | ALC_LINK |
                ALC_MODE_LINK_CELL_ACTIVITY;
            break;

        case kLedStateOff: 
            AP_REGS->ulAdslLedCtrl &= ALC_LED_SPEED_MASK;
            break;

        case kLedStateSlowBlinkContinues:
            AP_REGS->ulAdslLedCtrl = ALC_EN | ALC_LINK |
                ALC_MODE_CELL_ACTIVITY | ALC_LED_SPEED_250MS;
            break;

        case kLedStateFastBlinkContinues:
            AP_REGS->ulAdslLedCtrl = ALC_EN | ALC_LINK |
                ALC_MODE_CELL_ACTIVITY | ALC_LED_SPEED_125MS;
            break;

        default:
            break;
    }
}
#elif defined(CONFIG_BCM96338)
/* ATMR_LINK is active low */
static void WanDataLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    AP_REGS->ulRxAalCfg &= (ATMR_RX_AAL5_SW_TRAILER_EN | ATMR_LED_SPEED_MASK);
    switch (ledState)
    {
        case kLedStateOn:
            AP_REGS->ulRxAalCfg |= ATMR_EN | ALC_MODE_LINK_CELL_ACTIVITY;
            AP_REGS->ulRxAalCfg &= ~ATMR_LINK;
            break;

        case kLedStateOff: 
            AP_REGS->ulRxAalCfg |= ATMR_EN | ATMR_LINK;
            break;

        case kLedStateSlowBlinkContinues:
            AP_REGS->ulRxAalCfg &= ~(ATMR_LINK | ATMR_LED_SPEED_MASK);
            AP_REGS->ulRxAalCfg |= ATMR_EN | ATMR_LED_SPEED_250MS;
            break;

        case kLedStateFastBlinkContinues:
            AP_REGS->ulRxAalCfg &= ~(ATMR_LINK | ATMR_LED_SPEED_MASK);
            AP_REGS->ulRxAalCfg |= ATMR_EN | ATMR_LED_SPEED_125MS;
            break;

        default:
            break;
    }
}
#endif

/***************************************************************************
 * Function Name: blaaUtilSetMirrorPort
 * Description  : Called in from the IOCTL routine handler to set the
 *                mirror port string name.
 * Returns      : None.
 ***************************************************************************/
void blaaUtilSetMirrorPort (int mirrorPort, char *pMirrorPort)
{
    if ((mirrorPort >= MIRROR_PORT_ETH0) && (mirrorPort <= MIRROR_PORT_USB)) {

        strcpy (pMirrorPort, blaaMirrorDevNames [mirrorPort-1]) ;
    }
    else 
        strcpy (pMirrorPort, blaaMirrorDevNames [0]) ;

    return ;
} /* blaaUtilSetMirrorPort */


/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( atmapi_init );
module_exit( atmapi_cleanup );
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(BcmAtm_Initialize);
EXPORT_SYMBOL(BcmAtm_Uninitialize);
EXPORT_SYMBOL(BcmAtm_GetInterfaceId);
EXPORT_SYMBOL(BcmAtm_GetPriorityPacketGroup);
EXPORT_SYMBOL(BcmAtm_SetPriorityPacketGroup);
EXPORT_SYMBOL(BcmAtm_GetTrafficDescrTableSize);
EXPORT_SYMBOL(BcmAtm_GetTrafficDescrTable);
EXPORT_SYMBOL(BcmAtm_SetTrafficDescrTable);
EXPORT_SYMBOL(BcmAtm_GetInterfaceCfg);
EXPORT_SYMBOL(BcmAtm_SetInterfaceCfg);
EXPORT_SYMBOL(BcmAtm_GetVccCfg);
EXPORT_SYMBOL(BcmAtm_SetVccCfg);
EXPORT_SYMBOL(BcmAtm_GetVccAddrs);
EXPORT_SYMBOL(BcmAtm_GetInterfaceStatistics);
EXPORT_SYMBOL(BcmAtm_GetVccStatistics);
EXPORT_SYMBOL(BcmAtm_SetInterfaceLinkInfo);
EXPORT_SYMBOL(BcmAtm_SetNotifyCallback);
EXPORT_SYMBOL(BcmAtm_ResetNotifyCallback);
EXPORT_SYMBOL(BcmAtm_AttachVcc);
EXPORT_SYMBOL(BcmAtm_AttachMgmtCells);
EXPORT_SYMBOL(BcmAtm_AttachTransparent);
EXPORT_SYMBOL(BcmAtm_Detach);
EXPORT_SYMBOL(BcmAtm_SetAal2ChannelIds);
EXPORT_SYMBOL(BcmAtm_SendVccData);
EXPORT_SYMBOL(BcmAtm_SendMgmtData);
EXPORT_SYMBOL(BcmAtm_SendTransparentData);
EXPORT_SYMBOL(g_pfnAdslSetVcEntry);
EXPORT_SYMBOL(g_pfnAdslSetAtmLoopbackMode);
EXPORT_SYMBOL(g_pfnAdslSetVcEntryEx);
EXPORT_SYMBOL(g_pfnAdslAtmClearVcTable);
EXPORT_SYMBOL(g_pfnAdslAtmAddVc);
EXPORT_SYMBOL(g_pfnAdslAtmDeleteVc);
EXPORT_SYMBOL(g_pfnAdslAtmSetMaxSdu);
EXPORT_SYMBOL(AtmOsDelay);
EXPORT_SYMBOL(AtmOsCreateThread);
EXPORT_SYMBOL(AtmOsCreateSem);
EXPORT_SYMBOL(AtmOsRequestSem);
EXPORT_SYMBOL(AtmOsReleaseSem);
EXPORT_SYMBOL(AtmOsDeleteSem);
EXPORT_SYMBOL(rfc2684InMirrorStatus);
EXPORT_SYMBOL(rfc2684OutMirrorStatus);
EXPORT_SYMBOL(mirrorInPort);
EXPORT_SYMBOL(mirrorOutPort);
/* start of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS
EXPORT_SYMBOL(g_ulLinkUpStreamRate);
EXPORT_SYMBOL(g_ulQosHWeight);
EXPORT_SYMBOL(g_ulQosLWeight);
#endif
#endif
/* end of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */

