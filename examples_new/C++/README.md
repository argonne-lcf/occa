# C++ Examples

## Runtime Examples

There are 6 runtime examples, covering the following topics: 
- Device selection
  - How to initialize a device
  - A helper function for choosing the best available backend
- Memory management
  - How to allocate memory on the device
  - How to transfer memory to and from the host
  - Casting types
  - Slicing
  - Attributes availble for the memory type
- Building and launching kernels
  - Building and launching a kernel
- Just-in-time compiling
  - Additional parameters (defines)
- Streams
  - Creating blocking or nonblocking streams
  - Changing between streams and queueing different work items to them 
  - Using streamTags to mark events 
- Memory Pools
  - Creating memory pools
  - Slicing
  - Adding reservations
  - Resizing 


The associated kernels are located in the `runtime_examples_kernels` folder 


## Kernel Examples 

There are also 5 OKL kernel language examples:
- A syntax introduction with a simple fill kernel
- A more complicated nested loop example to generate the mandelbrot set
  - a python script for plotting the resulting image is located in `mandelbrot-plot.py`
- Shared memory and barriers 
- Atomic operations
- Standard math functions
A script to run all of the kernel examples is `run-kernel-examples.cpp`.

