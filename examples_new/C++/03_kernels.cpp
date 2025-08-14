#include <iostream>

#include <occa.hpp>



int main(int argc, const char **argv) {


    // Set up information on host memory 
    int entries = 12;

    float *a  = new float[entries];
    float *b  = new float[entries];
    float *ab = new float[entries];

    for (int i = 0; i < entries; ++i) {
        a[i]  = i;
        b[i]  = 1 - i;
        ab[i] = 1;
    }



    // Initialize device and allocate memory
    occa::device device({{"mode", "Serial"}});

    occa::memory o_a, o_b, o_ab;

    o_a = device.malloc<float>(entries);
    o_b = device.malloc<float>(entries);
    o_ab = device.malloc<float>(entries);

    // Copy memory to the device
    o_a.copyFrom(a);
    o_b.copyFrom(b);

  
    // // Compile at runtime    
    occa::kernel addVectors = device.buildKernel("addVectors.okl","addVectors");

    // Launch device kernel
    addVectors(entries, o_a, o_b, o_ab);

    // Copy result to the host
    o_ab.copyTo(ab);

    // Verify results
    for (int i = 0; i < entries; ++i) {
        if (!occa::areBitwiseEqual(ab[i], a[i] + b[i])) {
            std::cout << ab[i]  << std::endl; 
        }
    }

    // Free device -- allocated memory, built kernels, created streams and streamTags
    device.free(); 

    // Free host memory
    delete [] a;
    delete [] b;
    delete [] ab;


    return 0;
}
