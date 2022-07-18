git clone https://github.com/RhysHuo/spmm_Vitis_Version_HLS.git
cd spmm_Vitis_Version_HLS
cp spmm_block.cpp ..
cp spmm_block.h ..
cd ..
rm -rf spmm_Vitis_Version_HLS
v++ -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 -c -k spmm -o'spmm.hw.xo' spmm_block.cpp spmm_block.h xcl2.hpp
