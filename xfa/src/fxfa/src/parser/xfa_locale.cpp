// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_utils.h"
#include "../common/xfa_object.h"
#include "../common/xfa_document.h"
#include "../common/xfa_parser.h"
#include "../common/xfa_script.h"
#include "../common/xfa_docdata.h"
#include "../common/xfa_doclayout.h"
#include "../common/xfa_debug.h"
#include "../common/xfa_localemgr.h"
#include "../common/xfa_fm2jsapi.h"
#include "xfa_debug_parser.h"
#include "xfa_locale.h"
const static FX_LPCWSTR g_FX_Percent =  (FX_LPCWSTR)L"z,zzz,zzz,zzz,zzz,zzz%";
const static FX_LPCWSTR g_FX_Currency = (FX_LPCWSTR)L"$z,zzz,zzz,zzz,zzz,zz9.99";
const static FX_LPCWSTR g_FX_Decimal = (FX_LPCWSTR)L"z,zzz,zzz,zzz,zzz,zz9.zzz";
const static FX_LPCWSTR g_FX_Integer = (FX_LPCWSTR)L"z,zzz,zzz,zzz,zzz,zzz";
CXFA_XMLLocale::CXFA_XMLLocale(CXML_Element* pLocaleData)
{
    m_pLocaleData = pLocaleData;
}
CXFA_XMLLocale::~CXFA_XMLLocale()
{
    if (m_pLocaleData) {
        delete m_pLocaleData;
    }
}
void CXFA_XMLLocale::Release()
{
    delete this;
}
CFX_WideString CXFA_XMLLocale::GetName()
{
    return m_pLocaleData ? m_pLocaleData->GetAttrValue(FX_BSTRC("name")) : CFX_WideString();
}
void CXFA_XMLLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType, CFX_WideString& wsNumSymbol) const
{
    CFX_ByteString bsSymbols;
    CFX_WideString wsName;
    switch (eType) {
        case FX_LOCALENUMSYMBOL_Decimal:
            bsSymbols = FX_BSTRC("numberSymbols");
            wsName = FX_WSTRC(L"decimal");
            break;
        case FX_LOCALENUMSYMBOL_Grouping:
            bsSymbols = FX_BSTRC("numberSymbols");
            wsName = FX_WSTRC(L"grouping");
            break;
        case FX_LOCALENUMSYMBOL_Percent:
            bsSymbols = FX_BSTRC("numberSymbols");
            wsName = FX_WSTRC(L"percent");
            break;
        case FX_LOCALENUMSYMBOL_Minus:
            bsSymbols = (FX_BSTRC("numberSymbols"));
            wsName = FX_WSTRC(L"minus");
            break;
        case FX_LOCALENUMSYMBOL_Zero:
            bsSymbols = (FX_BSTRC("numberSymbols"));
            wsName = FX_WSTRC(L"zero");
            break;
        case FX_LOCALENUMSYMBOL_CurrencySymbol:
            bsSymbols = (FX_BSTRC("currencySymbols"));
            wsName = FX_WSTRC(L"symbol");
            break;
        case FX_LOCALENUMSYMBOL_CurrencyName:
            bsSymbols = (FX_BSTRC("currencySymbols"));
            wsName = FX_WSTRC(L"isoname");
            break;
        default:
            return;
    }
    CXML_Element* pElement = m_pLocaleData->GetElement(FX_BSTRC(""), bsSymbols);
    if (!pElement) {
        return;
    }
    GetPattern(pElement, CFX_ByteStringC((FX_LPCSTR)bsSymbols, bsSymbols.GetLength() - 1), wsName, wsNumSymbol);
}
void CXFA_XMLLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const
{
    if (!m_pLocaleData) {
        return;
    }
    CFX_ByteString bsSpace;
    CXML_Element* pNumberSymbols = m_pLocaleData->GetElement(bsSpace, FX_BSTRC("dateTimeSymbols"));
    if (!pNumberSymbols) {
        return;
    }
    wsDtSymbol = pNumberSymbols->GetContent(0);
}
void CXFA_XMLLocale::GetMonthName(FX_INT32 nMonth, CFX_WideString& wsMonthName, FX_BOOL bAbbr ) const
{
    wsMonthName = GetCalendarSymbol(FX_BSTRC("month"), nMonth, bAbbr);
}
void CXFA_XMLLocale::GetDayName(FX_INT32 nWeek, CFX_WideString& wsDayName, FX_BOOL bAbbr ) const
{
    wsDayName = GetCalendarSymbol(FX_BSTRC("day"), nWeek, bAbbr);
}
void CXFA_XMLLocale::GetMeridiemName(CFX_WideString& wsMeridiemName, FX_BOOL bAM ) const
{
    wsMeridiemName = GetCalendarSymbol(FX_BSTRC("meridiem"), bAM ? 0 : 1, FALSE);
}
void CXFA_XMLLocale::GetTimeZone(FX_TIMEZONE& tz) const
{
    IXFA_TimeZoneProvider* pProvider = IXFA_TimeZoneProvider::Get();
    pProvider->GetTimeZone(tz);
}
void CXFA_XMLLocale::GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD ) const
{
    wsEraName = GetCalendarSymbol(FX_BSTRC("era"), bAD ? 1 : 0, FALSE);
}
CFX_WideString CXFA_XMLLocale::GetCalendarSymbol(FX_BSTR symbol, int index, FX_BOOL bAbbr) const
{
    CFX_ByteString pstrSymbolNames = symbol + "Names";
    CFX_WideString wsSymbolName = L"";
    if (m_pLocaleData) {
        CXML_Element *pChild = m_pLocaleData->GetElement("", FX_BSTRC("calendarSymbols"));
        if (pChild) {
            CXML_Element *pSymbolNames = pChild->GetElement("", pstrSymbolNames);
            if (pSymbolNames) {
                if (pSymbolNames->GetAttrInteger(FX_BSTRC("abbr")) != bAbbr) {
                    pSymbolNames = pChild->GetElement("", pstrSymbolNames, 1);
                }
                if (pSymbolNames && pSymbolNames->GetAttrInteger(FX_BSTRC("abbr")) == bAbbr) {
                    CXML_Element *pSymbolName = pSymbolNames->GetElement("", symbol, index);
                    if (pSymbolName) {
                        wsSymbolName = pSymbolName->GetContent(0);
                    }
                }
            }
        }
    }
    return wsSymbolName;
}
void CXFA_XMLLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    CXML_Element* pElement = m_pLocaleData->GetElement(FX_BSTRC(""), FX_BSTRC("datePatterns"));
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
    GetPattern(pElement, FX_BSTRC("datePattern"), wsName, wsPattern);
}
void CXFA_XMLLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    CXML_Element* pElement = m_pLocaleData->GetElement(FX_BSTRC(""), FX_BSTRC("timePatterns"));
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
    GetPattern(pElement, FX_BSTRC("timePattern"), wsName, wsPattern);
}
void CXFA_XMLLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    CXML_Element* pElement = m_pLocaleData->GetElement(FX_BSTRC(""), FX_BSTRC("numberPatterns"));
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
void CXFA_XMLLocale::GetPattern(CXML_Element* pElement, FX_BSTR bsTag, FX_WSTR wsName, CFX_WideString& wsPattern) const
{
    FX_INT32 iCount = pElement->CountElements(FX_BSTRC(""), bsTag);
    for (FX_INT32 i = 0; i < iCount; i++) {
        CXML_Element* pChild = pElement->GetElement(FX_BSTRC(""), bsTag, i);
        if (pChild->GetAttrValue(FX_BSTRC("name")) == wsName) {
            wsPattern = pChild->GetContent(0);
            return;
        }
    }
}
CXFA_NodeLocale::CXFA_NodeLocale(CXFA_Node *pLocale)
{
    m_pLocale = pLocale;
}
CXFA_NodeLocale::~CXFA_NodeLocale()
{
}
void CXFA_NodeLocale::Release()
{
    delete this;
}
CFX_WideString CXFA_NodeLocale::GetName()
{
    return m_pLocale ? m_pLocale->GetCData(XFA_ATTRIBUTE_Name) : NULL;
}
void CXFA_NodeLocale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType, CFX_WideString& wsNumSymbol) const
{
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
            wsNumSymbol = GetSymbol(XFA_ELEMENT_CurrencySymbols, FX_WSTRC(L"isoname"));
            break;
    }
}
void CXFA_NodeLocale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const
{
    CXFA_Node *pSymbols = m_pLocale ? m_pLocale->GetChild(0, XFA_ELEMENT_DateTimeSymbols) : NULL;
    wsDtSymbol = pSymbols ? pSymbols->GetContent() : CFX_WideString();
}
void CXFA_NodeLocale::GetMonthName(FX_INT32 nMonth, CFX_WideString& wsMonthName, FX_BOOL bAbbr ) const
{
    wsMonthName = GetCalendarSymbol(XFA_ELEMENT_MonthNames, nMonth, bAbbr);
}
void CXFA_NodeLocale::GetDayName(FX_INT32 nWeek, CFX_WideString& wsDayName, FX_BOOL bAbbr ) const
{
    wsDayName = GetCalendarSymbol(XFA_ELEMENT_DayNames, nWeek, bAbbr);
}
void CXFA_NodeLocale::GetMeridiemName(CFX_WideString& wsMeridiemName, FX_BOOL bAM ) const
{
    wsMeridiemName = GetCalendarSymbol(XFA_ELEMENT_MeridiemNames, bAM ? 0 : 1, FALSE);
}
void CXFA_NodeLocale::GetTimeZone(FX_TIMEZONE& tz) const
{
    IXFA_TimeZoneProvider* pProvider = IXFA_TimeZoneProvider::Get();
    pProvider->GetTimeZone(tz);
}
void CXFA_NodeLocale::GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD ) const
{
    wsEraName = GetCalendarSymbol(XFA_ELEMENT_EraNames, bAD ? 1 : 0, FALSE);
}
void CXFA_NodeLocale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
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
void CXFA_NodeLocale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
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
void CXFA_NodeLocale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType, CFX_WideString& wsPattern) const
{
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
CXFA_Node* CXFA_NodeLocale::GetNodeByName(CXFA_Node *pParent, FX_WSTR wsName) const
{
    CXFA_Node *pChild = pParent ? pParent->GetNodeItem(XFA_NODEITEM_FirstChild) : NULL;
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
CFX_WideString CXFA_NodeLocale::GetSymbol(XFA_ELEMENT eElement, FX_WSTR symbol_type) const
{
    CXFA_Node *pSymbols = m_pLocale ? m_pLocale->GetChild(0, eElement) : NULL;
    CXFA_Node *pSymbol = GetNodeByName(pSymbols, symbol_type);
    return pSymbol ? pSymbol->GetContent() : CFX_WideString();
}
CFX_WideString CXFA_NodeLocale::GetCalendarSymbol(XFA_ELEMENT eElement, int index, FX_BOOL bAbbr) const
{
    CXFA_Node *pCalendar = m_pLocale ? m_pLocale->GetChild(0, XFA_ELEMENT_CalendarSymbols) : NULL;
    if (pCalendar) {
        CXFA_Node *pNode = pCalendar->GetFirstChildByClass(eElement);
        for (; pNode; pNode = pNode->GetNextSameClassSibling(eElement)) {
            if (pNode->GetBoolean(XFA_ATTRIBUTE_Abbr) == bAbbr) {
                CXFA_Node *pSymbol = pNode->GetChild(index, XFA_ELEMENT_UNKNOWN);
                return pSymbol ? pSymbol->GetContent() : CFX_WideString();
            }
        }
    }
    return CFX_WideString();
}
