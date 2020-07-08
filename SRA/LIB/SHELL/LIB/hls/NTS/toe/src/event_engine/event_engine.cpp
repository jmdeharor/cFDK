/************************************************
Copyright (c) 2016-2019, IBM Research.
Copyright (c) 2015, Xilinx, Inc.

All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************/

/*****************************************************************************
 * @file       : event_engine.cpp
 * @brief      : Event Engine (EVe) of the TCP Offload Engine (TOE)
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Transport Session (NTS)
 * Language    : Vivado HLS
 *
 *****************************************************************************/

#include "event_engine.hpp"
#include "../../test/test_toe_utils.hpp"

using namespace hls;

/************************************************
 * HELPERS FOR THE DEBUGGING TRACES
 *  .e.g: DEBUG_LEVEL = (MDL_TRACE | IPS_TRACE)
 ************************************************/
#ifndef __SYNTHESIS__
  extern bool gTraceEvent;
#endif

#define THIS_NAME "TOE/EVe"

#define TRACE_OFF  0x0000
#define TRACE_EVE 1 <<  1
#define TRACE_ALL  0xFFFF

#define DEBUG_LEVEL (TRACE_OFF | TRACE_EVE)

#ifndef __SYNTHESIS__
    const char* eventTypeStrings[] = {
             "TX", "TXbis", "RT", "RTbis", "ACK", "SYN", "SYN_ACK", "FIN", "RST", "ACK_NODELAY" };
    /***************************************************************************
     * @brief Converts an event type ENUM into a string.
     *
     * @param[in]   ev  The event type ENUM.
     * @returns the event type as a string.
     **************************************************************************/
    const char *getEventType(EventType ev) {
        return eventTypeStrings[ev];
    }
#endif

/*****************************************************************************
 * @brief The Event Engine (EVe) arbitrates the incoming events and forwards
 *         them to the Tx Engine (TXe).
 *
 * @param[in]  siTAi_Event,      Event from TxApplicationInterface (TAi).
 * @param[in]  siRXe_Event,      Event from RxEngine (RXe).
 * @param[in]  siTIm_Event,      Event from Timers (TIm).
 * @param[out] soAKd_Event,      Event to   AckDelayer (AKd).
 * @param[in]  siAKd_RxEventSig, The ACK Delayer just received an event.
 * @param[in]  siAKd_TxEventSig, The ACK Delayer just forwarded an event.
*  @param[in]  siTXe_RxEventSig, The Tx Engine (TXe)  just received an event.
 *
 *****************************************************************************/
void event_engine(
        stream<Event>           &siTAi_Event,
        stream<ExtendedEvent>   &siRXe_Event,
        stream<Event>           &siTIm_Event,
        stream<ExtendedEvent>   &soAKd_Event,
        stream<SigBit>          &siAKd_RxEventSig,
        stream<SigBool>         &siAKd_TxEventSig,
        stream<SigBit>          &siTXe_RxEventSig)
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS PIPELINE II=1

    const char *myName  = THIS_NAME;

    //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
    //---- Warning: the following counters depend on the FiFo depth between EVe and AKd
    static ap_uint<8>            eveTxEventCnt; // Keeps track of the #events forwarded to [AckDelayer]
    #pragma HLS RESET variable = eveTxEventCnt
    static ap_uint<8>            akdRxEventCnt; // Keeps track of the #events received  by [AckDelayer]
    #pragma HLS RESET variable = akdRxEventCnt
    static ap_uint<8>            akdTxEventCnt; // Keeps track of the #events forwarded to [TxEngine] by [AckDelayer]
    #pragma HLS RESET variable = akdTxEventCnt
    static ap_uint<8>            txeRxEventCnt; // Keeps track of the #events received  by [TxEngine]
    #pragma HLS RESET variable = txeRxEventCnt

    //-- DYNAMIC VARIABLES ----------------------------------------------------
    ExtendedEvent ev;

    //------------------------------------------
    // Handle input from [RxEngine]
    //------------------------------------------
    if (!siRXe_Event.empty() && !soAKd_Event.full()) {
        siRXe_Event.read(ev);
        soAKd_Event.write(ev);
        eveTxEventCnt++;
    }
    else if (eveTxEventCnt == akdRxEventCnt &&
             akdTxEventCnt == txeRxEventCnt) {
        //------------------------------------------
        // Handle input from [Timers]
        //------------------------------------------
        // RetransmitTimer and ProbeTimer events have priority
        if (!siTIm_Event.empty()) {
            siTIm_Event.read(ev);
            soAKd_Event.write(ev);
            eveTxEventCnt++;
            if (DEBUG_LEVEL & TRACE_EVE) {
                if (ev.type == RT_EVENT) {
                    printInfo(myName, "Received RT event from [TIm].\n");
                }
            }
        }
        //--------------------------------------------
        // Handle input from [TcpApplicationInterface]
        //--------------------------------------------
        else if (!siTAi_Event.empty()) {
            siTAi_Event.read(ev);
            assessSize(myName, soAKd_Event, "soAKd_Event", 4);
            soAKd_Event.write(ev);
            eveTxEventCnt++;
            if (DEBUG_LEVEL & TRACE_EVE) {
                printInfo(myName, "Received TX event from [TAi].\n");
            }
        }
    }

    //------------------------------------------
    // Handle input from [AckDelayer]
    //------------------------------------------
    if (!siAKd_RxEventSig.empty()) {
        // Remote [AckDelayer] just received an event from [EventEngine]
        siAKd_RxEventSig.read();
        akdRxEventCnt++;
    }

    if (!siAKd_TxEventSig.empty()) {
        // Remote [AckDelayer] just forwarded an event to [TxEngine]
        siAKd_TxEventSig.read();
        akdTxEventCnt++;
    }

    //------------------------------------------
    // Handle input from [TxEngine]
    //------------------------------------------
    if (!siTXe_RxEventSig.empty()) {
        siTXe_RxEventSig.read();
        txeRxEventCnt++;
    }
}
