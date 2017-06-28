// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_BYTESTRING_H_
#define CORE_FXCRT_CFX_BYTESTRING_H_

#include <functional>
#include <sstream>
#include <utility>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_c_template.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"

class CFX_WideString;

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_ByteString {
 public:
  using CharType = char;
  using const_iterator = const CharType*;

  CFX_ByteString();
  CFX_ByteString(const CFX_ByteString& other);
  CFX_ByteString(CFX_ByteString&& other) noexcept;

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_ByteString(char ch);
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_ByteString(const char* ptr);

  // No implicit conversions from wide strings.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_ByteString(wchar_t) = delete;

  CFX_ByteString(const char* ptr, FX_STRSIZE len);
  CFX_ByteString(const uint8_t* ptr, FX_STRSIZE len);

  explicit CFX_ByteString(const CFX_ByteStringC& bstrc);
  CFX_ByteString(const CFX_ByteStringC& bstrc1, const CFX_ByteStringC& bstrc2);
  CFX_ByteString(const std::initializer_list<CFX_ByteStringC>& list);
  explicit CFX_ByteString(std::ostringstream& outStream);

  ~CFX_ByteString();

  void clear() { m_pData.Reset(); }

  static CFX_ByteString FromUnicode(const wchar_t* ptr, FX_STRSIZE len = -1);
  static CFX_ByteString FromUnicode(const CFX_WideString& str);

  // Explicit conversion to C-style string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const char* c_str() const { return m_pData ? m_pData->m_String : ""; }

  // Explicit conversion to uint8_t*.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const uint8_t* raw_str() const {
    return m_pData ? reinterpret_cast<const uint8_t*>(m_pData->m_String)
                   : nullptr;
  }

  // Explicit conversion to CFX_ByteStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_ByteStringC AsStringC() const {
    return CFX_ByteStringC(raw_str(), GetLength());
  }

  // Note: Any subsequent modification of |this| will invalidate iterators.
  const_iterator begin() const { return m_pData ? m_pData->m_String : nullptr; }
  const_iterator end() const {
    return m_pData ? m_pData->m_String + m_pData->m_nDataLength : nullptr;
  }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  int Compare(const CFX_ByteStringC& str) const;
  bool EqualNoCase(const CFX_ByteStringC& str) const;

  bool operator==(const char* ptr) const;
  bool operator==(const CFX_ByteStringC& str) const;
  bool operator==(const CFX_ByteString& other) const;

  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_ByteString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_ByteString& str) const;

  const CFX_ByteString& operator=(const char* str);
  const CFX_ByteString& operator=(const CFX_ByteStringC& bstrc);
  const CFX_ByteString& operator=(const CFX_ByteString& stringSrc);

  const CFX_ByteString& operator+=(char ch);
  const CFX_ByteString& operator+=(const char* str);
  const CFX_ByteString& operator+=(const CFX_ByteString& str);
  const CFX_ByteString& operator+=(const CFX_ByteStringC& bstrc);

  uint8_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  uint8_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, char ch);
  FX_STRSIZE Insert(FX_STRSIZE index, char ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const char* lpszFormat, ...);
  void FormatV(const char* lpszFormat, va_list argList);

  void Reserve(FX_STRSIZE len);
  char* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  CFX_ByteString Mid(FX_STRSIZE first) const;
  CFX_ByteString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_ByteString Left(FX_STRSIZE count) const;
  CFX_ByteString Right(FX_STRSIZE count) const;

  FX_STRSIZE Find(const CFX_ByteStringC& lpszSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(char ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE ReverseFind(char ch) const;

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(char chTarget);
  void TrimRight(const CFX_ByteStringC& lpszTargets);

  void TrimLeft();
  void TrimLeft(char chTarget);
  void TrimLeft(const CFX_ByteStringC& lpszTargets);

  FX_STRSIZE Replace(const CFX_ByteStringC& lpszOld,
                     const CFX_ByteStringC& lpszNew);

  FX_STRSIZE Remove(char ch);

  CFX_WideString UTF8Decode() const;

  uint32_t GetID(FX_STRSIZE start_pos = 0) const;

#define FXFORMAT_SIGNED 1
#define FXFORMAT_HEX 2
#define FXFORMAT_CAPITAL 4

  static CFX_ByteString FormatInteger(int i, uint32_t flags = 0);
  static CFX_ByteString FormatFloat(float f, int precision = 0);

 protected:
  using StringData = CFX_StringDataTemplate<char>;

  void ReallocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocCopy(CFX_ByteString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const char* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const char* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;

  friend class fxcrt_ByteStringConcat_Test;
  friend class fxcrt_ByteStringPool_Test;
};

inline bool operator==(const char* lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}

inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, const char* str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const char* str1, const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, char ch) {
  return CFX_ByteString(str1, CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(char ch, const CFX_ByteStringC& str2) {
  return CFX_ByteString(ch, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, char ch) {
  return CFX_ByteString(str1.AsStringC(), CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(char ch, const CFX_ByteString& str2) {
  return CFX_ByteString(ch, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, const char* str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const char* str1, const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}

uint32_t FX_HashCode_GetA(const CFX_ByteStringC& str, bool bIgnoreCase);

std::ostream& operator<<(std::ostream& os, const CFX_ByteString& str);
std::ostream& operator<<(std::ostream& os, const CFX_ByteStringC& str);

namespace std {

template <>
struct hash<CFX_ByteString> {
  std::size_t operator()(const CFX_ByteString& str) const {
    return FX_HashCode_GetA(str.AsStringC(), false);
  }
};

}  // namespace std

extern template struct std::hash<CFX_ByteString>;

#endif  // CORE_FXCRT_CFX_BYTESTRING_H_
