#ifndef _MPI_H_
#define _MPI_H_


#include <stdint.h>
#include <stdio.h>
#include "ap_int.h"
#include "ap_utils.h"
#include <hls_stream.h>

using namespace hls;

#define WAIT_CYCLES 100
//Display0
#define RECV_CNT_SHIFT 8
#define SEND_CNT_SHIFT 4
#define AP_DONE_SHIFT 12
#define AP_INIT_SHIFT 13

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

#define MPI_SEND_INT 0
#define MPI_RECV_INT 1
#define MPI_SEND_FLOAT 2
#define MPI_RECV_FLOAT 3
#define MPI_BARRIER 4
#define mpiCall uint8_t

/*
 * MPI-F Interface
 */
struct MPI_Interface {
  ap_uint<8>     mpi_call;
  ap_uint<32>    count;
  ap_uint<32>    rank;
  MPI_Interface() {}
};


#define MPI_Status uint8_t
#define MPI_Comm   uint8_t
#define MPI_Datatype uint8_t

#define MPI_COMM_WORLD 0
#define MPI_INTEGER 0
#define MPI_FLOAT   1


//void MPI_Init(int* argc, char*** argv);
void MPI_Init();
void MPI_Comm_rank(MPI_Comm communicator, int* rank);
void MPI_Comm_size( MPI_Comm communicator, int* size);


void MPI_Send(
    void* data,
    int count,
    MPI_Datatype datatype,
    int destination,
    int tag,
    MPI_Comm communicator);

void MPI_Recv(
    void* data,
    int count,
    MPI_Datatype datatype,
    int source,
    int tag,
    MPI_Comm communicator,
    MPI_Status* status);

void MPI_Finalize();

//void MPI_Barrier(MPI_Comm communicator);



//void mpi_wrapper(
//    // ----- system reset ---
//    ap_uint<1> sys_reset,
//    //EMIF Registers
//    ap_uint<16> *MMIO_in,
//    ap_uint<16> *MMIO_out,
//    // ----- MPI_Interface -----
//    stream<MPI_Interface> &siMPIif,
//    stream<MPI_Interface> &soMPIif,
//    stream<Axis<8> > &siMPI_data,
//    stream<Axis<8> > &soMPI_data,
//    // ----- FROM SMC -----
//    ap_uint<32> *role_rank,
//    ap_uint<32> *cluster_size
//    );
//void mpi_wrapper();
void mpi_wrapper(
    // ----- system reset ---
    ap_uint<1> sys_reset
    );

void c_testbench_access(
    // ----- system reset ---
    ap_uint<1> *sys_reset_arg,
    //EMIF Registers
    ap_uint<16> *MMIO_in_arg,
    ap_uint<16> *MMIO_out_arg,
    // ----- MPI_Interface -----
    stream<MPI_Interface> *siMPIif_arg,
    stream<MPI_Interface> *soMPIif_arg,
    stream<Axis<8> > *siMPI_data_arg,
    stream<Axis<8> > *soMPI_data_arg,
    // ----- FROM SMC -----
    ap_uint<32> *role_rank_arg,
    ap_uint<32> *cluster_size_arg
    );

#endif
