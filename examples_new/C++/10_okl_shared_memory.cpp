#include <occa.hpp>

#include <iostream>
#include <vector>

namespace {

occa::device selectDevice(int device_id = 0, int platform_id = 0) {
  occa::json device_properties;
  device_properties["device_id"] = device_id;
  device_properties["platform_id"] = platform_id;
  device_properties["mode"] = "Serial";  // Default mode

  std::vector<std::string> preferred_modes = {"CUDA", "HIP", "dpcpp"};
  for (auto& mode : preferred_modes) {
    if (occa::modeIsEnabled(mode)) {
      device_properties["mode"] = mode;
      break;
    }
  }
  return occa::device(device_properties);
}

}  // namespace

int main() {
  occa::device occa_device = selectDevice();

  const int M = 8;
  const int N = 8192;
  const int size = M * M * N;

  std::vector<double> hX(size, 1.0);
  occa::memory dX = occa_device.malloc<double>(hX.size(), hX.data());
  dX.copyFrom(hX.data());

  std::vector<double> hY(size);
  occa::memory dY = occa_device.malloc<double>(hY.size());

  std::vector<double> hA(M * M, 1.0);
  occa::memory dA = occa_device.malloc<double>(hA.size(), hA.data());

  // Pass value of matrix size at kernel compile-time
  occa::json gemmProps({{"defines/p_M", M}});

  // Create a kernel defined in an external file
  const std::string kernel_name = "gemm";
  const std::string kernel_file = "kernels/gemm.okl";
  occa::kernel gemm_kernel = occa_device.buildKernel(kernel_file, kernel_name, gemmProps);
  // Kernel is jitted during construction

  // Call the kernel like any function
  gemm_kernel(N, dX, M, dA, dY);
  // Work on device occurs in-order

  // Copy back to the host
  dY.copyTo(hY.data());

  // Occa handles garbage collection for device memory
  // No need to explicitly call occa::free
  return 0;
}
