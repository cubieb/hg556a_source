/*
<:copyright-broadcom 
 
 Copyright (c) 2003 Broadcom Corporation 
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
/**************************************************************************
 * File Name  : bcm63xx_usb.h
 *
 * Description: This file contains constant definitions, structure definitions
 *              and function prototypes for a USB network interface driver
 *              that executes on the BCM963xx reference platforms.
 *
 * Updates    : 02/19/2002  lat.   Created for Linux.
 *              11/04/2003         Changes for BCM6348.
 ***************************************************************************/

#if !defined(_BCM63XX_USB_H)
#define _BCM63XX_USB_H

#if defined __cplusplus
extern "C" {
#endif

/* Standard Request Codes */
#define GET_STATUS                      0
#define CLEAR_FEATURE                   1
#define SET_FEATURE                     3
#define SET_ADDRESS                     5
#define GET_DESCRIPTOR                  6
#define SET_DESCRIPTOR                  7
#define GET_CONFIGURATION               8
#define SET_CONFIGURATION               9
#define GET_INTERFACE                   10
#define SET_INTERFACE                   11
#define SYNCH_FRAME                     12

/* supported class specific Request Codes */
#define SEND_ENCAPSULATED_COMMAND       0x00
#define GET_ENCAPSULATED_RESPONSE       0x01
#define SET_ETHERNET_PACKET_FILTER      0x43

/* bmRequestType Types */
#define RT_TYPE_MASK                    0x60
#define RT_TYPE_STANDARD                0x00
#define RT_TYPE_CLASS                   0x20
#define RT_TYPE_VENDOR                  0x40

/* These packet filter definitions are defined in the Microsoft DDK ndis.h. */
#define NDIS_PACKET_TYPE_DIRECTED       0x00000001
#define NDIS_PACKET_TYPE_MULTICAST      0x00000002
#define NDIS_PACKET_TYPE_ALL_MULTICAST  0x00000004
#define NDIS_PACKET_TYPE_BROADCAST      0x00000008
#define NDIS_PACKET_TYPE_SOURCE_ROUTING 0x00000010
#define NDIS_PACKET_TYPE_PROMISCUOUS    0x00000020
#define NDIS_PACKET_TYPE_SMT            0x00000040
#define NDIS_PACKET_TYPE_ALL_LOCAL      0x00000080
#define NDIS_PACKET_TYPE_GROUP          0x00001000
#define NDIS_PACKET_TYPE_ALL_FUNCTIONAL 0x00002000
#define NDIS_PACKET_TYPE_FUNCTIONAL     0x00004000
#define NDIS_PACKET_TYPE_MAC_FRAME      0x00008000

/* These packet filter definitions are defined in the USB CDC specification. */
#define USB_PACKET_TYPE_MULTICAST       0x0010  /* disable is optional */
#define USB_PACKET_TYPE_BROADCAST       0x0008  /* disable is optional */
#define USB_PACKET_TYPE_DIRECTED        0x0004  /* disable is optional */
#define USB_PACKET_TYPE_ALL_MULTICAST   0x0002  /* disable is MUST */
#define USB_PACKET_TYPE_PROMISCUOUS     0x0001  /* disable is MUST */

/* Descriptor Types */
#define DEVICE_DESC                     1
#define CONFIGURATION_DESC              2
#define STRING_DESC                     3
#define INTERFACE_DESC                  4
#define ENDPOINT_DESC                   5

/* Miscellaneous definitions */
#define USB_DMA_MAX_BURST_LENGTH        8       /* in 32 bit words */
#define USB_VENDOR_ID                   0x0a5c
#define USB_DSL_PRODUCT_ID              0x6300
#define STRING_TABLE_SIZE               7
#define MAX_USB_FIFO_WAIT_CNT           2000000
#define BCM_USB_NIC_DBG                 0x00

/* NDIS and RNDIS definitions that are not in rndis.h but are documented on
 * MSDN.
 */
#define RNDIS_OID_GEN_PHYSICAL_MEDIUM           0x00010202
#define RNDIS_OID_GEN_RNDIS_CONFIG_PARAMETER    0x0001021b
#define NdisPhysicalMediumDSL                   5
#define NdisHardwareStatusReady                 0
#define NDIS_MAC_OPTION_NO_LOOPBACK             8

/*----------------------------------------------------------------------
 * endianess conversion MACROS
 *---------------------------------------------------------------------*/

/* Endian conversions. */
#if defined(CONFIG_CPU_LITTLE_ENDIAN) /*processor is configured as big endian*/

#define BE_SWAP2(x) (x)
#define BE_SWAP4(x) (x)
#define LE_SWAP2(x) (((unsigned short) (x) << 8) | ((unsigned short) (x) >> 8))
#define LE_SWAP4(x) (((unsigned long) (x) << 24) | \
                      (((unsigned long) (x) << 8) & 0x00ff0000) | \
                      (((unsigned long) (x) >> 8) & 0x0000ff00) | \
                      ((unsigned long) (x) >> 24))

#else /* processor is configured as big endian */

#define BE_SWAP2(x) (((unsigned short) (x) << 8) | ((unsigned short) (x) >> 8))
#define BE_SWAP4(x) (((unsigned long) (x) << 24) | \
                      (((unsigned long) (x) << 8) & 0x00ff0000) | \
                      (((unsigned long) (x) >> 8) & 0x0000ff00) | \
                      ((unsigned long) (x) >> 24))
#define LE_SWAP2(x) (x)
#define LE_SWAP4(x) (x)

#endif

                              
/*----------------------------------------------------------------------
 * USB protocol structures
 *---------------------------------------------------------------------*/

/* Enable structure packing. */
#pragma pack(1)

#if defined(__GNUC__)
#define __packed__
#define PACKED  __attribute__((packed))
#else
#define PACKED
#endif

typedef enum
{
    USB_NT_LINKSTATUS,
    USB_NT_SPEEDCHANGE,
    USB_NT_NUMITEMS
} USB_NOTIFY_TYPE;    

/* USB setup command */
typedef struct
{
    UINT8  bmRequestType;
    UINT8  bRequest;      
    UINT16 wValue;
    UINT16 wIndex;
    UINT16 wLength;   
} PACKED USB_setup;

/* Device descriptor */
typedef struct
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT16 bcdUSB;
    UINT8  bDeviceClass;
    UINT8  bDeviceSubClass;
    UINT8  bDeviceProtocol;
    UINT8  bMaxPacketSize0;
    UINT16 idVendor;
    UINT16 idProduct;
    UINT16 bcdDevice;
    UINT8  iManufacturer;
    UINT8  iProduct;
    UINT8  iSerialNumber;
    UINT8  bNumConfigurations;
} PACKED MCNS_DeviceDesc;

/* Configuration descriptor */
typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 wTotalLengthLow;
    UINT8 wTotalLengthHigh;
    UINT8 bNumInterfaces;
    UINT8 bConfigurationValue;
    UINT8 iConfiguration;
    UINT8 bmAttributes;
    UINT8 MaxPower;
} PACKED MCNS_ConfigDesc;

/* Communication Class descriptor */
typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bInterfaceNumber;
    UINT8 bAlternateSetting;
    UINT8 bNumEndpoints;
    UINT8 bInterfaceClass;
    UINT8 bInterfaceSubClass;
    UINT8 bInterfaceProtocol;
    UINT8 iInterface;
} PACKED MCNS_InterfaceDesc;

/* Communication Class (ethernet specific) descriptor */
typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bDescriptorSubType; 
    UINT8 bcdCDCLow;
    UINT8 bcdCDCHigh;
  
    UINT8 bLength_0;
    UINT8 bDescriptorType_0;
    UINT8 bDescriptorSubType_0;
    UINT8 bMasterInterface;
    UINT8 bSlaveInterface0;

    UINT8 bLength_1;
    UINT8 bDescriptorType_1;
    UINT8 bDescriptorSubType_1;
    UINT8 bMacAddressIndex; /* CDC 1.10  UINT8 bMacAddress[6];  // CDC 1.08 */
    UINT8 bmEthernetStatisticsLowest;
    UINT8 bmEthernetStatisticsLow;
    UINT8 bmEthernetStatisticsHigh;
    UINT8 bmEthernetStatisticsHighest;
    UINT8 wMaxSegmentSizeLow;
    UINT8 wMaxSegmentSizeHigh;
    UINT8 wNumberMCFiltersLow;
    UINT8 wNumberMCFiltersHigh;
    UINT8 bNumberPowerFilters;
} PACKED COMM_classDesc;

typedef struct
{
    UINT8 bLength_1;
    UINT8 bDescriptorType_1;
    UINT8 bDescriptorSubType_1; 
    UINT8 bcdCDCLow;
    UINT8 bcdCDCHigh;
  
    UINT8 bLength_2;
    UINT8 bDescriptorType_2;
    UINT8 bDescriptorSubType_2;
    UINT8 bmCapabilities_2;
    UINT8 bDataInterface;
  
    UINT8 bLength_3;
    UINT8 bDescriptorType_3;
    UINT8 bDescriptorSubType_3;
    UINT8 bmCapabilities_3;

    UINT8 bLength_4;
    UINT8 bDescriptorType_4;
    UINT8 bDescriptorSubType_4;
    UINT8 bMasterInterface;
    UINT8 bSlaveInterface0;
} PACKED RNDIS_classDesc;

/* Endpoint Descriptor */
typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bEndpointAddress;
    UINT8 bmAttributes;
    UINT8 wMaxPacketSizeLow;
    UINT8 wMaxPacketSizeHigh;
    UINT8 bInterval;  
} PACKED MCNS_EndpointDesc;

typedef struct
{
    MCNS_DeviceDesc deviceDesc;

    MCNS_ConfigDesc rndis_configDesc;
    MCNS_InterfaceDesc rndis_interfaceDesc;
    RNDIS_classDesc rndis_comm_classDesc;
    MCNS_EndpointDesc rndis_endpoint_notificationDesc;
    MCNS_InterfaceDesc rndis_dataClassDesc;
    MCNS_EndpointDesc rndis_endpoint_bulkDataInDesc;
    MCNS_EndpointDesc rndis_endpoint_bulkDataOutDesc;

    MCNS_ConfigDesc cdc_configDesc;
    MCNS_InterfaceDesc cdc_interfaceDesc;
    COMM_classDesc cdc_comm_classDesc;
    MCNS_EndpointDesc cdc_endpoint_notificationDesc;
    MCNS_InterfaceDesc cdc_dataClassDesc;
    MCNS_InterfaceDesc cdc_dataClassDescAlt;
    MCNS_EndpointDesc cdc_endpoint_bulkDataInDesc;
    MCNS_EndpointDesc cdc_endpoint_bulkDataOutDesc;
} PACKED UsbConfiguration;

/* String Descriptor structure (Generic) */
#define GENERIC_STRING_LEN  128
typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 genericString[GENERIC_STRING_LEN];      
} PACKED StringDesc;          

typedef struct
{
    char string[GENERIC_STRING_LEN];
} PACKED StringTable;

#undef PACKED
#pragma pack()

#define  USB_NIC_ALIGN(addr, bound) (((UINT32) addr + bound - 1) & ~(bound - 1))

/*----------------------------------------------------------------------
 * Linux driver Variables and Prototypes
 *---------------------------------------------------------------------*/

/* Configuration items */
//#define USB_NUM_RX_PKTS             (1 << 7)
#define USB_NUM_RX_PKTS             (1 << 6)
#define USB_RX_BD_RING_SIZE         (1 << 6)
#define USB_NR_RX_BDS               USB_RX_BD_RING_SIZE
#define USB_NR_TX_BDS               100
#define NR_TX_HDRS                  (USB_NR_TX_BDS / 2)
#define NR_RX_CTRL_BDS              4
#define USB_MAX_FRAME_SIZE          1500
#define USB_MAX_MTU_SIZE            (USB_MAX_FRAME_SIZE + sizeof(ENET_HDR))
#define RX_USB_BUF_OVERHEAD         22
#define RX_USB_SKB_RESERVE          16
#define RX_USB_ALIGN                ((USB_DMA_MAX_BURST_LENGTH * 8) - 1)
#define RX_USB_NIC_BUF_SIZE         ((USB_MAX_MTU_SIZE + RX_USB_BUF_OVERHEAD + \
                                     sizeof(RNDIS_MESSAGE) + RX_USB_ALIGN) \
                                     & ~RX_USB_ALIGN)
#define RX_USB_ALLOC_BUF_SIZE       ((RX_USB_NIC_BUF_SIZE + RX_USB_SKB_RESERVE +\
                                      0x0f) & ~0x0f)
#define RX_USB_ALLOC_TOTAL_SIZE     (RX_USB_ALLOC_BUF_SIZE + \
                                     ((sizeof(struct skb_shared_info) + 0x0f) & \
                                      ~0x0f))
#define SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define MAC_ADDR_LEN                6
#define SERIAL_NUM_INDEX            3
#define EH_SIZE                     14
#define USB_TIMEOUT                 (HZ/20)
#define MAX_MULTICAST_ADDRESSES     32
#define MAX_CTRL_PKT_SIZE           224
#define RNDIS_PACKET_OVERHEAD       8

#define USB_FLAGS_PLUGGED_IN        0x01 /* USB cable is physically plugged in*/
#define USB_FLAGS_CONNECTED         0x02 /* data can now be sent and received */
#define USB_FLAGS_OPENED            0x04 /* OS has opened this network device */
#define USB_FLAGS_RNDIS_SOFT_RESET  0x08 /* RNDIS_RESET_REQUEST */

#define HOST_DRIVER_RNDIS           0x01
#define HOST_DRIVER_CDC             0x02

#define RNDIS_STATE_UNINITIALIZED       0
#define RNDIS_STATE_INITIALIZED         1
#define RNDIS_STATE_DATA_INITIALIZED    2

#define REMOTE_NDIS_INVALID_MSG     0x7fffffff

#if defined(CONFIG_BCM96348)
#define NR_USB_DMA_CHANNELS         6
#define FIRST_USB_DMA_CHANNEL       0
#define USB_BULK_RX_CHAN            0
#define USB_BULK_TX_CHAN            1
#define USB_ISO_RX_CHAN             2
#define USB_ISO_TX_CHAN             3
#define USB_CNTL_RX_CHAN            4
#define USB_CNTL_TX_CHAN            5
#elif defined(CONFIG_BCM96338)
#define NR_USB_DMA_CHANNELS         4
#define FIRST_USB_DMA_CHANNEL       2
#define USB_CNTL_RX_CHAN            2
#define USB_CNTL_TX_CHAN            3
#define USB_BULK_RX_CHAN            4
#define USB_BULK_TX_CHAN            5
#elif defined(CONFIG_BCM96358)
#define NR_USB_DMA_CHANNELS         4
#define FIRST_USB_DMA_CHANNEL       4
#define USB_CNTL_RX_CHAN            4
#define USB_CNTL_TX_CHAN            5
#define USB_BULK_RX_CHAN            6
#define USB_BULK_TX_CHAN            7
#endif

typedef struct EnetHdr
{
    UINT8 dest[ETH_ALEN];
    UINT8 src[ETH_ALEN];
    UINT16 type;
} ENET_HDR, *PENET_HDR;

typedef struct UsbTxHdr
{
    struct UsbTxHdr *pNext;      /* ptr to next header in free list */
    volatile DmaDesc *pBdAddr;
    struct sk_buff *pSockBuf;
    RNDIS_MESSAGE PktHdr;
} USB_TX_HDR, *PUSB_TX_HDR;

typedef struct CtrlPktBufs
{
    UINT8 ulRxBuf[MAX_CTRL_PKT_SIZE];
    UINT8 ulTxBuf[MAX_CTRL_PKT_SIZE];
    volatile DmaDesc *pRxBd;
    UINT32 ulReserved[7];
} CTRL_PKT_BUFS, *PCTRL_PKT_BUFS;

/* The definition of the driver control structure */
typedef struct usbnic_dev_context
{
    /* Linux driver fields. */
    struct net_device *pDev;        
    struct net_device_stats DevStats;
#ifdef USE_BH
    struct tasklet_struct BhBulkRx;
    struct tasklet_struct BhRndisMsg;
#endif
    struct timer_list ReclaimTimer;
    IOCTL_MIB_INFO MibInfo;

    /* USB fields. */
    UINT16 usPacketFilterBitmap;
    StringTable *pStrTbl;
    StringDesc StrDesc;
    UINT32 ulHostDriverType;
    UsbConfiguration *pUsbCfg;
    UINT32 ulUsbCfgSize;
    UINT32 ulFlags;
    UINT8 ucPermanentHostMacAddr[ETH_ALEN];
    UINT8 ucCurrentHostMacAddr[ETH_ALEN];

    /* DMA fields. */
    volatile DmaRegs *pDmaRegs;
    volatile DmaChannelCfg *pRxDma;
    volatile DmaChannelCfg *pTxDma;
    volatile DmaChannelCfg *pRxCtrlDma;
    volatile DmaChannelCfg *pTxCtrlDma;
    char RxBds[(USB_NR_RX_BDS * sizeof(DmaDesc)) + 16];
    char TxBds[(USB_NR_TX_BDS * sizeof(DmaDesc)) + sizeof(long)];
    char RxCtrlBds[(NR_RX_CTRL_BDS * sizeof(DmaDesc)) + 16];
    char TxCtrlBd[sizeof(DmaDesc) + 16];
    char CtrlPkts[(NR_RX_CTRL_BDS * sizeof(CTRL_PKT_BUFS)) + 16];
    PCTRL_PKT_BUFS pCtrlPktsBase;
    volatile DmaDesc *pRxCtrlBdBase;
    volatile DmaDesc *pRxCtrlBdCurr;
    volatile DmaDesc *pRxCtrlBdNext;
    volatile DmaDesc *pTxCtrlBdBase;
    volatile DmaDesc *pRxBdBase;
    volatile DmaDesc *pRxBdAssign;
    volatile DmaDesc *pRxBdRead;
    volatile DmaDesc *pTxBdBase;
    volatile DmaDesc *pTxBdNext;
    char TxHdrs[(NR_TX_HDRS * sizeof(USB_TX_HDR)) + sizeof(long)];
    PUSB_TX_HDR pTxHdrBase;
    PUSB_TX_HDR pTxHdrFreeHead;
    PUSB_TX_HDR pTxHdrFreeTail;
    PUSB_TX_HDR pTxHdrReclaimHead;
    PUSB_TX_HDR pTxHdrReclaimTail;
    struct sk_buff *pFreeSockBufList;
    UINT8 SockBufs[(USB_NUM_RX_PKTS * SKB_ALIGNED_SIZE) + 0x10];
    char *pRxBufs;

    /* Remote NDIS fields. */
    UINT32 ulRndisState;
    UINT8 ucRndisMulticastList[MAX_MULTICAST_ADDRESSES * ETH_ALEN];
    UINT8 ucRndisNumMulticastListEntries;
} USBNIC_DEV_CONTEXT, *PUSBNIC_DEV_CONTEXT;

#if defined __cplusplus
}
#endif

#endif

