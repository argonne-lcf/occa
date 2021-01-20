#include <occa/core/base.hpp>
#include <occa/internal/utils/env.hpp>
#include <occa/internal/utils/sys.hpp>
#include <occa/internal/modes/dpcpp/device.hpp>
#include <occa/internal/modes/dpcpp/kernel.hpp>
#include <occa/internal/modes/dpcpp/memory.hpp>
#include <occa/internal/modes/dpcpp/stream.hpp>
#include <occa/internal/modes/dpcpp/streamTag.hpp>
#include <occa/internal/modes/dpcpp/utils.hpp>
#include <occa/internal/modes/serial/device.hpp>
#include <occa/internal/modes/serial/kernel.hpp>
#include <occa/types/primitive.hpp>
#include <occa/internal/lang/kernelMetadata.hpp>
#include <occa/internal/lang/modes/dpcpp.hpp>

namespace occa
{
  namespace dpcpp
  {
    device::device(const occa::json &properties_)
        : occa::launchedModeDevice_t(properties_)
    {
      if (!properties.has("wrapped"))
      {
        platformID = getPlatformID(properties);
        deviceID = getDeviceID(properties);
        dpcppDevice = ::sycl::device(getDeviceByID(platformID, deviceID));

        std::cout << "Target Device is: " << dpcppDevice.get_info<::sycl::info::device::name>() << "\n";
        //        dpcppContext = clCreateContext(NULL, 1, &clDevice, NULL, NULL, &error);
        //        OCCA_DPCPP_ERROR("Device: Creating Context", error);
      }

      occa::json &kernelProps = properties["kernel"];
      std::string compilerFlags;

      //@todo: Refactor to utils
      if (env::var("OCCA_DPCPP_COMPILER_FLAGS").size())
      {
        compilerFlags = env::var("OCCA_DPCPP_COMPILER_FLAGS");
      }
      else if (kernelProps.has("compiler_flags"))
      {
        compilerFlags = (std::string)kernelProps["compiler_flags"];
      }

      kernelProps["compiler_flags"] = compilerFlags;
      kernelProps["compiler"] = "dpcpp";
      kernelProps["compiler_linker_flags"] = "-shared -fPIC";
    }

    //@todo: add error handling
    void device::finish() const
    {
      getCommandQueue()->wait();
    }

    //@todo: update kernel hashing
    hash_t device::hash() const
    {
      if (!hash_.initialized)
      {
        std::stringstream ss;
        ss << "platform: " << platformID << ' '
           << "device: " << deviceID;
        hash_ = occa::hash(ss.str());
      }
      return hash_;
    }

    hash_t device::kernelHash(const occa::json &props) const
    {
      return (
          occa::hash(props["compiler_flags"]) ^ props["compiler_flags"]);
    }

    lang::okl::withLauncher *device::createParser(const occa::json &props) const
    {
      return new lang::okl::dpcppParser(props);
    }

    //---[ Stream ]---------------------
    modeStream_t *device::createStream(const occa::json &props)
    {
      ::sycl::queue *q = new ::sycl::queue(dpcppDevice);
      return new stream(this, props, q);
    }

    occa::streamTag device::tagStream()
    {
      ::sycl::event dpcpp_event;
      return new occa::dpcpp::streamTag(this, dpcpp_event);
    }

    void device::waitFor(occa::streamTag tag)
    {
    }

    double device::timeBetween(const occa::streamTag &startTag,
                               const occa::streamTag &endTag)
    {
      occa::dpcpp::streamTag *dpcppStartTag = (dynamic_cast<occa::dpcpp::streamTag *>(startTag.getModeStreamTag()));
      occa::dpcpp::streamTag *dpcppEndTag = (dynamic_cast<occa::dpcpp::streamTag *>(endTag.getModeStreamTag()));

      finish();

      return (dpcppEndTag->getTime() - dpcppStartTag->getTime());
    }

    ::sycl::queue *device::getCommandQueue() const
    {
      occa::dpcpp::stream *stream = (occa::dpcpp::stream *)currentStream.getModeStream();
      return stream->commandQueue;
    }
    //==================================

    //---[ Kernel ]---------------------
    modeKernel_t *device::buildKernelFromProcessedSource(
        const hash_t kernelHash,
        const std::string &hashDir,
        const std::string &kernelName,
        const std::string &sourceFilename,
        const std::string &binaryFilename,
        const bool usingOkl,
        lang::sourceMetadata_t &launcherMetadata,
        lang::sourceMetadata_t &deviceMetadata,
        const occa::json &kernelProps,
        io::lock_t lock)
    {
      return nullptr;
    }

    void device::setArchCompilerFlags(occa::json &kernelProps) {}

    void device::compileKernel(const std::string &hashDir,
                               const std::string &kernelName,
                               const occa::json &kernelProps,
                               io::lock_t &lock) {}

    modeKernel_t *device::buildOKLKernelFromBinary(const hash_t kernelHash,
                                                   const std::string &hashDir,
                                                   const std::string &kernelName,
                                                   lang::sourceMetadata_t &launcherMetadata,
                                                   lang::sourceMetadata_t &deviceMetadata,
                                                   const occa::json &kernelProps,
                                                   io::lock_t lock)
    {
      return nullptr;
    }

    modeKernel_t *device::buildKernelFromBinary(const std::string &filename,
                                                const std::string &kernelName,
                                                const occa::json &kernelProps)
    {
      return nullptr;
    }
    //==================================

    //---[ Memory ]---------------------
    modeMemory_t *device::malloc(const udim_t bytes,
                                 const void *src,
                                 const occa::json &props)
    {
      if (props.get("mapped", false))
        return mappedAlloc(bytes, src, props);

      memory *mem = new memory(this, bytes, props);

      ::sycl::queue *q = getCommandQueue();

      mem->ptr = static_cast<char *>(malloc_device(bytes, *q));

      if (nullptr != src)
      {
        q->memcpy(mem->ptr, src, bytes);
        q->wait();
      }

      return mem;
    }

    modeMemory_t *device::mappedAlloc(const udim_t bytes,
                                      const void *src,
                                      const occa::json &props)
    {
      memory *mem = new memory(this, bytes, props);

      ::sycl::queue *q = getCommandQueue();

      mem->ptr = static_cast<char *>(malloc_host(bytes, *q));

      if (nullptr != src)
      {
        q->memcpy(mem->ptr, src, bytes);
        finish();
      }
      return mem;
    }

    modeMemory_t *device::unifiedAlloc(const udim_t bytes,
                                       const void *src,
                                       const occa::json &props)
    {
      return nullptr;
    }

    modeMemory_t *device::wrapMemory(const void *ptr,
                               const udim_t bytes,
                               const occa::json &props)
    {
      memory *mem = new memory(this,
                               bytes,
                               props);

      mem->ptr = (char *)ptr;

      return mem;
    }

    udim_t device::memorySize() const
    {
      return dpcpp::getDeviceMemorySize(dpcppDevice);
    }
    //==================================
  } // namespace dpcpp
} // namespace occa
