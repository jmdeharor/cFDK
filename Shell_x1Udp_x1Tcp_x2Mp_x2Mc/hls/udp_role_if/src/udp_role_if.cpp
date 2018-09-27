/*****************************************************************************
 * @file       : udp_role_if.cpp
 * @brief      : UDP Role Interface (URIF)
 *
 * System:     : cloudFPGA
 * Component   : Shell, Network Transport Session (NTS)
 * Language    : Vivado HLS
 *
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2018 - IBM Research - All Rights Reserved.
 *
 *----------------------------------------------------------------------------
 *
 * @details    : This process exposes the UDP data path to the role. The main
 *  purpose of the URIF is to hide the socket mechanism from the user's role
 *  application. As a result, the user application sends and receives its data
 *  octets over plain Tx and Rx AXI4-Stream interfaces.
 *
 * @note       : The Tx data path (i.e. THIS->UDP) will block until a data
 *                chunk and its corresponding metadata is received by the Rx
 *                path.
 * @remark     : { remark text }
 * @warning    : For the time being, all communications occur over port #80.
 * @todo       : [TODO - Port# must configurable]
 *
 * @see        : https://www.stack.nl/~dimitri/doxygen/manual/commands.html
 *
 *****************************************************************************/

#include "udp_role_if.hpp"

#define USE_DEPRECATED_DIRECTIVES

/*****************************************************************************
 * @brief Update the payload length based on the setting of the 'tkeep' bits.
 * @ingroup udp_role_if
 *
 * @param[in]     axisChunk, a pointer to an AXI4-Stream chunk.
 * @param[in,out] pldLen, a pointer to the payload length of the corresponding
 *                     AXI4-Stream.
 * @return Nothing.
 ******************************************************************************/
void updatePayloadLength(UdpWord *axisChunk, UdpPLen *pldLen) {
    if (axisChunk->tlast) {
        int bytCnt = 0;
        for (int i = 0; i < 8; i++) {
            if (axisChunk->tkeep.bit(i) == 1) {
                bytCnt++;
            }
        }
        *pldLen += bytCnt;
    } else
        *pldLen += 8;
}


/*****************************************************************************
 * @brief TxPath - Enqueue data onto a FiFo stream while computing the
 *         payload length of incoming frame on the fly.
 * @ingroup udp_role_if
 *
 * @param[in]  siROLE_Data, the data stream to enqueue.
 * @param[out] soFifo_Data, the FiFo stream to write.
 * @param[out] soPLen,      the length of the enqueued payload.
 *
 * @return Nothing.
 *****************************************************************************/
 void pTxP_Enqueue (
        stream<UdpWord>    &siROLE_Data,
        stream<UdpWord>    &soFifo_Data,
        stream<UdpPLen>    &soPLen)
{
	 //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
	#pragma HLS DATAFLOW interval=1

    //-- LOCAL VARIABLES ------------------------------------------------------
    static UdpPLen    pldLen;

    static enum FsmState {FSM_RST=0, FSM_ACC, FSM_LAST_ACC} fsmState;

    switch(fsmState) {

        case FSM_RST:
            pldLen = 0;
            fsmState = FSM_ACC;
            break;

        case FSM_ACC:

            // Default stream handling
            if ( !siROLE_Data.empty() && !soFifo_Data.full() ) {

                // Read incoming data chunk
                UdpWord aWord = siROLE_Data.read();

                // Increment the payload length
                updatePayloadLength(&aWord, &pldLen);

                // Enqueue data chunk into FiFo
                soFifo_Data.write(aWord);

                // Until LAST bit is set
                if (aWord.tlast)
                    fsmState = FSM_LAST_ACC;
            }
            break;

        case FSM_LAST_ACC:

            // Forward the payload length
            soPLen.write(pldLen);

            // Reset the payload length
            pldLen = 0;

            // Start over
            fsmState = FSM_ACC;

            break;
    }
}


/*****************************************************************************
 * @brief TxPath - Dequeue data from a FiFo stream and forward the metadata
 *         when the LAST bit is set.
 * @ingroup udp_role_if
 *
 * @param[in]  siFifo, the FiFo stream to read from.
 * @param[in]  siMeta, the socket pair information provided by the Rx path.
 * @param[in]  siPLen, the length of the enqueued payload.
 * @param[out] soUDMX_Data, the output data stream.
 * @param[out] soUDMX_Meta, the output metadata stream.
 * @param[out] soUDMX_PLen, the output payload length.
 *
 * @return Nothing.
 *****************************************************************************/
void pTxP_Dequeue (
        stream<UdpWord>  &siFifo,
        stream<UdpMeta>  &siMeta,
        stream<UdpPLen>  &siPLen,
        stream<UdpWord>  &soUDMX_Data,
        stream<UdpMeta>  &soUDMX_Meta,
        stream<UdpPLen>  &soUDMX_PLen)
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS DATAFLOW interval=1

    //-- LOCAL VARIABLES ------------------------------------------------------
    static UdpMeta txSocketPairReg;

    static enum FsmState {FSM_W8FORMETA=0, FSM_FIRST_ACC, FSM_ACC} fsmState;

    switch(fsmState) {

        case FSM_W8FORMETA:

            // The very first time, wait until the Rx path provides us with the
            // socketPair information before continuing
            if ( !siMeta.empty() ) {
                txSocketPairReg = siMeta.read();
                fsmState = FSM_FIRST_ACC;
            }
            break;

        case FSM_FIRST_ACC:

            // Send out the first data together with the metadata and payload length information
            if ( !siFifo.empty() && !siPLen.empty() ) {
                if ( !soUDMX_Data.full() && !soUDMX_Meta.full() && !soUDMX_PLen.full() ) {
                    // Forward data chunk, metadata and payload length
                    UdpWord    aWord = siFifo.read();
                    if (!aWord.tlast) {
                        soUDMX_Data.write(aWord);
                        soUDMX_Meta.write(txSocketPairReg);
                        soUDMX_PLen.write(siPLen.read());
                        fsmState = FSM_ACC;
                    }
                }
            }

            // Always drain the 'siMeta' stream to avoid any blocking on the Rx side
            if ( !siMeta.empty() )
                txSocketPairReg = siMeta.read();

            break;

        case FSM_ACC:

            // Default stream handling
            if ( !siFifo.empty() && !soUDMX_Data.full() ) {
                // Forward data chunk
                UdpWord    aWord = siFifo.read();
                soUDMX_Data.write(aWord);
                // Until LAST bit is set
                if (aWord.tlast)
                    fsmState = FSM_FIRST_ACC;
            }

            // Always drain the 'siMeta' stream to avoid any blocking on the Rx side
            if ( !siMeta.empty() )
                txSocketPairReg = siMeta.read();

            break;
    }
}


/*****************************************************************************
 * @brief Tx Path - From ROLE to UDP core.
 * @ingroup udp_role_if
 *
 * @param[in]  siROL_Data,  data from ROLE.
 * @param[in]  sRxP_Meta,   metadata from the RxPath process.
 * @param[out] soUDMX_Data, data to UDP-mux.
 * @param[out] soUDMX_Meta, meta-data to UDP-mux.
 * @param[out] soUDMX_PLen, packet length to UDP-mux.
 *
 * @return Nothing.
 *****************************************************************************/
void pTxP(
        stream<UdpWord>     &siROL_Data,
        stream<UdpMeta>     &siRxP_Meta,
        stream<UdpWord>     &soUDMX_Data,
        stream<UdpMeta>     &soUDMX_Meta,
        stream<UdpPLen>     &soUDMX_PLen)
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS DATAFLOW interval=1

    //-- LOCAL STREAMS --------------------------------------------------------
    static stream<UdpPLen>        sPLen     ("sPLen");
    static stream<UdpWord>        sFifo_Data("sFifo_Data");

    #pragma HLS STREAM variable=sPLen depth=1
    #pragma HLS STREAM variable=sFifo_Data depth=2048    // Must be able to contain MTU

    //-- PROCESS FUNCTIONS ----------------------------------------------------
    pTxP_Enqueue(siROL_Data,
                 sFifo_Data, sPLen);

    pTxP_Dequeue(sFifo_Data,  siRxP_Meta,  sPLen,
                 soUDMX_Data, soUDMX_Meta, soUDMX_PLen);
}


/*****************************************************************************
 * @brief Rx Path - From UDP core to ROLE.
 * @ingroup udp_role_if
 *
 * @param[in]  siUDMX_Data,   data from UDP-mux.
 * @param[in]  siUDMX_Meta,   metadata from UDP-mux.
 * @param[out] soUDMX_OpnReq, open port request to UDP-mux.
 * @param[in]  siUDMX_OpnAck, open port acknowledgment from UDP-mux.
 * @param[out] soROLE_Data,   data to ROLE.
 * @param[out] soTxP_Meta,    the socketPair info for the TxPath process.
 *
 * @return Nothing.
 ******************************************************************************/
void pRxP(
        stream<UdpWord>      &siUDMX_Data,
        stream<UdpMeta>      &siUDMX_Meta,
        stream<UdpPort>      &soUDMX_OpnReq,
        stream<AxisAck>      &siUDMX_OpnAck,
        stream<UdpWord>      &soROLE_Data,
        stream<UdpMeta>      &soTxP_Meta)
{
	#pragma HLS DATAFLOW interval=1

    //-- LOCAL VARIABLES ------------------------------------------------------
    static ap_uint<8>   openPortWaitTime;

    static enum FsmState {FSM_RESET=0, FSM_IDLE, FSM_W8FORPORT, FSM_FIRST_ACC, FSM_ACC} fsmState;

    switch(fsmState) {

        case FSM_RESET:
            openPortWaitTime = 10;
            fsmState = FSM_IDLE;
            break;

        case FSM_IDLE:
            if ( !soUDMX_OpnReq.full() && openPortWaitTime == 0 ) {
                // Request to open UDP port 80 -- [FIXME - Port# should be a parameter]
                soUDMX_OpnReq.write(0x0050);
                fsmState = FSM_W8FORPORT;
            }
            else
                openPortWaitTime--;
            break;

        case FSM_W8FORPORT:
            if ( !siUDMX_OpnAck.empty() ) {
                // Read the acknowledgment
                AxisAck sOpenAck = siUDMX_OpnAck.read();
                fsmState = FSM_FIRST_ACC;
            }
            break;

        case FSM_FIRST_ACC:
            // Wait until both the first data chunk and the first metadata are received from UDP
            if ( !siUDMX_Data.empty() && !siUDMX_Meta.empty() ) {
                if ( !soROLE_Data.full() && !soTxP_Meta.full()) {
                    // Forward data chunk to ROLE
                    UdpWord    udpWord = siUDMX_Data.read();
                    if (!udpWord.tlast) {
                        soROLE_Data.write(udpWord);
                        fsmState = FSM_ACC;
                    }

                    // Swap the source and destination socket addresses
                    UdpMeta udpRxMeta = siUDMX_Meta.read();
                    UdpMeta udpTxMeta;
                    udpTxMeta.dst = udpRxMeta.src;
                    udpTxMeta.src = udpRxMeta.dst;
                    // Forward the socketPair information to pTxPath
                    soTxP_Meta.write(udpTxMeta);
                }
            }
            break;

        case FSM_ACC:
            // Default stream handling
            if ( !siUDMX_Data.empty() && !soROLE_Data.full() ) {
                // Forward data chunk to ROLE
                UdpWord    udpWord = siUDMX_Data.read();
                soROLE_Data.write(udpWord);
                // Until LAST bit is set
                if (udpWord.tlast)
                    fsmState = FSM_FIRST_ACC;
            }
            break;
    }
}


/*****************************************************************************
 * @brief   Main process of the UDP Role Interface
 * @ingroup udp_role_if
 *
 * @param[in]  siROL_This_Data     UDP data stream from the ROLE.
 * @param[out] soTHIS_Rol_Data     UDP data stream to the ROLE.
 * @param[in]  siUDMX_This_OpnAck  Open port acknowledgment from UDP-Mux.
 * @param[out] soTHIS_Udmx_OpnReq  Open port request to UDP-Mux.
 * @param[in]  siUDMX_This_Data    Data path from the UDP-Mux.
 * @param[in]  siUDMX_This_Meta    Metadata from the UDP-Mux.
 * @param[out] soTHIS_Udmx_Data    Data path to the UDP-Mux.
 * @param[out] soTHIS_Udmx_Meta    Metadata to the UDP-Mux.
 * @param[out] soTHIS_Udmx_PLen    Payload length to the UDP-Mux.
 *
 * @return Nothing.
 *****************************************************************************/
void udp_role_if (

        //------------------------------------------------------
        //-- ROLE / This / Udp Interfaces
        //------------------------------------------------------
        stream<UdpWord>     &siROL_This_Data,
        stream<UdpWord>     &soTHIS_Rol_Data,

        //------------------------------------------------------
        //-- UDMX / This / Open-Port Interfaces
        //------------------------------------------------------
        stream<AxisAck>     &siUDMX_This_OpnAck,
        stream<UdpPort>     &soTHIS_Udmx_OpnReq,

        //------------------------------------------------------
        //-- UDMX / This / Data & MetaData Interfaces
        //------------------------------------------------------
        stream<UdpWord>     &siUDMX_This_Data,
        stream<UdpMeta>     &siUDMX_This_Meta,
        stream<UdpWord>     &soTHIS_Udmx_Data,
        stream<UdpMeta>     &soTHIS_Udmx_Meta,
        stream<UdpPLen>     &soTHIS_Udmx_PLen)
{

    /*********************************************************************/
    /*** For the time being, we continue designing with the DEPRECATED ***/
    /*** directives because the new PRAGMAs do not work for us.        ***/
    /*********************************************************************/

#if defined(USE_DEPRECATED_DIRECTIVES)

	//-- DIRECTIVES FOR THE BLOCK ---------------------------------------------
	#pragma HLS INTERFACE ap_ctrl_none port=return

	#pragma HLS resource core=AXI4Stream variable=siROL_This_Data    metadata="-bus_bundle siROL_This_Data"
	#pragma HLS resource core=AXI4Stream variable=soTHIS_Rol_Data    metadata="-bus_bundle soTHIS_Rol_Data"

	#pragma HLS resource core=AXI4Stream variable=siUDMX_This_OpnAck metadata="-bus_bundle siUDMX_This_OpnAck"
	#pragma HLS resource core=AXI4Stream variable=soTHIS_Udmx_OpnReq metadata="-bus_bundle soTHIS_Udmx_OpnReq"

	#pragma HLS resource core=AXI4Stream variable=siUDMX_This_Data   metadata="-bus_bundle siUDMX_This_Data"
	#pragma HLS resource core=AXI4Stream variable=siUDMX_This_Meta   metadata="-bus_bundle siUDMX_This_Meta "
	#pragma HLS DATA_PACK                variable=siUDMX_This_Meta

	#pragma HLS resource core=AXI4Stream variable=soTHIS_Udmx_Data   metadata="-bus_bundle soTHIS_Udmx_Data"    
	#pragma HLS resource core=AXI4Stream variable=soTHIS_Udmx_Meta   metadata="-bus_bundle soTHIS_Udmx_Meta"
	#pragma HLS DATA_PACK                variable=soTHIS_Udmx_Meta
	#pragma HLS resource core=AXI4Stream variable=soTHIS_Udmx_PLen   metadata="-bus_bundle soTHIS_Udmx_PLen"

#else

	//-- DIRECTIVES FOR THE BLOCK ---------------------------------------------
	#pragma HLS INTERFACE ap_ctrl_none port=return

    #pragma HLS INTERFACE axis register off port=siROL_This_Data
    #pragma HLS INTERFACE axis register off port=soTHIS_Rol_Data

    #pragma HLS INTERFACE axis register off port=siUDMX_This_OpnAck
    #pragma HLS INTERFACE axis register off port=soTHIS_Udmx_OpnReq

    #pragma HLS INTERFACE axis register off port=siUDMX_This_Data
    #pragma HLS INTERFACE axis register off port=siUDMX_This_Meta
    #pragma HLS DATA_PACK               variable=siUDMX_This_Meta instance=siUDMX_This_Meta

    #pragma HLS INTERFACE axis register off port=soTHIS_Udmx_Data
    #pragma HLS INTERFACE axis register off port=soTHIS_Udmx_Meta
    #pragma HLS DATA_PACK               variable=soTHIS_Udmx_Meta instance=soTHIS_Udmx_Meta
    #pragma HLS INTERFACE axis register off port=soTHIS_Udmx_PLen

#endif

    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS DATAFLOW interval=1

    //-- LOCAL STREAMS --------------------------------------------------------
    //OBSOLETE#pragma HLS STREAM variable=sTxPLen depth=2
    //OBSOLETE#pragma HLS STREAM variable=sTxFifo depth=2048    // Must be able to contain MTU

    static stream<UdpMeta>      sRxToTx_Meta("sRxToTx_Meta");

    #pragma HLS STREAM    variable=sRxToTx_Meta depth=2
    #pragma HLS DATA_PACK variable=sRxToTx_Meta instance=sRxToTx_Meta

    //-- PROCESS FUNCTIONS ----------------------------------------------------
    pTxP(siROL_This_Data,  sRxToTx_Meta,
         soTHIS_Udmx_Data, soTHIS_Udmx_Meta, soTHIS_Udmx_PLen);

    //---- From UDMX to ROLE
    pRxP(siUDMX_This_Data,  siUDMX_This_Meta,
        soTHIS_Udmx_OpnReq, siUDMX_This_OpnAck,
        soTHIS_Rol_Data,    sRxToTx_Meta);

}

