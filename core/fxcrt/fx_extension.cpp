// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_extension.h"

#include <algorithm>
#include <cwctype>

float FXSYS_wcstof(const wchar_t* pwsStr, int32_t iLength, int32_t* pUsedLen) {
  ASSERT(pwsStr);
  if (iLength < 0)
    iLength = static_cast<int32_t>(wcslen(pwsStr));
  if (iLength == 0)
    return 0.0f;

  int32_t iUsedLen = 0;
  bool bNegtive = false;
  switch (pwsStr[iUsedLen]) {
    case '-':
      bNegtive = true;
    case '+':
      iUsedLen++;
      break;
  }

  float fValue = 0.0f;
  while (iUsedLen < iLength) {
    wchar_t wch = pwsStr[iUsedLen];
    if (!std::iswdigit(wch))
      break;

    fValue = fValue * 10.0f + (wch - L'0');
    iUsedLen++;
  }

  if (iUsedLen < iLength && pwsStr[iUsedLen] == L'.') {
    float fPrecise = 0.1f;
    while (++iUsedLen < iLength) {
      wchar_t wch = pwsStr[iUsedLen];
      if (!std::iswdigit(wch))
        break;

      fValue += (wch - L'0') * fPrecise;
      fPrecise *= 0.1f;
    }
  }
  if (pUsedLen)
    *pUsedLen = iUsedLen;

  return bNegtive ? -fValue : fValue;
}

wchar_t* FXSYS_wcsncpy(wchar_t* dstStr, const wchar_t* srcStr, size_t count) {
  ASSERT(dstStr && srcStr && count > 0);
  for (size_t i = 0; i < count; ++i)
    if ((dstStr[i] = srcStr[i]) == L'\0')
      break;
  return dstStr;
}

int32_t FXSYS_wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t count) {
  ASSERT(s1 && s2 && count > 0);
  wchar_t wch1 = 0, wch2 = 0;
  while (count-- > 0) {
    wch1 = static_cast<wchar_t>(FXSYS_tolower(*s1++));
    wch2 = static_cast<wchar_t>(FXSYS_tolower(*s2++));
    if (wch1 != wch2)
      break;
  }
  return wch1 - wch2;
}

uint32_t FX_HashCode_GetA(const ByteStringView& str, bool bIgnoreCase) {
  uint32_t dwHashCode = 0;
  if (bIgnoreCase) {
    for (const auto& c : str)
      dwHashCode = 31 * dwHashCode + FXSYS_tolower(c);
  } else {
    for (const auto& c : str)
      dwHashCode = 31 * dwHashCode + c;
  }
  return dwHashCode;
}

uint32_t FX_HashCode_GetW(const WideStringView& str, bool bIgnoreCase) {
  uint32_t dwHashCode = 0;
  if (bIgnoreCase) {
    for (const auto& c : str)
      dwHashCode = 1313 * dwHashCode + FXSYS_tolower(c);
  } else {
    for (const auto& c : str)
      dwHashCode = 1313 * dwHashCode + c;
  }
  return dwHashCode;
}

void FXSYS_IntToTwoHexChars(uint8_t n, char* buf) {
  static const char kHex[] = "0123456789ABCDEF";
  buf[0] = kHex[n / 16];
  buf[1] = kHex[n % 16];
}

void FXSYS_IntToFourHexChars(uint16_t n, char* buf) {
  FXSYS_IntToTwoHexChars(n / 256, buf);
  FXSYS_IntToTwoHexChars(n % 256, buf + 2);
}

size_t FXSYS_ToUTF16BE(uint32_t unicode, char* buf) {
  ASSERT(unicode <= 0xD7FF || (unicode > 0xDFFF && unicode <= 0x10FFFF));
  if (unicode <= 0xFFFF) {
    FXSYS_IntToFourHexChars(unicode, buf);
    return 4;
  }
  unicode -= 0x010000;
  // High ten bits plus 0xD800
  FXSYS_IntToFourHexChars(0xD800 + unicode / 0x400, buf);
  // Low ten bits plus 0xDC00
  FXSYS_IntToFourHexChars(0xDC00 + unicode % 0x400, buf + 4);
  return 8;
}

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits) {
  ASSERT(0 < nbits && nbits <= 32);
  const uint8_t* dataPtr = &pData[bitpos / 8];
  int bitShift;
  int bitMask;
  int dstShift;
  int bitCount = bitpos & 0x07;
  if (nbits < 8 && nbits + bitCount <= 8) {
    bitShift = 8 - nbits - bitCount;
    bitMask = (1 << nbits) - 1;
    dstShift = 0;
  } else {
    bitShift = 0;
    int bitOffset = 8 - bitCount;
    bitMask = (1 << std::min(bitOffset, nbits)) - 1;
    dstShift = nbits - bitOffset;
  }
  uint32_t result =
      static_cast<uint32_t>((*dataPtr++ >> bitShift & bitMask) << dstShift);
  while (dstShift >= 8) {
    dstShift -= 8;
    result |= *dataPtr++ << dstShift;
  }
  if (dstShift > 0) {
    bitShift = 8 - dstShift;
    bitMask = (1 << dstShift) - 1;
    result |= *dataPtr++ >> bitShift & bitMask;
  }
  return result;
}
