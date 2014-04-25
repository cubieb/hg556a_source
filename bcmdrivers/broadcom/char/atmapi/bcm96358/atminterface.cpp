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
// File Name  : AtmInterface.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM interface
//              class.  This class handles the processing that is associated
//              with an ATM port.
//
// Updates    : 01/03/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"
#include "bcm_map.h"
#if defined(VXWORKS)
#include "..\adsl\BcmCore\AdslCoreMap.h"
#else
#include "AdslCoreMap.h"
#endif


//**************************************************************************
// ATM_INTERFACE Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_INTERFACE
// Description  : Constructor for the ATM interface class.
// Returns      : None.
//**************************************************************************
ATM_INTERFACE::ATM_INTERFACE( void )
{
    m_pOsServices = NULL;
    memset( &m_Stats, 0x00, sizeof(m_Stats) );
    m_Stats.ulStructureId = ID_ATM_INTERFACE_STATS;
    memset( &m_Cfg, 0x00, sizeof(m_Cfg) );
    m_Cfg.ulStructureId = ID_ATM_INTERFACE_CFG;
    m_Cfg.ulIfAdminStatus = ADMSTS_DOWN;
    m_ucPhyPortNum = PHY_0;
    m_ulInterfaceId = 0;
    m_ucPortType = 0;
    m_ulInterfaceId = 0;
    m_ulAdslCfg = 0;
    m_ulUtopiaCfg = 0;
    m_ucUtopiaFlags = 0;
    m_pApplTable = NULL;
    m_ulLinkState = LINK_DOWN;
    m_pOamF5EToE = NULL;
    m_pOamF5Seg = NULL;
    m_pOamF4EToE = NULL;
    m_pOamF4Seg = NULL;
    m_pRm = NULL;
    m_pErrorCells = NULL;
    m_pTransparentCells = NULL;
    m_ulStatsLock = 0;
    m_ulAdslTxDescrTableAddr = 0;
    m_ulAdslRxDescrTableAddr = 0;
    m_ulPhyPort1Enabled = 0;
    m_pSoftSar = NULL;
} // ATM_INTERFACE


//**************************************************************************
// Function Name: ~ATM_INTERFACE
// Description  : Destructor for the ATM interface class.
// Returns      : None.
//**************************************************************************
ATM_INTERFACE::~ATM_INTERFACE( void )
{
    Uninitialize();
} // ~ATM_INTERFACE


//**************************************************************************
// Function Name: Initialize
// Description  : Saves the supplied parameters to local member data.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::Initialize( ATM_OS_SERVICES *pOsServices,
    UINT8 ucPhyPortNum, PATM_PORT_CFG pPortCfg, UINT32 ulAdslCfg,
    ATM_APPL_TABLE *pApplTable, void *pSoftSar )
{
    BCMATM_STATUS baStatus;

    m_pOsServices = pOsServices;
    m_ulInterfaceId = pPortCfg->ulInterfaceId;
    m_ucPhyPortNum = ucPhyPortNum & (AP_MAX_PORTS - 1);
    m_ucPortType = pPortCfg->ucPortType;
    m_ulAdslCfg = ulAdslCfg;
    m_pApplTable = pApplTable;
    m_pSoftSar = pSoftSar;

    /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifndef CONFIG_BCM_VDSL
    if( (m_ucPortType == PT_ADSL_INTERLEAVED || m_ucPortType == PT_ADSL_FAST) &&
        m_pOsServices->pfnAdslSetVcEntry == NULL )
    {
        // ADSL is not initialized.
        baStatus = STS_LINK_DOWN;
    }
    else
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
    /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
    {
        if( (m_ulStatsLock = (*m_pOsServices->pfnCreateSem) (1)) == 0 )
            baStatus = STS_ERROR;
        else
        {
            if( (m_ucPortType == PT_ADSL_INTERLEAVED && m_ucPhyPortNum!=PHY_0) ||
                (m_ucPortType == PT_ADSL_FAST && m_ucPhyPortNum != PHY_1) )
            {
                baStatus = STS_PARAMETER_ERROR;
            }
            else
            {
                switch( m_ucPortType )
                {
                case PT_ADSL_INTERLEAVED:
                case PT_ADSL_FAST:
                case PT_LOOPBACK:
                    if((*ADSL_INTERNAL_PORTS_REG&ADSL_PORT1_EN) == ADSL_PORT1_EN)
                        m_ulPhyPort1Enabled = 1;
                    else
                        m_ulPhyPort1Enabled = 0;
                    baStatus = AdslInit();
                    break;

                case PT_UTOPIA:
                    UtopiaInit( pPortCfg->ucPortFlags );
                    baStatus = STS_SUCCESS;
                    break;

                case PT_DISABLED:
                    baStatus = STS_SUCCESS;
                    break;

                default:
                    baStatus = STS_PARAMETER_ERROR;
                    break;
                }
 
                if( baStatus == STS_SUCCESS )
                {
                    ConfigureAtmProcessor( 0 );
                    if( m_ucPortType != PT_DISABLED )
                        m_Cfg.ulIfAdminStatus = ADMSTS_UP;
                }
            }
        }
    }

    return( baStatus );
} // Initialize


//**************************************************************************
// Function Name: Uninitialize
// Description  : Uninitializes the ATM interface.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::Uninitialize( void )
{
    if( m_pOsServices )
    {
        ConfigureAtmProcessor( 0 );

        if( m_ulStatsLock )
        {
            (*m_pOsServices->pfnDeleteSem) (m_ulStatsLock);
            m_ulStatsLock = 0;
        }

        if( m_ulAdslTxDescrTableAddr )
        {
            (*m_pOsServices->pfnFree) ((char *) m_ulAdslTxDescrTableAddr);
            m_ulAdslTxDescrTableAddr = 0;
        }

        if( m_ulAdslRxDescrTableAddr )
        {
            (*m_pOsServices->pfnFree) ((char *) m_ulAdslRxDescrTableAddr);
            m_ulAdslRxDescrTableAddr = 0;
        }
    }

    return( STS_SUCCESS );
} // Uninitialize


//**************************************************************************
// Function Name: SetCfg
// Description  : Saves the caller supplied configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::SetCfg( PATM_INTERFACE_CFG pCfg )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( pCfg->ulStructureId == ID_ATM_INTERFACE_CFG )
    {
        if( m_ucPortType != PT_DISABLED )
        {
            memcpy( &m_Cfg, pCfg, sizeof(ATM_INTERFACE_CFG) );

            if( m_ucPortType == PT_UTOPIA && m_Cfg.ulPortType == PT_UTOPIA &&
                m_Cfg.ulPortFlags != m_ucUtopiaFlags  )
            {
                // The UTOPIA port flags have changed.
                UtopiaInit( (UINT8) m_Cfg.ulPortFlags );
            }
            else
                if( m_ucPortType == PT_ADSL_INTERLEAVED &&
                    m_Cfg.ulPortType == PT_UTOPIA )
                {
                    // The ATM port changed from ADSL to UTOPIA.
                    UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;
                    pulAdslBase[RCV_CTL_INTR] &= ~0x00000001;
                    ConfigureAtmProcessor( 0 );
                    AP_REGS->ulAdslLastDescCfg = m_ulAdslCfg =
                        ALDC_TX_UTOPIA_UTOPIA | ALDC_RX_UTOPIA_UTOPIA;
                    m_ucPortType = PT_UTOPIA;
                    UtopiaInit( (UINT8) m_Cfg.ulPortFlags );
                }
                else
                    if( m_ucPhyPortNum == PHY_0 &&
                        m_ucPortType == PT_UTOPIA &&
                        m_Cfg.ulPortType == PT_ADSL_INTERLEAVED )
                    {
                        // The ATM port changed from UTOPIA to ADSL.
                        ConfigureAtmProcessor( 0 );
                        m_ucPortType = PT_ADSL_INTERLEAVED;
                        AP_REGS->ulAdslLastDescCfg = m_ulAdslCfg =
                            ALDC_TX_ADSL_ADSL | ALDC_RX_ADSL_ADSL;
                        baStatus = AdslInit();
                    }

            if( m_Cfg.ulIfAdminStatus == ADMSTS_UP )
                ConfigureAtmProcessor( 1 );
            else
                ConfigureAtmProcessor( 0 );
        }
        else
            baStatus = STS_STATE_ERROR;
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // SetCfg


//**************************************************************************
// Function Name: GetCfg
// Description  : Returns the current configuration record.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::GetCfg( PATM_INTERFACE_CFG pCfg,
    ATM_VCC_INFO_TABLE *pVccInfo )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( pCfg->ulStructureId == ID_ATM_INTERFACE_CFG )
    {
        memcpy( pCfg, &m_Cfg, sizeof(ATM_INTERFACE_CFG) );
        pCfg->ulIfOperStatus =
            (IsLinkUp() && IsInterfaceUp()) ? OPRSTS_UP : OPRSTS_DOWN;
        pCfg->ulPortType = m_ucPortType;
        pCfg->ulPortFlags = m_ucUtopiaFlags;

        // Calculate the number of configured VCCs for this interface.
        pCfg->ulAtmInterfaceConfVccs = 0;
        UINT32 ulNumAddrs = 0;
        pVccInfo->GetAllAddrs( m_ulInterfaceId, NULL, 0, &ulNumAddrs );
        if( ulNumAddrs )
        {
            PATM_VCC_ADDR pAddrs = (PATM_VCC_ADDR)
                (*m_pOsServices->pfnAlloc) (sizeof(ATM_VCC_ADDR) * ulNumAddrs);
            if( pAddrs )
            {
                UINT32 i;
                PATM_VCC_ADDR pAddr;
                pVccInfo->GetAllAddrs( m_ulInterfaceId, pAddrs, ulNumAddrs,
                    &ulNumAddrs );
                for( i = 0, pAddr = pAddrs; i < ulNumAddrs; i++, pAddr++ )
                {
                    if( pAddr->ulInterfaceId == m_ulInterfaceId )
                        pCfg->ulAtmInterfaceConfVccs++;
                }

                (*m_pOsServices->pfnFree) (pAddrs);
            }
        }
    }
    else
        baStatus = STS_PARAMETER_ERROR;

    return( baStatus );
} // GetCfg


//**************************************************************************
// Function Name: GetStats
// Description  : Returns the current statistics.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::GetStats( PATM_INTERFACE_STATS pStats,
    UINT32 ulReset )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( pStats->ulStructureId == ID_ATM_INTERFACE_STATS && m_ulStatsLock )
    {
        (*m_pOsServices->pfnRequestSem) (m_ulStatsLock, MAX_TIMEOUT);

#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
        UINT32 ulRxAal5Octs=0, ulTxAal5Octs=0, ulRxAal0Octs=0, ulTxAal0Octs=0;

        switch( m_ucPhyPortNum )
        {
        case PHY_0:
            // The ATM service category determines which port that data is sent.
            // Therefore, sum the transmit cell count for both ports 0 and 1.
            // Data is always received on port 0.
            ulRxAal5Octs = AP_REGS->ulRxAal5CellCnt[m_ucPhyPortNum] * 48;
            ulTxAal5Octs = (AP_REGS->ulTxAal5CellCnt[PHY_0] +
                AP_REGS->ulTxAal5CellCnt[PHY_1]) * 48;
            ulRxAal0Octs = AP_REGS->ulRxAal0CellCnt[m_ucPhyPortNum] * 48;
            ulTxAal0Octs = (AP_REGS->ulTxAal0CellCnt[PHY_0] +
                AP_REGS->ulTxAal0CellCnt[PHY_1]) * 48;
            break;

        case PHY_2:
        case PHY_3:
            ulRxAal5Octs = AP_REGS->ulRxAal5CellCnt[m_ucPhyPortNum] * 48;
            ulTxAal5Octs = AP_REGS->ulTxAal5CellCnt[m_ucPhyPortNum] * 48;
            ulRxAal0Octs = AP_REGS->ulRxAal0CellCnt[m_ucPhyPortNum] * 48;
            ulTxAal0Octs = AP_REGS->ulTxAal0CellCnt[m_ucPhyPortNum] * 48;
            break;
        }

        // Assign statistic values maintained by the ATM Processor.
        m_Stats.Aal5IntfStats.ulIfInOctets += ulRxAal5Octs;
        m_Stats.Aal5IntfStats.ulIfOutOctets += ulTxAal5Octs;
        m_Stats.Aal0IntfStats.ulIfInOctets += ulRxAal0Octs;
        m_Stats.Aal0IntfStats.ulIfOutOctets += ulTxAal0Octs;

        m_Stats.AtmIntfStats.ulIfInOctets += ulRxAal5Octs + ulRxAal0Octs;
        m_Stats.AtmIntfStats.ulIfOutOctets += ulTxAal5Octs + ulTxAal0Octs;
#elif defined(CONFIG_BCM96338)
        m_Stats.AtmIntfStats.ulIfInOctets =
            m_Stats.Aal5IntfStats.ulIfInOctets +
            m_Stats.Aal0IntfStats.ulIfInOctets;
        m_Stats.AtmIntfStats.ulIfOutOctets =
            m_Stats.Aal5IntfStats.ulIfOutOctets +
            m_Stats.Aal0IntfStats.ulIfOutOctets;

#if 0 /* DEBUG */
        ((ATM_SOFT_SAR *) m_pSoftSar)->DebugDumpInfo();
#endif
#endif

        memcpy( pStats, &m_Stats, sizeof(ATM_INTERFACE_STATS) );

        if( ulReset )
        {
            memset( &m_Stats, 0x00, sizeof(ATM_INTERFACE_STATS) );
            m_Stats.ulStructureId = ID_ATM_INTERFACE_STATS;
        }

        (*m_pOsServices->pfnReleaseSem) (m_ulStatsLock);
    }
    else
        baStatus = (m_ulStatsLock == 0) ? STS_STATE_ERROR : STS_PARAMETER_ERROR;

    return( baStatus );
} // GetStats


//**************************************************************************
// Function Name: SetLinkInfo
// Description  : Saves the current link state and if the link is up, also
//                saves the line rate and updates the Cell Interval Timer
//                ATM Processor register.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::SetLinkInfo( UINT32 ulLinkState )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;

    if( m_Cfg.ulIfAdminStatus == ADMSTS_UP && m_ucPortType != PT_DISABLED )
    {
        if( (m_ulLinkState = ulLinkState) == LINK_UP )
        {
            // Enable ATM receive.
            AP_REGS->ulRxAtmCfg[m_ucPhyPortNum] |= ATMR_RX_PORT_EN;

            // If ADSL PHY is configured, enable receive for port 0
            // (PT_ADSL_INTERLEAVED).  Receive for port 1 (PT_ADSL_FAST) is
            // dedicated for ADSL diagnostics.
            if( m_ucPortType == PT_ADSL_INTERLEAVED ||
                m_ucPortType == PT_LOOPBACK )
            {
                pulAdslBase[RCV_CTL_INTR] |= 0x00000001;

                // Initialize ATM ADSL PHY register for port 0. The receive
                // cell counter is initialized to three cells.
                *((UINT8 *) &AP_REGS->ulAdslLastDescCellCnts + 1) = 3;
            }

#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
            // Enable UTOPIA if configured.

            if( m_ucPortType == PT_UTOPIA) {
               AP_REGS->ulUtopiaCfg = m_ulUtopiaCfg;
            }
#endif
        }
        else
        {
            // If ADSL PHY is configured, disable transmit and receive.
            if( m_ucPortType == PT_ADSL_INTERLEAVED ||
                m_ucPortType == PT_LOOPBACK )
            {
                pulAdslBase[RCV_CTL_INTR] &= ~0x00000001;
            }

            // Disable ATM receive.
            AP_REGS->ulRxAtmCfg[m_ucPhyPortNum] &= ~ATMR_RX_PORT_EN;

#if defined(CONFIG_BCM96358)||defined(CONFIG_BCM96348)
            if( m_ucPortType == PT_UTOPIA) {
               // Disable UTOPIA transmit and receive.
               AP_REGS->ulUtopiaCfg = 0;
            }
#endif
        }
    }
    else
        baStatus = STS_STATE_ERROR;

    m_Cfg.ulIfLastChange = (*m_pOsServices->pfnGetSystemTick) () / 10;

    return( baStatus );
} // SetLinkInfo


//**************************************************************************
// Function Name: SetMgmtCellAppl
// Description  : Assigns a management application object to a particular
//                type of OAM or RM cell.  This assignment is used by this
//                interface object to route received OAM and RM cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::SetMgmtCellAppl( ATM_MGMT_APPL *pMgmtAppl,
    UINT32 ulFlags )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( (ulFlags & AVAP_ALLOW_OAM_F5_SEGMENT_CELLS) != 0 )
    {
        if( m_pOamF5Seg == NULL )
            m_pOamF5Seg = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F5_END_TO_END_CELLS) != 0 )
    {
        if( m_pOamF5EToE == NULL )
            m_pOamF5EToE = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( (ulFlags & AVAP_ALLOW_RM_CELLS) != 0 )
    {
        if( m_pRm == NULL )
            m_pRm = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F4_SEGMENT_CELLS) != 0 )
    {
        if( m_pOamF4Seg == NULL )
            m_pOamF4Seg = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F4_END_TO_END_CELLS) != 0 )
    {
        if( m_pOamF4EToE == NULL )
            m_pOamF4EToE = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( (ulFlags & AVAP_ALLOW_CELLS_WITH_ERRORS) != 0 )
    {
        if( m_pErrorCells == NULL )
            m_pErrorCells = pMgmtAppl;
        else
            baStatus = STS_IN_USE;
    }

    if( baStatus != STS_SUCCESS )
        ResetMgmtCellAppl( pMgmtAppl, ulFlags );

    return( baStatus );
} // SetMgmtCellAppl


//**************************************************************************
// Function Name: ResetMgmtCellAppl
// Description  : Clears the management application object assignment to a
//                particular type of OAM or RM cell.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::ResetMgmtCellAppl( ATM_MGMT_APPL *pMgmtAppl,
    UINT32 ulFlags )
{
    if( (ulFlags & AVAP_ALLOW_OAM_F5_SEGMENT_CELLS) != 0 &&
        m_pOamF5Seg == pMgmtAppl )
    {
        m_pOamF5Seg = NULL;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F5_END_TO_END_CELLS) != 0 &&
        m_pOamF5EToE == pMgmtAppl )
    {
        m_pOamF5EToE = NULL;
    }

    if( (ulFlags & AVAP_ALLOW_RM_CELLS) != 0 && m_pRm == pMgmtAppl )
    {
        m_pRm = NULL;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F4_SEGMENT_CELLS) != 0 &&
        m_pOamF4Seg == pMgmtAppl )
    {
        m_pOamF4Seg = NULL;
    }

    if( (ulFlags & AVAP_ALLOW_OAM_F4_END_TO_END_CELLS) != 0 &&
        m_pOamF4EToE == pMgmtAppl )
    {
        m_pOamF4EToE = NULL;
    }

    if( (ulFlags & AVAP_ALLOW_CELLS_WITH_ERRORS) != 0 &&
        m_pErrorCells == pMgmtAppl )
    {
        m_pErrorCells = NULL;
    }

    return( STS_SUCCESS );
} // ResetMgmtCellAppl


//**************************************************************************
// Function Name: SetTransparentAppl
// Description  : Assigns an application object to handle transparent
//                cells.  This assignment is used by this interface object
//                to route received cells that have a VPI/VCI error and
//                therefore believed to be transparent cells.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::SetTransparentAppl(
    ATM_TRANSPARENT_APPL *pTransparentAppl )
{
    BCMATM_STATUS baStatus = STS_SUCCESS;

    if( m_pTransparentCells == NULL )
        m_pTransparentCells = pTransparentAppl;
    else
        baStatus = STS_IN_USE;

    return( baStatus );
} // SetTransparentAppl


//**************************************************************************
// Function Name: ResetTransparentAppl
// Description  : Clears the transparent cell handler.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::ResetTransparentAppl(
    ATM_TRANSPARENT_APPL *pTransparentAppl )
{
    if( m_pTransparentCells == pTransparentAppl )
        m_pTransparentCells = NULL;

    return( STS_SUCCESS );
} // ResetTransparentAppl


//**************************************************************************
// Function Name: Receive
// Description  : Processes a management or error cell.  This function is
//                called by a ATM_VCC_APPL object if it does not handle
//                the cell type.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_INTERFACE::Receive( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDataParms )
{
    const UINT8 ucOamF5Segment  = (0x04 << 1);
    const UINT8 ucOamF5EndToEnd = (0x05 << 1);

    BCMATM_STATUS baStatus = STS_SUCCESS;
    UINT32 ulDataProcessed = 1;
    UINT32 ulVci;
    UINT32 ulLen = pDataParms->pAtmBuffer->ulDataLen - 4; // minus ATM header
    UINT8 *pBuf = pDataParms->pAtmBuffer->pDataBuf;
    ATM_VCC_ADDR VccAddr;

    switch( pDataParms->baReceiveStatus )
    {
    case STS_PKTERR_INVALID_VPI_VCI:
        // A VPI/VCI error can occur (1) if the cell is an OAM F4 (and the OAM
        // F4 VCI is not registered), (2) if the cell is for an application
        // (such as a software ATM stack) that is receiving transparent ATM
        // cells or (3) if VPI and/or VCI is really corrupted.  If the cell is
        // OAM F4, give the cell to the ATM_MGMT_APPL that is registered to
        // receive it.  Otherwise, give the cell to an ATM_TRANSPARENT_APPL
        // object if one is registered or to a ATM_MGMT_APPL that is handling
        // error cells.
        ulVci = *(UINT32 *) &pDataParms->pAtmBuffer->pDataBuf[0];
        ulVci = (ulVci >> 4) & 0xffff;

        if( ulVci == VCI_OAM_F4_SEGMENT || ulVci == VCI_OAM_F4_END_TO_END )
            ProcessOamCell( pDataParms );

        if( ulVci == VCI_OAM_F4_SEGMENT && m_pOamF4Seg )
        {
            ChangeCircuitType( &VccAddr, pDataParms, CT_OAM_F4_ANY );
            m_Stats.Aal0IntfStats.ulIfInOctets += ulLen;
            m_Stats.Aal0IntfStats.ulIfInUcastPkts++;
            baStatus = m_pOamF4Seg->Receive( &VccAddr, pDataParms );
        }
        else if( ulVci == VCI_OAM_F4_END_TO_END && m_pOamF4EToE )
        {
            ChangeCircuitType( &VccAddr, pDataParms, CT_OAM_F4_ANY );
            m_Stats.Aal0IntfStats.ulIfInOctets += ulLen;
            m_Stats.Aal0IntfStats.ulIfInUcastPkts++;
            baStatus = m_pOamF4EToE->Receive( &VccAddr, pDataParms );
        }
        else if( (pBuf[3] & ucOamF5Segment) == ucOamF5Segment && m_pOamF5Seg )
        {
            ChangeCircuitType( &VccAddr, pDataParms, CT_OAM_F5_SEGMENT );
            m_Stats.Aal0IntfStats.ulIfInOctets += ulLen;
            m_Stats.Aal0IntfStats.ulIfInUcastPkts++;
            baStatus = m_pOamF5Seg->Receive( &VccAddr, pDataParms );
        }
        else if( (pBuf[3] & ucOamF5EndToEnd) == ucOamF5EndToEnd && m_pOamF5EToE )
        {
            ChangeCircuitType( &VccAddr, pDataParms, CT_OAM_F5_END_TO_END );
            m_Stats.Aal0IntfStats.ulIfInOctets += ulLen;
            m_Stats.Aal0IntfStats.ulIfInUcastPkts++;
            baStatus = m_pOamF5EToE->Receive( &VccAddr, pDataParms );
        }
        else if( m_pTransparentCells != NULL )
            baStatus = m_pTransparentCells->Receive( pDataParms );
        else if( m_pErrorCells != NULL )
        {
            VccAddr.ulInterfaceId = m_ulInterfaceId;
            VccAddr.usVpi = ((*(UINT16 *)
                &pDataParms->pAtmBuffer->pDataBuf[0]) >> 4) & 0xff; 
            VccAddr.usVci = (UINT16) ulVci;
            baStatus = m_pErrorCells->Receive(&VccAddr, pDataParms);
        }
        else
            ulDataProcessed = 0;
        break;

    case STS_SUCCESS:
        switch( pDataParms->ucCircuitType )
        {
        case CT_OAM_F5_SEGMENT:
            if( m_pOamF5Seg != NULL )
                baStatus = m_pOamF5Seg->Receive( pVccAddr, pDataParms );
            else
                ulDataProcessed = 0;
            break;

        case CT_OAM_F5_END_TO_END:
            if( m_pOamF5EToE != NULL )
                baStatus = m_pOamF5EToE->Receive( pVccAddr, pDataParms );
            else
                ulDataProcessed = 0;
            break;

        case CT_RM:
            if( m_pRm != NULL )
                baStatus = m_pRm->Receive( pVccAddr, pDataParms );
            else
                ulDataProcessed = 0;
            break;

        default:
            ulDataProcessed = 0;
            break;
        }
        break;

    default: // error status
        if( m_pErrorCells != NULL )
            baStatus = m_pErrorCells->Receive( pVccAddr, pDataParms );
        else
            ulDataProcessed = 0;
        break;
    }

    // If the received data was not handled, free the buffer.
    if( ulDataProcessed == 0 )
        (*pDataParms->pFnFreeDataParms) (pDataParms);

    return( baStatus );
} // Receive


//**************************************************************************
// Function Name: AdslInit
// Description  : Initializes the ADSL registers.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
#if !defined(CONFIG_ATM_SOFTSAR)
BCMATM_STATUS ATM_INTERFACE::AdslInit( void )
{
    const UINT32 ulBoundary = 4096;
    const UINT32 ulDescrTableSize = 8;
    const UINT32 ulDescrBufSize = 64;
    const UINT32 ulDescrEndOfTable = 0x10000000;

    struct DescrTable
    {
        UINT32 ulCountFlags;
        UINT32 ulFifoAddr;
    };

    BCMATM_STATUS baStatus = STS_ALLOC_ERROR;

    m_ulUtopiaCfg = 0;

    // The ADSL descriptor tables are 64 bytes and must be aligned on a 4K
    // boundary.
    if( m_ulAdslTxDescrTableAddr == 0 )
    {
        m_ulAdslTxDescrTableAddr = (UINT32) (*m_pOsServices->pfnAlloc)
            ((ulDescrTableSize * sizeof(DescrTable) + ulBoundary));
    }

    if( m_ulAdslTxDescrTableAddr )
    {
        // Initialize the ADSL transmit descriptor table.
        UINT32 ulBase = (m_ucPhyPortNum == PHY_0)
            ? (UINT32) &AP_REGS->ulTxAdslDmaDescrTableP0[0]
            : (UINT32) &AP_REGS->ulTxAdslDmaDescrTableP1[0];
        DescrTable *pTableBase = (DescrTable *)
            ((m_ulAdslTxDescrTableAddr + ulBoundary) & ~(ulBoundary - 1));
        DescrTable *pTable = pTableBase;
        UINT32 i;

        for( i = 0; i < ulDescrTableSize; i++, pTable++ )
        {
            pTable->ulCountFlags = ulDescrBufSize;
            pTable->ulFifoAddr = ulBase;
            if( i == ulDescrTableSize - 1 )
                pTable->ulCountFlags |= ulDescrEndOfTable;
        }

        m_pOsServices->pfnFlushCache( pTableBase,
            ulDescrTableSize * sizeof(DescrTable) );

        // Set ADSL transmit descriptor tables.
        const UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;
        UINT32 *pulAdslTxAddr = (m_ucPhyPortNum == PHY_0)
            ? (UINT32 *) &pulAdslBase[XMT_ADDR_INTR]
            : (UINT32 *) &pulAdslBase[XMT_ADDR_FAST];

        *pulAdslTxAddr = CACHE_TO_PHYS(pTableBase);

        // The ADSL PHY receive DMA is only used by port 0.  Port 1 receive
        // DMA is used by ADSL diagnostics.
        if( m_ucPhyPortNum == PHY_0 )
        {
            if( m_ulAdslRxDescrTableAddr == 0 )
            {
                m_ulAdslRxDescrTableAddr = (UINT32) (*m_pOsServices->pfnAlloc)
                    ((ulDescrTableSize * sizeof(DescrTable) + ulBoundary));
            }

            if( m_ulAdslRxDescrTableAddr )
            {
                baStatus = STS_SUCCESS;

                // Initialize the ADSL receive descriptor table.
                ulBase = (UINT32) &AP_REGS->ulRxAdslDmaDescrTableP0[0];
                pTableBase = (DescrTable *)
                    ((m_ulAdslRxDescrTableAddr + ulBoundary) & ~(ulBoundary-1));
                pTable = pTableBase;

                for( i = 0; i < ulDescrTableSize; i++, pTable++ )
                {
                    pTable->ulCountFlags = ulDescrBufSize;
                    pTable->ulFifoAddr = ulBase;
                    if( i == ulDescrTableSize - 1 )
                        pTable->ulCountFlags |= ulDescrEndOfTable;
                }

                m_pOsServices->pfnFlushCache( pTableBase,
                    ulDescrTableSize * sizeof(DescrTable) );

                // Set ADSL receive descriptor tables.
                UINT32 *pulAdslRxAddr = (UINT32 *) &pulAdslBase[RCV_ADDR_INTR];
                *pulAdslRxAddr = CACHE_TO_PHYS(pTableBase);

                /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifdef CONFIG_BCM_VDSL
                if( m_ucPortType == PT_LOOPBACK && NULL != m_pOsServices->pfnAdslSetAtmLoopbackMode)
#else
                if( m_ucPortType == PT_LOOPBACK )
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
                {
                    (*m_pOsServices->pfnAdslSetAtmLoopbackMode) ();
                }
                /*End modify:l37298 2006-05-30 for deleting adsl feature*/	
            }
            else
            {
                (*m_pOsServices->pfnFree) ((char *) m_ulAdslTxDescrTableAddr);
                m_ulAdslTxDescrTableAddr = NULL;
            }
        }
        else
            baStatus = STS_SUCCESS;
    }

    return( baStatus );
} // AdslInit

#else
BCMATM_STATUS ATM_INTERFACE::AdslInit( void )
{
    BCMATM_STATUS baStatus = STS_PARAMETER_ERROR;

    m_ulUtopiaCfg = 0;

    // The ADSL PHY receive DMA is only used by port 0.  Port 1 receive
    // DMA is used by ADSL diagnostics.  The BCM6338 soft SAR only uses
    // ADSL PHY port 0 for transmit.
    if( m_ucPhyPortNum == PHY_0 )
    {
        const UINT32 ulBoundary = 4096;
        const UINT32 ulDescrTxTableSize = ATM_ADSL_TX_DESCR_TBL_SIZE;
        const UINT32 ulDescrRxTableSize = 8;
        const UINT32 ulDescrBufSize = 64;
        const UINT32 ulDescrEndOfTable = 0x10000000;

        struct DescrTable
        {
            UINT32 ulCountFlags;
            UINT32 ulAddr;
        };

        m_ulAdslTxHeadIdx = 0;
        m_ulAdslTxTableLength = ulDescrTxTableSize;

        // The ADSL transmit descriptor tables must be aligned on a 4K
        // boundary.  It is used to by the MIPS software SAR to pass cells
        // to the ADSL PHY.
        if( m_ulAdslTxDescrTableAddr == 0 )
        {
            m_ulAdslTxDescrTableAddr = (UINT32)
                (*m_pOsServices->pfnAlloc) ((ulBoundary * 2) +
                (m_ulAdslTxTableLength * ((sizeof(DescrTable)+ulDescrBufSize))));
        }

        if( m_ulAdslTxDescrTableAddr )
        {
            // Initialize the ADSL transmit descriptor table.
            DescrTable *pTableBase = (DescrTable *)
                ((m_ulAdslTxDescrTableAddr + ulBoundary) & ~(ulBoundary - 1));
            DescrTable *pTable = pTableBase;
            UINT8 *pDescrBufBase = (UINT8 *) pTable + ulBoundary;
            UINT8 *pDescrBuf = pDescrBufBase;
            UINT32 i;

            for( i = 0; i < m_ulAdslTxTableLength; i++, pTable++,
                 pDescrBuf += ulDescrBufSize )
            {
                pTable->ulCountFlags = ulDescrBufSize;
                pTable->ulAddr = CACHE_TO_PHYS(pDescrBuf);
                if( i == m_ulAdslTxTableLength - 1 )
                    pTable->ulCountFlags |= ulDescrEndOfTable;
            }

            m_pOsServices->pfnFlushCache( pTableBase,
                m_ulAdslTxTableLength * sizeof(DescrTable) );
            m_pOsServices->pfnFlushCache( pDescrBufBase,
                m_ulAdslTxTableLength * ulDescrBufSize );

            // Set ADSL transmit descriptor tables.
            const UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;
            UINT32 *pulAdslTxAddr = (m_ucPhyPortNum == PHY_0)
                ? (UINT32 *) &pulAdslBase[XMT_ADDR_INTR]
                : (UINT32 *) &pulAdslBase[XMT_ADDR_FAST];

            *pulAdslTxAddr = CACHE_TO_PHYS(pTableBase);

            // The ADSL receive descriptor tables must be aligned on a 4K
            // boundary.  It is used to pass cells from the ADSL PHY to the
            // ATM Processor (hardware SAR).
            if( m_ulAdslRxDescrTableAddr == 0 )
            {
                m_ulAdslRxDescrTableAddr = (UINT32) (*m_pOsServices->pfnAlloc)
                    ((ulDescrRxTableSize * sizeof(DescrTable) + ulBoundary));
            }

            if( m_ulAdslRxDescrTableAddr )
            {
                baStatus = STS_SUCCESS;

                // Initialize the ADSL receive descriptor table.
                UINT32 ulBase = (UINT32) &AP_REGS->ulRxAdslDmaDescrTableP0[0];
                pTableBase = (DescrTable *)
                    ((m_ulAdslRxDescrTableAddr + ulBoundary) & ~(ulBoundary-1));
                pTable = pTableBase;

                for( i = 0; i < ulDescrRxTableSize; i++, pTable++ )
                {
                    pTable->ulCountFlags = ulDescrBufSize;
                    pTable->ulAddr = ulBase;
                    if( i == ulDescrRxTableSize - 1 )
                        pTable->ulCountFlags |= ulDescrEndOfTable;
                }

                m_pOsServices->pfnFlushCache( pTableBase,
                    ulDescrRxTableSize * sizeof(DescrTable) );

                // Set ADSL receive descriptor tables.
                UINT32 *pulAdslRxAddr = (UINT32 *) &pulAdslBase[RCV_ADDR_INTR];
                *pulAdslRxAddr = CACHE_TO_PHYS(pTableBase);

                /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifdef CONFIG_BCM_VDSL
                if( m_ucPortType == PT_LOOPBACK && NULL != m_pOsServices->pfnAdslSetAtmLoopbackMode)
#else
                if( m_ucPortType == PT_LOOPBACK )
#endif
                {
                    (*m_pOsServices->pfnAdslSetAtmLoopbackMode) ();
                }
/*End modify : s48571 2006-10-26 for adsl porting*/                 
                /*End modify:l37298 2006-05-30 for deleting adsl feature*/	

                // Start a timer that calls the soft SAR to get cells to
                // put into the ADSL descriptor table.
                (*m_pOsServices->pfnStartTimer) ((void *) ATM_INTERFACE::TimerCb,
                    (UINT32) this, ATM_TIMEOUT_MS);
            }
            else
            {
                (*m_pOsServices->pfnFree) ((char *) m_ulAdslTxDescrTableAddr);
                m_ulAdslTxDescrTableAddr = NULL;
                baStatus = STS_ALLOC_ERROR;
            }
        }
        else
            baStatus = STS_ALLOC_ERROR;
    }

    return( baStatus );
} // AdslInit
#endif


//**************************************************************************
// Function Name: UtopiaInit
// Description  : Initializes the ATM port to use the UTOPIA interface.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::UtopiaInit( UINT8 ucPortFlags )
{
    m_ucUtopiaFlags = ucPortFlags;
    m_ulUtopiaCfg = ATMR_TX_UTOPIA_EN | ATMR_RX_UTOPIA_EN;

    if( (m_ucUtopiaFlags & PF_UTOPIA_LEVEL_2) != 0 )
        m_ulUtopiaCfg |= ATMR_TX_UTOPIA_LVL2_EN | ATMR_RX_UTOPIA_LVL2_EN;

    if((m_ucUtopiaFlags & PF_UTOPIA_NEG_EDGE_SEL) != 0)
        m_ulUtopiaCfg |= ATMR_TX_UTOPIA_NEG_EDGE_SEL|ATMR_RX_UTOPIA_NEG_EDGE_SEL;

#ifdef CONFIG_BCM96358
    if (!(GPIO->StrapBus & UTOPIA_MASTER_ON)) {
       /* Slave */
        m_ulUtopiaCfg |= 0x08 ; /* Select bit to multiplex the 
                                   utopia and the phy */
    }
#endif
} // UtopiaInit


//**************************************************************************
// Function Name: ConfigureAtmProcessor
// Description  : Sets ATM Processor "per port" configuration registers.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::ConfigureAtmProcessor( UINT32 ulEnable )
{
    const UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;
    UINT32 *pulAdslTxEnable = (m_ucPhyPortNum == PHY_0)
        ? (UINT32 *) &pulAdslBase[XMT_CTL_INTR]
        : (UINT32 *) &pulAdslBase[XMT_CTL_FAST];

    if( ulEnable )
    {
        AP_REGS->ulAdslLastDescCfg = m_ulAdslCfg;

        // Configure ATM receive.  It is enabled in SetLinkInfo.
#if !defined(CONFIG_ATM_RX_SOFTSAR)
        AP_REGS->ulRxAtmCfg[m_ucPhyPortNum] = ATMR_ERROR_CELLS_TO_PRQ;
#else
        AP_REGS->ulRxAtmCfg[m_ucPhyPortNum] = 0;
#endif

        AP_REGS->ulRxAalCfg |= ATMR_RX_AAL5_SW_TRAILER_EN;

#if !defined(CONFIG_ATM_SOFTSAR)
        // Configure ATM transmit.
        UINT32 ulBitIdx = 1 << m_ucPhyPortNum;
        AP_REGS->ulTxAtmHdrCfg |= (UINT32)
            (ulBitIdx << ATMR_TX_PORT_EN_SHIFT) |
            (ulBitIdx << ATMR_OAM_CRC10_EN_SHIFT) |
            (ulBitIdx << ATMR_OUT_FIFO_RESET_SHIFT);

        AP_REGS->ulTxAtmHdrCfg &= (UINT32)
            ~(ulBitIdx << ATMR_OUT_FIFO_RESET_SHIFT);

        // Enable the ATM scheduler.
        AP_REGS->ulTxAtmSchedCfg |= (ATMR_SCHED_EN << m_ucPhyPortNum);
#endif

        if( m_ucPortType == PT_ADSL_INTERLEAVED ||
            m_ucPortType == PT_ADSL_FAST || m_ucPortType == PT_LOOPBACK )
        {
            *pulAdslTxEnable |= 0x00000001;
        }

        // Configure link up/down related registers based on current link state.
        if( m_ulLinkState == LINK_UP || m_ucPortType == PT_LOOPBACK )
            SetLinkInfo( LINK_UP );
        else
            SetLinkInfo( LINK_DOWN );
    }
    else // disable
    {
        if( m_ucPortType == PT_ADSL_INTERLEAVED ||
            m_ucPortType == PT_ADSL_FAST || m_ucPortType == PT_LOOPBACK )
        {
            *pulAdslTxEnable &= ~0x00000001;
        }

#if !defined(CONFIG_ATM_SOFTSAR)
        // Disable ATM transmit.
        UINT32 ulBitIdx = 1 << m_ucPhyPortNum;
        AP_REGS->ulTxAtmHdrCfg &= (UINT32)
            ~((ulBitIdx << ATMR_TX_PORT_EN_SHIFT) |
              (ulBitIdx << ATMR_OAM_CRC10_EN_SHIFT));

        AP_REGS->ulTxAtmSchedCfg &= ~(ATMR_SCHED_EN << m_ucPhyPortNum);

        // Disable UTOPIA transmit and receive.
        AP_REGS->ulUtopiaCfg = 0;
#endif

        // Disable ATM receive.
        AP_REGS->ulRxAtmCfg[m_ucPhyPortNum] &= ~ATMR_RX_PORT_EN;

        // Reset interface enabled timestamp.
        m_Cfg.ulIfLastChange = (*m_pOsServices->pfnGetSystemTick) () / 10;
    }
} // ConfigureAtmProcessor


//**************************************************************************
// Function Name: ProcessOamCell
// Description  : Examines an OAM cell and sends a response if the function
//                type is Alarm Indication Signal (AIS) or Loopback requeest.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::ProcessOamCell( PATM_VCC_DATA_PARMS pDataParms )
{
    const UINT32 culAtmCellHdrSize = 4;
    const UINT32 culOamTypeFuncTypeIndex = 0;
    const UINT32 culLoopbackIndicatorIndex = 1;
    const UINT8 cucFaultMgmtAis = 0x10;
    const UINT8 cucFaultMgmtRdi = 0x11;
    const UINT8 cucFaultMgmtLoopback = 0x18;

    UINT8 *pHdr = pDataParms->pAtmBuffer->pDataBuf;
    UINT8 *pDataBuf = pHdr + culAtmCellHdrSize;
    UINT32 ulDataLen = pDataParms->pAtmBuffer->ulDataLen - culAtmCellHdrSize;

    if( pDataBuf[culOamTypeFuncTypeIndex] == cucFaultMgmtAis ||
        (pDataBuf[culOamTypeFuncTypeIndex] == cucFaultMgmtLoopback && 
         pDataBuf[culLoopbackIndicatorIndex] == 1) )
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
            // Send OAM response.
            UINT8 *pucBuffer = pOamRsp->ucBuffer;
            UINT16 usVpi = (pHdr[0] << 4) | (pHdr[1] >> 4);
            UINT16 usVci = ((UINT16) pHdr[1] << 12) |
                ((UINT16) pHdr[2] << 4) | ((UINT16) pHdr[3] >> 4);
            ATM_TX_BDQ *pTxQ = NULL;
            PATM_VCC_DATA_PARMS pDp = &pOamRsp->DataParms;
            PATM_BUFFER pAb = &pOamRsp->AtmBuffer;
            UINT32 ulGfcIdx = (usVci == VCI_OAM_F4_SEGMENT)
                ? GFC_OAM_F4_SEG_IDX : GFC_OAM_F4_ETOE_IDX; 

            // Copy the original OAM F4 data to the response buffer.
            memcpy( pucBuffer, pDataBuf, ulDataLen );

            // Change AIS (Alarm Indication Signal) request to RDI (Remote
            // Defect Indicator) or Loopback request to Loopback response.
            if( pucBuffer[culOamTypeFuncTypeIndex] == cucFaultMgmtAis )
                pucBuffer[culOamTypeFuncTypeIndex] = cucFaultMgmtRdi;
            else // loopback
                pucBuffer[culLoopbackIndicatorIndex] = 0;

            memcpy( pDp, pDataParms, sizeof(ATM_VCC_DATA_PARMS) );
            pDp->ucCircuitType = CT_OAM_F4_ANY;
            pDp->ucSendPriority = ANY_PRIORITY;
            pDp->pAtmBuffer = pAb;
            pDp->pFnFreeDataParms = (FN_FREE_DATA_PARMS) ATM_INTERFACE::FreeDp;
            pDp->ulParmFreeDataParms = (UINT32) this;

            pAb->pNextAtmBuf = NULL;
            pAb->pDataBuf = pOamRsp->ucBuffer;
            pAb->ulDataLen = ulDataLen;
            pAb->ulReserved = ulGfcIdx;

            // Find any transmit queue that is not enabled for multi-priority
            // to send the cell.  The VCC address will be taken from the GFC
            // table in the ADSL phy.
            for( UINT32 j = 0; j < MAX_QUEUE_SHAPERS; j++)
            {
                pTxQ = m_pApplTable->GetTxQByShaperNum( j );
                if( pTxQ != NULL && pTxQ->IsMpEnabled() == 0)
                {
                    /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifdef CONFIG_BCM_VDSL
                	if( NULL != m_pOsServices->pfnAdslSetVcEntry )
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
                	{
	                    (*m_pOsServices->pfnAdslSetVcEntry) (ulGfcIdx,
	                        (int) m_ucPhyPortNum, usVpi, usVci);
                    }
                    /*End modify:l37298 2006-05-30 for deleting adsl feature*/	

                    // Send the OAM F4 cell.
                    pTxQ->Add( pDp );
                    break;
                }
            }

            if( pTxQ == NULL )
            {
                // No ATM application is currently active so no transmit queue
                // is in use.  Temporarily create a transmit queue to send
                // the OAM F4 cell on.
                ATM_TRAFFIC_DESCR_PARM_ENTRY Entry =
                    { ID_ATM_TRAFFIC_DESCR_PARM_ENTRY, 0,
                      TDT_ATM_NO_TRAFFIC_DESCRIPTOR, 0, 0, 0, 0, 0, 0,
                      SC_UBR, 0, };

                // The second parameter is suppose to be the application
                // semaphore but this object does not have access to it.
                ATM_TX_BDQ TxQ( m_pOsServices, m_ulStatsLock,
                    m_ulPhyPort1Enabled, m_pSoftSar );

                // Create the transmit queue.
                BCMATM_STATUS baStatus =
                    TxQ.Create( 4, 1, 0, PHY_0, MAX_VCIDS - 1, &Entry );
                if( baStatus == STS_SUCCESS )
                {
                    /*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
/*Start modify : s48571 2006-10-26 for adsl porting*/  
#ifdef CONFIG_BCM_VDSL
                	if( NULL != m_pOsServices->pfnAdslSetVcEntry)
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
                	{
	                    (*m_pOsServices->pfnAdslSetVcEntry) (ulGfcIdx,
	                        (int) m_ucPhyPortNum, usVpi, usVci);
					}
					/*End modify:l37298 2006-05-30 for deleting adsl feature*/	
					
                    // Send the OAM F4 cell.
                    baStatus = TxQ.Add( pDp );
                    if( baStatus == STS_SUCCESS )
                    {
                        UINT32 i;

                        // Wait for the cell to be sent.
                        TxQ.Reclaim();
                        for( i = 0; i < 10000 && TxQ.IsQueueEmpty() == 0; i++ )
                            TxQ.Reclaim();
                    }

                    // Destroy the queue.
                    TxQ.Destroy();
                }
            }
        }
    }
} // ProcessOamCell


//**************************************************************************
// Function Name: FreeDp
// Description  : Frees memory used to send an OAM response packet.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::FreeDp( PATM_VCC_DATA_PARMS pDataParms )
{
    ATM_INTERFACE *pThis = (ATM_INTERFACE *) pDataParms->ulParmFreeDataParms;
    (*pThis->m_pOsServices->pfnFree) (pDataParms);
} // FreeDp


//**************************************************************************
// Function Name: ChangeCircuitType
// Description  : Changes the circuit type field in the ATM_VCC_DATA_PARMS
//                structure from CT_TRANSPARENT to CT_OAM_F4_ANY.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::ChangeCircuitType( PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDataParms, UINT8 ucCt )
{
    const UINT32 culAtmCellHdrSize = 4;
    PATM_BUFFER pAb = pDataParms->pAtmBuffer;
    UINT8 *pHdr = pAb->pDataBuf;
    UINT16 usVpi = (pHdr[0] << 4) | (pHdr[1] >> 4);
    UINT16 usVci = ((UINT16) pHdr[1] << 12) |
        ((UINT16) pHdr[2] << 4) | ((UINT16) pHdr[3] >> 4);

    // Set the VCC address.
    pVccAddr->ulInterfaceId = m_ulInterfaceId;
    pVccAddr->usVpi = usVpi;
    pVccAddr->usVci = usVci;

    // Change the circuit type and status.
    pDataParms->ucCircuitType = ucCt;
    pDataParms->baReceiveStatus = STS_SUCCESS;

    // Skip the ATM cell header.
    pAb->pDataBuf += culAtmCellHdrSize;
    pAb->ulDataLen -= culAtmCellHdrSize;
} // ChangeCircuitType


#if defined(CONFIG_ATM_SOFTSAR)
//**************************************************************************
// Function Name: TimerCb
// Description  : Called at periodic time intervals.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::TimerCb( UINT32 ulParm )
{
    ATM_INTERFACE *pIntf = (ATM_INTERFACE *) ulParm;

    ((ATM_SOFT_SAR *) pIntf->m_pSoftSar)->CheckTimerValues(ATM_TIMEOUT_MS);
    pIntf->SoftSarTxCells();

    (*pIntf->m_pOsServices->pfnStartTimer) ((void *) ATM_INTERFACE::TimerCb,
        ulParm, ATM_TIMEOUT_MS);
} // AtmLayerTimerCb


//**************************************************************************
// Function Name: SoftSarTxCells
// Description  : Gets cells to be transmitted from the Soft SAR and puts them
//                in the ADSL PHY transmit descriptor table.
// Returns      : None.
//**************************************************************************
void ATM_INTERFACE::SoftSarTxCells( void )
{
    struct DescrTable
    {
        UINT32 ulCountFlags;
        UINT32 ulAddr;
    };

    const UINT32 ulIdxMask = 0x00000fff;
    const UINT32 ulDescrEndOfFrame = 0x40000000;
    const UINT32 ulDescrStartOfTable = 0x80000000;

    // Only ADSL PHY port 0 is checked.
    UINT32 *pulAdslBase = (UINT32 *) ADSL_ENUM_BASE;
    DescrTable *pTableBase = (DescrTable *)
        PHYS_TO_NONCACHE(pulAdslBase[XMT_ADDR_INTR]);
    DescrTable *pTable = pTableBase + m_ulAdslTxHeadIdx;
    atmssCell *pCell = (atmssCell *) PHYS_TO_NONCACHE(pTable->ulAddr);
    ATM_SOFT_SAR *pSs = (ATM_SOFT_SAR *) m_pSoftSar;
    UINT32 ulHeadIdx;
    UINT32 ulUpdateHeadIdx = 0;

    // The ADSL descriptor table indicies are stored as index * 8
    // (or index << 3).
    UINT32 ulTailIdx = (pulAdslBase[XMT_STATUS_INTR] & ulIdxMask) >> 3;

    while( ((m_ulAdslTxHeadIdx + 1) % m_ulAdslTxTableLength) != ulTailIdx )
    {
        if( pSs->GetCell( pCell, CELL_TYPE_ADSL ) != STS_SUCCESS )
            break;

        pTable->ulCountFlags |= ulDescrStartOfTable | ulDescrEndOfFrame;
        m_ulAdslTxHeadIdx = (m_ulAdslTxHeadIdx + 1) % m_ulAdslTxTableLength;
        ulUpdateHeadIdx = 1;

        pTable = pTableBase + m_ulAdslTxHeadIdx;
        pCell = (atmssCell *) PHYS_TO_NONCACHE(pTable->ulAddr);
    }

    if( ulUpdateHeadIdx == 1 )
    {
        ulHeadIdx = (pulAdslBase[XMT_PTR_INTR] & ~ulIdxMask) |
            (m_ulAdslTxHeadIdx << 3);
        pulAdslBase[XMT_PTR_INTR] = ulHeadIdx;
    }
} // SoftSarTxCells
#endif

