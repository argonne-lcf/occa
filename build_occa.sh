#!/bin/bash

# FC=ifx
# -DCMAKE_Fortran_COMPILER=${FC} \

module purge
module use /home/bertoni/modulefiles
module load hipcl/20210525-release
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
make -j8 VERBOSE=1
