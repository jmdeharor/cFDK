# ******************************************************************************
# * Copyright 2016 -- 2021 IBM Corporation
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# ******************************************************************************

# ******************************************************************************
# *
# *                                cloudFPGA
# *
# *-----------------------------------------------------------------------------
# *
# * Title   : Default timing constraints for the module FMKU60.
# *
# * File    : topFMKU60_timg_impl.xdc
# *
# * Tools   : Vivado v2016.4, v2017.4 v2019.2 (64-bit)
# *
# * Description : This file contains the timing constraints related to the
# *     operation of the FMKU60 module. Refer to 'topFMKU60_pins.xdc' and
# *     'topFMKU60.xdc' files for more specific physical constraints such
# *     as pin locations and voltage levels.
# *
# *     The constraints are grouped by board device and connector names:
# *       - the synchronous dynamic random access memory (SDRM or DDR4)
# *       - the programmable system on chip controller (PSOC)
# *       - the configuration Flash memory (FLASH)
# *       - the clock tree generator (CLKT)
# *       - the edge backplane connector (ECON)
# *       - the top extension connector (XCON)
# *
# *     The SDRM has the following interfaces:
# *       - a memory channel 0 (MC0)
# *       - a memory channel 1 (MC1)
# *     The PSOC has the following interfaces:
# *       - an External memory interface (PSOC_Emif)
# *       - an Fpga configuration interface (PSOC_Fcfg)
# *     The FLASH has the following interfaces:
# *       - a byte peripheral interface (FLASH_Bpi)
# *     The CLKT has the following interfaces:
# *       - two user clocks 0 and 1 (CLKT_Usr0 and CLKT_Usr1)
# *       - two DDR4 memory clocks 0 and 1 (CLKT_Mem0Clk, CLKT_Mem1Clk)
# *       - one gigabit transceiver clock (CLKT_10Ge)
# *     The ECON has the following interfaces:
# *       - one 10GbE Rx and Tx transceivers (ECON_10Ge0)
# *     The XCON is not used.
# *
# *-----------------------------------------------------------------------------
# * Constraints Methodology and Recommendations (according to Xilinx UG903)
# *   - Xilinx recommends to separate timing constraints and physical constraints
# *     by saving them into two distinct files.
# *   - Organize your constraints in the following sequence.
# *     [1] Timing Assertions Section
# *       - Primary clocks
# *       -  Virtual clocks
# *       -  Generated clocks
# *       -  Clock Groups
# *       -  Bus Skew constraints
# *       -  Input and output delay constraints
# *     [2] Timing Exceptions Section
# *       -  False Paths
# *       -  Max Delay / Min Delay
# *       -  Multicycle Paths
# *       -  Case Analysis
# *       -  Disable Timing
# *     [3] Physical Constraints Section
# *       -  located anywhere in the file, preferably before or after the timing constraints
# *       -  or stored in a separate constraint file
# *
# ******************************************************************************


################################################################################
#                                                                              #
#  WARNING: This file makes use of constants which are defined in a TCL file.  #
#           Please see the local file: 'xdc_settings.tcl'.                     #
#                                                                              #
################################################################################

#===============================================================================
# Create the Primary Root Clocks as generated by the Clock Tree (CLKT)
#===============================================================================

# CLKT / Reference clock for GTH transceivers of the 10GE Interface
#  [INFO] This clock is already constrained by the IP core.
#  create_clock -name piCLKT_10GeClk -period 6.400 [get_ports piCLKT_10GeClk_p]

# CLKT / Reference clock for the User clock #0
create_clock -period 6.400 -name piCLKT_Usr0Clk_p [get_ports piCLKT_Usr0Clk_p]

# CLKT / Reference clock for the User clock #1
# [NOT-USED] create_clock -period 4.000 -name piCLKT_Usr1Clk_p [get_ports piCLKT_Usr1Clk_p]

# CLKT / Reference clock for the DRAM block 0
#  [INFO] This clock is already constrained by the IP core.
# create_clock -name piCLKT_Mem0Clk -period 3.333 [get_ports piCLKT_Mem0Clk_p]

# CLKT / Reference clock for the DRAM block 1
#  [INFO] This clock is already constrained by the IP core.
# create_clock -name piCLKT_Mem1Clk -period 3.333 [get_ports piCLKT_Mem1Clk_p]

#===============================================================================
# Create the Primary Root Clock as generated by the EMIF bus I/F of the PSOC
#===============================================================================
create_clock -period 166.667 -name piPSOC_Emif_Clk -waveform {0.000 83.333} [get_ports piPSOC_Emif_Clk]

#===============================================================================
# Rename some Internally Generated Clocks for Convenience
#-------------------------------------------------------------------------------
#
# [OPTION-1] See UG903-Ch3-Section-"Renaming Auto-Derived Clocks"
#     create_generated_clock -name new_name source_object
#   The "source object" should be the same object that is used as the source of
#   the auto-derived clock. This can only be the pin wherethe clock originates,
#   such as at the output of a clock modifying blockm (PLL, MMCM, . . .).
#   In particular, an auto-derived clock cannot be renamed at the output of a
#   BUFG even though it propagates through it.
#     E.g., create_generated_clock -name DRP_CLK
#              [get_pins SHELL/SuperCfg.ETH0/ETH/ALCG/MMCME3_BASE_U0/CLKOUT0]
#
# [OPTION-2] Use a TCL variable
#   Another alternative is not to rename the clock, but to just use a variable
#   instead and refer to it whenever you need that clock.
#     E.g., set myDRP_CLK [get_clocks -of_objects
#              [get_pins SHELL/SuperCfg.ETH0/ETH/ALCG/MMCME3_BASE_U0/CLKOUT0]
#
#===============================================================================
create_generated_clock -name SHELL_CLK [get_pins SHELL/SuperCfg.ETH0/ETH/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O]
create_generated_clock -name ETH_RXCLK [get_pins SHELL/SuperCfg.ETH0/ETH/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_block_i/bd_b7e6_xpcs_0_local_clock_reset_block/rxusrclk2_bufg_gt_i/O]

create_generated_clock -name MC0_CLKOUT0 [get_pins SHELL/MEM/MC0/MCC/*/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0]
create_generated_clock -name MC1_CLKOUT0 [get_pins SHELL/MEM/MC1/MCC/*/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0]

create_generated_clock -name MC0_CLKOUT6 [get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT6]
create_generated_clock -name MC1_CLKOUT6 [get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT6]

create_generated_clock -name {SHELL/sEmifReg_reg[195]_bufg_place} -source [get_pins SHELL/SuperCfg.ETH0/ETH/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O] -divide_by 2 [get_pins SHELL/SW_RESET_LY3/DOUT]

#===============================================================================
# Create the Group Constraints among Clocks
#===============================================================================

#-- [MC0_CLKOUT0] -------------------------------
set_clock_groups -asynchronous -group MC0_CLKOUT0 -group SHELL_CLK

#-- [MC1_CLKOUT0] -------------------------------
set_clock_groups -asynchronous -group MC1_CLKOUT0 -group SHELL_CLK

#-- [MC0_CLKOUT6] -------------------------------

#-- [MC1_CLKOUT6] -------------------------------

#-- [piCLKT_10GeClk_p] --------------------------
set_max_delay -datapath_only -from piCLKT_10GeClk_p -to SHELL_CLK 6.400

#-- [piCLKT_Mem0Clk_p] --------------------------

#-- [piCLKT_Mem1Clk_p] --------------------------

#-- [piCLKT_Usr0Clk_p] --------------------------
set_clock_groups -asynchronous -group piCLKT_Usr0Clk_p -group SHELL_CLK

#-- [piCLKT_Usr1Clk_p] --------------------------
#   [This clock is not used by this design]

#-- [piPSOC_Emif_Clk] ---------------------------
set_max_delay -datapath_only -from piPSOC_Emif_Clk -to SHELL_CLK 20.000

#-- [sEthAxiLiteClk] ----------------------------

#-- [ETH_RXCLK] ---------------------------------

#-- [SHELL_CLK] ---------------------------------
set_max_delay -datapath_only -from SHELL_CLK -to piPSOC_Emif_Clk 20.000

#=====================================================================
# Constraints related to the Synchronous Dynamic RAM (DDR4)
#   DDR4 / Memory Channel #0 / Reset
#=====================================================================
set_max_delay -from [get_pins {SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C}] -to [get_ports poDDR4_Mem_Mc0_Reset_n] 10.000

#=====================================================================
# Constraints related to the Synchronous Dynamic RAM (DDR4)
#   DDR4 / Memory Channel #1 / Reset
#=====================================================================
set_max_delay -from [get_pins {SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C}] -to [get_ports poDDR4_Mem_Mc1_Reset_n] 10.000

#=====================================================================
# PSOC / External Memory Interface (see PSoC Creator Component v1.30)
#---------------------------------------------------------------------
#
#             +---+   +---+   +---+   +---+   +---+   +---+   +---+   +---+
# Bus Clock   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
#          ---+   +---+   +---+   +---+   +---+   +---+   +---+   +---+   +---
#
#                         +---+                           +---+
# EMIF Clk                |   |                           |   |
#          ---------------+   +---------------------------+   +---------------
#
#              +-----------------------------+ +------------------------------
# EMIF Addr --X                               X
#              +-----------------------------+ +------------------------------
#
#            ---------+       +-----------------------+       +---------------
# EMIF CTtl           |       |                       |       |
# (WE,CE,ADS)         +-------+                       +-------+
#
#          -------------------------------------------+               +------
# EMIF OE                                             |               |
#                                                     +---------------+
#
#                    +--------------------------------+             +----+
# EMIF DATA ---------|    Write Cycle                 |-------------| Rd |---
#                    +--------------------------------+             +----+
#
#=====================================================================

# PSoC / Emif - Address[7:0] - Write setup and hold times
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -min -add_delay 140.000 [get_ports {piPSOC_Emif_Addr[*]}]
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -max -add_delay 146.667 [get_ports {piPSOC_Emif_Addr[*]}]

# PSOC / Emif - Chip select - Access setup and hold times
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -min -add_delay 140.000 [get_ports piPSOC_Emif_Cs_n]
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -max -add_delay 146.667 [get_ports piPSOC_Emif_Cs_n]

# PSOC / Emif - Write enable - Write setup and hold times
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -min -add_delay 140.000 [get_ports piPSOC_Emif_We_n]
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -max -add_delay 146.667 [get_ports piPSOC_Emif_We_n]

# PSoC / Emif - Output enable - Read setup and hold times
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -min -add_delay 140.000 [get_ports piPSOC_Emif_Oe_n]
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -max -add_delay 146.667 [get_ports piPSOC_Emif_Oe_n]

# PSoC / Emif - Data[7:0] - Write setup and hold times
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -min -add_delay 140.000 [get_ports {pioPSOC_Emif_Data[*]}]
set_input_delay -clock [get_clocks piPSOC_Emif_Clk] -max -add_delay 146.667 [get_ports {pioPSOC_Emif_Data[*]}]

# PSOC / Fcfg - Asynchronous Reset Input
set_input_delay -clock [get_clocks piCLKT_Usr0Clk_p] -min -add_delay 0.400 [get_ports piPSOC_Fcfg_Rst_n]
set_input_delay -clock [get_clocks piCLKT_Usr0Clk_p] -max -add_delay 0.400 [get_ports piPSOC_Fcfg_Rst_n]

#=====================================================================
# Timing Exception for the SHELL Clock to Sample the PSOC Clock
#=====================================================================
set_false_path -from [get_ports piPSOC_Emif_Clk] -to [get_pins SHELL/MMIO/EMIF/sBus_ClkMts_reg/D]

#=====================================================================
# Timing Exceptions related to the Heart Beat LED
#=====================================================================
set_false_path -from [get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/calDone_gated_reg/C] -to [get_pins {SHELL/MMIO/EMIF/sFab_Data_reg[*]/D}]
set_false_path -from [get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/calDone_gated_reg/C] -to [get_pins {SHELL/MMIO/EMIF/sFab_Data_reg[*]/D}]

set_false_path -from [get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/calDone_gated_reg/C] -to [get_pins SHELL/sLed_HeartBeat_reg*/D]
set_false_path -from [get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/calDone_gated_reg/C] -to [get_pins SHELL/sLed_HeartBeat_reg*/D]

set_max_delay -from [get_pins SHELL/sLed_HeartBeat_reg*/C] -to [get_ports poLED_HeartBeat_n] 10.000

#=====================================================================
# Other Timing Exceptions
#=====================================================================


#=====================================================================
# Here are the Constraints added by the Timing Constraint Wizard
#=====================================================================













