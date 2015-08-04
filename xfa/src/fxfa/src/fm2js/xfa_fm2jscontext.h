// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM2JS_CONTEXT_H
#define _XFA_FM2JS_CONTEXT_H
class CXFA_FM2JSContext {
 public:
  static void Abs(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Avg(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Ceil(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Count(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Floor(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Max(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Min(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Mod(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Round(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Sum(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Date(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Date2Num(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void DateFmt(FXJSE_HOBJECT hThis,
                      const CFX_ByteStringC& szFuncName,
                      CFXJSE_Arguments& args);
  static void IsoDate2Num(FXJSE_HOBJECT hThis,
                          const CFX_ByteStringC& szFuncName,
                          CFXJSE_Arguments& args);
  static void IsoTime2Num(FXJSE_HOBJECT hThis,
                          const CFX_ByteStringC& szFuncName,
                          CFXJSE_Arguments& args);
  static void LocalDateFmt(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void LocalTimeFmt(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void Num2Date(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void Num2GMTime(FXJSE_HOBJECT hThis,
                         const CFX_ByteStringC& szFuncName,
                         CFXJSE_Arguments& args);
  static void Num2Time(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void Time(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Time2Num(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void TimeFmt(FXJSE_HOBJECT hThis,
                      const CFX_ByteStringC& szFuncName,
                      CFXJSE_Arguments& args);

  static FX_BOOL IsIsoDateFormat(const FX_CHAR* pData,
                                 int32_t iLength,
                                 int32_t& iStyle,
                                 int32_t& iYear,
                                 int32_t& iMonth,
                                 int32_t& iDay);
  static FX_BOOL IsIsoTimeFormat(const FX_CHAR* pData,
                                 int32_t iLength,
                                 int32_t& iHour,
                                 int32_t& iMinute,
                                 int32_t& iSecond,
                                 int32_t& iMilliSecond,
                                 int32_t& iZoneHour,
                                 int32_t& iZoneMinute);
  static FX_BOOL IsIsoDateTimeFormat(const FX_CHAR* pData,
                                     int32_t iLength,
                                     int32_t& iYear,
                                     int32_t& iMonth,
                                     int32_t& iDay,
                                     int32_t& iHour,
                                     int32_t& iMinute,
                                     int32_t& iSecond,
                                     int32_t& iMillionSecond,
                                     int32_t& iZoneHour,
                                     int32_t& iZoneMinute);
  static FX_BOOL Local2IsoDate(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szDate,
                               const CFX_ByteStringC& szFormat,
                               const CFX_ByteStringC& szLocale,
                               CFX_ByteString& strIsoDate);
  static FX_BOOL Local2IsoTime(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szTime,
                               const CFX_ByteStringC& szFormat,
                               const CFX_ByteStringC& szLocale,
                               CFX_ByteString& strIsoTime);
  static FX_BOOL IsoDate2Local(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szDate,
                               const CFX_ByteStringC& szFormat,
                               const CFX_ByteStringC& szLocale,
                               CFX_ByteString& strLocalDate);
  static FX_BOOL IsoTime2Local(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szTime,
                               const CFX_ByteStringC& szFormat,
                               const CFX_ByteStringC& szLocale,
                               CFX_ByteString& strLocalTime);
  static FX_BOOL GetGMTTime(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szTime,
                            const CFX_ByteStringC& szFormat,
                            const CFX_ByteStringC& szLocale,
                            CFX_ByteString& strGMTTime);
  static int32_t DateString2Num(const CFX_ByteStringC& szDateString);
  static void GetLocalDateFormat(FXJSE_HOBJECT hThis,
                                 int32_t iStyle,
                                 const CFX_ByteStringC& szLocalStr,
                                 CFX_ByteString& strFormat,
                                 FX_BOOL bStandard);
  static void GetLocalTimeFormat(FXJSE_HOBJECT hThis,
                                 int32_t iStyle,
                                 const CFX_ByteStringC& szLocalStr,
                                 CFX_ByteString& strFormat,
                                 FX_BOOL bStandard);
  static void GetStandardDateFormat(FXJSE_HOBJECT hThis,
                                    int32_t iStyle,
                                    const CFX_ByteStringC& szLocalStr,
                                    CFX_ByteString& strFormat);
  static void GetStandardTimeFormat(FXJSE_HOBJECT hThis,
                                    int32_t iStyle,
                                    const CFX_ByteStringC& szLocalStr,
                                    CFX_ByteString& strFormat);

  static void Num2AllTime(FXJSE_HOBJECT hThis,
                          int32_t iTime,
                          const CFX_ByteStringC& szFormat,
                          const CFX_ByteStringC& szLocale,
                          FX_BOOL bGM,
                          CFX_ByteString& strTime);
  static void GetLocalTimeZone(int32_t& iHour, int32_t& iMin, int32_t& iSec);

  static void Apr(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void CTerm(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void FV(FXJSE_HOBJECT hThis,
                 const CFX_ByteStringC& szFuncName,
                 CFXJSE_Arguments& args);
  static void IPmt(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void NPV(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Pmt(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void PPmt(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void PV(FXJSE_HOBJECT hThis,
                 const CFX_ByteStringC& szFuncName,
                 CFXJSE_Arguments& args);
  static void Rate(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Term(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Choose(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void Exists(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void HasValue(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void Oneof(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Within(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void If(FXJSE_HOBJECT hThis,
                 const CFX_ByteStringC& szFuncName,
                 CFXJSE_Arguments& args);
  static void Eval(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Ref(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void UnitType(FXJSE_HOBJECT hThis,
                       const CFX_ByteStringC& szFuncName,
                       CFXJSE_Arguments& args);
  static void UnitValue(FXJSE_HOBJECT hThis,
                        const CFX_ByteStringC& szFuncName,
                        CFXJSE_Arguments& args);

  static void At(FXJSE_HOBJECT hThis,
                 const CFX_ByteStringC& szFuncName,
                 CFXJSE_Arguments& args);
  static void Concat(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void Decode(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void DecodeURL(const CFX_ByteStringC& szURLString,
                        CFX_ByteTextBuf& szResultBuf);
  static void DecodeHTML(const CFX_ByteStringC& szHTMLString,
                         CFX_ByteTextBuf& szResultBuf);
  static void DecodeXML(const CFX_ByteStringC& szXMLString,
                        CFX_ByteTextBuf& szResultBuf);
  static void Encode(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void EncodeURL(const CFX_ByteStringC& szURLString,
                        CFX_ByteTextBuf& szResultBuf);
  static void EncodeHTML(const CFX_ByteStringC& szHTMLString,
                         CFX_ByteTextBuf& szResultBuf);
  static void EncodeXML(const CFX_ByteStringC& szXMLString,
                        CFX_ByteTextBuf& szResultBuf);
  static FX_BOOL HTMLSTR2Code(const CFX_WideStringC& pData, uint32_t& iCode);
  static FX_BOOL HTMLCode2STR(uint32_t iCode, CFX_WideString& wsHTMLReserve);
  static void Format(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void Left(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Len(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Lower(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Ltrim(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Parse(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Replace(FXJSE_HOBJECT hThis,
                      const CFX_ByteStringC& szFuncName,
                      CFXJSE_Arguments& args);
  static void Right(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Rtrim(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Space(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Str(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Stuff(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void Substr(FXJSE_HOBJECT hThis,
                     const CFX_ByteStringC& szFuncName,
                     CFXJSE_Arguments& args);
  static void Uuid(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Upper(FXJSE_HOBJECT hThis,
                    const CFX_ByteStringC& szFuncName,
                    CFXJSE_Arguments& args);
  static void WordNum(FXJSE_HOBJECT hThis,
                      const CFX_ByteStringC& szFuncName,
                      CFXJSE_Arguments& args);
  static void TrillionUS(const CFX_ByteStringC& szData,
                         CFX_ByteTextBuf& strBuf);
  static void WordUS(const CFX_ByteStringC& szData,
                     int32_t iStyle,
                     CFX_ByteTextBuf& strBuf);

  static void Get(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void Post(FXJSE_HOBJECT hThis,
                   const CFX_ByteStringC& szFuncName,
                   CFXJSE_Arguments& args);
  static void Put(FXJSE_HOBJECT hThis,
                  const CFX_ByteStringC& szFuncName,
                  CFXJSE_Arguments& args);
  static void assign_value_operator(FXJSE_HOBJECT hThis,
                                    const CFX_ByteStringC& szFuncName,
                                    CFXJSE_Arguments& args);
  static void logical_or_operator(FXJSE_HOBJECT hThis,
                                  const CFX_ByteStringC& szFuncName,
                                  CFXJSE_Arguments& args);
  static void logical_and_operator(FXJSE_HOBJECT hThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args);
  static void equality_operator(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args);
  static void notequality_operator(FXJSE_HOBJECT hThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args);
  static FX_BOOL fm_ref_equal(FXJSE_HOBJECT hThis, CFXJSE_Arguments& args);
  static void less_operator(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args);
  static void lessequal_operator(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args);
  static void greater_operator(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args);
  static void greaterequal_operator(FXJSE_HOBJECT hThis,
                                    const CFX_ByteStringC& szFuncName,
                                    CFXJSE_Arguments& args);
  static void plus_operator(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args);
  static void minus_operator(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args);
  static void multiple_operator(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args);
  static void divide_operator(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args);
  static void positive_operator(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args);
  static void negative_operator(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args);
  static void logical_not_operator(FXJSE_HOBJECT hThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args);
  static void dot_accessor(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void dotdot_accessor(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args);
  static void eval_translation(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args);
  static void is_fm_object(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void is_fm_array(FXJSE_HOBJECT hThis,
                          const CFX_ByteStringC& szFuncName,
                          CFXJSE_Arguments& args);
  static void get_fm_value(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void get_fm_jsobj(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args);
  static void fm_var_filter(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args);
  static void concat_fm_object(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args);

  static int32_t hvalue_get_array_length(FXJSE_HOBJECT hThis, FXJSE_HVALUE arg);
  static FX_BOOL simpleValueCompare(FXJSE_HOBJECT hThis,
                                    FXJSE_HVALUE firstValue,
                                    FXJSE_HVALUE secondValue);
  static void unfoldArgs(FXJSE_HOBJECT hThis,
                         CFXJSE_Arguments& args,
                         FXJSE_HVALUE*& resultValues,
                         int32_t& iCount,
                         int32_t iStart = 0);
  static void GetObjectDefaultValue(FXJSE_HVALUE hObjectValue,
                                    FXJSE_HVALUE hDefaultValue);
  static FX_BOOL SetObjectDefaultValue(FXJSE_HVALUE hObjectValue,
                                       FXJSE_HVALUE hNewValue);
  static void GenerateSomExpression(const CFX_ByteStringC& szName,
                                    int32_t iIndexFlags,
                                    int32_t iIndexValue,
                                    FX_BOOL bIsStar,
                                    CFX_ByteString& szSomExp);
  static FX_BOOL GetObjectByName(FXJSE_HOBJECT hThis,
                                 FXJSE_HVALUE accessorValue,
                                 const CFX_ByteStringC& szAccessorName);
  static int32_t ResolveObjects(FXJSE_HOBJECT hThis,
                                FXJSE_HVALUE hParentValue,
                                const CFX_ByteStringC& bsSomExp,
                                XFA_RESOLVENODE_RS& resoveNodeRS,
                                FX_BOOL bdotAccessor = TRUE,
                                FX_BOOL bHasNoResolveName = FALSE);
  static void ParseResolveResult(FXJSE_HOBJECT hThis,
                                 const XFA_RESOLVENODE_RS& resoveNodeRS,
                                 FXJSE_HVALUE hParentValue,
                                 FXJSE_HVALUE*& resultValues,
                                 int32_t& iSize,
                                 FX_BOOL& bAttribute);

  static FXJSE_HVALUE GetSimpleHValue(FXJSE_HOBJECT hThis,
                                      CFXJSE_Arguments& args,
                                      uint32_t index);
  static FX_BOOL HValueIsNull(FXJSE_HOBJECT hThis, FXJSE_HVALUE hValue);
  static int32_t HValueToInteger(FXJSE_HOBJECT hThis, FXJSE_HVALUE hValue);
  static FX_DOUBLE StringToDouble(const CFX_ByteStringC& szStringVal);
  static FX_FLOAT HValueToFloat(FXJSE_HOBJECT hThis, FXJSE_HVALUE hValue);
  static FX_DOUBLE HValueToDouble(FXJSE_HOBJECT hThis, FXJSE_HVALUE hValue);
  static void HValueToUTF8String(FXJSE_HVALUE hValue,
                                 CFX_ByteString& outputValue);
  CXFA_FM2JSContext();
  ~CXFA_FM2JSContext();
  static CXFA_FM2JSContext* Create();
  void Initialize(FXJSE_HRUNTIME hScriptRuntime,
                  FXJSE_HCONTEXT hScriptContext,
                  CXFA_Document* pDoc);
  void GlobalPropertyGetter(FXJSE_HVALUE hValue);
  void Release();
  FXJSE_HRUNTIME GetScriptRuntime() const { return m_hScriptRuntime; }
  CXFA_Document* GetDocument() const { return m_pDocument; }
  void ThrowScriptErrorMessage(int32_t iStringID, ...);

 private:
  FXJSE_HRUNTIME m_hScriptRuntime;
  FXJSE_CLASS m_fmClass;
  FXJSE_HCLASS m_hFMClass;
  FXJSE_HVALUE m_hValue;
  CXFA_Document* m_pDocument;
};
#endif
