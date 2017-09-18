// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_WIDESTRING_H_
#define CORE_FXCRT_WIDESTRING_H_

#include <functional>
#include <iterator>
#include <utility>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/string_view_template.h"
#include "third_party/base/optional.h"

class WideString_ConcatInPlace_Test;
class fxcrt_WideStringPool_Test;

namespace fxcrt {

class ByteString;

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class WideString {
 public:
  using CharType = wchar_t;
  using const_iterator = const CharType*;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  WideString();
  WideString(const WideString& other);
  WideString(WideString&& other) noexcept;

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  WideString(wchar_t ch);
  // NOLINTNEXTLINE(runtime/explicit)
  WideString(const wchar_t* ptr);

  // No implicit conversions from byte strings.
  // NOLINTNEXTLINE(runtime/explicit)
  WideString(char) = delete;

  WideString(const wchar_t* ptr, FX_STRSIZE len);

  explicit WideString(const WideStringView& str);
  WideString(const WideStringView& str1, const WideStringView& str2);
  WideString(const std::initializer_list<WideStringView>& list);

  ~WideString();

  static WideString FromLocal(const ByteStringView& str);
  static WideString FromCodePage(const ByteStringView& str, uint16_t codepage);

  static WideString FromUTF8(const ByteStringView& str);
  static WideString FromUTF16LE(const unsigned short* str, FX_STRSIZE len);

  static FX_STRSIZE WStringLength(const unsigned short* str);

  // Explicit conversion to C-style wide string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const wchar_t* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Explicit conversion to WideStringView.
  // Note: Any subsequent modification of |this| will invalidate the result.
  WideStringView AsStringView() const {
    return WideStringView(c_str(), GetLength());
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

  void clear() { m_pData.Reset(); }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  FX_STRSIZE GetStringLength() const {
    return m_pData ? FXSYS_wcslen(m_pData->m_String) : 0;
  }
  bool IsEmpty() const { return !GetLength(); }
  bool IsValidIndex(FX_STRSIZE index) const { return index < GetLength(); }
  bool IsValidLength(FX_STRSIZE length) const { return length <= GetLength(); }

  const WideString& operator=(const wchar_t* str);
  const WideString& operator=(const WideString& stringSrc);
  const WideString& operator=(const WideStringView& stringSrc);

  const WideString& operator+=(const wchar_t* str);
  const WideString& operator+=(wchar_t ch);
  const WideString& operator+=(const WideString& str);
  const WideString& operator+=(const WideStringView& str);

  bool operator==(const wchar_t* ptr) const;
  bool operator==(const WideStringView& str) const;
  bool operator==(const WideString& other) const;

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const WideStringView& str) const { return !(*this == str); }
  bool operator!=(const WideString& other) const { return !(*this == other); }

  bool operator<(const WideString& str) const;

  CharType operator[](const FX_STRSIZE index) const {
    ASSERT(IsValidIndex(index));
    return m_pData ? m_pData->m_String[index] : 0;
  }

  CharType First() const { return GetLength() ? (*this)[0] : 0; }
  CharType Last() const { return GetLength() ? (*this)[GetLength() - 1] : 0; }

  void SetAt(FX_STRSIZE index, wchar_t c);

  int Compare(const wchar_t* str) const;
  int Compare(const WideString& str) const;
  int CompareNoCase(const wchar_t* str) const;

  WideString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  WideString Left(FX_STRSIZE count) const;
  WideString Right(FX_STRSIZE count) const;

  FX_STRSIZE Insert(FX_STRSIZE index, wchar_t ch);
  FX_STRSIZE InsertAtFront(wchar_t ch) { return Insert(0, ch); }
  FX_STRSIZE InsertAtBack(wchar_t ch) { return Insert(GetLength(), ch); }
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const wchar_t* lpszFormat, ...);
  void FormatV(const wchar_t* lpszFormat, va_list argList);

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(wchar_t chTarget);
  void TrimRight(const WideStringView& pTargets);

  void TrimLeft();
  void TrimLeft(wchar_t chTarget);
  void TrimLeft(const WideStringView& pTargets);

  void Reserve(FX_STRSIZE len);
  wchar_t* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len);

  int GetInteger() const;
  float GetFloat() const;

  pdfium::Optional<FX_STRSIZE> Find(const WideStringView& pSub,
                                    FX_STRSIZE start = 0) const;
  pdfium::Optional<FX_STRSIZE> Find(wchar_t ch, FX_STRSIZE start = 0) const;

  bool Contains(const WideStringView& lpszSub, FX_STRSIZE start = 0) const {
    return Find(lpszSub, start).has_value();
  }

  bool Contains(char ch, FX_STRSIZE start = 0) const {
    return Find(ch, start).has_value();
  }

  FX_STRSIZE Replace(const WideStringView& pOld, const WideStringView& pNew);
  FX_STRSIZE Remove(wchar_t ch);

  ByteString UTF8Encode() const;
  ByteString UTF16LE_Encode() const;

 protected:
  using StringData = CFX_StringDataTemplate<wchar_t>;

  void ReallocBeforeWrite(FX_STRSIZE nLen);
  void AllocBeforeWrite(FX_STRSIZE nLen);
  void AllocCopy(WideString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const wchar_t* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const wchar_t* lpszSrcData, FX_STRSIZE nSrcLen);

  // Returns true unless we ran out of space.
  bool TryVSWPrintf(FX_STRSIZE size, const wchar_t* format, va_list argList);

  CFX_RetainPtr<StringData> m_pData;

  friend WideString_ConcatInPlace_Test;
  friend fxcrt_WideStringPool_Test;
};

inline WideString operator+(const WideStringView& str1,
                            const WideStringView& str2) {
  return WideString(str1, str2);
}
inline WideString operator+(const WideStringView& str1, const wchar_t* str2) {
  return WideString(str1, str2);
}
inline WideString operator+(const wchar_t* str1, const WideStringView& str2) {
  return WideString(str1, str2);
}
inline WideString operator+(const WideStringView& str1, wchar_t ch) {
  return WideString(str1, WideStringView(ch));
}
inline WideString operator+(wchar_t ch, const WideStringView& str2) {
  return WideString(ch, str2);
}
inline WideString operator+(const WideString& str1, const WideString& str2) {
  return WideString(str1.AsStringView(), str2.AsStringView());
}
inline WideString operator+(const WideString& str1, wchar_t ch) {
  return WideString(str1.AsStringView(), WideStringView(ch));
}
inline WideString operator+(wchar_t ch, const WideString& str2) {
  return WideString(ch, str2.AsStringView());
}
inline WideString operator+(const WideString& str1, const wchar_t* str2) {
  return WideString(str1.AsStringView(), str2);
}
inline WideString operator+(const wchar_t* str1, const WideString& str2) {
  return WideString(str1, str2.AsStringView());
}
inline WideString operator+(const WideString& str1,
                            const WideStringView& str2) {
  return WideString(str1.AsStringView(), str2);
}
inline WideString operator+(const WideStringView& str1,
                            const WideString& str2) {
  return WideString(str1, str2.AsStringView());
}
inline bool operator==(const wchar_t* lhs, const WideString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const WideStringView& lhs, const WideString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const WideString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const WideStringView& lhs, const WideString& rhs) {
  return rhs != lhs;
}

std::wostream& operator<<(std::wostream& os, const WideString& str);
std::ostream& operator<<(std::ostream& os, const WideString& str);
std::wostream& operator<<(std::wostream& os, const WideStringView& str);
std::ostream& operator<<(std::ostream& os, const WideStringView& str);

}  // namespace fxcrt

using WideString = fxcrt::WideString;

uint32_t FX_HashCode_GetW(const WideStringView& str, bool bIgnoreCase);

namespace std {

template <>
struct hash<WideString> {
  std::size_t operator()(const WideString& str) const {
    return FX_HashCode_GetW(str.AsStringView(), false);
  }
};

}  // namespace std

extern template struct std::hash<WideString>;

#endif  // CORE_FXCRT_WIDESTRING_H_
