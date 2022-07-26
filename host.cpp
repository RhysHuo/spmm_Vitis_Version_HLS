#include <iostream>
#include <fstream>
#include <string.h>
#include <chrono>
#include <algorithm>
#include <vector>

#include "xcl2.hpp"
#include <CL/cl.h>
#include <CL/cl2.hpp>
#include "math.h"
#include "spmm_block.h"

//20220706 18:27 successful

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }


u32 golden_spmm_ternary(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

    //std::cout << "gold_spmm_ternary: check point 1" << std::endl;
	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				//y0 += values[j] * x[nvc*col_size+col_indices[j]];
				for(int z = 0; z < DTYPE_LENGTH; z+=2) {
					            DATA_TYPE values_val1 = values[j];
					        	ap_int<2> values_val = values_val1.range(z+1,z);
					        	int x_value = nvc*col_size+col_indices[j];
					        	int x_up = x_value >> 4;
					        	int x_down = (x_value & 0xF);
								DATA_TYPE values_val_temp = values_val;
						       	ap_int<2> x_temp;
								switch(values_val_temp)
								{
									 case 0:
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
									 case 1:
										 x_temp = x[x_up].range(x_down*2+1,x_down*2);
										 y0 += x_temp;
										 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
									 case -1:
										 x_temp = x[x_up].range(x_down*2+1,x_down*2);
										 y0 -= x_temp;
										 //std::cout << "B is" << b[k][j].range(z+1,z) << std::endl;
										 //std::cout << "C is" << C[j] << std::endl;
										 break;
								}

				}
				//std::cout << "y0 is " << y0 << std::endl;
			}
			y[nvc*row_size+i] = y0;
		}
	}

	return 0;
}

void golden_spmm_byte(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				for(int z = 0; z < DTYPE_LENGTH; z+=8) {
					            DATA_TYPE values_val1 = values[j];
								ap_int<8> values_val = values_val1.range(z+7,z);
								int x_value = nvc*col_size+col_indices[j];
								int x_up = x_value >> 2;
								int x_down = (x_value & 0x3);
						       	y0 += values_val * x[x_up].range(x_down*8+7,x_down*8);

				}
			}
			y[nvc*row_size+i] = y0;
		}
	}
}

u32 golden_spmm_quad(DATA_TYPE * values, u32 *row_ptr, u32* col_indices, DATA_TYPE_X * x, u32 no_vectors, DATA_TYPE_OUT *y, u32 row_size, u32 col_size) {

    //std::cout << "golden_spmm_quad: check point 3" << std::endl;
	u32 nvc = 0, i = 0, j = 0, rowStart = 0, rowEnd = row_size;

	DATA_TYPE_OUT y0 = 0;
	u32 last_j = 0;
	for (nvc = 0; nvc < no_vectors; nvc++) {
		for (i = rowStart; i < rowEnd; ++i) {
			y0 = 0;
			for (j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
				//y0 += values[j] * x[nvc*col_size+col_indices[j]];
				for(int z = 0; z < DTYPE_LENGTH; z+=4) {
					            DATA_TYPE values_val1 = values[j];
								ap_int<4> values_val = values_val1.range(z+3,z);
								int x_value = nvc*col_size+col_indices[j];
								int x_up = x_value >> 3;
								int x_down = (x_value & 0x7);
						       	y0 += values_val * x[x_up].range(x_down*4+3,x_down*4);
						       	//std::cout << "y0 " << y0 << std::endl;

				}
				//std::cout << "y0 is " << y0 << std::endl;
			}
			//std::cout << "y0 is " << y0 << std::endl;
			y[nvc*row_size+i] = y0;
		}
	}

	return 0;
}

/*
void init_array_golden(ap_uint<2> ternary, DATA_TYPE_X *x, u32 row, u32 col)
{
        if(ternary==0)
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < col; j++) {
				x[i*col+j] = 0x01010101;
			}
		}
	}
	else if (ternary==1)
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < col; j++) {
				x[i*col+j] = 0x55555555;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < col; j++) {
				x[i*col+j] = 0x11111111;
			}
		}
	}
}
*/

void init_array(ap_uint<2> ternary, DATA_TYPE_X *x, u32 row, u32 col)
{
        if(ternary==0)
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < (col>>2); j++) {
				x[i*(col>>2)+j] = 0x01010101;
			}
		}
	}
	else if (ternary==1)
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < (col>>2); j++) {
				x[i*(col>>2)+j] = 0x55555555;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < row; i++) {
			for (u32 j = 0; j < (col>>2); j++) {
				x[i*(col>>2)+j] = 0x11111111;
			}
		}
	}
}

static int result_check(DATA_TYPE_OUT *y, DATA_TYPE_OUT *y_golden, u32 row, u32 col)
{
	for (int i = 0; i < row * col; i++) {
		if (y_golden[i] != y[i]) {
			std::cout 	<< "Mismatch: data index= " << i << " golden = " << y_golden[i]
						<< ", kernel = " << y[i] << std::endl;
			return 1;
		}
	}
    std::cout 	<< "TEST PASSED !" <<  std::endl;
	return 0;
}

//MAIN
int main(int argc, char** argv) {

    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <xclbin>" << " myFile" << " ternary" << std::endl;
        return EXIT_FAILURE;
    }

    std::string xclbinFilename = argv[1];
    std::vector<cl::Device> devices;
    cl_int err;
    cl::Context context;
    cl::CommandQueue q;
    cl::Kernel krnl;
    cl::Program program;
    std::vector<cl::Platform> platforms;
    bool found_device = false;

    // traversing all Platforms To find Xilinx Platform and targeted
    // Device in Xilinx Platform
    cl::Platform::get(&platforms);
    for (size_t i = 0; (i < platforms.size()) & (found_device == false); i++) {
        cl::Platform platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
        if (platformName == "Xilinx") {
            devices.clear();
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
            if (devices.size()) {
                found_device = true;
                break;
            }
        }
    }
    if (found_device == false) {
        std::cout << "Error: Unable to find Target Device " << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "INFO: Reading " << xclbinFilename << std::endl;
    FILE* fp;
    if ((fp = fopen(xclbinFilename.c_str(), "r")) == nullptr) {
        printf("ERROR: %s xclbin not available please build\n", xclbinFilename.c_str());
        exit(EXIT_FAILURE);
    }
    // Load xclbin
    std::cout << "Loading: '" << xclbinFilename << "'\n";
    std::ifstream bin_file(xclbinFilename, std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    unsigned nb = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    char* buf = new char[nb];
    bin_file.read(buf, nb);

    // Creating Program from Binary File
    cl::Program::Binaries bins;
    bins.push_back({buf, nb});
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        program = cl::Program(context, {device}, bins, NULL, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl = cl::Kernel(program, "spmm", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

	FILE *fp_input;
	fp_input = fopen(argv[2], "r");
    //fp_input = argv[2];
    ap_uint<2> S_ternary = atoi(argv[3]);

	u32 r;
	u32 c;
	DATA_TYPE v;

    DATA_TYPE *array_values;
    u32* array_colIndices;
    u32* array_rowPtr;

    u32 row_size;
    u32 col_size;
    u32 nnz;

	if (fp_input != NULL) {
        //std::cout << "read_mtx_spmm: check point 2" << std::endl;
		char line_1[1000];
	//std::cout << "has defined a char line[1000]" << std::endl;
		if(fgets(line_1, sizeof(line_1), fp_input) != NULL){
			sscanf(line_1, "%u %u %u", &row_size, &col_size, &nnz);
			//std::cout << "row_size = " <<  row_size << " col_size = " << col_size << " nnz = " << nnz << std::endl;
		}
		
		/*
        	while (fgets(line_1, sizeof(line_1), fp_input) != NULL) {
			//std::cout << "has entered while" << std::endl;
			if (line_1[0] != '%') {
				//std::cout << "has entered if, start to sscanf" << std::endl;
				sscanf(line_1, "%u %u %u", &row_size, &col_size, &nnz);
				//std::cout << "row_size = " <<  *row_size << " col_size = " << *col_size << " nnz = " << *nnz << std::endl;
				std::cout << "row_size = " <<  row_size << " col_size = " << col_size << " nnz = " << nnz << std::endl;
				//std::cout << "read_mtx_spmm: check point 3" << std::endl;
			}
		}
		*/
	}
	else {
		//perror(argv[1]); //print the error message on stderr.
		std::cout << "Error with input file name" << std::endl;
		exit(EXIT_FAILURE);
	}

    u32 no_vectors = 512;

    // Map our user-allocated buffers as OpenCL buffers using a shared host pointer
    OCL_CHECK(err, cl::Buffer buffer_array_values(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR , nnz * sizeof(DATA_TYPE), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_array_colIndices(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR , nnz * sizeof(u32), NULL, &err));    
    OCL_CHECK(err, cl::Buffer buffer_array_rowPtr(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR , (row_size + 1) * sizeof(u32), NULL, &err));
    //OCL_CHECK(err, cl::Buffer buffer_array_x(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR , col_size * no_vectors * sizeof(DATA_TYPE_X)/4, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_array_x(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR , col_size * no_vectors * sizeof(DATA_TYPE_X)/4, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_array_y(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR , row_size * no_vectors * sizeof(DATA_TYPE_OUT), NULL, &err));

    DATA_TYPE_X *array_x;
    //DATA_TYPE_X *array_x_golden = new DATA_TYPE_X[col_size * no_vectors];
    DATA_TYPE_OUT *array_y;
    DATA_TYPE_OUT * array_y_golden = new DATA_TYPE_OUT[row_size * no_vectors];

    u32 S_begin = 0;
    u32 S_end = row_size;

    //Map buffers to userspace pointers
    OCL_CHECK(err, array_values = (DATA_TYPE*)q.enqueueMapBuffer(buffer_array_values, CL_TRUE, CL_MAP_WRITE, 0, nnz * sizeof(DATA_TYPE), nullptr, nullptr, &err));
    OCL_CHECK(err, array_colIndices = (u32*)q.enqueueMapBuffer(buffer_array_colIndices, CL_TRUE, CL_MAP_WRITE, 0, nnz * sizeof(u32), nullptr, nullptr, &err));
    OCL_CHECK(err, array_rowPtr = (u32*)q.enqueueMapBuffer(buffer_array_rowPtr, CL_TRUE, CL_MAP_WRITE, 0, (row_size + 1) * sizeof(u32), nullptr, nullptr, &err));
    //OCL_CHECK(err, array_x = (DATA_TYPE_X*)q.enqueueMapBuffer(buffer_array_x, CL_TRUE, CL_MAP_WRITE, 0, col_size * no_vectors * sizeof(DATA_TYPE_X)/4, nullptr, nullptr, &err));
	OCL_CHECK(err, array_x = (DATA_TYPE_X*)q.enqueueMapBuffer(buffer_array_x, CL_TRUE, CL_MAP_WRITE, 0, col_size * no_vectors * sizeof(DATA_TYPE_X)/4, nullptr, nullptr, &err));
	OCL_CHECK(err, array_y = (DATA_TYPE_OUT*)q.enqueueMapBuffer(buffer_array_y, CL_TRUE, CL_MAP_READ, 0, row_size * no_vectors * sizeof(DATA_TYPE_OUT), nullptr, nullptr, &err));
	
    //Initialization
    init_array(S_ternary, array_x, no_vectors, col_size);
	//init_array_golden(S_ternary, array_x_golden, no_vectors, col_size);
	
	std::cout << "col = " << col_size << std::endl;
	std::cout << "col>>2 = " << (col_size>>2) << std::endl;
	std::cout << "size of array_x = " << sizeof(array_x) << std::endl;
	std::cout << "size of array_x_golden = " << sizeof(array_x_golden) << std::endl;
	
	std::cout << "Complete : Init_arrays." << std::endl;
	
	if (fp_input != NULL) {
		char line_2[1000];
		u32 line_number = 0;
                while (fgets(line_2, sizeof(line_2), fp_input) != NULL) {
			if (line_number < nnz) {
				//std::cout << "has entered if, start to sscanf" << std::endl;
				sscanf(line_2, "%d %d", &c, &v);

				//printf("colindices %d val %f\n", c, v);
				//std::cout << "colindices" << c << " val " << v << std::endl;

				//*(array_colIndices + line_number) = c;
				array_colIndices[line_number] = c;
				//std::cout << "array_colIndices = " << array_colIndices[line_number] << std::endl;
				//*(array_values + line_number) = v;
				array_values[line_number] = v;
				//std::cout << "array_values = " << array_values[line_number] << std::endl;
				//std::cout << "(if) Pass 'something could go wrong' stage" << std::endl;

			}
			else {
				sscanf(line_2, "%d", &r);

				//printf("rowptr %d \n", r);
				//std::cout << "rowptr " << c << std::endl;
				//*(array_rowPtr + (line_number - (nnz))) = r;
				array_rowPtr[line_number - nnz] = r;
				//std::cout << "array_rowPtr = " << array_rowPtr[line_number - nnz] << std::endl;
				//std::cout << "(else) Pass 'something could go wrong' stage" << std::endl;
			}
			line_number++;
		}
	}
	std::cout << "Complete : Load_arrays." << std::endl;
	
	// Set the kernal argument
	int narg = 0;
    OCL_CHECK(err, err = krnl.setArg(narg++, S_ternary));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_rowPtr));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_colIndices));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_colIndices));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_colIndices));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_colIndices));
    OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_values));
    OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_values));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_values));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_values));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_y));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_y));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_y));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_y));
    OCL_CHECK(err, err = krnl.setArg(narg++, buffer_array_x));
    OCL_CHECK(err, err = krnl.setArg(narg++, no_vectors));
    OCL_CHECK(err, err = krnl.setArg(narg++, col_size));
    OCL_CHECK(err, err = krnl.setArg(narg++, row_size));
    OCL_CHECK(err, err = krnl.setArg(narg++, nnz));
    OCL_CHECK(err, err = krnl.setArg(narg++, S_begin));
    OCL_CHECK(err, err = krnl.setArg(narg++, S_end));
	OCL_CHECK(err, err = krnl.setArg(narg++, array_rowPtr[S_begin]));	
    
    // Date will be migrate to the kernal space
auto fpga_begin = std::chrono::high_resolution_clock::now();
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_array_values, buffer_array_colIndices, buffer_array_rowPtr, buffer_array_x}, 0));
    
    // Lauch the kernal
    OCL_CHECK(err, err = q.enqueueTask(krnl));
    
    // To view the results, this call will transfer the data from FPGA to the host

	// Rather than manually enqueueing a migration, we can instead just map the buffer. 
	// The OpenCL runtime will recognize that the buffer contents are currently resident in 
	// the Alveo Data Center accelerator card global memory and will take care of 
	// migrating the buffer back to the host for us. This is a coding style choice you must make.

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_array_y}, CL_MIGRATE_MEM_OBJECT_HOST));
    
    q.finish();
	auto fpga_end = std::chrono::high_resolution_clock::now();
	std::cout << "Complete : Kernel execution." << std::endl;

	std::cout << "Start : mmult_golden." << std::endl;
	
	auto cpu_begin = std::chrono::high_resolution_clock::now();
	if (S_ternary==0)
    	{
       	    	golden_spmm_byte(
		    array_values,
		    array_rowPtr,
		    array_colIndices,
		    array_x,
		    no_vectors,
		    array_y_golden,
		    row_size,
		    col_size
            	);
    	}
	else if (S_ternary==1)
    	{
            	golden_spmm_ternary(
		    array_values,
		    array_rowPtr,
		    array_colIndices,
		    array_x,
		    no_vectors,
		    array_y_golden,
		    row_size,
		    col_size
        	);
    	}
	else
    	{
        	golden_spmm_quad(
		    array_values,
		    array_rowPtr,
		    array_colIndices,
		    array_x,
		    no_vectors,
		    array_y_golden,
		    row_size,
		    col_size
        	);
   	}
   	auto cpu_end = std::chrono::high_resolution_clock::now();
	

    // Compare the results of the Device to the simulation
    std::cout << "Complete : mmult_golden." << std::endl;
	std::cout << "Start : result_check." << std::endl;

    if(result_check(array_y, array_y_golden, row_size, no_vectors))
        return 1;
	
	std::chrono::duration<double> fpga_duration = fpga_end - fpga_begin;
	std::chrono::duration<double> cpu_duration = cpu_end - cpu_begin;
	//float fpga_throughput = (double) numRuns*3*nbytes / fpga_duration.count() / (1024.0*1024.0);
     	//float cpu_throughput  = (double) numRuns*3*nbytes / cpu_duration.count() / (1024.0*1024.0);
	
	std::cout << std::endl;
	std::cout << "----------------------------------------------------------------------------"   << std::endl;
	std::cout << "         Performance  " << std::endl;
    	//std::cout << "          Total data: " << total << " MBits" << std::endl;
    	std::cout << "           FPGA Time: " << fpga_duration.count() * 1000.0 << " ms" << std::endl;
    	//std::cout << "     FPGA Throughput: " << total / fpga_duration.count() << " MBits/s" << std::endl;
    	//std::cout << "FPGA PCIe Throughput: " << (2*total) / fpga_duration.count() << " MBits/s" << std::endl;
	std::cout << "            CPU Time: " << cpu_duration.count() * 1000.0 << " ms" << std::endl;
	std::cout << "       FPGA Speedup : " << cpu_duration.count() / fpga_duration.count() << " x" << std::endl;
	std::cout << "----------------------------------------------------------------------------"   << std::endl;
	

	OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_array_values, array_values));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_array_colIndices, array_colIndices));
	OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_array_rowPtr, array_rowPtr));
	OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_array_x, array_x));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_array_y, array_y));
	q.finish();

}
