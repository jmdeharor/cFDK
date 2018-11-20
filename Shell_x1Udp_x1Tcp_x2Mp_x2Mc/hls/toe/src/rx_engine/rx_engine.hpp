/*****************************************************************************
 * @file       : rx_engine.hpp
 * @brief      : Rx Engine (RXE) of the TCP Offload Engine (TOE).
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Transport Session (NTS)
 * Language    : Vivado HLS
 *
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2018 - IBM Research - All Rights Reserved.
 *
 *----------------------------------------------------------------------------
 *
 * @details    : Data structures, types and prototypes definitions for the
 *               TCP Rx Engine.
 *
 *****************************************************************************/

#include "../toe.hpp"
#include "../toe_utils.hpp"

using namespace hls;


/********************************************
 * RXe - MetaData Interface
 ********************************************/
struct rxEngineMetaData
{
    ap_uint<32> seqNumb;
    ap_uint<32> ackNumb;
    ap_uint<16> winSize;
    ap_uint<16> length;     // Segment Length
    ap_uint<1>  ack;
    ap_uint<1>  rst;
    ap_uint<1>  syn;
    ap_uint<1>  fin;
    //ap_uint<16> dstPort;
};

/********************************************
 * RXe - FsmMetaData Interface
 ********************************************/
struct rxFsmMetaData
{
    ap_uint<16>         sessionID;
    ap_uint<32>         srcIpAddress;
    ap_uint<16>         dstIpPort;
    rxEngineMetaData    meta; //check if all needed
    rxFsmMetaData() {}
    rxFsmMetaData(ap_uint<16> id, ap_uint<32> ipAddr, ap_uint<16> ipPort, rxEngineMetaData meta) :
        sessionID(id), srcIpAddress(ipAddr), dstIpPort(ipPort), meta(meta) {}
};


/*****************************************************************************
 * @brief   Main process of the TCP Rx Engine (RXe).
 *
 * @ingroup rx_engine
 *****************************************************************************/
void rx_engine(
		stream<Ip4Word>						&siIPRX_Pkt,
		stream<sessionLookupReply>			&siSLc_LookupRsp,
		stream<sessionState>				&stateTable2rxEng_upd_rsp,
		stream<bool>						&siPRt_PortCheckRsp,
		stream<rxSarEntry>					&rxSar2rxEng_upd_rsp,
		stream<rxTxSarReply>				&txSar2rxEng_upd_rsp,
		stream<mmStatus>					&rxBufferWriteStatus,
		stream<axiWord>						&rxBufferWriteData,
		stream<stateQuery>					&rxEng2stateTable_upd_req,
		stream<TcpPort>						&soQryPortState,
		stream<sessionLookupQuery>			&soLookupReq,
		stream<rxSarRecvd>					&rxEng2rxSar_upd_req,
		stream<rxTxSarQuery>				&rxEng2txSar_upd_req,
		stream<rxRetransmitTimerUpdate>		&rxEng2timer_clearRetransmitTimer,
		stream<ap_uint<16> >				&rxEng2timer_clearProbeTimer,
		stream<ap_uint<16> >				&rxEng2timer_setCloseTimer,
		stream<openStatus>					&openConStatusOut,
		stream<extendedEvent>				&rxEng2eventEng_setEvent,
		stream<mmCmd>						&rxBufferWriteCmd,
		stream<appNotification>				&rxEng2rxApp_notification
);
