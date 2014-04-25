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
// File Name  : AtmQueueFifo.cpp for BCM63xx
//
// Description: This file contains the implementation for the queue class.
//
// Updates    : 01/02/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"


#if !defined(CONFIG_ATM_SOFTSAR)
//**************************************************************************
// ATM_SHAPER Class
//**************************************************************************

UINT32 ATM_SHAPER::ms_ulSitUt = ADSL_SIT_UNIT_OF_TIME;

//**************************************************************************
// Function Name: ATM_SHAPER
// Description  : Constructor for shaper class.
// Returns      : None.
//**************************************************************************
ATM_SHAPER::ATM_SHAPER( void )
{
    m_pulSstCtrl = m_pulSstVbr = m_pulSstMcr = NULL;
} // ATM_SHAPER


//**************************************************************************
// Function Name: ~ATM_SHAPER
// Description  : Destructor for transmit buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_SHAPER::~ATM_SHAPER( void )
{
    if( m_pulSstCtrl )
    {
        *m_pulSstCtrl = SST_SHAPER_RESET;
        m_pulSstCtrl = NULL;
    }

    if( m_pulSstVbr )
    {
        *m_pulSstVbr = 0;
        m_pulSstVbr = NULL;
    }

    if( m_pulSstMcr )
    {
        *m_pulSstMcr = 0;
        m_pulSstMcr = NULL;
    }
} // ~ATM_SHAPER


//**************************************************************************
// Function Name: GetAdjustedPriority
// Description  : Calculates the final priority for a transmit queue.
// Returns      : Adjust priority number or 0xff to indicate an error
//**************************************************************************
UINT8 ATM_SHAPER::GetAdjustedPriority( UINT32 ulServiceCategory,
    UINT32 ulPriority )
{
    const UINT8 constMaxOther           = 3;
    const UINT8 constMaxCbr             = 3;
    const UINT8 constMaxRtVbr           = 3;
    const UINT8 constMaxNrtVbr          = 2;
    const UINT8 constMaxUbr             = 3;

    UINT8 ucReturnPri = 0xff;

    // The ulPriority parameter range is from 1 to 4.  The return priority range
    // is from 0 to 3.  The return priority for CBR and rtVBR is always 3.  The
    // return priority for nrtVBR is from 0 to 2.  The return priority for UBR
    // is from 0 to 3.
    switch( ulServiceCategory )
    {
    case SC_OTHER:
        if( ulPriority >= 1 && ulPriority <= constMaxOther )
            ucReturnPri = (UINT8) (ulPriority - 1);
        else
            ucReturnPri = constMaxOther;
        break;

    case SC_CBR:
        ucReturnPri = constMaxCbr;
        break;

    case SC_RT_VBR:
        ucReturnPri = constMaxRtVbr;
        break;

    case SC_NRT_VBR:
        if( ulPriority >= 1 && ulPriority <= constMaxNrtVbr )
            ucReturnPri = (UINT8) (ulPriority - 1);
        else
            ucReturnPri = constMaxNrtVbr;
        break;

    case SC_UBR:
        if( ulPriority >= 1 && ulPriority <= constMaxUbr )
            ucReturnPri = (UINT8) (ulPriority - 1);
        else
            ucReturnPri = constMaxUbr;
        break;
    }

    return( ucReturnPri );
} // GetAdjustedPriority


//**************************************************************************
// Function Name: Configure
// Description  : Configure an ATM Processor shaper.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_SHAPER::Configure( UINT8 ucShaperNum, UINT8 ucPriority,
    UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulShapingAlgorithm = 0;

    m_pulSstCtrl = AP_REGS->ulSstCtrl + ucShaperNum;
    m_pulSstVbr = AP_REGS->ulSstVbr + ucShaperNum;
    UINT32 ulPcr = 0;
    UINT32 ulScr = 0;
    UINT32 ulMbs = 1;
    UINT32 ulMcr = 0;

    // Determine shaper register values from the traffic descriptor table
    // parameter entry.
    switch( pEntry->ulTrafficDescrType )
    {
    case TDT_ATM_NO_TRAFFIC_DESCRIPTOR:
        switch( pEntry->ulServiceCategory )
        {
        case SC_UBR:
            ulShapingAlgorithm = SST_UBR_NO_PCR;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

    case TDT_ATM_NO_CLP_NO_SCR:
    case TDT_ATM_NO_CLP_NO_SCR_CDVT:
        ulPcr = pEntry->ulTrafficDescrParm1;
        switch( pEntry->ulServiceCategory )
        {
        case SC_UBR:
            ulShapingAlgorithm = SST_UBR_PCR;
            break;

        case SC_CBR:
            ulShapingAlgorithm = SST_CBR;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

    case TDT_ATM_NO_CLP_SCR:
    case TDT_ATM_NO_CLP_SCR_CDVT:
        ulPcr = pEntry->ulTrafficDescrParm1;
        ulScr = pEntry->ulTrafficDescrParm2;
        ulMbs = pEntry->ulTrafficDescrParm3;

        switch( pEntry->ulServiceCategory )
        {
        case SC_RT_VBR:
        case SC_NRT_VBR:
            ulShapingAlgorithm = SST_VBR_1;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96348)
    case TDT_ATM_CLP_NO_TAGGING_MCR:
        m_pulSstMcr = AP_REGS->ulSstMcr + ucShaperNum;
        ulPcr = pEntry->ulTrafficDescrParm1;
        ulMcr = pEntry->ulTrafficDescrParm3;

        switch( pEntry->ulServiceCategory )
        {
        case SC_CBR:
            ulShapingAlgorithm = SST_CBR;
            break;

        case SC_RT_VBR:
        case SC_NRT_VBR:
            ulShapingAlgorithm = SST_VBR_1;
            break;

        case SC_UBR:
            ulShapingAlgorithm = SST_UBR_PCR;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;
#endif

    case TDT_ATM_CLP_TRANSPARENT_NO_SCR:
        ulPcr = pEntry->ulTrafficDescrParm1;
        switch( pEntry->ulServiceCategory )
        {
        case SC_CBR:
            ulShapingAlgorithm = SST_CBR;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

    case TDT_ATM_CLP_TRANSPARENT_SCR:
        ulPcr = pEntry->ulTrafficDescrParm1;
        ulScr = pEntry->ulTrafficDescrParm2;
        ulMbs = pEntry->ulTrafficDescrParm3;
        switch( pEntry->ulServiceCategory )
        {
        case SC_RT_VBR:
        case SC_NRT_VBR:
            ulShapingAlgorithm = SST_VBR_1;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

    case TDT_ATM_NO_CLP_TAGGING_NO_SCR:
        ulPcr = pEntry->ulTrafficDescrParm1;
        switch( pEntry->ulServiceCategory )
        {
        case SC_UBR:
            ulShapingAlgorithm = SST_UBR_PCR;
            break;

        default:
            baStatus = STS_PARAMETER_ERROR;
            break;
        }
        break;

    default:
        baStatus = STS_NOT_SUPPORTED;
    }

    if( baStatus == STS_SUCCESS )
    {
        const UINT32 ulUsPerSec = 1000000;      // microseconds per second
        const UINT32 ulUt = ms_ulSitUt;         // unit of time in us

        // Convert from cell rates to rate percentages which is required by
        // the ATM Processor. The unit of time, ulUt, is defined 100 times
        // larger than its actual value and is, therefore, divided by 100.
        // This is done to reduce rounding errors.
        UINT32 ulIp = (ulPcr) ? ulUsPerSec / ((ulUt * ulPcr) / 100) : 0;
        UINT32 ulIs = (ulScr) ? ulUsPerSec / ((ulUt * ulScr) / 100) : 0;
        UINT32 ulIm = (ulMcr) ? ulUsPerSec / ((ulUt * ulMcr) / 100) : 0;
        UINT32 ulBt = (ulMbs - 1) * (ulIs - ulIp); // Burst Tolerance

        // The ATM Processor expects IP, IS and IM values to be one less than the
        // computed value.  But don't subtract one on the BCM6348 if the Shaper
        // Interval Time is odd.
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96348)
        UINT32 ulSit = (AP_REGS->ulTxAtmSchedCfg&ATMR_SIT_MASK)>>ATMR_SIT_SHIFT;
#else
        UINT32 ulSit = 0;
#endif

        if( (ulSit & 0x01) == 0 )
        {
            if( ulIp )
                ulIp--;

            if( ulIs )
                ulIs--;

            if( ulIm )
                ulIm--;
        }

        // Calculate new shaper register values.
        UINT32 ulCtrl = SST_EN | ((UINT32) ucVcid << SST_VCID_SHIFT) |
            ((UINT32) ucPortId << SST_PORT_ID_SHIFT) | ulShapingAlgorithm |
            ((UINT32) ucPriority << SST_SRC_PRI_SHIFT) |
            (ulIp << SST_RATE_PCR_SHIFT);

        UINT32 ulVbr = ((ulBt << SST_RATE_BT_SHIFT) & SST_RATE_BT_MASK) |
            ((ulIs << SST_RATE_SCR_SHIFT) & SST_RATE_SCR_MASK);

        if( ulMpEnable )
            ulCtrl |= SST_MP_EN;

        // Change shaper registers if they are different than current values.
        if( *m_pulSstCtrl != ulCtrl || *m_pulSstVbr != ulVbr ||
            (m_pulSstMcr && *m_pulSstMcr != ulIm) )
        {
            // Disable shaper.
            *m_pulSstCtrl &= ~SST_EN;

            // Reset shaper.
            *m_pulSstCtrl |= SST_SHAPER_RESET;

            // Set new values for cell rate and control registers.
            *m_pulSstVbr = ulVbr;
            if( m_pulSstMcr )
                *m_pulSstMcr = ulIm;
            *m_pulSstCtrl = ulCtrl & ~SST_EN;

            // Enable shaper.
            *m_pulSstCtrl |= SST_EN;
        }
    }

    return( baStatus );
} // Configure


//**************************************************************************
// ATM_TX_BDQ Class (Hardware SAR)
//**************************************************************************

#define CRC10_TABLE_NOT_INITIALIZED         0x1234
UINT16 ATM_TX_BDQ::ms_usByteCrc10Table[] = {CRC10_TABLE_NOT_INITIALIZED};

//**************************************************************************
// Function Name: ATM_TX_BDQ
// Description  : Constructor for transmit buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_TX_BDQ::ATM_TX_BDQ( ATM_OS_SERVICES *pOsServices, UINT32 ulAtmApplSem,
    UINT32 ulPhyPort1Enabled, void *pSoftSar )
{
    m_pOsServices = pOsServices;
    m_ulAtmApplSem = ulAtmApplSem;
    m_ulPhyPort1Enabled = ulPhyPort1Enabled;
    m_pStartBase = NULL;
    m_pStart = NULL;
    m_usLength = 0;
    m_usHeadIdx = 0;
    m_pucHeadIdx = m_pucTailIdx = NULL;
    m_ucPriority = 0;
    m_ucShaperNum = 0xff;
    m_ucBasePriority = 1;
    m_ulMpEnable = 0;
    m_ucVcid = 0;
    m_ucPortId = 0;
    m_ulShaped = 0;
    m_ulCurrBdIdx = m_ulTxPending = 0;
    m_ulInUse = 1;
    m_pTxBdInfo = NULL;
    m_ulMemSize = CACHE_TO_PHYS((*m_pOsServices->pfnGetTopMemAddr) ());

    GenByteCrc10Table();
} // ATM_TX_BDQ


//**************************************************************************
// Function Name: ~ATM_TX_BDQ
// Description  : Destructor for transmit buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_TX_BDQ::~ATM_TX_BDQ( void )
{
    Destroy();
} // ~ATM_TX_BDQ


//**************************************************************************
// Function Name: Create
// Description  : Create a new transmit queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Create( UINT32 ulLength, UINT8 ucBasePriority,
    UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    BCMATM_STATUS baStatus = STS_ERROR;

    // Create a semaphore that will be used for accessing the transmit
    // queue.
    if( (m_ulMuSem = (*m_pOsServices->pfnCreateSem) (1)) != 0 )
    {
        const UINT16 usDefaultLength = 64;

        // Save the traffic descriptor parameter entry.
        memcpy( &m_TdpEntry, pEntry, sizeof(m_TdpEntry) );

        // Save input parameters to member data variables so they can be
        // used later in other member functions.
        m_usLength = (ulLength < 2 || ulLength > MAX_TX_QUEUE_SIZE)
            ? usDefaultLength : (UINT16) ulLength;
        /*begine of modify the queue length is 10 by 139225 2006-7-10*/
        m_usLength = m_usLength - 1;
        /*end of modify the queue length is 10 by 139225 2006-7-10**/
        UINT32 ulAllocLength = m_usLength *
            (sizeof(ATM_DMA_BD) + sizeof(ATM_TX_BD_INFO));

        // Allocate memory for the transmit queue.
        m_pStartBase=(char *) (*m_pOsServices->pfnAlloc)(ulAllocLength + 0x10);
        if( m_pStartBase != NULL )
        {
            // Align m_pStart on a quad word (16 byte) boundary.
            m_pStart = (ATM_DMA_BD *) (((UINT32) m_pStartBase + 0x0f) & ~0x0f);
            memset( m_pStart, 0x00, ulAllocLength );
            m_pOsServices->pfnFlushCache( m_pStart, ulAllocLength );
            m_pTxBdInfo = (ATM_TX_BD_INFO *) (m_pStart + m_usLength);
            m_pStart = (ATM_DMA_BD *) CACHE_TO_NONCACHE(m_pStart);

            // Find an available queue/shaper to use.
            (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
            m_ucShaperNum = 0xff;
            for( UINT32 i = 0; i < MAX_QUEUE_SHAPERS; i++)
            {
                if( AP_REGS->ulTxQAddr[i] == 0 )
                {
                    // Set transmit queue length.
                    UINT32 ulHtl = (((m_usLength - 1) << DMAR_TX_LEN_SHIFT) &
                        DMAR_TX_LEN_MASK);

                    // Configure the ATM Processor transmit DMA registers.
                    AP_REGS->ulTxQAddr[i] = (UINT32) NONCACHE_TO_PHYS(m_pStart);

                    AP_REGS->ulTxQHeadTailLen[i] = ulHtl;

                    m_ucShaperNum = i;
                    break;
                }
            }
            (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

            if( m_ucShaperNum != 0xff )
            {
                // If the ADSL PHY has enabled the second DMA port and the
                // traffic descriptor type is UBR, send the packet to ADSL
                // PHY port 1 which is a lower priority port.
                if( m_ulPhyPort1Enabled==1 && pEntry->ulServiceCategory==SC_UBR )
                    ucPortId = PHY_1;

                // Save parameters to private data members.
                m_ucVcid = ucVcid;
                m_ucPortId = ucPortId;
                m_ucBasePriority = ucBasePriority;
                m_ulMpEnable = ulMpEnable;
                if( pEntry->ulTrafficDescrType != TDT_ATM_NO_TRAFFIC_DESCRIPTOR )
                    m_ulShaped = 1;
                else
                    m_ulShaped = 0;

                // Configure the ATM shaper.
                if( (m_ucPriority = m_Shaper.GetAdjustedPriority(
                    pEntry->ulServiceCategory, m_ucBasePriority )) != 0xff )
                {
                    baStatus = m_Shaper.Configure( m_ucShaperNum, m_ucPriority,
                        m_ulMpEnable, m_ucPortId, m_ucVcid, pEntry );
                }
                else
                    baStatus = STS_PARAMETER_ERROR;

                if( baStatus == STS_SUCCESS )
                {

                    UINT8 *p = (UINT8 *) &AP_REGS->ulTxQHeadTailLen[m_ucShaperNum];

                    m_pucHeadIdx = p + 1;
                    m_pucTailIdx = p + 2;
                    m_usHeadIdx = 0;
                    m_ulCurrBdIdx = 0;

                    // Enable DMA transmit for this queue.
                    (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
                    AP_REGS->ulTxSdramChEnable |= (1 << m_ucShaperNum);
                    (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);
                }
                else
                {
                    (*m_pOsServices->pfnFree)(m_pStartBase);
                    m_pStartBase = NULL;
                    m_pStart = NULL;
                    m_pTxBdInfo = NULL;
                    AP_REGS->ulTxQAddr[m_ucShaperNum] = 0;
                    AP_REGS->ulTxQHeadTailLen[m_ucShaperNum] = 0;
                }
            }
            else
                baStatus = STS_RESOURCE_ERROR;
        }
        else
        {
            baStatus = STS_ALLOC_ERROR;
        }
    }
    else
    {
        baStatus = STS_ERROR;
    }

    return( baStatus );
} // Create


//**************************************************************************
// Function Name: Destroy
// Description  : Destroy the transmit queue.
// Returns      : None.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Destroy( void )
{
    ATM_TX_BD_INFO *pTbi;
    UINT32 i;

    if( m_pStartBase )
    {
        // Disable DMA transmit for this queue.
        (*m_pOsServices->pfnRequestSem) (m_ulAtmApplSem, MAX_TIMEOUT);
        AP_REGS->ulTxSdramChEnable &= ~(1 << m_ucShaperNum);
        (*m_pOsServices->pfnReleaseSem) (m_ulAtmApplSem);

        AP_REGS->ulSstCtrl[m_ucShaperNum] = SST_SHAPER_RESET;

        AP_REGS->ulTxQAddr[m_ucShaperNum] = (UINT32) 0;
        AP_REGS->ulTxQHeadTailLen[m_ucShaperNum] = (UINT32) 0;
        m_pucHeadIdx = m_pucTailIdx = NULL;

        // Wait up to three seconds for all transmit operations to complete.
        for( i = 0; m_ulTxPending && i < 30 ; i++ )
            (*m_pOsServices->pfnDelay) (100);

        m_ulTxPending = 0;

        // If there are still uncompleted transmit operations, free the buffers.
        for( i = 0, pTbi = m_pTxBdInfo; i < m_usLength; i++, pTbi++ )
            if( pTbi->pDataParms )
            {
                (*pTbi->pDataParms->pFnFreeDataParms) (pTbi->pDataParms);
                pTbi->pDataParms = NULL;
            }

        (*m_pOsServices->pfnFree) (m_pStartBase);
        m_pStartBase = NULL;
        m_pStart = NULL;
        m_pTxBdInfo = NULL;
    }

    if( m_ulMuSem )
    {
        (*m_pOsServices->pfnDeleteSem) (m_ulMuSem);
        m_ulMuSem = 0;
    }

    return( STS_SUCCESS );
} // Destroy


//**************************************************************************
// Function Name: Add
// Description  : Adds an ATM data buffer to the transmit queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Add( PATM_VCC_DATA_PARMS pDataParms, UINT8 ucVcid )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    ATM_DMA_BD *pHead;
    ATM_TX_BD_INFO *pTxInfo;
    PATM_BUFFER pAtmBuf = pDataParms->pAtmBuffer;
    UINT32 ulCt = (UINT32) pDataParms->ucCircuitType;
    UINT32 ulFlags = pDataParms->ucFlags << BD_FLAGS_SHIFT;
    UINT8 ucPortId = m_ucPortId << BD_TX_PORT_ID_SHIFT;
    int nError;

    // Reclaim buffers that have already been transmitted.
    if( m_ulTxPending )
        Reclaim();

    // This function can be called by a task from different thread contexts
    // so a mutex is used when adding a buffer to the transmit queue.
    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    pTxInfo = m_pTxBdInfo + m_usHeadIdx;
    pTxInfo->pDataParms = pDataParms;
    pTxInfo->ucNumBds = 0;
    pTxInfo->ucFirstIdx = (UINT8) m_usHeadIdx;

    nError = 0;

    // Assign a DMA buffer descriptor address to each buffer in the
    // pDataParms->pAtmBuffer chain.
    while( pAtmBuf )
    {
        pHead = m_pStart + m_usHeadIdx;

        if( ((m_usHeadIdx + 1) % m_usLength) == (UINT16) m_ulCurrBdIdx )
        {
            // Queue is full.
            baStatus = STS_RESOURCE_ERROR;
            nError = 1;
            break;
        }

        pTxInfo->ucNumBds++;

        pHead->ulFlags_NextRxBd = ulFlags;
        if( pAtmBuf->pNextAtmBuf == NULL )
        {
            // Set the EOP bit for the last buffer of an AAL5 packet.
            if( ulCt == CT_AAL5 )
                pHead->ulFlags_NextRxBd |= BD_FLAG_EOP;

            pTxInfo->usLastIdx = m_usHeadIdx;
        }

        if( pAtmBuf->ulReserved == 0 )
        {
            // Not a transparent cell.
            pHead->ulCt_BufPtr = ulCt << BD_CT_SHIFT;
            BD_SET_ADDR(pHead->ulCt_BufPtr, pAtmBuf->pDataBuf);
            pHead->usLength = (UINT16) pAtmBuf->ulDataLen;
            pHead->ucRxPortId_Vcid = ucVcid;
            pHead->ucUui8 = pDataParms->ucUuData8;
            pHead->ucTxPortId_Gfc = ucPortId;
        }
        else
        {
            // The data contains a transparent cell (four byte ATM header and
            // payload) or OAM cell (no ATM header).  Convert the data to an
            // AAL0 cell with a non-zero GFC. The GFC index is store in the
            // ATM_BUFFER reserved field.
            UINT32 ulAtmHdrSize;

            if( ulCt != CT_TRANSPARENT )
            {
                // The BCM63xx ATM Processor can calculate the CRC10 in hardware.
                // However, if this queue is configured for AAL0 cells, the CRC10
                // generation in hardware may not be configured.  Therefore,
                // calculate the CRC10 in software.
                const UINT16 usOam10Mask = 0x03ff;
                const int nOamCrc10Offset = 46;

                UINT16 *pusCrc10 = (UINT16 *)(pAtmBuf->pDataBuf+nOamCrc10Offset);

                // Zero the CRC10 field.
                *pusCrc10 &= ~usOam10Mask;

                // Calculate the CRC10.
                UINT16 usCrc10 = UpdateCrc10ByBytes( 0, pAtmBuf->pDataBuf, 48 );

                // Update OAM cell with CRC.
                *pusCrc10 |= usCrc10;

                ulAtmHdrSize = 0;

                pHead->ulCt_BufPtr = CT_AAL0_PACKET << BD_CT_SHIFT;
            }
            else
            {
                ulAtmHdrSize = 4;
                pHead->ulCt_BufPtr = CT_AAL0_CELL_CRC << BD_CT_SHIFT;
            }

            BD_SET_ADDR(pHead->ulCt_BufPtr, pAtmBuf->pDataBuf + ulAtmHdrSize);
            pHead->usLength = (UINT16) (pAtmBuf->ulDataLen - ulAtmHdrSize);
            pHead->ucRxPortId_Vcid = pHead->ucUui8 = 0;
            pHead->ucTxPortId_Gfc = ucPortId | pAtmBuf->ulReserved; // GFC index
            pHead->ulFlags_NextRxBd |= BD_FLAG_EOP;
            pAtmBuf->ulReserved = 0;
        }

        if( ++m_usHeadIdx == m_usLength )
            m_usHeadIdx = 0;

        m_pOsServices->pfnFlushCache(pAtmBuf->pDataBuf, pAtmBuf->ulDataLen);

        pAtmBuf = pAtmBuf->pNextAtmBuf;
    }

    if( nError == 0 )
    {
        // Updating the DMA transmit queue head index causes the DMA transfer
        // to start.
        m_ulTxPending = 1;
        *m_pucHeadIdx = (UINT8) m_usHeadIdx;
    }
    else
    {
        // Not enough buffer descriptors.  Reset head index.
        /*start of ATM 3.2.1 porting by l39225 20060504*/
        m_ulTxPending = 1;
        /*end of ATM 3.2.1 porting by l39225 20060504*/
        pTxInfo->pDataParms = NULL;
        m_usHeadIdx = *m_pucHeadIdx;
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( baStatus );
} // Add


//**************************************************************************
// Function Name: Reclaim
// Description  : Returns buffers that have been sent from the DMA transmit
//                queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Reclaim( void )
{
    ATM_TX_BD_INFO *pTbi;
    PATM_VCC_DATA_PARMS pDp;
    UINT16 usTailIdx;

    /* start of wan data disable by y42304 20060522: wan led°´TR068´¦Àí */
    #if 0
    if( m_ulTxPending && m_pOsServices->pfnBlinkLed )
    {
        (*m_pOsServices->pfnBlinkLed) ();
    }
    #endif
    /* end of wan data disable by y42304 20060522 */

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    pTbi = m_pTxBdInfo + m_ulCurrBdIdx;
    pDp = pTbi->pDataParms; 
    usTailIdx = *m_pucTailIdx;

    while( pDp && (pTbi->ucNumBds - 1) <
        Q_ELS_FILLED(usTailIdx, pTbi->ucFirstIdx, m_usLength) )
    {
        // All BDs in the current transmit packet have been sent.  Free the ATM
        // API transmit data parameter structure.
        (*pDp->pFnFreeDataParms) (pDp);
        pTbi->pDataParms = NULL;

        if( (m_ulCurrBdIdx = pTbi->usLastIdx + 1) == m_usLength )
            m_ulCurrBdIdx = 0;

        pTbi = m_pTxBdInfo + m_ulCurrBdIdx;
        pDp = pTbi->pDataParms;
        usTailIdx = *m_pucTailIdx;
    }

    m_ulTxPending = (pDp) ? 1 : 0;

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( STS_SUCCESS );
} // Reclaim


//**************************************************************************
// Function Name: ReconfigureShaper
// Description  : Reconfigure an ATM Processor shaper.
// Returns      : None.
//**************************************************************************
void ATM_TX_BDQ::ReconfigureShaper( PATM_TRAFFIC_DESCR_PARM_ENTRY
    pEntry )
{
    // If non-NULL, save the traffic descriptor parameter entry.  It may be NULL
    // if the ms_ulSitUt value was changed in order to reconfigure shaping
    // registers.
    if( pEntry )
        memcpy( &m_TdpEntry, pEntry, sizeof(m_TdpEntry) );

    // Configure the ATM shaper.
    if( (m_ucPriority = m_Shaper.GetAdjustedPriority(
        m_TdpEntry.ulServiceCategory, m_ucBasePriority )) != 0xff )
    {
        if( AP_REGS->ulUtopiaCfg == 0 && m_ulPhyPort1Enabled == 1 &&
            m_TdpEntry.ulServiceCategory == SC_UBR )
        {
            m_ucPortId = PHY_1;
        }
        else
        {
            m_ucPortId = PHY_0;
        }

        if( m_TdpEntry.ulTrafficDescrType != TDT_ATM_NO_TRAFFIC_DESCRIPTOR )
            m_ulShaped = 1;
        else
            m_ulShaped = 0;

        m_Shaper.Configure( m_ucShaperNum, m_ucPriority, m_ulMpEnable,
            m_ucPortId, m_ucVcid, &m_TdpEntry );
    }
} // ReconfigureShaper

#else
//**************************************************************************
// ATM_TX_BDQ Class (Software SAR)
//**************************************************************************

#define CRC10_TABLE_NOT_INITIALIZED         0x1234
UINT16 ATM_TX_BDQ::ms_usByteCrc10Table[] = {CRC10_TABLE_NOT_INITIALIZED};
UINT32 ATM_TX_BDQ::m_ulGfcAtmHdrs[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//**************************************************************************
// Function Name: ATM_TX_BDQ
// Description  : Constructor for transmit buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_TX_BDQ::ATM_TX_BDQ( ATM_OS_SERVICES *pOsServices, UINT32 ulAtmApplSem,
    UINT32 ulPhyPort1Enabled, void *pSoftSar )
{
    pOsServices->pfnAdslSetVcEntry = (FN_ADSL_SET_VC_ENTRY) SetVcEntry;
    pOsServices->pfnAdslSetVcEntryEx = (FN_ADSL_SET_VC_ENTRY_EX) SetVcEntryEx;

    m_pSoftSar = (ATM_SOFT_SAR *) pSoftSar;
    m_ulInUse = 1;

    GenByteCrc10Table();
} // ATM_TX_BDQ


//**************************************************************************
// Function Name: ~ATM_TX_BDQ
// Description  : Destructor for transmit buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_TX_BDQ::~ATM_TX_BDQ( void )
{
    m_pSoftSar->DestroyVc( m_ucVcid );
} // ~ATM_TX_BDQ


//**************************************************************************
// Function Name: Create
// Description  : Create a new transmit queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Create( UINT32 ulLength, UINT8 ucBasePriority,
    UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
    PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    m_ucPortId = ucPortId;
    if( ucVcid < MAX_VCIDS )
    {
        m_ucVcid = ucVcid;
        baStatus = m_pSoftSar->CreateVc( ucVcid, ulLength,
            (ulMpEnable) ? ulMpEnable : 1, pEntry );
    }
    else
        m_ucVcid = 0;

    return( baStatus );
} // Create


//**************************************************************************
// Function Name: Destroy
// Description  : Destroy the transmit queue.
// Returns      : None.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Destroy( void )
{
    return( m_pSoftSar->DestroyVc( m_ucVcid ) );
} // Destroy


//**************************************************************************
// Function Name: Add
// Description  : Adds an ATM data buffer to the transmit queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Add( PATM_VCC_DATA_PARMS pDataParms, UINT8 ucVcid  )
{
    const UINT16 usOam10Mask = 0x03ff;
    const int nOamCrc10Offset = kAtmCellDataSize - sizeof(UINT16);

    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;
    unsigned long i, *pul;
    UINT16 *pusCrc10, usCrc10;

    switch( pDataParms->ucCircuitType )
    {
    case CT_AAL5:
        baStatus = m_pSoftSar->AddPkt( m_ucVcid, pDataParms );
        break;

    case CT_OAM_F5_SEGMENT:
    case CT_OAM_F5_END_TO_END:
    case CT_OAM_F4_ANY:
    case CT_AAL0_CELL_CRC:
        // Update OAM cell with CRC.
        pusCrc10 = (UINT16 *)(pAb->pDataBuf+nOamCrc10Offset);
        *pusCrc10 &= ~usOam10Mask;
        usCrc10 = UpdateCrc10ByBytes( 0, pAb->pDataBuf, 48 );
        *pusCrc10 |= usCrc10;

        // Shift cell buffer by four bytes.  The ATM header goes in the first
        // four bytes.
        for( i = 0, pul = (unsigned long *)
             (pAb->pDataBuf + kAtmCellDataSize - sizeof(long));
             i < kAtmCellDataSize / sizeof(long); i++, pul-- )
        {
            *(pul + 1) = *pul;
        }

        if( pAb->ulReserved > 0 &&
            pAb->ulReserved < sizeof(m_ulGfcAtmHdrs) / sizeof(long) )
        {
            *(unsigned long *) pAb->pDataBuf = m_ulGfcAtmHdrs[pAb->ulReserved];
            baStatus = m_pSoftSar->AddCell( pDataParms );
        }
        else
        {
            const UINT8 ucOamF5Segment  = (0x04 << 1);
            const UINT8 ucOamF5EndToEnd = (0x05 << 1);
            ATM_VCC_ADDR *pAddr;

            if( (pAddr = m_pSoftSar->GetVccAddr( ucVcid )) != NULL )
            {
                UINT32 ulAtmHdr = ((unsigned long) pAddr->usVpi << 20) |
                    ((unsigned long) pAddr->usVci <<  4);

                if( pDataParms->ucCircuitType == CT_OAM_F5_SEGMENT )
                    ulAtmHdr |= ucOamF5Segment;
                else
                    if( pDataParms->ucCircuitType == CT_OAM_F5_END_TO_END )
                        ulAtmHdr |= ucOamF5EndToEnd;

                *(unsigned long *) pAb->pDataBuf = ulAtmHdr;
                baStatus = m_pSoftSar->AddCell( pDataParms );
            }
            else
            {
                (*pDataParms->pFnFreeDataParms) (pDataParms);
                baStatus = STS_PARAMETER_ERROR;
            }
        }
        break;

    case CT_TRANSPARENT:
        baStatus = m_pSoftSar->AddCell( pDataParms );
        break;

    default:
        (*pDataParms->pFnFreeDataParms) (pDataParms);
        baStatus = STS_PARAMETER_ERROR;
        break;
    }

    return( baStatus );
} // Add


//**************************************************************************
// Function Name: Reclaim
// Description  : Returns buffers that have been sent from the DMA transmit
//                queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_TX_BDQ::Reclaim( void )
{
    return( STS_SUCCESS );
} // Reclaim


//**************************************************************************
// Function Name: ReconfigureShaper
// Description  : Reconfigure an ATM Processor shaper.
// Returns      : None.
//**************************************************************************
void ATM_TX_BDQ::ReconfigureShaper( PATM_TRAFFIC_DESCR_PARM_ENTRY
    pEntry )
{
    m_pSoftSar->SetShapingValues( m_ucVcid, pEntry );
} // ReconfigureShaper


//**************************************************************************
// Function Name: SetVcEntry
// Description  : Maps VPI/VCI to a GFC index.
// Returns      : 0
//**************************************************************************
UINT32 ATM_TX_BDQ::SetVcEntry( int gfc, int port, int vpi, int vci )
{
    if( (unsigned long) gfc < sizeof(m_ulGfcAtmHdrs) / sizeof(long) )
    {
        m_ulGfcAtmHdrs[gfc] = ((unsigned long) vpi << 20) |
                              ((unsigned long) vci <<  4);
    }

    return( 0 );
} // SetVcEntry


//**************************************************************************
// Function Name: SetVcEntryEx
// Description  : Maps VPI/VCI to a GFC index.
// Returns      : 0
//**************************************************************************
UINT32 ATM_TX_BDQ::SetVcEntryEx(int gfc, int port, int vpi, int vci, int pti_clp)
{
    if( (unsigned long) gfc < sizeof(m_ulGfcAtmHdrs) / sizeof(long) )
    {
        m_ulGfcAtmHdrs[gfc] = ((unsigned long) vpi << 20) |
                              ((unsigned long) vci <<  4) |
                              ((unsigned long) pti_clp);
    }

    return( 0 );
} // SetVcEntryEx

#endif

//**************************************************************************
// Function Name: GenByteCrc10Table
// Description  : Generate the table of CRC-10 remainders for all possible
//                bytes.
// Returns      : None.
//**************************************************************************
void ATM_TX_BDQ::GenByteCrc10Table( void )
{
    if( ms_usByteCrc10Table[0] == CRC10_TABLE_NOT_INITIALIZED )
    {
        int i, j;
        unsigned short crc10_accum;

        for( i = 0;  i < 256;  i++ )
        {
            crc10_accum = ((unsigned short) i << 2);
            for ( j = 0;  j < 8;  j++ )
                if ((crc10_accum <<= 1) & 0x400) crc10_accum ^= 0x633;

            ms_usByteCrc10Table[i] = crc10_accum;
        }
    }
} // GenByteCrc10Table


//**************************************************************************
// Function Name: UpdateCrc10ByBytes
// Description  : Update the data block CRC-10 remainder one byte at a time.
// Returns      : None.
//**************************************************************************
UINT16 ATM_TX_BDQ::UpdateCrc10ByBytes( UINT16 usCrc10Accum, UINT8 *pBuf,
    int nDataBlkSize )
{
    int i;

    for( i = 0;  i < nDataBlkSize;  i++ )
    {
        usCrc10Accum = ((usCrc10Accum << 8) & 0x3ff)
            ^ ms_usByteCrc10Table[(usCrc10Accum >> 2) & 0xff]
            ^ *pBuf++;
    }

    return( usCrc10Accum );
} // UpdateCrc10ByBytes


//**************************************************************************
// ATM_RX_FREE_BDQ Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_RX_FREE_BDQ
// Description  : Constructor for the base class receive and free buffer
//                descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_RX_FREE_BDQ::ATM_RX_FREE_BDQ( ATM_OS_SERVICES *pOsServices )
{
    m_pOsServices = pOsServices;
    m_pulRegisterBase = NULL;
    m_ulLength = 0;
    m_ppStartBase = NULL;
    m_ppStart = NULL;
    m_pulHeadIdx = 0;
    m_pulTailIdx = 0;
} // ATM_RX_FREE_BDQ


//**************************************************************************
// Function Name: ~ATM_RX_FREE_BDQ
// Description  : Destructor for the base class receive and free buffer
//                descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_RX_FREE_BDQ::~ATM_RX_FREE_BDQ( void )
{
    Destroy();
} // ~ATM_RX_FREE_BDQ


//**************************************************************************
// Function Name: Create
// Description  : Create a new queue of ATM_DMA_BD buffer descriptor (BD)
//                pointers.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_RX_FREE_BDQ::Create( volatile UINT32 *pulRegisterBase,
    UINT32 ulLength )
{
    BCMATM_STATUS baStatus = STS_ERROR;

    const UINT32 ulDefaultLength = 128;

    m_ulLength = (ulLength == 0) ? ulDefaultLength : ulLength;
    UINT32 ulAllocLength = ((sizeof(ATM_DMA_BD *) * m_ulLength) + 0x1f) & ~0x0f;

    if( (m_ulMuSem = (*m_pOsServices->pfnCreateSem) (1)) != 0 )
    {
        // Allocate memory for the transmit queue.
        m_ppStartBase = (ATM_DMA_BD **)
            (*m_pOsServices->pfnAlloc) (ulAllocLength + 0x10);
        if( m_ppStartBase != NULL )
        {

            m_ppStart = (ATM_DMA_BD **)(((UINT32) m_ppStartBase + 0x0f) & ~0x0f);
            memset( m_ppStart, 0x00, ulAllocLength );
            m_pOsServices->pfnFlushCache( m_ppStart, ulAllocLength );
            m_ppStart = (ATM_DMA_BD **) CACHE_TO_NONCACHE( m_ppStart );

            m_pulRegisterBase = pulRegisterBase;

            m_pulRegisterBase[RX_FREE_ADDR_OFFSET] = (UINT32)
                NONCACHE_TO_PHYS(m_ppStart);
            m_pulRegisterBase[RX_FREE_HEAD_OFFSET] = (UINT32) 0;
            m_pulRegisterBase[RX_FREE_TAIL_OFFSET] = (UINT32) 0;
            m_pulRegisterBase[RX_FREE_LEN_OFFSET]  = (UINT32) m_ulLength - 1;

            m_pulHeadIdx = &m_pulRegisterBase[RX_FREE_HEAD_OFFSET];
            m_pulTailIdx = &m_pulRegisterBase[RX_FREE_TAIL_OFFSET];

            baStatus = STS_SUCCESS;
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }
    else
        baStatus = STS_ERROR;

    return( baStatus );
} // Create


//**************************************************************************
// Function Name: Destroy
// Description  : Destroy the queue.
// Returns      : None.
//**************************************************************************
BCMATM_STATUS ATM_RX_FREE_BDQ::Destroy( void )
{
    if( m_ppStartBase )
    {
        m_pulRegisterBase[RX_FREE_ADDR_OFFSET] = 0;
        m_pulRegisterBase[RX_FREE_HEAD_OFFSET] = 0;
        m_pulRegisterBase[RX_FREE_TAIL_OFFSET] = 0;
        m_pulRegisterBase[RX_FREE_LEN_OFFSET]  = 0;

        m_pulRegisterBase = NULL;
        m_pulHeadIdx = m_pulTailIdx = NULL;

        (*m_pOsServices->pfnFree) ((void *) NONCACHE_TO_CACHE(m_ppStartBase));
        m_ppStartBase = m_ppStart = NULL;
    }

    if( m_ulMuSem )
    {
        (*m_pOsServices->pfnDeleteSem) (m_ulMuSem);
        m_ulMuSem = 0;
    }

    return( STS_SUCCESS );
} // Destroy


//**************************************************************************
// Function Name: Add
// Description  : Adds a pointer to a buffer descriptor to the queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_RX_FREE_BDQ::Add( ATM_DMA_BD *pBd )
{
    BCMATM_STATUS baStatus;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Add buffer descriptor to the receive queue.
    UINT32 ulHeadIdx = *m_pulHeadIdx;
    UINT32 ulNextHeadIdx = (ulHeadIdx + 1) % m_ulLength;
    ATM_DMA_BD **ppBd = &m_ppStart[ulHeadIdx];

    if( ulNextHeadIdx != *m_pulTailIdx )
    {
        *ppBd = (ATM_DMA_BD *) NONCACHE_TO_PHYS(pBd);
        *m_pulHeadIdx = ulNextHeadIdx;

        baStatus = STS_SUCCESS;
    }
    else
        baStatus = STS_RESOURCE_ERROR;

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( baStatus );
} // Add


//**************************************************************************
// Function Name: GetNumFilledElements
// Description  : Returns the number of queue elements that are filled.
// Returns      : Number of queue elements that are filled.
//**************************************************************************
UINT32 ATM_RX_FREE_BDQ::GetNumFilledElements( void )
{
    UINT32 ulHeadIdx = *m_pulHeadIdx;
    UINT32 ulTailIdx = *m_pulTailIdx;
    return( Q_ELS_FILLED(ulHeadIdx, ulTailIdx, (m_ulLength - 1)) );
} // GetNumFilledElements


//**************************************************************************
// Function Name: GetNumAvailableElements
// Description  : Returns the number of queue elements that are not filled.
// Returns      : Number of queue elements that are filled.
//**************************************************************************
UINT32 ATM_RX_FREE_BDQ::GetNumAvailableElements( void )
{
    UINT32 ulHeadIdx = *m_pulHeadIdx;
    UINT32 ulTailIdx = *m_pulTailIdx;
    return( (m_ulLength-1) - Q_ELS_FILLED(ulHeadIdx, ulTailIdx, m_ulLength-1) );
} // GetNumAvailableElements


//**************************************************************************
// ATM_RX_BDQ Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_RX_BDQ
// Description  : Constructor for the receive buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_RX_BDQ::ATM_RX_BDQ( ATM_OS_SERVICES *pOsServices )
    : ATM_RX_FREE_BDQ( pOsServices )
{
} // ATM_RX_BDQ


//**************************************************************************
// Function Name: ~ATM_RX_BDQ
// Description  : Destructor for the receive buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_RX_BDQ::~ATM_RX_BDQ( void )
{
} // ~ATM_RX_BDQ


//**************************************************************************
// Function Name: Remove
// Description  : Removes a pointer to a buffer descriptor from the queue.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_RX_BDQ::Remove( ATM_DMA_BD **ppBd )
{
    BCMATM_STATUS baStatus;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Get buffer descriptor from the receive queue.
    UINT32 ulTailIdx = *m_pulTailIdx;
    if( *m_pulHeadIdx != ulTailIdx )
    {
        ATM_DMA_BD *pBd = (ATM_DMA_BD *) PHYS_TO_NONCACHE(m_ppStart[ulTailIdx]);
        ulTailIdx = (ulTailIdx + 1) % m_ulLength;
        *m_pulTailIdx = ulTailIdx;
        *ppBd = pBd;
        baStatus = STS_SUCCESS;
    }
    else
    {
        *ppBd = NULL;
        baStatus = STS_NOT_FOUND;
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( baStatus );
} // Remove


//**************************************************************************
// ATM_FREE_BDQ Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_FREE_BDQ
// Description  : Constructor for the free buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_FREE_BDQ::ATM_FREE_BDQ( ATM_OS_SERVICES *pOsServices )
    : ATM_RX_FREE_BDQ( pOsServices )
{
} // ATM_FREE_BDQ


//**************************************************************************
// Function Name: ~ATM_FREE_BDQ
// Description  : Destructor for the free buffer descriptor queue class.
// Returns      : None.
//**************************************************************************
ATM_FREE_BDQ::~ATM_FREE_BDQ( void )
{
} // ~ATM_FREE_BDQ

