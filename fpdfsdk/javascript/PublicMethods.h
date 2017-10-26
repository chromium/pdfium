// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_PUBLICMETHODS_H_
#define FPDFSDK_JAVASCRIPT_PUBLICMETHODS_H_

#include <string>
#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"

class CJS_PublicMethods : public CJS_Object {
 public:
  explicit CJS_PublicMethods(v8::Local<v8::Object> pObject)
      : CJS_Object(pObject) {}
  ~CJS_PublicMethods() override {}

  static CJS_Return AFNumber_Format(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFNumber_Keystroke(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFPercent_Format(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFPercent_Keystroke(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFDate_FormatEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFDate_KeystrokeEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFDate_Format(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFDate_Keystroke(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFTime_FormatEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFTime_KeystrokeEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFTime_Format(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFTime_Keystroke(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFSpecial_Format(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFSpecial_Keystroke(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFSpecial_KeystrokeEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFSimple(CJS_Runtime* pRuntime,
                             const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFMakeNumber(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFSimple_Calculate(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFRange_Validate(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFMergeChange(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFParseDateEx(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  static CJS_Return AFExtractNums(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);

  static void AFNumber_Format_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFNumber_Keystroke_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFPercent_Format_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFPercent_Keystroke_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFDate_FormatEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFDate_KeystrokeEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFDate_Format_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFDate_Keystroke_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFTime_FormatEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFTime_KeystrokeEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFTime_Format_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFTime_Keystroke_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFSpecial_Format_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFSpecial_Keystroke_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFSpecial_KeystrokeEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFSimple_static(const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFMakeNumber_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFSimple_Calculate_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFRange_Validate_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFMergeChange_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFParseDateEx_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void AFExtractNums_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  static const JSMethodSpec GlobalFunctionSpecs[];
  static void DefineJSObjects(CFXJS_Engine* pEngine);
  static int ParseStringInteger(const WideString& string,
                                size_t nStart,
                                size_t& nSkip,
                                size_t nMaxStep);
  static WideString ParseStringString(const WideString& string,
                                      size_t nStart,
                                      size_t& nSkip);
  static double MakeRegularDate(const WideString& value,
                                const WideString& format,
                                bool* bWrongFormat);
  static WideString MakeFormatDate(double dDate, const WideString& format);
  static double ParseNormalDate(const WideString& value, bool* bWrongFormat);
  static double MakeInterDate(const WideString& value);

  static bool IsNumber(const WideString& str);

  static bool maskSatisfied(wchar_t c_Change, wchar_t c_Mask);
  static bool isReservedMaskChar(wchar_t ch);

  static double AF_Simple(const wchar_t* sFuction,
                          double dValue1,
                          double dValue2);
  static v8::Local<v8::Array> AF_MakeArrayFromList(CJS_Runtime* pRuntime,
                                                   v8::Local<v8::Value> val);
};

#endif  // FPDFSDK_JAVASCRIPT_PUBLICMETHODS_H_
