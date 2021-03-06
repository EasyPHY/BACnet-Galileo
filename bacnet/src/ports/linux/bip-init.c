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
#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

 //#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
//#include "bacdcode.h"
//#include "bip.h"
//#include "datalink.h"
//#include "net.h"
//#include "CEDebug.h"
#include "multiport.h"

/** @file linux/bip-init.c  Initializes BACnet/IP interface (Linux). */

static bool BIP_Debug = true ;

/* gets an IP address by name, where name can be a
   string that is an IP address in dotted form, or
   a name that is a domain name
   returns 0 if not found, or
   an IP address in network byte order */
long bip_getaddrbyname(
    const char *host_name)
    {
    struct hostent *host_ent;

    if ((host_ent = gethostbyname(host_name)) == NULL) return 0;

    return *(long *)host_ent->h_addr;
    }

static int get_local_ifr_ioctl(
    char *ifname,
    struct ifreq *ifr,
    int request)
    {
    int fd;
    int rv;     /* return value */

  struct ifreq tifr ;
  memset(&tifr, 0, sizeof(tifr));

    strncpy(ifr->ifr_name, ifname, sizeof(ifr->ifr_name));
    fd = socket(AF_INET, SOCK_DGRAM, 17 ) ; //  IPPROTO_IP);

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)ifr, sizeof(struct ifreq)) < 0)
        {
        printf("Failed to bind to [%s] - Did you run SUDO?? \n", ifr->ifr_name);
        close(fd);
        return -1;
        }

    if (fd < 0)
        {
        printf("Socket open failed 1\n");
        rv = fd;
        }
    else
        {
        // printf("request = %d\n", request);
        rv = ioctl(fd, request, ifr);
        // printf("rv return code is %d\n", rv);
        close(fd);
        }

    return rv;
    }

int get_local_address_ioctl(
    PORT_SUPPORT *portParams,
    char *ifname,
    struct in_addr *addr,
    int request)
    {
    struct ifreq ifr = { { { 0 } } };
    struct sockaddr_in *tcpip_address;
    int rv;     /* return value */

    //hexdump("ifr", &ifr, sizeof(ifr));

    rv = get_local_ifr_ioctl(ifname, &ifr, request);
    if (rv >= 0)
        {
        tcpip_address = (struct sockaddr_in *) (void *) &ifr.ifr_addr;
        memcpy(addr, &tcpip_address->sin_addr, sizeof(struct in_addr));
        }

    //hexdump("ifr2", &ifr, sizeof(ifr));

    return rv;
    }

/** Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        Eg, for Linux, ifname is eth0, ath0, arc0, and others.
 */
void bip_set_interface(
    PORT_SUPPORT *portParams,
    char *ifname)
    {
    struct in_addr local_address;
    struct in_addr broadcast_address;
    int rv ;

    /* setup local address */
    rv = get_local_address_ioctl(portParams, ifname, &local_address, SIOCGIFADDR);
    if (rv < 0)
        {
        local_address.s_addr = 0;
        }
    bip_set_addr(portParams, local_address.s_addr);
    if (BIP_Debug)
        {
        fprintf(stderr, "Interface: %s\n", ifname);
        fprintf(stderr, "IP Address: %s\n", inet_ntoa(local_address));
        }
    /* setup local broadcast address */
    rv = get_local_address_ioctl(portParams, ifname, &broadcast_address, SIOCGIFBRDADDR);
    if (rv < 0)
        {
        broadcast_address.s_addr = ~0;
        }

    bip_set_broadcast_addr( portParams, broadcast_address.s_addr);
    if (BIP_Debug)
        {
        fprintf(stderr, "IP Broadcast Address2 : %s\n",
                inet_ntoa(broadcast_address));
//      fprintf(stderr, "UDP Port: 0x%04X [%hu]\n", ntohs(bip_get_port()),
//              ntohs(bip_get_port()));
        }
    }

/** Initialize the BACnet/IP services at the given interface.
 * @ingroup DLBIP
 * -# Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 * -# Opens a UDP socket
 * -# Configures the socket for sending and receiving
 * -# Configures the socket so it can send broadcasts
 * -# Binds the socket to the local IP address at the specified port for
 *    BACnet/IP (by default, 0xBAC0 = 47808).
 *
 * @note For Linux, ifname is eth0, ath0, arc0, and others.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "eth0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(
    PORT_SUPPORT *portParams,
    char *ifname)
    {
    int status = 0;     /* return from socket lib calls */
    struct sockaddr_in sin;
    int sockopt = 0;
    // int sock_fd = -1;
    struct ifreq ifr;

    if (ifname) bip_set_interface(portParams, ifname);
    else bip_set_interface(portParams, "eth0");

    /* assumes that the driver has already been initialized */
    portParams->bipParams.socket = socket(AF_INET, SOCK_DGRAM, 17 ) ; // IPPROTO_UDP);
    // bip_set_socket(sock_fd);
    if (portParams->bipParams.socket < 0) return false;

    // for R-Pi with Eth and Lon, we need to be sure to bind to the device, else we will get packet leakage
    // binds to lon interface, but needs to operate with root privileges
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), ifname );
    if (setsockopt(portParams->bipParams.socket, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
        {
        printf("failed to set socket option BINDTODEVICE [%s]\n", ifr.ifr_name);
        exit(-1);
        }

    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    sockopt = 1;
    status = setsockopt(portParams->bipParams.socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
    if (status < 0)
        {
        close(portParams->bipParams.socket);
        portParams->bipParams.socket = -1 ;
        return status;
        }
    /* allow us to send a broadcast */
    status = setsockopt(portParams->bipParams.socket, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof(sockopt));
    if (status < 0)
        {
        close(portParams->bipParams.socket);
        portParams->bipParams.socket = -1 ;
        return false;
        }
    /* bind the socket to the local port number and IP address */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = portParams->bipParams.nwoPort;
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
    status = bind(portParams->bipParams.socket, (const struct sockaddr *)&sin, sizeof(struct sockaddr));
    if (status < 0)
        {
        close(portParams->bipParams.socket);
        portParams->bipParams.socket = -1 ;
        return false;
        }

    return true;
    }

/** Cleanup and close out the BACnet/IP services by closing the socket.
 * @ingroup DLBIP
  */
void bip_cleanup(
    void)
    {
//    int sock_fd = 0;

//  if (bip_valid())
//      {
//      sock_fd = bip_socket();
//      close(sock_fd);
//      }
    // bip_set_socket(-1);

    return;
    }

/** Get the netmask of the BACnet/IP's interface via an ioctl() call.
 * @param netmask [out] The netmask, in host order.
 * @return 0 on success, else the error from the ioctl() call.
 */
//int bip_get_local_netmask(
    //PORT_SUPPORT *portParams,
    //struct in_addr *netmask)
    //{
    //int rv;
    //char *ifname = getenv("BACNET_IFACE");      /* will probably be null */
    //if (ifname == NULL) ifname = "eth0";
    //rv = get_local_address_ioctl(portParams, ifname, netmask, SIOCGIFNETMASK);
    //return rv;
    //}
