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
 * @file       : hss.cpp
 * @brief      : The Housekeeping Sub System (HSS) of the NAL core.
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Abstraction Layer (NAL)
 * Language    : Vivado HLS
 * 
 * \ingroup NAL
 * \addtogroup NAL
 * \{
 *****************************************************************************/

#include "uss.hpp"
#include "nal.hpp"

using namespace hls;

/**
 * This is the process that has to know which config value means what
 * And to where it should be send:
 *
 * Returns:
 *  0 : no propagation
 *  1 : to TCP tables
 *  2 : to Port logic
 *  3 : to own rank receivers
 */
uint8_t selectConfigUpdatePropagation(uint16_t config_addr)
{
  switch(config_addr) {
    default:
      return 0;
    case NAL_CONFIG_OWN_RANK:
      return 3;
    case NAL_CONFIG_SAVED_FMC_PORTS:
    case NAL_CONFIG_SAVED_TCP_PORTS:
    case NAL_CONFIG_SAVED_UDP_PORTS:
      return 2;
  }
}


void axi4liteProcessing(
    ap_uint<1>          *layer_4_enabled,
    ap_uint<32>   ctrlLink[MAX_MRT_SIZE + NUMBER_CONFIG_WORDS + NUMBER_STATUS_WORDS],
    ap_uint<32>   *mrt_version_processed,
    stream<NalConfigUpdate>   &sToTcpAgency,
    stream<NalConfigUpdate>   &sToPortLogic,
    stream<NalConfigUpdate>   &sToUdpRx,
    stream<NalConfigUpdate>   &sToTcpRx,
    stream<NalConfigUpdate>   &sToStatusProc,
    //stream<NalMrtUpdate>    &sMrtUpdate,
    stream<NalStatusUpdate>   &sStatusUpdate,
    stream<NodeId>        &sGetIpReq_UdpTx,
    stream<Ip4Addr>       &sGetIpRep_UdpTx,
    stream<NodeId>        &sGetIpReq_TcpTx,
    stream<Ip4Addr>       &sGetIpRep_TcpTx,
    stream<Ip4Addr>       &sGetNidReq_UdpRx,
    stream<NodeId>        &sGetNidRep_UdpRx,
    stream<Ip4Addr>       &sGetNidReq_TcpRx,
    stream<NodeId>        &sGetNidRep_TcpRx,
    stream<Ip4Addr>       &sGetNidReq_TcpTx,
    stream<NodeId>        &sGetNidRep_TcpTx
)
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
  //#pragma HLS pipeline II=1

  //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
  static uint16_t tableCopyVariable = 0;
  static bool tables_initialized = false;

#pragma HLS reset variable=tableCopyVariable
#pragma HLS reset variable=tables_initialized

  //-- STATIC DATAFLOW VARIABLES --------------------------------------------
  static ap_uint<32> localMRT[MAX_MRT_SIZE];
  static ap_uint<32> config[NUMBER_CONFIG_WORDS];
  static ap_uint<32> status[NUMBER_STATUS_WORDS];


  //-- LOCAL DATAFLOW VARIABLES ---------------------------------------------

  if(*layer_4_enabled == 0)
  {
    //also, all sessions should be lost
    tables_initialized = false;
  }
  // ----- tables init -----

  if(!tables_initialized)
  {
    for(int i = 0; i < MAX_MRT_SIZE; i++)
    {
      localMRT[i] = 0x0;
    }
    for(int i = 0; i < NUMBER_CONFIG_WORDS; i++)
    {
      config[i] = 0x0;
    }
    for(int i = 0; i < NUMBER_STATUS_WORDS; i++)
    {
      status[i] = 0x0;
    }
    tables_initialized = true;
  }
  // ----- always apply and serve updates -----

  if(!sStatusUpdate.empty())
  {
    NalStatusUpdate su = sStatusUpdate.read();
    status[su.status_addr] = su.new_value;
    printf("[A4l] got status update for address %d with value %d\n", (int) su.status_addr, (int) su.new_value);
  }

  if(!sGetIpReq_UdpTx.empty() && !sGetIpRep_UdpTx.full())
  {
    NodeId rank = sGetIpReq_UdpTx.read();
    if(rank > MAX_MRT_SIZE)
    {
      //return zero on failure
      sGetIpRep_UdpTx.write(0);
    } else {
      sGetIpRep_UdpTx.write(localMRT[rank]);
      //if request is issued, requester should be ready to read --> "blocking" write
    }
  }

  if(!sGetIpReq_TcpTx.empty() && !sGetIpRep_TcpTx.full())
  {
    NodeId rank = sGetIpReq_TcpTx.read();
    if(rank > MAX_MRT_SIZE)
    {
      //return zero on failure
      sGetIpRep_TcpTx.write(0);
    } else {
      sGetIpRep_TcpTx.write(localMRT[rank]);
    }
  }

  if(!sGetNidReq_UdpRx.empty() && !sGetNidRep_UdpRx.full())
  {
    ap_uint<32> ipAddr = sGetNidReq_UdpRx.read();
    printf("[HSS-INFO] Searching for Node ID of IP %d.\n", (int) ipAddr);
    NodeId rep = INVALID_MRT_VALUE;
    for(uint32_t i = 0; i< MAX_MRT_SIZE; i++)
    {
#pragma HLS unroll factor=8
      if(localMRT[i] == ipAddr)
      {
        rep = (NodeId) i;
        break;
      }
    }
    sGetNidRep_UdpRx.write(rep);
  }

  if(!sGetNidReq_TcpRx.empty() && !sGetNidRep_TcpRx.full())
  {
    ap_uint<32> ipAddr = sGetNidReq_TcpRx.read();
    printf("[HSS-INFO] Searching for Node ID of IP %d.\n", (int) ipAddr);
    NodeId rep = INVALID_MRT_VALUE;
    for(uint32_t i = 0; i< MAX_MRT_SIZE; i++)
    {
#pragma HLS unroll factor=8
      if(localMRT[i] == ipAddr)
      {
        rep = (NodeId) i;
        break;
      }
    }
    sGetNidRep_TcpRx.write(rep);
  }

  if(!sGetNidReq_TcpTx.empty() && !sGetNidRep_TcpTx.full())
  {
    ap_uint<32> ipAddr = sGetNidReq_TcpTx.read();
    printf("[HSS-INFO] Searching for Node ID of IP %d.\n", (int) ipAddr);
    NodeId rep = INVALID_MRT_VALUE;
    for(uint32_t i = 0; i< MAX_MRT_SIZE; i++)
    {
#pragma HLS unroll factor=8
      if(localMRT[i] == ipAddr)
      {
        rep = (NodeId) i;
        break;
      }
    }
    sGetNidRep_TcpTx.write(rep);
  }


  // ----- AXI4Lite Processing -----

  //TODO: necessary? Or does this AXI4Lite anyways "in the background"?
  //or do we need to copy it explicetly, but could do this also every ~2 seconds?
  if(tableCopyVariable < NUMBER_CONFIG_WORDS)
  {
    ap_uint<16> new_word = ctrlLink[tableCopyVariable];
    if(new_word != config[tableCopyVariable])
    {
      ap_uint<16> configProp = selectConfigUpdatePropagation(tableCopyVariable);
      NalConfigUpdate cu = NalConfigUpdate(tableCopyVariable, new_word);
      switch (configProp) {
        default:
        case 0:
          //NOP
          break;
        case 1:
          sToTcpAgency.write(cu);
          break;
        case 2:
          sToPortLogic.write(cu);
          break;
        case 3:
          sToUdpRx.write(cu);
          sToTcpRx.write(cu);
          sToStatusProc.write(cu);
          printf("[A4l] Issued rank update: %d\n", (int) cu.update_value);
          break;
      }
      config[tableCopyVariable] = new_word;
    }
  }
  if(tableCopyVariable < MAX_MRT_SIZE)
  {
    ap_uint<32> new_ip4node = ctrlLink[tableCopyVariable + NUMBER_CONFIG_WORDS + NUMBER_STATUS_WORDS];
    if (new_ip4node != localMRT[tableCopyVariable])
    {
      //NalMrtUpdate mu = NalMrtUpdate(tableCopyVariable, new_ip4node);
      //sMrtUpdate.write(mu);
      localMRT[tableCopyVariable] = new_ip4node;
    }
  }

  if(tableCopyVariable < NUMBER_STATUS_WORDS)
  {
    ctrlLink[NUMBER_CONFIG_WORDS + tableCopyVariable] = status[tableCopyVariable];
  }

  if(tableCopyVariable >= MAX_MRT_SIZE)
  {
    tableCopyVariable = 0;
    //acknowledge the processed version
    ap_uint<32> new_mrt_version = config[NAL_CONFIG_MRT_VERSION];
    // if(new_mrt_version > mrt_version_processed)
    // {
    //invalidate cache
    //cached_udp_rx_ipaddr = 0;
    //cached_tcp_rx_session_id = UNUSED_SESSION_ENTRY_VALUE;
    //cached_tcp_tx_tripple = UNUSED_TABLE_ENTRY_VALUE;
    //   detected_cache_invalidation = true;
    //moved to outer process
    //}
    *mrt_version_processed = new_mrt_version;
  }  else {
    tableCopyVariable++;
  }


}


void pPortAndResetLogic(
    ap_uint<1>        *layer_4_enabled,
    ap_uint<1>        *layer_7_enabled,
    ap_uint<1>        *role_decoupled,
    ap_uint<1>        *piNTS_ready,
    ap_uint<16>       *piMMIO_FmcLsnPort,
    ap_uint<32>           *pi_udp_rx_ports,
    ap_uint<32>         *pi_tcp_rx_ports,
    stream<NalConfigUpdate> &sConfigUpdate,
    stream<UdpPort>     &sUdpPortsToOpen,
    stream<UdpPort>     &sUdpPortsToClose,
    stream<TcpPort>     &sTcpPortsToOpen,
    stream<bool>      &sUdpPortsOpenFeedback,
    stream<bool>      &sTcpPortsOpenFeedback,
	stream<bool>      &sMarkToDel_unpriv,
    bool          *detected_cache_invalidation,
    ap_uint<32>       *status_udp_ports,
    ap_uint<32>       *status_tcp_ports,
    ap_uint<16>       *status_fmc_ports,
    bool          *start_tcp_cls_fsm
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
  //#pragma HLS pipeline II=1

  //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
  static ap_uint<16> processed_FMC_listen_port = 0;
  static bool fmc_port_opened = false;
  static ap_uint<32> tcp_rx_ports_processed = 0;
  static bool pr_was_done_flag = false;
  static ap_uint<32> udp_rx_ports_processed = 0;
  static bool wait_for_udp_port_open = false;
  static bool wait_for_tcp_port_open = false;
  static bool need_write_sMarkToDel_unpriv = false;

#ifndef __SYNTHESIS__
  static ap_uint<16>  mmio_stabilize_counter = 1;
#else
  static ap_uint<16>  mmio_stabilize_counter = NAL_MMIO_STABILIZE_TIME;
#endif

#pragma HLS reset variable=pr_was_done_flag

#pragma HLS reset variable=mmio_stabilize_counter
#pragma HLS reset variable=processed_FMC_listen_port
#pragma HLS reset variable=fmc_port_opened
#pragma HLS reset variable=udp_rx_ports_processed
#pragma HLS reset variable=tcp_rx_ports_processed
#pragma HLS reset variable=wait_for_udp_port_open
#pragma HLS reset variable=wait_for_tcp_port_open
#pragma HLS reset variable=need_write_sMarkToDel_unpriv

  //-- STATIC DATAFLOW VARIABLES --------------------------------------------
  static ap_uint<16> new_relative_port_to_req_udp = 0;
  static ap_uint<16> new_relative_port_to_req_tcp = 0;


  //-- LOCAL DATAFLOW VARIABLES ---------------------------------------------
  //===========================================================
  // restore saved states

  if(!sConfigUpdate.empty())
  {
    NalConfigUpdate ca = sConfigUpdate.read();
    switch(ca.config_addr)
    {
      case NAL_CONFIG_SAVED_FMC_PORTS:
        processed_FMC_listen_port = (ap_uint<16>) ca.update_value;
        break;
      case NAL_CONFIG_SAVED_UDP_PORTS:
        udp_rx_ports_processed = ca.update_value;
        break;
      case NAL_CONFIG_SAVED_TCP_PORTS:
        tcp_rx_ports_processed = ca.update_value;
        break;
      default:
        printf("[ERROR] invalid config update received!\n");
        break;
    }

  }

  //   // > to avoid loop at 0
  //      if(config[NAL_CONFIG_SAVED_FMC_PORTS] > processed_FMC_listen_port)
  //      {
  //        processed_FMC_listen_port = (ap_uint<16>) config[NAL_CONFIG_SAVED_FMC_PORTS];
  //      }
  //
  //      if(*layer_7_enabled == 1 && *role_decoupled == 0)
  //      { // looks like only we were reset
  //        // since the user cannot close ports (up to now), the > should work...
  //        if(config[NAL_CONFIG_SAVED_UDP_PORTS] > udp_rx_ports_processed)
  //        {
  //          udp_rx_ports_processed = config[NAL_CONFIG_SAVED_UDP_PORTS];
  //        }
  //
  //        if(config[NAL_CONFIG_SAVED_TCP_PORTS] > tcp_rx_ports_processed)
  //        {
  //          tcp_rx_ports_processed = config[NAL_CONFIG_SAVED_TCP_PORTS];
  //        }
  //      }

  //if layer 4 is reset, ports will be closed
  if(*layer_4_enabled == 0)
  {
    processed_FMC_listen_port = 0x0;
    udp_rx_ports_processed = 0x0;
    tcp_rx_ports_processed = 0x0;
    //also, all sessions should be lost
    //tables_initalized = false;
    //we don't need to close ports any more
    //clsFsmState_Tcp = CLS_IDLE;
    //clsFsmState_Udp = CLS_IDLE;
    //and we shouldn't expect smth
    //expect_FMC_response = false;
    //invalidate cache
    //cached_udp_rx_ipaddr = 0;
    //cached_tcp_rx_session_id = UNUSED_SESSION_ENTRY_VALUE;
    //cached_tcp_tx_tripple = UNUSED_TABLE_ENTRY_VALUE;
    *detected_cache_invalidation = true;
    need_write_sMarkToDel_unpriv = false;
  }

  if(*layer_7_enabled == 0 || *role_decoupled == 1 )
  {
    if(*layer_4_enabled == 1 && *piNTS_ready == 1)
    {
      if(udp_rx_ports_processed > 0)
      {

        //mark all UDP ports as to be deleted
        //udp_rx_ports_to_close = udp_rx_ports_processed;
        ap_uint<16> newRelativePortToClose = 0;
        ap_uint<16> newAbsolutePortToClose = 0;
        newRelativePortToClose = getRightmostBitPos(udp_rx_ports_processed);
        while(newRelativePortToClose != 0)
        {
          newAbsolutePortToClose = NAL_RX_MIN_PORT + newRelativePortToClose;
          sUdpPortsToClose.write(newAbsolutePortToClose);
          ap_uint<32> one_cold_closed_port = ~(((ap_uint<32>) 1) << (newRelativePortToClose));
          udp_rx_ports_processed &= one_cold_closed_port;
          printf("new UDP port ports to close: %#04x\n",(unsigned int) udp_rx_ports_processed);
          newRelativePortToClose = getRightmostBitPos(udp_rx_ports_processed);
        }
        //start closing FSM UDP
        //clsFsmState_Udp = CLS_NEXT;
        //start_udp_cls_fsm = true;
      }

      if(tcp_rx_ports_processed > 0)
      {
        //mark all TCP ports as to be deleted
        //markCurrentRowsAsToDelete_unprivileged();
    	  need_write_sMarkToDel_unpriv = true;
        if( *role_decoupled == 0 )
        {//start closing FSM TCP
          //clsFsmState_Tcp = CLS_NEXT;
          *start_tcp_cls_fsm = true;
        } else {
          //FMC is using TCP!
          pr_was_done_flag = true;
        }
      }
    }
    //in all cases
    udp_rx_ports_processed = 0x0;
    tcp_rx_ports_processed = 0x0;
    if( *role_decoupled == 0)
    { //invalidate cache
      //cached_udp_rx_ipaddr = 0;
      //cached_tcp_rx_session_id = UNUSED_SESSION_ENTRY_VALUE;
      //cached_tcp_tx_tripple = UNUSED_TABLE_ENTRY_VALUE;
      *detected_cache_invalidation = true;
    } else {
      //FMC is using TCP!
      pr_was_done_flag = true;
    }
  }
  if(pr_was_done_flag && *role_decoupled == 0)
  {//so, after the PR was done
    //invalidate cache
    //cached_udp_rx_ipaddr = 0;
    //cached_tcp_rx_session_id = UNUSED_SESSION_ENTRY_VALUE;
    //cached_tcp_tx_tripple = UNUSED_TABLE_ENTRY_VALUE;
    //start closing FSM TCP
    //ports have been marked earlier
    //clsFsmState_Tcp = CLS_NEXT;
    *start_tcp_cls_fsm = true;
    //FSM will wait until RDP and WRP are done
    pr_was_done_flag = false;
  }

  //only if NTS is ready
  if(*piNTS_ready == 1 && *layer_4_enabled == 1)
  {
    //===========================================================
    //  port requests
    //  only if a user application is running...
    if((udp_rx_ports_processed != *pi_udp_rx_ports) && *layer_7_enabled == 1
        && *role_decoupled == 0 && !wait_for_udp_port_open
        && !sUdpPortsToOpen.full() )
    {
      //we close ports only if layer 7 is reset, so only look for new ports
      ap_uint<32> tmp = udp_rx_ports_processed | *pi_udp_rx_ports;
      ap_uint<32> diff = udp_rx_ports_processed ^ tmp;
      //printf("rx_ports IN: %#04x\n",(int) *pi_udp_rx_ports);
      //printf("udp_rx_ports_processed: %#04x\n",(int) udp_rx_ports_processed);
      printf("UDP port diff: %#04x\n",(unsigned int) diff);
      if(diff != 0)
      {//we have to open new ports, one after another
        //new_relative_port_to_req_udp = getRightmostBitPos(diff);
        //need_udp_port_req = true;
        UdpPort new_port_to_open = NAL_RX_MIN_PORT + getRightmostBitPos(diff);
        sUdpPortsToOpen.write(new_port_to_open);
        wait_for_udp_port_open = true;
        new_relative_port_to_req_udp = getRightmostBitPos(diff);
      }
    } else if(wait_for_udp_port_open && !sUdpPortsOpenFeedback.empty())
    {
      bool fed = sUdpPortsOpenFeedback.read();
      if(fed)
      {
        udp_rx_ports_processed |= ((ap_uint<32>) 1) << (new_relative_port_to_req_udp);
        printf("new udp_rx_ports_processed: %#03x\n",(int) udp_rx_ports_processed);
      } else {
        printf("[ERROR] UDP port opening failed.\n");
        //TODO: add block list for ports? otherwise we will try it again and again
      }
      //in all cases
      wait_for_udp_port_open = false;
    }

    if(processed_FMC_listen_port != *piMMIO_FmcLsnPort
        && !wait_for_tcp_port_open
        && !sTcpPortsToOpen.full())
    {
      if(mmio_stabilize_counter == 0)
      {
        fmc_port_opened = false;
        //need_tcp_port_req = true;
#ifndef __SYNTHESIS__
        mmio_stabilize_counter = 1;
#else
        mmio_stabilize_counter = NAL_MMIO_STABILIZE_TIME;
#endif
        printf("Need FMC port request: %#02x\n",(unsigned int) *piMMIO_FmcLsnPort);
        sTcpPortsToOpen.write(*piMMIO_FmcLsnPort);
        wait_for_tcp_port_open = true;
      } else {
        mmio_stabilize_counter--;
      }

    } else if((tcp_rx_ports_processed != *pi_tcp_rx_ports) && *layer_7_enabled == 1
        && *role_decoupled == 0 && !wait_for_tcp_port_open  && !sTcpPortsToOpen.full() )
    { //  only if a user application is running...
      //we close ports only if layer 7 is reset, so only look for new ports
      ap_uint<32> tmp = tcp_rx_ports_processed | *pi_tcp_rx_ports;
      ap_uint<32> diff = tcp_rx_ports_processed ^ tmp;
      //printf("rx_ports IN: %#04x\n",(int) *pi_tcp_rx_ports);
      //printf("tcp_rx_ports_processed: %#04x\n",(int) tcp_rx_ports_processed);
      printf("TCP port diff: %#04x\n",(unsigned int) diff);
      if(diff != 0)
      {//we have to open new ports, one after another
        new_relative_port_to_req_tcp = getRightmostBitPos(diff);
        TcpPort new_port = NAL_RX_MIN_PORT + new_relative_port_to_req_tcp;
        sTcpPortsToOpen.write(new_port);
        wait_for_tcp_port_open = true;
        //need_tcp_port_req = true;
      }
    } else if(wait_for_tcp_port_open && !fmc_port_opened && !sTcpPortsOpenFeedback.empty())
    {
      bool fed = sTcpPortsOpenFeedback.read();
      if(fed)
      {
        processed_FMC_listen_port = *piMMIO_FmcLsnPort;
        printf("FMC Port opened: %#03x\n",(int) processed_FMC_listen_port);
        fmc_port_opened = true;
      } else {
        printf("[ERROR] FMC TCP port opening failed.\n");
        //TODO: add block list for ports? otherwise we will try it again and again
      }
      //in all cases
      wait_for_tcp_port_open = false;
    }
    else if(wait_for_tcp_port_open && !sTcpPortsOpenFeedback.empty())
    {
      bool fed = sTcpPortsOpenFeedback.read();
      if(fed)
      {
        tcp_rx_ports_processed |= ((ap_uint<32>) 1) << (new_relative_port_to_req_tcp);
        printf("new tcp_rx_ports_processed: %#03x\n",(int) tcp_rx_ports_processed);
      } else {
        printf("[ERROR] TCP port opening failed.\n");
        //TODO: add block list for ports? otherwise we will try it again and again
      }
      //in all cases
      wait_for_tcp_port_open = false;
    }
  }


  if(need_write_sMarkToDel_unpriv && !sMarkToDel_unpriv.full())
  {
	  sMarkToDel_unpriv.write(true);
	  need_write_sMarkToDel_unpriv = false;
  }

  //"publish" current processed ports
  *status_udp_ports = udp_rx_ports_processed;
  *status_tcp_ports = tcp_rx_ports_processed;
  *status_fmc_ports = processed_FMC_listen_port;


}


void pTcpAgency(
    stream<SessionId>     &sGetTripleFromSid_Req,
    stream<NalTriple>       &sGetTripleFromSid_Rep,
    stream<NalTriple>       &sGetSidFromTriple_Req,
    stream<SessionId>       &sGetSidFromTriple_Rep,
    stream<NalNewTableEntry>  &sAddNewTriple_TcpRrh,
    stream<NalNewTableEntry>  &sAddNewTriple_TcpCon,
    stream<SessionId>     &sDeleteEntryBySid,
    stream<SessionId>     &sMarkAsPriv,
    stream<bool>        &sMarkToDel_unpriv,
    stream<bool>        &sGetNextDelRow_Req,
    stream<SessionId>     &sGetNextDelRow_Rep,
    const bool              *nts_ready_and_enabled
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
#pragma HLS pipeline II=1

  //-- STATIC CONTROL VARIABLES (with RESET) --------------------------------
  static  TableFsmStates agencyFsm = TAB_FSM_READ;
  static  bool tables_initialized = false;

#pragma HLS RESET variable=agencyFsm
#pragma HLS RESET variable=tables_initialized
  //-- STATIC DATAFLOW VARIABLES --------------------------------------------
  static NalTriple  tripleList[MAX_NAL_SESSIONS];
  static SessionId   sessionIdList[MAX_NAL_SESSIONS];
  static ap_uint<1>  usedRows[MAX_NAL_SESSIONS];
  static ap_uint<1>  rowsToDelete[MAX_NAL_SESSIONS];
  static ap_uint<1>  privilegedRows[MAX_NAL_SESSIONS];

  //-- LOCAL DATAFLOW VARIABLES ---------------------------------------------

  if(!*nts_ready_and_enabled)
  {
    agencyFsm = TAB_FSM_READ;
    tables_initialized = false;
  }

  if (!tables_initialized)
  {
    printf("init tables...\n");
    for(int i = 0; i<MAX_NAL_SESSIONS; i++)
    {
      //#pragma HLS unroll
      sessionIdList[i] = 0;
      tripleList[i] = 0;
      usedRows[i]  =  0;
      rowsToDelete[i] = 0;
      privilegedRows[i] = 0;
    }
    tables_initialized = true;
  }

  //in order to be able to serve Read and write requests immediately
  //for(uint8_t again = 0; again < 2; again++)
  //{
  switch(agencyFsm)
  {
    case TAB_FSM_READ:
      if(!sGetTripleFromSid_Req.empty() && sGetTripleFromSid_Rep.full())
      {
        SessionId sessionID = sGetTripleFromSid_Req.read();
        printf("searching for session: %d\n", (int) sessionID);
        uint32_t i = 0;
        NalTriple ret = UNUSED_TABLE_ENTRY_VALUE;
        bool found_smth = false;
        for(i = 0; i < MAX_NAL_SESSIONS; i++)
        {
          //#pragma HLS unroll factor=8
          if(sessionIdList[i] == sessionID && usedRows[i] == 1 && rowsToDelete[i] == 0)
          {
            ret = tripleList[i];
            printf("found triple entry: %d | %d |  %llu\n",(int) i, (int) sessionID, (unsigned long long) ret);
            found_smth = true;
            break;
          }
        }
        if(!found_smth)
        {
          //unkown session TODO
          printf("[TcpAgency:INFO] Unknown session requested\n");
        }
        sGetTripleFromSid_Rep.write(ret);
      }
      if(!sGetSidFromTriple_Req.empty() && sGetSidFromTriple_Rep.full())
      {
        NalTriple triple = sGetSidFromTriple_Req.read();
        printf("Searching for triple: %llu\n", (unsigned long long) triple);
        uint32_t i = 0;
        SessionId ret = UNUSED_SESSION_ENTRY_VALUE;
        bool found_smth = false;
        for(i = 0; i < MAX_NAL_SESSIONS; i++)
        {
          //#pragma HLS unroll factor=8
          if(tripleList[i] == triple && usedRows[i] == 1 && rowsToDelete[i] == 0)
          {
            ret = sessionIdList[i];
            found_smth = true;
            break;
          }
          if(!found_smth)
          {
            //there is (not yet) a connection TODO
            printf("[TcpAgency:INFO] Unknown triple requested\n");
          }
          sGetSidFromTriple_Rep.write(ret);
        }
        agencyFsm = TAB_FSM_WRITE;
        break;
        case TAB_FSM_WRITE:
        if(!sAddNewTriple_TcpRrh.empty() || !sAddNewTriple_TcpCon.empty())
        {
          NalNewTableEntry ne_struct;
          if(!sAddNewTriple_TcpRrh.empty())
          {
            ne_struct = sAddNewTriple_TcpRrh.read();
          } else {
            ne_struct = sAddNewTriple_TcpCon.read();
          }
          SessionId sessionID = ne_struct.sessId;
          NalTriple new_entry = ne_struct.new_triple;
          printf("new tripple entry: %d |  %llu\n",(int) sessionID, (unsigned long long) new_entry);
          //first check for duplicates!
          //ap_uint<64> test_tripple = getTrippleFromSessionId(sessionID);
          bool found_smth = false;
          for(i = 0; i < MAX_NAL_SESSIONS; i++)
          {
            //#pragma HLS unroll factor=8
            if(sessionIdList[i] == sessionID && usedRows[i] == 1 && rowsToDelete[i] == 0)
            {
              ret = tripleList[i];
              printf("found triple entry: %d | %d |  %llu\n",(int) i, (int) sessionID, (unsigned long long) ret);
              found_smth = true;
              break;
            }
          }
          if(found_smth)
          {
            printf("session/triple already known, skipping. \n");
            //break; no break, because other may want to run too
          } else {
            bool stored = false;
            uint32_t i = 0;
            for(i = 0; i < MAX_NAL_SESSIONS; i++)
            {
              //#pragma HLS unroll factor=8
              if(usedRows[i] == 0)
              {//next free one, tables stay in sync
                sessionIdList[i] = sessionID;
                tripleList[i] = new_entry;
                usedRows[i] = 1;
                privilegedRows[i] = 0;
                printf("stored triple entry: %d | %d |  %llu\n",(int) i, (int) sessionID, (unsigned long long) new_entry);
                stored = true;
                break;
              }
            }
            if(!stored)
            {
              //we run out of sessions... TODO
              //actually, should not happen, since we have same table size as TOE
              printf("[TcpAgency:ERROR] no free space left in table!\n");
            }
          }
        }
        if(!sDeleteEntryBySid.empty())
        {
          SessionId sessionID = sDeleteEntryBySid.read();
          printf("try to delete session: %d\n", (int) sessionID);
          for(uint32_t i = 0; i < MAX_NAL_SESSIONS; i++)
          {
            //#pragma HLS unroll factor=8
            if(sessionIdList[i] == sessionID && usedRows[i] == 1)
            {
              usedRows[i] = 0;
              privilegedRows[i] = 0;
              printf("found and deleting session: %d\n", (int) sessionID);
              //printf("invalidating TCP RX cache\n");
              //cached_tcp_rx_session_id = UNUSED_SESSION_ENTRY_VALUE;
              break;
            }
          }
          //nothing to delete, nothing to do...
        }
        if(!sMarkAsPriv.empty())
        {
          SessionId sessionID = sMarkAsPriv.read();
          printf("mark session as privileged: %d\n", (int) sessionID);
          for(uint32_t i = 0; i < MAX_NAL_SESSIONS; i++)
          {
            //#pragma HLS unroll factor=8
            if(sessionIdList[i] == sessionID && usedRows[i] == 1)
            {
              privilegedRows[i] = 1;
              rowsToDelete[i] = 0;
              return;
            }
          }
          //nothing found, nothing to do...
        }
        if(!sMarkToDel_unpriv.empty())
        {
          if(sMarkToDel_unpriv.read())
          {
            for(uint32_t i = 0; i< MAX_NAL_SESSIONS; i++)
            {
              //#pragma HLS unroll factor=8
              if(privilegedRows[i] == 1)
              {
                continue;
              } else {
                rowsToDelete[i] = usedRows[i];
              }
            }
          }
        }
        if(!sGetNextDelRow_Req.empty() && !sGetNextDelRow_Rep.full())
        {
          if(sGetNextDelRow_Req.read())
          {
            SessionId ret = UNUSED_SESSION_ENTRY_VALUE;
            bool found_smth = false;
            for(uint32_t i = 0; i< MAX_NAL_SESSIONS; i++)
            {
              //#pragma HLS unroll factor=8
              if(rowsToDelete[i] == 1)
              {
                ret = sessionIdList[i];
                //sessionIdList[i] = 0x0; //not necessary
                //tripleList[i] = 0x0;
                usedRows[i] = 0;
                rowsToDelete[i] = 0;
                //privilegedRows[i] = 0; //not necessary
                printf("Closing session %d at table row %d.\n",(int) ret, (int) i);
                found_smth = true;
                break;
              }
            }
            if(!found_smth)
            {
              //Tables are empty
              printf("TCP tables are empty\n");
            }
            sGetNextDelRow_Rep.write(ret);
          }
        }
        agencyFsm = TAB_FSM_READ;
        break;
      }
      //}


}


/*! \} */


