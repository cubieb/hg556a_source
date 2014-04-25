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
// File Name  : AtmApiImpl.h for BCM63xx
//
// Description: This file contains class definitions for the ATM API module
//              implementation.
//
// Updates    : 12/26/2001  lat.  Created from BCM6352.
//**************************************************************************

#if !defined(_ATMAPIIMPL_H_)
#define _ATMAPIIMPL_H_

#if defined(__KERNEL__)
#include <linux/config.h>
#endif

#if defined(_CFE_)
#define memset(d,s,c) lib_memset(d,s,c)
#define memcpy(d,s,c) lib_memcpy(d,s,c)

#if defined(_BCM96348_)
#define CONFIG_BCM96348
#elif defined(_BCM96338_)
#define CONFIG_BCM96338
#endif
#endif

// The following definitions are defined here because the ATM API source files
// do not include standard operating system header files.  The ATM API source
// files are operating system indepdendent.
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;

#define NULL            0
#define TRUE            1
#define FALSE           0

extern "C" {
extern int memset( void *, int, unsigned int ); 
extern void *memcpy( void *, const void *, unsigned int ); 
}

#include "bcmatmapi.h"
#include "atmprocregs.h"

#define MAX_TX_PRIORITIES           4

#if defined(CONFIG_BCM96338)
#define CONFIG_ATM_SOFTSAR
#if defined(__KERNEL__)
#define CONFIG_ATM_RX_SOFTSAR
#endif
#include "atmsoftsar.h"
#endif

// MIPS Processor specific defintions.
#define CACHE_TO_NONCACHE(x)        ((unsigned)(x)|0xA0000000)
#define NONCACHE_TO_CACHE(x)        ((unsigned)(x)&0x9FFFFFFF)
#define CACHE_TO_PHYS(x)            ((unsigned)(x)&0x1FFFFFFF)
#define NONCACHE_TO_PHYS(x)         ((unsigned)(x)&0x1FFFFFFF)
#define PHYS_TO_CACHE(x)            ((unsigned)(x)|0x80000000)
#define PHYS_TO_NONCACHE(x)         ((unsigned)(x)|0xA0000000)

// Default definitions.
#define DEFAULT_FREE_PKT_Q_SIZE     600
#define DEFAULT_FREE_CELL_Q_SIZE    600
#define DEFAULT_RECEIVE_PKT_Q_SIZE  600
#define DEFAULT_RECEIVE_CELL_Q_SIZE 600
#define DEFAULT_PKT_BUF_SIZE        1600

// Limit definitions.
#define MAX_INTERFACES              AP_MAX_PORTS 
#define MAX_VCCS                    AP_MAX_VCCS 
#define MAX_VCIDS                   AP_MAX_VCIDS
#define MAX_RX_VCIDS                AP_MAX_RX_VCIDS
#define MAX_QUEUE_SHAPERS           AP_MAX_QUEUES
#define MAX_NOTIFICATION_ENTRIES    4
#define MAX_MGMT_TRANSPARENT_APPLS  6
#define MAX_APPL_TABLE_ENTRIES      (MAX_VCCS + MAX_MGMT_TRANSPARENT_APPLS)
#define MAX_AAL_IDXS                32
#define MAX_TX_QUEUES               8
#define MAX_TX_QUEUE_SIZE           256
#define MAX_VCC_INFOS               16
#define MAX_GFC_TABLE_SIZE          16
#define MAX_GFC_TABLE_ENTRIES       11
#define MAX_TIMEOUT                 ((UINT32) -1)

// Size definitions.
#define FREE_CELL_Q_BUF_SIZE        64
#if defined(CONFIG_BCM96358)
#define FREE_CELL_Q_BUF_SIZE_EXP    1
#else
#define FREE_CELL_Q_BUF_SIZE_EXP    6
#endif

// Shaping Interval Timer (SIT) "unit of time" * 100.  The "* 100" is to
// reduce arithmatic rounding errors.
#define ADSL_SIT_UNIT_OF_TIME       500 // SIT value is 5
#define UTOPIA_SIT_UNIT_OF_TIME     125 // SIT value is 1.25

// Flag bits for ATM_DMA_BD ulFlags_NextRxBd
#define BD_FLAG_EOP                 0x80000000 // End Of Packet
#define BD_FLAG_CLP                 0x40000000 // Cell Loss Priority
#define BD_FLAG_CI                  0x20000000 // Congestion Indicator
#define BD_FLAG_NEG                 0x10000000 // Negative length

// BD bit twiddling.
#define BD_CT_SHIFT                 27
#define BD_FLAGS_SHIFT              27
#define BD_ADDR_MASK                0x07ffffff
#define BD_SET_ADDR(F,V)            F = ((F & ~BD_ADDR_MASK) | \
                                        NONCACHE_TO_PHYS((UINT32) (V)))
#define BD_SET_CT(F,V)              F = ((F & BD_ADDR_MASK) | \
                                        ((UINT32) (V) << BD_CT_SHIFT))
#define BD_GET_CADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_CACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_GET_NCADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_NONCACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_SET_RX_RBL(BD,V) \
    BD->ucRxAalErrors_RblHigh = (BD->ucRxAalErrors_RblHigh & 0x8e) | ((V)>>3); \
    BD->ucRxRblLow  = (BD->ucRxRblLow  & 0x1f) | (((V) & 0x07) << 5)
#define BD_GET_RX_RBL(BD) \
    (((BD->ucRxAalErrors_RblHigh & 0x01) << 3) | ((BD->ucRxRblLow & 0xe0) >> 5)

#define BD_TX_PORT_ID_MASK          0x30
#define BD_TX_PORT_ID_SHIFT         4

#if defined(CONFIG_BCM96358)
#define BD_RX_VCID_MASK             0x0f
#define BD_RX_PORT_ID_MASK          0x30
#define BD_RX_PORT_ID_SHIFT         4
#elif defined(CONFIG_BCM96348)
#define BD_RX_VCID_MASK             0x0f
#define BD_RX_PORT_ID_MASK          0x10
#define BD_RX_PORT_ID_SHIFT         4
#elif defined(CONFIG_BCM96338)
#define BD_RX_VCID_MASK             0x03
#define BD_RX_PORT_ID_MASK          0x04
#define BD_RX_PORT_ID_SHIFT         2
#endif

#define BD_RX_AAL_ERROR_MASK        (0xf0 & ~RXAAL5AAL0_LENGTH_ERROR)

// ATM Error Indicators.
#define RXATM_PORT_NOT_ENABLED      0x80
#define RXATM_HEC_ERROR             0x40
#define RXATM_PTI_ERROR             0x20
#define RXATM_RECEIVED_IDLE_CELL    0x10
#define RXATM_INVALID_VPI_VCI       0x08
#define RXATM_NOT_USED              0x04
#define RXATM_OAM_RM_CRC_ERROR      0x02
#define RXATM_GFC_ERROR             0x01

// AAL5 and AAL0 Error Indicators.
#define RXAAL5AAL0_CRC_ERROR        0x80
#define RXAAL5AAL0_SHORT_PKT_ERROR  0x40
#define RXAAL5AAL0_LENGTH_ERROR     0x20
#define RXAAL5AAL0_BIG_PKT_ERROR    0x10

// Determines the number of queue elements that are filled.
#define Q_ELS_FILLED(X,Y,L)         (((X)>=(Y)) ? ((X)-(Y)) : ((L)-(Y)+(X)))

// States for ATM_APPL m_ulState
#define ST_APPL_CONSTRUCTED         0
#define ST_APPL_ATTACHED            1
#define ST_APPL_DETACH_PENDING      2
#define ST_APPL_ERROR               3

// Transparent Table masks.
#define GFCTBL_GFC_MASK             0xf0000000
#define GFCTBL_GFC_SHIFT            28
#define GFCTBL_VPI_MASK             0x0ff00000
#define GFCTBL_VPI_SHIFT            20
#define GFCTBL_VCI_MASK             0x000ffff0
#define GFCTBL_VCI_SHIFT            4
#define GFCTBL_PTI_CLP_MASK         0x0000000f

// GFC indicies for sending OAM cells.
#define GFC_OAM_F5_ETOE_IDX         (MAX_GFC_TABLE_ENTRIES + 1)
#define GFC_OAM_F5_SEG_IDX          (MAX_GFC_TABLE_ENTRIES + 2)
#define GFC_OAM_F4_ETOE_IDX         (MAX_GFC_TABLE_ENTRIES + 3)
#define GFC_OAM_F4_SEG_IDX          (MAX_GFC_TABLE_ENTRIES + 4)

// Values for ATM_VCC_DATA_PARMS ucFlags that is not exposed to applications.
#define RX_DATA_PENDING             0x80

// BCM6338 specific timer timeout value.
#define PKT_CHK_TIMEOUT_MS          100

// Soft SAR specific definitions.
#define ATM_TIMEOUT_MS              5

// Approximate calculation:
// (atm_timeout_ms * maximum_possible_upstream_bits_per_sec) /
// (424 bits_per_cell * 1000 ms_per_sec)
#define ATM_ADSL_TX_DESCR_TBL_SIZE  20

// Used for receive packet priority (voice and data).
#define DROP_PACKET_MINIMUM         20
#define DROP_PACKET_INCREMENT       5

// Forward references.
class ATM_INTERFACE;
class ATM_APPL_TABLE;

// ATM_OS_SERVICES contains function callback addresses to operating system
// services.
typedef void* (*FN_ALLOC) (UINT32 ulSize);
typedef void (*FN_FREE) (void *pBuf);
typedef void (*FN_DELAY) (UINT32 ulTimeoutMs);
typedef UINT32 (*FN_CREATE_SEM) (UINT32 ulInitialState);
typedef UINT32 (*FN_REQUEST_SEM) (UINT32 ulSem, UINT32 ulTimeoutMs);
typedef UINT32 (*FN_RELEASE_SEM) (UINT32 ulSem);
typedef UINT32 (*FN_DELETE_SEM) (UINT32 ulSem);
typedef UINT32 (*FN_DISABLE_INTS) (void);
typedef void (*FN_ENABLE_INTS) (UINT32 ulLevel);
typedef void (*FN_INVALIDATE_CACHE) (void *pAddr, UINT32 ulSize);
typedef void (*FN_FLUSH_CACHE) (void *pAddr, UINT32 ulSize);
typedef UINT32 (*FN_GET_TOP_MEM_ADDR) (void);
typedef UINT32 (*FN_ADSL_SET_VC_ENTRY) (int gfc, int port, int vpi, int vci);
typedef UINT32 (*FN_ADSL_SET_ATM_LOOPBACK_MODE) (void);
typedef void (*FN_BLINK_LED) (void);
typedef UINT32 (*FN_GET_SYSTEM_TICK) (void);
typedef UINT32 (*FN_ADSL_SET_VC_ENTRY_EX) (int gfc, int port, int vpi, int vci,
    int pti_clp);
typedef void (*FN_ADSL_ATM_CLEAR_VC_TABLE) (void);
typedef void (*FN_ADSL_ATM_ADD_VC) (int vpi, int vci);
typedef void (*FN_ADSL_ATM_DELETE_VC) (int vpi, int vci);
typedef void (*FN_ADSL_ATM_SET_MAX_SDU) (UINT16 maxsdu);
typedef UINT32 (*FN_START_TIMER) (void *, UINT32, UINT32);
typedef void (*FN_PRINTF) (char *, ...);

#define ID_ATM_OS_SERVICES                  4
struct ATM_OS_SERVICES
{
    UINT32 ulStructureId;
    FN_ALLOC pfnAlloc;
    FN_FREE pfnFree;
    FN_DELAY pfnDelay;
    FN_CREATE_SEM pfnCreateSem;
    FN_REQUEST_SEM pfnRequestSem;
    FN_RELEASE_SEM pfnReleaseSem;
    FN_DELETE_SEM pfnDeleteSem;
    FN_DISABLE_INTS pfnDisableInts;
    FN_ENABLE_INTS pfnEnableInts;
    FN_INVALIDATE_CACHE pfnInvalidateCache;
    FN_FLUSH_CACHE pfnFlushCache;
    FN_GET_TOP_MEM_ADDR pfnGetTopMemAddr;
    FN_ADSL_SET_VC_ENTRY pfnAdslSetVcEntry;
    FN_ADSL_SET_ATM_LOOPBACK_MODE pfnAdslSetAtmLoopbackMode;
    FN_BLINK_LED pfnBlinkLed;
    FN_GET_SYSTEM_TICK pfnGetSystemTick;
    FN_ADSL_SET_VC_ENTRY_EX pfnAdslSetVcEntryEx;
    FN_START_TIMER pfnStartTimer;
    FN_PRINTF pfnPrintf;
    FN_ADSL_ATM_CLEAR_VC_TABLE pfnAdslAtmClearVcTable;
    FN_ADSL_ATM_ADD_VC pfnAdslAtmAddVc;
    FN_ADSL_ATM_DELETE_VC pfnAdslAtmDeleteVc;
    FN_ADSL_ATM_SET_MAX_SDU pfnAdslAtmSetMaxSdu;
};


// ATM DMA Buffer Descriptor
struct ATM_DMA_BD
{
    UINT32 ulCt_BufPtr;
    UINT32 ulFlags_NextRxBd;
    UINT8 ucUui8;
    UINT8 ucRxPortId_Vcid;
    UINT16 usLength;
    union
    {
        UINT8 ucTxPortId_Gfc;
        UINT8 ucRxAtmErrors;
    };
    UINT8 ucRxAalErrors_RblHigh;
    union
    {
        UINT8 ucRxRblLow;
        UINT8 ucFreeRbl;
    };
    UINT8 ucReserved;
};


#if !defined(CONFIG_ATM_SOFTSAR)
// ATM_SHAPER configures the ATM Processor shaper registers.
class ATM_SHAPER
{
public:
    ATM_SHAPER( void );
    ~ATM_SHAPER( void );

    UINT8 GetAdjustedPriority( UINT32 ulServiceCategory, UINT32 ulPriority );
    BCMATM_STATUS Configure( UINT8 ucShaperNum, UINT8 ucPriority,
        UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    static void SetSitUt( UINT32 ulSitUt )
        { ms_ulSitUt = ulSitUt; }

private:
    // Passed in Configure.
    UINT32 *m_pulSstCtrl;
    UINT32 *m_pulSstVbr;
    UINT32 *m_pulSstMcr; // [BCM6348 Only]

    static UINT32 ms_ulSitUt;
};


// ATM_TX_BDQ maintains a queue of ATM DMA buffer descriptors that are used to
// copy data to the ATM Processor.
class ATM_TX_BDQ
{
public:
    ATM_TX_BDQ( ATM_OS_SERVICES *pOsServices, UINT32 ulAtmApplSem,
        UINT32 ulPhyPort1Enabled, void *pSoftSar );
    ~ATM_TX_BDQ( void );

    BCMATM_STATUS Create( UINT32 ulLength, UINT8 ucBasePriority,
        UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    BCMATM_STATUS Destroy( void );
    BCMATM_STATUS Add( PATM_VCC_DATA_PARMS pDataParms, UINT8 ucVcid = 0 );
    BCMATM_STATUS Reclaim( void );
    UINT8 GetQIdx( void )
        { return( m_ucShaperNum ); }
    void ReconfigureShaper( PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry = NULL );
    UINT32 IsQueueEmpty( void )
        { return( (*m_pucHeadIdx == *m_pucTailIdx) ? 1 : 0 ); }
    UINT32 IsMpEnabled( void )
        { return( m_ulMpEnable ); }
    UINT8 GetPort( void )
        { return( m_ucPortId ); }
    UINT32 IsShaped( void )
        { return( m_ulShaped ); }
    void IncInUse( void )
        { m_ulInUse++; }
    UINT32 DecInUse( void )
        { m_ulInUse--; return( m_ulInUse ); }
    UINT32 GetInUse( void )
        { return( m_ulInUse ); }

private:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;
    UINT32 m_ulAtmApplSem;
    UINT32 m_ulPhyPort1Enabled;

    // Passed in Create.
    UINT16 m_usLength;      // length of queue
    UINT8 m_ucPriority;     // priority of queue, used by ATM Processor
    UINT8 m_ucVcid;
    UINT8 m_ucPortId;
    UINT32 m_ulShaped;
    ATM_TRAFFIC_DESCR_PARM_ENTRY m_TdpEntry;

    // Other private member data.
    ATM_SHAPER m_Shaper;
    UINT8  m_ucBasePriority;// Base priority of shaper
    UINT8  m_ucShaperNum;
    UINT16 m_usHeadIdx;     // local copy of head index
    UINT32 m_ulMuSem;       // semaphore used as a mutex
    UINT32 m_ulMemSize;     // Amount of memory on the board.
    UINT32 m_ulMpEnable;    // Use multi-priority queues
    char *m_pStartBase;     // base address of queue in SDRAM
    ATM_DMA_BD *m_pStart;   // address of queue in SDRAM on quadword boundary
    volatile UINT8 *m_pucHeadIdx;  // addr of ATM Processor register head index
    volatile UINT8 *m_pucTailIdx;  // addr of ATM Processor register tail index
    UINT32 m_ulCurrBdIdx;
    UINT32 m_ulTxPending;
    UINT32 m_ulInUse;
    struct ATM_TX_BD_INFO
    {
        PATM_VCC_DATA_PARMS pDataParms;
        UINT16 usLastIdx;
        UINT8 ucFirstIdx;
        UINT8 ucNumBds;
    } *m_pTxBdInfo;

    static UINT16 ms_usByteCrc10Table[256];

    // Private member functions.
    void GenByteCrc10Table( void );
    UINT16 UpdateCrc10ByBytes(UINT16 usCrc10Accum, UINT8 *pBuf,int nDataBlkSize);
};

#else
class ATM_TX_BDQ
{
public:
    ATM_TX_BDQ( ATM_OS_SERVICES *pOsServices, UINT32 ulAtmApplSem,
        UINT32 ulPhyPort1Enabled, void *pSoftSar );
    ~ATM_TX_BDQ( void );

    BCMATM_STATUS Create( UINT32 ulLength, UINT8 ucBasePriority,
        UINT32 ulMpEnable, UINT8 ucPortId, UINT8 ucVcid,
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    BCMATM_STATUS Destroy( void );
    BCMATM_STATUS Add( PATM_VCC_DATA_PARMS pDataParms, UINT8 ucVcid = 0 );
    BCMATM_STATUS Reclaim( void );
    UINT8 GetQIdx( void )
        { return( m_ucVcid ); }
    void ReconfigureShaper( PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    UINT32 IsQueueEmpty( void )
        { return( 1 ); }
    UINT32 IsMpEnabled( void )
        { return( 0 ); }
    UINT8 GetPort( void )
        { return( m_ucPortId ); }
    UINT32 IsShaped( void )
        { return( 0 ); }
    void IncInUse( void )
        { m_ulInUse++; }
    UINT32 DecInUse( void )
        { m_ulInUse--; return( m_ulInUse ); }
    UINT32 GetInUse( void )
        { return( m_ulInUse ); }

private:
    ATM_SOFT_SAR *m_pSoftSar;
    UINT8 m_ucVcid;
    UINT8 m_ucPortId;
    UINT32 m_ulInUse;
    static UINT32 m_ulGfcAtmHdrs[16];
    static UINT16 ms_usByteCrc10Table[256];

    // Private member functions.
    void GenByteCrc10Table( void );
    UINT16 UpdateCrc10ByBytes(UINT16 usCrc10Accum, UINT8 *pBuf,int nDataBlkSize);
    static UINT32 SetVcEntry( int gfc, int port, int vpi, int vci );
    static UINT32 SetVcEntryEx(int gfc, int port, int vpi, int vci, int pti_clp);
};
#endif


// ATM_RX_FREE_BDQ maintains a queue of pointers to DMA buffer descriptors.
class ATM_RX_FREE_BDQ
{
public:
    ATM_RX_FREE_BDQ( ATM_OS_SERVICES *pOsServices );
    ~ATM_RX_FREE_BDQ( void );

    BCMATM_STATUS Create( volatile UINT32 *pulRegisterBase, UINT32 ulLength );
    BCMATM_STATUS Destroy( void );
    BCMATM_STATUS Add( ATM_DMA_BD *pBd );
    UINT32 GetSize( void )
        { return( m_ulLength ); }
    UINT32 GetNumFilledElements( void );
    UINT32 GetNumAvailableElements( void );

protected:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;

    // Passed in Create.
    UINT32 m_ulLength;      // length of queue

    // Other private member data.
    UINT32 m_ulMuSem;       // semaphore used as a mutex
    volatile UINT32 *m_pulRegisterBase;
    ATM_DMA_BD **m_ppStartBase; // address of queue in SDRAM
    ATM_DMA_BD **m_ppStart;     // address of queue in SDRAM on 16 byte boundary
    volatile UINT32 *m_pulHeadIdx; // addr of ATM Processor register head index
    volatile UINT32 *m_pulTailIdx; // addr of ATM Processor register tail index
};


// ATM_RX_BDQ maintains a queue of pointers to DMA buffer descriptors (BDs).
// The ATM Processor adds BDs to this queue.  The ATM API module removes BDs
// from this queue.
class ATM_RX_BDQ : public ATM_RX_FREE_BDQ
{
public:
    ATM_RX_BDQ( ATM_OS_SERVICES *pOsServices );
    ~ATM_RX_BDQ( void );

    BCMATM_STATUS Remove( ATM_DMA_BD **ppBd );
};


// ATM_FREE_BDQ maintains a queue of pointers to DMA buffer descriptors (BDs).
// The ATM API module adds BDs to this queue.  The ATM Processor removes BDs
// from this queue.
class ATM_FREE_BDQ : public ATM_RX_FREE_BDQ
{
public:
    ATM_FREE_BDQ( ATM_OS_SERVICES *pOsServices );
    ~ATM_FREE_BDQ( void );

#if defined(CONFIG_BCM96338)
    // BCM6338 HW Rx SAR uses packet free queue indicies 1 to 100 rather 0 to 99.
    // Subtract the base address by one element to compensate.
    BCMATM_STATUS Create( volatile UINT32 *pulRegisterBase, UINT32 ulLength )
        { BCMATM_STATUS baStatus =
              ATM_RX_FREE_BDQ::Create( pulRegisterBase, ulLength );
          if( pulRegisterBase == &AP_REGS->ulFreePktQAddr )
          {
              m_pulRegisterBase[RX_FREE_ADDR_OFFSET] = (UINT32)
                NONCACHE_TO_PHYS(((UINT32) m_ppStart - sizeof(ATM_DMA_BD **)));
          }
          return( baStatus );
        }
#endif
};


// ATM_NOTIFY maintains information about ATM notification events.
class ATM_NOTIFY
{
public:
    ATM_NOTIFY( ATM_OS_SERVICES *pOsServices );
    ~ATM_NOTIFY( void );

    BCMATM_STATUS AddCallback( FN_NOTIFY_CB pFnNotifyCb );
    BCMATM_STATUS RemoveCallback( FN_NOTIFY_CB pFnNotifyCb );
    BCMATM_STATUS SendEvent( PATM_NOTIFY_PARMS pNotifyParms );

private:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;

    // Other private member data.
    FN_NOTIFY_CB m_List[MAX_NOTIFICATION_ENTRIES];
};


// ATM_VCC_INFO contains VCC Information.
struct ATM_VCC_INFO
{
    ATM_VCC_INFO *pNext;
    ATM_VCC_ADDR Addr;
    ATM_VCC_CFG Cfg;
    ATM_VCC_STATS Stats;
    ATM_INTERFACE *pInterface;
    UINT8 ucVcid;
    UINT8 ucReserved[3];
};


// ATM_VCC_INFO_TABLE contains a lookup table for VCC information.
class ATM_VCC_INFO_TABLE
{
public:
    ATM_VCC_INFO_TABLE( ATM_OS_SERVICES *pOsServices );
    ~ATM_VCC_INFO_TABLE( void );

    void SetSem( UINT32 ulMuSem )
        { m_ulMuSem = ulMuSem; }
    BCMATM_STATUS AddReplace( PATM_VCC_ADDR pAddr, PATM_VCC_CFG pCfg,
        ATM_INTERFACE *pInterface );
    void Remove( PATM_VCC_ADDR pAddr );
    ATM_VCC_INFO *Get( PATM_VCC_ADDR pAddr );
    BCMATM_STATUS GetAllAddrs( UINT32 ulInterfaceId, PATM_VCC_ADDR pAddrs,
        UINT32 ulNumAddrs, UINT32 *pulNumReturned );

private:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;

    // Passed in SetSem.
    UINT32 m_ulMuSem;

    // Other private member data.
    ATM_VCC_INFO *m_pVccInfos[MAX_VCC_INFOS]; // index is least significant
};                                            // four bits of VCI


// ATM_APPL keeps information about an application's attachment to the ATM
// Processor.
class ATM_APPL
{
public:
    ATM_APPL( ATM_OS_SERVICES *pOsServices, ATM_APPL_TABLE *pApplTable,
        UINT32 ulAtmApplSem, void *pSoftSar );
    virtual ~ATM_APPL( void );

    void SendComplete( UINT32 ulShaperNum )
        { if( m_pTxQViaShaperNum[ulShaperNum] != NULL )
              m_pTxQViaShaperNum[ulShaperNum]->Reclaim(); }
    virtual BCMATM_STATUS Detach( void )
        { return( STS_NOT_SUPPORTED ); }
    virtual void SetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescrTable, UINT32  ulTrafficDescrTableSize )
        { }
    virtual void SetVclLastChange( UINT32 ulLastChange )
        { }

protected:
    // Passed in Constructor.
    ATM_OS_SERVICES *m_pOsServices;
    ATM_APPL_TABLE *m_pApplTable;
    UINT32 m_ulAtmApplSem;
    void *m_pSoftSar;

    // Other protected member data.
    ATM_TX_BDQ *m_pTxQViaShaperNum[MAX_QUEUE_SHAPERS]; // index on shaper number
    ATM_TX_BDQ *m_pTxQViaPriority[MAX_TX_PRIORITIES + 1];  // index on priority
    ATM_TX_BDQ *m_pAnyTxQ;

    // Protected member functions.
    BCMATM_STATUS AttachTxQueues( PATM_VCC_ATTACH_PARMS pAttachParms,
        UINT8 ucPortId, UINT8 ucVcid, PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry,
        UINT32 ulPhyPort1Enabled );
    BCMATM_STATUS DetachTxQueues( void );
};


// ATM_VCC_APPL keeps information for an application that sends and receives
// data on a particular VCC.
class ATM_VCC_APPL : public ATM_APPL
{
public:
    ATM_VCC_APPL( ATM_OS_SERVICES *pOsServices, ATM_APPL_TABLE *pApplTable,
        UINT32 ulAtmApplSem, void *pSoftSar );
    ~ATM_VCC_APPL( void );

    BCMATM_STATUS Attach( ATM_VCC_INFO *pInfo,
        PATM_VCC_ATTACH_PARMS pAttachParms );
    BCMATM_STATUS Send( PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS SetAal2ChannelIds(
        PATM_VCC_AAL2_CHANNEL_ID_PARMS pChannelIdParms,
        UINT32 ulNumChannelIdParms );
    BCMATM_STATUS Receive( PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS Detach( void );
    void SetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescrTable, UINT32  ulTrafficDescrTableSize );
    void SetVclLastChange( UINT32 ulLastChange )
        { m_pVccInfo->Cfg.ulAtmVclLastChange = ulLastChange; }

private:
    // Passed in Attach.
    ATM_VCC_INFO *m_pVccInfo;
    UINT32 m_ulFlags;                   // AVAP_ALLOW_... values
    FN_RECEIVE_CB m_pFnReceiveDataCb;
    UINT32 m_ulParmReceiveData;

    // Passed in SetTrafficDescrTable.
    PATM_TRAFFIC_DESCR_PARM_ENTRY m_pTrafficDescrTable;
    UINT32  m_ulTrafficDescrTableSize;

    // Other private member data.
    ATM_INTERFACE *m_pInterface;
    ATM_INTERFACE_STATS *m_pIntfStats;
    UINT32 m_ulState;
    UINT8 m_ucVcid;
    UINT32 m_ulMpIdx;

    // Private member functions.
    BCMATM_STATUS AssignVcid( void );
    PATM_TRAFFIC_DESCR_PARM_ENTRY GetTdtEntry( UINT32 ulTdtIdx );
    BCMATM_STATUS ReserveMultiPriorityIndex( void );
    void SetRxVpiVciCam( UINT32 ulServiceCategory );
    BCMATM_STATUS ShareExistingTxQueue( UINT32 ulPriority );
    void ProcessOamCell( PATM_VCC_DATA_PARMS pDataParms );
    static void FreeDp( PATM_VCC_DATA_PARMS pDataParms );
    void UpdateAal5VccStats( PATM_VCC_DATA_PARMS pDataParms );
    void UpdateAal0PktVccStats( PATM_VCC_DATA_PARMS pDataParms );
    void UpdateAal0CellVccStats( PATM_VCC_DATA_PARMS pDataParms );
    void UpdateAtmStats( PATM_VCC_DATA_PARMS pDataParms,
        PATM_INTF_ATM_STATS pStats );
};

// ATM_MGMT_APPL keeps information about a management application that sends and
// receives cells on all VCCs.
class ATM_MGMT_APPL : public ATM_APPL
{
public:
    ATM_MGMT_APPL( ATM_OS_SERVICES *pOsServices, ATM_APPL_TABLE *pApplTable,
        UINT32 ulAtmApplSem, void *pSoftSar );
    ~ATM_MGMT_APPL( void );

    BCMATM_STATUS Attach( ATM_INTERFACE *pInterfaces, UINT32 ulNumInterfaces,
        PATM_VCC_ATTACH_PARMS pAttachParms );
    BCMATM_STATUS Send(PATM_VCC_ADDR pVccAddr, PATM_VCC_DATA_PARMS pDataParms);
    BCMATM_STATUS Receive(PATM_VCC_ADDR pAddr, PATM_VCC_DATA_PARMS pDataParms);
    BCMATM_STATUS Detach( void );

private:
    // Passed in Attach.
    UINT32 m_ulFlags;                // AVAP_ALLOW_... values
    ATM_INTERFACE *m_pInterfaces;
    UINT32 m_ulNumInterfaces;
    FN_RECEIVE_CB m_pFnReceiveDataCb;
    UINT32 m_ulParmReceiveData;

    // Other private member data.
    UINT32 m_ulState;
};


// ATM_TRANSPARENT_APPL keeps information about an application that sends and
// receives transparent ATM cells.
#if !defined(_CFE_BRIDGE_)
class ATM_TRANSPARENT_APPL : public ATM_APPL
{
public:
    ATM_TRANSPARENT_APPL(ATM_OS_SERVICES *pOsServices,
        ATM_APPL_TABLE *pApplTable, UINT32 ulAtmApplSem, void *pSoftSar );
    ~ATM_TRANSPARENT_APPL( void );

    BCMATM_STATUS Attach( ATM_INTERFACE *pInterfaces, UINT32 ulNumInterfaces,
        PATM_VCC_ATTACH_PARMS pAttachParms );
    BCMATM_STATUS Send( UINT32 ulInterfaceId, PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS Receive( PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS Detach( void );

private:
    // Passed in Attach.
    ATM_INTERFACE *m_pInterfaces;
    UINT32 m_ulNumInterfaces;
    FN_RECEIVE_CB m_pFnReceiveDataCb;
    UINT32 m_ulParmReceiveData;

    // Other private member data.
    UINT32 m_ulState;
    UINT8 m_ucShaperNums[MAX_INTERFACES];
    ATM_TX_BDQ *m_pTxQViaPort[MAX_INTERFACES];

    struct GFC_TBL_INFO
    {
        struct GFC_TBL_ENTRY
        {
            GFC_TBL_ENTRY *pNext;
            UINT32 ulGfcIdxAtmHdr;
        } *pGfcTableFreePool,
          *pGfcTable[MAX_GFC_TABLE_SIZE],
          GfcTableEntries[MAX_GFC_TABLE_ENTRIES];
    } m_GfcTableInfo[MAX_INTERFACES];

    // Private member functions.
    BCMATM_STATUS SetGfcIndicies( PATM_VCC_DATA_PARMS pDataParms,
        UINT8 ucPhyPort );
};
#else
class ATM_TRANSPARENT_APPL : public ATM_APPL
{
public:
    ATM_TRANSPARENT_APPL(ATM_OS_SERVICES *pOsServices,
        ATM_APPL_TABLE *pApplTable, UINT32 ulAtmApplSem, void *pSoftSar )
        : ATM_APPL( pOsServices, pApplTable, ulAtmApplSem, pSoftSar ){}
    ~ATM_TRANSPARENT_APPL( void ){}

    BCMATM_STATUS Attach( ATM_INTERFACE *pInterfaces, UINT32 ulNumInterfaces,
        PATM_VCC_ATTACH_PARMS pAttachParms )
        {return(STS_ERROR);}
    BCMATM_STATUS Send( UINT32 ulInterfaceId, PATM_VCC_DATA_PARMS pDataParms )
        {return(STS_ERROR);}
    BCMATM_STATUS Receive( PATM_VCC_DATA_PARMS pDataParms )
        {return(STS_ERROR);}
};
#endif


// ATM_APPL_TABLE contains a lookup table for ATM application objects.
class ATM_APPL_TABLE
{
public:
    ATM_APPL_TABLE( ATM_OS_SERVICES *pOsServices );
    ~ATM_APPL_TABLE( void );

    void SetSem( UINT32 ulMuSem )
        { m_ulMuSem = ulMuSem; }

    void RegisterAppl( ATM_APPL *pAppl );
    void UnregisterAppl( ATM_APPL *pAppl );
    ATM_APPL *EnumAppl( UINT32 *pulIdx );

    BCMATM_STATUS SetByVccAddr( PATM_VCC_ADDR pAddr, ATM_VCC_APPL *pAppl );
    BCMATM_STATUS SetTxQByShaperNum( UINT8 ucShaperNum, ATM_TX_BDQ *pTxQ );
    BCMATM_STATUS SetByRxVcid( UINT8 ucVcid, ATM_VCC_APPL *pAppl );

    ATM_VCC_APPL *GetByVccAddr( PATM_VCC_ADDR pAddr );
    ATM_VCC_APPL *GetByRxVcid( UINT8 ucRxVcid )
        { return( m_pRxVcids[ucRxVcid] ); }
    ATM_TX_BDQ *GetTxQByShaperNum( UINT8 ucShaperNum )
        { return( m_pTxQShaperNums[ucShaperNum] ); }

    BCMATM_STATUS Reset(ATM_APPL *pAppl, UINT8 ucShaperNum=MAX_QUEUE_SHAPERS+1);

private:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;

    // Passed in SetSem.
    UINT32 m_ulMuSem;

    // Other private member data.
    ATM_APPL *m_pApplTable[MAX_APPL_TABLE_ENTRIES + 1];
    ATM_VCC_APPL *m_pRxVcids[MAX_VCIDS];
    ATM_TX_BDQ *m_pTxQShaperNums[MAX_QUEUE_SHAPERS];
    struct VccAddrAtmAppl
    {
        ATM_VCC_ADDR Addr;
        ATM_VCC_APPL *pAppl;
    } m_VccAddrMap[MAX_VCCS];
};


// ATM_INTERFACE keeps information about an ATM interface (physical port).
class ATM_INTERFACE
{
public:
    ATM_INTERFACE( void );
    ~ATM_INTERFACE( void );

    BCMATM_STATUS Initialize( ATM_OS_SERVICES *pOsServices,
        UINT8 ucPhyPortNum, PATM_PORT_CFG pPortCfg, UINT32 ulAdslCfg,
        ATM_APPL_TABLE *pApplTable, void *pSoftSar );
    BCMATM_STATUS Uninitialize( void );
    UINT8 GetPhyPortNum( void )
        { return( m_ucPhyPortNum ); }
    UINT32 GetInterfaceId( void )
        { return( m_ulInterfaceId ); }
    BCMATM_STATUS SetCfg( PATM_INTERFACE_CFG pCfg );
    BCMATM_STATUS GetCfg(PATM_INTERFACE_CFG pCfg, ATM_VCC_INFO_TABLE *pVccInfo);
    BCMATM_STATUS GetStats( PATM_INTERFACE_STATS pStats, UINT32 ulReset );
    BCMATM_STATUS SetLinkInfo( UINT32 ulLinkState );
    UINT32 IsLinkUp( void )
        { return( (m_ulLinkState == LINK_UP) ? 1 : 0 ); }
    UINT32 IsInterfaceUp( void )
        { return( (m_Cfg.ulIfAdminStatus == ADMSTS_UP &&
          m_ucPortType != PT_DISABLED) ? 1 : 0 ); }
    ATM_INTERFACE_STATS *GetStats( void )
        { return( &m_Stats ); }
    void LockStats( void )
        { (*m_pOsServices->pfnRequestSem) (m_ulStatsLock, MAX_TIMEOUT); }
    void UnlockStats( void )
        { (*m_pOsServices->pfnReleaseSem) (m_ulStatsLock); }
    UINT8 GetPortType( void )
        { return( m_ucPortType ); }
#if defined(CONFIG_ATM_SOFTSAR)
    UINT32 IsPhyPort1Enabled( void )
        { return( 0 ); }
#else
    UINT32 IsPhyPort1Enabled( void )
        { return( (m_ulPhyPort1Enabled == 1 && AP_REGS->ulUtopiaCfg == 0  &&
            (AP_REGS->ulTxAtmHdrCfg & (1 << PHY_1)) == (1 << PHY_1))
            ? 1 : 0 ); }
#endif
    BCMATM_STATUS SetMgmtCellAppl( ATM_MGMT_APPL *pMgmtAppl, UINT32 ulFlags );
    BCMATM_STATUS ResetMgmtCellAppl( ATM_MGMT_APPL *pMgmtAppl, UINT32 ulFlags );
    BCMATM_STATUS SetTransparentAppl( ATM_TRANSPARENT_APPL *pTransparentAppl );
    BCMATM_STATUS ResetTransparentAppl(ATM_TRANSPARENT_APPL *pTransparentAppl);
    BCMATM_STATUS Receive( PATM_VCC_ADDR pVccAddr,
        PATM_VCC_DATA_PARMS pDataParms );
#if defined(CONFIG_ATM_SOFTSAR)
    void SoftSarTxCells( void );
#endif

private:
    // Passed in constructor.
    ATM_OS_SERVICES *m_pOsServices;

    // Passed in Initialize.
    UINT8 m_ucPhyPortNum;
    UINT8 m_ucPortType;
    UINT8 m_ucUtopiaFlags;
    UINT32 m_ulInterfaceId;
    UINT32 m_ulAdslCfg;
    UINT32 m_ulUtopiaCfg;
    ATM_APPL_TABLE *m_pApplTable;
    void *m_pSoftSar;

    // Passed in SetCfg.
    ATM_INTERFACE_CFG m_Cfg;

    // Passed in SetLinkState.
    UINT32 m_ulLinkState;

    // Passed in SetMgmtCellAppl.
    ATM_MGMT_APPL *m_pOamF5EToE;
    ATM_MGMT_APPL *m_pOamF5Seg;
    ATM_MGMT_APPL *m_pOamF4EToE;
    ATM_MGMT_APPL *m_pOamF4Seg;
    ATM_MGMT_APPL *m_pRm;
    ATM_MGMT_APPL *m_pErrorCells;

    // Passed in SetTransparentAppl.
    ATM_TRANSPARENT_APPL *m_pTransparentCells;

    // Other private member data.
    ATM_INTERFACE_STATS m_Stats;
    UINT32 m_ulStatsLock;
    UINT32 m_ulAdslTxDescrTableAddr;
    UINT32 m_ulAdslRxDescrTableAddr;
    UINT32 m_ulPhyPort1Enabled;
#if defined(CONFIG_ATM_SOFTSAR)
    UINT32 m_ulAdslTxHeadIdx;
    UINT32 m_ulAdslTxTableLength;
#endif

    // Private member functions.
    BCMATM_STATUS AdslInit( void );
    void UtopiaInit( UINT8 ucPortFlags );
    void ConfigureAtmProcessor( UINT32 ulEnable );
    void ProcessOamCell( PATM_VCC_DATA_PARMS pDataParms );
    static void FreeDp( PATM_VCC_DATA_PARMS pDataParms );
    void ChangeCircuitType( PATM_VCC_ADDR pVccAddr,
        PATM_VCC_DATA_PARMS pDataParms, UINT8 ucCt );
#if defined(CONFIG_ATM_SOFTSAR)
    static void TimerCb( UINT32 ulParm );
#endif
};


// ATM_PROCESSOR keeps information about an ATM Processor.
class ATM_PROCESSOR
{
public:
    ATM_PROCESSOR( void );
    ~ATM_PROCESSOR( void );

    BCMATM_STATUS Initialize( PATM_INITIALIZATION_PARMS pInitParms,
        ATM_OS_SERVICES *pOsServices, UINT32 ulBusSpeed );
    BCMATM_STATUS Uninitialize( void );
    BCMATM_STATUS GetInterfaceId( UINT8 ucPhyPortNum, UINT32 *pulInterfaceId );
    BCMATM_STATUS GetPriorityPacketGroup(UINT32 ulGroupNumber,
        PATM_PRIORITY_PACKET_ENTRY pPriorityPackets,
        UINT32 *pulPriorityPacketsSize);
    BCMATM_STATUS SetPriorityPacketGroup(UINT32 ulGroupNumber,
        PATM_PRIORITY_PACKET_ENTRY pPriorityPackets,
        UINT32 ulPriorityPacketsSize);
    BCMATM_STATUS GetTrafficDescrTableSize( UINT32 *pulTrafficDescrTableSize );
    BCMATM_STATUS GetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescrTable, UINT32 ulTrafficDescrTableSize );
    BCMATM_STATUS SetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
        pTrafficDescrTable, UINT32  ulTrafficDescrTableSize );
    BCMATM_STATUS GetInterfaceCfg( UINT32 ulInterfaceId, PATM_INTERFACE_CFG
        pInterfaceCfg );
    BCMATM_STATUS SetInterfaceCfg( UINT32 ulInterfaceId, PATM_INTERFACE_CFG
        pInterfaceCfg );
    BCMATM_STATUS GetVccCfg( PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg );
    BCMATM_STATUS SetVccCfg( PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg );
    BCMATM_STATUS GetVccAddrs( UINT32 ulInterfaceId, PATM_VCC_ADDR pVccAddrs,
        UINT32 ulNumVccs, UINT32 *pulNumReturned );
    BCMATM_STATUS GetInterfaceStatistics( UINT32 ulInterfaceId,
        PATM_INTERFACE_STATS pStatistics, UINT32 ulReset );
    BCMATM_STATUS GetVccStatistics( PATM_VCC_ADDR pVccAddr, PATM_VCC_STATS
        pVccStatistics, UINT32 ulReset );
    BCMATM_STATUS SetInterfaceLinkInfo( UINT32 ulInterfaceId,
        PATM_INTERFACE_LINK_INFO pInterfaceLinkInfo );
    BCMATM_STATUS SetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb );
    BCMATM_STATUS ResetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb );
    BCMATM_STATUS AttachVcc( PATM_VCC_ADDR pVccAddr, PATM_VCC_ATTACH_PARMS
        pAttachParms );
    BCMATM_STATUS AttachMgmtCells( UINT32 ulInterfaceId, PATM_VCC_ATTACH_PARMS
        pAttachParms );
    BCMATM_STATUS AttachTransparent( UINT32 ulInterfaceId, PATM_VCC_ATTACH_PARMS
        pAttachParms );

    void ConfigureInterrupts( void );
    void PrioritizeReceivePkts( void );
    void ProcessEvent( void );

private:
    // Passed in Initialize.
    ATM_OS_SERVICES m_OsServices;
    UINT32 m_ulPrioritizeReceivePkts;

    // Passed in SetTrafficDescrTable.
    PATM_TRAFFIC_DESCR_PARM_ENTRY m_pTrafficDescrEntries;
    UINT32 m_ulNumAtdpEntries;

    // Other private member data.
    ATM_NOTIFY m_Notify;
    ATM_APPL_TABLE m_ApplTable;
    ATM_VCC_INFO_TABLE m_VccTable;
    ATM_INTERFACE m_Interfaces[MAX_INTERFACES];
    ATM_FREE_BDQ m_FreePktQ;
    ATM_FREE_BDQ m_FreeCellQ;
    ATM_RX_BDQ m_RxPktQ;
    ATM_RX_BDQ m_RxCellQ;
    ATM_RX_BDQ m_HighPriRxPktQ;
    ATM_RX_BDQ m_LowPriRxPktQ;
    UINT32 m_ulHighPriRxPktQRegs[4]; // addr, head, tail, length
    UINT32 m_ulLowPriRxPktQRegs[4];  // addr, head, tail, length
    char *m_pPktQMem;
    char *m_pPktBufMem[16];
    char *m_pCellQMem;
    char *m_pCellBufMem[16];
    UINT32 m_ulAtmApplSem;
    UINT32 m_ulAdslSit;
    UINT32 m_ulUtopiaSit;
    UINT16 m_usFreePktQBufSize;
    UINT8 m_ucFreePktQBufSizeExp;
    UINT32 m_ulFreeQueueEmpty;
    UINT32 m_ulDropPktThreshold;
    ATM_PRIORITY_PACKET_ENTRY m_ulPriPktGroups[NUM_PRI_PKT_GROUPS]
        [NUM_ENTRIES_PER_PRI_PKT_GROUP + 1];
    UINT32 m_ulPriPktNumbers[MAX_VCIDS][NUM_PRI_PKT_GROUPS + 1];
    UINT32 m_ulPriPktsReceived;
    void *m_pSoftSar;
#if defined(CONFIG_ATM_SOFTSAR) || defined(CONFIG_ATM_RX_SOFTSAR)
    ATM_SOFT_SAR m_SoftSar;
#endif

    // Private member functions.
    void ProcessTransmitIntrComplete( UINT32 ulChStatus );
    void ProcessReceiveCellQueue( void );
    void ProcessReceivePktQueue( ATM_RX_BDQ *pRxPktQ );
    UINT32 IsHighPriorityPkt( ATM_DMA_BD *pBd );
    void CalculateReceiveBufferSize(UINT16 usConfiguredSize,
        UINT16 *pusActualSize, UINT8 *pucActualExp);
    BCMATM_STATUS CreateBdQ( char **ppQMem, char **ppBufMem, ATM_FREE_BDQ *pBdQ,
        UINT32 ulQSize, UINT16 usQBufSize, UINT8 ucQBufSizeExp,
        UINT16 usQBufOffset, FN_FREE_DATA_PARMS pfnFreeDp );
    static void FreeCell( PATM_VCC_DATA_PARMS pDataParms );
    static void FreePkt( PATM_VCC_DATA_PARMS pDataParms );
#if defined(CONFIG_BCM96338)
    static void TimerCb( UINT32 ulParm );
#endif
};

#endif // _ATMAPIIMPL_H_

