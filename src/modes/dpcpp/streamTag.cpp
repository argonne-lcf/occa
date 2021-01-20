#include <occa/modes/dpcpp/streamTag.hpp>
#include <occa/modes/dpcpp/utils.hpp>

namespace occa {
  namespace dpcpp {
    streamTag::streamTag(modeDevice_t *modeDevice_) : modeStreamTag_t(modeDevice_){}

    streamTag::~streamTag() {
    }

    double streamTag::getTime() {
	return 0.0;
    }
  }
}