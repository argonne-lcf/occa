#!/bin/bash

# FC=ifx
# -DCMAKE_Fortran_COMPILER=${FC} \

module purge
module use /home/bertoni/modulefiles
module load hipcl/experimental-13
#module load hipcl
module load intel_compute_runtime
module load cmake

#CC=icx
#CXX=icpx

CC=clang
CXX=clang++

BUILD_TYPE=RelWithDebInfo
BUILD_DIR=build
# INSTALL_DIR=install

#PREFIX_PATHS="${A21_SDK_ROOT}/compiler/latest/linux/;\
#${A21_SDK_ROOT}/compiler/latest/linux/include/sycl/;\
#${A21_SDK_ROOT}/compiler/latest/linux/include/sycl/CL"

HIP_PATH="$HIPCL_DIR"
LIBRARY_PATHS="/soft/libraries/khronos/loader/master-2021.03.29;${HIP_PATH}"
INCLUDE_PATHS="/soft/libraries/khronos/headers/master-2021.03.29/include;${HIP_PATH}/include"

# PREFIX_PATHS="${LIBRARY_PATHS};${INCLUDE_PATHS}"
# PREFIX_PATHS="${HIPCL_DIR}"


rm -rf $BUILD_DIR
cmake -S . -B ${BUILD_DIR} \
  -DHIP_PLATFORM=hipcl \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_PREFIX_PATH=${PREFIX_PATHS} \
  -DCMAKE_C_COMPILER=${CC} \
  -DCMAKE_CXX_COMPILER=${CXX} \
  -DENABLE_OPENMP="OFF" \
  -DENABLE_OPENCL="OFF" \
  -DENABLE_DPCPP="OFF" \
  -DENABLE_CUDA="OFF" \
  -DENABLE_HIP="ON" \
  -DENABLE_METAL="OFF" \
  -DENABLE_MPI="OFF" \
  -DENABLE_FORTRAN="OFF" \
  -DENABLE_TESTS="ON" \
  -DENABLE_EXAMPLES="ON"

#cmake --build build --parallel 4
#cmake --build build 
cd build
export OCCA_DPCPP_ENABLED=0
make -j24 VERBOSE=1

cd examples/cpp/01_add_vectors/
clang++ -std=c++17  -o main  main.cpp -I../../../${BUILD_DIR}/include -I../../../src -I../../../include -L../../../${BUILD_DIR}/lib    -locca -lm -lrt -ldl -lhipcl -lOpenCL

OCCA_CXXFLAGS="-std=c++11 -lhipcl -lOpenCL" OCCA_LDFLAGS="-lhipcl -lOpenCL" OCCA_VERBOSE=1 OCCA_HIP_COMPILER="clang++ -c -lhipcl -lOpenCL" OCCA_HIP_COMPILER_FLAGS="-std=c++11 -lhipcl -lOpenCL" OCCA_CXX="clang++ -lhipcl -lOpenCL"   LD_LIBRARY_PATH=../../../build/lib:$LD_LIBRARY_PATH ./main -d "{mode: 'HIP',device_id: 0}"
