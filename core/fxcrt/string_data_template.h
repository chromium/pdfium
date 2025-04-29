// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_STRING_DATA_TEMPLATE_H_
#define CORE_FXCRT_STRING_DATA_TEMPLATE_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

namespace fxcrt {

template <typename CharType>
class StringDataTemplate {
 public:
  static RetainPtr<StringDataTemplate> Create(size_t nLen);
  static RetainPtr<StringDataTemplate> Create(pdfium::span<const CharType> str);

  void Retain() { ++refs_; }
  void Release();

  bool CanOperateInPlace(size_t nTotalLen) const {
    return refs_ <= 1 && nTotalLen <= alloc_length_;
  }

  void CopyContents(const StringDataTemplate& other);
  void CopyContents(pdfium::span<const CharType> str);
  void CopyContentsAt(size_t offset, pdfium::span<const CharType> str);

  pdfium::span<CharType> span() {
    // SAFETY: data_length_ is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, data_length_));
  }
  pdfium::span<const CharType> span() const {
    // SAFETY: data_length_ is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, data_length_));
  }

  // Only a const-form is provided to preclude modifying the terminator.
  pdfium::span<const CharType> span_with_terminator() const {
    // SAFETY: data_length_ is within string_ and there is always a
    // terminator character following it.
    return UNSAFE_BUFFERS(pdfium::span(string_, data_length_ + 1));
  }

  pdfium::span<CharType> alloc_span() {
    // SAFETY: alloc_length_ is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, alloc_length_));
  }
  pdfium::span<const CharType> alloc_span() const {
    // SAFETY: alloc_length_ is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, alloc_length_));
  }

  // Includes the terminating NUL not included in lengths.
  pdfium::span<CharType> capacity_span() {
    // SAFETY: alloc_length_ + 1 is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, alloc_length_ + 1));
  }
  pdfium::span<const CharType> capacity_span() const {
    // SAFETY: alloc_length_ + 1 is within string_.
    return UNSAFE_BUFFERS(pdfium::span(string_, alloc_length_ + 1));
  }

  // Return length as determined by the location of the first embedded NUL.
  size_t GetStringLength() const {
    return std::char_traits<CharType>::length(string_);
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
  intptr_t refs_ = 0;

  // These lengths are in terms of number of characters, not bytes, and do not
  // include the terminating NUL character, but the underlying buffer is sized
  // to be capable of holding it.
  size_t data_length_;
  const size_t alloc_length_;

  // Not really 1, variable size.
  CharType string_[1];

 private:
  StringDataTemplate(size_t dataLen, size_t allocLen);
  ~StringDataTemplate() = delete;
};

extern template class StringDataTemplate<char>;
extern template class StringDataTemplate<wchar_t>;

}  // namespace fxcrt

using fxcrt::StringDataTemplate;

#endif  // CORE_FXCRT_STRING_DATA_TEMPLATE_H_
