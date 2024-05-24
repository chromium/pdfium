// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_
#define FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_

#include <stdint.h>

#include <functional>
#include <optional>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widetext_buffer.h"
#include "fxjs/xfa/fxjse.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"

class CFXJSE_Context;
class CXFA_Document;

namespace cppgc {
class Heap;
}  // namespace cppgc

class CFXJSE_FormCalcContext final : public CFXJSE_HostObject {
 public:
  CFXJSE_FormCalcContext(v8::Isolate* pIsolate,
                         CFXJSE_Context* pScriptContext,
                         CXFA_Document* pDoc);
  ~CFXJSE_FormCalcContext() override;

  // CFXJSE_HostObject:
  CFXJSE_FormCalcContext* AsFormCalcContext() override;

  static void Abs(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Avg(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Ceil(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Count(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Floor(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Max(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Min(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Mod(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Round(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Sum(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Date(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Date2Num(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void DateFmt(CFXJSE_HostObject* pThis,
                      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void IsoDate2Num(CFXJSE_HostObject* pThis,
                          const v8::FunctionCallbackInfo<v8::Value>& info);
  static void IsoTime2Num(CFXJSE_HostObject* pThis,
                          const v8::FunctionCallbackInfo<v8::Value>& info);
  static void LocalDateFmt(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void LocalTimeFmt(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Num2Date(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Num2GMTime(CFXJSE_HostObject* pThis,
                         const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Num2Time(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Time(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Time2Num(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void TimeFmt(CFXJSE_HostObject* pThis,
                      const v8::FunctionCallbackInfo<v8::Value>& info);

  static ByteString Local2IsoDate(CFXJSE_HostObject* pThis,
                                  ByteStringView bsDate,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString IsoDate2Local(CFXJSE_HostObject* pThis,
                                  ByteStringView bsDate,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString IsoTime2Local(CFXJSE_HostObject* pThis,
                                  ByteStringView bsTime,
                                  ByteStringView bsFormat,
                                  ByteStringView bsLocale);
  static ByteString GetLocalDateFormat(CFXJSE_HostObject* pThis,
                                       int32_t iStyle,
                                       ByteStringView bsLocale,
                                       bool bStandard);
  static ByteString GetLocalTimeFormat(CFXJSE_HostObject* pThis,
                                       int32_t iStyle,
                                       ByteStringView bsLocale,
                                       bool bStandard);
  static ByteString GetStandardDateFormat(CFXJSE_HostObject* pThis,
                                          int32_t iStyle,
                                          ByteStringView bsLocale);
  static ByteString GetStandardTimeFormat(CFXJSE_HostObject* pThis,
                                          int32_t iStyle,
                                          ByteStringView bsLocale);
  static ByteString Num2AllTime(CFXJSE_HostObject* pThis,
                                int32_t iTime,
                                ByteStringView bsFormat,
                                ByteStringView bsLocale,
                                bool bGM);

  static void Apr(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void CTerm(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void FV(CFXJSE_HostObject* pThis,
                 const v8::FunctionCallbackInfo<v8::Value>& info);
  static void IPmt(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void NPV(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Pmt(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void PPmt(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void PV(CFXJSE_HostObject* pThis,
                 const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Rate(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Term(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Choose(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Exists(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void HasValue(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Oneof(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Within(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void If(CFXJSE_HostObject* pThis,
                 const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Eval(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Ref(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void UnitType(CFXJSE_HostObject* pThis,
                       const v8::FunctionCallbackInfo<v8::Value>& info);
  static void UnitValue(CFXJSE_HostObject* pThis,
                        const v8::FunctionCallbackInfo<v8::Value>& info);

  static void At(CFXJSE_HostObject* pThis,
                 const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Concat(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Decode(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Encode(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Format(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Left(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Len(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Lower(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Ltrim(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Parse(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Replace(CFXJSE_HostObject* pThis,
                      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Right(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Rtrim(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Space(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Str(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Stuff(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Substr(CFXJSE_HostObject* pThis,
                     const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Uuid(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Upper(CFXJSE_HostObject* pThis,
                    const v8::FunctionCallbackInfo<v8::Value>& info);
  static void WordNum(CFXJSE_HostObject* pThis,
                      const v8::FunctionCallbackInfo<v8::Value>& info);

  static void Get(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Post(CFXJSE_HostObject* pThis,
                   const v8::FunctionCallbackInfo<v8::Value>& info);
  static void Put(CFXJSE_HostObject* pThis,
                  const v8::FunctionCallbackInfo<v8::Value>& info);
  static void assign_value_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void logical_or_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void logical_and_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void equality_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void notequality_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static bool fm_ref_equal(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void less_operator(CFXJSE_HostObject* pThis,
                            const v8::FunctionCallbackInfo<v8::Value>& info);
  static void lessequal_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void greater_operator(CFXJSE_HostObject* pThis,
                               const v8::FunctionCallbackInfo<v8::Value>& info);
  static void greaterequal_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void plus_operator(CFXJSE_HostObject* pThis,
                            const v8::FunctionCallbackInfo<v8::Value>& info);
  static void minus_operator(CFXJSE_HostObject* pThis,
                             const v8::FunctionCallbackInfo<v8::Value>& info);
  static void multiple_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void divide_operator(CFXJSE_HostObject* pThis,
                              const v8::FunctionCallbackInfo<v8::Value>& info);
  static void positive_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void negative_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void logical_not_operator(
      CFXJSE_HostObject* pThis,
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void dot_accessor(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void dotdot_accessor(CFXJSE_HostObject* pThis,
                              const v8::FunctionCallbackInfo<v8::Value>& info);
  static void eval_translation(CFXJSE_HostObject* pThis,
                               const v8::FunctionCallbackInfo<v8::Value>& info);
  static void is_fm_object(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void is_fm_array(CFXJSE_HostObject* pThis,
                          const v8::FunctionCallbackInfo<v8::Value>& info);
  static void get_fm_value(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void get_fm_jsobj(CFXJSE_HostObject* pThis,
                           const v8::FunctionCallbackInfo<v8::Value>& info);
  static void fm_var_filter(CFXJSE_HostObject* pThis,
                            const v8::FunctionCallbackInfo<v8::Value>& info);
  static void concat_fm_object(CFXJSE_HostObject* pThis,
                               const v8::FunctionCallbackInfo<v8::Value>& info);

  static std::optional<WideTextBuffer> Translate(cppgc::Heap* pHeap,
                                                 WideStringView wsFormcalc);

  v8::Local<v8::Value> GlobalPropertyGetter();
  v8::Isolate* GetIsolate() const { return m_pIsolate; }
  CXFA_Document* GetDocument() const { return m_pDocument.Get(); }

 private:
  friend class FormCalcContextTest_GenerateSomExpression_Test;
  friend class FormCalcContextTest_IsIsoDateFormat_Test;
  friend class FormCalcContextTest_IsIsoTimeFormat_Test;

  static ByteString GenerateSomExpression(ByteStringView bsName,
                                          int32_t iIndexFlags,
                                          int32_t iIndexValue,
                                          bool bIsStar);

  static void DotAccessorCommon(CFXJSE_HostObject* pThis,
                                const v8::FunctionCallbackInfo<v8::Value>& info,
                                bool bDotAccessor);

  static bool IsIsoDateTimeFormat(ByteStringView bsData,
                                  int32_t* pYear,
                                  int32_t* pMonth,
                                  int32_t* pDay);

  static bool IsIsoDateFormat(ByteStringView bsData,
                              int32_t* pYear,
                              int32_t* pMonth,
                              int32_t* pDay);

  static bool IsIsoTimeFormat(ByteStringView bsData);

  static int32_t DateString2Num(ByteStringView bsDate);

  bool ApplyToExpansion(
      std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
      const v8::FunctionCallbackInfo<v8::Value>& info,
      bool bStrict);

  bool ApplyToArray(v8::Isolate* pIsolate,
                    std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
                    v8::Local<v8::Array> pArray);

  void ApplyToObject(v8::Isolate* pIsolate,
                     std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
                     v8::Local<v8::Object> pObject);

  void ThrowArgumentMismatchException() const;
  void ThrowNoDefaultPropertyException(ByteStringView name) const;
  void ThrowCompilerErrorException() const;
  void ThrowDivideByZeroException() const;
  void ThrowServerDeniedException() const;
  void ThrowPropertyNotInObjectException(ByteStringView name,
                                         ByteStringView exp) const;
  void ThrowParamCountMismatchException(ByteStringView method) const;
  void ThrowException(ByteStringView str) const;

  UnownedPtr<v8::Isolate> const m_pIsolate;
  v8::Global<v8::Value> m_Value;
  cppgc::WeakPersistent<CXFA_Document> const m_pDocument;
};

#endif  // FXJS_XFA_CFXJSE_FORMCALC_CONTEXT_H_
