/*******************************************************************************
 * Copyright 2016 -- 2021 IBM Corporation
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

/************************************************
Copyright (c) 2015, Xilinx, Inc.

All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************/


// *****************************************************************************
// *
// *                             cloudFPGA
// *
// *----------------------------------------------------------------------------
// *
// * Title : Toplevel of the 10G Ethernet interface instantiated by the SHELL. 
// *
// * File    : tenGigEth.v
// *
// * Created : Nov. 2017
// * Authors : Francois Abel <fab@zurich.ibm.com>
// *
// * Devices : xcku060-ffva1156-2-i
// * Tools   : Vivado v2016.4 (64-bit)
// * Depends : None
// *
// * Description : This is the toplevel design for the 10 Gigabit Ethernet I/F
// *    instantiated by the shell of the FMKU2595 module equipped with a XCKU60
// *    device. It is derived from the code of the example design generated by
// *    the Vivado software environment and as such, it contains Rx and Tx FIFO
// *    blocks that both implement an AXI4-Stream user interface through which
// *    the frame data are received and transmitted.
// * 
// *****************************************************************************

`timescale 1ps / 1ps

(* DowngradeIPIdentifiedWarnings = "yes" *)

// *****************************************************************************
// **  MODULE - 10G ETHERNET FOR FMKU60
// *****************************************************************************

module TenGigEth (
 
  //-- Clocks and Resets inputs ------------------
  input             piTOP_156_25Clk,    // Freerunning
  input             piCLKT_Gt_RefClk_n,
  input             piCLKT_Gt_RefClk_p,
  input             piTOP_Reset,

  //-- Clocks and Resets outputs -----------------
  output            poSHL_CoreClk,
  output            poSHL_CoreResetDone,

  //-- MMIO : Ctrl inputs and Status outputs -----
  input             piMMIO_RxEqualizerMode,
  input  [ 3:0]     piMMIO_TxDriverSwing,
  input  [ 4:0]     piMMIO_TxPreCursor,
  input  [ 4:0]     piMMIO_TxPostCursor,
  input             piMMIO_PcsLoopbackEn,
  output            poMMIO_CoreReady,
  output            poMMIO_QpllLock,
  
  //-- ECON : Gigabit Transceivers ---------------
  input             piECON_Gt_n,
  input             piECON_Gt_p,
  output            poECON_Gt_n,
  output            poECON_Gt_p,
   
  //-- LY3 : Layer-3 Input Interface -------------
  input     [63:0]  siLY3_Data_tdata,
  input     [7:0]   siLY3_Data_tkeep,
  input             siLY3_Data_tlast,
  input             siLY3_Data_tvalid,
  output            siLY3_Data_tready,
  
  //-- LY3 : Layer-3 Output Interface ------------
  output     [63:0] soLY3_Data_tdata,
  output     [7:0]  soLY3_Data_tkeep,
  output            soLY3_Data_tlast,
  output            soLY3_Data_tvalid,
   input            soLY3_Data_tready
  
  ); // End of PortList
   

// *****************************************************************************
// **  STRUCTURE
// *****************************************************************************

  // Set FIFO memory size
  localparam        FIFO_SIZE  = 1024;

  //============================================================================
  //  SIGNAL DECLARATIONS
  //============================================================================
  wire              sVlanEn;
  wire              sBlockLock;   
  wire              sCORE_Clk;
  wire              sCORE_GtRxClk;
   
  //-- Axi Lite Clock generator 
  (* keep="true" *) 
  wire              sALCG_DrpClk;
  wire              sALCG_DcmLocked;

  wire              sNoRemoteAndLocalFaults;
  wire    [79 : 0]  sMacTxConfigurationVector;
  wire    [79 : 0]  sMacRxConfigurationVector;
  wire     [2 : 0]  sCORE_MacStatusVector;
  wire   [535 : 0]  sPcsPmaConfigurationVector;
  wire   [447 : 0]  sCORE_PcsPmaStatusVector;

  wire              sTxStatBit;
  wire              sRxStatBit;
  wire     [25:0]   sCORE_TxStatVec;
  wire              sCORE_TxStatVal;
  reg               sTxStatValReg;
  reg      [27:0]   sTxStatVecReg = 0;
  wire              sSerializedStats;
   
  wire     [29:0]   sCORE_RxStatVec;
  wire              sCORE_RxStatVal;
  reg               sRxStatValReg;
  reg      [31:0]   sRxStatVecReg = 0;
   
  wire              sReset_n;
   
  wire              sCORE_ResetDone;
  wire      [7:0]   sCORE_MiscPcsPmaStatus;

  // Metastable signals
  wire              sMETA0_PcsLoopbackEn;
    
  //-- End of signal declarations ---------------


  //============================================================================
  //  COMB: CONTINIOUS ASSIGNMENTS
  //============================================================================
  
  // Active Low Reset
  assign sReset_n = ~piTOP_Reset;

  // Enable or disable VLAN mode
  assign sVlanEn = 0;

  // Set the configuration vectors
  assign sMacRxConfigurationVector = {72'd0, 1'd0, 4'd0, sVlanEn, 2'b10};
  assign sMacTxConfigurationVector = {72'd0, 1'd0, 4'd0, sVlanEn, 2'b10};
  assign sPcsPmaConfigurationVector = {425'd0,sMETA0_PcsLoopbackEn,110'd0};

  assign sBlockLock                 = sCORE_MiscPcsPmaStatus[0];
  assign sNoRemoteAndLocalFaults    = !sCORE_MacStatusVector[0] && !sCORE_MacStatusVector[1] ;

  // The serialized statistics wire is only intended to prevent logic stripping
  assign sSerializedStats = sTxStatBit || sRxStatBit;
  
  // Output Ports Assignments
  assign poSHL_CoreClk    = sCORE_Clk;
  assign poMMIO_CoreReady = sBlockLock && sNoRemoteAndLocalFaults;
 
 
  //============================================================================
  //  INST: ANTI-METASTABILITY BLOCKS
  //============================================================================
  TenGigEth_SyncBlock META0 (
    .data_in     (piMMIO_PcsLoopbackEn),
    .clk         (sCORE_Clk),
    .data_out    (sMETA0_PcsLoopbackEn)
  );
  
  
  //============================================================================
  //  INST: ETHERNET CORE = DATA LINK LAYER + PHYSICAL LAYER
  //============================================================================
  TenGigEth_Core # (
  
    .gEthId                           (0),  // Instanciate Ethernet I/F #0 (.i.e ETH0)
    .gAutoNeg                         (0),  // Use PCS/PMA in 10GBASE-R mode
    .FIFO_SIZE                        (FIFO_SIZE)
  
  ) CORE (
  
    //-- Clocks and Resets inputs ----------------
    .refclk_n                         (piCLKT_Gt_RefClk_n),
    .refclk_p                         (piCLKT_Gt_RefClk_p),
    .dclk                             (sALCG_DrpClk),
    .reset                            (piTOP_Reset),
    
    //-- Clocks and Resets outputs ---------------
    .resetdone_out                    (sCORE_ResetDone),
    .coreclk_out                      (sCORE_Clk),
    .rxrecclk_out                     (/* sCORE_GtRxClk */),
    .qplllock_out                     (poMMIO_QpllLock),

    //-- AXI4 Input Stream Interface -------------  
    .tx_axis_mac_aresetn              (sReset_n),
    .tx_axis_fifo_aresetn             (sReset_n),
    .tx_axis_fifo_tdata               (siLY3_Data_tdata),
    .tx_axis_fifo_tkeep               (siLY3_Data_tkeep),
    .tx_axis_fifo_tvalid              (siLY3_Data_tvalid),
    .tx_axis_fifo_tlast               (siLY3_Data_tlast),
    .tx_axis_fifo_tready              (siLY3_Data_tready),

    //-- AXI4 Output Stream Interface ------------
    .rx_axis_fifo_aresetn             (sReset_n),
    .rx_axis_mac_aresetn              (sReset_n),
    .rx_axis_fifo_tdata               (soLY3_Data_tdata),
    .rx_axis_fifo_tkeep               (soLY3_Data_tkeep),
    .rx_axis_fifo_tvalid              (soLY3_Data_tvalid),
    .rx_axis_fifo_tlast               (soLY3_Data_tlast),
    .rx_axis_fifo_tready              (soLY3_Data_tready),
    
    //-- ECON : Gigabit Transceivers -------------
    .txp                              (poECON_Gt_p),
    .txn                              (poECON_Gt_n),
    .rxp                              (piECON_Gt_p),
    .rxn                              (piECON_Gt_n),
    
    //---- GT Configuration and Status Signals
    .transceiver_debug_gt_rxlpmen     (piMMIO_RxEqualizerMode),  // 0:DFE or 1:LPM
    .transceiver_debug_gt_txdiffctrl  (piMMIO_TxDriverSwing),    // c.f. UG576
    .transceiver_debug_gt_txprecursor (piMMIO_TxPreCursor),      // c.f. UG576
    .transceiver_debug_gt_txpostcursor(piMMIO_TxPostCursor),     // c.f. UG576
    
    //-- PCS/PMA Configuration and Status Signals 
    .pcs_pma_configuration_vector     (sPcsPmaConfigurationVector),
    .pcs_pma_status_vector            (sCORE_PcsPmaStatusVector),

    //---- PCS/PMA Miscellaneous Ports -------------- 
    .pcspma_status                    (sCORE_MiscPcsPmaStatus),
        
    //-- MAC Configuration and Status Signals ----
    .mac_tx_configuration_vector      (sMacTxConfigurationVector),
    .mac_rx_configuration_vector      (sMacRxConfigurationVector),
    .mac_status_vector                (sCORE_MacStatusVector),
    
    //-- Pause Control Interface ------------------
    .pause_val                        (16'b0),
    .pause_req                        (1'b0),
    
    //-- Optical Module Interface -----------------
    .signal_detect                    (1'b1),
    .tx_fault                         (1'b0),
    .tx_disable                       (/*NC*/),
 
    //-- Statistics Vectors Outputs --------------
    .tx_statistics_vector             (sCORE_TxStatVec),
    .tx_statistics_valid              (sCORE_TxStatVal),
    .rx_statistics_vector             (sCORE_RxStatVec),
    .rx_statistics_valid              (sCORE_RxStatVal),

    //-- OTHER : Ctrl and Status Signals --------- 
    .sim_speedup_control              (1'b0),
    .tx_ifg_delay                     (8'd0)
   
  );


  //==========================================================================
  //  INST: AXI-LITE/DRPCLK Clock Generator
  //==========================================================================
  TenGigEth_AxiLiteClk ALCG (
    .piClk                           (piTOP_156_25Clk),
    .poEthAxiLiteClk                 (sALCG_DrpClk),
    .poMmcmLocked                    (sALCG_DcmLocked)
  );


  //============================================================================
  //  PROC: SERIALISE CORE STATISTICS VECTORS 
  //    To ensure logic isn't stripped during synthesis and to reduce the IO
  //    required by this design.  
  //============================================================================
  always @(posedge sCORE_Clk)
  begin
    sTxStatValReg <= sCORE_TxStatVal;
    if (sCORE_TxStatVal & !sTxStatValReg) begin
       sTxStatVecReg <= {2'b01, sCORE_TxStatVec};
    end
    else begin
       sTxStatVecReg <= {sTxStatVecReg[26:0], 1'b0};
    end
  end

  assign sTxStatBit = sTxStatVecReg[27];

  always @(posedge sCORE_Clk)
  begin
    sRxStatValReg <= sCORE_RxStatVal;
    if (sCORE_RxStatVal & !sRxStatValReg) begin
      sRxStatVecReg <= {2'b01, sCORE_RxStatVec};
    end
    else begin
      sRxStatVecReg <= {sRxStatVecReg[30:0], 1'b0};
    end
  end

  assign sRxStatBit = sRxStatVecReg[31];
  
  //============================================================================
  //  COMB: CONTINUOUS OUTPUT PORT ASSIGNMENTS
  //============================================================================
  assign poSHL_CoreResetDone = sCORE_ResetDone;

endmodule
