// *****************************************************************************
// *
// *                             cloudFPGA
// *            All rights reserved -- Property of IBM
// *
// *----------------------------------------------------------------------------
// *
// * Title : Toplevel of the echo application in store-and-forward mode.
// *
// * File    : RoleEchoStoreAndForward.cpp
// *
// * Created : Apr. 2018
// * Authors : Francois Abel <fab@zurich.ibm.com>
// *           Jagath Weerasinghe
// *
// * Devices : xcku060-ffva1156-2-i
// * Tools   : Vivado v2017.4 (64-bit)
// * Depends : None
// *
// * Description : This role application implements an echo loopback on the UDP
// *   and TCP connections. The application is said to be operating in
// *   "store-and-forward" mode because every received packet is first stored
// *   in the DDR4 before being read from that memory and being sent back.
// * 
// * Infos & Remarks:
// *   The SHELL provides two logical memory ports for the ROLE to access the
// *   a physical channel of the DDR4. The interfaces of these two ports enable
// *   high throughput transfer of data between the AXI4 memory-mapped domain of
// *   the DDR4 and the AXI4-Stream domains of the ROLE. The interfaces are 
// *   based on an underlying AXI-DataMover-IP from Xilinx which provides
// *   specific Memory-Map-to-Stream (MM2S) and a Stream-to-Memory-Map (S2MM)
// *   channels for handling the transaction between the two domains.   
// *   For more details, refer to Xilinx/LogiCORE-IP-Product-Guide (PG022). 
// *
// * Conventions:
// *   <pi> stands for "PortIn".
// *   <po> stands for "PortOut".
// *   <si> stands for "StreamIn". 
// *   <so> stands for "StreamOut".
// *   <si><SRC>_<Itf1>_<Itf1.1>_<Itf1.1.1>_tdata stands for the "data" signals
// *        of an Axi4-Stream generated by the source (i.e. master) "SRC", out
// *        of its interface "Itf1" and its sub-interfaces "Itf1.1" and 
// *        "Itf1.1.1".
// *
// *****************************************************************************

#include "EchoStoreAndForward.hpp"

using namespace hls;


/*****************************************************************************/
/* @brief 	  Counts the number of 1s an 8-bit value.
 * @ingroup   RoleEchoHls
 *
 * @param[in] keepVal is the parameter to check.
 *
 * @return    The number of bit set in the .
 *****************************************************************************/
ap_uint<4> keepToLen(ap_uint<8> keepVal) {
	ap_uint<4> count = 0;

	switch(keepVal){
		case 0x01: count = 1; break;
		case 0x03: count = 2; break;
		case 0x07: count = 3; break;
		case 0x0F: count = 4; break;
		case 0x1F: count = 5; break;
		case 0x3F: count = 6; break;
		case 0x7F: count = 7; break;
		case 0xFF: count = 8; break;
	}
	return count;
}

/************************************************
void conv_64_to_512(stream<axiWord>& 			iData,
			   	    stream<axiMemWord>&			oData)
{
#pragma HLS PIPELINE II=1

	axiWord temp_word_64;
	//axiMemWord temp_word_512;
	static axiMemWord word_512(0,0,0);
	static ap_uint<4> rd_count = 0;

	if(!iData.empty() && !oData.full()){
		iData.read(temp_word_64);
		word_512.data(((rd_count+1)*64)-1, rd_count*64) = temp_word_64.data(63,0);
		word_512.keep(((rd_count+1)*8)-1, rd_count*8) = temp_word_64.keep(7,0);

		if(!temp_word_64.last){
			word_512.last = 0;
			rd_count = rd_count +1;

		} else if (temp_word_64.last || rd_count == 3) {
			word_512.last = 1;
			rd_count = 0;
			oData.write(word_512);
		}

	}
}
*************************************************/

/************************************************
void conv_512_to_64(stream<axiWord>& 			iData,
			   	    stream<axiMemWord>&			oData)
{
#pragma HLS PIPELINE II=1

	if(!iData.empty() && !oData.full()){

	}

}
*************************************************/

/************************************************
void traffic_sink(
	stream<axiWord>& 			iRxData,
	stream<axiWord>& 			oTxData)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

#pragma HLS INTERFACE axis port=iRxData
#pragma HLS INTERFACE axis port=oTxData

static bool tx =  true;

	if(!iRxData.empty() ){
		iRxData.read();
	}

	if(tx){
		if(!oTxData.full()){
			oTxData.write(axiWord(0,0,0));
			tx = false;
		}
	}
}
*************************************************/


/*****************************************************************************/
/* @brief 	Main process of the ECHO application.
 * @ingroup RoleEchoHls
 *
 * @param[in]     siUdp is the incoming UDP stream (from SHELL/Nts)
 * @param[out]    soUdp is the outgoing UDP stream (to   SHELL/Nts)
 * @param[in]     siTcp is the incoming TCP stream (from SHELL/Nts)
 * @param[out]    soTcp is the outgoing TCP stream (to   SHELL/Nts)
 * @param[out]    soMemRdCmd0 is the outgoing memory read command for port 0
 * @param[in]     siMemRdSts0 is the incoming memory read status for port 0
 * @param[in]     siMemRead0  is the incoming data read from port 0
 * @param[out]    soMemWrCmd0 is the outgoing memory write command for port 0
 * @param[in]     siMemWrSts0 is the incoming memory write status for port 0
 * @param[out]    soMemWrite0 is the outgoing data write for port 0
 * @param[out]    soMemRdCmd1 is the outgoing memory read command for port 1
 * @param[in]     siMemRdSts1 is the incoming memory read status for port 1
 * @param[in]     siMemRead1  is the incoming data read from port 1
 * @param[out]    soMemWrCmd1 is the outgoing memory write command for port 1
 * @param[in]     siMemWrSts1 is the incoming memory write status for port 1
 * @param[out]    soMemWrite1 is the outgoing data write for port 1
 * 
 * @return { description of the return value }.
 *****************************************************************************/

void EchoStoreAndForward(

	//------------------------------------------------------
	//-- SHELL / Role / Nts0 / Udp Interface
	//------------------------------------------------------
    stream<axiWord>			&siUdp,
	stream<axiWord>			&soUdp,
	
	//------------------------------------------------------
	//-- SHELL / Role / Nts0 / Tcp Interface
	//------------------------------------------------------
	stream<axis<64> >		&siTcp,
	stream<axis<64> >		&soTcp,
	
	//------------------------------------------------------
	//-- SHELL / Role / Mem / Mp0 Interface
	//------------------------------------------------------
	//---- Read Path (MM2S) ------------
	stream<mmCmd>			&soMemRdCmdP0,
	stream<mmStatus>		&siMemRdStsP0,
	stream<axiMemWord>		&siMemReadP0,
	//---- Write Path (S2MM) -----------
	stream<mmCmd>			&soMemWrCmdP0,
    stream<mmStatus>		&siMemWrStsP0,
    stream<axiMemWord>		&soMemWriteP0,

    //------------------------------------------------------
	//-- SHELL / Role / Mem / Mp1 Interface
	//------------------------------------------------------
	//---- Read Path (MM2S) ------------
    stream<mmCmd>			&soMemRdCmdP1,
    stream<mmStatus>		&simemRdStsP1,
    stream<axiMemWord>		&siMemReadP1,
    //---- Write Path (S2MM) -----------
    stream<mmCmd>			&soMemWrCmdP1,
    stream<mmStatus>		&siMemWrStsP1,
    stream<axiMemWord>		&soMemWriteP1

) {

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

// Bundling: SHELL / Role / Nts0 / Udp Interface
#pragma HLS RESOURCE core=AXI4Stream variable=siUdp        metadata="-bus_bundle siSHL_Rol_Nts0_Udp"
#pragma HLS RESOURCE core=AXI4Stream variable=soUdp        metadata="-bus_bundle soROL_Shl_Nts0_Udp"

// Bundling: SHELL / Role / Nts0 / Tcp Interface
#pragma HLS RESOURCE core=AXI4Stream variable=siTcp        metadata="-bus_bundle siSHL_Rol_Nts0_Tcp"
#pragma HLS RESOURCE core=AXI4Stream variable=soTcp        metadata="-bus_bundle soROL_Shl_Nts0_Tcp"

// Bundling: SHELL / Role / Mem / Mp0 / Read Interface
#pragma HLS RESOURCE core=AXI4Stream variable=soMemRdCmd0  metadata="-bus_bundle soROL_Shl_Mem_Mp0_RdCmd"
#pragma HLS DATA_PACK                variable=soMemRdCmd0
#pragma HLS RESOURCE core=AXI4Stream variable=siMemRdSts0  metadata="-bus_bundle siSHL_Rol_Mem_Mp0_RdSts"
#pragma HLS RESOURCE core=AXI4Stream variable=siMemRead0   metadata="-bus_bundle siSHL_Rol_Mem_Mp0_Read"

// Bundling: SHELL / Role / Mem / Mp0 / Write Interface
#pragma HLS RESOURCE core=AXI4Stream variable=soMemWrCmd0  metadata="-bus_bundle soROL_Shl_Mem_Mp0_WrCmd"
#pragma HLS DATA_PACK                variable=soMemWrCmd0
#pragma HLS RESOURCE core=AXI4Stream variable=siMemWrSts0  metadata="-bus_bundle siSHL_Rol_Mem_Mp0_WrSts"
#pragma HLS DATA_PACK                variable=siMemWrSts0
#pragma HLS RESOURCE core=AXI4Stream variable=soMemWrite0  metadata="-bus_bundle soROL_Shl_Mem_Mp0_Write"

// Bundling: SHELL / Role / Mem / Mp1 / Read Interface
#pragma HLS RESOURCE core=AXI4Stream variable=soMemRdCmd1  metadata="-bus_bundle soROL_Shl_Mem_Mp1_RdCmd"
#pragma HLS DATA_PACK                variable=soMemRdCmd1
#pragma HLS RESOURCE core=AXI4Stream variable=simemRdSts1  metadata="-bus_bundle siSHL_Rol_Mem_Mp1_RdSts"
#pragma HLS RESOURCE core=AXI4Stream variable=siMemRead1   metadata="-bus_bundle siSHL_Rol_Mem_Mp1_Read"

// Bundling: SHELL / Role / Mem / Mp1 / Write Interface
#pragma HLS RESOURCE core=AXI4Stream variable=soMemRdCmd1  metadata="-bus_bundle soROL_Shl_Mem_Mp1_WrCmd"
#pragma HLS DATA_PACK                variable=soMemRdCmd1
#pragma HLS RESOURCE core=AXI4Stream variable=siMemWrSts1  metadata="-bus_bundle siSHL_Rol_Mem_Mp1_WrSts"
#pragma HLS DATA_PACK                variable=siMemWrSts1
#pragma HLS RESOURCE core=AXI4Stream variable=soMemWrite1  metadata="-bus_bundle soROL_Shl_Mem_Mp1_Write"


  static enum UdpState { FSM_UDP_RX_IDLE = 0,
	  	  	  	  	  	 FSM_MEM_WR_CMD_P0, FSM_MEM_WRITE_P0, FSM_MEM_WR_STS_P0,
						 FSM_MEM_RD_CMD_P0, FSM_MEM_READ_P0,  FSM_MEM_RD_STS_P0,
						 FSM_UDP_TX } udpState;

  static stream<axiWord> udpRxStream;
#pragma HLS STREAM variable=udpRxStream depth=1024

  static stream<axiWord> memRdStream;
#pragma HLS STREAM variable=memRdStream depth=1024

  axiWord                tmpUdpAxiWord;
  axiMemWord             tmpMemAxiWord;
  static ap_uint<16>     cntUdpRxBytes = 0;
  static ap_uint<32>     cUDP_BUF_BASE_ADDR = 0x00000000;	// The address of the UDP buffer in DDR4

  switch(udpState) {

  case FSM_UDP_RX_IDLE:
    if (!siUdp.empty() && !udpRxStream.full()) {
      //-- Read data from SHELL/Nts/Udp
      siUdp.read(tmpUdpAxiWord);
      udpRxStream.write(tmpUdpAxiWord);
      cntUdpRxBytes = cntUdpRxBytes + keepToLen(tmpUdpAxiWord.keep);

      if(tmpUdpAxiWord.last)
        udpState = FSM_MEM_WR_CMD_P0;
    }
    break;

  case FSM_MEM_WR_CMD_P0:
    if (!soMemWrCmdP0.full()) {
   	  //-- Post a memory write command to SHELL/Mem/Mp0
      soMemWrCmdP0.write(mmCmd(cUDP_BUF_BASE_ADDR , cntUdpRxBytes));
      udpState = FSM_MEM_WRITE_P0;
    }
    break;

  case FSM_MEM_WRITE_P0:
    if (!udpRxStream.empty() && !soMemWriteP0.full()) {
      //-- Assemble a memory word and write it to DRAM
      udpRxStream.read(tmpUdpAxiWord);
      tmpMemAxiWord.data = (0x0000000000000000, 0x0000000000000000, 0x0000000000000000, tmpUdpAxiWord.data(63,0));
      tmpMemAxiWord.keep = (0x00, 0x00, 0x00, tmpUdpAxiWord.keep);
      tmpMemAxiWord.last = tmpUdpAxiWord.last;
      soMemWriteP0.write(tmpMemAxiWord);

      if (tmpUdpAxiWord.last)
        udpState = FSM_MEM_WR_STS_P0;
    }
    break;

  case FSM_MEM_WR_STS_P0:
    if (!siMemWrStsP0.empty()) {
      //-- Get the memory write status for Mem/Mp0
      siMemWrStsP0.read();
      udpState = FSM_MEM_RD_CMD_P0;
    }
    break;

  case FSM_MEM_RD_CMD_P0:
    if (!soMemRdCmdP0.full()) {
      //-- Post a memory read command to SHELL/Mem/Mp0
      soMemRdCmdP0.write(mmCmd(cUDP_BUF_BASE_ADDR, cntUdpRxBytes));
      udpState = FSM_MEM_READ_P0;
    }
    break;

  case FSM_MEM_READ_P0:
    if (!siMemReadP0.empty() && !memRdStream.full()) {
      //-- Read a memory word from DRAM
      siMemReadP0.read(tmpMemAxiWord);
      tmpUdpAxiWord.data(63,0) = tmpMemAxiWord.data(63,0);
      tmpUdpAxiWord.keep       = tmpMemAxiWord.keep(7,0);
      tmpUdpAxiWord.last       = tmpMemAxiWord.last;
      memRdStream.write(tmpUdpAxiWord);

      if (tmpUdpAxiWord.last)
        udpState = FSM_MEM_RD_STS_P0;
    }
    break;

  case FSM_MEM_RD_STS_P0:
	if (!siMemRdStsP0.empty()) {
        //-- Get the memory read status for Mem/Mp0
        siMemWrStsP0.read();
        udpState = FSM_UDP_TX;
      }
      break;

  case FSM_UDP_TX:
    if (!memRdStream.empty() && !soUdp.full()) {
      //-- Write data to SHELL/Nts/Udp
      memRdStream.read(tmpUdpAxiWord);
      soUdp.write(tmpUdpAxiWord);
      if (tmpUdpAxiWord.last) {
        udpState = FSM_UDP_RX_IDLE;
        cntUdpRxBytes = 0;
      }
    }
    break;

  }
}


