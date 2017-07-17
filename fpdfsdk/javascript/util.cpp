// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/util.h"

#include <time.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/PublicMethods.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"

#if _FX_OS_ == _FX_ANDROID_
#include <ctype.h>
#endif

JSConstSpec CJS_Util::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Util::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Util::MethodSpecs[] = {
    {"printd", printd_static},         {"printf", printf_static},
    {"printx", printx_static},         {"scand", scand_static},
    {"byteToChar", byteToChar_static}, {0, 0}};

IMPLEMENT_JS_CLASS(CJS_Util, util)

namespace {

// Map PDF-style directives to equivalent wcsftime directives. Not
// all have direct equivalents, though.
struct TbConvert {
  const wchar_t* lpszJSMark;
  const wchar_t* lpszCppMark;
};

// Map PDF-style directives lacking direct wcsftime directives to
// the value with which they will be replaced.
struct TbConvertAdditional {
  const wchar_t* lpszJSMark;
  int iValue;
};

const TbConvert TbConvertTable[] = {
    {L"mmmm", L"%B"}, {L"mmm", L"%b"}, {L"mm", L"%m"},   {L"dddd", L"%A"},
    {L"ddd", L"%a"},  {L"dd", L"%d"},  {L"yyyy", L"%Y"}, {L"yy", L"%y"},
    {L"HH", L"%H"},   {L"hh", L"%I"},  {L"MM", L"%M"},   {L"ss", L"%S"},
    {L"TT", L"%p"},
#if defined(_WIN32)
    {L"tt", L"%p"},   {L"h", L"%#I"},
#else
    {L"tt", L"%P"},   {L"h", L"%l"},
#endif
};

}  // namespace

util::util(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {}

util::~util() {}

bool util::printf(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  CFX_WideString& sError) {
  const size_t iSize = params.size();
  if (iSize < 1)
    return false;

  std::wstring unsafe_fmt_string(params[0].ToCFXWideString(pRuntime).c_str());
  std::vector<std::wstring> unsafe_conversion_specifiers;
  int iOffset = 0;
  int iOffend = 0;
  unsafe_fmt_string.insert(unsafe_fmt_string.begin(), L'S');
  while (iOffset != -1) {
    iOffend = unsafe_fmt_string.find(L"%", iOffset + 1);
    std::wstring strSub;
    if (iOffend == -1)
      strSub = unsafe_fmt_string.substr(iOffset);
    else
      strSub = unsafe_fmt_string.substr(iOffset, iOffend - iOffset);
    unsafe_conversion_specifiers.push_back(strSub);
    iOffset = iOffend;
  }

  std::wstring c_strResult;
  for (size_t iIndex = 0; iIndex < unsafe_conversion_specifiers.size();
       ++iIndex) {
    std::wstring c_strFormat = unsafe_conversion_specifiers[iIndex];
    if (iIndex == 0) {
      c_strResult = c_strFormat;
      continue;
    }

    if (iIndex >= iSize) {
      c_strResult += c_strFormat;
      continue;
    }

    CFX_WideString strSegment;
    switch (ParseDataType(&c_strFormat)) {
      case UTIL_INT:
        strSegment.Format(c_strFormat.c_str(), params[iIndex].ToInt(pRuntime));
        break;
      case UTIL_DOUBLE:
        strSegment.Format(c_strFormat.c_str(),
                          params[iIndex].ToDouble(pRuntime));
        break;
      case UTIL_STRING:
        strSegment.Format(c_strFormat.c_str(),
                          params[iIndex].ToCFXWideString(pRuntime).c_str());
        break;
      default:
        strSegment.Format(L"%ls", c_strFormat.c_str());
        break;
    }
    c_strResult += strSegment.c_str();
  }

  c_strResult.erase(c_strResult.begin());
  vRet = CJS_Value(pRuntime, c_strResult.c_str());
  return true;
}

bool util::printd(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  CFX_WideString& sError) {
  const size_t iSize = params.size();
  if (iSize < 2)
    return false;

  const CJS_Value& p1 = params[0];
  const CJS_Value& p2 = params[1];
  CJS_Date jsDate;
  if (!p2.ConvertToDate(pRuntime, jsDate)) {
    sError = JSGetStringFromID(IDS_STRING_JSPRINT1);
    return false;
  }

  if (!jsDate.IsValidDate(pRuntime)) {
    sError = JSGetStringFromID(IDS_STRING_JSPRINT2);
    return false;
  }

  if (p1.GetType() == CJS_Value::VT_number) {
    CFX_WideString swResult;
    switch (p1.ToInt(pRuntime)) {
      case 0:
        swResult.Format(L"D:%04d%02d%02d%02d%02d%02d", jsDate.GetYear(pRuntime),
                        jsDate.GetMonth(pRuntime) + 1, jsDate.GetDay(pRuntime),
                        jsDate.GetHours(pRuntime), jsDate.GetMinutes(pRuntime),
                        jsDate.GetSeconds(pRuntime));
        break;
      case 1:
        swResult.Format(L"%04d.%02d.%02d %02d:%02d:%02d",
                        jsDate.GetYear(pRuntime), jsDate.GetMonth(pRuntime) + 1,
                        jsDate.GetDay(pRuntime), jsDate.GetHours(pRuntime),
                        jsDate.GetMinutes(pRuntime),
                        jsDate.GetSeconds(pRuntime));
        break;
      case 2:
        swResult.Format(L"%04d/%02d/%02d %02d:%02d:%02d",
                        jsDate.GetYear(pRuntime), jsDate.GetMonth(pRuntime) + 1,
                        jsDate.GetDay(pRuntime), jsDate.GetHours(pRuntime),
                        jsDate.GetMinutes(pRuntime),
                        jsDate.GetSeconds(pRuntime));
        break;
      default:
        sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
        return false;
    }

    vRet = CJS_Value(pRuntime, swResult.c_str());
    return true;
  }

  if (p1.GetType() == CJS_Value::VT_string) {
    if (iSize > 2 && params[2].ToBool(pRuntime)) {
      sError = JSGetStringFromID(IDS_STRING_JSNOTSUPPORT);
      return false;  // currently, it doesn't support XFAPicture.
    }

    // Convert PDF-style format specifiers to wcsftime specifiers. Remove any
    // pre-existing %-directives before inserting our own.
    std::basic_string<wchar_t> cFormat = p1.ToCFXWideString(pRuntime).c_str();
    cFormat.erase(std::remove(cFormat.begin(), cFormat.end(), '%'),
                  cFormat.end());

    for (size_t i = 0; i < FX_ArraySize(TbConvertTable); ++i) {
      int iStart = 0;
      int iEnd;
      while ((iEnd = cFormat.find(TbConvertTable[i].lpszJSMark, iStart)) !=
             -1) {
        cFormat.replace(iEnd, FXSYS_wcslen(TbConvertTable[i].lpszJSMark),
                        TbConvertTable[i].lpszCppMark);
        iStart = iEnd;
      }
    }

    int iYear = jsDate.GetYear(pRuntime);
    if (iYear < 0) {
      sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
      return false;
    }

    int iMonth = jsDate.GetMonth(pRuntime);
    int iDay = jsDate.GetDay(pRuntime);
    int iHour = jsDate.GetHours(pRuntime);
    int iMin = jsDate.GetMinutes(pRuntime);
    int iSec = jsDate.GetSeconds(pRuntime);

    static const TbConvertAdditional cTableAd[] = {
        {L"m", iMonth + 1}, {L"d", iDay},
        {L"H", iHour},      {L"h", iHour > 12 ? iHour - 12 : iHour},
        {L"M", iMin},       {L"s", iSec},
    };

    for (size_t i = 0; i < FX_ArraySize(cTableAd); ++i) {
      CFX_WideString sValue;
      sValue.Format(L"%d", cTableAd[i].iValue);

      int iStart = 0;
      int iEnd;
      while ((iEnd = cFormat.find(cTableAd[i].lpszJSMark, iStart)) != -1) {
        if (iEnd > 0) {
          if (cFormat[iEnd - 1] == L'%') {
            iStart = iEnd + 1;
            continue;
          }
        }
        cFormat.replace(iEnd, FXSYS_wcslen(cTableAd[i].lpszJSMark),
                        sValue.c_str());
        iStart = iEnd;
      }
    }

    struct tm time = {};
    time.tm_year = iYear - 1900;
    time.tm_mon = iMonth;
    time.tm_mday = iDay;
    time.tm_hour = iHour;
    time.tm_min = iMin;
    time.tm_sec = iSec;

    wchar_t buf[64] = {};
    FXSYS_wcsftime(buf, 64, cFormat.c_str(), &time);
    cFormat = buf;
    vRet = CJS_Value(pRuntime, cFormat.c_str());
    return true;
  }

  sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
  return false;
}

bool util::printx(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  CFX_WideString& sError) {
  if (params.size() < 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  vRet = CJS_Value(pRuntime, printx(params[0].ToCFXWideString(pRuntime),
                                    params[1].ToCFXWideString(pRuntime))
                                 .c_str());

  return true;
}

enum CaseMode { kPreserveCase, kUpperCase, kLowerCase };

static wchar_t TranslateCase(wchar_t input, CaseMode eMode) {
  if (eMode == kLowerCase && FXSYS_isupper(input))
    return input | 0x20;
  if (eMode == kUpperCase && FXSYS_islower(input))
    return input & ~0x20;
  return input;
}

CFX_WideString util::printx(const CFX_WideString& wsFormat,
                            const CFX_WideString& wsSource) {
  CFX_WideString wsResult;
  FX_STRSIZE iSourceIdx = 0;
  FX_STRSIZE iFormatIdx = 0;
  CaseMode eCaseMode = kPreserveCase;
  bool bEscaped = false;
  while (iFormatIdx < wsFormat.GetLength()) {
    if (bEscaped) {
      bEscaped = false;
      wsResult += wsFormat[iFormatIdx];
      ++iFormatIdx;
      continue;
    }
    switch (wsFormat[iFormatIdx]) {
      case '\\': {
        bEscaped = true;
        ++iFormatIdx;
      } break;
      case '<': {
        eCaseMode = kLowerCase;
        ++iFormatIdx;
      } break;
      case '>': {
        eCaseMode = kUpperCase;
        ++iFormatIdx;
      } break;
      case '=': {
        eCaseMode = kPreserveCase;
        ++iFormatIdx;
      } break;
      case '?': {
        if (iSourceIdx < wsSource.GetLength()) {
          wsResult += TranslateCase(wsSource[iSourceIdx], eCaseMode);
          ++iSourceIdx;
        }
        ++iFormatIdx;
      } break;
      case 'X': {
        if (iSourceIdx < wsSource.GetLength()) {
          if (FXSYS_iswalnum(wsSource[iSourceIdx])) {
            wsResult += TranslateCase(wsSource[iSourceIdx], eCaseMode);
            ++iFormatIdx;
          }
          ++iSourceIdx;
        } else {
          ++iFormatIdx;
        }
      } break;
      case 'A': {
        if (iSourceIdx < wsSource.GetLength()) {
          if (FXSYS_iswalpha(wsSource[iSourceIdx])) {
            wsResult += TranslateCase(wsSource[iSourceIdx], eCaseMode);
            ++iFormatIdx;
          }
          ++iSourceIdx;
        } else {
          ++iFormatIdx;
        }
      } break;
      case '9': {
        if (iSourceIdx < wsSource.GetLength()) {
          if (std::iswdigit(wsSource[iSourceIdx])) {
            wsResult += wsSource[iSourceIdx];
            ++iFormatIdx;
          }
          ++iSourceIdx;
        } else {
          ++iFormatIdx;
        }
      } break;
      case '*': {
        if (iSourceIdx < wsSource.GetLength()) {
          wsResult += TranslateCase(wsSource[iSourceIdx], eCaseMode);
          ++iSourceIdx;
        } else {
          ++iFormatIdx;
        }
      } break;
      default: {
        wsResult += wsFormat[iFormatIdx];
        ++iFormatIdx;
      } break;
    }
  }
  return wsResult;
}

bool util::scand(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 CFX_WideString& sError) {
  if (params.size() < 2)
    return false;

  CFX_WideString sFormat = params[0].ToCFXWideString(pRuntime);
  CFX_WideString sDate = params[1].ToCFXWideString(pRuntime);
  double dDate = JS_GetDateTime();
  if (sDate.GetLength() > 0) {
    dDate = CJS_PublicMethods::MakeRegularDate(sDate, sFormat, nullptr);
  }

  if (!JS_PortIsNan(dDate)) {
    vRet = CJS_Value(pRuntime, CJS_Date(pRuntime, dDate));
  } else {
    vRet.SetNull(pRuntime);
  }

  return true;
}

bool util::byteToChar(CJS_Runtime* pRuntime,
                      const std::vector<CJS_Value>& params,
                      CJS_Value& vRet,
                      CFX_WideString& sError) {
  if (params.size() < 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  int arg = params[0].ToInt(pRuntime);
  if (arg < 0 || arg > 255) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return false;
  }

  CFX_WideString wStr(static_cast<wchar_t>(arg));
  vRet = CJS_Value(pRuntime, wStr.c_str());
  return true;
}

// Ensure that sFormat contains at most one well-understood printf formatting
// directive which is safe to use with a single argument, and return the type
// of argument expected, or -1 otherwise. If -1 is returned, it is NOT safe
// to use sFormat with printf() and it must be copied byte-by-byte.
int util::ParseDataType(std::wstring* sFormat) {
  enum State { BEFORE, FLAGS, WIDTH, PRECISION, SPECIFIER, AFTER };

  int result = -1;
  State state = BEFORE;
  size_t precision_digits = 0;
  size_t i = 0;
  while (i < sFormat->length()) {
    wchar_t c = (*sFormat)[i];
    switch (state) {
      case BEFORE:
        if (c == L'%')
          state = FLAGS;
        break;
      case FLAGS:
        if (c == L'+' || c == L'-' || c == L'#' || c == L' ') {
          // Stay in same state.
        } else {
          state = WIDTH;
          continue;  // Re-process same character.
        }
        break;
      case WIDTH:
        if (c == L'*')
          return -1;
        if (std::iswdigit(c)) {
          // Stay in same state.
        } else if (c == L'.') {
          state = PRECISION;
        } else {
          state = SPECIFIER;
          continue;  // Re-process same character.
        }
        break;
      case PRECISION:
        if (c == L'*')
          return -1;
        if (std::iswdigit(c)) {
          // Stay in same state.
          ++precision_digits;
        } else {
          state = SPECIFIER;
          continue;  // Re-process same character.
        }
        break;
      case SPECIFIER:
        if (c == L'c' || c == L'C' || c == L'd' || c == L'i' || c == L'o' ||
            c == L'u' || c == L'x' || c == L'X') {
          result = UTIL_INT;
        } else if (c == L'e' || c == L'E' || c == L'f' || c == L'g' ||
                   c == L'G') {
          result = UTIL_DOUBLE;
        } else if (c == L's' || c == L'S') {
          // Map s to S since we always deal internally with wchar_t strings.
          // TODO(tsepez): Probably 100% borked. %S is not a standard
          // conversion.
          (*sFormat)[i] = L'S';
          result = UTIL_STRING;
        } else {
          return -1;
        }
        state = AFTER;
        break;
      case AFTER:
        if (c == L'%')
          return -1;
        // Stay in same state until string exhausted.
        break;
    }
    ++i;
  }
  // See https://crbug.com/740166
  if (result == UTIL_INT && precision_digits > 2)
    return -1;

  return result;
}
