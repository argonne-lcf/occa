#include <occa/internal/modes/dpcpp/utils.hpp>
#include <occa/internal/modes/dpcpp/memory.hpp>
#include <occa/internal/modes/dpcpp/device.hpp>
#include <occa/internal/modes/dpcpp/stream.hpp>
#include <occa/internal/modes/dpcpp/streamTag.hpp>
#include <occa/internal/utils/sys.hpp>

namespace occa
{
  namespace dpcpp
  {
    memory::memory(modeDevice_t *modeDevice_,
                   udim_t size_,
                   const occa::json &properties_)
        : occa::modeMemory_t(modeDevice_, size_, properties_)
    {
    }

    memory::~memory()
    {
      if (isOrigin)
      {
        auto& dpcpp_device = getDpcppDevice(modeDevice);
        OCCA_DPCPP_ERROR("Memory: Freeing SYCL alloc'd memory",
                         ::sycl::free(this->ptr,dpcpp_device.dpcppContext));
      }
      this->ptr = nullptr;
      size = 0;
    }

    void *memory::getKernelArgPtr() const
    {
      return ptr;
    }

    modeMemory_t *memory::addOffset(const dim_t offset)
    {
      memory *m = new memory(modeDevice,
                             size - offset,
                             properties);

      m->ptr = this->ptr + offset;
      return m;
    }

    void *memory::getPtr(const occa::json &props)
    {
      return this->ptr;
    }

    void memory::copyFrom(const void *src,
                          const udim_t bytes,
                          const udim_t offset,
                          const occa::json &props)
    {
      const bool async = props.get("async", false);

      occa::dpcpp::stream& q = getDpcppStream(modeDevice->currentStream);
      occa::dpcpp::streamTag e = q.memcpy(&(this->ptr)[offset], &((char *)src)[offset], bytes);

      if(!async)
        e.waitFor();
    }

    void memory::copyFrom(const modeMemory_t *src,
                          const udim_t bytes,
                          const udim_t destOffset,
                          const udim_t srcOffset,
                          const occa::json &props)
    {
      const bool async = props.get("async", false);

      occa::dpcpp::stream& q = getDpcppStream(modeDevice->currentStream);
      occa::dpcpp::streamTag e = q.memcpy(&(this->ptr)[destOffset], &(src->ptr)[srcOffset], bytes);

      if(!async)
        e.waitFor();
    }

    void memory::copyTo(void *dest,
                        const udim_t bytes,
                        const udim_t offset,
                        const occa::json &props) const
    {

      const bool async = props.get("async", false);

      occa::dpcpp::stream& q = getDpcppStream(modeDevice->currentStream);
      occa::dpcpp::streamTag e = q.memcpy(&((char *)dest)[offset], &(this->ptr)[offset], bytes);

      if(!async)
        e.waitFor();
    }

    void memory::detach()
    {
      size = 0;
    }

  } // namespace dpcpp
} // namespace occa
