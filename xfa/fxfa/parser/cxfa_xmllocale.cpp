// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xmllocale.h"

#include <utility>

#include "core/fxcrt/fx_xml.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_nodelocale.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_XMLLocale::CXFA_XMLLocale(std::unique_ptr<CXML_Element> pLocaleData)
    : m_pLocaleData(std::move(pLocaleData)) {}

CXFA_XMLLocale::~CXFA_XMLLocale() {}

CFX_WideString CXFA_XMLLocale::GetName() const {
  return m_pLocaleData ? m_pLocaleData->GetAttrValue("name") : CFX_WideString();
}

void CXFA_XMLLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                       CFX_WideString& wsNumSymbol) const {
  CFX_ByteString bsSymbols;
  CFX_WideString wsName;
  switch (eType) {
    case FX_LOCALENUMSYMBOL_Decimal:
      bsSymbols = "numberSymbols";
      wsName = L"decimal";
      break;
    case FX_LOCALENUMSYMBOL_Grouping:
      bsSymbols = "numberSymbols";
      wsName = L"grouping";
      break;
    case FX_LOCALENUMSYMBOL_Percent:
      bsSymbols = "numberSymbols";
      wsName = L"percent";
      break;
    case FX_LOCALENUMSYMBOL_Minus:
      bsSymbols = "numberSymbols";
      wsName = L"minus";
      break;
    case FX_LOCALENUMSYMBOL_Zero:
      bsSymbols = "numberSymbols";
      wsName = L"zero";
      break;
    case FX_LOCALENUMSYMBOL_CurrencySymbol:
      bsSymbols = "currencySymbols";
      wsName = L"symbol";
      break;
    case FX_LOCALENUMSYMBOL_CurrencyName:
      bsSymbols = "currencySymbols";
      wsName = L"isoname";
      break;
    default:
      return;
  }
  CXML_Element* pElement = m_pLocaleData->GetElement("", bsSymbols.AsStringC());
  if (!pElement)
    return;

  GetPattern(pElement,
             CFX_ByteStringC(bsSymbols.c_str(), bsSymbols.GetLength() - 1),
             wsName.AsStringC(), wsNumSymbol);
}

void CXFA_XMLLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const {
  if (!m_pLocaleData)
    return;

  CFX_ByteString bsSpace;
  CXML_Element* pNumberSymbols =
      m_pLocaleData->GetElement(bsSpace.AsStringC(), "dateTimeSymbols");
  if (!pNumberSymbols)
    return;

  wsDtSymbol = pNumberSymbols->GetContent(0);
}

void CXFA_XMLLocale::GetMonthName(int32_t nMonth,
                                  CFX_WideString& wsMonthName,
                                  bool bAbbr) const {
  wsMonthName = GetCalendarSymbol("month", nMonth, bAbbr);
}

void CXFA_XMLLocale::GetDayName(int32_t nWeek,
                                CFX_WideString& wsDayName,
                                bool bAbbr) const {
  wsDayName = GetCalendarSymbol("day", nWeek, bAbbr);
}

void CXFA_XMLLocale::GetMeridiemName(CFX_WideString& wsMeridiemName,
                                     bool bAM) const {
  wsMeridiemName = GetCalendarSymbol("meridiem", bAM ? 0 : 1, false);
}

void CXFA_XMLLocale::GetTimeZone(FX_TIMEZONE* tz) const {
  CXFA_TimeZoneProvider provider;
  provider.GetTimeZone(tz);
}

void CXFA_XMLLocale::GetEraName(CFX_WideString& wsEraName, bool bAD) const {
  wsEraName = GetCalendarSymbol("era", bAD ? 1 : 0, false);
}

CFX_WideString CXFA_XMLLocale::GetCalendarSymbol(const CFX_ByteStringC& symbol,
                                                 int index,
                                                 bool bAbbr) const {
  CFX_ByteString pstrSymbolNames = symbol + "Names";
  CFX_WideString wsSymbolName = L"";
  if (m_pLocaleData) {
    CXML_Element* pChild = m_pLocaleData->GetElement("", "calendarSymbols");
    if (pChild) {
      CXML_Element* pSymbolNames =
          pChild->GetElement("", pstrSymbolNames.AsStringC());
      if (pSymbolNames) {
        if ((!!pSymbolNames->GetAttrInteger("abbr")) != bAbbr) {
          pSymbolNames = pChild->GetElement("", pstrSymbolNames.AsStringC(), 1);
        }
        if (pSymbolNames && (!!pSymbolNames->GetAttrInteger("abbr")) == bAbbr) {
          CXML_Element* pSymbolName =
              pSymbolNames->GetElement("", symbol, index);
          if (pSymbolName)
            wsSymbolName = pSymbolName->GetContent(0);
        }
      }
    }
  }
  return wsSymbolName;
}

void CXFA_XMLLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "datePatterns");
  if (!pElement)
    return;

  CFX_WideString wsName;
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsName = L"short";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
      wsName = L"med";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsName = L"full";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsName = L"long";
      break;
  }
  GetPattern(pElement, "datePattern", wsName.AsStringC(), wsPattern);
}

void CXFA_XMLLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "timePatterns");
  if (!pElement)
    return;

  CFX_WideString wsName;
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsName = L"short";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
      wsName = L"med";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsName = L"full";
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsName = L"long";
      break;
  }
  GetPattern(pElement, "timePattern", wsName.AsStringC(), wsPattern);
}

void CXFA_XMLLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                                   CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "numberPatterns");
  if (!pElement)
    return;

  wsPattern = XFA_PatternToString(eType);
}

void CXFA_XMLLocale::GetPattern(CXML_Element* pElement,
                                const CFX_ByteStringC& bsTag,
                                const CFX_WideStringC& wsName,
                                CFX_WideString& wsPattern) const {
  int32_t iCount = pElement->CountElements("", bsTag);
  for (int32_t i = 0; i < iCount; i++) {
    CXML_Element* pChild = pElement->GetElement("", bsTag, i);
    if (pChild->GetAttrValue("name") == wsName) {
      wsPattern = pChild->GetContent(0);
      return;
    }
  }
}
