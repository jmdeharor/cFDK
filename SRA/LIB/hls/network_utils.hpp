//  *
//  *                       cloudFPGA
//  *     Copyright IBM Research, All Rights Reserved
//  *    =============================================
//  *     Created: Apr 2019
//  *     Authors: FAB, WEI, NGL
//  *
//  *     Description:
//  *        This file contains network types and functions that are shared accross HLS cores. 
//  *

#ifndef _CF_NETWORK_UTILS_
#define _CF_NETWORK_UTILS_


#include <stdint.h>
#include "ap_int.h"
#include "ap_utils.h"
#include <hls_stream.h>

using namespace hls;

/*
 * A generic unsigned AXI4-Stream interface used all over the cloudFPGA place.
 */
 template<int D>
   struct Axis {
     ap_uint<D>       tdata;
     ap_uint<(D+7)/8> tkeep;
     ap_uint<1>       tlast;
     Axis() {}
     Axis(ap_uint<D> single_data) : tdata((ap_uint<D>)single_data), tkeep(1), tlast(1) {}
   };


 struct IPMeta {
   ap_uint<32> ipAddress;
   IPMeta() {}
   IPMeta(ap_uint<32> ip) : ipAddress(ip) {}
 };


/********************************************
 * Socket transport address.
 ********************************************/

struct SocketAddr {
     ap_uint<16>    port;   // Port in network byte order
     ap_uint<32>    addr;   // IPv4 address (or node_id)
};


/********************************************
 * Generic Streaming Interfaces.
 ********************************************/

typedef bool AxisAck;       // Acknowledgment over Axi4-Stream I/F


/********************************************
 * UDP Specific Streaming Interfaces.
 ********************************************/

struct UdpWord {            // UDP Streaming Chunk (i.e. 8 bytes)
    ap_uint<64>    tdata;
    ap_uint<8>     tkeep;
    ap_uint<1>     tlast;
    UdpWord()      {}
    UdpWord(ap_uint<64> tdata, ap_uint<8> tkeep, ap_uint<1> tlast) :
                   tdata(tdata), tkeep(tkeep), tlast(tlast) {}
};

struct UdpMeta {            // UDP Socket Pair Association
    SocketAddr      src;    // Source socket address
    SocketAddr      dst;    // Destination socket address 
    //UdpMeta()       {}
};

typedef ap_uint<16>     UdpPLen; // UDP Payload Length

typedef ap_uint<16>     UdpPort; // UDP Port Number


typedef ap_uint<16>     NrcPort; // UDP/TCP Port Number
typedef ap_uint<8>      NodeId;  // Cluster Node Id
#define MAX_CF_NODE_ID 128

struct NrcMeta {
  NodeId  dst_rank; //ATTENTION: don't use 'id' in a struct...will be ignored by axis directive and lead to segfaults...
  NrcPort dst_port;
  NodeId  src_rank;
  NrcPort src_port;
  //ap_uint<16> padding;
  NrcMeta() {}
  //"alphabetical order"
  NrcMeta(NodeId d_id, NrcPort d_port, NodeId s_id, NrcPort s_port) :
    dst_rank(d_id), dst_port(d_port), src_rank(s_id), src_port(s_port) {}
 };

//ATTENTION: split between NrcMeta and NrcMetaStream is necessary, due to flaws in Vivados hls::stream library
struct NrcMetaStream {
  NrcMeta tdata; 
  //ap_uint<(sizeof(NrcMeta)+7)/8> tkeep; TODO: sizeof seems not to work with ap_ctrl_none!
  ap_uint<6> tkeep;
  ap_uint<1> tlast;
  NrcMetaStream() {}
  NrcMetaStream(NrcMeta single_data) : tdata(single_data), tkeep(1), tlast(1) {}
};


// ================ some functions ==========


void convertAxisToNtsWidth(stream<Axis<8> > &small, Axis<64> &out);
void convertAxisToMpiWidth(Axis<64> big, stream<Axis<8> > &out);

ap_uint<32> bigEndianToInteger(ap_uint<8> *buffer, int lsb);
void integerToBigEndian(ap_uint<32> n, ap_uint<8> *bytes);


#endif


