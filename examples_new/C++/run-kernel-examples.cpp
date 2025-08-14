#include <iostream>
#include <fstream>

#include <occa.hpp>

int main(int argc, const char **argv) {
    int numElems = 25 * 25; 
    float* A = new float[numElems];
    for (int i=0; i<numElems; ++i) {
        A[i] = i + 1.f; 
    }
    float* B = new float[numElems];


    occa::device device({{"mode", "Serial"}});

    occa::memory o_A = device.malloc<float>(numElems);
    occa::memory o_B = device.malloc<float>(numElems);
    o_A.copyFrom(A);

// ========== Kernel Example 01
    std::cout << "Running example kernel01" << std::endl;
    occa::kernel intro = device.buildKernel("kernel01-intro.okl","fillKernel");
    intro(numElems, o_B, 0.f);
    
// ========== Kernel Example 02
    occa::kernel loops = device.buildKernel("kernel02-loops.okl", "mandelbrot");
    //setup input arguments for mandelbrot
    float xMin = -2.0f;
    float xMax = 1.0f;
    float yMin = -1.0f;
    float yMax = 1.0f;
    int maxIter = 2000;
    //picture resolution
    int numX = 800;
    int numY = 400;
    
    occa::memory iters = device.malloc<int>(numX * numY);
    std::cout <<  "Running example kernel03" << std::endl;
    loops(xMax, yMax, xMin, yMin, maxIter, numX, numY, iters);

    // write iters to a file for plotting later 
    int* itersHost = new int[numX * numY * 5];
    iters.copyTo(itersHost);
    std::ofstream outFile("mandelbrot.txt");
    for (int i = 0; i < numX * numY; ++i) {
        outFile << itersHost[i] << ",";
        if (i % numX == 0) {
            outFile << std::endl;
        }
    }
    outFile.close(); 

    occa::kernel sync = device.buildKernel("kernel03-sync.okl", "reverseVector");
    occa::kernel atomics = device.buildKernel("kernel05-atomics.okl", "reduction");
    occa::kernel math = device.buildKernel("kernel06-math.okl", "mathExample");

    device.free(); 
    
    return 0;
}