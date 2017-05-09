// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xmllocale.h"

#include <utility>

#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_element.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_nodelocale.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_XMLLocale::CXFA_XMLLocale(std::unique_ptr<CXML_Element> pLocaleData)
    : m_pLocaleData(std::move(pLocaleData)) {}

CXFA_XMLLocale::~CXFA_XMLLocale() {}

CFX_WideString CXFA_XMLLocale::GetName() const {
  return m_pLocaleData ? m_pLocaleData->GetAttrValue("name") : CFX_WideString();
}

CFX_WideString CXFA_XMLLocale::GetNumbericSymbol(
    FX_LOCALENUMSYMBOL eType) const {
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
      return CFX_WideString();
  }
  CXML_Element* pElement =
      m_pLocaleData->GetElement("", bsSymbols.AsStringC(), 0);
  if (!pElement)
    return CFX_WideString();

  return GetPattern(
      pElement, CFX_ByteStringC(bsSymbols.c_str(), bsSymbols.GetLength() - 1),
      wsName.AsStringC());
}

CFX_WideString CXFA_XMLLocale::GetDateTimeSymbols() const {
  if (!m_pLocaleData)
    return CFX_WideString();

  CXML_Element* pNumberSymbols =
      m_pLocaleData->GetElement("", "dateTimeSymbols", 0);
  if (!pNumberSymbols)
    return CFX_WideString();

  CXML_Content* pContent = ToContent(pNumberSymbols->GetChild(0));
  if (!pContent)
    return CFX_WideString();

  return pContent->m_Content;
}

CFX_WideString CXFA_XMLLocale::GetMonthName(int32_t nMonth, bool bAbbr) const {
  return GetCalendarSymbol("month", nMonth, bAbbr);
}

CFX_WideString CXFA_XMLLocale::GetDayName(int32_t nWeek, bool bAbbr) const {
  return GetCalendarSymbol("day", nWeek, bAbbr);
}

CFX_WideString CXFA_XMLLocale::GetMeridiemName(bool bAM) const {
  return GetCalendarSymbol("meridiem", bAM ? 0 : 1, false);
}

FX_TIMEZONE CXFA_XMLLocale::GetTimeZone() const {
  return CXFA_TimeZoneProvider().GetTimeZone();
}

CFX_WideString CXFA_XMLLocale::GetEraName(bool bAD) const {
  return GetCalendarSymbol("era", bAD ? 1 : 0, false);
}

CFX_WideString CXFA_XMLLocale::GetCalendarSymbol(const CFX_ByteStringC& symbol,
                                                 int index,
                                                 bool bAbbr) const {
  if (!m_pLocaleData)
    return CFX_WideString();

  CXML_Element* pChild = m_pLocaleData->GetElement("", "calendarSymbols", 0);
  if (!pChild)
    return CFX_WideString();

  CFX_ByteString pstrSymbolNames = symbol + "Names";
  CXML_Element* pSymbolNames =
      pChild->GetElement("", pstrSymbolNames.AsStringC(), 0);
  if (!pSymbolNames)
    return CFX_WideString();

  if ((!!pSymbolNames->GetAttrInteger("abbr")) != bAbbr)
    pSymbolNames = pChild->GetElement("", pstrSymbolNames.AsStringC(), 1);

  if (!pSymbolNames || (!!pSymbolNames->GetAttrInteger("abbr")) != bAbbr)
    return CFX_WideString();

  CXML_Element* pSymbolName = pSymbolNames->GetElement("", symbol, index);
  if (!pSymbolName)
    return CFX_WideString();

  CXML_Content* pContent = ToContent(pSymbolName->GetChild(0));
  return pContent ? pContent->m_Content : CFX_WideString();
}

CFX_WideString CXFA_XMLLocale::GetDatePattern(
    FX_LOCALEDATETIMESUBCATEGORY eType) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "datePatterns", 0);
  if (!pElement)
    return CFX_WideString();

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
  return GetPattern(pElement, "datePattern", wsName.AsStringC());
}

CFX_WideString CXFA_XMLLocale::GetTimePattern(
    FX_LOCALEDATETIMESUBCATEGORY eType) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "timePatterns", 0);
  if (!pElement)
    return CFX_WideString();

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
  return GetPattern(pElement, "timePattern", wsName.AsStringC());
}

CFX_WideString CXFA_XMLLocale::GetNumPattern(
    FX_LOCALENUMSUBCATEGORY eType) const {
  return m_pLocaleData->GetElement("", "numberPatterns", 0)
             ? XFA_PatternToString(eType)
             : CFX_WideString();
}

CFX_WideString CXFA_XMLLocale::GetPattern(CXML_Element* pElement,
                                          const CFX_ByteStringC& bsTag,
                                          const CFX_WideStringC& wsName) const {
  int32_t iCount = pElement->CountElements("", bsTag);
  for (int32_t i = 0; i < iCount; i++) {
    CXML_Element* pChild = pElement->GetElement("", bsTag, i);
    if (pChild->GetAttrValue("name") == wsName) {
      CXML_Content* pContent = ToContent(pChild->GetChild(0));
      return pContent ? pContent->m_Content : CFX_WideString();
    }
  }
  return CFX_WideString();
}
