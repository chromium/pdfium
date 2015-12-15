// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stddef.h>  // For offsetof().
#include <cctype>

#include "core/include/fxcrt/fx_basic.h"
#include "third_party/base/numerics/safe_math.h"

static int _Buffer_itoa(char* buf, int i, FX_DWORD flags) {
  if (i == 0) {
    buf[0] = '0';
    return 1;
  }
  char buf1[32];
  int buf_pos = 31;
  FX_DWORD u = i;
  if ((flags & FXFORMAT_SIGNED) && i < 0) {
    u = -i;
  }
  int base = 10;
  const FX_CHAR* string = "0123456789abcdef";
  if (flags & FXFORMAT_HEX) {
    base = 16;
    if (flags & FXFORMAT_CAPITAL) {
      string = "0123456789ABCDEF";
    }
  }
  while (u != 0) {
    buf1[buf_pos--] = string[u % base];
    u = u / base;
  }
  if ((flags & FXFORMAT_SIGNED) && i < 0) {
    buf1[buf_pos--] = '-';
  }
  int len = 31 - buf_pos;
  for (int ii = 0; ii < len; ii++) {
    buf[ii] = buf1[ii + buf_pos + 1];
  }
  return len;
}
CFX_ByteString CFX_ByteString::FormatInteger(int i, FX_DWORD flags) {
  char buf[32];
  return CFX_ByteStringC(buf, _Buffer_itoa(buf, i, flags));
}

// static
CFX_ByteString::StringData* CFX_ByteString::StringData::Create(int nLen) {
  // |nLen| is currently declared as in |int|. TODO(palmer): It should be
  // a |size_t|, or at least unsigned.
  if (nLen == 0 || nLen < 0) {
    return NULL;
  }

  // Fixed portion of header plus a NUL char not included in m_nAllocLength.
  // sizeof(FX_CHAR) is always 1, used for consistency with CFX_Widestring.
  int overhead = offsetof(StringData, m_String) + sizeof(FX_CHAR);
  pdfium::base::CheckedNumeric<int> nSize = nLen;
  nSize += overhead;

  // Now round to an 8-byte boundary. We'd expect that this is the minimum
  // granularity of any of the underlying allocators, so there may be cases
  // where we can save a re-alloc when adding a few characters to a string
  // by using this otherwise wasted space.
  nSize += 7;
  int totalSize = nSize.ValueOrDie() & ~7;
  int usableSize = totalSize - overhead;
  FXSYS_assert(usableSize >= nLen);

  void* pData = FX_Alloc(uint8_t, totalSize);
  return new (pData) StringData(nLen, usableSize);
}
CFX_ByteString::~CFX_ByteString() {
  if (m_pData) {
    m_pData->Release();
  }
}
CFX_ByteString::CFX_ByteString(const FX_CHAR* lpsz, FX_STRSIZE nLen) {
  if (nLen < 0) {
    nLen = lpsz ? FXSYS_strlen(lpsz) : 0;
  }
  if (nLen) {
    m_pData = StringData::Create(nLen);
    if (m_pData) {
      FXSYS_memcpy(m_pData->m_String, lpsz, nLen);
    }
  } else {
    m_pData = NULL;
  }
}
CFX_ByteString::CFX_ByteString(const uint8_t* lpsz, FX_STRSIZE nLen) {
  if (nLen > 0) {
    m_pData = StringData::Create(nLen);
    if (m_pData) {
      FXSYS_memcpy(m_pData->m_String, lpsz, nLen);
    }
  } else {
    m_pData = NULL;
  }
}
CFX_ByteString::CFX_ByteString(char ch) {
  m_pData = StringData::Create(1);
  if (m_pData) {
    m_pData->m_String[0] = ch;
  }
}
CFX_ByteString::CFX_ByteString(const CFX_ByteString& stringSrc) {
  if (!stringSrc.m_pData) {
    m_pData = NULL;
    return;
  }
  if (stringSrc.m_pData->m_nRefs >= 0) {
    m_pData = stringSrc.m_pData;
    m_pData->Retain();
  } else {
    m_pData = NULL;
    *this = stringSrc;
  }
}
CFX_ByteString::CFX_ByteString(const CFX_ByteStringC& stringSrc) {
  if (stringSrc.IsEmpty()) {
    m_pData = NULL;
    return;
  }
  m_pData = NULL;
  *this = stringSrc;
}
CFX_ByteString::CFX_ByteString(const CFX_ByteStringC& str1,
                               const CFX_ByteStringC& str2) {
  m_pData = NULL;
  int nNewLen = str1.GetLength() + str2.GetLength();
  if (nNewLen == 0) {
    return;
  }
  m_pData = StringData::Create(nNewLen);
  if (m_pData) {
    FXSYS_memcpy(m_pData->m_String, str1.GetCStr(), str1.GetLength());
    FXSYS_memcpy(m_pData->m_String + str1.GetLength(), str2.GetCStr(),
                 str2.GetLength());
  }
}
const CFX_ByteString& CFX_ByteString::operator=(const FX_CHAR* lpsz) {
  if (!lpsz || lpsz[0] == 0) {
    Empty();
  } else {
    AssignCopy(FXSYS_strlen(lpsz), lpsz);
  }
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(const CFX_ByteStringC& str) {
  if (str.IsEmpty()) {
    Empty();
  } else {
    AssignCopy(str.GetLength(), str.GetCStr());
  }
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(
    const CFX_ByteString& stringSrc) {
  if (m_pData == stringSrc.m_pData) {
    return *this;
  }
  if (stringSrc.IsEmpty()) {
    Empty();
  } else if ((m_pData && m_pData->m_nRefs < 0) ||
             (stringSrc.m_pData && stringSrc.m_pData->m_nRefs < 0)) {
    AssignCopy(stringSrc.m_pData->m_nDataLength, stringSrc.m_pData->m_String);
  } else {
    Empty();
    m_pData = stringSrc.m_pData;
    if (m_pData) {
      m_pData->Retain();
    }
  }
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(const CFX_BinaryBuf& buf) {
  Load(buf.GetBuffer(), buf.GetSize());
  return *this;
}
void CFX_ByteString::Load(const uint8_t* buf, FX_STRSIZE len) {
  Empty();
  if (len) {
    m_pData = StringData::Create(len);
    if (m_pData) {
      FXSYS_memcpy(m_pData->m_String, buf, len);
    }
  } else {
    m_pData = NULL;
  }
}
const CFX_ByteString& CFX_ByteString::operator+=(const FX_CHAR* lpsz) {
  if (lpsz) {
    ConcatInPlace(FXSYS_strlen(lpsz), lpsz);
  }
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(char ch) {
  ConcatInPlace(1, &ch);
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(const CFX_ByteString& string) {
  if (!string.m_pData) {
    return *this;
  }
  ConcatInPlace(string.m_pData->m_nDataLength, string.m_pData->m_String);
  return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(
    const CFX_ByteStringC& string) {
  if (string.IsEmpty()) {
    return *this;
  }
  ConcatInPlace(string.GetLength(), string.GetCStr());
  return *this;
}
bool CFX_ByteString::Equal(const char* ptr) const {
  if (!m_pData) {
    return !ptr || ptr[0] == '\0';
  }
  if (!ptr) {
    return m_pData->m_nDataLength == 0;
  }
  return FXSYS_strlen(ptr) == m_pData->m_nDataLength &&
         FXSYS_memcmp(ptr, m_pData->m_String, m_pData->m_nDataLength) == 0;
}
bool CFX_ByteString::Equal(const CFX_ByteStringC& str) const {
  if (!m_pData) {
    return str.IsEmpty();
  }
  return m_pData->m_nDataLength == str.GetLength() &&
         FXSYS_memcmp(m_pData->m_String, str.GetCStr(), str.GetLength()) == 0;
}
bool CFX_ByteString::Equal(const CFX_ByteString& other) const {
  if (IsEmpty()) {
    return other.IsEmpty();
  }
  if (other.IsEmpty()) {
    return false;
  }
  return other.m_pData->m_nDataLength == m_pData->m_nDataLength &&
         FXSYS_memcmp(other.m_pData->m_String, m_pData->m_String,
                      m_pData->m_nDataLength) == 0;
}
void CFX_ByteString::Empty() {
  if (m_pData) {
    m_pData->Release();
    m_pData = NULL;
  }
}
bool CFX_ByteString::EqualNoCase(const CFX_ByteStringC& str) const {
  if (!m_pData) {
    return str.IsEmpty();
  }
  FX_STRSIZE len = str.GetLength();
  if (m_pData->m_nDataLength != len) {
    return false;
  }
  const uint8_t* pThis = (const uint8_t*)m_pData->m_String;
  const uint8_t* pThat = str.GetPtr();
  for (FX_STRSIZE i = 0; i < len; i++) {
    if ((*pThis) != (*pThat)) {
      uint8_t bThis = *pThis;
      if (bThis >= 'A' && bThis <= 'Z') {
        bThis += 'a' - 'A';
      }
      uint8_t bThat = *pThat;
      if (bThat >= 'A' && bThat <= 'Z') {
        bThat += 'a' - 'A';
      }
      if (bThis != bThat) {
        return false;
      }
    }
    pThis++;
    pThat++;
  }
  return true;
}
void CFX_ByteString::AssignCopy(FX_STRSIZE nSrcLen,
                                const FX_CHAR* lpszSrcData) {
  AllocBeforeWrite(nSrcLen);
  FXSYS_memcpy(m_pData->m_String, lpszSrcData, nSrcLen);
  m_pData->m_nDataLength = nSrcLen;
  m_pData->m_String[nSrcLen] = 0;
}
void CFX_ByteString::CopyBeforeWrite() {
  if (!m_pData || m_pData->m_nRefs <= 1) {
    return;
  }
  StringData* pData = m_pData;
  m_pData->Release();
  FX_STRSIZE nDataLength = pData->m_nDataLength;
  m_pData = StringData::Create(nDataLength);
  if (m_pData) {
    FXSYS_memcpy(m_pData->m_String, pData->m_String, nDataLength + 1);
  }
}
void CFX_ByteString::AllocBeforeWrite(FX_STRSIZE nLen) {
  if (m_pData && m_pData->m_nRefs <= 1 && m_pData->m_nAllocLength >= nLen) {
    return;
  }
  Empty();
  m_pData = StringData::Create(nLen);
}
void CFX_ByteString::ReleaseBuffer(FX_STRSIZE nNewLength) {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (nNewLength == -1) {
    nNewLength = FXSYS_strlen((const FX_CHAR*)m_pData->m_String);
  }
  if (nNewLength == 0) {
    Empty();
    return;
  }
  FXSYS_assert(nNewLength <= m_pData->m_nAllocLength);
  m_pData->m_nDataLength = nNewLength;
  m_pData->m_String[nNewLength] = 0;
}
void CFX_ByteString::Reserve(FX_STRSIZE len) {
  GetBuffer(len);
  ReleaseBuffer(GetLength());
}
FX_CHAR* CFX_ByteString::GetBuffer(FX_STRSIZE nMinBufLength) {
  if (!m_pData && nMinBufLength == 0) {
    return NULL;
  }
  if (m_pData && m_pData->m_nRefs <= 1 &&
      m_pData->m_nAllocLength >= nMinBufLength) {
    return m_pData->m_String;
  }
  if (!m_pData) {
    m_pData = StringData::Create(nMinBufLength);
    if (!m_pData) {
      return NULL;
    }
    m_pData->m_nDataLength = 0;
    m_pData->m_String[0] = 0;
    return m_pData->m_String;
  }
  StringData* pOldData = m_pData;
  FX_STRSIZE nOldLen = pOldData->m_nDataLength;
  if (nMinBufLength < nOldLen) {
    nMinBufLength = nOldLen;
  }
  m_pData = StringData::Create(nMinBufLength);
  if (!m_pData) {
    return NULL;
  }
  FXSYS_memcpy(m_pData->m_String, pOldData->m_String, (nOldLen + 1));
  m_pData->m_nDataLength = nOldLen;
  pOldData->Release();
  return m_pData->m_String;
}
FX_STRSIZE CFX_ByteString::Delete(FX_STRSIZE nIndex, FX_STRSIZE nCount) {
  if (!m_pData) {
    return 0;
  }
  if (nIndex < 0) {
    nIndex = 0;
  }
  FX_STRSIZE nOldLength = m_pData->m_nDataLength;
  if (nCount > 0 && nIndex < nOldLength) {
    FX_STRSIZE mLength = nIndex + nCount;
    if (mLength >= nOldLength) {
      m_pData->m_nDataLength = nIndex;
      return m_pData->m_nDataLength;
    }
    CopyBeforeWrite();
    int nBytesToCopy = nOldLength - mLength + 1;
    FXSYS_memmove(m_pData->m_String + nIndex, m_pData->m_String + mLength,
                  nBytesToCopy);
    m_pData->m_nDataLength = nOldLength - nCount;
  }
  return m_pData->m_nDataLength;
}
void CFX_ByteString::ConcatInPlace(FX_STRSIZE nSrcLen,
                                   const FX_CHAR* lpszSrcData) {
  if (nSrcLen == 0 || !lpszSrcData) {
    return;
  }
  if (!m_pData) {
    m_pData = StringData::Create(nSrcLen);
    if (!m_pData) {
      return;
    }
    FXSYS_memcpy(m_pData->m_String, lpszSrcData, nSrcLen);
    return;
  }
  if (m_pData->m_nRefs > 1 ||
      m_pData->m_nDataLength + nSrcLen > m_pData->m_nAllocLength) {
    ConcatCopy(m_pData->m_nDataLength, m_pData->m_String, nSrcLen, lpszSrcData);
  } else {
    FXSYS_memcpy(m_pData->m_String + m_pData->m_nDataLength, lpszSrcData,
                 nSrcLen);
    m_pData->m_nDataLength += nSrcLen;
    m_pData->m_String[m_pData->m_nDataLength] = 0;
  }
}
void CFX_ByteString::ConcatCopy(FX_STRSIZE nSrc1Len,
                                const FX_CHAR* lpszSrc1Data,
                                FX_STRSIZE nSrc2Len,
                                const FX_CHAR* lpszSrc2Data) {
  int nNewLen = nSrc1Len + nSrc2Len;
  if (nNewLen <= 0) {
    return;
  }
  // Don't release until done copying, might be one of the arguments.
  StringData* pOldData = m_pData;
  m_pData = StringData::Create(nNewLen);
  if (m_pData) {
    memcpy(m_pData->m_String, lpszSrc1Data, nSrc1Len);
    memcpy(m_pData->m_String + nSrc1Len, lpszSrc2Data, nSrc2Len);
  }
  pOldData->Release();
}
CFX_ByteString CFX_ByteString::Mid(FX_STRSIZE nFirst) const {
  if (!m_pData) {
    return CFX_ByteString();
  }
  return Mid(nFirst, m_pData->m_nDataLength - nFirst);
}
CFX_ByteString CFX_ByteString::Mid(FX_STRSIZE nFirst, FX_STRSIZE nCount) const {
  if (nFirst < 0) {
    nFirst = 0;
  }
  if (nCount < 0) {
    nCount = 0;
  }
  if (nFirst + nCount > m_pData->m_nDataLength) {
    nCount = m_pData->m_nDataLength - nFirst;
  }
  if (nFirst > m_pData->m_nDataLength) {
    nCount = 0;
  }
  if (nFirst == 0 && nFirst + nCount == m_pData->m_nDataLength) {
    return *this;
  }
  CFX_ByteString dest;
  AllocCopy(dest, nCount, nFirst);
  return dest;
}
void CFX_ByteString::AllocCopy(CFX_ByteString& dest,
                               FX_STRSIZE nCopyLen,
                               FX_STRSIZE nCopyIndex) const {
  // |FX_STRSIZE| is currently typedef'd as in |int|. TODO(palmer): It
  // should be a |size_t|, or at least unsigned.
  if (nCopyLen == 0 || nCopyLen < 0) {
    return;
  }
  ASSERT(!dest.m_pData);
  dest.m_pData = StringData::Create(nCopyLen);
  if (dest.m_pData) {
    FXSYS_memcpy(dest.m_pData->m_String, m_pData->m_String + nCopyIndex,
                 nCopyLen);
  }
}
#define FORCE_ANSI 0x10000
#define FORCE_UNICODE 0x20000
#define FORCE_INT64 0x40000
void CFX_ByteString::FormatV(const FX_CHAR* lpszFormat, va_list argList) {
  va_list argListSave;
#if defined(__ARMCC_VERSION) ||                                              \
    (!defined(_MSC_VER) && (_FX_CPU_ == _FX_X64_ || _FX_CPU_ == _FX_IA64_ || \
                            _FX_CPU_ == _FX_ARM64_)) ||                      \
    defined(__native_client__)
  va_copy(argListSave, argList);
#else
  argListSave = argList;
#endif
  int nMaxLen = 0;
  for (const FX_CHAR* lpsz = lpszFormat; *lpsz != 0; lpsz++) {
    if (*lpsz != '%' || *(lpsz = lpsz + 1) == '%') {
      nMaxLen += FXSYS_strlen(lpsz);
      continue;
    }
    int nItemLen = 0;
    int nWidth = 0;
    for (; *lpsz != 0; lpsz++) {
      if (*lpsz == '#') {
        nMaxLen += 2;
      } else if (*lpsz == '*') {
        nWidth = va_arg(argList, int);
      } else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' || *lpsz == ' ')
        ;
      else {
        break;
      }
    }
    if (nWidth == 0) {
      nWidth = FXSYS_atoi(lpsz);
      while (std::isdigit(*lpsz))
        lpsz++;
    }
    if (nWidth < 0 || nWidth > 128 * 1024) {
      lpszFormat = "Bad width";
      nMaxLen = 10;
      break;
    }
    int nPrecision = 0;
    if (*lpsz == '.') {
      lpsz++;
      if (*lpsz == '*') {
        nPrecision = va_arg(argList, int);
        lpsz++;
      } else {
        nPrecision = FXSYS_atoi(lpsz);
        while (std::isdigit(*lpsz))
          lpsz++;
      }
    }
    if (nPrecision < 0 || nPrecision > 128 * 1024) {
      lpszFormat = "Bad precision";
      nMaxLen = 14;
      break;
    }
    int nModifier = 0;
    if (FXSYS_strncmp(lpsz, "I64", 3) == 0) {
      lpsz += 3;
      nModifier = FORCE_INT64;
    } else {
      switch (*lpsz) {
        case 'h':
          nModifier = FORCE_ANSI;
          lpsz++;
          break;
        case 'l':
          nModifier = FORCE_UNICODE;
          lpsz++;
          break;
        case 'F':
        case 'N':
        case 'L':
          lpsz++;
          break;
      }
    }
    switch (*lpsz | nModifier) {
      case 'c':
      case 'C':
        nItemLen = 2;
        va_arg(argList, int);
        break;
      case 'c' | FORCE_ANSI:
      case 'C' | FORCE_ANSI:
        nItemLen = 2;
        va_arg(argList, int);
        break;
      case 'c' | FORCE_UNICODE:
      case 'C' | FORCE_UNICODE:
        nItemLen = 2;
        va_arg(argList, int);
        break;
      case 's': {
        const FX_CHAR* pstrNextArg = va_arg(argList, const FX_CHAR*);
        if (pstrNextArg) {
          nItemLen = FXSYS_strlen(pstrNextArg);
          if (nItemLen < 1) {
            nItemLen = 1;
          }
        } else {
          nItemLen = 6;
        }
      } break;
      case 'S': {
        FX_WCHAR* pstrNextArg = va_arg(argList, FX_WCHAR*);
        if (pstrNextArg) {
          nItemLen = FXSYS_wcslen(pstrNextArg);
          if (nItemLen < 1) {
            nItemLen = 1;
          }
        } else {
          nItemLen = 6;
        }
      } break;
      case 's' | FORCE_ANSI:
      case 'S' | FORCE_ANSI: {
        const FX_CHAR* pstrNextArg = va_arg(argList, const FX_CHAR*);
        if (pstrNextArg) {
          nItemLen = FXSYS_strlen(pstrNextArg);
          if (nItemLen < 1) {
            nItemLen = 1;
          }
        } else {
          nItemLen = 6;
        }
      } break;
      case 's' | FORCE_UNICODE:
      case 'S' | FORCE_UNICODE: {
        FX_WCHAR* pstrNextArg = va_arg(argList, FX_WCHAR*);
        if (pstrNextArg) {
          nItemLen = FXSYS_wcslen(pstrNextArg);
          if (nItemLen < 1) {
            nItemLen = 1;
          }
        } else {
          nItemLen = 6;
        }
      } break;
    }
    if (nItemLen != 0) {
      if (nPrecision != 0 && nItemLen > nPrecision) {
        nItemLen = nPrecision;
      }
      if (nItemLen < nWidth) {
        nItemLen = nWidth;
      }
    } else {
      switch (*lpsz) {
        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'X':
        case 'o':
          if (nModifier & FORCE_INT64) {
            va_arg(argList, int64_t);
          } else {
            va_arg(argList, int);
          }
          nItemLen = 32;
          if (nItemLen < nWidth + nPrecision) {
            nItemLen = nWidth + nPrecision;
          }
          break;
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
          va_arg(argList, double);
          nItemLen = 128;
          if (nItemLen < nWidth + nPrecision) {
            nItemLen = nWidth + nPrecision;
          }
          break;
        case 'f':
          if (nWidth + nPrecision > 100) {
            nItemLen = nPrecision + nWidth + 128;
          } else {
            char pszTemp[256];
            double f = va_arg(argList, double);
            memset(pszTemp, 0, sizeof(pszTemp));
            FXSYS_snprintf(pszTemp, sizeof(pszTemp) - 1, "%*.*f", nWidth,
                           nPrecision + 6, f);
            nItemLen = FXSYS_strlen(pszTemp);
          }
          break;
        case 'p':
          va_arg(argList, void*);
          nItemLen = 32;
          if (nItemLen < nWidth + nPrecision) {
            nItemLen = nWidth + nPrecision;
          }
          break;
        case 'n':
          va_arg(argList, int*);
          break;
      }
    }
    nMaxLen += nItemLen;
  }
  nMaxLen += 32;  // Fudge factor.
  GetBuffer(nMaxLen);
  if (m_pData) {
    memset(m_pData->m_String, 0, nMaxLen);
    FXSYS_vsnprintf(m_pData->m_String, nMaxLen - 1, lpszFormat, argListSave);
    ReleaseBuffer();
  }
  va_end(argListSave);
}
void CFX_ByteString::Format(const FX_CHAR* lpszFormat, ...) {
  va_list argList;
  va_start(argList, lpszFormat);
  FormatV(lpszFormat, argList);
  va_end(argList);
}
FX_STRSIZE CFX_ByteString::Insert(FX_STRSIZE nIndex, FX_CHAR ch) {
  CopyBeforeWrite();
  if (nIndex < 0) {
    nIndex = 0;
  }
  FX_STRSIZE nNewLength = m_pData ? m_pData->m_nDataLength : 0;
  if (nIndex > nNewLength) {
    nIndex = nNewLength;
  }
  nNewLength++;
  if (!m_pData || m_pData->m_nAllocLength < nNewLength) {
    StringData* pOldData = m_pData;
    const FX_CHAR* pstr = m_pData->m_String;
    m_pData = StringData::Create(nNewLength);
    if (!m_pData) {
      return 0;
    }
    if (pOldData) {
      FXSYS_memmove(m_pData->m_String, pstr, (pOldData->m_nDataLength + 1));
      pOldData->Release();
    } else {
      m_pData->m_String[0] = 0;
    }
  }
  FXSYS_memmove(m_pData->m_String + nIndex + 1, m_pData->m_String + nIndex,
                (nNewLength - nIndex));
  m_pData->m_String[nIndex] = ch;
  m_pData->m_nDataLength = nNewLength;
  return nNewLength;
}
CFX_ByteString CFX_ByteString::Right(FX_STRSIZE nCount) const {
  if (!m_pData) {
    return CFX_ByteString();
  }
  if (nCount < 0) {
    nCount = 0;
  }
  if (nCount >= m_pData->m_nDataLength) {
    return *this;
  }
  CFX_ByteString dest;
  AllocCopy(dest, nCount, m_pData->m_nDataLength - nCount);
  return dest;
}
CFX_ByteString CFX_ByteString::Left(FX_STRSIZE nCount) const {
  if (!m_pData) {
    return CFX_ByteString();
  }
  if (nCount < 0) {
    nCount = 0;
  }
  if (nCount >= m_pData->m_nDataLength) {
    return *this;
  }
  CFX_ByteString dest;
  AllocCopy(dest, nCount, 0);
  return dest;
}
FX_STRSIZE CFX_ByteString::Find(FX_CHAR ch, FX_STRSIZE nStart) const {
  if (!m_pData) {
    return -1;
  }
  FX_STRSIZE nLength = m_pData->m_nDataLength;
  if (nStart >= nLength) {
    return -1;
  }
  const FX_CHAR* lpsz = FXSYS_strchr(m_pData->m_String + nStart, ch);
  return lpsz ? (int)(lpsz - m_pData->m_String) : -1;
}
FX_STRSIZE CFX_ByteString::ReverseFind(FX_CHAR ch) const {
  if (!m_pData) {
    return -1;
  }
  FX_STRSIZE nLength = m_pData->m_nDataLength;
  while (nLength) {
    if (m_pData->m_String[nLength - 1] == ch) {
      return nLength - 1;
    }
    nLength--;
  }
  return -1;
}
const FX_CHAR* FX_strstr(const FX_CHAR* str1,
                         int len1,
                         const FX_CHAR* str2,
                         int len2) {
  if (len2 > len1 || len2 == 0) {
    return NULL;
  }
  const FX_CHAR* end_ptr = str1 + len1 - len2;
  while (str1 <= end_ptr) {
    int i = 0;
    while (1) {
      if (str1[i] != str2[i]) {
        break;
      }
      i++;
      if (i == len2) {
        return str1;
      }
    }
    str1++;
  }
  return NULL;
}
FX_STRSIZE CFX_ByteString::Find(const CFX_ByteStringC& lpszSub,
                                FX_STRSIZE nStart) const {
  if (!m_pData) {
    return -1;
  }
  FX_STRSIZE nLength = m_pData->m_nDataLength;
  if (nStart > nLength) {
    return -1;
  }
  const FX_CHAR* lpsz =
      FX_strstr(m_pData->m_String + nStart, m_pData->m_nDataLength - nStart,
                lpszSub.GetCStr(), lpszSub.GetLength());
  return lpsz ? (int)(lpsz - m_pData->m_String) : -1;
}
void CFX_ByteString::MakeLower() {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return;
  }
  FXSYS_strlwr(m_pData->m_String);
}
void CFX_ByteString::MakeUpper() {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return;
  }
  FXSYS_strupr(m_pData->m_String);
}
FX_STRSIZE CFX_ByteString::Remove(FX_CHAR chRemove) {
  if (!m_pData) {
    return 0;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return 0;
  }
  FX_CHAR* pstrSource = m_pData->m_String;
  FX_CHAR* pstrDest = m_pData->m_String;
  FX_CHAR* pstrEnd = m_pData->m_String + m_pData->m_nDataLength;
  while (pstrSource < pstrEnd) {
    if (*pstrSource != chRemove) {
      *pstrDest = *pstrSource;
      pstrDest++;
    }
    pstrSource++;
  }
  *pstrDest = 0;
  FX_STRSIZE nCount = (FX_STRSIZE)(pstrSource - pstrDest);
  m_pData->m_nDataLength -= nCount;
  return nCount;
}
FX_STRSIZE CFX_ByteString::Replace(const CFX_ByteStringC& lpszOld,
                                   const CFX_ByteStringC& lpszNew) {
  if (!m_pData) {
    return 0;
  }
  if (lpszOld.IsEmpty()) {
    return 0;
  }
  FX_STRSIZE nSourceLen = lpszOld.GetLength();
  FX_STRSIZE nReplacementLen = lpszNew.GetLength();
  FX_STRSIZE nCount = 0;
  const FX_CHAR* pStart = m_pData->m_String;
  FX_CHAR* pEnd = m_pData->m_String + m_pData->m_nDataLength;
  while (1) {
    const FX_CHAR* pTarget = FX_strstr(pStart, (FX_STRSIZE)(pEnd - pStart),
                                       lpszOld.GetCStr(), nSourceLen);
    if (!pTarget) {
      break;
    }
    nCount++;
    pStart = pTarget + nSourceLen;
  }
  if (nCount == 0) {
    return 0;
  }
  FX_STRSIZE nNewLength =
      m_pData->m_nDataLength + (nReplacementLen - nSourceLen) * nCount;
  if (nNewLength == 0) {
    Empty();
    return nCount;
  }
  StringData* pNewData = StringData::Create(nNewLength);
  if (!pNewData) {
    return 0;
  }
  pStart = m_pData->m_String;
  FX_CHAR* pDest = pNewData->m_String;
  for (FX_STRSIZE i = 0; i < nCount; i++) {
    const FX_CHAR* pTarget = FX_strstr(pStart, (FX_STRSIZE)(pEnd - pStart),
                                       lpszOld.GetCStr(), nSourceLen);
    FXSYS_memcpy(pDest, pStart, pTarget - pStart);
    pDest += pTarget - pStart;
    FXSYS_memcpy(pDest, lpszNew.GetCStr(), lpszNew.GetLength());
    pDest += lpszNew.GetLength();
    pStart = pTarget + nSourceLen;
  }
  FXSYS_memcpy(pDest, pStart, pEnd - pStart);
  m_pData->Release();
  m_pData = pNewData;
  return nCount;
}
void CFX_ByteString::SetAt(FX_STRSIZE nIndex, FX_CHAR ch) {
  if (!m_pData) {
    return;
  }
  FXSYS_assert(nIndex >= 0);
  FXSYS_assert(nIndex < m_pData->m_nDataLength);
  CopyBeforeWrite();
  m_pData->m_String[nIndex] = ch;
}
CFX_WideString CFX_ByteString::UTF8Decode() const {
  CFX_UTF8Decoder decoder;
  for (FX_STRSIZE i = 0; i < GetLength(); i++) {
    decoder.Input((uint8_t)m_pData->m_String[i]);
  }
  return decoder.GetResult();
}
CFX_ByteString CFX_ByteString::FromUnicode(const FX_WCHAR* str,
                                           FX_STRSIZE len) {
  if (len < 0) {
    len = FXSYS_wcslen(str);
  }
  CFX_ByteString bstr;
  bstr.ConvertFrom(CFX_WideString(str, len));
  return bstr;
}
CFX_ByteString CFX_ByteString::FromUnicode(const CFX_WideString& str) {
  return FromUnicode(str.c_str(), str.GetLength());
}
void CFX_ByteString::ConvertFrom(const CFX_WideString& str,
                                 CFX_CharMap* pCharMap) {
  if (!pCharMap) {
    pCharMap = CFX_CharMap::GetDefaultMapper();
  }
  *this = (*pCharMap->m_GetByteString)(pCharMap, str);
}
int CFX_ByteString::Compare(const CFX_ByteStringC& str) const {
  if (!m_pData) {
    return str.IsEmpty() ? 0 : -1;
  }
  int this_len = m_pData->m_nDataLength;
  int that_len = str.GetLength();
  int min_len = this_len < that_len ? this_len : that_len;
  for (int i = 0; i < min_len; i++) {
    if ((uint8_t)m_pData->m_String[i] < str.GetAt(i)) {
      return -1;
    }
    if ((uint8_t)m_pData->m_String[i] > str.GetAt(i)) {
      return 1;
    }
  }
  if (this_len < that_len) {
    return -1;
  }
  if (this_len > that_len) {
    return 1;
  }
  return 0;
}
void CFX_ByteString::TrimRight(const CFX_ByteStringC& lpszTargets) {
  if (!m_pData || lpszTargets.IsEmpty()) {
    return;
  }
  CopyBeforeWrite();
  FX_STRSIZE pos = GetLength();
  if (pos < 1) {
    return;
  }
  while (pos) {
    FX_STRSIZE i = 0;
    while (i < lpszTargets.GetLength() &&
           lpszTargets[i] != m_pData->m_String[pos - 1]) {
      i++;
    }
    if (i == lpszTargets.GetLength()) {
      break;
    }
    pos--;
  }
  if (pos < m_pData->m_nDataLength) {
    m_pData->m_String[pos] = 0;
    m_pData->m_nDataLength = pos;
  }
}
void CFX_ByteString::TrimRight(FX_CHAR chTarget) {
  TrimRight(CFX_ByteStringC(chTarget));
}
void CFX_ByteString::TrimRight() {
  TrimRight("\x09\x0a\x0b\x0c\x0d\x20");
}
void CFX_ByteString::TrimLeft(const CFX_ByteStringC& lpszTargets) {
  if (!m_pData) {
    return;
  }
  if (lpszTargets.IsEmpty()) {
    return;
  }
  CopyBeforeWrite();
  FX_STRSIZE len = GetLength();
  if (len < 1) {
    return;
  }
  FX_STRSIZE pos = 0;
  while (pos < len) {
    FX_STRSIZE i = 0;
    while (i < lpszTargets.GetLength() &&
           lpszTargets[i] != m_pData->m_String[pos]) {
      i++;
    }
    if (i == lpszTargets.GetLength()) {
      break;
    }
    pos++;
  }
  if (pos) {
    FX_STRSIZE nDataLength = len - pos;
    FXSYS_memmove(m_pData->m_String, m_pData->m_String + pos,
                  (nDataLength + 1) * sizeof(FX_CHAR));
    m_pData->m_nDataLength = nDataLength;
  }
}
void CFX_ByteString::TrimLeft(FX_CHAR chTarget) {
  TrimLeft(CFX_ByteStringC(chTarget));
}
void CFX_ByteString::TrimLeft() {
  TrimLeft("\x09\x0a\x0b\x0c\x0d\x20");
}
FX_DWORD CFX_ByteString::GetID(FX_STRSIZE start_pos) const {
  return CFX_ByteStringC(*this).GetID(start_pos);
}
FX_DWORD CFX_ByteStringC::GetID(FX_STRSIZE start_pos) const {
  if (m_Length == 0) {
    return 0;
  }
  if (start_pos < 0 || start_pos >= m_Length) {
    return 0;
  }
  FX_DWORD strid = 0;
  if (start_pos + 4 > m_Length) {
    for (FX_STRSIZE i = 0; i < m_Length - start_pos; i++) {
      strid = strid * 256 + m_Ptr[start_pos + i];
    }
    strid = strid << ((4 - m_Length + start_pos) * 8);
  } else {
    for (int i = 0; i < 4; i++) {
      strid = strid * 256 + m_Ptr[start_pos + i];
    }
  }
  return strid;
}
FX_STRSIZE FX_ftoa(FX_FLOAT d, FX_CHAR* buf) {
  buf[0] = '0';
  buf[1] = '\0';
  if (d == 0.0f) {
    return 1;
  }
  FX_BOOL bNegative = FALSE;
  if (d < 0) {
    bNegative = TRUE;
    d = -d;
  }
  int scale = 1;
  int scaled = FXSYS_round(d);
  while (scaled < 100000) {
    if (scale == 1000000) {
      break;
    }
    scale *= 10;
    scaled = FXSYS_round(d * scale);
  }
  if (scaled == 0) {
    return 1;
  }
  char buf2[32];
  int buf_size = 0;
  if (bNegative) {
    buf[buf_size++] = '-';
  }
  int i = scaled / scale;
  FXSYS_itoa(i, buf2, 10);
  FX_STRSIZE len = FXSYS_strlen(buf2);
  FXSYS_memcpy(buf + buf_size, buf2, len);
  buf_size += len;
  int fraction = scaled % scale;
  if (fraction == 0) {
    return buf_size;
  }
  buf[buf_size++] = '.';
  scale /= 10;
  while (fraction) {
    buf[buf_size++] = '0' + fraction / scale;
    fraction %= scale;
    scale /= 10;
  }
  return buf_size;
}
CFX_ByteString CFX_ByteString::FormatFloat(FX_FLOAT d, int precision) {
  FX_CHAR buf[32];
  FX_STRSIZE len = FX_ftoa(d, buf);
  return CFX_ByteString(buf, len);
}
