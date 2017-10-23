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

  static bool AFNumber_Format(CJS_Runtime* pRuntime,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              WideString& sError);
  static bool AFNumber_Keystroke(CJS_Runtime* pRuntime,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 WideString& sError);
  static bool AFPercent_Format(CJS_Runtime* pRuntime,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               WideString& sError);
  static bool AFPercent_Keystroke(CJS_Runtime* pRuntime,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  WideString& sError);
  static bool AFDate_FormatEx(CJS_Runtime* pRuntime,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              WideString& sError);
  static bool AFDate_KeystrokeEx(CJS_Runtime* pRuntime,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 WideString& sError);
  static bool AFDate_Format(CJS_Runtime* pRuntime,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            WideString& sError);
  static bool AFDate_Keystroke(CJS_Runtime* pRuntime,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               WideString& sError);
  static bool AFTime_FormatEx(CJS_Runtime* pRuntime,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              WideString& sError);  //
  static bool AFTime_KeystrokeEx(CJS_Runtime* pRuntime,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 WideString& sError);
  static bool AFTime_Format(CJS_Runtime* pRuntime,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            WideString& sError);
  static bool AFTime_Keystroke(CJS_Runtime* pRuntime,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               WideString& sError);
  static bool AFSpecial_Format(CJS_Runtime* pRuntime,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               WideString& sError);
  static bool AFSpecial_Keystroke(CJS_Runtime* pRuntime,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  WideString& sError);
  static bool AFSpecial_KeystrokeEx(CJS_Runtime* pRuntime,
                                    const std::vector<CJS_Value>& params,
                                    CJS_Value& vRet,
                                    WideString& sError);  //
  static bool AFSimple(CJS_Runtime* pRuntime,
                       const std::vector<CJS_Value>& params,
                       CJS_Value& vRet,
                       WideString& sError);
  static bool AFMakeNumber(CJS_Runtime* pRuntime,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           WideString& sError);
  static bool AFSimple_Calculate(CJS_Runtime* pRuntime,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 WideString& sError);
  static bool AFRange_Validate(CJS_Runtime* pRuntime,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               WideString& sError);
  static bool AFMergeChange(CJS_Runtime* pRuntime,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            WideString& sError);
  static bool AFParseDateEx(CJS_Runtime* pRuntime,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            WideString& sError);
  static bool AFExtractNums(CJS_Runtime* pRuntime,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            WideString& sError);

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

  static JSMethodSpec GlobalFunctionSpecs[];
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
  static CJS_Array AF_MakeArrayFromList(CJS_Runtime* pRuntime, CJS_Value val);
};

#endif  // FPDFSDK_JAVASCRIPT_PUBLICMETHODS_H_
