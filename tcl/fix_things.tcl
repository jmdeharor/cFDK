## This file contains tcl commands, that must be sourced for partial reconfiguration bitGen. 

set_property UNAVAILABLE_DURING_CALIBRATION TRUE [get_ports piCLKT_Usr1Clk_p]

set_property bitstream.general.perFrameCRC yes [current_design]

