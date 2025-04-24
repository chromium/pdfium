// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_
#define CORE_FXCRT_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_

// IWYU pragma: private, include "core/fxcrt/numerics/safe_conversions.h"

#include <stdint.h>

#include <algorithm>
#include <concepts>
#include <type_traits>

#include "core/fxcrt/numerics/safe_conversions_impl.h"

namespace pdfium {
namespace internal {

// Fast saturation to a destination type.
template <typename Dst, typename Src>
struct SaturateFastAsmOp {
  static constexpr bool is_supported =
      kEnableAsmCode && std::signed_integral<Src> && std::integral<Dst> &&
      kIntegerBitsPlusSign<Src> <= kIntegerBitsPlusSign<int32_t> &&
      kIntegerBitsPlusSign<Dst> <= kIntegerBitsPlusSign<int32_t> &&
      !kIsTypeInRangeForNumericType<Dst, Src>;

  __attribute__((always_inline)) static Dst Do(Src value) {
    int32_t src = value;
    if constexpr (std::is_signed_v<Dst>) {
      int32_t result;
      asm("ssat %[dst], %[shift], %[src]"
          : [dst] "=r"(result)
          : [src] "r"(src), [shift] "n"(
                                std::min(kIntegerBitsPlusSign<Dst>, 32)));
      return static_cast<Dst>(result);
    } else {
      uint32_t result;
      asm("usat %[dst], %[shift], %[src]"
          : [dst] "=r"(result)
          : [src] "r"(src), [shift] "n"(
                                std::min(kIntegerBitsPlusSign<Dst>, 31)));
      return static_cast<Dst>(result);
    }
  }
};

}  // namespace internal
}  // namespace pdfium

#endif  // CORE_FXCRT_NUMERICS_SAFE_CONVERSIONS_ARM_IMPL_H_
