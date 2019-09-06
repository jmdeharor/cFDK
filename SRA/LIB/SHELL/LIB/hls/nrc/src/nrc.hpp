//  *
//  *                       cloudFPGA
//  *     Copyright IBM Research, All Rights Reserved
//  *    =============================================
//  *     Created: Apr 2019
//  *     Authors: FAB, WEI, NGL
//  *
//  *     Description:
//  *        The Network Routing Core manages the NTS Stack and maps the network streams to the 
//  *        user's ROLE or the FMC.
//  *

#ifndef _NRC_H_
#define _NRC_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>

/* AXI4Lite Register MAP 
 * =============================
 * piSMC_NRC_ctrlLink_AXI
 * 0x0000 : Control signals
 *          bit 0  - ap_start (Read/Write/COH)
 *          bit 1  - ap_done (Read/COR)
 *          bit 2  - ap_idle (Read)
 *          bit 3  - ap_ready (Read)
 *          bit 7  - auto_restart (Read/Write)
 *          others - reserved
 * 0x0004 : Global Interrupt Enable Register
 *          bit 0  - Global Interrupt Enable (Read/Write)
 *          others - reserved
 * 0x0008 : IP Interrupt Enable Register (Read/Write)
 *          bit 0  - Channel 0 (ap_done)
 *          bit 1  - Channel 1 (ap_ready)
 *          others - reserved
 * 0x000c : IP Interrupt Status Register (Read/TOW)
 *          bit 0  - Channel 0 (ap_done)
 *          bit 1  - Channel 1 (ap_ready)
 *          others - reserved
 * 0x2000 ~
 * 0x3fff : Memory 'ctrlLink_V' (1044 * 32b)
 *          Word n : bit [31:0] - ctrlLink_V[n]
 * (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)
 * 
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_AP_CTRL         0x0000
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_GIE             0x0004
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_IER             0x0008
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_ISR             0x000c
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_CTRLLINK_V_BASE 0x2000
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_CTRLLINK_V_HIGH 0x3fff
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_WIDTH_CTRLLINK_V     32
 * #define XNRC_PISMC_NRC_CTRLLINK_AXI_DEPTH_CTRLLINK_V     1044
 *
 *
 *
 * HLS DEFINITONS
 * ==============================
 */

#define NRC_AXI_CTRL_REGISTER 0
#define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_CTRLLINK_V_BASE 0x2000
#define XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_CTRLLINK_V_HIGH 0x3fff

#define NRC_CTRL_LINK_SIZE (XNRC_PISMC_NRC_CTRLLINK_AXI_ADDR_CTRLLINK_V_HIGH/4)
#define NRC_CTRL_LINK_CONFIG_START_ADDR (0x2000/4)
#define NRC_CTRL_LINK_CONFIG_END_ADDR (0x203F/4)
#define NRC_CTRL_LINK_STATUS_START_ADDR (0x2040/4)
#define NRC_CTRL_LINK_STATUS_END_ADDR (0x207F/4)
#define NRC_CTRL_LINK_MRT_START_ADDR (0x2080/4)
#define NRC_CTRL_LINK_MRT_END_ADDR (0x307F/4)

//HLS DEFINITONS END

#include "../../fmc/src/fmc.hpp"

#include "../../../../../hls/network.hpp"
#include "../../network_utils.hpp"
#include "../../memory_utils.hpp"
#include "../../simulation_utils.hpp"


using namespace hls;

#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718

//enum FsmState {FSM_RESET=0, FSM_IDLE, FSM_W8FORPORT, FSM_FIRST_ACC, FSM_ACC} fsmState;
//static enum FsmState {FSM_RST=0, FSM_ACC, FSM_LAST_ACC} fsmState;
//static enum FsmState {FSM_W8FORMETA=0, FSM_FIRST_ACC, FSM_ACC} fsmState;
#define FSM_RESET 0
#define FSM_IDLE  1
#define FSM_W8FORPORT 2
#define FSM_FIRST_ACC 3
#define FSM_ACC 4
#define FSM_LAST_ACC 5
#define FSM_W8FORMETA 6
#define FSM_WRITE_META 7
#define FSM_DROP_PACKET 8
#define FsmStateUdp uint8_t


//#define WRITE_IDLE 0
//#define WRITE_START 1
//#define WRITE_DATA 2
//#define WRITE_ERROR 3
//#define WRITE_STANDBY 4
//#define sendState uint8_t 
//
//
//#define READ_IDLE 0
//#define READ_DATA 2
//#define READ_ERROR 3
//#define READ_STANDBY 4
//#define receiveState uint8_t


#define OpnFsmStates uint8_t
#define OPN_IDLE 0
#define OPN_REQ 1
#define OPN_REP 2
#define OPN_DONE 3

#define LsnFsmStates uint8_t
#define LSN_IDLE 0
#define LSN_SEND_REQ 1
#define LSN_WAIT_ACK 2
#define LSN_DONE 3

#define RrhFsmStates uint8_t
#define RRH_WAIT_NOTIF 0
#define RRH_SEND_DREQ 1

#define RdpFsmStates uint8_t
#define RDP_WAIT_META 0
#define RDP_STREAM_ROLE 1
#define RDP_STREAM_FMC 2
#define RDP_WRITE_META_ROLE 3
#define RDP_WRITE_META_FMC 4
#define RDP_DROP_PACKET 5

#define WrpFsmStates uint8_t
#define WRP_WAIT_META 0
#define WRP_STREAM_FMC 1
#define WRP_WAIT_CONNECTION 2
#define WRP_STREAM_ROLE 3
#define WRP_DROP_PACKET 4

#define MAX_NRC_SESSIONS 32

#define MAX_MRT_SIZE 1024
#define NUMBER_CONFIG_WORDS 16
#define NUMBER_STATUS_WORDS 16
#define NRC_NUMBER_CONFIG_WORDS NUMBER_CONFIG_WORDS
#define NRC_NUMBER_STATUS_WORDS NUMBER_STATUS_WORDS 
#define NRC_READ_TIMEOUT 160000000 //is a little more than one second with 156Mhz 

 /*
  * ctrlLINK Structure:
  * 1.         0 --            NUMBER_CONFIG_WORDS -1 :  possible configuration from SMC to NRC
  * 2. NUMBER_CONFIG_WORDS --  NUMBER_STATUS_WORDS -1 :  possible status from NRC to SMC
  * 3. NUMBER_STATUS_WORDS --  MAX_MRT_SIZE +
  *                              NUMBER_CONFIG_WORDS +
  *                              NUMBER_STATUS_WORDS    : Message Routing Table (MRT) 
  *
  *
  * CONFIG[0] = own rank 
  *
  * STATUS[5] = WRITE_ERROR_CNT
  */
#define NRC_CONFIG_OWN_RANK 0

//#define NRC_STATUS_WRITE_ERROR_CNT 4
//#define NRC_STATUS_UNUSED_1 4
#define NRC_UNAUTHORIZED_ACCESS 4
//#define NRC_STATUS_READ_ERROR_CNT 5
#define NRC_AUTHORIZED_ACCESS 5
//#define NRC_STATUS_UNUSED_2 5
#define NRC_STATUS_SEND_STATE 6
#define NRC_STATUS_RECEIVE_STATE 7
#define NRC_STATUS_GLOBAL_STATE 8
#define NRC_STATUS_LAST_RX_NODE_ID 9
#define NRC_STATUS_RX_NODEID_ERROR 10
#define NRC_STATUS_LAST_TX_NODE_ID 11
#define NRC_STATUS_TX_NODEID_ERROR 12
#define NRC_STATUS_TX_PORT_CORRECTIONS 13
#define NRC_STATUS_PACKET_CNT_RX 14
#define NRC_STATUS_PACKET_CNT_TX 15



void nrc_main(
    // ----- link to FMC -----
    ap_uint<32> ctrlLink[MAX_MRT_SIZE + NUMBER_CONFIG_WORDS + NUMBER_STATUS_WORDS],
    // ready signal from NTS
    ap_uint<1>  *piNTS_ready,
    // ----- link to MMIO ----
    ap_uint<16> *piMMIO_FmcLsnPort,
    ap_uint<32> *piMMIO_CfrmIp4Addr,
    // -- my IP address 
    ap_uint<32>                 *myIpAddress,

    //-- ROLE UDP connection
    ap_uint<32>                 *pi_udp_rx_ports,
    stream<UdpWord>             &siUdp_data,
    stream<UdpWord>             &soUdp_data,
    stream<NetworkMetaStream>   &siUdp_meta,
    stream<NetworkMetaStream>   &soUdp_meta,
    
    // -- ROLE TCP connection
    ap_uint<32>                 *pi_tcp_rx_ports,
    stream<TcpWord>             &siTcp_data,
    stream<NetworkMetaStream>   &siTcp_meta,
    stream<TcpWord>             &soTcp_data,
    stream<NetworkMetaStream>   &soTcp_meta,

    // -- FMC TCP connection
    stream<TcpWord>             &siFMC_Tcp_data,
    stream<Axis<16> >           &siFMC_Tcp_SessId,
    stream<TcpWord>             &soFMC_Tcp_data,
    stream<Axis<16> >           &soFMC_Tcp_SessId,

    //-- UDMX / This / Open-Port Interfaces
    stream<AxisAck>     &siUDMX_This_OpnAck,
    stream<UdpPort>     &soTHIS_Udmx_OpnReq,

    //-- UDMX / This / Data & MetaData Interfaces
    stream<UdpWord>     &siUDMX_This_Data,
    stream<UdpMeta>     &siUDMX_This_Meta,
    stream<UdpWord>     &soTHIS_Udmx_Data,
    stream<UdpMeta>     &soTHIS_Udmx_Meta,
    stream<UdpPLen>     &soTHIS_Udmx_PLen, 

    //-- TOE / Rx Data Interfaces
    stream<AppNotif>    &siTOE_Notif,
    stream<AppRdReq>    &soTOE_DReq,
    stream<NetworkWord> &siTOE_Data,
    stream<AppMeta>     &siTOE_SessId,
    //-- TOE / Listen Interfaces
    stream<AppLsnReq>   &soTOE_LsnReq,
    stream<AppLsnAck>   &siTOE_LsnAck,
    //-- TOE / Tx Data Interfaces
    stream<NetworkWord> &soTOE_Data,
    stream<AppMeta>     &soTOE_SessId,
    stream<AppWrSts>    &siTOE_DSts,
    //-- TOE / Open Interfaces
    stream<AppOpnReq>   &soTOE_OpnReq,
    stream<AppOpnSts>   &siTOE_OpnRep,
    //-- TOE / Close Interfaces
    stream<AppClsReq>   &soTOE_ClsReq
);

#endif


