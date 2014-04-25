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
// File Name  : AtmApplTable.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM application
//              object lookup table class.  This class contains member functions
//              that return an ATM_VCC_APPL object for a shaper number, virtual
//              channel id and VCC address.
//
// Updates    : 01/03/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"


//**************************************************************************
// ATM_APPL_TABLE Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_APPL_TABLE
// Description  : Constructor for the ATM application lookup table class.
// Returns      : None.
//**************************************************************************
ATM_APPL_TABLE::ATM_APPL_TABLE( ATM_OS_SERVICES *pOsServices )
{
    m_pOsServices = pOsServices;
    m_ulMuSem = 0;
    memset( m_pApplTable, 0x00, sizeof(m_pApplTable) );
    memset( m_pRxVcids, 0x00, sizeof(m_pRxVcids) );
    memset( m_VccAddrMap, 0x00, sizeof(m_VccAddrMap) );
    memset( m_pTxQShaperNums, 0x00, sizeof(m_pTxQShaperNums) );
} // ATM_APPL_TABLE


//**************************************************************************
// Function Name: ~ATM_APPL_TABLE
// Description  : Destructor for the ATM application lookup table class.
// Returns      : None.
//**************************************************************************
ATM_APPL_TABLE::~ATM_APPL_TABLE()
{
    int i;
    ATM_APPL *pAppl;

    if( m_ulMuSem )
        (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    for( i = 0; i <= MAX_APPL_TABLE_ENTRIES; i++ )
    {
        if( (pAppl = m_pApplTable[i]) != NULL )
        {
            Reset( pAppl );
            m_pApplTable[i] = NULL;
            delete pAppl;
        }
    }

    if( m_ulMuSem )
    {
        (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

        (*m_pOsServices->pfnDeleteSem) (m_ulMuSem);
        m_ulMuSem = 0;
    }
} // ~ATM_APPL_TABLE


//**************************************************************************
// Function Name: RegisterAppl
// Description  : Finds an unused index in a table of application objects and
//                assigns the application object pointer to it.
// Returns      : None.
//**************************************************************************
void ATM_APPL_TABLE::RegisterAppl( ATM_APPL *pAppl )
{
    UINT32 ulIdx = 0;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    for( UINT32 i = 1; i <= MAX_APPL_TABLE_ENTRIES; i++ )
    {
        if( m_pApplTable[i] == pAppl )
        {
            // Application is already registered.
            ulIdx = i;
            break;
        }
        else
            if( m_pApplTable[i] == NULL && ulIdx == 0 )
                ulIdx = i;
    }

    if( ulIdx != 0 )
        m_pApplTable[ulIdx] = pAppl;

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);
} // RegisterAppl


//**************************************************************************
// Function Name: UnregisterAppl
// Description  : Resets the handle array index that contains the specified
//                ATM_APPL object.
// Returns      : None.
//**************************************************************************
void ATM_APPL_TABLE::UnregisterAppl( ATM_APPL *pAppl )
{
    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    for( UINT32 i = 1; i <= MAX_APPL_TABLE_ENTRIES; i++ )
    {
        if( m_pApplTable[i] == pAppl )
        {
            m_pApplTable[i] = NULL;
            break;
        }
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);
} // UnregisterAppl


//**************************************************************************
// Function Name: EnumAppl
// Description  : Returns the next ATM_APPL object in a list.
// Returns      : ATM_APPL object or NULL.
//**************************************************************************
ATM_APPL *ATM_APPL_TABLE::EnumAppl( UINT32 *pulIdx )
{
    ATM_APPL *pAppl = 0;
    UINT32 i;

    for( i = *pulIdx + 1; i <= MAX_APPL_TABLE_ENTRIES; i++ )
        if( m_pApplTable[i] )
        {
            pAppl = m_pApplTable[i];
            break;
        }

    *pulIdx = i;

    return( pAppl );
} // EnumAppl


//**************************************************************************
// Function Name: SetByVccAddr
// Description  : Add a ATM_VCC_APPL object to the VCC address lookup table.
//                In the case of AAL2 VCCs, there can be more than one
//                ATM_VCC_APPL object for the AAL2 VCC.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL_TABLE::SetByVccAddr( PATM_VCC_ADDR pAddr,
    ATM_VCC_APPL *pAppl )
{
    BCMATM_STATUS baStatus = STS_RESOURCE_ERROR;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    for( UINT32 i = 0; i < MAX_VCCS; i++ )
    {
        if( m_VccAddrMap[i].pAppl == NULL )
        {
            VccAddrAtmAppl *pVaaa = &m_VccAddrMap[i];
            pVaaa->pAppl = pAppl;
            pVaaa->Addr.ulInterfaceId = pAddr->ulInterfaceId;
            pVaaa->Addr.usVpi = pAddr->usVpi;
            pVaaa->Addr.usVci = pAddr->usVci;

            baStatus = STS_SUCCESS;
            break;
        }
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( baStatus );
} // SetByVccAddr


//**************************************************************************
// Function Name: SetByShaperNum
// Description  : Add a ATM_TX_BDQ object to the shaper number lookup table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL_TABLE::SetTxQByShaperNum(UINT8 ucShaperNum,
    ATM_TX_BDQ *pTxQ)
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if(ucShaperNum < MAX_QUEUE_SHAPERS && m_pTxQShaperNums[ucShaperNum] == NULL)
        m_pTxQShaperNums[ucShaperNum] = pTxQ;
    else
        baStatus = STS_RESOURCE_ERROR;

    return( baStatus );
} // SetByShaperNum


//**************************************************************************
// Function Name: SetByRxVcid
// Description  : Add a ATM_VCC_APPL object to the receive VCID lookup table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL_TABLE::SetByRxVcid( UINT8 ucRxVcid, ATM_VCC_APPL *pAppl )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( ucRxVcid < MAX_VCIDS )
    {
        if( m_pRxVcids[ucRxVcid] == NULL )
            m_pRxVcids[ucRxVcid] = pAppl;
        else
            baStatus = STS_RESOURCE_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // SetByRxVcid


//**************************************************************************
// Function Name: GetByVccAddr
// Description  : Return the application object for a particular VCC address.
//                The first application object found is returned.  In the
//                case of AAL2 VCCs, there can be more than one application
//                object for the AAL2 VCC.
// Returns      : ATM_VCC_APPL object or NULL.
//**************************************************************************
ATM_VCC_APPL *ATM_APPL_TABLE::GetByVccAddr( PATM_VCC_ADDR pAddr )
{
    ATM_VCC_APPL *pAppl = NULL;
    PATM_VCC_ADDR pVccAddr;
    for( UINT32 i = 0; i < MAX_VCCS; i++ )
    {
        pVccAddr = &m_VccAddrMap[i].Addr;

        if( pVccAddr->ulInterfaceId == pAddr->ulInterfaceId &&
            pVccAddr->usVpi == pAddr->usVpi &&
            pVccAddr->usVci == pAddr->usVci )
        {
            pAppl = m_VccAddrMap[i].pAppl;
            break;
        }
    }

    return( pAppl );
} // GetByVccAddr


//**************************************************************************
// Function Name: Reset
// Description  : Removes all entries of the specified ATM_VCC_APPL object.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_APPL_TABLE::Reset( ATM_APPL *pAppl, UINT8 ucShaperNum )
{
    UINT32 i;

    if( ucShaperNum < MAX_QUEUE_SHAPERS)
        m_pTxQShaperNums[ucShaperNum] = NULL;

    for( i = 0; i < MAX_VCIDS; i++ )
    {
        if( m_pRxVcids[i] == pAppl )
            m_pRxVcids[i] = NULL;
    }

    for( i = 0; i < MAX_VCCS; i++ )
    {
        if( m_VccAddrMap[i].pAppl == pAppl )
        {
            VccAddrAtmAppl *pVaaa = &m_VccAddrMap[i];
            pVaaa->Addr.ulInterfaceId = 0;
            pVaaa->Addr.usVpi = pVaaa->Addr.usVci = 0;
            pVaaa->pAppl = NULL;
        }
    }

    return( STS_SUCCESS );
} // Reset

