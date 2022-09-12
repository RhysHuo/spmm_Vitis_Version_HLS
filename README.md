# SPMM Accelerator on FPGA

The kernel is reproduced from https://github.com/eejlny/gemm_spmm

The host program is contained in `host.cpp`. The `xcl2.hpp` should be included when you build host.exe.

The kernel is contained in `spmm_block.cpp` and `spmm_block.h` in which multiple parameters are defined.

## Before You Start

Make sure you set up Vitis and XRT environment first so that libraries can be included on the system path.

## Build the Kernel

First you need to compile the kernel code using `-c` option and the Xilinx object `.xo` file will be generated.

Then you can link the Xilinx object `.xo` file to the platform using `-l` or `--link` option.
This will generate the Xilinx binary `.xclbin` file which will be used to program the FPGA.

```
v++ -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 -c -k spmm -o'spmm.hw.xo' spmm_block.cpp spmm_block.h xcl2.hpp
v++ -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 --link spmm.hw.xo -o'spmm.hw.xclbin'
```

## Build the Host

Arbitrary precision type is used in this accelerator. The header file can be found in [HLS_arbitrary_Precision_Types](https://github.com/Xilinx/HLS_arbitrary_Precision_Types).

You can clone this and add to your working path. You can include it when building the host using `-I` option.

By running the following command, you can get `host.exe`.

```
g++ -g -std=c++14 -I$XILINX_XRT/include -L${XILINX_XRT}/lib/ -I/HLS_arbitrary_Precision_Types/include -o host.exe host.cpp spmm_block.h -lOpenCL -pthread -lrt -lstdc++
```

## Run the Application

When you successfully running commands above, you should have two files `spmm.hw.xclbin` and `host.exe` in your folder.

One of matrices is read from data file so you should have this data file in the right format, such as [weights_byte_099.csr](https://github.com/RhysHuo/spmm_Vitis_Version_HLS/blob/main/weights_byte_099.csr).

You can run this application using the following command:

```
./host.exe spmm.hw.xclbin <data_file> <precision_controller>
```
You can choose the precision `<precision_controller>` of data.

Precision controller : `0` for 8 bits, `1` for 2 bits, `2` for 4 bits.
