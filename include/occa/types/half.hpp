#ifndef OCCA_TYPES_HALF_HEADER
#define OCCA_TYPES_HALF_HEADER

#include <cstdint>
#include <cstring>

#include <occa/types/tuples.hpp>

namespace occa {
  // IEEE 754 binary16 storage-only host shim. No host arithmetic — `half_t`
  // exists so that `occa::memory<half_t>` and `kernelArg(const half_t&)` flow
  // through OCCA unchanged and arrive at CUDA/DPC++ kernels as bit-identical
  // `__half` / `sycl::half`.
  struct half_t {
    uint16_t bits;

    half_t() : bits(0) {}
    explicit half_t(uint16_t bits_) : bits(bits_) {}
    half_t(float f) : bits(fromFloat(f)) {}

    operator float() const { return toFloat(bits); }

    // Round-to-nearest-even fp32 → fp16. Handles subnormals, infinities,
    // NaN, and overflow-to-infinity per IEEE 754.
    static uint16_t fromFloat(float f) {
      uint32_t x;
      std::memcpy(&x, &f, sizeof(x));

      const uint32_t sign = (x >> 31) & 0x1u;
      const uint32_t exp  = (x >> 23) & 0xffu;
      uint32_t       mant = x & 0x7fffffu;

      if (exp == 0xff) {
        return static_cast<uint16_t>((sign << 15) | (0x1fu << 10) |
                                     (mant ? 0x200u : 0u));
      }
      if (exp == 0) {
        return static_cast<uint16_t>(sign << 15);
      }

      int32_t newExp = static_cast<int32_t>(exp) - 127 + 15;

      if (newExp >= 0x1f) {
        return static_cast<uint16_t>((sign << 15) | (0x1fu << 10));
      }

      if (newExp <= 0) {
        if (newExp < -10) {
          return static_cast<uint16_t>(sign << 15);
        }
        mant |= 0x800000u;
        const int shift = 14 - newExp;
        uint32_t   rounded = mant >> shift;
        const uint32_t roundBit = (mant >> (shift - 1)) & 1u;
        const uint32_t sticky   = mant & ((1u << (shift - 1)) - 1u);
        if (roundBit && (sticky || (rounded & 1u))) {
          rounded += 1u;
        }
        return static_cast<uint16_t>((sign << 15) | rounded);
      }

      uint32_t       rounded  = mant >> 13;
      const uint32_t roundBit = (mant >> 12) & 1u;
      const uint32_t sticky   = mant & 0xfffu;
      if (roundBit && (sticky || (rounded & 1u))) {
        rounded += 1u;
        if (rounded == 0x400u) {
          rounded = 0u;
          newExp += 1;
          if (newExp >= 0x1f) {
            return static_cast<uint16_t>((sign << 15) | (0x1fu << 10));
          }
        }
      }
      return static_cast<uint16_t>((sign << 15) |
                                   (static_cast<uint32_t>(newExp) << 10) |
                                   rounded);
    }

    static float toFloat(uint16_t h) {
      const uint32_t sign = (h >> 15) & 0x1u;
      const uint32_t exp  = (h >> 10) & 0x1fu;
      const uint32_t mant = h & 0x3ffu;

      uint32_t x;
      if (exp == 0) {
        if (mant == 0) {
          x = sign << 31;
        } else {
          int      e = -1;
          uint32_t m = mant;
          while ((m & 0x400u) == 0u) {
            m <<= 1;
            --e;
          }
          m &= 0x3ffu;
          x = (sign << 31) |
              (static_cast<uint32_t>(127 + e - 14) << 23) |
              (m << 13);
        }
      } else if (exp == 0x1f) {
        x = (sign << 31) | (0xffu << 23) | (mant << 13);
      } else {
        x = (sign << 31) |
            (static_cast<uint32_t>((static_cast<int>(exp) - 15) + 127) << 23) |
            (mant << 13);
      }
      float f;
      std::memcpy(&f, &x, sizeof(f));
      return f;
    }
  };

  static_assert(sizeof(half_t) == 2, "half_t must be 2 bytes");

  typedef type2<half_t> half2;
  typedef type4<half_t> half3;
  typedef type4<half_t> half4;
}

#endif
