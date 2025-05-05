// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/bytestring.h"

#include <ctype.h>
#include <stddef.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/string_pool_template.h"

// Instantiate.
template class fxcrt::StringViewTemplate<char>;
template class fxcrt::StringPoolTemplate<ByteString>;
template struct std::hash<ByteString>;

namespace {

constexpr char kTrimChars[] = "\x09\x0a\x0b\x0c\x0d\x20";

}  // namespace

namespace fxcrt {

static_assert(sizeof(ByteString) <= sizeof(char*),
              "Strings must not require more space than pointers");

// static
ByteString ByteString::FormatInteger(int i) {
  char buf[32];
  FXSYS_snprintf(buf, sizeof(buf), "%d", i);
  return ByteString(buf);
}

// static
// TODO(tsepez): Should be UNSAFE_BUFFER_USAGE.
ByteString ByteString::FormatV(const char* pFormat, va_list argList) {
  va_list argListCopy;
  va_copy(argListCopy, argList);

  // SAFETY: required from caller.
  int nMaxLen = UNSAFE_BUFFERS(vsnprintf(nullptr, 0, pFormat, argListCopy));
  va_end(argListCopy);

  if (nMaxLen <= 0) {
    return ByteString();
  }

  ByteString ret;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<char> buf = ret.GetBuffer(nMaxLen);

    // SAFETY: In the following two calls, there's always space in the buffer
    // for a terminating NUL that's not included in nMaxLen, and hence not
    // included in the span.
    UNSAFE_BUFFERS(FXSYS_memset(buf.data(), 0, nMaxLen + 1));
    va_copy(argListCopy, argList);
    UNSAFE_TODO(vsnprintf(buf.data(), nMaxLen + 1, pFormat, argListCopy));
    va_end(argListCopy);
  }
  ret.ReleaseBuffer(ret.GetStringLength());
  return ret;
}

// static
ByteString ByteString::Format(const char* pFormat, ...) {
  va_list argList;
  va_start(argList, pFormat);
  ByteString ret = FormatV(pFormat, argList);
  va_end(argList);

  return ret;
}

ByteString::ByteString(const char* pStr, size_t nLen) {
  if (nLen) {
    // SAFETY: caller ensures `pStr` points to at least `nLen` chars.
    data_ = StringData::Create(UNSAFE_BUFFERS(pdfium::span(pStr, nLen)));
  }
}

ByteString::ByteString(const uint8_t* pStr, size_t nLen)
    // SAFETY: caller ensures `pStr` points to at least `nLen` chars.
    : UNSAFE_BUFFERS(ByteString(reinterpret_cast<const char*>(pStr), nLen)) {}

ByteString::ByteString(char ch) {
  data_ = StringData::Create(1);
  data_->string_[0] = ch;
}

ByteString::ByteString(const char* ptr)
    // SAFETY: caller ensures `ptr` is NUL-terminated.
    : UNSAFE_BUFFERS(ByteString(ptr, ptr ? strlen(ptr) : 0)) {}

ByteString::ByteString(ByteStringView bstrc) {
  if (!bstrc.IsEmpty()) {
    data_ = StringData::Create(bstrc.span());
  }
}

ByteString::ByteString(ByteStringView str1, ByteStringView str2) {
  FX_SAFE_SIZE_T nSafeLen = str1.GetLength();
  nSafeLen += str2.GetLength();

  size_t nNewLen = nSafeLen.ValueOrDie();
  if (nNewLen == 0) {
    return;
  }

  data_ = StringData::Create(nNewLen);
  data_->CopyContents(str1.span());
  data_->CopyContentsAt(str1.GetLength(), str2.span());
}

ByteString::ByteString(const std::initializer_list<ByteStringView>& list) {
  FX_SAFE_SIZE_T nSafeLen = 0;
  for (const auto& item : list) {
    nSafeLen += item.GetLength();
  }

  size_t nNewLen = nSafeLen.ValueOrDie();
  if (nNewLen == 0) {
    return;
  }

  data_ = StringData::Create(nNewLen);

  size_t nOffset = 0;
  for (const auto& item : list) {
    data_->CopyContentsAt(nOffset, item.span());
    nOffset += item.GetLength();
  }
}

ByteString::ByteString(const fxcrt::ostringstream& outStream) {
  auto str = outStream.str();
  if (!str.empty()) {
    data_ = StringData::Create(pdfium::span(str));
  }
}

ByteString& ByteString::operator=(const char* str) {
  if (!str || !str[0]) {
    clear();
  } else {
    // SAFETY: required from caller.
    AssignCopy(str, UNSAFE_BUFFERS(strlen(str)));
  }
  return *this;
}

ByteString& ByteString::operator=(ByteStringView str) {
  if (str.IsEmpty()) {
    clear();
  } else {
    AssignCopy(str.unterminated_c_str(), str.GetLength());
  }

  return *this;
}

ByteString& ByteString::operator=(const ByteString& that) {
  if (data_ != that.data_) {
    data_ = that.data_;
  }

  return *this;
}

ByteString& ByteString::operator=(ByteString&& that) noexcept {
  if (data_ != that.data_) {
    data_ = std::move(that.data_);
  }

  return *this;
}

// TODO(tsepez): Should be UNSAFE_BUFFER_USAGE
ByteString& ByteString::operator+=(const char* str) {
  if (str) {
    // SAFETY: required from caller.
    Concat(str, UNSAFE_BUFFERS(strlen(str)));
  }
  return *this;
}

ByteString& ByteString::operator+=(char ch) {
  Concat(&ch, 1);
  return *this;
}

ByteString& ByteString::operator+=(const ByteString& str) {
  if (str.data_) {
    Concat(str.data_->string_, str.data_->data_length_);
  }

  return *this;
}

ByteString& ByteString::operator+=(ByteStringView str) {
  if (!str.IsEmpty()) {
    Concat(str.unterminated_c_str(), str.GetLength());
  }

  return *this;
}

bool operator==(const ByteString& lhs, const char* rhs) {
  if (lhs.IsEmpty()) {
    return !rhs || !rhs[0];
  }
  if (!rhs) {
    return false;
  }

  // SAFETY: required from caller.
  return UNSAFE_BUFFERS(strcmp(lhs.data_->string_, rhs)) == 0;
}

// TODO(tsepez): Should be UNSAFE_BUFFER_USAGE.
bool ByteString::operator<(const char* ptr) const {
  if (!data_ && !ptr) {
    return false;
  }
  if (c_str() == ptr) {
    return false;
  }

  // SAFETY: required from caller.
  size_t other_len = ptr ? UNSAFE_BUFFERS(strlen(ptr)) : 0;
  size_t len = GetLength();

  // SAFETY: Comparison limited to minimum valid length of either argument.
  int result =
      UNSAFE_BUFFERS(FXSYS_memcmp(c_str(), ptr, std::min(len, other_len)));
  return result < 0 || (result == 0 && len < other_len);
}

bool ByteString::operator<(ByteStringView str) const {
  return Compare(str) < 0;
}

bool ByteString::operator<(const ByteString& other) const {
  if (data_ == other.data_) {
    return false;
  }

  size_t len = GetLength();
  size_t other_len = other.GetLength();

  // SAFETY: Comparison limited to minimum valid length of either argument.
  int result = UNSAFE_BUFFERS(
      FXSYS_memcmp(c_str(), other.c_str(), std::min(len, other_len)));
  return result < 0 || (result == 0 && len < other_len);
}

bool ByteString::EqualNoCase(ByteStringView str) const {
  if (!data_) {
    return str.IsEmpty();
  }
  if (data_->data_length_ != str.GetLength()) {
    return false;
  }
  pdfium::span<const uint8_t> this_span = pdfium::as_bytes(data_->span());
  pdfium::span<const uint8_t> that_span = str.unsigned_span();
  while (!this_span.empty()) {
    uint8_t this_char = this_span.front();
    uint8_t that_char = that_span.front();
    if (this_char != that_char && tolower(this_char) != tolower(that_char)) {
      return false;
    }
    this_span = this_span.subspan<1u>();
    that_span = that_span.subspan<1u>();
  }
  return true;
}

intptr_t ByteString::ReferenceCountForTesting() const {
  return data_ ? data_->refs_ : 0;
}

ByteString ByteString::Substr(size_t offset) const {
  // Unsigned underflow is well-defined and out-of-range is handled by Substr().
  return Substr(offset, GetLength() - offset);
}

ByteString ByteString::Substr(size_t first, size_t count) const {
  if (!data_) {
    return ByteString();
  }
  if (first == 0 && count == data_->data_length_) {
    return *this;
  }
  return ByteString(AsStringView().Substr(first, count));
}

ByteString ByteString::First(size_t count) const {
  return Substr(0, count);
}

ByteString ByteString::Last(size_t count) const {
  // Unsigned underflow is well-defined and out-of-range is handled by Substr().
  return Substr(GetLength() - count, count);
}

void ByteString::MakeLower() {
  if (IsEmpty()) {
    return;
  }

  ReallocBeforeWrite(data_->data_length_);
  FXSYS_strlwr(data_->string_);
}

void ByteString::MakeUpper() {
  if (IsEmpty()) {
    return;
  }

  ReallocBeforeWrite(data_->data_length_);
  FXSYS_strupr(data_->string_);
}

int ByteString::Compare(ByteStringView str) const {
  if (!data_) {
    return str.IsEmpty() ? 0 : -1;
  }

  size_t this_len = data_->data_length_;
  size_t that_len = str.GetLength();
  size_t min_len = std::min(this_len, that_len);

  // SAFETY: Comparison limited to minimum valid length of either argument.
  int result = UNSAFE_BUFFERS(
      FXSYS_memcmp(data_->string_, str.unterminated_c_str(), min_len));
  if (result != 0) {
    return result;
  }
  if (this_len == that_len) {
    return 0;
  }
  return this_len < that_len ? -1 : 1;
}

void ByteString::TrimWhitespace() {
  TrimWhitespaceBack();
  TrimWhitespaceFront();
}

void ByteString::TrimWhitespaceFront() {
  TrimFront(kTrimChars);
}

void ByteString::TrimWhitespaceBack() {
  TrimBack(kTrimChars);
}

std::ostream& operator<<(std::ostream& os, const ByteString& str) {
  return os.write(str.c_str(), str.GetLength());
}

std::ostream& operator<<(std::ostream& os, ByteStringView str) {
  return os.write(str.unterminated_c_str(), str.GetLength());
}

}  // namespace fxcrt

uint32_t FX_HashCode_GetA(ByteStringView str) {
  uint32_t dwHashCode = 0;
  for (ByteStringView::UnsignedType c : str) {
    dwHashCode = 31 * dwHashCode + c;
  }
  return dwHashCode;
}

uint32_t FX_HashCode_GetLoweredA(ByteStringView str) {
  uint32_t dwHashCode = 0;
  for (ByteStringView::UnsignedType c : str) {
    dwHashCode = 31 * dwHashCode + tolower(c);
  }
  return dwHashCode;
}

uint32_t FX_HashCode_GetAsIfW(ByteStringView str) {
  uint32_t dwHashCode = 0;
  for (ByteStringView::UnsignedType c : str) {
    dwHashCode = 1313 * dwHashCode + c;
  }
  return dwHashCode;
}

uint32_t FX_HashCode_GetLoweredAsIfW(ByteStringView str) {
  uint32_t dwHashCode = 0;
  for (ByteStringView::UnsignedType c : str) {
    dwHashCode = 1313 * dwHashCode + FXSYS_towlower(c);
  }
  return dwHashCode;
}
