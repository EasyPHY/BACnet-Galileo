/* ---------------------------------------------------------------------------------------

EasyPHY(R) Reference BACnet Stack and Application

Copyright (C) 2015 ConnectEx, Inc.  All rights reserverd.

All software deliverables are provided "AS IS" and ConnectEx, Inc. makes no
representations or warranties, either express or implied, in respect to any of
the foregoing, including without limitation, statuary or implied warranties
or conditions of merchantablility, satisfactory quality and acceptance, fitness
for a particular purpose or arising from a course of dealing or usage of
trade, all of which are expressly disclaimed. The disclaimers and exclusions
of this shall apply notwithstanding any failure of essential purpose of any
limited remedy.

License is granted for the use of this code solely for the development, test
and support of ConnectEx's EasyPHY(R) line of communication modules.
Commercial use of this software only granted for hardware platforms that support
ConnectEx's EasyPHY(R) line of communication modules.

Contact us at info@connect-ex.com for further information.

------------------------------------------------------------------------------------------*/

#ifndef MULTIPORT_H
#define MULTIPORT_H

#include "config.h"
#include "bacdef.h"
#include "net.h"
#include "npdu.h"

#ifdef _MSC_VER
#include <inaddr.h>
#endif

typedef struct {
        int socket;
        uint16_t nwoPort;                   // network order
        uint32_t local_addr ;               // network order
        uint32_t broadcast_addr ;           // network order
        uint32_t netmask;
    } BIP_PARAMS ;

typedef struct {
        uint32_t baudrate;
//        PARITY parity;
        uint8_t databits;
        uint8_t stopbits;
    } PTP_PARAMS ;

struct port_support {
    union
    {
        BIP_PARAMS bipParams;
        PTP_PARAMS ptpParams;
    } ;

    uint8_t *txBuf;     // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).   
    uint8_t *rxBuf;     // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).   
    uint16_t max_buf;

    int(*SendPdu) (const struct port_support *portSupport, BACNET_ADDRESS *phyDest, BACNET_NPDU_DATA *npdu_data, uint8_t *pdu, uint16_t pdu_len);
    int(*RecvPdu) (
        const struct port_support *portParams,
        BACNET_ADDRESS *src,        /* source address */
        uint8_t * pdu,              /* PDU data */
        uint16_t max_pdu,           /* amount of space available in the PDU  */
        unsigned timeout);

//    void(*get_broadcast_address) ( const struct port_support *portParams, BACNET_ADDRESS *bcastAddr);
//    void (*get_my_address) ( const struct port_support *portParams, BACNET_MAC_ADDRESS *my_address);
    BACNET_ADDRESS  bcastAddr;
    BACNET_ADDRESS  myAddress;
    struct port_support *next;
} ;

typedef struct port_support PORT_SUPPORT  ;

void SendToMsgQueue(PORT_SUPPORT *portSupport, BACNET_ADDRESS *dest, BACNET_NPDU_DATA *npdu_data, uint8_t *buffer, uint16_t apdu_len);

#endif
