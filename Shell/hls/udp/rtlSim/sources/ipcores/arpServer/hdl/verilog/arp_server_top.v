// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2014.4
// Copyright (C) 2014 Xilinx Inc. All rights reserved.
// 
// ==============================================================

`timescale 1 ns / 1 ps
module arp_server_top (
arpDataIn_TVALID,
arpDataIn_TREADY,
arpDataIn_TDATA,
arpDataIn_TKEEP,
arpDataIn_TLAST,
arpDataOut_TVALID,
arpDataOut_TREADY,
arpDataOut_TDATA,
arpDataOut_TKEEP,
arpDataOut_TLAST,
macIpEncode_req_TVALID,
macIpEncode_req_TREADY,
macIpEncode_req_TDATA,
macIpEncode_rsp_TVALID,
macIpEncode_rsp_TREADY,
macIpEncode_rsp_TDATA,
macLookup_req_TVALID,
macLookup_req_TREADY,
macLookup_req_TDATA,
macLookup_resp_TVALID,
macLookup_resp_TREADY,
macLookup_resp_TDATA,
macUpdate_req_TVALID,
macUpdate_req_TREADY,
macUpdate_req_TDATA,
macUpdate_resp_TVALID,
macUpdate_resp_TREADY,
macUpdate_resp_TDATA,
aresetn,
aclk,
regIpAddress_V,
myMacAddress_V
);

parameter RESET_ACTIVE_LOW = 1;

input arpDataIn_TVALID ;
output arpDataIn_TREADY ;
input [64 - 1:0] arpDataIn_TDATA ;
input [8 - 1:0] arpDataIn_TKEEP ;
input [1 - 1:0] arpDataIn_TLAST ;


output arpDataOut_TVALID ;
input arpDataOut_TREADY ;
output [64 - 1:0] arpDataOut_TDATA ;
output [8 - 1:0] arpDataOut_TKEEP ;
output [1 - 1:0] arpDataOut_TLAST ;


input macIpEncode_req_TVALID ;
output macIpEncode_req_TREADY ;
input [32 - 1:0] macIpEncode_req_TDATA ;


output macIpEncode_rsp_TVALID ;
input macIpEncode_rsp_TREADY ;
output [56 - 1:0] macIpEncode_rsp_TDATA ;


output macLookup_req_TVALID ;
input macLookup_req_TREADY ;
output [40 - 1:0] macLookup_req_TDATA ;


input macLookup_resp_TVALID ;
output macLookup_resp_TREADY ;
input [56 - 1:0] macLookup_resp_TDATA ;


output macUpdate_req_TVALID ;
input macUpdate_req_TREADY ;
output [88 - 1:0] macUpdate_req_TDATA ;


input macUpdate_resp_TVALID ;
output macUpdate_resp_TREADY ;
input [56 - 1:0] macUpdate_resp_TDATA ;

input aresetn ;

input aclk ;

input [32 - 1:0] regIpAddress_V ;
input [48 - 1:0] myMacAddress_V ;


wire arpDataIn_TVALID;
wire arpDataIn_TREADY;
wire [64 - 1:0] arpDataIn_TDATA;
wire [8 - 1:0] arpDataIn_TKEEP;
wire [1 - 1:0] arpDataIn_TLAST;


wire arpDataOut_TVALID;
wire arpDataOut_TREADY;
wire [64 - 1:0] arpDataOut_TDATA;
wire [8 - 1:0] arpDataOut_TKEEP;
wire [1 - 1:0] arpDataOut_TLAST;


wire macIpEncode_req_TVALID;
wire macIpEncode_req_TREADY;
wire [32 - 1:0] macIpEncode_req_TDATA;


wire macIpEncode_rsp_TVALID;
wire macIpEncode_rsp_TREADY;
wire [56 - 1:0] macIpEncode_rsp_TDATA;


wire macLookup_req_TVALID;
wire macLookup_req_TREADY;
wire [40 - 1:0] macLookup_req_TDATA;


wire macLookup_resp_TVALID;
wire macLookup_resp_TREADY;
wire [56 - 1:0] macLookup_resp_TDATA;


wire macUpdate_req_TVALID;
wire macUpdate_req_TREADY;
wire [88 - 1:0] macUpdate_req_TDATA;


wire macUpdate_resp_TVALID;
wire macUpdate_resp_TREADY;
wire [56 - 1:0] macUpdate_resp_TDATA;

wire aresetn;


wire [64 - 1:0] sig_arp_server_arpDataIn_V_data_V_dout;
wire sig_arp_server_arpDataIn_V_data_V_empty_n;
wire sig_arp_server_arpDataIn_V_data_V_read;
wire [8 - 1:0] sig_arp_server_arpDataIn_V_keep_V_dout;
wire sig_arp_server_arpDataIn_V_keep_V_empty_n;
wire sig_arp_server_arpDataIn_V_keep_V_read;
wire [1 - 1:0] sig_arp_server_arpDataIn_V_last_V_dout;
wire sig_arp_server_arpDataIn_V_last_V_empty_n;
wire sig_arp_server_arpDataIn_V_last_V_read;

wire [64 - 1:0] sig_arp_server_arpDataOut_V_data_V_din;
wire sig_arp_server_arpDataOut_V_data_V_full_n;
wire sig_arp_server_arpDataOut_V_data_V_write;
wire [8 - 1:0] sig_arp_server_arpDataOut_V_keep_V_din;
wire sig_arp_server_arpDataOut_V_keep_V_full_n;
wire sig_arp_server_arpDataOut_V_keep_V_write;
wire [1 - 1:0] sig_arp_server_arpDataOut_V_last_V_din;
wire sig_arp_server_arpDataOut_V_last_V_full_n;
wire sig_arp_server_arpDataOut_V_last_V_write;

wire [32 - 1:0] sig_arp_server_macIpEncode_req_V_V_dout;
wire sig_arp_server_macIpEncode_req_V_V_empty_n;
wire sig_arp_server_macIpEncode_req_V_V_read;

wire [49 - 1:0] sig_arp_server_macIpEncode_rsp_V_din;
wire sig_arp_server_macIpEncode_rsp_V_full_n;
wire sig_arp_server_macIpEncode_rsp_V_write;

wire [33 - 1:0] sig_arp_server_macLookup_req_V_din;
wire sig_arp_server_macLookup_req_V_full_n;
wire sig_arp_server_macLookup_req_V_write;

wire [49 - 1:0] sig_arp_server_macLookup_resp_V_dout;
wire sig_arp_server_macLookup_resp_V_empty_n;
wire sig_arp_server_macLookup_resp_V_read;

wire [82 - 1:0] sig_arp_server_macUpdate_req_V_din;
wire sig_arp_server_macUpdate_req_V_full_n;
wire sig_arp_server_macUpdate_req_V_write;

wire [50 - 1:0] sig_arp_server_macUpdate_resp_V_dout;
wire sig_arp_server_macUpdate_resp_V_empty_n;
wire sig_arp_server_macUpdate_resp_V_read;

wire sig_arp_server_ap_rst;



arp_server arp_server_U(
    .arpDataIn_V_data_V_dout(sig_arp_server_arpDataIn_V_data_V_dout),
    .arpDataIn_V_data_V_empty_n(sig_arp_server_arpDataIn_V_data_V_empty_n),
    .arpDataIn_V_data_V_read(sig_arp_server_arpDataIn_V_data_V_read),
    .arpDataIn_V_keep_V_dout(sig_arp_server_arpDataIn_V_keep_V_dout),
    .arpDataIn_V_keep_V_empty_n(sig_arp_server_arpDataIn_V_keep_V_empty_n),
    .arpDataIn_V_keep_V_read(sig_arp_server_arpDataIn_V_keep_V_read),
    .arpDataIn_V_last_V_dout(sig_arp_server_arpDataIn_V_last_V_dout),
    .arpDataIn_V_last_V_empty_n(sig_arp_server_arpDataIn_V_last_V_empty_n),
    .arpDataIn_V_last_V_read(sig_arp_server_arpDataIn_V_last_V_read),
    .arpDataOut_V_data_V_din(sig_arp_server_arpDataOut_V_data_V_din),
    .arpDataOut_V_data_V_full_n(sig_arp_server_arpDataOut_V_data_V_full_n),
    .arpDataOut_V_data_V_write(sig_arp_server_arpDataOut_V_data_V_write),
    .arpDataOut_V_keep_V_din(sig_arp_server_arpDataOut_V_keep_V_din),
    .arpDataOut_V_keep_V_full_n(sig_arp_server_arpDataOut_V_keep_V_full_n),
    .arpDataOut_V_keep_V_write(sig_arp_server_arpDataOut_V_keep_V_write),
    .arpDataOut_V_last_V_din(sig_arp_server_arpDataOut_V_last_V_din),
    .arpDataOut_V_last_V_full_n(sig_arp_server_arpDataOut_V_last_V_full_n),
    .arpDataOut_V_last_V_write(sig_arp_server_arpDataOut_V_last_V_write),
    .macIpEncode_req_V_V_dout(sig_arp_server_macIpEncode_req_V_V_dout),
    .macIpEncode_req_V_V_empty_n(sig_arp_server_macIpEncode_req_V_V_empty_n),
    .macIpEncode_req_V_V_read(sig_arp_server_macIpEncode_req_V_V_read),
    .macIpEncode_rsp_V_din(sig_arp_server_macIpEncode_rsp_V_din),
    .macIpEncode_rsp_V_full_n(sig_arp_server_macIpEncode_rsp_V_full_n),
    .macIpEncode_rsp_V_write(sig_arp_server_macIpEncode_rsp_V_write),
    .macLookup_req_V_din(sig_arp_server_macLookup_req_V_din),
    .macLookup_req_V_full_n(sig_arp_server_macLookup_req_V_full_n),
    .macLookup_req_V_write(sig_arp_server_macLookup_req_V_write),
    .macLookup_resp_V_dout(sig_arp_server_macLookup_resp_V_dout),
    .macLookup_resp_V_empty_n(sig_arp_server_macLookup_resp_V_empty_n),
    .macLookup_resp_V_read(sig_arp_server_macLookup_resp_V_read),
    .macUpdate_req_V_din(sig_arp_server_macUpdate_req_V_din),
    .macUpdate_req_V_full_n(sig_arp_server_macUpdate_req_V_full_n),
    .macUpdate_req_V_write(sig_arp_server_macUpdate_req_V_write),
    .macUpdate_resp_V_dout(sig_arp_server_macUpdate_resp_V_dout),
    .macUpdate_resp_V_empty_n(sig_arp_server_macUpdate_resp_V_empty_n),
    .macUpdate_resp_V_read(sig_arp_server_macUpdate_resp_V_read),
    .ap_rst(sig_arp_server_ap_rst),
    .ap_clk(aclk),
    .regIpAddress_V(regIpAddress_V),
    .myMacAddress_V(myMacAddress_V)
);

arp_server_arpDataIn_if arp_server_arpDataIn_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .arpDataIn_V_data_V_dout(sig_arp_server_arpDataIn_V_data_V_dout),
    .arpDataIn_V_data_V_empty_n(sig_arp_server_arpDataIn_V_data_V_empty_n),
    .arpDataIn_V_data_V_read(sig_arp_server_arpDataIn_V_data_V_read),
    .arpDataIn_V_keep_V_dout(sig_arp_server_arpDataIn_V_keep_V_dout),
    .arpDataIn_V_keep_V_empty_n(sig_arp_server_arpDataIn_V_keep_V_empty_n),
    .arpDataIn_V_keep_V_read(sig_arp_server_arpDataIn_V_keep_V_read),
    .arpDataIn_V_last_V_dout(sig_arp_server_arpDataIn_V_last_V_dout),
    .arpDataIn_V_last_V_empty_n(sig_arp_server_arpDataIn_V_last_V_empty_n),
    .arpDataIn_V_last_V_read(sig_arp_server_arpDataIn_V_last_V_read),
    .TVALID(arpDataIn_TVALID),
    .TREADY(arpDataIn_TREADY),
    .TDATA(arpDataIn_TDATA),
    .TKEEP(arpDataIn_TKEEP),
    .TLAST(arpDataIn_TLAST));

arp_server_arpDataOut_if arp_server_arpDataOut_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .arpDataOut_V_data_V_din(sig_arp_server_arpDataOut_V_data_V_din),
    .arpDataOut_V_data_V_full_n(sig_arp_server_arpDataOut_V_data_V_full_n),
    .arpDataOut_V_data_V_write(sig_arp_server_arpDataOut_V_data_V_write),
    .arpDataOut_V_keep_V_din(sig_arp_server_arpDataOut_V_keep_V_din),
    .arpDataOut_V_keep_V_full_n(sig_arp_server_arpDataOut_V_keep_V_full_n),
    .arpDataOut_V_keep_V_write(sig_arp_server_arpDataOut_V_keep_V_write),
    .arpDataOut_V_last_V_din(sig_arp_server_arpDataOut_V_last_V_din),
    .arpDataOut_V_last_V_full_n(sig_arp_server_arpDataOut_V_last_V_full_n),
    .arpDataOut_V_last_V_write(sig_arp_server_arpDataOut_V_last_V_write),
    .TVALID(arpDataOut_TVALID),
    .TREADY(arpDataOut_TREADY),
    .TDATA(arpDataOut_TDATA),
    .TKEEP(arpDataOut_TKEEP),
    .TLAST(arpDataOut_TLAST));

arp_server_macIpEncode_req_if arp_server_macIpEncode_req_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macIpEncode_req_V_V_dout(sig_arp_server_macIpEncode_req_V_V_dout),
    .macIpEncode_req_V_V_empty_n(sig_arp_server_macIpEncode_req_V_V_empty_n),
    .macIpEncode_req_V_V_read(sig_arp_server_macIpEncode_req_V_V_read),
    .TVALID(macIpEncode_req_TVALID),
    .TREADY(macIpEncode_req_TREADY),
    .TDATA(macIpEncode_req_TDATA));

arp_server_macIpEncode_rsp_if arp_server_macIpEncode_rsp_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macIpEncode_rsp_V_din(sig_arp_server_macIpEncode_rsp_V_din),
    .macIpEncode_rsp_V_full_n(sig_arp_server_macIpEncode_rsp_V_full_n),
    .macIpEncode_rsp_V_write(sig_arp_server_macIpEncode_rsp_V_write),
    .TVALID(macIpEncode_rsp_TVALID),
    .TREADY(macIpEncode_rsp_TREADY),
    .TDATA(macIpEncode_rsp_TDATA));

arp_server_macLookup_req_if arp_server_macLookup_req_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macLookup_req_V_din(sig_arp_server_macLookup_req_V_din),
    .macLookup_req_V_full_n(sig_arp_server_macLookup_req_V_full_n),
    .macLookup_req_V_write(sig_arp_server_macLookup_req_V_write),
    .TVALID(macLookup_req_TVALID),
    .TREADY(macLookup_req_TREADY),
    .TDATA(macLookup_req_TDATA));

arp_server_macLookup_resp_if arp_server_macLookup_resp_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macLookup_resp_V_dout(sig_arp_server_macLookup_resp_V_dout),
    .macLookup_resp_V_empty_n(sig_arp_server_macLookup_resp_V_empty_n),
    .macLookup_resp_V_read(sig_arp_server_macLookup_resp_V_read),
    .TVALID(macLookup_resp_TVALID),
    .TREADY(macLookup_resp_TREADY),
    .TDATA(macLookup_resp_TDATA));

arp_server_macUpdate_req_if arp_server_macUpdate_req_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macUpdate_req_V_din(sig_arp_server_macUpdate_req_V_din),
    .macUpdate_req_V_full_n(sig_arp_server_macUpdate_req_V_full_n),
    .macUpdate_req_V_write(sig_arp_server_macUpdate_req_V_write),
    .TVALID(macUpdate_req_TVALID),
    .TREADY(macUpdate_req_TREADY),
    .TDATA(macUpdate_req_TDATA));

arp_server_macUpdate_resp_if arp_server_macUpdate_resp_if_U(
    .ACLK(aclk),
    .ARESETN(aresetn),
    .macUpdate_resp_V_dout(sig_arp_server_macUpdate_resp_V_dout),
    .macUpdate_resp_V_empty_n(sig_arp_server_macUpdate_resp_V_empty_n),
    .macUpdate_resp_V_read(sig_arp_server_macUpdate_resp_V_read),
    .TVALID(macUpdate_resp_TVALID),
    .TREADY(macUpdate_resp_TREADY),
    .TDATA(macUpdate_resp_TDATA));

arp_server_ap_rst_if #(
    .RESET_ACTIVE_LOW(RESET_ACTIVE_LOW))
ap_rst_if_U(
    .dout(sig_arp_server_ap_rst),
    .din(aresetn));

endmodule
