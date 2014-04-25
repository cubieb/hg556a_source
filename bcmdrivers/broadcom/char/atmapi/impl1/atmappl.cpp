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
// File Name  : AtmAppl.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM application
//              classes.  These classes implement an application access to a
//              VCC or VCCs.  There are two kinds of application access:
//              * access to a single VCC
//              * access to all VCCs but only for sending and receiving
//                ATM management data (OAM and RM cells)
//
// Updates    : 01/04/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"


//**************************************************************************
// ATM_APPL Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_APPL
// Description  : Constructor for the ATM application access base class.
// Returns      : None.
//**************************************************************************
ATM_APPL::ATM_APPL( ATM_OS_SERVICES *pOsServices, ATM_APPL_TABLE *pApplTable,
    UINT32 ulAtmApplSem, void *pSoftSar )
{
    m_pOsServices = pOsServices;
    m_pApplTable = pApplTable;
    m_ulAtmApplSem = ulAtmApplSem;
    m_pSoftSar = pSoftSar;
    memset( m_pTxQViaShaperNum, 0x00, sizeof(m_pTxQViaShaperNum) );
    memset( m_pTxQViaPriority, 0x00, sizeof(m_pTxQViaPriority) );
    m_pAnyTxQ = NULL;
} // ATM_APPL


//**************************************************************************
// Function Name: ~ATM_APPL
// Description  : Destructor for the ATM application access base class.
// Returns      : None.
//**************************************************************************
ATM_APPL::~ATM_APPL( void )
{
} // ~ATM_APPL


//**************************************************************************
// Function Name: AttachTxQueues
// Description  : Creates and initializes transmit queues.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL::AttachTxQueues( PATM_VCC_ATTACH_PARMS pAttachParms,
        UINT8 ucPortId, UINT8 ucVcid, PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry,
        UINT32 ulPhyPort1Enabled )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_VCC_TRANSMIT_QUEUE_PARMS pTxQParms;

    // For all transmit queues specified in the attach parameters...
    for( UINT8 i = 0; i < pAttachParms->ulTransmitQParmsSize; i++ )
    {
        pTxQParms = &pAttachParms->pTransmitQParms[i];

        ATM_TX_BDQ *pTxQ = new ATM_TX_BDQ( m_pOsServices, m_ulAtmApplSem,
            ulPhyPort1Enabled, m_pSoftSar );
        if( pTxQ )
        {
            // A new transmit queue was allocated.
            const UINT32 ulDefaultSize = 64;
            UINT32 ulSize = pTxQParms->ulSize;
            if( ulSize < 2 || ulSize > MAX_TX_QUEUE_SIZE )
                ulSize = ulDefaultSize;

            UINT32 ulMpEnable =
                (pAttachParms->ulTransmitQParmsSize > 1)
                    ? pAttachParms->ulTransmitQParmsSize : 0;

            // Create the transmit queue.
            baStatus = pTxQ->Create( ulSize, (UINT8) pTxQParms->ulPriority,
                ulMpEnable, ucPortId, ucVcid, pEntry );

            if( baStatus == STS_SUCCESS )
            {
                // Transmit queue was successfully created.  Set its object
                // address in some lookup tables.  Note that a "shaper number"
                // and a "transmit queue index" are the same value.
                UINT8 ucShaperNum = pTxQ->GetQIdx();
                m_pTxQViaShaperNum[ucShaperNum] = pTxQ;
                m_pTxQViaPriority[pTxQParms->ulPriority] = pTxQ;
                m_pApplTable->SetTxQByShaperNum( ucShaperNum, pTxQ );

                if( i == 0 )
                    m_pAnyTxQ = pTxQ;
            }
            else
            {
                delete pTxQ;
                break;
            }
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }

    return( baStatus );
} // AttachTxQueues


//**************************************************************************
// Function Name: DetachTxQueues
// Description  : Deletes transmit queues.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL::DetachTxQueues( void )
{
    ATM_TX_BDQ *pTxQ;

    // Delete all transmit queues and make the queue index available.
    for( UINT8 i = 0; i < MAX_QUEUE_SHAPERS; i++ )
    {
        if( m_pTxQViaShaperNum[i] )
        {
            pTxQ = m_pTxQViaShaperNum[i];
            m_pTxQViaShaperNum[i] = NULL;
            if( pTxQ->DecInUse() == 0 )
            {
                // Clear this object from ATM application object lookup table.
                m_pApplTable->Reset( this, i );

                delete pTxQ;
            }
            else
            {
                // Clear this object from ATM application object lookup table.
                m_pApplTable->Reset( this );
            }
        }
    }

    // Clear transmit priority lookup table.
    memset( m_pTxQViaPriority, 0x00, sizeof(m_pTxQViaPriority) );

    return( STS_SUCCESS );
} // DetachTxQueues


//**************************************************************************
// ATM_VCC_APPL Class
//**************************************************************************

//**************************************************************************
// Function Name: ATM_VCC_APPL
// Description  : Constructor for the ATM VCC application access class.
// Returns      : None.
//**************************************************************************
ATM_VCC_APPL::ATM_VCC_APPL( ATM_OS_SERVICES *pOsServices,
    ATM_APPL_TABLE *pApplTable, UINT32 ulAtmApplSem, void *pSoftSar )
    : ATM_APPL( pOsServices, pApplTable, ulAtmApplSem, pSoftSar )
{
    m_pVccInfo = NULL;
    m_ulFlags = 0;
    m_pFnReceiveDataCb = NULL;
    m_ulParmReceiveData = 0;

    m_pTrafficDescrTable = NULL;
    m_ulTrafficDescrTableSize = 0;

    m_pInterface = NULL;
    m_pIntfStats = NULL;
    m_ulState = ST_APPL_CONSTRUCTED;
    m_ucVcid = 0xff;
    m_ulMpIdx = 0xff;
} // ATM_VCC_APPL


//**************************************************************************
// Function Name: ~ATM_VCC_APPL
// Description  : Destructor for the ATM VCC application access class.
// Returns      : None.
//**************************************************************************
ATM_VCC_APPL::~ATM_VCC_APPL( void )
{
    Detach();
} // ~ATM_VCC_APPL


//**************************************************************************
// Function Name: Attach
// Description  : Attach an application for accessing a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_APPL::Attach( ATM_VCC_INFO *pInfo,
    PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_VCC_TRANSMIT_QUEUE_PARMS pSaveTxQParms = pAttachParms->pTransmitQParms;
    UINT32 ulSaveTxQSize = pAttachParms->ulTransmitQParmsSize;
    UINT32 ulTxQSize;

    // If transmit queues parameters are not passed, use the configured transmit
    // queue parameters.
    if( pAttachParms->pTransmitQParms == NULL )
    {
        pAttachParms->pTransmitQParms = pInfo->Cfg.TransmitQParms;
        pAttachParms->ulTransmitQParmsSize = pInfo->Cfg.ulTransmitQParmsSize;
    }

    ulTxQSize = pAttachParms->ulTransmitQParmsSize;
    pAttachParms->ulHandle = 0;

    // Verify that the caller is using the same structure format as this module.
    if( pAttachParms->ulStructureId != ID_ATM_VCC_ATTACH_PARMS ||
        ulTxQSize == 0 ||
        (pInfo->Cfg.ulAalType != AAL_5 && pInfo->Cfg.ulAalType!=AAL_0_PACKET &&
         ulTxQSize != 1) )
    {
        baStatus = STS_PARAMETER_ERROR;
    }
    else
        // Verify that an Attach operation is allowed.
        if( m_ulState != ST_APPL_CONSTRUCTED )
            baStatus = STS_STATE_ERROR;
        else
            // Verify that the interface is up.
            if( !pInfo->pInterface->IsInterfaceUp() )
                baStatus = STS_INTERFACE_DOWN;
            else
                // Verify that the VCC is up.
                if( pInfo->Cfg.ulAtmVclAdminStatus == ADMSTS_DOWN )
                    baStatus = STS_VCC_DOWN;

    if( baStatus == STS_SUCCESS )
    {
        m_pVccInfo = pInfo;
        m_pInterface = pInfo->pInterface;
        m_pIntfStats = m_pInterface->GetStats();
        m_ulFlags = pAttachParms->ulFlags;
        m_pFnReceiveDataCb = pAttachParms->pFnReceiveDataCb;
        m_ulParmReceiveData = pAttachParms->ulParmReceiveData;

        baStatus = AssignVcid();

        // If more than one transmit queue is used, a multi priority index must
        // be reserved.
        if( baStatus == STS_SUCCESS && ulTxQSize > 1 )
            baStatus = ReserveMultiPriorityIndex();

        // Verify that the Traffic Descriptor Table index is valid.
        if( baStatus == STS_SUCCESS )
        {
            // Find traffic descriptor table entry.
            UINT32 ulTdi = m_pVccInfo->Cfg.ulAtmVclTransmitTrafficDescrIndex;
            PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry = GetTdtEntry( ulTdi );
            if( pEntry )
            {
                // Set the receive CAM.
                SetRxVpiVciCam( pEntry->ulServiceCategory );

#if !defined(_CFE_BRIDGE_) && !defined(CONFIG_ATM_SOFTSAR)
                // If the VCC needs one unshaped queue, look for an existing
                // one to share.  Otherwise, create new queue(s).
                if( pEntry->ulTrafficDescrType==TDT_ATM_NO_TRAFFIC_DESCRIPTOR &&
                    pAttachParms->ulTransmitQParmsSize == 1 )
                {
                    PATM_VCC_TRANSMIT_QUEUE_PARMS pTxQParms;

                    pTxQParms = &pAttachParms->pTransmitQParms[0];
                    baStatus = ShareExistingTxQueue(pTxQParms->ulPriority);
                }
                else
                    baStatus = STS_NOT_FOUND;


                if( baStatus == STS_NOT_FOUND )
                {
                    baStatus = AttachTxQueues( pAttachParms,
                        m_pInterface->GetPhyPortNum(), m_ucVcid, pEntry,
                        m_pInterface->IsPhyPort1Enabled() );
                }
#else
                baStatus = AttachTxQueues( pAttachParms,
                    m_pInterface->GetPhyPortNum(), m_ucVcid, pEntry,
                    m_pInterface->IsPhyPort1Enabled() );
#endif
            }
            else
            {
                // Traffic Descriptor Table has not been set.
                baStatus = STS_STATE_ERROR;
            }
        }

        if( baStatus == STS_SUCCESS )
        {
            m_ulState = ST_APPL_ATTACHED;
            m_pApplTable->SetByVccAddr( &m_pVccInfo->Addr, this );
            m_pApplTable->SetByRxVcid( m_ucVcid, this );
            m_pApplTable->RegisterAppl( this );
            pAttachParms->ulHandle = (UINT32) this;

            if( m_pOsServices->pfnAdslAtmAddVc )
            {
                // This function call causes the ADSL PHY to count the number
                // of cells before an AAL5 EOP bit is set in the ATM header.
                // If the number of cells will exceed the MaxSDU value, the
                // ADSL PHY sets the AAL5 EOP bit in the ATM header of the cell.
                (*m_pOsServices->pfnAdslAtmAddVc) ((int) m_pVccInfo->Addr.usVpi,
                    (int) m_pVccInfo->Addr.usVci);
            }
        }
        else
        {
            // There was an error.  Clean up.
            m_ulState = ST_APPL_ERROR;
            Detach();
        }
    }

    pAttachParms->pTransmitQParms = pSaveTxQParms;
    pAttachParms->ulTransmitQParmsSize = ulSaveTxQSize;

    return( baStatus );
} // Attach

//**************************************************************************
// Function Name: Detach
// Description  : Detach an application from accessing a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_APPL::Detach( void )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( m_ulState == ST_APPL_ATTACHED || m_ulState == ST_APPL_ERROR )
    {
        m_ulState = ST_APPL_DETACH_PENDING;

        m_pApplTable->UnregisterAppl( this );

        DetachTxQueues();

        if( m_ucVcid != 0xff )
        {
#if !defined(CONFIG_ATM_SOFTSAR)
            // Unreserve Multi Priority VCID.
            if( m_ulMpIdx < AP_MAX_MULTI_PRIORITY_VCIDS )
            {
                (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
                AP_REGS->ulTxAtmMpAalCfg &=
                    ~((UINT32) ATMR_MP_MASK << (m_ulMpIdx * ATMR_MP_SHIFT));
                m_ulMpIdx = 0xff;
                (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);
            }

            // Reset the Tx VPI/VCI/Port CAM.
            AP_REGS->ulTxVpiVciTable[m_ucVcid] = 0;

#else
            ((ATM_SOFT_SAR *) m_pSoftSar)->UnassignVcid( m_ucVcid );
#endif

            // Reset Rx VPI/VCI/Port CAM.
            if( m_ucVcid < MAX_RX_VCIDS )
            {
                AP_REGS->ulRxVpiVciCam[m_ucVcid * 2] = 0;
                AP_REGS->ulRxVpiVciCam[(m_ucVcid * 2) + 1] = 0;
            }
#if defined(CONFIG_ATM_RX_SOFTSAR)
            else
                ((ATM_SOFT_SAR *) m_pSoftSar)->UnregisterRxAppl( this );
#endif

            // Flush a partially received packet on the VCC.
            if( (m_pVccInfo->Cfg.ulAalType == AAL_5 ||
                m_pVccInfo->Cfg.ulAalType == AAL_0_PACKET) &&
                m_ucVcid < MAX_RX_VCIDS )
            {
                AP_REGS->ulRxFlushBd |= ((UINT32) 1 << m_ucVcid);
            }

            // Reset flush bit.
            if( (m_pVccInfo->Cfg.ulAalType == AAL_5 ||
                m_pVccInfo->Cfg.ulAalType == AAL_0_PACKET) &&
                m_ucVcid < MAX_RX_VCIDS )
            {
                AP_REGS->ulRxFlushBd &= ~((UINT32) 1 << m_ucVcid);
            }

            m_pVccInfo->ucVcid = 0xff;
            m_ucVcid = 0xff;

            if( m_pOsServices->pfnAdslAtmDeleteVc )
            {
                (*m_pOsServices->pfnAdslAtmDeleteVc) ((int) m_pVccInfo->Addr.usVpi,
                    (int) m_pVccInfo->Addr.usVci);
            }
        }

        m_ulState = ST_APPL_CONSTRUCTED;
    }
    else
        baStatus = STS_STATE_ERROR;

    return( baStatus );
} // Detach


//**************************************************************************
// Function Name: Send
// Description  : Send data on a VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_APPL::Send( PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus;

    if( m_ulState == ST_APPL_ATTACHED && m_pInterface->IsLinkUp() )
    {
        // Get the transmit queue to send the data on.
        ATM_TX_BDQ *pTxQ = NULL;
        if( pDataParms->ucSendPriority == ANY_PRIORITY )
            pTxQ = m_pAnyTxQ;
        else
            if( pDataParms->ucSendPriority <= MAX_TX_PRIORITIES )
                pTxQ = m_pTxQViaPriority[pDataParms->ucSendPriority];

        if( pTxQ == NULL )
            pTxQ = m_pAnyTxQ;

        UINT8 ucCt = pDataParms->ucCircuitType;

        // Send OAM F5 cells transparently though the ATM Processor.
        if( ucCt == CT_OAM_F5_SEGMENT || ucCt == CT_OAM_F5_END_TO_END )
        {
            const UINT8 ucOamF5Segment  = (0x04 << 1);
            const UINT8 ucOamF5EndToEnd = (0x05 << 1);
            PATM_BUFFER pAb = pDataParms->pAtmBuffer;

            pAb->ulReserved = (ucCt == CT_OAM_F5_SEGMENT)
                ? GFC_OAM_F5_SEG_IDX : GFC_OAM_F5_ETOE_IDX;
            
            /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
            if( NULL != m_pOsServices->pfnAdslSetVcEntryEx)
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
            {
	            (*m_pOsServices->pfnAdslSetVcEntryEx) ( (int) pAb->ulReserved,
	                (int) m_pInterface->GetPhyPortNum(),
	                (int) m_pVccInfo->Addr.usVpi,
	                (int) m_pVccInfo->Addr.usVci,
	                (int) (ucCt == CT_OAM_F5_SEGMENT)
	                    ? ucOamF5Segment : ucOamF5EndToEnd );
            }
            /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
            
            m_pIntfStats->Aal0IntfStats.ulIfOutUcastPkts++;
        }

        // Send the data.
        baStatus = pTxQ->Add( pDataParms, m_ucVcid );

        if( baStatus == STS_SUCCESS )
        {
            // Update statistics.
            if( ucCt == CT_AAL5 )
            {
                m_pIntfStats->Aal5IntfStats.ulIfOutUcastPkts++;
#if defined(CONFIG_BCM96338)
                PATM_BUFFER pAb;
                for( pAb = pDataParms->pAtmBuffer; pAb; pAb = pAb->pNextAtmBuf )
                    m_pIntfStats->Aal5IntfStats.ulIfOutOctets += pAb->ulDataLen;
#endif
            }
            else
                if( ucCt == CT_AAL0_PACKET || ucCt == CT_AAL0_CELL_CRC )
                    m_pIntfStats->Aal0IntfStats.ulIfOutUcastPkts++;
        }
    }
    else
    {
        baStatus = (m_ulState != ST_APPL_ATTACHED)
            ? STS_STATE_ERROR : STS_LINK_DOWN;
    }

    return( baStatus );
} // Send


//**************************************************************************
// Function Name: Receive
// Description  : Process a buffer received on this VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_APPL::Receive( PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
    extern int g_nPtmTraffic;
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

    if( m_ulState == ST_APPL_ATTACHED )
    {
        // Check for a management cell.  Not that ucReserved[0] contains the ATM
        // Processor ATM status byte.
        UINT8 ucCt = pDataParms->ucCircuitType;
        if( ucCt >= CT_OAM_F5_SEGMENT && ucCt <= CT_RM &&
            pDataParms->ucReserved[0] == 0 )
        {
            // If it is an OAM cell, call a function to examine the cell bits
            // and send a OAM response cell if necessary.
            if( ucCt == CT_OAM_F5_SEGMENT || ucCt == CT_OAM_F5_END_TO_END )
                ProcessOamCell( pDataParms );

            m_pIntfStats->Aal0IntfStats.ulIfInUcastPkts++;
            m_pIntfStats->Aal0IntfStats.ulIfInOctets +=
                pDataParms->pAtmBuffer->ulDataLen;

            if( (ucCt == CT_OAM_F5_SEGMENT &&
                 (m_ulFlags & AVAP_ALLOW_OAM_F5_SEGMENT_CELLS) == 0) ||
                (ucCt == CT_OAM_F5_END_TO_END &&
                 (m_ulFlags & AVAP_ALLOW_OAM_F5_END_TO_END_CELLS) == 0) ||
                (ucCt == CT_RM &&
                 (m_ulFlags & AVAP_ALLOW_RM_CELLS) == 0) )
            {
                // The data contains a management cell that this object does
                // not support based on the flag bits passed in Attach.
                // Pass the data to the interface object.
                baStatus = m_pInterface->Receive(&m_pVccInfo->Addr, pDataParms);
            }
            else
            {
                // Pass the received data to the application.
                pDataParms->ucFlags |= RX_DATA_PENDING;
                (*m_pFnReceiveDataCb) ((UINT32) this, &m_pVccInfo->Addr,
                    pDataParms, m_ulParmReceiveData);
            }
        }
        else
        {
            // If there was an error, update statistics.  The ATM Processor
            // buffer descriptor ATM status byte is stored in ucReserved[0]
            // and the AALx status byte is stored in ucReserved[1].
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
            if( (pDataParms->ucReserved[0] + pDataParms->ucReserved[1] != 0 ) &&
                !g_nPtmTraffic )
#else
            if( pDataParms->ucReserved[0] + pDataParms->ucReserved[1] != 0 )
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
            {
                m_pInterface->LockStats();

                // Process AAL errors.
                if( pDataParms->ucReserved[1] != 0 )
                {
                    if( ucCt == CT_AAL5 )
                    {
                        m_pIntfStats->Aal5IntfStats.ulIfInErrors++;
                        UpdateAal5VccStats( pDataParms );
                    }
                    else
                        if( ucCt == CT_AAL0_PACKET )
                        {
                            m_pIntfStats->Aal0IntfStats.ulIfInErrors++;
                            UpdateAal0PktVccStats( pDataParms );
                        }
                        else
                            if( ucCt == CT_AAL0_CELL_CRC )
                            {
                                m_pIntfStats->Aal0IntfStats.ulIfInErrors++;
                                UpdateAal0CellVccStats( pDataParms );
                            }
                }

                // Process ATM errors.
                if( pDataParms->ucReserved[0] != 0 )
                    UpdateAtmStats( pDataParms, &m_pIntfStats->AtmIntfStats );

                m_pInterface->UnlockStats();

                // If this object does not accept error cells, pass it to the
                // interface object.
                if( (m_ulFlags & AVAP_ALLOW_CELLS_WITH_ERRORS) == 0 &&
                   m_pVccInfo->Cfg.u.Aal5Cfg.ulAtmVccCpcsAcceptCorruptedPdus==0)
                {
                    baStatus = m_pInterface->Receive( &m_pVccInfo->Addr,
                        pDataParms );
                }
                else
                {
                    // Pass the received data to the application.
                    pDataParms->ucFlags |= RX_DATA_PENDING;
                    (*m_pFnReceiveDataCb) ((UINT32) this, &m_pVccInfo->Addr,
                        pDataParms, m_ulParmReceiveData);
                }
            }
            else
            {
                // No error. Update statistics.
                if( ucCt == CT_AAL5 )
                {
                    m_pIntfStats->Aal5IntfStats.ulIfInUcastPkts++;
                    // AAL5 ulIfInOctets are counted in the ATM Processor
                    // for BCM6345 and BCM6348.
#if defined(CONFIG_BCM96338)
                    PATM_BUFFER pAb;
                    for(pAb = pDataParms->pAtmBuffer; pAb; pAb=pAb->pNextAtmBuf)
                        m_pIntfStats->Aal5IntfStats.ulIfInOctets+=pAb->ulDataLen;
#endif
                }
                else
                {
                    if( ucCt == CT_AAL0_PACKET || ucCt == CT_AAL0_CELL_CRC )
                    {
                        m_pIntfStats->Aal0IntfStats.ulIfInUcastPkts++;
                        // AAL0 ulIfInOctets are counted in ATM Processor.
                    }
                }

                // Pass the received data to the application.
                pDataParms->ucFlags |= RX_DATA_PENDING;
                (*m_pFnReceiveDataCb) ((UINT32) this, &m_pVccInfo->Addr,
                    pDataParms, m_ulParmReceiveData);
            }
        }
    }
    else
    {
        baStatus = STS_STATE_ERROR;
        (*pDataParms->pFnFreeDataParms) (pDataParms);
    }

    return( baStatus );
} // Receive


//**************************************************************************
// Function Name: SetTrafficDescrTable
// Description  : Updates shaping values based on a new traffic descriptor
//                table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
void ATM_VCC_APPL::SetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescrTable, UINT32  ulTrafficDescrTableSize )
{
    m_pTrafficDescrTable = pTrafficDescrTable;
    m_ulTrafficDescrTableSize = ulTrafficDescrTableSize;

    if( m_ulState == ST_APPL_ATTACHED )
    {
        // Get the traffic descriptor table entry for this VCC.
        UINT32 ulTdi = m_pVccInfo->Cfg.ulAtmVclTransmitTrafficDescrIndex;
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry = GetTdtEntry( ulTdi );

        if( pEntry )
        {
            SetRxVpiVciCam( pEntry->ulServiceCategory );

            // Update the shaping values for each transmit queue.
            ATM_TX_BDQ *pTxQ;
            for( UINT32 i = 1; i <= MAX_TX_PRIORITIES; i++ )
                if( m_pTxQViaPriority[i] )
                {
                    pTxQ = m_pTxQViaPriority[i];
                    pTxQ->ReconfigureShaper( pEntry );
                }
        }
    }
} // SetTrafficDescrTable


//**************************************************************************
// Function Name: AssignVcid
// Description  : Assigns a Vcid which is a value between 0 and MAX_VCIDS which
//                maps to VPI/VCI/Port.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
#if !defined(CONFIG_ATM_SOFTSAR)
BCMATM_STATUS ATM_VCC_APPL::AssignVcid( void )
{
    BCMATM_STATUS baStatus = STS_RESOURCE_ERROR;

    // Find a unique VCID.
    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);

    // Initialize to invalid value.
    m_ucVcid = 0xff;

    for( UINT8 i = 0; i < MAX_VCIDS; i++ )
    {
        if( (AP_REGS->ulTxVpiVciTable[i] & TXTBL_SW_DEFINED) == 0 )
        {
            PATM_VCC_ADDR pAddr = &m_pVccInfo->Addr;

            // A unique VCID is found.
            m_ucVcid = i;

            // Create the value to write to the Tx VPI/VCI table.
            UINT32 ulValue = TXTBL_SW_DEFINED | ((UINT32) pAddr->usVci) |
                (((UINT32) pAddr->usVpi << TXTBL_VPI_SHIFT) &
                 TXTBL_VPI_MASK);

            if( m_pVccInfo->Cfg.ulAalType == AAL_0_CELL_CRC &&
                (m_ulFlags & AVAP_ADD_AAL0_CRC10) == AVAP_ADD_AAL0_CRC10 )
            {
                ulValue |= TXTBL_UD_CRC10_EN;
            }

            // Write the value to the Tx VPI/VCI table.
            AP_REGS->ulTxVpiVciTable[m_ucVcid] = ulValue;

            baStatus = STS_SUCCESS;
            break;
        }
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( baStatus );
} // AssignVcid

#else
BCMATM_STATUS ATM_VCC_APPL::AssignVcid( void )
{
    BCMATM_STATUS baStatus = STS_RESOURCE_ERROR;
    
    m_ucVcid = ((ATM_SOFT_SAR *) m_pSoftSar)->AssignVcid( &m_pVccInfo->Addr );
    if( m_ucVcid != 0xff )
        baStatus = STS_SUCCESS;

    return( baStatus );
} // AssignVcid
#endif


//**************************************************************************
// Function Name: GetTdtIdx
// Description  : Returns a traffic descriptor table entry for the specified
//                index.
// Returns      : Traffic descriptor table entry or NULL.
//**************************************************************************
PATM_TRAFFIC_DESCR_PARM_ENTRY ATM_VCC_APPL::GetTdtEntry( UINT32 ulTdtIdx )
{
    PATM_TRAFFIC_DESCR_PARM_ENTRY pFound = NULL;
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry;
    UINT32 i;
    for( i = 0, pEntry = m_pTrafficDescrTable; i < m_ulTrafficDescrTableSize;
        i++, pEntry++ )
    {
        if( pEntry->ulTrafficDescrIndex == ulTdtIdx )
        {
            pFound = pEntry;
            break;
        }
    }

    return( pFound );
} // GetTdtEntry


//**************************************************************************
// Function Name: ReserveMultiPriorityIndex
// Description  : The ATM Processor allows up to two AAL5 VCC to use multiple
//                priority transmit queues.  This function tries to reserve one
//                of the two VCCs that can use mulitple queues.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
#if !defined(CONFIG_ATM_SOFTSAR)
BCMATM_STATUS ATM_VCC_APPL::ReserveMultiPriorityIndex( void )
{
    BCMATM_STATUS baStatus = STS_RESOURCE_ERROR;

    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);

    UINT32 ulMpEn = AP_REGS->ulTxAtmMpAalCfg;
    UINT32 i, ulShift;
    for(i = 0, ulShift = 0; i < AP_MAX_MULTI_PRIORITY_VCIDS;
        i++, ulShift += ATMR_MP_SHIFT)
    {
        if( (ulMpEn & (ATMR_MP_VC_EN << ulShift)) == 0 )
        {
            AP_REGS->ulTxAtmMpAalCfg &= ~((UINT32) ATMR_MP_MASK << ulShift);
            AP_REGS->ulTxAtmMpAalCfg |=
                ((m_ucVcid << ATMR_MP_VCID_SHIFT) | ATMR_MP_VC_EN) << ulShift;
            m_ulMpIdx = i;
            baStatus = STS_SUCCESS;
            break;
        }
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( baStatus );
} // ReserveMultiPriorityIndex

#else
BCMATM_STATUS ATM_VCC_APPL::ReserveMultiPriorityIndex( void )
{
    return( STS_SUCCESS );
} // ReserveMultiPriorityIndex
#endif


//**************************************************************************
// Function Name: SetRxVpiVciCam
// Description  : Writes an entry to the receive VPI/VCI/Port CAM.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::SetRxVpiVciCam( UINT32 ulServiceCategory )
{
    if( m_ucVcid < MAX_RX_VCIDS )
    {
        UINT32 ulValue1, ulValue2;
        PATM_VCC_ADDR pAddr = &m_pVccInfo->Addr;
        UINT8 ucPort = m_pInterface->GetPhyPortNum();
        UINT8 ucAalType = (UINT8) m_pVccInfo->Cfg.ulAalType;

        // Create value to write to the first part of the Rx VPI/VCI/Port CAM.
        ulValue1 = RXCAM_VALID | ((UINT32) ucPort) |
            (((UINT32) pAddr->usVci << RXCAM_VCI_SHIFT) & RXCAM_VCI_MASK) |
            (((UINT32) pAddr->usVpi << RXCAM_VPI_SHIFT) & RXCAM_VPI_MASK);

        // Create value to write to the second part of the Rx VPI/VCI/Port CAM.
        ulValue2 = RXCAM_RM_IMMED_RSP | RXCAM_OAM_IMMED_RSP |
            (((UINT32) m_ucVcid << RXCAM_VCID_SHIFT) & RXCAM_VCID_MASK);

        // Enable immediate response mode for high priority service categories
        // CBR and VBR.
        if( ulServiceCategory == SC_CBR || ulServiceCategory == SC_RT_VBR )
            ulValue2 |= RXCAM_NON_OAM_RM_IMMED_RSP;

        switch( ucAalType )
        {
        case AAL_5:
            ulValue2 |= RXCAM_CT_AAL5 | RXCAM_UD_RC_RPQ;
            break;

        case AAL_0_PACKET:
            ulValue2 |= RXCAM_CT_AAL0_PACKET | RXCAM_UD_RC_RPQ;
            break;

        case AAL_0_CELL_CRC:
            ulValue2 |= RXCAM_CT_AAL0_CELL;
            if( (m_ulFlags & AVAP_ADD_AAL0_CRC10) == AVAP_ADD_AAL0_CRC10 )
                ulValue2 |= RXCAM_UD_CRC10_EN;
            break;
        }

        // Write the values to the Rx VPI/VCI/Port CAM.
        AP_REGS->ulRxVpiVciCam[(m_ucVcid * 2) + 1] = ulValue2;
        AP_REGS->ulRxVpiVciCam[m_ucVcid * 2] = ulValue1;
    }
#if defined(CONFIG_ATM_RX_SOFTSAR)
    else
        ((ATM_SOFT_SAR *) m_pSoftSar)->RegisterRxAppl( &m_pVccInfo->Addr, this );
#endif
} // SetRxVpiVciCam

//**************************************************************************
// Function Name: ShareExistingTxQueue
// Description  : Looks for an existing transmit queue to share.
// Returns      : STS_SUCCESS
//**************************************************************************
BCMATM_STATUS ATM_VCC_APPL::ShareExistingTxQueue( UINT32 ulPriority )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;
    ATM_TX_BDQ *pTxQ = NULL;

    for( UINT32 i = 0; i < MAX_QUEUE_SHAPERS; i++)
    {
        pTxQ = m_pApplTable->GetTxQByShaperNum( i );
        if( pTxQ != NULL && pTxQ->IsMpEnabled() == 0 && pTxQ->IsShaped() == 0 )
        {
            /* Found an existing non-priority enabled, non-shaped transmit
             * queue to share.
             */
            UINT8 ucShaperNum = pTxQ->GetQIdx();

            m_pTxQViaShaperNum[ucShaperNum] = pTxQ;
            m_pTxQViaPriority[ulPriority] = pTxQ;
            m_pAnyTxQ = pTxQ;
            pTxQ->IncInUse();

            baStatus = STS_SUCCESS;
            break;
        }
    }

    return( baStatus );
} // ShareExistingTxQueue

//**************************************************************************
// Function Name: ProcessOamCell
// Description  : Examines an OAM cell and sends a response if the function
//                type is Alarm Indication Signal (AIS) or Loopback requeest.
// Returns      : None
//**************************************************************************
void ATM_VCC_APPL::ProcessOamCell( PATM_VCC_DATA_PARMS pDataParms )
{
    const UINT32 culOamTypeFuncTypeIndex = 0;
    const UINT32 culLoopbackIndicatorIndex = 1;
    const UINT8 cucFaultMgmtAis = 0x10;
    const UINT8 cucFaultMgmtRdi = 0x11;
    const UINT8 cucFaultMgmtLoopback = 0x18;

    UINT8 *pData = pDataParms->pAtmBuffer->pDataBuf;

    if( pData[culOamTypeFuncTypeIndex] == cucFaultMgmtAis ||
        (pData[culOamTypeFuncTypeIndex] == cucFaultMgmtLoopback &&
         pData[culLoopbackIndicatorIndex] == 1) )
    {
        struct OamRspStruct
        {
            ATM_VCC_DATA_PARMS DataParms;
            ATM_BUFFER AtmBuffer;
            UINT8 ucBuffer[48];
        } *pOamRsp = (OamRspStruct *)
            (*m_pOsServices->pfnAlloc) (sizeof(OamRspStruct));

        if( pOamRsp )
        {
            // Create an OAM response.
            PATM_VCC_DATA_PARMS pDp = &pOamRsp->DataParms;
            PATM_BUFFER pAb = &pOamRsp->AtmBuffer;
            memcpy( pDp, pDataParms, sizeof(ATM_VCC_DATA_PARMS) );
            pDp->ucFlags = 0;
            pDp->ucSendPriority = ANY_PRIORITY;
            pDp->pAtmBuffer = pAb;
            pDp->pFnFreeDataParms = (FN_FREE_DATA_PARMS) ATM_VCC_APPL::FreeDp;
            pDp->ulParmFreeDataParms = (UINT32) this;
            pAb->pNextAtmBuf = NULL;
            pAb->pDataBuf = pOamRsp->ucBuffer;
            pAb->ulDataLen = pDataParms->pAtmBuffer->ulDataLen;
            pAb->ulReserved = 0;
            memcpy( pAb->pDataBuf, pData, pAb->ulDataLen );
            pData = pAb->pDataBuf;

            // Chaage AIS (Alarm Indication Signal) request to RDI (Remote
            // Defect Indicator) or Loopback request to Loopback response.
            if( pData[culOamTypeFuncTypeIndex] == cucFaultMgmtAis )
                pData[culOamTypeFuncTypeIndex] = cucFaultMgmtRdi;
            else // loopback
                pData[culLoopbackIndicatorIndex] = 0;

            // Send the OAM response.
            Send( &pOamRsp->DataParms );
        }
    }
} // ProcessOamCell


//**************************************************************************
// Function Name: FreeDp
// Description  : Frees memory used to send an OAM response packet.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::FreeDp( PATM_VCC_DATA_PARMS pDataParms )
{
    ATM_VCC_APPL *pThis = (ATM_VCC_APPL *) pDataParms->ulParmFreeDataParms;
    (*pThis->m_pOsServices->pfnFree) (pDataParms);
} // FreeDp


//**************************************************************************
// Function Name: UpdateAal5VccStats
// Description  : Updates AAL5 VCC statistics.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::UpdateAal5VccStats( PATM_VCC_DATA_PARMS pDataParms )
{
    // The ATM Processor buffer descriptor AAL error status byte is temporarily
    // stored in the data parms ucReserved[1].
    UINT8 ucRxAalError = pDataParms->ucReserved[1];
    PATM_VCC_AAL5_STATS pStats = &m_pVccInfo->Stats.u.AtmVccAal5Stats;

    pDataParms->baReceiveStatus = STS_ERROR;

    if( (ucRxAalError & RXAAL5AAL0_CRC_ERROR) != 0 )
    {
        pStats->ulAal5VccCrcErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_CRC_ERROR;
    }

    if( (ucRxAalError & RXAAL5AAL0_SHORT_PKT_ERROR) != 0 )
    {
        pStats->ulAal5VccShortPacketErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_SHORT_PKT_ERROR;
    }

    if( (ucRxAalError & RXAAL5AAL0_LENGTH_ERROR) != 0 )
    {
        pStats->ulAal5VccLengthErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_LENGTH_ERROR;
    }

    if( (ucRxAalError & RXAAL5AAL0_BIG_PKT_ERROR) != 0 )
    {
        pStats->ulAal5VccOverSizedSdus++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_BIG_PKT_ERROR;
    }
} // UpdateAal5VccStats


//**************************************************************************
// Function Name: UpdateAal0VccStats
// Description  : Updates AAL0 VCC statistics.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::UpdateAal0PktVccStats( PATM_VCC_DATA_PARMS pDataParms )
{
    // The ATM Processor buffer descriptor AAL error status byte is temporarily
    // stored in the data parms ucReserved[1].
    UINT8 ucRxAalError = pDataParms->ucReserved[1];
    PATM_VCC_AAL0PKT_STATS pStats = &m_pVccInfo->Stats.u.AtmVccAal0PktStats;

    pDataParms->baReceiveStatus = STS_ERROR;

    if( (ucRxAalError & RXAAL5AAL0_BIG_PKT_ERROR) != 0 )
    {
        pStats->ulAal0VccOverSizedSdus++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_BIG_PKT_ERROR;
    }

} // UpdateAal0VccStats


//**************************************************************************
// Function Name: UpdateAal0VccStats
// Description  : Updates AAL0 VCC statistics.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::UpdateAal0CellVccStats( PATM_VCC_DATA_PARMS pDataParms )
{
    // The ATM Processor buffer descriptor AAL error status byte is temporarily
    // stored in the data parms ucReserved[1].
    UINT8 ucRxAalError = pDataParms->ucReserved[1];
    PATM_VCC_AAL0CELL_STATS pStats = &m_pVccInfo->Stats.u.AtmVccAal0CellStats;

    pDataParms->baReceiveStatus = STS_ERROR;

    if( (ucRxAalError & RXAAL5AAL0_CRC_ERROR) != 0 )
    {
        pStats->ulAal0VccCrcErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_AAL5_AAL0_CRC_ERROR;
    }

} // UpdateAal0VccStats


//**************************************************************************
// Function Name: UpdateAtmStats
// Description  : Updates ATM statistics.
// Returns      : None.
//**************************************************************************
void ATM_VCC_APPL::UpdateAtmStats( PATM_VCC_DATA_PARMS pDataParms,
    PATM_INTF_ATM_STATS pStats )
{
    // The ATM Processor buffer descriptor ATM error status byte is temporarily
    // stored in the data parms ucReserved[0].
    UINT8 ucRxAtmError = pDataParms->ucReserved[0];

    if( (ucRxAtmError & RXATM_INVALID_VPI_VCI) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInInvalidVpiVciErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_INVALID_VPI_VCI;
    }

    if( (ucRxAtmError & RXATM_HEC_ERROR) != 0 )
    {
        pStats->ulIfInErrors++;
        pStats->ulIfInHecErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_HEC_ERROR;
    }

    if( (ucRxAtmError & RXATM_PORT_NOT_ENABLED) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInPortNotEnabledErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_PORT_NOT_ENABLED;
    }

    if( (ucRxAtmError & RXATM_PTI_ERROR) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInPtiErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_PTI_ERROR;
    }

    if( (ucRxAtmError & RXATM_RECEIVED_IDLE_CELL) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInIdleCells++;
        pDataParms->baReceiveStatus = STS_PKTERR_RECEIVED_IDLE_CELL;
    }

    if( (ucRxAtmError & RXATM_OAM_RM_CRC_ERROR) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInOamRmCrcErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_OAM_RM_CRC_ERROR;
    }

    if( (ucRxAtmError & RXATM_GFC_ERROR) != 0 )
    {
        pStats->ulIfInUnknownProtos++;
        pStats->ulIfInGfcErrors++;
        pDataParms->baReceiveStatus = STS_PKTERR_GFC_ERROR;
    }
} // UpdateAtmStats


//**************************************************************************
// ATM_MGMT_APPL Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_MGMT_APPL
// Description  : Constructor for the ATM management application access class.
// Returns      : None.
//**************************************************************************
ATM_MGMT_APPL::ATM_MGMT_APPL( ATM_OS_SERVICES *pOsServices,
    ATM_APPL_TABLE *pApplTable, UINT32 ulAtmApplSem, void *pSoftSar )
    : ATM_APPL( pOsServices, pApplTable, ulAtmApplSem, pSoftSar )
{
    m_ulFlags = 0;
    m_pInterfaces = NULL;
    m_ulNumInterfaces = 0;
    m_pFnReceiveDataCb = NULL;
    m_ulParmReceiveData = 0;
    m_ulState = ST_APPL_CONSTRUCTED;
} // ATM_MGMT_APPL


//**************************************************************************
// Function Name: ~ATM_MGMT_APPL
// Description  : Destructor for the ATM management application access class.
// Returns      : None.
//**************************************************************************
ATM_MGMT_APPL::~ATM_MGMT_APPL( void )
{
    Detach();
} // ~ATM_MGMT_APPL


//**************************************************************************
// Function Name: Attach
// Description  : Attach an application for send and receiving ATM management
//                (OAM and RM) cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_MGMT_APPL::Attach( ATM_INTERFACE *pInterfaces,
    UINT32 ulNumInterfaces, PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    pAttachParms->ulHandle = 0;

    // Verify that the caller is using the same structure format as this module.
    if( pAttachParms->ulStructureId == ID_ATM_VCC_ATTACH_PARMS &&
        pAttachParms->ulTransmitQParmsSize == 0 )
    {
        if( m_ulState == ST_APPL_CONSTRUCTED )
        {
            // Save some parameters to local data members.
            m_pInterfaces = pInterfaces;
            m_ulNumInterfaces = ulNumInterfaces;
            m_ulFlags = pAttachParms->ulFlags;
            m_pFnReceiveDataCb = pAttachParms->pFnReceiveDataCb;
            m_ulParmReceiveData = pAttachParms->ulParmReceiveData;

            // Call each supported interface with the cell types and this
            // object address.  The interface object uses this information
            // to route received data.
            UINT32 i;
            for( i = 0; i < m_ulNumInterfaces && baStatus == STS_SUCCESS; i++ )
                baStatus = m_pInterfaces[i].SetMgmtCellAppl(this, m_ulFlags);

            if( baStatus == STS_SUCCESS )
            {
                m_pApplTable->RegisterAppl( this );
                pAttachParms->ulHandle = (UINT32) this;
                m_ulState = ST_APPL_ATTACHED;
            }
            else
            {
                m_ulState = ST_APPL_ERROR;
                Detach();
            }
        }
        else
            baStatus = STS_STATE_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // Attach


//**************************************************************************
// Function Name: Detach
// Description  : Detach a management application from send and receiving
//                ATM management cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_MGMT_APPL::Detach( void )
{
    if( m_ulState == ST_APPL_ATTACHED || m_ulState == ST_APPL_ERROR )
    {
        m_ulState = ST_APPL_DETACH_PENDING;
        m_pApplTable->UnregisterAppl( this );

        for( UINT32 i = 0; i < m_ulNumInterfaces; i++ )
            m_pInterfaces[i].ResetMgmtCellAppl( this, m_ulFlags );

        m_pInterfaces = NULL;
        m_ulNumInterfaces = 0;
        m_ulState = ST_APPL_CONSTRUCTED;
    }

    return( STS_SUCCESS );
} // Detach


//**************************************************************************
// Function Name: Send
// Description  : Attach an application for send and receiving ATM management
//                (OAM and RM) cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_MGMT_APPL::Send( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( m_ulState == ST_APPL_ATTACHED &&
        pDataParms->ulStructureId == ID_ATM_VCC_DATA_PARMS )
    {
        // Verify that this object handles the interface that the data is
        // being sent to.
        ATM_INTERFACE *pIntf;
        baStatus = STS_NOT_FOUND;
        for( UINT32 i = 0; i < m_ulNumInterfaces; i++ )
        {
            if( m_pInterfaces[i].GetInterfaceId() == pVccAddr->ulInterfaceId )
            {
                pIntf = &m_pInterfaces[i];
                baStatus = STS_SUCCESS;
                break;
            }
        }

        if( baStatus == STS_SUCCESS )
        {
            const UINT8 ucOamF5Segment  = (0x04 << 1);
            const UINT8 ucOamF5EndToEnd = (0x05 << 1);

            UINT8 ucCt = 0;
            UINT32 ulGfcIdx = 0;
            UINT32 ucPti = 0;

            if( pVccAddr->usVci == VCI_OAM_F4_SEGMENT &&
                (m_ulFlags & AVAP_ALLOW_OAM_F4_SEGMENT_CELLS) != 0 )
            {
                ucCt = CT_OAM_F4_ANY;
                ulGfcIdx = GFC_OAM_F4_SEG_IDX; 
            }
            else if( pVccAddr->usVci == VCI_OAM_F4_END_TO_END &&
                (m_ulFlags & AVAP_ALLOW_OAM_F4_END_TO_END_CELLS) != 0 )
            {
                ucCt = CT_OAM_F4_ANY;
                ulGfcIdx = GFC_OAM_F4_ETOE_IDX; 
            }
            else if( pDataParms->ucCircuitType == CT_OAM_F5_SEGMENT &&
                 (m_ulFlags & AVAP_ALLOW_OAM_F5_SEGMENT_CELLS) != 0 )
            {
                ucCt = CT_OAM_F5_SEGMENT;
                ulGfcIdx = GFC_OAM_F5_SEG_IDX; 
                ucPti = ucOamF5Segment;
            }
            else if( pDataParms->ucCircuitType == CT_OAM_F5_END_TO_END &&
                 (m_ulFlags & AVAP_ALLOW_OAM_F5_END_TO_END_CELLS) != 0 )
            {
                ucCt = CT_OAM_F5_END_TO_END;
                ulGfcIdx = GFC_OAM_F5_ETOE_IDX; 
                ucPti = ucOamF5EndToEnd;
            }
            else if( pDataParms->ucCircuitType == CT_RM &&
                 (m_ulFlags & AVAP_ALLOW_RM_CELLS) != 0 )
            {
                ucCt = CT_RM;
                ulGfcIdx = GFC_OAM_F5_ETOE_IDX; /* borrow index */
            }
            else
                baStatus = STS_PARAMETER_ERROR;

            if( baStatus == STS_SUCCESS )
            {
                UINT32 i;
                ATM_TX_BDQ *pTxQ = NULL;
                ATM_INTERFACE_STATS *pIntfStats = pIntf->GetStats();

                pDataParms->pAtmBuffer->ulReserved = ulGfcIdx;
                pDataParms->ucCircuitType = ucCt;
                pDataParms->ucSendPriority = ANY_PRIORITY;

                // Find any transmit queue to send the cell.  The VCC
                // address will be taken from the GFC table in the
                // ADSL phy.
                for( i = 0; i < MAX_QUEUE_SHAPERS; i++)
                {
                    pTxQ = m_pApplTable->GetTxQByShaperNum( i );
                    if( pTxQ != NULL )
                    {
                        /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
                    	if( NULL != m_pOsServices->pfnAdslSetVcEntryEx)
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
                    	{
	                        (*m_pOsServices->pfnAdslSetVcEntryEx) (
	                            ulGfcIdx, (int)pIntf->GetPhyPortNum(),
	                            pVccAddr->usVpi, pVccAddr->usVci, ucPti);
                        }
                        /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
                        
                        // Send the OAM cell.
                        pTxQ->Add( pDataParms );
                        pIntfStats->Aal0IntfStats.ulIfOutUcastPkts++;
#if defined(CONFIG_BCM96338)
                        pIntfStats->Aal0IntfStats.ulIfOutOctets +=
                            pDataParms->pAtmBuffer->ulDataLen;
#endif
                        break;
                    }
                }

                if( pTxQ == NULL )
                {
                    // No ATM application is currently active so no transmit
                    // queue is in use.  Temporarily create a transmit queue to
                    // send the OAM cell on.
                    ATM_TRAFFIC_DESCR_PARM_ENTRY Entry =
                        { ID_ATM_TRAFFIC_DESCR_PARM_ENTRY, 0,
                          TDT_ATM_NO_TRAFFIC_DESCRIPTOR, 0, 0, 0, 0, 0, 0,
                          SC_UBR, 0, };
                    ATM_TX_BDQ TxQ( m_pOsServices, m_ulAtmApplSem,
                        m_pInterfaces->IsPhyPort1Enabled(), m_pSoftSar );

                    // Create the transmit queue.
                    baStatus = TxQ.Create( 4, 1, 0, PHY_0, MAX_VCIDS-1, &Entry );
                    if( baStatus == STS_SUCCESS )
                    {
                        /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
                    	if( NULL != m_pOsServices->pfnAdslSetVcEntryEx)
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
                    	{
	                        (*m_pOsServices->pfnAdslSetVcEntryEx) ( ulGfcIdx,
	                            (int) pIntf->GetPhyPortNum(), pVccAddr->usVpi,
	                            pVccAddr->usVci, ucPti);
                        }
                        /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
                        
                        // Send the OAM cell.
                        baStatus = TxQ.Add( pDataParms );
                        if( baStatus == STS_SUCCESS )
                        {
                            pIntfStats->Aal0IntfStats.ulIfOutUcastPkts++;
#if defined(CONFIG_BCM96338)
                            pIntfStats->Aal0IntfStats.ulIfOutOctets +=
                                pDataParms->pAtmBuffer->ulDataLen;
#endif

                            // Wait for the cell to be sent.
                            TxQ.Reclaim();
                            for(i = 0; i < 10000 && TxQ.IsQueueEmpty() == 0; i++)
                                TxQ.Reclaim();
                        }

                        // Destroy the queue.
                        TxQ.Destroy();
                    }
                }
            }
        }
    }
    else
    {
        baStatus = (m_ulState != ST_APPL_ATTACHED )
            ? STS_STATE_ERROR : STS_PARAMETER_ERROR;
    }

    return( baStatus );
} // Send


//**************************************************************************
// Function Name: Receive
// Description  : Handles receive data for a management application.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_MGMT_APPL::Receive( PATM_VCC_ADDR pAddr,
    PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus;

    if( m_ulState == ST_APPL_ATTACHED )
    {
        // Pass the received data to the application.
        pDataParms->ucFlags |= RX_DATA_PENDING;
        (*m_pFnReceiveDataCb) ((UINT32) this, pAddr, pDataParms,
            m_ulParmReceiveData);
        baStatus = STS_SUCCESS;
    }
    else
        baStatus = STS_STATE_ERROR;

    return( baStatus );
} // Receive


#if !defined(_CFE_BRIDGE_)
//**************************************************************************
// ATM_TRANSPARENT_APPL Class
//**************************************************************************

//**************************************************************************
// Function Name: ATM_TRANSPARENT_APPL
// Description  : Constructor for the ATM management application access class.
// Returns      : None.
//**************************************************************************
ATM_TRANSPARENT_APPL::ATM_TRANSPARENT_APPL( ATM_OS_SERVICES *pOsServices,
    ATM_APPL_TABLE *pApplTable, UINT32 ulAtmApplSem, void *pSoftSar )
    : ATM_APPL( pOsServices, pApplTable, ulAtmApplSem, pSoftSar )
{
    m_pInterfaces = NULL;
    m_ulNumInterfaces = 0;
    m_pFnReceiveDataCb = NULL;
    m_ulParmReceiveData = 0;
    m_ulState = ST_APPL_CONSTRUCTED;
    memset( m_pTxQViaPort, 0x00, sizeof(m_pTxQViaPort) );
    memset( m_GfcTableInfo, 0x00, sizeof(m_GfcTableInfo) );
} // ATM_TRANSPARENT_APPL


//**************************************************************************
// Function Name: ~ATM_TRANSPARENT_APPL
// Description  : Destructor for the ATM management application access class.
// Returns      : None.
//**************************************************************************
ATM_TRANSPARENT_APPL::~ATM_TRANSPARENT_APPL( void )
{
    Detach();
} // ~ATM_MGMT_APPL


//**************************************************************************
// Function Name: Attach
// Description  : Attach an application for sending and receiving transparent
//                ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TRANSPARENT_APPL::Attach( ATM_INTERFACE *pInterfaces,
    UINT32 ulNumInterfaces, PATM_VCC_ATTACH_PARMS pAttachParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    pAttachParms->ulHandle = 0;

    // Verify parameters.
    if( pAttachParms->ulStructureId == ID_ATM_VCC_ATTACH_PARMS &&
        pAttachParms->ulTransmitQParmsSize == 1 &&
        m_ulState == ST_APPL_CONSTRUCTED )
    {
        // Save some parameters to local data members.
        m_pInterfaces = pInterfaces;
        m_ulNumInterfaces = ulNumInterfaces;
        m_pFnReceiveDataCb = pAttachParms->pFnReceiveDataCb;
        m_ulParmReceiveData = pAttachParms->ulParmReceiveData;

        PATM_VCC_TRANSMIT_QUEUE_PARMS pTxQParms;
        pTxQParms = &pAttachParms->pTransmitQParms[0];
        ATM_INTERFACE *pInterface = NULL;
        UINT32 i;
        for( i = 0; i < m_ulNumInterfaces && baStatus == STS_SUCCESS; i++ )
        {
            // Call each supported interface with this object address.  The
            // interface object uses this information to route received data.
            pInterface = &m_pInterfaces[i];
            if( pInterface->IsInterfaceUp() )
            {
                baStatus = pInterface->SetTransparentAppl( this );

                // Create transmit queues if information is provided in the
                // attach structure.
                if( baStatus == STS_SUCCESS )
                {
                    UINT8 ucPortNum = pInterface->GetPhyPortNum();

                    // A transmit queue is being used instead of the MIPS
                    // Cell FIFO.  A different transmit queue is created for
                    // each interface requested.
                    ATM_TRAFFIC_DESCR_PARM_ENTRY Entry =
                        { ID_ATM_TRAFFIC_DESCR_PARM_ENTRY,
                        0, // Index
                        TDT_ATM_CLP_TRANSPARENT_NO_SCR, // Type
                        365566, // PCR = 155 Mbs / (53 * 8) cells
                        0, // Parm2.
                        0, // Parm3.
                        0, // Parm4.
                        0, // Parm5.
                        0, // Row Status.
                        SC_CBR, // Service category.
                        0, // Frame discard.
                        };

                    baStatus = AttachTxQueues(pAttachParms, ucPortNum, MAX_VCIDS,
                        &Entry, pInterface->IsPhyPort1Enabled());

                    if( baStatus == STS_SUCCESS )
                    {
                        m_pTxQViaPort[ucPortNum] =
                            m_pTxQViaPriority[pTxQParms->ulPriority];

                        GFC_TBL_INFO *pGti = &m_GfcTableInfo[ucPortNum];
                        GFC_TBL_INFO::GFC_TBL_ENTRY *pGte = pGti->GfcTableEntries;
                        pGti->pGfcTableFreePool = pGte;
                        memset(pGti->pGfcTable, 0x00, sizeof(pGti->pGfcTable));
                        for( UINT32 i = 0; i < MAX_GFC_TABLE_ENTRIES; i++ )
                        {
                            pGte[i].pNext = &pGte[i + 1];
                            pGte[i].ulGfcIdxAtmHdr = (i+1) << GFCTBL_GFC_SHIFT;
                        }
                        pGte[MAX_GFC_TABLE_ENTRIES - 1].pNext = NULL;
                    }
                }
            }
            else
                baStatus = STS_INTERFACE_DOWN;
        }

        if( baStatus == STS_SUCCESS )
        {
            m_pApplTable->RegisterAppl( this );
            pAttachParms->ulHandle = (UINT32) this;
            m_ulState = ST_APPL_ATTACHED;
        }
        else
        {
            m_ulState = ST_APPL_ERROR;
            Detach();
        }
    }
    else
        baStatus = (m_ulState == ST_APPL_CONSTRUCTED)
            ? STS_PARAMETER_ERROR : STS_STATE_ERROR;

    return( baStatus );
} // Attach


//**************************************************************************
// Function Name: Detach
// Description  : Detach an application from send and receiving transparent
//                ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TRANSPARENT_APPL::Detach( void )
{
    if( m_ulState == ST_APPL_ATTACHED || m_ulState == ST_APPL_ERROR )
    {
        m_ulState = ST_APPL_DETACH_PENDING;
        m_pApplTable->UnregisterAppl( this );

        for( UINT32 i = 0; i < m_ulNumInterfaces; i++ )
        {
            UINT8 ucPhyPort = m_pInterfaces[i].GetPhyPortNum();
            m_pInterfaces[i].ResetTransparentAppl( this );
            /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
            for( int j = 1; j < MAX_GFC_TABLE_ENTRIES && (NULL != m_pOsServices->pfnAdslSetVcEntry); j++ )
#else
            for( int j = 1; j < MAX_GFC_TABLE_ENTRIES; j++ )
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
            {
                (*m_pOsServices->pfnAdslSetVcEntry) (j, (int) ucPhyPort, 0, 0);
            }
            /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
        }

        DetachTxQueues();
        memset( m_pTxQViaPort, 0x00, sizeof(m_pTxQViaPort) );
        m_pInterfaces = NULL;
        m_ulState = ST_APPL_CONSTRUCTED;
    }

    return( STS_SUCCESS );
} // Detach


//**************************************************************************
// Function Name: Send
// Description  : Attach an application for send and receiving transparent
//                ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TRANSPARENT_APPL::Send( UINT32 ulInterfaceId,
    PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT8 ucPhyPort = 0;
    ATM_INTERFACE *pIntf;
    UINT32 i;

    if( m_ulState == ST_APPL_ATTACHED &&
        pDataParms->ulStructureId == ID_ATM_VCC_DATA_PARMS &&
        pDataParms->ucCircuitType == CT_TRANSPARENT )
    {
        // Verify that this object handles the interface that the data is
        // being sent to.
        baStatus = STS_NOT_FOUND;
        for( i = 0, pIntf = m_pInterfaces; i < m_ulNumInterfaces; i++, pIntf++ )
        {
            if( pIntf->GetInterfaceId() == ulInterfaceId )
            {
                if( pIntf->IsLinkUp() )
                {
                    // Add the ATM physical port to the circuit type.
                    ucPhyPort = pIntf->GetPhyPortNum();
                    ATM_TX_BDQ *pTxQ = m_pTxQViaPort[ucPhyPort];
                    if( pTxQ )
                    {
                        baStatus = SetGfcIndicies( pDataParms, ucPhyPort );
                        if( baStatus == STS_SUCCESS )
                            baStatus = pTxQ->Add( pDataParms );
                    }
                    else
                        baStatus = STS_PARAMETER_ERROR;
                }
                else
                    baStatus = STS_LINK_DOWN;
                break;
            }
        }
    }
    else
    {
        baStatus = (m_ulState != ST_APPL_ATTACHED)
            ? STS_STATE_ERROR : STS_PARAMETER_ERROR;
    }

    return( baStatus );
} // Send


//**************************************************************************
// Function Name: Receive
// Description  : Handles receive data for a application that send and
//                receive transparent ATM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TRANSPARENT_APPL::Receive( PATM_VCC_DATA_PARMS pDataParms )
{
    BCMATM_STATUS baStatus;

    if( m_ulState == ST_APPL_ATTACHED )
    {
        // Transparent cells are received with a VPI/VCI error.  Change the
        // status to success.
        pDataParms->baReceiveStatus = STS_SUCCESS;

        // Force the circuit type to transparent.
        pDataParms->ucCircuitType = CT_TRANSPARENT;

        // Pass the received data to the application.
        pDataParms->ucFlags |= RX_DATA_PENDING;
        (*m_pFnReceiveDataCb) ((UINT32) this, NULL, pDataParms,
            m_ulParmReceiveData);

        baStatus = STS_SUCCESS;
    }
    else
        baStatus = STS_STATE_ERROR;

    return( baStatus );
} // Receive


//**************************************************************************
// Function Name: SetGfcIndicies
// Description  : Finds a GFC index for the VPI/VCI in each ATM_BUFFER.  Each
//                ATM_BUFFER contains a raw ATM cell - four byte ATM Header
//                (no HEC) and up to 48 byte payload.  If the index is not
//                found, a new index is added to the table.
// Returns      : GFC index or 0 if no index is found
//**************************************************************************
BCMATM_STATUS ATM_TRANSPARENT_APPL::SetGfcIndicies( PATM_VCC_DATA_PARMS
    pDataParms, UINT8 ucPhyPort )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;
    UINT32 ulAtmHdr, ulHashValue;
    GFC_TBL_INFO *pGti = &m_GfcTableInfo[ucPhyPort];
    GFC_TBL_INFO::GFC_TBL_ENTRY *pGte;

    // The ATM_VCC_DATA_PARMS structure contains one or more ATM_BUFFER
    // structures.  Each ATM_BUFFER structure contains a raw ATM cell that can
    // potentially have a different VPI/VCI than the previous ATM_BUFFER.
    while( pAb )
    {
        ulAtmHdr = *(UINT32 *) pAb->pDataBuf;
        ulHashValue = (UINT32)
            ((ulAtmHdr >> GFCTBL_VCI_SHIFT) & (MAX_GFC_TABLE_SIZE - 1));

        // Look for an existing GFC index for the current ATM header.
        for( pGte = pGti->pGfcTable[ulHashValue]; pGte; pGte = pGte->pNext )
        {
            if( (pGte->ulGfcIdxAtmHdr & ~GFCTBL_GFC_MASK) == ulAtmHdr )
            {
                // The GFC index for the ATM header was found.
                pAb->ulReserved =
                    (pGte->ulGfcIdxAtmHdr&GFCTBL_GFC_MASK) >> GFCTBL_GFC_SHIFT;
                break;
            }
        }

        if( pGte == NULL )
        {
            // The GFC index for the ATM header was not found.
            if( pGti->pGfcTableFreePool )
            {
                // Assign a new GFC index to this ATM header.
                pGte = pGti->pGfcTableFreePool;
                pGti->pGfcTableFreePool = pGti->pGfcTableFreePool->pNext;

                pAb->ulReserved =
                    (pGte->ulGfcIdxAtmHdr&GFCTBL_GFC_MASK) >> GFCTBL_GFC_SHIFT;
                pGte->ulGfcIdxAtmHdr |= ulAtmHdr;
                pGte->pNext = pGti->pGfcTable[ulHashValue];
                pGti->pGfcTable[ulHashValue] = pGte;
				
				/*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
				if( NULL != m_pOsServices->pfnAdslSetVcEntryEx )
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/
				{
	                (*m_pOsServices->pfnAdslSetVcEntryEx) ( (int) pAb->ulReserved,
	                    (int) ucPhyPort,
	                    (int) ((ulAtmHdr & GFCTBL_VPI_MASK) >> GFCTBL_VPI_SHIFT),
	                    (int) ((ulAtmHdr & GFCTBL_VCI_MASK) >> GFCTBL_VCI_SHIFT),
	                    (int) (ulAtmHdr & GFCTBL_PTI_CLP_MASK) );
                }
                /*End modify:l37298 2006-05-30 for deleting adsl feature*/	

            }
            else
            {
                // All available GFC indicies (MAX_GFC_TABLE_ENTRIES) are in
                // use.  There is currently no mechanism to reassign the least
                // recently used GFC index to this VPI/VCI.  This change can be
                // made if there becomes a need.
                baStatus = STS_RESOURCE_ERROR;
                break;
            }
        }

        pAb = pAb->pNextAtmBuf;
    }

    return( baStatus );
} // SetGfcIndicies
#endif

