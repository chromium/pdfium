// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_
#define XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_

#include <memory>

#include "core/fxcrt/ifx_locale.h"

class CXML_Element;

class CXFA_XMLLocale : public IFX_Locale {
 public:
  explicit CXFA_XMLLocale(std::unique_ptr<CXML_Element> pLocaleData);
  ~CXFA_XMLLocale() override;

  // IFX_Locale
  WideString GetName() const override;
  WideString GetNumbericSymbol(FX_LOCALENUMSYMBOL eType) const override;

  WideString GetDateTimeSymbols() const override;
  WideString GetMonthName(int32_t nMonth, bool bAbbr) const override;
  WideString GetDayName(int32_t nWeek, bool bAbbr) const override;
  WideString GetMeridiemName(bool bAM) const override;
  FX_TIMEZONE GetTimeZone() const override;
  WideString GetEraName(bool bAD) const override;

  WideString GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType) const override;
  WideString GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType) const override;
  WideString GetNumPattern(FX_LOCALENUMSUBCATEGORY eType) const override;

 private:
  WideString GetPattern(CXML_Element* pElement,
                        const ByteStringView& bsTag,
                        const WideStringView& wsName) const;
  WideString GetCalendarSymbol(const ByteStringView& symbol,
                               int index,
                               bool bAbbr) const;

  std::unique_ptr<CXML_Element> m_pLocaleData;
};

#endif  // XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_
