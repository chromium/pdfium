// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodelocale.h"

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"
#include "xfa/fxfa/parser/cxfa_datetimesymbols.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

const wchar_t kFxPercent[] = L"z,zzz,zzz,zzz,zzz,zzz%";
const wchar_t kFxCurrency[] = L"$z,zzz,zzz,zzz,zzz,zz9.99";
const wchar_t kFxDecimal[] = L"z,zzz,zzz,zzz,zzz,zz9.zzz";
const wchar_t kFxInteger[] = L"z,zzz,zzz,zzz,zzz,zzz";

}  // namespace

WideString XFA_PatternToString(LocaleIface::NumSubcategory category) {
  switch (category) {
    case LocaleIface::NumSubcategory::kPercent:
      return kFxPercent;
    case LocaleIface::NumSubcategory::kCurrency:
      return kFxCurrency;
    case LocaleIface::NumSubcategory::kDecimal:
      return kFxDecimal;
    case LocaleIface::NumSubcategory::kInteger:
      return kFxInteger;
  }
  return WideString();
}

CXFA_NodeLocale::CXFA_NodeLocale(CXFA_Node* pNode) : m_pNode(pNode) {}

CXFA_NodeLocale::~CXFA_NodeLocale() = default;

void CXFA_NodeLocale::Trace(cppgc::Visitor* visitor) const {
  GCedLocaleIface::Trace(visitor);
  visitor->Trace(m_pNode);
}

WideString CXFA_NodeLocale::GetName() const {
  return WideString(m_pNode ? m_pNode->JSObject()->GetCData(XFA_Attribute::Name)
                            : nullptr);
}

WideString CXFA_NodeLocale::GetDecimalSymbol() const {
  return GetSymbol(XFA_Element::NumberSymbols, L"decimal");
}

WideString CXFA_NodeLocale::GetGroupingSymbol() const {
  return GetSymbol(XFA_Element::NumberSymbols, L"grouping");
}

WideString CXFA_NodeLocale::GetPercentSymbol() const {
  return GetSymbol(XFA_Element::NumberSymbols, L"percent");
}

WideString CXFA_NodeLocale::GetMinusSymbol() const {
  return GetSymbol(XFA_Element::NumberSymbols, L"minus");
}

WideString CXFA_NodeLocale::GetCurrencySymbol() const {
  return GetSymbol(XFA_Element::CurrencySymbols, L"symbol");
}

WideString CXFA_NodeLocale::GetDateTimeSymbols() const {
  CXFA_DateTimeSymbols* pSymbols =
      m_pNode ? m_pNode->GetChild<CXFA_DateTimeSymbols>(
                    0, XFA_Element::DateTimeSymbols, false)
              : nullptr;
  return pSymbols ? pSymbols->JSObject()->GetContent(false) : WideString();
}

WideString CXFA_NodeLocale::GetMonthName(int32_t nMonth, bool bAbbr) const {
  return GetCalendarSymbol(XFA_Element::MonthNames, nMonth, bAbbr);
}

WideString CXFA_NodeLocale::GetDayName(int32_t nWeek, bool bAbbr) const {
  return GetCalendarSymbol(XFA_Element::DayNames, nWeek, bAbbr);
}

WideString CXFA_NodeLocale::GetMeridiemName(bool bAM) const {
  return GetCalendarSymbol(XFA_Element::MeridiemNames, bAM ? 0 : 1, false);
}

int CXFA_NodeLocale::GetTimeZoneInMinutes() const {
  return CXFA_TimeZoneProvider().GetTimeZoneInMinutes();
}

WideString CXFA_NodeLocale::GetEraName(bool bAD) const {
  return GetCalendarSymbol(XFA_Element::EraNames, bAD ? 1 : 0, false);
}

WideString CXFA_NodeLocale::GetDatePattern(DateTimeSubcategory eType) const {
  switch (eType) {
    case DateTimeSubcategory::kShort:
      return GetSymbol(XFA_Element::DatePatterns, L"short");
    case DateTimeSubcategory::kMedium:
    case DateTimeSubcategory::kDefault:
      return GetSymbol(XFA_Element::DatePatterns, L"med");
    case DateTimeSubcategory::kFull:
      return GetSymbol(XFA_Element::DatePatterns, L"full");
    case DateTimeSubcategory::kLong:
      return GetSymbol(XFA_Element::DatePatterns, L"long");
  }
  return WideString();
}

WideString CXFA_NodeLocale::GetTimePattern(DateTimeSubcategory eType) const {
  switch (eType) {
    case DateTimeSubcategory::kShort:
      return GetSymbol(XFA_Element::TimePatterns, L"short");
    case DateTimeSubcategory::kMedium:
    case DateTimeSubcategory::kDefault:
      return GetSymbol(XFA_Element::TimePatterns, L"med");
    case DateTimeSubcategory::kFull:
      return GetSymbol(XFA_Element::TimePatterns, L"full");
    case DateTimeSubcategory::kLong:
      return GetSymbol(XFA_Element::TimePatterns, L"long");
  }
  return WideString();
}

WideString CXFA_NodeLocale::GetNumPattern(NumSubcategory eType) const {
  return XFA_PatternToString(eType);
}

CXFA_Node* CXFA_NodeLocale::GetNodeByName(CXFA_Node* pParent,
                                          WideStringView wsName) const {
  CXFA_Node* pChild = pParent ? pParent->GetFirstChild() : nullptr;
  while (pChild) {
    if (pChild->JSObject()->GetAttributeByEnum(XFA_Attribute::Name) == wsName)
      return pChild;

    pChild = pChild->GetNextSibling();
  }
  return nullptr;
}

WideString CXFA_NodeLocale::GetSymbol(XFA_Element eElement,
                                      WideStringView symbol_type) const {
  CXFA_Node* pSymbols =
      m_pNode ? m_pNode->GetChild<CXFA_Node>(0, eElement, false) : nullptr;
  CXFA_Node* pSymbol = GetNodeByName(pSymbols, symbol_type);
  return pSymbol ? pSymbol->JSObject()->GetContent(false) : WideString();
}

WideString CXFA_NodeLocale::GetCalendarSymbol(XFA_Element eElement,
                                              int index,
                                              bool bAbbr) const {
  CXFA_CalendarSymbols* pCalendar =
      m_pNode ? m_pNode->GetChild<CXFA_CalendarSymbols>(
                    0, XFA_Element::CalendarSymbols, false)
              : nullptr;
  if (!pCalendar)
    return WideString();

  for (CXFA_Node* pNode = pCalendar->GetFirstChildByClass<CXFA_Node>(eElement);
       pNode; pNode = pNode->GetNextSameClassSibling<CXFA_Node>(eElement)) {
    if (pNode->JSObject()->GetBoolean(XFA_Attribute::Abbr) == bAbbr) {
      CXFA_Node* pSymbol =
          pNode->GetChild<CXFA_Node>(index, XFA_Element::Unknown, false);
      return pSymbol ? pSymbol->JSObject()->GetContent(false) : WideString();
    }
  }
  return WideString();
}
