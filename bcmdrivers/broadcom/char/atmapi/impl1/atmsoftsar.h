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
// File Name  : AtmSsImpl.h
//
// Description: This file contains class definitions for the ATM Soft SAR
//              implementation.
//
//**************************************************************************

#if !defined(_ATMSSIMPL_H_)
#define _ATMSSIMPL_H_


// ATM service category types
#define kAtmSrvcCBR                     0x01 // Constant Bit Rate
#define kAtmSrvcVBR                     0x02 // Variable Bit Rate
#define kAtmSrvcUBR                     0x03 // Unspecified Bit Rate
#define kAtmSrvcABR                     0x04 // Available Bit Rate
#define kAtmSrvcUBRNoPCR                0x05 // Unspecified Bit Rate
#define kAtmSrvcUnknown                 0xFF

// ATM AAL types (as encoded at UNI)
#define kAtmAal5                        5

// ATM Traffic Descriptor types (as encoded at UNI)
#define kTrafficMaxTolerance            0x7FFFFFFF

// ATM setup bit definition
#define kAtmTxIdleTimeoutMask           0x6
#define kAtmTxIdleNoTimeout             0
#define kAtmTxIdleTimeout10s            2
#define kAtmTxIdleTimeout30s            4
#define kAtmTxIdleTimeout60s            6

// ATM status codes
#define kAtmStatRxDiscarded             100
#define kAtmStatTxDelayed               101

#define kAtmStatVcCreated               102
#define kAtmStatVcStarted               103
#define kAtmStatVcStopped               104
#define kAtmStatVcDeleted               105

#define kAtmStatTimeout                 106
#define kAtmStatNoCellMemory            107
#define kAtmStatPrintCell               108
#define kAtmStatInvalidCell             109
#define kAtmStatUnassignedCell          110
#define kAtmStatOamF4SegmentCell        111
#define kAtmStatOamF4End2EndCell        112
#define kAtmStatOamI371Cell             113
#define kAtmStatOamF5SegmentCell        114
#define kAtmStatOamF5End2EndCell        115
#define kAtmStatReservedCell            116

#define kAtmStatConnected               117
#define kAtmStatDisconnected            118

// ATM cell description
#define kAtmCellSize                    53
#define kAtmCellHdrSize                 5
#define kAtmCellDataSize                (kAtmCellSize - kAtmCellHdrSize)
#define kAtmHdrCfgNone                  0

// Pre-assigned VCIs
#define kAtmHdrVciUnassigned            0
#define kAtmHdrVciF4Segment             3
#define kAtmHdrVciF4End2End             4
#define kAtmHdrVciI371                  6
#define kAtmHdrVciReserved              7
#define kAtmHdrPtiNoCongSDU0            0
#define kAtmHdrPtiNoCongSDU1            1
#define kAtmHdrPtiCongSDU0              2
#define kAtmHdrPtiCongSDU1              3
#define kAtmHdrPtiF5Segment             4
#define kAtmHdrPtiF5End2End             5
#define kAtmHdrPtiI371                  6
#define kAtmHdrPtiReserved              7

#define kAtmVcTableSize                 AP_MAX_VCCS

// ATM Service Type Priorities
#define kPriorityCbr                    12
#define kPriorityRtVbr                  8
#define kPriorityNrtVbr                 4
#define kPriorityUbr                    0

// Time calculations.
#define TIME_SEC_TO_ATM(t)              ((UINT32) (t) * (CPU_SPEED / 2))

// ATM VC Table function prototypes
#define AtmVcTableInit(t,n,p)           (t)->vctblHdr.Init(n, \
                                             sizeof(atmssVcTableItem),p)
#define AtmVcTableFindKey(t,key)        (t)->vctblHdr.FindKey(key)
#define AtmVcTableFindOrAddKey(t,key,f) (t)->vctblHdr.FindOrAddKey(key,f)
#define AtmVcTableAddKey(t,key)         (t)->vctblHdr.AddKey(key)
#define AtmVcTableDelKey(t,key)         (t)->vctblHdr.DelKey(key)

// AAL5 definitions.
#define ATM_AAL5_PAD            0
#define ATM_AAL5_MAX_DATA_LEN   0xFFFF

#define AAL5_CRC_INIT           ((long) -1)
#define AAL5_CRC_FINAL(crc)     ((crc) ^ 0xFFFFFFFF)
#define Aal5UpdateCrc(crc,b)    ((crc)<<8) ^ ms_AtmCrc32Table[((crc)>>24) ^ (b)]

#define AAL5_GOOD_CRC           0xC704DD7B

#define CELL_TYPE_ATM           0
#define CELL_TYPE_ADSL          1

// Hash table definitions.
#define HASH_TBL_NULL       0xFFFF
#define hashTblItemData(h)  (((UINT8 *) (h)) + sizeof(atmssHashTableItemHeader))
#define hashTblData()       (((UINT8 *) (this)) + sizeof(atmssHashTable))
#define HashTblEntry(n)     ((atmssHashTableItemHeader *) \
                            (void *) (hashTblData() + (n) * m_itemLength))

// For now, put definition here.
#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96348)
#define CPU_SPEED           240000000
#endif

// GNU compiler and MIPS processor specific macro.
#define __read_cp0_count() \
({  int __res;             \
    __asm__ __volatile__(  \
    "mfc0\t%0, $9\n\t"     \
    : "=r" (__res));       \
    __res;                 \
})


extern void (*g_pfnSsPrintf) (char *, ...); 
struct ATM_OS_SERVICES;
class ATM_VCC_APPL;

typedef UINT32 atmssCellHeader;
typedef UINT8 atmssCellData[kAtmCellDataSize];

struct atmssAtmCell
{
    atmssCellData   data;
};

struct atmssAdslCell
{
    UINT32          reserved1;
    atmssCellData   data;
    UINT32          reserved2[2];
};

struct atmssCell
{
    atmssCellHeader hdr;
    union
    {
        atmssAtmCell atmCell;
        atmssAdslCell adslCell;
    };
};

// Double linked list operations.
class atmssDllHeader
{
public:
    atmssDllHeader( void )
    {
        m_next = m_prev = NULL;
    }

    ~atmssDllHeader( void ){}

    void Init( void )
    {
        if( m_next != NULL || m_prev != NULL )
        {
            (*g_pfnSsPrintf) ("(%8.8lx) Init - next or prev is "
                "not NULL, next=0x%8.8lx, prev=0x%8.8lx\n", m_next, m_prev);
        }
        m_next = m_prev = this;
    }

    atmssDllHeader *Next( void )
    {
        return( m_next );
    }

    atmssDllHeader *Prev( void )
    {
        return( m_prev );
    }

    int EntryLinked( void )
    {
        return( m_next != NULL );
    }

    void UnlinkEntry( void )
    {
        m_next = m_prev = NULL;
    }

    atmssDllHeader *First( void )
    {
        return( m_next );
    }

    int Valid( atmssDllHeader *pEntry )
    {
        return( this != pEntry );
    }

    int Empty( void )
    {
        return( this == m_next );
    }

    void InsertAfter( atmssDllHeader *pEntry )
    {
        if( pEntry->m_next )
        {
            (*g_pfnSsPrintf) ("(%8.8lx) InsertAfter 0x%8.8lx - ENTRY ALREADY "
                "LINKED\n           next=0x%8.8lx, prev=0x%8.8lx\n",
                (UINT32) this, (UINT32) pEntry, (UINT32) pEntry->m_next,
                (UINT32) pEntry->m_prev);
        }
        pEntry->m_next = m_next;
        pEntry->m_prev = this;
        m_next->m_prev = pEntry;
        m_next = pEntry;
    }

    void InsertBefore( atmssDllHeader *pEntry )
    {
        if( pEntry->m_next )
        {
            (*g_pfnSsPrintf) ("(%8.8lx) InsertBefore 0x%8.8lx - ENTRY ALREADY "
                "LINKED\n           next=0x%8.8lx, prev=0x%8.8lx\n",
                (UINT32) this, (UINT32) pEntry, (UINT32) pEntry->m_next,
                (UINT32) pEntry->m_prev);
        }
        pEntry->m_next = this;
        pEntry->m_prev = m_prev;
        m_prev->m_next = pEntry;
        m_prev = pEntry;
    }

    void InsertTail( atmssDllHeader *pEntry )
    {
        InsertBefore( pEntry );
    }

    void InsertHead( atmssDllHeader *pEntry )
    {
        InsertAfter( pEntry );
    }

    void Remove( void )
    {
        m_prev->m_next = m_next;
        m_next->m_prev = m_prev;
        m_next = m_prev = NULL;
    }

    void SetParam( void *param )
    {
        m_param = param;
    }

    void *GetParam( void )
    {
        return( m_param );
    }

private:
    atmssDllHeader *m_next;
    atmssDllHeader *m_prev;
    void *m_param;
};

class atmssFrame;
class atmssFramePool;
class atmssFrameBufferPool;

// Contains information about ATM packet data.
class atmssFrameBuffer
{
public:
    friend class atmssFrame;
    friend class atmssFrameBufferPool;

    UINT32 GetLength( void )
    {
        return( m_length );
    }

    UINT8 *GetAddress( void )
    {
        return( m_addr );
    }

    UINT32 GetBufSize( void )
    {
        return( m_bufsize );
    }

    void SetLength( UINT32 length )
    {
        m_length = length;
    }

    void SetAddress( UINT8 *address )
    {
        m_addr = address;
    }

    void SetBufSize( UINT32 bufsize )
    {
        m_bufsize = bufsize;
    }

    atmssFrameBuffer *GetNextBuffer( void )
    {
        return( m_next );
    }

private:
    struct atmssFrameBuffer *m_next; // link to the next buffer in the frame
    UINT8 *m_addr;                   // pointer to data
    UINT32 m_length;                 // size (in bytes) of data
    UINT32 m_bufsize;                // size of allocated buffer
};

// Manages preallocated array of atmFrameBuffers.
class atmssFrameBufferPool
{
public:
    UINT32 AllocPool( ATM_OS_SERVICES *pOsServices, UINT32 frameBufNum );
    void FreePool( ATM_OS_SERVICES *pOsServices );

    atmssFrameBuffer *GetFrameBuffer( void )
    {
        atmssFrameBuffer *b = m_first;

        if( m_first )
        {
            m_first = m_first->m_next;
            b->m_next = NULL;
        }

        return( b );
    }

    void PutFrameBuffer( atmssFrameBuffer *b )
    {
        b->m_next = m_first;
        m_first = b;
    }

private:
    atmssFrameBuffer *m_pool;
    atmssFrameBuffer *m_first;
};

// The starting point of an ATM packet.
class atmssFrame
{
public:
    friend class atmssFramePool;

    void Init( void );
    atmssFrameBuffer *DequeBuffer( void );
    void EnqueBufferAtFront( atmssFrameBuffer *b );
    void EnqueBufferAtBack( atmssFrameBuffer *b );
    void EnqueFrameAtFront( atmssFrame *f );

    void SetUserParam( UINT32 userParam )
    {
        m_userParam = userParam;
    }

    UINT32 GetUserParam( void )
    {
        return( m_userParam );
    }

    UINT32 GetLength( void )
    {
        return( m_totalLength );
    }

    UINT32 GetBufCount( void )
    {
        return( m_bufCnt );
    }

    atmssFrameBuffer *GetFirstBuffer( void )
    {
        return( m_head );
    }

    atmssDllHeader *GetLinkFieldAddress()
    {
        return( &m_DllHeader );
    }

private:
    atmssDllHeader m_DllHeader; // link on VC list
    struct atmssFrame *m_next;  // link on frame pool
    UINT32 m_userParam;
    UINT32 m_totalLength;       // total amount of data in the packet
    UINT32 m_bufCnt;            // buffer counter
    atmssFrameBuffer *m_head;   // first buffer in the chain
    atmssFrameBuffer *m_tail;   // last buffer in the chain

    void BufRemove( void )
    {
        if( m_head )
        {
            m_head = m_head->m_next;
            if( m_head == NULL )
                m_tail = NULL;
        }
    }

    void BufAddFirst( atmssFrameBuffer *b )
    {
        if( m_tail == NULL )
            m_tail = b;
        b->m_next = m_head;
        m_head = b;
    }

    void BufAddLast( atmssFrameBuffer *b )
    {
        b->m_next = NULL;
        if( m_tail )
            m_tail->m_next = b;
        else
            m_head = b;
        m_tail = b;
    }

    void BufAppend( atmssFrame *f )
    {
        if( m_tail == NULL )
            m_head = f->m_head;
        else
            m_tail->m_next = f->m_head;

        if( f->m_tail )
            m_tail = f->m_tail;
    }

    void BufPrepend( atmssFrame *f )
    {
        if( f->m_tail == NULL )
            f->m_head = m_head;
        else
            f->m_tail->m_next = m_head;

        if( m_tail )
            f->m_tail = m_tail;

        m_head = f->m_head;
        m_tail = f->m_tail;
    }
};

// Manages preallocated array of atmFrames.
class atmssFramePool
{
public:
    atmssFramePool(void){m_pool = m_first = NULL;}
    ~atmssFramePool(void){}

    UINT32 AllocPool( ATM_OS_SERVICES *pOsServices, UINT32 frameNum );
    void FreePool( ATM_OS_SERVICES *pOsServices );

    atmssFrame *GetFrame( void )
    {
        atmssFrame *f = m_first;

        if( m_first )
        {
            m_first = m_first->m_next;
            f->m_next = NULL;
        }

        return( f );
    }

    void PutFrame( atmssFrame *f )
    {
        f->m_next = m_first;
        m_first = f;
    }

private:
    atmssFrame *m_pool;
    atmssFrame *m_first;
};

struct atmssHashTableItemHeader
{
    struct atmssHashTableItemHeader *link;
    UINT32 key;
    UINT16 next;
    UINT16 head;
};

// Lookup table for finding a VC.
class atmssHashTable
{
public:
    UINT32 Init( int items, int itemLength, int hashParam );
    void *FindKey( UINT32 key );
    void *FindOrAddKey( UINT32 key, UINT32 *newFlag );
    void DelKey( UINT32 key );

private:
    int m_hashParam;
    int m_hashParamRecip;
    int m_items;
    int m_itemLength;
    atmssHashTableItemHeader *m_hashTblEmptyList;

    UINT16 GetHashIndex( UINT32 key );
};

struct aal5StateType
{
    atmssDllHeader    dlistTxWaiting[MAX_TX_PRIORITIES];
    UINT32            dlistTxWaitingCount[MAX_TX_PRIORITIES];
    UINT32            dlistTxWaitingMaxSize;
    atmssFrame       *pTxPacket;
    atmssFrameBuffer *pTxCellBuffer;
    UINT8            *pTxCellData;
    UINT8            *pTxCellDataEnd;
    UINT32            txCrc;
    UINT32            txLength;
    UINT32            bTxPadNext;
#if defined(CONFIG_ATM_RX_SOFTSAR)
    atmssFrame       *pRxPacket;
    UINT32            rxCrc;
#endif
};

// Traffic shaping parameters.
struct atmssTrafficParams
{
    long ServiceType;       // CBR, VBR, UBR, etc.
    long tPCR;              // CLP = 0+1
    long tSCR;              // CLP = 0+1
    long BT;                // tolerance for SCR in us
    long Priority;          // priority of the service type
    long CurrHighSubPri;    // current highest sub-priority packet queued
};

// Virtual channel parameters.
struct atmssVcParams
{
    UINT32 vpivci;
    UINT32 clp;
    UINT8  uui;
    UINT8  cpi;
    atmssTrafficParams txTrafficParams;
};

// Information about a VC.
struct atmssVcState 
{
    atmssVcParams vcParams;  // VC params, has to the 1st
    aal5StateType aal5State; // AAL specific state

    // Transmit frame and buffer structures
    atmssFramePool       txFrameList;
    atmssFrameBufferPool txBufferList;

    // Traffic management data
    atmssDllHeader dlistAllVc;
    atmssDllHeader dlistTxVcWaiting;
    UINT32         txPcrLCS; // PCR time that last cell was sent
    UINT32         txScrLCS; // SCR time that last cell was sent
};

struct atmssVcTableItem
{
    atmssHashTableItemHeader hdr;
    atmssVcState vc;
};

struct atmssVcTable 
{
    atmssHashTable vctblHdr;
    atmssVcTableItem VcTableData[kAtmVcTableSize];
};

// Top level class for an ATM Soft SAR. (Transmit Only.)
class ATM_SOFT_SAR
{
public:
    ATM_SOFT_SAR( ATM_OS_SERVICES *pOsServices );
    ~ATM_SOFT_SAR( void );

    BCMATM_STATUS Initialize( UINT32 ulAtmApplSem )
    {
        m_ulAtmApplSem = ulAtmApplSem;
        return( AtmLayerInit() );
    }
    void SetConnectionRate( UINT32 ulConnectRate )
    {
        m_connectRate = ulConnectRate;
        if( ulConnectRate )
            m_cellTicks = 424 * ((CPU_SPEED / 2) / ulConnectRate);
    }
    UINT8 AssignVcid( ATM_VCC_ADDR *pAddr );
    void UnassignVcid( UINT8 ucVcid );
    ATM_VCC_ADDR *GetVccAddr( UINT8 ucVcid );
    BCMATM_STATUS CreateVc( UINT8 ucVcid, UINT32 ulTxPoolSize,
        UINT32 ulNumPriorities, PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    BCMATM_STATUS DestroyVc( UINT8 ucVcid );
    BCMATM_STATUS AddPkt( UINT8 ucVcid, PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS AddCell( PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS GetCell( atmssCell *pCell, UINT32 ulCellType )
    {
        return( AtmLayerGetNextTxCell( pCell, ulCellType ) );
    }
    void CheckTimerValues( UINT32 ulIntervalMs )
    {
        AtmLayerTimer( ulIntervalMs );
    }
    void SetShapingValues( UINT8 ucVcid, PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
#if defined(CONFIG_ATM_RX_SOFTSAR)
    BCMATM_STATUS CreateRxBufs( UINT32 ulNumRxBufs, UINT32 ulRxBufSize,
        UINT16 usQBufOffset );
    PATM_VCC_DATA_PARMS ProcessRxCell( PATM_VCC_DATA_PARMS pDataParms );
    BCMATM_STATUS RegisterRxAppl( ATM_VCC_ADDR *pAddr, ATM_VCC_APPL *pAppl );
    BCMATM_STATUS UnregisterRxAppl( ATM_VCC_APPL *pAppl );
    ATM_VCC_APPL *FindRxAppl( UINT32 ulAtmHdr )
    {
        if( m_vccApplMap[0].ulAtmHdr == ulAtmHdr )
            return( m_vccApplMap[0].pAppl );
        else
            if( m_vccApplMap[1].ulAtmHdr == ulAtmHdr )
                return( m_vccApplMap[1].pAppl );
            else
                if( m_vccApplMap[2].ulAtmHdr == ulAtmHdr )
                    return( m_vccApplMap[2].pAppl );
                else
                    if( m_vccApplMap[3].ulAtmHdr == ulAtmHdr )
                        return( m_vccApplMap[3].pAppl );
                    else
                        return( NULL );
    }
#endif

#if 0 /* DEBUG */
    void DebugDumpInfo( void );
#endif

private:
    ATM_OS_SERVICES         *m_pOsServices;
    UINT32                   m_ulAtmApplSem;

    ATM_VCC_ADDR            *m_VccAddrForVcid[32];
    atmssVcTable             m_VcTable;
    atmssVcState            *m_VcStateForVcid[32];
    UINT32                   m_vcidMask;

    // Traffic management data
    atmssDllHeader           m_dlistAllVc;
    atmssDllHeader           m_dlistTxVcWaiting;
    atmssDllHeader           m_dlistTxVcUbrWaiting;
    UINT32                   m_timeCurrent;        // current ATM time
    UINT32                   m_lastTxCellTime;
    long                     m_txIdleTimeOut;
    long                     m_MaxDiff;
    long                     m_MinDiff;

    // Timer data
    UINT32                   m_lastIdleTimerTime;
    UINT32                   m_atmssConnected;
    UINT32                   m_connectRate;
    UINT32                   m_cellTicks;
    UINT32                   m_cellsToFill;

    // Cell list
    PATM_VCC_DATA_PARMS      m_cellHead;
    PATM_VCC_DATA_PARMS      m_cellTail;

#if defined(CONFIG_ATM_RX_SOFTSAR)
    // Receive pool
    atmssFramePool           m_rxFrameList;
    UINT8                   *m_rxMem[16];
    struct vcc_appl_map
    {
        UINT32 ulAtmHdr;
        ATM_VCC_APPL *pAppl;
    } m_vccApplMap[AP_MAX_RX_VCIDS];
#endif

    static UINT32            ms_AtmCrc32Table[256];

    // Soft SAR Layer
    void StatusHandler( UINT32 statusCode, ... );

    // ATM Layer
    BCMATM_STATUS AtmLayerInit( void );
    BCMATM_STATUS AtmLayerVcAllocate( UINT32 vpivci, UINT32 ulTxPoolSize,
        UINT32 ulNumPriorities, atmssVcState **ppVc,
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    void AtmLayerSetShapingValues( atmssTrafficParams *pTxParams,
        PATM_TRAFFIC_DESCR_PARM_ENTRY pEntry );
    BCMATM_STATUS AtmLayerVcActivate( atmssVcState *pVc );
    void AtmLayerVcDeactivate( atmssVcState *pVc );
    void AtmLayerVcFree( atmssVcState *pVc );
    void AtmLayerQueueFrame(atmssVcState *pVc, atmssFrame *pFrame, UINT32 ulPri);
    BCMATM_STATUS AtmLayerGetNextTxCell( atmssCell *pCell, UINT32 cellType );
    void AtmLayerSendComplete( atmssVcState *pVc, atmssFrame *pFrame );
    void AtmLayerTimer( UINT32 ulIntervalMs );
    UINT32 AtmLayerGetCpuTick()
    {
        UINT32 ret = __read_cp0_count();
        return( ret );
    }
    void TxFlush( void );
    long TimeDiff( UINT32 t1, UINT32 t2 )
    {
        long lRet = (long) (t1 - t2);
        if( lRet < m_MinDiff || lRet > m_MaxDiff )
            lRet = (long) (((UINT32) -1 - t2) + t1 + 1);
        return( lRet );
    }

    // ATM AAL5
    void AtmAal5VcInit( atmssVcState *pVc );
    UINT32 AtmAal5TxCell( atmssVcState *pVc, atmssCell *pCell, UINT32 cellType );
    void AtmAal5TxCompletePDU( atmssVcState *pVc, UINT8 *pData,
        UINT8 *pDataEnd, atmssCell *pCell, UINT32 cellType );
    void AtmAal5TxNextSDU( atmssVcState *pVc );
    void AtmAal5TxCellNextByte(atmssVcState *pVc, UINT8 *pData, UINT8 b);
    void AtmAal5VcTxFlush( atmssVcState *pVc );
    void AtmAal5VcClose( atmssVcState *pVc );
    UINT32 AtmAal5MakeCellHeader(UINT32 vpivci, UINT32 gfc, UINT32 pt,UINT32 clp)
    {
        return( ((gfc << 28) & 0xf0000000) | ((vpivci << 4)  & 0x0ffffff0) |
                ((pt << 1) & 0x0000000e)   | (clp & 0x00000001) );
    }

#if defined(CONFIG_ATM_RX_SOFTSAR)
    PATM_VCC_DATA_PARMS AtmAal5RxCell( atmssVcState *pVc, atmssCell *pCell );
    PATM_VCC_DATA_PARMS AtmOamF4RxCell( PATM_VCC_DATA_PARMS pDataParms );
    PATM_VCC_DATA_PARMS AtmOamF5RxCell( UINT8 pti,
        PATM_VCC_DATA_PARMS pDataParms );
    static void FreeRxPkt( PATM_VCC_DATA_PARMS pDataParms );
#endif
};

#endif

