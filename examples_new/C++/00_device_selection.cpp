#include <occa.hpp>

#include <vector>
#include <iostream>

namespace {

  occa::device selectDevice(int device_id = 0, int platform_id = 0) {
    occa::json device_properties;
    device_properties["device_id"] = device_id;
    device_properties["platform_id"] = platform_id;
    device_properties["mode"] = "Serial"; // Default mode

    std::vector<std::string> preferred_modes = {"CUDA","HIP","dpcpp","OpenCL","OpenMP"};
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

  int device_id = 0;
  
  // Use a helper function
  occa::device occa_device_1 = selectDevice(device_id); 
  std::cout << occa_device_1.mode() << " mode was selected\n";

  // Fancy initialization using a lambda
  // Leave out OpenCL and OpenMP mode this time.
  std::vector<std::string> preferred_modes = {"CUDA", "HIP", "dpcpp"};
  int platform_id = 0;
  
  occa::device occa_device_2 = [&]() {
    for (auto& mode : preferred_modes) {
      if (occa::modeIsEnabled(mode)) {
        // Define the json properties inline
        return occa::device({{"mode", mode}, {"device_id", device_id}, {"platform_id", platform_id}});
      }
    }
    return occa::device({{"mode","Serial"}});
  }();
  std::cout << occa_device_2.mode() << " mode was selected\n";

  return 0;
}
