// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_CODE_POINT_VIEW_H_
#define CORE_FXCRT_CODE_POINT_VIEW_H_

#include "build/build_config.h"
#include "core/fxcrt/string_view_template.h"
#include "third_party/base/check_op.h"

namespace pdfium {

#if defined(WCHAR_T_IS_UTF16)
// A view over a UTF-16 `WideStringView` suitable for iterating by code point
// using a range-based `for` loop.
class CodePointView final {
 public:
  class Iterator {
   public:
    bool operator==(const Iterator& other) const {
      return current_ == other.current_;
    }

    bool operator!=(const Iterator& other) const {
      return current_ != other.current_;
    }

    Iterator& operator++() {
      DCHECK_LT(current_, end_);
      current_ += code_point_ < kFirstSupplementary ? 1 : 2;
      code_point_ = Decode();
      return *this;
    }

    char32_t operator*() const {
      DCHECK_NE(kSentinel, code_point_);
      return code_point_;
    }

   private:
    friend class CodePointView;

    static constexpr char32_t kSentinel = -1;
    static constexpr char32_t kFirstSupplementary = 0x10000;

    Iterator(const wchar_t* begin, const wchar_t* end)
        : current_(begin), end_(end), code_point_(Decode()) {}

    char32_t Decode() {
      if (current_ >= end_) {
        // No remaining code units.
        return kSentinel;
      }

      char32_t code_point = *current_;
      if (code_point >= 0xd800 && code_point < 0xdc00) {
        // First code unit is a high surrogate.
        const wchar_t* next = current_ + 1;
        if (next < end_ && *next >= 0xdc00 && *next < 0xe000) {
          // Second code unit is a low surrogate.
          code_point = (code_point & 0x3ff) << 10;
          code_point |= *next & 0x3ff;
          code_point += kFirstSupplementary;
        }
      }

      return code_point;
    }

    const wchar_t* current_;
    const wchar_t* end_;
    char32_t code_point_;
  };

  explicit CodePointView(WideStringView backing)
      : begin_(backing.begin()), end_(backing.end()) {
    DCHECK_LE(begin_, end_);
  }

  Iterator begin() const { return Iterator(begin_, end_); }

  Iterator end() const { return Iterator(end_, end_); }

 private:
  // Note that a `WideStringView` member would make the constructor too complex.
  const wchar_t* begin_;
  const wchar_t* end_;
};
#else
using CodePointView = WideStringView;
#endif  // defined(WCHAR_T_IS_UTF16)

}  // namespace pdfium

#endif  // CORE_FXCRT_CODE_POINT_VIEW_H_
