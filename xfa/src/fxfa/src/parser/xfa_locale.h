// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_LOCALE_H
#define _FXFA_LOCALE_H
class CXFA_XMLLocale : public IFX_Locale {
 public:
  CXFA_XMLLocale(CXML_Element* pLocaleData);
  virtual void Release();
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
  ~CXFA_XMLLocale();
  void GetPattern(CXML_Element* pElement,
                  const CFX_ByteStringC& bsTag,
                  const CFX_WideStringC& wsName,
                  CFX_WideString& wsPattern) const;
  CFX_WideString GetCalendarSymbol(const CFX_ByteStringC& symbol,
                                   int index,
                                   FX_BOOL bAbbr) const;

 private:
  CXML_Element* m_pLocaleData;
};
class CXFA_NodeLocale : public IFX_Locale {
 public:
  CXFA_NodeLocale(CXFA_Node* pLocale);
  virtual void Release();
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
  ~CXFA_NodeLocale();
  CXFA_Node* GetNodeByName(CXFA_Node* pParent,
                           const CFX_WideStringC& wsName) const;
  CFX_WideString GetSymbol(XFA_ELEMENT eElement,
                           const CFX_WideStringC& symbol_type) const;
  CFX_WideString GetCalendarSymbol(XFA_ELEMENT eElement,
                                   int index,
                                   FX_BOOL bAbbr) const;

  CXFA_Node* m_pLocale;
};
#endif
