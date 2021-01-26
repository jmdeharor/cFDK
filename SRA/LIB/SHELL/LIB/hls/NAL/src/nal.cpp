/*******************************************************************************
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
 *******************************************************************************/

/*****************************************************************************
 * @file       : nal.hpp
 * @brief      : The cloudFPGA Network Abstraction Layer (NAL) between NTS and ROlE.
 *               The NAL core manages the NTS Stack and maps the network streams to 
 *               the user's ROLE or the FMC while hiding some complexity of UOE and 
 *               TOE.
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Abstraction Layer (NAL)
 * Language    : Vivado HLS
 * 
 * \ingroup NAL
 * \addtogroup NAL
 * \{
 *****************************************************************************/

#include "nal.hpp"

////returns the ZERO-based bit position (so 0 for LSB)
//ap_uint<32> getRightmostBitPos(ap_uint<32> num)
//{
//#pragma HLS INLINE
//  ap_uint<32> pos = 0;
//  for (int i = 0; i < 32; i++)
//  {
//#pragma HLS unroll
////    if(!(num & (1 << i)))
////    {
////      pos++;
////    }
////    else {
////      break;
////    }
//    if(num & (1 << i))
//    {
//      pos = i;
//      break;
//    }
//  }
//  return pos;
//}

//#include "hls_math.h"
//ap_uint<32> getRightmostBitPos(ap_uint<32> num)
//{
//#pragma HLS INLINE
//  return (ap_uint<32>) ((half) hls::log2((half)(num & -num)));
//}

//returns the ZERO-based bit position (so 0 for LSB)
ap_uint<32> getRightmostBitPos(ap_uint<32> num)
{
#pragma HLS INLINE off
  ap_uint<32> one_hot = num & -num;
  ap_uint<32> pos = 0;
  for (int i = 0; i < 32; i++)
  {
#pragma HLS unroll
    if(one_hot <= 1)
    {
      break;
    }
    pos++;
    one_hot >>= 1;
  }
  //printf("[NAL:RigthmostBit] returning %d for input %d\n", (int) pos, (int) num);
  return pos;
}

NalTriple newTriple(Ip4Addr ipRemoteAddres, TcpPort tcpRemotePort, TcpPort tcpLocalPort)
{
#pragma HLS INLINE
  NalTriple new_entry = (((ap_uint<64>) ipRemoteAddres) << 32) | (((ap_uint<32>) tcpRemotePort) << 16) | tcpLocalPort;
  return new_entry;
}


Ip4Addr getRemoteIpAddrFromTriple(NalTriple triple)
{
#pragma HLS INLINE
  ap_uint<32> ret = ((ap_uint<32>) (triple >> 32)) & 0xFFFFFFFF;
  return (Ip4Addr) ret;
}


TcpPort getRemotePortFromTriple(NalTriple triple)
{
#pragma HLS INLINE
  ap_uint<16> ret = ((ap_uint<16>) (triple >> 16)) & 0xFFFF;
  return (TcpPort) ret;
}


TcpPort getLocalPortFromTriple(NalTriple triple)
{
#pragma HLS INLINE
  ap_uint<16> ret = (ap_uint<16>) (triple & 0xFFFF);
  return (TcpPort) ret;
}


uint8_t extractByteCnt(Axis<64> currWord)
{
#pragma HLS INLINE

  uint8_t ret = 0;

  switch (currWord.getTKeep()) {
    case 0b11111111:
      ret = 8;
      break;
    case 0b01111111:
      ret = 7;
      break;
    case 0b00111111:
      ret = 6;
      break;
    case 0b00011111:
      ret = 5;
      break;
    case 0b00001111:
      ret = 4;
      break;
    case 0b00000111:
      ret = 3;
      break;
    case 0b00000011:
      ret = 2;
      break;
    default:
    case 0b00000001:
      ret = 1;
      break;
  }
  return ret;
}

void pStatusMemory(
    stream<NalEventNotif>  &internal_event_fifo,
    ap_uint<1>       *layer_7_enabled,
    ap_uint<1>       *role_decoupled,
    //const NodeId       *own_rank,
    stream<NalConfigUpdate>   &sConfigUpdate,
    //const ap_uint<32>      *mrt_version_processed,
    stream<uint32_t>      &mrt_version_update,
    //const ap_uint<32>    *udp_rx_ports_processed,
    //const ap_uint<32>    *tcp_rx_ports_processed,
    //const ap_uint<16>    *processed_FMC_listen_port,
    stream<NalPortUpdate>     &sNalPortUpdate,
    stream<NalStatusUpdate>   &sStatusUpdate

    )
{  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
  //#pragma HLS INLINE //yes, inline it!
#pragma HLS INLINE off
#pragma HLS pipeline II=1
  //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
  static ap_uint<32> node_id_missmatch_RX_cnt = 0;
  static NodeId last_rx_node_id = 0;
  static NrcPort last_rx_port = 0;
  static ap_uint<32> node_id_missmatch_TX_cnt = 0;
  static NodeId last_tx_node_id = 0;
  static NrcPort last_tx_port = 0;
  static ap_uint<16> port_corrections_TX_cnt = 0;
  static ap_uint<32> unauthorized_access_cnt = 0;
  static ap_uint<32> authorized_access_cnt = 0;
  static ap_uint<32> fmc_tcp_bytes_cnt = 0;

  static ap_uint<32> packet_count_RX = 0;
  static ap_uint<32> packet_count_TX = 0;
  static NodeId own_rank = 0;

  static ap_uint<16> tcp_new_connection_failure_cnt = 0;


  static bool tables_initialized = false;
  static uint8_t status_update_i = 0;

#pragma HLS reset variable=node_id_missmatch_RX_cnt
#pragma HLS reset variable=node_id_missmatch_TX_cnt
#pragma HLS reset variable=port_corrections_TX_cnt
#pragma HLS reset variable=packet_count_RX
#pragma HLS reset variable=packet_count_TX
#pragma HLS reset variable=last_rx_node_id
#pragma HLS reset variable=last_rx_port
#pragma HLS reset variable=last_tx_node_id
#pragma HLS reset variable=last_tx_port
#pragma HLS reset variable=unauthorized_access_cnt
#pragma HLS reset variable=authorized_access_cnt
#pragma HLS reset variable=fmc_tcp_bytes_cnt
#pragma HLS reset variable=tcp_new_connection_failure_cnt
#pragma HLS reset variable=tables_initialized
#pragma HLS reset variable=status_update_i
#pragma HLS reset variable=own_rank

  //-- STATIC DATAFLOW VARIABLES --------------------------------------------

  static ap_uint<32> status[NUMBER_STATUS_WORDS];
  static ap_uint<32> old_status[NUMBER_STATUS_WORDS];

#pragma HLS ARRAY_PARTITION variable=status complete dim=1
#pragma HLS ARRAY_PARTITION variable=old_status complete dim=1

  // ----- tables init -----
  if(!tables_initialized)
  {
    for(int i = 0; i < NUMBER_STATUS_WORDS; i++)
    {
      status[i] = 0x0;
      old_status[i] = 0x0;
    }
    tables_initialized = true;
  } else if(*layer_7_enabled == 0 || *role_decoupled == 1 )
  {
    //reset counters
    packet_count_TX = 0x0;
    packet_count_RX = 0x0;
    last_rx_port = 0x0;
    last_rx_node_id = 0x0;
    last_tx_port = 0x0;
    last_tx_node_id = 0x0;
    //return;
  } else if(!sConfigUpdate.empty())
  {
    NalConfigUpdate ca = sConfigUpdate.read();
    if(ca.config_addr == NAL_CONFIG_OWN_RANK)
    {
      own_rank = ca.update_value;
      status[NAL_STATUS_OWN_RANK] = (ap_uint<32>) own_rank;
    }
  } else if(!mrt_version_update.empty())
  {
    status[NAL_STATUS_MRT_VERSION] = mrt_version_update.read();
  }
  else if(!sNalPortUpdate.empty())
  {
    NalPortUpdate update = sNalPortUpdate.read();
    switch(update.port_type)
    {
      case FMC:
        status[NAL_STATUS_FMC_PORT_PROCESSED] = update.new_value;
        break;
      case UDP:
        status[NAL_STATUS_OPEN_UDP_PORTS] = update.new_value;
        break;
      case TCP:
        status[NAL_STATUS_OPEN_TCP_PORTS] = update.new_value;
        break;
    }
  }
  else if(!internal_event_fifo.empty())
  {
    NalEventNotif nevs = internal_event_fifo.read();
    //printf("[DEBUG] Process event %d with update value %d\n", \
    //    (int) nevs.type, (int) nevs.update_value);

    switch(nevs.type)
    {
      case NID_MISS_RX:
        node_id_missmatch_RX_cnt += nevs.update_value;
        break;
      case NID_MISS_TX:
        node_id_missmatch_TX_cnt += nevs.update_value;
        break;
      case PCOR_TX:
        port_corrections_TX_cnt += (ap_uint<16>) nevs.update_value;
        break;
      case TCP_CON_FAIL:
        tcp_new_connection_failure_cnt += (ap_uint<16>) nevs.update_value;
        break;
      case LAST_RX_PORT:
        last_rx_port = (ap_uint<16>) nevs.update_value;
        break;
      case LAST_RX_NID:
        last_rx_node_id = (NodeId) nevs.update_value;
        break;
      case LAST_TX_PORT:
        last_tx_port = (ap_uint<16>) nevs.update_value;
        break;
      case LAST_TX_NID:
        last_tx_node_id = (NodeId)  nevs.update_value;
        break;
      case PACKET_RX:
        packet_count_RX += nevs.update_value;
        break;
      case PACKET_TX:
        packet_count_TX += nevs.update_value;
        break;
      case UNAUTH_ACCESS:
        unauthorized_access_cnt += nevs.update_value;
        break;
      case AUTH_ACCESS:
        authorized_access_cnt += nevs.update_value;
        break;
      case FMC_TCP_BYTES:
        fmc_tcp_bytes_cnt += nevs.update_value;
        break;
      default:
        printf("[ERROR] Internal Event Processing received invalid event %d with update value %d\n", \
            (int) nevs.type, (int) nevs.update_value);
        break;
    }

  } else {

    //update status entries
    //status[NAL_STATUS_MRT_VERSION] = *mrt_version_processed;
    //status[NAL_STATUS_OPEN_UDP_PORTS] = *udp_rx_ports_processed;
    //status[NAL_STATUS_OPEN_TCP_PORTS] = *tcp_rx_ports_processed;
    //status[NAL_STATUS_FMC_PORT_PROCESSED] = (ap_uint<32>) *processed_FMC_listen_port;
    //status[NAL_STATUS_OWN_RANK] = (ap_uint<32>) *own_rank;

    //udp
    //status[NAL_STATUS_SEND_STATE] = (ap_uint<32>) fsmStateRX_Udp;
    //status[NAL_STATUS_RECEIVE_STATE] = (ap_uint<32>) fsmStateTXenq_Udp;
    //status[NAL_STATUS_GLOBAL_STATE] = (ap_uint<32>) fsmStateTXdeq_Udp;

    //tcp
    //status[NAL_STATUS_SEND_STATE] = (ap_uint<32>) wrpFsmState;
    //status[NAL_STATUS_RECEIVE_STATE] = (ap_uint<32>) rdpFsmState;
    //status[NAL_STATUS_GLOBAL_STATE] = (ap_uint<32>) opnFsmState;

    status[NAL_STATUS_GLOBAL_STATE] = fmc_tcp_bytes_cnt;

    //status[NAL_STATUS_RX_NODEID_ERROR] = (ap_uint<32>) node_id_missmatch_RX_cnt;
    status[NAL_STATUS_RX_NODEID_ERROR] = (((ap_uint<32>) port_corrections_TX_cnt) << 16) | ( 0xFFFF & ((ap_uint<16>) node_id_missmatch_RX_cnt));
    status[NAL_STATUS_LAST_RX_NODE_ID] = (ap_uint<32>) (( (ap_uint<32>) last_rx_port) << 16) | ( (ap_uint<32>) last_rx_node_id);
    //status[NAL_STATUS_TX_NODEID_ERROR] = (ap_uint<32>) node_id_missmatch_TX_cnt;
    status[NAL_STATUS_TX_NODEID_ERROR] = (((ap_uint<32>) tcp_new_connection_failure_cnt) << 16) | ( 0xFFFF & ((ap_uint<16>) node_id_missmatch_TX_cnt));
    status[NAL_STATUS_LAST_TX_NODE_ID] = (ap_uint<32>) (((ap_uint<32>) last_tx_port) << 16) | ((ap_uint<32>) last_tx_node_id);
    //status[NAL_STATUS_TX_PORT_CORRECTIONS] = (((ap_uint<32>) tcp_new_connection_failure_cnt) << 16) | ((ap_uint<16>) port_corrections_TX_cnt);
    status[NAL_STATUS_PACKET_CNT_RX] = (ap_uint<32>) packet_count_RX;
    status[NAL_STATUS_PACKET_CNT_TX] = (ap_uint<32>) packet_count_TX;

    status[NAL_UNAUTHORIZED_ACCESS] = (ap_uint<32>) unauthorized_access_cnt;
    status[NAL_AUTHORIZED_ACCESS] = (ap_uint<32>) authorized_access_cnt;


    //check for differences
    if(!sStatusUpdate.full())
    {
      if(old_status[status_update_i] != status[status_update_i])
      {
        ap_uint<32> uv = status[status_update_i];
        NalStatusUpdate su = NalStatusUpdate(status_update_i, uv);
        sStatusUpdate.write(su);
        old_status[status_update_i] = status[status_update_i];
        //printf("[INFO] Internal Event Processing detected status change on address %d with new value %d\n", \
        //       (int) su.status_addr, (int) su.new_value);
        //}
        //one at a time is enough
    }
    status_update_i++;
    if(status_update_i >= NUMBER_STATUS_WORDS)
    {
      status_update_i = 0;
    }
  }
} // else

}

void eventFifoMerge(
    //const ap_uint<1>          *layer_4_enabled,
    //const ap_uint<1>       *layer_7_enabled,
    //const ap_uint<1>       *role_decoupled,
    //const ap_uint<32>      *mrt_version_processed,
    //const ap_uint<32>    *udp_rx_ports_processed,
    //const ap_uint<32>    *tcp_rx_ports_processed,
    //const ap_uint<16>    *processed_FMC_listen_port,
    //stream<NalConfigUpdate>   &sConfigUpdate,
    stream<NalEventNotif>  &internal_event_fifo_0,
    stream<NalEventNotif>  &internal_event_fifo_1,
    stream<NalEventNotif>  &internal_event_fifo_2,
    stream<NalEventNotif>  &internal_event_fifo_3,
    //stream<NalStatusUpdate>   &sStatusUpdate
    stream<NalEventNotif>  &merged_fifo
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
#pragma HLS pipeline II=1
  //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
  //-- STATIC DATAFLOW VARIABLES --------------------------------------------
  //-- LOCAL DATAFLOW VARIABLES ---------------------------------------------

  //if else tree to lower latency, the event fifos should have enough buffer...

      if(!internal_event_fifo_0.empty() && !merged_fifo.full())
      {
        NalEventNotif tmp = internal_event_fifo_0.read();
        printf("[INFO] Internal Event Processing received event %d with update value %d from fifo_0\n", \
            (int) tmp.type, (int) tmp.update_value);
        merged_fifo.write(tmp);
      }
      else if(!internal_event_fifo_1.empty() && !merged_fifo.full() )
      {
        NalEventNotif tmp = internal_event_fifo_1.read();
        printf("[INFO] Internal Event Processing received event %d with update value %d from fifo_1\n", \
            (int) tmp.type, (int) tmp.update_value);
        merged_fifo.write(tmp);
      }
      else if(!internal_event_fifo_2.empty()  && !merged_fifo.full())
      {
        NalEventNotif tmp = internal_event_fifo_2.read();
        printf("[INFO] Internal Event Processing received event %d with update value %d from fifo_2\n", \
            (int) tmp.type, (int) tmp.update_value);
        merged_fifo.write(tmp);
      }
      else if(!internal_event_fifo_3.empty() && !merged_fifo.full())
      {
        NalEventNotif tmp = internal_event_fifo_3.read();
        printf("[INFO] Internal Event Processing received event %d with update value %d from fifo_3\n", \
            (int) tmp.type, (int) tmp.update_value);
        merged_fifo.write(tmp);
      }
}


/*****************************************************************************
 * @brief   Main process of the UDP Role Interface
 *
 *****************************************************************************/
void nal_main(
    // ----- link to FMC -----
    ap_uint<32> ctrlLink[MAX_MRT_SIZE + NUMBER_CONFIG_WORDS + NUMBER_STATUS_WORDS],
    //state of the FPGA
    ap_uint<1> *layer_4_enabled,
    ap_uint<1> *layer_7_enabled,
    ap_uint<1> *role_decoupled,
    // ready signal from NTS
    ap_uint<1>  *piNTS_ready,
    // ----- link to MMIO ----
    ap_uint<16> *piMMIO_FmcLsnPort,
    ap_uint<32> *piMMIO_CfrmIp4Addr,
    // -- my IP address 
    ap_uint<32>                 *myIpAddress,

    //-- ROLE UDP connection
    ap_uint<32>                 *pi_udp_rx_ports,
    stream<NetworkWord>             &siUdp_data,
    stream<NetworkWord>             &soUdp_data,
    stream<NetworkMetaStream>   &siUdp_meta,
    stream<NetworkMetaStream>   &soUdp_meta,

    // -- ROLE TCP connection
    ap_uint<32>                 *pi_tcp_rx_ports,
    stream<NetworkWord>          &siTcp_data,
    stream<NetworkMetaStream>   &siTcp_meta,
    stream<NetworkWord>          &soTcp_data,
    stream<NetworkMetaStream>   &soTcp_meta,

    // -- FMC TCP connection
    stream<TcpAppData>          &siFMC_Tcp_data,
    stream<TcpAppMeta>          &siFMC_Tcp_SessId,
    ap_uint<1>                  *piFMC_Tcp_data_FIFO_prog_full,
    stream<TcpAppData>          &soFMC_Tcp_data,
    ap_uint<1>                  *piFMC_Tcp_sessid_FIFO_prog_full,
    stream<TcpAppMeta>          &soFMC_Tcp_SessId,

    //-- UOE / Control Port Interfaces
    stream<UdpPort>             &soUOE_LsnReq,
    stream<StsBool>             &siUOE_LsnRep,
    stream<UdpPort>             &soUOE_ClsReq,
    stream<StsBool>             &siUOE_ClsRep,

    //-- UOE / Rx Data Interfaces
    stream<UdpAppData>          &siUOE_Data,
    stream<UdpAppMeta>          &siUOE_Meta,

    //-- UOE / Tx Data Interfaces
    stream<UdpAppData>          &soUOE_Data,
    stream<UdpAppMeta>          &soUOE_Meta,
    stream<UdpAppDLen>          &soUOE_DLen,

    //-- TOE / Rx Data Interfaces
    stream<TcpAppNotif>    &siTOE_Notif,
    stream<TcpAppRdReq>    &soTOE_DReq,
    stream<TcpAppData>     &siTOE_Data,
    stream<TcpAppMeta>     &siTOE_SessId,
    //-- TOE / Listen Interfaces
    stream<TcpAppLsnReq>   &soTOE_LsnReq,
    stream<TcpAppLsnRep>   &siTOE_LsnRep,
    //-- TOE / Tx Data Interfaces
    stream<TcpAppData>      &soTOE_Data,
    stream<TcpAppSndReq>    &soTOE_SndReq,
    stream<TcpAppSndRep>    &siTOE_SndRep,
    //stream<AppWrSts>    &siTOE_DSts,
    //-- TOE / Open Interfaces
    stream<TcpAppOpnReq>      &soTOE_OpnReq,
    stream<TcpAppOpnRep>   &siTOE_OpnRep,
    //-- TOE / Close Interfaces
    stream<TcpAppClsReq>   &soTOE_ClsReq
    )
{

  // ----- directives for AXI buses (AXI4 stream, AXI4 Lite) -----
#ifdef USE_DEPRECATED_DIRECTIVES
#pragma HLS RESOURCE core=AXI4LiteS variable=ctrlLink metadata="-bus_bundle piFMC_NAL_ctrlLink_AXI"

#pragma HLS RESOURCE core=AXI4Stream variable=siUdp_data    metadata="-bus_bundle siUdp_data"
#pragma HLS RESOURCE core=AXI4Stream variable=soUdp_data    metadata="-bus_bundle soUdp_data"

#pragma HLS RESOURCE core=AXI4Stream variable=siUdp_meta    metadata="-bus_bundle siUdp_meta"
#pragma HLS DATA_PACK          variable=siUdp_meta
#pragma HLS RESOURCE core=AXI4Stream variable=soUdp_meta    metadata="-bus_bundle soUdp_meta"
#pragma HLS DATA_PACK          variable=soUdp_meta

#pragma HLS RESOURCE core=AXI4Stream variable=soUOE_LsnReq  metadata="-bus_bundle soUOE_LsnReq"
#pragma HLS RESOURCE core=AXI4Stream variable=siUOE_LsnRep  metadata="-bus_bundle siUOE_LsnRep"
#pragma HLS RESOURCE core=AXI4Stream variable=soUOE_ClsReq  metadata="-bus_bundle soUOE_ClsReq"
#pragma HLS RESOURCE core=AXI4Stream variable=siUOE_ClsRep  metadata="-bus_bundle siUOE_ClsRep"

#pragma HLS RESOURCE core=AXI4Stream variable=siUOE_Data    metadata="-bus_bundle siUOE_Data"
#pragma HLS RESOURCE core=AXI4Stream variable=siUOE_Meta    metadata="-bus_bundle siUOE_Meta"
#pragma HLS DATA_PACK                variable=siUOE_Meta

#pragma HLS RESOURCE core=AXI4Stream variable=soUOE_Data    metadata="-bus_bundle soUOE_Data"
#pragma HLS RESOURCE core=AXI4Stream variable=soUOE_Meta    metadata="-bus_bundle soUOE_Meta"
#pragma HLS DATA_PACK                variable=soUOE_Meta
#pragma HLS RESOURCE core=AXI4Stream variable=soUOE_DLen    metadata="-bus_bundle soUOE_DLen"

#pragma HLS RESOURCE core=AXI4Stream variable=siTcp_data    metadata="-bus_bundle siTcp_data"
#pragma HLS RESOURCE core=AXI4Stream variable=soTcp_data    metadata="-bus_bundle soTcp_data"
#pragma HLS RESOURCE core=AXI4Stream variable=siTcp_meta    metadata="-bus_bundle siTcp_meta"
#pragma HLS DATA_PACK          variable=siTcp_meta
#pragma HLS RESOURCE core=AXI4Stream variable=soTcp_meta    metadata="-bus_bundle soTcp_meta"
#pragma HLS DATA_PACK          variable=soTcp_meta

#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_Notif   metadata="-bus_bundle siTOE_Notif"
#pragma HLS DATA_PACK                variable=siTOE_Notif
#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_DReq    metadata="-bus_bundle soTOE_DReq"
#pragma HLS DATA_PACK                variable=soTOE_DReq
#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_Data    metadata="-bus_bundle siTOE_Data"
#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_SessId  metadata="-bus_bundle siTOE_SessId"

#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_LsnReq  metadata="-bus_bundle soTOE_LsnReq"
#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_LsnRep  metadata="-bus_bundle siTOE_LsnRep"

#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_Data    metadata="-bus_bundle soTOE_Data"
#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_SessId  metadata="-bus_bundle soTOE_SessId"
  //#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_DSts  metadata="-bus_bundle "

#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_OpnReq  metadata="-bus_bundle soTOE_OpnReq"
#pragma HLS DATA_PACK                variable=soTOE_OpnReq
#pragma HLS RESOURCE core=AXI4Stream variable=siTOE_OpnRep  metadata="-bus_bundle siTOE_OpnRep"
#pragma HLS DATA_PACK                variable=siTOE_OpnRep

#pragma HLS RESOURCE core=AXI4Stream variable=soTOE_ClsReq  metadata="-bus_bundle soTOE_ClsReq"

#else
#pragma HLS INTERFACE s_axilite depth=512 port=ctrlLink bundle=piFMC_NAL_ctrlLink_AXI

#pragma HLS INTERFACE axis register both port=siUdp_data
#pragma HLS INTERFACE axis register both port=soUdp_data

#pragma HLS INTERFACE axis register both port=siUdp_meta
#pragma HLS INTERFACE axis register both port=soUdp_meta

#pragma HLS INTERFACE axis register both port=soUOE_LsnReq
#pragma HLS INTERFACE axis register both port=siUOE_LsnRep
#pragma HLS INTERFACE axis register both port=soUOE_ClsReq
#pragma HLS INTERFACE axis register both port=siUOE_ClsRep

#pragma HLS INTERFACE axis register both port=siUOE_Data
#pragma HLS INTERFACE axis register both port=siUOE_Meta
#pragma HLS DATA_PACK                variable=siUOE_Meta

#pragma HLS INTERFACE axis register both port=soUOE_Data
#pragma HLS INTERFACE axis register both port=soUOE_Meta
#pragma HLS DATA_PACK                variable=soUOE_Meta
#pragma HLS INTERFACE axis register both port=soUOE_DLen

#pragma HLS INTERFACE axis register both port=siTcp_data
#pragma HLS INTERFACE axis register both port=soTcp_data
#pragma HLS INTERFACE axis register both port=siTcp_meta
#pragma HLS INTERFACE axis register both port=soTcp_meta

#pragma HLS INTERFACE axis register both port=siTOE_Notif
#pragma HLS DATA_PACK                variable=siTOE_Notif
#pragma HLS INTERFACE axis register both port=soTOE_DReq
#pragma HLS DATA_PACK                variable=soTOE_DReq
#pragma HLS INTERFACE axis register both port=siTOE_Data
#pragma HLS INTERFACE axis register both port=siTOE_SessId

#pragma HLS INTERFACE axis register both port=soTOE_LsnReq
#pragma HLS INTERFACE axis register both port=siTOE_LsnRep

#pragma HLS INTERFACE axis register both port=soTOE_Data
#pragma HLS INTERFACE axis register both port=soTOE_SndReq
#pragma HLS DATA_PACK            variable=soTOE_SndReq
#pragma HLS INTERFACe axis register both port=siTOE_SndRep
#pragma HLS DATA_PACK        variable=siTOE_SndRep

#pragma HLS INTERFACE axis register both port=soTOE_OpnReq
#pragma HLS DATA_PACK                variable=soTOE_OpnReq
#pragma HLS INTERFACE axis register both port=siTOE_OpnRep
#pragma HLS DATA_PACK                variable=siTOE_OpnRep

#pragma HLS INTERFACE axis register both port=soTOE_ClsReq

#endif

  // ----- common directives -----

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INTERFACE ap_vld register port=layer_4_enabled name=piLayer4enabled
#pragma HLS INTERFACE ap_vld register port=layer_7_enabled name=piLayer7enabled
#pragma HLS INTERFACE ap_vld register port=role_decoupled  name=piRoleDecoup_active
#pragma HLS INTERFACE ap_vld register port=piNTS_ready name=piNTS_ready

#pragma HLS INTERFACE ap_vld register port=myIpAddress name=piMyIpAddress
#pragma HLS INTERFACE ap_vld register port=pi_udp_rx_ports name=piROL_Udp_Rx_ports
#pragma HLS INTERFACE ap_vld register port=piMMIO_FmcLsnPort name=piMMIO_FmcLsnPort
#pragma HLS INTERFACE ap_vld register port=piMMIO_CfrmIp4Addr name=piMMIO_CfrmIp4Addr

#pragma HLS INTERFACE ap_vld register port=pi_tcp_rx_ports name=piROL_Tcp_Rx_ports

#pragma HLS INTERFACE ap_fifo port=siFMC_Tcp_data
#pragma HLS INTERFACE ap_fifo port=soFMC_Tcp_data
#pragma HLS INTERFACE ap_fifo port=siFMC_Tcp_SessId
#pragma HLS INTERFACE ap_fifo port=soFMC_Tcp_SessId
#pragma HLS INTERFACE ap_vld register port=piFMC_Tcp_data_FIFO_prog_full name=piFMC_Tcp_data_FIFO_prog_full
#pragma HLS INTERFACE ap_vld register port=piFMC_Tcp_sessid_FIFO_prog_full name=piFMC_Tcp_sessid_FIFO_prog_full

  //TODO: add internal streams

#pragma HLS DATAFLOW
  //#pragma HLS PIPELINE II=1 //FIXME

  //=================================================================================================
  // Variable Pragmas

  //#pragma HLS ARRAY_PARTITION variable=tripleList cyclic factor=4 dim=1
  //#pragma HLS ARRAY_PARTITION variable=sessionIdList cyclic factor=4 dim=1
  //#pragma HLS ARRAY_PARTITION variable=usedRows cyclic factor=4 dim=1
  //#pragma HLS ARRAY_PARTITION variable=privilegedRows cyclic factor=4 dim=1
  //#pragma HLS ARRAY_PARTITION variable=rowsToDelete cyclic factor=4 dim=1
  //#pragma HLS ARRAY_PARTITION variable=localMRT complete dim=1

  //#pragma HLS ARRAY_PARTITION variable=status cyclic factor=4 dim=1


  //===========================================================
  //  core wide STATIC variables

  //static ap_uint<32> mrt_version_processed = 0;
  //static ap_uint<32> mrt_version_used = 0; //no reset needed

  //static bool expect_FMC_response = false; //pTcpRDP and pTcpWRp need this
  //static bool start_tcp_cls_fsm = false;

  static stream<NalEventNotif> internal_event_fifo_0 ("internal_event_fifo_0");
  static stream<NalEventNotif> internal_event_fifo_1 ("internal_event_fifo_1");
  static stream<NalEventNotif> internal_event_fifo_2 ("internal_event_fifo_2");
  static stream<NalEventNotif> internal_event_fifo_3 ("internal_event_fifo_3");
  static stream<NalEventNotif> merged_fifo           ("sEvent_Merged_Fifo");
  //static stream<NalConfigUpdate>   sA4lToTcpAgency    ("sA4lToTcpAgency"); //(currently not used)
  static stream<NalConfigUpdate>   sA4lToPortLogic    ("sA4lToPortLogic");
  static stream<NalConfigUpdate>   sA4lToUdpRx        ("sA4lToUdpRx");
  static stream<NalConfigUpdate>   sA4lToTcpRx        ("sA4lToTcpRx");
  static stream<NalConfigUpdate>   sA4lToStatusProc   ("sA4lToStatusProc");
  //static stream<NalMrtUpdate>      sA4lMrtUpdate      ("sA4lMrtUpdate");
  static stream<NalStatusUpdate>   sStatusUpdate    ("sStatusUpdate");
  static stream<NodeId>            sGetIpReq_UdpTx    ("sGetIpReq_UdpTx");
  static stream<Ip4Addr>           sGetIpRep_UdpTx    ("sGetIpRep_UdpTx");
  static stream<NodeId>            sGetIpReq_TcpTx    ("sGetIpReq_TcpTx");
  static stream<Ip4Addr>           sGetIpRep_TcpTx    ("sGetIpRep_TcpTx");
  static stream<Ip4Addr>           sGetNidReq_UdpRx    ("sGetNidReq_UdpRx");
  static stream<NodeId>            sGetNidRep_UdpRx    ("sGetNidRep_UdpRx");
  static stream<Ip4Addr>           sGetNidReq_TcpRx    ("sGetNidReq_TcpRx");
  static stream<NodeId>            sGetNidRep_TcpRx    ("sGetNidRep_TcpRx");
  static stream<Ip4Addr>           sGetNidReq_TcpTx    ("sGetNidReq_TcpTx");
  static stream<NodeId>            sGetNidRep_TcpTx    ("sGetNidRep_TcpTx");

  static stream<UdpPort>      sUdpPortsToClose   ("sUdpPortsToClose");
  static stream<UdpPort>      sUdpPortsToOpen    ("sUdpPortsToOpen");
  static stream<TcpPort>      sTcpPortsToOpen      ("sTcpPortsToOpen");
  static stream<bool>       sUdpPortsOpenFeedback ("sUdpPortsOpenFeedback");
  static stream<bool>       sTcpPortsOpenFeedback ("sTcpPortsOpenFeedback");


  static stream<NetworkWord>          sRoleTcpDataRx_buffer ("sRoleTcpDataRx_buffer");
  static stream<NetworkMetaStream>    sRoleTcpMetaRx_buffer ("sRoleTcpMetaRx_buffer");
  static stream<TcpAppData>           sTcpWrp2Wbu_data  ("sTcpWrp2Wbu_data");
  static stream<TcpAppMeta>           sTcpWrp2Wbu_sessId ("sTcpWrp2Wbu_sessId");
  static stream<TcpDatLen>            sTcpWrp2Wbu_len  ("sTcpWrp2Wbu_len");

  static stream<SessionId>          sGetTripleFromSid_Req    ("sGetTripleFromSid_Req");
  static stream<NalTriple>          sGetTripleFromSid_Rep    ("sGetTripleFromSid_Rep");
  static stream<NalTriple>          sGetSidFromTriple_Req    ("sGetSidFromTriple_Req");
  static stream<SessionId>          sGetSidFromTriple_Rep    ("sGetSidFromTriple_Rep");
  static stream<NalNewTableEntry>   sAddNewTriple_TcpRrh     ("sAddNewTriple_TcpRrh");
  static stream<NalNewTableEntry>   sAddNewTriple_TcpCon     ("sAddNewTriple_TcpCon");
  static stream<SessionId>          sDeleteEntryBySid        ("sDeleteEntryBySid");
  static stream<SessionId>          sMarkAsPriv              ("sMarkAsPriv");
  static stream<bool>               sMarkToDel_unpriv        ("sMarkToDel_unpriv");
  static stream<bool>               sGetNextDelRow_Req       ("sGetNextDelRow_Req");
  static stream<SessionId>          sGetNextDelRow_Rep       ("sGetNextDelRow_Rep");
  static stream<TcpAppRdReq>        sRDp_ReqNotif      ("sRDp_ReqNotif");

  static stream<NalTriple>      sNewTcpCon_Req       ("sNewTcpCon_Req");
  static stream<NalNewTcpConRep>    sNewTcpCon_Rep           ("sNewTcpConRep");
  static stream<TcpAppNotif>       sTcpNotif_buffer   ("sTcpNotif_buffer");

  static ap_uint<32> localMRT[MAX_MRT_SIZE];

  static stream<uint32_t>           sMrtVersionUpdate_0 ("sMrtVersionUpdate_0");
  static stream<uint32_t>           sMrtVersionUpdate_1 ("sMrtVersionUpdate_1");
    
  static stream<bool>      sCacheInvalSig_0      ("sCacheInvalSig_0");
  static stream<bool>      sCacheInvalSig_1      ("sCacheInvalSig_1");
  static stream<bool>      sCacheInvalSig_2      ("sCacheInvalSig_2");
  static stream<bool>      sCacheInvalSig_3      ("sCacheInvalSig_3");

  static stream<bool>      sRoleFifoEmptySig     ("sRoleFifoEmptySig");

  static stream<bool>      sStartTclCls_sig      ("sStartTclCls_sig");
  static stream<NalPortUpdate> sNalPortUpdate    ("sNalPortUpdate");
  
  static stream<NetworkWord>          sRoleUdpDataRx_buffer ("sRoleUdpDataRx_buffer");
  static stream<NetworkMetaStream>    sRoleUdpMetaRx_buffer ("sRoleUdpMetaRx_buffer");


#pragma HLS STREAM variable=internal_event_fifo_0 depth=16
#pragma HLS STREAM variable=internal_event_fifo_1 depth=16
#pragma HLS STREAM variable=internal_event_fifo_2 depth=16
#pragma HLS STREAM variable=internal_event_fifo_3 depth=16
#pragma HLS STREAM variable=merged_fifo depth=64

  //#pragma HLS STREAM variable=sA4lToTcpAgency  depth=16 //(currently not used)
#pragma HLS STREAM variable=sA4lToPortLogic  depth=8
#pragma HLS STREAM variable=sA4lToUdpRx      depth=8
#pragma HLS STREAM variable=sA4lToTcpRx      depth=8
#pragma HLS STREAM variable=sA4lToStatusProc depth=8
//#pragma HLS STREAM variable=sA4lMrtUpdate    depth=16
#pragma HLS STREAM variable=sStatusUpdate    depth=128 //should be larger than ctrlLink size

#pragma HLS STREAM variable=sGetIpReq_UdpTx  depth=8 //MRT process takes longer -> better more buffer
#pragma HLS STREAM variable=sGetIpRep_UdpTx  depth=8
#pragma HLS STREAM variable=sGetIpReq_TcpTx  depth=8
#pragma HLS STREAM variable=sGetIpRep_TcpTx  depth=8
#pragma HLS STREAM variable=sGetNidReq_UdpRx depth=8
#pragma HLS STREAM variable=sGetNidRep_UdpRx depth=8
#pragma HLS STREAM variable=sGetNidReq_TcpRx depth=8
#pragma HLS STREAM variable=sGetNidRep_TcpRx depth=8
#pragma HLS STREAM variable=sGetNidReq_TcpTx depth=8
#pragma HLS STREAM variable=sGetNidRep_TcpTx depth=8

#pragma HLS STREAM variable=sUdpPortsToClose      depth=4
#pragma HLS STREAM variable=sUdpPortsToOpen       depth=4
#pragma HLS STREAM variable=sUdpPortsOpenFeedback depth=4
#pragma HLS STREAM variable=sTcpPortsToOpen       depth=4
#pragma HLS STREAM variable=sTcpPortsOpenFeedback depth=4


#pragma HLS STREAM variable=sRoleTcpDataRx_buffer depth=252 //NAL_MAX_FIFO_DEPTS_BYTES/8 (+2)
#pragma HLS STREAM variable=sRoleTcpMetaRx_buffer depth=32

#pragma HLS STREAM variable=sTcpWrp2Wbu_data      depth=252 //NAL_MAX_FIFO_DEPTS_BYTES/8 (+2)
#pragma HLS STREAM variable=sTcpWrp2Wbu_sessId    depth=32
#pragma HLS STREAM variable=sTcpWrp2Wbu_len       depth=32


#pragma HLS STREAM variable=sGetTripleFromSid_Req    depth=4
#pragma HLS STREAM variable=sGetTripleFromSid_Rep    depth=4
#pragma HLS STREAM variable=sGetSidFromTriple_Req    depth=4
#pragma HLS STREAM variable=sGetSidFromTriple_Rep    depth=4
#pragma HLS STREAM variable=sAddNewTriple_TcpRrh     depth=4
#pragma HLS STREAM variable=sAddNewTriple_TcpCon     depth=4
#pragma HLS STREAM variable=sDeleteEntryBySid        depth=4
#pragma HLS STREAM variable=sMarkAsPriv              depth=4
#pragma HLS STREAM variable=sMarkToDel_unpriv        depth=4
#pragma HLS STREAM variable=sGetNextDelRow_Req       depth=4
#pragma HLS STREAM variable=sGetNextDelRow_Rep       depth=4
#pragma HLS STREAM variable=sRDp_ReqNotif            depth=4

#pragma HLS STREAM variable=sNewTcpCon_Req       depth=4
#pragma HLS STREAM variable=sNewTcpCon_Rep       depth=4

#pragma HLS STREAM variable=sTcpNotif_buffer     depth=1024

//#pragma HLS RESOURCE variable=localMRT core=RAM_2P_BRAM
//#pragma HLS ARRAY_PARTITION variable=localMRT cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=localMRT complete dim=1

#pragma HLS STREAM variable=sMrtVersionUpdate_0  depth=4
#pragma HLS STREAM variable=sMrtVersionUpdate_1  depth=4

#pragma HLS STREAM variable=sCacheInvalSig_0 depth=4
#pragma HLS STREAM variable=sCacheInvalSig_1 depth=4
#pragma HLS STREAM variable=sCacheInvalSig_2 depth=4
#pragma HLS STREAM variable=sCacheInvalSig_3 depth=4

#pragma HLS STREAM variable=sRoleFifoEmptySig depth=8

#pragma HLS STREAM variable=sStartTclCls_sig depth=4
#pragma HLS STREAM variable=sNalPortUpdate   depth=8

#pragma HLS STREAM variable=sRoleUdpDataRx_buffer depth=252 //NAL_MAX_FIFO_DEPTS_BYTES/8 (+2)
#pragma HLS STREAM variable=sRoleUdpMetaRx_buffer depth=32



  //=================================================================================================
  // Reset static variables

//#pragma HLS reset variable=mrt_version_processed
//
//#pragma HLS reset variable=expect_FMC_response
//#pragma HLS reset variable=start_tcp_cls_fsm

  //===========================================================
  //  core wide STATIC DATAFLOW variables
  
  //static bool role_fifo_empty;
  //static ap_uint<32>   status_udp_ports;
  //static ap_uint<32> status_tcp_ports;
  //static ap_uint<16> status_fmc_ports;
  //static bool detected_cache_invalidation;
  //static bool nts_ready_and_enabled;

  //===========================================================
  //  core wide variables (for one iteration)

  //ap_uint<32> ipAddrBE = *myIpAddress;
  //bool nts_ready_and_enabled = (*piNTS_ready == 1 && *layer_4_enabled == 1);
  //bool start_udp_cls_fsm = false;




  //===========================================================
  // restore saved states and ports handling & check for resets

//  pPortAndResetLogic(layer_4_enabled, layer_7_enabled, role_decoupled, piNTS_ready, piMMIO_FmcLsnPort,
//      pi_udp_rx_ports, pi_tcp_rx_ports, sA4lToPortLogic, sUdpPortsToOpen, sUdpPortsToClose,
//      sTcpPortsToOpen, sUdpPortsOpenFeedback, sTcpPortsOpenFeedback, sMarkToDel_unpriv, &detected_cache_invalidation, &nts_ready_and_enabled,
//      &status_udp_ports, &status_tcp_ports, &status_fmc_ports, &start_tcp_cls_fsm, &mrt_version_processed, &mrt_version_used);

  pCacheInvalDetection(layer_4_enabled, role_decoupled, piNTS_ready, sMrtVersionUpdate_0, 
      sCacheInvalSig_0, sCacheInvalSig_1, sCacheInvalSig_2, sCacheInvalSig_3);

  pPortLogic(layer_4_enabled, layer_7_enabled, role_decoupled, piNTS_ready, piMMIO_FmcLsnPort,
      pi_udp_rx_ports, pi_tcp_rx_ports, sA4lToPortLogic, sUdpPortsToOpen, sUdpPortsToClose,
      sTcpPortsToOpen, sUdpPortsOpenFeedback, sTcpPortsOpenFeedback, sMarkToDel_unpriv,
      sNalPortUpdate, sStartTclCls_sig);

  //=================================================================================================
  // TX UDP

  pUdpTX(siUdp_data, siUdp_meta, soUOE_Data, soUOE_Meta, soUOE_DLen,
      sGetIpReq_UdpTx, sGetIpRep_UdpTx,
      myIpAddress, sCacheInvalSig_0, internal_event_fifo_0);

  //=================================================================================================
  // RX UDP

  pUdpLsn(soUOE_LsnReq, siUOE_LsnRep, sUdpPortsToOpen, sUdpPortsOpenFeedback);

  pUdpRx(sRoleUdpDataRx_buffer, sRoleUdpMetaRx_buffer, siUOE_Data, siUOE_Meta,
      sA4lToUdpRx, sGetNidReq_UdpRx, sGetNidRep_UdpRx,
      sCacheInvalSig_1, internal_event_fifo_1);

  pRoleUdpRxDeq(layer_7_enabled, role_decoupled, sRoleUdpDataRx_buffer, sRoleUdpMetaRx_buffer,
      soUdp_data, soUdp_meta);

  //=================================================================================================
  // UDP Port Close

  pUdpCls(soUOE_ClsReq, siUOE_ClsRep, sUdpPortsToClose);

  //=================================================================================================
  // TCP pListen
  pTcpLsn(soTOE_LsnReq, siTOE_LsnRep, sTcpPortsToOpen, sTcpPortsOpenFeedback);

  //=================================================================================================
  // TCP Read Request Handler

  pTcpRxNotifEnq(siTOE_Notif, sTcpNotif_buffer);

  pTcpRRh(sTcpNotif_buffer, soTOE_DReq, sAddNewTriple_TcpRrh, sDeleteEntryBySid,  sRDp_ReqNotif,
      piFMC_Tcp_data_FIFO_prog_full, piFMC_Tcp_sessid_FIFO_prog_full, sRoleFifoEmptySig);

  //=================================================================================================
  // TCP Read Path
  pTcpRDp(sRDp_ReqNotif, siTOE_Data, siTOE_SessId, soFMC_Tcp_data, soFMC_Tcp_SessId,
      //soTcp_data, soTcp_meta,
      sRoleTcpDataRx_buffer, sRoleTcpMetaRx_buffer,
      sA4lToTcpRx, sGetNidReq_TcpRx, sGetNidRep_TcpRx, sGetTripleFromSid_Req, sGetTripleFromSid_Rep,
      sMarkAsPriv, piMMIO_CfrmIp4Addr, piMMIO_FmcLsnPort, layer_7_enabled, role_decoupled,
      sCacheInvalSig_2, internal_event_fifo_2);


  pRoleTcpRxDeq(layer_7_enabled, role_decoupled, sRoleTcpDataRx_buffer, sRoleTcpMetaRx_buffer, soTcp_data, soTcp_meta, sRoleFifoEmptySig);

  //=================================================================================================
  // TCP Write Path
  pTcpWRp(siFMC_Tcp_data, siFMC_Tcp_SessId, siTcp_data, siTcp_meta,
      sTcpWrp2Wbu_data, sTcpWrp2Wbu_sessId, sTcpWrp2Wbu_len,
      sGetIpReq_TcpTx, sGetIpRep_TcpTx,
      //sGetNidReq_TcpTx, sGetNidRep_TcpTx,
      sGetSidFromTriple_Req, sGetSidFromTriple_Rep, sNewTcpCon_Req, sNewTcpCon_Rep,
      sCacheInvalSig_3, internal_event_fifo_3);

  pTcpWBu(layer_4_enabled, piNTS_ready, sTcpWrp2Wbu_data, sTcpWrp2Wbu_sessId, sTcpWrp2Wbu_len,
      soTOE_Data, soTOE_SndReq, siTOE_SndRep);

  //=================================================================================================
  // TCP start remote connection
  pTcpCOn(soTOE_OpnReq, siTOE_OpnRep, sAddNewTriple_TcpCon, sNewTcpCon_Req, sNewTcpCon_Rep);

  //=================================================================================================
  // TCP connection close
  pTcpCls(soTOE_ClsReq, sGetNextDelRow_Req, sGetNextDelRow_Rep, sStartTclCls_sig);

  //=================================================================================================
  // TCP Table Management

  pTcpAgency(sGetTripleFromSid_Req, sGetTripleFromSid_Rep, sGetSidFromTriple_Req, sGetSidFromTriple_Rep,
      sAddNewTriple_TcpRrh, sAddNewTriple_TcpCon, sDeleteEntryBySid, sMarkAsPriv, sMarkToDel_unpriv,
      sGetNextDelRow_Req, sGetNextDelRow_Rep);

  //===========================================================
  //  update status, config, MRT

  //eventFifoMerge(layer_4_enabled, layer_7_enabled, role_decoupled, &mrt_version_used, &status_udp_ports, &status_tcp_ports,
  //    &status_fmc_ports, sA4lToStatusProc, internal_event_fifo_0, internal_event_fifo_1, internal_event_fifo_2, internal_event_fifo_3, sStatusUpdate);

  eventFifoMerge( internal_event_fifo_0, internal_event_fifo_1, internal_event_fifo_2, internal_event_fifo_3, merged_fifo);

  pStatusMemory(merged_fifo, layer_7_enabled, role_decoupled, sA4lToStatusProc, sMrtVersionUpdate_1, sNalPortUpdate, sStatusUpdate);


  axi4liteProcessing(ctrlLink,
      //sA4lToTcpAgency, //(currently not used)
      sA4lToPortLogic, sA4lToUdpRx,
      sA4lToTcpRx, sA4lToStatusProc,
      localMRT,
      sMrtVersionUpdate_0, sMrtVersionUpdate_1,
      sStatusUpdate);


  pMrtAgency(localMRT, sGetIpReq_UdpTx, sGetIpRep_UdpTx, sGetIpReq_TcpTx, sGetIpRep_TcpTx, sGetNidReq_UdpRx, sGetNidRep_UdpRx, sGetNidReq_TcpRx, sGetNidRep_TcpRx);


}


/*! \} */

