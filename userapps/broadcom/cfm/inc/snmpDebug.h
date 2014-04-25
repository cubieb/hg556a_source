#ifndef	_SNMP_DEBUG_
#define	_SNMP_DEBUG_

/* first least significant byte is debug level */
#define SNMP_DEBUG_DISABLE               0
#define SNMP_DEBUG_LEVEL_SNMP            1
#define SNMP_DEBUG_LEVEL_TRANSPORT       2
#define SNMP_DEBUG_LEVEL_ALL             3

/* second least significant byte is transport level, only when debug_level_transport
   is set */
#define SNMP_DEBUG_LEVEL_TRANSPORT_UDP   0x1
#define SNMP_DEBUG_LEVEL_TRANSPORT_AAL5  0x2
#define SNMP_DEBUG_LEVEL_TRANSPORT_EOC   0x4
#define SNMP_DEBUG_LEVEL_TRANSPORT_ALL   0x7
#define SNMP_DEBUG_LEVEL_TRANSPORT_NONE  0x0

/* third least significant byte is MIB debug level; only when debug_level_snmp is set */
#define SNMP_DEBUG_LEVEL_MIB_IF          0x1
#define SNMP_DEBUG_LEVEL_MIB_ATM         0x2
#define SNMP_DEBUG_LEVEL_MIB_PPP         0x4
#define SNMP_DEBUG_LEVEL_MIB_CPE         0x8
#define SNMP_DEBUG_LEVEL_MIB_ALL         0xf
#define SNMP_DEBUG_LEVEL_MIB_NONE        0x0

/* most significant byte is trap send debug */
#define SNMP_DEBUG_SEND_BOOT_TRAP        0x1
#define SNMP_DEBUG_SEND_PING_TRAP        0x2
#define SNMP_DEBUG_SEND_FTP_TRAP         0x4
#define SNMP_DEBUG_SEND_PPP_TRAP         0x8
#define SNMP_DEBUG_SEND_COLD_TRAP        0x10
#define SNMP_DEBUG_SEND_TRAP_NONE        0x0

#define SNMP_CONFIG_ID_LEN               4

typedef struct snmpDebugFlag {
  unsigned int
  level:8,
  transport:8,
  mib:8,
  trap:8;
} SNMP_DEBUG_FLAG;


#endif	/* _SNMP_DEBUG_ */
