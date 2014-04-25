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
// File Name  : AtmNotify.cpp for BCM63xx
//
// Description: This file contains the implementation for the ATM notification
//              event class.
//
// Updates    : 01/03/2002  lat.  Created from BCM6352.
//**************************************************************************

#include "atmapiimpl.h"


//**************************************************************************
// ATM_NOTIFY Class
//**************************************************************************


//**************************************************************************
// Function Name: ATM_NOTIFY
// Description  : Constructor for the ATM notification event class.
// Returns      : None.
//**************************************************************************
ATM_NOTIFY::ATM_NOTIFY( ATM_OS_SERVICES *pOsServices )
{
    m_pOsServices = pOsServices;
    memset( m_List, 0x00, sizeof(m_List) );
} // ATM_NOTIFY


//**************************************************************************
// Function Name: ~ATM_NOTIFY
// Description  : Destructor for the ATM notification event class.
// Returns      : None.
//**************************************************************************
ATM_NOTIFY::~ATM_NOTIFY( void )
{
} // ~ATM_NOTIFY


//**************************************************************************
// Function Name: AddCallback
// Description  : Adds an application callback function address to the list
//                of function addresses to call when an ATM notification
//                event occurs.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_NOTIFY::AddCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    int i;

    for( i = 0; i < MAX_NOTIFICATION_ENTRIES; i++ )
        if( m_List[i] == NULL )
        {
            m_List[i] = pFnNotifyCb;
            break;
        }

    return((i != MAX_NOTIFICATION_ENTRIES) ? STS_SUCCESS : STS_RESOURCE_ERROR);
} // AddCallback


//**************************************************************************
// Function Name: RemoveCallback
// Description  : Removes an application callback function address from the
//                list of function addresses.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_NOTIFY::RemoveCallback( FN_NOTIFY_CB pFnNotifyCb )
{
    for( int i = 0; i < MAX_NOTIFICATION_ENTRIES; i++ )
        if( m_List[i] == pFnNotifyCb )
            m_List[i] = NULL;

    return( STS_SUCCESS );
} // RemoveCallback


//**************************************************************************
// Function Name: SendEvent
// Description  : Calls all application callback functions with the
//                specified notification event.
// Returns      : STS_SUCCESS if successful or error status.
//**************************************************************************
BCMATM_STATUS ATM_NOTIFY::SendEvent( PATM_NOTIFY_PARMS pNotifyParms )
{

    for( int i = 0; i < MAX_NOTIFICATION_ENTRIES; i++ )
        if( m_List[i] )
            (*m_List[i]) (pNotifyParms);

    return( STS_SUCCESS );
} // RemoveCallback

