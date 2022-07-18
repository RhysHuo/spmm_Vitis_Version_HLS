git clone https://github.com/RhysHuo/spmm_Vitis_Version_HLS.git
cd spmm_Vitis_Version_HLS
cp host.cpp ..
cd ..
rm -rf spmm_Vitis_Version_HLS
g++ -g -std=c++14 -I$XILINX_XRT/include -L${XILINX_XRT}/lib/ -I/mnt/scratch/rhyhuo/HLS_arbitrary_Precision_Types/include -o host.exe host.cpp spmm_block.h -lOpenCL -pthread -lrt -lstdc++
