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
  CFX_WideString GetName() const override;
  CFX_WideString GetNumbericSymbol(FX_LOCALENUMSYMBOL eType) const override;

  CFX_WideString GetDateTimeSymbols() const override;
  CFX_WideString GetMonthName(int32_t nMonth, bool bAbbr) const override;
  CFX_WideString GetDayName(int32_t nWeek, bool bAbbr) const override;
  CFX_WideString GetMeridiemName(bool bAM) const override;
  FX_TIMEZONE GetTimeZone() const override;
  CFX_WideString GetEraName(bool bAD) const override;

  CFX_WideString GetDatePattern(
      FX_LOCALEDATETIMESUBCATEGORY eType) const override;
  CFX_WideString GetTimePattern(
      FX_LOCALEDATETIMESUBCATEGORY eType) const override;
  CFX_WideString GetNumPattern(FX_LOCALENUMSUBCATEGORY eType) const override;

 private:
  CFX_WideString GetPattern(CXML_Element* pElement,
                            const CFX_ByteStringC& bsTag,
                            const CFX_WideStringC& wsName) const;
  CFX_WideString GetCalendarSymbol(const CFX_ByteStringC& symbol,
                                   int index,
                                   bool bAbbr) const;

  std::unique_ptr<CXML_Element> m_pLocaleData;
};

#endif  // XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_
