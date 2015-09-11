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
#include <errno.h>
#include <string.h>
#include "config.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "bacaddr.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "multiport.h"
#include "dcc.h"
#include "ihave.h"
/* some demo stuff needed */
#include "handlers.h"

/** @file s_ihave.c  Send an I-Have (property) message. */

/** Broadcast an I Have message.
 * @ingroup DMDOB
 *
 * @param device_id [in] My device ID.
 * @param object_type [in] The BACNET_OBJECT_TYPE that I Have.
 * @param object_instance [in] The Object ID that I Have.
 * @param object_name [in] The Name of the Object I Have.
 */
void Send_I_Have(
    PORT_SUPPORT *portParams,
    uint32_t device_id,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_I_HAVE_DATA data;
    BACNET_NPDU_DATA npdu_data;
    // BACNET_ADDRESS my_address;

    // datalink_get_my_address(&my_address);
    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return;
    /* Who-Has is a global broadcast */
    set_global_broadcast(portParams, &dest);
    
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&portParams->txBuf[0], &dest, &portParams->myAddress,
        &npdu_data);

    /* encode the APDU portion of the packet */
    data.device_id.type = OBJECT_DEVICE;
    data.device_id.instance = device_id;
    data.object_id.type = object_type;
    data.object_id.instance = object_instance;
    characterstring_copy(&data.object_name, object_name);
    len = ihave_encode_apdu(&portParams->txBuf[pdu_len], &data);
    pdu_len += len;
    /* send the data */
    bytes_sent =
        portParams->SendPdu(portParams, &dest, &npdu_data, &portParams->txBuf[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0) {
        fprintf(stderr, "Failed to Send I-Have Reply (%s)!\n",
            strerror(errno));
    }
#else
    bytes_sent = bytes_sent;
#endif
}
