// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stddef.h>  // For offsetof().

#include <algorithm>
#include <cctype>

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_ext.h"
#include "third_party/base/numerics/safe_math.h"

// static
CFX_WideString::StringData* CFX_WideString::StringData::Create(int nLen) {
  // TODO(palmer): |nLen| should really be declared as |size_t|, or
  // at least unsigned.
  if (nLen == 0 || nLen < 0) {
    return NULL;
  }

  // Fixed portion of header plus a NUL wide char not in m_nAllocLength.
  int overhead = offsetof(StringData, m_String) + sizeof(FX_WCHAR);
  pdfium::base::CheckedNumeric<int> iSize = nLen;
  iSize *= sizeof(FX_WCHAR);
  iSize += overhead;

  // Now round to an 8-byte boundary. We'd expect that this is the minimum
  // granularity of any of the underlying allocators, so there may be cases
  // where we can save a re-alloc when adding a few characters to a string
  // by using this otherwise wasted space.
  iSize += 7;
  int totalSize = iSize.ValueOrDie() & ~7;
  int usableLen = (totalSize - overhead) / sizeof(FX_WCHAR);
  FXSYS_assert(usableLen >= nLen);

  void* pData = FX_Alloc(uint8_t, totalSize);
  return new (pData) StringData(nLen, usableLen);
}
CFX_WideString::~CFX_WideString() {
  if (m_pData) {
    m_pData->Release();
  }
}
CFX_WideString::CFX_WideString(const CFX_WideString& stringSrc) {
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
CFX_WideString::CFX_WideString(const FX_WCHAR* lpsz, FX_STRSIZE nLen) {
  if (nLen < 0) {
    nLen = lpsz ? FXSYS_wcslen(lpsz) : 0;
  }
  if (nLen) {
    m_pData = StringData::Create(nLen);
    if (m_pData) {
      FXSYS_memcpy(m_pData->m_String, lpsz, nLen * sizeof(FX_WCHAR));
    }
  } else {
    m_pData = NULL;
  }
}
CFX_WideString::CFX_WideString(FX_WCHAR ch) {
  m_pData = StringData::Create(1);
  if (m_pData) {
    m_pData->m_String[0] = ch;
  }
}
CFX_WideString::CFX_WideString(const CFX_WideStringC& str) {
  if (str.IsEmpty()) {
    m_pData = NULL;
    return;
  }
  m_pData = StringData::Create(str.GetLength());
  if (m_pData) {
    FXSYS_memcpy(m_pData->m_String, str.GetPtr(),
                 str.GetLength() * sizeof(FX_WCHAR));
  }
}
CFX_WideString::CFX_WideString(const CFX_WideStringC& str1,
                               const CFX_WideStringC& str2) {
  m_pData = NULL;
  int nNewLen = str1.GetLength() + str2.GetLength();
  if (nNewLen == 0) {
    return;
  }
  m_pData = StringData::Create(nNewLen);
  if (m_pData) {
    FXSYS_memcpy(m_pData->m_String, str1.GetPtr(),
                 str1.GetLength() * sizeof(FX_WCHAR));
    FXSYS_memcpy(m_pData->m_String + str1.GetLength(), str2.GetPtr(),
                 str2.GetLength() * sizeof(FX_WCHAR));
  }
}
void CFX_WideString::ReleaseBuffer(FX_STRSIZE nNewLength) {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (nNewLength == -1) {
    nNewLength = m_pData ? FXSYS_wcslen(m_pData->m_String) : 0;
  }
  if (nNewLength == 0) {
    Empty();
    return;
  }
  FXSYS_assert(nNewLength <= m_pData->m_nAllocLength);
  m_pData->m_nDataLength = nNewLength;
  m_pData->m_String[nNewLength] = 0;
}
const CFX_WideString& CFX_WideString::operator=(const FX_WCHAR* lpsz) {
  if (!lpsz || lpsz[0] == 0) {
    Empty();
  } else {
    AssignCopy(FXSYS_wcslen(lpsz), lpsz);
  }
  return *this;
}
const CFX_WideString& CFX_WideString::operator=(
    const CFX_WideStringC& stringSrc) {
  if (stringSrc.IsEmpty()) {
    Empty();
  } else {
    AssignCopy(stringSrc.GetLength(), stringSrc.GetPtr());
  }
  return *this;
}
const CFX_WideString& CFX_WideString::operator=(
    const CFX_WideString& stringSrc) {
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
const CFX_WideString& CFX_WideString::operator+=(FX_WCHAR ch) {
  ConcatInPlace(1, &ch);
  return *this;
}
const CFX_WideString& CFX_WideString::operator+=(const FX_WCHAR* lpsz) {
  if (lpsz) {
    ConcatInPlace(FXSYS_wcslen(lpsz), lpsz);
  }
  return *this;
}
const CFX_WideString& CFX_WideString::operator+=(const CFX_WideString& string) {
  if (!string.m_pData) {
    return *this;
  }
  ConcatInPlace(string.m_pData->m_nDataLength, string.m_pData->m_String);
  return *this;
}
const CFX_WideString& CFX_WideString::operator+=(
    const CFX_WideStringC& string) {
  if (string.IsEmpty()) {
    return *this;
  }
  ConcatInPlace(string.GetLength(), string.GetPtr());
  return *this;
}
bool CFX_WideString::Equal(const wchar_t* ptr) const {
  if (!m_pData) {
    return !ptr || ptr[0] == L'\0';
  }
  if (!ptr) {
    return m_pData->m_nDataLength == 0;
  }
  return wcslen(ptr) == m_pData->m_nDataLength &&
         wmemcmp(ptr, m_pData->m_String, m_pData->m_nDataLength) == 0;
}
bool CFX_WideString::Equal(const CFX_WideStringC& str) const {
  if (!m_pData) {
    return str.IsEmpty();
  }
  return str.GetLength() == m_pData->m_nDataLength &&
         wmemcmp(str.GetPtr(), m_pData->m_String, m_pData->m_nDataLength) == 0;
}
bool CFX_WideString::Equal(const CFX_WideString& other) const {
  if (IsEmpty()) {
    return other.IsEmpty();
  }
  if (other.IsEmpty()) {
    return false;
  }
  return other.m_pData->m_nDataLength == m_pData->m_nDataLength &&
         wmemcmp(other.m_pData->m_String, m_pData->m_String,
                 m_pData->m_nDataLength) == 0;
}
void CFX_WideString::Empty() {
  if (m_pData) {
    m_pData->Release();
    m_pData = NULL;
  }
}
void CFX_WideString::ConcatInPlace(FX_STRSIZE nSrcLen,
                                   const FX_WCHAR* lpszSrcData) {
  if (nSrcLen == 0 || !lpszSrcData) {
    return;
  }
  if (!m_pData) {
    m_pData = StringData::Create(nSrcLen);
    if (m_pData) {
      FXSYS_memcpy(m_pData->m_String, lpszSrcData, nSrcLen * sizeof(FX_WCHAR));
    }
    return;
  }
  if (m_pData->m_nRefs > 1 ||
      m_pData->m_nDataLength + nSrcLen > m_pData->m_nAllocLength) {
    ConcatCopy(m_pData->m_nDataLength, m_pData->m_String, nSrcLen, lpszSrcData);
  } else {
    FXSYS_memcpy(m_pData->m_String + m_pData->m_nDataLength, lpszSrcData,
                 nSrcLen * sizeof(FX_WCHAR));
    m_pData->m_nDataLength += nSrcLen;
    m_pData->m_String[m_pData->m_nDataLength] = 0;
  }
}
void CFX_WideString::ConcatCopy(FX_STRSIZE nSrc1Len,
                                const FX_WCHAR* lpszSrc1Data,
                                FX_STRSIZE nSrc2Len,
                                const FX_WCHAR* lpszSrc2Data) {
  FX_STRSIZE nNewLen = nSrc1Len + nSrc2Len;
  if (nNewLen <= 0) {
    return;
  }
  // Don't release until done copying, might be one of the arguments.
  StringData* pOldData = m_pData;
  m_pData = StringData::Create(nNewLen);
  if (m_pData) {
    wmemcpy(m_pData->m_String, lpszSrc1Data, nSrc1Len);
    wmemcpy(m_pData->m_String + nSrc1Len, lpszSrc2Data, nSrc2Len);
  }
  pOldData->Release();
}
void CFX_WideString::CopyBeforeWrite() {
  if (!m_pData || m_pData->m_nRefs <= 1) {
    return;
  }
  StringData* pData = m_pData;
  m_pData->Release();
  FX_STRSIZE nDataLength = pData->m_nDataLength;
  m_pData = StringData::Create(nDataLength);
  if (m_pData) {
    FXSYS_memcpy(m_pData->m_String, pData->m_String,
                 (nDataLength + 1) * sizeof(FX_WCHAR));
  }
}
void CFX_WideString::AllocBeforeWrite(FX_STRSIZE nLen) {
  if (m_pData && m_pData->m_nRefs <= 1 && m_pData->m_nAllocLength >= nLen) {
    return;
  }
  Empty();
  m_pData = StringData::Create(nLen);
}
void CFX_WideString::AssignCopy(FX_STRSIZE nSrcLen,
                                const FX_WCHAR* lpszSrcData) {
  AllocBeforeWrite(nSrcLen);
  FXSYS_memcpy(m_pData->m_String, lpszSrcData, nSrcLen * sizeof(FX_WCHAR));
  m_pData->m_nDataLength = nSrcLen;
  m_pData->m_String[nSrcLen] = 0;
}
int CFX_WideString::Compare(const FX_WCHAR* lpsz) const {
  if (m_pData)
    return FXSYS_wcscmp(m_pData->m_String, lpsz);
  return (!lpsz || lpsz[0] == 0) ? 0 : -1;
}
CFX_ByteString CFX_WideString::UTF8Encode() const {
  return FX_UTF8Encode(*this);
}
CFX_ByteString CFX_WideString::UTF16LE_Encode() const {
  if (!m_pData) {
    return CFX_ByteString("\0\0", 2);
  }
  int len = m_pData->m_nDataLength;
  CFX_ByteString result;
  FX_CHAR* buffer = result.GetBuffer(len * 2 + 2);
  for (int i = 0; i < len; i++) {
    buffer[i * 2] = m_pData->m_String[i] & 0xff;
    buffer[i * 2 + 1] = m_pData->m_String[i] >> 8;
  }
  buffer[len * 2] = 0;
  buffer[len * 2 + 1] = 0;
  result.ReleaseBuffer(len * 2 + 2);
  return result;
}
void CFX_WideString::ConvertFrom(const CFX_ByteString& str,
                                 CFX_CharMap* pCharMap) {
  if (!pCharMap) {
    pCharMap = CFX_CharMap::GetDefaultMapper();
  }
  *this = pCharMap->m_GetWideString(pCharMap, str);
}
void CFX_WideString::Reserve(FX_STRSIZE len) {
  GetBuffer(len);
  ReleaseBuffer(GetLength());
}
FX_WCHAR* CFX_WideString::GetBuffer(FX_STRSIZE nMinBufLength) {
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
  FXSYS_memcpy(m_pData->m_String, pOldData->m_String,
               (nOldLen + 1) * sizeof(FX_WCHAR));
  m_pData->m_nDataLength = nOldLen;
  pOldData->Release();
  return m_pData->m_String;
}
CFX_WideString CFX_WideString::FromLocal(const char* str, FX_STRSIZE len) {
  CFX_WideString result;
  result.ConvertFrom(CFX_ByteString(str, len));
  return result;
}
CFX_WideString CFX_WideString::FromUTF8(const char* str, FX_STRSIZE len) {
  if (!str || 0 == len) {
    return CFX_WideString();
  }

  CFX_UTF8Decoder decoder;
  for (FX_STRSIZE i = 0; i < len; i++) {
    decoder.Input(str[i]);
  }
  return decoder.GetResult();
}
CFX_WideString CFX_WideString::FromUTF16LE(const unsigned short* wstr,
                                           FX_STRSIZE wlen) {
  if (!wstr || 0 == wlen) {
    return CFX_WideString();
  }

  CFX_WideString result;
  FX_WCHAR* buf = result.GetBuffer(wlen);
  for (int i = 0; i < wlen; i++) {
    buf[i] = wstr[i];
  }
  result.ReleaseBuffer(wlen);
  return result;
}
FX_STRSIZE CFX_WideString::WStringLength(const unsigned short* str) {
  FX_STRSIZE len = 0;
  if (str)
    while (str[len])
      len++;
  return len;
}

void CFX_WideString::AllocCopy(CFX_WideString& dest,
                               FX_STRSIZE nCopyLen,
                               FX_STRSIZE nCopyIndex) const {
  // |FX_STRSIZE| is currently typedef'd as in |int|. TODO(palmer): It
  // should be a |size_t|, or at least unsigned.
  if (nCopyLen == 0 || nCopyLen < 0) {
    return;
  }
  pdfium::base::CheckedNumeric<FX_STRSIZE> iSize =
      static_cast<FX_STRSIZE>(sizeof(FX_WCHAR));
  iSize *= nCopyLen;
  ASSERT(!dest.m_pData);
  dest.m_pData = StringData::Create(nCopyLen);
  if (dest.m_pData) {
    FXSYS_memcpy(dest.m_pData->m_String, m_pData->m_String + nCopyIndex,
                 iSize.ValueOrDie());
  }
}
CFX_WideString CFX_WideString::Left(FX_STRSIZE nCount) const {
  if (!m_pData) {
    return CFX_WideString();
  }
  if (nCount < 0) {
    nCount = 0;
  }
  if (nCount >= m_pData->m_nDataLength) {
    return *this;
  }
  CFX_WideString dest;
  AllocCopy(dest, nCount, 0);
  return dest;
}
CFX_WideString CFX_WideString::Mid(FX_STRSIZE nFirst) const {
  return Mid(nFirst, m_pData->m_nDataLength - nFirst);
}
CFX_WideString CFX_WideString::Mid(FX_STRSIZE nFirst, FX_STRSIZE nCount) const {
  if (!m_pData) {
    return CFX_WideString();
  }
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
  CFX_WideString dest;
  AllocCopy(dest, nCount, nFirst);
  return dest;
}
CFX_WideString CFX_WideString::Right(FX_STRSIZE nCount) const {
  if (!m_pData) {
    return CFX_WideString();
  }
  if (nCount < 0) {
    nCount = 0;
  }
  if (nCount >= m_pData->m_nDataLength) {
    return *this;
  }
  CFX_WideString dest;
  AllocCopy(dest, nCount, m_pData->m_nDataLength - nCount);
  return dest;
}
int CFX_WideString::CompareNoCase(const FX_WCHAR* lpsz) const {
  if (!m_pData) {
    return (!lpsz || lpsz[0] == 0) ? 0 : -1;
  }
  return FXSYS_wcsicmp(m_pData->m_String, lpsz);
}
int CFX_WideString::Compare(const CFX_WideString& str) const {
  if (!m_pData) {
    if (!str.m_pData) {
      return 0;
    }
    return -1;
  }
  if (!str.m_pData) {
    return 1;
  }
  int this_len = m_pData->m_nDataLength;
  int that_len = str.m_pData->m_nDataLength;
  int min_len = this_len < that_len ? this_len : that_len;
  for (int i = 0; i < min_len; i++) {
    if (m_pData->m_String[i] < str.m_pData->m_String[i]) {
      return -1;
    }
    if (m_pData->m_String[i] > str.m_pData->m_String[i]) {
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
void CFX_WideString::SetAt(FX_STRSIZE nIndex, FX_WCHAR ch) {
  if (!m_pData) {
    return;
  }
  ASSERT(nIndex >= 0);
  ASSERT(nIndex < m_pData->m_nDataLength);
  CopyBeforeWrite();
  m_pData->m_String[nIndex] = ch;
}
void CFX_WideString::MakeLower() {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return;
  }
  FXSYS_wcslwr(m_pData->m_String);
}
void CFX_WideString::MakeUpper() {
  if (!m_pData) {
    return;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return;
  }
  FXSYS_wcsupr(m_pData->m_String);
}
FX_STRSIZE CFX_WideString::Find(const FX_WCHAR* lpszSub,
                                FX_STRSIZE nStart) const {
  FX_STRSIZE nLength = GetLength();
  if (nLength < 1 || nStart > nLength) {
    return -1;
  }
  const FX_WCHAR* lpsz = FXSYS_wcsstr(m_pData->m_String + nStart, lpszSub);
  return lpsz ? (int)(lpsz - m_pData->m_String) : -1;
}
FX_STRSIZE CFX_WideString::Find(FX_WCHAR ch, FX_STRSIZE nStart) const {
  if (!m_pData) {
    return -1;
  }
  FX_STRSIZE nLength = m_pData->m_nDataLength;
  if (nStart >= nLength) {
    return -1;
  }
  const FX_WCHAR* lpsz = FXSYS_wcschr(m_pData->m_String + nStart, ch);
  return (lpsz) ? (int)(lpsz - m_pData->m_String) : -1;
}
void CFX_WideString::TrimRight(const FX_WCHAR* lpszTargetList) {
  FXSYS_assert(lpszTargetList);
  if (!m_pData || *lpszTargetList == 0) {
    return;
  }
  CopyBeforeWrite();
  FX_STRSIZE len = GetLength();
  if (len < 1) {
    return;
  }
  FX_STRSIZE pos = len;
  while (pos) {
    if (!FXSYS_wcschr(lpszTargetList, m_pData->m_String[pos - 1])) {
      break;
    }
    pos--;
  }
  if (pos < len) {
    m_pData->m_String[pos] = 0;
    m_pData->m_nDataLength = pos;
  }
}
void CFX_WideString::TrimRight(FX_WCHAR chTarget) {
  FX_WCHAR str[2] = {chTarget, 0};
  TrimRight(str);
}
void CFX_WideString::TrimRight() {
  TrimRight(L"\x09\x0a\x0b\x0c\x0d\x20");
}
void CFX_WideString::TrimLeft(const FX_WCHAR* lpszTargets) {
  FXSYS_assert(lpszTargets);
  if (!m_pData || *lpszTargets == 0) {
    return;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return;
  }
  const FX_WCHAR* lpsz = m_pData->m_String;
  while (*lpsz != 0) {
    if (!FXSYS_wcschr(lpszTargets, *lpsz)) {
      break;
    }
    lpsz++;
  }
  if (lpsz != m_pData->m_String) {
    int nDataLength =
        m_pData->m_nDataLength - (FX_STRSIZE)(lpsz - m_pData->m_String);
    FXSYS_memmove(m_pData->m_String, lpsz,
                  (nDataLength + 1) * sizeof(FX_WCHAR));
    m_pData->m_nDataLength = nDataLength;
  }
}
void CFX_WideString::TrimLeft(FX_WCHAR chTarget) {
  FX_WCHAR str[2] = {chTarget, 0};
  TrimLeft(str);
}
void CFX_WideString::TrimLeft() {
  TrimLeft(L"\x09\x0a\x0b\x0c\x0d\x20");
}
FX_STRSIZE CFX_WideString::Replace(const FX_WCHAR* lpszOld,
                                   const FX_WCHAR* lpszNew) {
  if (GetLength() < 1) {
    return 0;
  }
  if (!lpszOld) {
    return 0;
  }
  FX_STRSIZE nSourceLen = FXSYS_wcslen(lpszOld);
  if (nSourceLen == 0) {
    return 0;
  }
  FX_STRSIZE nReplacementLen = lpszNew ? FXSYS_wcslen(lpszNew) : 0;
  FX_STRSIZE nCount = 0;
  FX_WCHAR* lpszStart = m_pData->m_String;
  FX_WCHAR* lpszEnd = m_pData->m_String + m_pData->m_nDataLength;
  FX_WCHAR* lpszTarget;
  {
    while ((lpszTarget = (FX_WCHAR*)FXSYS_wcsstr(lpszStart, lpszOld)) &&
           lpszStart < lpszEnd) {
      nCount++;
      lpszStart = lpszTarget + nSourceLen;
    }
  }
  if (nCount > 0) {
    CopyBeforeWrite();
    FX_STRSIZE nOldLength = m_pData->m_nDataLength;
    FX_STRSIZE nNewLength =
        nOldLength + (nReplacementLen - nSourceLen) * nCount;
    if (m_pData->m_nAllocLength < nNewLength || m_pData->m_nRefs > 1) {
      StringData* pOldData = m_pData;
      const FX_WCHAR* pstr = m_pData->m_String;
      m_pData = StringData::Create(nNewLength);
      if (!m_pData) {
        return 0;
      }
      FXSYS_memcpy(m_pData->m_String, pstr,
                   pOldData->m_nDataLength * sizeof(FX_WCHAR));
      pOldData->Release();
    }
    lpszStart = m_pData->m_String;
    lpszEnd = m_pData->m_String + std::max(m_pData->m_nDataLength, nNewLength);
    {
      while ((lpszTarget = (FX_WCHAR*)FXSYS_wcsstr(lpszStart, lpszOld)) !=
                 NULL &&
             lpszStart < lpszEnd) {
        FX_STRSIZE nBalance =
            nOldLength -
            (FX_STRSIZE)(lpszTarget - m_pData->m_String + nSourceLen);
        FXSYS_memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
                      nBalance * sizeof(FX_WCHAR));
        FXSYS_memcpy(lpszTarget, lpszNew, nReplacementLen * sizeof(FX_WCHAR));
        lpszStart = lpszTarget + nReplacementLen;
        lpszStart[nBalance] = 0;
        nOldLength += (nReplacementLen - nSourceLen);
      }
    }
    ASSERT(m_pData->m_String[nNewLength] == 0);
    m_pData->m_nDataLength = nNewLength;
  }
  return nCount;
}
FX_STRSIZE CFX_WideString::Insert(FX_STRSIZE nIndex, FX_WCHAR ch) {
  CopyBeforeWrite();
  if (nIndex < 0) {
    nIndex = 0;
  }
  FX_STRSIZE nNewLength = GetLength();
  if (nIndex > nNewLength) {
    nIndex = nNewLength;
  }
  nNewLength++;
  if (!m_pData || m_pData->m_nAllocLength < nNewLength) {
    StringData* pOldData = m_pData;
    const FX_WCHAR* pstr = m_pData->m_String;
    m_pData = StringData::Create(nNewLength);
    if (!m_pData) {
      return 0;
    }
    if (pOldData) {
      FXSYS_memmove(m_pData->m_String, pstr,
                    (pOldData->m_nDataLength + 1) * sizeof(FX_WCHAR));
      pOldData->Release();
    } else {
      m_pData->m_String[0] = 0;
    }
  }
  FXSYS_memmove(m_pData->m_String + nIndex + 1, m_pData->m_String + nIndex,
                (nNewLength - nIndex) * sizeof(FX_WCHAR));
  m_pData->m_String[nIndex] = ch;
  m_pData->m_nDataLength = nNewLength;
  return nNewLength;
}
FX_STRSIZE CFX_WideString::Delete(FX_STRSIZE nIndex, FX_STRSIZE nCount) {
  if (GetLength() < 1) {
    return 0;
  }
  if (nIndex < 0) {
    nIndex = 0;
  }
  FX_STRSIZE nOldLength = m_pData->m_nDataLength;
  if (nCount > 0 && nIndex < nOldLength) {
    CopyBeforeWrite();
    int nBytesToCopy = nOldLength - (nIndex + nCount) + 1;
    FXSYS_memmove(m_pData->m_String + nIndex,
                  m_pData->m_String + nIndex + nCount,
                  nBytesToCopy * sizeof(FX_WCHAR));
    m_pData->m_nDataLength = nOldLength - nCount;
  }
  return m_pData->m_nDataLength;
}
FX_STRSIZE CFX_WideString::Remove(FX_WCHAR chRemove) {
  if (!m_pData) {
    return 0;
  }
  CopyBeforeWrite();
  if (GetLength() < 1) {
    return 0;
  }
  FX_WCHAR* pstrSource = m_pData->m_String;
  FX_WCHAR* pstrDest = m_pData->m_String;
  FX_WCHAR* pstrEnd = m_pData->m_String + m_pData->m_nDataLength;
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
#define FORCE_ANSI 0x10000
#define FORCE_UNICODE 0x20000
#define FORCE_INT64 0x40000
void CFX_WideString::FormatV(const FX_WCHAR* lpszFormat, va_list argList) {
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
  for (const FX_WCHAR* lpsz = lpszFormat; *lpsz != 0; lpsz++) {
    if (*lpsz != '%' || *(lpsz = lpsz + 1) == '%') {
      nMaxLen += FXSYS_wcslen(lpsz);
      continue;
    }
    int nItemLen = 0;
    int nWidth = 0;
    for (; *lpsz != 0; lpsz++) {
      if (*lpsz == '#') {
        nMaxLen += 2;
      } else if (*lpsz == '*') {
        nWidth = va_arg(argList, int);
      } else if (*lpsz != '-' && *lpsz != '+' && *lpsz != '0' && *lpsz != ' ') {
        break;
      }
    }
    if (nWidth == 0) {
      nWidth = FXSYS_wtoi(lpsz);
      while (std::iswdigit(*lpsz))
        ++lpsz;
    }
    if (nWidth < 0 || nWidth > 128 * 1024) {
      lpszFormat = L"Bad width";
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
        nPrecision = FXSYS_wtoi(lpsz);
        while (std::iswdigit(*lpsz))
          ++lpsz;
      }
    }
    if (nPrecision < 0 || nPrecision > 128 * 1024) {
      lpszFormat = L"Bad precision";
      nMaxLen = 14;
      break;
    }
    int nModifier = 0;
    if (*lpsz == L'I' && *(lpsz + 1) == L'6' && *(lpsz + 2) == L'4') {
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
        const FX_WCHAR* pstrNextArg = va_arg(argList, const FX_WCHAR*);
        if (pstrNextArg) {
          nItemLen = FXSYS_wcslen(pstrNextArg);
          if (nItemLen < 1) {
            nItemLen = 1;
          }
        } else {
          nItemLen = 6;
        }
      } break;
      case 'S': {
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
            double f;
            char pszTemp[256];
            f = va_arg(argList, double);
            FXSYS_snprintf(pszTemp, sizeof(pszTemp), "%*.*f", nWidth,
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
  GetBuffer(nMaxLen);
  if (m_pData) {
    FXSYS_vswprintf((wchar_t*)m_pData->m_String, nMaxLen + 1,
                    (const wchar_t*)lpszFormat, argListSave);
    ReleaseBuffer();
  }
  va_end(argListSave);
}
void CFX_WideString::Format(const FX_WCHAR* lpszFormat, ...) {
  va_list argList;
  va_start(argList, lpszFormat);
  FormatV(lpszFormat, argList);
  va_end(argList);
}
FX_FLOAT FX_wtof(const FX_WCHAR* str, int len) {
  if (len == 0) {
    return 0.0;
  }
  int cc = 0;
  FX_BOOL bNegative = FALSE;
  if (str[0] == '+') {
    cc++;
  } else if (str[0] == '-') {
    bNegative = TRUE;
    cc++;
  }
  int integer = 0;
  while (cc < len) {
    if (str[cc] == '.') {
      break;
    }
    integer = integer * 10 + FXSYS_toDecimalDigitWide(str[cc]);
    cc++;
  }
  FX_FLOAT fraction = 0;
  if (str[cc] == '.') {
    cc++;
    FX_FLOAT scale = 0.1f;
    while (cc < len) {
      fraction += scale * FXSYS_toDecimalDigitWide(str[cc]);
      scale *= 0.1f;
      cc++;
    }
  }
  fraction += (FX_FLOAT)integer;
  return bNegative ? -fraction : fraction;
}
int CFX_WideString::GetInteger() const {
  return m_pData ? FXSYS_wtoi(m_pData->m_String) : 0;
}
FX_FLOAT CFX_WideString::GetFloat() const {
  return m_pData ? FX_wtof(m_pData->m_String, m_pData->m_nDataLength) : 0.0f;
}
static CFX_ByteString _DefMap_GetByteString(CFX_CharMap* pCharMap,
                                            const CFX_WideString& widestr) {
  int src_len = widestr.GetLength();
  int codepage = pCharMap->m_GetCodePage ? pCharMap->m_GetCodePage() : 0;
  int dest_len = FXSYS_WideCharToMultiByte(codepage, 0, widestr.c_str(),
                                           src_len, NULL, 0, NULL, NULL);
  if (dest_len == 0) {
    return CFX_ByteString();
  }
  CFX_ByteString bytestr;
  FX_CHAR* dest_buf = bytestr.GetBuffer(dest_len);
  FXSYS_WideCharToMultiByte(codepage, 0, widestr.c_str(), src_len, dest_buf,
                            dest_len, NULL, NULL);
  bytestr.ReleaseBuffer(dest_len);
  return bytestr;
}
static CFX_WideString _DefMap_GetWideString(CFX_CharMap* pCharMap,
                                            const CFX_ByteString& bytestr) {
  int src_len = bytestr.GetLength();
  int codepage = pCharMap->m_GetCodePage ? pCharMap->m_GetCodePage() : 0;
  int dest_len =
      FXSYS_MultiByteToWideChar(codepage, 0, bytestr, src_len, NULL, 0);
  if (dest_len == 0) {
    return CFX_WideString();
  }
  CFX_WideString widestr;
  FX_WCHAR* dest_buf = widestr.GetBuffer(dest_len);
  FXSYS_MultiByteToWideChar(codepage, 0, bytestr, src_len, dest_buf, dest_len);
  widestr.ReleaseBuffer(dest_len);
  return widestr;
}
static int _DefMap_GetGBKCodePage() {
  return 936;
}
static int _DefMap_GetUHCCodePage() {
  return 949;
}
static int _DefMap_GetJISCodePage() {
  return 932;
}
static int _DefMap_GetBig5CodePage() {
  return 950;
}
static const CFX_CharMap g_DefaultMapper = {&_DefMap_GetWideString,
                                            &_DefMap_GetByteString, NULL};
static const CFX_CharMap g_DefaultGBKMapper = {
    &_DefMap_GetWideString, &_DefMap_GetByteString, &_DefMap_GetGBKCodePage};
static const CFX_CharMap g_DefaultJISMapper = {
    &_DefMap_GetWideString, &_DefMap_GetByteString, &_DefMap_GetJISCodePage};
static const CFX_CharMap g_DefaultUHCMapper = {
    &_DefMap_GetWideString, &_DefMap_GetByteString, &_DefMap_GetUHCCodePage};
static const CFX_CharMap g_DefaultBig5Mapper = {
    &_DefMap_GetWideString, &_DefMap_GetByteString, &_DefMap_GetBig5CodePage};
CFX_CharMap* CFX_CharMap::GetDefaultMapper(int32_t codepage) {
  switch (codepage) {
    case 0:
      return (CFX_CharMap*)&g_DefaultMapper;
    case 932:
      return (CFX_CharMap*)&g_DefaultJISMapper;
    case 936:
      return (CFX_CharMap*)&g_DefaultGBKMapper;
    case 949:
      return (CFX_CharMap*)&g_DefaultUHCMapper;
    case 950:
      return (CFX_CharMap*)&g_DefaultBig5Mapper;
  }
  return NULL;
}
