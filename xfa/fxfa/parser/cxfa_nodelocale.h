// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODELOCALE_H_
#define XFA_FXFA_PARSER_CXFA_NODELOCALE_H_

#include <memory>

#include "core/fxcrt/ifx_locale.h"
#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Node;

CFX_WideString XFA_PatternToString(FX_LOCALENUMSUBCATEGORY category);

class CXFA_NodeLocale : public IFX_Locale {
 public:
  explicit CXFA_NodeLocale(CXFA_Node* pLocale);
  ~CXFA_NodeLocale() override;

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
  CXFA_Node* GetNodeByName(CXFA_Node* pParent,
                           const CFX_WideStringC& wsName) const;
  CFX_WideString GetSymbol(XFA_Element eElement,
                           const CFX_WideStringC& symbol_type) const;
  CFX_WideString GetCalendarSymbol(XFA_Element eElement,
                                   int index,
                                   bool bAbbr) const;

  CFX_UnownedPtr<CXFA_Node> const m_pLocale;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODELOCALE_H_
