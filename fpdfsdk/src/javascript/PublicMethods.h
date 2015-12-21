// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_SRC_JAVASCRIPT_PUBLICMETHODS_H_
#define FPDFSDK_SRC_JAVASCRIPT_PUBLICMETHODS_H_

#include "JS_Define.h"

class CJS_PublicMethods : public CJS_Object {
 public:
  explicit CJS_PublicMethods(v8::Local<v8::Object> pObject)
      : CJS_Object(pObject) {}
  ~CJS_PublicMethods() override {}

 public:
  static FX_BOOL AFNumber_Format(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError);
  static FX_BOOL AFNumber_Keystroke(IJS_Context* cc,
                                    const std::vector<CJS_Value>& params,
                                    CJS_Value& vRet,
                                    CFX_WideString& sError);
  static FX_BOOL AFPercent_Format(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError);
  static FX_BOOL AFPercent_Keystroke(IJS_Context* cc,
                                     const std::vector<CJS_Value>& params,
                                     CJS_Value& vRet,
                                     CFX_WideString& sError);
  static FX_BOOL AFDate_FormatEx(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError);
  static FX_BOOL AFDate_KeystrokeEx(IJS_Context* cc,
                                    const std::vector<CJS_Value>& params,
                                    CJS_Value& vRet,
                                    CFX_WideString& sError);
  static FX_BOOL AFDate_Format(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError);
  static FX_BOOL AFDate_Keystroke(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError);
  static FX_BOOL AFTime_FormatEx(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError);  //
  static FX_BOOL AFTime_KeystrokeEx(IJS_Context* cc,
                                    const std::vector<CJS_Value>& params,
                                    CJS_Value& vRet,
                                    CFX_WideString& sError);
  static FX_BOOL AFTime_Format(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError);
  static FX_BOOL AFTime_Keystroke(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError);
  static FX_BOOL AFSpecial_Format(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError);
  static FX_BOOL AFSpecial_Keystroke(IJS_Context* cc,
                                     const std::vector<CJS_Value>& params,
                                     CJS_Value& vRet,
                                     CFX_WideString& sError);
  static FX_BOOL AFSpecial_KeystrokeEx(IJS_Context* cc,
                                       const std::vector<CJS_Value>& params,
                                       CJS_Value& vRet,
                                       CFX_WideString& sError);  //
  static FX_BOOL AFSimple(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError);
  static FX_BOOL AFMakeNumber(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError);
  static FX_BOOL AFSimple_Calculate(IJS_Context* cc,
                                    const std::vector<CJS_Value>& params,
                                    CJS_Value& vRet,
                                    CFX_WideString& sError);
  static FX_BOOL AFRange_Validate(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError);
  static FX_BOOL AFMergeChange(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError);
  static FX_BOOL AFParseDateEx(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError);
  static FX_BOOL AFExtractNums(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError);

 public:
  JS_STATIC_GLOBAL_FUN(AFNumber_Format);
  JS_STATIC_GLOBAL_FUN(AFNumber_Keystroke);
  JS_STATIC_GLOBAL_FUN(AFPercent_Format);
  JS_STATIC_GLOBAL_FUN(AFPercent_Keystroke);
  JS_STATIC_GLOBAL_FUN(AFDate_FormatEx);
  JS_STATIC_GLOBAL_FUN(AFDate_KeystrokeEx);
  JS_STATIC_GLOBAL_FUN(AFDate_Format);
  JS_STATIC_GLOBAL_FUN(AFDate_Keystroke);
  JS_STATIC_GLOBAL_FUN(AFTime_FormatEx);
  JS_STATIC_GLOBAL_FUN(AFTime_KeystrokeEx);
  JS_STATIC_GLOBAL_FUN(AFTime_Format);
  JS_STATIC_GLOBAL_FUN(AFTime_Keystroke);
  JS_STATIC_GLOBAL_FUN(AFSpecial_Format);
  JS_STATIC_GLOBAL_FUN(AFSpecial_Keystroke);
  JS_STATIC_GLOBAL_FUN(AFSpecial_KeystrokeEx);
  JS_STATIC_GLOBAL_FUN(AFSimple);
  JS_STATIC_GLOBAL_FUN(AFMakeNumber);
  JS_STATIC_GLOBAL_FUN(AFSimple_Calculate);
  JS_STATIC_GLOBAL_FUN(AFRange_Validate);
  JS_STATIC_GLOBAL_FUN(AFMergeChange);
  JS_STATIC_GLOBAL_FUN(AFParseDateEx);
  JS_STATIC_GLOBAL_FUN(AFExtractNums);

  JS_STATIC_DECLARE_GLOBAL_FUN();

 public:
  static int ParseStringInteger(const CFX_WideString& string,
                                int nStart,
                                int& nSkip,
                                int nMaxStep);
  static CFX_WideString ParseStringString(const CFX_WideString& string,
                                          int nStart,
                                          int& nSkip);
  static double MakeRegularDate(const CFX_WideString& value,
                                const CFX_WideString& format,
                                bool* bWrongFormat);
  static CFX_WideString MakeFormatDate(double dDate,
                                       const CFX_WideString& format);
  static FX_BOOL ConvertStringToNumber(const FX_WCHAR* swSource,
                                       double& dRet,
                                       FX_BOOL& bDot);
  static double ParseStringToNumber(const FX_WCHAR* swSource);
  static double ParseNormalDate(const CFX_WideString& value,
                                bool* bWrongFormat);
  static double MakeInterDate(CFX_WideString strValue);
  static double ParseNumber(const FX_WCHAR* swSource,
                            FX_BOOL& bAllDigits,
                            FX_BOOL& bDot,
                            FX_BOOL& bSign,
                            FX_BOOL& bKXJS);

 public:
  static CFX_WideString StrLTrim(const FX_WCHAR* pStr);
  static CFX_WideString StrRTrim(const FX_WCHAR* pStr);
  static CFX_WideString StrTrim(const FX_WCHAR* pStr);

  static CFX_ByteString StrLTrim(const FX_CHAR* pStr);
  static CFX_ByteString StrRTrim(const FX_CHAR* pStr);
  static CFX_ByteString StrTrim(const FX_CHAR* pStr);

  static FX_BOOL IsNumber(const FX_CHAR* string);
  static FX_BOOL IsNumber(const FX_WCHAR* string);

  static FX_BOOL maskSatisfied(wchar_t c_Change, wchar_t c_Mask);
  static FX_BOOL isReservedMaskChar(wchar_t ch);

  static double AF_Simple(const FX_WCHAR* sFuction,
                          double dValue1,
                          double dValue2);
  static CJS_Array AF_MakeArrayFromList(CJS_Runtime* pRuntime, CJS_Value val);
};

#endif  // FPDFSDK_SRC_JAVASCRIPT_PUBLICMETHODS_H_
