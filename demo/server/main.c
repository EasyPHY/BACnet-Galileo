/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
*
*********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "config.h"
#include "server.h"
#include "address.h"
#include "bacdef.h"
#include "handlers.h"
#include "dlenv.h"
#include "bacdcode.h"
#include "npdu.h"
#include "apdu.h"
#include "iam.h"
#include "device.h"
#include "multiport.h"
#include "dcc.h"
#include "net.h"
#include "version.h"
#include "bip.h"

#ifdef _MSC_VER
// todonext4 #include <vld.h>
#endif

PORT_SUPPORT *headPort = NULL;       /* pointer to list of ports */

void Init_Ports(void)
{
    headPort = malloc(sizeof(PORT_SUPPORT));
    if (!headPort) return;

    headPort->next = NULL;
    headPort->bcastAddr;
    headPort->txBuf = malloc(1496);
    headPort->rxBuf = malloc(1496);
    headPort->max_buf = 1496;
    headPort->SendPdu = bip_send_pdu;
    headPort->RecvPdu = bip_receive;

}

void print_help_main(void)
{
    printf("Help:\n");
    printf("  q - Quit\n");
    printf("  i - Send I-Am Router to all\n");
    printf("\n");
}

void Send_I_Am_Broadcast(void)
{
    PORT_SUPPORT *port = headPort;

    while (port != NULL)
    {
        Send_I_Am(port, &port->txBuf[0]);
        port = port->next;
    }
}


/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* handle the data coming back from private requests */
}

static void print_usage(char *filename)
{
    printf("Usage: %s [device-instance [device-name]]\n", filename);
    printf("       [--version][--help]\n");
}

static void print_help(char *filename)
{
    printf("Simulate a BACnet server device\n"
        "device-instance:\n"
        "BACnet Device Object Instance number that you are\n"
        "trying simulate.\n"
        "device-name:\n"
        "The Device object-name is the text name for the device.\n"
        "\nExample:\n");
    printf("To simulate Device 123, use the following command:\n"
        "%s 123\n", filename);
    printf("To simulate Device 123 named Fred, use following command:\n"
        "%s 123 Fred\n", filename);
}


int main(
    int argc,
    char *argv[])
{
    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 1;       /* milliseconds */
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    uint32_t elapsed_seconds = 0;
    uint32_t elapsed_milliseconds = 0;
    uint32_t address_binding_tmr = 0;
    bool keepGoing = true;
    uint32_t recipient_scan_tmr = 0;

#ifdef _MSC_VER
    // Visual Leak Detection enable, operates on a PER THREAD basis (i.e. make sure enabled on other threads).
//    VLDEnable();

    // todonext ! Re-enable this.
    /*
    // Other debug flags
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_ALLOC_MEM_DF;
    tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
    // tmpDbgFlag |= _CRTDBG_CHECK_CRT_DF;				// library leaks - ignore them
    tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    //*/
#else
    printf("mxxxx: Remember to re-enable VLD\n\n");
#endif

    /* allow the device ID to be set */
    if (argc > 1) {
        Device_Set_Object_Instance_Number(strtol(argv[1], NULL, 0));
    }
    if (argc > 2) {
        Device_Object_Name_ANSI_Init(argv[2]);
    }

    printf("BACnet Galileo Server Demo\n" "BACnet Stack Version %s\n"
        "BACnet Device ID: %u\n" "Max APDU: %d\n", BACnet_Version,
        Device_Object_Instance_Number(), MAX_APDU);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init();
    Init_Service_Handlers();

    Init_Ports();

    //     dlenv_init();
    //     atexit(datalink_cleanup);
    /* configure the timeout values */
    Send_I_Am_Broadcast();
    last_seconds = time(NULL);
    while (keepGoing)
    {
        /* input */
        current_seconds = time(NULL);

        /* returns 0 bytes on timeout */
        PORT_SUPPORT *port = headPort;
        while (port)
        {
            pdu_len = port->RecvPdu( port, &src, port->rxBuf, port->max_buf, timeout);

            /* process */
            if (pdu_len) {
                npdu_handler( port, &src, port->rxBuf, pdu_len);
            }

            port = port->next;
        }

        /* at least one second has passed */
        elapsed_seconds = (uint32_t)(current_seconds - last_seconds);
        if (elapsed_seconds) {
            last_seconds = current_seconds;
            dcc_timer_seconds(elapsed_seconds);
        }

        /* output */

        /* blink LEDs, Turn on or off outputs, etc */
    }

    return 0;
}

/* @} */

/* End group ServerDemo */
