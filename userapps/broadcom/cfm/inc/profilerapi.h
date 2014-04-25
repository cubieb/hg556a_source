/*****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       profilerapi.h
//
******************************************************************************
//  Description:
//      Define the Profiler Shell CLI Api functions.
//
*****************************************************************************/
#ifndef PROFILER_API__H__INCLUDED
#define PROFILER_API__H__INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmProfiler_Start( void );
extern void BcmProfiler_Stop( void );
extern void BcmProfiler_InitData( void );
extern void BcmProfiler_Dump( void );
extern void BcmProfiler_RecSeq_Dump( void );
extern void BcmProfiler_ShowCmdSyntax( void );
#if defined(__cplusplus)
}
#endif

#endif /* PROFILER_API__H__INCLUDED */
