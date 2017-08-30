// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_bytestring.h"

#include <stddef.h>

#include <algorithm>
#include <cctype>
#include <string>

#include "core/fxcrt/cfx_string_pool_template.h"
#include "core/fxcrt/cfx_utf8decoder.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/stl_util.h"

template class CFX_StringDataTemplate<char>;
template class CFX_StringCTemplate<char>;
template class CFX_StringPoolTemplate<CFX_ByteString>;
template struct std::hash<CFX_ByteString>;

namespace {

int Buffer_itoa(char* buf, int i, uint32_t flags) {
  if (i == 0) {
    buf[0] = '0';
    return 1;
  }
  char buf1[32];
  int buf_pos = 31;
  uint32_t u = i;
  if ((flags & FXFORMAT_SIGNED) && i < 0) {
    u = -i;
  }
  int base = 10;
  const char* str = "0123456789abcdef";
  if (flags & FXFORMAT_HEX) {
    base = 16;
    if (flags & FXFORMAT_CAPITAL) {
      str = "0123456789ABCDEF";
    }
  }
  while (u != 0) {
    buf1[buf_pos--] = str[u % base];
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

const char* FX_strstr(const char* haystack,
                      int haystack_len,
                      const char* needle,
                      int needle_len) {
  if (needle_len > haystack_len || needle_len == 0) {
    return nullptr;
  }
  const char* end_ptr = haystack + haystack_len - needle_len;
  while (haystack <= end_ptr) {
    int i = 0;
    while (1) {
      if (haystack[i] != needle[i]) {
        break;
      }
      i++;
      if (i == needle_len) {
        return haystack;
      }
    }
    haystack++;
  }
  return nullptr;
}

#ifndef NDEBUG
bool IsValidCodePage(uint16_t codepage) {
  switch (codepage) {
    case FX_CODEPAGE_DefANSI:
    case FX_CODEPAGE_ShiftJIS:
    case FX_CODEPAGE_ChineseSimplified:
    case FX_CODEPAGE_Hangul:
    case FX_CODEPAGE_ChineseTraditional:
      return true;
    default:
      return false;
  }
}
#endif

CFX_ByteString GetByteString(uint16_t codepage, const CFX_WideStringC& wstr) {
  ASSERT(IsValidCodePage(codepage));

  int src_len = wstr.GetLength();
  int dest_len =
      FXSYS_WideCharToMultiByte(codepage, 0, wstr.unterminated_c_str(), src_len,
                                nullptr, 0, nullptr, nullptr);
  if (!dest_len)
    return CFX_ByteString();

  CFX_ByteString bstr;
  char* dest_buf = bstr.GetBuffer(dest_len);
  FXSYS_WideCharToMultiByte(codepage, 0, wstr.unterminated_c_str(), src_len,
                            dest_buf, dest_len, nullptr, nullptr);
  bstr.ReleaseBuffer(dest_len);
  return bstr;
}

}  // namespace

static_assert(sizeof(CFX_ByteString) <= sizeof(char*),
              "Strings must not require more space than pointers");

CFX_ByteString::CFX_ByteString(const char* pStr, FX_STRSIZE nLen) {
  ASSERT(nLen >= 0);
  if (nLen < 0)
    nLen = pStr ? FXSYS_strlen(pStr) : 0;

  if (nLen)
    m_pData.Reset(StringData::Create(pStr, nLen));
}

CFX_ByteString::CFX_ByteString(const uint8_t* pStr, FX_STRSIZE nLen) {
  ASSERT(nLen >= 0);
  if (nLen > 0) {
    m_pData.Reset(
        StringData::Create(reinterpret_cast<const char*>(pStr), nLen));
  }
}

CFX_ByteString::CFX_ByteString() {}

CFX_ByteString::CFX_ByteString(const CFX_ByteString& other)
    : m_pData(other.m_pData) {}

CFX_ByteString::CFX_ByteString(CFX_ByteString&& other) noexcept {
  m_pData.Swap(other.m_pData);
}

CFX_ByteString::CFX_ByteString(char ch) {
  m_pData.Reset(StringData::Create(1));
  m_pData->m_String[0] = ch;
}

CFX_ByteString::CFX_ByteString(const char* ptr)
    : CFX_ByteString(ptr, ptr ? FXSYS_strlen(ptr) : 0) {}

CFX_ByteString::CFX_ByteString(const CFX_ByteStringC& stringSrc) {
  if (!stringSrc.IsEmpty())
    m_pData.Reset(StringData::Create(stringSrc.unterminated_c_str(),
                                     stringSrc.GetLength()));
}

CFX_ByteString::CFX_ByteString(const CFX_ByteStringC& str1,
                               const CFX_ByteStringC& str2) {
  FX_SAFE_STRSIZE nSafeLen = str1.GetLength();
  nSafeLen += str2.GetLength();

  FX_STRSIZE nNewLen = nSafeLen.ValueOrDie();
  if (nNewLen == 0)
    return;

  m_pData.Reset(StringData::Create(nNewLen));
  m_pData->CopyContents(str1.unterminated_c_str(), str1.GetLength());
  m_pData->CopyContentsAt(str1.GetLength(), str2.unterminated_c_str(),
                          str2.GetLength());
}

CFX_ByteString::CFX_ByteString(
    const std::initializer_list<CFX_ByteStringC>& list) {
  FX_SAFE_STRSIZE nSafeLen = 0;
  for (const auto& item : list)
    nSafeLen += item.GetLength();

  FX_STRSIZE nNewLen = nSafeLen.ValueOrDie();
  if (nNewLen == 0)
    return;

  m_pData.Reset(StringData::Create(nNewLen));

  FX_STRSIZE nOffset = 0;
  for (const auto& item : list) {
    m_pData->CopyContentsAt(nOffset, item.unterminated_c_str(),
                            item.GetLength());
    nOffset += item.GetLength();
  }
}

CFX_ByteString::CFX_ByteString(const std::ostringstream& outStream) {
  std::string str = outStream.str();
  if (str.length() > 0)
    m_pData.Reset(StringData::Create(str.c_str(), str.length()));
}

CFX_ByteString::~CFX_ByteString() {}

const CFX_ByteString& CFX_ByteString::operator=(const char* pStr) {
  if (!pStr || !pStr[0])
    clear();
  else
    AssignCopy(pStr, FXSYS_strlen(pStr));

  return *this;
}

const CFX_ByteString& CFX_ByteString::operator=(
    const CFX_ByteStringC& stringSrc) {
  if (stringSrc.IsEmpty())
    clear();
  else
    AssignCopy(stringSrc.unterminated_c_str(), stringSrc.GetLength());

  return *this;
}

const CFX_ByteString& CFX_ByteString::operator=(
    const CFX_ByteString& stringSrc) {
  if (m_pData != stringSrc.m_pData)
    m_pData = stringSrc.m_pData;

  return *this;
}

const CFX_ByteString& CFX_ByteString::operator+=(const char* pStr) {
  if (pStr)
    Concat(pStr, FXSYS_strlen(pStr));

  return *this;
}

const CFX_ByteString& CFX_ByteString::operator+=(char ch) {
  Concat(&ch, 1);
  return *this;
}

const CFX_ByteString& CFX_ByteString::operator+=(const CFX_ByteString& str) {
  if (str.m_pData)
    Concat(str.m_pData->m_String, str.m_pData->m_nDataLength);

  return *this;
}

const CFX_ByteString& CFX_ByteString::operator+=(const CFX_ByteStringC& str) {
  if (!str.IsEmpty())
    Concat(str.unterminated_c_str(), str.GetLength());

  return *this;
}

bool CFX_ByteString::operator==(const char* ptr) const {
  if (!m_pData)
    return !ptr || !ptr[0];

  if (!ptr)
    return m_pData->m_nDataLength == 0;

  return FXSYS_strlen(ptr) == m_pData->m_nDataLength &&
         memcmp(ptr, m_pData->m_String, m_pData->m_nDataLength) == 0;
}

bool CFX_ByteString::operator==(const CFX_ByteStringC& str) const {
  if (!m_pData)
    return str.IsEmpty();

  return m_pData->m_nDataLength == str.GetLength() &&
         memcmp(m_pData->m_String, str.unterminated_c_str(), str.GetLength()) ==
             0;
}

bool CFX_ByteString::operator==(const CFX_ByteString& other) const {
  if (m_pData == other.m_pData)
    return true;

  if (IsEmpty())
    return other.IsEmpty();

  if (other.IsEmpty())
    return false;

  return other.m_pData->m_nDataLength == m_pData->m_nDataLength &&
         memcmp(other.m_pData->m_String, m_pData->m_String,
                m_pData->m_nDataLength) == 0;
}

bool CFX_ByteString::operator<(const CFX_ByteString& str) const {
  if (m_pData == str.m_pData)
    return false;

  int result =
      memcmp(c_str(), str.c_str(), std::min(GetLength(), str.GetLength()));
  return result < 0 || (result == 0 && GetLength() < str.GetLength());
}

bool CFX_ByteString::EqualNoCase(const CFX_ByteStringC& str) const {
  if (!m_pData)
    return str.IsEmpty();

  FX_STRSIZE len = str.GetLength();
  if (m_pData->m_nDataLength != len)
    return false;

  const uint8_t* pThis = (const uint8_t*)m_pData->m_String;
  const uint8_t* pThat = str.raw_str();
  for (FX_STRSIZE i = 0; i < len; i++) {
    if ((*pThis) != (*pThat)) {
      uint8_t bThis = FXSYS_tolower(*pThis);
      uint8_t bThat = FXSYS_tolower(*pThat);
      if (bThis != bThat)
        return false;
    }
    pThis++;
    pThat++;
  }
  return true;
}

void CFX_ByteString::AssignCopy(const char* pSrcData, FX_STRSIZE nSrcLen) {
  AllocBeforeWrite(nSrcLen);
  m_pData->CopyContents(pSrcData, nSrcLen);
  m_pData->m_nDataLength = nSrcLen;
}

void CFX_ByteString::ReallocBeforeWrite(FX_STRSIZE nNewLength) {
  if (m_pData && m_pData->CanOperateInPlace(nNewLength))
    return;

  if (nNewLength <= 0) {
    clear();
    return;
  }

  CFX_RetainPtr<StringData> pNewData(StringData::Create(nNewLength));
  if (m_pData) {
    FX_STRSIZE nCopyLength = std::min(m_pData->m_nDataLength, nNewLength);
    pNewData->CopyContents(m_pData->m_String, nCopyLength);
    pNewData->m_nDataLength = nCopyLength;
  } else {
    pNewData->m_nDataLength = 0;
  }
  pNewData->m_String[pNewData->m_nDataLength] = 0;
  m_pData.Swap(pNewData);
}

void CFX_ByteString::AllocBeforeWrite(FX_STRSIZE nNewLength) {
  if (m_pData && m_pData->CanOperateInPlace(nNewLength))
    return;

  if (nNewLength <= 0) {
    clear();
    return;
  }

  m_pData.Reset(StringData::Create(nNewLength));
}

void CFX_ByteString::ReleaseBuffer(FX_STRSIZE nNewLength) {
  ASSERT(nNewLength >= 0);
  if (!m_pData)
    return;

  nNewLength = std::min(nNewLength, m_pData->m_nAllocLength);
  if (nNewLength == 0) {
    clear();
    return;
  }

  ASSERT(m_pData->m_nRefs == 1);
  m_pData->m_nDataLength = nNewLength;
  m_pData->m_String[nNewLength] = 0;
  if (m_pData->m_nAllocLength - nNewLength >= 32) {
    // Over arbitrary threshold, so pay the price to relocate.  Force copy to
    // always occur by holding a second reference to the string.
    CFX_ByteString preserve(*this);
    ReallocBeforeWrite(nNewLength);
  }
}

void CFX_ByteString::Reserve(FX_STRSIZE len) {
  GetBuffer(len);
}

char* CFX_ByteString::GetBuffer(FX_STRSIZE nMinBufLength) {
  if (!m_pData) {
    if (nMinBufLength == 0)
      return nullptr;

    m_pData.Reset(StringData::Create(nMinBufLength));
    m_pData->m_nDataLength = 0;
    m_pData->m_String[0] = 0;
    return m_pData->m_String;
  }

  if (m_pData->CanOperateInPlace(nMinBufLength))
    return m_pData->m_String;

  nMinBufLength = std::max(nMinBufLength, m_pData->m_nDataLength);
  if (nMinBufLength == 0)
    return nullptr;

  CFX_RetainPtr<StringData> pNewData(StringData::Create(nMinBufLength));
  pNewData->CopyContents(*m_pData);
  pNewData->m_nDataLength = m_pData->m_nDataLength;
  m_pData.Swap(pNewData);
  return m_pData->m_String;
}

FX_STRSIZE CFX_ByteString::Delete(FX_STRSIZE index, FX_STRSIZE count) {
  if (!m_pData)
    return 0;

  FX_STRSIZE old_length = m_pData->m_nDataLength;
  if (count <= 0 || index != pdfium::clamp(index, 0, old_length))
    return old_length;

  FX_STRSIZE removal_length = index + count;
  if (removal_length > old_length)
    return old_length;

  ReallocBeforeWrite(old_length);
  FX_STRSIZE chars_to_copy = old_length - removal_length + 1;
  memmove(m_pData->m_String + index, m_pData->m_String + removal_length,
          chars_to_copy);
  m_pData->m_nDataLength = old_length - count;
  return m_pData->m_nDataLength;
}

void CFX_ByteString::Concat(const char* pSrcData, FX_STRSIZE nSrcLen) {
  if (!pSrcData || nSrcLen <= 0)
    return;

  if (!m_pData) {
    m_pData.Reset(StringData::Create(pSrcData, nSrcLen));
    return;
  }

  if (m_pData->CanOperateInPlace(m_pData->m_nDataLength + nSrcLen)) {
    m_pData->CopyContentsAt(m_pData->m_nDataLength, pSrcData, nSrcLen);
    m_pData->m_nDataLength += nSrcLen;
    return;
  }

  CFX_RetainPtr<StringData> pNewData(
      StringData::Create(m_pData->m_nDataLength + nSrcLen));
  pNewData->CopyContents(*m_pData);
  pNewData->CopyContentsAt(m_pData->m_nDataLength, pSrcData, nSrcLen);
  m_pData.Swap(pNewData);
}

CFX_ByteString CFX_ByteString::Mid(FX_STRSIZE first, FX_STRSIZE count) const {
  if (!m_pData)
    return CFX_ByteString();

  if (!IsValidIndex(first))
    return CFX_ByteString();

  if (count == 0 || !IsValidLength(count))
    return CFX_ByteString();

  if (!IsValidIndex(first + count - 1))
    return CFX_ByteString();

  if (first == 0 && count == m_pData->m_nDataLength)
    return *this;

  CFX_ByteString dest;
  AllocCopy(dest, count, first);
  return dest;
}

CFX_ByteString CFX_ByteString::Left(FX_STRSIZE count) const {
  if (count == 0 || !IsValidLength(count))
    return CFX_ByteString();
  return Mid(0, count);
}

CFX_ByteString CFX_ByteString::Right(FX_STRSIZE count) const {
  if (count == 0 || !IsValidLength(count))
    return CFX_ByteString();
  return Mid(GetLength() - count, count);
}

void CFX_ByteString::AllocCopy(CFX_ByteString& dest,
                               FX_STRSIZE nCopyLen,
                               FX_STRSIZE nCopyIndex) const {
  if (nCopyLen <= 0)
    return;

  CFX_RetainPtr<StringData> pNewData(
      StringData::Create(m_pData->m_String + nCopyIndex, nCopyLen));
  dest.m_pData.Swap(pNewData);
}

#define FORCE_ANSI 0x10000
#define FORCE_UNICODE 0x20000
#define FORCE_INT64 0x40000

CFX_ByteString CFX_ByteString::FormatInteger(int i, uint32_t flags) {
  char buf[32];
  return CFX_ByteString(buf, Buffer_itoa(buf, i, flags));
}

void CFX_ByteString::FormatV(const char* pFormat, va_list argList) {
  va_list argListSave;
  FX_VA_COPY(argListSave, argList);
  FX_STRSIZE nMaxLen = vsnprintf(nullptr, 0, pFormat, argList);
  if (nMaxLen > 0) {
    GetBuffer(nMaxLen);
    if (m_pData) {
      // In the following two calls, there's always space in the buffer for
      // a terminating NUL that's not included in nMaxLen.
      memset(m_pData->m_String, 0, nMaxLen + 1);
      vsnprintf(m_pData->m_String, nMaxLen + 1, pFormat, argListSave);
      ReleaseBuffer(GetStringLength());
    }
  }
  va_end(argListSave);
}

void CFX_ByteString::Format(const char* pFormat, ...) {
  va_list argList;
  va_start(argList, pFormat);
  FormatV(pFormat, argList);
  va_end(argList);
}

void CFX_ByteString::SetAt(FX_STRSIZE index, char c) {
  ASSERT(IsValidIndex(index));
  ReallocBeforeWrite(m_pData->m_nDataLength);
  m_pData->m_String[index] = c;
}

FX_STRSIZE CFX_ByteString::Insert(FX_STRSIZE location, char ch) {
  const FX_STRSIZE cur_length = m_pData ? m_pData->m_nDataLength : 0;
  if (!IsValidLength(location))
    return cur_length;

  const FX_STRSIZE new_length = cur_length + 1;
  ReallocBeforeWrite(new_length);
  memmove(m_pData->m_String + location + 1, m_pData->m_String + location,
          new_length - location);
  m_pData->m_String[location] = ch;
  m_pData->m_nDataLength = new_length;
  return new_length;
}

pdfium::Optional<FX_STRSIZE> CFX_ByteString::Find(char ch,
                                                  FX_STRSIZE start) const {
  if (!m_pData)
    return pdfium::Optional<FX_STRSIZE>();

  if (!IsValidIndex(start))
    return pdfium::Optional<FX_STRSIZE>();

  const char* pStr = static_cast<const char*>(
      memchr(m_pData->m_String + start, ch, m_pData->m_nDataLength - start));
  return pStr ? pdfium::Optional<FX_STRSIZE>(
                    static_cast<FX_STRSIZE>(pStr - m_pData->m_String))
              : pdfium::Optional<FX_STRSIZE>();
}

pdfium::Optional<FX_STRSIZE> CFX_ByteString::Find(const CFX_ByteStringC& subStr,
                                                  FX_STRSIZE start) const {
  if (!m_pData)
    return pdfium::Optional<FX_STRSIZE>();

  if (!IsValidIndex(start))
    return pdfium::Optional<FX_STRSIZE>();

  const char* pStr =
      FX_strstr(m_pData->m_String + start, m_pData->m_nDataLength - start,
                subStr.unterminated_c_str(), subStr.GetLength());
  return pStr ? pdfium::Optional<FX_STRSIZE>(
                    static_cast<FX_STRSIZE>(pStr - m_pData->m_String))
              : pdfium::Optional<FX_STRSIZE>();
}

pdfium::Optional<FX_STRSIZE> CFX_ByteString::ReverseFind(char ch) const {
  if (!m_pData)
    return pdfium::Optional<FX_STRSIZE>();

  FX_STRSIZE nLength = m_pData->m_nDataLength;
  while (nLength--) {
    if (m_pData->m_String[nLength] == ch)
      return pdfium::Optional<FX_STRSIZE>(nLength);
  }
  return pdfium::Optional<FX_STRSIZE>();
}

void CFX_ByteString::MakeLower() {
  if (!m_pData)
    return;

  ReallocBeforeWrite(m_pData->m_nDataLength);
  FXSYS_strlwr(m_pData->m_String);
}

void CFX_ByteString::MakeUpper() {
  if (!m_pData)
    return;

  ReallocBeforeWrite(m_pData->m_nDataLength);
  FXSYS_strupr(m_pData->m_String);
}

FX_STRSIZE CFX_ByteString::Remove(char chRemove) {
  if (!m_pData || m_pData->m_nDataLength < 1)
    return 0;

  char* pstrSource = m_pData->m_String;
  char* pstrEnd = m_pData->m_String + m_pData->m_nDataLength;
  while (pstrSource < pstrEnd) {
    if (*pstrSource == chRemove)
      break;
    pstrSource++;
  }
  if (pstrSource == pstrEnd)
    return 0;

  ptrdiff_t copied = pstrSource - m_pData->m_String;
  ReallocBeforeWrite(m_pData->m_nDataLength);
  pstrSource = m_pData->m_String + copied;
  pstrEnd = m_pData->m_String + m_pData->m_nDataLength;

  char* pstrDest = pstrSource;
  while (pstrSource < pstrEnd) {
    if (*pstrSource != chRemove) {
      *pstrDest = *pstrSource;
      pstrDest++;
    }
    pstrSource++;
  }

  *pstrDest = 0;
  FX_STRSIZE nCount = static_cast<FX_STRSIZE>(pstrSource - pstrDest);
  m_pData->m_nDataLength -= nCount;
  return nCount;
}

FX_STRSIZE CFX_ByteString::Replace(const CFX_ByteStringC& pOld,
                                   const CFX_ByteStringC& pNew) {
  if (!m_pData || pOld.IsEmpty())
    return 0;

  FX_STRSIZE nSourceLen = pOld.GetLength();
  FX_STRSIZE nReplacementLen = pNew.GetLength();
  FX_STRSIZE nCount = 0;
  const char* pStart = m_pData->m_String;
  char* pEnd = m_pData->m_String + m_pData->m_nDataLength;
  while (1) {
    const char* pTarget = FX_strstr(pStart, static_cast<int>(pEnd - pStart),
                                    pOld.unterminated_c_str(), nSourceLen);
    if (!pTarget)
      break;

    nCount++;
    pStart = pTarget + nSourceLen;
  }
  if (nCount == 0)
    return 0;

  FX_STRSIZE nNewLength =
      m_pData->m_nDataLength + (nReplacementLen - nSourceLen) * nCount;

  if (nNewLength == 0) {
    clear();
    return nCount;
  }

  CFX_RetainPtr<StringData> pNewData(StringData::Create(nNewLength));
  pStart = m_pData->m_String;
  char* pDest = pNewData->m_String;
  for (FX_STRSIZE i = 0; i < nCount; i++) {
    const char* pTarget = FX_strstr(pStart, static_cast<int>(pEnd - pStart),
                                    pOld.unterminated_c_str(), nSourceLen);
    memcpy(pDest, pStart, pTarget - pStart);
    pDest += pTarget - pStart;
    memcpy(pDest, pNew.unterminated_c_str(), pNew.GetLength());
    pDest += pNew.GetLength();
    pStart = pTarget + nSourceLen;
  }
  memcpy(pDest, pStart, pEnd - pStart);
  m_pData.Swap(pNewData);
  return nCount;
}

CFX_WideString CFX_ByteString::UTF8Decode() const {
  CFX_UTF8Decoder decoder;
  for (FX_STRSIZE i = 0; i < GetLength(); i++) {
    decoder.Input(static_cast<uint8_t>(m_pData->m_String[i]));
  }
  return CFX_WideString(decoder.GetResult());
}

// static
CFX_ByteString CFX_ByteString::FromUnicode(const CFX_WideString& str) {
  return GetByteString(0, str.AsStringC());
}

int CFX_ByteString::Compare(const CFX_ByteStringC& str) const {
  if (!m_pData) {
    return str.IsEmpty() ? 0 : -1;
  }
  FX_STRSIZE this_len = m_pData->m_nDataLength;
  FX_STRSIZE that_len = str.GetLength();
  FX_STRSIZE min_len = std::min(this_len, that_len);
  for (FX_STRSIZE i = 0; i < min_len; i++) {
    if (static_cast<uint8_t>(m_pData->m_String[i]) < str[i]) {
      return -1;
    }
    if (static_cast<uint8_t>(m_pData->m_String[i]) > str[i]) {
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

void CFX_ByteString::TrimRight(const CFX_ByteStringC& pTargets) {
  if (!m_pData || pTargets.IsEmpty()) {
    return;
  }
  FX_STRSIZE pos = GetLength();
  if (pos < 1) {
    return;
  }
  while (pos) {
    FX_STRSIZE i = 0;
    while (i < pTargets.GetLength() &&
           pTargets[i] != m_pData->m_String[pos - 1]) {
      i++;
    }
    if (i == pTargets.GetLength()) {
      break;
    }
    pos--;
  }
  if (pos < m_pData->m_nDataLength) {
    ReallocBeforeWrite(m_pData->m_nDataLength);
    m_pData->m_String[pos] = 0;
    m_pData->m_nDataLength = pos;
  }
}

void CFX_ByteString::TrimRight(char chTarget) {
  TrimRight(CFX_ByteStringC(chTarget));
}

void CFX_ByteString::TrimRight() {
  TrimRight("\x09\x0a\x0b\x0c\x0d\x20");
}

void CFX_ByteString::TrimLeft(const CFX_ByteStringC& pTargets) {
  if (!m_pData || pTargets.IsEmpty())
    return;

  FX_STRSIZE len = GetLength();
  if (len < 1)
    return;

  FX_STRSIZE pos = 0;
  while (pos < len) {
    FX_STRSIZE i = 0;
    while (i < pTargets.GetLength() && pTargets[i] != m_pData->m_String[pos]) {
      i++;
    }
    if (i == pTargets.GetLength()) {
      break;
    }
    pos++;
  }
  if (pos) {
    ReallocBeforeWrite(len);
    FX_STRSIZE nDataLength = len - pos;
    memmove(m_pData->m_String, m_pData->m_String + pos,
            (nDataLength + 1) * sizeof(char));
    m_pData->m_nDataLength = nDataLength;
  }
}

void CFX_ByteString::TrimLeft(char chTarget) {
  TrimLeft(CFX_ByteStringC(chTarget));
}

void CFX_ByteString::TrimLeft() {
  TrimLeft("\x09\x0a\x0b\x0c\x0d\x20");
}

FX_STRSIZE FX_ftoa(float d, char* buf) {
  buf[0] = '0';
  buf[1] = '\0';
  if (d == 0.0f) {
    return 1;
  }
  bool bNegative = false;
  if (d < 0) {
    bNegative = true;
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
  FX_STRSIZE buf_size = 0;
  if (bNegative) {
    buf[buf_size++] = '-';
  }
  int i = scaled / scale;
  FXSYS_itoa(i, buf2, 10);
  FX_STRSIZE len = FXSYS_strlen(buf2);
  memcpy(buf + buf_size, buf2, len);
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

CFX_ByteString CFX_ByteString::FormatFloat(float d, int precision) {
  char buf[32];
  FX_STRSIZE len = FX_ftoa(d, buf);
  return CFX_ByteString(buf, len);
}

std::ostream& operator<<(std::ostream& os, const CFX_ByteString& str) {
  return os.write(str.c_str(), str.GetLength());
}

std::ostream& operator<<(std::ostream& os, const CFX_ByteStringC& str) {
  return os.write(str.unterminated_c_str(), str.GetLength());
}
