// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodelocale.h"

#include <utility>

#include "core/fxcrt/fx_xml.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

const wchar_t g_FX_Percent[] = L"z,zzz,zzz,zzz,zzz,zzz%";
const wchar_t g_FX_Currency[] = L"$z,zzz,zzz,zzz,zzz,zz9.99";
const wchar_t g_FX_Decimal[] = L"z,zzz,zzz,zzz,zzz,zz9.zzz";
const wchar_t g_FX_Integer[] = L"z,zzz,zzz,zzz,zzz,zzz";

}  // namespace

CFX_WideString XFA_PatternToString(FX_LOCALENUMSUBCATEGORY category) {
  switch (category) {
    case FX_LOCALENUMPATTERN_Percent:
      return g_FX_Percent;
    case FX_LOCALENUMPATTERN_Currency:
      return g_FX_Currency;
    case FX_LOCALENUMPATTERN_Decimal:
      return g_FX_Decimal;
    case FX_LOCALENUMPATTERN_Integer:
      return g_FX_Integer;
  }
  return L"";
}

CXFA_NodeLocale::CXFA_NodeLocale(CXFA_Node* pLocale) : m_pLocale(pLocale) {}

CXFA_NodeLocale::~CXFA_NodeLocale() {}

CFX_WideString CXFA_NodeLocale::GetName() const {
  return CFX_WideString(m_pLocale ? m_pLocale->GetCData(XFA_ATTRIBUTE_Name)
                                  : nullptr);
}

void CXFA_NodeLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                        CFX_WideString& wsNumSymbol) const {
  switch (eType) {
    case FX_LOCALENUMSYMBOL_Decimal:
      wsNumSymbol = GetSymbol(XFA_Element::NumberSymbols, L"decimal");
      break;
    case FX_LOCALENUMSYMBOL_Grouping:
      wsNumSymbol = GetSymbol(XFA_Element::NumberSymbols, L"grouping");
      break;
    case FX_LOCALENUMSYMBOL_Percent:
      wsNumSymbol = GetSymbol(XFA_Element::NumberSymbols, L"percent");
      break;
    case FX_LOCALENUMSYMBOL_Minus:
      wsNumSymbol = GetSymbol(XFA_Element::NumberSymbols, L"minus");
      break;
    case FX_LOCALENUMSYMBOL_Zero:
      wsNumSymbol = GetSymbol(XFA_Element::NumberSymbols, L"zero");
      break;
    case FX_LOCALENUMSYMBOL_CurrencySymbol:
      wsNumSymbol = GetSymbol(XFA_Element::CurrencySymbols, L"symbol");
      break;
    case FX_LOCALENUMSYMBOL_CurrencyName:
      wsNumSymbol = GetSymbol(XFA_Element::CurrencySymbols, L"isoname");
      break;
  }
}

void CXFA_NodeLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const {
  CXFA_Node* pSymbols =
      m_pLocale ? m_pLocale->GetChild(0, XFA_Element::DateTimeSymbols)
                : nullptr;
  wsDtSymbol = pSymbols ? pSymbols->GetContent() : CFX_WideString();
}

void CXFA_NodeLocale::GetMonthName(int32_t nMonth,
                                   CFX_WideString& wsMonthName,
                                   bool bAbbr) const {
  wsMonthName = GetCalendarSymbol(XFA_Element::MonthNames, nMonth, bAbbr);
}

void CXFA_NodeLocale::GetDayName(int32_t nWeek,
                                 CFX_WideString& wsDayName,
                                 bool bAbbr) const {
  wsDayName = GetCalendarSymbol(XFA_Element::DayNames, nWeek, bAbbr);
}

void CXFA_NodeLocale::GetMeridiemName(CFX_WideString& wsMeridiemName,
                                      bool bAM) const {
  wsMeridiemName =
      GetCalendarSymbol(XFA_Element::MeridiemNames, bAM ? 0 : 1, false);
}

void CXFA_NodeLocale::GetTimeZone(FX_TIMEZONE* tz) const {
  CXFA_TimeZoneProvider provider;
  provider.GetTimeZone(tz);
}

void CXFA_NodeLocale::GetEraName(CFX_WideString& wsEraName, bool bAD) const {
  wsEraName = GetCalendarSymbol(XFA_Element::EraNames, bAD ? 1 : 0, false);
}

void CXFA_NodeLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                     CFX_WideString& wsPattern) const {
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsPattern = GetSymbol(XFA_Element::DatePatterns, L"short");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
      wsPattern = GetSymbol(XFA_Element::DatePatterns, L"med");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsPattern = GetSymbol(XFA_Element::DatePatterns, L"full");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsPattern = GetSymbol(XFA_Element::DatePatterns, L"long");
      break;
  }
}

void CXFA_NodeLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                     CFX_WideString& wsPattern) const {
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsPattern = GetSymbol(XFA_Element::TimePatterns, L"short");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
      wsPattern = GetSymbol(XFA_Element::TimePatterns, L"med");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsPattern = GetSymbol(XFA_Element::TimePatterns, L"full");
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsPattern = GetSymbol(XFA_Element::TimePatterns, L"long");
      break;
  }
}

void CXFA_NodeLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  wsPattern = XFA_PatternToString(eType);
}

CXFA_Node* CXFA_NodeLocale::GetNodeByName(CXFA_Node* pParent,
                                          const CFX_WideStringC& wsName) const {
  CXFA_Node* pChild =
      pParent ? pParent->GetNodeItem(XFA_NODEITEM_FirstChild) : nullptr;
  while (pChild) {
    CFX_WideString wsChild;
    if (pChild->GetAttribute(XFA_ATTRIBUTE_Name, wsChild)) {
      if (wsChild == wsName)
        return pChild;
    }
    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return nullptr;
}

CFX_WideString CXFA_NodeLocale::GetSymbol(
    XFA_Element eElement,
    const CFX_WideStringC& symbol_type) const {
  CXFA_Node* pSymbols = m_pLocale ? m_pLocale->GetChild(0, eElement) : nullptr;
  CXFA_Node* pSymbol = GetNodeByName(pSymbols, symbol_type);
  return pSymbol ? pSymbol->GetContent() : CFX_WideString();
}

CFX_WideString CXFA_NodeLocale::GetCalendarSymbol(XFA_Element eElement,
                                                  int index,
                                                  bool bAbbr) const {
  CXFA_Node* pCalendar =
      m_pLocale ? m_pLocale->GetChild(0, XFA_Element::CalendarSymbols)
                : nullptr;
  if (pCalendar) {
    CXFA_Node* pNode = pCalendar->GetFirstChildByClass(eElement);
    for (; pNode; pNode = pNode->GetNextSameClassSibling(eElement)) {
      if (pNode->GetBoolean(XFA_ATTRIBUTE_Abbr) == bAbbr) {
        CXFA_Node* pSymbol = pNode->GetChild(index, XFA_Element::Unknown);
        return pSymbol ? pSymbol->GetContent() : CFX_WideString();
      }
    }
  }
  return CFX_WideString();
}
