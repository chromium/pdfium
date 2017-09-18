// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_BYTESTRING_H_
#define CORE_FXCRT_BYTESTRING_H_

#include <functional>
#include <iterator>
#include <sstream>
#include <utility>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/string_view_template.h"
#include "third_party/base/optional.h"

class ByteString_Concat_Test;
class fxcrt_ByteStringPool_Test;

namespace fxcrt {

class WideString;

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class ByteString {
 public:
  using CharType = char;
  using const_iterator = const CharType*;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  ByteString();
  ByteString(const ByteString& other);
  ByteString(ByteString&& other) noexcept;

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  ByteString(char ch);
  // NOLINTNEXTLINE(runtime/explicit)
  ByteString(const char* ptr);

  // No implicit conversions from wide strings.
  // NOLINTNEXTLINE(runtime/explicit)
  ByteString(wchar_t) = delete;

  ByteString(const char* ptr, FX_STRSIZE len);
  ByteString(const uint8_t* ptr, FX_STRSIZE len);

  explicit ByteString(const ByteStringView& bstrc);
  ByteString(const ByteStringView& bstrc1, const ByteStringView& bstrc2);
  ByteString(const std::initializer_list<ByteStringView>& list);
  explicit ByteString(const std::ostringstream& outStream);

  ~ByteString();

  void clear() { m_pData.Reset(); }

  static ByteString FromUnicode(const WideString& str);

  // Explicit conversion to C-style string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const char* c_str() const { return m_pData ? m_pData->m_String : ""; }

  // Explicit conversion to uint8_t*.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const uint8_t* raw_str() const {
    return m_pData ? reinterpret_cast<const uint8_t*>(m_pData->m_String)
                   : nullptr;
  }

  // Explicit conversion to ByteStringView.
  // Note: Any subsequent modification of |this| will invalidate the result.
  ByteStringView AsStringView() const {
    return ByteStringView(raw_str(), GetLength());
  }

  // Note: Any subsequent modification of |this| will invalidate iterators.
  const_iterator begin() const { return m_pData ? m_pData->m_String : nullptr; }
  const_iterator end() const {
    return m_pData ? m_pData->m_String + m_pData->m_nDataLength : nullptr;
  }

  // Note: Any subsequent modification of |this| will invalidate iterators.
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  FX_STRSIZE GetStringLength() const {
    return m_pData ? FXSYS_strlen(m_pData->m_String) : 0;
  }
  bool IsEmpty() const { return !GetLength(); }
  bool IsValidIndex(FX_STRSIZE index) const { return index < GetLength(); }
  bool IsValidLength(FX_STRSIZE length) const { return length <= GetLength(); }

  int Compare(const ByteStringView& str) const;
  bool EqualNoCase(const ByteStringView& str) const;

  bool operator==(const char* ptr) const;
  bool operator==(const ByteStringView& str) const;
  bool operator==(const ByteString& other) const;

  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const ByteStringView& str) const { return !(*this == str); }
  bool operator!=(const ByteString& other) const { return !(*this == other); }

  bool operator<(const ByteString& str) const;

  const ByteString& operator=(const char* str);
  const ByteString& operator=(const ByteStringView& bstrc);
  const ByteString& operator=(const ByteString& stringSrc);

  const ByteString& operator+=(char ch);
  const ByteString& operator+=(const char* str);
  const ByteString& operator+=(const ByteString& str);
  const ByteString& operator+=(const ByteStringView& bstrc);

  CharType operator[](const FX_STRSIZE index) const {
    ASSERT(IsValidIndex(index));
    return m_pData ? m_pData->m_String[index] : 0;
  }

  CharType First() const { return GetLength() ? (*this)[0] : 0; }
  CharType Last() const { return GetLength() ? (*this)[GetLength() - 1] : 0; }

  void SetAt(FX_STRSIZE index, char c);

  FX_STRSIZE Insert(FX_STRSIZE index, char ch);
  FX_STRSIZE InsertAtFront(char ch) { return Insert(0, ch); }
  FX_STRSIZE InsertAtBack(char ch) { return Insert(GetLength(), ch); }
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const char* lpszFormat, ...);
  void FormatV(const char* lpszFormat, va_list argList);

  void Reserve(FX_STRSIZE len);
  char* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len);

  ByteString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  ByteString Left(FX_STRSIZE count) const;
  ByteString Right(FX_STRSIZE count) const;

  pdfium::Optional<FX_STRSIZE> Find(const ByteStringView& lpszSub,
                                    FX_STRSIZE start = 0) const;
  pdfium::Optional<FX_STRSIZE> Find(char ch, FX_STRSIZE start = 0) const;
  pdfium::Optional<FX_STRSIZE> ReverseFind(char ch) const;

  bool Contains(const ByteStringView& lpszSub, FX_STRSIZE start = 0) const {
    return Find(lpszSub, start).has_value();
  }

  bool Contains(char ch, FX_STRSIZE start = 0) const {
    return Find(ch, start).has_value();
  }

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(char chTarget);
  void TrimRight(const ByteStringView& lpszTargets);

  void TrimLeft();
  void TrimLeft(char chTarget);
  void TrimLeft(const ByteStringView& lpszTargets);

  FX_STRSIZE Replace(const ByteStringView& lpszOld,
                     const ByteStringView& lpszNew);

  FX_STRSIZE Remove(char ch);

  WideString UTF8Decode() const;

  uint32_t GetID() const { return AsStringView().GetID(); }

  static ByteString FormatInteger(int i);
  static ByteString FormatFloat(float f, int precision = 0);

 protected:
  using StringData = CFX_StringDataTemplate<char>;

  void ReallocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocCopy(ByteString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const char* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const char* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;

  friend ByteString_Concat_Test;
  friend fxcrt_ByteStringPool_Test;
};

inline bool operator==(const char* lhs, const ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const ByteStringView& lhs, const ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const ByteString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const ByteStringView& lhs, const ByteString& rhs) {
  return rhs != lhs;
}

inline ByteString operator+(const ByteStringView& str1,
                            const ByteStringView& str2) {
  return ByteString(str1, str2);
}
inline ByteString operator+(const ByteStringView& str1, const char* str2) {
  return ByteString(str1, str2);
}
inline ByteString operator+(const char* str1, const ByteStringView& str2) {
  return ByteString(str1, str2);
}
inline ByteString operator+(const ByteStringView& str1, char ch) {
  return ByteString(str1, ByteStringView(ch));
}
inline ByteString operator+(char ch, const ByteStringView& str2) {
  return ByteString(ch, str2);
}
inline ByteString operator+(const ByteString& str1, const ByteString& str2) {
  return ByteString(str1.AsStringView(), str2.AsStringView());
}
inline ByteString operator+(const ByteString& str1, char ch) {
  return ByteString(str1.AsStringView(), ByteStringView(ch));
}
inline ByteString operator+(char ch, const ByteString& str2) {
  return ByteString(ch, str2.AsStringView());
}
inline ByteString operator+(const ByteString& str1, const char* str2) {
  return ByteString(str1.AsStringView(), str2);
}
inline ByteString operator+(const char* str1, const ByteString& str2) {
  return ByteString(str1, str2.AsStringView());
}
inline ByteString operator+(const ByteString& str1,
                            const ByteStringView& str2) {
  return ByteString(str1.AsStringView(), str2);
}
inline ByteString operator+(const ByteStringView& str1,
                            const ByteString& str2) {
  return ByteString(str1, str2.AsStringView());
}

std::ostream& operator<<(std::ostream& os, const ByteString& str);
std::ostream& operator<<(std::ostream& os, const ByteStringView& str);

}  // namespace fxcrt

using ByteString = fxcrt::ByteString;

uint32_t FX_HashCode_GetA(const ByteStringView& str, bool bIgnoreCase);

namespace std {

template <>
struct hash<ByteString> {
  std::size_t operator()(const ByteString& str) const {
    return FX_HashCode_GetA(str.AsStringView(), false);
  }
};

}  // namespace std

extern template struct std::hash<ByteString>;

#endif  // CORE_FXCRT_BYTESTRING_H_
