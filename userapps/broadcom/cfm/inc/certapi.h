/*****************************************************************************
//
//  Copyright (c) 2000-2001  Broadcom Corporation
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
//  Filename:       certapi.h
//  Author:         Peter T. Tran
//  Creation Date:  06/11/02
//
******************************************************************************
//  Description:
//      Define the certificate functions.
//
*****************************************************************************/

#ifndef __CERT_API_H__
#define __CERT_API_H__

/********************** Include Files ***************************************/

#include "certdefs.h"
#include "certtbl.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmCcm_init(void);
extern void BcmCcm_unInit(void);
extern void * BcmCcm_startCertIteration(int type);
extern void * BcmCcm_getNextCert(void *p, PCERT_DATA_ENTRY pObject, int type);
extern int BcmCcm_findCert(char *name, PCERT_DATA_ENTRY entry, int type);
extern int BcmCcm_setCert(char *name, CERT_DATA_ENTRY entry, int type);
extern int BcmCcm_addCert(PCERT_DATA_ENTRY pObject, int type);
extern int BcmCcm_removeCert(char *certName, int type);
extern int BcmCcm_incRefCount(char *name);
extern int BcmCcm_decRefCount(char *name);
extern void BcmCcm_resetRefCount(void);
extern int BcmCcm_createCertReq(PCERT_ADD_INFO pAddInfo, PCERT_DATA_ENTRY pObject);
extern int BcmCcm_verifyCertVReq(PCERT_DATA_ENTRY pObject) ;
extern int BcmCcm_procImpCert(PCERT_DATA_ENTRY pObject);
extern int BcmCcm_retrieveSubject(PCERT_DATA_ENTRY pObject);
extern void BcmCcm_store(void);
extern int BcmCcm_getSize(int type);
extern void BcmCcm_getDefaults(PCERT_DATA_ENTRY entry);
extern void BcmCcm_setCACertLinks(void);
#if defined(__cplusplus)
}
#endif

#endif
