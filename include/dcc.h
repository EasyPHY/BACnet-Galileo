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
#ifndef DCC_H
#define DCC_H

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"
#include "bacstr.h"

/* return the status */
    BACNET_COMMUNICATION_ENABLE_DISABLE dcc_enable_status(
        void);
    bool dcc_communication_enabled(
        void);
    bool dcc_communication_disabled(
        void);
    bool dcc_communication_initiation_disabled(
        void);
/* return the time */
    uint32_t dcc_duration_seconds(
        void);
/* called every second or so.  If more than one second,
  then seconds should be the number of seconds to tick away */
    void dcc_timer_seconds(
        uint32_t seconds);
/* setup the communication values */
    bool dcc_set_status_duration(
        BACNET_COMMUNICATION_ENABLE_DISABLE status,
        uint16_t minutes);

/* encode service */
    int dcc_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        uint16_t timeDuration,  /* 0=optional */
        BACNET_COMMUNICATION_ENABLE_DISABLE enable_disable,
        BACNET_CHARACTER_STRING * password);    /* NULL=optional */

/* decode the service request only */
    int dcc_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        uint16_t * timeDuration,
        BACNET_COMMUNICATION_ENABLE_DISABLE * enable_disable,
        BACNET_CHARACTER_STRING * password);

#endif
