/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : EndpointDrv.h
 *
 * Description: This file contains the definitions and structures for the
 *              Linux IOCTL interface that used between the user mode Endpoint
 *              API library and the kernel Endpoint API driver.
 *
 * Updates    : 04/04/2002  YD.  Created.
 ***************************************************************************/

#if !defined(_ENDPOINTDRV_H_)
#define _ENDPOINTDRV_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Includes. */
#include <bcmtypes.h>
#include <vrgEndpt.h>
#include <endptvoicestats.h>


#include "../../../../xChange/dslx_common/xchg_drivers/bcm6358/slic/si3215/slic6358Si3215Defs.h"
/* Maximum size for the event data passed with the event callback */
#define MAX_EVENTDATA_SIZE    256

#ifndef PSTN_LIFE_LINE_SUPPORT
#define PSTN_LIFE_LINE_SUPPORT 0
#endif

typedef enum ENDPOINTIOCTL_INDEX
{
   ENDPTIO_INIT_INDEX = 0,
   ENDPTIO_DEINIT_INDEX,
   ENDPTIO_CREATE_INDEX,
   ENDPTIO_CAPABILITIES_INDEX,
   ENDPTIO_SIGNAL_INDEX,
   ENDPTIO_CREATE_CONNECTION_INDEX,
   ENDPTIO_MODIFY_CONNECTION_INDEX,
   ENDPTIO_DELETE_CONNECTION_INDEX,
   ENDPTIO_MUTE_CONNECTION_INDEX,
   ENDPTIO_PACKET_INDEX,
   ENDPTIO_GET_PACKET_INDEX,
   ENDPTIO_GET_EVENT_INDEX,
   ENDPTIO_GET_CODECMAP_INDEX,
   ENDPTIO_VOICESTAT_INDEX,
   ENDPTIO_ISINITIALIZED_INDEX,
   ENDPTIO_CONSOLE_CMD_INDEX,
   ENDPTIO_TEST_INDEX,
   ENDPTIO_ENDPOINTCOUNT_INDEX,
   ENDPTIO_CONTROL_SLIC_REG_INDEX,   
   ENDPTIO_CONFIG_SLIC_PARAM_INDEX,
   ENDPTIO_CONFIG_CAS_CONTROL_INDEX,
   ENDPTIO_HOOKSTAT_INDEX,
   ENDPTIO_SET_RELAY_INDEX,
   ENDPTIO_GET_PSTN_INDEX,
   ENDPTIO_REINJECTION_INDEX,
   /*start of additon by chenyong 2008-10-28 for Inner call*/
   ENDPTIO_CONNECTFORINNERCALL_INDEX,
   /*end of additon by chenyong 2008-10-28 for Inner call*/
   ENDPTIO_CONFIG_DAA_PARAM_INDEX,   
   /* start of l68693 added 20090222: 支持装备ReInection测试 */ 
   ENDPTIO_CONTROL_REINJECTION_INDEX,   
   /* end of l68693 added 20090222: 支持装备ReInection测试 */    
   ENDPTIO_MAX_INDEX
} ENDPOINTIOCTL_INDEX;


/* Defines. */
#define ENDPOINTDRV_MAJOR            209 /* arbitrary unused value */

#define ENDPOINTIOCTL_ENDPT_INIT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_INIT_INDEX, ENDPOINTDRV_INIT_PARAM)

#define ENDPOINTIOCTL_ENDPT_DEINIT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_DEINIT_INDEX, ENDPOINTDRV_INIT_PARAM)

#define ENDPOINTIOCTL_ENDPT_CREATE \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CREATE_INDEX, ENDPOINTDRV_CREATE_PARM)

#define ENDPOINTIOCTL_ENDPT_CAPABILITIES \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CAPABILITIES_INDEX, ENDPOINTDRV_CAP_PARM)

#define ENDPOINTIOCTL_ENDPT_SIGNAL \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_SIGNAL_INDEX, ENDPOINTDRV_SIGNAL_PARM)

#define ENDPOINTIOCTL_ENDPT_CREATE_CONNECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CREATE_CONNECTION_INDEX, ENDPOINTDRV_CONNECTION_PARM)

#define ENDPOINTIOCTL_ENDPT_MODIFY_CONNECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_MODIFY_CONNECTION_INDEX, ENDPOINTDRV_CONNECTION_PARM)

#define ENDPOINTIOCTL_ENDPT_DELETE_CONNECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_DELETE_CONNECTION_INDEX, ENDPOINTDRV_DELCONNECTION_PARM)

#define ENDPOINTIOCTL_ENDPT_MUTE_CONNECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_MUTE_CONNECTION_INDEX, ENDPOINTDRV_MUTECONNECTION_PARM)

#define ENDPOINTIOCTL_ENDPT_PACKET \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_PACKET_INDEX, ENDPOINTDRV_PACKET_PARM)

#define ENDPOINTIOCTL_ENDPT_GET_PACKET \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_GET_PACKET_INDEX, ENDPOINTDRV_PACKET_PARM)

#define ENDPOINTIOCTL_ENDPT_GET_EVENT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_GET_EVENT_INDEX, ENDPOINTDRV_EVENT_PARM)

#define ENDPOINTIOCTL_ENDPT_GET_CODECMAP \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_GET_CODECMAP_INDEX, ENDPOINTDRV_CODECMAP_PARM)

#define ENDPOINTIOCTL_ENDPT_VOICESTAT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_VOICESTAT_INDEX, ENDPOINTDRV_VOICESTAT_PARM)

#define ENDPOINTIOCTL_ISINITIALIZED \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_ISINITIALIZED_INDEX, ENDPOINTDRV_ISINITIALIZED_PARM)

#define ENDPOINTIOCTL_ENDPT_CONSOLE_CMD \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONSOLE_CMD_INDEX, ENDPOINTDRV_CONSOLE_CMD_PARM)

#define ENDPOINTIOCTL_TEST \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_TEST_INDEX, ENDPOINTDRV_TESTPARM)

#define ENDPOINTIOCTL_ENDPOINTCOUNT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_ENDPOINTCOUNT_INDEX, ENDPOINTDRV_ENDPOINTCOUNT_PARM)

#define ENDPOINTIOCTL_CONTROL_SLIC \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONTROL_SLIC_REG_INDEX, ENDPOINTDRV_SLIC_CMD_PARM)

#define ENDPOINTIOCTL_CONFIG_SLIC_PARAM \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONFIG_SLIC_PARAM_INDEX, SI_CMDLINE_CONFIG_S)    

#define ENDPOINTIOCTL_CONFIG_CAS_CONTROL_PARAM \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONFIG_CAS_CONTROL_INDEX, CAS_CONTROL_PARMS)    

#define ENDPOINTIOCTL_HOOKSTAT \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_HOOKSTAT_INDEX, ENDPOINTDRV_HOOKSTAT_CMD_PARM)

#define ENDPOINTIOCTL_SET_RELAY \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_SET_RELAY_INDEX, ENDPOINTDRV_RELAY_CMD_PARM)

/* start of l68693 added 20080521: 支持用户态控制Re-Injection */   
#define ENDPOINTIOCTL_GET_PSTN \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_GET_PSTN_INDEX, ENDPOINTDRV_GET_PSTN_PARM)

#define ENDPOINTIOCTL_REINJECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_REINJECTION_INDEX, ENDPOINTDRV_REINJECTION_PARM)
/* end of l68693 added 20080521 */    

/*start of additon by chenyong 2008-10-28 for Inner call*/
#define ENDPOINTIOCTL_CONNECTFORINNERCALL \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONNECTFORINNERCALL_INDEX, ENDPOINTDRV_CONNECTFORINNERCALL_PARM)
/*end of additon by chenyong 2008-10-28 for Inner call*/

#define ENDPOINTIOCTL_CONFIG_DAA_PARAM  \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONFIG_DAA_PARAM_INDEX, ENDPOINTDRV_CONFIGDAA_PARM)

/* start of l68693 added 20090222: 支持装备ReInection测试 */ 
#define ENDPOINTIOCTL_CONTROL_REINJECTION \
    _IOWR(ENDPOINTDRV_MAJOR, ENDPTIO_CONTROL_REINJECTION_INDEX, ENDPOINTDRV_CONFIGREINJECTION_PARM)
/* end of l68693 added 20090222: 支持装备ReInection测试 */ 


#define MAX_ENDPOINTDRV_IOCTL_COMMANDS   ENDPTIO_MAX_INDEX


typedef struct
{
   UINT32      size;    /* Size of the structure (including the size field) */
   VRG_ENDPT_INIT_CFG *endptInitCfg;
   EPSTATUS    epStatus;
} ENDPOINTDRV_INIT_PARAM, *PENDPOINTDRV_INIT_PARAM;

typedef struct
{
   UINT32   size;       /* Size of the structure (including the size field) */
   UINT32   physId;
   UINT32   lineId;
   VRG_ENDPT_STATE* endptState;
   EPSTATUS epStatus;
} ENDPOINTDRV_CREATE_PARM, *PENDPOINTDRV_CREATE_PARM;

typedef struct
{
   UINT32   size;       /* Size of the structure (including the size field) */
   EPZCAP*  capabilities;
   ENDPT_STATE* state;
   EPSTATUS epStatus;
} ENDPOINTDRV_CAP_PARM, *PENDPOINTDRV_CAP_PARM;

typedef struct
{
   UINT32      size;    /* Size of the structure (including the size field) */
   ENDPT_STATE* state;
   UINT32      cnxId;
   EPSIG       signal;
   UINT32      value;      // Reserve an array, can be a pointer
   EPSTATUS    epStatus;
   int         duration;
   int         period;
   int         repetition;
} ENDPOINTDRV_SIGNAL_PARM, *PENDPOINTDRV_SIGNAL_PARM;

typedef struct
{
   UINT32      size;    /* Size of the structure (including the size field) */
   ENDPT_STATE* state;
   int         cnxId;
   EPZCNXPARAM*   cnxParam;
   EPSTATUS    epStatus;
} ENDPOINTDRV_CONNECTION_PARM, *PENDPOINTDRV_CONNECTION_PARM;

typedef struct
{
   UINT32         size; /* Size of the structure (including the size field) */
   ENDPT_STATE*   state;
   int            cnxId;
   EPSTATUS       epStatus;
} ENDPOINTDRV_DELCONNECTION_PARM, *PENDPOINTDRV_DELCONNECTION_PARM;

typedef struct
{
   UINT32         size; /* Size of the structure (including the size field) */
   ENDPT_STATE*   state;
   int            cnxId;
   VRG_BOOL       mute;
   EPSTATUS       epStatus;
} ENDPOINTDRV_MUTECONNECTION_PARM, *PENDPOINTDRV_MUTECONNECTION_PARM;

typedef struct
{
   UINT32   size;       /* Size of the structure (including the size field) */
   ENDPT_STATE*   state;
   int      cnxId;
   EPPACKET*      epPacket;
   int      length;
   UINT32   bufDesc;
   EPSTATUS epStatus;
} ENDPOINTDRV_PACKET_PARM, *PENDPOINTDRV_PACKET_PARM;

typedef struct
{
   UINT32   size;       /* Size of the structure (including the size field) */
   int      lineId;
   int      cnxId;
   int      length;
   EPEVT    event;
   UINT8    eventData[MAX_EVENTDATA_SIZE];
   UINT16   intData;
} ENDPOINTDRV_EVENT_PARM, *PENDPOINTDRV_EVENT_PARM;

typedef struct
{
   UINT32         size; /* Size of the structure (including the size field) */
   int            isInitialized;
} ENDPOINTDRV_ISINITIALIZED_PARM, *PENDPOINTDRV_ISINITIALIZED_PARM;

typedef struct
{
   UINT32         size; /* Size of the structure (including the size field) */
   UINT32         testParm1;
   UINT32         testParm2;
   EPZCNXPARAM*   testParm3;
   EPSTATUS       epStatus;
} ENDPOINTDRV_TESTPARM, *PENDPOINTDRV_TESTPARM;

typedef struct ENDPOINTDRV_PACKET
{
   UINT32   size;       /* Size of the structure (including the size field) */
   int      cnxId;
   int      length;
   EPMEDIATYPE mediaType;
   UINT8    data[1024];
} ENDPOINTDRV_PACKET;

typedef struct ENDPOINTDRV_VOICESTAT_PARM
{
   int lineId;
   ENDPT_VOICE_STATS* stats;
   EPSTATUS epStatus;
} ENDPOINTDRV_VOICESTAT_PARM, *PENDPOINTDRV_VOICESTAT_PARM;

typedef struct ENDPOINTDRV_CONSOLE_CMD_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   ENDPT_STATE*   state;
   int            cmd;
   int            lineId;
   EPCMD_PARMS*   consoleCmdParams;
   EPSTATUS       epStatus;
} ENDPOINTDRV_CONSOLE_CMD_PARM, *PENDPOINTDRV_CONSOLE_CMD_PARM;

#if defined(CONFIG_SLIC_3210) || defined(CONFIG_SLIC_3215)
typedef struct ENDPOINTDRV_SLIC_CMD_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            cmd;
   int            lineId;
   int            arg[3];
   UINT16         retVal;   
   EPSTATUS       epStatus;
} ENDPOINTDRV_SLIC_CMD_PARM, *PENDPOINTDRV_SLIC_CMD_PARM;
#elif defined(CONFIG_SLIC_LE88221)
typedef struct ENDPOINTDRV_SLIC_CMD_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            cmd;
   int            lineId;
   int            arg[3];
   UINT8          regValue[16];   
   UINT16         retVal;   
   EPSTATUS       epStatus;
} ENDPOINTDRV_SLIC_CMD_PARM, *PENDPOINTDRV_SLIC_CMD_PARM;
#endif

typedef struct
{
   UINT32         size; /* Size of the structure (including the size field) */
   int            endpointNum;
} ENDPOINTDRV_ENDPOINTCOUNT_PARM, *PENDPOINTDRV_ENDPOINTCOUNT_PARM;


typedef struct ENDPOINTDRV_HOOKSTAT_CMD_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   EPCASSTATE     hookstat;
   EPSTATUS       epStatus;
} ENDPOINTDRV_HOOKSTAT_CMD_PARM, *PENDPOINTDRV_HOOKSTAT_CMD_PARM;

typedef struct ENDPOINTDRV_RELAY_CMD_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   BOOL           cmd;
   EPSTATUS       epStatus;
} ENDPOINTDRV_RELAY_CMD_PARM, *PENDPOINTDRV_RELAY_CMD_PARM;

/* start of l68693 added 20080521: 支持用户态控制Re-Injection */   
typedef struct ENDPOINTDRV_GET_PSTN_PARM
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   BOOL           bHasPstn;
   EPSTATUS       epStatus;   
} ENDPOINTDRV_GET_PSTN_PARM, *PENDPOINTDRV_GET_PSTN_PARM;

typedef struct ENDPOINTDRV_REINJECTION
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   BOOL           cmd;
   EPSTATUS       epStatus;   
   
} ENDPOINTDRV_REINJECTION_PARM, *PENDPOINTDRV_REINJECTION_PARM;
/* end of l68693 added 20080521 */   

/*start of additon by chenyong 2008-10-28 for Inner call*/
typedef struct ENDPOINTDRV_CONNECTFORINNERCALL
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   BOOL           cmd;
   EPSTATUS       epStatus;   
   
} ENDPOINTDRV_CONNECTFORINNERCALL_PARM, *PENDPOINTDRV_CONNECTFORINNERCALL_PARM;
/*end of additon by chenyong 2008-10-28 for Inner call*/

/* start of l68693 added 20090222: 支持装备ReInection测试 */ 
typedef struct ENDPOINTDRV_CONFIGDAA
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   int            cmd;
   EPSTATUS       epStatus;   
   
} ENDPOINTDRV_CONFIGDAA_PARM, *PENDPOINTDRV_CONFIGDAA_PARM;

typedef struct ENDPOINTDRV_CONFIGREINJECTION
{
   UINT32         size;       /* Size of the structure (including the size field) */
   int            lineId;
   int            cmd;
   EPSTATUS       epStatus;   
   
} ENDPOINTDRV_CONFIGREINJECTION_PARM, *PENDPOINTDRV_CONFIGREINJECTION_PARM;
/* end of l68693 added 20090222: 支持装备ReInection测试 */ 


#if defined(__cplusplus)
}
#endif

#endif // _ENDPOINTDRV_H_
