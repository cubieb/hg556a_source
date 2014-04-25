/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <paulp@go2net.com>
 *  Some changes Copyright (C) 1996,97 Larry Doolittle <ldoolitt@jlab.org>
 *  Some changes Copyright (C) 1997 Jon Nelson <jnelson@boa.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* $Id: globals.h,v 1.1.2.2 2009/11/26 04:12:04 l43571 Exp $*/

//add by kathy , define type

#define PACKED __attribute__ ((packed))

#define CHAR            signed char
#define INT             signed int
#define SHORT           signed short
#define UINT            unsigned int 
#undef  ULONG           
#define ULONG           unsigned long /* 32-bit in 32-bit CPU or
                                         64-bit in 64-bit CPU */
#define USHORT          unsigned short
#define UCHAR           unsigned char

#define TRUE            1
#define FALSE           0

#define s64             signed long long
#define u64             unsigned long long
//add by kathy, rt_linux.h
#define BOOLEAN         unsigned char
//#define LARGE_INTEGER s64
#define VOID            void
#define LONG            long
#define LONGLONG        s64
#define ULONGLONG       u64

typedef VOID            *PVOID;
typedef CHAR            *PCHAR;
typedef UCHAR           *PUCHAR;
typedef USHORT          *PUSHORT;
typedef LONG            *PLONG;
typedef ULONG           *PULONG;

typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    } ;
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
    s64 QuadPart;
} LARGE_INTEGER;


#ifndef _GLOBALS_H
#define _GLOBALS_H

/********************** METHODS **********************/
enum HTTP_METHOD { M_GET = 1, M_HEAD, M_PUT, M_POST,
    M_DELETE, M_LINK, M_UNLINK, M_MOVE, M_TRACE
};

/******************* HTTP VERSIONS *******************/
enum HTTP_VERSION { HTTP09=1, HTTP10, HTTP11 };

/************** REQUEST STATUS (req->status) ***************/
enum REQ_STATUS { READ_HEADER, ONE_CR, ONE_LF, TWO_CR,
    BODY_READ, BODY_WRITE,
    WRITE,
    PIPE_READ, PIPE_WRITE,
    IOSHUFFLE,
    DONE,
    TIMED_OUT,
    DEAD
};

/******************* RESPONSE CODES ******************/
enum RESPONSE_CODE { R_CONTINUE = 100,
                     R_REQUEST_OK = 200,
                     R_CREATED,
                     R_ACCEPTED,
                     R_PROVISIONAL,
                     R_NO_CONTENT,
                     R_205,
                     R_PARTIAL_CONTENT,
                     R_MULTIPLE = 300,
                     R_MOVED_PERM,
                     R_MOVED_TEMP,
                     R_303,
                     R_NOT_MODIFIED,
                     R_BAD_REQUEST = 400,
                     R_UNAUTHORIZED,
                     R_PAYMENT,
                     R_FORBIDDEN,
                     R_NOT_FOUND,
                     R_METHOD_NA, /* method not allowed */
                     R_NON_ACC,   /* non acceptable */
                     R_PROXY,     /* proxy auth required */
                     R_REQUEST_TO, /* request timeout */
                     R_CONFLICT,
                     R_GONE,
                     R_LENGTH_REQUIRED,
                     R_PRECONDITION_FAILED,
                     R_REQUEST_URI_TOO_LONG = 414,
                     R_INVALID_RANGE = 416,
                     R_ERROR = 500,
                     R_NOT_IMP,
                     R_BAD_GATEWAY,
                     R_SERVICE_UNAV,
                     R_GATEWAY_TO, /* gateway timeout */
                     R_BAD_VERSION };

/************* ALIAS TYPES (aliasp->type) ***************/
enum ALIAS { ALIAS, SCRIPTALIAS, REDIRECT };

/*********** KEEPALIVE CONSTANTS (req->keepalive) *******/
enum KA_STATUS { KA_INACTIVE, KA_ACTIVE, KA_STOPPED };

/********* CGI STATUS CONSTANTS (req->cgi_status) *******/
enum CGI_STATUS { CGI_PARSE, CGI_BUFFER, CGI_DONE };

/************** CGI TYPE (req->is_cgi) ******************/
enum CGI_TYPE { NPH = 1, CGI };

/**************** STRUCTURES ****************************/
struct range {
    unsigned long start;
    unsigned long stop;
    struct range *next;
};

typedef struct range Range;

struct mmap_entry {
    dev_t dev;
    ino_t ino;
    char *mmap;
    int use_count;
    off_t len;
};
#if 0
struct request {                /* pending requests */
    enum REQ_STATUS status;
    enum KA_STATUS keepalive;   /* keepalive status */
    enum HTTP_VERSION http_version;
    enum HTTP_METHOD method;    /* M_GET, M_POST, etc. */
    enum RESPONSE_CODE response_status; /* R_NOT_FOUND, etc.. */

    enum CGI_TYPE cgi_type;
    enum CGI_STATUS cgi_status;

    /* should pollfd_id be zeroable or no ? */
#ifdef HAVE_POLL
    int pollfd_id;
#endif

    char *pathname;             /* pathname of requested file */

    Range *ranges;              /* our Ranges */
    int numranges;

    int data_fd;                /* fd of data */
    unsigned long filesize;     /* filesize */
    unsigned long filepos;      /* position in file */
    unsigned long bytes_written; /* total bytes written (sans header) */
    char *data_mem;             /* mmapped/malloced char array */

#ifdef USE_AUTH
    char *authorization;
#endif

    char *logline;              /* line to log file */

    char *header_line;          /* beginning of un or incompletely processed header line */
    char *header_end;           /* last known end of header, or end of processed data */
    int parse_pos;              /* how much have we parsed */

    int buffer_start;           /* where the buffer starts */
    int buffer_end;             /* where the buffer ends */

    char *if_modified_since;    /* If-Modified-Since */
    time_t last_modified;       /* Last-modified: */

    /* CGI vars */
    int cgi_env_index;          /* index into array */

    /* Agent and referer for logfiles */
    char *header_host;
    char *header_user_agent;
    char *header_referer;
    char *header_ifrange;
    char *host;                 /* what we end up using for 'host', no matter the contents of header_host */

    int post_data_fd;           /* fd for post data tmpfile */

    char *path_info;            /* env variable */
    char *path_translated;      /* env variable */
    char *script_name;          /* env variable */
    char *query_string;         /* env variable */
    char *content_type;         /* env variable */
    char *content_length;       /* env variable */

    struct mmap_entry *mmap_entry_var;

    /* everything **above** this line is zeroed in sanitize_request */
    /* this may include 'fd' */
    /* in sanitize_request with the 'new' parameter set to 1,
     * kacount is set to ka_max and client_stream_pos is also zeroed.
     * Also, time_last is set to 'NOW'
     */
    int fd;                     /* client's socket fd */
    time_t time_last;           /* time of last succ. op. */
    char local_ip_addr[BOA_NI_MAXHOST]; /* for virtualhost */
    char remote_ip_addr[BOA_NI_MAXHOST]; /* after inet_ntoa */
    unsigned int remote_port;            /* could be used for ident */

    unsigned int kacount;                /* keepalive count */
    int client_stream_pos;      /* how much have we read... */

    char user[16];            /* user's login name */

    /* everything below this line is kept regardless */
    char buffer[BUFFER_SIZE + 1]; /* generic I/O buffer */
    char request_uri[MAX_HEADER_LENGTH + 1]; /* uri */
    char client_stream[CLIENT_STREAM_SIZE]; /* data from client - fit or be hosed */
    char *cgi_env[CGI_ENV_MAX + 4]; /* CGI environment */

#ifdef ACCEPT_ON
    char accept[MAX_ACCEPT_LENGTH]; /* Accept: fields */
#endif

    struct request *next;       /* next */
    struct request *prev;       /* previous */
};
#endif
typedef struct request request;

struct status {
    long requests;
    long errors;
};

/* 2004.12.21 add by arvin tai */
#define MAX_SSID_NUMBER        4
typedef void cgi_handler(char *data, FILE *out);
typedef void ssi_handler( FILE *out, char *ssi_arg);
typedef int multipart_handler(FILE *stream, int len, int len_boundary);

typedef struct 
{
    const char *name;      /* The name of the script as given in the URL */
    cgi_handler *handler;  /* The function which should be called */
    multipart_handler *multi_handler;  /* The function which should be called */
} CGI_ENTRY;

/* add by Dennis Lee to add the SSI Control*/

typedef struct 
{
    const char *name;      /* The name of the script as given in the URL */
    ssi_handler *handler;  /* The function which should be called */
} SSI_ENTRY;

struct name_val {
    char * name;
    char * value;
};

struct httpform {
    int nv_ct;    /* number of entry in name value pairs */
    struct name_val nameval[1];
};

struct www_name {
    char name[100];
};

struct www_file {
    int f_cnt;
    struct www_name f_name[1];
};

typedef struct 
{
    const char *suffix;  /* Suffix of filename, excluding '.', e.g. "txt" */ 
    const char *type;    /* Content-type, e.g. "text/plain" */
} FILETYPE;
typedef struct site_survey_tbl {
    unsigned char      macAddr[20];
    char               ssid[33];
    char                auth[8];
    char               channel[4];
    char               wirelessmode[4];
    unsigned char      rssi;
}site_survey_tbl;

typedef struct multi_ssid_s {
    int  enable;
    char SSID[33];
    int  SecurityMode;
    char AuthMode[14];
    char EncrypType[8];
    char WPAPSK[65];
    int  DefaultKeyID;
    int  Key1Type;
    char Key1Str[27];
    int  Key2Type;
    char Key2Str[27];
    int  Key3Type;
    char Key3Str[27];
    int  Key4Type;
    char Key4Str[27];
    int  IEEE8021X;
    int  TxRate;
    int  HideSSID;
    int  PreAuth;
    char VlanName[20];
    int  VlanId;
    int  VlanPriority;
}multi_ssid_s;

typedef struct web_chantbl
{
    char channel[30];
} web_chantbl;

extern int  reboot_flag;
extern int  redirectSetupPage;
extern int  ATE_ACTION_MODE;
extern int  ATE_ACTION_START;
extern char ateda[];
extern char atesa[];
extern char atebssid[];
extern char atetxpow;
extern int  atechannel;
extern char atetxfreqoffset;
extern int  atetxlen;
extern long atetxcnt;
extern int  atetxrate;
extern int  eeprom_read_write;
extern int  eeprom_offset;
extern char eeprom_value[];
extern int  mac_read_write;
extern int  mac_offset;
extern char mac_value[];
extern int  bbp_read_write;
extern int  bbp_offset;
extern char bbp_value[];
/* end */

extern struct status status;

extern char *optarg;            /* For getopt */

extern request *request_ready;  /* first in ready list */
extern request *request_block;  /* first in blocked list */
extern request *request_free;   /* first in free list */

#ifdef HAVE_POLL
extern struct pollfd *pfds;
extern unsigned int pfd_len;
#else
extern fd_set block_read_fdset; /* fds blocked on read */
extern fd_set block_write_fdset; /* fds blocked on write */
extern int max_fd;
#endif

/* global server variables */

extern char *access_log_name;
extern char *error_log_name;
extern char *cgi_log_name;
extern int cgi_log_fd;
extern int use_localtime;

extern unsigned int user_server_port; // assign vale from user mode
extern unsigned int server_port;
extern uid_t server_uid;
extern gid_t server_gid;
extern char *server_admin;
extern char *server_root;
extern char *server_name;
extern char *server_ip;

extern char *user_document_root;
extern char *document_root;
extern char *user_dir;
extern char *directory_index;
extern char *default_type;
extern char *default_charset;
extern char *dirmaker;
extern char *mime_types;
extern char *pid_file;
extern char *cachedir;

extern const char *tempdir;

extern char *cgi_path;
extern short common_cgi_env_count;
extern int single_post_limit;
extern int conceal_server_identity;

extern int unsigned ka_timeout;
extern int unsigned default_timeout;
extern int unsigned ka_max;

extern int sighup_flag;
extern int sigchld_flag;
extern int sigalrm_flag;
extern int sigterm_flag;
extern time_t start_time;

extern int pending_requests;
extern unsigned max_connections;

extern int verbose_cgi_logs;

extern int backlog;
extern time_t current_time;

extern int virtualhost;
extern char *vhost_root;
extern const char *default_vhost;

extern unsigned total_connections;
extern unsigned int system_bufsize;      /* Default size of SNDBUF given by system */

//extern sigjmp_buf env;
extern int handle_sigbus;
extern unsigned int cgi_umask;


//kathy, 2006-0810
char *http_boundary;
int  firmware_fd;
int  total_count;

//copy from RT61 Utility

typedef struct _PAIR_CHANNEL_FREQ_ENTRY
{
    unsigned long    lChannel;
    unsigned long    lFreq;
} PAIR_CHANNEL_FREQ_ENTRY, *PPAIR_CHANNEL_FREQ_ENTRY;
//
// IEEE 802.11 Structures and definitions
//
// new types for Media Specific Indications

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16

#define ETH_LENGTH_OF_ADDRESS         6
#define MAX_NUM_OF_EVENT              10  // entry # in EVENT table
#define MAX_LEN_OF_MAC_TABLE          32
#define MAC_ADDR_LEN                6
typedef ULONGLONG NDIS_802_11_KEY_RSC;

typedef UCHAR  NDIS_802_11_MAC_ADDRESS[MAC_ADDR_LEN];

// Key mapping keys require a BSSID
typedef struct _NDIS_802_11_KEY
{
    UINT           Length;             // Length of this structure
    UINT           KeyIndex;           
    UINT           KeyLength;          // length of key in bytes
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_KEY_RSC KeyRSC;
    UCHAR           KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_KEY, *PNDIS_802_11_KEY;

typedef struct _NDIS_802_11_REMOVE_KEY
{
    UINT           Length;             // Length of this structure
    UINT           KeyIndex;           
    NDIS_802_11_MAC_ADDRESS BSSID;      
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

typedef struct _NDIS_802_11_WEP
{
   UINT     Length;        // Length of this structure
   UINT     KeyIndex;           // 0 is the per-client key, 1-N are the
                                        // global keys
   UINT     KeyLength;     // length of key in bytes
   UCHAR    KeyMaterial[1];// variable length depending on above field
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;

typedef enum _NDIS_802_11_POWER_MODE
{
    Ndis802_11PowerModeCAM,
    Ndis802_11PowerModeMAX_PSP,
    Ndis802_11PowerModeFast_PSP,
    Ndis802_11PowerModeMax      // not a real mode, defined as an upper bound
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef enum _RT_802_11_PREAMBLE {
    Rt802_11PreambleLong,
    Rt802_11PreambleShort,
    Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;


typedef enum _NDIS_802_11_WEP_STATUS
{
    Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS,
  NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
    Ndis802_11IBSS,
    Ndis802_11Infrastructure,
    Ndis802_11AutoUnknown,
    Ndis802_11InfrastructureMax         // Not a real value, defined as upper bound
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

// Add new authentication modes
typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
    Ndis802_11AuthModeOpen,
    Ndis802_11AuthModeShared,
    Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeWPA,
    Ndis802_11AuthModeWPAPSK,
    Ndis802_11AuthModeWPANone,
    Ndis802_11AuthModeWPA2,
    Ndis802_11AuthModeWPA2PSK,
    Ndis802_11AuthModeMax               // Not a real mode, defined as upper bound
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

#ifdef WPA_SUPPLICANT_SUPPORT

// wpa_supplicant.conf
#define IDENTITY_LENGTH    32
#define CERT_PATH_LENGTH    64
#define PRIVATE_KEY_PATH_LENGTH    64

typedef enum _RT_WPA_SUPPLICANT_KEY_MGMT {
    Rtwpa_supplicantKeyMgmtWPAPSK,
    Rtwpa_supplicantKeyMgmtWPAEAP,
    Rtwpa_supplicantKeyMgmtIEEE8021X,
    Rtwpa_supplicantKeyMgmtNONE
} RT_WPA_SUPPLICANT_KEY_MGMT, *PRT_WPA_SUPPLICANT_KEY_MGMT;

typedef enum _RT_WPA_SUPPLICANT_EAP {
    Rtwpa_supplicantEAPMD5,
    Rtwpa_supplicantEAPMSCHAPV2,
    Rtwpa_supplicantEAPOTP,
    Rtwpa_supplicantEAPGTC,
    Rtwpa_supplicantEAPTLS,
    Rtwpa_supplicantEAPPEAP,
    Rtwpa_supplicantEAPTTLS,
    Rtwpa_supplicantEAPNONE
} RT_WPA_SUPPLICANT_EAP, *PRT_WPA_SUPPLICANT_EAP;

typedef enum _RT_WPA_SUPPLICANT_EAPOL_FLAGS {
    Rtwpa_supplicantEAPOLFLAGSSTATICWEPKEY,
    Rtwpa_supplicantEAPOLFLAGSDYNAMICALUNICASTWEPKEY,
    Rtwpa_supplicantEAPOLFLAGSDYNAMICALBROADCASTWEPKEY,
    Rtwpa_supplicantEAPOLFLAGSDYNAMICALWEPKEY,
    Rtwpa_supplicantEAPOLFLAGSDEFAULT = Rtwpa_supplicantEAPOLFLAGSDYNAMICALWEPKEY
} _RT_WPA_SUPPLICANT_EAPOL_FLAGS, *P_RT_WPA_SUPPLICANT_EAPOL_FLAGS;

typedef enum _RT_WPA_SUPPLICANT_GROUP {
    Rtwpa_supplicantGROUPCCMP,
    Rtwpa_supplicantGROUPTKIP,
    Rtwpa_supplicantGROUPWEP104,
    Rtwpa_supplicantGROUPWEP40,
    Rtwpa_supplicantGROUPDEFAULT  // CCMP, TKIP, WEP104, WEP40
} RT_WPA_SUPPLICANT_GROUP, *PRT_WPA_SUPPLICANT_GROUP;

typedef enum _RT_WPA_SUPPLICANT_PAIRWISE {
    Rtwpa_supplicantPAIRWISECCMP,
    Rtwpa_supplicantPAIRWISETKIP,
    Rtwpa_supplicantPAIRWISENONE,
    Rtwpa_supplicantPAIRWISEDEFAULT
} RT_WPA_SUPPLICANT_PAIRWISE, *PRT_WPA_SUPPLICANT_PAIRWISE;

typedef enum _RT_WPA_SUPPLICANT_PROTO {
    Rtwpa_supplicantPROTOWPA,    //WPA
    Rtwpa_supplicantPROTORSN,  //WPA2
    Rtwpa_supplicantPROTODEFAULT    //WPA RSN
} RT_WPA_SUPPLICANT_PROTO, *PRT_WPA_SUPPLICANT_PROTO;

typedef enum _RT_WPA_SUPPLICANT_TUNNEL {
    Rtwpa_supplicantTUNNELMSCHAP,
    Rtwpa_supplicantTUNNELMSCHAPV2,
    Rtwpa_supplicantTUNNELPAP,
    Rtwpa_supplicantTUNNENONE
} RT_WPA_SUPPLICANT_TUNNEL, *PRT_WPA_SUPPLICANT_TUNNEL;
#endif //wpa_supplicant_support

typedef struct _RT_PROFILE_SETTING {
    UINT                             ProfileDataType; //0x18140201
    UCHAR                            Profile[32+1];
    UCHAR                            SSID[NDIS_802_11_LENGTH_SSID+1];
    UINT                            SsidLen;
    UINT                            Channel;
    NDIS_802_11_AUTHENTICATION_MODE    Authentication; //Ndis802_11AuthModeOpen, Ndis802_11AuthModeShared
                                                                //Ndis802_11AuthModeWPAPSK
    NDIS_802_11_WEP_STATUS            Encryption; //Ndis802_11WEPEnabled, Ndis802_11WEPDisabled
                                                            //Ndis802_11Encryption2Enabled, Ndis802_11Encryption3Enabled
    NDIS_802_11_NETWORK_INFRASTRUCTURE    NetworkType;
    UINT                            KeyDefaultId;
    UINT                            Key1Type;
    UINT                            Key2Type;
    UINT                            Key3Type;
    UINT                            Key4Type;
    UINT                            Key1Length;
    UINT                            Key2Length;
    UINT                            Key3Length;
    UINT                            Key4Length;
    CHAR                            Key1[26+1];
    CHAR                            Key2[26+1];
    CHAR                            Key3[26+1];
    CHAR                            Key4[26+1];
    CHAR                            WpaPsk[64+1];//[32+1];
    UINT                            TransRate;
    UINT                            TransPower;
    BOOLEAN                            RTSCheck;  //boolean
    UINT                             RTS;
    BOOLEAN                                FragmentCheck; //boolean
    UINT                            Fragment;
    NDIS_802_11_POWER_MODE            PSmode;
    RT_802_11_PREAMBLE                PreamType;
    UINT                            AntennaRx;
    UINT                            AntennaTx;
    UINT                            CountryRegion;
    //Advance                    
    //RT_802_11_TX_RATES                ConfigSta;
    UINT                            AdhocMode;
    //UCHAR                            reserved[64];
    UINT                            Active; // 0 is the profile is set as connection profile, 1 is not. add by kathy, 2006-1023
#ifdef WPA_SUPPLICANT_SUPPORT  

    RT_WPA_SUPPLICANT_KEY_MGMT        KeyMgmt;
    RT_WPA_SUPPLICANT_EAP            EAP;
    UCHAR                            Identity[IDENTITY_LENGTH];
    UCHAR                            Password[32];
    UCHAR                            CACert[CERT_PATH_LENGTH];
    UCHAR                            ClientCert[CERT_PATH_LENGTH];
    UCHAR                            PrivateKey[PRIVATE_KEY_PATH_LENGTH];
    UCHAR                            PrivateKeyPassword[32];
    //UINT                            EapolFlag;
    //RT_WPA_SUPPLICANT_PROTO            Proto;
    //RT_WPA_SUPPLICANT_PAIRWISE        Pairwise;
    //RT_WPA_SUPPLICANT_GROUP            group;
    //UCHAR                            Phase1[32];
    RT_WPA_SUPPLICANT_TUNNEL        Tunnel;
#endif
    struct  _RT_PROFILE_SETTING        *Next;
} RT_PROFILE_SETTING, *PRT_PROFILE_SETTING;

typedef UCHAR   NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];        // Set of 8 data rates
typedef UCHAR   NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];  // Set of 16 data rates

typedef struct PACKED _NDIS_802_11_SSID
{
    UINT    SsidLength;         // length of SSID field below, in bytes;
                                    // this can be zero.
    UCHAR    Ssid[NDIS_802_11_LENGTH_SSID];           // SSID information field
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

PRT_PROFILE_SETTING    selectedProfileSetting, headerProfileSetting, currentProfileSetting;


/*typedef enum _NDIS_MEDIA_STATE
{
    NdisMediaStateConnected,
    NdisMediaStateDisconnected
} NDIS_MEDIA_STATE, *PNDIS_MEDIA_STATE;
*/
//RT2860 defined , kathy
#define NdisMediaStateConnected        1
#define NdisMediaStateDisconnected    0
typedef unsigned int    NDIS_MEDIA_STATE;


// Added new types for OFDM 5G and 2.4G
typedef enum _NDIS_802_11_NETWORK_TYPE
{
    Ndis802_11FH,
    Ndis802_11DS,
    Ndis802_11OFDM5,
    Ndis802_11OFDM24,
    Ndis802_11Automode,
    Ndis802_11OFDM5_N,
    Ndis802_11OFDM24_N,
    Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST
{
    UINT                NumberOfItems;  // in list below, at least 1
    NDIS_802_11_NETWORK_TYPE    NetworkType [1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

//
// Received Signal Strength Indication
//
typedef LONG   NDIS_802_11_RSSI;           // in dBm

typedef struct _NDIS_802_11_CONFIGURATION_FH
{
    ULONG            Length;             // Length of structure
    ULONG            HopPattern;         // As defined by 802.11, MSB set
    ULONG            HopSet;             // to one if non-802.11
    ULONG            DwellTime;          // units are Kusec
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION
{
    ULONG            Length;             // Length of structure
    ULONG            BeaconPeriod;       // units are Kusec
    ULONG            ATIMWindow;         // units are Kusec
    ULONG            DSConfig;           // Frequency, units are kHz
    NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef struct PACKED _NDIS_WLAN_BSSID
{
    ULONG                                Length;             // Length of this structure
    NDIS_802_11_MAC_ADDRESS                MacAddress;         // BSSID
    UCHAR                                Reserved[2];
    NDIS_802_11_SSID                    Ssid;               // SSID
    ULONG                                Privacy;            // WEP encryption requirement
    NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION            Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE    InfrastructureMode;
    NDIS_802_11_RATES                    SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

// Added Capabilities, IELength and IEs for each BSSID
typedef struct PACKED _NDIS_WLAN_BSSID_EX
{
    ULONG                                Length;             // Length of this structure
    NDIS_802_11_MAC_ADDRESS                MacAddress;         // BSSID
    UCHAR                                Reserved[2];
    NDIS_802_11_SSID                    Ssid;               // SSID
    UINT                                Privacy;            // WEP encryption requirement
    NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION            Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE    InfrastructureMode;
    NDIS_802_11_RATES_EX                SupportedRates;
    ULONG                                IELength;
    UCHAR                                IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

typedef struct PACKED _NDIS_802_11_BSSID_LIST_EX
{
    UINT                    NumberOfItems;      // in list below, at least 1
    NDIS_WLAN_BSSID_EX        Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

typedef struct PACKED _NDIS_802_11_FIXED_IEs 
{
    UCHAR     Timestamp[8];
    USHORT     BeaconInterval;
    USHORT    Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;

typedef struct _NDIS_802_11_VARIABLE_IEs 
{
    UCHAR    ElementID;
    UCHAR    Length;    // Number of bytes in data field
    UCHAR    data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

typedef struct _RT_802_11_STA_CONFIG {
    ULONG   EnableTxBurst;      // 0-disable, 1-enable
    ULONG   EnableTurboRate;    // 0-disable, 1-enable 72/100mbps turbo rate
    ULONG   UseBGProtection;    // 0-AUTO, 1-always ON, 2-always OFF
    ULONG   UseShortSlotTime;   // 0-no use, 1-use 9-us short slot time when applicable
    ULONG   AdhocMode;            // 0-11b rates only (WIFI spec), 1 - b/g mixed, 2 - g only
    ULONG   HwRadioStatus;      // 0-OFF, 1-ON, default is 1, Read-Only
    ULONG   Rsv1;               // must be 0
    ULONG   SystemErrorBitmap;  // ignore upon SET, return system error upon QUERY
} RT_802_11_STA_CONFIG, *PRT_802_11_STA_CONFIG;

#ifdef RT61
typedef enum _RT_802_11_PHY_MODE {
    PHY_11BG_MIXED,
    PHY_11B,
    PHY_11A,
    PHY_11ABG_MIXED,
    PHY_11G
} RT_802_11_PHY_MODE;
#else
typedef enum _RT_802_11_PHY_MODE {
    PHY_11BG_MIXED = 0,
    PHY_11B,
    PHY_11A,
    PHY_11ABG_MIXED,
    PHY_11G,
    PHY_11ABGN_MIXED,    // both band   5
    PHY_11N,            //    6
    PHY_11GN_MIXED,        // 2.4G band      7
    PHY_11AN_MIXED,        // 5G  band       8
    PHY_11BGN_MIXED,    // if check 802.11b.      9
    PHY_11AGN_MIXED,    // if check 802.11b.      10
} RT_802_11_PHY_MODE;
#endif

// HtMode
#define HTMODE_MM 0
#define HTMODE_GF 1 
// ExtOffset
#define EXTCHA_NONE 0  
#define EXTCHA_ABOVE 0x1
#define EXTCHA_BELOW 0x3
// BW
#define BW_20       0
#define BW_40       1
// SHORTGI
#define GI_400      1 // only support in HT mode
#define GI_800      0 
// STBC
#define STBC_NONE   0
#define STBC_USE    1 // limited use in rt2860b phy
typedef struct {
    RT_802_11_PHY_MODE    PhyMode;
    UCHAR        TransmitNo;
    UCHAR        HtMode;     //HTMODE_GF or HTMODE_MM
    UCHAR        ExtOffset;    //extension channel above or below
    UCHAR        MCS;
    UCHAR       BW;
    UCHAR        STBC;
    UCHAR        SHORTGI;
    UCHAR        rsv;
} OID_SET_HT_PHYMODE, *POID_SET_HT_PHYMODE;
//RT61
#ifdef RT61
typedef struct _NDIS_802_11_STATISTICS
{
    ULONG     Length;             // Length of structure
    LARGE_INTEGER   TransmittedFragmentCount;
    LARGE_INTEGER   MulticastTransmittedFrameCount;
    LARGE_INTEGER   FailedCount;
    LARGE_INTEGER   RetryCount;
    LARGE_INTEGER   MultipleRetryCount;
    LARGE_INTEGER   RTSSuccessCount;
    LARGE_INTEGER   RTSFailureCount;
    LARGE_INTEGER   ACKFailureCount;
    LARGE_INTEGER   FrameDuplicateCount;
    LARGE_INTEGER   ReceivedFragmentCount;
    LARGE_INTEGER   MulticastReceivedFrameCount;
    LARGE_INTEGER   FCSErrorCount;
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;
#else

typedef struct _NDIS_802_11_STATISTICS
{
    ULONG           Length;             // Length of structure
    LARGE_INTEGER   TransmittedFragmentCount;
    LARGE_INTEGER   MulticastTransmittedFrameCount;
    LARGE_INTEGER   FailedCount;
    LARGE_INTEGER   RetryCount;
    LARGE_INTEGER   MultipleRetryCount;
    LARGE_INTEGER   RTSSuccessCount;
    LARGE_INTEGER   RTSFailureCount;
    LARGE_INTEGER   ACKFailureCount;
    LARGE_INTEGER   FrameDuplicateCount;
    LARGE_INTEGER   ReceivedFragmentCount;
    LARGE_INTEGER   MulticastReceivedFrameCount;
    LARGE_INTEGER   FCSErrorCount;
    LARGE_INTEGER   TKIPLocalMICFailures;
    LARGE_INTEGER   TKIPRemoteMICErrors;
    LARGE_INTEGER   TKIPICVErrors;
    LARGE_INTEGER   TKIPCounterMeasuresInvoked;
    LARGE_INTEGER   TKIPReplays;
    LARGE_INTEGER   CCMPFormatErrors;
    LARGE_INTEGER   CCMPReplays;
    LARGE_INTEGER   CCMPDecryptErrors;
    LARGE_INTEGER   FourWayHandshakeFailures;   
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;
#endif

#define    GENERAL_LINK_UP            0x0            // Link is Up
#define    GENERAL_LINK_DOWN        0x1            // Link is Down
#define    HW_RADIO_OFF            0x2            // Hardware radio off
#define    SW_RADIO_OFF            0x3            // Software radio off
#define    AUTH_FAIL                0x4            // Open authentication fail
#define    AUTH_FAIL_KEYS            0x5            // Shared authentication fail
#define    ASSOC_FAIL                0x6            // Association failed
#define    EAP_MIC_FAILURE            0x7            // Deauthencation because MIC failure
#define    EAP_4WAY_TIMEOUT        0x8            // Deauthencation on 4-way handshake timeout
#define    EAP_GROUP_KEY_TIMEOUT    0x9            // Deauthencation on group key handshake timeout
#define    EAP_SUCCESS                0xa            // EAP succeed
#define    EXTRA_INFO_MAX            0xa            // EXTRA INFO MAX

#define IDS_GENERAL_LINK_UP        "Link is Up"
#define IDS_GENERAL_LINK_DOWN        "Link is Down"
#define IDS_HW_RADIO_OFF        "Hardware radio off"
#define IDS_SW_RADIO_OFF        "Software radio off"
#define IDS_AUTH_FAIL            "Open authentication fail"
#define IDS_AUTH_FAIL_KEYS        "Shared authentication fail"
#define IDS_ASSOC_FAIL            "Association failed"
#define IDS_EAP_MIC_FAILURE        "Deauthencation because MIC failure"
#define IDS_EAP_4WAY_TIMEOUT        "Deauthencation on 4-way handshake timeout"
#define IDS_EAP_GROUP_KEY_TIMEOUT    "Deauthencation on group key handshake timeout"
#define IDS_EAP_SUCCESS            "EAP successd"

#define IDS_RADIOOFF            "RF OFF"
#define IDS_DISCONNECTED        "Disconnected"

#define IDS_LINK_GOOD            "Good"
#define IDS_LINK_NORMAL            "Normal"
#define IDS_LINK_WEAK            "Weak"



#define HuaWeiWlancmdVer  "wlancmd_atpv100R002C05\n"
#define HuaWeiWLANDriverVer "driver_atpv100r002c05\n"
#define FAIL -1
#define SUCCE 0

typedef struct _EXTRA_INFO_DATA
{
    ULONG        nExtraInfoDef;
    char*        nExtraInfoString;
} EXTRA_INFO_DATA, *PEXTRA_INFO_DATA;


// put all proprietery for-query objects here to reduce # of Query_OID
typedef struct _RT_802_11_LINK_STATUS {
    ULONG   CurrTxRate;         // in units of 0.5Mbps
    ULONG   ChannelQuality;     // 0..100 %
    ULONG   TxByteCount;        // both ok and fail
    ULONG   RxByteCount;        // both ok and fail
    ULONG    CentralChannel;        // 40MHz central channel number
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

/*****************************************************/
typedef union _MACHTTRANSMIT_SETTING {
struct {
unsigned short MCS:7; // MCS
unsigned short BW:1; //channel bandwidth 20MHz or 40 MHz
unsigned short ShortGI:1;
unsigned short STBC:2; //SPACE
unsigned short rsv:3;
unsigned short MODE:2; // Use definition MODE_xxx.
} field;
unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
    unsigned char Addr[6];
    char    apidx;
    unsigned char Aid;
    unsigned char Psm; // 0:PWR_ACTIVE, 1:PWR_SAVE
    unsigned char MimoPs; // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
    char          AvgRssi0;
    char          AvgRssi1;
    char          AvgRssi2;
    unsigned long ConnectedTime;
    MACHTTRANSMIT_SETTING TxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
    unsigned long Num;
    char         PhyMode; 
    RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;
/***********************************************************/

typedef struct _RT_VERSION_INFO{
    UCHAR       DriverVersionW;
    UCHAR       DriverVersionX;
    UCHAR       DriverVersionY;
    UCHAR       DriverVersionZ;
    UINT        DriverBuildYear;
    UINT        DriverBuildMonth;
    UINT        DriverBuildDay;
} RT_VERSION_INFO, *PRT_VERSION_INFO;

typedef ULONG   NDIS_802_11_FRAGMENTATION_THRESHOLD;

typedef ULONG   NDIS_802_11_RTS_THRESHOLD;

typedef ULONG   NDIS_802_11_TX_POWER_LEVEL; // in milliwatts

#define MAX_TX_POWER_LEVEL                100   /* mW */
#define MAX_RTS_THRESHOLD                 2347  /* byte count */


//RT2860 HT
typedef union  _HTTRANSMIT_SETTING {
 struct {
     USHORT  MCS:7;                 // MCS
     USHORT  BW:1; //channel bandwidth 20MHz or 40 MHz
     USHORT  ShortGI:1;
     USHORT  STBC:2; //SPACE
     USHORT  rsv:3;  
     USHORT  MODE:2; // 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
     } field;
     USHORT  word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

//Block ACK , rt2860, kathy, 2007-0126
#define MAX_TX_REORDERBUF        64
#define MAX_RX_REORDERBUF        68
#define DEFAULT_TX_TIMEOUT        30
#define DEFAULT_RX_TIMEOUT        30
#define MAX_BARECI_SESSION        8

//
//    For    OID    Query or Set about BA structure
//
typedef    struct    _OID_BACAP_STRUC    {
        UCHAR        RxBAWinLimit;
        UCHAR        TxBAWinLimit;
        UCHAR        Policy;        // 0: DELAY_BA 1:IMMED_BA  (//BA Policy    subfiled value in ADDBA    frame)     2:BA-not use. other value invalid
        UCHAR        MpduDensity;// 0: DELAY_BA 1:IMMED_BA  (//BA Policy    subfiled value in ADDBA    frame)     2:BA-not use. other value invalid
        UCHAR        AmsduEnable;//Enable AMSDU transmisstion
        UCHAR        AmsduSize;    // 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]    = {    3839, 7935};
        UCHAR        MMPSmode;    // MIMO    power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo    enable
        UCHAR        AutoBA;        // Auto    BA will    automatically , BOOLEAN    
} OID_BACAP_STRUC, *POID_BACAP_STRUC;


//rt2860, kathy 2007-0118
// definition for DLS
#define    MAX_NUM_OF_INIT_DLS_ENTRY   1
#define    MAX_NUM_OF_DLS_ENTRY        4

// structure for DLS
typedef struct _RT_802_11_DLS{
    USHORT                TimeOut;        // unit: second , set by UI
    USHORT                CountDownTimer;    // unit: second , used by driver only
    NDIS_802_11_MAC_ADDRESS        MacAddr;        // set by UI
    UCHAR                Status;            // 0: none , 1: wait STAkey, 2: finish DLS setup , set by driver only
    UCHAR                Valid;            // 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
} RT_802_11_DLS, *PRT_802_11_DLS;

typedef enum _RT_802_11_DLS_MODE {
    DLS_NONE,
    DLS_WAIT_KEY,
    DLS_FINISH
} RT_802_11_DLS_MODE;

//Block ACK
#define TID_SIZE 8

typedef enum _REC_BLOCKACK_STATUS
{    
    Recipient_NONE=0,
    Recipient_USED,
    Recipient_HandleRes,
    Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS
{
    Originator_NONE=0,
    Originator_USED,
    Originator_WaitRes,
    Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;


//For QureyBATableOID use
typedef struct  _OID_BA_REC_ENTRY
{
    UCHAR   MACAddr[MAC_ADDR_LEN];
    UCHAR   BaBitmap;   // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize
    UCHAR   rsv; 
    UCHAR   BufSize[8];
    REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

//For QureyBATableOID use
typedef struct  _OID_BA_ORI_ENTRY
{
    UCHAR   MACAddr[MAC_ADDR_LEN];
    UCHAR   BaBitmap;  // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status
    UCHAR   rsv; 
    UCHAR   BufSize[8];
    ORI_BLOCKACK_STATUS  ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

//For SetBATable use
typedef struct {
    UCHAR    IsRecipient;
    NDIS_802_11_MAC_ADDRESS    MACAddr;
    UCHAR   TID;
    UCHAR   BufSize;
    USHORT    TimeOut;
    UCHAR     AllTid;  // If True, delete all TID for BA sessions with this MACaddr.
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

typedef struct _QUERYBA_TABLE
{
    OID_BA_ORI_ENTRY       BAOriEntry[32];
    OID_BA_REC_ENTRY       BARecEntry[32];
    UCHAR   OriNum;// Number of below BAOriEntry
    UCHAR   RecNum;// Number of below BARecEntry
} QUERYBA_TABLE, *PQUERYBA_TABLE;


//WPS , copy from RT2860 wsc.h, kathy
// WSC configured credential
typedef    struct    _WSC_CREDENTIAL
{
    NDIS_802_11_SSID    SSID;                // mandatory
    USHORT                AuthType;            // mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk
    USHORT                EncrType;            // mandatory, 1: none, 2: wep, 4: tkip, 8: aes
    UCHAR                Key[64];            // mandatory, Maximum 64 byte
    USHORT                KeyLength;
    UCHAR                MacAddr[6];            // mandatory, AP MAC address
    UCHAR                KeyIndex;            // optional, default is 1
    UCHAR                Rsvd[3];            // Make alignment
}    WSC_CREDENTIAL, *PWSC_CREDENTIAL;

// WSC configured profiles
typedef    struct    _WSC_PROFILE
{
	UINT			ProfileCnt;
	UINT			ApplyProfileIdx;  // add by johnli, fix WPS test plan 5.1.1
	WSC_CREDENTIAL	Profile[8];				// Support up to 8 profiles
}	WSC_PROFILE, *PWSC_PROFILE;

#endif

