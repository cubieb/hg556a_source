/*
<:copyright-broadcom 
 
 Copyright (c) 2004 Broadcom Corporation 
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
// File Name  : AtmSoftSar.cpp
//
// Description: This file contains the ATM Soft SAR implementation.
//
//**************************************************************************

#include "atmapiimpl.h"

FN_PRINTF g_pfnSsPrintf = NULL; 

#if defined(CONFIG_ATM_SOFTSAR) || defined(CONFIG_ATM_RX_SOFTSAR)

//**************************************************************************
// ATM_SOFT_SAR class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_SOFT_SAR
// Description  : Constructor for the ATM Soft SAR class.
// Returns      : None.
//**************************************************************************
ATM_SOFT_SAR::ATM_SOFT_SAR( ATM_OS_SERVICES *pOsServices )
{
    m_pOsServices = pOsServices;
    memset( m_VccAddrForVcid, 0x00, sizeof(m_VccAddrForVcid) );
    memset( &m_VcTable, 0x00, sizeof(m_VcTable) );
    m_vcidMask = 0;
    m_timeCurrent = 0;
    m_lastTxCellTime = 0;
    m_txIdleTimeOut = 0;
    m_MaxDiff = 0;
    m_MinDiff = 0;
    m_lastIdleTimerTime = 0;
    m_atmssConnected = 0;
    m_connectRate = 0;
    m_cellTicks = 0;
    m_cellsToFill = 0;
    m_cellHead = m_cellTail = NULL;
#if defined(CONFIG_ATM_RX_SOFTSAR)
    memset( m_rxMem, 0x00, sizeof(m_rxMem) );
    memset( m_vccApplMap, 0x00, sizeof(m_vccApplMap) );
#endif
} // ATM_SOFT_SAR


//**************************************************************************
// Function Name: ~ATM_SOFT_SAR
// Description  : Destructor for the ATM Soft SAR class.
// Returns      : None.
//**************************************************************************
ATM_SOFT_SAR::~ATM_SOFT_SAR( void )
{
#if defined(CONFIG_ATM_RX_SOFTSAR)
    int i;
    for( i = 0; m_rxMem[i] != NULL; i++ )
    {
        (*m_pOsServices->pfnFree) (m_rxMem[i]);
        m_rxMem[i] = NULL;
    }
#endif
} // ~ATM_SOFT_SAR


//**************************************************************************
// Function Name: AssignVcid
// Description  : Assigns a unique index for VCC.
// Returns      : a VCID value or 0xff if one is not available
//**************************************************************************
UINT8 ATM_SOFT_SAR::AssignVcid( PATM_VCC_ADDR pAddr )
{
    UINT8 ucRet = 0xff;
    int i, ulMaxVccs = MAX_VCIDS;

#if !defined(CONFIG_ATM_RX_SOFTSAR)
    ulMaxVccs -= MAX_VCCS - MAX_RX_VCIDS;
#endif

    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
    for( i = 0; i < ulMaxVccs; i++ )
    {
        if( (m_vcidMask & (1 << i)) == 0 )
        {
            ucRet = i;
            m_vcidMask |= (1 << i);
            m_VccAddrForVcid[i] = pAddr;
            break;
        }
    }
    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( ucRet );
} // AssignVcid


//**************************************************************************
// Function Name: UnassignVcid
// Description  : Unassigns a unique index for VCC.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::UnassignVcid( UINT8 ucVcid )
{
    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
    if( ucVcid < MAX_VCIDS )
    {
        m_vcidMask &= ~(1 << ucVcid);
        m_VccAddrForVcid[ucVcid] = NULL;
    }
    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);
} // UnassignVcid


//**************************************************************************
// Function Name: GetVccAddr
// Description  : Returns the VCC address for the specified vcid.
// Returns      : None.
//**************************************************************************
ATM_VCC_ADDR *ATM_SOFT_SAR::GetVccAddr( UINT8 ucVcid )
{
    return( (ucVcid < MAX_VCIDS) ? m_VccAddrForVcid[ucVcid] : NULL );
} // GetVccAddr


//**************************************************************************
// Function Name: CreateVc
// Description  : Allocates the soft SAR resources to handle sending data
//                on a VCC.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::CreateVc( UINT8 ucVcid, UINT32 ulTxPoolSize,
    UINT32 ulNumPriorities, PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    BCMATM_STATUS baStatus = STS_PARAMETER_ERROR;
    atmssVcState *pVc = NULL;
    PATM_VCC_ADDR pAddr = m_VccAddrForVcid[ucVcid];

    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
    if( pAddr )
    {
        UINT32 vpivci = ((UINT32) pAddr->usVpi << 16) | pAddr->usVci;
        baStatus = AtmLayerVcAllocate( vpivci, ulTxPoolSize, ulNumPriorities,
            &pVc, pEntry );
        if( baStatus == STS_SUCCESS )
        {
            baStatus = AtmLayerVcActivate( pVc );
            if( baStatus == STS_SUCCESS )
                m_VcStateForVcid[ucVcid] = pVc;
        }
        else
            // This function will be called more than one time for the VCC if
            // IP QOS is enabled.  This is OK.
            if( baStatus == STS_IN_USE )
                baStatus = STS_SUCCESS;
    }
    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( baStatus );
} // CreateVc


//**************************************************************************
// Function Name: DestroyVc
// Description  : Deallocates the soft SAR resources that were allocated in
//                CreateVc
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::DestroyVc( UINT8 ucVcid )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    atmssVcState *pVc = m_VcStateForVcid[ucVcid];

    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
    if( pVc )
    {
        int i, size = sizeof(m_VcStateForVcid) / sizeof(atmssVcState *);

        // Only clean up VCC resources if no other VCID is using it.
        m_VcStateForVcid[ucVcid] = NULL;
        for( i = 0; i < size && m_VcStateForVcid[i] != pVc; i++ )
            ;

        if( i == size )
        {
            // No other VCID is using this VCC.
            AtmAal5VcClose( pVc );
            AtmLayerVcDeactivate( pVc );
            AtmLayerVcFree( pVc );
        }
    }
    else
        baStatus = STS_PARAMETER_ERROR;
    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( baStatus );
} // DestroyVc


//**************************************************************************
// Function Name: AddPkt
// Description  : Adds a packet to be segmented and transmited out the
//                ADSL PHY.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AddPkt(UINT8 ucVcid, PATM_VCC_DATA_PARMS pDataParms)
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    atmssVcState *pVc = m_VcStateForVcid[ucVcid];
    UINT32 ulPri = pDataParms->ucSendPriority - 1;

    if( ulPri >= MAX_TX_PRIORITIES )
        ulPri = 0;

    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
    if( pVc && pVc->aal5State.dlistTxWaitingCount[ulPri] <
        pVc->aal5State.dlistTxWaitingMaxSize )
    {
        atmssFrame *pFrame = pVc->txFrameList.GetFrame();
        if( pFrame )
        {
            atmssFrameBuffer *pBuf;
            PATM_BUFFER pAb;

            pFrame->Init();
            pFrame->SetUserParam((UINT32) pDataParms);
            for( pAb = pDataParms->pAtmBuffer; pAb; pAb = pAb->pNextAtmBuf )
            {
                pBuf = pVc->txBufferList.GetFrameBuffer();
                if( pBuf )
                {
                    pBuf->SetLength( pAb->ulDataLen );
                    pBuf->SetAddress( pAb->pDataBuf );
                    pFrame->EnqueBufferAtBack( pBuf );
                }
                else
                {
                    baStatus = STS_RESOURCE_ERROR;
                    while( (pBuf = pFrame->DequeBuffer()) != NULL )
                        pVc->txBufferList.PutFrameBuffer( pBuf );
                    pVc->txFrameList.PutFrame( pFrame );
                    break;
                }
            }

            if( baStatus == STS_SUCCESS )
                AtmLayerQueueFrame(pVc, pFrame, ulPri);
        }
        else
            baStatus = STS_RESOURCE_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;
    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

    return( baStatus );
} // AddPkt


//**************************************************************************
// Function Name: AddCell
// Description  : Adds a cell to be transmited out the ADSL PHY.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AddCell(PATM_VCC_DATA_PARMS pDataParms)
{
    if( m_cellHead == NULL )
        m_cellHead = m_cellTail = pDataParms;
    else
    {
        m_cellTail->pApplicationLink = pDataParms;
        m_cellTail = pDataParms;
    }
    m_cellTail->pApplicationLink = NULL;

    return( STS_SUCCESS );
} // AddCell


//**************************************************************************
// Function Name: SetShapingValues
// Description  : Sets shaping values.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::SetShapingValues( UINT8 ucVcid,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    atmssVcState *pVc = m_VcStateForVcid[ucVcid];
    if( pVc )
        AtmLayerSetShapingValues( &pVc->vcParams.txTrafficParams, pEntry );
}

//**************************************************************************
// Function Name: StatusHandler
// Description  : Called by Atm Layer to notify about it's internal
//                events
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::StatusHandler( UINT32 statusCode, ... )
{
    UINT32 *value;
    value = (UINT32 *) ((char *) &statusCode + sizeof(UINT32)); 
    switch (statusCode) 
    {
    case kAtmStatRxDiscarded:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM Rx cell discarded "
            "(contract violation). VC = %lu.\n", value[0]);
        break;

    case kAtmStatTxDelayed:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM Tx cell delayed "
            "(contract violation). VC = %lu.\n", value[0]);
        break;

#if 0
    case kAtmStatVcCreated:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM VC=%lu created. "
            "VCI=%8.8lx, AALType=%lu, FwdCellTime=%lu.\n", value[0], value[1],
            value[2], value[3]);
        break;

    case kAtmStatVcStarted:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM VC = %lu started.\n",
            value[0]);
        break;

    case kAtmStatVcStopped:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM VC = %lu stopped.\n",
            value[0]);
        break;

    case kAtmStatVcDeleted:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM VC = %lu deleted.\n",
            value[0]);
        break;
#endif

    case kAtmStatTimeout:
        (*m_pOsServices->pfnPrintf)("ATM Soft SAR: No cells transmitted for "
            "%lu ms.\n",value[0]);
        break;

    case kAtmStatNoCellMemory:
#if 0
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: No memory for Rx cells.\n");
#endif
        break;

    case kAtmStatUnassignedCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: Unassigned cell. hdr = "
            "0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatOamF4SegmentCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: Segment OAM F4 cell, "
            "hdr=0x%8.8lx.\n",value[0]);
        break;

    case kAtmStatOamF4End2EndCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: End-to-end OAM F4 cell, "
            "hdr = 0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatOamI371Cell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: VP resource management "
            "cell (I.371), hdr = 0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatOamF5SegmentCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: Segment OAM F5 cell, "
            "hdr=0x%8.8lx\n",value[0]);
        break;

    case kAtmStatOamF5End2EndCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: End-to-end OAM F5 cell, "
            "hdr = 0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatReservedCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: Reserved cell, hdr = "
            "0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatInvalidCell:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: Invalid cell header "
            "(no VC), hdr = 0x%8.8lx.\n", value[0]);
        break;

    case kAtmStatConnected:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM link connected.\n");
        break;

    case kAtmStatDisconnected:
        (*m_pOsServices->pfnPrintf) ("ATM Soft SAR: ATM link disconnected.\n");
        break;

    default:
        break;
    }
} // StatusHandler

#if defined(CONFIG_ATM_RX_SOFTSAR)
//**************************************************************************
// Function Name: CreateRxBufs
// Description  : Allocates and initializes data structures for the software
//                receive SAR.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::CreateRxBufs( UINT32 ulNumRxBufs, UINT32 ulRxBufSize,
    UINT16 usQBufOffset )
{
    const UINT32 ulMaxRxBufs = 512;
    const UINT32 ulAllocBlockSize = 128 * 1024;
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulSize = (sizeof(ATM_VCC_DATA_PARMS) + sizeof(atmssFrame) +
        sizeof(ATM_BUFFER) + sizeof(atmssFrameBuffer) + usQBufOffset +
        ulRxBufSize + 0x0f + 0x0f) & ~0x0f;
    UINT8 *pcBufAddrs[ulMaxRxBufs];
    UINT8 *p;
    UINT32 i;

    if( ulNumRxBufs < ulMaxRxBufs )
    {
        UINT32 ulBufsPerBlock = ulAllocBlockSize / ulSize;
        UINT32 ulNumBufs = ulNumRxBufs;
        UINT32 ulRxMemIdx = 0;
        UINT32 ulBufAddrsIdx = 0;
        UINT32 ulCurr;
        UINT32 ulAlloc;

        memset( pcBufAddrs, 0x00, sizeof(pcBufAddrs) );
        while( ulNumBufs )
        {
            ulCurr = (ulNumBufs < ulBufsPerBlock) ? ulNumBufs : ulBufsPerBlock;
            ulAlloc = (ulCurr==ulNumBufs) ? (ulCurr * ulSize) : ulAllocBlockSize;

            p = m_rxMem[ulRxMemIdx++] = (UINT8 *)
                (*m_pOsServices->pfnAlloc) (ulAlloc);

            if( p )
            {
                memset(p, 0x00, ulAlloc);
                m_pOsServices->pfnInvalidateCache( p, ulAlloc );

                for( i = 0; i < ulCurr; i++ )
                    pcBufAddrs[ulBufAddrsIdx++] = p + (i * ulSize);

                ulNumBufs -= ulCurr;
            }
            else
            {
                baStatus = STS_ALLOC_ERROR;
                break;
            }
        }
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    if( baStatus == STS_SUCCESS )
    {
        PATM_VCC_DATA_PARMS pDp;
        atmssFrame *pFrm;
        PATM_BUFFER pAb;
        atmssFrameBuffer *pFrmBuf;
        UINT8 *pBuf;

        for( i = 0; i < ulNumRxBufs; i++ )
        {
            p = pcBufAddrs[i];
            pDp = (PATM_VCC_DATA_PARMS) p;
            pFrm = (atmssFrame *) (pDp + 1);
            pAb = (PATM_BUFFER) (pFrm + 1);
            pFrmBuf = (atmssFrameBuffer *) (pAb + 1);
            pBuf = (UINT8 *) (pFrmBuf + 1);
            pBuf = (UINT8 *) (((UINT32) pBuf + 0x0f) & ~0x0f);

            pDp->ulStructureId = ID_ATM_VCC_DATA_PARMS;
            pDp->pFnFreeDataParms = ATM_SOFT_SAR::FreeRxPkt;
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

            pFrm->Init();
            pFrm->SetUserParam((UINT32) pDp);
            pFrmBuf->SetLength( 0 );
            pFrmBuf->SetAddress( pAb->pDataBuf );
            pFrmBuf->SetBufSize( ulRxBufSize );
            pFrm->EnqueBufferAtBack( pFrmBuf );

            m_rxFrameList.PutFrame( pFrm );
        }
    }
    else
        baStatus = STS_ALLOC_ERROR;

    return( baStatus );
} // CreateRxBufs


//**************************************************************************
// Function Name: FreeRxPkt
// Description  : Called by application after it has processed a received
//                data buffer that contained a packet.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::FreeRxPkt( PATM_VCC_DATA_PARMS pDataParms )
{
    ATM_SOFT_SAR *pThis = (ATM_SOFT_SAR *) pDataParms->ulParmFreeDataParms;
    atmssFrame *pFrm = (atmssFrame *) (pDataParms + 1);
    atmssFrameBuffer *pFrmBuf = pFrm->GetFirstBuffer();

    pFrmBuf->SetLength( 0 );
    pThis->m_rxFrameList.PutFrame( pFrm );
    pDataParms->ucFlags &= ~RX_DATA_PENDING;
} // FreeRxPkt


//**************************************************************************
// Function Name: ProcessRxCell
// Description  : Processes a cell received on VCC that does not have a
//                hardware VCID.
// Returns      : ATM_VCC_DATA_PARMS struct for an AAL5 packet or NULL
//**************************************************************************
PATM_VCC_DATA_PARMS ATM_SOFT_SAR::ProcessRxCell( PATM_VCC_DATA_PARMS pDataParms )
{
    PATM_VCC_DATA_PARMS pRetDp = NULL;
    atmssCell *pCell = (atmssCell *) pDataParms->pAtmBuffer->pDataBuf;
    UINT32 ulHdr = pCell->hdr;
    atmssVcState *pVc = (atmssVcState *) AtmVcTableFindKey(&m_VcTable, ulHdr>>4);
    UINT8 pti = (ulHdr >> 1) & 0x07;
    UINT16 usVci = (ulHdr >> 4) & 0xffff;

    if( pVc )
    {
        switch( pti )
        {
        case kAtmHdrPtiF5Segment:
        case kAtmHdrPtiF5End2End:
            pRetDp = AtmOamF5RxCell( pti, pDataParms );
            break;

        case kAtmHdrPtiNoCongSDU0:
        case kAtmHdrPtiNoCongSDU1:
        case kAtmHdrPtiCongSDU0:
        case kAtmHdrPtiCongSDU1:
            if( usVci == VCI_OAM_F4_SEGMENT || usVci == VCI_OAM_F4_END_TO_END )
                pRetDp = AtmOamF4RxCell( pDataParms );
            else
            {
                pRetDp = AtmAal5RxCell( pVc, pCell );
                (*pDataParms->pFnFreeDataParms) (pDataParms);
            }
            break;

        default:
            (*pDataParms->pFnFreeDataParms) (pDataParms);
            break;
        }
    }
    else
        (*pDataParms->pFnFreeDataParms) (pDataParms);

    return( pRetDp );
} // ProcessRxCell


//**************************************************************************
// Function Name: AtmOamF4RxCell
// Description  : Converts an OAM F4 ATM cell into an ATM_VCC_DATA_PARMS
//                packet with circuit type CT_OAM_F4_ANY
// Returns      : ATM_VCC_DATA_PARMS struct for an OAM F4 packet
//**************************************************************************
PATM_VCC_DATA_PARMS ATM_SOFT_SAR::AtmOamF4RxCell(PATM_VCC_DATA_PARMS pDataParms)
{
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;
    UINT8 *pBuf = pAb->pDataBuf;

    /* Remove the ATM header. */
    pAb->ulDataLen -= sizeof(UINT32);
    memcpy( pBuf, pBuf + sizeof(UINT32), pAb->ulDataLen );

    /* Fix up pDataParms. */
    pDataParms->ucCircuitType = CT_OAM_F4_ANY;
    pDataParms->baReceiveStatus = STS_SUCCESS;
    pDataParms->ucReserved[0] = pDataParms->ucReserved[1] = 0;

    return( pDataParms );
} // AtmOamF4RxCell


//**************************************************************************
// Function Name: AtmOamF5RxCell
// Description  : Converts an OAM F5 ATM cell into an ATM_VCC_DATA_PARMS
//                packet with circuit type CT_OAM_F5_SEGMENT or
//                CT_OAM_F5_END_TO_END.
// Returns      : ATM_VCC_DATA_PARMS struct for an OAM F5 packet
//**************************************************************************
PATM_VCC_DATA_PARMS ATM_SOFT_SAR::AtmOamF5RxCell( UINT8 pti,
    PATM_VCC_DATA_PARMS pDataParms )
{
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;
    UINT8 *pBuf = pAb->pDataBuf;

    /* Remove the ATM header. */
    pAb->ulDataLen -= sizeof(UINT32);
    memcpy( pBuf, pBuf + sizeof(UINT32), pAb->ulDataLen );

    /* Fix up pDataParms. */
    pDataParms->ucCircuitType = (pti == kAtmHdrPtiF5Segment)
        ? CT_OAM_F5_SEGMENT : CT_OAM_F5_END_TO_END;
    pDataParms->baReceiveStatus = STS_SUCCESS;
    pDataParms->ucReserved[0] = pDataParms->ucReserved[1] = 0;

    return( pDataParms );
} // AtmOamF5RxCell


//**************************************************************************
// Function Name: RegisterRxAppl
// Description  : Registers an ATM VCC application object to use the receive
//                software SAR.
// Returns      : STS_SUCCESS or error
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::RegisterRxAppl( ATM_VCC_ADDR *pAddr,
    ATM_VCC_APPL *pAppl )
{
    BCMATM_STATUS baStatus = STS_RESOURCE_ERROR;
    int i;
    struct vcc_appl_map *pEntry;
    struct vcc_appl_map *pEntryAvailable = NULL;

    for(i = 0, pEntry = m_vccApplMap; i < MAX_VCCS - MAX_RX_VCIDS; i++, pEntry++)
    {
        if( pEntry->pAppl == pAppl )
        {
            // pAppl is already registered.
            baStatus = STS_SUCCESS;
            break;
        }
        else
            if( pEntry->pAppl == NULL && pEntryAvailable == NULL )
                pEntryAvailable = pEntry;
    }

    if( baStatus == STS_RESOURCE_ERROR && pEntryAvailable )
    {
        UINT32 vpivci =
            ((UINT32) pAddr->usVpi << 20) | ((UINT32) pAddr->usVci << 4);

        pEntryAvailable->ulAtmHdr = vpivci;
        pEntryAvailable->pAppl = pAppl;

#if !defined(CONFIG_ATM_SOFTSAR)
        UINT32 newFlag;
        AtmVcTableFindOrAddKey(&m_VcTable, vpivci >> 4, &newFlag);
#endif

        baStatus = STS_SUCCESS;
    }

    return( baStatus );
} // RegisterRxAppl


//**************************************************************************
// Function Name: UnregisterRxAppl
// Description  : Unregisters an ATM VCC application object from using the
//                receive software SAR.
// Returns      : STS_SUCCESS or error
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::UnregisterRxAppl( ATM_VCC_APPL *pAppl )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;
    int i;
    struct vcc_appl_map *pEntry;

    for(i = 0, pEntry = m_vccApplMap; i < MAX_VCCS - MAX_RX_VCIDS; i++, pEntry++)
    {
        if( pEntry->pAppl == pAppl )
        {
            baStatus = STS_SUCCESS;
            pEntry->pAppl = NULL;
            pEntry->ulAtmHdr = 0;
            break;
        }
    }

    return( baStatus );
} // UnregisterRxAppl
#endif

#if 0 /* DEBUG */
//**************************************************************************
// Function Name: DebugDumpInfo
// Description  : Dump softsar information.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::DebugDumpInfo( void )
{
    atmssDllHeader *p1, *p2;
    atmssVcState  *pVc;

    for( p1 = m_dlistAllVc.Next();
         m_dlistAllVc.Valid( p1 );
         p1 = p1->Next() )
    {
        pVc = (atmssVcState *) p1->GetParam();

        (*m_pOsServices->pfnPrintf)("SSDBG: VC 0x%8.8lx:\n",
            pVc->vcParams.vpivci);

        p2 = m_dlistTxVcUbrWaiting.Next();
        if( m_dlistTxVcUbrWaiting.Valid( p2 ) )
        {
            (*m_pOsServices->pfnPrintf)("       is on "
                "m_dlistTxVcUbrWaiting list\n", pVc->vcParams.vpivci);
        }

        p2 = pVc->dlistAllVc.Next();
        if( pVc->dlistAllVc.Valid( p2 ) )
        {
            (*m_pOsServices->pfnPrintf)("       is on VC "
                "dlistAllVc list\n", pVc->vcParams.vpivci);
        }

        p2 = pVc->dlistTxVcWaiting.Next();
        if( pVc->dlistTxVcWaiting.Valid( p2 ) )
        {
            (*m_pOsServices->pfnPrintf)("       is on VC "
                "dlistTxVcWaiting list\n", pVc->vcParams.vpivci);
        }

//      p2 = pVc->aal5State.dlistTxWaiting.Next();
//      if( pVc->aal5State.dlistTxWaiting.Valid( p2 ) )
//      {
//          (*m_pOsServices->pfnPrintf)("       is on VC aal5State."
//              "dlistTxWaiting list\n", pVc->vcParams.vpivci);
//      }

        p2 = m_dlistTxVcUbrWaiting.Next();
        if( m_dlistTxVcUbrWaiting.Valid( p2 ) )
        {
            (*m_pOsServices->pfnPrintf)("       pTxPacket=0x%8.8lx\n",
                (UINT32) pVc->aal5State.pTxPacket);
            (*m_pOsServices->pfnPrintf)("       pTxCellBuffer=0x%8.8lx\n",
                (UINT32) pVc->aal5State.pTxCellBuffer);
            (*m_pOsServices->pfnPrintf)("       pTxCellData=0x%8.8lx\n",
                (UINT32) pVc->aal5State.pTxCellData);
            (*m_pOsServices->pfnPrintf)("       pTxCellDataEnd=0x%8.8lx\n",
                (UINT32) pVc->aal5State.pTxCellDataEnd);
            (*m_pOsServices->pfnPrintf)("       txLength=0x%8.8lx\n",
                (UINT32) pVc->aal5State.txLength);
            (*m_pOsServices->pfnPrintf)("       bTxPadNext=0x%8.8lx\n",
                (UINT32) pVc->aal5State.bTxPadNext);
        }
        (*m_pOsServices->pfnPrintf)("\n");
    }
}
#endif


//**************************************************************************
// ATM Layer Member Functions
//**************************************************************************


//**************************************************************************
// Function Name: AtmLayerInit
// Description  : Initialize ATM layer.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AtmLayerInit( void )
{
    BCMATM_STATUS baStatus = STS_PARAMETER_ERROR;

    g_pfnSsPrintf = m_pOsServices->pfnPrintf;

    m_lastTxCellTime = m_lastIdleTimerTime = AtmLayerGetCpuTick();
    m_txIdleTimeOut = TIME_SEC_TO_ATM(10);    // 10 seconds
    m_MaxDiff = TIME_SEC_TO_ATM(12);
    m_MinDiff = -TIME_SEC_TO_ATM(12);
    m_dlistAllVc.Init();
    m_dlistTxVcWaiting.Init();
    m_dlistTxVcUbrWaiting.Init();

    if( AtmVcTableInit(&m_VcTable, kAtmVcTableSize, kAtmVcTableSize-1) )
        baStatus = STS_SUCCESS;

    return( baStatus );
} // AtmLayerInit


//**************************************************************************
// Function Name: AtmLayerVcAllocate
// Description  : Allocates resources for a VCC.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AtmLayerVcAllocate( UINT32 vpivci,
    UINT32 ulTxPoolSize, UINT32 ulNumPriorities, atmssVcState **ppVc,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 newFlag;
    UINT32 ulSize = ulTxPoolSize * ulNumPriorities;

    atmssVcState *pVc = (atmssVcState *)
        AtmVcTableFindOrAddKey(&m_VcTable, vpivci, &newFlag);

    if( pVc && newFlag == TRUE )
    {
        if( pVc->txFrameList.AllocPool( m_pOsServices, ulSize ) &&
            pVc->txBufferList.AllocPool( m_pOsServices, ulSize * 2 ) )
        {

            atmssVcParams *pVcParams = &pVc->vcParams;

            pVcParams->vpivci = vpivci;
            pVcParams->clp = 0;
            pVcParams->uui = 0;
            pVcParams->cpi = 0;

            pVc->dlistAllVc.SetParam( pVc );
            pVc->dlistTxVcWaiting.SetParam( pVc );

            pVc->aal5State.dlistTxWaitingMaxSize = ulTxPoolSize;

            AtmLayerSetShapingValues( &pVcParams->txTrafficParams, pEntry );

            *ppVc = pVc;
            StatusHandler( kAtmStatVcCreated, pVcParams->vpivci, 
                pVcParams->vpivci, kAtmAal5, pVcParams->txTrafficParams.tPCR );
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }
    else
        baStatus = (pVc == NULL) ? STS_RESOURCE_ERROR : STS_IN_USE;

    return( baStatus );
} // AtmLayerVcAllocate


//**************************************************************************
// Function Name: AtmLayerSetShapingValues
// Description  : Sets shaping values.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerSetShapingValues( atmssTrafficParams *pTxParams,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    pTxParams->tPCR = pTxParams->tSCR = pTxParams->BT =  0;
    switch( pEntry->ulTrafficDescrType )
    {
    case TDT_ATM_NO_CLP_NO_SCR:
    case TDT_ATM_NO_CLP_NO_SCR_CDVT:
    case TDT_ATM_CLP_TRANSPARENT_NO_SCR:
    case TDT_ATM_NO_CLP_TAGGING_NO_SCR:
        if( pEntry->ulTrafficDescrParm1 > 0 )
            pTxParams->tPCR = TIME_SEC_TO_ATM(1) / pEntry->ulTrafficDescrParm1;
        switch( pEntry->ulServiceCategory )
        {
        case SC_UBR:
            pTxParams->ServiceType = kAtmSrvcUBR;
            pTxParams->Priority = kPriorityUbr;
            break;

        case SC_CBR:
            pTxParams->ServiceType = kAtmSrvcCBR;
            pTxParams->Priority = kPriorityCbr;
            break;

        default:
            break;
        }
        break;

    case TDT_ATM_NO_CLP_SCR:
    case TDT_ATM_NO_CLP_SCR_CDVT:
    case TDT_ATM_CLP_TRANSPARENT_SCR:
        if( pEntry->ulTrafficDescrParm1 > 0 )
            pTxParams->tPCR = TIME_SEC_TO_ATM(1) / pEntry->ulTrafficDescrParm1;
        if( pEntry->ulTrafficDescrParm2 > 0 )
            pTxParams->tSCR = TIME_SEC_TO_ATM(1) / pEntry->ulTrafficDescrParm2;
        if( pEntry->ulTrafficDescrParm3 > 0 )
            pTxParams->BT = -1 * ((pEntry->ulTrafficDescrParm3 - 1) *
                (pTxParams->tSCR - pTxParams->tPCR));

        // If the burst tolerance is 0, force PCR to SCR because the VCC should
        // shape at the SCR rate.
        if( pTxParams->BT == 0 )
            pTxParams->tPCR = pTxParams->tSCR;

        switch( pEntry->ulServiceCategory )
        {
        case SC_RT_VBR:
            pTxParams->ServiceType = kAtmSrvcVBR;
            pTxParams->Priority = kPriorityRtVbr;
            break;

        case SC_NRT_VBR:
            pTxParams->ServiceType = kAtmSrvcVBR;
            pTxParams->Priority = kPriorityNrtVbr;
            break;

        default:
            break;
        }
        break;

    //case TDT_ATM_NO_TRAFFIC_DESCRIPTOR:
    default:
        pTxParams->ServiceType = kAtmSrvcUBRNoPCR;
        pTxParams->Priority = kPriorityUbr;
        break;
    }
}


//**************************************************************************
// Function Name: AtmLayerVcActivate
// Description  : Makes a VCC ready to use.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AtmLayerVcActivate( atmssVcState *pVc )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    pVc->dlistTxVcWaiting.UnlinkEntry();
    pVc->dlistAllVc.UnlinkEntry();
    AtmAal5VcInit(pVc);
    m_dlistAllVc.InsertTail( &pVc->dlistAllVc );

    pVc->txPcrLCS = m_timeCurrent;
    pVc->txScrLCS = m_timeCurrent;

    StatusHandler( kAtmStatVcStarted, pVc->vcParams.vpivci );

    return( baStatus );
} // AtmLayerVcActivate


//**************************************************************************
// Function Name: AtmLayerVcDeactivate
// Description  : Deactivates a VCC.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerVcDeactivate( atmssVcState *pVc )
{
    if( pVc->dlistAllVc.EntryLinked() )
    {
        pVc->dlistAllVc.Remove();
        pVc->dlistAllVc.UnlinkEntry();
    }

    if( pVc->dlistTxVcWaiting.EntryLinked() )
    {
        pVc->dlistTxVcWaiting.Remove();
        pVc->dlistTxVcWaiting.UnlinkEntry();
    }

    StatusHandler( kAtmStatVcStopped, pVc->vcParams.vpivci );
} // AtmLayerVcDeactivate


//**************************************************************************
// Function Name: AtmLayerVcFree
// Description  : Removes a VCC from the hash table.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerVcFree( atmssVcState *pVc )
{
    pVc->txFrameList.FreePool( m_pOsServices );
    pVc->txBufferList.FreePool( m_pOsServices );
    AtmVcTableDelKey(&m_VcTable, pVc->vcParams.vpivci);
    StatusHandler( kAtmStatVcDeleted, pVc->vcParams.vpivci );
} // AtmLayerVcFree


//**************************************************************************
// Function Name: AtmLayerQueueFrame
// Description  : Queues a packet for transmission over ADSL.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerQueueFrame( atmssVcState *pVc, atmssFrame *pFrame,
    UINT32 ulPri )
{
    if( pVc->dlistTxVcWaiting.EntryLinked() == 0 )
    {
        if (kAtmSrvcUBRNoPCR == pVc->vcParams.txTrafficParams.ServiceType)
            m_dlistTxVcUbrWaiting.InsertTail(&pVc->dlistTxVcWaiting);
        else
        {
            if( TimeDiff(m_timeCurrent, pVc->txPcrLCS) >
                pVc->vcParams.txTrafficParams.tPCR )
            {
                // There has been a delay of at least one peak cell time since
                // the last packet was sent.  Update the "Last Cell Sent" time
                // to the current time.
                pVc->txPcrLCS = m_timeCurrent;
                pVc->txScrLCS = m_timeCurrent;
            }
            m_dlistTxVcWaiting.InsertTail(&pVc->dlistTxVcWaiting);
        }
    }

    // Queue the packet on the appropriate priority waiting queue.
    pVc->aal5State.dlistTxWaiting[ulPri].InsertTail(pFrame->GetLinkFieldAddress());
    pVc->aal5State.dlistTxWaitingCount[ulPri]++;
    if( ulPri > (UINT32) pVc->vcParams.txTrafficParams.CurrHighSubPri )
        pVc->vcParams.txTrafficParams.CurrHighSubPri = ulPri;
} // AtmLayerQueueFrame


//**************************************************************************
// Function Name: AtmLayerGetNextTxCell
// Description  : Creates a cell to be transmitted from one of the queued
//                packets.
// Returns      : STS_SUCCESS or error.
//**************************************************************************
BCMATM_STATUS ATM_SOFT_SAR::AtmLayerGetNextTxCell( atmssCell *pCell,
    UINT32 cellType )
{
    BCMATM_STATUS baStatus = STS_NOT_FOUND;
 
    if( m_connectRate && m_cellsToFill > 0 )
    {
        atmssDllHeader *p;
        atmssVcState  *pVc, *pVc0;
        atmssTrafficParams *pTxParams;
        long tmPcr, tmScr, tDiffMax, tDiffPri, tDiffSubPri;
        UINT32 haveShapedPkt = FALSE;

        if( m_cellHead )
        {
            // OAM cells.
            PATM_VCC_DATA_PARMS pDataParms = m_cellHead;
            PATM_BUFFER pAb = pDataParms->pAtmBuffer;
            UINT8 *pData;

            if( m_cellHead->pApplicationLink == NULL )
                m_cellHead = m_cellTail = NULL;
            else
                m_cellHead = m_cellHead->pApplicationLink;
            pDataParms->pApplicationLink = NULL;

            pCell->hdr = *(atmssCellHeader *) pAb->pDataBuf;
            if( cellType == CELL_TYPE_ADSL )
                pData = (UINT8 *) &pCell->adslCell.data;
            else
                pData = (UINT8 *) &pCell->atmCell.data;
            memcpy( pData, pAb->pDataBuf + sizeof(atmssCellHeader),
                kAtmCellDataSize );

            (*pDataParms->pFnFreeDataParms) (pDataParms);

            haveShapedPkt = TRUE;
            baStatus = STS_SUCCESS;
        }
        else
        {
            m_lastTxCellTime = m_timeCurrent;
            if (!m_atmssConnected)
            {
                m_atmssConnected = TRUE;
                StatusHandler( kAtmStatConnected );
            }

            for( ; ; )
            {
                // Find the "best" VC to send cell from.
                pVc0  = NULL;
                tDiffMax = -1;
                tDiffPri = -1;
                tDiffSubPri = -1;

                // First look for a shaped VC.
                p = m_dlistTxVcWaiting.Next();
                while( m_dlistTxVcWaiting.Valid( p ) )
                {
                    haveShapedPkt = TRUE;
                    pVc = (atmssVcState *) p->GetParam();
                    pTxParams = &pVc->vcParams.txTrafficParams;

                    tmPcr = TimeDiff(m_timeCurrent, pVc->txPcrLCS);
                    if( pTxParams->BT != 0 )
                        tmScr = TimeDiff(m_timeCurrent, pVc->txScrLCS);
                    else
                        tmScr = 0;
                    if( tmPcr >= pTxParams->tPCR && tmScr >= pTxParams->BT )
                    {
                        // A cell can be sent on this VC based on its shaping
                        // parameters.  If there is more than one VC that can
                        // send a cell, the VC with the highest priority is
                        // used.  For VCs with the same priority, the VC that
                        // is farthest past the time it was supposed to send
                        // the cell is used.
                        if( pTxParams->Priority > tDiffPri ||
                            (pTxParams->Priority == tDiffPri &&
                             pTxParams->CurrHighSubPri > tDiffSubPri) ||
                            (pTxParams->Priority == tDiffPri &&
                             pTxParams->CurrHighSubPri == tDiffSubPri &&
                             tmPcr - pTxParams->tPCR > tDiffMax) )
                        {
                            tDiffMax = tmPcr - pTxParams->tPCR;
                            tDiffPri = pTxParams->Priority;
                            tDiffSubPri = pTxParams->CurrHighSubPri;
                            pVc0 = pVc;
                        }
                    }
                    p = p->Next();
                }

                if (NULL != pVc0)
                {
                    // Found a shaped VC.
                    pVc = pVc0;
                    pTxParams = &pVc->vcParams.txTrafficParams;
                    if( AtmAal5TxCell( pVc, pCell, cellType ) )
                    {
                        pVc->dlistTxVcWaiting.Remove();
                        m_dlistTxVcWaiting.InsertTail( &pVc->dlistTxVcWaiting );
                        pVc->txPcrLCS += pTxParams->tPCR;
                        pVc->txScrLCS += pTxParams->tSCR;
                        baStatus = STS_SUCCESS;
                    }
                    else
                    {
                        pVc->dlistTxVcWaiting.Remove();
                        pVc->dlistTxVcWaiting.UnlinkEntry();
                        continue;
                    }
                }
                else
                {
                    // A shaped VC was not found.  Look for a non-shaped VC.
                    pVc0  = NULL;
                    tDiffSubPri = -1;

                    p = m_dlistTxVcUbrWaiting.Next();
                    while( m_dlistTxVcUbrWaiting.Valid( p ) )
                    {
                        pVc = (atmssVcState *) p->GetParam();
                        pTxParams = &pVc->vcParams.txTrafficParams;

                        if( pTxParams->CurrHighSubPri > tDiffSubPri )
                        {
                            tDiffSubPri = pTxParams->CurrHighSubPri;
                            pVc0 = pVc;
                        }
                        p = p->Next();
                    }

                    if (NULL != pVc0)
                    {
                        // Found a non-shaped VC.
                        pVc = pVc0;
                        if( AtmAal5TxCell( pVc, pCell, cellType ) )
                        {
                            pVc->dlistTxVcWaiting.Remove();
                            m_dlistTxVcUbrWaiting.InsertTail(
                                &pVc->dlistTxVcWaiting);
                            baStatus = STS_SUCCESS;
                        }
                        else
                        {
                            pVc->dlistTxVcWaiting.Remove();
                            pVc->dlistTxVcWaiting.UnlinkEntry();
                            continue;
                        }
                    }
                }

                break;
            }

            if( NULL == pVc0 && haveShapedPkt )
            {
                // There is at least one packet on a shaped VC to send but the
                // shaping parameters do not allow a cell from a packet to
                // be sent at this time.  Therefore, send an idle cell in this
                // time slot.
                UINT8 *pData;
                pCell->hdr = 0;
                if( cellType == CELL_TYPE_ADSL )
                    pData = (UINT8 *) &pCell->adslCell.data;
                else
                    pData = (UINT8 *) &pCell->atmCell.data;
                memset( pData, 0x6a, kAtmCellDataSize );
                baStatus = STS_SUCCESS;
            }
        }

        // If there is at least one packet on a shaped VC, the number of
        // cells (data cells and idle cells) are counted and the current
        // time is updated by the time it takes to send one cell.
        if( baStatus == STS_SUCCESS )
        {
            if( haveShapedPkt )
            {
                m_cellsToFill--;
                m_timeCurrent += m_cellTicks;
            }
        }
        else
            m_cellsToFill = 0;
    }

    return( baStatus );
} // AtmLayerGetNextTxCell


//**************************************************************************
// Function Name: AtmLayerSendComplete
// Description  : Returns a transmitted packet to the caller.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerSendComplete( atmssVcState *pVc, atmssFrame *pFrame )
{
    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);

    PATM_VCC_DATA_PARMS pDp = (PATM_VCC_DATA_PARMS) pFrame->GetUserParam();
    (*pDp->pFnFreeDataParms) (pDp);

    atmssFrameBuffer *pBuf;
    while( (pBuf = pFrame->DequeBuffer()) != NULL )
        pVc->txBufferList.PutFrameBuffer( pBuf );
    pVc->txFrameList.PutFrame( pFrame );

    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);
} // AtmLayerSendComplete


//**************************************************************************
// Function Name: AtmLayerTimer
// Description  : Checks ATM timeout values.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmLayerTimer( UINT32 ulIntervalMs )
{
    long diff;

    m_timeCurrent = AtmLayerGetCpuTick();

    // Determine the number of cells that can be sent in the time interval
    // parameter, ulIntervalMs.  Add 2 ms for latency.  This calculated value
    // is the number of cells (data and idle) that are allowed to be put into
    // the ADSL descriptor table and transmitted.
    m_cellsToFill += (((ulIntervalMs + 2) * m_connectRate) / 424000) + 1;
    if( m_cellsToFill > ATM_ADSL_TX_DESCR_TBL_SIZE )
        m_cellsToFill = ATM_ADSL_TX_DESCR_TBL_SIZE;

    diff = TimeDiff(m_timeCurrent, m_lastIdleTimerTime);
    if (diff >= m_txIdleTimeOut )
    {
        atmssDllHeader *p;
        atmssVcState *pVc;

        p = m_dlistAllVc.Next();
        while( m_dlistAllVc.Valid( p ) )
        {
            pVc = (atmssVcState *) p->GetParam();

            // If cells were not sent or received on a VC for a long time
            // system current time gets father and farther away from
            // scheduled send/rcv times on that VC. The danger is that the
            // time difference can get so far that it changes sign and then
            // no cells will be sent or received. The code below will
            // update scheduled times if they idle out.
            if (TimeDiff(m_timeCurrent, pVc->txPcrLCS) > m_txIdleTimeOut)
                pVc->txPcrLCS = m_timeCurrent;
            if (TimeDiff(m_timeCurrent, pVc->txScrLCS) > m_txIdleTimeOut)
                pVc->txScrLCS = m_timeCurrent;
            p = p->Next();
        }

        m_lastIdleTimerTime = m_timeCurrent;
    }
} // AtmLayerTimer


//**************************************************************************
// Function Name: TxFlush
// Description  : Flushes transmit packets on each VCC.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::TxFlush( void )
{
    atmssDllHeader *p;
    atmssVcState *pVc;

    p = m_dlistTxVcWaiting.Next();
    while( m_dlistTxVcWaiting.Valid( p ) )
    {
        pVc = (atmssVcState *) p->GetParam();
        AtmAal5VcTxFlush( pVc );
        pVc->dlistTxVcWaiting.Remove();
        pVc->dlistTxVcWaiting.UnlinkEntry();
        p = m_dlistTxVcWaiting.Next();
    }

    p = m_dlistTxVcUbrWaiting.Next();
    while( m_dlistTxVcUbrWaiting.Valid( p ) )
    {
        pVc = (atmssVcState *) p->GetParam();
        AtmAal5VcTxFlush( pVc );
        pVc->dlistTxVcWaiting.Remove();
        pVc->dlistTxVcWaiting.UnlinkEntry();
        p = m_dlistTxVcUbrWaiting.Next();
    }
} // TxFlush


//**************************************************************************
// AAL5 Layer Member Functions
//**************************************************************************


//**************************************************************************
// Function Name: AtmAal5VcInit
// Description  : Initializes AAL5 specific values.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5VcInit( atmssVcState *pVc )
{
    for( int i = 0; i < MAX_TX_PRIORITIES; i++ )
    {
        pVc->aal5State.dlistTxWaiting[i].Init();
        pVc->aal5State.dlistTxWaitingCount[i] = 0;
    }
    pVc->aal5State.pTxPacket        = NULL;
    pVc->aal5State.pTxCellBuffer    = NULL;
    pVc->aal5State.pTxCellData      = NULL;
    pVc->aal5State.pTxCellDataEnd   = NULL;
    pVc->aal5State.txCrc            = AAL5_CRC_INIT;
    pVc->aal5State.txLength         = 0;
    pVc->aal5State.bTxPadNext       = FALSE;
#if defined(CONFIG_ATM_RX_SOFTSAR)
    pVc->aal5State.pRxPacket        = NULL;
    pVc->aal5State.rxCrc            = AAL5_CRC_INIT;
#endif
} // AtmAal5VcInit


//**************************************************************************
// Function Name: AtmAal5TxCell
// Description  : Extracts an ATM cell from a packet.
// Returns      : TRUE - Cell created.  FALSE - Cell not created.
//**************************************************************************
UINT32 ATM_SOFT_SAR::AtmAal5TxCell( atmssVcState *pVc, atmssCell *pCell,
    UINT32 cellType )
{
    UINT8 *pData, *pDataEnd, b;
    UINT32 len;

    if( cellType == CELL_TYPE_ADSL )
    {
        pData = (UINT8 *) &pCell->adslCell.data;
    }
    else
        pData = (UINT8 *) &pCell->atmCell.data;
    pDataEnd = pData + kAtmCellDataSize;
    if (pVc->aal5State.bTxPadNext)
    {
        pVc->aal5State.bTxPadNext = FALSE;
        AtmAal5TxCompletePDU( pVc, pData,pDataEnd, pCell, cellType );
        return TRUE;
    }
    do
    {
        if( pVc->aal5State.pTxCellData == pVc->aal5State.pTxCellDataEnd )
        {
            if( pVc->aal5State.txLength == ATM_AAL5_MAX_DATA_LEN )
            {
                AtmAal5TxCompletePDU( pVc, pData, pDataEnd, pCell, cellType );
                pVc->aal5State.pTxCellDataEnd = (UINT8 *)
                    pVc->aal5State.pTxCellBuffer->GetAddress() +
                    pVc->aal5State.pTxCellBuffer->GetLength();
                pVc->aal5State.txLength = pVc->aal5State.pTxCellDataEnd -
                    pVc->aal5State.pTxCellData;
                if( pVc->aal5State.txLength > ATM_AAL5_MAX_DATA_LEN )
                {
                    pVc->aal5State.txLength = ATM_AAL5_MAX_DATA_LEN;
                    pVc->aal5State.pTxCellDataEnd = pVc->aal5State.pTxCellData +
                        pVc->aal5State.txLength;
                }
                return TRUE;
            }
            if( NULL != pVc->aal5State.pTxCellBuffer )
            {
                pVc->aal5State.pTxCellBuffer =
                    pVc->aal5State.pTxCellBuffer->GetNextBuffer();
            }
            if( NULL == pVc->aal5State.pTxCellBuffer )
            {
                if( NULL != pVc->aal5State.pTxPacket )
                {
                    AtmLayerSendComplete( pVc, pVc->aal5State.pTxPacket );
                    AtmAal5TxCompletePDU(pVc, pData, pDataEnd, pCell, cellType);
                    pVc->aal5State.pTxPacket = NULL;
                    return TRUE;
                }
                AtmAal5TxNextSDU( pVc );
                if( NULL == pVc->aal5State.pTxCellBuffer )
                    return FALSE;
            }
            pVc->aal5State.pTxCellData =
                pVc->aal5State.pTxCellBuffer->GetAddress();
            len = pVc->aal5State.pTxCellBuffer->GetLength();
            pVc->aal5State.txLength += len;
            if (pVc->aal5State.txLength > ATM_AAL5_MAX_DATA_LEN)
            {
                len -= pVc->aal5State.txLength - ATM_AAL5_MAX_DATA_LEN;
                pVc->aal5State.txLength = ATM_AAL5_MAX_DATA_LEN;
            }
            pVc->aal5State.pTxCellDataEnd = pVc->aal5State.pTxCellData + len;
        }
        else
        {
            b = *pVc->aal5State.pTxCellData++;
            AtmAal5TxCellNextByte( pVc, pData, b );
            pData++;
        }
    } while (pData != pDataEnd);

    pCell->hdr = AtmAal5MakeCellHeader( pVc->vcParams.vpivci, kAtmHdrCfgNone,
        kAtmHdrPtiNoCongSDU0, pVc->vcParams.clp );

    return TRUE;
} // AtmAal5TxCell


//**************************************************************************
// Function Name: AtmAal5TxCompletePDU
// Description  : Adds AAL5 pad characters and AAL5 trailer.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5TxCompletePDU( atmssVcState *pVc, UINT8 *pData,
    UINT8 *pDataEnd, atmssCell *pCell, UINT32 cellType )
{
    UINT8 b;

    if( (pDataEnd - pData) < 8 )
    {
        while( pData != pDataEnd )
        {
            AtmAal5TxCellNextByte(pVc, pData, ATM_AAL5_PAD);
            pData++;
        }
        pVc->aal5State.bTxPadNext = TRUE;
        pCell->hdr = AtmAal5MakeCellHeader(pVc->vcParams.vpivci, kAtmHdrCfgNone,
            kAtmHdrPtiNoCongSDU0, pVc->vcParams.clp );
    }
    else
    {
        while( pData != (pDataEnd - 8) )
        {
            AtmAal5TxCellNextByte(pVc,pData,ATM_AAL5_PAD);
            pData++;
        }
        AtmAal5TxCellNextByte(pVc,pData, pVc->vcParams.uui);
        pData++;
        AtmAal5TxCellNextByte(pVc,pData, pVc->vcParams.cpi);
        pData++;
        b = (UINT8)(pVc->aal5State.txLength >> 8);
        AtmAal5TxCellNextByte(pVc,pData,b);
        pData++;
        b = (UINT8)(pVc->aal5State.txLength & 0xFF);
        AtmAal5TxCellNextByte(pVc,pData,b);
        pData++;
        pVc->aal5State.txCrc = AAL5_CRC_FINAL(pVc->aal5State.txCrc);
        *pData++ = (UINT8)(pVc->aal5State.txCrc >> 24);
        *pData++ = (UINT8)(pVc->aal5State.txCrc >> 16);
        *pData++ = (UINT8)(pVc->aal5State.txCrc >> 8);
        *pData++ = (UINT8)(pVc->aal5State.txCrc);
        pVc->aal5State.txLength = 0;
        pVc->aal5State.txCrc = AAL5_CRC_INIT;
        pCell->hdr = AtmAal5MakeCellHeader( pVc->vcParams.vpivci, kAtmHdrCfgNone,
            kAtmHdrPtiNoCongSDU1, pVc->vcParams.clp );
    }
} // AtmAal5TxCompletePDU


//**************************************************************************
// Function Name: AtmAal5TxNextSDU
// Description  : Unlinks the next atmssFrame element from a transmist list.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5TxNextSDU( atmssVcState *pVc )
{
    atmssDllHeader *p;

    pVc->aal5State.pTxCellBuffer = NULL;
    pVc->aal5State.pTxPacket = NULL;
    pVc->vcParams.txTrafficParams.CurrHighSubPri = 0;

    // Take the packet with the highest priority.
    for( int i = MAX_TX_PRIORITIES - 1; i >= 0; i-- )
    {
        p = pVc->aal5State.dlistTxWaiting[i].Next();
        if( pVc->aal5State.dlistTxWaiting[i].Valid( p ) )
        {
            p->Remove();
            pVc->aal5State.dlistTxWaitingCount[i]--;
            pVc->aal5State.pTxPacket = (atmssFrame *) p->GetParam();
            pVc->aal5State.pTxCellBuffer =
                pVc->aal5State.pTxPacket->GetFirstBuffer();
            pVc->vcParams.txTrafficParams.CurrHighSubPri = i;
            break;
        }
    }
} // AtmAal5TxNextSDU


//**************************************************************************
// Function Name: AtmAal5TxCellNextByte
// Description  : Adds a byte to a cell that is being created.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5TxCellNextByte(atmssVcState *pVc, UINT8 *pData, UINT8 b)
{
    *pData = b;
    pVc->aal5State.txCrc = Aal5UpdateCrc(pVc->aal5State.txCrc, b);
} // AtmAal5TxCellNextByte


//**************************************************************************
// Function Name: AtmAal5VcTxFlush
// Description  : Frees queued transmit packets.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5VcTxFlush( atmssVcState *pVc )
{
    atmssDllHeader *p;
    for( int i = 0; i < MAX_TX_PRIORITIES; i++ )
    {
        p = pVc->aal5State.dlistTxWaiting[i].Next();
        while( pVc->aal5State.dlistTxWaiting[i].Valid( p ) )
        {
            atmssDllHeader *p0 = p;
            p = p->Next();
            AtmLayerSendComplete( pVc, (atmssFrame *) p0->GetParam() );
        }
        pVc->aal5State.dlistTxWaiting[i].UnlinkEntry();
        pVc->aal5State.dlistTxWaiting[i].Init();
        pVc->aal5State.dlistTxWaitingCount[i] = 0;
    }
    if (NULL != pVc->aal5State.pTxPacket)
        AtmLayerSendComplete( pVc, pVc->aal5State.pTxPacket );
    pVc->aal5State.pTxPacket      = NULL;
    pVc->aal5State.pTxCellBuffer  = NULL;
    pVc->aal5State.pTxCellData    = NULL;
    pVc->aal5State.pTxCellDataEnd = NULL;
    pVc->aal5State.txCrc          = AAL5_CRC_INIT;
    pVc->aal5State.txLength       = 0;
    pVc->aal5State.bTxPadNext     = FALSE;
} // AtmAal5VcTxFlush

#if defined(CONFIG_ATM_RX_SOFTSAR)
//**************************************************************************
// Function Name: AtmAal5RxCell
// Description  : Reassembles a cell into an AAL5 packet.
// Returns      : None.
//**************************************************************************
PATM_VCC_DATA_PARMS ATM_SOFT_SAR::AtmAal5RxCell( atmssVcState *pVc,
    atmssCell *pCell )
{
    PATM_VCC_DATA_PARMS pRetDp = NULL;
    PATM_BUFFER pAb;
    UINT8 *pData, *pDataEnd;
    UINT32 padLen;
    UINT32 len;
    UINT32 pti = (pCell->hdr >> 1) & 0x7;
    atmssFrame *pFrm;
    atmssFrameBuffer *pFrmBuf;

    if( pVc->aal5State.pRxPacket == NULL )
    {
        pVc->aal5State.pRxPacket = m_rxFrameList.GetFrame();
        pVc->aal5State.rxCrc = AAL5_CRC_INIT;
    }

    if( (pFrm = pVc->aal5State.pRxPacket) != NULL &&
        (pFrmBuf = pFrm->GetFirstBuffer()) != NULL )
    {
        len = pFrmBuf->GetLength() + kAtmCellDataSize;
        pData = pFrmBuf->GetAddress() + pFrmBuf->GetLength();
        pDataEnd = pFrmBuf->GetAddress() + len;
        if( len < pFrmBuf->GetBufSize() )
        {
            memcpy( pData, pCell->atmCell.data, kAtmCellDataSize );
            pFrmBuf->SetLength( len );
#if 1 /* if 0 doesn't calculate CRC */
            UINT32 rxCrc = pVc->aal5State.rxCrc;
            do
            {
                rxCrc = Aal5UpdateCrc( rxCrc, *pData );
                pData++;
            } while( pData != pDataEnd );
            pVc->aal5State.rxCrc = rxCrc;
#endif
            if( pti == kAtmHdrPtiNoCongSDU1 || pti == kAtmHdrPtiCongSDU1 )
            {
                pRetDp = (PATM_VCC_DATA_PARMS) pFrm->GetUserParam();
                pAb = pRetDp->pAtmBuffer;
                pAb->ulDataLen = *(UINT16 *) (pDataEnd - 6);
                padLen = len - pAb->ulDataLen;

#if 1 /* if 0 doesn't check CRC */
                if( AAL5_GOOD_CRC == pVc->aal5State.rxCrc &&
                    padLen <= kAtmCellDataSize )
                {
                    pRetDp->baReceiveStatus = STS_SUCCESS;
                }
                else
                    pRetDp->baReceiveStatus = STS_PKTERR_AAL5_AAL0_CRC_ERROR;
#else
                pRetDp->baReceiveStatus = STS_SUCCESS;
#endif

                pVc->aal5State.pRxPacket = NULL;
            }
        }
        else
        {
            pRetDp = (PATM_VCC_DATA_PARMS) pFrm->GetUserParam();
            pRetDp->baReceiveStatus = STS_PKTERR_AAL5_AAL0_BIG_PKT_ERROR;
            pVc->aal5State.pRxPacket = NULL;
        }
    }

    return( pRetDp );
} // AtmAal5RxCell
#endif


//**************************************************************************
// Function Name: AtmAal5VcClose
// Description  : Returns resources for a VCC.
// Returns      : None.
//**************************************************************************
void ATM_SOFT_SAR::AtmAal5VcClose( atmssVcState *pVc )
{
    AtmAal5VcTxFlush (pVc);
    for( int i = 0; i < MAX_TX_PRIORITIES; i++ )
    {
        pVc->aal5State.dlistTxWaiting[i].UnlinkEntry();
        pVc->aal5State.dlistTxWaitingCount[i] = 0;
    }
#if defined(CONFIG_ATM_RX_SOFTSAR)
    if( pVc->aal5State.pRxPacket )
    {
        atmssFrame *pFrm = pVc->aal5State.pRxPacket;
        atmssFrameBuffer *pFrmBuf = pFrm->GetFirstBuffer();

        pFrmBuf->SetLength( 0 );
        m_rxFrameList.PutFrame( pFrm );
        pVc->aal5State.pRxPacket = NULL;
    }
#endif
} // AtmAal5VcClose


UINT32 ATM_SOFT_SAR::ms_AtmCrc32Table[256] =
{
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005, 
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, 
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75, 
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD, 
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D, 
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95, 
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072, 
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02, 
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA, 
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692, 
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A, 
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A, 
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB, 
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53, 
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B, 
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B, 
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3, 
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3, 
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24, 
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC, 
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C, 
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C, 
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};


//**************************************************************************
// atmssFramePool class 
//**************************************************************************


//**************************************************************************
// Function Name: AllocPool
// Description  : Allocates memory for a pools of atmssFrame elements
//                and links them together.
// Returns      : TRUE = success, FALSE = not so successful
//**************************************************************************
UINT32 atmssFramePool::AllocPool( ATM_OS_SERVICES *pOsServices, UINT32 frameNum )
{
    UINT32 ret = FALSE;

    m_pool = (atmssFrame *)
        (*pOsServices->pfnAlloc) (sizeof(atmssFrame) * frameNum);

    if( m_pool )
    {
        atmssFrame *p;
        UINT32 i;

        for( i = 0, p = m_pool; i < frameNum; i++, p++ )
        {
            p->m_next = p + 1;
            p->m_DllHeader.SetParam( p );
        }

        m_pool[frameNum - 1].m_next = NULL;

        m_first = m_pool;
        ret = TRUE;
    }

    return( ret );
} // AllocPool


//**************************************************************************
// Function Name: FreePool
// Description  : Frees the memory allocated for the frame pool.
// Returns      : None.
//**************************************************************************
void atmssFramePool::FreePool( ATM_OS_SERVICES *pOsServices )
{
    if( m_pool )
    {
        (*pOsServices->pfnFree) (m_pool);
        m_pool = m_first = NULL;
    }
} // FreePool


//**************************************************************************
// atmssFrame class 
//**************************************************************************


//**************************************************************************
// Function Name: Init
// Description  : Initializes member data.
// Returns      : None.
//**************************************************************************
void atmssFrame::Init( void )
{
    m_next = NULL;
    m_userParam = 0;
    m_totalLength = 0;
    m_bufCnt = 0;
    m_head = NULL;
    m_tail = NULL;
    m_DllHeader.UnlinkEntry();
} // Init


//**************************************************************************
// Function Name: DequeBuffer
// Description  : Removes the frame buffer from the head of the frame.
// Returns      : None.
//**************************************************************************
atmssFrameBuffer *atmssFrame::DequeBuffer( void )
{
    atmssFrameBuffer *pBuf = m_head;

    if( pBuf )
    {
        m_totalLength -= pBuf->GetLength();
        m_bufCnt--;
        BufRemove();
    }

    return( pBuf );
} // DequeBuffer


//**************************************************************************
// Function Name: EnqueBufferAtFront
// Description  : Adds the frame buffer from the head of the frame.
// Returns      : None.
//**************************************************************************
void atmssFrame::EnqueBufferAtFront( atmssFrameBuffer *b )
{
    BufAddFirst( b );
    m_totalLength += b->GetLength();
    m_bufCnt++;
} // EnqueBufferAtFront


//**************************************************************************
// Function Name: EnqueBufferAtBack
// Description  : Adds the frame buffer from the end of the frame.
// Returns      : None.
//**************************************************************************
void atmssFrame::EnqueBufferAtBack( atmssFrameBuffer *b )
{
    BufAddLast( b );
    m_totalLength += b->GetLength();
    m_bufCnt++;
} // EnqueBufferAtBack


//**************************************************************************
// Function Name: EnqueFrameAtFront
// Description  : Adds all of the frame buffers from the specified frame
//                to the head of this frame.
// Returns      : None.
//**************************************************************************
void atmssFrame::EnqueFrameAtFront( atmssFrame *f )
{
    BufPrepend( f );
    m_totalLength += f->m_totalLength;
    m_bufCnt += f->m_bufCnt;
} // EnqueFrameAtFront


//**************************************************************************
// atmssFrameBufferPool class 
//**************************************************************************


//**************************************************************************
// Function Name: AllocPool
// Description  : Allocates memory for a pools of atmssFrameBuffer elements
//                and links them together.
// Returns      : TRUE = success, FALSE = not so successful
//**************************************************************************
UINT32 atmssFrameBufferPool::AllocPool( ATM_OS_SERVICES *pOsServices,
     UINT32 frameBufNum )
{
    UINT32 ret = FALSE;

    m_pool = (atmssFrameBuffer *)
        (*pOsServices->pfnAlloc) (sizeof(atmssFrameBuffer) * frameBufNum);

    if( m_pool )
    {
        atmssFrameBuffer *p;
        UINT32 i;

        for( i = 0, p = m_pool; i < frameBufNum; i++, p++ )
            p->m_next = p + 1;

        m_pool[frameBufNum - 1].m_next = NULL;

        m_first = m_pool;
        ret = TRUE;
    }

    return( ret );
} // AllocPool


//**************************************************************************
// Function Name: FreePool
// Description  : Frees the memory allocated for the frame buffer pool.
// Returns      : None.
//**************************************************************************
void atmssFrameBufferPool::FreePool( ATM_OS_SERVICES *pOsServices )
{
    if( m_pool )
    {
        (*pOsServices->pfnFree) (m_pool);
        m_pool = m_first = NULL;
    }
} // FreePool


//**************************************************************************
// atmssHashTable class 
//**************************************************************************


//**************************************************************************
// Function Name: Init
// Description  : Initialize the hash table.
// Returns      : None.
//**************************************************************************
UINT32 atmssHashTable::Init( int items, int itemLength, int hashParam )
{
    UINT32 ret = FALSE;
    atmssHashTableItemHeader *itemHdr;

    if( itemLength > (int) sizeof(atmssHashTableItemHeader) )
    {
        ret = TRUE;
        m_hashParam = hashParam;
        m_hashParamRecip = 0x8000 / hashParam;
        m_items = items;
        m_itemLength = itemLength;

        // Define all the slots as empty and build a list of them
        m_hashTblEmptyList = NULL;
        for( int i = 0; i < m_items; i++ )
        {
            itemHdr = HashTblEntry(i);
            itemHdr->head = HASH_TBL_NULL;
            itemHdr->link = m_hashTblEmptyList;
            m_hashTblEmptyList = itemHdr;
        }
    }

    return( ret );
} // Init


//**************************************************************************
// Function Name: FindKey
// Description  : Find an entry for the specified hash table key.
// Returns      : Pointer to the entry or NULL
//**************************************************************************
void *atmssHashTable::FindKey( UINT32 key )
{
    UINT16 n = HashTblEntry(GetHashIndex(key))->head;

    while ((n != HASH_TBL_NULL) && (HashTblEntry(n)->key < key))
        n = HashTblEntry(n)->next;

    if( n != HASH_TBL_NULL && HashTblEntry(n)->key != key )
        n = HASH_TBL_NULL;

    return( (HASH_TBL_NULL != n) ? hashTblItemData(HashTblEntry(n)) : NULL );
} // FindKey


//**************************************************************************
// Function Name: FindOrAddKey
// Description  : Find or add an entry for the specified hash table key.
// Returns      : Pointer to the entry or NULL
//**************************************************************************
void *atmssHashTable::FindOrAddKey( UINT32 key, UINT32 *pNewFlag )
{
    UINT16 n, next, *prev;
    atmssHashTableItemHeader *p;

    n = GetHashIndex(key);
    prev = &HashTblEntry(n)->head;
    next = *prev;
    while( next != HASH_TBL_NULL && HashTblEntry(next)->key < key )
    {
        prev = &HashTblEntry(next)->next;
        next = *prev;
    }

    if( next == HASH_TBL_NULL || HashTblEntry(next)->key != key )
    {
        *pNewFlag = TRUE;
        p = m_hashTblEmptyList;
        if( NULL != p )
        {
            m_hashTblEmptyList = m_hashTblEmptyList->link;
            n = ((UINT8 *) p - hashTblData()) / m_itemLength;
            p->next  = next;
            p->key   = key;
            *prev = n;
        }
        else
            return NULL;
    }
    else
    {
        *pNewFlag = FALSE;
        n = next;
    }

    return( (HASH_TBL_NULL != n) ? hashTblItemData(HashTblEntry(n)) : NULL );
} // FindOrAddKey


//**************************************************************************
// Function Name: DelKey
// Description  : Remove an entry for the specified hash table key.
// Returns      : None.
//**************************************************************************
void atmssHashTable::DelKey( UINT32 key )
{
    UINT16  next, *prev;

    next = GetHashIndex(key);
    prev = &HashTblEntry(next)->head;
    next = *prev;
    while( next != HASH_TBL_NULL && HashTblEntry(next)->key < key )
    {
        prev = &HashTblEntry(next)->next;
        next = *prev;
    }

    if( next != HASH_TBL_NULL && HashTblEntry(next)->key == key )
    {
        atmssHashTableItemHeader *p = HashTblEntry(next);

        *prev = p->next;
        p->link = m_hashTblEmptyList;
        m_hashTblEmptyList = p;
    }
} // DelKey


//**************************************************************************
// Function Name: GetHashIndex
// Description  : 
// Returns      : None.
//**************************************************************************
UINT16 atmssHashTable::GetHashIndex( UINT32 key )
{
    UINT32 n;
    UINT32 x = (((key) ^ ((key) >> 16)) & 0xFFFF); // HKEY(key);

    n = x - (((x * m_hashParamRecip) >> 15) * m_hashParam);
    if( n >= (UINT32) m_items )
        n = 0;

    return( (UINT16) n );
} // GetHashKey

#endif

