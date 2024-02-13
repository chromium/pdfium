// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_STRING_TEMPLATE_H_
#define CORE_FXCRT_STRING_TEMPLATE_H_

#include <stddef.h>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/string_data_template.h"
#include "core/fxcrt/string_view_template.h"

namespace fxcrt {

// Base class for a  mutable string with shared buffers using copy-on-write
// semantics that avoids std::string's iterator stability guarantees.
template <typename T>
class StringTemplate {
 public:
  using CharType = T;
  using const_iterator = T*;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Holds on to buffer if possible for later re-use. Use assignment
  // to force immediate release if desired.
  void clear();

  // Increase the backing store of the string so that it is capable of storing
  // at least `nMinBufLength` chars. Returns a span to the entire buffer,
  // which may be larger than `nMinBufLength` due to rounding by allocators.
  // Note: any modification of the string (including ReleaseBuffer()) may
  // invalidate the span, which must not outlive its buffer.
  pdfium::span<T> GetBuffer(size_t nMinBufLength);

  // Sets the size of the string to `nNewLength` chars. Call this after a call
  // to GetBuffer(), to indicate how much of the buffer was actually used.
  void ReleaseBuffer(size_t nNewLength);

 protected:
  using StringView = StringViewTemplate<T>;
  using StringData = StringDataTemplate<T>;

  void ReallocBeforeWrite(size_t nNewLen);
  void AllocBeforeWrite(size_t nNewLen);
  void AssignCopy(const T* pSrcData, size_t nSrcLen);
  void Concat(const T* pSrcData, size_t nSrcLen);

  RetainPtr<StringData> m_pData;
};

extern template class StringTemplate<char>;
extern template class StringTemplate<wchar_t>;

}  // namespace fxcrt

#endif  // CORE_FXCRT_STRING_TEMPLATE_H_
