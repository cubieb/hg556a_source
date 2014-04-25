
/*----------------------------------------------------------------------*
<:copyright-broadcom 
 
 Copyright (c) 2005 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
 *----------------------------------------------------------------------*
 * File Name  : appdefs.h
 *
 * Description: general application wide defines for tr69
 * $Revision: 1.1 $
 * $Id: appdefs.h,v 1.1 2008/08/25 12:51:32 l65130 Exp $
 *----------------------------------------------------------------------*/
/* Buffer sizes */

#define MAXWEBBUFSZ	132217728	/* 2**28 - Default web message/download max buffer size */
#ifdef WT104_SUPPORT
#define PARAMLISTSZ (60000)    /*size of built-up parameter lists */
#define XMLBUFSZ    (70000)    /*size of complete SOAP message */
#else
#ifdef WIRELESS
#define PARAMLISTSZ (60000)    /*size of built-up parameter lists */
#define XMLBUFSZ    (70000)    /*size of complete SOAP message */
#else
#ifdef USE_SSL
#define PARAMLISTSZ (60000)    /*size of built-up parameter lists */
#define XMLBUFSZ    (70000)    /*size of complete SOAP message */
#else
#define PARAMLISTSZ (30000)    /*size of built-up parameter lists */
#define XMLBUFSZ    (35000)    /*size of complete SOAP message */
#endif   // USE_SSL
#endif   // WIRELESS
#endif   // WT104_SUPPORT
#define ATTRIBUTESAVESZ	2048	/* maximum size of notification attribute */
								/* storage in scratch pad */
/* compile time options */
/* xml debug output options */
//#define		DUMPSOAPOUT 		/* DUMP xml msg sent to ACS to stderr */
//#define		DUMPSOAPIN	 		/* DUMP xml msg from ACS to stderr */
/* xml formatting options */
/* #define 	OMIT_INDENT	  */  /* don't intend xml tag lines - smaller messages */

/* use openSSL lib to support https: */
/*#define    USE_SSL  */
#define    WRITEPIDFILE

/* Allow Reboot and Factory Reset on ACS disconnect error */
#define		ALLOW_DISCONNECT_ERROR	/* acsDisconnect will call reboot and reset clean up*/
									/* even if there is an ACS disconnect error */
/* Authentication options */
#define		ALLOW_AUTH_RECONNECT	/* Some web servers such as Apache will close */
									/* the connection when sending a 401 status	  */
									/* This allows the CPE to close the current connection */
									/* and reconnect to the server with the Authorization */
									/* header in the first POST message. The CPE will */
									/* attempt this if the connection is closed following */
									/* the 401 status from the server */
									/* Undefining this will prohibit the CPE from sending */
									/* the Authorization on a new connection */

/* Generic compile time flags combination may be defined for specific ACS below */

#define GENERATE_SOAPACTION_HDR      /* generate SOAPAction header in POST reqs*/

/* TR-069 schema flags */
//#define SUPPRESS_SOAP_ARRAYTYPE		/* suppress generation of soap-env arraytype */
									/* such as
										<Event SOAP-END:arrayType="cwmp:EventStruct[x]">
									   SUPPRESS_SOAP_ARRAYTYPE generates
									    <Event>
									***/

/* ACS Connection Initiation */
#define ACSCONN_PORT         30005       /* PORT to listen for acs connections */
#define ACSREALM    "IgdAuthentication"
#define ACSDOMAIN   "/"
#define ACSCONNPATH "/"
#define CFMLISTENPORT		 30006		/* port to listen for signals from cfm*/

/* Timer valuse */
#define	LOCK_CHECK_INTERVAL	 1000		/* interval to attempt to lock local CFM */
										/* configuration data */
										
#define ACSINFORMDELAY  500     /* initial delay (ms) after pgm start before */
                                /* sending inform to ACS */
#define CHECKWANINTERVAL    (60*1000) /* check wan up */
#define ACSRESPONSETIME    (30*1000) /* MAX Time to wait on ACS response */
#define CFMKICKDELAY		(3*1000) /* time to delay following a msg from the cfm*/
									/* before checking notifications. Allows the*/
									/* cfm to complete before starting */
						/* Retry intervals for ACS connect failures    */
						/* Retry time decays by 
						   CONN_DECAYTIME*<consecutivefailures> upto a
						   maximum of CONNECT_DECAYTIME*CONNDECAYMAX */ 
#define CONN_DECAYMAX		6	/* maximum number for decaying multiple */
#define CONN_DECAYTIME		10	/* decay time per multiple */
                                
 
/* Constants                           */

#define USER_AGENT_NAME    "GS_TR69_CPE-0.2"

#define TR69_DIR   "/var/tr69"
#ifdef TESTBOX
#define TR69_PID_FILE  "tr69pid"
#else
#define TR69_PID_FILE  "/var/tr69/tr69pid"
#endif

#define SHELL      "/bin/sh"

#define USE_CERTIFICATES
/* ACS Server Certificate File path */
#define	CERT_FILE	"/var/cert/acscert.cacert"

#ifdef USE_CERTIFICATES
#define ACS_CIPHERS "RSA:DES:SHA+RSA:RC4:SAH+MEDIUM"
#else
//#define ACS_CIPHERS "EXPORT40:SSLv3"
#define ACS_CIPHERS "SSLv3"
#endif


 
/************************************************************/
/* compile time conditions for specific ACS                 */
/* Uncomment the required definition                        */ 
/************************************************************/
//#define SUPPORT_ACS_CISCO
//#define SUPPORT_ACS_GS
//#define SUPPORT_ACS_DIMARK
//#define SUPPORT_ACS_PIRELLI
//#define SUPPORT_ACS_2WIRE
 

/* set conditional compile flags based on ACS choice */

#ifdef SUPPORT_ACS_CISCO
#define    FORCE_NULL_AFTER_INFORM      /* Set timer to force null http after sending Inform*/
#define 	SUPPRESS_EMPTY_PARAM         /* Cisco doesn't handle <param></param> form */
/*#define	SUPPRESS_XML_NEWLINES */  /* replaces \n with space to avoid cisco tool parser problem*/                                        
#endif /* SUPPORT_ACS_CISCO */



/* #ifdef */ /* next ACS ????? */

/*#endif*/   /*        */
