// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/util.h"

#include <time.h>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <string>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/PublicMethods.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_eventhandler.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"

#if _FX_OS_ == _FX_OS_ANDROID_
#include <ctype.h>
#endif

JSConstSpec CJS_Util::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Util::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Util::MethodSpecs[] = {
    {"printd", printd_static},         {"printf", printf_static},
    {"printx", printx_static},         {"scand", scand_static},
    {"byteToChar", byteToChar_static}, {0, 0}};

const char* CJS_Util::g_pClassName = "util";
int CJS_Util::g_nObjDefnID = -1;

void CJS_Util::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_Util::JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  CJS_Object* pObj = new CJS_Util(obj);
  pObj->SetEmbedObject(new util(pObj));
  pEngine->SetObjectPrivate(obj, pObj);
  pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));
}

void CJS_Util::JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  delete static_cast<CJS_Util*>(pEngine->GetObjectPrivate(obj));
}

void CJS_Util::DefineProps(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(PropertySpecs) - 1; ++i) {
    pEngine->DefineObjProperty(g_nObjDefnID, PropertySpecs[i].pName,
                               PropertySpecs[i].pPropGet,
                               PropertySpecs[i].pPropPut);
  }
}

void CJS_Util::DefineMethods(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(MethodSpecs) - 1; ++i) {
    pEngine->DefineObjMethod(g_nObjDefnID, MethodSpecs[i].pName,
                             MethodSpecs[i].pMethodCall);
  }
}

void CJS_Util::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj(CJS_Util::g_pClassName, eObjType,
                                    JSConstructor, JSDestructor);
  DefineConsts(pEngine);
  DefineProps(pEngine);
  DefineMethods(pEngine);
}

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

CJS_Return util::printf(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params) {
  const size_t iSize = params.size();
  if (iSize < 1)
    return CJS_Return(false);

  std::wstring unsafe_fmt_string(pRuntime->ToWideString(params[0]).c_str());
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

    WideString strSegment;
    switch (ParseDataType(&c_strFormat)) {
      case UTIL_INT:
        strSegment.Format(c_strFormat.c_str(),
                          pRuntime->ToInt32(params[iIndex]));
        break;
      case UTIL_DOUBLE:
        strSegment.Format(c_strFormat.c_str(),
                          pRuntime->ToDouble(params[iIndex]));
        break;
      case UTIL_STRING:
        strSegment.Format(c_strFormat.c_str(),
                          pRuntime->ToWideString(params[iIndex]).c_str());
        break;
      default:
        strSegment.Format(L"%ls", c_strFormat.c_str());
        break;
    }
    c_strResult += strSegment.c_str();
  }

  c_strResult.erase(c_strResult.begin());
  return CJS_Return(pRuntime->NewString(c_strResult.c_str()));
}

CJS_Return util::printd(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params) {
  const size_t iSize = params.size();
  if (iSize < 2)
    return CJS_Return(false);

  if (params[1].IsEmpty() || !params[1]->IsDate())
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSPRINT1));

  v8::Local<v8::Date> v8_date = params[1].As<v8::Date>();
  if (v8_date.IsEmpty() || std::isnan(pRuntime->ToDouble(v8_date)))
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSPRINT2));

  double date = JS_LocalTime(pRuntime->ToDouble(v8_date));
  int year = JS_GetYearFromTime(date);
  int month = JS_GetMonthFromTime(date) + 1;  // One-based.
  int day = JS_GetDayFromTime(date);
  int hour = JS_GetHourFromTime(date);
  int min = JS_GetMinFromTime(date);
  int sec = JS_GetSecFromTime(date);

  if (params[0]->IsNumber()) {
    WideString swResult;
    switch (pRuntime->ToInt32(params[0])) {
      case 0:
        swResult.Format(L"D:%04d%02d%02d%02d%02d%02d", year, month, day, hour,
                        min, sec);
        break;
      case 1:
        swResult.Format(L"%04d.%02d.%02d %02d:%02d:%02d", year, month, day,
                        hour, min, sec);
        break;
      case 2:
        swResult.Format(L"%04d/%02d/%02d %02d:%02d:%02d", year, month, day,
                        hour, min, sec);
        break;
      default:
        return CJS_Return(JSGetStringFromID(IDS_STRING_JSVALUEERROR));
    }

    return CJS_Return(pRuntime->NewString(swResult.c_str()));
  }

  if (params[0]->IsString()) {
    // We don't support XFAPicture at the moment.
    if (iSize > 2 && pRuntime->ToBoolean(params[2]))
      return CJS_Return(JSGetStringFromID(IDS_STRING_JSNOTSUPPORT));

    // Convert PDF-style format specifiers to wcsftime specifiers. Remove any
    // pre-existing %-directives before inserting our own.
    std::basic_string<wchar_t> cFormat =
        pRuntime->ToWideString(params[0]).c_str();
    cFormat.erase(std::remove(cFormat.begin(), cFormat.end(), '%'),
                  cFormat.end());

    for (size_t i = 0; i < FX_ArraySize(TbConvertTable); ++i) {
      int iStart = 0;
      int iEnd;
      while ((iEnd = cFormat.find(TbConvertTable[i].lpszJSMark, iStart)) !=
             -1) {
        cFormat.replace(iEnd, wcslen(TbConvertTable[i].lpszJSMark),
                        TbConvertTable[i].lpszCppMark);
        iStart = iEnd;
      }
    }

    if (year < 0)
      return CJS_Return(JSGetStringFromID(IDS_STRING_JSVALUEERROR));

    static const TbConvertAdditional cTableAd[] = {
        {L"m", month}, {L"d", day},
        {L"H", hour},  {L"h", hour > 12 ? hour - 12 : hour},
        {L"M", min},   {L"s", sec},
    };

    for (size_t i = 0; i < FX_ArraySize(cTableAd); ++i) {
      WideString sValue;
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
        cFormat.replace(iEnd, wcslen(cTableAd[i].lpszJSMark), sValue.c_str());
        iStart = iEnd;
      }
    }

    struct tm time = {};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_hour = hour;
    time.tm_min = min;
    time.tm_sec = sec;

    wchar_t buf[64] = {};
    FXSYS_wcsftime(buf, 64, cFormat.c_str(), &time);
    cFormat = buf;
    return CJS_Return(pRuntime->NewString(cFormat.c_str()));
  }

  return CJS_Return(JSGetStringFromID(IDS_STRING_JSTYPEERROR));
}

CJS_Return util::printx(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() < 2)
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSPARAMERROR));

  return CJS_Return(
      pRuntime->NewString(printx(pRuntime->ToWideString(params[0]),
                                 pRuntime->ToWideString(params[1]))
                              .c_str()));
}

enum CaseMode { kPreserveCase, kUpperCase, kLowerCase };

static wchar_t TranslateCase(wchar_t input, CaseMode eMode) {
  if (eMode == kLowerCase && FXSYS_isupper(input))
    return input | 0x20;
  if (eMode == kUpperCase && FXSYS_islower(input))
    return input & ~0x20;
  return input;
}

WideString util::printx(const WideString& wsFormat,
                        const WideString& wsSource) {
  WideString wsResult;
  size_t iSourceIdx = 0;
  size_t iFormatIdx = 0;
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

CJS_Return util::scand(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() < 2)
    return CJS_Return(false);

  WideString sFormat = pRuntime->ToWideString(params[0]);
  WideString sDate = pRuntime->ToWideString(params[1]);
  double dDate = JS_GetDateTime();
  if (sDate.GetLength() > 0)
    dDate = CJS_PublicMethods::MakeRegularDate(sDate, sFormat, nullptr);

  if (std::isnan(dDate))
    return CJS_Return(pRuntime->NewUndefined());
  return CJS_Return(pRuntime->NewDate(dDate));
}

CJS_Return util::byteToChar(CJS_Runtime* pRuntime,
                            const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() < 1)
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSPARAMERROR));

  int arg = pRuntime->ToInt32(params[0]);
  if (arg < 0 || arg > 255)
    return CJS_Return(JSGetStringFromID(IDS_STRING_JSVALUEERROR));

  WideString wStr(static_cast<wchar_t>(arg));
  return CJS_Return(pRuntime->NewString(wStr.c_str()));
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
