// Copyright 2019 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_BYTEORDER_H_
#define CORE_FXCRT_BYTEORDER_H_

#include "build/build_config.h"
#include "third_party/base/sys_byteorder.h"

namespace fxcrt {

// Converts the bytes in |x| from host order (endianness) to little endian, and
// returns the result.
inline uint16_t ByteSwapToLE16(uint16_t x) {
  return pdfium::base::ByteSwapToLE16(x);
}

inline uint32_t ByteSwapToLE32(uint32_t x) {
  return pdfium::base::ByteSwapToLE32(x);
}

// Converts the bytes in |x| from host order (endianness) to big endian, and
// returns the result.
inline uint16_t ByteSwapToBE16(uint16_t x) {
#if defined(ARCH_CPU_LITTLE_ENDIAN)
  return pdfium::base::ByteSwap(x);
#else
  return x;
#endif
}

inline uint32_t ByteSwapToBE32(uint32_t x) {
#if defined(ARCH_CPU_LITTLE_ENDIAN)
  return pdfium::base::ByteSwap(x);
#else
  return x;
#endif
}

}  // namespace fxcrt

#endif  // CORE_FXCRT_BYTEORDER_H_
