# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Dec 2017
# * Authors : Francois Abel, Jagath Weerasinghe  
# * 
# * Description : A Tcl script for the HLS batch syhthesis of the IP Rx handler
# *   process used in the SHELL of the cloudFPGA module.
# *   project.
# * 
# * Synopsis : vivado_hls -f <this_file>
# *
# *
# * Reference documents:
# *  - UG902 / Ch.4 / High-Level Synthesis Reference Guide.
# *
# *-----------------------------------------------------------------------------
# * Modification History:
# *  Fab: Jan-16-2018 Adds header and variables.
# *  Fab: Feb-15-2018 Changed the export procedure.
# ******************************************************************************

# User defined settings
#-------------------------------------------------
set projectName    "iprx_handler"
set solutionName   "solution1"
set xilPartName    "xcku060-ffva1156-2-i"

set ipName         ${projectName}
set ipDisplayName  "IP Rx Handler for cloudFPGA"
set ipDescription  "Parses the received IP packest and forwards the ARP, ICMP, TCP and UDP accordingly."
set ipVendor       "IBM"
set ipLibrary      "hls"
set ipVersion      "1.0"
set ipPkgFormat    "ip_catalog"

# Set Project Environment Variables  
#-------------------------------------------------
set currDir      [pwd]
set srcDir       ${currDir}/src
set testDir      ${currDir}/test
set implDir      ${currDir}/${projectName}_prj/${solutionName}/impl/ip 
set repoDir      ${currDir}/../../ip

# Open and Setup Project
#-------------------------------------------------
open_project  ${projectName}_prj
set_top       ${projectName}

add_files     ${srcDir}/${projectName}.cpp
add_files -tb ${testDir}/test_${projectName}.cpp

open_solution ${solutionName}

set_part      ${xilPartName}
create_clock -period 6.4 -name default

# Run C Synthesis
#-------------------------------------------------
#csim_design -clean              [FIXME]
#csim_design -clean -setup       [FIXME]
csynth_design
#cosim_design -tool xsim -rtl verilog -trace_level all

# Export RTL
#-------------------------------------------------
export_design -format ${ipPkgFormat} -library ${ipLibrary} -display_name ${ipDisplayName} -description ${ipDescription} -vendor ${ipVendor} -version ${ipVersion}

# Exit Vivado HLS
#--------------------------------------------------
exit

