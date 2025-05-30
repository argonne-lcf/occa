#include <occa.hpp>

#include <iostream>
#include <vector>

namespace {

  occa::device selectDevice(int device_id=0, int platform_id=0) {
    occa::json device_properties;
    device_properties["device_id"] = device_id;
    device_properties["platform_id"] = platform_id;
    device_properties["mode"] = "Serial"; // Default mode

    std::vector<std::string> preferred_modes = {"CUDA","HIP","dpcpp"};
    for (auto& mode : preferred_modes) {
      if (occa::modeIsEnabled(mode)) {
        device_properties["mode"] = mode;
        break;
      }
    }
    return occa::device(device_properties);
  }

}

int main() {

  occa::device occa_device = selectDevice();

  const int N = 1024 * 1024 * 1024;
  const double alpha = 1.0; 
  std::vector<double> hX(N,1.0);
  std::vector<double> hY(N,1.0);

  // Allocate + initialize device memory
  occa::memory dX = occa_device.malloc<double>(hX.size(),hX.data());
  occa::memory dY = occa_device.malloc<double>(hY.size(),hY.data());

  // Create a kernel defined in an external file
  const std::string kernel_name = "daxpy";
  const std::string kernel_file = "daxpy.okl";
  occa::kernel daxpy_kernel = occa_device.buildKernel(kernel_file,kernel_name);
  // Kernel is jitted during construction

  // Call the kernel like any function
  daxpy_kernel(N,dX,dY);
  // Work on device occurs in-order

  // Copy back to the host
  dY.copyTo(hY.data());

  // Verify the results
  bool is_correct = true;
  for (const auto& y : hY) {
    if (2.0 != y) is_correct = false;
  }

  if (is_correct) std::cout << "Success!\n";
  else std::cout << "Verification failed\n";

  return 0;
}
