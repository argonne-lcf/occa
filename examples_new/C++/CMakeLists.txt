if (NOT TARGET OCCA::libocca)
  find_package(OCCA REQUIRED)
endif()

function(add_occa_cpp_example example_name)
  add_executable(${example_name} ${example_name}.cpp)
  target_link_libraries(${example_name} OCCA::libocca)
endfunction()

add_occa_cpp_example(00_device_selection)
add_occa_cpp_example(01_memory_management)
add_occa_cpp_example(02_kernels)


add_occa_cpp_example(01_device_selection)
add_occa_cpp_example(02_memory_management)
add_occa_cpp_example(03_kernels)
add_occa_cpp_example(04_jitting)
add_occa_cpp_example(05_streams) 
add_occa_cpp_example(06_memoryPools)
add_occa_cpp_example(run-kernel-examples)
