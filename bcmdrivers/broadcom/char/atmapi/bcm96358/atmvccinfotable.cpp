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
// File Name  : AtmVccInfoTable.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM VCC
//              information lookup table class.
//
// Updates    : 01/03/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"


//**************************************************************************
// ATM_VCC_INFO_TABLE Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_VCC_INFO_TABLE
// Description  : Constructor for the ATM VCC information table lookup class.
// Returns      : None.
//**************************************************************************
ATM_VCC_INFO_TABLE::ATM_VCC_INFO_TABLE( ATM_OS_SERVICES *pOsServices )
{
    m_pOsServices = pOsServices;
    m_ulMuSem = 0;
    memset( m_pVccInfos, 0x00, sizeof(m_pVccInfos) );
} // ATM_VCC_INFO_TABLE


//**************************************************************************
// Function Name: ~ATM_VCC_INFO_TABLE
// Description  : Destructor for the ATM VCC information table lookup class.
// Returns      : None.
//**************************************************************************
ATM_VCC_INFO_TABLE::~ATM_VCC_INFO_TABLE( void )
{
    // Free all ATM_VCC_INFO structures.
    ATM_VCC_INFO *pInfo, *pInfo2;
    int nSize = sizeof(m_pVccInfos) / sizeof(ATM_VCC_INFO *);
    for( int i = 0; i < nSize; i++ )
    {
        pInfo = m_pVccInfos[i];

        while( pInfo )
        {
            pInfo2 = pInfo->pNext;
            (*m_pOsServices->pfnFree) (pInfo);
            pInfo = pInfo2;
        }

        m_pVccInfos[i] = NULL;
    }

    if( m_ulMuSem )
    {
        (*m_pOsServices->pfnDeleteSem) (m_ulMuSem);
        m_ulMuSem = 0;
    }
} // ~ATM_VCC_INFO_TABLE


//**************************************************************************
// Function Name: AddReplace
// Description  : Adds or replaces an ATM_VCC_INFO structure in the lookup
//                table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_INFO_TABLE::AddReplace( PATM_VCC_ADDR pAddr,
    PATM_VCC_CFG pCfg, ATM_INTERFACE *pInterface )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    PATM_VCC_ADDR pInfoAddr;
    UINT32 ulHashValue = (UINT32) (pAddr->usVci & (MAX_VCC_INFOS - 1));
    ATM_VCC_INFO *pInfo = m_pVccInfos[ulHashValue];

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Look for an existing record with the same VCC address.
    while( pInfo )
    {
        pInfoAddr = &pInfo->Addr;

        if( pAddr->ulInterfaceId == pInfoAddr->ulInterfaceId &&
            pAddr->usVpi == pInfoAddr->usVpi &&
            pAddr->usVci == pInfoAddr->usVci )
        {
            // ATM_VCC_INFO record was found with the same VCC address.
            // Replace the VCC information with the supplied values.
            memcpy( &pInfo->Addr, pAddr, sizeof(pInfo->Addr) );
            memcpy( &pInfo->Cfg, pCfg, sizeof(pInfo->Cfg) );
            pInfo->Stats.ulStructureId = ID_ATM_VCC_STATS;
            pInfo->Stats.ulAalType = pInfo->Cfg.ulAalType;
            pInfo->pInterface = pInterface;
            break;
        }

        pInfo = pInfo->pNext;
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    // If an existing record was not found, create a new one.
    if( pInfo == NULL )
    {
        pInfo = (ATM_VCC_INFO *)
            (*m_pOsServices->pfnAlloc) (sizeof(ATM_VCC_INFO));

        if( pInfo )
        {
            memset( pInfo, 0x00, sizeof(ATM_VCC_INFO) );
            memcpy( &pInfo->Addr, pAddr, sizeof(pInfo->Addr) );
            memcpy( &pInfo->Cfg, pCfg, sizeof(pInfo->Cfg) );
            pInfo->Stats.ulAalType = pInfo->Cfg.ulAalType;
            pInfo->pInterface = pInterface;

            (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);
            pInfo->pNext = m_pVccInfos[ulHashValue];
            m_pVccInfos[ulHashValue] = pInfo;
            (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }

    return( baStatus );
} // AddReplace


//**************************************************************************
// Function Name: Remove
// Description  : Removes an ATM_VCC_INFO structure from the lookup table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
void ATM_VCC_INFO_TABLE::Remove( PATM_VCC_ADDR pAddr )
{
    PATM_VCC_ADDR pInfoAddr;
    UINT32 ulHashValue = (UINT32) (pAddr->usVci & (MAX_VCC_INFOS - 1));
    ATM_VCC_INFO *pInfo = m_pVccInfos[ulHashValue];
    ATM_VCC_INFO *pInfoPrev = NULL;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Find the ATM_VCC_INFO record to remove.
    while( pInfo )
    {
        pInfoAddr = &pInfo->Addr;
        if( pAddr->ulInterfaceId == pInfoAddr->ulInterfaceId &&
            pAddr->usVpi == pInfoAddr->usVpi &&
            pAddr->usVci == pInfoAddr->usVci )
        {
            // ATM_VCC_INFO record was found.  Remove it from the chain.
            if( pInfoPrev == NULL )
                m_pVccInfos[ulHashValue] = pInfo->pNext;
            else
                pInfoPrev->pNext = pInfo->pNext;

            // Free up the memory.
            (*m_pOsServices->pfnFree) (pInfo);
            break;
        }

        pInfoPrev = pInfo;
        pInfo = pInfo->pNext;
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);
} // Remove


//**************************************************************************
// Function Name: Get
// Description  : Returns an ATM_VCC_INFO structure from the lookup table
//                but does not remove it from the table.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
ATM_VCC_INFO *ATM_VCC_INFO_TABLE::Get( PATM_VCC_ADDR pAddr )
{
    PATM_VCC_ADDR pInfoAddr;
    UINT32 ulHashValue = (UINT32) (pAddr->usVci & (MAX_VCC_INFOS - 1));
    ATM_VCC_INFO *pInfo = m_pVccInfos[ulHashValue];

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Find the ATM_VCC_INFO record to remove.
    while( pInfo )
    {
        pInfoAddr = &pInfo->Addr;
        if( pAddr->ulInterfaceId == pInfoAddr->ulInterfaceId &&
            pAddr->usVpi == pInfoAddr->usVpi &&
            pAddr->usVci == pInfoAddr->usVci )
        {
            break;
        }

        pInfo = pInfo->pNext;
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    return( pInfo );
} // Get


//**************************************************************************
// Function Name: GetAllAddrs
// Description  : Copies the VCC address in all ATM_VCC_INFO structures
//                for a specified interface to the supplied array of VCC
//                addresses.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_VCC_INFO_TABLE::GetAllAddrs( UINT32 ulInterfaceId,
    PATM_VCC_ADDR pAddrs, UINT32 ulNumAddrs, UINT32 *pulNumReturned )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulNumReturned = 0;
    ATM_VCC_INFO *pInfo;

    (*m_pOsServices->pfnRequestSem) (m_ulMuSem, MAX_TIMEOUT);

    // Search the all table elements.
    int nSize = sizeof(m_pVccInfos) / sizeof(ATM_VCC_INFO *);
    for( int i = 0; i < nSize; i++ )
    {
        pInfo = m_pVccInfos[i];

        while( pInfo )
        {
            if( pInfo->Addr.ulInterfaceId == ulInterfaceId )
            {
                // Copy the VCC address if the caller supplied array has space
                // for it.
                if( ulNumReturned < ulNumAddrs )
                {
                    memcpy( pAddrs + ulNumReturned, &pInfo->Addr,
                        sizeof(ATM_VCC_ADDR) );
                }
                else
                    baStatus = STS_PARAMETER_ERROR;

                ulNumReturned++;
            }

            pInfo = pInfo->pNext;
        }
    }

    (*m_pOsServices->pfnReleaseSem) (m_ulMuSem);

    // *pulNumReturned contains the number of VCC addresses returned if the
    // return status is STS_SUCCESS or it contains the required array size
    // if the return status is STS_PARAMETER_ERROR.
    *pulNumReturned = ulNumReturned;

    return( baStatus );
} // GetAllAddrs

