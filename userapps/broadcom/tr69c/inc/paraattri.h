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
//  Filename:       wirelessparams.h
//
******************************************************************************
//  Description:
//             Wireless supported object varable names
//
//
*****************************************************************************/

#ifndef CWMP_PARAMS_H
#define CWMP_PARAMS_H
#define  CWMPERROR                                                                  -1

#define  NONOTIFY                                                                       0
#define  PASSIVENOTIFY                                                             1
#define  ACTIVENOTIFY                                                               2
#define  CWMPPARAMETERINDEXVALUE                                     2

#define    INTERNETGATEWAYDEVICE                                        0



//DEVICEINFO
#define    DEVICEINFO                                                               (INTERNETGATEWAYDEVICE+CWMPPARAMETERINDEXVALUE)
#define    MANUFACTUREOUI                                                         (DEVICEINFO+CWMPPARAMETERINDEXVALUE)
#define    SERIALNUM                                                                (MANUFACTUREOUI+CWMPPARAMETERINDEXVALUE)
#define    MODEMFIRMWARE                                                    (SERIALNUM+CWMPPARAMETERINDEXVALUE)
#define    SOFTWARE                                                                  (MODEMFIRMWARE+CWMPPARAMETERINDEXVALUE)
#define    HARDWARE                                                                 (SOFTWARE+CWMPPARAMETERINDEXVALUE)
#define    PROVISIONINGCODE                                                   (HARDWARE+CWMPPARAMETERINDEXVALUE)
#define    UPTIME                                                                       (PROVISIONINGCODE+CWMPPARAMETERINDEXVALUE)
//END 该对象的结束位置为4


//MANAGEMENTSERVER 开始位置为6
#define    MANAGERMENT                                                          (UPTIME+CWMPPARAMETERINDEXVALUE)
#define    MANAGERMENTURL                                                    (MANAGERMENT+CWMPPARAMETERINDEXVALUE)
#define    USERNAME                                                                 (MANAGERMENTURL+CWMPPARAMETERINDEXVALUE)
#define    PASSWORD                                                                 (USERNAME+CWMPPARAMETERINDEXVALUE)
#define    PERIODICINFORMENABLE                                           (PASSWORD+CWMPPARAMETERINDEXVALUE)
#define    PERIODICINFORMINTERVAL                                       (PERIODICINFORMENABLE+CWMPPARAMETERINDEXVALUE)
#define    PERIODICINFORMTIME                                               (PERIODICINFORMINTERVAL+CWMPPARAMETERINDEXVALUE)
#define    PARAMETERKEY                                                           (PERIODICINFORMTIME+CWMPPARAMETERINDEXVALUE)
#define    CONNECTTIONQUESTURL                                           (PARAMETERKEY+CWMPPARAMETERINDEXVALUE)
#define    CONNECTTIONQUESTUSERNAME                                  (CONNECTTIONQUESTURL+CWMPPARAMETERINDEXVALUE)
#define    CONNECTIONREQUESTPASSWORD                               (CONNECTTIONQUESTUSERNAME+CWMPPARAMETERINDEXVALUE)
#define    KICKURL                                                                     (CONNECTIONREQUESTPASSWORD+CWMPPARAMETERINDEXVALUE)
#define    UPGRADESMANAGED                                                  (KICKURL+CWMPPARAMETERINDEXVALUE)
#define    MANAGEABLEDEVICENUMBEROFENTRIES                  (UPGRADESMANAGED+CWMPPARAMETERINDEXVALUE)
#define    MANAGEABLEDEVICENOTIFICATIONLIMIT                (MANAGEABLEDEVICENUMBEROFENTRIES+CWMPPARAMETERINDEXVALUE)
//end 该对象的结束位置为30 


//MANAGEABLEDEVICE  该对象的起始位置为32
#define    MANAGEABLEDEVICE                                                 (MANAGEABLEDEVICENOTIFICATIONLIMIT+CWMPPARAMETERINDEXVALUE)

#define    MANAGEABLEDEVICE_1                                             (MANAGEABLEDEVICE+CWMPPARAMETERINDEXVALUE)
#define    MANAGEABLEDEVICE1_MANUFACTURE                     (MANAGEABLEDEVICE_1+CWMPPARAMETERINDEXVALUE)
#define    MANAGEABLEDEVICE1_SERIALNUM                           (MANAGEABLEDEVICE1_MANUFACTURE+CWMPPARAMETERINDEXVALUE)
#define    MANAGEABLEDEVICE1_PRODUCTCLASS                     (MANAGEABLEDEVICE1_SERIALNUM+CWMPPARAMETERINDEXVALUE)


//中间需要容纳 4个实例
//END 结束位置为64


//USERINTERFACE  起始位置为66
#define    USERINTERFACE                                                        (MANAGEABLEDEVICE1_PRODUCTCLASS+CWMPPARAMETERINDEXVALUE+((MANAGEABLEDEVICE1_PRODUCTCLASS)-(MANAGEABLEDEVICE))*3) 

#define    UPGRADEAVAILABLE                                                  (USERINTERFACE+CWMPPARAMETERINDEXVALUE)
//END 结束位置68


//LAYER3FORWARDING  起始位置为70
#define    LAYER3FORWARDING                                                (UPGRADEAVAILABLE+CWMPPARAMETERINDEXVALUE)
#define    FORWARDING                                                            (LAYER3FORWARDING+CWMPPARAMETERINDEXVALUE)
 
#define   FORWARDING1                                                           (FORWARDING+CWMPPARAMETERINDEXVALUE)
#define   FORWARDING1_ENABLE                                             (FORWARDING1+CWMPPARAMETERINDEXVALUE)
#define   FORWARDING1_TYPE                                                 (FORWARDING1_ENABLE+CWMPPARAMETERINDEXVALUE)
#define   FORWARDING1_DESTIPADDRESS                               (FORWARDING1_TYPE+CWMPPARAMETERINDEXVALUE)
#define    FORWARDING1_DESTSUBNETMASK                           (FORWARDING1_DESTIPADDRESS+ CWMPPARAMETERINDEXVALUE)
#define    FORWARDING1_GATEWAYIPADDRESS                       (FORWARDING1_DESTSUBNETMASK+CWMPPARAMETERINDEXVALUE)
#define    FORWARDING1_INTERFACE                                       (FORWARDING1_GATEWAYIPADDRESS+CWMPPARAMETERINDEXVALUE)
//允许32条路有
//END 结束位置为520

//SERVICES   起始地址为522
#define  SERVICES                                                                      (FORWARDING1_INTERFACE+CWMPPARAMETERINDEXVALUE+((FORWARDING1_INTERFACE) -(FORWARDING))*31)


#define   VOICESERVICE                                                              (SERVICES+CWMPPARAMETERINDEXVALUE)
#define   VOICEPROFILENUM                                                      (VOICESERVICE+CWMPPARAMETERINDEXVALUE)
//VOICE PROFILE 这里有两个隐藏的实例数起始地址为528
#define  VOICEPROFILE                                                              (VOICEPROFILENUM+CWMPPARAMETERINDEXVALUE)

#define  VOICEPROFILE1                                                            (VOICEPROFILE+CWMPPARAMETERINDEXVALUE)
#define  PHYINTERFACE                                                              (VOICEPROFILE1+CWMPPARAMETERINDEXVALUE)

#define  DTMFMETHOD                                                               (PHYINTERFACE+CWMPPARAMETERINDEXVALUE)
#define  REGION                                                                         (DTMFMETHOD+CWMPPARAMETERINDEXVALUE)
#define  DIGITMAP                                                                     (REGION+CWMPPARAMETERINDEXVALUE)


  

#define  SERVICERTP                                                                  (DIGITMAP+CWMPPARAMETERINDEXVALUE)


#define  LOCALPORTMIN                                                             (SERVICERTP+CWMPPARAMETERINDEXVALUE)
#define  PROFILERTPRTCP                                                          (LOCALPORTMIN+ CWMPPARAMETERINDEXVALUE)   

#define  TXREPEATEINTERVAL                                                    (PROFILERTPRTCP+CWMPPARAMETERINDEXVALUE)




#define  NUMBERINGPLAN                                                          (TXREPEATEINTERVAL+CWMPPARAMETERINDEXVALUE)
#define  INTERDIGITTIMERSTD                                                  (NUMBERINGPLAN+CWMPPARAMETERINDEXVALUE)
#define  PREFIXINFO                                                                  (INTERDIGITTIMERSTD+CWMPPARAMETERINDEXVALUE)

#define  PREFIXINFO1                                                                (PREFIXINFO+CWMPPARAMETERINDEXVALUE)   
#define  PREFIXRANGE                                                               (PREFIXINFO1+CWMPPARAMETERINDEXVALUE)                                                   
#define  PREFIXINFOEND                                                           (PREFIXRANGE+CWMPPARAMETERINDEXVALUE+((PREFIXRANGE)-(PREFIXINFO))*63)
//END 需要预留64个对象


#define  VPNBUTTONDES                                                            (PREFIXINFOEND+CWMPPARAMETERINDEXVALUE)
//需要预留10个对象
#define  QUICKDIALNUMBER                                                       (VPNBUTTONDES+CWMPPARAMETERINDEXVALUE)                                        



#define  VPFAXT38DESC                                                            ( QUICKDIALNUMBER+CWMPPARAMETERINDEXVALUE +(CWMPPARAMETERINDEXVALUE)*9)
#define  FAXT38ENABLE                                                             (VPFAXT38DESC+CWMPPARAMETERINDEXVALUE)

#define  VPNLINE                                                                        (FAXT38ENABLE+CWMPPARAMETERINDEXVALUE)                                          

//这里需要增加两个Line 的实例起始地址为566
#define  VPNLINE1                                                                      (VPNLINE+CWMPPARAMETERINDEXVALUE)
#define  DIRECTORYNUMBER                                                       (VPNLINE1+CWMPPARAMETERINDEXVALUE)                                        

#define  LINESIP                                                                         (DIRECTORYNUMBER+CWMPPARAMETERINDEXVALUE)
#define  AUTHUSERNAME                                                            (LINESIP+ CWMPPARAMETERINDEXVALUE)                  
#define  AUTHPASSWORD                                                           (AUTHUSERNAME+CWMPPARAMETERINDEXVALUE)      

#define  VOICEPROCESSING                                                       (AUTHPASSWORD+CWMPPARAMETERINDEXVALUE)
#define  TRANSMITGAIN                                                             (VOICEPROCESSING+CWMPPARAMETERINDEXVALUE)
#define   RECEIVEGAIN                                                               (TRANSMITGAIN+CWMPPARAMETERINDEXVALUE)                                    
#define   ECHOCANCELLATIONENABLE                                        (RECEIVEGAIN+CWMPPARAMETERINDEXVALUE)       

#define  LINECODEC                                                                    (ECHOCANCELLATIONENABLE+CWMPPARAMETERINDEXVALUE)
//需要增加7个LIST实例 起始地址为586
#define  LINELIST1                                                                       (LINECODEC+CWMPPARAMETERINDEXVALUE)
#define  LINECODECLISTPACKETIZATIONPERIOID                       (LINELIST1+CWMPPARAMETERINDEXVALUE)
#define   LINECODECLISTSILENTCESUPPRESSION                       (LINECODECLISTPACKETIZATIONPERIOID+CWMPPARAMETERINDEXVALUE)
#define   PRIORITY                                                                       (LINECODECLISTSILENTCESUPPRESSION+CWMPPARAMETERINDEXVALUE)
#define   ENDLIST1                                                                       (PRIORITY+CWMPPARAMETERINDEXVALUE+((PRIORITY)-(LINECODEC))*6)
//END 对象结束地址为 642

#define   ENDOFLINE                                                                   (ENDLIST1 +CWMPPARAMETERINDEXVALUE+((ENDLIST1)-(VPNLINE)))
//Line对象的结束地址为722

//两个line 实例的结束 起始地址为724
#define   PROFILESIP                                                                  (ENDOFLINE+CWMPPARAMETERINDEXVALUE)                                           
#define   PROXYSERVER                                                                (PROFILESIP+CWMPPARAMETERINDEXVALUE)
#define   PROXYPORT                                                                    (PROXYSERVER+CWMPPARAMETERINDEXVALUE)
#define   REGISTERSERVER                                                           (PROXYPORT+ CWMPPARAMETERINDEXVALUE)                                         

#define   REGISTERSERVERPORT                                                  (REGISTERSERVER+CWMPPARAMETERINDEXVALUE)

#define    USERAGENTDOMAIN                                                    (REGISTERSERVERPORT+CWMPPARAMETERINDEXVALUE)                            
#define    USERAGENTPORT                                                        (USERAGENTDOMAIN+CWMPPARAMETERINDEXVALUE)
#define    REGISTEREXPIRES                                                   (USERAGENTPORT+CWMPPARAMETERINDEXVALUE)

//END 需要容纳两个profile实例profile 结束地址为948


// LAN DEVICE 起始地址为950
#define   LANDEVICE                                                                   (REGISTEREXPIRES+CWMPPARAMETERINDEXVALUE+((REGISTEREXPIRES)- (VOICEPROFILE)))

#define   LANHOSTCONFIGMANAGEMENT                                   (LANDEVICE+CWMPPARAMETERINDEXVALUE)

#define  DHCPSERVERENABLE                                                   (LANHOSTCONFIGMANAGEMENT+CWMPPARAMETERINDEXVALUE)
#define  MINADDRESS                                                                (DHCPSERVERENABLE+CWMPPARAMETERINDEXVALUE)
#define  MAXADDRESS                                                                (MINADDRESS+CWMPPARAMETERINDEXVALUE)
#define  RESERVERADDRESS                                                       (MAXADDRESS+CWMPPARAMETERINDEXVALUE)
#define  DNSSERVER                                                                   (RESERVERADDRESS+CWMPPARAMETERINDEXVALUE)
#define  LANDEVICEDOMAINPOS                                                    (DNSSERVER+CWMPPARAMETERINDEXVALUE)
#define  IPROUTERS                                                                    (LANDEVICEDOMAINPOS +CWMPPARAMETERINDEXVALUE)
#define  DHCPLEASETIME                                                            (IPROUTERS+CWMPPARAMETERINDEXVALUE)
/*start of diaplay the UseAllocatedWAN by l129990,2009-11-5*/
#define  ALLOCATEDWAN                                                          (DHCPLEASETIME+CWMPPARAMETERINDEXVALUE)
#define  ALLOWEDMACADDR                                                        (ALLOCATEDWAN+CWMPPARAMETERINDEXVALUE)
#define  LANIPINTERFCE                                                         (ALLOWEDMACADDR+CWMPPARAMETERINDEXVALUE)
/*end of diaplay the UseAllocatedWAN by l129990,2009-11-5*/
#define  LANENABLE                                                                         ( LANIPINTERFCE+CWMPPARAMETERINDEXVALUE)
#define  IPINTERFACEIPADDRESS                                               (LANENABLE+ CWMPPARAMETERINDEXVALUE)
#define  IPINTERFACESUBNETMASK                                           (IPINTERFACEIPADDRESS+CWMPPARAMETERINDEXVALUE)
#define  IPINTERFACEADDRESSTYPE                                          (IPINTERFACESUBNETMASK+CWMPPARAMETERINDEXVALUE)

#define  LANETHERNETINTERFACE                                             (IPINTERFACEADDRESSTYPE+CWMPPARAMETERINDEXVALUE)
#define  LANETHERNETENABLE                                                   (LANETHERNETINTERFACE+CWMPPARAMETERINDEXVALUE)
#define  LANETHINTERFACESTATUS                                             (LANETHERNETENABLE+CWMPPARAMETERINDEXVALUE)
#define  LANETHMACADDR                                                            (LANETHINTERFACESTATUS+CWMPPARAMETERINDEXVALUE)
#define  LANUSBINTERFACECONFIG                                            (LANETHMACADDR+CWMPPARAMETERINDEXVALUE)
#define  LANUSBENABLE                                                              (LANUSBINTERFACECONFIG+CWMPPARAMETERINDEXVALUE)

/*start of l68693 2008.06.11 HG553V100R001C02B018  AU8D00724*/
#define  WLANCONFIGURATION                                                    (LANUSBENABLE+CWMPPARAMETERINDEXVALUE)
#define  WLANENABLE                                                      (WLANCONFIGURATION+CWMPPARAMETERINDEXVALUE)
#define  WLANSTATUS                                                             (WLANENABLE+CWMPPARAMETERINDEXVALUE)
#define  CHANNEL                                                                (WLANSTATUS+CWMPPARAMETERINDEXVALUE)
#define  PERIODICALLYCHECKCHANNEL                                                  (CHANNEL+CWMPPARAMETERINDEXVALUE)
#define  WLSSID                                                                    (PERIODICALLYCHECKCHANNEL+CWMPPARAMETERINDEXVALUE)
#define  WLSSIDHIDE                                                                 (WLSSID+CWMPPARAMETERINDEXVALUE)
#define  WLWMMENABLE                                                            (WLSSIDHIDE+CWMPPARAMETERINDEXVALUE)
#define  WLMAXASSOCIATIONS                                                     (WLWMMENABLE+CWMPPARAMETERINDEXVALUE)
#define  WLBEACONTYPE                                                    (WLMAXASSOCIATIONS+CWMPPARAMETERINDEXVALUE)
#define  MACADDRESSCONTROL                                                    (WLBEACONTYPE+CWMPPARAMETERINDEXVALUE)
/*start of set up the White MAC Addresss by l129990,2009-10-9*/
#define  ALLOWEDWHITEMACADDR                                               (MACADDRESSCONTROL+CWMPPARAMETERINDEXVALUE)
#define  RADIOCONTROL                                                      (ALLOWEDWHITEMACADDR+CWMPPARAMETERINDEXVALUE)
#define  WEPKEYINDEX                                                     (RADIOCONTROL+CWMPPARAMETERINDEXVALUE)
/*end of set up the White MAC Addresss by l129990,2009-10-9*/
#define  WEPENCRYLEVEL                                                         (WEPKEYINDEX+CWMPPARAMETERINDEXVALUE)
#define  BASICENCRY                                                          (WEPENCRYLEVEL+CWMPPARAMETERINDEXVALUE)
#define  WPABSCAUTHMODE                                                         (BASICENCRY+CWMPPARAMETERINDEXVALUE)
#define  WPAENCRPTMODE                                                      (WPABSCAUTHMODE+CWMPPARAMETERINDEXVALUE)
#define  WPAAUTHMODE                                                         (WPAENCRPTMODE+CWMPPARAMETERINDEXVALUE)
#define  IEEE11IENCRYPTMODE                                                    (WPAAUTHMODE+CWMPPARAMETERINDEXVALUE)
#define  IEEE11IAUTHMODE                                                (IEEE11IENCRYPTMODE+CWMPPARAMETERINDEXVALUE)
#define  WLTOTALBYTESSENT                                                  (IEEE11IAUTHMODE+CWMPPARAMETERINDEXVALUE)
#define  WLTOTALBYTESRECV                                                 (WLTOTALBYTESSENT+CWMPPARAMETERINDEXVALUE)
#define  WLTOTALPACKETSSENT                                               (WLTOTALBYTESRECV+CWMPPARAMETERINDEXVALUE)
#define  WLTOTALPACKETSRECV                                             (WLTOTALPACKETSSENT+CWMPPARAMETERINDEXVALUE)
#define  WEPKEY                                                         (WLTOTALPACKETSRECV+CWMPPARAMETERINDEXVALUE)
#define  WEPKEY1                                                                    (WEPKEY+CWMPPARAMETERINDEXVALUE)
//需要增加4个wlan wepkey 对象
#define  WLANPSK                                                                   (WEPKEY1+CWMPPARAMETERINDEXVALUE+(CWMPPARAMETERINDEXVALUE)*3)
#define  WLANPSK1                                                                  (WLANPSK+CWMPPARAMETERINDEXVALUE)

//END LAN DEVICE 起始地址为1006


//WAN DEVICE  起始地址为1008
#define  WANDEVICE                                                                (WLANPSK1+CWMPPARAMETERINDEXVALUE)
/*end of l68693 2008.06.11 HG553V100R001C02B018  AU8D00724*/

//
#define  WANCOMMONINTERFACE                                                (WANDEVICE+CWMPPARAMETERINDEXVALUE)
#define  ENABLEINTERNET                                                           (WANCOMMONINTERFACE+CWMPPARAMETERINDEXVALUE)

// 
#define  WANDSLLINKINTERFACE                                                (ENABLEINTERNET+CWMPPARAMETERINDEXVALUE)
#define  WANDSLLINKENABLE                                                      (WANDSLLINKINTERFACE+CWMPPARAMETERINDEXVALUE)
//#define  STATUS                                                                          ENABLE+CWMPPARAMETERINDEXVALUE
//#define  MODULATIONTYPE                                                          STATUS+CWMPPARAMETERINDEXVALUE
//#define  LINEENCODING                                                               MODULATIONTYPE+CWMPPARAMETERINDEXVALUE
//
#define WANDCONNECTMANAGEMENTPOS                                          (WANDSLLINKENABLE+CWMPPARAMETERINDEXVALUE)


//
#define WANDSLDIAGONISTICPOS                                                      (WANDCONNECTMANAGEMENTPOS+CWMPPARAMETERINDEXVALUE)
//
#define WANCONNECTDEVICEPOS                                                      (WANDSLDIAGONISTICPOS+CWMPPARAMETERINDEXVALUE)

//start   WANCONNECTION 1 
#define WANCONNECTDEVICE_1                                                   (WANCONNECTDEVICEPOS+CWMPPARAMETERINDEXVALUE)

//WANDSLLINK
#define WANDSLLINK1                                                                   (WANCONNECTDEVICE_1+CWMPPARAMETERINDEXVALUE)
#define WANDSLLINKENABLE1                                                       (WANDSLLINK1+ CWMPPARAMETERINDEXVALUE)
#define LINKTYPE1                                                                         (WANDSLLINKENABLE1+CWMPPARAMETERINDEXVALUE)
#define DESTINATIONADDRESS1                                                     (LINKTYPE1+CWMPPARAMETERINDEXVALUE)
#define ATMENCAPSULATION1                                                         (DESTINATIONADDRESS1+CWMPPARAMETERINDEXVALUE)

//增加了4个ATM Qos的参数
#define ATMQoS1                                                                               (ATMENCAPSULATION1+CWMPPARAMETERINDEXVALUE)
#define ATMPeakCellRate1                                                                (ATMQoS1+CWMPPARAMETERINDEXVALUE)
#define ATMMaximumBurst1                                                            (ATMPeakCellRate1+CWMPPARAMETERINDEXVALUE)
#define ATMSustainableCellRate1                                                    (ATMMaximumBurst1+CWMPPARAMETERINDEXVALUE)

//WANIPCONNECTTION  起始地址为1036
#define  WANIPCONNECTION1                                                          (ATMSustainableCellRate1+CWMPPARAMETERINDEXVALUE)
#define  WANIPCONNECTENABLE1                                                    (WANIPCONNECTION1+CWMPPARAMETERINDEXVALUE)
#define  CONNECTIONSTATUS1                                                         (WANIPCONNECTENABLE1+CWMPPARAMETERINDEXVALUE)
#define  CONNECTIONTYPE1                                                             (CONNECTIONSTATUS1+CWMPPARAMETERINDEXVALUE)
#define  NAME1                                                                                 (CONNECTIONTYPE1+CWMPPARAMETERINDEXVALUE)
#define  NATENABLED1                                                                     (NAME1+CWMPPARAMETERINDEXVALUE)
#define  ADDRESSINGTYPE1                                                             (NATENABLED1+CWMPPARAMETERINDEXVALUE)
#define  EXTERNALIPADDRESS1                                                       (ADDRESSINGTYPE1+CWMPPARAMETERINDEXVALUE)
#define  SUBNETMASK1                                                                     (EXTERNALIPADDRESS1+CWMPPARAMETERINDEXVALUE)
#define  DEFAULTGATEWAY1                                                            (SUBNETMASK1+CWMPPARAMETERINDEXVALUE)
#define  DNSSERVER1                                                                        (DEFAULTGATEWAY1+CWMPPARAMETERINDEXVALUE)
#define  CONNECTIONTRIGGER1                                                       (DNSSERVER1+CWMPPARAMETERINDEXVALUE)

//支持32个多实例节点
#define  IPPORTMAPPING1                                                                  (CONNECTIONTRIGGER1+CWMPPARAMETERINDEXVALUE)
#define  IPPORTMAPPINGENABLE1                                                      (IPPORTMAPPING1+CWMPPARAMETERINDEXVALUE)
#define  IPEXTERNALPORT1                                                                 (IPPORTMAPPINGENABLE1+CWMPPARAMETERINDEXVALUE)
#define  IPINTERNALPORT1                                                                  (IPEXTERNALPORT1+CWMPPARAMETERINDEXVALUE)
#define  IPPORTMAPPINGPROTOCOL1                                                   (IPINTERNALPORT1+CWMPPARAMETERINDEXVALUE)
#define  IPINTERNALCLIENT1                                                              (IPPORTMAPPINGPROTOCOL1+CWMPPARAMETERINDEXVALUE)
#define  IPPORTMAPPINGDESCRIPTION1                                             (IPINTERNALCLIENT1+CWMPPARAMETERINDEXVALUE)

#define  IPPORTMAPINGLATGE                                                             ((IPPORTMAPPINGDESCRIPTION1)-(CONNECTIONTRIGGER1))
//这里需要容纳32个多实例节点

//WANPPPCONNECTTION  起始地址为1508
#define WANPPPCONNECTION1                                                              (IPPORTMAPPINGDESCRIPTION1+CWMPPARAMETERINDEXVALUE+((IPPORTMAPPINGDESCRIPTION1)-(CONNECTIONTRIGGER1))*31)

#define  PPPX_HW_DELAYTIMEFORREDIAL1                                      (WANPPPCONNECTION1+CWMPPARAMETERINDEXVALUE)
#define  PPPX_HW_TIMEOUT1                                                                  (PPPX_HW_DELAYTIMEFORREDIAL1+CWMPPARAMETERINDEXVALUE)
#define  PPPX_HW_MAXDIALUPDELAYTIME1                                      (PPPX_HW_TIMEOUT1+CWMPPARAMETERINDEXVALUE)
#define  PPPCONNECTIONTRIGGER                                             (PPPX_HW_MAXDIALUPDELAYTIME1+CWMPPARAMETERINDEXVALUE)
#define  PPPX_HW_MTU1                                                                            (PPPCONNECTIONTRIGGER+CWMPPARAMETERINDEXVALUE)
#define  PPPX_HW_ENBLBridgeMIXED1                                                  (PPPX_HW_MTU1+CWMPPARAMETERINDEXVALUE)
#define  PPPLCPECHO1                                                                               (PPPX_HW_ENBLBridgeMIXED1+CWMPPARAMETERINDEXVALUE)
#define  PPPLCPECHOTIME1                                                                              (PPPLCPECHO1+CWMPPARAMETERINDEXVALUE)
#define  PPPENABLE1                                                                              (PPPLCPECHOTIME1+CWMPPARAMETERINDEXVALUE)
#define  PPPCONNECTIONSTATUS1                                                         (PPPENABLE1+CWMPPARAMETERINDEXVALUE)
#define  PPPCONNECTIONTYPE1                                                              (PPPCONNECTIONSTATUS1+CWMPPARAMETERINDEXVALUE)
#define  PPPNAME1                                                                                  (PPPCONNECTIONTYPE1+CWMPPARAMETERINDEXVALUE)
#define  PPPIDLEDISCONNECTTIME1                                                      (PPPNAME1+CWMPPARAMETERINDEXVALUE)
#define  PPPNATENABLED1                                                                     (PPPIDLEDISCONNECTTIME1+CWMPPARAMETERINDEXVALUE)
#define  PPPUSERNAME1                                                                          (PPPNATENABLED1+CWMPPARAMETERINDEXVALUE)
#define  PPPPASSWD1                                                                             (PPPUSERNAME1+CWMPPARAMETERINDEXVALUE)
#define  PPPADDRESSINGTYPE1                                                              (PPPPASSWD1+CWMPPARAMETERINDEXVALUE)
#define  PPPEXTERNALIPADDRESS1                                                         (PPPADDRESSINGTYPE1+CWMPPARAMETERINDEXVALUE)
#define  PPPDNSSERVER1                                                                         (PPPEXTERNALIPADDRESS1+CWMPPARAMETERINDEXVALUE)
#define  PPPCONNECTIONTRIGGER1                                                         (PPPDNSSERVER1+CWMPPARAMETERINDEXVALUE)

//这里需要容纳32个多实例节点 
#define  PPPPORTMAPPING1                                                                  (PPPCONNECTIONTRIGGER1+CWMPPARAMETERINDEXVALUE)
#define  PPPPORTMAPPINGENABLE1                                                      (PPPPORTMAPPING1+CWMPPARAMETERINDEXVALUE)
#define  PPPEXTERNALPORT1                                                                 (PPPPORTMAPPINGENABLE1+CWMPPARAMETERINDEXVALUE)
#define  PPPINTERNALPORT1                                                                 (PPPEXTERNALPORT1+CWMPPARAMETERINDEXVALUE)
#define  PPPPORTMAPPINGPROTOCOL1                                                  (PPPINTERNALPORT1+CWMPPARAMETERINDEXVALUE)
#define  PPPINTERNALCLIENT1                                                              (PPPPORTMAPPINGPROTOCOL1+CWMPPARAMETERINDEXVALUE)
#define  PPPPORTMAPPINGDESCRIPTION1                                             (PPPINTERNALCLIENT1+CWMPPARAMETERINDEXVALUE)

#define  PPPORTMAPINGLATGE                                                              ((PPPPORTMAPPINGDESCRIPTION1)-(PPPCONNECTIONTRIGGER1))
#define  WANVIBINTERFACECONFIG                                                      (PPPPORTMAPPINGDESCRIPTION1+CWMPPARAMETERINDEXVALUE+((PPPPORTMAPPINGDESCRIPTION1)-(PPPCONNECTIONTRIGGER1))*31)
//这里需要容纳8条wan 接口
//end of  WANCONNECTTION  结束位置 1982

#define WANTREESTARDLONELARGE                                                        ((WANVIBINTERFACECONFIG)-(WANCONNECTDEVICEPOS))




//TIME 对象起始位置8802
#define  ADSLWANDEVICEEND                                                                (WANVIBINTERFACECONFIG+CWMPPARAMETERINDEXVALUE+((WANVIBINTERFACECONFIG)-(WANDEVICE ))*7)

//需要容纳HSPA上行参数
#define VIBINTERFACECONFIG                                                              (ADSLWANDEVICEEND+CWMPPARAMETERINDEXVALUE)
#define VIBCONNECTIONORDER                                                             (VIBINTERFACECONFIG+CWMPPARAMETERINDEXVALUE)
#define VIBBAND                                                                                    (VIBCONNECTIONORDER+CWMPPARAMETERINDEXVALUE)

#define VIBSTAT                                                                                     (VIBBAND+CWMPPARAMETERINDEXVALUE)
#define VIBDATAWORKING                                                                     (VIBSTAT+CWMPPARAMETERINDEXVALUE)
#define VIBVOICIEWORKING                                                                  (VIBDATAWORKING+CWMPPARAMETERINDEXVALUE)
#define VIBREGISTERTYPE                                                                     (VIBVOICIEWORKING+CWMPPARAMETERINDEXVALUE)
#define VIBREGISTERBAND                                                                    (VIBREGISTERTYPE+CWMPPARAMETERINDEXVALUE)
#define VIBREGISTERNETWORK                                                             (VIBREGISTERBAND+CWMPPARAMETERINDEXVALUE)
#define VIBREGISTERNETQOS                                                                (VIBREGISTERNETWORK+CWMPPARAMETERINDEXVALUE)
#define VIBASSIGNEDQOS                                                                      (VIBREGISTERNETQOS+CWMPPARAMETERINDEXVALUE)
#define VIBSIGNEDQOS                                                                          (VIBASSIGNEDQOS+CWMPPARAMETERINDEXVALUE)
/*start of 增加Global HSPA定制参数 by c00131380 at 080830*/
#define VIBCONNECTED                                                                     (VIBSIGNEDQOS+CWMPPARAMETERINDEXVALUE)
/*end of 增加Global HSPA定制参数 by c00131380 at 080830*/


#define VIBDEVICEINFO                                                                        (VIBCONNECTED+CWMPPARAMETERINDEXVALUE)
#define VIBCARMODE                                                                             (VIBDEVICEINFO+CWMPPARAMETERINDEXVALUE)
#define VIBCARIMSI                                                                               (VIBCARMODE+CWMPPARAMETERINDEXVALUE)
#define VIBCARIMEI                                                                               (VIBCARIMSI+CWMPPARAMETERINDEXVALUE)
#define VIBMANUFACTURE                                                                     (VIBCARIMEI+CWMPPARAMETERINDEXVALUE)
#define VIBHARDWARE                                                                          (VIBMANUFACTURE+CWMPPARAMETERINDEXVALUE)
#define VIBSOFTWARE                                                                           (VIBHARDWARE+CWMPPARAMETERINDEXVALUE)
#define VIBSERIANUM                                                                            (VIBSOFTWARE+CWMPPARAMETERINDEXVALUE)

/*start of 增加glb vdf 关于hspa 新需求 by s53329  at  20080909*/
#define SWITCHALGORITHM                                                                      (VIBSERIANUM+CWMPPARAMETERINDEXVALUE)
#define IFINSTANTMODE                                                                        (SWITCHALGORITHM+CWMPPARAMETERINDEXVALUE)
#define DATAPREFER                                                                           (IFINSTANTMODE+CWMPPARAMETERINDEXVALUE)
#define PROBEMETHOD                                                                              (DATAPREFER+CWMPPARAMETERINDEXVALUE)
#define ICMPCYCLE                                                                              (PROBEMETHOD+CWMPPARAMETERINDEXVALUE)
#define SWITCHFIRM                                                                             (ICMPCYCLE+CWMPPARAMETERINDEXVALUE)
#define DSL2HSPADELAYCS                                                                         (SWITCHFIRM+CWMPPARAMETERINDEXVALUE)
#define DSL2HSPADELAYPS                                                                         (DSL2HSPADELAYCS+CWMPPARAMETERINDEXVALUE)
#define HSPA2DSLDELAYCS                                                                         (DSL2HSPADELAYPS+CWMPPARAMETERINDEXVALUE)
#define HSPA2DSLDELAYPS                                                                          (HSPA2DSLDELAYCS+CWMPPARAMETERINDEXVALUE)
/*end of 增加glb vdf 关于hspa 新需求 by s53329  at  20080909*/

#define  VIBLINKCONFIG                                                                        (HSPA2DSLDELAYPS+CWMPPARAMETERINDEXVALUE)
#define  VIBPROFILENAME                                                                    (VIBLINKCONFIG+CWMPPARAMETERINDEXVALUE)
#define  VIBAPN                                                                                    (VIBPROFILENAME+CWMPPARAMETERINDEXVALUE)
#define  VIBDIALNUM                                                                            (VIBAPN+CWMPPARAMETERINDEXVALUE)
#define  VIBPASSWDREQ                                                                       (VIBDIALNUM+CWMPPARAMETERINDEXVALUE)
#define  VIBUSERNAME                                                                         (VIBPASSWDREQ+CWMPPARAMETERINDEXVALUE)
#define  VIBPASSWD                                                                             (VIBUSERNAME+CWMPPARAMETERINDEXVALUE)
#define  VIBIPADDRESS                                                                        (VIBPASSWD+CWMPPARAMETERINDEXVALUE)
#define  VIBPRIMARYDNS                                                                      (VIBIPADDRESS+CWMPPARAMETERINDEXVALUE)
#define  VIBSECONDDNS                                                                        (VIBPRIMARYDNS+CWMPPARAMETERINDEXVALUE)
#define  VIBAUTHPROTOCOL                                                                  (VIBSECONDDNS+CWMPPARAMETERINDEXVALUE)
#define  VIBHEARDCOMPRESSION                                                          (VIBAUTHPROTOCOL+CWMPPARAMETERINDEXVALUE)

//对象结束
#define  TIME                                                                                         (VIBHEARDCOMPRESSION+CWMPPARAMETERINDEXVALUE)
#define  NTPSERVER1                                                                             (TIME+CWMPPARAMETERINDEXVALUE)
#define  NTPSERVER2                                                                             (NTPSERVER1+CWMPPARAMETERINDEXVALUE)
//BEGIN:add by zhourongfei to config 3rd ntpserver
#define  NTPSERVER3                                                                             (NTPSERVER2+CWMPPARAMETERINDEXVALUE)
//END:add by zhourongfei to config 3rd ntpserver
#define  CURRENTTIME                                                                            (NTPSERVER3+CWMPPARAMETERINDEXVALUE)
#define  LOCALTIMEZONE                                                                       (CURRENTTIME+CWMPPARAMETERINDEXVALUE)
#define  LOCALTIMEZONENAME                                                              (LOCALTIMEZONE+CWMPPARAMETERINDEXVALUE)
#define  NTPREFRESHFAILTIME                                                              (LOCALTIMEZONENAME+CWMPPARAMETERINDEXVALUE)
#define  NTPREFRESHSUCTIME                                                                (NTPREFRESHFAILTIME+CWMPPARAMETERINDEXVALUE)
#define  NTPREFRESHSUM                                                                    (NTPREFRESHSUCTIME+CWMPPARAMETERINDEXVALUE)
/*start of enable or disable Upnp Video control function by l129990,2010,2,20*/
#define  UPNPVIDEO                                                                          (NTPREFRESHSUM+CWMPPARAMETERINDEXVALUE)
#define  CONFIGUPNPVIDEOPROV                                                                (UPNPVIDEO+CWMPPARAMETERINDEXVALUE)
#define  CONFIGUPNPVIDEOSERV                                                                (CONFIGUPNPVIDEOPROV+CWMPPARAMETERINDEXVALUE)
/*end of enable or disable Upnp Video control function by l129990,2010,2,20*/
/*start of 增加Tr069上支持组播组数可配功能 by l129990 2008,9,13*/
#define  IGMPGROUP                                                                          (CONFIGUPNPVIDEOSERV+CWMPPARAMETERINDEXVALUE)
#define  CONFIGGROUP                                                                        (IGMPGROUP+CWMPPARAMETERINDEXVALUE)
/*end of 增加Tr069上支持组播组数可配功能 by l129990 2008,9,13*/
//SYSLOG 对象 起始位置8812
/*start of 增加Tr069上支持组播组数可配功能 by l129990 2008,9,13*/
#define  SYSLOGCONFIG                                                                        (IGMPGROUP +CWMPPARAMETERINDEXVALUE)
/*end of 增加Tr069上支持组播组数可配功能 by l129990 2008,9,13*/

/*start of 增加tr143 功能*/
#define CAPABILITIES                                                                          (SYSLOGCONFIG+CWMPPARAMETERINDEXVALUE)
/*end of 增加tr143 功能*/

#define  SYSLOGENABLE                                                                        (SYSLOGCONFIG+CWMPPARAMETERINDEXVALUE)
#define  SYSLOGLEVEL                                                                          (SYSLOGENABLE+CWMPPARAMETERINDEXVALUE)
#define  SYSLOGDISPLAYLEVEL                                                           (SYSLOGLEVEL+CWMPPARAMETERINDEXVALUE)
#define  SYSLOGMODE                                                                         (SYSLOGDISPLAYLEVEL+CWMPPARAMETERINDEXVALUE)
#define  SYSLOGSERVERIP                                                                   (SYSLOGMODE+CWMPPARAMETERINDEXVALUE)
#define  SYSLOGSERVERUDPPORT                                                        (SYSLOGSERVERIP+CWMPPARAMETERINDEXVALUE)
/*add by z67625 增加支持防火墙日志记录flash开关 start*/
#define  FWLOGRECORD                                                                 (SYSLOGSERVERUDPPORT+CWMPPARAMETERINDEXVALUE)
/*add by z67625 增加支持防火墙日志记录flash开关 end*/

//QUEUE    起始位置8826
/*modified by z67625 增加支持防火墙日志记录flash开关 start*/
#define  QUEUEMANAGEMENT                                                               (FWLOGRECORD+CWMPPARAMETERINDEXVALUE)
/*modified by z67625 增加支持防火墙日志记录flash开关 end*/
#define  QUEUEENANBLE                                                                       (QUEUEMANAGEMENT+CWMPPARAMETERINDEXVALUE)
/*start   of 修改问题单AU8D01018 by s53329 at  210081021*/
#define  HWSERVICE                                                                           (QUEUEENANBLE+CWMPPARAMETERINDEXVALUE)
#define  HWSERVICE1                                                                           (HWSERVICE+CWMPPARAMETERINDEXVALUE)
#define  SERVICENAME                                                                           (HWSERVICE1+CWMPPARAMETERINDEXVALUE)
#define  SERVICECFG                                                                             (SERVICENAME+CWMPPARAMETERINDEXVALUE)
#define  SERVICECFG1                                                                             (SERVICECFG+CWMPPARAMETERINDEXVALUE)
#define  CFGPROTOCOL                                                                              (SERVICECFG1+CWMPPARAMETERINDEXVALUE)
#define  HWDESTPORT                                                                                  (CFGPROTOCOL+CWMPPARAMETERINDEXVALUE)
#define  HWDESTPORTRANGEMAX                                                                        (HWDESTPORT+CWMPPARAMETERINDEXVALUE)
#define  HWSOURCEPORT                                                                               (HWDESTPORTRANGEMAX+CWMPPARAMETERINDEXVALUE)
#define  HWSOURCEPORTRANGEMAX                                                                       (HWSOURCEPORT+CWMPPARAMETERINDEXVALUE)
#define  ENDSERVICECFG                                                                            (HWSOURCEPORTRANGEMAX+CWMPPARAMETERINDEXVALUE+((HWSOURCEPORTRANGEMAX)-(SERVICECFG))*31)                                               
#define  ENDHWSERVICE                                                                            (HWSERVICE+CWMPPARAMETERINDEXVALUE+(((ENDSERVICECFG)-(HWSERVICE))*32))
/*end   of 修改问题单AU8D01018 by s53329 at  210081021*/
#define  CLASSIFICATION                                                                          (ENDHWSERVICE+CWMPPARAMETERINDEXVALUE)

#define   CLASSIFICATION1                                                                        (CLASSIFICATION+CWMPPARAMETERINDEXVALUE)
#define   CLASSINTERFACE                                                                         (CLASSIFICATION1+CWMPPARAMETERINDEXVALUE)
#define   DESTIP                                                                                          (CLASSINTERFACE+CWMPPARAMETERINDEXVALUE)
#define   SOURCEIP                                                                                      (DESTIP+CWMPPARAMETERINDEXVALUE)
#define   PROTOCOL                                                                                     (SOURCEIP+CWMPPARAMETERINDEXVALUE)
#define   HWSERVICENAME                                                                                  (PROTOCOL+CWMPPARAMETERINDEXVALUE)
/**START -- w00135358 add & modify for VDF GLOBAL QOS 20080915*/
#define   DESTMASK                                                                                     (HWSERVICENAME+CWMPPARAMETERINDEXVALUE)
#define   SOURCEMASK                                                                                   (DESTMASK+CWMPPARAMETERINDEXVALUE)
#define   DESTPORT                                                                                     (SOURCEMASK+CWMPPARAMETERINDEXVALUE)
#define   DESTPORTRANGEMAX                                                                             (DESTPORT+CWMPPARAMETERINDEXVALUE)
#define   SOURCEPORT                                                                                   (DESTPORTRANGEMAX+CWMPPARAMETERINDEXVALUE)
#define   SOURCEPORTRANGEMAX                                                                           (SOURCEPORT+CWMPPARAMETERINDEXVALUE)
#define   DESTMACADDR                                                                                  (SOURCEPORTRANGEMAX+CWMPPARAMETERINDEXVALUE)
#define   SOURCEMACADDR                                                                                (DESTMACADDR+CWMPPARAMETERINDEXVALUE)
#define   QOSDSCPCHECK                                                                                 (SOURCEMACADDR+CWMPPARAMETERINDEXVALUE)
#define   ETHERPRIOCHECK                                                                               (QOSDSCPCHECK+CWMPPARAMETERINDEXVALUE)
#define   VLANIDCHECK                                                                                  (ETHERPRIOCHECK+CWMPPARAMETERINDEXVALUE)
#define   ETHERTYPE                                                                                    (VLANIDCHECK+CWMPPARAMETERINDEXVALUE)
#define   VLANIDMARK                                                                                   (ETHERTYPE+CWMPPARAMETERINDEXVALUE)
#define   ETHERPRIOMARK                                                                                (VLANIDMARK+CWMPPARAMETERINDEXVALUE)
#define   QOSDSCPMARK                                                                                  (ETHERPRIOMARK+CWMPPARAMETERINDEXVALUE)
/*END -- w00135358 add modify for VDF GLOBAL QOS 20080915*/


#define   CLASSQUEUE                                                                                 (QOSDSCPMARK+CWMPPARAMETERINDEXVALUE)
//需要容纳32条qos规则 起始位置9536
#define   CWMPQUEUE                                                                                 (CLASSQUEUE+CWMPPARAMETERINDEXVALUE+((CLASSQUEUE)-(CLASSIFICATION))*31)

#define   QUEUE1                                                                                        (CWMPQUEUE+CWMPPARAMETERINDEXVALUE)
#define   QUEUEWEIGHT                                                                             (QUEUE1+CWMPPARAMETERINDEXVALUE)
#define   QUEUEPRECEDENCE                                                                      (QUEUEWEIGHT+CWMPPARAMETERINDEXVALUE)
#define   QUEUESCHEDULER                                                                        (QUEUEPRECEDENCE+CWMPPARAMETERINDEXVALUE)
//需要容纳32条qos队列


/*start of  增加VDF Security 模块参数改变通知属性功能 by s53329  at  20080516*/
//安全
#define  QUEUESCHEDULEREND                                                                 (QUEUESCHEDULER+CWMPPARAMETERINDEXVALUE+((QUEUESCHEDULER)-(CWMPQUEUE))*5)

#define  ENDQUEUE                                                                               30000
#define  HW_SECURITY                                                                            (ENDQUEUE+CWMPPARAMETERINDEXVALUE)
#define  HW_ACCESSTRL                                                                          (HW_SECURITY+CWMPPARAMETERINDEXVALUE)
#define  HW_ENABLEIGMP                                                                       (HW_ACCESSTRL+CWMPPARAMETERINDEXVALUE)

#define  HW_ACCESSTRLRULE                                                                 (HW_ENABLEIGMP+CWMPPARAMETERINDEXVALUE)
#define  HW_ACCESSTRLRULE1                                                                 (HW_ACCESSTRLRULE+CWMPPARAMETERINDEXVALUE)
#define  HW_SOURCEIPEND                                                                      (HW_ACCESSTRLRULE1+CWMPPARAMETERINDEXVALUE)
 
#define  HW_FIREWALL                                                                          (HW_SOURCEIPEND+CWMPPARAMETERINDEXVALUE+((HW_SOURCEIPEND)-(HW_ACCESSTRLRULE))*31)
#define  HW_LEVEL                                                                                 (HW_FIREWALL+CWMPPARAMETERINDEXVALUE)
/*add by z67625 增加ip过滤支持开关 start*/
#define  HW_IPFLT_ENABLE                                                                    (HW_LEVEL+CWMPPARAMETERINDEXVALUE)
/*add by z67625 增加ip过滤支持开关 start*/

#define  HW_CUSTOME                                                                           (HW_IPFLT_ENABLE+CWMPPARAMETERINDEXVALUE)


/*modifiedby z67625 增加ip过滤支持开关 start*/
#define  HW_CUSTOME1                                                                        (HW_CUSTOME+CWMPPARAMETERINDEXVALUE)
/*modified by z67625 增加ip过滤支持开关 start*/

#define  HW_CUSTOME_NAME                                                                 (HW_CUSTOME1+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_SOURCEIPSTART                                                (HW_CUSTOME_NAME+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_SOURCEIPEND                                                    (HW_CUSTOME_SOURCEIPSTART+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_DESTIPSTART                                                (HW_CUSTOME_SOURCEIPEND+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_DESTIPEND                                                    (HW_CUSTOME_DESTIPSTART+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_PROTOCOL                                                      (HW_CUSTOME_DESTIPEND+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_ACTION                                                           (HW_CUSTOME_PROTOCOL+CWMPPARAMETERINDEXVALUE)
#define  HW_CUSTOME_END                                                                 (HW_CUSTOME_ACTION+CWMPPARAMETERINDEXVALUE+((HW_CUSTOME_ACTION)-(HW_CUSTOME))*31)

#define  HW_DOS                                                                                  (HW_CUSTOME_END+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ENABLE                                                                    (HW_DOS+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSYNFLOOD                                                          (HW_DOS_ENABLE+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSYNSPEED                                                        (HW_DOS_TCPSYNFLOOD+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPFINFLOOD                                                           (HW_DOS_TCPSYNSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPFINSPEED                                                            (HW_DOS_TCPFINFLOOD+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_UDPFLOOD                                                                 (HW_DOS_TCPFINSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_UDPFLOODSPEED                                                                 (HW_DOS_UDPFLOOD+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ICMPFLOOD                                                                 (HW_DOS_UDPFLOODSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ICMPFLOODSPEED                                                       (HW_DOS_ICMPFLOOD+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSYNFLOODPEER                                                    (HW_DOS_ICMPFLOODSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSYNFLOODPEERSPEED                                          (HW_DOS_TCPSYNFLOODPEER+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPFINFLOODPEER                                                     (HW_DOS_TCPSYNFLOODPEERSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPFINFLOODPEERSPEED                                           (HW_DOS_TCPFINFLOODPEER+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_UDPFLOODPEER                                                         (HW_DOS_TCPFINFLOODPEERSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_UDPFLOODPEERSPEED                                              (HW_DOS_UDPFLOODPEER+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ICMPFLOODPEER                                                       (HW_DOS_UDPFLOODPEERSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ICMPFLOODPEERSPEED                                             (HW_DOS_ICMPFLOODPEER+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_PORTSCAN                                                                  (HW_DOS_ICMPFLOODPEERSPEED+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_PORTSCANSENS                                                         (HW_DOS_PORTSCAN+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_ICMPSMURF                                                                (HW_DOS_PORTSCANSENS+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_IPLAND                                                                       (HW_DOS_ICMPSMURF+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_IPSPOOF                                                                     (HW_DOS_IPLAND+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_IPTEARDROP                                                               (HW_DOS_IPSPOOF+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_PINGOFDEATH                                                             (HW_DOS_IPTEARDROP+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSCAN                                                                     (HW_DOS_PINGOFDEATH+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_TCPSYNWITHDATA                                                      (HW_DOS_TCPSCAN+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_UDPBOMB                                                                     (HW_DOS_TCPSYNWITHDATA+CWMPPARAMETERINDEXVALUE)

/*add by z67625 增加支持5种攻击方式 start*/
#define  HW_DOS_SAMESRCDST                                                                  (HW_DOS_UDPBOMB+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_BRDCSTSRC                                                                   (HW_DOS_SAMESRCDST+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_LANSRCIP                                                                    (HW_DOS_BRDCSTSRC+CWMPPARAMETERINDEXVALUE)
#define  HW_DOS_INVALIDTCPFLG                                                               (HW_DOS_LANSRCIP+CWMPPARAMETERINDEXVALUE)  
#define  HW_DOS_FRGMTHDR                                                                    (HW_DOS_INVALIDTCPFLG+CWMPPARAMETERINDEXVALUE)  
/*add by z67625 增加支持5种攻击方式 end*/
/*add by z67625 增加支持mac过滤开关 start*/
#define HW_MACFILTER                                                                        (HW_DOS_FRGMTHDR+CWMPPARAMETERINDEXVALUE)
#define HW_MACFLTERENABLE                                                                  (HW_MACFILTER+CWMPPARAMETERINDEXVALUE)
/*add by z67625 增加支持mac过滤开关 end*/

#define HW_URLFILTER                                                                           (HW_DOS_UDPBOMB+CWMPPARAMETERINDEXVALUE)
#define HW_URLFILTER1                                                                        (HW_URLFILTER+CWMPPARAMETERINDEXVALUE)
#define HW_URLFILTER_URL                                                                    (HW_URLFILTER1+CWMPPARAMETERINDEXVALUE)
/*end of  增加VDF Security 模块参数改变通知属性功能 by s53329  at  20080516*/









#define HW_URLFILTER_URLEND                                                   (HW_URLFILTER_URL+CWMPPARAMETERINDEXVALUE+((HW_URLFILTER_URL)-(HW_URLFILTER))*31)

#define SIPALG                                                                  (HW_URLFILTER_URLEND+CWMPPARAMETERINDEXVALUE)                             

#define SIPALG_ENABLE                                                            (SIPALG+CWMPPARAMETERINDEXVALUE)
#define SIPALG_PORT                                                              (SIPALG_ENABLE+CWMPPARAMETERINDEXVALUE)

#define INTERNETGATEWAYDEVICEEND                                                     (SIPALG_PORT+CWMPPARAMETERINDEXVALUE)


/* start of 增加Global MAC 地址扩容 c00131380 080826 */
#define MACADDITIONAL                                                            (INTERNETGATEWAYDEVICEEND+CWMPPARAMETERINDEXVALUE)
#define MACADDRESS                                                               (MACADDITIONAL+CWMPPARAMETERINDEXVALUE)
#define INUSED                                                                   (MACADDRESS+CWMPPARAMETERINDEXVALUE)
/* end of 增加Global MAC 地址扩容 c00131380 080826 */
//BEGIN:add by zhourongfei to snmp config
#define SNMPDEVICE									 (INUSED+CWMPPARAMETERINDEXVALUE)
#define SYSCONTACT									(SNMPDEVICE+CWMPPARAMETERINDEXVALUE)
#define SYSNAME									(SYSCONTACT+CWMPPARAMETERINDEXVALUE)
#define RWCOMMUNITY									(SYSNAME+CWMPPARAMETERINDEXVALUE)
#define ROCOMMUNITY									(RWCOMMUNITY+CWMPPARAMETERINDEXVALUE)
#define SYSLOCATION									 (ROCOMMUNITY+CWMPPARAMETERINDEXVALUE)
#define SNMPSTATUS								 (SYSLOCATION+CWMPPARAMETERINDEXVALUE)
//END:add by zhourongfei to snmp config
//BEGIN:add by zhourongfei to web config
#define WEBDEVICE									(SNMPSTATUS+CWMPPARAMETERINDEXVALUE)
#define LANDISPLAY									(WEBDEVICE+CWMPPARAMETERINDEXVALUE)
#define HSPADISPLAY									(LANDISPLAY+CWMPPARAMETERINDEXVALUE)
#define PINDISPLAY									 (HSPADISPLAY+CWMPPARAMETERINDEXVALUE)
//END:add by zhourongfei to web config

/*Begin:Added by luokunling l192527 for IMSI inform requirment,2011/9/23*/
#define IMSINUM                                                               (PINDISPLAY+CWMPPARAMETERINDEXVALUE)
/*End:Added by luokunling l192527,2011/9/23*/

#endif   // WIRELESS_PARAMS_H
