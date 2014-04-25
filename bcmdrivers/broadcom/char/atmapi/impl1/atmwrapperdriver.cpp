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
//**************************************************************************
// File Name  : AtmWrapperDriver.cpp for BCM63xx
//
// Description: This file contains the implementation for the initial ATM API
//              function entry points.
//
// Updates    : 10/12/2000  lat.  Created from BCM6352.
//**************************************************************************

// Includes.
#include "atmapiimpl.h"
#include "atmosservices.h"
#include "board.h"
#include "bcm_map.h"
#include "bcm_intr.h"
#if defined(__KERNEL__)
#include "boardparms.h"
#endif

// Defines.
#define INTERRUPT_NOT_MAPPED    0
#define INTERRUPT_MAPPED        1
#define INTERRUPT_NOT_USED      2

//#define CONFIG_BCM96358_UTOPIA

// Structs.
struct ATM_INFO
{
    ATM_PROCESSOR *pAtmProcessor;
    ATM_OS_SERVICES OsServices;
    ATM_OS_FUNCS OsFuncs;
    UINT32 ulThreadId;
    UINT32 ulEventId;
    UINT32 ulExitThread;
    UINT32 ulInterruptMapped;
    UINT32 ulPrioritizeReceivePkts;
    ATM_INTERFACE_LINK_INFO InitialLinkInfo;
};

// Globals.

// Set this constant to 0 to enable ATM transmit shaping changes for the
// BCM6348B0 and greater.  Set this constant to 1 to use the same ATM transmit
// shaping as previous chip revisions.

static const UINT32 g_ulUseBothAdslPortsForShaping = 0;

ATM_INFO g_AtmInfo = {NULL, {0}, {0}, 0, 0, 0, 0};

// Forwards.
extern "C" {
void InitOsServices( ATM_OS_FUNCS *pOsFuncs, ATM_OS_SERVICES *pOsServices );
void InitAdslServices( ATM_OS_SERVICES *pOsServices );
#if defined(__KERNEL__)
int BcmAtm_Isr(int irq, void * dev_id, struct pt_regs * regs);
#else
unsigned int BcmAtm_Isr( unsigned int );
#endif
BCMATM_STATUS BcmAtm_SetIntrHandling( ATM_INFO *pAi,
    PATM_INITIALIZATION_PARMS pInitParms  );
void BcmAtm_ResetIntrHandling( ATM_INFO *pAi );
}


//**************************************************************************
// Function Name: BcmAtm_Initialize
// Description  : Initializes the object.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_Initialize( PATM_INITIALIZATION_PARMS pInitParms )
{
    BCMATM_STATUS baStatus;
    ATM_INFO *pAi = &g_AtmInfo;
    int ulBusSpeed = 0;
    int i = 0;

    // Verify that the caller is using the same structure format as this module.
    if( pInitParms->ulStructureId == ID_ATM_INITIALIZATION_PARMS &&
        pAi->pAtmProcessor == NULL )
    {
       /*start add  of  初始化发送队列寄存器by l39225 2006-7-7*/
        for( i = 0; i < MAX_QUEUE_SHAPERS; i++)
        {
            AP_REGS->ulTxQAddr[i] =  (UINT32)(0);
        }
        /*end add  of  初始化发送队列寄存器by l39225 2006-7-7*/
        // Instantiate a new ATM Processor object.
        if( (pAi->pAtmProcessor = new ATM_PROCESSOR) != NULL )
        {
            // Initialize operating system specific functions.
            AtmOsInitialize( &pAi->OsFuncs );

#if defined(CONFIG_BCM_ENDPOINT) || defined(CONFIG_BCM_ENDPOINT_MODULE)
			//set ucPrioritizeReceivePkts to 0 for HG553 Bug #97 20091130 
            //pInitParms->ucPrioritizeReceivePkts = 1;
            pInitParms->ucPrioritizeReceivePkts = 0;
            AtmOsPrintf( "atmapi: prioritize receive packets\n" );
#else
            pInitParms->ucPrioritizeReceivePkts = 0;
#endif
            pAi->ulPrioritizeReceivePkts = pInitParms->ucPrioritizeReceivePkts;
            if( pAi->ulPrioritizeReceivePkts == 1 )
            {
                pAi->OsFuncs.pfnRequestSem = (FN_GENERIC) AtmOsRequestSemCli;
                pAi->OsFuncs.pfnReleaseSem = (FN_GENERIC) AtmOsReleaseSemSti;
            }

            // Assign the operating system callback functions that
            // are called by the ATM Processor object.
            InitOsServices( &pAi->OsFuncs, &pAi->OsServices );
            InitAdslServices( &pAi->OsServices );

#if defined(CONFIG_BCM96348)
            // If UTOPIA is configured, enable the SAR clock and appropriate
            // GPIO mode bits.
            if( pInitParms->PortCfg[PHY_0].ucPortType == PT_UTOPIA ||
                pInitParms->PortCfg[PHY_1].ucPortType == PT_UTOPIA )
            {
                PERF->blkEnables |= SAR_CLK_EN;
                GPIO->GPIOMode &= ~(GROUP4_MASK | GROUP3_MASK | GROUP1_MASK);
                GPIO->GPIOMode |= GROUP4_UTOPIA | GROUP3_UTOPIA | GROUP1_UTOPIA;

                // Set UTOPIA clock to 50MHz
                PERF->pll_control |= MPI_UTOPIA_50MHZ;
            }

            // Calculate the bus speed.
            const UINT32 ulFreq = 64 / 4;
            UINT32 ulPllStrap = PERF->PllStrap;
            UINT32 ulNumerator = ulFreq *
                    (((ulPllStrap & PLL_N1_MASK) >> PLL_N1_SHFT) + 1) *
                    (((ulPllStrap & PLL_N2_MASK) >> PLL_N2_SHFT) + 2);
            UINT32 ulMbus =
                    (((ulPllStrap & PLL_M1_BUS_MASK) >> PLL_M1_BUS_SHFT) + 1) *
                    (((ulPllStrap & PLL_M2_BUS_MASK) >> PLL_M2_BUS_SHFT) + 4);
            ulBusSpeed = ulNumerator / ulMbus;
#elif defined(CONFIG_BCM96358)
            // If UTOPIA is configured, enable the SAR clock and appropriate
            // GPIO mode bits.
            if( pInitParms->PortCfg[PHY_0].ucPortType == PT_UTOPIA ||
                pInitParms->PortCfg[PHY_1].ucPortType == PT_UTOPIA ||
                pInitParms->PortCfg[PHY_2].ucPortType == PT_UTOPIA ||
                pInitParms->PortCfg[PHY_3].ucPortType == PT_UTOPIA )
            {
                PERF->blkEnables |= SAR_CLK_EN;
                GPIO->GPIOMode |= GPIO_MODE_UTOPIA_OVERLAY ;
            }

#ifdef CONFIG_BCM96358_UTOPIA
            if (GPIO->strap_bus & UTOPIA_MASTER_ON) {
               g_AtmInfo.OsServices.pfnPrintf ("\n\n ******** UTOPIA MASTER CONFIG ******** \n\n") ;
            }
            else {
               g_AtmInfo.OsServices.pfnPrintf ("\n\n ******** UTOPIA SLAVE CONFIG ******** \n\n") ;
            }
#endif
#if 0
            // Calculate the bus speed.
            const UINT32 ulFreq = 64 / 4;
            UINT32 ulPllStrap = PERF->PllStrap;
            UINT32 ulNumerator = ulFreq *
                    (((ulPllStrap & PLL_N1_MASK) >> PLL_N1_SHFT) + 1) *
                    (((ulPllStrap & PLL_N2_MASK) >> PLL_N2_SHFT) + 2);
            UINT32 ulMbus =
                    (((ulPllStrap & PLL_M1_BUS_MASK) >> PLL_M1_BUS_SHFT) + 1) *
                    (((ulPllStrap & PLL_M2_BUS_MASK) >> PLL_M2_BUS_SHFT) + 4);
            ulBusSpeed = ulNumerator / ulMbus;
#endif
#endif


            if( g_ulUseBothAdslPortsForShaping == 1 ||
                (pInitParms->PortCfg[PHY_1].ucPortType != PT_UTOPIA &&
                 PERF->RevID < 0x634800B0) )
            {
                // Force ADSL PHY port 1 to be enabled.  It is not really used as
                // an ADSL fast channel as is currently documented.  Rather,
                // packets that use a queue that is shaped with a PCR value are
                // sent to port 0 and packets that use a queue that is not shaped
                // with a PCR value (UBR without PCR) are sent to port 1.
                // BCM6348B0 shapes correctly sending all packets through port 0.
                pInitParms->PortCfg[1].ucPortType = PT_ADSL_FAST;
            }

            // Register the interrupt handler but enable interrupts when the link
            // comes up the first time.
            pAi->ulInterruptMapped = INTERRUPT_NOT_MAPPED;
            BcmHalInterruptDisable( INTERRUPT_ID_ATM );
            BcmHalMapInterrupt( BcmAtm_Isr, 0, INTERRUPT_ID_ATM);

            if( (baStatus = pAi->pAtmProcessor->Initialize( pInitParms,
                &pAi->OsServices, ulBusSpeed )) == STS_SUCCESS )
            {
                baStatus = BcmAtm_SetIntrHandling( pAi, pInitParms );
            }

            if( baStatus != STS_SUCCESS )
                BcmAtm_Uninitialize();
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }
    else
        baStatus = (pAi->pAtmProcessor) ? STS_PARAMETER_ERROR : STS_STATE_ERROR;

    return( baStatus );
} // BcmAtm_Initialize


//**************************************************************************
// Function Name: BcmAtm_Uninitialize
// Description  : Clean up resources allocated during BcmAtm_Initialize.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_Uninitialize( void )
{
    ATM_INFO *pAi = &g_AtmInfo;
    ATM_PROCESSOR *pAtmProc = pAi->pAtmProcessor;

    pAi->pAtmProcessor = NULL;

    // Disable and unregister the ATM interrupt.
    BcmHalInterruptDisable( INTERRUPT_ID_ATM );

    BcmAtm_ResetIntrHandling( pAi );

    // Delete the ATM Processor object.
    if( pAtmProc )
        delete pAtmProc;

    return( STS_SUCCESS );
} // BcmAtm_Uninitialize


//**************************************************************************
// Function Name: BcmAtm_GetInterfaceId
// Description  : Returns the interface id for the specified ATM port.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetInterfaceId( UINT8 ucPhyPort,
    UINT32 *pulInterfaceId )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetInterfaceId( ucPhyPort, pulInterfaceId )
        : STS_STATE_ERROR );
} // BcmAtm_GetInterfaceId


//**************************************************************************
// Function Name: BcmAtm_GetPriorityPacketGroup
// Description  : Returns offset/value entries that describe a high
//                priority packet.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 *pulPriorityPacketsSize)
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetPriorityPacketGroup( ulGroupNumber,
            pPriorityPackets, pulPriorityPacketsSize )
        : STS_STATE_ERROR );
} // BcmAtm_GetPriorityPacketGroup


//**************************************************************************
// Function Name: BcmAtm_SetPriorityPacketGroup
// Description  : Sets offset/value entries to describe a high priority packet.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 ulPriorityPacketsSize)
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->SetPriorityPacketGroup( ulGroupNumber,
            pPriorityPackets, ulPriorityPacketsSize )
        : STS_STATE_ERROR );
} // BcmAtm_SetPriorityPacketGroup


//**************************************************************************
// Function Name: BcmAtm_GetTrafficDescrTableSize
// Description  : Returns the number of entries in the Traffic Descriptor
//                Table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetTrafficDescrTableSize(
    UINT32 *pulTrafficDescrTableSize )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetTrafficDescrTableSize(
            pulTrafficDescrTableSize )
        : STS_STATE_ERROR );
} // BcmAtm_GetTrafficDescrTableSize


//**************************************************************************
// Function Name: BcmAtm_GetTrafficDescrTable
// Description  : Returns the Traffic Descriptor Table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetTrafficDescrTable(
    PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 ulTrafficDescrTableSize )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetTrafficDescrTable( pTrafficDescrTable,
            ulTrafficDescrTableSize )
        : STS_STATE_ERROR );
} // BcmAtm_GetTrafficDescrTable


//**************************************************************************
// Function Name: BcmAtm_SetTrafficDescrTable
// Description  : Saves the supplied Traffic Descriptor Table to a private
//                data member.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetTrafficDescrTable(
    PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 ulTrafficDescrTableSize )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->SetTrafficDescrTable( pTrafficDescrTable,
            ulTrafficDescrTableSize )
        : STS_STATE_ERROR );
} // BcmAtm_SetTrafficDescrTable


//**************************************************************************
// Function Name: BcmAtm_GetInterfaceCfg
// Description  : Calls the interface object for the specified interface id
//                to return the interface configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetInterfaceCfg( UINT32 ulInterfaceId,
    PATM_INTERFACE_CFG pInterfaceCfg )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetInterfaceCfg(ulInterfaceId, pInterfaceCfg)
        : STS_STATE_ERROR );
} // BcmAtm_GetInterfaceCfg


//**************************************************************************
// Function Name: BcmAtm_SetInterfaceCfg
// Description  : Calls the interface object for the specified interface id
//                to save a new interface configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetInterfaceCfg( UINT32 ulInterfaceId,
    PATM_INTERFACE_CFG pInterfaceCfg )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_INTERFACE_LINK_INFO pLinkInfo = &g_AtmInfo.InitialLinkInfo;

    if( g_AtmInfo.pAtmProcessor )
    {
        baStatus = g_AtmInfo.pAtmProcessor->SetInterfaceCfg(ulInterfaceId,
            pInterfaceCfg);

#if defined(CONFIG_BCM96348)
        // If UTOPIA is configured, enable the SAR clock and appropriate
        // GPIO mode bits.
        if( pInterfaceCfg->ulPortType == PT_UTOPIA &&
            (PERF->blkEnables & SAR_CLK_EN) != SAR_CLK_EN )
        {
            PERF->blkEnables |= SAR_CLK_EN;
            GPIO->GPIOMode &= ~(GROUP4_MASK | GROUP3_MASK | GROUP1_MASK);
            GPIO->GPIOMode |= GROUP4_UTOPIA | GROUP3_UTOPIA | GROUP1_UTOPIA;

            // Set UTOPIA clock to 50MHz
            PERF->pll_control |= MPI_UTOPIA_50MHZ;
        }
#elif defined(CONFIG_BCM96358)
        // If UTOPIA is configured, enable the SAR clock and appropriate
        // GPIO mode bits.
        if( pInterfaceCfg->ulPortType == PT_UTOPIA &&
            (PERF->blkEnables & SAR_CLK_EN) != SAR_CLK_EN )
        {
            PERF->blkEnables |= SAR_CLK_EN;
            GPIO->GPIOMode |= GPIO_MODE_UTOPIA_OVERLAY ;
        }
#endif

        if( baStatus == STS_SUCCESS &&
            pLinkInfo->ulStructureId == ID_ATM_INTERFACE_LINK_INFO &&
            pLinkInfo->ulLinkState == LINK_UP )
        {
            BcmAtm_SetInterfaceLinkInfo( ulInterfaceId, pLinkInfo );
            pLinkInfo->ulStructureId = 0;
        }
    }
    else
        baStatus = STS_STATE_ERROR;

    return( baStatus );
} // BcmAtm_SetInterfaceCfg


//**************************************************************************
// Function Name: BcmAtm_GetVccCfg
// Description  : Returns the VCC configuration record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetVccCfg( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_CFG pVccCfg )
{
   BCMATM_STATUS status ;

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId += 2 ;
#endif
    status = ( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetVccCfg( pVccAddr, pVccCfg )
        : STS_STATE_ERROR );

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId -= 2 ;
#endif

    return (status);

} // BcmAtm_GetVccCfg


//**************************************************************************
// Function Name: BcmAtm_SetVccCfg
// Description  : Saves the VCC configuration record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetVccCfg( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_CFG pVccCfg )
{
   BCMATM_STATUS status ;

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId += 2 ;
#endif

    status = ( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->SetVccCfg( pVccAddr, pVccCfg )
        : STS_STATE_ERROR );

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId -= 2 ;
#endif

    return (status);
} // BcmAtm_SetVccCfg


//**************************************************************************
// Function Name: BcmAtm_GetVccAddrs
// Description  : Returns the configured VCC addresses for an interface. 
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetVccAddrs( UINT32 ulInterfaceId, PATM_VCC_ADDR pVccAddrs,
    UINT32 ulNumVccs, UINT32 *pulNumReturned )
{
   BCMATM_STATUS status = STS_SUCCESS ;
   UINT32 ulIntfId = 0;

   *pulNumReturned = 0;
   BcmAtm_GetInterfaceId( PHY_0, &ulIntfId );

   if (ulInterfaceId == ulIntfId) {

#ifdef CONFIG_BCM96358_UTOPIA
      ulInterfaceId += 2 ;
#endif
      status = ( (g_AtmInfo.pAtmProcessor)
            ? g_AtmInfo.pAtmProcessor->GetVccAddrs( ulInterfaceId, pVccAddrs,
               ulNumVccs, pulNumReturned )
            : STS_STATE_ERROR );

#ifdef CONFIG_BCM96358_UTOPIA
      {
         int i ;
         for (i = 0 ; i < ulNumVccs ; i++) {
            pVccAddrs[i].ulInterfaceId -= 2 ;
         }
      }
#endif
   }

   return (status) ;

} // BcmAtm_GetVccAddrs


//**************************************************************************
// Function Name: BcmAtm_GetInterfaceStatistics
// Description  : Returns the statistics record for an interface.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetInterfaceStatistics( UINT32 ulInterfaceId,
    PATM_INTERFACE_STATS pStatistics, UINT32 ulReset )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetInterfaceStatistics( ulInterfaceId,
            pStatistics, ulReset )
        : STS_STATE_ERROR );
} // BcmAtm_GetInterfaceStatistics


//**************************************************************************
// Function Name: BcmAtm_GetVccStatistics
// Description  : Returns the VCC statistics record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_GetVccStatistics( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_STATS pVccStatistics, UINT32 ulReset )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->GetVccStatistics( pVccAddr, pVccStatistics,
            ulReset )
        : STS_STATE_ERROR );
} // BcmAtm_GetVccStatistics


//**************************************************************************
// Function Name: BcmAtm_SetInterfaceLinkInfo
// Description  : Calls the interface object for the specified interface id
//                to set physical link information.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetInterfaceLinkInfo( UINT32 ulInterfaceId,
    PATM_INTERFACE_LINK_INFO pInterfaceLinkInfo )
{
    // If this is the first time that any link is up, set up ISR and enable
    // interrupts.
    if( g_AtmInfo.ulInterruptMapped == INTERRUPT_NOT_MAPPED &&
        g_AtmInfo.pAtmProcessor != NULL &&
        pInterfaceLinkInfo->ulLinkState == LINK_UP )
    {
        g_AtmInfo.ulInterruptMapped = INTERRUPT_MAPPED;

        // The ATM Processor was successfully initialized.
        // Enable interrupts.
        BcmHalInterruptEnable( INTERRUPT_ID_ATM );

        // Enable ATM Processor interrupts.
        g_AtmInfo.pAtmProcessor->ConfigureInterrupts();
    }
#if defined(__KERNEL__)
    unsigned short usValue;
    if( BpGetWanDataLedGpio( &usValue ) != BP_SUCCESS )
    {
        if( pInterfaceLinkInfo->ulLinkState == LINK_UP )
            kerSysLedCtrl(kLedWanData, kLedStateFastBlinkContinues);
        else
            kerSysLedCtrl(kLedWanData, kLedStateOff);
    }
#endif

    // Save the ADSL link state.  This ASSUMES that there is only one ATM
    // physical port.
    memcpy( &g_AtmInfo.InitialLinkInfo, pInterfaceLinkInfo,
        sizeof(g_AtmInfo.InitialLinkInfo) );

    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->SetInterfaceLinkInfo( ulInterfaceId,
            pInterfaceLinkInfo )
        : STS_STATE_ERROR );
} // BcmAtm_SetInterfaceLinkInfo


//**************************************************************************
// Function Name: BcmAtm_SetNotifyCallback
// Description  : Adds the specified callback function to the list of
//                functions that are called when an ATM notification
//                event occurs.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->SetNotifyCallback( pFnNotifyCb )
        : STS_STATE_ERROR );
} // BcmAtm_SetNotifyCallback


//**************************************************************************
// Function Name: BcmAtm_ResetNotifyCallback
// Description  : Removes the specified callback function from the list of
//                functions that are called when an ATM notification
//                event occurs.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_ResetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->ResetNotifyCallback( pFnNotifyCb )
        : STS_STATE_ERROR );
} // BcmAtm_ResetNotifyCallback


//**************************************************************************
// Function Name: BcmAtm_AttachVcc
// Description  : Attaches an application to a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_AttachVcc( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
   BCMATM_STATUS status ;

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId += 2 ;
#endif
    status = ( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->AttachVcc( pVccAddr, pAttachParms )
        : STS_STATE_ERROR );

#ifdef CONFIG_BCM96358_UTOPIA
                pVccAddr->ulInterfaceId -= 2 ;
#endif

    return (status) ;
} // BcmAtm_AttachVcc


//**************************************************************************
// Function Name: BcmAtm_AttachMgmtCells
// Description  : Attaches an application to send and receive ATM managment
//                cells on any VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_AttachMgmtCells( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->AttachMgmtCells(ulInterfaceId, pAttachParms)
        : STS_STATE_ERROR );
} // BcmAtm_AttachMgmtCells


//**************************************************************************
// Function Name: BcmAtm_AttachTransparent
// Description  : Attaches an application to send and receive transparent
//                ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_AttachTransparent( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    return( (g_AtmInfo.pAtmProcessor)
        ? g_AtmInfo.pAtmProcessor->AttachTransparent(ulInterfaceId,pAttachParms)
        : STS_STATE_ERROR );
} // BcmAtm_AttachTransparent


//**************************************************************************
// Function Name: BcmAtm_Detach
// Description  : Ends an application attachment to a VCC, management cells
//                or transparent cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_Detach( UINT32 ulHandle )
{
    BCMATM_STATUS baStatus = STS_STATE_ERROR;

    if( g_AtmInfo.pAtmProcessor && ulHandle )
    {
        ((ATM_APPL *) ulHandle)->Detach();
        delete ((ATM_APPL *) ulHandle);
        baStatus = STS_SUCCESS;
    }

    return( baStatus );
} // BcmAtm_Detach


//**************************************************************************
// Function Name: BcmAtm_SetAal2ChannelIds
// Description  : Specifies a list of AAL2 channel ids that an application
//                wants to send and receive data for an AAL2 VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetAal2ChannelIds( UINT32 ulHandle,
    PATM_VCC_AAL2_CHANNEL_ID_PARMS pChannelIdParms, UINT32 ulNumChannelIdParms )
{
    return( STS_NOT_SUPPORTED );
} // BcmAtm_SetAal2ChannelIds


//**************************************************************************
// Function Name: BcmAtm_SendVccData
// Description  : Sends data on a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SendVccData( UINT32 ulHandle,
    PATM_VCC_DATA_PARMS pDataParms )
{
    return( (g_AtmInfo.pAtmProcessor && ulHandle)
        ? ((ATM_VCC_APPL *) ulHandle)->Send( pDataParms )
        : STS_STATE_ERROR );
} // BcmAtm_SendVccData


//**************************************************************************
// Function Name: BcmAtm_SendMgmtData
// Description  : Sends a managment cell.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SendMgmtData( UINT32 ulHandle,
    PATM_VCC_ADDR pVccAddr, PATM_VCC_DATA_PARMS pDataParms )
{
    return( (g_AtmInfo.pAtmProcessor && ulHandle)
        ? ((ATM_MGMT_APPL *) ulHandle)->Send( pVccAddr, pDataParms )
        : STS_STATE_ERROR );
} // BcmAtm_SendMgmtData


//**************************************************************************
// Function Name: BcmAtm_SendTransparentData
// Description  : Sends a transparent ATM cell.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SendTransparentData( UINT32 ulHandle,
    UINT32 ulInterfaceId, PATM_VCC_DATA_PARMS pDataParms )
{
    return( (g_AtmInfo.pAtmProcessor && ulHandle)
        ? ((ATM_TRANSPARENT_APPL *) ulHandle)->Send( ulInterfaceId, pDataParms )
        : STS_STATE_ERROR );
} // BcmAtm_SendTransparentData


//**************************************************************************
// Function Name: InitOsServices
// Description  : Assigns function addresses that have been filled into the
//                ATM_OS_FUNCS structure by the operating system specific
//                AtmOs"xxx" file to the ATM_OS_SERVICES structure which is
//                used by the operating system independent ATM_PROCESSOR class.
// Returns      : None.
//**************************************************************************
void InitOsServices( ATM_OS_FUNCS *pOsFuncs, ATM_OS_SERVICES *pOsServices )
{
    pOsServices->ulStructureId      = ID_ATM_OS_SERVICES;
    pOsServices->pfnAlloc           = (FN_ALLOC) pOsFuncs->pfnAlloc;
    pOsServices->pfnFree            = (FN_FREE) pOsFuncs->pfnFree;
    pOsServices->pfnDelay           = (FN_DELAY) pOsFuncs->pfnDelay;
    pOsServices->pfnCreateSem       = (FN_CREATE_SEM) pOsFuncs->pfnCreateSem;
    pOsServices->pfnRequestSem      = (FN_REQUEST_SEM) pOsFuncs->pfnRequestSem;
    pOsServices->pfnReleaseSem      = (FN_RELEASE_SEM) pOsFuncs->pfnReleaseSem;
    pOsServices->pfnDeleteSem       = (FN_DELETE_SEM) pOsFuncs->pfnDeleteSem;
    pOsServices->pfnDisableInts     = (FN_DISABLE_INTS) pOsFuncs->pfnDisableInts;
    pOsServices->pfnEnableInts      = (FN_ENABLE_INTS) pOsFuncs->pfnEnableInts;
    pOsServices->pfnInvalidateCache = (FN_INVALIDATE_CACHE)
                                      pOsFuncs->pfnInvalidateCache;
    pOsServices->pfnFlushCache      = (FN_FLUSH_CACHE) pOsFuncs->pfnFlushCache;
    pOsServices->pfnGetTopMemAddr   = (FN_GET_TOP_MEM_ADDR)
                                      pOsFuncs->pfnGetTopMemAddr;
    pOsServices->pfnBlinkLed        = (FN_BLINK_LED) pOsFuncs->pfnBlinkLed;
    pOsServices->pfnGetSystemTick   = (FN_GET_SYSTEM_TICK)
                                      pOsFuncs->pfnGetSystemTick;
    pOsServices->pfnStartTimer      = (FN_START_TIMER) pOsFuncs->pfnStartTimer;
    pOsServices->pfnPrintf          = (FN_PRINTF) pOsFuncs->pfnPrintf;
} // InitOsServices


#if defined(__KERNEL__) || defined(_CFE_)

//**************************************************************************
// The functions within this "#if defined" are only compiled in the Linux
// kernel build.
//**************************************************************************

//**************************************************************************
// Function Name: InitAdslServices (Linux version)
// Description  : Assigns function addresses of ADSL functions to the
//                ATM_OS_SERVICES structure.
// Returns      : None.
//**************************************************************************
extern "C" {

extern FN_ADSL_SET_VC_ENTRY g_pfnAdslSetVcEntry;
extern FN_ADSL_SET_ATM_LOOPBACK_MODE g_pfnAdslSetAtmLoopbackMode;
extern FN_ADSL_SET_VC_ENTRY_EX g_pfnAdslSetVcEntryEx;
extern FN_ADSL_ATM_CLEAR_VC_TABLE g_pfnAdslAtmClearVcTable;
extern FN_ADSL_ATM_ADD_VC g_pfnAdslAtmAddVc;
extern FN_ADSL_ATM_DELETE_VC g_pfnAdslAtmDeleteVc;
extern FN_ADSL_ATM_SET_MAX_SDU g_pfnAdslAtmSetMaxSdu;

void InitAdslServices( ATM_OS_SERVICES *pOsServices )
{
    pOsServices->pfnAdslSetVcEntry = (FN_ADSL_SET_VC_ENTRY) g_pfnAdslSetVcEntry;
    pOsServices->pfnAdslSetAtmLoopbackMode = (FN_ADSL_SET_ATM_LOOPBACK_MODE)
        g_pfnAdslSetAtmLoopbackMode;
    pOsServices->pfnAdslSetVcEntryEx = (FN_ADSL_SET_VC_ENTRY_EX)
        g_pfnAdslSetVcEntryEx;
#if defined(CONFIG_ATM_EOP_MONITORING)
    AtmOsPrintf( "atmapi: enable ADSL EOP monitoring\n" );
    pOsServices->pfnAdslAtmClearVcTable = (FN_ADSL_ATM_CLEAR_VC_TABLE)
        g_pfnAdslAtmClearVcTable;
    pOsServices->pfnAdslAtmAddVc = (FN_ADSL_ATM_ADD_VC) g_pfnAdslAtmAddVc;
    pOsServices->pfnAdslAtmDeleteVc = (FN_ADSL_ATM_DELETE_VC)
        g_pfnAdslAtmDeleteVc;
    pOsServices->pfnAdslAtmSetMaxSdu = (FN_ADSL_ATM_SET_MAX_SDU)
        g_pfnAdslAtmSetMaxSdu;
#endif
} // InitAdslServices
}


//**************************************************************************
// Function Name: BcmAtm_DeferredHandler (Linux version)
// Description  : Runs in the context of a Linux process that is scheduled
//                when an ATM Processor event occurs.  An ATM Processor event
//                is transmit interrupt complete, receive data ready or timeout.
// Returns      : None.
//**************************************************************************
extern "C"
void BcmAtm_DeferredHandler( ATM_INFO *pAi )
{
    if( pAi->pAtmProcessor )
    {
        pAi->pAtmProcessor->ProcessEvent();
        if( pAi->ulInterruptMapped == INTERRUPT_MAPPED )
            BcmHalInterruptEnable( INTERRUPT_ID_ATM );
    }
} // BcmAtm_DeferredHandler


//**************************************************************************
// Function Name: BcmAtm_Isr (Linux version)
// Description  : Interrupt service routine that is called when there is an
//                ATM Processor interrupt.
// Returns      : 1
//**************************************************************************
#if defined(__KERNEL__)
extern "C"
int BcmAtm_Isr(int irq, void * dev_id, struct pt_regs * regs)
{
    ATM_INFO *pAi = &g_AtmInfo;
    if( pAi->pAtmProcessor )
    {
        if( pAi->ulPrioritizeReceivePkts == 1 )
            pAi->pAtmProcessor->PrioritizeReceivePkts();
        AtmOsScheduleDeferred( pAi->ulEventId );
    }
    return( 1 );
} // BcmAtm_Isr
#else
extern "C"
unsigned int BcmAtm_Isr(unsigned int)
{
    if( g_AtmInfo.pAtmProcessor )
        AtmOsScheduleDeferred( g_AtmInfo.ulEventId );
    return( 1 );
} // BcmAtm_Isr
#endif


//**************************************************************************
// Function Name: BcmAtm_SetIntrHandling (Linux version)
// Description  : Sets up support for handling interrupts from the ATM
//                Processor.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetIntrHandling( ATM_INFO *pAi,
    PATM_INITIALIZATION_PARMS pInitParms  )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulTimeout = MAX_TIMEOUT; // actually, no timeout

    if( (pAi->ulEventId = AtmOsInitDeferredHandler( (void *)
        BcmAtm_DeferredHandler, (UINT32) pAi, ulTimeout )) == 0 )
    {
            baStatus = STS_ERROR;
    }

    return( baStatus );
} // BcmAtm_SetIntrHandling


//**************************************************************************
// Function Name: BcmAtm_ResetIntrHandling (Linux version)
// Description  : Resets support for handling interrupts from the ATM
//                Processor.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
void BcmAtm_ResetIntrHandling( ATM_INFO *pAi )
{
    if( pAi->ulEventId != 0 )
        AtmOsUninitDeferredHandler( pAi->ulEventId );
} // BcmAtm_ResetIntrHandling

#else // VxWorks

//**************************************************************************
// The functions within this "#else" are only compiled in the VxWorks build.
//**************************************************************************

//**************************************************************************
// Function Name: InitAdslServices (VxWorks version)
// Description  : Assigns function addresses of ADSL functions to the
//                ATM_OS_SERVICES structure.
// Returns      : None.
//**************************************************************************
extern "C" {

extern UINT32 BcmAdsl_SetVcEntry (int gfc, int port, int vpi, int vci);
extern UINT32 BcmAdsl_SetAtmLoopbackMode();
extern UINT32 BcmAdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci,
    int pti_clp);
extern void BcmAdsl_AtmClearVcTable(void);
extern void BcmAdsl_AtmAddVc(int vpi, int vci);
extern void BcmAdsl_AtmDeleteVc(vpi, vci);
extern void BcmAdsl_AtmSetMaxSdu(unsigned short maxsdu);

void InitAdslServices( ATM_OS_SERVICES *pOsServices )
{
    pOsServices->pfnAdslSetVcEntry = (FN_ADSL_SET_VC_ENTRY) BcmAdsl_SetVcEntry;
    pOsServices->pfnAdslSetAtmLoopbackMode = (FN_ADSL_SET_ATM_LOOPBACK_MODE)
        BcmAdsl_SetAtmLoopbackMode;
    pOsServices->pfnAdslSetVcEntryEx = (FN_ADSL_SET_VC_ENTRY_EX)
        BcmAdsl_SetVcEntryEx;
    pOsServices->pfnAdslAtmClearVcTable = (FN_ADSL_ATM_CLEAR_VC_TABLE)
        BcmAdsl_AtmClearVcTable;
    pOsServices->pfnAdslAtmAddVc = (FN_ADSL_ATM_ADD_VC) BcmAdsl_AtmAddVc;
    pOsServices->pfnAdslAtmDeleteVc = (FN_ADSL_ATM_DELETE_VC)
        BcmAdsl_AtmDeleteVc;
    pOsServices->pfnAdslAtmSetMaxSdu = (FN_ADSL_ATM_SET_MAX_SDU)
        BcmAdsl_AtmSetMaxSdu;
} // InitAdslServices
}


//**************************************************************************
// Function Name: BcmAtm_EventThread (Vxworks version)
// Description  : Runs in a separate thread of execution. Returns from blocking
//                on an event when an ATM Processor event occurs.  An event
//                is transmit interrupt complete, receive data ready or timeout.
// Returns      : None.
//**************************************************************************
extern "C"
void BcmAtm_EventThread( ATM_INFO *pAi )
{
    const UINT32 ulTimeout = MAX_TIMEOUT;

    ATM_PROCESSOR *pAtmProc = pAi->pAtmProcessor;

    for( ; ; )
    {
        AtmOsRequestSem( pAi->ulEventId, ulTimeout );
        if( pAi->ulExitThread )
            break;

        pAtmProc->ProcessEvent();

        if( pAi->ulInterruptMapped == INTERRUPT_MAPPED )
            BcmHalInterruptEnable( INTERRUPT_ID_ATM );
    }

    pAi->ulExitThread = 0;
} // BcmAtm_EventThread


//**************************************************************************
// Function Name: BcmAtm_Isr (VxWorks version)
// Description  : Interrupt service routine that is called when there is an
//                ATM Processor interrupt.  Signals the event thread task to
//                process the interrupt condition.
// Returns      : 1
//**************************************************************************
extern "C"
unsigned int BcmAtm_Isr( unsigned int )
{
    if( g_AtmInfo.pAtmProcessor )
        AtmOsReleaseSem( g_AtmInfo.ulEventId );
    return( 1 );
} // BcmAtm_Isr


//**************************************************************************
// Function Name: BcmAtm_SetIntrHandling (VxWorks version)
// Description  : Sets up support for handling interrupts from the ATM
//                Processor.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
BCMATM_STATUS BcmAtm_SetIntrHandling( ATM_INFO *pAi,
    PATM_INITIALIZATION_PARMS pInitParms  )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    // Create event that is used by the event thread function.
    if( (pAi->ulEventId = AtmOsCreateSem( 0 )) != 0 )
    {
        // Create the event thread.
        if( AtmOsCreateThread( "ATM", (void *) BcmAtm_EventThread,
            (UINT32) pAi, pInitParms->ulThreadPriority, 20 * 1024,
            &pAi->ulThreadId ) != RTN_SUCCESS )
        {
            baStatus = STS_ERROR;
        }
    }
    else
        baStatus = STS_ERROR;

    return( baStatus );
} // BcmAtm_SetIntrHandling


//**************************************************************************
// Function Name: BcmAtm_ResetIntrHandling (VxWorks version)
// Description  : Resets support for handling interrupts from the ATM
//                Processor.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
extern "C"
void BcmAtm_ResetIntrHandling( ATM_INFO *pAi )
{
    // Tell the thread to exit.
    if( pAi->ulThreadId )
    {
        pAi->ulExitThread = 1;

        AtmOsReleaseSem( pAi->ulEventId );

        // Wait up 3 seconds (300 * 10 milliseconds) for the thread to exit.
        for( UINT32 i = 0; pAi->ulExitThread == 1 && i < 300; i++ )
            AtmOsDelay( 10 );
    }

    // Delete the semaphore that was used with the thread.
    if( pAi->ulEventId != 0 )
    {
        AtmOsDeleteSem( pAi->ulEventId );
        pAi->ulEventId = 0;
    }
} // BcmAtm_ResetIntrHandling

#endif

