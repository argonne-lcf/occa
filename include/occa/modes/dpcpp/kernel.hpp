#ifndef OCCA_MODES_DPCPP_KERNEL_HEADER
#define OCCA_MODES_DPCPP_KERNEL_HEADER

#include <occa/core/launchedKernel.hpp>
#include <occa/modes/dpcpp/polyfill.hpp>
#include <occa/modes/dpcpp/utils.hpp>
#include <CL/sycl.hpp>

namespace occa {
  namespace dpcpp {
    class device;

    class kernel : public occa::launchedModeKernel_t {
      friend class device;
      friend cl_kernel getCLKernel(occa::kernel kernel);

    private:
      ::sycl::device dpcppDevice;
      ::sycl::kernel dpcppKernel;
     

    public:
      kernel(modeDevice_t *modeDevice_,
             const std::string &name_,
             const std::string &sourceFilename_,
             const occa::properties &properties_);

      kernel(modeDevice_t *modeDevice_,
             const std::string &name_,
             const std::string &sourceFilename_,
             ::sycl::device dpcppDevice_,
             ::sycl::kernel dpcppKernel_,
             const occa::properties &properties_);

      ~kernel();

      queue& getCommandQueue() const;

      int maxDims() const;
      dim maxOuterDims() const;
      dim maxInnerDims() const;

      void deviceRun() const;
    };
  }
}

#endif
