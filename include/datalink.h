/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef DATALINK_H
#define DATALINK_H

#include "config.h"
#include "bacdef.h"
// #include "bip.h"
#include "net.h"
#include "npdu.h"

#ifdef _MSC_VER
#include <inaddr.h>
#endif

/* port specific parameters */
// 2014.02.17 - EKH: I moved this structure here, and added IP parameters so multiple ports can be defined.

typedef struct {
        int socket;
        uint16_t nwoPort;                          // network order
        // uint16_t hoPort;
        // unsigned int port ;
        // struct in_addr local_addr;
        // struct in_addr broadcast_addr;
        uint32_t local_addr ;                   // network order
        uint32_t broadcast_addr ;               // network order
        uint32_t netmask;
    } BIP_PARAMS ;

typedef struct {
        uint32_t baudrate;
//        PARITY parity;
        uint8_t databits;
        uint8_t stopbits;
        uint8_t max_master;
        uint8_t max_frames;
    } MSTP_PARAMS ;

struct port_support {
    union
    {
        BIP_PARAMS bipParams;
        MSTP_PARAMS mstpParams;
    } ;

    uint8_t *txBuff;    // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).   
    uint16_t max_buff;

    int(*SendPdu) ( const struct port_support *portSupport, BACNET_ADDRESS *phyDest, BACNET_NPCI_DATA *npdu_data, uint8_t *pdu, uint16_t pdu_len);
    void(*get_broadcast_address) ( const struct port_support *portParams, BACNET_ADDRESS *bcastAddr);
    void (*get_my_address) ( const struct port_support *portParams, BACNET_MAC_ADDRESS *my_address);
} ;

typedef struct port_support PORT_SUPPORT  ;


void SendToMsgQueue(PORT_SUPPORT *portSupport, BACNET_ADDRESS *dest, BACNET_NPCI_DATA *npdu_data, uint8_t *buffer, uint16_t apdu_len);

#endif
