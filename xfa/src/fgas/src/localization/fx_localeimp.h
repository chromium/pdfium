// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_LOCALE_IMP_H_
#define _FX_LOCALE_IMP_H_
class CFX_LCNumeric;
class CFX_Locale : public IFX_Locale {
 public:
  CFX_Locale(CXML_Element* pLocaleData);
  virtual void Release() { delete this; }

  virtual CFX_WideString GetName();
  virtual void GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                 CFX_WideString& wsNumSymbol) const;

  virtual void GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const;
  virtual void GetMonthName(int32_t nMonth,
                            CFX_WideString& wsMonthName,
                            FX_BOOL bAbbr = TRUE) const;
  virtual void GetDayName(int32_t nWeek,
                          CFX_WideString& wsDayName,
                          FX_BOOL bAbbr = TRUE) const;
  virtual void GetMeridiemName(CFX_WideString& wsMeridiemName,
                               FX_BOOL bAM = TRUE) const;
  virtual void GetTimeZone(FX_TIMEZONE& tz) const;
  virtual void GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD = TRUE) const;

  virtual void GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                              CFX_WideString& wsPattern) const;
  virtual void GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                              CFX_WideString& wsPattern) const;
  virtual void GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                             CFX_WideString& wsPattern) const;

 protected:
  virtual ~CFX_Locale();
  CXML_Element* m_pElement;
};
class CFX_FormatString : public IFX_FormatString {
 public:
  CFX_FormatString(IFX_LocaleMgr* pLocaleMgr, FX_BOOL bUseLCID);
  virtual void Release() { delete this; }

  virtual void SplitFormatString(const CFX_WideString& wsFormatString,
                                 CFX_WideStringArray& wsPatterns);
  virtual FX_LOCALECATEGORY GetCategory(const CFX_WideString& wsPattern);
  virtual FX_WORD GetLCID(const CFX_WideString& wsPattern);
  virtual CFX_WideString GetLocaleName(const CFX_WideString& wsPattern);
  virtual FX_BOOL ParseText(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsValue);
  virtual FX_BOOL ParseNum(const CFX_WideString& wsSrcNum,
                           const CFX_WideString& wsPattern,
                           FX_FLOAT& fValue);
  virtual FX_BOOL ParseNum(const CFX_WideString& wsSrcNum,
                           const CFX_WideString& wsPattern,
                           CFX_WideString& wsValue);
  virtual FX_BOOL ParseDateTime(const CFX_WideString& wsSrcDateTime,
                                const CFX_WideString& wsPattern,
                                FX_DATETIMETYPE eDateTimeType,
                                CFX_Unitime& dtValue);
  virtual FX_BOOL ParseZero(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern);
  virtual FX_BOOL ParseNull(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern);
  virtual FX_BOOL FormatText(const CFX_WideString& wsSrcText,
                             const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput);
  virtual FX_BOOL FormatNum(const CFX_WideString& wsSrcNum,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsOutput);
  virtual FX_BOOL FormatNum(FX_FLOAT fNum,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsOutput);
  virtual FX_BOOL FormatDateTime(const CFX_WideString& wsSrcDateTime,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput);
  virtual FX_BOOL FormatDateTime(const CFX_WideString& wsSrcDateTime,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput,
                                 FX_DATETIMETYPE eDateTimeType);
  virtual FX_BOOL FormatDateTime(const CFX_Unitime& dt,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput);
  virtual FX_BOOL FormatZero(const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput);
  virtual FX_BOOL FormatNull(const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput);

 protected:
  virtual ~CFX_FormatString();
  IFX_Locale* GetTextFormat(const CFX_WideString& wsPattern,
                            const CFX_WideStringC& wsCategory,
                            CFX_WideString& wsPurgePattern);
  IFX_Locale* GetNumericFormat(const CFX_WideString& wsPattern,
                               int32_t& iDotIndex,
                               FX_DWORD& dwStyle,
                               CFX_WideString& wsPurgePattern);
  FX_BOOL FormatStrNum(const CFX_WideStringC& wsInputNum,
                       const CFX_WideString& wsPattern,
                       CFX_WideString& wsOutput);
  FX_BOOL FormatLCNumeric(CFX_LCNumeric& lcNum,
                          const CFX_WideString& wsPattern,
                          CFX_WideString& wsOutput);
  FX_DATETIMETYPE GetDateTimeFormat(const CFX_WideString& wsPattern,
                                    IFX_Locale*& pLocale,
                                    CFX_WideString& wsDatePattern,
                                    CFX_WideString& wsTimePattern);
  IFX_Locale* GetPatternLocale(const CFX_WideStringC& wsLocale);
  IFX_LocaleMgr* m_pLocaleMgr;
  FX_BOOL m_bUseLCID;
};
#endif
