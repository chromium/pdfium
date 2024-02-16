// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_STRING_DATA_TEMPLATE_H_
#define CORE_FXCRT_STRING_DATA_TEMPLATE_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

namespace fxcrt {

template <typename CharType>
class StringDataTemplate {
 public:
  static RetainPtr<StringDataTemplate> Create(size_t nLen);
  static RetainPtr<StringDataTemplate> Create(pdfium::span<const CharType> str);

  void Retain() { ++m_nRefs; }
  void Release();

  bool CanOperateInPlace(size_t nTotalLen) const {
    return m_nRefs <= 1 && nTotalLen <= m_nAllocLength;
  }

  void CopyContents(const StringDataTemplate& other);
  void CopyContents(pdfium::span<const CharType> str);
  void CopyContentsAt(size_t offset, pdfium::span<const CharType> str);

  pdfium::span<CharType> span() {
    return pdfium::make_span(m_String, m_nDataLength);
  }
  pdfium::span<const CharType> span() const {
    return pdfium::make_span(m_String, m_nDataLength);
  }

  // Includes the terminating NUL not included in lengths.
  pdfium::span<CharType> capacity_span() {
    return pdfium::make_span(m_String, m_nAllocLength + 1);
  }
  pdfium::span<const CharType> capacity_span() const {
    return pdfium::make_span(m_String, m_nAllocLength + 1);
  }

  // Unlike std::string::front(), this is always safe and returns a
  // NUL char when the string is empty.
  CharType Front() const { return !span().empty() ? span().front() : 0; }

  // Unlike std::string::back(), this is always safe and returns a
  // NUL char when the string is empty.
  CharType Back() const { return !span().empty() ? span().back() : 0; }

  // To ensure ref counts do not overflow, consider the worst possible case:
  // the entire address space contains nothing but pointers to this object.
  // Since the count increments with each new pointer, the largest value is
  // the number of pointers that can fit into the address space. The size of
  // the address space itself is a good upper bound on it.
  intptr_t m_nRefs = 0;

  // These lengths are in terms of number of characters, not bytes, and do not
  // include the terminating NUL character, but the underlying buffer is sized
  // to be capable of holding it.
  size_t m_nDataLength;
  const size_t m_nAllocLength;

  // Not really 1, variable size.
  CharType m_String[1];

 private:
  StringDataTemplate(size_t dataLen, size_t allocLen);
  ~StringDataTemplate() = delete;
};

extern template class StringDataTemplate<char>;
extern template class StringDataTemplate<wchar_t>;

}  // namespace fxcrt

using fxcrt::StringDataTemplate;

#endif  // CORE_FXCRT_STRING_DATA_TEMPLATE_H_
