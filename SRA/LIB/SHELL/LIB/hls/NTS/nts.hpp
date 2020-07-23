/*
 * Copyright 2016 -- 2020 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*******************************************************************************
 * @file       : nts.hpp
 * @brief      : Definition of the Network Transport Stack (NTS) component
 *               as if it was an HLS IP core.
 *
 * System:     : cloudFPGA
 * Component   : Shell
 * Language    : Vivado HLS
 *
 * \ingroup NTS
 * \addtogroup NTS
 * \{
 *******************************************************************************/

#ifndef _NTS_H_
#define _NTS_H_

#include "nts_types.hpp"

using namespace hls;


/*******************************************************************************
 * INTERFACE - TCP APPLICATION INTERFACE (TAIF)
 *******************************************************************************
 * This section defines the interfaces between the Network and Transport Stack
 * (NTS) and the TCP Application Interface (TAIF) layer.
 *******************************************************************************/

//=========================================================
//== TAIF / RECEIVED & TRANSMITTED SEGMENT INTERFACES
//=========================================================

//---------------------------------------------------------
//-- TCP APP - DATA
//--  The data section of a TCP segment over an AXI4S I/F.
//---------------------------------------------------------
typedef AxisApp     TcpAppData;

//---------------------------------------------------------
//-- TCP APP - METADATA
//--  The session identifier of a connection.
//---------------------------------------------------------
typedef TcpSessId   TcpAppMeta;

//---------------------------------------------------------
//-- TCP APP - NOTIFICATION
//--  Notifies the availability of data for the application
//--  in the TCP Rx buffer.
//---------------------------------------------------------
class TcpAppNotif {
  public:
    SessionId          sessionID;
    TcpSegLen          tcpSegLen;
    Ip4Addr            ip4SrcAddr;
    TcpPort            tcpSrcPort;
    TcpPort            tcpDstPort;
    TcpState           tcpState;
    TcpAppNotif() {}
    TcpAppNotif(SessionId  sessId,              TcpState  tcpState) :
        sessionID( sessId), tcpSegLen( 0),      ip4SrcAddr(0),
        tcpSrcPort(0),      tcpDstPort(0),      tcpState(tcpState) {}
    TcpAppNotif(SessionId  sessId,  TcpSegLen   segLen,  Ip4Addr    sa,
        TcpPort    sp,      TcpPort    dp) :
        sessionID( sessId), tcpSegLen( segLen), ip4SrcAddr(sa),
        tcpSrcPort(sp),     tcpDstPort(dp),     tcpState(CLOSED) {}
    TcpAppNotif(SessionId  sessId,  TcpSegLen   segLen,  Ip4Addr    sa,
        TcpPort    sp,      TcpPort    dp,      TcpState tcpState) :
        sessionID( sessId), tcpSegLen( segLen), ip4SrcAddr(sa),
        tcpSrcPort(sp),     tcpDstPort(dp),     tcpState(tcpState) {}
};

//---------------------------------------------------------
//-- TCP APP - DATA READ REQUEST
//--  Used by the application to request data from the
//--  TCP Rx buffer.
//---------------------------------------------------------
class TcpAppRdReq {
  public:
    SessionId   sessionID;
    TcpSegLen   length;
    TcpAppRdReq() {}
    TcpAppRdReq(SessionId id,  TcpSegLen len) :
        sessionID(id), length(len) {}
};

//---------------------------------------------------------
//-- TCP APP - DATA WRITE STATUS
//--  Status returned by NTS after a data send transfer.
//---------------------------------------------------------
class TcpAppWrSts {
public:
    TcpSegLen    segLen;  // The #bytes written or an error code if status==0
    StsBit       status;  // OK=1
    TcpAppWrSts() {}
    TcpAppWrSts(StsBit sts, TcpSegLen len) :
        status(sts), segLen(len) {}
};

//=========================================================
//== TAIF / OPEN & CLOSE CONNECTION INTERFACES
//=========================================================

//--------------------------------------------------------
//-- TCP APP - OPEN CONNECTION REQUEST
//--  The socket address to be opened.
//--------------------------------------------------------
typedef SockAddr        TcpAppOpnReq;

//--------------------------------------------------------
//-- TCP APP - OPEN CONNECTION REPLY
//--  Reports the state of a TCP connection according to RFC-793.
//--------------------------------------------------------
class TcpAppOpnRep {
  public:
    SessionId   sessId;
    TcpState    tcpState;
    TcpAppOpnRep() {}
    TcpAppOpnRep(SessionId sessId, TcpState tcpState) :
        sessId(sessId), tcpState(tcpState) {}
};

//--------------------------------------------------------
//-- TCP APP - CLOSE CONNECTION REQUEST
//--  The socket address to be closed.
//--  [FIXME-What about creating a class 'AppConReq' with a member 'opn/cls']
//--------------------------------------------------------
typedef SessionId       TcpAppClsReq;

//=========================================================
//== TAIF / LISTEN PORT INTERFACES
//=========================================================

//---------------------------------------------------------
//-- TCP APP - LISTEN REQUEST
//--  The TCP port to open for listening.
//--  [FIXME-What about creating a class 'AppLsnReq' with a member 'start/stop']
//---------------------------------------------------------
typedef TcpPort     TcpAppLsnReq;

//---------------------------------------------------------
//-- TCP APP - LISTEN REPLY
//--  The port status returned by NTS upon listen request.
//---------------------------------------------------------
typedef StsBool     TcpAppLsnRep;


/*******************************************************************************
 * INTERFACE - UDP APPLICATION INTERFACE (UAIF)
 *******************************************************************************
 * This section defines the interfaces between the Network and Transport Stack
 * (NTS) and the UDP Application Interface (UAIF) layer.
 *******************************************************************************/
//-- [FIXME - TODO]

//-- UAIF / Control Interfaces
typedef UdpPort     UdpAppLsnReq;
typedef StsBool     UdpAppLsnRep;
typedef UdpPort     UdpAppClsReq; // [FIXME-What about creating a class 'AppLsnReq' with a member 'start/stop']
typedef StsBool     UdpAppClsRep;
//-- UAIF / Datagram Interfaces
typedef AxisApp     UdpAppData;
typedef SocketPair  UdpAppMeta;
typedef UdpLen      UdpAppDLen;


/*******************************************************************************
 *
 * ENTITY - NETWORK TRANSPORT STACK (NTS)
 *
 *******************************************************************************/
void nts(

        //------------------------------------------------------
        //-- TAIF / Received Segment Interfaces
        //------------------------------------------------------
        stream<TcpAppNotif>     &soTAIF_Notif,
        stream<TcpAppRdReq>     &siTAIF_DReq,
        stream<TcpAppData>      &soTAIF_Data,
        stream<TcpAppMeta>      &soTAIF_Meta,

        //------------------------------------------------------
        //-- TAIF / Listen Port Interfaces
        //------------------------------------------------------
        stream<TcpAppLsnReq>    &siTAIF_LsnReq,
        stream<TcpAppLsnRep>    &soTAIF_LsnRep,

        //------------------------------------------------------
        //-- TAIF / Transmit Segment Interfaces
        //------------------------------------------------------
        stream<TcpAppData>      &siTAIF_Data,
        stream<TcpAppMeta>      &siTAIF_Meta,
        stream<TcpAppWrSts>     &soTAIF_DSts,

        //------------------------------------------------------
        //-- TAIF / Open Connection Interfaces
        //------------------------------------------------------
        stream<TcpAppOpnReq>    &siTAIF_OpnReq,
        stream<TcpAppOpnRep>    &soTAIF_OpnRep,

        //------------------------------------------------------
        //-- TAIF / Close Connection Interfaces
        //------------------------------------------------------
        stream<TcpAppClsReq>    &siTAIF_ClsReq,
        //-- Not USed           &soTAIF_ClsSts,

        //------------------------------------------------------
        //-- UAIF / Control Port Interfaces
        //------------------------------------------------------
        stream<UdpAppLsnReq>    &siUAIF_LsnReq,
        stream<UdpAppLsnRep>    &soUAIF_LsnRep,
        stream<UdpAppClsReq>    &siUAIF_ClsReq,

        //------------------------------------------------------
        //-- UAIF / Received Datagram Interfaces
        //------------------------------------------------------
        stream<UdpAppData>      &soUAIF_Data,
        stream<UdpAppMeta>      &soUAIF_Meta,

        //------------------------------------------------------
        //-- UAIF / Transmit Datatagram Interfaces
        //------------------------------------------------------
        stream<UdpAppData>      &siUAIF_Data,
        stream<UdpAppMeta>      &siUAIF_Meta,
        stream<UdpAppDLen>      &siUAIF_DLen

);

#endif

/*! \} */


