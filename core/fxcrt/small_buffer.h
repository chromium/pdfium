// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_SMALL_BUFFER_H_
#define CORE_FXCRT_SMALL_BUFFER_H_

#include <string.h>

#include <array>
#include <memory>

#include "core/fxcrt/fx_memory_wrappers.h"

namespace fxcrt {

template <class T, size_t FixedSize>
class SmallBuffer {
 public:
  explicit SmallBuffer(size_t actual_size) : m_pSize(actual_size) {
    if (actual_size > FixedSize) {
      m_pDynamicData.reset(FX_Alloc(T, actual_size));
      return;
    }
    if (actual_size)
      memset(m_FixedData.data(), 0, sizeof(T) * actual_size);
  }
  size_t size() const { return m_pSize; }
  T* data() {
    return m_pDynamicData ? m_pDynamicData.get() : m_FixedData.data();
  }
  T* begin() { return data(); }
  T* end() { return begin() + size(); }

  // Callers shouldn't need to know these details.
  T* fixed_for_test() { return m_FixedData.data(); }
  T* dynamic_for_test() { return m_pDynamicData.get(); }

 private:
  const size_t m_pSize;
  std::unique_ptr<T, FxFreeDeleter> m_pDynamicData;
  std::array<T, FixedSize> m_FixedData;
};

}  // namespace fxcrt

#endif  // CORE_FXCRT_SMALL_BUFFER_H_
