#include <iostream>

#include <occa.hpp>


int main(int argc, const char **argv) {

    // Declare and initialize host memory

    int entries = 12;

    float *a  = new float[entries];
    float *b  = new float[entries];
    float *ab = new float[entries];

    for (int i = 0; i < entries; ++i) {
        a[i]  = i;
        b[i]  = 1 - i;
        ab[i] = 0;
    }


    // Decalre and initialize the device 

    occa::device device({{"mode", "Serial"}});

    // Allocate memory on the device
    
    occa::memory o_a, o_b;

    o_a = device.malloc<float>(entries);

    // We can also allocate memory without a dtype
    // WARNING: This will disable runtime type checking
    o_b = device.malloc(entries * sizeof(float));

    // Cast o_b to float
    occa::memory casted_o_b = o_b.cast(occa::dtype::get<float>());

    // Copy memory from the host to the device
    o_a.copyFrom(a);
    o_b.copyFrom(b);
    casted_o_b.copyFrom(b);

    // Check how much memory we have allocated, and how much is remaining
    std::cout << "========= Device Memory Info =========\n";
    std::cout << "Device memory allocated: " << device.memoryAllocated() << " bytes\n";
    std::cout << "Device memory size: " << device.memorySize() << " bytes\n";
    // std::cout << "Device memory remaining: " << device.availableMemory() << " bytes\n";
    // todo the above line is implemented a different branch 
    
    // Verify the data type 
    std::cout << "\n========= Verify the data type =========\n";
    std::cout << "Data type of o_a: " << o_a.dtype().name() << '\n';
    std::cout << "Data type of o_b: " << o_b.dtype().name() << '\n';
    std::cout << "Data type of casted_o_b: " << casted_o_b.dtype().name() << '\n';

    // Verify the element count of the memory objects
    std::cout << "\n========= Verify the element count =========\n";
    std::cout << "Length of o_a: " << o_a.length() << " elements\n";
    std::cout << "Length of o_b: " << o_b.length() << " elements\n";
    std::cout << "Length of casted_o_b: " << casted_o_b.length() << " elements\n";

    // Verify the byte size of the memory objects
    std::cout << "\n========= Verify the byte size =========\n";
    std::cout << "Byte size of o_a: " << o_a.byte_size() << " bytes\n";
    std::cout << "Byte size of o_b: " << o_b.byte_size() << " bytes\n";
    std::cout << "Byte size of casted_o_b: " << casted_o_b.byte_size() << " bytes\n";   

    // Copy back to the host
    o_a.copyTo(a);


    // Memory Slicing
    // Create two slices of o_a
    occa::memory firstHalf = o_a.slice(0, entries/2);
    occa::memory secondHalf = o_a.slice(entries/2, entries/2); 

    // Verify the element count of each 
    std::cout << "\n========= Verify the slice length =========\n";
    std::cout << "Length of firstHalf: " << firstHalf.length() << " elements\n";
    std::cout << "Length of secondHalf: " << secondHalf.length() << " elements\n";

  

    // Free device -- allocated memory, built kernels, created streams and streamTags
    device.free(); 

    // Free host memory
    delete [] a;
    delete [] b;
    delete [] ab;


    return 0;
}
