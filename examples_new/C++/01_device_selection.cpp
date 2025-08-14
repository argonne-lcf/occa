#include <iostream>

#include <occa.hpp>

// A helper function
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

int main(int argc, const char **argv) {


    // Declare and initialize the device 
    occa::device cudaDevice;
    cudaDevice.setup({
        {"mode", "cuda"}, 
        {"device_id", 0} 
    });
    
    // Alternatively, pass the same json into the device constructor  
    // This is the defaults, equivalent to `occa::device device()`; 
    occa::device defaultDevice({{"mode", "Serial"}});

    occa::device openmpSerialDevice({
        {"mode", "OpenMP"},
        {"schedule", "compact"},
        {"chunk", 10}
    });

    // The other support backends are: "HIP", "OpenCL", "Metal", "dpcpp"

    occa::device availableDevice = selectDevice();

    // View device information
    std::cout << "CUDA Device:\n" << cudaDevice.arch() << std::endl;
    std::cout << "\nDefault Device:\n" << defaultDevice.arch() << std::endl;
    std::cout << "\nOpenMP Serial Device:\n" << openmpSerialDevice.arch() << std::endl;
    std::cout << "\nSelected device:\n" << availableDevice.arch() << std::endl; 

    // Free devices, which deletes allocated memory, built kernels, streams, and streamTags
    cudaDevice.free();
    defaultDevice.free();
    openmpSerialDevice.free();
    availableDevice.free(); 


    return 0;
}