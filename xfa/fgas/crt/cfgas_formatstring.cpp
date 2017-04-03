// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/cfgas_formatstring.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/fx_ext.h"
#include "core/fxcrt/xml/cxml_element.h"

#define FX_LOCALECATEGORY_DateHash 0xbde9abde
#define FX_LOCALECATEGORY_TimeHash 0x2d71b00f
#define FX_LOCALECATEGORY_DateTimeHash 0x158c72ed
#define FX_LOCALECATEGORY_NumHash 0x0b4ff870
#define FX_LOCALECATEGORY_TextHash 0x2d08af85
#define FX_LOCALECATEGORY_ZeroHash 0x568cb500
#define FX_LOCALECATEGORY_NullHash 0x052931bb

namespace {

struct FX_LOCALESUBCATEGORYINFO {
  uint32_t uHash;
  const wchar_t* pName;
  int32_t eSubCategory;
};

const FX_LOCALESUBCATEGORYINFO g_FXLocaleDateTimeSubCatData[] = {
    {0x14da2125, L"default", FX_LOCALEDATETIMESUBCATEGORY_Default},
    {0x9041d4b0, L"short", FX_LOCALEDATETIMESUBCATEGORY_Short},
    {0xa084a381, L"medium", FX_LOCALEDATETIMESUBCATEGORY_Medium},
    {0xcdce56b3, L"full", FX_LOCALEDATETIMESUBCATEGORY_Full},
    {0xf6b4afb0, L"long", FX_LOCALEDATETIMESUBCATEGORY_Long},
};
const int32_t g_iFXLocaleDateTimeSubCatCount =
    sizeof(g_FXLocaleDateTimeSubCatData) / sizeof(FX_LOCALESUBCATEGORYINFO);

const FX_LOCALESUBCATEGORYINFO g_FXLocaleNumSubCatData[] = {
    {0x46f95531, L"percent", FX_LOCALENUMPATTERN_Percent},
    {0x4c4e8acb, L"currency", FX_LOCALENUMPATTERN_Currency},
    {0x54034c2f, L"decimal", FX_LOCALENUMPATTERN_Decimal},
    {0x7568e6ae, L"integer", FX_LOCALENUMPATTERN_Integer},
};
const int32_t g_iFXLocaleNumSubCatCount =
    sizeof(g_FXLocaleNumSubCatData) / sizeof(FX_LOCALESUBCATEGORYINFO);

struct FX_LOCALETIMEZONEINFO {
  uint32_t uHash;
  int16_t iHour;
  int16_t iMinute;
};

const FX_LOCALETIMEZONEINFO g_FXLocaleTimeZoneData[] = {
    {FXBSTR_ID(0, 'C', 'D', 'T'), -5, 0}, {FXBSTR_ID(0, 'C', 'S', 'T'), -6, 0},
    {FXBSTR_ID(0, 'E', 'D', 'T'), -4, 0}, {FXBSTR_ID(0, 'E', 'S', 'T'), -5, 0},
    {FXBSTR_ID(0, 'M', 'D', 'T'), -6, 0}, {FXBSTR_ID(0, 'M', 'S', 'T'), -7, 0},
    {FXBSTR_ID(0, 'P', 'D', 'T'), -7, 0}, {FXBSTR_ID(0, 'P', 'S', 'T'), -8, 0},
};

const wchar_t gs_wsTimeSymbols[] = L"hHkKMSFAzZ";
const wchar_t gs_wsDateSymbols[] = L"DJMEeGgYwW";
const wchar_t gs_wsConstChars[] = L",-:/. ";

int32_t ParseTimeZone(const wchar_t* pStr, int32_t iLen, FX_TIMEZONE* tz) {
  tz->tzHour = 0;
  tz->tzMinute = 0;
  if (iLen < 0)
    return 0;

  int32_t iStart = 1;
  int32_t iEnd = iStart + 2;
  while (iStart < iLen && iStart < iEnd)
    tz->tzHour = tz->tzHour * 10 + pStr[iStart++] - '0';

  if (iStart < iLen && pStr[iStart] == ':')
    iStart++;

  iEnd = iStart + 2;
  while (iStart < iLen && iStart < iEnd)
    tz->tzMinute = tz->tzMinute * 10 + pStr[iStart++] - '0';

  if (pStr[0] == '-')
    tz->tzHour = -tz->tzHour;

  return iStart;
}

CFX_WideString GetLiteralText(const wchar_t* pStrPattern,
                              int32_t& iPattern,
                              int32_t iLenPattern) {
  CFX_WideString wsOutput;
  if (pStrPattern[iPattern] != '\'') {
    return wsOutput;
  }
  iPattern++;
  int32_t iQuote = 1;
  while (iPattern < iLenPattern) {
    if (pStrPattern[iPattern] == '\'') {
      iQuote++;
      if ((iPattern + 1 >= iLenPattern) ||
          ((pStrPattern[iPattern + 1] != '\'') && (iQuote % 2 == 0))) {
        break;
      } else {
        iQuote++;
      }
      iPattern++;
    } else if (pStrPattern[iPattern] == '\\' && (iPattern + 1 < iLenPattern) &&
               pStrPattern[iPattern + 1] == 'u') {
      int32_t iKeyValue = 0;
      iPattern += 2;
      int32_t i = 0;
      while (iPattern < iLenPattern && i++ < 4) {
        wchar_t ch = pStrPattern[iPattern++];
        if ((ch >= '0' && ch <= '9')) {
          iKeyValue = iKeyValue * 16 + ch - '0';
        } else if ((ch >= 'a' && ch <= 'f')) {
          iKeyValue = iKeyValue * 16 + ch - 'a' + 10;
        } else if ((ch >= 'A' && ch <= 'F')) {
          iKeyValue = iKeyValue * 16 + ch - 'A' + 10;
        }
      }
      if (iKeyValue != 0) {
        wsOutput += (wchar_t)(iKeyValue & 0x0000FFFF);
      }
      continue;
    }
    wsOutput += pStrPattern[iPattern++];
  }
  return wsOutput;
}

CFX_WideString GetLiteralTextReverse(const wchar_t* pStrPattern,
                                     int32_t& iPattern) {
  CFX_WideString wsOutput;
  if (pStrPattern[iPattern] != '\'') {
    return wsOutput;
  }
  iPattern--;
  int32_t iQuote = 1;
  while (iPattern >= 0) {
    if (pStrPattern[iPattern] == '\'') {
      iQuote++;
      if (iPattern - 1 >= 0 ||
          ((pStrPattern[iPattern - 1] != '\'') && (iQuote % 2 == 0))) {
        break;
      }
      iQuote++;
      iPattern--;
    } else if (pStrPattern[iPattern] == '\\' &&
               pStrPattern[iPattern + 1] == 'u') {
      iPattern--;
      int32_t iKeyValue = 0;
      int32_t iLen = wsOutput.GetLength();
      int32_t i = 1;
      for (; i < iLen && i < 5; i++) {
        wchar_t ch = wsOutput[i];
        if ((ch >= '0' && ch <= '9')) {
          iKeyValue = iKeyValue * 16 + ch - '0';
        } else if ((ch >= 'a' && ch <= 'f')) {
          iKeyValue = iKeyValue * 16 + ch - 'a' + 10;
        } else if ((ch >= 'A' && ch <= 'F')) {
          iKeyValue = iKeyValue * 16 + ch - 'A' + 10;
        }
      }
      if (iKeyValue != 0) {
        wsOutput.Delete(0, i);
        wsOutput = (wchar_t)(iKeyValue & 0x0000FFFF) + wsOutput;
      }
      continue;
    }
    wsOutput = pStrPattern[iPattern--] + wsOutput;
  }
  return wsOutput;
}

bool GetNumericDotIndex(const CFX_WideString& wsNum,
                        const CFX_WideString& wsDotSymbol,
                        int32_t& iDotIndex) {
  int32_t ccf = 0;
  int32_t iLenf = wsNum.GetLength();
  const wchar_t* pStr = wsNum.c_str();
  int32_t iLenDot = wsDotSymbol.GetLength();
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      GetLiteralText(pStr, ccf, iLenf);
    } else if (ccf + iLenDot <= iLenf &&
               !wcsncmp(pStr + ccf, wsDotSymbol.c_str(), iLenDot)) {
      iDotIndex = ccf;
      return true;
    }
    ccf++;
  }
  iDotIndex = wsNum.Find('.');
  if (iDotIndex < 0) {
    iDotIndex = iLenf;
    return false;
  }
  return true;
}

bool ParseLocaleDate(const CFX_WideString& wsDate,
                     const CFX_WideString& wsDatePattern,
                     IFX_Locale* pLocale,
                     CFX_DateTime* datetime,
                     int32_t& cc) {
  int32_t year = 1900;
  int32_t month = 1;
  int32_t day = 1;
  int32_t ccf = 0;
  const wchar_t* str = wsDate.c_str();
  int32_t len = wsDate.GetLength();
  const wchar_t* strf = wsDatePattern.c_str();
  int32_t lenf = wsDatePattern.GetLength();
  CFX_WideStringC wsDateSymbols(gs_wsDateSymbols);
  while (cc < len && ccf < lenf) {
    if (strf[ccf] == '\'') {
      CFX_WideString wsLiteral = GetLiteralText(strf, ccf, lenf);
      int32_t iLiteralLen = wsLiteral.GetLength();
      if (cc + iLiteralLen > len ||
          wcsncmp(str + cc, wsLiteral.c_str(), iLiteralLen)) {
        return false;
      }
      cc += iLiteralLen;
      ccf++;
      continue;
    } else if (wsDateSymbols.Find(strf[ccf]) == -1) {
      if (strf[ccf] != str[cc])
        return false;
      cc++;
      ccf++;
      continue;
    }
    uint32_t dwSymbolNum = 1;
    wchar_t dwCharSymbol = strf[ccf++];
    while (ccf < lenf && strf[ccf] == dwCharSymbol) {
      ccf++;
      dwSymbolNum++;
    }
    uint32_t dwSymbol = (dwCharSymbol << 8) | (dwSymbolNum + '0');
    if (dwSymbol == FXBSTR_ID(0, 0, 'D', '1')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      day = str[cc++] - '0';
      if (cc < len && FXSYS_isDecimalDigit(str[cc])) {
        day = day * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'D', '2')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      day = str[cc++] - '0';
      if (cc < len) {
        day = day * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '1')) {
      int i = 0;
      while (cc < len && i < 3 && FXSYS_isDecimalDigit(str[cc])) {
        cc++;
        i++;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '3')) {
      cc += 3;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      month = str[cc++] - '0';
      if (cc < len && FXSYS_isDecimalDigit(str[cc])) {
        month = month * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      month = str[cc++] - '0';
      if (cc < len) {
        month = month * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '3')) {
      CFX_WideString wsMonthNameAbbr;
      uint16_t i = 0;
      for (; i < 12; i++) {
        wsMonthNameAbbr = pLocale->GetMonthName(i, true);
        if (wsMonthNameAbbr.IsEmpty())
          continue;
        if (!wcsncmp(wsMonthNameAbbr.c_str(), str + cc,
                     wsMonthNameAbbr.GetLength())) {
          break;
        }
      }
      if (i < 12) {
        cc += wsMonthNameAbbr.GetLength();
        month = i + 1;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '4')) {
      CFX_WideString wsMonthName;
      uint16_t i = 0;
      for (; i < 12; i++) {
        wsMonthName = pLocale->GetMonthName(i, false);
        if (wsMonthName.IsEmpty())
          continue;
        if (!wcsncmp(wsMonthName.c_str(), str + cc, wsMonthName.GetLength())) {
          break;
        }
      }
      if (i < 12) {
        cc += wsMonthName.GetLength();
        month = i + 1;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '1')) {
      cc += 1;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '3')) {
      CFX_WideString wsDayNameAbbr;
      uint16_t i = 0;
      for (; i < 7; i++) {
        wsDayNameAbbr = pLocale->GetDayName(i, true);
        if (wsDayNameAbbr.IsEmpty())
          continue;
        if (!wcsncmp(wsDayNameAbbr.c_str(), str + cc,
                     wsDayNameAbbr.GetLength())) {
          break;
        }
      }
      if (i < 12) {
        cc += wsDayNameAbbr.GetLength();
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '4')) {
      CFX_WideString wsDayName;
      int32_t i = 0;
      for (; i < 7; i++) {
        wsDayName = pLocale->GetDayName(i, false);
        if (wsDayName == L"")
          continue;
        if (!wcsncmp(wsDayName.c_str(), str + cc, wsDayName.GetLength())) {
          break;
        }
      }
      if (i < 12) {
        cc += wsDayName.GetLength();
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'e', '1')) {
      cc += 1;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'G', '1')) {
      cc += 2;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '2')) {
      if (cc + 2 > len) {
        return false;
      }
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      year = str[cc++] - '0';
      if (cc >= len || !FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      year = year * 10 + str[cc++] - '0';
      if (year <= 29) {
        year += 2000;
      } else {
        year += 1900;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '4')) {
      int i = 0;
      year = 0;
      if (cc + 4 > len) {
        return false;
      }
      while (i < 4) {
        if (!FXSYS_isDecimalDigit(str[cc])) {
          return false;
        }
        year = year * 10 + str[cc] - '0';
        cc++;
        i++;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'w', '1')) {
      cc += 1;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'W', '2')) {
      cc += 2;
    }
  }
  if (cc < len)
    return false;

  datetime->SetDate(year, month, day);
  return !!cc;
}

void ResolveZone(uint8_t& wHour,
                 uint8_t& wMinute,
                 FX_TIMEZONE tzDiff,
                 IFX_Locale* pLocale) {
  int32_t iMinuteDiff = wHour * 60 + wMinute;
  FX_TIMEZONE tzLocale = pLocale->GetTimeZone();
  iMinuteDiff += tzLocale.tzHour * 60 +
                 (tzLocale.tzHour < 0 ? -tzLocale.tzMinute : tzLocale.tzMinute);
  iMinuteDiff -= tzDiff.tzHour * 60 +
                 (tzDiff.tzHour < 0 ? -tzDiff.tzMinute : tzDiff.tzMinute);
  while (iMinuteDiff > 1440) {
    iMinuteDiff -= 1440;
  }
  while (iMinuteDiff < 0) {
    iMinuteDiff += 1440;
  }
  wHour = iMinuteDiff / 60;
  wMinute = iMinuteDiff % 60;
}

bool ParseLocaleTime(const CFX_WideString& wsTime,
                     const CFX_WideString& wsTimePattern,
                     IFX_Locale* pLocale,
                     CFX_DateTime* datetime,
                     int32_t& cc) {
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
  uint16_t millisecond = 0;
  int32_t ccf = 0;
  const wchar_t* str = wsTime.c_str();
  int len = wsTime.GetLength();
  const wchar_t* strf = wsTimePattern.c_str();
  int lenf = wsTimePattern.GetLength();
  bool bHasA = false;
  bool bPM = false;
  CFX_WideStringC wsTimeSymbols(gs_wsTimeSymbols);
  while (cc < len && ccf < lenf) {
    if (strf[ccf] == '\'') {
      CFX_WideString wsLiteral = GetLiteralText(strf, ccf, lenf);
      int32_t iLiteralLen = wsLiteral.GetLength();
      if (cc + iLiteralLen > len ||
          wcsncmp(str + cc, wsLiteral.c_str(), iLiteralLen)) {
        return false;
      }
      cc += iLiteralLen;
      ccf++;
      continue;
    } else if (wsTimeSymbols.Find(strf[ccf]) == -1) {
      if (strf[ccf] != str[cc])
        return false;
      cc++;
      ccf++;
      continue;
    }
    uint32_t dwSymbolNum = 1;
    wchar_t dwCharSymbol = strf[ccf++];
    while (ccf < lenf && strf[ccf] == dwCharSymbol) {
      ccf++;
      dwSymbolNum++;
    }
    uint32_t dwSymbol = (dwCharSymbol << 8) | (dwSymbolNum + '0');
    if (dwSymbol == FXBSTR_ID(0, 0, 'k', '1') ||
        dwSymbol == FXBSTR_ID(0, 0, 'H', '1') ||
        dwSymbol == FXBSTR_ID(0, 0, 'h', '1') ||
        dwSymbol == FXBSTR_ID(0, 0, 'K', '1')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      hour = str[cc++] - '0';
      if (cc < len && FXSYS_isDecimalDigit(str[cc])) {
        hour = hour * 10 + str[cc++] - '0';
      }
      if (dwSymbol == FXBSTR_ID(0, 0, 'K', '1') && hour == 24) {
        hour = 0;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '2') ||
               dwSymbol == FXBSTR_ID(0, 0, 'H', '2') ||
               dwSymbol == FXBSTR_ID(0, 0, 'h', '2') ||
               dwSymbol == FXBSTR_ID(0, 0, 'K', '2')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      hour = str[cc++] - '0';
      if (cc >= len) {
        return false;
      }
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      hour = hour * 10 + str[cc++] - '0';
      if (dwSymbol == FXBSTR_ID(0, 0, 'K', '2') && hour == 24) {
        hour = 0;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      minute = str[cc++] - '0';
      if (cc < len && FXSYS_isDecimalDigit(str[cc])) {
        minute = minute * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      minute = str[cc++] - '0';
      if (cc >= len) {
        return false;
      }
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      minute = minute * 10 + str[cc++] - '0';
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '1')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      second = str[cc++] - '0';
      if (cc < len && FXSYS_isDecimalDigit(str[cc])) {
        second = second * 10 + str[cc++] - '0';
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '2')) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      second = str[cc++] - '0';
      if (cc >= len) {
        return false;
      }
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      second = second * 10 + str[cc++] - '0';
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'F', '3')) {
      if (cc + 3 >= len) {
        return false;
      }
      int i = 0;
      while (i < 3) {
        if (!FXSYS_isDecimalDigit(str[cc])) {
          return false;
        }
        millisecond = millisecond * 10 + str[cc++] - '0';
        i++;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'A', '1')) {
      CFX_WideString wsAM = pLocale->GetMeridiemName(true);
      CFX_WideString wsPM = pLocale->GetMeridiemName(false);
      if ((cc + wsAM.GetLength() <= len) &&
          (CFX_WideStringC(str + cc, wsAM.GetLength()) == wsAM)) {
        cc += wsAM.GetLength();
        bHasA = true;
      } else if ((cc + wsPM.GetLength() <= len) &&
                 (CFX_WideStringC(str + cc, wsPM.GetLength()) == wsPM)) {
        cc += wsPM.GetLength();
        bHasA = true;
        bPM = true;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Z', '1')) {
      if (cc + 3 > len) {
        continue;
      }
      uint32_t dwHash = str[cc++];
      dwHash = (dwHash << 8) | str[cc++];
      dwHash = (dwHash << 8) | str[cc++];
      if (dwHash == FXBSTR_ID(0, 'G', 'M', 'T')) {
        FX_TIMEZONE tzDiff;
        tzDiff.tzHour = 0;
        tzDiff.tzMinute = 0;
        if (cc < len && (str[cc] == '-' || str[cc] == '+'))
          cc += ParseTimeZone(str + cc, len - cc, &tzDiff);

        ResolveZone(hour, minute, tzDiff, pLocale);
      } else {
        const FX_LOCALETIMEZONEINFO* pEnd =
            g_FXLocaleTimeZoneData + FX_ArraySize(g_FXLocaleTimeZoneData);
        const FX_LOCALETIMEZONEINFO* pTimeZoneInfo =
            std::lower_bound(g_FXLocaleTimeZoneData, pEnd, dwHash,
                             [](const FX_LOCALETIMEZONEINFO& info,
                                uint32_t hash) { return info.uHash < hash; });
        if (pTimeZoneInfo < pEnd && dwHash == pTimeZoneInfo->uHash) {
          hour += pTimeZoneInfo->iHour;
          minute += pTimeZoneInfo->iHour > 0 ? pTimeZoneInfo->iMinute
                                             : -pTimeZoneInfo->iMinute;
        }
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'z', '1')) {
      if (str[cc] != 'Z') {
        FX_TIMEZONE tzDiff;
        cc += ParseTimeZone(str + cc, len - cc, &tzDiff);
        ResolveZone(hour, minute, tzDiff, pLocale);
      } else {
        cc++;
      }
    }
  }
  if (bHasA) {
    if (bPM) {
      hour += 12;
      if (hour == 24) {
        hour = 12;
      }
    } else {
      if (hour == 12) {
        hour = 0;
      }
    }
  }
  datetime->SetTime(hour, minute, second, millisecond);
  return !!cc;
}

int32_t GetNumTrailingLimit(const CFX_WideString& wsFormat,
                            int iDotPos,
                            bool& bTrimTailZeros) {
  if (iDotPos < 0)
    return 0;

  int32_t iCount = wsFormat.GetLength();
  int32_t iTreading = 0;
  for (iDotPos++; iDotPos < iCount; iDotPos++) {
    wchar_t wc = wsFormat[iDotPos];
    if (wc == L'z' || wc == L'9' || wc == 'Z') {
      iTreading++;
      bTrimTailZeros = (wc == L'9' ? false : true);
    }
  }
  return iTreading;
}

uint16_t GetSolarMonthDays(uint16_t year, uint16_t month) {
  if (month % 2)
    return 31;
  if (month == 2)
    return FX_IsLeapYear(year) ? 29 : 28;
  return 30;
}

uint16_t GetWeekDay(uint16_t year, uint16_t month, uint16_t day) {
  uint16_t g_month_day[] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
  uint16_t nDays =
      (year - 1) % 7 + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;
  nDays += g_month_day[month - 1] + day;
  if (FX_IsLeapYear(year) && month > 2)
    nDays++;
  return nDays % 7;
}

uint16_t GetWeekOfMonth(uint16_t year, uint16_t month, uint16_t day) {
  uint16_t week_day = GetWeekDay(year, month, 1);
  uint16_t week_index = 0;
  week_index += day / 7;
  day = day % 7;
  if (week_day + day > 7)
    week_index++;
  return week_index;
}

uint16_t GetWeekOfYear(uint16_t year, uint16_t month, uint16_t day) {
  uint16_t nDays = 0;
  for (uint16_t i = 1; i < month; i++)
    nDays += GetSolarMonthDays(year, i);

  nDays += day;
  uint16_t week_day = GetWeekDay(year, 1, 1);
  uint16_t week_index = 1;
  week_index += nDays / 7;
  nDays = nDays % 7;
  if (week_day + nDays > 7)
    week_index++;
  return week_index;
}

bool DateFormat(const CFX_WideString& wsDatePattern,
                IFX_Locale* pLocale,
                const CFX_DateTime& datetime,
                CFX_WideString& wsResult) {
  bool bRet = true;
  int32_t year = datetime.GetYear();
  uint8_t month = datetime.GetMonth();
  uint8_t day = datetime.GetDay();
  int32_t ccf = 0;
  const wchar_t* strf = wsDatePattern.c_str();
  int32_t lenf = wsDatePattern.GetLength();
  CFX_WideStringC wsDateSymbols(gs_wsDateSymbols);
  while (ccf < lenf) {
    if (strf[ccf] == '\'') {
      wsResult += GetLiteralText(strf, ccf, lenf);
      ccf++;
      continue;
    } else if (wsDateSymbols.Find(strf[ccf]) == -1) {
      wsResult += strf[ccf++];
      continue;
    }
    uint32_t dwSymbolNum = 1;
    wchar_t dwCharSymbol = strf[ccf++];
    while (ccf < lenf && strf[ccf] == dwCharSymbol) {
      ccf++;
      dwSymbolNum++;
    }
    uint32_t dwSymbol = (dwCharSymbol << 8) | (dwSymbolNum + '0');
    if (dwSymbol == FXBSTR_ID(0, 0, 'D', '1')) {
      CFX_WideString wsDay;
      wsDay.Format(L"%d", day);
      wsResult += wsDay;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'D', '2')) {
      CFX_WideString wsDay;
      wsDay.Format(L"%02d", day);
      wsResult += wsDay;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '1')) {
      uint16_t nDays = 0;
      for (int i = 1; i < month; i++) {
        nDays += GetSolarMonthDays(year, i);
      }
      nDays += day;
      CFX_WideString wsDays;
      wsDays.Format(L"%d", nDays);
      wsResult += wsDays;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '3')) {
      uint16_t nDays = 0;
      for (int i = 1; i < month; i++) {
        nDays += GetSolarMonthDays(year, i);
      }
      nDays += day;
      CFX_WideString wsDays;
      wsDays.Format(L"%03d", nDays);
      wsResult += wsDays;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
      CFX_WideString wsMonth;
      wsMonth.Format(L"%d", month);
      wsResult += wsMonth;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
      CFX_WideString wsMonth;
      wsMonth.Format(L"%02d", month);
      wsResult += wsMonth;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '3')) {
      wsResult += pLocale->GetMonthName(month - 1, true);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '4')) {
      wsResult += pLocale->GetMonthName(month - 1, false);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '1')) {
      uint16_t wWeekDay = GetWeekDay(year, month, day);
      CFX_WideString wsWeekDay;
      wsWeekDay.Format(L"%d", wWeekDay + 1);
      wsResult += wsWeekDay;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '3')) {
      uint16_t wWeekDay = GetWeekDay(year, month, day);
      wsResult += pLocale->GetDayName(wWeekDay, true);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '4')) {
      uint16_t wWeekDay = GetWeekDay(year, month, day);
      if (pLocale)
        wsResult += pLocale->GetDayName(wWeekDay, false);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'e', '1')) {
      uint16_t wWeekDay = GetWeekDay(year, month, day);
      CFX_WideString wsWeekDay;
      wsWeekDay.Format(L"%d", wWeekDay ? wWeekDay : 7);
      wsResult += wsWeekDay;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'G', '1')) {
      wsResult += pLocale->GetEraName(year < 0);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '2')) {
      CFX_WideString wsYear;
      wsYear.Format(L"%02d", year % 100);
      wsResult += wsYear;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '4')) {
      CFX_WideString wsYear;
      wsYear.Format(L"%d", year);
      wsResult += wsYear;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'w', '1')) {
      uint16_t week_index = GetWeekOfMonth(year, month, day);
      CFX_WideString wsWeekInMonth;
      wsWeekInMonth.Format(L"%d", week_index);
      wsResult += wsWeekInMonth;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'W', '2')) {
      uint16_t week_index = GetWeekOfYear(year, month, day);
      CFX_WideString wsWeekInYear;
      wsWeekInYear.Format(L"%02d", week_index);
      wsResult += wsWeekInYear;
    }
  }
  return bRet;
}

bool TimeFormat(const CFX_WideString& wsTimePattern,
                IFX_Locale* pLocale,
                const CFX_DateTime& datetime,
                CFX_WideString& wsResult) {
  bool bGMT = false;
  bool bRet = true;
  uint8_t hour = datetime.GetHour();
  uint8_t minute = datetime.GetMinute();
  uint8_t second = datetime.GetSecond();
  uint16_t millisecond = datetime.GetMillisecond();
  int32_t ccf = 0;
  const wchar_t* strf = wsTimePattern.c_str();
  int32_t lenf = wsTimePattern.GetLength();
  uint16_t wHour = hour;
  bool bPM = false;
  if (wsTimePattern.Find('A') != -1) {
    if (wHour >= 12) {
      bPM = true;
    }
  }
  CFX_WideStringC wsTimeSymbols(gs_wsTimeSymbols);
  while (ccf < lenf) {
    if (strf[ccf] == '\'') {
      wsResult += GetLiteralText(strf, ccf, lenf);
      ccf++;
      continue;
    } else if (wsTimeSymbols.Find(strf[ccf]) == -1) {
      wsResult += strf[ccf++];
      continue;
    }
    uint32_t dwSymbolNum = 1;
    wchar_t dwCharSymbol = strf[ccf++];
    while (ccf < lenf && strf[ccf] == dwCharSymbol) {
      ccf++;
      dwSymbolNum++;
    }
    uint32_t dwSymbol = (dwCharSymbol << 8) | (dwSymbolNum + '0');
    if (dwSymbol == FXBSTR_ID(0, 0, 'h', '1')) {
      if (wHour > 12) {
        wHour -= 12;
      }
      CFX_WideString wsHour;
      wsHour.Format(L"%d", wHour == 0 ? 12 : wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'h', '2')) {
      if (wHour > 12) {
        wHour -= 12;
      }
      CFX_WideString wsHour;
      wsHour.Format(L"%02d", wHour == 0 ? 12 : wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'K', '1')) {
      CFX_WideString wsHour;
      wsHour.Format(L"%d", wHour == 0 ? 24 : wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'K', '2')) {
      CFX_WideString wsHour;
      wsHour.Format(L"%02d", wHour == 0 ? 24 : wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '1')) {
      if (wHour > 12) {
        wHour -= 12;
      }
      CFX_WideString wsHour;
      wsHour.Format(L"%d", wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'H', '1')) {
      CFX_WideString wsHour;
      wsHour.Format(L"%d", wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '2')) {
      if (wHour > 12) {
        wHour -= 12;
      }
      CFX_WideString wsHour;
      wsHour.Format(L"%02d", wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'H', '2')) {
      CFX_WideString wsHour;
      wsHour.Format(L"%02d", wHour);
      wsResult += wsHour;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
      CFX_WideString wsMinute;
      wsMinute.Format(L"%d", minute);
      wsResult += wsMinute;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
      CFX_WideString wsMinute;
      wsMinute.Format(L"%02d", minute);
      wsResult += wsMinute;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '1')) {
      CFX_WideString wsSecond;
      wsSecond.Format(L"%d", second);
      wsResult += wsSecond;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '2')) {
      CFX_WideString wsSecond;
      wsSecond.Format(L"%02d", second);
      wsResult += wsSecond;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'F', '3')) {
      CFX_WideString wsMilliseconds;
      wsMilliseconds.Format(L"%03d", millisecond);
      wsResult += wsMilliseconds;
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'A', '1')) {
      wsResult += pLocale->GetMeridiemName(!bPM);
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'Z', '1')) {
      wsResult += L"GMT";
      FX_TIMEZONE tz = pLocale->GetTimeZone();
      if (!bGMT && (tz.tzHour != 0 || tz.tzMinute != 0)) {
        wsResult += tz.tzHour < 0 ? L"-" : L"+";

        CFX_WideString wsTimezone;
        wsTimezone.Format(L"%02d:%02d", abs(tz.tzHour), tz.tzMinute);
        wsResult += wsTimezone;
      }
    } else if (dwSymbol == FXBSTR_ID(0, 0, 'z', '1')) {
      FX_TIMEZONE tz = pLocale->GetTimeZone();
      if (!bGMT && tz.tzHour != 0 && tz.tzMinute != 0) {
        wsResult += tz.tzHour < 0 ? L"-" : L"+";

        CFX_WideString wsTimezone;
        wsTimezone.Format(L"%02d:%02d", abs(tz.tzHour), tz.tzMinute);
        wsResult += wsTimezone;
      }
    }
  }
  return bRet;
}

bool FormatDateTimeInternal(const CFX_DateTime& dt,
                            const CFX_WideString& wsDatePattern,
                            const CFX_WideString& wsTimePattern,
                            bool bDateFirst,
                            IFX_Locale* pLocale,
                            CFX_WideString& wsOutput) {
  bool bRet = true;
  CFX_WideString wsDateOut, wsTimeOut;
  if (!wsDatePattern.IsEmpty())
    bRet &= DateFormat(wsDatePattern, pLocale, dt, wsDateOut);
  if (!wsTimePattern.IsEmpty())
    bRet &= TimeFormat(wsTimePattern, pLocale, dt, wsTimeOut);

  wsOutput = bDateFirst ? wsDateOut + wsTimeOut : wsTimeOut + wsDateOut;
  return bRet;
}

}  // namespace

bool FX_DateFromCanonical(const CFX_WideString& wsDate,
                          CFX_DateTime* datetime) {
  int32_t year = 1900;
  int32_t month = 1;
  int32_t day = 1;
  uint16_t wYear = 0;
  int cc_start = 0, cc = 0;
  const wchar_t* str = wsDate.c_str();
  int len = wsDate.GetLength();
  if (len > 10) {
    return false;
  }
  while (cc < len && cc < 4) {
    if (!FXSYS_isDecimalDigit(str[cc])) {
      return false;
    }
    wYear = wYear * 10 + str[cc++] - '0';
  }
  year = wYear;
  if (cc < 4 || wYear < 1900) {
    return false;
  }
  if (cc < len) {
    if (str[cc] == '-') {
      cc++;
    }
    cc_start = cc;
    uint8_t tmpM = 0;
    while (cc < len && cc < cc_start + 2) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      tmpM = tmpM * 10 + str[cc++] - '0';
    }
    month = tmpM;
    if (cc == cc_start + 1 || tmpM > 12 || tmpM < 1) {
      return false;
    }
    if (cc < len) {
      if (str[cc] == '-') {
        cc++;
      }
      uint8_t tmpD = 0;
      cc_start = cc;
      while (cc < len && cc < cc_start + 2) {
        if (!FXSYS_isDecimalDigit(str[cc])) {
          return false;
        }
        tmpD = tmpD * 10 + str[cc++] - '0';
      }
      day = tmpD;
      if (tmpD < 1) {
        return false;
      }
      if ((tmpM == 1 || tmpM == 3 || tmpM == 5 || tmpM == 7 || tmpM == 8 ||
           tmpM == 10 || tmpM == 12) &&
          tmpD > 31) {
        return false;
      }
      if ((tmpM == 4 || tmpM == 6 || tmpM == 9 || tmpM == 11) && tmpD > 30) {
        return false;
      }
      bool iLeapYear;
      if ((wYear % 4 == 0 && wYear % 100 != 0) || wYear % 400 == 0) {
        iLeapYear = true;
      } else {
        iLeapYear = false;
      }
      if ((iLeapYear && tmpM == 2 && tmpD > 29) ||
          (!iLeapYear && tmpM == 2 && tmpD > 28)) {
        return false;
      }
    }
  }
  datetime->SetDate(year, month, day);
  return true;
}

bool FX_TimeFromCanonical(const CFX_WideStringC& wsTime,
                          CFX_DateTime* datetime,
                          IFX_Locale* pLocale) {
  if (wsTime.GetLength() == 0)
    return false;

  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
  uint16_t millisecond = 0;
  int cc_start = 0, cc = cc_start;
  const wchar_t* str = wsTime.c_str();
  int len = wsTime.GetLength();
  while (cc < len && cc < 2) {
    if (!FXSYS_isDecimalDigit(str[cc])) {
      return false;
    }
    hour = hour * 10 + str[cc++] - '0';
  }
  if (cc < 2 || hour >= 24) {
    return false;
  }
  if (cc < len) {
    if (str[cc] == ':') {
      cc++;
    }
    cc_start = cc;
    while (cc < len && cc < cc_start + 2) {
      if (!FXSYS_isDecimalDigit(str[cc])) {
        return false;
      }
      minute = minute * 10 + str[cc++] - '0';
    }
    if (cc == cc_start + 1 || minute >= 60) {
      return false;
    }
    if (cc < len) {
      if (str[cc] == ':') {
        cc++;
      }
      cc_start = cc;
      while (cc < len && cc < cc_start + 2) {
        if (!FXSYS_isDecimalDigit(str[cc])) {
          return false;
        }
        second = second * 10 + str[cc++] - '0';
      }
      if (cc == cc_start + 1 || second >= 60) {
        return false;
      }
      if (cc < len) {
        if (str[cc] == '.') {
          cc++;
          cc_start = cc;
          while (cc < len && cc < cc_start + 3) {
            if (!FXSYS_isDecimalDigit(str[cc])) {
              return false;
            }
            millisecond = millisecond * 10 + str[cc++] - '0';
          }
          if (cc < cc_start + 3)
            return false;
        }
        if (cc < len) {
          FX_TIMEZONE tzDiff;
          tzDiff.tzHour = 0;
          tzDiff.tzMinute = 0;
          if (str[cc] != 'Z')
            cc += ParseTimeZone(str + cc, len - cc, &tzDiff);
          ResolveZone(hour, minute, tzDiff, pLocale);
        }
      }
    }
  }
  datetime->SetTime(hour, minute, second, millisecond);
  return true;
}

CFGAS_FormatString::CFGAS_FormatString(CXFA_LocaleMgr* pLocaleMgr)
    : m_pLocaleMgr(pLocaleMgr) {}

CFGAS_FormatString::~CFGAS_FormatString() {}

void CFGAS_FormatString::SplitFormatString(
    const CFX_WideString& wsFormatString,
    std::vector<CFX_WideString>& wsPatterns) {
  int32_t iStrLen = wsFormatString.GetLength();
  const wchar_t* pStr = wsFormatString.c_str();
  const wchar_t* pToken = pStr;
  const wchar_t* pEnd = pStr + iStrLen;
  bool iQuote = false;
  while (true) {
    if (pStr >= pEnd) {
      wsPatterns.push_back(CFX_WideString(pToken, pStr - pToken));
      return;
    }
    if (*pStr == '\'') {
      iQuote = !iQuote;
    } else if (*pStr == L'|' && !iQuote) {
      wsPatterns.push_back(CFX_WideString(pToken, pStr - pToken));
      pToken = pStr + 1;
    }
    pStr++;
  }
}

FX_LOCALECATEGORY CFGAS_FormatString::GetCategory(
    const CFX_WideString& wsPattern) {
  FX_LOCALECATEGORY eCategory = FX_LOCALECATEGORY_Unknown;
  int32_t ccf = 0;
  int32_t iLenf = wsPattern.GetLength();
  const wchar_t* pStr = wsPattern.c_str();
  bool bBraceOpen = false;
  CFX_WideStringC wsConstChars(gs_wsConstChars);
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      GetLiteralText(pStr, ccf, iLenf);
    } else if (!bBraceOpen && wsConstChars.Find(pStr[ccf]) == -1) {
      CFX_WideString wsCategory(pStr[ccf]);
      ccf++;
      while (true) {
        if (ccf == iLenf) {
          return eCategory;
        }
        if (pStr[ccf] == '.' || pStr[ccf] == '(') {
          break;
        }
        if (pStr[ccf] == '{') {
          bBraceOpen = true;
          break;
        }
        wsCategory += pStr[ccf];
        ccf++;
      }
      uint32_t dwHash = FX_HashCode_GetW(wsCategory.AsStringC(), false);
      if (dwHash == FX_LOCALECATEGORY_DateHash) {
        if (eCategory == FX_LOCALECATEGORY_Time) {
          return FX_LOCALECATEGORY_DateTime;
        }
        eCategory = FX_LOCALECATEGORY_Date;
      } else if (dwHash == FX_LOCALECATEGORY_TimeHash) {
        if (eCategory == FX_LOCALECATEGORY_Date) {
          return FX_LOCALECATEGORY_DateTime;
        }
        eCategory = FX_LOCALECATEGORY_Time;
      } else if (dwHash == FX_LOCALECATEGORY_DateTimeHash) {
        return FX_LOCALECATEGORY_DateTime;
      } else if (dwHash == FX_LOCALECATEGORY_TextHash) {
        return FX_LOCALECATEGORY_Text;
      } else if (dwHash == FX_LOCALECATEGORY_NumHash) {
        return FX_LOCALECATEGORY_Num;
      } else if (dwHash == FX_LOCALECATEGORY_ZeroHash) {
        return FX_LOCALECATEGORY_Zero;
      } else if (dwHash == FX_LOCALECATEGORY_NullHash) {
        return FX_LOCALECATEGORY_Null;
      }
    } else if (pStr[ccf] == '}') {
      bBraceOpen = false;
    }
    ccf++;
  }
  return eCategory;
}

CFX_WideString CFGAS_FormatString::GetLocaleName(
    const CFX_WideString& wsPattern) {
  int32_t ccf = 0;
  int32_t iLenf = wsPattern.GetLength();
  const wchar_t* pStr = wsPattern.c_str();
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      GetLiteralText(pStr, ccf, iLenf);
    } else if (pStr[ccf] == '(') {
      ccf++;
      CFX_WideString wsLCID;
      while (ccf < iLenf && pStr[ccf] != ')') {
        wsLCID += pStr[ccf++];
      }
      return wsLCID;
    }
    ccf++;
  }
  return CFX_WideString();
}

IFX_Locale* CFGAS_FormatString::GetTextFormat(const CFX_WideString& wsPattern,
                                              const CFX_WideStringC& wsCategory,
                                              CFX_WideString& wsPurgePattern) {
  IFX_Locale* pLocale = nullptr;
  int32_t ccf = 0;
  int32_t iLenf = wsPattern.GetLength();
  const wchar_t* pStr = wsPattern.c_str();
  bool bBrackOpen = false;
  CFX_WideStringC wsConstChars(gs_wsConstChars);
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      int32_t iCurChar = ccf;
      GetLiteralText(pStr, ccf, iLenf);
      wsPurgePattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
    } else if (!bBrackOpen && wsConstChars.Find(pStr[ccf]) == -1) {
      CFX_WideString wsSearchCategory(pStr[ccf]);
      ccf++;
      while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' &&
             pStr[ccf] != '(') {
        wsSearchCategory += pStr[ccf];
        ccf++;
      }
      if (wsSearchCategory != wsCategory) {
        continue;
      }
      while (ccf < iLenf) {
        if (pStr[ccf] == '(') {
          ccf++;
          CFX_WideString wsLCID;
          while (ccf < iLenf && pStr[ccf] != ')') {
            wsLCID += pStr[ccf++];
          }
          pLocale = GetPatternLocale(wsLCID);
        } else if (pStr[ccf] == '{') {
          bBrackOpen = true;
          break;
        }
        ccf++;
      }
    } else if (pStr[ccf] != '}') {
      wsPurgePattern += pStr[ccf];
    }
    ccf++;
  }
  if (!bBrackOpen) {
    wsPurgePattern = wsPattern;
  }
  if (!pLocale) {
    pLocale = m_pLocaleMgr->GetDefLocale();
  }
  return pLocale;
}
#define FX_NUMSTYLE_Percent 0x01
#define FX_NUMSTYLE_Exponent 0x02
#define FX_NUMSTYLE_DotVorv 0x04
IFX_Locale* CFGAS_FormatString::GetNumericFormat(
    const CFX_WideString& wsPattern,
    int32_t& iDotIndex,
    uint32_t& dwStyle,
    CFX_WideString& wsPurgePattern) {
  dwStyle = 0;
  IFX_Locale* pLocale = nullptr;
  int32_t ccf = 0;
  int32_t iLenf = wsPattern.GetLength();
  const wchar_t* pStr = wsPattern.c_str();
  bool bFindDot = false;
  bool bBrackOpen = false;
  CFX_WideStringC wsConstChars(gs_wsConstChars);
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      int32_t iCurChar = ccf;
      GetLiteralText(pStr, ccf, iLenf);
      wsPurgePattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
    } else if (!bBrackOpen && wsConstChars.Find(pStr[ccf]) == -1) {
      CFX_WideString wsCategory(pStr[ccf]);
      ccf++;
      while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' &&
             pStr[ccf] != '(') {
        wsCategory += pStr[ccf];
        ccf++;
      }
      if (wsCategory != L"num") {
        bBrackOpen = true;
        ccf = 0;
        continue;
      }
      while (ccf < iLenf) {
        if (pStr[ccf] == '(') {
          ccf++;
          CFX_WideString wsLCID;
          while (ccf < iLenf && pStr[ccf] != ')') {
            wsLCID += pStr[ccf++];
          }
          pLocale = GetPatternLocale(wsLCID);
        } else if (pStr[ccf] == '{') {
          bBrackOpen = true;
          break;
        } else if (pStr[ccf] == '.') {
          CFX_WideString wsSubCategory;
          ccf++;
          while (ccf < iLenf && pStr[ccf] != '(' && pStr[ccf] != '{') {
            wsSubCategory += pStr[ccf++];
          }
          uint32_t dwSubHash =
              FX_HashCode_GetW(wsSubCategory.AsStringC(), false);
          FX_LOCALENUMSUBCATEGORY eSubCategory = FX_LOCALENUMPATTERN_Decimal;
          for (int32_t i = 0; i < g_iFXLocaleNumSubCatCount; i++) {
            if (g_FXLocaleNumSubCatData[i].uHash == dwSubHash) {
              eSubCategory = (FX_LOCALENUMSUBCATEGORY)g_FXLocaleNumSubCatData[i]
                                 .eSubCategory;
              break;
            }
          }
          if (!pLocale)
            pLocale = m_pLocaleMgr->GetDefLocale();

          ASSERT(pLocale);

          wsSubCategory = pLocale->GetNumPattern(eSubCategory);
          iDotIndex = wsSubCategory.Find('.');
          if (iDotIndex > 0) {
            iDotIndex += wsPurgePattern.GetLength();
            bFindDot = true;
            dwStyle |= FX_NUMSTYLE_DotVorv;
          }
          wsPurgePattern += wsSubCategory;
          if (eSubCategory == FX_LOCALENUMPATTERN_Percent) {
            dwStyle |= FX_NUMSTYLE_Percent;
          }
          continue;
        }
        ccf++;
      }
    } else if (pStr[ccf] == 'E') {
      dwStyle |= FX_NUMSTYLE_Exponent;
      wsPurgePattern += pStr[ccf];
    } else if (pStr[ccf] == '%') {
      dwStyle |= FX_NUMSTYLE_Percent;
      wsPurgePattern += pStr[ccf];
    } else if (pStr[ccf] != '}') {
      wsPurgePattern += pStr[ccf];
    }
    if (!bFindDot) {
      if (pStr[ccf] == '.' || pStr[ccf] == 'V' || pStr[ccf] == 'v') {
        bFindDot = true;
        iDotIndex = wsPurgePattern.GetLength() - 1;
        dwStyle |= FX_NUMSTYLE_DotVorv;
      }
    }
    ccf++;
  }
  if (!bFindDot) {
    iDotIndex = wsPurgePattern.GetLength();
  }
  if (!pLocale) {
    pLocale = m_pLocaleMgr->GetDefLocale();
  }
  return pLocale;
}

bool CFGAS_FormatString::ParseText(const CFX_WideString& wsSrcText,
                                   const CFX_WideString& wsPattern,
                                   CFX_WideString& wsValue) {
  wsValue.clear();
  if (wsSrcText.IsEmpty() || wsPattern.IsEmpty()) {
    return false;
  }
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"text", wsTextFormat);
  if (wsTextFormat.IsEmpty()) {
    return false;
  }
  int32_t iText = 0, iPattern = 0;
  const wchar_t* pStrText = wsSrcText.c_str();
  int32_t iLenText = wsSrcText.GetLength();
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern && iText < iLenText) {
    switch (pStrPattern[iPattern]) {
      case '\'': {
        CFX_WideString wsLiteral =
            GetLiteralText(pStrPattern, iPattern, iLenPattern);
        int32_t iLiteralLen = wsLiteral.GetLength();
        if (iText + iLiteralLen > iLenText ||
            wcsncmp(pStrText + iText, wsLiteral.c_str(), iLiteralLen)) {
          wsValue = wsSrcText;
          return false;
        }
        iText += iLiteralLen;
        iPattern++;
        break;
      }
      case 'A':
        if (FXSYS_iswalpha(pStrText[iText])) {
          wsValue += pStrText[iText];
          iText++;
        }
        iPattern++;
        break;
      case 'X':
        wsValue += pStrText[iText];
        iText++;
        iPattern++;
        break;
      case 'O':
      case '0':
        if (FXSYS_isDecimalDigit(pStrText[iText]) ||
            FXSYS_iswalpha(pStrText[iText])) {
          wsValue += pStrText[iText];
          iText++;
        }
        iPattern++;
        break;
      case '9':
        if (FXSYS_isDecimalDigit(pStrText[iText])) {
          wsValue += pStrText[iText];
          iText++;
        }
        iPattern++;
        break;
      default:
        if (pStrPattern[iPattern] != pStrText[iText]) {
          wsValue = wsSrcText;
          return false;
        }
        iPattern++;
        iText++;
        break;
    }
  }
  return iPattern == iLenPattern && iText == iLenText;
}

bool CFGAS_FormatString::ParseNum(const CFX_WideString& wsSrcNum,
                                  const CFX_WideString& wsPattern,
                                  CFX_WideString& wsValue) {
  wsValue.clear();
  if (wsSrcNum.IsEmpty() || wsPattern.IsEmpty())
    return false;

  int32_t dot_index_f = -1;
  uint32_t dwFormatStyle = 0;
  CFX_WideString wsNumFormat;
  IFX_Locale* pLocale =
      GetNumericFormat(wsPattern, dot_index_f, dwFormatStyle, wsNumFormat);
  if (!pLocale || wsNumFormat.IsEmpty())
    return false;

  int32_t iExponent = 0;
  CFX_WideString wsDotSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal);
  CFX_WideString wsGroupSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping);
  int32_t iGroupLen = wsGroupSymbol.GetLength();
  CFX_WideString wsMinus = pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus);
  int32_t iMinusLen = wsMinus.GetLength();
  int cc = 0, ccf = 0;
  const wchar_t* str = wsSrcNum.c_str();
  int len = wsSrcNum.GetLength();
  const wchar_t* strf = wsNumFormat.c_str();
  int lenf = wsNumFormat.GetLength();
  bool bHavePercentSymbol = false;
  bool bNeg = false;
  bool bReverseParse = false;
  int32_t dot_index = 0;
  if (!GetNumericDotIndex(wsSrcNum, wsDotSymbol, dot_index) &&
      (dwFormatStyle & FX_NUMSTYLE_DotVorv)) {
    bReverseParse = true;
  }
  bReverseParse = false;
  ccf = dot_index_f - 1;
  cc = dot_index - 1;
  while (ccf >= 0 && cc >= 0) {
    switch (strf[ccf]) {
      case '\'': {
        CFX_WideString wsLiteral = GetLiteralTextReverse(strf, ccf);
        int32_t iLiteralLen = wsLiteral.GetLength();
        cc -= iLiteralLen - 1;
        if (cc < 0 || wcsncmp(str + cc, wsLiteral.c_str(), iLiteralLen)) {
          return false;
        }
        cc--;
        ccf--;
        break;
      }
      case '9':
        if (!FXSYS_isDecimalDigit(str[cc])) {
          return false;
        }
        wsValue = str[cc] + wsValue;
        cc--;
        ccf--;
        break;
      case 'z':
        if (FXSYS_isDecimalDigit(str[cc])) {
          wsValue = str[cc] + wsValue;
          cc--;
        }
        ccf--;
        break;
      case 'Z':
        if (str[cc] != ' ') {
          if (FXSYS_isDecimalDigit(str[cc])) {
            wsValue = str[cc] + wsValue;
            cc--;
          }
        } else {
          cc--;
        }
        ccf--;
        break;
      case 'S':
        if (str[cc] == '+' || str[cc] == ' ') {
          cc--;
        } else {
          cc -= iMinusLen - 1;
          if (cc < 0 || wcsncmp(str + cc, wsMinus.c_str(), iMinusLen)) {
            return false;
          }
          cc--;
          bNeg = true;
        }
        ccf--;
        break;
      case 's':
        if (str[cc] == '+') {
          cc--;
        } else {
          cc -= iMinusLen - 1;
          if (cc < 0 || wcsncmp(str + cc, wsMinus.c_str(), iMinusLen)) {
            return false;
          }
          cc--;
          bNeg = true;
        }
        ccf--;
        break;
      case 'E': {
        if (cc >= dot_index) {
          return false;
        }
        bool bExpSign = false;
        while (cc >= 0) {
          if (str[cc] == 'E' || str[cc] == 'e') {
            break;
          }
          if (FXSYS_isDecimalDigit(str[cc])) {
            iExponent = iExponent + (str[cc] - '0') * 10;
            cc--;
            continue;
          } else if (str[cc] == '+') {
            cc--;
            continue;
          } else if (cc - iMinusLen + 1 > 0 &&
                     !wcsncmp(str + (cc - iMinusLen + 1), wsMinus.c_str(),
                              iMinusLen)) {
            bExpSign = true;
            cc -= iMinusLen;
          } else {
            return false;
          }
        }
        cc--;
        iExponent = bExpSign ? -iExponent : iExponent;
        ccf--;
      } break;
      case '$': {
        CFX_WideString wsSymbol =
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol);
        int32_t iSymbolLen = wsSymbol.GetLength();
        cc -= iSymbolLen - 1;
        if (cc < 0 || wcsncmp(str + cc, wsSymbol.c_str(), iSymbolLen)) {
          return false;
        }
        cc--;
        ccf--;
      } break;
      case 'r':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
          if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
            bNeg = true;
            cc -= 2;
          }
          ccf -= 2;
        } else {
          ccf--;
        }
        break;
      case 'R':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
          if (str[cc] == ' ') {
            cc++;
          } else if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
            bNeg = true;
            cc -= 2;
          }
          ccf -= 2;
        } else {
          ccf--;
        }
        break;
      case 'b':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
          if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
            bNeg = true;
            cc -= 2;
          }
          ccf -= 2;
        } else {
          ccf--;
        }
        break;
      case 'B':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
          if (str[cc] == ' ') {
            cc++;
          } else if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
            bNeg = true;
            cc -= 2;
          }
          ccf -= 2;
        } else {
          ccf--;
        }
        break;
      case '.':
      case 'V':
      case 'v':
        return false;
      case '%': {
        CFX_WideString wsSymbol =
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent);
        int32_t iSysmbolLen = wsSymbol.GetLength();
        cc -= iSysmbolLen - 1;
        if (cc < 0 || wcsncmp(str + cc, wsSymbol.c_str(), iSysmbolLen)) {
          return false;
        }
        cc--;
        ccf--;
        bHavePercentSymbol = true;
      } break;
      case '8':
        return false;
      case ',': {
        if (cc >= 0) {
          cc -= iGroupLen - 1;
          if (cc >= 0 &&
              wcsncmp(str + cc, wsGroupSymbol.c_str(), iGroupLen) == 0) {
            cc--;
          } else {
            cc += iGroupLen - 1;
          }
        }
        ccf--;
      } break;
      case '(':
        if (str[cc] == L'(') {
          bNeg = true;
        } else if (str[cc] != L' ') {
          return false;
        }
        cc--;
        ccf--;
        break;
      case ')':
        if (str[cc] == L')') {
          bNeg = true;
        } else if (str[cc] != L' ') {
          return false;
        }
        cc--;
        ccf--;
        break;
      default:
        if (strf[ccf] != str[cc]) {
          return false;
        }
        cc--;
        ccf--;
    }
  }
  if (cc >= 0) {
    if (str[cc] == '-') {
      bNeg = true;
      cc--;
    }
    if (cc >= 0) {
      return false;
    }
  }
  if (dot_index < len && (dwFormatStyle & FX_NUMSTYLE_DotVorv)) {
    wsValue += '.';
  }
  if (!bReverseParse) {
    ccf = dot_index_f + 1;
    cc = (dot_index == len) ? len : dot_index + 1;
    while (cc < len && ccf < lenf) {
      switch (strf[ccf]) {
        case '\'': {
          CFX_WideString wsLiteral = GetLiteralText(strf, ccf, lenf);
          int32_t iLiteralLen = wsLiteral.GetLength();
          if (cc + iLiteralLen > len ||
              wcsncmp(str + cc, wsLiteral.c_str(), iLiteralLen)) {
            return false;
          }
          cc += iLiteralLen;
          ccf++;
          break;
        }
        case '9':
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          { wsValue += str[cc]; }
          cc++;
          ccf++;
          break;
        case 'z':
          if (FXSYS_isDecimalDigit(str[cc])) {
            wsValue += str[cc];
            cc++;
          }
          ccf++;
          break;
        case 'Z':
          if (str[cc] != ' ') {
            if (FXSYS_isDecimalDigit(str[cc])) {
              wsValue += str[cc];
              cc++;
            }
          } else {
            cc++;
          }
          ccf++;
          break;
        case 'S':
          if (str[cc] == '+' || str[cc] == ' ') {
            cc++;
          } else {
            if (cc + iMinusLen > len ||
                wcsncmp(str + cc, wsMinus.c_str(), iMinusLen)) {
              return false;
            }
            bNeg = true;
            cc += iMinusLen;
          }
          ccf++;
          break;
        case 's':
          if (str[cc] == '+') {
            cc++;
          } else {
            if (cc + iMinusLen > len ||
                wcsncmp(str + cc, wsMinus.c_str(), iMinusLen)) {
              return false;
            }
            bNeg = true;
            cc += iMinusLen;
          }
          ccf++;
          break;
        case 'E': {
          if (cc >= len || (str[cc] != 'E' && str[cc] != 'e')) {
            return false;
          }
          bool bExpSign = false;
          cc++;
          if (cc < len) {
            if (str[cc] == '+') {
              cc++;
            } else if (str[cc] == '-') {
              bExpSign = true;
              cc++;
            }
          }
          while (cc < len) {
            if (!FXSYS_isDecimalDigit(str[cc])) {
              break;
            }
            iExponent = iExponent * 10 + str[cc] - '0';
            cc++;
          }
          iExponent = bExpSign ? -iExponent : iExponent;
          ccf++;
        } break;
        case '$': {
          CFX_WideString wsSymbol =
              pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol);
          int32_t iSymbolLen = wsSymbol.GetLength();
          if (cc + iSymbolLen > len ||
              wcsncmp(str + cc, wsSymbol.c_str(), iSymbolLen)) {
            return false;
          }
          cc += iSymbolLen;
          ccf++;
        } break;
        case 'c':
          if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
            if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
              bNeg = true;
              cc += 2;
            }
            ccf += 2;
          }
          break;
        case 'C':
          if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
            if (str[cc] == ' ') {
              cc++;
            } else if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
              bNeg = true;
              cc += 2;
            }
            ccf += 2;
          }
          break;
        case 'd':
          if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
            if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
              bNeg = true;
              cc += 2;
            }
            ccf += 2;
          }
          break;
        case 'D':
          if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
            if (str[cc] == ' ') {
              cc++;
            } else if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
              bNeg = true;
              cc += 2;
            }
            ccf += 2;
          }
          break;
        case '.':
        case 'V':
        case 'v':
          return false;
        case '%': {
          CFX_WideString wsSymbol =
              pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent);
          int32_t iSysmbolLen = wsSymbol.GetLength();
          if (cc + iSysmbolLen <= len &&
              !wcsncmp(str + cc, wsSymbol.c_str(), iSysmbolLen)) {
            cc += iSysmbolLen;
          }
          ccf++;
          bHavePercentSymbol = true;
        } break;
        case '8': {
          while (ccf < lenf && strf[ccf] == '8') {
            ccf++;
          }
          while (cc < len && FXSYS_isDecimalDigit(str[cc])) {
            wsValue += str[cc];
            cc++;
          }
        } break;
        case ',': {
          if (cc + iGroupLen <= len &&
              wcsncmp(str + cc, wsGroupSymbol.c_str(), iGroupLen) == 0) {
            cc += iGroupLen;
          }
          ccf++;
        } break;
        case '(':
          if (str[cc] == L'(') {
            bNeg = true;
          } else if (str[cc] != L' ') {
            return false;
          }
          cc++;
          ccf++;
          break;
        case ')':
          if (str[cc] == L')') {
            bNeg = true;
          } else if (str[cc] != L' ') {
            return false;
          }
          cc++;
          ccf++;
          break;
        default:
          if (strf[ccf] != str[cc]) {
            return false;
          }
          cc++;
          ccf++;
      }
    }
    if (cc != len) {
      return false;
    }
  }
  if (iExponent || bHavePercentSymbol) {
    CFX_Decimal decimal = CFX_Decimal(wsValue.AsStringC());
    if (iExponent) {
      decimal = decimal *
                CFX_Decimal(FXSYS_pow(10, static_cast<float>(iExponent)), 3);
    }
    if (bHavePercentSymbol) {
      decimal = decimal / CFX_Decimal(100);
    }
    wsValue = decimal;
  }
  if (bNeg) {
    wsValue = L'-' + wsValue;
  }
  return true;
}

FX_DATETIMETYPE CFGAS_FormatString::GetDateTimeFormat(
    const CFX_WideString& wsPattern,
    IFX_Locale*& pLocale,
    CFX_WideString& wsDatePattern,
    CFX_WideString& wsTimePattern) {
  pLocale = nullptr;
  CFX_WideString wsTempPattern;
  FX_LOCALECATEGORY eCategory = FX_LOCALECATEGORY_Unknown;
  int32_t ccf = 0;
  int32_t iLenf = wsPattern.GetLength();
  const wchar_t* pStr = wsPattern.c_str();
  int32_t iFindCategory = 0;
  bool bBraceOpen = false;
  CFX_WideStringC wsConstChars(gs_wsConstChars);
  while (ccf < iLenf) {
    if (pStr[ccf] == '\'') {
      int32_t iCurChar = ccf;
      GetLiteralText(pStr, ccf, iLenf);
      wsTempPattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
    } else if (!bBraceOpen && iFindCategory != 3 &&
               wsConstChars.Find(pStr[ccf]) == -1) {
      CFX_WideString wsCategory(pStr[ccf]);
      ccf++;
      while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' &&
             pStr[ccf] != '(') {
        if (pStr[ccf] == 'T') {
          wsDatePattern = wsPattern.Left(ccf);
          wsTimePattern = wsPattern.Right(wsPattern.GetLength() - ccf);
          wsTimePattern.SetAt(0, ' ');
          if (!pLocale) {
            pLocale = m_pLocaleMgr->GetDefLocale();
          }
          return FX_DATETIMETYPE_DateTime;
        }
        wsCategory += pStr[ccf];
        ccf++;
      }
      if (!(iFindCategory & 1) && wsCategory == L"date") {
        iFindCategory |= 1;
        eCategory = FX_LOCALECATEGORY_Date;
        if (iFindCategory & 2) {
          iFindCategory = 4;
        }
      } else if (!(iFindCategory & 2) && wsCategory == L"time") {
        iFindCategory |= 2;
        eCategory = FX_LOCALECATEGORY_Time;
      } else if (wsCategory == L"datetime") {
        iFindCategory = 3;
        eCategory = FX_LOCALECATEGORY_DateTime;
      } else {
        continue;
      }
      while (ccf < iLenf) {
        if (pStr[ccf] == '(') {
          ccf++;
          CFX_WideString wsLCID;
          while (ccf < iLenf && pStr[ccf] != ')') {
            wsLCID += pStr[ccf++];
          }
          pLocale = GetPatternLocale(wsLCID);
        } else if (pStr[ccf] == '{') {
          bBraceOpen = true;
          break;
        } else if (pStr[ccf] == '.') {
          CFX_WideString wsSubCategory;
          ccf++;
          while (ccf < iLenf && pStr[ccf] != '(' && pStr[ccf] != '{') {
            wsSubCategory += pStr[ccf++];
          }
          uint32_t dwSubHash =
              FX_HashCode_GetW(wsSubCategory.AsStringC(), false);
          FX_LOCALEDATETIMESUBCATEGORY eSubCategory =
              FX_LOCALEDATETIMESUBCATEGORY_Medium;
          for (int32_t i = 0; i < g_iFXLocaleDateTimeSubCatCount; i++) {
            if (g_FXLocaleDateTimeSubCatData[i].uHash == dwSubHash) {
              eSubCategory =
                  (FX_LOCALEDATETIMESUBCATEGORY)g_FXLocaleDateTimeSubCatData[i]
                      .eSubCategory;
              break;
            }
          }
          if (!pLocale) {
            pLocale = m_pLocaleMgr->GetDefLocale();
          }
          ASSERT(pLocale);
          switch (eCategory) {
            case FX_LOCALECATEGORY_Date:
              wsDatePattern =
                  wsTempPattern + pLocale->GetDatePattern(eSubCategory);
              break;
            case FX_LOCALECATEGORY_Time:
              wsTimePattern =
                  wsTempPattern + pLocale->GetTimePattern(eSubCategory);
              break;
            case FX_LOCALECATEGORY_DateTime:
              wsDatePattern =
                  wsTempPattern + pLocale->GetDatePattern(eSubCategory);
              wsTimePattern = pLocale->GetTimePattern(eSubCategory);
              break;
            default:
              break;
          }
          wsTempPattern.clear();
          continue;
        }
        ccf++;
      }
    } else if (pStr[ccf] == '}') {
      bBraceOpen = false;
      if (!wsTempPattern.IsEmpty()) {
        if (eCategory == FX_LOCALECATEGORY_Time) {
          wsTimePattern = wsTempPattern;
        } else if (eCategory == FX_LOCALECATEGORY_Date) {
          wsDatePattern = wsTempPattern;
        }
        wsTempPattern.clear();
      }
    } else {
      wsTempPattern += pStr[ccf];
    }
    ccf++;
  }
  if (!wsTempPattern.IsEmpty()) {
    if (eCategory == FX_LOCALECATEGORY_Date) {
      wsDatePattern += wsTempPattern;
    } else {
      wsTimePattern += wsTempPattern;
    }
  }
  if (!pLocale) {
    pLocale = m_pLocaleMgr->GetDefLocale();
  }
  if (!iFindCategory) {
    wsTimePattern.clear();
    wsDatePattern = wsPattern;
  }
  return (FX_DATETIMETYPE)iFindCategory;
}

bool CFGAS_FormatString::ParseDateTime(const CFX_WideString& wsSrcDateTime,
                                       const CFX_WideString& wsPattern,
                                       FX_DATETIMETYPE eDateTimeType,
                                       CFX_DateTime* dtValue) {
  dtValue->Reset();

  if (wsSrcDateTime.IsEmpty() || wsPattern.IsEmpty()) {
    return false;
  }
  CFX_WideString wsDatePattern, wsTimePattern;
  IFX_Locale* pLocale = nullptr;
  FX_DATETIMETYPE eCategory =
      GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
  if (!pLocale) {
    return false;
  }
  if (eCategory == FX_DATETIMETYPE_Unknown) {
    eCategory = eDateTimeType;
  }
  if (eCategory == FX_DATETIMETYPE_Unknown) {
    return false;
  }
  if (eCategory == FX_DATETIMETYPE_TimeDate) {
    int32_t iStart = 0;
    if (!ParseLocaleTime(wsSrcDateTime, wsTimePattern, pLocale, dtValue,
                         iStart)) {
      return false;
    }
    if (!ParseLocaleDate(wsSrcDateTime, wsDatePattern, pLocale, dtValue,
                         iStart)) {
      return false;
    }
  } else {
    int32_t iStart = 0;
    if ((eCategory & FX_DATETIMETYPE_Date) &&
        !ParseLocaleDate(wsSrcDateTime, wsDatePattern, pLocale, dtValue,
                         iStart)) {
      return false;
    }
    if ((eCategory & FX_DATETIMETYPE_Time) &&
        !ParseLocaleTime(wsSrcDateTime, wsTimePattern, pLocale, dtValue,
                         iStart)) {
      return false;
    }
  }
  return true;
}
bool CFGAS_FormatString::ParseZero(const CFX_WideString& wsSrcText,
                                   const CFX_WideString& wsPattern) {
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"zero", wsTextFormat);
  int32_t iText = 0, iPattern = 0;
  const wchar_t* pStrText = wsSrcText.c_str();
  int32_t iLenText = wsSrcText.GetLength();
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern && iText < iLenText) {
    if (pStrPattern[iPattern] == '\'') {
      CFX_WideString wsLiteral =
          GetLiteralText(pStrPattern, iPattern, iLenPattern);
      int32_t iLiteralLen = wsLiteral.GetLength();
      if (iText + iLiteralLen > iLenText ||
          wcsncmp(pStrText + iText, wsLiteral.c_str(), iLiteralLen)) {
        return false;
      }
      iText += iLiteralLen;
      iPattern++;
      continue;
    } else if (pStrPattern[iPattern] != pStrText[iText]) {
      return false;
    } else {
      iText++;
      iPattern++;
    }
  }
  return iPattern == iLenPattern && iText == iLenText;
}
bool CFGAS_FormatString::ParseNull(const CFX_WideString& wsSrcText,
                                   const CFX_WideString& wsPattern) {
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"null", wsTextFormat);
  int32_t iText = 0, iPattern = 0;
  const wchar_t* pStrText = wsSrcText.c_str();
  int32_t iLenText = wsSrcText.GetLength();
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern && iText < iLenText) {
    if (pStrPattern[iPattern] == '\'') {
      CFX_WideString wsLiteral =
          GetLiteralText(pStrPattern, iPattern, iLenPattern);
      int32_t iLiteralLen = wsLiteral.GetLength();
      if (iText + iLiteralLen > iLenText ||
          wcsncmp(pStrText + iText, wsLiteral.c_str(), iLiteralLen)) {
        return false;
      }
      iText += iLiteralLen;
      iPattern++;
      continue;
    } else if (pStrPattern[iPattern] != pStrText[iText]) {
      return false;
    } else {
      iText++;
      iPattern++;
    }
  }
  return iPattern == iLenPattern && iText == iLenText;
}
bool CFGAS_FormatString::FormatText(const CFX_WideString& wsSrcText,
                                    const CFX_WideString& wsPattern,
                                    CFX_WideString& wsOutput) {
  if (wsPattern.IsEmpty()) {
    return false;
  }
  int32_t iLenText = wsSrcText.GetLength();
  if (iLenText == 0) {
    return false;
  }
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"text", wsTextFormat);
  int32_t iText = 0, iPattern = 0;
  const wchar_t* pStrText = wsSrcText.c_str();
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern) {
    switch (pStrPattern[iPattern]) {
      case '\'': {
        wsOutput += GetLiteralText(pStrPattern, iPattern, iLenPattern);
        iPattern++;
        break;
      }
      case 'A':
        if (iText >= iLenText || !FXSYS_iswalpha(pStrText[iText])) {
          return false;
        }
        wsOutput += pStrText[iText++];
        iPattern++;
        break;
      case 'X':
        if (iText >= iLenText) {
          return false;
        }
        wsOutput += pStrText[iText++];
        iPattern++;
        break;
      case 'O':
      case '0':
        if (iText >= iLenText || (!FXSYS_isDecimalDigit(pStrText[iText]) &&
                                  !FXSYS_iswalpha(pStrText[iText]))) {
          return false;
        }
        wsOutput += pStrText[iText++];
        iPattern++;
        break;
      case '9':
        if (iText >= iLenText || !FXSYS_isDecimalDigit(pStrText[iText])) {
          return false;
        }
        wsOutput += pStrText[iText++];
        iPattern++;
        break;
      default:
        wsOutput += pStrPattern[iPattern++];
        break;
    }
  }
  return iText == iLenText;
}

bool CFGAS_FormatString::FormatStrNum(const CFX_WideStringC& wsInputNum,
                                      const CFX_WideString& wsPattern,
                                      CFX_WideString& wsOutput) {
  if (wsInputNum.IsEmpty() || wsPattern.IsEmpty()) {
    return false;
  }
  int32_t dot_index_f = -1;
  uint32_t dwNumStyle = 0;
  CFX_WideString wsNumFormat;
  IFX_Locale* pLocale =
      GetNumericFormat(wsPattern, dot_index_f, dwNumStyle, wsNumFormat);
  if (!pLocale || wsNumFormat.IsEmpty()) {
    return false;
  }
  int32_t cc = 0, ccf = 0;
  const wchar_t* strf = wsNumFormat.c_str();
  int lenf = wsNumFormat.GetLength();
  CFX_WideString wsSrcNum(wsInputNum);
  wsSrcNum.TrimLeft('0');
  if (wsSrcNum.IsEmpty() || wsSrcNum[0] == '.') {
    wsSrcNum.Insert(0, '0');
  }
  CFX_Decimal decimal = CFX_Decimal(wsSrcNum.AsStringC());
  if (dwNumStyle & FX_NUMSTYLE_Percent) {
    decimal = decimal * CFX_Decimal(100);
    wsSrcNum = decimal;
  }
  int32_t exponent = 0;
  if (dwNumStyle & FX_NUMSTYLE_Exponent) {
    int fixed_count = 0;
    while (ccf < dot_index_f) {
      switch (strf[ccf]) {
        case '\'':
          GetLiteralText(strf, ccf, dot_index_f);
          break;
        case '9':
        case 'z':
        case 'Z':
          fixed_count++;
          break;
      }
      ccf++;
    }
    int threshold = 1;
    while (fixed_count > 1) {
      threshold *= 10;
      fixed_count--;
    }
    if (decimal != CFX_Decimal(0)) {
      if (decimal < CFX_Decimal(threshold)) {
        decimal = decimal * CFX_Decimal(10);
        exponent = -1;
        while (decimal < CFX_Decimal(threshold)) {
          decimal = decimal * CFX_Decimal(10);
          exponent -= 1;
        }
      } else if (decimal > CFX_Decimal(threshold)) {
        threshold *= 10;
        while (decimal > CFX_Decimal(threshold)) {
          decimal = decimal / CFX_Decimal(10);
          exponent += 1;
        }
      }
    }
  }
  bool bTrimTailZeros = false;
  int32_t iTreading =
      GetNumTrailingLimit(wsNumFormat, dot_index_f, bTrimTailZeros);
  int32_t scale = decimal.GetScale();
  if (iTreading < scale) {
    decimal.SetScale(iTreading);
    wsSrcNum = decimal;
  }
  if (bTrimTailZeros && scale > 0 && iTreading > 0) {
    wsSrcNum.TrimRight(L"0");
    wsSrcNum.TrimRight(L".");
  }
  CFX_WideString wsGroupSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping);
  bool bNeg = false;
  if (wsSrcNum[0] == '-') {
    bNeg = true;
    wsSrcNum.Delete(0, 1);
  }
  bool bAddNeg = false;
  const wchar_t* str = wsSrcNum.c_str();
  int len = wsSrcNum.GetLength();
  int dot_index = wsSrcNum.Find('.');
  if (dot_index == -1) {
    dot_index = len;
  }
  ccf = dot_index_f - 1;
  cc = dot_index - 1;
  while (ccf >= 0) {
    switch (strf[ccf]) {
      case '9':
        if (cc >= 0) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          wsOutput = str[cc] + wsOutput;
          cc--;
        } else {
          wsOutput = L'0' + wsOutput;
        }
        ccf--;
        break;
      case 'z':
        if (cc >= 0) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          if (str[0] != '0') {
            wsOutput = str[cc] + wsOutput;
          }
          cc--;
        }
        ccf--;
        break;
      case 'Z':
        if (cc >= 0) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          if (str[0] == '0') {
            wsOutput = L' ' + wsOutput;
          } else {
            wsOutput = str[cc] + wsOutput;
          }
          cc--;
        } else {
          wsOutput = L' ' + wsOutput;
        }
        ccf--;
        break;
      case 'S':
        if (bNeg) {
          wsOutput =
              pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;
          bAddNeg = true;
        } else {
          wsOutput = L' ' + wsOutput;
        }
        ccf--;
        break;
      case 's':
        if (bNeg) {
          wsOutput =
              pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;
          bAddNeg = true;
        }
        ccf--;
        break;
      case 'E': {
        CFX_WideString wsExp;
        wsExp.Format(L"E%+d", exponent);
        wsOutput = wsExp + wsOutput;
      }
        ccf--;
        break;
      case '$': {
        wsOutput =
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol) +
            wsOutput;
      }
        ccf--;
        break;
      case 'r':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
          if (bNeg) {
            wsOutput = L"CR" + wsOutput;
          }
          ccf -= 2;
          bAddNeg = true;
        }
        break;
      case 'R':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
          if (bNeg) {
            wsOutput = L"CR" + wsOutput;
          } else {
            wsOutput = L"  " + wsOutput;
          }
          ccf -= 2;
          bAddNeg = true;
        }
        break;
      case 'b':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
          if (bNeg) {
            wsOutput = L"db" + wsOutput;
          }
          ccf -= 2;
          bAddNeg = true;
        }
        break;
      case 'B':
        if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
          if (bNeg) {
            wsOutput = L"DB" + wsOutput;
          } else {
            wsOutput = L"  " + wsOutput;
          }
          ccf -= 2;
          bAddNeg = true;
        }
        break;
      case '%': {
        wsOutput =
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent) + wsOutput;
      }
        ccf--;
        break;
      case ',':
        if (cc >= 0) {
          wsOutput = wsGroupSymbol + wsOutput;
        }
        ccf--;
        break;
      case '(':
        if (bNeg) {
          wsOutput = L"(" + wsOutput;
        } else {
          wsOutput = L" " + wsOutput;
        }
        bAddNeg = true;
        ccf--;
        break;
      case ')':
        if (bNeg) {
          wsOutput = L")" + wsOutput;
        } else {
          wsOutput = L" " + wsOutput;
        }
        ccf--;
        break;
      case '\'':
        wsOutput = GetLiteralTextReverse(strf, ccf) + wsOutput;
        ccf--;
        break;
      default:
        wsOutput = strf[ccf] + wsOutput;
        ccf--;
    }
  }
  if (cc >= 0) {
    int nPos = dot_index % 3;
    wsOutput.clear();
    for (int32_t i = 0; i < dot_index; i++) {
      if (i % 3 == nPos && i != 0) {
        wsOutput += wsGroupSymbol;
      }
      wsOutput += wsSrcNum[i];
    }
    if (dot_index < len) {
      wsOutput += pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal);
      wsOutput += wsSrcNum.Right(len - dot_index - 1);
    }
    if (bNeg) {
      wsOutput =
          pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;
    }
    return false;
  }
  if (dot_index_f == wsNumFormat.GetLength()) {
    if (!bAddNeg && bNeg) {
      wsOutput =
          pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;
    }
    return true;
  }

  CFX_WideString wsDotSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal);
  if (strf[dot_index_f] == 'V') {
    wsOutput += wsDotSymbol;
  } else if (strf[dot_index_f] == '.') {
    if (dot_index < len) {
      wsOutput += wsDotSymbol;
    } else {
      if (strf[dot_index_f + 1] == '9' || strf[dot_index_f + 1] == 'Z') {
        wsOutput += wsDotSymbol;
      }
    }
  }
  ccf = dot_index_f + 1;
  cc = dot_index + 1;
  while (ccf < lenf) {
    switch (strf[ccf]) {
      case '\'':
        wsOutput += GetLiteralText(strf, ccf, lenf);
        ccf++;
        break;
      case '9':
        if (cc < len) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          wsOutput += str[cc];
          cc++;
        } else {
          wsOutput += L'0';
        }
        ccf++;
        break;
      case 'z':
        if (cc < len) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          wsOutput += str[cc];
          cc++;
        }
        ccf++;
        break;
      case 'Z':
        if (cc < len) {
          if (!FXSYS_isDecimalDigit(str[cc])) {
            return false;
          }
          wsOutput += str[cc];
          cc++;
        } else {
          wsOutput += L'0';
        }
        ccf++;
        break;
      case 'E': {
        CFX_WideString wsExp;
        wsExp.Format(L"E%+d", exponent);
        wsOutput += wsExp;
        ccf++;
      } break;
      case '$':
        wsOutput +=
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol);
        ccf++;
        break;
      case 'c':
        if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
          if (bNeg) {
            wsOutput += L"CR";
          }
          ccf += 2;
          bAddNeg = true;
        }
        break;
      case 'C':
        if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
          if (bNeg) {
            wsOutput += L"CR";
          } else {
            wsOutput += L"  ";
          }
          ccf += 2;
          bAddNeg = true;
        }
        break;
      case 'd':
        if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
          if (bNeg) {
            wsOutput += L"db";
          }
          ccf += 2;
          bAddNeg = true;
        }
        break;
      case 'D':
        if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
          if (bNeg) {
            wsOutput += L"DB";
          } else {
            wsOutput += L"  ";
          }
          ccf += 2;
          bAddNeg = true;
        }
        break;
      case '%':
        wsOutput += pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent);
        ccf++;
        break;
      case '8':
        while (ccf < lenf && strf[ccf] == '8')
          ccf++;
        while (cc < len && FXSYS_isDecimalDigit(str[cc])) {
          wsOutput += str[cc];
          cc++;
        }
        break;
      case ',':
        wsOutput += wsGroupSymbol;
        ccf++;
        break;
      case '(':
        if (bNeg) {
          wsOutput += '(';
        } else {
          wsOutput += ' ';
        }
        bAddNeg = true;
        ccf++;
        break;
      case ')':
        if (bNeg) {
          wsOutput += ')';
        } else {
          wsOutput += ' ';
        }
        ccf++;
        break;
      default:
        ccf++;
    }
  }
  if (!bAddNeg && bNeg) {
    wsOutput = pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) +
               wsOutput[0] + wsOutput.Mid(1, wsOutput.GetLength() - 1);
  }
  return true;
}

bool CFGAS_FormatString::FormatNum(const CFX_WideString& wsSrcNum,
                                   const CFX_WideString& wsPattern,
                                   CFX_WideString& wsOutput) {
  if (wsSrcNum.IsEmpty() || wsPattern.IsEmpty()) {
    return false;
  }
  return FormatStrNum(wsSrcNum.AsStringC(), wsPattern, wsOutput);
}

bool CFGAS_FormatString::FormatDateTime(const CFX_WideString& wsSrcDateTime,
                                        const CFX_WideString& wsPattern,
                                        CFX_WideString& wsOutput,
                                        FX_DATETIMETYPE eDateTimeType) {
  if (wsSrcDateTime.IsEmpty() || wsPattern.IsEmpty()) {
    return false;
  }
  CFX_WideString wsDatePattern, wsTimePattern;
  IFX_Locale* pLocale = nullptr;
  FX_DATETIMETYPE eCategory =
      GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
  if (!pLocale) {
    return false;
  }
  if (eCategory == FX_DATETIMETYPE_Unknown) {
    if (eDateTimeType == FX_DATETIMETYPE_Time) {
      wsTimePattern = wsDatePattern;
      wsDatePattern.clear();
    }
    eCategory = eDateTimeType;
  }
  if (eCategory == FX_DATETIMETYPE_Unknown) {
    return false;
  }
  CFX_DateTime dt;
  int32_t iT = wsSrcDateTime.Find(L"T");
  if (iT < 0) {
    if (eCategory == FX_DATETIMETYPE_Date &&
        FX_DateFromCanonical(wsSrcDateTime, &dt)) {
      return FormatDateTimeInternal(dt, wsDatePattern, wsTimePattern, true,
                                    pLocale, wsOutput);
    }
    if (eCategory == FX_DATETIMETYPE_Time &&
        FX_TimeFromCanonical(wsSrcDateTime.AsStringC(), &dt, pLocale)) {
      return FormatDateTimeInternal(dt, wsDatePattern, wsTimePattern, true,
                                    pLocale, wsOutput);
    }
  } else {
    CFX_WideString wsSrcDate(wsSrcDateTime.c_str(), iT);
    CFX_WideStringC wsSrcTime(wsSrcDateTime.c_str() + iT + 1,
                              wsSrcDateTime.GetLength() - iT - 1);
    if (wsSrcDate.IsEmpty() || wsSrcTime.IsEmpty())
      return false;

    if (FX_DateFromCanonical(wsSrcDate, &dt) &&
        FX_TimeFromCanonical(wsSrcTime, &dt, pLocale)) {
      return FormatDateTimeInternal(dt, wsDatePattern, wsTimePattern,
                                    eCategory != FX_DATETIMETYPE_TimeDate,
                                    pLocale, wsOutput);
    }
  }
  return false;
}

bool CFGAS_FormatString::FormatZero(const CFX_WideString& wsPattern,
                                    CFX_WideString& wsOutput) {
  if (wsPattern.IsEmpty()) {
    return false;
  }
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"zero", wsTextFormat);
  int32_t iPattern = 0;
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern) {
    if (pStrPattern[iPattern] == '\'') {
      wsOutput += GetLiteralText(pStrPattern, iPattern, iLenPattern);
      iPattern++;
      continue;
    } else {
      wsOutput += pStrPattern[iPattern++];
      continue;
    }
  }
  return true;
}
bool CFGAS_FormatString::FormatNull(const CFX_WideString& wsPattern,
                                    CFX_WideString& wsOutput) {
  if (wsPattern.IsEmpty()) {
    return false;
  }
  CFX_WideString wsTextFormat;
  GetTextFormat(wsPattern, L"null", wsTextFormat);
  int32_t iPattern = 0;
  const wchar_t* pStrPattern = wsTextFormat.c_str();
  int32_t iLenPattern = wsTextFormat.GetLength();
  while (iPattern < iLenPattern) {
    if (pStrPattern[iPattern] == '\'') {
      wsOutput += GetLiteralText(pStrPattern, iPattern, iLenPattern);
      iPattern++;
      continue;
    } else {
      wsOutput += pStrPattern[iPattern++];
      continue;
    }
  }
  return true;
}

IFX_Locale* CFGAS_FormatString::GetPatternLocale(
    const CFX_WideString& wsLocale) {
  return m_pLocaleMgr->GetLocaleByName(wsLocale);
}
