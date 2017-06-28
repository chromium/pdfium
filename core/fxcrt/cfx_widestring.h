// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WIDESTRING_H_
#define CORE_FXCRT_CFX_WIDESTRING_H_

#include <functional>
#include <utility>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_c_template.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"

class CFX_ByteString;

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_WideString {
 public:
  using CharType = wchar_t;
  using const_iterator = const CharType*;

  CFX_WideString();
  CFX_WideString(const CFX_WideString& other);
  CFX_WideString(CFX_WideString&& other) noexcept;

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(wchar_t ch);
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(const wchar_t* ptr);

  // No implicit conversions from byte strings.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(char) = delete;

  CFX_WideString(const wchar_t* ptr, FX_STRSIZE len);

  explicit CFX_WideString(const CFX_WideStringC& str);
  CFX_WideString(const CFX_WideStringC& str1, const CFX_WideStringC& str2);
  CFX_WideString(const std::initializer_list<CFX_WideStringC>& list);

  ~CFX_WideString();

  static CFX_WideString FromLocal(const CFX_ByteStringC& str);
  static CFX_WideString FromCodePage(const CFX_ByteStringC& str,
                                     uint16_t codepage);

  static CFX_WideString FromUTF8(const CFX_ByteStringC& str);
  static CFX_WideString FromUTF16LE(const unsigned short* str, FX_STRSIZE len);

  static FX_STRSIZE WStringLength(const unsigned short* str);

  // Explicit conversion to C-style wide string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const wchar_t* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Explicit conversion to CFX_WideStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(c_str(), GetLength());
  }

  // Note: Any subsequent modification of |this| will invalidate iterators.
  const_iterator begin() const { return m_pData ? m_pData->m_String : nullptr; }
  const_iterator end() const {
    return m_pData ? m_pData->m_String + m_pData->m_nDataLength : nullptr;
  }

  void clear() { m_pData.Reset(); }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  const CFX_WideString& operator=(const wchar_t* str);
  const CFX_WideString& operator=(const CFX_WideString& stringSrc);
  const CFX_WideString& operator=(const CFX_WideStringC& stringSrc);

  const CFX_WideString& operator+=(const wchar_t* str);
  const CFX_WideString& operator+=(wchar_t ch);
  const CFX_WideString& operator+=(const CFX_WideString& str);
  const CFX_WideString& operator+=(const CFX_WideStringC& str);

  bool operator==(const wchar_t* ptr) const;
  bool operator==(const CFX_WideStringC& str) const;
  bool operator==(const CFX_WideString& other) const;

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_WideString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_WideString& str) const;

  wchar_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  wchar_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, wchar_t ch);

  int Compare(const wchar_t* str) const;
  int Compare(const CFX_WideString& str) const;
  int CompareNoCase(const wchar_t* str) const;

  CFX_WideString Mid(FX_STRSIZE first) const;
  CFX_WideString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_WideString Left(FX_STRSIZE count) const;
  CFX_WideString Right(FX_STRSIZE count) const;

  FX_STRSIZE Insert(FX_STRSIZE index, wchar_t ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const wchar_t* lpszFormat, ...);
  void FormatV(const wchar_t* lpszFormat, va_list argList);

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(wchar_t chTarget);
  void TrimRight(const CFX_WideStringC& pTargets);

  void TrimLeft();
  void TrimLeft(wchar_t chTarget);
  void TrimLeft(const CFX_WideStringC& pTargets);

  void Reserve(FX_STRSIZE len);
  wchar_t* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  int GetInteger() const;
  float GetFloat() const;

  FX_STRSIZE Find(const CFX_WideStringC& pSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(wchar_t ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE Replace(const CFX_WideStringC& pOld, const CFX_WideStringC& pNew);
  FX_STRSIZE Remove(wchar_t ch);

  CFX_ByteString UTF8Encode() const;
  CFX_ByteString UTF16LE_Encode() const;

 protected:
  using StringData = CFX_StringDataTemplate<wchar_t>;

  void ReallocBeforeWrite(FX_STRSIZE nLen);
  void AllocBeforeWrite(FX_STRSIZE nLen);
  void AllocCopy(CFX_WideString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const wchar_t* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const wchar_t* lpszSrcData, FX_STRSIZE nSrcLen);

  // Returns true unless we ran out of space.
  bool TryVSWPrintf(FX_STRSIZE size, const wchar_t* format, va_list argList);

  CFX_RetainPtr<StringData> m_pData;

  friend class fxcrt_WideStringConcatInPlace_Test;
  friend class fxcrt_WideStringPool_Test;
};

inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1, wchar_t ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideStringC& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1, wchar_t ch) {
  return CFX_WideString(str1.AsStringC(), CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideString& str2) {
  return CFX_WideString(ch, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline bool operator==(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}

uint32_t FX_HashCode_GetW(const CFX_WideStringC& str, bool bIgnoreCase);

std::wostream& operator<<(std::wostream& os, const CFX_WideString& str);
std::ostream& operator<<(std::ostream& os, const CFX_WideString& str);
std::wostream& operator<<(std::wostream& os, const CFX_WideStringC& str);
std::ostream& operator<<(std::ostream& os, const CFX_WideStringC& str);

namespace std {

template <>
struct hash<CFX_WideString> {
  std::size_t operator()(const CFX_WideString& str) const {
    return FX_HashCode_GetW(str.AsStringC(), false);
  }
};

}  // namespace std

extern template struct std::hash<CFX_WideString>;

#endif  // CORE_FXCRT_CFX_WIDESTRING_H_
