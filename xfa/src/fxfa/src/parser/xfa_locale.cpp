// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcrt/fx_xml.h"
#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_locale.h"

static const FX_WCHAR* g_FX_Percent = L"z,zzz,zzz,zzz,zzz,zzz%";
static const FX_WCHAR* g_FX_Currency = L"$z,zzz,zzz,zzz,zzz,zz9.99";
static const FX_WCHAR* g_FX_Decimal = L"z,zzz,zzz,zzz,zzz,zz9.zzz";
static const FX_WCHAR* g_FX_Integer = L"z,zzz,zzz,zzz,zzz,zzz";
CXFA_XMLLocale::CXFA_XMLLocale(CXML_Element* pLocaleData) {
  m_pLocaleData = pLocaleData;
}
CXFA_XMLLocale::~CXFA_XMLLocale() {
  if (m_pLocaleData) {
    delete m_pLocaleData;
  }
}
void CXFA_XMLLocale::Release() {
  delete this;
}
CFX_WideString CXFA_XMLLocale::GetName() {
  return m_pLocaleData ? m_pLocaleData->GetAttrValue("name") : CFX_WideString();
}
void CXFA_XMLLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                       CFX_WideString& wsNumSymbol) const {
  CFX_ByteString bsSymbols;
  CFX_WideString wsName;
  switch (eType) {
    case FX_LOCALENUMSYMBOL_Decimal:
      bsSymbols = "numberSymbols";
      wsName = FX_WSTRC(L"decimal");
      break;
    case FX_LOCALENUMSYMBOL_Grouping:
      bsSymbols = "numberSymbols";
      wsName = FX_WSTRC(L"grouping");
      break;
    case FX_LOCALENUMSYMBOL_Percent:
      bsSymbols = "numberSymbols";
      wsName = FX_WSTRC(L"percent");
      break;
    case FX_LOCALENUMSYMBOL_Minus:
      bsSymbols = "numberSymbols";
      wsName = FX_WSTRC(L"minus");
      break;
    case FX_LOCALENUMSYMBOL_Zero:
      bsSymbols = "numberSymbols";
      wsName = FX_WSTRC(L"zero");
      break;
    case FX_LOCALENUMSYMBOL_CurrencySymbol:
      bsSymbols = "currencySymbols";
      wsName = FX_WSTRC(L"symbol");
      break;
    case FX_LOCALENUMSYMBOL_CurrencyName:
      bsSymbols = "currencySymbols";
      wsName = FX_WSTRC(L"isoname");
      break;
    default:
      return;
  }
  CXML_Element* pElement = m_pLocaleData->GetElement("", bsSymbols);
  if (!pElement) {
    return;
  }
  GetPattern(pElement, CFX_ByteStringC((const FX_CHAR*)bsSymbols,
                                       bsSymbols.GetLength() - 1),
             wsName, wsNumSymbol);
}
void CXFA_XMLLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const {
  if (!m_pLocaleData) {
    return;
  }
  CFX_ByteString bsSpace;
  CXML_Element* pNumberSymbols =
      m_pLocaleData->GetElement(bsSpace, "dateTimeSymbols");
  if (!pNumberSymbols) {
    return;
  }
  wsDtSymbol = pNumberSymbols->GetContent(0);
}
void CXFA_XMLLocale::GetMonthName(int32_t nMonth,
                                  CFX_WideString& wsMonthName,
                                  FX_BOOL bAbbr) const {
  wsMonthName = GetCalendarSymbol("month", nMonth, bAbbr);
}
void CXFA_XMLLocale::GetDayName(int32_t nWeek,
                                CFX_WideString& wsDayName,
                                FX_BOOL bAbbr) const {
  wsDayName = GetCalendarSymbol("day", nWeek, bAbbr);
}
void CXFA_XMLLocale::GetMeridiemName(CFX_WideString& wsMeridiemName,
                                     FX_BOOL bAM) const {
  wsMeridiemName = GetCalendarSymbol("meridiem", bAM ? 0 : 1, FALSE);
}
void CXFA_XMLLocale::GetTimeZone(FX_TIMEZONE& tz) const {
  IXFA_TimeZoneProvider* pProvider = IXFA_TimeZoneProvider::Get();
  pProvider->GetTimeZone(tz);
}
void CXFA_XMLLocale::GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD) const {
  wsEraName = GetCalendarSymbol("era", bAD ? 1 : 0, FALSE);
}
CFX_WideString CXFA_XMLLocale::GetCalendarSymbol(const CFX_ByteStringC& symbol,
                                                 int index,
                                                 FX_BOOL bAbbr) const {
  CFX_ByteString pstrSymbolNames = symbol + "Names";
  CFX_WideString wsSymbolName = L"";
  if (m_pLocaleData) {
    CXML_Element* pChild = m_pLocaleData->GetElement("", "calendarSymbols");
    if (pChild) {
      CXML_Element* pSymbolNames = pChild->GetElement("", pstrSymbolNames);
      if (pSymbolNames) {
        if (pSymbolNames->GetAttrInteger("abbr") != bAbbr) {
          pSymbolNames = pChild->GetElement("", pstrSymbolNames, 1);
        }
        if (pSymbolNames && pSymbolNames->GetAttrInteger("abbr") == bAbbr) {
          CXML_Element* pSymbolName =
              pSymbolNames->GetElement("", symbol, index);
          if (pSymbolName) {
            wsSymbolName = pSymbolName->GetContent(0);
          }
        }
      }
    }
  }
  return wsSymbolName;
}
void CXFA_XMLLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "datePatterns");
  if (pElement == NULL) {
    return;
  }
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
  GetPattern(pElement, "datePattern", wsName, wsPattern);
}
void CXFA_XMLLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "timePatterns");
  if (pElement == NULL) {
    return;
  }
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
  GetPattern(pElement, "timePattern", wsName, wsPattern);
}
void CXFA_XMLLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                                   CFX_WideString& wsPattern) const {
  CXML_Element* pElement = m_pLocaleData->GetElement("", "numberPatterns");
  if (pElement == NULL) {
    return;
  }
  switch (eType) {
    case FX_LOCALENUMPATTERN_Percent:
      wsPattern = g_FX_Percent;
      break;
    case FX_LOCALENUMPATTERN_Currency:
      wsPattern = g_FX_Currency;
      break;
    case FX_LOCALENUMPATTERN_Decimal:
      wsPattern = g_FX_Decimal;
      break;
    case FX_LOCALENUMPATTERN_Integer:
      wsPattern = g_FX_Integer;
      break;
  }
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
CXFA_NodeLocale::CXFA_NodeLocale(CXFA_Node* pLocale) {
  m_pLocale = pLocale;
}
CXFA_NodeLocale::~CXFA_NodeLocale() {}
void CXFA_NodeLocale::Release() {
  delete this;
}
CFX_WideString CXFA_NodeLocale::GetName() {
  return m_pLocale ? m_pLocale->GetCData(XFA_ATTRIBUTE_Name) : NULL;
}
void CXFA_NodeLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                        CFX_WideString& wsNumSymbol) const {
  switch (eType) {
    case FX_LOCALENUMSYMBOL_Decimal:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_NumberSymbols, FX_WSTRC(L"decimal"));
      break;
    case FX_LOCALENUMSYMBOL_Grouping:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_NumberSymbols, FX_WSTRC(L"grouping"));
      break;
    case FX_LOCALENUMSYMBOL_Percent:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_NumberSymbols, FX_WSTRC(L"percent"));
      break;
    case FX_LOCALENUMSYMBOL_Minus:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_NumberSymbols, FX_WSTRC(L"minus"));
      break;
    case FX_LOCALENUMSYMBOL_Zero:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_NumberSymbols, FX_WSTRC(L"zero"));
      break;
    case FX_LOCALENUMSYMBOL_CurrencySymbol:
      wsNumSymbol = GetSymbol(XFA_ELEMENT_CurrencySymbols, FX_WSTRC(L"symbol"));
      break;
    case FX_LOCALENUMSYMBOL_CurrencyName:
      wsNumSymbol =
          GetSymbol(XFA_ELEMENT_CurrencySymbols, FX_WSTRC(L"isoname"));
      break;
  }
}
void CXFA_NodeLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const {
  CXFA_Node* pSymbols =
      m_pLocale ? m_pLocale->GetChild(0, XFA_ELEMENT_DateTimeSymbols) : NULL;
  wsDtSymbol = pSymbols ? pSymbols->GetContent() : CFX_WideString();
}
void CXFA_NodeLocale::GetMonthName(int32_t nMonth,
                                   CFX_WideString& wsMonthName,
                                   FX_BOOL bAbbr) const {
  wsMonthName = GetCalendarSymbol(XFA_ELEMENT_MonthNames, nMonth, bAbbr);
}
void CXFA_NodeLocale::GetDayName(int32_t nWeek,
                                 CFX_WideString& wsDayName,
                                 FX_BOOL bAbbr) const {
  wsDayName = GetCalendarSymbol(XFA_ELEMENT_DayNames, nWeek, bAbbr);
}
void CXFA_NodeLocale::GetMeridiemName(CFX_WideString& wsMeridiemName,
                                      FX_BOOL bAM) const {
  wsMeridiemName =
      GetCalendarSymbol(XFA_ELEMENT_MeridiemNames, bAM ? 0 : 1, FALSE);
}
void CXFA_NodeLocale::GetTimeZone(FX_TIMEZONE& tz) const {
  IXFA_TimeZoneProvider* pProvider = IXFA_TimeZoneProvider::Get();
  pProvider->GetTimeZone(tz);
}
void CXFA_NodeLocale::GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD) const {
  wsEraName = GetCalendarSymbol(XFA_ELEMENT_EraNames, bAD ? 1 : 0, FALSE);
}
void CXFA_NodeLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                     CFX_WideString& wsPattern) const {
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsPattern = GetSymbol(XFA_ELEMENT_DatePatterns, FX_WSTRC(L"short"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
      wsPattern = GetSymbol(XFA_ELEMENT_DatePatterns, FX_WSTRC(L"med"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsPattern = GetSymbol(XFA_ELEMENT_DatePatterns, FX_WSTRC(L"full"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsPattern = GetSymbol(XFA_ELEMENT_DatePatterns, FX_WSTRC(L"long"));
      break;
  }
}
void CXFA_NodeLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                                     CFX_WideString& wsPattern) const {
  switch (eType) {
    case FX_LOCALEDATETIMESUBCATEGORY_Short:
      wsPattern = GetSymbol(XFA_ELEMENT_TimePatterns, FX_WSTRC(L"short"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Medium:
    case FX_LOCALEDATETIMESUBCATEGORY_Default:
      wsPattern = GetSymbol(XFA_ELEMENT_TimePatterns, FX_WSTRC(L"med"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Full:
      wsPattern = GetSymbol(XFA_ELEMENT_TimePatterns, FX_WSTRC(L"full"));
      break;
    case FX_LOCALEDATETIMESUBCATEGORY_Long:
      wsPattern = GetSymbol(XFA_ELEMENT_TimePatterns, FX_WSTRC(L"long"));
      break;
  }
}
void CXFA_NodeLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                                    CFX_WideString& wsPattern) const {
  switch (eType) {
    case FX_LOCALENUMPATTERN_Percent:
      wsPattern = g_FX_Percent;
      break;
    case FX_LOCALENUMPATTERN_Currency:
      wsPattern = g_FX_Currency;
      break;
    case FX_LOCALENUMPATTERN_Decimal:
      wsPattern = g_FX_Decimal;
      break;
    case FX_LOCALENUMPATTERN_Integer:
      wsPattern = g_FX_Integer;
      break;
  }
}
CXFA_Node* CXFA_NodeLocale::GetNodeByName(CXFA_Node* pParent,
                                          const CFX_WideStringC& wsName) const {
  CXFA_Node* pChild =
      pParent ? pParent->GetNodeItem(XFA_NODEITEM_FirstChild) : NULL;
  while (pChild) {
    CFX_WideString wsChild;
    if (pChild->GetAttribute(XFA_ATTRIBUTE_Name, wsChild)) {
      if (wsChild == wsName) {
        return pChild;
      }
    }
    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return NULL;
}
CFX_WideString CXFA_NodeLocale::GetSymbol(
    XFA_ELEMENT eElement,
    const CFX_WideStringC& symbol_type) const {
  CXFA_Node* pSymbols = m_pLocale ? m_pLocale->GetChild(0, eElement) : NULL;
  CXFA_Node* pSymbol = GetNodeByName(pSymbols, symbol_type);
  return pSymbol ? pSymbol->GetContent() : CFX_WideString();
}
CFX_WideString CXFA_NodeLocale::GetCalendarSymbol(XFA_ELEMENT eElement,
                                                  int index,
                                                  FX_BOOL bAbbr) const {
  CXFA_Node* pCalendar =
      m_pLocale ? m_pLocale->GetChild(0, XFA_ELEMENT_CalendarSymbols) : NULL;
  if (pCalendar) {
    CXFA_Node* pNode = pCalendar->GetFirstChildByClass(eElement);
    for (; pNode; pNode = pNode->GetNextSameClassSibling(eElement)) {
      if (pNode->GetBoolean(XFA_ATTRIBUTE_Abbr) == bAbbr) {
        CXFA_Node* pSymbol = pNode->GetChild(index, XFA_ELEMENT_UNKNOWN);
        return pSymbol ? pSymbol->GetContent() : CFX_WideString();
      }
    }
  }
  return CFX_WideString();
}
