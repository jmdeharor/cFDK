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
// * Authors : Jagath Weerasinghe, Francois Abel <fab@zurich.ibm.com>
// *
// * Devices : xcku060-ffva1156-2-i
// * Tools   : Vivado v2017.4 (64-bit)
// * Depends : None
// *
// * Description : This version of the role implements an echo application made
// *    of a UDP loopback and a TCP loopback connections. The role is said to be
// *    operating in "store-and-forward" mode because every received packet is
// *    first stored in the DDR4 before being read from that memory and being
// *    sent back.          
// * 
// * Comments:
// *
// *****************************************************************************


//
//  !!! THIS CODE IS UNDER CONSTRUCTION. IT REMAINS TO BE ADDAPTED FOR FMKU60 !!!
//  
// 



#include "echo_app_with_ddr3.hpp"

using namespace hls;

/** @ingroup echo_server_application
 *
 */

//#if 0

/*
ap_uint<4> keep_to_len(ap_uint<8> keepValue) { 		// This function counts the number of 1s in an 8-bit value
	ap_uint<4> counter = 0;
	for (ap_uint<4> i=0;i<8;++i) {
		if (keepValue.bit(i) == 1)
			counter++;
	}
	return counter;
}
*/

ap_uint<4> keep_to_len(ap_uint<8> keepValue) { 		// This function counts the number of 1s in an 8-bit value
	ap_uint<4> counter = 0;

	switch(keepValue){
		case 0x01: counter = 1; break;
		case 0x03: counter = 2; break;
		case 0x07: counter = 3; break;
		case 0x0F: counter = 4; break;
		case 0x1F: counter = 5; break;
		case 0x3F: counter = 6; break;
		case 0x7F: counter = 7; break;
		case 0xFF: counter = 8; break;
	}
	return counter;
}


//#if 0
void echo_app(
	stream<axiWord>& 			iRxData,
	stream<axiWord>& 			oTxData)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

#pragma HLS INTERFACE axis port=iRxData
#pragma HLS INTERFACE axis port=oTxData

	if(!iRxData.empty() && !oTxData.full()){
		oTxData.write(iRxData.read());
	}
}

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

void conv_512_to_64(stream<axiWord>& 			iData,
			   	    stream<axiMemWord>&			oData)
{
#pragma HLS PIPELINE II=1

	if(!iData.empty() && !oData.full()){

	}

}

//DRAM 2 Ports

void echo_app_with_ddr3(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,

	//DRAM Port 1
	//mem wr
	stream<mmCmd>&				oMemWrCmd_p1,
	stream<mmStatus>&			iMemWrtatus_p1,
	stream<axiMemWord>&			oMemWrData_p1,

	//mem rd
	stream<mmCmd>&				oMemRdCmd_p1,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiMemWord>&			iMemRdData_p1,

	//DRAM Port 2
	//mem wr
	stream<mmCmd>&				oMemWrCmd_p2,
	stream<mmStatus>&			iMemWrtatus_p2,
	stream<axiMemWord>&			oMemWrData_p2,

	//mem rd
	stream<mmCmd>&				oMemRdCmd_p2,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiMemWord>&			iMemRdData_p2)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle net_p1_s_axis_read_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle net_p1_m_axis_write_data"

//DRAM Port 1
#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_p1 metadata="-bus_bundle dram_p1_m_axis_write_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_p1 metadata="-bus_bundle dram_p1_s_axis_write_sts"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_p1 metadata="-bus_bundle dram_p1_m_axis_write_data"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_p1 metadata="-bus_bundle dram_p1_m_axis_read_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_p1 metadata="-bus_bundle dram_p1_s_axis_read_data"

#pragma HLS DATA_PACK variable=oMemWrCmd_p1
#pragma HLS DATA_PACK variable=oMemRdCmd_p1
#pragma HLS DATA_PACK variable=iMemWrtatus_p1

//DRAM Port 2
#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_p2 metadata="-bus_bundle dram_p2_m_axis_write_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_p2 metadata="-bus_bundle dram_p2_s_axis_write_sts"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_p2 metadata="-bus_bundle dram_p2_m_axis_write_data"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_p2 metadata="-bus_bundle dram_p2_m_axis_read_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_p2 metadata="-bus_bundle dram_p2_s_axis_read_data"

#pragma HLS DATA_PACK variable=oMemWrCmd_p2
#pragma HLS DATA_PACK variable=oMemRdCmd_p2
#pragma HLS DATA_PACK variable=iMemWrtatus_p2


static enum state { APP_NET_RX_IDLE = 0,
	                APP_MEM_WR_CMD_P1, APP_MEM_WR_P1, APP_MEM_WR_STS_P1, APP_MEM_RD_CMD_P1, APP_MEM_RD_P1,
	                APP_MEM_WR_CMD_P2, APP_MEM_WR_P2, APP_MEM_WR_STS_P2, APP_MEM_RD_CMD_P2, APP_MEM_RD_P2,
	                APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
axiMemWord temp_mem_data;
static ap_uint<4> protocol = 0;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;



	switch(app_state){
		//net read
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_CMD_P1;

			}
			break;
		//dram port 1 write read
		case APP_MEM_WR_CMD_P1:
			if (!oMemWrCmd_p1.full()) {
				oMemWrCmd_p1.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_P1;
			}
			break;
		case APP_MEM_WR_P1:
			if (!net_rx_data.empty() && !oMemWrData_p1.full()) {
				net_rx_data.read(net_rx_temp_data);
				temp_mem_data.data = (0x0000000000000000, 0x0000000000000000, 0x0000000000000000,net_rx_temp_data.data(63,0));
				temp_mem_data.keep = (0x00, 0x00, 0x00, net_rx_temp_data.keep);
				temp_mem_data.last = net_rx_temp_data.last;
				oMemWrData_p1.write(temp_mem_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_P1;
			}
		break;
		case APP_MEM_WR_STS_P1:
			if(!iMemWrtatus_p1.empty()){
				iMemWrtatus_p1.read();
				app_state = APP_MEM_RD_CMD_P1;
			}
		break;
		case APP_MEM_RD_CMD_P1:
			if(!oMemRdCmd_p1.full()){
				oMemRdCmd_p1.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_P1;
			}
		break;
		case APP_MEM_RD_P1:
			if(!iMemRdData_p1.empty() && !mem_rd_data.full() ){
				iMemRdData_p1.read(temp_mem_data);
				net_rx_temp_data.data(63,0) = temp_mem_data.data(63,0);
				net_rx_temp_data.keep = temp_mem_data.keep(7,0);
				net_rx_temp_data.last = temp_mem_data.last;
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_CMD_P2;
			}
		break;

		//dram port 2 write read
		case APP_MEM_WR_CMD_P2:
			if (!oMemWrCmd_p2.full()) {
				oMemWrCmd_p2.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_P2;
			}
			break;
		case APP_MEM_WR_P2:
			if (!mem_rd_data.empty() && !oMemWrData_p2.full()) {
				mem_rd_data.read(net_rx_temp_data);
				temp_mem_data.data = (0x0000000000000000, 0x0000000000000000, 0x0000000000000000,net_rx_temp_data.data(63,0));
				temp_mem_data.keep = (0x00, 0x00, 0x00, net_rx_temp_data.keep);
				temp_mem_data.last = net_rx_temp_data.last;
				oMemWrData_p2.write(temp_mem_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_P2;
			}
		break;
		case APP_MEM_WR_STS_P2:
			if(!iMemWrtatus_p2.empty()){
				iMemWrtatus_p2.read();
				app_state = APP_MEM_RD_CMD_P2;
			}
		break;
		case APP_MEM_RD_CMD_P2:
			if(!oMemRdCmd_p2.full()){
				oMemRdCmd_p2.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_P2;
			}
		break;
		case APP_MEM_RD_P2:
			if(!iMemRdData_p2.empty() && !mem_rd_data.full() ){
				iMemRdData_p2.read(temp_mem_data);
				net_rx_temp_data.data(63,0) = temp_mem_data.data(63,0);
				net_rx_temp_data.keep = temp_mem_data.keep(7,0);
				net_rx_temp_data.last = temp_mem_data.last;
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;

		//net write
		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last) {
					app_state = APP_NET_RX_IDLE;
					num_net_rx_data_bytes = 0;
				}
			}
		break;
	}
}

#if 0
//DRAM 1 Port
void echo_app_with_ddr3(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,
	//mem wr
	stream<mmCmd>&				oMemWrCmd,
	stream<mmStatus>&			iMemWrtatus,
	stream<axiMemWord>&			oMemWrData,

	//mem rd
	stream<mmCmd>&				oMemRdCmd,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiMemWord>&			iMemRdData)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle net_p1_s_axis_read_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle net_p1_m_axis_write_data"

#pragma HLS resource core=AXI4Stream variable=oMemWrCmd metadata="-bus_bundle dram_p1_m_axis_write_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus metadata="-bus_bundle dram_p1_s_axis_write_sts"
#pragma HLS resource core=AXI4Stream variable=oMemWrData metadata="-bus_bundle dram_p1_m_axis_write_data"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd metadata="-bus_bundle dram_p1_m_axis_read_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemRdData metadata="-bus_bundle dram_p1_s_axis_read_data"

#pragma HLS DATA_PACK variable=oMemWrCmd
#pragma HLS DATA_PACK variable=oMemRdCmd
#pragma HLS DATA_PACK variable=iMemWrtatus


static enum state { APP_NET_RX_IDLE = 0, APP_MEM_WR_CMD, APP_MEM_WR, APP_MEM_WR_STS, APP_MEM_RD_CMD, APP_MEM_RD, APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
axiMemWord temp_mem_data;
static ap_uint<4> protocol = 0;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;



	switch(app_state){
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_CMD;

			}
			break;
		case APP_MEM_WR_CMD:
			if (!oMemWrCmd.full()) {
				oMemWrCmd.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR;
			}
			break;
		case APP_MEM_WR:
			if (!net_rx_data.empty() && !oMemWrData.full()) {
				net_rx_data.read(net_rx_temp_data);
				temp_mem_data.data = (0x0000000000000000, 0x0000000000000000, 0x0000000000000000,net_rx_temp_data.data(63,0));
				temp_mem_data.keep = (0x00, 0x00, 0x00, net_rx_temp_data.keep);
				temp_mem_data.last = net_rx_temp_data.last;
				oMemWrData.write(temp_mem_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS;
			}
		break;
		case APP_MEM_WR_STS:
			if(!iMemWrtatus.empty()){
				iMemWrtatus.read();
				app_state = APP_MEM_RD_CMD;
			}
		break;
		case APP_MEM_RD_CMD:
			if(!oMemRdCmd.full()){
				oMemRdCmd.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD;
			}
		break;
		case APP_MEM_RD:
			if(!iMemRdData.empty() && !mem_rd_data.full() ){
				iMemRdData.read(temp_mem_data);
				net_rx_temp_data.data(63,0) = temp_mem_data.data(63,0);
				net_rx_temp_data.keep = temp_mem_data.keep(7,0);
				net_rx_temp_data.last = temp_mem_data.last;
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;
		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last) {
					app_state = APP_NET_RX_IDLE;
					num_net_rx_data_bytes = 0;
				}
			}
		break;
	}
}
#endif


#if 0
void echo_app_with_ddr3(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,
	//mem wr
	stream<mmCmd>&				oMemWrCmd,
	stream<mmStatus>&			iMemWrtatus,
	stream<axiWord>&			oMemWrData,

	//mem rd
	stream<mmCmd>&				oMemRdCmd,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle s_axis_net_rx_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle s_axis_net_tx_data"
#pragma HLS resource core=AXI4Stream variable=oMemWrCmd metadata="-bus_bundle s_axis_mem_wr_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus metadata="-bus_bundle s_axis_mem_wr_sts"
#pragma HLS resource core=AXI4Stream variable=oMemWrData metadata="-bus_bundle s_axis_mem_wr_data"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd metadata="-bus_bundle s_axis_mem_rd_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemRdData metadata="-bus_bundle s_axis_mem_rd_data"

#pragma HLS DATA_PACK variable=oMemWrCmd
#pragma HLS DATA_PACK variable=oMemRdCmd
#pragma HLS DATA_PACK variable=iMemWrtatus


static enum state { APP_NET_RX_IDLE = 0, APP_MEM_WR_CMD, APP_MEM_WR, APP_MEM_WR_STS, APP_MEM_RD_CMD, APP_MEM_RD, APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
static ap_uint<4> protocol = 0;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;


	switch(app_state){
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_CMD;

			}
			break;
		case APP_MEM_WR_CMD:
			if (!oMemWrCmd.full()) {
				oMemWrCmd.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR;
			}
			break;
		case APP_MEM_WR:
			if (!net_rx_data.empty() && !oMemWrData.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS;
			}
		break;
		case APP_MEM_WR_STS:
			if(!iMemWrtatus.empty()){
				iMemWrtatus.read();
				app_state = APP_MEM_RD_CMD;
			}
		break;
		case APP_MEM_RD_CMD:
			if(!oMemRdCmd.full()){
				oMemRdCmd.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD;
			}
		break;
		case APP_MEM_RD:
			if(!iMemRdData.empty() && !mem_rd_data.full() ){
				iMemRdData.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;
		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last) {
					app_state = APP_NET_RX_IDLE;
					num_net_rx_data_bytes = 0;
				}
			}
		break;
	}
}
#endif



#if 0
void echo_app_net_rx_ddr_wr_ddr_rd_net_tx_2_ports(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,

	//port 1
	//mem wr
	stream<mmCmd>&				oMemWrCmd_1,
	stream<mmStatus>&			iMemWrtatus_1,
	stream<axiWord>&			oMemWrData_1,
	//mem rd
	stream<mmCmd>&				oMemRdCmd_1,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData_1,

	//port 2
	//mem wr
	stream<mmCmd>&				oMemWrCmd_2,
	stream<mmStatus>&			iMemWrtatus_2,
	stream<axiWord>&			oMemWrData_2,
	//mem rd
	stream<mmCmd>&				oMemRdCmd_2,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData_2)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

/*
#pragma HLS INTERFACE axis port=iNetRxData
#pragma HLS INTERFACE axis port=oNetTxData

#pragma HLS INTERFACE axis port=oMemWrCmd
#pragma HLS INTERFACE axis port=iMemWrtatus
#pragma HLS INTERFACE axis port=oMemWrData
#pragma HLS INTERFACE axis port=oMemRdCmd
#pragma HLS INTERFACE axis port=iMemRdData
*/

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle s_axis_net_rx_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle s_axis_net_tx_data"

#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_1 metadata="-bus_bundle s_axis_mem_wr_cmd_1"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_1 metadata="-bus_bundle s_axis_mem_wr_sts_1"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_1 metadata="-bus_bundle s_axis_mem_wr_data_1"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_1 metadata="-bus_bundle s_axis_mem_rd_cmd_1"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_1 metadata="-bus_bundle s_axis_mem_rd_data_1"

#pragma HLS DATA_PACK variable=oMemWrCmd_1
#pragma HLS DATA_PACK variable=oMemRdCmd_1
#pragma HLS DATA_PACK variable=iMemWrtatus_1

#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_2 metadata="-bus_bundle s_axis_mem_wr_cmd_2"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_2 metadata="-bus_bundle s_axis_mem_wr_sts_2"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_2 metadata="-bus_bundle s_axis_mem_wr_data_2"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_2 metadata="-bus_bundle s_axis_mem_rd_cmd_2"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_2 metadata="-bus_bundle s_axis_mem_rd_data_2"

#pragma HLS DATA_PACK variable=oMemWrCmd_2
#pragma HLS DATA_PACK variable=oMemRdCmd_2
#pragma HLS DATA_PACK variable=iMemWrtatus_2



static enum state { APP_NET_RX_IDLE = 0,
					APP_MEM_WR_CMD_1,
					APP_MEM_WR_1,
					APP_MEM_WR_STS_1,
					APP_MEM_RD_CMD_1,
					APP_MEM_RD_1,

					APP_MEM_WR_CMD_2,
					APP_MEM_WR_2,
					APP_MEM_WR_STS_2,
					APP_MEM_RD_CMD_2,
					APP_MEM_RD_2,

					APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
static ap_uint<4> protocol = 0;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;
static bool port_switch = true;


	switch(app_state){
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last){
					if(port_switch)
						app_state = APP_MEM_WR_CMD_1;
					else
						app_state = APP_MEM_WR_CMD_2;

				port_switch = !port_switch;
				}
			}
		break;
		//port 1
		case APP_MEM_WR_CMD_1:
			if (!oMemWrCmd_1.full()) {
				oMemWrCmd_1.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_1;
			}
			break;
		case APP_MEM_WR_1:
			if (!net_rx_data.empty() && !oMemWrData_1.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData_1.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_1;
			}
		break;
		case APP_MEM_WR_STS_1:
			if(!iMemWrtatus_1.empty()){
				iMemWrtatus_1.read();
				app_state = APP_MEM_RD_CMD_1;
			}
		break;
		case APP_MEM_RD_CMD_1:
			if(!oMemRdCmd_1.full()){
				oMemRdCmd_1.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_1;
			}
		break;
		case APP_MEM_RD_1:
			if(!iMemRdData_1.empty() && !mem_rd_data.full() ){
				iMemRdData_1.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;

		//port 2
		case APP_MEM_WR_CMD_2:
			if (!oMemWrCmd_2.full()) {
				oMemWrCmd_2.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_2;
			}
			break;
		case APP_MEM_WR_2:
			if (!net_rx_data.empty() && !oMemWrData_2.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData_2.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_2;
			}
		break;
		case APP_MEM_WR_STS_2:
			if(!iMemWrtatus_2.empty()){
				iMemWrtatus_2.read();
				app_state = APP_MEM_RD_CMD_2;
			}
		break;
		case APP_MEM_RD_CMD_2:
			if(!oMemRdCmd_2.full()){
				oMemRdCmd_2.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_2;
			}
		break;
		case APP_MEM_RD_2:
			if(!iMemRdData_2.empty() && !mem_rd_data.full() ){
				iMemRdData_2.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;

		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last) {
					app_state = APP_NET_RX_IDLE;
					num_net_rx_data_bytes = 0;
				}
			}
		break;
	}
}
#endif

void echo_app_net_rx_ddr_wr_ddr_rd_net_tx_2_ports_mem_address_in_net_data(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,

	//port 1
	//mem wr
	stream<mmCmd>&				oMemWrCmd_1,
	stream<mmStatus>&			iMemWrtatus_1,
	stream<axiWord>&			oMemWrData_1,
	//mem rd
	stream<mmCmd>&				oMemRdCmd_1,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData_1,

	//port 2
	//mem wr
	stream<mmCmd>&				oMemWrCmd_2,
	stream<mmStatus>&			iMemWrtatus_2,
	stream<axiWord>&			oMemWrData_2,
	//mem rd
	stream<mmCmd>&				oMemRdCmd_2,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData_2)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW interval=1

/*
#pragma HLS INTERFACE axis port=iNetRxData
#pragma HLS INTERFACE axis port=oNetTxData

#pragma HLS INTERFACE axis port=oMemWrCmd
#pragma HLS INTERFACE axis port=iMemWrtatus
#pragma HLS INTERFACE axis port=oMemWrData
#pragma HLS INTERFACE axis port=oMemRdCmd
#pragma HLS INTERFACE axis port=iMemRdData
*/

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle s_axis_net_rx_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle s_axis_net_tx_data"

#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_1 metadata="-bus_bundle s_axis_mem_wr_cmd_1"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_1 metadata="-bus_bundle s_axis_mem_wr_sts_1"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_1 metadata="-bus_bundle s_axis_mem_wr_data_1"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_1 metadata="-bus_bundle s_axis_mem_rd_cmd_1"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_1 metadata="-bus_bundle s_axis_mem_rd_data_1"

#pragma HLS DATA_PACK variable=oMemWrCmd_1
#pragma HLS DATA_PACK variable=oMemRdCmd_1
#pragma HLS DATA_PACK variable=iMemWrtatus_1

#pragma HLS resource core=AXI4Stream variable=oMemWrCmd_2 metadata="-bus_bundle s_axis_mem_wr_cmd_2"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus_2 metadata="-bus_bundle s_axis_mem_wr_sts_2"
#pragma HLS resource core=AXI4Stream variable=oMemWrData_2 metadata="-bus_bundle s_axis_mem_wr_data_2"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd_2 metadata="-bus_bundle s_axis_mem_rd_cmd_2"
#pragma HLS resource core=AXI4Stream variable=iMemRdData_2 metadata="-bus_bundle s_axis_mem_rd_data_2"

#pragma HLS DATA_PACK variable=oMemWrCmd_2
#pragma HLS DATA_PACK variable=oMemRdCmd_2
#pragma HLS DATA_PACK variable=iMemWrtatus_2



static enum state { APP_NET_RX_IDLE = 0,
					APP_NET_RX_DATA,
					APP_MEM_WR_CMD_1,
					APP_MEM_WR_1,
					APP_MEM_WR_STS_1,
					APP_MEM_RD_CMD_1,
					APP_MEM_RD_1,

					APP_MEM_WR_CMD_2,
					APP_MEM_WR_2,
					APP_MEM_WR_STS_2,
					APP_MEM_RD_CMD_2,
					APP_MEM_RD_2,

					APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
static ap_uint<4> protocol = 0;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;
static bool port_switch = true;


	switch(app_state){
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !mem_rd_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				mem_wr_addr(31,0) = (net_rx_temp_data.data(7,0),net_rx_temp_data.data(15,8),net_rx_temp_data.data(23,16),net_rx_temp_data.data(31,24));
				if(port_switch) {
					mem_rd_data.write(axiWord(0x00000001, 0xff, 0x0));
				}else {
					mem_rd_data.write(axiWord(0x00000002, 0xff, 0x0));
				}
				app_state = APP_NET_RX_DATA;
			}
		break;
		case APP_NET_RX_DATA:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last){
					if(port_switch)
						app_state = APP_MEM_WR_CMD_1;
					else
						app_state = APP_MEM_WR_CMD_2;

					port_switch = !port_switch;
				}
			}
		break;
		//port 1
		case APP_MEM_WR_CMD_1:
			if (!oMemWrCmd_1.full()) {
				oMemWrCmd_1.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_1;
			}
			break;
		case APP_MEM_WR_1:
			if (!net_rx_data.empty() && !oMemWrData_1.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData_1.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_1;
			}
		break;
		case APP_MEM_WR_STS_1:
			if(!iMemWrtatus_1.empty()){
				iMemWrtatus_1.read();
				app_state = APP_MEM_RD_CMD_1;
			}
		break;
		case APP_MEM_RD_CMD_1:
			if(!oMemRdCmd_1.full()){
				oMemRdCmd_1.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_1;
			}
		break;
		case APP_MEM_RD_1:
			if(!iMemRdData_1.empty() && !mem_rd_data.full() ){
				iMemRdData_1.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;

		//port 2
		case APP_MEM_WR_CMD_2:
			if (!oMemWrCmd_2.full()) {
				oMemWrCmd_2.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR_2;
			}
			break;
		case APP_MEM_WR_2:
			if (!net_rx_data.empty() && !oMemWrData_2.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData_2.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS_2;
			}
		break;
		case APP_MEM_WR_STS_2:
			if(!iMemWrtatus_2.empty()){
				iMemWrtatus_2.read();
				app_state = APP_MEM_RD_CMD_2;
			}
		break;
		case APP_MEM_RD_CMD_2:
			if(!oMemRdCmd_2.full()){
				oMemRdCmd_2.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD_2;
			}
		break;
		case APP_MEM_RD_2:
			if(!iMemRdData_2.empty() && !mem_rd_data.full() ){
				iMemRdData_2.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;

		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last) {
					app_state = APP_NET_RX_IDLE;
					num_net_rx_data_bytes = 0;
				}
			}
		break;
	}
}



#if 0
void echo_app_net_rx_ddr_wr_ddr_rd_net_tx(
	//net rx
	stream<axiWord>& 			iNetRxData,
	//net tx
	stream<axiWord>& 			oNetTxData,
	//mem wr
	stream<mmCmd>&				oMemWrCmd,
	stream<mmStatus>&			iMemWrtatus,
	stream<axiWord>&			oMemWrData,

	//mem rd
	stream<mmCmd>&				oMemRdCmd,
	//stream<mmStatus>&			iMemRdtatus,
	stream<axiWord>&			iMemRdData)
{

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW //interval=1

/*
#pragma HLS INTERFACE axis port=iNetRxData
#pragma HLS INTERFACE axis port=oNetTxData

#pragma HLS INTERFACE axis port=oMemWrCmd
#pragma HLS INTERFACE axis port=iMemWrtatus
#pragma HLS INTERFACE axis port=oMemWrData
#pragma HLS INTERFACE axis port=oMemRdCmd
#pragma HLS INTERFACE axis port=iMemRdData
*/

#pragma HLS resource core=AXI4Stream variable=iNetRxData metadata="-bus_bundle s_axis_net_rx_data"
#pragma HLS resource core=AXI4Stream variable=oNetTxData metadata="-bus_bundle s_axis_net_tx_data"
#pragma HLS resource core=AXI4Stream variable=oMemWrCmd metadata="-bus_bundle s_axis_mem_wr_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemWrtatus metadata="-bus_bundle s_axis_mem_wr_sts"
#pragma HLS resource core=AXI4Stream variable=oMemWrData metadata="-bus_bundle s_axis_mem_wr_data"
#pragma HLS resource core=AXI4Stream variable=oMemRdCmd metadata="-bus_bundle s_axis_mem_rd_cmd"
#pragma HLS resource core=AXI4Stream variable=iMemRdData metadata="-bus_bundle s_axis_mem_rd_data"

#pragma HLS DATA_PACK variable=oMemWrCmd
#pragma HLS DATA_PACK variable=oMemRdCmd
#pragma HLS DATA_PACK variable=iMemWrtatus


static enum state { APP_NET_RX_IDLE = 0, APP_MEM_WR_CMD, APP_MEM_WR, APP_MEM_WR_STS, APP_MEM_RD_CMD, APP_MEM_RD, APP_NET_TX} app_state;

static stream<axiWord> net_rx_data;
#pragma HLS STREAM variable=net_rx_data depth=1024
static stream<axiWord> mem_rd_data;
#pragma HLS STREAM variable=mem_rd_data depth=1024
axiWord net_rx_temp_data;
static ap_uint<16> num_net_rx_data_bytes = 0;
static ap_uint<32> mem_wr_addr= 0x100;


	switch(app_state){
		case APP_NET_RX_IDLE:
			if (!iNetRxData.empty() && !net_rx_data.full()) {
				iNetRxData.read(net_rx_temp_data);
				net_rx_data.write(net_rx_temp_data);
				num_net_rx_data_bytes = num_net_rx_data_bytes + keep_to_len(net_rx_temp_data.keep);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_CMD;

			}
			break;
		case APP_MEM_WR_CMD:
			if (!oMemWrCmd.full()) {
				oMemWrCmd.write(mmCmd(mem_wr_addr, num_net_rx_data_bytes));
				app_state = APP_MEM_WR;
			}
			break;
		case APP_MEM_WR:
			if (!net_rx_data.empty() && !oMemWrData.full()) {
				net_rx_data.read(net_rx_temp_data);
				oMemWrData.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_MEM_WR_STS;
			}
		break;
		case APP_MEM_WR_STS:
			if(!iMemWrtatus.empty()){
				iMemWrtatus.read();
				app_state = APP_MEM_RD_CMD;
			}
		break;
		case APP_MEM_RD_CMD:
			if(!oMemRdCmd.full()){
				oMemRdCmd.write(mmCmd(mem_wr_addr,num_net_rx_data_bytes));
				app_state = APP_MEM_RD;
			}
		break;
		case APP_MEM_RD:
			if(!iMemRdData.empty() && !mem_rd_data.full() ){
				iMemRdData.read(net_rx_temp_data);
				mem_rd_data.write(net_rx_temp_data);

				if(net_rx_temp_data.last)
					app_state = APP_NET_TX;
			}
		break;
		case APP_NET_TX:
			if(!mem_rd_data.empty() && !oNetTxData.full()){
				mem_rd_data.read(net_rx_temp_data);
				oNetTxData.write(net_rx_temp_data);
				if(net_rx_temp_data.last)
					app_state = APP_NET_RX_IDLE;
			}
		break;
	}
}
#endif


//#endif

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

