// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_
#define FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/xfa/fxjse.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

class CFXJSE_Context;
class CFX_WideTextBuf;
class CXFA_Document;

class CFXJSE_FormCalcContext final : public CFXJSE_HostObject {
 public:
  CFXJSE_FormCalcContext(v8::Isolate* pScriptIsolate,
                         CFXJSE_Context* pScriptContext,
                         CXFA_Document* pDoc);
  ~CFXJSE_FormCalcContext() override;

  // CFXJSE_HostObject:
  CFXJSE_FormCalcContext* AsFormCalcContext() override;

  static void Abs(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Avg(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Ceil(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Count(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Floor(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Max(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Min(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Mod(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Round(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Sum(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Date(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Date2Num(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void DateFmt(CFXJSE_Value* pThis,
                      ByteStringView bsFuncName,
                      const v8::FunctionCallbackInfo<v8::Value>& info,
                      CFXJSE_Value* pRetValue);
  static void IsoDate2Num(CFXJSE_Value* pThis,
                          ByteStringView bsFuncName,
                          const v8::FunctionCallbackInfo<v8::Value>& info,
                          CFXJSE_Value* pRetValue);
  static void IsoTime2Num(CFXJSE_Value* pThis,
                          ByteStringView bsFuncName,
                          const v8::FunctionCallbackInfo<v8::Value>& info,
                          CFXJSE_Value* pRetValue);
  static void LocalDateFmt(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void LocalTimeFmt(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void Num2Date(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void Num2GMTime(CFXJSE_Value* pThis,
                         ByteStringView bsFuncName,
                         const v8::FunctionCallbackInfo<v8::Value>& info,
                         CFXJSE_Value* pRetValue);
  static void Num2Time(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void Time(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Time2Num(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void TimeFmt(CFXJSE_Value* pThis,
                      ByteStringView bsFuncName,
                      const v8::FunctionCallbackInfo<v8::Value>& info,
                      CFXJSE_Value* pRetValue);

  static ByteString Local2IsoDate(CFXJSE_Value* pThis,
                                  ByteStringView bsDate,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString IsoDate2Local(CFXJSE_Value* pThis,
                                  ByteStringView bsDate,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString IsoTime2Local(CFXJSE_Value* pThis,
                                  ByteStringView bsTime,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString GetLocalDateFormat(CFXJSE_Value* pThis,
                                       int32_t iStyle,
                                       ByteStringView bsLocale,
                                       bool bStandard);
  static ByteString GetLocalTimeFormat(CFXJSE_Value* pThis,
                                       int32_t iStyle,
                                       ByteStringView bsLocale,
                                       bool bStandard);
  static ByteString GetStandardDateFormat(CFXJSE_Value* pThis,
                                          int32_t iStyle,
                                          ByteStringView bsLocale);
  static ByteString GetStandardTimeFormat(CFXJSE_Value* pThis,
                                          int32_t iStyle,
                                          ByteStringView bsLocale);
  static ByteString Num2AllTime(CFXJSE_Value* pThis,
                                int32_t iTime,
                                ByteStringView bsFormat,
                                ByteStringView bsLocale,
                                bool bGM);

  static void Apr(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void CTerm(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void FV(CFXJSE_Value* pThis,
                 ByteStringView bsFuncName,
                 const v8::FunctionCallbackInfo<v8::Value>& info,
                 CFXJSE_Value* pRetValue);
  static void IPmt(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void NPV(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Pmt(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void PPmt(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void PV(CFXJSE_Value* pThis,
                 ByteStringView bsFuncName,
                 const v8::FunctionCallbackInfo<v8::Value>& info,
                 CFXJSE_Value* pRetValue);
  static void Rate(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Term(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Choose(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Exists(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void HasValue(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void Oneof(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Within(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void If(CFXJSE_Value* pThis,
                 ByteStringView bsFuncName,
                 const v8::FunctionCallbackInfo<v8::Value>& info,
                 CFXJSE_Value* pRetValue);
  static void Eval(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Ref(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void UnitType(CFXJSE_Value* pThis,
                       ByteStringView bsFuncName,
                       const v8::FunctionCallbackInfo<v8::Value>& info,
                       CFXJSE_Value* pRetValue);
  static void UnitValue(CFXJSE_Value* pThis,
                        ByteStringView bsFuncName,
                        const v8::FunctionCallbackInfo<v8::Value>& info,
                        CFXJSE_Value* pRetValue);

  static void At(CFXJSE_Value* pThis,
                 ByteStringView bsFuncName,
                 const v8::FunctionCallbackInfo<v8::Value>& info,
                 CFXJSE_Value* pRetValue);
  static void Concat(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Decode(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Encode(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Format(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Left(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Len(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Lower(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Ltrim(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Parse(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Replace(CFXJSE_Value* pThis,
                      ByteStringView bsFuncName,
                      const v8::FunctionCallbackInfo<v8::Value>& info,
                      CFXJSE_Value* pRetValue);
  static void Right(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Rtrim(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Space(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Str(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Stuff(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void Substr(CFXJSE_Value* pThis,
                     ByteStringView bsFuncName,
                     const v8::FunctionCallbackInfo<v8::Value>& info,
                     CFXJSE_Value* pRetValue);
  static void Uuid(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Upper(CFXJSE_Value* pThis,
                    ByteStringView bsFuncName,
                    const v8::FunctionCallbackInfo<v8::Value>& info,
                    CFXJSE_Value* pRetValue);
  static void WordNum(CFXJSE_Value* pThis,
                      ByteStringView bsFuncName,
                      const v8::FunctionCallbackInfo<v8::Value>& info,
                      CFXJSE_Value* pRetValue);

  static void Get(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void Post(CFXJSE_Value* pThis,
                   ByteStringView bsFuncName,
                   const v8::FunctionCallbackInfo<v8::Value>& info,
                   CFXJSE_Value* pRetValue);
  static void Put(CFXJSE_Value* pThis,
                  ByteStringView bsFuncName,
                  const v8::FunctionCallbackInfo<v8::Value>& info,
                  CFXJSE_Value* pRetValue);
  static void assign_value_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void logical_or_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void logical_and_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void equality_operator(CFXJSE_Value* pThis,
                                ByteStringView bsFuncName,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                CFXJSE_Value* pRetValue);
  static void notequality_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static bool fm_ref_equal(CFXJSE_Value* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void less_operator(CFXJSE_Value* pThis,
                            ByteStringView bsFuncName,
                            const v8::FunctionCallbackInfo<v8::Value>& info,
                            CFXJSE_Value* pRetValue);
  static void lessequal_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void greater_operator(CFXJSE_Value* pThis,
                               ByteStringView bsFuncName,
                               const v8::FunctionCallbackInfo<v8::Value>& info,
                               CFXJSE_Value* pRetValue);
  static void greaterequal_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void plus_operator(CFXJSE_Value* pThis,
                            ByteStringView bsFuncName,
                            const v8::FunctionCallbackInfo<v8::Value>& info,
                            CFXJSE_Value* pRetValue);
  static void minus_operator(CFXJSE_Value* pThis,
                             ByteStringView bsFuncName,
                             const v8::FunctionCallbackInfo<v8::Value>& info,
                             CFXJSE_Value* pRetValue);
  static void multiple_operator(CFXJSE_Value* pThis,
                                ByteStringView bsFuncName,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                CFXJSE_Value* pRetValue);
  static void divide_operator(CFXJSE_Value* pThis,
                              ByteStringView bsFuncName,
                              const v8::FunctionCallbackInfo<v8::Value>& info,
                              CFXJSE_Value* pRetValue);
  static void positive_operator(CFXJSE_Value* pThis,
                                ByteStringView bsFuncName,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                CFXJSE_Value* pRetValue);
  static void negative_operator(CFXJSE_Value* pThis,
                                ByteStringView bsFuncName,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                CFXJSE_Value* pRetValue);
  static void logical_not_operator(
      CFXJSE_Value* pThis,
      ByteStringView bsFuncName,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      CFXJSE_Value* pRetValue);
  static void dot_accessor(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void dotdot_accessor(CFXJSE_Value* pThis,
                              ByteStringView bsFuncName,
                              const v8::FunctionCallbackInfo<v8::Value>& info,
                              CFXJSE_Value* pRetValue);
  static void eval_translation(CFXJSE_Value* pThis,
                               ByteStringView bsFuncName,
                               const v8::FunctionCallbackInfo<v8::Value>& info,
                               CFXJSE_Value* pRetValue);
  static void is_fm_object(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void is_fm_array(CFXJSE_Value* pThis,
                          ByteStringView bsFuncName,
                          const v8::FunctionCallbackInfo<v8::Value>& info,
                          CFXJSE_Value* pRetValue);
  static void get_fm_value(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void get_fm_jsobj(CFXJSE_Value* pThis,
                           ByteStringView bsFuncName,
                           const v8::FunctionCallbackInfo<v8::Value>& info,
                           CFXJSE_Value* pRetValue);
  static void fm_var_filter(CFXJSE_Value* pThis,
                            ByteStringView bsFuncName,
                            const v8::FunctionCallbackInfo<v8::Value>& info,
                            CFXJSE_Value* pRetValue);
  static void concat_fm_object(CFXJSE_Value* pThis,
                               ByteStringView bsFuncName,
                               const v8::FunctionCallbackInfo<v8::Value>& info,
                               CFXJSE_Value* pRetValue);

  static int32_t hvalue_get_array_length(CFXJSE_Value* pThis,
                                         CFXJSE_Value* arg);
  static bool simpleValueCompare(CFXJSE_Value* pThis,
                                 CFXJSE_Value* firstValue,
                                 CFXJSE_Value* secondValue);
  static std::vector<std::unique_ptr<CFXJSE_Value>> unfoldArgs(
      CFXJSE_Value* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static ByteString GenerateSomExpression(ByteStringView bsName,
                                          int32_t iIndexFlags,
                                          int32_t iIndexValue,
                                          bool bIsStar);
  static bool GetObjectForName(CFXJSE_Value* pThis,
                               CFXJSE_Value* accessorValue,
                               ByteStringView bsAccessorName);
  static bool ResolveObjects(CFXJSE_Value* pThis,
                             CFXJSE_Value* pParentValue,
                             ByteStringView bsSomExp,
                             XFA_RESOLVENODE_RS* resolveNodeRS,
                             bool bdotAccessor,
                             bool bHasNoResolveName);
  static void ParseResolveResult(
      CFXJSE_Value* pThis,
      const XFA_RESOLVENODE_RS& resolveNodeRS,
      CFXJSE_Value* pParentValue,
      std::vector<std::unique_ptr<CFXJSE_Value>>* resultValues,
      bool* bAttribute);

  static std::unique_ptr<CFXJSE_Value> GetSimpleValue(
      CFXJSE_Value* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      uint32_t index);
  static bool ValueIsNull(CFXJSE_Value* pThis, CFXJSE_Value* pValue);
  static int32_t ValueToInteger(CFXJSE_Value* pThis, CFXJSE_Value* pValue);
  static float ValueToFloat(CFXJSE_Value* pThis, CFXJSE_Value* pValue);
  static double ValueToDouble(CFXJSE_Value* pThis, CFXJSE_Value* pValue);
  static ByteString ValueToUTF8String(CFXJSE_Value* pValue);
  static double ExtractDouble(CFXJSE_Value* pThis,
                              CFXJSE_Value* src,
                              bool* ret);

  static bool Translate(WideStringView wsFormcalc,
                        CFX_WideTextBuf* wsJavascript);

  void GlobalPropertyGetter(CFXJSE_Value* pValue);

 private:
  static void DotAccessorCommon(CFXJSE_Value* pThis,
                                ByteStringView bsFuncName,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                CFXJSE_Value* pRetValue,
                                bool bDotAccessor);

  v8::Isolate* GetScriptRuntime() const { return m_pIsolate.Get(); }
  CXFA_Document* GetDocument() const { return m_pDocument.Get(); }

  void ThrowNoDefaultPropertyException(ByteStringView name) const;
  void ThrowCompilerErrorException() const;
  void ThrowDivideByZeroException() const;
  void ThrowServerDeniedException() const;
  void ThrowPropertyNotInObjectException(const WideString& name,
                                         const WideString& exp) const;
  void ThrowArgumentMismatchException() const;
  void ThrowParamCountMismatchException(const WideString& method) const;
  void ThrowException(const WideString& str) const;

  UnownedPtr<v8::Isolate> m_pIsolate;
  std::unique_ptr<CFXJSE_Value> m_pValue;
  UnownedPtr<CXFA_Document> const m_pDocument;
};

#endif  // FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_
