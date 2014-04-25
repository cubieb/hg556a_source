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
 * File Name  : blaadd.c (Broadcom Linux Atm Api Device Driver)
 *
 * Description: This file contains Linux ATM API device driver entry point
 *              functions.  It calls the Broadcom ATM API driver to carry
 *              out the operation.
 *
 *              The Broadcom ATM API initialization functions BcmAtm_Initialize,
 *              BcmAtm_SetTrafficDescrTable, BcmAtm_SetInterfaceCfg,
 *              BcmAtm_SetVccCfg and BcmAtm_SetInterfaceLinkInfo must be
 *              called before a Linux socket application can connect and
 *              send / receive data.
 *
 * Updates    : 09/06/2001  lat.  Created.
 *            : 10/28/2004  songw. Redesigned BlaaFreeDataParam using callback.
 *                                 Change to use FreeRxHdrList to optimze
 ***************************************************************************/


/* Includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/atm.h>
#include <linux/ip.h>
#include <linux/atmdev.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/if_arp.h>
#include <linux/param.h>

#include <bcmtypes.h>
#include <bcmatmapi.h>
#include <bcm_common.h>

#include <BcmBlaaMirror.h>
/* start of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS // macro QoS for KPN
#include <atmapidrv.h>
#endif
#endif
/* end of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */

#define BLAA_DEFAULT_NUM_TX_HDRS        64
#define BLAA_NUM_RX_HDRS                200

#define MIB_INFO_TIMEOUT                (HZ * 3)

#define MODULE_NOT_IN_USE               0
#define MODULE_IN_USE                   1
#define MODULE_CLOSING                  2

#define SKB_ALIGNED_SIZE                ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)

#if 0
#define DPRINTK(format,args...) printk(KERN_DEBUG format,##args)
#else
#define DPRINTK(format,args...)
#endif
/* start of protocol QoS for KPN加入优先级判断的宏定义 by zhangchen z45221 2006年7月17日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS   // macro QoS for KPN
#define RATE_VARIABLE_SCOPE             20  // 速率波动范围20kByteps
#define PRIORITY_SET_TO_ONE             0   //  将优先级2置为1
#define PRIORITY_NO_CHANGE              1   // 优先级不变
#define PRIORITY_EXCHANGE               2   // 优先级1,2交换
#endif
#endif
/* end of protocol QoS for KPN加入优先级判断的宏定义 by zhangchen z45221 2006年7月17日 */

/* Typedefs. */
struct _BLAA_RX_INFO;
struct _BLAA_RX_HDR
{
    ATM_VCC_DATA_PARMS *pDataParms;
    struct _BLAA_RX_HDR *nextFree;
    struct _BLAA_RX_INFO *pRxInfo;
}; 

typedef struct _BLAA_RX_HDR BLAA_RX_HDR, *PBLAA_RX_HDR;

typedef struct _BLAA_RX_INFO
{
    UINT32 ulModuleInUse;
    PBLAA_RX_HDR FreeRxHdrList;    
    BLAA_RX_HDR RxHdrs[BLAA_NUM_RX_HDRS];
    struct sk_buff *pFreeSockBufList;
    char SockBufs[(BLAA_NUM_RX_HDRS * SKB_ALIGNED_SIZE) + 0x10];
} BLAA_RX_INFO, *PBLAA_RX_INFO;

typedef struct
{
    ATM_VCC_DATA_PARMS DataParms;
    ATM_BUFFER AtmBuf;
    struct list_head List;
} BLAA_TX_HDR, *PBLAA_TX_HDR;

typedef struct
{
    struct list_head TxFreeHdrList;
    UINT32 ulAttachHandle;
    UINT32 ulNumTxHdrs;
    PBLAA_TX_HDR pTxHdrs;
} BLAA_VCC_INFO, *PBLAA_VCC_INFO;

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
extern int g_nPtmConfig;
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/* start of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS   // macro QoS for KPN
extern unsigned int g_ulLinkUpStreamRate;
extern unsigned int g_ulQosHWeight;
extern unsigned int g_ulQosLWeight;
#endif
#endif
/* end of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */

/* Prototypes. */
static int __init blaa_detect( void );
static void __exit blaa_cleanup( void );
static int blaa_open(struct atm_vcc *pVcc);
static PBLAA_VCC_INFO AllocVccInfo(struct atm_vcc *pVcc, PATM_VCC_ADDR pVccAddr);
static int AttachVcc( UINT32 ulAalType, UINT16 usVpi, UINT16 usVci,
    PATM_VCC_ADDR pVccAddr, PATM_VCC_ATTACH_PARMS pAttachParms );
static void blaa_close( struct atm_vcc *pVcc );
static int blaa_setsockopt(struct atm_vcc *pVcc, int nLevel, int nOptName,
    void *pvOptVal, int nOptLen);
static int blaa_send(struct atm_vcc *pVcc, struct sk_buff *pSockBuf );
static void BlaaFreeDpCb( PATM_VCC_DATA_PARMS pDataParms );
static void BlaaRcvCb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, struct atm_vcc *pVcc );
static void BlaaMibInfoTo( UINT32 ulNotUsed );
static int MapError( BCMATM_STATUS baStatus );

//#define LOOPBACK_TEST
#ifdef LOOPBACK_TEST
unsigned int inFrameCount = 0, outFrameCount = 0 ;
void blaaSendInFrame (struct atm_vcc *pVcc, struct sk_buff *pSockBuf, unsigned int len);
#endif

/* Globals. */
static const struct atmdev_ops g_Ops =
{
    .open       = blaa_open,
    .close      = blaa_close,
    .setsockopt = blaa_setsockopt,
    .send       = blaa_send,
};

static struct atm_dev *g_pAtmDev = NULL;
static PBLAA_RX_INFO g_pRxInfo = NULL;
static struct net_device *g_pAtmNetDev = NULL;
static struct net_device *g_pCpcsNetDev = NULL;
static struct timer_list g_MibInfoTimer;

/* start of protocol QoS by zhangchen z45221 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS    // macro QoS for KPN
unsigned int g_ulHPrioPktLen    = 0;                // high and medium nfmark & 0x0f = 0x04 or 0x03
unsigned int g_ulHWeightPktLen  = 0;                // low priority and high weight nfmark & 0x0F = 0x02
unsigned int g_ulLWeightPktLen  = 0;                // low priority and low  weight nfmark & 0x0f = 0x01
unsigned int g_ulOtherPktLen    = 0;                
unsigned int g_ulTotalPktLen    = 0;                 

unsigned int g_ulHWeightActuLen = 0;                // 高权值的实际发送长度
unsigned int g_ulLWeightActuLen = 0;                // 低权值的实际发送长度

unsigned int g_ulHPrioPktRate   = 0;
unsigned int g_ulHWeightPktRate = 0;           
unsigned int g_ulLWeightPktRate = 0;           
unsigned int g_ulTotalPktRate   = 0;               
unsigned int g_ulHWeightPRate   = 0;                  
unsigned int g_ulLWeightPRate   = 0;                  

unsigned int g_ulHWeightActuRate = 0;           //高权值的实际发送速率
unsigned int g_ulLWeightActuRate = 0;           //低权值的实际发送速率
#endif
#endif
/* end of protocol QoS by zhangchen z45221 2006年7月11日 */

extern int rfc2684InMirrorStatus ;
extern int rfc2684OutMirrorStatus ;
extern char mirrorInPort  [MIRROR_NAME_MAX_LEN] ;
extern char mirrorOutPort [MIRROR_NAME_MAX_LEN] ;

/***************************************************************************
 * Function Name: atm_config_netdev
 * Description  : Configure a network device for ATM interface
 * Returns      : None.
 ***************************************************************************/

struct net_device_stats atm_if_stats;

static struct net_device_stats *atm_get_stats(struct net_device *dev)
{
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

    return &atm_if_stats;
}

static void atm_setup( struct net_device *dev ) {
        
       dev->type = ARPHRD_ATM;
       dev->mtu = 0;
       dev->get_stats = atm_get_stats;
       dev->trans_start = jiffies * 100 / HZ;        
}        

static int atm_config_netdev( void ) {

    g_pAtmNetDev = alloc_netdev(0, ATM_CELL_LAYER_IFNAME,  atm_setup);
    if (g_pAtmNetDev)
       return register_netdev(g_pAtmNetDev);
    else 
        return -ENOMEM;
}

/***************************************************************************
 * Function Name: cpcs_config_netdev
 * Description  : Configure a network device for CPCS interface
 * Returns      : None.
 ***************************************************************************/

struct net_device_stats cpcs_if_stats;

static struct net_device_stats *cpcs_get_stats(struct net_device *dev)
{
    UINT32 interfaceId;
    ATM_INTERFACE_STATS KStats;
    ATM_INTERFACE_CFG Cfg;
    PATM_INTF_AAL5_AAL0_STATS pAal5Stats = &KStats.Aal5IntfStats;

    BcmAtm_GetInterfaceId(0, &interfaceId);
    Cfg.ulStructureId = ID_ATM_INTERFACE_CFG;
    KStats.ulStructureId = ID_ATM_INTERFACE_STATS;
    if ( BcmAtm_GetInterfaceCfg(interfaceId, &Cfg)==STS_SUCCESS &&
            BcmAtm_GetInterfaceStatistics(interfaceId, &KStats, 0)==STS_SUCCESS)
    {
        cpcs_if_stats.rx_bytes = (unsigned long) pAal5Stats->ulIfInOctets;
        cpcs_if_stats.tx_bytes = (unsigned long) pAal5Stats->ulIfOutOctets;
        cpcs_if_stats.rx_errors = (unsigned long) pAal5Stats->ulIfInErrors;
        cpcs_if_stats.tx_errors = (unsigned long) pAal5Stats->ulIfOutErrors;
    }
    return &cpcs_if_stats;

}

static void cpcs_setup( struct net_device *dev) {
        
       dev->type = ARPHRD_CPCS;
       dev->mtu = 0xFFFF;
       dev->get_stats = cpcs_get_stats;
       dev->trans_start = jiffies * 100 / HZ;        
}        
static int cpcs_config_netdev(void) {

    g_pCpcsNetDev = alloc_netdev(0, AAL5_CPCS_LAYER_IFNAME, cpcs_setup);
    if (g_pCpcsNetDev)
       return register_netdev(g_pCpcsNetDev);
    else
       return -ENOMEM;
}


/***************************************************************************
 * Function Name: AllocRxInfo
 * Description  : Allocates and initializes receive data structures.
 * Returns      : Address of BLAA_RX_INFO structure or NULL
 ***************************************************************************/
PBLAA_RX_INFO __init AllocRxInfo( void )
{
    PBLAA_RX_INFO pRxInfo = kmalloc( sizeof(BLAA_RX_INFO), GFP_KERNEL );

    if( pRxInfo )
    {
        UINT32 i;
        PBLAA_RX_HDR pRxHdr;
        struct sk_buff *pSockBuf;
        unsigned char *p;

        memset( pRxInfo, 0x00, sizeof(BLAA_RX_INFO) );

        /* Initialize RxHdrs. */
        for(i = 0, pRxHdr = pRxInfo->RxHdrs; i < BLAA_NUM_RX_HDRS;i++,pRxHdr++)
        {
            pRxHdr->pRxInfo = pRxInfo;
            pRxHdr->nextFree = pRxHdr+1;
        }
        pRxInfo->RxHdrs[BLAA_NUM_RX_HDRS-1].nextFree = NULL;
        pRxInfo->FreeRxHdrList = pRxInfo->RxHdrs;

        /* Chain socket buffers. */
        for( i = 0, p = (unsigned char *)
             (((unsigned long) pRxInfo->SockBufs + 0x0f) & ~0x0f);
             i < BLAA_NUM_RX_HDRS; i++, p += SKB_ALIGNED_SIZE )
        {
            pSockBuf = (struct sk_buff *) p;
            pSockBuf->retfreeq_context = pRxInfo->pFreeSockBufList;
            pRxInfo->pFreeSockBufList = pSockBuf;
        }

        /* Initialize and start a timer that checks for the upper layer (bridge,
         * TCP/IP) being done with a receive buffer so it can be returned to the
         * Broadcom ATM API buffer pool.
         */
        pRxInfo->ulModuleInUse = MODULE_IN_USE;
    }

    return( pRxInfo );
} /* AllocRxInfo */


/***************************************************************************
 * Function Name: blaa_init
 * Description  : Initial function.
 * Returns      : 1 device.
 ***************************************************************************/
static int __init blaa_detect( void )
{
    int ret;
    printk( "blaadd: blaa_detect entry\n" );

    if( (g_pAtmDev = atm_dev_register("bcmatm", &g_Ops, -1, NULL)) != NULL )
    {
        if( (g_pRxInfo = AllocRxInfo()) != NULL )
        {
            g_pAtmDev->ci_range.vpi_bits = 12;
            g_pAtmDev->ci_range.vci_bits = 16;

            if ( atm_config_netdev() )
            {
                printk( "blaadd: failed to create %s\n", ATM_CELL_LAYER_IFNAME);
                return( -1 );
            }

            if ( cpcs_config_netdev() )
            {
                printk( "blaadd: failed to create %s\n", AAL5_CPCS_LAYER_IFNAME);
                return( -1 );
            }

            init_timer( &g_MibInfoTimer );
            g_MibInfoTimer.expires = jiffies + MIB_INFO_TIMEOUT;
            g_MibInfoTimer.function = BlaaMibInfoTo;
            g_MibInfoTimer.data = 0;
            add_timer( &g_MibInfoTimer );

            ret = 0;
        }
        else
            ret = -1;
    }
    else
        ret = -1;

    return( ret );
} /* blaa_detect */


/***************************************************************************
 * Function Name: blaa_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit blaa_cleanup( void )
{
    printk( "blaadd: blaa_cleanup entry\n" );

    if( g_pRxInfo )
    {
        kfree( g_pRxInfo );
        g_pRxInfo = NULL;
    }

    if( g_pAtmDev )
    {
        del_timer( &g_MibInfoTimer );
        atm_dev_deregister( g_pAtmDev );
        g_pAtmDev = NULL;
    }
} /* blaa_cleanup */


/***************************************************************************
 * Function Name: blaa_open
 * Description  : This function is called when an application wants to use
 *                a VCC.
 * Returns      : None.
 ***************************************************************************/
static int blaa_open(struct atm_vcc *pVcc)
{
    int nRet = 0;
    short sVpi = pVcc->vpi;
    int nVci = pVcc->vci;

    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    static UINT32 ulHandle = 0;
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    DPRINTK( "blaadd: blaa_open entry, vpi=%d, vci=%d\n", sVpi, nVci );

    
    pVcc->dev_data = NULL;
    if( pVcc->qos.aal == ATM_AAL5 || pVcc->qos.aal == ATM_AAL2 ||
        pVcc->qos.aal == ATM_BCM_AAL0 )
    {
        if( nVci != ATM_VPI_UNSPEC && sVpi != ATM_VCI_UNSPEC )
        {
            ATM_VCC_ADDR VccAddr;
#define ATMCTL_SET_TRANSMITQUEUE
#ifdef ATMCTL_SET_TRANSMITQUEUE
            ATM_VCC_ATTACH_PARMS AttachParms =
                { ID_ATM_VCC_ATTACH_PARMS, 0, (FN_RECEIVE_CB) BlaaRcvCb,
                (UINT32) pVcc, NULL, 1, 0, 0 };
#else
            ATM_VCC_TRANSMIT_QUEUE_PARMS TxQ =
                {ID_ATM_VCC_TRANSMIT_QUEUE_PARMS, BLAA_DEFAULT_NUM_TX_HDRS,1,0};
            ATM_VCC_ATTACH_PARMS AttachParms =
                { ID_ATM_VCC_ATTACH_PARMS, 0, (FN_RECEIVE_CB) BlaaRcvCb,
                (UINT32) pVcc, &TxQ, 1, 0, 0 };
#endif


            UINT32 ulAalType;

            switch( pVcc->qos.aal )
            {
            case ATM_AAL5:
                ulAalType = AAL_5;
                break;

            case ATM_AAL2:
                ulAalType = AAL_2;
                AttachParms.ulFlags = AVAP_DSP;
                break;

            case ATM_BCM_AAL0:
                ulAalType = AAL_0_CELL_CRC;
                break;

            default:
                ulAalType = (UINT32) -1;
                break;
            }

            if( (nRet = AttachVcc( ulAalType, (USHORT) sVpi,
                (USHORT) (nVci & 0xffff), &VccAddr, &AttachParms )) == 0 )
            {
                PBLAA_VCC_INFO pVccInfo = AllocVccInfo( pVcc, &VccAddr );
                if( pVccInfo )
                {
                    pVccInfo->ulAttachHandle = AttachParms.ulHandle;
                    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
                    #ifdef CONFIG_BCM_VDSL
                    printk("AttachVcc g_nPtmConfig:%d \r\n", g_nPtmConfig);
                    /* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
                    if ((g_nPtmConfig == PTM_TRAFFIC) || (g_nPtmConfig == PTM_TRAFFIC_PRTCL) || (g_nPtmConfig == PTM_TRAFFIC_BR))
                    {
                        if(ulHandle == 0)
                          ulHandle  = pVccInfo->ulAttachHandle;
                        else
                        	 pVccInfo->ulAttachHandle = ulHandle;
                    }
                    /* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
                    #endif
                    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
                    pVcc->vpi = (short) VccAddr.usVpi;
                    pVcc->vci = (int) VccAddr.usVci;
                    pVcc->dev_data = pVccInfo;
                    pVcc->sk->sk_sndbuf = 0x7fffffff;
                    pVcc->sk->sk_rcvbuf = 0x7fffffff;

                    /* Indicate readiness. */
                    set_bit(ATM_VF_READY,&pVcc->flags);
                }
                else
                {
                    nRet = -ENOMEM;
                    kfree( pVccInfo );
                }
            }
        }
        else
            nRet = -EAFNOSUPPORT;
    }
    else
    {
        /* TBD - support ATM_AAL0 using BcmAtm_AttachTransparent */
        nRet = -EINVAL;
    }

    if( nRet != 0 )
        printk( "blaadd: open error %d\n", nRet );

    /* If an error is returned and this device is configured for bridging,
     * an exception occurs.  Therefore, always return success.
     */
    return( 0 );
} /* blaa_open */


/***************************************************************************
 * Function Name: AllocVccInfo
 * Description  : This function allocates and initializes a BLAA_VCC_INFO
 *                structure.
 * Returns      : Pointer to an allocates BLAA_VCC_INFO structure or NULL.
 ***************************************************************************/
static PBLAA_VCC_INFO AllocVccInfo(struct atm_vcc *pVcc, PATM_VCC_ADDR pVccAddr)
{
    PBLAA_VCC_INFO pVccInfo = NULL;
    PBLAA_TX_HDR pTxHdr;
    UINT32 i, ulSize = 0;
    UINT32 ulNumTxHdrs = 0;
    ATM_VCC_CFG VccCfg = {ID_ATM_VCC_CFG};

    /* Calculate the number transmit header structures to allocate.  The number
     * is the sum of the size of all queues used by the VCC.
     */
    if( BcmAtm_GetVccCfg(pVccAddr, &VccCfg) == STS_SUCCESS )
    {
        if( VccCfg.ulTransmitQParmsSize == 0 )
            ulNumTxHdrs = BLAA_DEFAULT_NUM_TX_HDRS;
        else
        {
            for( i = 0; i < VccCfg.ulTransmitQParmsSize; i++ )
                ulNumTxHdrs += VccCfg.TransmitQParms[i].ulSize;
        }

        ulSize = sizeof(BLAA_VCC_INFO) + (ulNumTxHdrs * sizeof(BLAA_TX_HDR));
        pVccInfo = kmalloc( ulSize, GFP_KERNEL);
    }

    if( pVccInfo )
    {
        memset( pVccInfo, 0x00, ulSize );

        pVccInfo->pTxHdrs = (PBLAA_TX_HDR) (pVccInfo + 1);
        pVccInfo->ulNumTxHdrs = ulNumTxHdrs;

        INIT_LIST_HEAD(&pVccInfo->TxFreeHdrList);

        for( i = 0, pTxHdr = pVccInfo->pTxHdrs; i < pVccInfo->ulNumTxHdrs;
             i++, pTxHdr++ )
        {
            list_add_tail(&pTxHdr->List, &pVccInfo->TxFreeHdrList);
            pTxHdr->DataParms.ulStructureId = ID_ATM_VCC_DATA_PARMS;

            if( pVcc->qos.aal == ATM_BCM_AAL0 )
                pTxHdr->DataParms.ucCircuitType = CT_AAL0_CELL_CRC;
            else
                pTxHdr->DataParms.ucCircuitType = CT_AAL5;

            pTxHdr->DataParms.ucSendPriority = 1;
            pTxHdr->DataParms.pAtmBuffer = &pTxHdr->AtmBuf;
            pTxHdr->DataParms.pFnFreeDataParms = BlaaFreeDpCb;
        }
    }

    return( pVccInfo );
} /* AllocVccInfo */


/***************************************************************************
 * Function Name: AttachVcc
 * Description  : This function resolves wildcards, if any, in the VPI and VCI
 *                and calls BcmAtm_AttachVcc to reserve the VCC.
 *
 *                Wildard addresses are resolved differently than the standard
 *                Linux atm_find_ci function that other Linux ATM device drivers
 *                use.  The atm_find_ci function finds a vpi/vci that is within
 *                a valid range and not already being used.  This function finds
 *                a vpi/vci that has been configured in the Broadcom ATM API
 *                driver and is not already being used.
 *
 * Returns      : 0 if success or error.
 ***************************************************************************/
static int AttachVcc( UINT32 ulAalType, UINT16 usVpi, UINT16 usVci,
    PATM_VCC_ADDR pVccAddr, PATM_VCC_ATTACH_PARMS pAttachParms )
{
    int nRet;
    UINT32 i, ulNumConfiguredVccs, ulAddrsReturned, ulInterfaceId, ulAddrIdx;
    UINT16 usLowVpi;
    UINT32 ulLowVci;
    PATM_VCC_ADDR pAddr, pAddrs, pSaveAddr;
    ATM_VCC_CFG VccCfg = {ID_ATM_VCC_CFG};
    BCMATM_STATUS baStatus;

    DPRINTK("blaad: AttachVcc enter vpi=%lu, vci=%lu\n", (UINT32) usVpi, \
        (UINT32) usVci);

    /* Determine how many configured VCCs. */
    for( i = 0, ulNumConfiguredVccs = 0; i < PHY_NUM_PORTS; i++ )
    {
        ulAddrsReturned = ulInterfaceId = 0;
        if( BcmAtm_GetInterfaceId( i, &ulInterfaceId ) == STS_SUCCESS )
        {
            BcmAtm_GetVccAddrs( ulInterfaceId, NULL, 0, &ulAddrsReturned );
            ulNumConfiguredVccs += ulAddrsReturned;
        }
    }

    /* Allocate memory for configured VCC addresses. */
    pAddrs = kmalloc( sizeof(ATM_VCC_ADDR) * ulNumConfiguredVccs, GFP_KERNEL );

    if( pAddrs )
    {
        /* Read all configured VCC addresses */
        for( i = 0, ulAddrIdx = 0; i < PHY_NUM_PORTS; i++ )
        {
            if( BcmAtm_GetInterfaceId( i, &ulInterfaceId ) == STS_SUCCESS )
            {
                BcmAtm_GetVccAddrs( ulInterfaceId, pAddrs + ulAddrIdx,
                    ulNumConfiguredVccs - ulAddrIdx, &ulAddrsReturned );
                ulAddrIdx += ulAddrsReturned;
            }
        }

        nRet = -1;

        /* Try to attach (by calling BcmAtm_AttachVcc) to a configured VCC
         * that matches the specified VPI/VCI or wildcard.
         */
        while( nRet == -1 )
        {
            pSaveAddr = NULL;

            /* Find the lowest VPI */
            if( usVpi == (UINT16) ATM_VPI_ANY )
            {
                usLowVpi = 0xfffe;
                for(i = 0, pAddr = pAddrs; i < ulNumConfiguredVccs; i++,pAddr++)
                {
                    if( usLowVpi > pAddr->usVpi  )
                        usLowVpi = pAddr->usVpi;
                }
            }
            else
                usLowVpi = usVpi;

            /* Find the lowest VCI for the VPI */
            ulLowVci = 0xffffffff;
            for(i = 0, pAddr = pAddrs; i < ulNumConfiguredVccs; i++,pAddr++)
            {
                if( usVci == (UINT16) ATM_VCI_ANY )
                {
                    if( usLowVpi == pAddr->usVpi && ulLowVci > pAddr->usVci  )
                    {
                        ulLowVci = pAddr->usVci;
                        memcpy(pVccAddr,pAddr,sizeof(ATM_VCC_ADDR));
                        pSaveAddr = pAddr;
                    }
                }
                else
                    if( usLowVpi == pAddr->usVpi && usVci == pAddr->usVci  )
                    {
                        ulLowVci = pAddr->usVci;
                        memcpy(pVccAddr,pAddr,sizeof(ATM_VCC_ADDR));
                        pSaveAddr = pAddr;
                        break;
                    }
            }

            /* Stop if a VPI or VCI could not be found. */
            if( usLowVpi != 0xfffe && ulLowVci != 0xffffffff )
            {
                /* Get the configuration record for the current VCC address
                 * being checked.
                 */

                if( BcmAtm_GetVccCfg(pVccAddr, &VccCfg) == STS_SUCCESS )
                {
                    /* Verify that the configured VCC found is the correct
                     * type.
                     */
                    if( VccCfg.ulAalType == ulAalType  )
                    {
                        /* Attach to the VCC. */
                        baStatus = BcmAtm_AttachVcc( pVccAddr, pAttachParms );
                        switch( baStatus )
                        {
                        case STS_SUCCESS:
                            /* VCC is successfully attached. */
                            nRet = 0;
                            DPRINTK("blaad: AttachVcc done vpi=%lu, vci=%lu\n",\
                                (UINT32) pVccAddr->usVpi, \
                                (UINT32) pVccAddr->usVci);
                            break;

                        case STS_IN_USE:
                            if( usVpi == (UINT16) ATM_VPI_ANY ||
                                usVci == (UINT16) ATM_VCI_ANY )
                            {
                                /* Make the address invalid and continue. */
                                pSaveAddr->usVci = 0xffff;
                            }
                            else
                            {
                                /* A VCC to use is not found. */
                                /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
                                #ifdef CONFIG_BCM_VDSL
                                /* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
                                if ((g_nPtmConfig == PTM_TRAFFIC) || (g_nPtmConfig == PTM_TRAFFIC_PRTCL) || (g_nPtmConfig == PTM_TRAFFIC_BR))
                                {
                                    nRet = 0;
                                }
                                else
                                {
                                    nRet = -EADDRINUSE;
                                }
                                /* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
                                #else
                                    nRet = -EADDRINUSE;
                                #endif
                                /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
                            }
                            break;

                        default:
                            /* There is an unexpected error attaching to the
                             * VCC.
                             */
                            nRet = MapError( baStatus );
                            break;
                        }
                    }
                    else
                    {
                        /* The VCC type is not the correct AAL type. */
                        if( usVpi == (UINT16) ATM_VPI_ANY ||
                            usVci == (UINT16) ATM_VCI_ANY )
                        {
                            /* Make the address invalid and continue. */
                            pSaveAddr->usVci = 0xffff;
                        }
                        else
                        {
                            /* A VCC to use is not found. */
                            nRet = -EADDRINUSE;
                        }
                    }
                }
                else
                {
                    /* Getting the configuration record for the VCC address
                     * failed.  This should not happen.
                     */
                    if( usVpi == (UINT16) ATM_VPI_ANY ||
                        usVci == (UINT16) ATM_VCI_ANY )
                    {
                        /* Make the address invalid and continue. */
                        pSaveAddr->usVci = 0xffff;
                    }
                    else
                    {
                        /* A VCC to use is not found. */
                        nRet = -EADDRINUSE;
                    }
                }
            }
            else
            {
                if( usVpi == (UINT16) ATM_VPI_ANY && usLowVpi != 0xfffe )
                {
                    /* A VCI is not found on current VPI.  Invalidate it and
                     * continue.
                     */
                    for( i = 0, pAddr = pAddrs; i < ulNumConfiguredVccs;
                        i++, pAddr++ )
                    {
                        if( usLowVpi == pAddr->usVpi  )
                            pAddr->usVpi = 0xffff;
                    }
                }
                else
                    /* A VCC to use is not found. */
                    nRet = -EADDRINUSE;
            }
        }

        kfree( pAddrs );
    }
    else
        nRet = -ENOMEM;

    return( nRet );
} /* AttachVcc */


/***************************************************************************
 * Function Name: blaa_close
 * Description  : This function is called when an application wants to use
 *                a VCC.
 * Returns      : None.
 ***************************************************************************/
static void blaa_close( struct atm_vcc *pVcc )
{
    PBLAA_VCC_INFO pVccInfo = (PBLAA_VCC_INFO) pVcc->dev_data;

    /* Indicate unreadiness. */
    clear_bit(ATM_VF_READY,&pVcc->flags);

    if( pVccInfo )
    {
        /* Detach from the Broadcom ATM API */
        if( pVccInfo->ulAttachHandle )
        {
            BcmAtm_Detach( pVccInfo->ulAttachHandle );
            pVccInfo->ulAttachHandle = 0;
        }

        kfree( pVccInfo );
        pVcc->dev_data = NULL;
    }

    /* Say the VPI/VCI is free again. */
    clear_bit(ATM_VF_ADDR,&pVcc->flags);
} /* blaa_close */


/***************************************************************************
 * Function Name: blaa_setsockopt
 * Description  : This function is called when an application calls the
 *                setsockopt function.
 * Returns      : 0 = success, -errno = error
 ***************************************************************************/
static int blaa_setsockopt(struct atm_vcc *pVcc, int nLevel, int nOptName,
    void *pvOptVal, int nOptLen)
{
    int nRet = -EINVAL;
    PBLAA_VCC_INFO pVccInfo = (PBLAA_VCC_INFO) pVcc->dev_data;

    if( nLevel == SOL_ATM && nOptName == 0 )
    {
        /* The parameter, pvOptVal, is not a pointer but contains the actual
         * option data.  The least significant byte is the AAL2 channel id,
         * the next byte is the voice routing option, next after that is the
         * BcmAtm_SetAal2ChannelIds flags value and the most significant byte
         * is not used.
         */
        UINT32 ulOptVal      = (UINT32) pvOptVal;
        UINT8 ucChannelId    = (UINT8)  (ulOptVal & 0xff);
        UINT8 ucVoiceRouting = (UINT8) ((ulOptVal >> 8) & 0xff);
        UINT8 ucFlags        = (UINT8) ((ulOptVal >> 16) & 0xff);
        ATM_VCC_AAL2_CHANNEL_ID_PARMS Parms = {ID_ATM_VCC_AAL2_CHANNEL_ID_PARMS,
            ucChannelId, ucVoiceRouting, ucFlags,};

        BCMATM_STATUS baSts = BcmAtm_SetAal2ChannelIds(pVccInfo->ulAttachHandle,
            &Parms, 1);

        if( baSts == STS_SUCCESS )
            nRet = 0;
        else
            nRet = MapError( baSts );
    }

    return( nRet );
} /* blaa_setsockopt */


/***************************************************************************
 * Function Name: blaa_send
 * Description  : This function is called when an application wants to send
 *                data on a VCC.
 * Returns      : 0 = success, -errno = error
 ***************************************************************************/
static int blaa_send(struct atm_vcc *pVcc, struct sk_buff *pSockBuf )
{
    int nRet = 0;
    PBLAA_VCC_INFO pVccInfo = (PBLAA_VCC_INFO) pVcc->dev_data;
    PBLAA_TX_HDR pTxHdr = NULL;
    PATM_BUFFER pAb;
    BCMATM_STATUS baStatus;
    int atmPriority = 0;

    /* start of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS
    static unsigned int ulPrevHPrioRate = 0;    /*先前的高优先级速率*/
    static unsigned int ulCurHPrioRate  = 0;    /*目前的高优先级速率*/
    
    static unsigned int ulPrevHWeightRate = 0;  /*先前的高权重速率*/
    static unsigned int ulCurHWeightRate  = 0;  /*目前的高权重速率*/
    static unsigned int ulPrevLWeightRate = 0;  /*先前的低权重速率*/
    static unsigned int ulCurLWeightRate  = 0;  /*目前的低权重速率*/

    unsigned int ulXdslUpStreamRate = 0; /* 1250kBps = 1250 * 8 = 10000kbit/s */

    static  unsigned int ulCount = 0;
    static  unsigned int ulState = 0;
    unsigned int ulHWeightPeakRate = 0;
    unsigned int ulLWeightPeakRate = 0;
    unsigned int ulTimeSlotRatio = 1;
    unsigned int ulPrioFlag        = PRIORITY_NO_CHANGE;
#endif
#endif
    /* end of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */

    DPRINTK("blaad: blaa_send entry\n");

#if (defined(PORT_MIRROR_DBG))
     printk ("BLAA_DD :- rfc2684OutMirrorStatus = %d \n", rfc2684OutMirrorStatus) ;
#endif

    if( pVccInfo && !list_empty(&pVccInfo->TxFreeHdrList) )
    {
        /* Get a transmit header from the free list of transmit headers */
        pTxHdr = list_entry(pVccInfo->TxFreeHdrList.next, BLAA_TX_HDR, List);
        list_del(&pTxHdr->List);

        ATM_SKB(pSockBuf)->vcc = pVcc;
        pTxHdr->DataParms.ulParmFreeDataParms = (UINT32) pSockBuf;

        if( pVcc->qos.aal == ATM_AAL5 )
        {
           /* The control here goes to Mirroring utility that puts the data
           ** onto the configured destination. Could be an ethernet port.
           **/
           if (rfc2684OutMirrorStatus == MIRROR_ENABLED) {
                 blaaMirrorRfc2684Frame (pSockBuf, pSockBuf->len, DIR_OUT,
                                       mirrorOutPort) ;
           }
        }

        DPRINTK("pSockBuf->nfmark=%d,ucSendPriority=%d\n",pSockBuf->nfmark,
               pTxHdr->DataParms.ucSendPriority);

        /* Reinitialize the priority in the pTxHdr, because it may still
           has the old priority set by the last packet that used this pTxHdr.
           So a packet with a nfmark always has the lowest priority 1 */
        pTxHdr->DataParms.ucSendPriority = 1;

#ifdef CONFIG_NETFILTER
        atmPriority = pSockBuf->nfmark & 0x0F;
        if (atmPriority <= 4 && atmPriority >= 1)
        /* bit 3-0 of the 32-bit nfmark is the atm priority, set by iptables
         * bit 7-4 is the Ethernet switch physical port number, set by lan port drivers.
         * bit 8-10 is the wanVlan priority bits
         */
            pTxHdr->DataParms.ucSendPriority = atmPriority;
        else if (pSockBuf->priority <= 4 && pSockBuf->priority >= 1)
                 pTxHdr->DataParms.ucSendPriority = pSockBuf->priority;
#else
        if (pSockBuf->priority <= 4 && pSockBuf->priority >= 1)
            pTxHdr->DataParms.ucSendPriority = pSockBuf->priority;
#endif
        DPRINTK("pSockBuf->nfmark=%x,ucSendPriority=%d\n",pSockBuf->nfmark,
               pTxHdr->DataParms.ucSendPriority);

        pAb = pTxHdr->DataParms.pAtmBuffer;
        pAb->pDataBuf = pSockBuf->data;
        pAb->ulDataLen = (UINT32) pSockBuf->len;

        /* start of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS
        /*
         KPN QoS实现方式
         1.采用ATM SAR来保证绝对优先级;
         2.对于WRED采用时间比例来保证速率比例;
         3.基本的公式:v1t1:v2t2=x:y;
           x:y为权值比,v1, v2分别为高低优先级的实际发包速率(实际上不需要关心他们的精确数值,
           只要保证比例关系正确即可);
         4.那么要保证权值的比例正确,只需要控制高低权值速率的发送时间比即可;
         5.再进一步说,需要控制时间比只有一种情况,即高低权值的轰包速率都大于配额速率才需要控
           制时间比;
         6.其他情况:高优先级的速率已经超过的激活速率,直接丢包;
                    所有的报文的轰包速率小于激活速率,不用作控制;
                    高优先级的速率小于激活速率,但是高权值小于配额速率,需要优先级保证;
                    高优先级的速率小于激活速率,但是低权值小于配额速率,需要交换高低权值的优先级;
                    高优先级的速率小于激活速率,但是高低权值均小于配额速率,需要控制时间比.
        */
        // 统计不同类型的报文长度
        if ((g_nPtmConfig == ATM_TRAFFIC_BR) || (g_nPtmConfig == PTM_TRAFFIC_BR))
        {
            g_ulTotalPktLen += (UINT32) pSockBuf->len;

            local_bh_disable();

            switch(pTxHdr->DataParms.ucSendPriority)
            {
                case 4:
                case 3:
                    g_ulHPrioPktLen += (UINT32)pSockBuf->len;
                    break;
                case 2:
                    g_ulHWeightPktLen += (UINT32)pSockBuf->len;
                    break;
                case 1:
                    g_ulLWeightPktLen += (UINT32)pSockBuf->len;  // or data_len
                    break;
                default:
                    g_ulOtherPktLen += (UINT32)pSockBuf->len;
                    break;
            }

            //__local_bh_enable();                //  return之前要使能中断,切切

            // 高中优先级速率 unit kBps
            if((g_ulHPrioPktRate - ulPrevHPrioRate) > RATE_VARIABLE_SCOPE || (ulPrevHPrioRate - g_ulHPrioPktRate) > RATE_VARIABLE_SCOPE)
            {
                ulCurHPrioRate = g_ulHPrioPktRate;
            }
            else
            {
                ulCurHPrioRate = ulPrevHPrioRate;
            }
            ulPrevHPrioRate = g_ulHPrioPktRate ;

            // 高权值速率 unit kBps
            if((g_ulHWeightPktRate - ulPrevHWeightRate) > RATE_VARIABLE_SCOPE || (ulPrevHWeightRate - g_ulHWeightPktRate) > RATE_VARIABLE_SCOPE)
            {
                ulCurHWeightRate = g_ulHWeightPktRate;
            }
            else
            {
                ulCurHWeightRate = ulPrevHWeightRate;
            }
            ulPrevHWeightRate = g_ulHWeightPktRate;

            // 低权值速率 unit kBps
            if((g_ulLWeightPktRate - ulPrevLWeightRate) > RATE_VARIABLE_SCOPE || (ulPrevLWeightRate - g_ulLWeightPktRate) > RATE_VARIABLE_SCOPE)
            {
                ulCurLWeightRate = g_ulLWeightPktRate;
            }
            else
            {
                ulCurLWeightRate = ulPrevLWeightRate;
            }
            ulPrevLWeightRate = g_ulLWeightPktRate;


            // 获取xdsl激活速率,这里只对g_AtmLinkInfoArg.InterfaceCfg的两个成员进行了操作,其他成员全部是0
            // unit bitps -> kByteps
            ulXdslUpStreamRate = g_ulLinkUpStreamRate / (1000 *8);

            // 高中优先级的报文已经占满带宽,丢弃低优先级的报文
            if (0 == ulXdslUpStreamRate)
            {
                ulPrioFlag = PRIORITY_NO_CHANGE;
            }
            else if (ulCurHPrioRate >= ulXdslUpStreamRate)
            {
                // drop low priority packet priority == 1 or 2
                if (2 == pTxHdr->DataParms.ucSendPriority ||  1 == pTxHdr->DataParms.ucSendPriority)
                {
                    list_add_tail(&pTxHdr->List, &pVccInfo->TxFreeHdrList);
                    nRet = (pVccInfo) ? -ENOBUFS : -EIO;
                    if( pVcc->pop )
                        pVcc->pop( pVcc, pSockBuf );
                    else
                        dev_kfree_skb_any( pSockBuf );  // shoule return right now!
                    __local_bh_enable();                //  return之前要使能中断,切切
                    return( nRet );
                }
            }
            // 高中优先级的带宽足够,对剩余带宽进行分配
            else
            {
                // 实际发送带宽均小于xdsl激活速率
                if( (ulCurHPrioRate + ulCurHWeightRate + ulCurLWeightRate) <= ulXdslUpStreamRate)
                {
                    // printk("=======case 1 :the band width is enough, divide as its actual rate\n");
                    // set sustain cell rate to its actural rate
                    g_ulHWeightPRate = ulCurHWeightRate;
                    g_ulLWeightPRate = ulCurLWeightRate;
                    ulPrioFlag = PRIORITY_SET_TO_ONE;     // 将优先级2置为1
                }
                else
                {
                    // 高权值的轰包速率小于分配速率
                    if(ulCurHWeightRate <= (( g_ulQosHWeight * (ulXdslUpStreamRate - ulCurHPrioRate )) / (g_ulQosHWeight + g_ulQosLWeight)))
                    {
                        // printk("=======case 2 : the band width is not enough but the hweight less than its assignment 3:1\n");
                        g_ulHWeightPRate = ulCurHWeightRate;
                        g_ulLWeightPRate = ulXdslUpStreamRate - ulCurHPrioRate - g_ulHWeightPRate;
                        ulPrioFlag = PRIORITY_NO_CHANGE;     // 优先级不变
                    }
                    // 低权值的实际速率小于分配速率
                    else if(ulCurLWeightRate <= ((g_ulQosLWeight * (ulXdslUpStreamRate - ulCurHPrioRate)) / (g_ulQosHWeight + g_ulQosLWeight)))
                    {
                        // printk("=======case 3 : the band width is not enough but the lweight less than its assignment\n");
                        g_ulLWeightPRate = ulCurLWeightRate;
                        g_ulHWeightPRate = ulXdslUpStreamRate - ulCurHPrioRate - g_ulLWeightPRate;
                        ulPrioFlag = PRIORITY_EXCHANGE;     // 优先级反转 1<->2
                    }                
                    else
                    {   
                        ulPrioFlag = PRIORITY_SET_TO_ONE;     // 优先级2置为1
                        // 1.确定上限速率
                        if(ulCurHWeightRate > (ulXdslUpStreamRate - ulCurHPrioRate) && ulCurLWeightRate > (ulXdslUpStreamRate - ulCurHPrioRate))
                        {   
                            // 两种权值都超过了配额速率,那么出口速率比例为1:1()其数值
                            // 为ulXdslUpStreamRate - ulCurHPrioRate
                            ulHWeightPeakRate = 1;
                            ulLWeightPeakRate = 1;
                        }
                        else if(ulCurHWeightRate > (ulXdslUpStreamRate - ulCurHPrioRate) && ulCurLWeightRate < (ulXdslUpStreamRate - ulCurHPrioRate))
                        {
                            ulHWeightPeakRate = (ulXdslUpStreamRate - ulCurHPrioRate) / 100;
                            ulLWeightPeakRate = ulCurLWeightRate / 100;
                        }
                        else if(ulCurHWeightRate < (ulXdslUpStreamRate - ulCurHPrioRate) && ulCurLWeightRate > (ulXdslUpStreamRate - ulCurHPrioRate))
                        {
                            ulHWeightPeakRate = ulCurHWeightRate / 100;
                            ulLWeightPeakRate = (ulXdslUpStreamRate - ulCurHPrioRate) / 100;
                        }
                        else
                        {
                            // 轰包速率小于可用速率,那么实际速率为轰包速率
                            ulHWeightPeakRate = ulCurHWeightRate / 100;
                            ulLWeightPeakRate = ulCurLWeightRate / 100;
                        }

                        // 2. 根据上限速率进行速率配置
                        if(ulState == 0)
                        {
                            ulCount = jiffies;
                            ulState = 1;
                        }
                        else
                        {
                            // ulTimeSlotRatio = ((3 * ulLWeightPeakRate)/(1 * ulHWeightPeakRate));
                            if(jiffies - ulCount <  (g_ulQosHWeight * ulLWeightPeakRate))
                            {
                                if (1 == pTxHdr->DataParms.ucSendPriority)
                                {
                                    list_add_tail(&pTxHdr->List, &pVccInfo->TxFreeHdrList);
                                    nRet = (pVccInfo) ? -ENOBUFS : -EIO;
                                    if( pVcc->pop )
                                        pVcc->pop( pVcc, pSockBuf );
                                    else
                                        dev_kfree_skb_any( pSockBuf );  // shoule return right now!
                                    __local_bh_enable();                //  return之前要使能中断,切切
                                    return( nRet );
                                }
                            }
                            else if( jiffies - ulCount < ((g_ulQosHWeight * ulLWeightPeakRate) + (g_ulQosLWeight * ulHWeightPeakRate) ))
                            {
                                if (2 == pTxHdr->DataParms.ucSendPriority)
                                {
                                    list_add_tail(&pTxHdr->List, &pVccInfo->TxFreeHdrList);
                                    nRet = (pVccInfo) ? -ENOBUFS : -EIO;
                                    if( pVcc->pop )
                                        pVcc->pop( pVcc, pSockBuf );
                                    else
                                        dev_kfree_skb_any( pSockBuf );  // shoule return right now!
                                    __local_bh_enable();                //  return之前要使能中断,切切
                                    return( nRet );
                                }
                            }
                            else 
                            {
                                ulCount = jiffies;
                            }
                        }
                    }
                }
            }
            __local_bh_enable();

            switch(ulPrioFlag)
            {
                case PRIORITY_SET_TO_ONE :
                    if( pTxHdr->DataParms.ucSendPriority == 2)
                    {
                        pTxHdr->DataParms.ucSendPriority = 1;
                    }
                    break;
                case PRIORITY_EXCHANGE :
                    if(pTxHdr->DataParms.ucSendPriority == 2)
                    {
                        pTxHdr->DataParms.ucSendPriority = 1;
                    }
                    else if(pTxHdr->DataParms.ucSendPriority == 1)
                    {
                        pTxHdr->DataParms.ucSendPriority = 2;
                    }
                    break;
                case PRIORITY_NO_CHANGE :
                    break;
                default :
                    break;
            }
        }
#endif
#endif
        /* end of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */
        if( pVcc->qos.aal == ATM_AAL2 )
        {
            /* For AAL2, the last byte of data contains the AAL2 channel id and
             * the second to last byte contains the circuit type and
             * the third to last byte contains the uui.
             */
            pTxHdr->DataParms.ucAal2ChannelId =
                pSockBuf->data[pSockBuf->len - 1];
            pTxHdr->DataParms.ucCircuitType =
                pSockBuf->data[pSockBuf->len - 2];
            pTxHdr->DataParms.ucUuData5 =
                pSockBuf->data[pSockBuf->len - 3];
            //DPRINTK("blaa_send: UUI=%d, ct=%d, cid=%d\n", pTxHdr->DataParms.ucUuData5, pTxHdr->DataParms.ucCircuitType, pTxHdr->DataParms.ucAal2ChannelId);
            pAb->ulDataLen -= 3;
        }

        /* Send the data using the Broadcom ATM API. */
        if( (baStatus = BcmAtm_SendVccData( pVccInfo->ulAttachHandle,
            &pTxHdr->DataParms )) != STS_SUCCESS )
        {
            list_add_tail(&pTxHdr->List, &pVccInfo->TxFreeHdrList);
            nRet = MapError( baStatus );
            if( pVcc->pop )
                pVcc->pop( pVcc, pSockBuf );
            else
                dev_kfree_skb_any( pSockBuf );
        }
        else {
           // printk ("blaa sent data Upstream \n") ;
        }
    }
    else
    {
        nRet = (pVccInfo) ? -ENOBUFS : -EIO;
        if( pVcc->pop )
            pVcc->pop( pVcc, pSockBuf );
        else
            dev_kfree_skb_any( pSockBuf );
    }

    return( nRet );
} /* blaa_send */

/***************************************************************************
 * Function Name: BlaaFreeDpCb
 * Description  : Called by ATM API module after it has processed a send
 *                data buffer that contained a packet.
 * Returns      : None.
 ***************************************************************************/
static void BlaaFreeDpCb( PATM_VCC_DATA_PARMS pDataParms )
{
    struct sk_buff *pSockBuf=(struct sk_buff *) pDataParms->ulParmFreeDataParms;
    struct atm_vcc *pVcc = (struct atm_vcc *) ATM_SKB(pSockBuf)->vcc;
    PBLAA_VCC_INFO pVccInfo = (PBLAA_VCC_INFO) pVcc->dev_data;

   if ((*((unsigned int *) &pSockBuf->cb [32])) != 0x12345678) {
    DPRINTK("blaad: BlaaFreeDpCb entry\n");

    // printk ("Freeing the Sock Buffer 1 \n") ;

    list_add_tail(&((PBLAA_TX_HDR) pDataParms)->List, &pVccInfo->TxFreeHdrList);

    if( pVcc && pVcc->pop )
        pVcc->pop( pVcc, pSockBuf );
    else
        dev_kfree_skb_any( pSockBuf );
   }
   else {
        // printk ("Freeing the Sock Buffer 2 \n") ;
        dev_kfree_skb_any( pSockBuf );
   }
} /* BlaaFreeDpCb */


/***************************************************************************
 * Function Name: BlaaFreeSkbOrData
 * Description  : This callback function returns the socket buffer header or
 *                socket buffer data that is no longer being used by the upper
 *                layers (bridge, TCP/IP).
 * Returns      : None.
 ***************************************************************************/
static void BlaaFreeSkbOrData( PBLAA_RX_HDR pRxHdr, void *pObj, int nFlag )
{
    PBLAA_RX_INFO pRxInfo = pRxHdr->pRxInfo;

    local_bh_disable();

    if( nFlag == FREE_SKB )
    {
        struct sk_buff *pSockBuf = (struct sk_buff *) pObj;

        pSockBuf->retfreeq_context = pRxInfo->pFreeSockBufList;
        pRxInfo->pFreeSockBufList = pSockBuf;
    }
    else
    {
        (*pRxHdr->pDataParms->pFnFreeDataParms)(pRxHdr->pDataParms);
        pRxHdr->pDataParms = NULL;
        pRxHdr->nextFree = pRxInfo->FreeRxHdrList;
        pRxInfo->FreeRxHdrList = pRxHdr;
    }

    __local_bh_enable();
}


/***************************************************************************
 * Function Name: BlaaRcvCb
 * Description  : Receives data from the ATM API module.
 * Returns      : None.
 ***************************************************************************/
static void BlaaRcvCb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, struct atm_vcc *pVcc )
{
    PBLAA_RX_INFO pRxInfo = (PBLAA_RX_INFO) g_pRxInfo;
    PBLAA_VCC_INFO pVccInfo = (PBLAA_VCC_INFO) pVcc->dev_data;
    UINT8 *pRcvBuf;
    UINT32 ulRcvLen, ulRcvOffset;
    PBLAA_RX_HDR pRxHdr;
    PATM_BUFFER pAb = pDp->pAtmBuffer;
    struct sk_buff *pSockBuf;

    if( likely(pVccInfo && pRxInfo->ulModuleInUse == MODULE_IN_USE) )
    {
#if (defined(PORT_MIRROR_DBG))
       printk ("BLAA_DD :- rfc2684InMirrorStatus = %d \n", rfc2684InMirrorStatus) ;
#endif

        if( pAb->pNextAtmBuf == NULL )
        {
            pRcvBuf = pAb->pDataBuf;
            ulRcvLen = pAb->ulDataLen;
            ulRcvOffset = (UINT32) pAb->usDataOffset;

            if( pVcc->qos.aal == ATM_AAL2 )
            {
                /* For AAL2, add to the end of the packet:
                 uui, circuit type and the AAL2 channel id.
                 */
                pRcvBuf[ulRcvLen] = pDp->ucUuData5;
                pRcvBuf[ulRcvLen + 1] = pDp->ucCircuitType;
                pRcvBuf[ulRcvLen + 2] = pDp->ucAal2ChannelId;
                ulRcvLen += 3;
            }

            /* Get a free socket buffer. */
            if( pRxInfo->FreeRxHdrList!=NULL && pRxInfo->pFreeSockBufList!=NULL )
            {
                pSockBuf = pRxInfo->pFreeSockBufList;
                pRxInfo->pFreeSockBufList =
                    pRxInfo->pFreeSockBufList->retfreeq_context;

                pRxHdr = pRxInfo->FreeRxHdrList;
                pRxInfo->FreeRxHdrList = pRxHdr->nextFree;
                pRxHdr->pDataParms = pDp;

                skb_hdrinit(ulRcvOffset, ulRcvLen, pSockBuf, pRcvBuf,
                    (void *) BlaaFreeSkbOrData, pRxHdr, FROM_WAN);

                atm_force_charge(pVcc, pSockBuf->truesize);

#ifdef LOOPBACK_TEST
                inFrameCount++ ;
                blaaSendInFrame (pVcc, pSockBuf, ulRcvLen) ;
#endif
                if( pVcc->qos.aal == ATM_AAL5 )
                {
                   /* The control here goes to Mirroring utility that puts the data
                   ** onto the configured destination. Could be an ethernet port.
                   **/
                   if (rfc2684InMirrorStatus == MIRROR_ENABLED) {
                       blaaMirrorRfc2684Frame (pSockBuf, ulRcvLen, DIR_IN,
                                               mirrorInPort) ;
                   }
                }

                /* Give the data in the socket buffer to the upper layer. */
                pVcc->push( pVcc, pSockBuf );
            }
            /* If there is no socket buffer available, free the received data.*/
            else
                (*pDp->pFnFreeDataParms) (pDp);
        }
        else
        {
            /* There is more than one buffer in the ATM Buffer scatter gather
             * list.  Copy the buffers into one socket buffer.
             */

            /* Caculate the size of the received data */
            for(pAb = pDp->pAtmBuffer,ulRcvLen = 0; pAb; pAb = pAb->pNextAtmBuf)
                ulRcvLen += pAb->ulDataLen;

            pSockBuf = atm_alloc_charge( pVcc, (int) ulRcvLen, GFP_ATOMIC );
            if( pSockBuf )
            {
                /* Copy the receive buffer to the socket buffer. */
                UINT8 *p = skb_put( pSockBuf, ulRcvLen );

                for( pAb = pDp->pAtmBuffer; pAb; pAb = pAb->pNextAtmBuf )
                {
                    memcpy( p, pAb->pDataBuf, pAb->ulDataLen );
                    p += pAb->ulDataLen;
                }

                /* Return ATM_VCC_DATA_PARMS structure to the ATM API module. */
                (*pDp->pFnFreeDataParms) (pDp);

                atomic_inc(&pVcc->stats->rx);

#ifdef LOOPBACK_TEST
                inFrameCount++ ;
                blaaSendInFrame (pVcc, pSockBuf,ulRcvLen) ;
#endif
                if( pVcc->qos.aal == ATM_AAL5 )
                {
                   /* The control here goes to Mirroring utility that puts the data
                   ** onto the configured destination. Could be an ethernet port.
                   **/
                   if (rfc2684InMirrorStatus == MIRROR_ENABLED) {
                      blaaMirrorRfc2684Frame (pSockBuf, ulRcvLen, DIR_IN,
                                               mirrorInPort) ;
                   }
                }

                /* Give the data in the socket buffer to the upper layer */
                pVcc->push( pVcc, pSockBuf );
            }
            else
            {
                /* The configured buffer size of the socket, sk->rcvbuf, is
                 * currently exceeded.  Drop the packet.
                 */
                (*pDp->pFnFreeDataParms) (pDp);
            }
        }
    }
    else
        (*pDp->pFnFreeDataParms) (pDp);
} /* BlaaRcvCb */


#ifdef LOOPBACK_TEST
void blaaSendInFrame (struct atm_vcc *pVcc, struct sk_buff *pSockBuf, unsigned int len)
{
   struct sk_buff   *pCloneBuf ;
   int ret ;

   /* If we are here, we need to send the frame for Mirroring. Let's clone
   ** the sk_buff hdr sharing the data with adjusted len information and send
   ** it onto Ethernet If 0.
   **/

   pCloneBuf = skb_copy (pSockBuf, GFP_ATOMIC) ;
   if (pCloneBuf == NULL) {
      printk ("sk_buff allocation failed. fatal error. No Mirroring \n") ;
      goto _EndProcessing ;
   }

   *((unsigned int *) &pCloneBuf->cb [32]) = 0x12345678 ;

   ++outFrameCount ;
   if ((ret = blaa_send (pVcc, pCloneBuf)) >= 0) {
      /* Success */
      // printk ("Blaa_dd: Out Data Frame = %d \n", ++outFrameCount) ;
   }
   else {
      // printk ("Blaa_dd: blaa_send failed - %d \n", ret) ;
   }

_EndProcessing :
   return ;
}
#endif

/***************************************************************************
 * Function Name: BlaaMibInfoTo
 * Description  : Timer function that is called at periodic intervals.  It
 *                retrieves ATM interface information which is used to update
 *                some MIB related fields.
 * Returns      : None.
 ***************************************************************************/
static void BlaaMibInfoTo( UINT32 ulNotUsed )
{
    struct timer_list *pTimer = &g_MibInfoTimer;
    UINT32 interfaceId = 0;
    ATM_INTERFACE_CFG Cfg = {ID_ATM_INTERFACE_CFG};

    BcmAtm_GetInterfaceId(0, &interfaceId);
    if( BcmAtm_GetInterfaceCfg( interfaceId, &Cfg ) == STS_SUCCESS )
    {
        g_pAtmNetDev->trans_start = Cfg.ulIfLastChange;
        g_pCpcsNetDev->trans_start = Cfg.ulIfLastChange;
    }

    /* start of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
#ifdef CONFIG_KPN_QOS
    if ((g_nPtmConfig == ATM_TRAFFIC_BR) || (g_nPtmConfig == PTM_TRAFFIC_BR) )
    {
        // blaad send累加,计数器请零
        // printk("\n\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        // printk("===g_ulHPrioPktLen           = %u\n",higherPriorityPacketLen);
        g_ulHPrioPktRate = (g_ulHPrioPktLen)/ (1024 * 3); // kBps
        g_ulHPrioPktLen  = 0;    // accurate?

        // printk("===g_ulHWeightPktLen         = %u\n",highWeightPacketLen);
        g_ulHWeightPktRate = (g_ulHWeightPktLen )/(1024 * 3); // kBps
        g_ulHWeightPktLen  = 0;    // accurate?

        // printk("===g_ulLWeightPktLen         = %u\n",lowWeightPacketLen);
        g_ulLWeightPktRate = (g_ulLWeightPktLen)/(1024 * 3); // kBps
        g_ulLWeightPktLen  = 0;    // accurate?

        // printk("===g_ulTotalPktLen            = %u\n\n",totalPacketLen);
        g_ulTotalPktRate = (g_ulTotalPktLen)/(1024 * 3);
        g_ulTotalPktLen  = 0;

#if 0
        printk("===g_ulQosHWeight                   = %u\n", g_ulQosHWeight);
        printk("===g_ulQosLWeight                   = %u\n", g_ulQosLWeight);
        printk("===g_ulLinkUpStreamRate             = %u\n", g_ulLinkUpStreamRate);
        printk("===g_ulHPrioPktRate                 = %u\n", g_ulHPrioPktRate);
        printk("===g_ulHWeightPktRate               = %u\n", g_ulHWeightPktRate);
        printk("===g_ulLWeightPktRate               = %u\n", g_ulLWeightPktRate);
        printk("===g_ulTotalPktRate                 = %u\n\n",g_ulTotalPktRate);
        
        printk("===g_ulOtherPktLen                  = %u\n\n",g_ulOtherPktLen);

        printk("===g_ulLWeightPRate                 = %u\n",g_ulLWeightPRate);
        printk("===g_ulHWeightPRate                 = %u\n\n",g_ulHWeightPRate);

        printk("===g_ulHWeightActuRate              = %u\n", g_ulHWeightActuRate);
        printk("===g_ulLWeightActuRate              = %u\n\n", g_ulLWeightActuRate);
#endif

    }
#endif
#endif
    /* end of protocol QoS for KPN by zhangchen z45221 2006年7月11日 */

    if( !timer_pending(pTimer) )
    {
        /* Restart the timer. */
        pTimer->expires = jiffies + MIB_INFO_TIMEOUT;
        pTimer->function = BlaaMibInfoTo;
        pTimer->data = 0;
        add_timer( pTimer );
    }
} /* BlaaMibInfoTo */


/***************************************************************************
 * Function Name: MapError
 * Description  : Maps an Broadcom ATM API error code to a Linux error code.
 * Returns      : Linux error code.
 ***************************************************************************/
static int MapError( BCMATM_STATUS baStatus )
{
    int nRet = 0;
    switch( baStatus )
    {
    case STS_SUCCESS:
        nRet = 0;
        break;

    case STS_ERROR:
        nRet = -EPROTO;
        break;

    case STS_STATE_ERROR:
        nRet = -EBADFD;
        break;

    case STS_PARAMETER_ERROR:
        nRet = -EINVAL;
        break;

    case STS_ALLOC_ERROR:
        nRet = -ENOMEM;
        break;

    case STS_RESOURCE_ERROR:
        nRet = -EBUSY;
        break;

    case STS_IN_USE:
        nRet = -EADDRINUSE;
        break;

    case STS_VCC_DOWN:
        nRet = -ENETDOWN;
        break;

    case STS_INTERFACE_DOWN:
        nRet = -ENETDOWN;
        break;

    case STS_LINK_DOWN:
        nRet = -ENOLINK;
        break;

    case STS_NOT_FOUND:
        nRet = -ENODATA;
        break;

    case STS_NOT_SUPPORTED:
        nRet = -ENOSYS;
        break;

    default:
        nRet = -EIO;
        break;
    }

    DPRINTK( "blaadd: MapError bcm=%lu, Linux=%d\n", (UINT32) baStatus, nRet );
    return( nRet );
} /* MapError */


/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( blaa_detect );
module_exit( blaa_cleanup );
MODULE_LICENSE("Proprietary");

