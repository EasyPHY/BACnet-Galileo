/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include <sys/socket.h>
#include <linux/in.h>
#include <memcopy.h>
#include <time.h>
#include "bacdcode.h"
#include "bacint.h"
#include "bip.h"
#include "bvlc.h"
// #include "net.h"        /* custom per port */
#if PRINT_ENABLED
// #include <stdio.h>      /* for standard i/o, like printing */
#endif
// #include "CEDebug.h"
#include "multiport.h"

void bip_set_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{       /* in network byte order */
    // BIP_Address.s_addr = net_address;
    portParams->bipParams.local_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_addr(PORT_SUPPORT *portParams)
{
    // return BIP_Address.s_addr;
    return portParams->bipParams.local_addr;
}

void bip_set_broadcast_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{       /* in network byte order */
    // BIP_Broadcast_Address.s_addr = net_address;
    portParams->bipParams.broadcast_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_addr(
    PORT_SUPPORT *portParams)
{
    // return BIP_Broadcast_Address.s_addr;
    return portParams->bipParams.broadcast_addr;
}


//void bip_set_port(
//    uint16_t port)
//{       /* in network byte order */
//    BIP_Port = port;
//}

/* returns network byte order */
//uint16_t bip_get_port(
//    void)
//{
//    return htons(47808);        // this is a BIG todo !!  -> need to depend on the individually set routerports...
//    // return BIP_Port;
//}

static void bip_decode_bip_address(
    uint8_t * bac_mac,
    struct in_addr *address,    /* in network format */
    uint16_t * port)
{       /* in network format */
        memcpy(&address->s_addr, &bac_mac[0], 4);
        memcpy(port, &bac_mac[4], 2);
}

/** Function to send a packet out the BACnet/IP socket (Annex J).
 * @ingroup DLBIP
 *
 * @param dest [in] Destination address (may encode an IP address and port #).
 * @param npdu_data [in] The NPDU header (Network) information (not used).
 * @param pdu [in] Buffer of data to be sent - may be null (why?).
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
int bip_send_pdu(
    const PORT_SUPPORT *portParams,
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    struct sockaddr_in bip_dest;
    // uint8_t mtu[MAX_MPDU_ETHERNET] = { 0 };
    // int mtu_len = 0;
    // int bytes_sent = 0;
    /* addr and port in host format */
    // struct in_addr address;
    // uint16_t port;
    uint8_t *targetMac;

    (void)npdu_data;
    /* assumes that the driver has already been initialized */
    //if (BIP_Socket < 0) {
    //    return BIP_Socket;
    //}

//    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = 2; // todonext4 AF_INET;

    // fill in the Router MAC if this message is for another network

    if ((dest->net == BACNET_BROADCAST_NETWORK) || (dest->mac_len == 0)) {
        /* broadcast */
        // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
        // // port = BIP_Port;
        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
        targetMac = NULL;   // broadcast
    }
    else if ((dest->net > 0) && (dest->len == 0)) {
        /* network specific broadcast */
        if (dest->mac_len == 6) {
            // bip_decode_bip_address(dest, &address, &port);
            targetMac = dest->mac ;
        }
        else {
            // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
            // // port = BIP_Port;
            targetMac = NULL;   // broadcast
        }
        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
    else if (dest->mac_len == 6) {
        // bip_decode_bip_address(dest, &address, &port);
        // mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        targetMac = dest->mac;
    }
    else {
        // panic("mxxx: Illegal MAC length for IP");
        return -1;
    }

    return bip_send_pdu_local_only(portParams, targetMac, pdu, pdu_len);
}


int bip_send_pdu_local_only(
    const PORT_SUPPORT *portParams,
    uint8_t *destMAC,       /* destination address, if null, then broadcast */
    uint8_t * pdu,          /* any data to be sent - may be null */
    uint16_t pdu_len)       /* number of bytes of data */
{       
    struct sockaddr_in bip_dest;
    
    // uint8_t *mtu = portParams->txBuf;
    uint8_t mtu[2000]; // todonext4 - we need to avoid this still use a prebuilt header somehow..

    int mtu_len;
    int bytes_sent ;
    /* addr and port in host format */
    struct in_addr address;
    uint16_t port;

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = 2; // todonext4  AF_INET;

    if (destMAC == NULL)
    {
        /* broadcast */
        address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
        // port = BIP_Port;
        port = portParams->bipParams.nwoPort;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
    else
    {
        bip_decode_bip_address(destMAC, &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    }

    bip_dest.sin_addr.s_addr = address.s_addr;
    bip_dest.sin_port = port;
    memset(&(bip_dest.sin_zero), '\0', 8);
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t)(pdu_len + 4 /*inclusive */));
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += pdu_len;

    /* Send the packet */
    bytes_sent =
        sendto(portParams->bipParams.socket, (char *)mtu, mtu_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    return bytes_sent;
}


/** Implementation of the receive() function for BACnet/IP; receives one
 * packet, verifies its BVLC header, and removes the BVLC header from
 * the PDU data before returning.
 *
 * @param src [out] Source of the packet - who should receive any response.
 * @param pdu [out] A buffer to hold the PDU portion of the received packet,
 * 					after the BVLC portion has been stripped off.
 * @param max_pdu [in] Size of the pdu[] buffer.
 * @param timeout [in] The number of milliseconds to wait for a packet.
 * @return The number of octets (remaining) in the PDU, or zero on failure.
 */
int bip_receive(
    const PORT_SUPPORT *portParams,
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{
    int received_bytes = 0;
    uint16_t pdu_len = 0;       /* return value */
    fd_set read_fds;
    int max = 0;
    struct timeval select_timeout;
    struct sockaddr_in sin = { 0 };
    int sin_len = sizeof(sin);
    uint16_t i = 0;
    int function = 0;

    /* Make sure the socket is open */
    if (portParams->bipParams.socket < 0)
    {
        return 0;
    }

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    if (timeout >= 1000) {
        select_timeout.tv_sec = timeout / 1000;
        select_timeout.tv_usec =
            1000 * (timeout - select_timeout.tv_sec * 1000);
    } else {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 1000 * timeout;
    }
    FD_ZERO(&read_fds);
    FD_SET( portParams->bipParams.socket, &read_fds);
    max = portParams->bipParams.socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0)
        received_bytes =
        recvfrom(portParams->bipParams.socket, (char *)&pdu[0], max_pdu, 0,
            (struct sockaddr *) &sin, &sin_len);
    else
        return 0;

    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }

    /* no problem, just no bytes */
    if (received_bytes == 0)
        return 0;

    /* the signature of a BACnet/IP packet */
    if (pdu[0] != BVLL_TYPE_BACNET_IP)
        return 0;

    if (bvlc_for_non_bbmd(portParams, &sin, pdu, received_bytes) > 0) {
        /* Handled, usually with a NACK. */
#if PRINT_ENABLED
        fprintf(stderr, "BIP: BVLC discarded!\n");
#endif
        return 0;
    }

    function = bvlc_get_function_code();        /* aka, pdu[1] */
    if ((function == BVLC_ORIGINAL_UNICAST_NPDU) ||
        (function == BVLC_ORIGINAL_BROADCAST_NPDU)) {
        /* ignore messages from me */
        if ((sin.sin_addr.s_addr == portParams->bipParams.local_addr ) &&
            (sin.sin_port == portParams->bipParams.nwoPort )) {
            pdu_len = 0;
#if 0
            fprintf(stderr, "BIP: src is me. Discarded!\n");
#endif
        } else {
            /* data in src->mac[] is in network format */
            src->mac_len = 6;
            memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
            memcpy(&src->mac[4], &sin.sin_port, 2);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&pdu[2], &pdu_len);
            /* subtract off the BVLC header */
            pdu_len -= 4;
            if (pdu_len < max_pdu) {
#if 0
                fprintf(stderr, "BIP: NPDU[%hu]:", pdu_len);
#endif
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < pdu_len; i++) {
                    pdu[i] = pdu[4 + i];
#if 0
                    fprintf(stderr, "%02X ", pdu[i]);
#endif
                }
#if 0
                fprintf(stderr, "\n");
#endif
            }
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            else {
                pdu_len = 0;
#if PRINT_ENABLED
                fprintf(stderr, "BIP: PDU too large. Discarded!.\n");
#endif
            }
        }
    } else if (function == BVLC_FORWARDED_NPDU) {
        memcpy(&sin.sin_addr.s_addr, &pdu[4], 4);
        memcpy(&sin.sin_port, &pdu[8], 2);
        if ((sin.sin_addr.s_addr == portParams->bipParams.local_addr) &&
            (sin.sin_port == portParams->bipParams.nwoPort )) {
            /* ignore messages from me */
            pdu_len = 0;
        } else {
            /* data in src->mac[] is in network format */
            src->mac_len = 6;
            memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
            memcpy(&src->mac[4], &sin.sin_port, 2);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&pdu[2], &pdu_len);
            /* subtract off the BVLC header */
            pdu_len -= 10;
            if (pdu_len < max_pdu) {
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < pdu_len; i++) {
                    pdu[i] = pdu[4 + 6 + i];
                }
            } else {
                /* ignore packets that are too large */
                /* clients should check my max-apdu first */
                pdu_len = 0;
            }
        }
    }

    return pdu_len;
}

void bip_get_my_address(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    //int i = 0;

    //if (my_address) {
        my_address->len = 6;
        memcpy(&my_address->adr[0], &portParams->bipParams.local_addr, 4);
        memcpy(&my_address->adr[4], &portParams->bipParams.nwoPort, 2);
    //    my_address->net = 0;    /* local only, no routing */
    //    my_address->len = 0;    /* no SLEN */
    //    for (i = 0; i < MAX_MAC_LEN; i++) {
    //        /* no SADR */
    //        my_address->adr[i] = 0;
    //    }
    //}

    //return;
}

void bip_get_broadcast_address(
    const PORT_SUPPORT *portParams,
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        dest->mac_len = 6;
        memcpy(&dest->mac[0], &portParams->bipParams.broadcast_addr, 4);
        memcpy(&dest->mac[4], &portParams->bipParams.nwoPort, 2);
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            dest->adr[i] = 0;
        }
    }

    return;
}
