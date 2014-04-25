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
// File Name  : AtmProcessor.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM processor
//              class.  This is the topmost class that contains or points to
//              all objects needed to handle the processing of the ATM processor.
//
// Updates    : 12/27/2001  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"
#include "bcm_map.h"


//**************************************************************************
// ATM_PROCESSOR Class
//**************************************************************************


// ATM Processor interrupt conditions used by this class.
#define INTR_MASK (INTR_TX_QUEUE | INTR_RCQ_ALMOST_FULL | \
    INTR_RPQ_ALMOST_FULL | INTR_RCQ_WD_TIMER | INTR_RPQ_WD_TIMER | \
    INTR_RCQ_IMMED_RSP | INTR_RPQ_IMMED_RSP | INTR_ERROR_VCAM_MULT_MATCH | \
    INTR_TX_QUEUE_ENABLE_ALL)

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
extern int g_nPtmTraffic;
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

//**************************************************************************
// Function Name: ATM_PROCESSOR
// Description  : Constructor for the ATM processor class.
// Returns      : None.
//**************************************************************************
ATM_PROCESSOR::ATM_PROCESSOR( void )
    : m_Notify( &m_OsServices ),
      m_ApplTable( &m_OsServices ),
      m_VccTable( &m_OsServices ),
      m_FreePktQ( &m_OsServices ),
      m_FreeCellQ( &m_OsServices ),
      m_RxPktQ( &m_OsServices ),
      m_RxCellQ( &m_OsServices ),
      m_HighPriRxPktQ( &m_OsServices ),
      m_LowPriRxPktQ( &m_OsServices )
#if defined(CONFIG_ATM_SOFTSAR) || defined(CONFIG_ATM_RX_SOFTSAR)
      ,m_SoftSar( &m_OsServices )
#endif
{
    memset( &m_OsServices, 0x00, sizeof(m_OsServices) );
    m_pTrafficDescrEntries = NULL;
    m_ulNumAtdpEntries = 0;
    m_usFreePktQBufSize = 0;
    m_ucFreePktQBufSizeExp = 0;
    m_ulFreeQueueEmpty = 0;
    m_ulDropPktThreshold = DROP_PACKET_MINIMUM;
    m_pPktQMem = NULL;
    memset( m_pPktBufMem, 0x00, sizeof(m_pPktBufMem) );
    m_pCellQMem = NULL;
    memset( m_pCellBufMem, 0x00, sizeof(m_pCellBufMem) );
    m_ulAtmApplSem = 0;
    m_pSoftSar = NULL;
    memset( m_ulHighPriRxPktQRegs, 0x00, sizeof(m_ulHighPriRxPktQRegs) );
    memset( m_ulLowPriRxPktQRegs, 0x00, sizeof(m_ulLowPriRxPktQRegs) );
    memset( m_ulPriPktGroups, 0x00, sizeof(m_ulPriPktGroups) );
    memset( m_ulPriPktNumbers, 0x00, sizeof(m_ulPriPktNumbers) );
} // ATM_PROCESSOR


//**************************************************************************
// Function Name: ~ATM_PROCESSOR
// Description  : Destructor for the ATM processor class.
// Returns      : None.
//**************************************************************************
ATM_PROCESSOR::~ATM_PROCESSOR( void )
{
    Uninitialize();
} // ~ATM_PROCESSOR


//**************************************************************************
// Function Name: Initialize
// Description  : Initializes the object.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::Initialize( PATM_INITIALIZATION_PARMS pInitParms,
    ATM_OS_SERVICES *pOsServices, UINT32 ulBusSpeed )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 i;

    // Verify that the caller is using the same structure format as this module.
    if( pInitParms->ulStructureId == ID_ATM_INITIALIZATION_PARMS &&
        pOsServices->ulStructureId == ID_ATM_OS_SERVICES )
    {
        // Save supplied parameters to local member data.
        memcpy( &m_OsServices, pOsServices, sizeof(m_OsServices) );
        m_ulPrioritizeReceivePkts = pInitParms->ucPrioritizeReceivePkts;

        // Create semaphore for ATM_VCC_APPL, ATM_APPL_TABLE and
        // ATM_VCC_INFO_TABLE objects.
        m_ulAtmApplSem = (*m_OsServices.pfnCreateSem) (1);
        m_ApplTable.SetSem( (*m_OsServices.pfnCreateSem) (1) );
        m_VccTable.SetSem( (*m_OsServices.pfnCreateSem) (1) ); 

        // Initialize AAL layer settings.
#if defined(CONFIG_BCM96338)
        AP_REGS->ulRxAalCfg = ATMR_EN | ATMR_LINK; /* Turn off ADSL LED. */
#else
        AP_REGS->ulRxAalCfg = 0;
#endif

        // Configure maximum Service Data Units for AAL5.
        AP_REGS->ulRxAalMaxSdu = (UINT32) pInitParms->usAal5CpcsMaxSduLength;

#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
        // Initialize ATM Processor RAM/Counters.
        for( UINT32 *p = &AP_REGS->ulTxAal5CellCnt[0];
             p < AP_REGS->ulRxAal0CellCnt + AP_MAX_PORTS; p++ )
        {
            i = *p; // read to clear
        }

        // Initialize transmit ATM registers.
        AP_REGS->ulTxAtmHdrCfg = 0;
        AP_REGS->ulTxAtmSchedCfg = 0;

        // Initialize transmit table and receive CAM.
        for( i = 0; i < AP_MAX_VCIDS; i++ )
            AP_REGS->ulTxVpiVciTable[i] = 0;

        // Reset shapers.
        for( i = 0; i < AP_MAX_SHAPERS; i++ )
            AP_REGS->ulSstCtrl[i] |= SST_SHAPER_RESET;
        for( i = 0; i < 100000; i++ )
            ;
        for( i = 0; i < AP_MAX_SHAPERS; i++ )
            AP_REGS->ulSstCtrl[i] &= ~SST_SHAPER_RESET;
#endif

        for( i = 0; i < AP_MAX_VCIDS * 2; i++ )
            AP_REGS->ulRxVpiVciCam[i] = 0;

#if defined(CONFIG_BCM96358)
        // Determine ADSL port configuration.
        UINT32 ulAdslCfg;
        if(( pInitParms->PortCfg[2].ucPortType == PT_UTOPIA ) ||
           ( pInitParms->PortCfg[3].ucPortType == PT_UTOPIA ))
        {
            if (GPIO->StrapBus & UTOPIA_MASTER_ON) {
               ulAdslCfg = ALDC_TX_UTOPIA_UTOPIA | ALDC_RX_UTOPIA_UTOPIA;
            }
            else {
               ulAdslCfg = ALDC_TX_ADSL_ADSL | ALDC_RX_ADSL_ADSL;
               // ulAdslCfg |= ALDC_RX_TX_UTOPIA_ADSL_OVERLAY ;
            }
        }
        else
        {
            ulAdslCfg = ALDC_TX_ADSL_ADSL | ALDC_RX_ADSL_ADSL;
        }
#else
        // Determine ADSL port configuration.
        UINT32 ulAdslCfg;
        if( pInitParms->PortCfg[0].ucPortType == PT_UTOPIA )
        {
            ulAdslCfg = ALDC_TX_UTOPIA_UTOPIA | ALDC_RX_UTOPIA_UTOPIA;
        }
        else
        {
            ulAdslCfg = ALDC_TX_ADSL_ADSL | ALDC_RX_ADSL_ADSL;
        }
#endif

#if defined(CONFIG_ATM_SOFTSAR)
        // Soft SAR only uses ADSL PHY port 0.
        pInitParms->PortCfg[1].ucPortType = PT_DISABLED;
        m_pSoftSar = &m_SoftSar;
        baStatus = m_SoftSar.Initialize( m_ulAtmApplSem );
#elif defined(CONFIG_ATM_RX_SOFTSAR)
        m_pSoftSar = &m_SoftSar;
        baStatus = m_SoftSar.Initialize( m_ulAtmApplSem );
#endif

#if defined(CONFIG_BCM96348)
        // TBD - BCM6358
        // Calculate the Shaper Interval Time for ADSL and UTOPIA.  The SIT value
        // is in 100 ns increments and the xxx_SIT_UNIT_OF_TIME constant is
        // "* 100".  Therefore the SIT value is divided by 100 * 100 = 10,000. 
        const UINT32 ulScalar = 20;
        m_ulAdslSit = (ulBusSpeed * ADSL_SIT_UNIT_OF_TIME * ulScalar) / 10000;
        m_ulUtopiaSit = (ulBusSpeed * UTOPIA_SIT_UNIT_OF_TIME * ulScalar) / 10000;
        AP_REGS->ulTxAtmSchedCfg = 0;
        if( pInitParms->PortCfg[1].ucPortType != PT_ADSL_FAST )
            AP_REGS->ulTxAtmSchedCfg |= ATMR_EARLY_SCHED_EN;
#elif defined(CONFIG_BCM96358)
        AP_REGS->ulTxAtmSchedCfg = 0;
        if( pInitParms->PortCfg[1].ucPortType != PT_ADSL_FAST )
            AP_REGS->ulTxAtmSchedCfg |= ATMR_EARLY_SCHED_EN;
#endif

        if( baStatus == STS_SUCCESS )
        {
            // Initialize the ATM interfaces.
            for( i = 0; i < MAX_INTERFACES; i++ )
            {
                baStatus = m_Interfaces[i].Initialize(&m_OsServices, i,
                    &pInitParms->PortCfg[i], ulAdslCfg,&m_ApplTable,m_pSoftSar);
            }
        }

        // Determine queue sizes.  If a queue size of zero is passed in the
        // initialization record, then this function determines a reasonable
        // value to use.
        UINT32 ulFreePktQSize = (pInitParms->usFreePktQSize == 0)
            ? DEFAULT_FREE_PKT_Q_SIZE : pInitParms->usFreePktQSize;
        UINT32 ulFreeCellQSize = (pInitParms->usFreeCellQSize == 0)
            ? DEFAULT_FREE_CELL_Q_SIZE : pInitParms->usFreeCellQSize;
        UINT32 ulReceivePktQSize = (pInitParms->usReceivePktQSize == 0)
            ? DEFAULT_RECEIVE_PKT_Q_SIZE : pInitParms->usReceivePktQSize;
        UINT32 ulReceiveCellQSize = (pInitParms->usReceiveCellQSize == 0)
            ? DEFAULT_RECEIVE_CELL_Q_SIZE : pInitParms->usReceiveCellQSize;

#if defined(CONFIG_ATM_RX_SOFTSAR)
        // A software receive SAR will receive a lot more cells than the
        // caller realizes so increase the cell queue size.
        if( ulFreeCellQSize < ulFreePktQSize / 2 )
            ulFreeCellQSize = ulReceiveCellQSize = ulFreePktQSize / 2;

        // Adjust the number of allocated receive data buffers.  Reduce the
        // buffers on the HW free queue and add buffers for the SW receive SAR.
        UINT32 ulSsReceivePktQSize;
        if( MAX_RX_VCIDS )
        {
            ulFreePktQSize /= 2;
            ulReceivePktQSize /= 2;
            ulSsReceivePktQSize = ulReceivePktQSize;
        }
        else
        {
            // No HW SAR receive resassembly.
            ulSsReceivePktQSize = (ulReceivePktQSize * 2) / 3;
            ulFreePktQSize = ulReceivePktQSize = 10;
        }
#endif

        // Create queues.
        m_FreePktQ.Create( &AP_REGS->ulFreePktQAddr, ulFreePktQSize );
        m_FreeCellQ.Create( &AP_REGS->ulFreeCellQAddr, ulFreeCellQSize );
        m_RxPktQ.Create( &AP_REGS->ulRxPktQAddr, ulReceivePktQSize );
        m_RxCellQ.Create( &AP_REGS->ulRxCellQAddr, ulReceiveCellQSize );
        m_HighPriRxPktQ.Create( (volatile UINT32 *) m_ulHighPriRxPktQRegs,
            ulReceivePktQSize );
        m_LowPriRxPktQ.Create( (volatile UINT32 *) m_ulLowPriRxPktQRegs,
            ulReceivePktQSize );

        // Determine the receive queue exponent.
        const UINT16 usCellPayloadSize = 48;
        UINT16 usInitParmsSize;

        if( pInitParms->usFreePktQBufferSize >= usCellPayloadSize )
            usInitParmsSize = pInitParms->usFreePktQBufferSize;
        else
            usInitParmsSize = DEFAULT_PKT_BUF_SIZE; 

        CalculateReceiveBufferSize(usInitParmsSize, &m_usFreePktQBufSize,
            &m_ucFreePktQBufSizeExp);

        if( m_OsServices.pfnAdslAtmSetMaxSdu )
            (*m_OsServices.pfnAdslAtmSetMaxSdu) (m_usFreePktQBufSize);

#if defined(CONFIG_ATM_RX_SOFTSAR)
        if( baStatus == STS_SUCCESS )
        {
#if defined(__KERNEL__)
            UINT16 usBufSize = m_usFreePktQBufSize;
#else
            UINT16 usBufSize = pInitParms->usFreePktQBufferSize;
#endif
            baStatus = m_SoftSar.CreateRxBufs( ulSsReceivePktQSize, usBufSize,
                pInitParms->usFreePktQBufferOffset );
        }
#endif

        if( baStatus == STS_SUCCESS )
        {
            // Allocate memory for, and initialize, ATM buffer descriptor queues.
            baStatus = CreateBdQ( &m_pPktQMem, m_pPktBufMem, &m_FreePktQ,
                ulFreePktQSize, m_usFreePktQBufSize, m_ucFreePktQBufSizeExp,
                pInitParms->usFreePktQBufferOffset, FreePkt );
        }

        if( baStatus == STS_SUCCESS )
        {
            baStatus = CreateBdQ( &m_pCellQMem, m_pCellBufMem, &m_FreeCellQ,
                ulFreeCellQSize, FREE_CELL_Q_BUF_SIZE,
                FREE_CELL_Q_BUF_SIZE_EXP, 0, FreeCell );
        }

        if( baStatus == STS_SUCCESS )
        {
            // Reduce watermark level for faster TCP ACKs
            ulFreePktQSize -= 2;
            ulFreeCellQSize = 1;
            ulReceivePktQSize = 1;
            ulReceiveCellQSize = 1;

            AP_REGS->ulIrqRxFreeCellQWatermark =
                ((ulReceiveCellQSize << INTR_RX_WM_SHIFT) & INTR_RX_WM_MASK) |
                ((ulFreeCellQSize << INTR_FREE_WM_SHIFT) & INTR_FREE_WM_MASK);

            AP_REGS->ulIrqRxFreePktQWatermark =
                ((ulReceivePktQSize << INTR_RX_WM_SHIFT) & INTR_RX_WM_MASK) |
                ((ulFreePktQSize << INTR_FREE_WM_SHIFT) & INTR_FREE_WM_MASK);

            // Set receive cell and packet queue watchdog timer to 1ms.
            // 1000us / 50us interval = 20 = 0x0014
            AP_REGS->ulIrqRxQWatchdog = 0x00140014;

#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
            // Set transmit queue watchdog timer to interrupt immediately.
            // This must be done to shape correctly.
            AP_REGS->ulIrqTxQWatchdog = 0x00;
#endif

            // Mask interrupts.
            AP_REGS->ulIrqMask = ~0;

            // Clear interrupts.
            AP_REGS->ulIrqStatus = ~0;
        }
        else
            Uninitialize();
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // Initialize


//**************************************************************************
// Function Name: Uninitialize
// Description  : Return resources allocated by this object or contained
//                objects.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::Uninitialize( void )
{
    UINT32 i;

    // Disable interrupts.
    AP_REGS->ulIrqMask = ~0;

    // Detach all applications.
    UINT32 ulIdx = 0;
    ATM_APPL *pAppl = m_ApplTable.EnumAppl( &ulIdx );
    while( pAppl )
    {
        pAppl->Detach();
        pAppl = m_ApplTable.EnumAppl( &ulIdx );
    }

    if( m_ulAtmApplSem )
    {
        (*m_OsServices.pfnDeleteSem) (m_ulAtmApplSem);
        m_ulAtmApplSem = 0;
    }

    // Uninitialize the ATM interfaces.
    for( i = 0; i < MAX_INTERFACES; i++ )
        m_Interfaces[i].Uninitialize();

    // Check if any outstanding receive packets are pending at the upper layers.
    UINT32 ulElementSize = (sizeof(ATM_DMA_BD) + sizeof(ATM_BUFFER) +
        sizeof(ATM_VCC_DATA_PARMS) + 0x0f) & ~0x0f;
    char *p;
    UINT32 ulRxPending, ulSize;
    UINT32 ulDelayCount = 0;

    if( m_pPktQMem && m_pCellQMem )
    {
        char *pPktQMem = (char *) (((UINT32) m_pPktQMem + 0x0f) & ~0x0f);
        char *pCellQMem = (char *) (((UINT32) m_pCellQMem + 0x0f) & ~0x0f);
        do
        {
            ulRxPending = 0;
            ulSize = m_FreePktQ.GetSize() - 1;
            for( i = 0, p = pPktQMem + sizeof(ATM_DMA_BD) + sizeof(ATM_BUFFER);
                i < ulSize; i++, p +=  ulElementSize )
            {
                if( (((PATM_VCC_DATA_PARMS) p)->ucFlags & RX_DATA_PENDING) != 0 )
                {
                    ulRxPending = 1;
                    break;
                }
            }

            if( ulRxPending == 0 )
            {
                ulSize = m_FreeCellQ.GetSize() - 1;
                for(i = 0, p = pCellQMem+sizeof(ATM_DMA_BD)+sizeof(ATM_BUFFER);
                    i < ulSize; i++, p +=  ulElementSize)
                {
                    if((((PATM_VCC_DATA_PARMS) p)->ucFlags&RX_DATA_PENDING) != 0)
                    {
                        ulRxPending = 1;
                        break;
                    }
                }
            }

            // Wait for pending receive buffers to be returned for up to five
            // seconds (500ms * 10).
            if( ulRxPending )
                (*m_OsServices.pfnDelay) (500);

        } while( ulRxPending && ++ulDelayCount < 10 );
    }

    m_FreePktQ.Destroy();
    m_FreeCellQ.Destroy();
    m_RxPktQ.Destroy();
    m_RxCellQ.Destroy();
    m_HighPriRxPktQ.Destroy();
    m_LowPriRxPktQ.Destroy();

    if( m_pPktQMem )
    {
        (*m_OsServices.pfnFree) (m_pPktQMem);
        m_pPktQMem = NULL;
    }

    for( i = 0; m_pPktBufMem[i] != NULL; i++ )
    {
        (*m_OsServices.pfnFree) (m_pPktBufMem[i]);
        m_pPktBufMem[i] = NULL;
    }

    if( m_pCellQMem )
    {
        (*m_OsServices.pfnFree) (m_pCellQMem);
        m_pCellQMem = NULL;
    }

    for( i = 0; m_pCellBufMem[i] != NULL; i++ )
    {
        (*m_OsServices.pfnFree) (m_pCellBufMem[i]);
        m_pCellBufMem[i] = NULL;
    }

    return( STS_SUCCESS );
} // Uninitialize


//**************************************************************************
// Function Name: GetInterfaceId
// Description  : Returns the interface id for the specified ATM port.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetInterfaceId( UINT8 ucPhyPort,
    UINT32 *pulInterfaceId )
{
    BCMATM_STATUS baStatus;

    if( ucPhyPort < MAX_INTERFACES )
    {
        *pulInterfaceId = m_Interfaces[ucPhyPort].GetInterfaceId();
        baStatus = STS_SUCCESS;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // GetInterfaceId


//**************************************************************************
// Function Name: GetPriorityPacketGroup
// Description  : Returns offset/value entries that describe a high
//                priority packet.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 *pulPriorityPacketsSize)
{
    BCMATM_STATUS baStatus = STS_PARAMETER_ERROR;

    if( ulGroupNumber >= 1 && ulGroupNumber <= NUM_PRI_PKT_GROUPS &&
        pPriorityPackets->ulStructureId == ID_ATM_PRIORITY_PACKET_ENTRY )
    {
        UINT32 ulSize = 0;
        PATM_PRIORITY_PACKET_ENTRY pEntry = m_ulPriPktGroups[ulGroupNumber - 1];
        PATM_PRIORITY_PACKET_ENTRY pEntryBase = pEntry;

        // Determine how many entries are in the priority packet group.
        while( pEntry->ulStructureId == ID_ATM_PRIORITY_PACKET_ENTRY )
        {
            ulSize++;
            pEntry++;
        }

        if( *pulPriorityPacketsSize >= ulSize )
        {
            // Copy the priority packet group entries to the caller's buffer.
            memset( pPriorityPackets, 0x00,
                sizeof(ATM_PRIORITY_PACKET_ENTRY) * *pulPriorityPacketsSize );
            memcpy( pPriorityPackets, pEntryBase,
                sizeof(ATM_PRIORITY_PACKET_ENTRY) * ulSize );
            baStatus = STS_SUCCESS;
        }

        *pulPriorityPacketsSize = ulSize;
    }

    return(baStatus);
} // GetPriorityPacketGroup


//**************************************************************************
// Function Name: BcmAtm_SetPriorityPacketGroup
// Description  : Sets offset/value entries to describe a high priority packet.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 ulPriorityPacketsSize)
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( ulGroupNumber >= 1 && ulGroupNumber <= NUM_PRI_PKT_GROUPS &&
        ulPriorityPacketsSize <= NUM_ENTRIES_PER_PRI_PKT_GROUP )
    {
        UINT32 i;
        PATM_PRIORITY_PACKET_ENTRY pEntry = m_ulPriPktGroups[ulGroupNumber - 1];
        PATM_PRIORITY_PACKET_ENTRY pEntryBase = pEntry;

        memset( pEntryBase, 0x00, sizeof(ATM_PRIORITY_PACKET_ENTRY) *
            NUM_ENTRIES_PER_PRI_PKT_GROUP );

        if( pPriorityPackets && ulPriorityPacketsSize > 0 )
        {
            // Add/replace the priority packet entries for the specified group.
            if( ulPriorityPacketsSize <= NUM_ENTRIES_PER_PRI_PKT_GROUP )
            {
                for( i = 0; i < ulPriorityPacketsSize; i++, pPriorityPackets++,
                     pEntry++ )
                {
                    if( pPriorityPackets->ulStructureId ==
                            ID_ATM_PRIORITY_PACKET_ENTRY &&
                        pPriorityPackets->ulPacketOffset < m_usFreePktQBufSize )
                    {
                        memcpy( pEntry, pPriorityPackets,
                            sizeof(ATM_PRIORITY_PACKET_ENTRY) );
                    }
                    else
                    {
                        baStatus = STS_PARAMETER_ERROR;
                        break;
                    }
                }
            }
            else
                baStatus = STS_PARAMETER_ERROR;
        }

        // If there are no entries in this priority group, remove the group
        // number from the VCCs that are are using it.
        if( pEntryBase->ulStructureId != ID_ATM_PRIORITY_PACKET_ENTRY )
        {
            UINT32 *pulGroupNum;
            for( i = 0; i < MAX_VCIDS; i++ )
            {
                pulGroupNum = m_ulPriPktNumbers[i];
                while( *pulGroupNum )
                {
                    if( *pulGroupNum == ulGroupNumber )
                    {
                        // Remove the group number by left shifting the
                        // remaining group numbers by one.
                        UINT32 *pul = pulGroupNum + 1;

                        do
                        {
                            *pulGroupNum++ = *pul;
                        }while( *pul++ );

                        break;
                    }

                    pulGroupNum++;
                }
            }
        }
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return(baStatus);
} // SetPriorityPacketGroup


//**************************************************************************
// Function Name: GetTrafficDescrTableSize
// Description  : Returns the number of entries in the Traffic Descriptor
//                Table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetTrafficDescrTableSize(
    UINT32 *pulTrafficDescrTableSize )
{
    *pulTrafficDescrTableSize = m_ulNumAtdpEntries;
    return( STS_SUCCESS );
} // GetTrafficDescrTableSize


//**************************************************************************
// Function Name: GetTrafficDescrTable
// Description  : Returns the Traffic Descriptor Table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetTrafficDescrTable(
    PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 ulTrafficDescrTableSize )
{
    BCMATM_STATUS baStatus;

    // Verify that the supplied Traffic Descriptor Table is big enough and
    // that the caller is using the same structure format as this module.
    if( ulTrafficDescrTableSize >= m_ulNumAtdpEntries &&
        pTrafficDescrTable[0].ulStructureId==ID_ATM_TRAFFIC_DESCR_PARM_ENTRY )
    {
        if( m_pTrafficDescrEntries )
        {
            // Copy the table.
            memcpy( pTrafficDescrTable, m_pTrafficDescrEntries,
                m_ulNumAtdpEntries * sizeof(ATM_TRAFFIC_DESCR_PARM_ENTRY) );
            baStatus = STS_SUCCESS;
        }
        else
            baStatus = STS_NOT_FOUND;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // GetTrafficDescrTable


//**************************************************************************
// Function Name: SetTrafficDescrTable
// Description  : Saves the supplied Traffic Descriptor Table to a private
//                data member.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetTrafficDescrTable(
    PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 ulTrafficDescrTableSize )
{
    BCMATM_STATUS baStatus;

    // Verify that the caller is using the same structure format as this module.
    if( pTrafficDescrTable[0].ulStructureId==ID_ATM_TRAFFIC_DESCR_PARM_ENTRY ||
        ulTrafficDescrTableSize == 0 )
    {
        // Free an existing table if it exists.
        if( m_pTrafficDescrEntries )
        {
            (*m_OsServices.pfnFree) (m_pTrafficDescrEntries);
            m_pTrafficDescrEntries = NULL;
            m_ulNumAtdpEntries = 0;
        }

        UINT32 ulSize;
        if( ulTrafficDescrTableSize )
            ulSize = ulTrafficDescrTableSize*sizeof(ATM_TRAFFIC_DESCR_PARM_ENTRY);
        else
            ulSize = sizeof(ATM_TRAFFIC_DESCR_PARM_ENTRY);

        // Allocate memory for the new table.
        m_pTrafficDescrEntries = (PATM_TRAFFIC_DESCR_PARM_ENTRY)
            (*m_OsServices.pfnAlloc) (ulSize);

        // Copy the table.
        if( m_pTrafficDescrEntries )
        {
            m_ulNumAtdpEntries = ulTrafficDescrTableSize;
            memcpy( m_pTrafficDescrEntries, pTrafficDescrTable, ulSize );

            for( UINT32 i = 0; i < ulTrafficDescrTableSize; i++ )
                m_pTrafficDescrEntries[i].ulTrafficDescrRowStatus = TDRS_ACTIVE;

            // Update ATM_VCC_APPL objects with the new table address.
            UINT32 ulIdx = 0;
            ATM_APPL *pAppl = m_ApplTable.EnumAppl( &ulIdx );
            while( pAppl )
            {
                pAppl->SetTrafficDescrTable( m_pTrafficDescrEntries,
                    m_ulNumAtdpEntries );
                pAppl = m_ApplTable.EnumAppl( &ulIdx );
            }

            baStatus = STS_SUCCESS;
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // SetTrafficDescrTable


//**************************************************************************
// Function Name: GetInterfaceCfg
// Description  : Calls the interface object for the specified interface id
//                to return the interface configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetInterfaceCfg( UINT32 ulInterfaceId,
    PATM_INTERFACE_CFG pInterfaceCfg )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;

    // Find the interface object for the supplied interface id.
    for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
    {
        if( m_Interfaces[i].GetInterfaceId() == ulInterfaceId )
        {
            // Interface object found.  Call its member function that
            // returns the configuration record.
            baStatus = m_Interfaces[i].GetCfg( pInterfaceCfg, &m_VccTable );
            break;
        }
    }

    return( baStatus );
} // GetInterfaceCfg


//**************************************************************************
// Function Name: SetInterfaceCfg
// Description  : Calls the interface object for the specified interface id
//                to save a new interface configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetInterfaceCfg( UINT32 ulInterfaceId,
    PATM_INTERFACE_CFG pInterfaceCfg )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;
    UINT8 ucPort0Type = m_Interfaces[PHY_0].GetPortType();
    UINT8 ucPort1Type = m_Interfaces[PHY_1].GetPortType();

    // Find the interface object for the supplied interface id.
    for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
    {
        // Port 1 can only be configured from the caller if it is not using
        // the ADSL PHY.
        if( (i != PHY_1 || ucPort1Type != PT_ADSL_FAST) &&
            m_Interfaces[i].GetInterfaceId() == ulInterfaceId )
        {
#if defined(CONFIG_BCM96348)
            // Set the Shaper Interval Timer based on port 0 configuration
            // type. TBD - one SIT value is needed for both ADSL and UTOPIA.
            if( i == PHY_0 )
            {
                AP_REGS->ulTxAtmSchedCfg &= ~ATMR_SIT_MASK;
                if( pInterfaceCfg->ulPortType == PT_UTOPIA )
                {
                    AP_REGS->ulTxAtmSchedCfg |= ATMR_SIT_MODE |
                        (m_ulUtopiaSit << ATMR_SIT_SHIFT);
                    ATM_SHAPER::SetSitUt( UTOPIA_SIT_UNIT_OF_TIME );
                }
                else
                {
                    AP_REGS->ulTxAtmSchedCfg |= ATMR_SIT_MODE |
                        (m_ulAdslSit << ATMR_SIT_SHIFT);
                    ATM_SHAPER::SetSitUt( ADSL_SIT_UNIT_OF_TIME );
                }

                for( UINT32 j = 0; j < MAX_QUEUE_SHAPERS; j++ )
                {
                    ATM_TX_BDQ *pTxQ;
                    if( (pTxQ = m_ApplTable.GetTxQByShaperNum( i )) != NULL )
                        pTxQ->ReconfigureShaper();
                }
            }
#endif

            // Interface object found.  Call its member function that
            // saves the configuration record.
            baStatus = m_Interfaces[i].SetCfg( pInterfaceCfg );

            // If port 0 is being configured to use the ADSL PHY and port 1
            // is also initialized for the ADSL PHY, then also configure port 1.
            // This port is used to send UBR shaped data.
            if( i == PHY_0 && ucPort0Type == PT_ADSL_INTERLEAVED &&
                ucPort1Type == PT_ADSL_FAST )
            {
                m_Interfaces[PHY_1].SetCfg( pInterfaceCfg );
            }
            break;
        }
    }

    return( baStatus );
} // SetInterfaceCfg

//**************************************************************************
// Function Name: GetVccCfg
// Description  : Returns the VCC configuration record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetVccCfg( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_CFG pVccCfg )
{
    BCMATM_STATUS baStatus;

    // Verify that the caller is using the same structure format as this module.
    if( pVccCfg->ulStructureId == ID_ATM_VCC_CFG )
    {
        // Get the VCC information structure for the supplied VCC address.
        ATM_VCC_INFO *pInfo = m_VccTable.Get( pVccAddr );

        if( pInfo )
        {
            // Copy the configuration record to the supplied buffer.
            memcpy( pVccCfg, &pInfo->Cfg, sizeof(pInfo->Cfg) );

            // If the interface status is down, the the VCC status must also be
            // down.
            if( pInfo->pInterface->IsInterfaceUp() == 0 ||
                pInfo->pInterface->IsLinkUp() == 0 )
            {
                pVccCfg->ulAtmVclOperStatus = OPRSTS_DOWN;
            }

            // Copy priority packet group numbers which are kept in this object.
            memcpy( pVccCfg->ulPriorityPacketGroupNumbers,
                m_ulPriPktNumbers[pInfo->ucVcid], NUM_PRI_PKT_GROUPS );

            baStatus = STS_SUCCESS;
        }
        else
            baStatus = STS_NOT_FOUND;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // GetVccCfg


//**************************************************************************
// Function Name: SetVccCfg
// Description  : Saves the VCC configuration record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetVccCfg( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_CFG pVccCfg )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( pVccCfg == NULL )
        m_VccTable.Remove( pVccAddr );
    else
    {
        // Verify that the caller is using the same struct format as this module.
        if( pVccCfg->ulStructureId == ID_ATM_VCC_CFG )
        {
            baStatus = STS_NOT_FOUND;

            // Find the interface object for the VCC address.
            for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
            {
                if( m_Interfaces[i].GetInterfaceId() == pVccAddr->ulInterfaceId )
                {
                    // Interface object was found. Add the VCC address,
                    // VCC configuration record and interface object address
                    // to the VCC information table.
                    ATM_VCC_CFG CurrentCfg = {ID_ATM_VCC_CFG};

                    baStatus = GetVccCfg( pVccAddr, &CurrentCfg );
                    if( baStatus != STS_SUCCESS ||
                        CurrentCfg.ulAtmVclAdminStatus !=
                        pVccCfg->ulAtmVclAdminStatus )
                    {
                        // There is a change in admin status.
                        pVccCfg->ulAtmVclLastChange =
                            (*m_OsServices.pfnGetSystemTick) () / 10;
                    }

                    // Set to VCC operation status based on the VCC admin status.
                    if( pVccCfg->ulAtmVclAdminStatus == ADMSTS_UP )
                        pVccCfg->ulAtmVclOperStatus = OPRSTS_UP;
                    else
                        pVccCfg->ulAtmVclOperStatus = OPRSTS_DOWN;

                    baStatus = m_VccTable.AddReplace( pVccAddr, pVccCfg,
                        &m_Interfaces[i] );

                    // Copy priority packet group numbers which are kept in
                    // this object.
                    ATM_VCC_INFO *pInfo = m_VccTable.Get( pVccAddr );
                    if( pInfo )
                    {
                        memcpy( m_ulPriPktNumbers[pInfo->ucVcid],
                            pVccCfg->ulPriorityPacketGroupNumbers,
                            NUM_PRI_PKT_GROUPS );
                    }
                    break;
                }
            }
        }
        else
            baStatus = STS_PARAMETER_ERROR;
    }

    return( baStatus );
} // SetVccCfg


//**************************************************************************
// Function Name: GetVccAddrs
// Description  : Updates the ATM Processor shaper registers for the new
//                line rate.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetVccAddrs( UINT32 ulInterfaceId,
    PATM_VCC_ADDR pVccAddrs, UINT32 ulNumVccs, UINT32 *pulNumReturned )
{
    return( m_VccTable.GetAllAddrs( ulInterfaceId, pVccAddrs, ulNumVccs,
        pulNumReturned ) );
} // GetVccAddrs


//**************************************************************************
// Function Name: GetInterfaceStatistics
// Description  : Returns the statistics record for an interface.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetInterfaceStatistics( UINT32 ulInterfaceId,
    PATM_INTERFACE_STATS pStatistics, UINT32 ulReset )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;

    // Find the interface object for the supplied interface id.
    for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
    {
        if( m_Interfaces[i].GetInterfaceId() == ulInterfaceId )
        {
            // Interface object found.  Call its member function that
            // returns the statistics record.
            baStatus = m_Interfaces[i].GetStats( pStatistics, ulReset );
            if( baStatus == STS_SUCCESS )
            {
                pStatistics->Aal5IntfStats.ulIfInPriPkts = m_ulPriPktsReceived;
                if( ulReset )
                    m_ulPriPktsReceived = 0;
            }
            break;
        }
    }

    return( baStatus );
} // GetInterfaceStatistics


//**************************************************************************
// Function Name: GetVccStatistics
// Description  : Returns the VCC statistics record for the specified VCC
//                address.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::GetVccStatistics( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_STATS pVccStatistics, UINT32 ulReset )
{
    BCMATM_STATUS baStatus;

    // Get the VCC information structure for the supplied VCC address.
    ATM_VCC_INFO *pInfo = m_VccTable.Get( pVccAddr );

    if( pInfo )
    {
        if( pVccStatistics->ulStructureId == ID_ATM_VCC_STATS )
        {
            pInfo->pInterface->LockStats();

            memcpy( pVccStatistics, &pInfo->Stats, sizeof(ATM_VCC_STATS) );
            if( ulReset )
            {
                UINT32 ulAalType = pInfo->Stats.ulAalType;

                memset( &pInfo->Stats, 0x00, sizeof(ATM_VCC_STATS) );
                pInfo->Stats.ulStructureId = ID_ATM_VCC_STATS;
                pInfo->Stats.ulAalType = ulAalType;
            }

            pInfo->pInterface->UnlockStats();

            baStatus = STS_SUCCESS;
        }
        else
            baStatus = STS_PARAMETER_ERROR;
    }
    else
        baStatus = STS_NOT_FOUND;

    return( baStatus );
} // GetVccStatistics


//**************************************************************************
// Function Name: SetInterfaceLinkInfo
// Description  : Calls the interface object for the specified interface id
//                to save a new interface configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetInterfaceLinkInfo( UINT32 ulInterfaceId,
    PATM_INTERFACE_LINK_INFO pInterfaceLinkInfo )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;

    if( pInterfaceLinkInfo->ulLinkState == LINK_UP &&
        pInterfaceLinkInfo->ulLineRate == 0 )
    {
        baStatus = STS_PARAMETER_ERROR;
    }
    else
    {
#if defined(CONFIG_ATM_SOFTSAR)
        m_SoftSar.SetConnectionRate( pInterfaceLinkInfo->ulLineRate );
#endif
        // Find the interface object for the supplied interface id.
        UINT32 i;
        ATM_INTERFACE *pIntf = m_Interfaces;
        for( i = 0, pIntf = m_Interfaces; i < MAX_INTERFACES; i++, pIntf++ )
        {
            if( pIntf->GetInterfaceId() == ulInterfaceId )
            {
                // Set the link up/down.
                baStatus = pIntf->SetLinkInfo(pInterfaceLinkInfo->ulLinkState);

                if( baStatus == STS_SUCCESS )
                {
                    // Notify applications of the link change.
                    ATM_NOTIFY_PARMS NotifyParms;
                    PAN_INTF_CHANGE_PARMS pIntfChange =
                        &NotifyParms.u.IntfChangeParms;

                    NotifyParms.ulNotifyType = ATM_NOTIFY_INTERFACE_CHANGE;

                    if( pInterfaceLinkInfo->ulLinkState == LINK_UP )
                        pIntfChange->ulInterfaceState = ATM_INTERFACE_UP;
                    else
                        pIntfChange->ulInterfaceState = ATM_INTERFACE_DOWN;

                    pIntfChange->ulInterfaceLineRate =
                        pInterfaceLinkInfo->ulLineRate;

                    m_Notify.SendEvent( &NotifyParms );
                }
                break;
            }
        }

        // Update ATM_VCC_APPL objects with change in operational status.
        UINT32 ulLastChange = (*m_OsServices.pfnGetSystemTick) () / 10;
        ATM_APPL *pAppl = m_ApplTable.EnumAppl( &i );
        while( pAppl )
        {
            pAppl->SetVclLastChange( ulLastChange );
            pAppl = m_ApplTable.EnumAppl( &i );
        }
    }

    return( baStatus );
} // SetInterfaceLinkInfo


//**************************************************************************
// Function Name: SetNotifyCallback
// Description  : Adds the specified callback function to the list of
//                functions that are called when an ATM notification
//                event occurs.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::SetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    return( m_Notify.AddCallback( pFnNotifyCb ) );
} // SetNotifyCallback


//**************************************************************************
// Function Name: ResetNotifyCallback
// Description  : Removes the specified callback function from the list of
//                functions that are called when an ATM notification
//                event occurs.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::ResetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    return( m_Notify.RemoveCallback( pFnNotifyCb ) );
} // ResetNotifyCallback


//**************************************************************************
// Function Name: AttachVcc
// Description  : Attaches an application to a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::AttachVcc( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus;

    // Get the VCC information structure for the supplied VCC address.
    ATM_VCC_INFO *pInfo = m_VccTable.Get( pVccAddr );

    if( pInfo )
    {
        // Only one application at a time can attach to a particular VCC.
        if( m_ApplTable.GetByVccAddr( &pInfo->Addr ) == NULL )
        {
            // The VCC is available.  Create a new application object and
            // call its attach member function which sets up the interface
            // with the ATM processor.
            ATM_VCC_APPL *pVccAppl;

            pVccAppl = new ATM_VCC_APPL( &m_OsServices, &m_ApplTable,
                m_ulAtmApplSem, m_pSoftSar );
            if( pVccAppl )
            {
                pVccAppl->SetTrafficDescrTable( m_pTrafficDescrEntries,
                    m_ulNumAtdpEntries );

                baStatus = pVccAppl->Attach( pInfo, pAttachParms );
                if( baStatus != STS_SUCCESS )
                    delete pVccAppl;
            }
            else
                baStatus = STS_ALLOC_ERROR;
        }
        else
            baStatus = STS_IN_USE;
    }
    else
        baStatus = STS_NOT_FOUND;

    return( baStatus );
} // AttachVcc


//**************************************************************************
// Function Name: AttachMgmtCells
// Description  : Attaches an application to send and receive ATM managment
//                cells on any VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::AttachMgmtCells( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    // If the interface id is not the constant ALL_INTERFACES, then find
    // the interface object for the supplied interface id.
    ATM_INTERFACE *pInterfaces = m_Interfaces;
    UINT32 ulNumInterfaces = MAX_INTERFACES;
    if( ulInterfaceId != ALL_INTERFACES )
    {
        baStatus = STS_NOT_FOUND;
        for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
        {
            if( m_Interfaces[i].GetInterfaceId() == ulInterfaceId )
            {
                // Interface object found.
                pInterfaces = &m_Interfaces[i];
                ulNumInterfaces = 1;
                baStatus = STS_SUCCESS;
                break;
            }
        }
    }

    if( baStatus == STS_SUCCESS )
    {
        // Create a new application object and call its attach member function
        // which allows it to send and receive managment cells on any VCC.
        ATM_MGMT_APPL *pMgmtAppl;

        pMgmtAppl = new ATM_MGMT_APPL( &m_OsServices, &m_ApplTable,
            m_ulAtmApplSem, m_pSoftSar );
        if( pMgmtAppl )
        {
            baStatus = pMgmtAppl->Attach( pInterfaces, ulNumInterfaces,
                pAttachParms );

            if( baStatus != STS_SUCCESS )
                delete pMgmtAppl;
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }

    return( baStatus );
} // AttachMgmtCells


//**************************************************************************
// Function Name: AttachTransparent
// Description  : Attaches an application to send and receive transparent
//                ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::AttachTransparent( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    // If the interface id is not the constant ALL_INTERFACES, then find
    // the interface object for the supplied interface id.
    ATM_INTERFACE *pInterfaces = m_Interfaces;
    UINT32 ulNumInterfaces = MAX_INTERFACES;
    if( ulInterfaceId != ALL_INTERFACES )
    {
        baStatus = STS_NOT_FOUND;
        for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
        {
            if( m_Interfaces[i].GetInterfaceId() == ulInterfaceId )
            {
                // Interface object found.
                pInterfaces = &m_Interfaces[i];
                ulNumInterfaces = 1;
                baStatus = STS_SUCCESS;
                break;
            }
        }
    }

    if( baStatus == STS_SUCCESS )
    {
        // Create a new application object and call its attach member function
        // which allows it to send and receive transparent ATM cells.
        ATM_TRANSPARENT_APPL *pTransparentAppl;

        pTransparentAppl = new ATM_TRANSPARENT_APPL( &m_OsServices,
            &m_ApplTable, m_ulAtmApplSem, m_pSoftSar );
        if( pTransparentAppl )
        {
            baStatus = pTransparentAppl->Attach( pInterfaces, ulNumInterfaces,
                pAttachParms );

            if( baStatus != STS_SUCCESS )
                delete pTransparentAppl;
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }

    return( baStatus );
} // AttachTransparent


//**************************************************************************
// Function Name: ConfigureInterrupts
// Description  : Enables ATM Processor interrupts.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
void ATM_PROCESSOR::ConfigureInterrupts( void )
{
    // Enable types of ATM Processor interrupts.
    AP_REGS->ulIrqMask = ~INTR_MASK;
} // ConfigureInterrupts


//**************************************************************************
// Function Name: ProcessTransmitIntrComplete
// Description  : This function processes a catestrophic error condition from
//                the ATM Processor.
// Returns      : None.
//**************************************************************************
inline void ATM_PROCESSOR::ProcessTransmitIntrComplete( UINT32 ulChStatus )
{
    // Process transmit interrupt complete.
    for(UINT32 i = 0; i < MAX_QUEUE_SHAPERS && ulChStatus; i++,ulChStatus >>= 1)
    {
        if( (ulChStatus & 0x01) != 0 )
        {
            ATM_TX_BDQ *pTxQ;
            if( (pTxQ = m_ApplTable.GetTxQByShaperNum( i )) != NULL )
                pTxQ->Reclaim();
        }
    }

    ulChStatus = 0;
} // ProcessTransmitIntrComplete


//**************************************************************************
// Function Name: ProcessReceiveCellQueue
// Description  : This function processes cells received on the receive cell
//                queue.
// Returns      : None.
//**************************************************************************
inline void ATM_PROCESSOR::ProcessReceiveCellQueue( void )
{
    PATM_VCC_DATA_PARMS pDataParms;
    PATM_BUFFER pAtmBuf;
    ATM_DMA_BD *pBd;
    ATM_VCC_APPL *pAppl;

    while( m_RxCellQ.Remove( &pBd ) == STS_SUCCESS )
    {
    #if 0  //y42304 delete: 删除点灯  	
        if( m_OsServices.pfnBlinkLed )
            (*m_OsServices.pfnBlinkLed) ();
    #endif             

        pAtmBuf = (PATM_BUFFER) NONCACHE_TO_CACHE(pBd + 1);
        pDataParms = (PATM_VCC_DATA_PARMS) (pAtmBuf + 1);
        pDataParms->baReceiveStatus = STS_SUCCESS;

        // Copy fields from the ATM_DMA_BD buffer descriptor to
        // ATM_VCC_DATA_PARMS structure.
        pDataParms->ucCircuitType =
            (UINT8) (pBd->ulCt_BufPtr>>BD_CT_SHIFT);

        pDataParms->ucUuData8 = pBd->ucUui8;

        if( (pBd->ulFlags_NextRxBd & BD_FLAG_CLP) != 0 )
            pDataParms->ucFlags = ATMDATA_CLP;

        if( (pBd->ulFlags_NextRxBd & BD_FLAG_CI) != 0 )
            pDataParms->ucFlags |= ATMDATA_CI;


        // Use reserved fields to temporarily store BD status codes.
        // They will be processed by the ATM_VCC_APPL object.
        pDataParms->ucReserved[0] = pBd->ucRxAtmErrors;
        pDataParms->ucReserved[1] = pBd->ucRxAalErrors_RblHigh &
            BD_RX_AAL_ERROR_MASK;

        // pAtmBuf->pDataBuf is set in initialization and free cell functions
        pAtmBuf->ulDataLen = pBd->usLength;

        if( pDataParms->ucReserved[0] != RXATM_INVALID_VPI_VCI )
        {
            // The received data has a VPI/VCI that is in the ATM Processor
            // CAM.  Call the application object to processor it.
            pAppl = m_ApplTable.GetByRxVcid(
                pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK );

            if( pAppl != NULL )
                pAppl->Receive( pDataParms );
            else
                (*pDataParms->pFnFreeDataParms) (pDataParms);
        }
        else
        {
#if defined(CONFIG_ATM_RX_SOFTSAR)
            pAppl = m_SoftSar.FindRxAppl( (*(UINT32 *)
                pAtmBuf->pDataBuf) & 0x0ffffff0);
            if( pAppl != NULL )
            {
                if( (pDataParms = m_SoftSar.ProcessRxCell(pDataParms)) != NULL )
                    pAppl->Receive( pDataParms );
            }
            else
#endif
            {
                UINT8 ucPhyPort = (pBd->ucRxPortId_Vcid & BD_RX_PORT_ID_MASK) >>
                    BD_RX_PORT_ID_SHIFT;

                pDataParms->ucCircuitType = CT_TRANSPARENT;
                pDataParms->baReceiveStatus = STS_PKTERR_INVALID_VPI_VCI;

                if( ucPhyPort < MAX_INTERFACES )
                    m_Interfaces[ucPhyPort].Receive( NULL, pDataParms );
                else
                    (*pDataParms->pFnFreeDataParms) (pDataParms);
            }
        }
    }
} // ProcessReceiveCellQueue


//**************************************************************************
// Function Name: ProcessReceivePktQueue
// Description  : This function processes packets received on the receive
//                packet queue.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::ProcessReceivePktQueue( ATM_RX_BDQ *pRxPktQ )
{
    PATM_VCC_DATA_PARMS pDataParms;
    PATM_BUFFER pAtmBuf;
    UINT8 ucVcid;
    ATM_DMA_BD *pBd;
    ATM_VCC_APPL *pAppl;

    while( pRxPktQ->Remove( &pBd ) == STS_SUCCESS )
    {
        /* start of wan data disable by y42304 20060522: wan led按TR068规范处理 */
        #if 0
        if( m_OsServices.pfnBlinkLed )
            (*m_OsServices.pfnBlinkLed) ();
        #endif
        /* end of wan data disable by y42304 20060522*/

        pAtmBuf = (PATM_BUFFER) NONCACHE_TO_CACHE(pBd + 1);
        pDataParms = (PATM_VCC_DATA_PARMS) (pAtmBuf + 1);
        pDataParms->baReceiveStatus = STS_SUCCESS;
        pDataParms->ucReserved[0] = pDataParms->ucReserved[1] = 0;

        // pAtmBuf->pDataBuf is set in initialization and free pkt functions
        pAtmBuf->pNextAtmBuf = NULL;

        ucVcid = pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK;

        ATM_DMA_BD *pBdNext =
            BD_GET_NCADDR((ATM_DMA_BD *), pBd->ulFlags_NextRxBd);

        // Save the last buffer descriptor for assigning pDataParms
        // fields.
        ATM_DMA_BD *pBdLast = NULL;
        if( (pBd->ulFlags_NextRxBd & BD_FLAG_EOP) != 0 )
        {
            if( pBd->ucRxAtmErrors == 0 &&
                (pBd->ucRxAalErrors_RblHigh & BD_RX_AAL_ERROR_MASK) == 0 )
            {
                // The length on the last buffer points the end of the AAL5
                // trailer.  Use the length field in the AAL5 trailer.
                pAtmBuf->ulDataLen = (UINT32)
                    *(UINT16 *) &pAtmBuf->pDataBuf[pBd->usLength - 6];
            }
            else
            {
                // This cell has an error.  The BD length is correct.
                /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
                #ifdef CONFIG_BCM_VDSL
                if (g_nPtmTraffic)
                {
                    pAtmBuf->ulDataLen = (UINT32)
                    *(UINT16 *) &pAtmBuf->pDataBuf[pBd->usLength - 6];
                }
                else
                {
                    pAtmBuf->ulDataLen = pBd->usLength;
                }
                #else
                    pAtmBuf->ulDataLen = pBd->usLength;
                #endif
                /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

                ucVcid = pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK;
            }

            pBdLast = pBd;
            pBd = NULL;
        }
        else
        {
            // pBd = pBdNext;

            // Chained BDs are not currently supported.  Discard the packet.
            ATM_DMA_BD *pBd2;
            UINT32 ulEop;
            do
            {
                pBd2 = BD_GET_NCADDR((ATM_DMA_BD *), pBd->ulFlags_NextRxBd);
                ulEop = pBd->ulFlags_NextRxBd & BD_FLAG_EOP;

                pBd->ucFreeRbl = m_ucFreePktQBufSizeExp;
                m_FreePktQ.Add( pBd );

                pBd = pBd2;
            } while( ulEop == 0 );

            continue;
        }

        // A received packet may have more than one buffer descriptor
        // chained together.
        while( pBd )
        {
            pAtmBuf->pNextAtmBuf = (PATM_BUFFER) NONCACHE_TO_CACHE(pBd + 1);
            pAtmBuf = pAtmBuf->pNextAtmBuf;

            // pAtmBuf->pDataBuf is set in initialization and free pkt functions
            pAtmBuf->ulDataLen = pBd->usLength;
            pAtmBuf->pNextAtmBuf = NULL;

            if( (pBd->ulFlags_NextRxBd & BD_FLAG_CLP) != 0 )
                pDataParms->ucFlags |= ATMDATA_CLP;

            if( (pBd->ulFlags_NextRxBd & BD_FLAG_CI) != 0 )
                pDataParms->ucFlags |= ATMDATA_CI;

            // Use reserved fields to temporarily store BD status
            // codes. They will be processed by the ATM_VCC_APPL object.
            pDataParms->ucReserved[0] |= pBd->ucRxAtmErrors;
            pDataParms->ucReserved[1] |= pBd->ucRxAalErrors_RblHigh &
                BD_RX_AAL_ERROR_MASK;

            pBdNext = BD_GET_NCADDR((ATM_DMA_BD *), pBd->ulFlags_NextRxBd);

            // For AAL5 only, it is possible that some of the AAL5
            // trailer was counted in the current BD (pBd) length.
            // The extra value that should be subtracted is contained
            // in the next BD (pBdNext).
            if(pBdNext && (pBdNext->ulFlags_NextRxBd & BD_FLAG_NEG)!=0)
            {
                if( pAtmBuf )
                    pAtmBuf->ulDataLen -= pBdNext->usLength;

                pBd->ucFreeRbl = m_ucFreePktQBufSizeExp;
                m_FreePktQ.Add( pBd );

                pBd = pBdNext;
                pBdNext =
                    BD_GET_NCADDR((ATM_DMA_BD *),pBd->ulFlags_NextRxBd);
            }

            // Save the last BD for assigning pDataParms fields.
            if( (pBd->ulFlags_NextRxBd & BD_FLAG_EOP) != 0 )
            {
                pBdLast = pBd;
                pBd = NULL;
            }
            else
            {
                pBd = pBdNext;
            }

        }

        // Update the pDataParms fields with info from the last BD.
        pDataParms->ucCircuitType = (UINT8)
            (pBdLast->ulCt_BufPtr >> BD_CT_SHIFT); 

        pDataParms->ucUuData8 = pBdLast->ucUui8;

        if( (pBdLast->ulFlags_NextRxBd & BD_FLAG_CLP) != 0 )
            pDataParms->ucFlags |= ATMDATA_CLP;

        if( (pBdLast->ulFlags_NextRxBd & BD_FLAG_CI) != 0 )
            pDataParms->ucFlags |= ATMDATA_CI;

        // Use reserved fields to temporarily store BD status codes.
        // They will be processed by the ATM_VCC_APPL object.
        pDataParms->ucReserved[0] |= pBdLast->ucRxAtmErrors;
        pDataParms->ucReserved[1] |= pBdLast->ucRxAalErrors_RblHigh &
            BD_RX_AAL_ERROR_MASK;

        if( pDataParms->ucReserved[0] == 0 )
        {
            // Find the application object to process the received packet.
            pAppl = m_ApplTable.GetByRxVcid( ucVcid );
            if( pAppl != NULL )
                pAppl->Receive( pDataParms );
            else
                (*pDataParms->pFnFreeDataParms) (pDataParms);
        }
        else
        {
            UINT8 ucPhyPort = (ucVcid & BD_RX_PORT_ID_MASK) ? 1 : 0;

            pDataParms->ucCircuitType = CT_TRANSPARENT;
            pDataParms->baReceiveStatus = STS_PKTERR_INVALID_VPI_VCI;

            if( ucPhyPort < MAX_INTERFACES )
                m_Interfaces[ucPhyPort].Receive( NULL, pDataParms );
            else
                (*pDataParms->pFnFreeDataParms) (pDataParms);
        }
    }

#if defined(CONFIG_BCM96338)
    // On the BCM6338, the "Receive Packet Queue Almost Full" interrupt does
    // not interrupt at the last queue index.  Therefore, start a timer to
    // check when the packet at that index has been received.
    if( AP_REGS->ulFreePktQTail == AP_REGS->ulFreePktQLen )
    {
        (*m_OsServices.pfnStartTimer) ((void *) ATM_PROCESSOR::TimerCb,
            (UINT32) this, PKT_CHK_TIMEOUT_MS);
    }
#endif
} // ProcessReceivePktQueue


#if defined(CONFIG_BCM96338)
//**************************************************************************
// Function Name: TimerCb
// Description  : Called at periodic time intervals to check for a packet
//                received on the last receive packet queue index.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::TimerCb( UINT32 ulParm )
{
    ATM_PROCESSOR *pProc = (ATM_PROCESSOR *) ulParm;
    pProc->ProcessReceivePktQueue( &pProc->m_RxPktQ );
} // TimerCb
#endif


//**************************************************************************
// Function Name: IsHighPriorityPkt
// Description  : Determines if the packet is a high priority packet.
// Returns      : 1 - is high priority packet, 0 - is not high priority packet
//**************************************************************************
UINT32 ATM_PROCESSOR::IsHighPriorityPkt( ATM_DMA_BD *pBd )
{
    int ulRet = 0; // default to is not high priority packet
    UINT8 *pucPkt = BD_GET_NCADDR((UINT8 *), pBd->ulCt_BufPtr);
    UINT16 usValue;
    PATM_PRIORITY_PACKET_ENTRY pEntry;
    UINT32 *pulGroupNum =
        m_ulPriPktNumbers[pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK];

    // There can be from 1 to NUM_PRI_PKT_GROUPS groups of offset/value entries
    // to compare.  If all entries in any single group compare, then the packet
    // is a high priority packet.
    while( *pulGroupNum && ulRet == 0 )
    {
        pEntry = m_ulPriPktGroups[*pulGroupNum - 1];

        // Only process group if there is at least one offset/value entry.
        if( pEntry->ulStructureId == ID_ATM_PRIORITY_PACKET_ENTRY )
        {
            // Set the default status to high priority.
            ulRet = 1;

            do
            {
                usValue = *(UINT16 *) &pucPkt[pEntry->ulPacketOffset];
                if( (usValue & pEntry->usValueMask) != pEntry->usPacketValue )
                {
                    // The value did not compare.  Reset the status to not high
                    // priority and check the next group.
                    ulRet = 0;
                    break;
                }
                pEntry++;
            } while( pEntry->ulStructureId == ID_ATM_PRIORITY_PACKET_ENTRY );
        }
        pulGroupNum++;
    }

    return( ulRet );
} // IsHighPriorityPkt


//**************************************************************************
// Function Name: PrioritizeReceivePkts
// Description  : This function takes a packet off the ATM Processor receive
//                packet queue and puts it on an internal high priority receive
//                packet queue, internal low priority receive packet queue or
//                frees the packet by putting it on the ATM Processor free
//                packet queue.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::PrioritizeReceivePkts( void )
{
    UINT32 ulAtmAvailableFreePkts;
    ATM_DMA_BD *pBd;
    UINT32 ulIrqSts = AP_REGS->ulIrqStatus &
        (INTR_RPQ_ALMOST_FULL | INTR_RPQ_WD_TIMER | INTR_RPQ_IMMED_RSP);

    m_ulFreeQueueEmpty = 0;
    while( m_RxPktQ.Remove( &pBd ) == STS_SUCCESS )
    {
        ulAtmAvailableFreePkts = m_FreePktQ.GetNumFilledElements();

        if( ulAtmAvailableFreePkts == 0 )
            m_ulFreeQueueEmpty++;

        if( IsHighPriorityPkt( pBd ) )
        {
            m_HighPriRxPktQ.Add( pBd ); /* put pkt on a high priority queue */
            m_ulPriPktsReceived++;
        }
        else
        {
            if( ulAtmAvailableFreePkts > m_ulDropPktThreshold )
                m_LowPriRxPktQ.Add( pBd ); /* put pkt on a low priority queue */
            else
            {
                /* drop the packet */
                pBd->ucFreeRbl = m_ucFreePktQBufSizeExp;
                m_FreePktQ.Add( pBd );
            }
        }
    }

    // Increase the drop packet threshold if the Free Packet Queue was empty.
    // Decrease the drop packet threshold if the current number of buffers on the
    // Free Packet Queue exceeds the threshold value by DROP_PACKET_INCREMENT.
    if( m_ulFreeQueueEmpty > 0 &&
        m_ulDropPktThreshold < ((m_FreePktQ.GetSize() * 2) / 3) )
    {
        m_ulDropPktThreshold += DROP_PACKET_INCREMENT;
    }
    else
        if((m_FreePktQ.GetNumFilledElements() > m_ulDropPktThreshold +
           DROP_PACKET_INCREMENT) &&
           (m_ulDropPktThreshold - DROP_PACKET_INCREMENT >= DROP_PACKET_MINIMUM))
        {
            m_ulDropPktThreshold -= DROP_PACKET_INCREMENT;
        }

    // Clear interrupt bits.
    AP_REGS->ulIrqStatus = ulIrqSts;
} // PrioritizeReceivePkts


//**************************************************************************
// Function Name: ProcessEvent
// Description  : This function is called from a separate task/thread
//                context.  It handles transmit interrupt complete and
//                receive data available ATM Processor events.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::ProcessEvent( void )
{
    UINT32 ulIrqSts;
    UINT32 ulIntrMask = INTR_MASK;

    if( m_ulPrioritizeReceivePkts == 1 )
    {
        // Mask out packet interrupt bits because a packet might be received
        // during the loop below but it will not be processed until the next
        // time the hardware ISR executes.
        ulIntrMask &=
            ~(INTR_RPQ_ALMOST_FULL|INTR_RPQ_WD_TIMER|INTR_RPQ_IMMED_RSP);
    }

    ulIrqSts = (AP_REGS->ulIrqStatus & ulIntrMask);

    do
    {
#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
        // Process transmitted packets that have completed.
        UINT32 ulChStatus = AP_REGS->ulTxSdramChStatus;
        if( ulChStatus != 0 )
            ProcessTransmitIntrComplete( ulChStatus );
#endif

        // Process data received in the cell queue.
        ProcessReceiveCellQueue();

        // Process data received in the packet queue.
        if( m_ulPrioritizeReceivePkts == 1 )
        {
            ProcessReceivePktQueue( &m_HighPriRxPktQ );
            ProcessReceivePktQueue( &m_LowPriRxPktQ );
        }
        else
            ProcessReceivePktQueue( &m_RxPktQ );

        // Clear interrupt bits, then get interrupt status again.
        AP_REGS->ulIrqStatus = ulIrqSts;

        // Need to delay a small amount of time before re-reading the status
        // register to prevent a problem where the status never gets reset.
        for( int i = 0; i < 10; i++ )
            ;

        ulIrqSts = (AP_REGS->ulIrqStatus & ulIntrMask);
    } while( ulIrqSts != 0 );
} // ProcessEvent


//**************************************************************************
// Function Name: CreateBdQ
// Description  : Allocates memory for BDs and buffers for the free packet
//                queue or free cell queue and adds them to the queue object.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
#if defined(CONFIG_BCM96358)
void ATM_PROCESSOR::CalculateReceiveBufferSize(UINT16 usConfiguredSize,
    UINT16 *pusActualSize, UINT8 *pucActualExp)
{
    const UINT16 usIncrement = 64;
    UINT16  usFreePktQBufSize;
    UINT8 ucFreePktQBufSizeExp;

    ucFreePktQBufSizeExp = usConfiguredSize / usIncrement;
    if( usConfiguredSize % usIncrement != 0 )
        ucFreePktQBufSizeExp++;

#if defined(__KERNEL__)
    ucFreePktQBufSizeExp += 4; // reserve for struct skb_shared_info
#endif

    usFreePktQBufSize = ucFreePktQBufSizeExp * usIncrement;

    *pusActualSize = usFreePktQBufSize;
    *pucActualExp = ucFreePktQBufSizeExp;
} // CalculateReceiveBufferSize
#else
void ATM_PROCESSOR::CalculateReceiveBufferSize(UINT16 usConfiguredSize,
    UINT16 *pusActualSize, UINT8 *pucActualExp)
{
    UINT16 usSize = usConfiguredSize;
    UINT16  usFreePktQBufSize;
    UINT8 ucFreePktQBufSizeExp;

    usFreePktQBufSize = 0x8000;
    for( ucFreePktQBufSizeExp = 15; ucFreePktQBufSizeExp > 0;
         ucFreePktQBufSizeExp-- )
    {
        if( (usSize & 0x8000) == 0x8000 )
            break;

        usSize <<= 1;
        usFreePktQBufSize >>= 1;
    }

    // If the free packet queue buffer size is not a power of 2, increase
    // the size to the next power of 2.
    if(usConfiguredSize < 0x8000 && usConfiguredSize > usFreePktQBufSize)
    {
        ucFreePktQBufSizeExp++;
        usFreePktQBufSize <<= 1;
    }

    *pusActualSize = usFreePktQBufSize;
    *pucActualExp = ucFreePktQBufSizeExp;
} // CalculateReceiveBufferSize
#endif


//**************************************************************************
// Function Name: CreateBdQ
// Description  : Allocates memory for BDs and buffers for the free packet
//                queue or free cell queue and adds them to the queue object.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_PROCESSOR::CreateBdQ( char **ppQMem, char **ppBufMem,
    ATM_FREE_BDQ *pBdQ, UINT32 ulQSize, UINT16 usQBufSize, UINT8 ucQBufSizeExp,
    UINT16 usQBufOffset, FN_FREE_DATA_PARMS pfnFreeDp )
{
    const UINT32 ulMaxQSize = 512;
    const UINT32 ulAllocBlockSize = 128 * 1024;
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulElementSize = (sizeof(ATM_DMA_BD) + sizeof(ATM_BUFFER) +
        sizeof(ATM_VCC_DATA_PARMS) + 0x0f) & ~0x0f;
    UINT32 ulQBufAllocSize = (usQBufSize + usQBufOffset + 0x0f) & ~0x0f;
    char *pcBufAddrs[ulMaxQSize];
    char *p;
    UINT32 i;

    // The BD queues always have one open element in order to determine
    // empty/full condition.  Therefore, memory only needs to be allocated
    // for the queue size minus one.
    ulQSize--;

    if( ulQSize < ulMaxQSize )
    {
        *ppQMem = (char *) (*m_OsServices.pfnAlloc)
            ((ulElementSize * ulQSize) + 0x10);

        if( ppQMem )
        {
            UINT32 ulBufsPerBlock = ulAllocBlockSize / ulQBufAllocSize;
            UINT32 ulSize = ulQSize;
            UINT32 ulBufMemIdx = 0;
            UINT32 ulBufAddrsIdx = 0;
            UINT32 ulCurr;
            UINT32 ulAlloc;

            memset( pcBufAddrs, 0x00, sizeof(pcBufAddrs) );
            while( ulSize )
            {
                ulCurr = (ulSize < ulBufsPerBlock) ? ulSize : ulBufsPerBlock;
                ulAlloc = (ulCurr == ulSize)
                    ? (ulCurr * ulQBufAllocSize) : ulAllocBlockSize;

                p = ppBufMem[ulBufMemIdx++] = (char *)
                    (*m_OsServices.pfnAlloc) (ulAlloc);

                if( p )
                {
                    memset(p, 0x00, ulAlloc);
                    m_OsServices.pfnInvalidateCache( p, ulAlloc );

                    for( i = 0; i < ulCurr; i++ )
                        pcBufAddrs[ulBufAddrsIdx++] = p + (i * ulQBufAllocSize);

                    ulSize -= ulCurr;
                }
                else
                {
                    baStatus = STS_ALLOC_ERROR;
                    break;
                }
            }
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    if( baStatus == STS_SUCCESS )
    {
        ATM_DMA_BD *pBd;
        PATM_BUFFER pAb;
        PATM_VCC_DATA_PARMS pDp;
        char *pBuf;

        p = (char *) (((UINT32) *ppQMem + 0x0f) & ~0x0f);
        memset( p, 0x00, ulQSize * ulElementSize );
        m_OsServices.pfnFlushCache( p, ulQSize * ulElementSize );

        for( i = 0; i < ulQSize; i++, p += ulElementSize )
        {
            pBd = (ATM_DMA_BD *) p;
            pAb = (PATM_BUFFER) (pBd + 1);
            pDp = (PATM_VCC_DATA_PARMS) (pAb + 1);

            pBd = (ATM_DMA_BD *) CACHE_TO_NONCACHE(pBd);
            pBuf = pcBufAddrs[i];
            BD_SET_ADDR(pBd->ulCt_BufPtr, pBuf + usQBufOffset);
            pBd->ucFreeRbl = ucQBufSizeExp;

            pDp->ulStructureId = ID_ATM_VCC_DATA_PARMS;
            pDp->pFnFreeDataParms = pfnFreeDp;
            pDp->ulParmFreeDataParms = (UINT32) this;
            pDp->pAtmBuffer = pAb;
            pDp->ucFlags = pDp->ucSendPriority = 0;
            pDp->ucReserved[0] = pDp->ucReserved[1] = 0;
            pDp->baReceiveStatus = STS_SUCCESS;
            pDp->pApplicationLink = NULL;

            pAb->pNextAtmBuf = NULL;
            pAb->pDataBuf = (UINT8 *) pBuf + usQBufOffset;
            pAb->usDataOffset = usQBufOffset;
            pAb->ulDataLen = 0;

            if( pBdQ->Add( pBd ) != STS_SUCCESS )
            {
                // This should not happen.
                baStatus = STS_RESOURCE_ERROR;
                break;
            }
        }
    }

    if( baStatus != STS_SUCCESS )
    {
        if( *ppQMem )
        {
            (*m_OsServices.pfnFree) (*ppQMem);
            *ppQMem = NULL;
        }

        for( i = 0; ppBufMem[i] != NULL; i++ )
        {
            (*m_OsServices.pfnFree) (ppBufMem[i]);
            ppBufMem[i] = NULL;
        }
    }

    return( baStatus );
} // CreateBdQ


//**************************************************************************
// Function Name: FreeCell
// Description  : Called by application after it has processed a received
//                data buffer that contained a cell.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::FreeCell( PATM_VCC_DATA_PARMS pDataParms )
{
    ATM_PROCESSOR *pThis = (ATM_PROCESSOR *) pDataParms->ulParmFreeDataParms;
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;

    while( pAb )
    {
        PATM_BUFFER pAb2 = pAb->pNextAtmBuf;
        if( pAb->pDataBuf )
        {
            // Try to put the buffer back onto the free queue. pBd should never
            // be NULL.
            ATM_DMA_BD *pBd = (ATM_DMA_BD *)((UINT32)pAb - sizeof(ATM_DMA_BD));
            if( pBd )
            {
                pBd = (ATM_DMA_BD *) CACHE_TO_NONCACHE(pBd);
                pAb->pDataBuf = BD_GET_CADDR((UINT8 *), pBd->ulCt_BufPtr);
                pThis->m_OsServices.pfnInvalidateCache( pAb->pDataBuf,
                    FREE_CELL_Q_BUF_SIZE );
                pBd->ucFreeRbl = FREE_CELL_Q_BUF_SIZE_EXP;
                pThis->m_FreeCellQ.Add( pBd );
            }
        }
        pAb = pAb2;
    }

    pDataParms->ucFlags &= ~RX_DATA_PENDING;
} // FreeCell


//**************************************************************************
// Function Name: FreePkt
// Description  : Called by application after it has processed a received
//                data buffer that contained a packet.
// Returns      : None.
//**************************************************************************
void ATM_PROCESSOR::FreePkt( PATM_VCC_DATA_PARMS pDataParms )
{
    ATM_PROCESSOR *pThis = (ATM_PROCESSOR *) pDataParms->ulParmFreeDataParms;
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;

    while( pAb )
    {
        PATM_BUFFER pAb2 = pAb->pNextAtmBuf;
        if( pAb->pDataBuf )
        {
            // Try to put the buffer back onto the free queue. pBd should never
            // be NULL.
            ATM_DMA_BD *pBd = (ATM_DMA_BD *)((UINT32)pAb - sizeof(ATM_DMA_BD));
            if( pBd )
            {
                pBd = (ATM_DMA_BD *) CACHE_TO_NONCACHE(pBd);
                pAb->pDataBuf = BD_GET_CADDR((UINT8 *), pBd->ulCt_BufPtr);
                pThis->m_OsServices.pfnInvalidateCache( pAb->pDataBuf,
                    pThis->m_usFreePktQBufSize );
                pBd->ucFreeRbl = pThis->m_ucFreePktQBufSizeExp;
                pThis->m_FreePktQ.Add( pBd );
            }
        }
        pAb = pAb2;
    }

    pDataParms->ucFlags &= ~RX_DATA_PENDING;
} // FreePkt

