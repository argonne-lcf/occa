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
  std::cout << occa_device.mode() << " mode was selected\n";

  std::vector<int> hX(100, 1);
  std::vector<double> hY(101, 1.0);

  // Allocate, then initialize via copy to the device
  occa::memory dX = occa_device.malloc<int>(hX.size());
  dX.copyFrom(hX.data());

  // Allocate + initialize
  occa::memory dY = occa_device.malloc<double>(hY.size(), hY.data());

  // Copies between host and device are synchronous by default

  for (auto& x : hX) x = -1;
  for (auto& y : hY) y = -1.0;

  // Copy back to the host
  dX.copyTo(hX.data());
  dY.copyTo(hY.data());

  // Verify the results
  bool x_is_correct = true;
  for (const auto& x : hX) {
    if (1 != x) x_is_correct = false;
  }

  bool y_is_correct = true;
  for (const auto& y : hY) {
    // Don't check for equality of floating-point values in produciton code!
    if (1.0 != y) y_is_correct = false;
  }

  if (x_is_correct && y_is_correct) {
    std::cout << "Success!\n";
  } else {
    if (!x_is_correct) std::cout << "Verification failed for X\n";
    if (!y_is_correct) std::cout << "Verification failed for Y\n";
  }

  // Occa handles garbage collection for device memory
  // No need to explicitly call occa::free
  return 0;
}
