#include <iostream>

#include <occa.hpp>


// adapt this for two kernels: float and int



int main(int argc, const char **argv) {


    // Set up information on host memory 
    int entries = 12;

    float *a_float  = new float[entries];
    float *b_float  = new float[entries];
    float *ans_float = new float[entries];
    int *a_int = new int[entries];
    int *b_int = new int[entries];
    int *ans_int = new int[entries];

    for (int i = 0; i < entries; ++i) {
        a_float[i]  = (float)i;
        a_int[i] = i;
        b_float[i]  = (float)(1-i);
        b_int[i] = 1 - i;    
        ans_float[i] = 1.0f;
        ans_int[i] = 1;
    }

    // initialize device and allocate memory
    occa::device device({{"mode", "Serial"}});
    occa::memory o_a_float, o_b_float, o_ans_float;
    o_a_float = device.malloc<float>(entries);
    o_b_float = device.malloc<float>(entries);
    o_ans_float = device.malloc<float>(entries);

    occa::memory o_a_int, o_b_int, o_ans_int;
    o_a_int = device.malloc<int>(entries);
    o_b_int = device.malloc<int>(entries);
    o_ans_int = device.malloc<int>(entries);

    // Pass data type and number of items per thread at compile time 
    occa::properties axpyPropsFloat;
    axpyPropsFloat["defines/exampleDtype"] = "float";
    axpyPropsFloat["defines/numPerThread"] = "4";

    // Compile and launch float kernel 
    occa::kernel axpy_float = device.buildKernel("axpy_defines.okl", "axpy", axpyPropsFloat);
    axpy_float(entries, 1.0f, o_a_float, o_b_float, o_ans_float);
    o_ans_float.copyTo(ans_float);

    // Compile and launch int kernel
    occa::properties axpyPropsInt;
    axpyPropsInt["defines/exampleDtype"] = "int";
    axpyPropsInt["defines/numPerThread"] = "8";
    occa::kernel axpy_int = device.buildKernel("axpy_defines.okl", "axpy", axpyPropsInt);
    axpy_int(entries, 1, o_a_int, o_b_int, o_ans_int);
    o_ans_int.copyTo(ans_int);

    

    // Verify results
    for (int i = 0; i < entries; ++i) {
        if (abs(a_float[i] + b_float[i] - ans_float[i]) > .0001) {
            std::cerr << "Error at float index " << i << ": "
                      << "Expected " << (a_float[i] + b_float[i])
                      << ", got " << ans_float[i] << std::endl;
        }
        if (a_int[i] + b_int[i] != ans_int[i]) { 
            std::cerr << "Error at int index " << i << ": "
                      << "Expected " << (a_int[i] + b_int[i])
                      << ", got " << ans_int[i] << std::endl;
        }
    }
    std::cout << "All results verified successfully!" << std::endl;

    // Free device -- allocated memory, built kernels, created streams and streamTags
    device.free(); 

    // Free host memory
    delete [] a_float;
    delete [] b_float;
    delete [] ans_float;
    delete [] a_int;
    delete [] b_int;
    delete [] ans_int;

    return 0;
}

