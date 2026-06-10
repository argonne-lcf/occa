#include <vector>

#include <occa.hpp>
#include <occa/internal/modes.hpp>
#include <occa/internal/utils/testing.hpp>

void testHostShim();
void testKernelOnMode(const std::string &mode);

int main(const int argc, const char **argv) {
  testHostShim();

  // Half-precision kernels are only meaningful on backends whose code-gen
  // injects a real fp16 type. Serial/OpenMP would compile `half` as an
  // unknown C++ identifier on the host compiler, so skip them.
  occa::strToModeMap &modes = occa::getModeMap();
  for (const std::string &mode : {"CUDA", "dpcpp"}) {
    if (modes.find(mode) != modes.end()) {
      testKernelOnMode(mode);
    } else {
      std::cout << "Skipping half-precision kernel test for [" << mode
                << "] mode (not enabled in this build).\n";
    }
  }

  return 0;
}

void testHostShim() {
  ASSERT_EQ(2, (int) sizeof(occa::half_t));
  ASSERT_EQ(4, (int) sizeof(occa::half2));
  ASSERT_EQ(8, (int) sizeof(occa::half4));
  ASSERT_EQ(8, (int) sizeof(occa::half3));

  ASSERT_EQ(occa::dtype::half_,
            occa::dtype::get<occa::half_t>());

  // Round-trip a few representative fp32 values through the storage shim.
  const std::vector<float> probes = {
    0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 1.5f, 1024.0f, -1024.0f
  };
  for (float f : probes) {
    occa::half_t h(f);
    float back = (float) h;
    ASSERT_EQ(f, back);
  }
}

void testKernelOnMode(const std::string &mode) {
  std::cout << "Testing half-precision kernel on mode: " << mode << '\n';

  occa::device device({{"mode", mode}});
  if (device.mode() != mode) {
    // OCCA fell back to Serial because the requested mode failed init.
    std::cout << "  Requested " << mode << " but got " << device.mode()
              << " — skipping kernel run.\n";
    return;
  }

  const int N = 256;

  std::vector<occa::half_t> hostA(N), hostB(N), hostC(N);
  for (int i = 0; i < N; ++i) {
    hostA[i] = occa::half_t(static_cast<float>(i));
    hostB[i] = occa::half_t(static_cast<float>(2 * i));
  }

  occa::memory devA = device.malloc<occa::half_t>(N, hostA.data());
  occa::memory devB = device.malloc<occa::half_t>(N, hostB.data());
  occa::memory devC = device.malloc<occa::half_t>(N);

  const std::string kernelSource =
    "@kernel void add(const int N,                    \n"
    "                 const half *a,                  \n"
    "                 const half *b,                  \n"
    "                 half *c) {                      \n"
    "  for (int i = 0; i < N; ++i; @tile(32, @outer, @inner)) {\n"
    "    c[i] = a[i] + b[i];                          \n"
    "  }                                              \n"
    "}                                                \n";

  occa::kernel add = device.buildKernelFromString(kernelSource, "add");

  add(N, devA, devB, devC);

  devC.copyTo(hostC.data());

  for (int i = 0; i < N; ++i) {
    const float expected = static_cast<float>(3 * i);
    const float actual   = static_cast<float>(hostC[i]);
    // fp16 has ~3-4 decimal digits of precision; tolerate a relative
    // error of 1e-2 plus a small absolute floor.
    const float diff = std::abs(actual - expected);
    const float tol  = 1e-2f * std::abs(expected) + 1e-3f;
    ASSERT_TRUE(diff <= tol);
  }
}
