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
 * @file       : AxisUdp.hpp
 * @brief      : A class to access UDP header fields within data chunks
 *               transmitted over an AXI4-Stream interface.
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Transport Stack (NTS)
 * Language    : Vivado HLS
 *
 *----------------------------------------------------------------------------
 *
 * @details : The User Datagram Protocol (UDP) fields defined in this class
 *  refer to the format generated by the 10GbE MAC of Xilinx which organizes
 *  its two 64-bit Rx and Tx interfaces into 8 lanes (see PG157).
 *  The result of this division into lanes, is that the UDP fields end up being
 *  stored in LITTLE-ENDIAN order instead of the initial BIG-ENDIAN order used
 *  to transmit bytes over the physical media.
 *  As an example, assume that the 16 bits of the UDP "Checksum" datagram has a
 *  value of '0xA1B2'. This field will be transmitted on the media in big-endian
 *  order .i.e, a '0xA1' followed by '0xB2'. However, this field will end up
 *  being ordered in little-endian mode (.i.e, 0xB2A1) by the AXI4-Stream
 *  interface of the 10GbE MAC.
 *
 *  Therefore, the format of a UDP datagram transferred over an AXI4-Stream
 *  interface of quadwords is done in LITTLE-ENDIAN and is mapped as follows:
 *
 *         6                   5                   4                   3                   2                   1                   0
 *   3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           Checksum            |            Length             |       Destination Port        |          Source Port          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                             Data                                                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * @info: What's this Little-Endian(LE) vs Big-Endian(BE) anyhow.
 *  FYI - The original source from Xilinx (available at:
 *   https://github.com/Xilinx/HLx_Examples/tree/master/Acceleration/tcp_ip) is
 *   entirely coded with respect to the above mapping of the media network
 *   stream  over a 64-bits interface in little-endian oder. This makes the initial
 *   code particularly difficult to read, maintain and test. Therefore, this
 *   class implements a set of methods to hide this complexity by accessing a
 *   raw Axis data streams as if it was encoded in the expected big-endian order.
 *
 * @warning : This class is to be used when a UDP datagram is aligned to a
 *  64-bit quadword. Refer to the methods of 'AxisIp4.hpp' to access the fields
 *  of a UDP datagram that is embedded into an IPv4 packet, or the the methods
 *  of 'AxisEth.hpp' to access the fields of an UDP datagram that is embedded
 *  into an IPv4 packet further embedded into an Ethernet frame.
 *
 *****************************************************************************/

#ifndef _AXIS_UDP_H_
#define _AXIS_UDP_H_

#include "AxisRaw.hpp"

/*********************************************************
 * UDP - HEADER SIZE.
 *  All UDP datagrams have a fix-sized header of 8 bytes
 *  and a variable-sized data section.
 *********************************************************/
#define UDP_HEADER_LEN  8           // In bytes

/*********************************************************
 * UDP - HEADER FIELDS IN LITTLE_ENDIAN (LE) ORDER.
 *********************************************************/
typedef ap_uint<16> LE_UdpSrcPort;  // UDP Source Port
typedef ap_uint<16> LE_UdpDstPort;  // UDP Destination Port
typedef ap_uint<16> LE_UdpPort;     // UDP source or destination Port
typedef ap_uint<16> LE_UdpLen;      // UDP header and data Length
typedef ap_uint<16> LE_UdpCsum;     // UDP header and data Checksum
typedef LE_tData    LE_UdpData;     // UDP Data unit of transfer

/*********************************************************
 * UDP - HEADER FIELDS IN NETWORK BYTE ORDER.
 *   Default Type Definitions (as used by HLS)
 *********************************************************/
typedef ap_uint<16> UdpSrcPort;     // UDP Source Port
typedef ap_uint<16> UdpDstPort;     // UDP Destination Port
typedef ap_uint<16> UdpPort;        // UDP source or destination Port
typedef ap_uint<16> UdpLen;         // UDP header and data Length
typedef ap_uint<16> UdpCsum;        // UDP header and data Checksum
typedef tData       UdpData;        // UDP Data unit of transfer
typedef tDataHalf   UdpDataHi;      // UDP High part of a data unit of transfer
typedef tDataHalf   UdpDataLo;      // UDP Low part of a data unit of transfer

/*********************************************************
 * UDP Data over AXI4-STREAMING
 *  As Encoded by the 10GbE MAC (.i.e LITTLE-ENDIAN order).
 *********************************************************/
class AxisUdp: public AxiWord {

  public:
    AxisUdp() {}
    AxisUdp(AxiWord axiWord) :
      AxiWord(axiWord.tdata, axiWord.tkeep, axiWord.tlast) {}
    AxisUdp(ap_uint<64> tdata, ap_uint<8> tkeep, ap_uint<1> tlast) :
      AxiWord(tdata, tkeep, tlast) {}

    // Set-Get the UDP Source Port
    void          setUdpSrcPort(UdpPort port)   {                  tdata.range(15,  0) = swapWord(port);  }
    UdpPort       getUdpSrcPort()               { return swapWord (tdata.range(15,  0));                  }
    LE_UdpPort getLE_UdpSrcPort()               {           return tdata.range(15,  0) ;                  }

    // Set-Get the UDP Destination Port
    void          setUdpDstPort(UdpPort port)   {                  tdata.range(31, 16) = swapWord(port);  }
    UdpPort       getUdpDstPort()               { return swapWord (tdata.range(31, 16));                  }
    LE_UdpPort getLE_UdpDstPort()               {           return tdata.range(31, 16);                   }

    // Set-Get the UDP length field
    void          setUdpLen(UdpLen length)      {                  tdata.range(47, 32) = swapWord(length);}
    UdpLen        getUdpLen()                   { return swapWord (tdata.range(47, 32));                  }
    LE_UdpLen     getLE_UdpLen()                   {           return tdata.range(47, 32);                   }

    // Set-Get the UDP Checksum field
    void          setUdpCsum(UdpCsum csum)      {                  tdata.range(63, 48) = swapWord(csum);  }
    UdpCsum       getUdpCsum()                  { return swapWord (tdata.range(63, 48));                  }

  private:
    // Swap the two bytes of a word (.i.e, 16 bits)
    ap_uint<16> swapWord(ap_uint<16> inpWord) {
      return (inpWord.range(7,0), inpWord.range(15, 8));
    }
    // Swap the four bytes of a double-word (.i.e, 32 bits)
    ap_uint<32> swapDWord(ap_uint<32> inpDWord) {
      return (inpDWord.range( 7, 0), inpDWord.range(15,  8),
              inpDWord.range(23,16), inpDWord.range(31, 24));
    }

}; // End of: AxisUdp

#endif
