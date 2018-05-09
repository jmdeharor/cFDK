# ******************************************************************************
# *
# *                        Zurich cloudFPGA
# *            All rights reserved -- Property of IBM
# *
# *-----------------------------------------------------------------------------
# *
# * Title   : Default timing constraint file for the FMKU2595 equipped with a
# *           XCKU060.
# * File    : top_FMKU60_Flash_timg.
# *
# * Created : Sep. 2017
# * Authors : Francois Abel <fab@zurich.ibm.com>
# *
# * Devices : xcku060-ffva1156-2-i
# * Tools   : Vivado v2016.4, v2017.4 (64-bit)
# * Depends : None
# *
# * Description : This file contains all the default timing constraints for
# *     operating a XCKU060 on a FMKU2595 module. The constraints are grouped by
# *      external device and connector entity names:
# *       - the synchronous dynamic random access memory (SDRM)
# *       - the programmable system on chip controller (PSOC)
# *       - the configuration Flash memory (FLASH)
# *       - the clock tree generator (CLKT)
# *       - the edge backplane connector (ECON)
# *       - the top extension connector (XCON)
# *
# *     The SDRM has the following interfaces:
# *       - a memory channel 0 (SDRM_Mch0)
# *       - a memory channel 1 (SDRM_Mch1)
# *     The PSOC has the following interfaces:
# *       - an External memory interface (PSOC_Emif)
# *       - an Fpga configuration interface (PSOC_Fcfg)
# *     The FLASH has the following interfaces:
# *       - a byte peripheral interface (FLASH_Bpi)
# *     The CLKT has the following interfaces:
# *       -
# *     The ECON has the following interfaces:
# *     The XCON has the following interfaces:
# *
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
# *       -  InpOBSOut and output delay constraints
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
# Create the Root Clocks as generated by the Clock Tree (CLKT)
#===============================================================================

# CLKT / Reference clock for GTH transceivers of the 10GE Interface 
#  [INFO] This clock is already constrained by the IP core.
#   create_clock -name piCLKT_10GeClk -period 6.400 [get_ports piCLKT_10GeClk_p]

# CLKT / Reference clock for the User clock #0
create_clock -name piCLKT_Usr0Clk -period 4.000 -waveform {0.000 2.000} [get_ports piCLKT_Usr0Clk_p]

# CLKT / Reference clock for the User clock #1
create_clock -name piCLKT_Usr1Clk -period 4.000 -waveform {0.000 2.000} [get_ports piCLKT_Usr1Clk_p]

# CLKT / Reference clock for the DRAM block 0
#  [INFO] This clock is already constrained by the IP core.
#   create_clock -name piCLKT_Mem0Clk -period 3.333 -waveform {0.000 1.667} [get_ports piCLKT_Mem0Clk_p]

# CLKT / Reference clock for the DRAM block 1
#  [INFO] This clock is already constrained by the IP core.
#   create_clock -name piCLKT_Mem1Clk -period 3.333 -waveform {0.000 1.667} [get_ports piCLKT_Mem1Clk_p]


#===============================================================================
# Create the Root Clock as generated by the EMIF bus I/F of the PSOC
#===============================================================================
create_clock -name piPSOC_Emif_Clk -period ${cPsocEmifClkPeriod} -waveform ${cPsocEmifClkWaveform} [ get_ports piPSOC_Emif_Clk ]


#===============================================================================
# Create a virtual clock to avoid "Critical Timing = NO_CLOCK".
#   These are constraints added to render the device fully constrained. 
#   In short, the characteristics of the clock driving every clockable cell must
#   be known by the tool. In general this is done by creating a clock on a port,
#   pin or net, upstream of the clock pin of the cell. This clock then propagates
#   forward to all clock pins that are combinatorially reachable from that port, 
#   pin or net.
#===============================================================================
create_clock -name topResetUsedAsClk -period 1000 [ get_pins TOP_META_RST/DOUT ]

#===============================================================================
# Create two virtual debug clocks to avoid "Critical Timing = NO_CLOCK".
#   These are constraints added to render the device fully constrained. 
#===============================================================================
#OBSOLETE-20180424 create_clock -name dbg_hub0_clk -period 13.328 [ get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/u_ddr_cal/U_XSDB_SLAVE/sl_iport_i[1] ]
#OBSOLETE-20180424 create_clock -name dbg_hub1_clk -period 13.328 [ get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/u_ddr_cal/U_XSDB_SLAVE/sl_iport_i[1] ]

#=====================================================================
# Definition of the Clock Interactions and their Constraints
#=====================================================================

# Asynchronous Clocks Between PSOC/Emif/Clk and the Rest of the Design
set_clock_groups -asynchronous -group [ get_clocks piPSOC_Emif_Clk ] -group [ get_clocks piCLKT_Usr0Clk ]
set_clock_groups -asynchronous -group [ get_clocks piPSOC_Emif_Clk ] -group [ get_clocks -of_objects [get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O] ]

# Asynchronous Clocks Between CLKT/10GE/Clk and PSOC/Emif/Clk
set_clock_groups -asynchronous -group [ get_clocks piCLKT_10GeClk_p ] -group [ get_clocks piPSOC_Emif_Clk ]

# Asynchronous Clocks Between SHELL/MEM/MC0/MCC/CLKOUT and the SHELL/CLK
set_clock_groups -asynchronous -group [ get_clocks -of_objects [ get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0] ] -group [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ]

# Asynchronous Clocks Between SHELL/MEM/MC1/MCC/CLKOUT and the SHELL/CLK
set_clock_groups -asynchronous -group [ get_clocks -of_objects [ get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ] -group [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ]

set_clock_groups -asynchronous -group [ get_clocks topResetUsedAsClk ] -group [ get_clocks piPSOC_Emif_Clk ]
set_clock_groups -asynchronous -group [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O]] -group [get_clocks topResetUsedAsClk]


#=====================================================================
# Setting Some Usefull TCL Variables and Aliases
#=====================================================================
#OBSOLETE-20180504 set myShellClk [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ]

#=====================================================================
# Constraints related to the Synchronous Dynamic RAM (DDR4)
#   DDR4 / Memory Channel #0
#=====================================================================

# DDR4 / Memory Channel #0  / Reset
#OBSOLETE-20180504 set myMmcmClkOut0_0 [ get_clocks -of_objects [ get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ]
#OBSOLETE-20180504 set_output_delay -clock ${myMmcmClkOut0_0} -max [ expr 2.0 * ${cMem0ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc0_Reset_n} ]
#OBSOLETE-20180504 set_output_delay -clock ${myMmcmClkOut0_0} -min [ expr 1.0 * ${cMem0ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc0_Reset_n} ]

set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ] -max [ expr 2.0 * ${cMem0ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc0_Reset_n} ]
set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/MEM/MC0/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ] -min [ expr 1.0 * ${cMem0ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc0_Reset_n} ]
set_multicycle_path -from [ get_pins {SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C} ] -to [ get_ports poTOP_Ddr4_Mc0_Reset_n ] -setup 5
set_multicycle_path -from [ get_pins {SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C} ] -to [ get_ports poTOP_Ddr4_Mc0_Reset_n ] -hold 4

# DDR4 / Memory Channel #0  / Slew & DataRate
set_property SLEW      FAST [ get_ports {poTOP_Ddr4_Mc0_Cs_n} ]
set_property DATA_RATE SDR  [ get_ports {poTOP_Ddr4_Mc0_Cs_n} ]


#=====================================================================
# Constraints related to the Synchronous Dynamic RAM (DDR4)
#   DDR4 / Memory Channel #1
#=====================================================================

# DDR4 / Memory Channel #1  / Reset
#OBSOLETE-20180504 set myMmcmClkOut0_1 [ get_clocks -of_objects [ get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ]
#OBSOLETE-20180504 set_output_delay -clock ${myMmcmClkOut0_1} -max [ expr 2.0 * ${cMem1ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc1_Reset_n} ]
#OBSOLETE-20180504 set_output_delay -clock ${myMmcmClkOut0_1} -min [ expr 1.0 * ${cMem1ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc1_Reset_n} ]
set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ] -max [ expr 2.0 * ${cMem1ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc1_Reset_n} ]
set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/MEM/MC1/MCC/inst/u_ddr4_infrastructure/gen_mmcme3.u_mmcme_adv_inst/CLKOUT0 ] ] -min [ expr 1.0 * ${cMem1ClkPeriod} ] [ get_ports {poTOP_Ddr4_Mc1_Reset_n} ]

set_multicycle_path -from [ get_pins {SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C} ] -to [ get_ports poTOP_Ddr4_Mc1_Reset_n ] -setup 5
set_multicycle_path -from [ get_pins {SHELL/MEM/MC1/MCC/inst/u_ddr4_mem_intfc/u_ddr_cal_top/cal_RESET_n_reg[0]/C} ] -to [ get_ports poTOP_Ddr4_Mc1_Reset_n ] -hold 4
#OBSOLETE-20180503 set_false_path -from [get_pins {SHELL/MEM/MC0/MCC/inst/u_ddr4_mem_intfc/u_ddr_ui/u_ddr_ui_rd_data/not_strict_mode.rd_buf.app_rd_data_ns_reg[0]/C}] -to [get_ports poTOP_Ddr4_Mc0_Reset_n]

# DDR4 / Memory Channel #1  / Slew & DataRate
set_property SLEW      FAST [ get_ports {poTOP_Ddr4_Mc1_Cs_n} ]
set_property DATA_RATE SDR  [ get_ports {poTOP_Ddr4_Mc1_Cs_n} ]


#=====================================================================
# Constraints related to the Backplane Edge Connector (ECON)
#=====================================================================
# ECON / 10 Gigabit Ethernet Rx/Tx links #1


#=====================================================================
# Constraints related to the Programmable SoC (PSOC)
#=====================================================================

# PSOC / FPGA Configuration Interface / Reset
#---------------------------------------------
set_input_delay -clock piCLKT_Usr0Clk -min 1.0 [ get_port piPSOC_Fcfg_Rst_n ]
set_input_delay -clock piCLKT_Usr0Clk -max 2.0 [ get_port piPSOC_Fcfg_Rst_n ]

set_false_path -through [ get_pins TOP_META_RST/DOUT ]


#---------------------------------------------------------------------
# PSOC / External Memory Interface (see PSoC Creator Component v1.30)
#---------------------------------------------------------------------
#
#             +---+   +---+   +---+   +---+   +---+   +---+   +---+
# Bus Clock   |   |   |   |   |   |   |   |   |   |   |   |   |   |
#          ---+   +---+   +---+   +---+   +---+   +---+   +---+   +---
#
#                         +---+                           +---+
# EMIF Clk                |   |                           |   |
#          ---------------+   +---------------------------+   +-------
#
#              +-----------------------------+ +----------------------
# EMIF Addr --X                               X
#              +-----------------------------+ +----------------------
#
#          ----------+        +-----------------------+        +------
# EMIF CTtl          |        |                       |        |
#                    +--------+                       +--------+
#
#----------------------------------------------------------------------

# PSoC / Emif - Address[7:0] - Write setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -${cPsocEmifAddrSetup}  [ get_ports {piPSOC_Emif_Addr[*]} ]
set_input_delay -clock piPSOC_Emif_Clk -min +${cPsocEmifAddrHold}   [ get_ports {piPSOC_Emif_Addr[*]} ]

# PSOC / Emif - Chip select - Access setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -${cPsocEmifCsSetup}    [ get_ports piPSOC_Emif_Cs_n ]
set_input_delay -clock piPSOC_Emif_Clk -min +${cPsocEmifCsHold}     [ get_ports piPSOC_Emif_Cs_n ]

# PSoC / Emif - Address Strobe - Access setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -${cPsocEmifAdsSetup}   [ get_ports piPSOC_Emif_AdS_n ]
set_input_delay -clock piPSOC_Emif_Clk -min +${cPsocEmifAdsHold}    [ get_ports piPSOC_Emif_AdS_n ]

# PSOC / Emif - Write enable - Write setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -${cPsocEmifWeSetup}    [ get_ports piPSOC_Emif_We_n ]
set_input_delay -clock piPSOC_Emif_Clk -min +${cPsocEmifWeHold}     [ get_ports piPSOC_Emif_We_n ]

# PSoC / Emif - Output enable - Read setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -${cPsocEmifOeSetup}    [ get_ports piPSOC_Emif_Oe_n ]
set_input_delay -clock piPSOC_Emif_Clk -min -${cPsocEmifOeHold}     [ get_ports piPSOC_Emif_Oe_n ]

# PSoC / Emif - Data[7:0] - Write setup and hold times
set_input_delay -clock piPSOC_Emif_Clk -max -$cPsocEmifDataWrSetup  [ get_ports {pioPSOC_Emif_Data[*]} ]
set_input_delay -clock piPSOC_Emif_Clk -min +$cPsocEmifDataWrHold   [ get_ports {pioPSOC_Emif_Data[*]} ]

# PSoC / Emif - Data[7:0] - Read setup and hold times
set_output_delay -clock piPSOC_Emif_Clk -max +${cPsocEmifDataRdSetup} [ get_ports {pioPSOC_Emif_Data[*]} ]
set_output_delay -clock piPSOC_Emif_Clk -min +${cPsocEmifDataRdHold}  [ get_ports {pioPSOC_Emif_Data[*]} ]


#---------------------------------------------------------------------
# FABRIC / External Memory Interface
#  Constraints added to avoid warnings because of missing input delay 
#---------------------------------------------------------------------
set_input_delay -clock piCLKT_10GeClk_p -max -12.8  [ get_ports {pioPSOC_Emif_Data[*]} ]
set_input_delay -clock piCLKT_10GeClk_p -min +12.8  [ get_ports {pioPSOC_Emif_Data[*]} ]
set_false_path -from [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ] -to [ get_pins SHELL/MMIO/EMIF/sDataReg_reg[*]/D ]


#=====================================================================
# Constraints related to the Heart Beat LED
#=====================================================================
#OBSOLETE-20180504 set_output_delay -clock ${myShellClk} -max ${cShellClockPeriod} [ get_ports {poTOP_Led_HeartBeat_n} ]
#OBSOLETE-20180504 set_output_delay -clock ${myShellClk} -min ${cShellClockPeriod} [ get_ports {poTOP_Led_HeartBeat_n} ]

set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ] -max ${cShellClockPeriod} [ get_ports {poTOP_Led_HeartBeat_n} ]
set_output_delay -clock [ get_clocks -of_objects [ get_pins SHELL/SuperCfg.ETH0/ETH0/CORE/IP/U0/xpcs/U0/ten_gig_eth_pcs_pma_shared_clock_reset_block/txusrclk2_bufg_gt_i/O ] ] -min ${cShellClockPeriod} [ get_ports {poTOP_Led_HeartBeat_n} ]
set_false_path -from [ get_pins SHELL/sLed_HeartBeat_reg_inv/C ] -to [ get_ports poTOP_Led_HeartBeat_n ]


#=====================================================================
# DEBUG_HUB Constraints added by the Timing Constraint Wizard
#=====================================================================
set_property C_CLK_INPUT_FREQ_HZ  300000000 [ get_debug_cores dbg_hub ]
set_property C_ENABLE_CLK_DIVIDER false      [ get_debug_cores dbg_hub ]
set_property C_USER_SCAN_CHAIN    1         [ get_debug_cores dbg_hub ]
connect_debug_port dbg_hub/clk              [ get_nets clk ]

