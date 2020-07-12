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

/*****************************************************************************
 * @file       : test_toe.hpp
 * @brief      : Testbench for the TCP Offload Engine (TOE)
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Transport Stack (NTS)
 * Language    : Vivado HLS
 *
 * \ingroup NTS
 * \addtogroup NTS_TOE
 * \{
 *****************************************************************************/

#ifndef _TEST_TOE_H_
#define _TEST_TOE_H_

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unistd.h>

#include "../src/toe.hpp"
#include "../../../NTS/nts_utils.hpp"
#include "../../../NTS/SimNtsUtils.hpp"
#include "../../../NTS/toecam/src/toecam.hpp"
#include "../../../NTS/SimIp4Packet.hpp"

//#include "../../../NTS/toe/test/dummy_memory/dummy_memory.hpp"
//OBSOLETE_20200630 #include "../../toe/src/session_lookup_controller/session_lookup_controller.hpp"
//OBSOLETE_20200630 #include "../../toe/src/tx_app_interface/tx_app_interface.hpp"


//---------------------------------------------------------
//-- TESTBENCH GLOBAL DEFINES
//    'STARTUP_DELAY' is used to delay the start of the [TB] functions.
//---------------------------------------------------------
#define TB_MAX_SIM_CYCLES 2500000
#define TB_STARTUP_DELAY  (TOE_SIZEOF_LISTEN_PORT_TABLE)
#define TB_GRACE_TIME     25   // Adds some cycles to drain the DUT before exiting

//---------------------------------------------------------
//-- DEFAULT LOCAL FPGA AND FOREIGN HOST SOCKETS
//--  By default, the following sockets will be used by the
//--  testbench, unless the user specifies new ones via the
//--  test vector files.
//---------------------------------------------------------
#define DEFAULT_FPGA_IP4_ADDR   0x0A0CC801  // TOE's local IP Address  = 10.12.200.01
#define DEFAULT_FPGA_LSN_PORT   0x0057      // TOE listens on port     =    87 (static  ports must be     0..32767)
#define DEFAULT_FPGA_SND_PORT   TOE_FIRST_EPHEMERAL_PORT_NUM // TOE's ephemeral port # = 32768

#define DEFAULT_HOST_IP4_ADDR   0x0A0CC832  // TB's foreign IP Address = 10.12.200.50
#define DEFAULT_HOST_LSN_PORT   0x0058      // TB listens on port      = 88
#define DEFAULT_HOST_SND_PORT   0x8058      // TB's ephemeral port #   = 32856

//---------------------------------------------------------
//-- TESTBENCH MODES OF OPERATION
//---------------------------------------------------------
enum TestingMode { RX_MODE='0', TX_MODE='1', BIDIR_MODE='2', ECHO_MODE='3' };

//---------------------------------------------------------
//-- C/RTL LATENCIES AND INITIAL INTERVALS
//--   Use numbers >= to those of the 'CoSimulation Report'
//---------------------------------------------------------
#define APP_RSP_LATENCY    10  // [FIXME - "ipRx_TwentyPkt.dat" will fail if latency goes down to 5.

#define MEM_RD_CMD_LATENCY 10
#define MEM_RD_DAT_LATENCY 10
#define MEM_RD_STS_LATENCY 10

#define MEM_WR_CMD_LATENCY 10
#define MEM_WR_DAT_LATENCY 10
#define MEM_WR_STS_LATENCY 10

#define CAM_LOOKUP_LATENCY  1
#define CAM_UPDATE_LATENCY 10

#define RTT_LINK           25

#define FPGA_CLIENT_CONNECT_TIMEOUT    250 // In clock cycles

//---------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//--  These variables might be updated/overwritten by the
//--  content of a test-vector file.
//---------------------------------------------------------
bool            gTraceEvent   = false;
bool            gFatalError   = false;
unsigned int    gSimCycCnt    = 0;
unsigned int    gMaxSimCycles = TB_STARTUP_DELAY + 1000;  // [FIXME - Should be computed by the TB]

Ip4Addr         gFpgaIp4Addr  = DEFAULT_FPGA_IP4_ADDR;  // IPv4 address (in NETWORK BYTE ORDER)
TcpPort         gFpgaLsnPort  = DEFAULT_FPGA_LSN_PORT;  // TCP  listen port
TcpPort         gFpgaSndPort  = TOE_FIRST_EPHEMERAL_PORT_NUM; // TCP source port
Ip4Addr         gHostIp4Addr  = DEFAULT_HOST_IP4_ADDR;  // IPv4 address (in NETWORK BYTE ORDER)
TcpPort         gHostLsnPort  = DEFAULT_HOST_LSN_PORT;  // TCP  listen port


/*******************************************************************
 * @brief Class Testbench Socket Address
 *  This class differs from the class 'AxiSockAddr' used by TOE from
 *  an ENDIANESS point of view. This class is ENDIAN independent as
 *  opposed to the one used by TOE which stores its data members in
 *  LITTLE-ENDIAN order.
 *******************************************************************/
/*** OBSOLETE ***
class TbSockAddr {  // Testbench Socket Address
  public:
    unsigned int addr;  // IPv4 address
    unsigned int port;  // TCP  port
    TbSockAddr() {}
    TbSockAddr(unsigned int addr, unsigned int port) :
        addr(addr), port(port) {}
};
****************/

/*******************************************************************
 * @brief Class Testbench Socket Pair
 *  This class differs from the class 'AxiSockAddr' used by TOE from
 *  an ENDIANESS point of view. This class is ENDIAN independent as
 *  opposed to the one used by TOE which stores its data members in
 *  LITTLE-ENDIAN order.
 *******************************************************************/
/*** OBSOLETE ***
class TbSocketPair {    // Socket Pair Association
  public:
    TbSockAddr  src;    // Source socket address in LITTLE-ENDIAN order !!!
    TbSockAddr  dst;    // Destination socket address in LITTLE-ENDIAN order !!!
    TbSocketPair() {}
    TbSocketPair(TbSockAddr src, TbSockAddr dst) :
        src(src), dst(dst) {}
};

inline bool operator < (TbSocketPair const &s1, TbSocketPair const &s2) {
        return ((s1.dst.addr < s2.dst.addr) ||
                (s1.dst.addr == s2.dst.addr && s1.src.addr < s2.src.addr));
}
******************/

#endif

/*! \} */
