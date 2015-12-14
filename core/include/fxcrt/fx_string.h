// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_STRING_H_
#define CORE_INCLUDE_FXCRT_FX_STRING_H_

#include <stdint.h>  // For intptr_t.
#include <algorithm>

#include "fx_memory.h"
#include "fx_system.h"

class CFX_BinaryBuf;
class CFX_ByteString;
class CFX_WideString;
struct CFX_CharMap;

// An immutable string with caller-provided storage which must outlive the
// string itself.
class CFX_ByteStringC {
 public:
  typedef FX_CHAR value_type;

  CFX_ByteStringC() {
    m_Ptr = NULL;
    m_Length = 0;
  }

  CFX_ByteStringC(const uint8_t* ptr, FX_STRSIZE size) {
    m_Ptr = ptr;
    m_Length = size;
  }

  CFX_ByteStringC(const FX_CHAR* ptr) {
    m_Ptr = (const uint8_t*)ptr;
    m_Length = ptr ? FXSYS_strlen(ptr) : 0;
  }

  // |ch| must be an lvalue that outlives the the CFX_ByteStringC. However,
  // the use of char rvalues are not caught at compile time.  They are
  // implicitly promoted to CFX_ByteString (see below) and then the
  // CFX_ByteStringC is constructed from the CFX_ByteString via the alternate
  // constructor below. The CFX_ByteString then typically goes out of scope
  // and |m_Ptr| may be left pointing to invalid memory. Beware.
  // TODO(tsepez): Mark single-argument string constructors as explicit.
  CFX_ByteStringC(FX_CHAR& ch) {
    m_Ptr = (const uint8_t*)&ch;
    m_Length = 1;
  }

  CFX_ByteStringC(const FX_CHAR* ptr, FX_STRSIZE len) {
    m_Ptr = (const uint8_t*)ptr;
    m_Length = (len == -1) ? FXSYS_strlen(ptr) : len;
  }

  CFX_ByteStringC(const CFX_ByteStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  CFX_ByteStringC(const CFX_ByteString& src);

  CFX_ByteStringC& operator=(const FX_CHAR* src) {
    m_Ptr = (const uint8_t*)src;
    m_Length = m_Ptr ? FXSYS_strlen(src) : 0;
    return *this;
  }

  CFX_ByteStringC& operator=(const CFX_ByteStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
    return *this;
  }

  CFX_ByteStringC& operator=(const CFX_ByteString& src);

  bool operator==(const char* ptr) const {
    return FXSYS_strlen(ptr) == m_Length &&
           FXSYS_memcmp(ptr, m_Ptr, m_Length) == 0;
  }
  bool operator==(const CFX_ByteStringC& other) const {
    return other.m_Length == m_Length &&
           FXSYS_memcmp(other.m_Ptr, m_Ptr, m_Length) == 0;
  }
  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& other) const {
    return !(*this == other);
  }

  FX_DWORD GetID(FX_STRSIZE start_pos = 0) const;

  const uint8_t* GetPtr() const { return m_Ptr; }

  const FX_CHAR* GetCStr() const { return (const FX_CHAR*)m_Ptr; }

  FX_STRSIZE GetLength() const { return m_Length; }

  bool IsEmpty() const { return m_Length == 0; }

  uint8_t GetAt(FX_STRSIZE index) const { return m_Ptr[index]; }

  CFX_ByteStringC Mid(FX_STRSIZE index, FX_STRSIZE count = -1) const {
    if (index < 0) {
      index = 0;
    }
    if (index > m_Length) {
      return CFX_ByteStringC();
    }
    if (count < 0 || count > m_Length - index) {
      count = m_Length - index;
    }
    return CFX_ByteStringC(m_Ptr + index, count);
  }

  const uint8_t& operator[](size_t index) const { return m_Ptr[index]; }

  bool operator<(const CFX_ByteStringC& that) const {
    int result = memcmp(m_Ptr, that.m_Ptr, std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

 protected:
  const uint8_t* m_Ptr;
  FX_STRSIZE m_Length;

 private:
  void* operator new(size_t) throw() { return NULL; }
};
inline bool operator==(const char* lhs, const CFX_ByteStringC& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const CFX_ByteStringC& rhs) {
  return rhs != lhs;
}
#define FXBSTR_ID(c1, c2, c3, c4) ((c1 << 24) | (c2 << 16) | (c3 << 8) | (c4))

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_ByteString {
 public:
  typedef FX_CHAR value_type;

  CFX_ByteString() : m_pData(nullptr) {}

  // Copy constructor.
  CFX_ByteString(const CFX_ByteString& str);

  // Move constructor.
  inline CFX_ByteString(CFX_ByteString&& other) {
    m_pData = other.m_pData;
    other.m_pData = nullptr;
  }

  CFX_ByteString(char ch);
  CFX_ByteString(const FX_CHAR* ptr)
      : CFX_ByteString(ptr, ptr ? FXSYS_strlen(ptr) : 0) {}

  CFX_ByteString(const FX_CHAR* ptr, FX_STRSIZE len);
  CFX_ByteString(const uint8_t* ptr, FX_STRSIZE len);

  CFX_ByteString(const CFX_ByteStringC& bstrc);
  CFX_ByteString(const CFX_ByteStringC& bstrc1, const CFX_ByteStringC& bstrc2);

  ~CFX_ByteString();

  static CFX_ByteString FromUnicode(const FX_WCHAR* ptr, FX_STRSIZE len = -1);

  static CFX_ByteString FromUnicode(const CFX_WideString& str);

  // Explicit conversion to raw string
  const FX_CHAR* c_str() const { return m_pData ? m_pData->m_String : ""; }

  // Implicit conversion to C-style string -- deprecated
  operator const FX_CHAR*() const { return m_pData ? m_pData->m_String : ""; }

  operator const uint8_t*() const {
    return m_pData ? (const uint8_t*)m_pData->m_String : NULL;
  }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }

  bool IsEmpty() const { return !GetLength(); }

  int Compare(const CFX_ByteStringC& str) const;

  bool Equal(const char* ptr) const;
  bool Equal(const CFX_ByteStringC& str) const;
  bool Equal(const CFX_ByteString& other) const;

  bool EqualNoCase(const CFX_ByteStringC& str) const;

  bool operator==(const char* ptr) const { return Equal(ptr); }
  bool operator==(const CFX_ByteStringC& str) const { return Equal(str); }
  bool operator==(const CFX_ByteString& other) const { return Equal(other); }

  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_ByteString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_ByteString& str) const {
    int result = FXSYS_memcmp(c_str(), str.c_str(),
                              std::min(GetLength(), str.GetLength()));
    return result < 0 || (result == 0 && GetLength() < str.GetLength());
  }

  void Empty();

  const CFX_ByteString& operator=(const FX_CHAR* str);

  const CFX_ByteString& operator=(const CFX_ByteStringC& bstrc);

  const CFX_ByteString& operator=(const CFX_ByteString& stringSrc);

  const CFX_ByteString& operator=(const CFX_BinaryBuf& buf);

  void Load(const uint8_t* str, FX_STRSIZE len);

  const CFX_ByteString& operator+=(FX_CHAR ch);

  const CFX_ByteString& operator+=(const FX_CHAR* str);

  const CFX_ByteString& operator+=(const CFX_ByteString& str);

  const CFX_ByteString& operator+=(const CFX_ByteStringC& bstrc);

  uint8_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  uint8_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, FX_CHAR ch);

  FX_STRSIZE Insert(FX_STRSIZE index, FX_CHAR ch);

  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const FX_CHAR* lpszFormat, ...);

  void FormatV(const FX_CHAR* lpszFormat, va_list argList);

  void Reserve(FX_STRSIZE len);

  FX_CHAR* GetBuffer(FX_STRSIZE len);

  void ReleaseBuffer(FX_STRSIZE len = -1);

  CFX_ByteString Mid(FX_STRSIZE first) const;

  CFX_ByteString Mid(FX_STRSIZE first, FX_STRSIZE count) const;

  CFX_ByteString Left(FX_STRSIZE count) const;

  CFX_ByteString Right(FX_STRSIZE count) const;

  FX_STRSIZE Find(const CFX_ByteStringC& lpszSub, FX_STRSIZE start = 0) const;

  FX_STRSIZE Find(FX_CHAR ch, FX_STRSIZE start = 0) const;

  FX_STRSIZE ReverseFind(FX_CHAR ch) const;

  void MakeLower();

  void MakeUpper();

  void TrimRight();

  void TrimRight(FX_CHAR chTarget);

  void TrimRight(const CFX_ByteStringC& lpszTargets);

  void TrimLeft();

  void TrimLeft(FX_CHAR chTarget);

  void TrimLeft(const CFX_ByteStringC& lpszTargets);

  FX_STRSIZE Replace(const CFX_ByteStringC& lpszOld,
                     const CFX_ByteStringC& lpszNew);

  FX_STRSIZE Remove(FX_CHAR ch);

  CFX_WideString UTF8Decode() const;

  void ConvertFrom(const CFX_WideString& str, CFX_CharMap* pCharMap = NULL);

  FX_DWORD GetID(FX_STRSIZE start_pos = 0) const;

#define FXFORMAT_SIGNED 1
#define FXFORMAT_HEX 2
#define FXFORMAT_CAPITAL 4

  static CFX_ByteString FormatInteger(int i, FX_DWORD flags = 0);
  static CFX_ByteString FormatFloat(FX_FLOAT f, int precision = 0);

 protected:
  // To ensure ref counts do not overflow, consider the worst possible case:
  // the entire address space contains nothing but pointers to this object.
  // Since the count increments with each new pointer, the largest value is
  // the number of pointers that can fit into the address space. The size of
  // the address space itself is a good upper bound on it; we need not go
  // larger.
  class StringData {
   public:
    static StringData* Create(int nLen);
    void Retain() { ++m_nRefs; }
    void Release() {
      if (--m_nRefs <= 0)
        FX_Free(this);
    }

    intptr_t m_nRefs;  // Would prefer ssize_t, but no windows support.
    FX_STRSIZE m_nDataLength;
    FX_STRSIZE m_nAllocLength;
    FX_CHAR m_String[1];

   private:
    StringData(FX_STRSIZE dataLen, FX_STRSIZE allocLen)
        : m_nRefs(1), m_nDataLength(dataLen), m_nAllocLength(allocLen) {
      FXSYS_assert(dataLen >= 0);
      FXSYS_assert(allocLen >= 0);
      FXSYS_assert(dataLen <= allocLen);
      m_String[dataLen] = 0;
    }
    ~StringData() = delete;
  };

  void AllocCopy(CFX_ByteString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(FX_STRSIZE nSrcLen, const FX_CHAR* lpszSrcData);
  void ConcatCopy(FX_STRSIZE nSrc1Len,
                  const FX_CHAR* lpszSrc1Data,
                  FX_STRSIZE nSrc2Len,
                  const FX_CHAR* lpszSrc2Data);
  void ConcatInPlace(FX_STRSIZE nSrcLen, const FX_CHAR* lpszSrcData);
  void CopyBeforeWrite();
  void AllocBeforeWrite(FX_STRSIZE nLen);

  StringData* m_pData;
  friend class fxcrt_ByteStringConcatInPlace_Test;
};
inline CFX_ByteStringC::CFX_ByteStringC(const CFX_ByteString& src) {
  m_Ptr = (const uint8_t*)src;
  m_Length = src.GetLength();
}
inline CFX_ByteStringC& CFX_ByteStringC::operator=(const CFX_ByteString& src) {
  m_Ptr = (const uint8_t*)src;
  m_Length = src.GetLength();
  return *this;
}

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
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const FX_CHAR* str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const FX_CHAR* str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, FX_CHAR ch) {
  return CFX_ByteString(str1, CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(FX_CHAR ch, const CFX_ByteStringC& str2) {
  return CFX_ByteString(ch, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, FX_CHAR ch) {
  return CFX_ByteString(str1, CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(FX_CHAR ch, const CFX_ByteString& str2) {
  return CFX_ByteString(ch, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const FX_CHAR* str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const FX_CHAR* str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2);
}
class CFX_WideStringC {
 public:
  typedef FX_WCHAR value_type;

  CFX_WideStringC() {
    m_Ptr = NULL;
    m_Length = 0;
  }

  CFX_WideStringC(const FX_WCHAR* ptr) {
    m_Ptr = ptr;
    m_Length = ptr ? FXSYS_wcslen(ptr) : 0;
  }

  CFX_WideStringC(FX_WCHAR& ch) {
    m_Ptr = &ch;
    m_Length = 1;
  }

  CFX_WideStringC(const FX_WCHAR* ptr, FX_STRSIZE len) {
    m_Ptr = ptr;
    m_Length = (len == -1) ? FXSYS_wcslen(ptr) : len;
  }

  CFX_WideStringC(const CFX_WideStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  CFX_WideStringC(const CFX_WideString& src);

  CFX_WideStringC& operator=(const FX_WCHAR* src) {
    m_Ptr = src;
    m_Length = FXSYS_wcslen(src);
    return *this;
  }

  CFX_WideStringC& operator=(const CFX_WideStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
    return *this;
  }

  CFX_WideStringC& operator=(const CFX_WideString& src);

  bool operator==(const wchar_t* ptr) const {
    return FXSYS_wcslen(ptr) == m_Length && wmemcmp(ptr, m_Ptr, m_Length) == 0;
  }
  bool operator==(const CFX_WideStringC& str) const {
    return str.m_Length == m_Length && wmemcmp(str.m_Ptr, m_Ptr, m_Length) == 0;
  }
  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }

  const FX_WCHAR* GetPtr() const { return m_Ptr; }

  FX_STRSIZE GetLength() const { return m_Length; }

  bool IsEmpty() const { return m_Length == 0; }

  FX_WCHAR GetAt(FX_STRSIZE index) const { return m_Ptr[index]; }

  CFX_WideStringC Left(FX_STRSIZE count) const {
    if (count < 1) {
      return CFX_WideStringC();
    }
    if (count > m_Length) {
      count = m_Length;
    }
    return CFX_WideStringC(m_Ptr, count);
  }

  CFX_WideStringC Mid(FX_STRSIZE index, FX_STRSIZE count = -1) const {
    if (index < 0) {
      index = 0;
    }
    if (index > m_Length) {
      return CFX_WideStringC();
    }
    if (count < 0 || count > m_Length - index) {
      count = m_Length - index;
    }
    return CFX_WideStringC(m_Ptr + index, count);
  }

  CFX_WideStringC Right(FX_STRSIZE count) const {
    if (count < 1) {
      return CFX_WideStringC();
    }
    if (count > m_Length) {
      count = m_Length;
    }
    return CFX_WideStringC(m_Ptr + m_Length - count, count);
  }

  const FX_WCHAR& operator[](size_t index) const { return m_Ptr[index]; }

  bool operator<(const CFX_WideStringC& that) const {
    int result = wmemcmp(m_Ptr, that.m_Ptr, std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

 protected:
  const FX_WCHAR* m_Ptr;
  FX_STRSIZE m_Length;

 private:
  void* operator new(size_t) throw() { return NULL; }
};
inline bool operator==(const wchar_t* lhs, const CFX_WideStringC& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideStringC& rhs) {
  return rhs != lhs;
}
#define FX_WSTRC(wstr) CFX_WideStringC(wstr, FX_ArraySize(wstr) - 1)

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_WideString {
 public:
  typedef FX_WCHAR value_type;

  CFX_WideString() : m_pData(nullptr) {}

  // Copy constructor.
  CFX_WideString(const CFX_WideString& str);

  // Move constructor.
  inline CFX_WideString(CFX_WideString&& other) {
    m_pData = other.m_pData;
    other.m_pData = nullptr;
  }

  CFX_WideString(const FX_WCHAR* ptr)
      : CFX_WideString(ptr, ptr ? FXSYS_wcslen(ptr) : 0) {}

  CFX_WideString(const FX_WCHAR* ptr, FX_STRSIZE len);

  CFX_WideString(FX_WCHAR ch);

  CFX_WideString(const CFX_WideStringC& str);

  CFX_WideString(const CFX_WideStringC& str1, const CFX_WideStringC& str2);

  ~CFX_WideString();

  static CFX_WideString FromLocal(const char* str, FX_STRSIZE len = -1);

  static CFX_WideString FromUTF8(const char* str, FX_STRSIZE len);

  static CFX_WideString FromUTF16LE(const unsigned short* str, FX_STRSIZE len);

  static FX_STRSIZE WStringLength(const unsigned short* str);

  // Explicit conversion to raw string
  const FX_WCHAR* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Implicit conversion to C-style wide string -- deprecated
  operator const FX_WCHAR*() const { return m_pData ? m_pData->m_String : L""; }

  void Empty();

  FX_BOOL IsEmpty() const { return !GetLength(); }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }

  const CFX_WideString& operator=(const FX_WCHAR* str);

  const CFX_WideString& operator=(const CFX_WideString& stringSrc);

  const CFX_WideString& operator=(const CFX_WideStringC& stringSrc);

  const CFX_WideString& operator+=(const FX_WCHAR* str);

  const CFX_WideString& operator+=(FX_WCHAR ch);

  const CFX_WideString& operator+=(const CFX_WideString& str);

  const CFX_WideString& operator+=(const CFX_WideStringC& str);

  bool operator==(const wchar_t* ptr) const { return Equal(ptr); }
  bool operator==(const CFX_WideStringC& str) const { return Equal(str); }
  bool operator==(const CFX_WideString& other) const { return Equal(other); }

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_WideString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_WideString& str) const {
    int result =
        wmemcmp(c_str(), str.c_str(), std::min(GetLength(), str.GetLength()));
    return result < 0 || (result == 0 && GetLength() < str.GetLength());
  }

  FX_WCHAR GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  FX_WCHAR operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, FX_WCHAR ch);

  int Compare(const FX_WCHAR* str) const;

  int Compare(const CFX_WideString& str) const;

  int CompareNoCase(const FX_WCHAR* str) const;

  bool Equal(const wchar_t* ptr) const;
  bool Equal(const CFX_WideStringC& str) const;
  bool Equal(const CFX_WideString& other) const;

  CFX_WideString Mid(FX_STRSIZE first) const;

  CFX_WideString Mid(FX_STRSIZE first, FX_STRSIZE count) const;

  CFX_WideString Left(FX_STRSIZE count) const;

  CFX_WideString Right(FX_STRSIZE count) const;

  FX_STRSIZE Insert(FX_STRSIZE index, FX_WCHAR ch);

  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const FX_WCHAR* lpszFormat, ...);

  void FormatV(const FX_WCHAR* lpszFormat, va_list argList);

  void MakeLower();

  void MakeUpper();

  void TrimRight();

  void TrimRight(FX_WCHAR chTarget);

  void TrimRight(const FX_WCHAR* lpszTargets);

  void TrimLeft();

  void TrimLeft(FX_WCHAR chTarget);

  void TrimLeft(const FX_WCHAR* lpszTargets);

  void Reserve(FX_STRSIZE len);

  FX_WCHAR* GetBuffer(FX_STRSIZE len);

  void ReleaseBuffer(FX_STRSIZE len = -1);

  int GetInteger() const;

  FX_FLOAT GetFloat() const;

  FX_STRSIZE Find(const FX_WCHAR* lpszSub, FX_STRSIZE start = 0) const;

  FX_STRSIZE Find(FX_WCHAR ch, FX_STRSIZE start = 0) const;

  FX_STRSIZE Replace(const FX_WCHAR* lpszOld, const FX_WCHAR* lpszNew);

  FX_STRSIZE Remove(FX_WCHAR ch);

  CFX_ByteString UTF8Encode() const;

  CFX_ByteString UTF16LE_Encode() const;

  void ConvertFrom(const CFX_ByteString& str, CFX_CharMap* pCharMap = NULL);

 protected:
  class StringData {
   public:
    static StringData* Create(int nLen);
    void Retain() { ++m_nRefs; }
    void Release() {
      if (--m_nRefs <= 0)
        FX_Free(this);
    }

    intptr_t m_nRefs;  // Would prefer ssize_t, but no windows support.
    FX_STRSIZE m_nDataLength;
    FX_STRSIZE m_nAllocLength;
    FX_WCHAR m_String[1];

   private:
    StringData(FX_STRSIZE dataLen, FX_STRSIZE allocLen)
        : m_nRefs(1), m_nDataLength(dataLen), m_nAllocLength(allocLen) {
      FXSYS_assert(dataLen >= 0);
      FXSYS_assert(allocLen >= 0);
      FXSYS_assert(dataLen <= allocLen);
      m_String[dataLen] = 0;
    }
    ~StringData() = delete;
  };

  void CopyBeforeWrite();
  void AllocBeforeWrite(FX_STRSIZE nLen);
  void ConcatInPlace(FX_STRSIZE nSrcLen, const FX_WCHAR* lpszSrcData);
  void ConcatCopy(FX_STRSIZE nSrc1Len,
                  const FX_WCHAR* lpszSrc1Data,
                  FX_STRSIZE nSrc2Len,
                  const FX_WCHAR* lpszSrc2Data);
  void AssignCopy(FX_STRSIZE nSrcLen, const FX_WCHAR* lpszSrcData);
  void AllocCopy(CFX_WideString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;

  StringData* m_pData;
  friend class fxcrt_WideStringConcatInPlace_Test;
};
inline CFX_WideStringC::CFX_WideStringC(const CFX_WideString& src) {
  m_Ptr = src.c_str();
  m_Length = src.GetLength();
}
inline CFX_WideStringC& CFX_WideStringC::operator=(const CFX_WideString& src) {
  m_Ptr = src.c_str();
  m_Length = src.GetLength();
  return *this;
}

inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const FX_WCHAR* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const FX_WCHAR* str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1, FX_WCHAR ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(FX_WCHAR ch, const CFX_WideStringC& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1, FX_WCHAR ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(FX_WCHAR ch, const CFX_WideString& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const FX_WCHAR* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const FX_WCHAR* str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2);
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
FX_FLOAT FX_atof(const CFX_ByteStringC& str);
void FX_atonum(const CFX_ByteStringC& str, FX_BOOL& bInteger, void* pData);
FX_STRSIZE FX_ftoa(FX_FLOAT f, FX_CHAR* buf);
CFX_ByteString FX_UTF8Encode(const FX_WCHAR* pwsStr, FX_STRSIZE len);
inline CFX_ByteString FX_UTF8Encode(const CFX_WideStringC& wsStr) {
  return FX_UTF8Encode(wsStr.GetPtr(), wsStr.GetLength());
}
inline CFX_ByteString FX_UTF8Encode(const CFX_WideString& wsStr) {
  return FX_UTF8Encode(wsStr.c_str(), wsStr.GetLength());
}

#endif  // CORE_INCLUDE_FXCRT_FX_STRING_H_
