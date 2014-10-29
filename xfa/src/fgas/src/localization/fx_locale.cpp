// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_localeimp.h"
#define FX_LOCALECATEGORY_DateHash			0xbde9abde
#define FX_LOCALECATEGORY_TimeHash			0x2d71b00f
#define FX_LOCALECATEGORY_DateTimeHash		0x158c72ed
#define FX_LOCALECATEGORY_NumHash			0x0b4ff870
#define FX_LOCALECATEGORY_TextHash			0x2d08af85
#define FX_LOCALECATEGORY_ZeroHash			0x568cb500
#define	FX_LOCALECATEGORY_NullHash			0x052931bb
typedef struct _FX_LOCALESUBCATEGORYINFO {
    FX_UINT32					uHash;
    FX_LPCWSTR					pName;
    FX_INT32					eSubCategory;
} FX_LOCALESUBCATEGORYINFO, * FX_LPLOCALESUBCATEGORYINFO;
typedef FX_LOCALESUBCATEGORYINFO const * FX_LPCLOCALESUBCATEGORYINFO;
const static FX_LOCALESUBCATEGORYINFO g_FXLocaleDateTimeSubCatData[] = {
    {0x14da2125, (FX_LPCWSTR)L"default", FX_LOCALEDATETIMESUBCATEGORY_Default},
    {0x9041d4b0, (FX_LPCWSTR)L"short", FX_LOCALEDATETIMESUBCATEGORY_Short},
    {0xa084a381, (FX_LPCWSTR)L"medium", FX_LOCALEDATETIMESUBCATEGORY_Medium},
    {0xcdce56b3, (FX_LPCWSTR)L"full", FX_LOCALEDATETIMESUBCATEGORY_Full},
    {0xf6b4afb0, (FX_LPCWSTR)L"long", FX_LOCALEDATETIMESUBCATEGORY_Long},
};
const static FX_INT32 g_iFXLocaleDateTimeSubCatCount = sizeof(g_FXLocaleDateTimeSubCatData) / sizeof(FX_LOCALESUBCATEGORYINFO);
const static FX_LOCALESUBCATEGORYINFO g_FXLocaleNumSubCatData[] = {
    {0x46f95531, (FX_LPCWSTR)L"percent", FX_LOCALENUMPATTERN_Percent},
    {0x4c4e8acb, (FX_LPCWSTR)L"currency", FX_LOCALENUMPATTERN_Currency},
    {0x54034c2f, (FX_LPCWSTR)L"decimal", FX_LOCALENUMPATTERN_Decimal},
    {0x7568e6ae, (FX_LPCWSTR)L"integer", FX_LOCALENUMPATTERN_Integer},
};
const static FX_INT32 g_iFXLocaleNumSubCatCount = sizeof(g_FXLocaleNumSubCatData) / sizeof(FX_LOCALESUBCATEGORYINFO);
typedef struct _FX_LOCALETIMEZONEINFO {
    FX_DWORD					uHash;
    FX_INT16					iHour;
    FX_INT16					iMinute;
} FX_LOCALETIMEZONEINFO, * FX_LPLOCALETIMEZONEINFO;
typedef FX_LOCALETIMEZONEINFO const * FX_LPCLOCALETIMEZONEINFO;
const static FX_LOCALETIMEZONEINFO g_FXLocaleTimeZoneData[] = {
    {FXBSTR_ID(0, 'C', 'D', 'T'), -5, 0}, {FXBSTR_ID(0, 'C', 'S', 'T'), -6, 0},
    {FXBSTR_ID(0, 'E', 'D', 'T'), -4, 0}, {FXBSTR_ID(0, 'E', 'S', 'T'), -5, 0},
    {FXBSTR_ID(0, 'M', 'D', 'T'), -6, 0}, {FXBSTR_ID(0, 'M', 'S', 'T'), -7, 0},
    {FXBSTR_ID(0, 'P', 'D', 'T'), -7, 0}, {FXBSTR_ID(0, 'P', 'S', 'T'), -8, 0},
};
const static FX_INT32 g_iFXLocaleTimeZoneCount = sizeof(g_FXLocaleTimeZoneData) / sizeof(FX_LOCALETIMEZONEINFO);
const static CFX_WideStringC gs_wsTextSymbols = FX_WSTRC(L"AXO09");
const static CFX_WideStringC gs_wsTimeSymbols = FX_WSTRC(L"hHkKMSFAzZ");
const static CFX_WideStringC gs_wsDateSymbols = FX_WSTRC(L"DJMEeGgYwW");
const static CFX_WideStringC gs_wsConstChars = FX_WSTRC(L",-:/. ");
static FX_STRSIZE FX_Local_Find(FX_WSTR wsSymbols, FX_WCHAR ch, FX_STRSIZE nStart = 0)
{
    FX_STRSIZE nLength = wsSymbols.GetLength();
    if (nLength < 1 || nStart > nLength) {
        return -1;
    }
    FX_LPCWSTR lpsz = (FX_LPCWSTR)FXSYS_wcschr(wsSymbols.GetPtr() + nStart, ch);
    return (lpsz == NULL) ? -1 : (FX_STRSIZE)(lpsz - wsSymbols.GetPtr());
}
const static FX_LPCWSTR gs_LocalNumberSymbols[] = {
    (FX_LPCWSTR)L"decimal", (FX_LPCWSTR)L"grouping", (FX_LPCWSTR)L"percent", (FX_LPCWSTR)L"minus", (FX_LPCWSTR)L"zero",
    (FX_LPCWSTR)L"currencySymbol", (FX_LPCWSTR)L"currencyName",
};
IFX_Locale* IFX_Locale::Create(CXML_Element* pLocaleData)
{
    return FX_NEW CFX_Locale(pLocaleData);
}
CFX_Locale::CFX_Locale(CXML_Element* pLocaleData)
{
    m_pElement = pLocaleData;
}
CFX_Locale::~CFX_Locale()
{
}
CFX_WideString CFX_Locale::GetName()
{
    return CFX_WideString();
}
static CFX_WideString FX_GetXMLContent(FX_BSTR bsSpace, CXML_Element* pxmlElement, FX_BSTR bsTag, FX_WSTR wsName)
{
    CXML_Element* pDatePattern = NULL;
    FX_INT32 nCount = pxmlElement->CountElements(bsSpace, bsTag);
    FX_INT32 i = 0;
    for (; i < nCount; i++) {
        pDatePattern = pxmlElement->GetElement(bsSpace, bsTag, i);
        if (pDatePattern->GetAttrValue(FX_BSTRC("name")) == wsName) {
            break;
        }
    }
    if (i < nCount && pDatePattern) {
        return pDatePattern->GetContent(0);
    }
    return L"";
}
void CFX_Locale::GetNumbericSymbol(FX_LOCALENUMSYMBOL eType, CFX_WideString& wsNumSymbol) const
{
    if (!m_pElement) {
        return;
    }
    CFX_ByteString bsSpace;
    CFX_WideString wsName = gs_LocalNumberSymbols[eType];
    CXML_Element* pNumberSymbols = m_pElement->GetElement(bsSpace, FX_BSTRC("numberSymbols"));
    if (!pNumberSymbols) {
        return;
    }
    wsNumSymbol = FX_GetXMLContent(bsSpace, pNumberSymbols, FX_BSTRC("numberSymbol"), wsName);
}
void CFX_Locale::GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const
{
    if (!m_pElement) {
        return;
    }
    CFX_ByteString bsSpace;
    CXML_Element* pNumberSymbols = m_pElement->GetElement(bsSpace, FX_BSTRC("dateTimeSymbols"));
    if (!pNumberSymbols) {
        return;
    }
    wsDtSymbol = pNumberSymbols->GetContent(0);
}
static void FX_GetCalendarSymbol(CXML_Element* pXmlElement, const CFX_ByteString &symbol_type, FX_INT32 index, FX_BOOL bAbbr, CFX_WideString &wsName)
{
    CFX_ByteString bsSpace;
    CFX_ByteString pstrSymbolNames = symbol_type + FX_BSTRC("Names");
    CXML_Element* pChild = pXmlElement->GetElement(bsSpace, FX_BSTRC("calendarSymbols"));
    if (!pChild) {
        return;
    }
    CXML_Element* pSymbolNames = pChild->GetElement(bsSpace, pstrSymbolNames);
    if (!pSymbolNames) {
        return;
    }
    if (pSymbolNames->GetAttrInteger(FX_BSTRC("abbr")) != bAbbr) {
        pSymbolNames = pChild->GetElement(bsSpace, pstrSymbolNames, 1);
    }
    if (pSymbolNames && pSymbolNames->GetAttrInteger(FX_BSTRC("abbr")) == bAbbr) {
        CXML_Element* pSymbolName = pSymbolNames->GetElement(bsSpace, symbol_type, index);
        if (pSymbolName) {
            wsName = pSymbolName->GetContent(0);
        }
    }
}
void CFX_Locale::GetMonthName(FX_INT32 nMonth, CFX_WideString& wsMonthName, FX_BOOL bAbbr ) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetCalendarSymbol(m_pElement, "month", nMonth, bAbbr, wsMonthName);
}
void CFX_Locale::GetDayName(FX_INT32 nWeek, CFX_WideString& wsDayName, FX_BOOL bAbbr ) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetCalendarSymbol(m_pElement, "day", nWeek, bAbbr, wsDayName);
}
void CFX_Locale::GetMeridiemName(CFX_WideString& wsMeridiemName, FX_BOOL bAM ) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetCalendarSymbol(m_pElement, "meridiem", bAM ? 0 : 1, FALSE, wsMeridiemName);
}
static FX_INT32 FX_ParseTimeZone(FX_LPCWSTR pStr, FX_INT32 iLen, FX_TIMEZONE& tz)
{
    tz.tzHour = 0;
    tz.tzMinute = 0;
    if (iLen < 0) {
        return 0;
    }
    FX_INT32 iStart = 1;
    FX_INT32 iEnd = iStart + 2;
    while (iStart < iLen && iStart < iEnd) {
        tz.tzHour = tz.tzHour * 10 + pStr[iStart++] - '0';
    }
    if (iStart < iLen && pStr[iStart] == ':') {
        iStart++;
    }
    iEnd = iStart + 2;
    while (iStart < iLen && iStart < iEnd) {
        tz.tzMinute = tz.tzMinute * 10 + pStr[iStart++] - '0';
    }
    if (pStr[0] == '-') {
        tz.tzHour = -tz.tzHour;
    }
    return iStart;
}
void CFX_Locale::GetTimeZone(FX_TIMEZONE& tz) const
{
    tz.tzHour = 0;
    tz.tzMinute = 0;
    if (!m_pElement) {
        return;
    }
    CXML_Element* pxmlTimeZone = m_pElement->GetElement("", FX_BSTRC("timeZone"));
    if (pxmlTimeZone) {
        CFX_WideString wsTimeZone = pxmlTimeZone->GetContent(0);
        FX_ParseTimeZone(wsTimeZone, wsTimeZone.GetLength(), tz);
    }
}
void CFX_Locale::GetEraName(CFX_WideString& wsEraName, FX_BOOL bAD ) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetCalendarSymbol(m_pElement, "era", bAD ? 0 : 1, FALSE, wsEraName);
}
static void FX_GetPattern(CXML_Element* pXmlElement, const CFX_ByteString& bsCategory, const CFX_WideString& wsSubCategory, CFX_WideString& wsPattern)
{
    CFX_ByteString bsSpace;
    CXML_Element* pDatePatterns = pXmlElement->GetElement(bsSpace, bsCategory + "s");
    if (!pDatePatterns) {
        return;
    }
    wsPattern = FX_GetXMLContent(bsSpace, pDatePatterns, bsCategory, wsSubCategory);
}
static void FX_GetDateTimePattern(CXML_Element* pXmlElement, const CFX_ByteString& bsCategory, FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern)
{
    CFX_WideString wsType = g_FXLocaleDateTimeSubCatData[eType].pName;
    FX_GetPattern(pXmlElement, bsCategory, wsType, wsPattern);
}
void CFX_Locale::GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetDateTimePattern(m_pElement, "datePattern", eType, wsPattern);
}
void CFX_Locale::GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    if (!m_pElement) {
        return;
    }
    FX_GetDateTimePattern(m_pElement, "timePattern", eType, wsPattern);
}
void CFX_Locale::GetNumPattern(FX_LOCALENUMSUBCATEGORY eType, CFX_WideString& wsPattern) const
{
    CFX_WideString wsType = g_FXLocaleNumSubCatData[eType].pName;
    FX_GetPattern(m_pElement, "numberPattern", wsType, wsPattern);
}
static FX_BOOL FX_IsDigit(FX_WCHAR c)
{
    return c >= '0' && c <= '9';
}
static FX_BOOL FX_IsAlpha(FX_WCHAR c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static FX_BOOL FX_IsSpace(FX_WCHAR c)
{
    return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}
static const FX_FLOAT gs_fraction_scales[] = {0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f,
                                              0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f, 0.00000000001f
                                             };
static const FX_INT32 gs_fraction_count = sizeof(gs_fraction_scales) / sizeof(FX_FLOAT);
class CFX_LCNumeric : public CFX_Object
{
public:
    CFX_LCNumeric();
    CFX_LCNumeric(FX_INT64 integral, FX_DWORD fractional = 0, FX_INT32 exponent = 0);
    CFX_LCNumeric(FX_FLOAT dbRetValue);
    CFX_LCNumeric(double dbvalue);
    CFX_LCNumeric(CFX_WideString& wsNumeric);

    FX_FLOAT				GetFloat() const;
    double					GetDouble() const;
    CFX_WideString			ToString() const;
    CFX_WideString			ToString(FX_INT32 nTreading, FX_BOOL bTrimTailZeros) const;
    FX_INT64				m_Integral;
    FX_DWORD				m_Fractional;
#ifdef FX_NUM_DOUBLE
    CFX_WideString			m_wsValue;
#endif
    FX_INT32				m_Exponent;
};
static FX_BOOL FX_WStringToNumeric(const CFX_WideString& wsValue, CFX_LCNumeric& lcnum)
{
    FX_INT64 *pIntegral = &lcnum.m_Integral;
    FX_DWORD *pFractional = &lcnum.m_Fractional;
    FX_INT32 *pExponent = &lcnum.m_Exponent;
    *pIntegral = 0;
    *pFractional = 0;
    *pExponent = 0;
#ifdef FX_NUM_DOUBLE
    lcnum.m_wsValue.Empty();
#endif
    if (wsValue.IsEmpty()) {
        return FALSE;
    }
    const FX_INT32 nIntegralMaxLen = 17;
    FX_INT32 cc = 0;
    FX_BOOL bNegative = FALSE, bExpSign = FALSE;
    FX_LPCWSTR str = (FX_LPCWSTR)wsValue;
    FX_INT32 len = wsValue.GetLength();
    while (cc < len && FX_IsSpace(str[cc])) {
        cc++;
    }
    if (cc >= len) {
        return FALSE;
    }
    if (str[cc] == '+') {
        cc++;
    } else if (str[cc] == '-') {
        bNegative = TRUE;
        cc++;
    }
    FX_INT32 nIntegralLen = 0;
    while (cc < len) {
        if (str[cc] == '.') {
            break;
        }
        if (!FX_IsDigit(str[cc])) {
            if ((str[cc] == 'E' || str[cc] == 'e')) {
                break;
            } else {
                return FALSE;
            }
        }
        if (nIntegralLen < nIntegralMaxLen) {
            *pIntegral = *pIntegral * 10 + str[cc] - '0';
            nIntegralLen++;
        }
        cc ++;
    }
    *pIntegral = bNegative ? -*pIntegral : *pIntegral;
    if (cc < len && str[cc] == '.') {
        int scale = 0;
        double fraction = 0.0;
        cc ++;
        while (cc < len) {
            if (scale >= gs_fraction_count) {
                while (cc < len) {
                    if (!FX_IsDigit(str[cc])) {
                        break;
                    }
                    cc++;
                }
            }
            if (!FX_IsDigit(str[cc])) {
                if ((str[cc] == 'E' || str[cc] == 'e')) {
                    break;
                } else {
                    return FALSE;
                }
            }
            fraction += gs_fraction_scales[scale] * (str[cc] - '0');
            scale ++;
            cc ++;
        }
        *pFractional = (FX_DWORD)(fraction * 4294967296.0);
    }
    if (cc < len && (str[cc] == 'E' || str[cc] == 'e')) {
        cc ++;
        if (cc < len) {
            if (str[cc] == '+') {
                cc++;
            } else if (str[cc] == '-') {
                bExpSign = TRUE;
                cc++;
            }
        }
        while (cc < len) {
            if (FX_IsDigit(str[cc])) {
                return FALSE;
            }
            *pExponent = *pExponent * 10 + str[cc] - '0';
            cc ++;
        }
        *pExponent = bExpSign ? -*pExponent : *pExponent;
    }
#ifdef FX_NUM_DOUBLE
    else {
        lcnum.m_wsValue = wsValue;
    }
#endif
    return TRUE;
}
CFX_LCNumeric::CFX_LCNumeric()
{
    m_Integral = 0;
    m_Fractional = 0;
    m_Exponent = 0;
}
CFX_LCNumeric::CFX_LCNumeric(FX_INT64 integral, FX_DWORD fractional , FX_INT32 exponent )
{
    m_Integral = integral;
    m_Fractional = fractional;
    m_Exponent = exponent;
}
CFX_LCNumeric::CFX_LCNumeric(FX_FLOAT dbRetValue)
{
    m_Integral = (FX_INT64)dbRetValue;
    m_Fractional = (FX_DWORD)(((dbRetValue > 0) ? (dbRetValue - m_Integral) : (m_Integral - dbRetValue)) * 4294967296);
    m_Exponent = 0;
}
CFX_LCNumeric::CFX_LCNumeric(double dbvalue)
{
    m_Integral = (FX_INT64)dbvalue;
    m_Fractional = (FX_DWORD)(((dbvalue > 0) ? (dbvalue - m_Integral) : (m_Integral - dbvalue)) * 4294967296);
    m_Exponent = 0;
}
CFX_LCNumeric::CFX_LCNumeric(CFX_WideString& wsNumeric)
{
    FX_WStringToNumeric(wsNumeric, *this);
}
FX_FLOAT CFX_LCNumeric::GetFloat() const
{
    FX_FLOAT dbRetValue = m_Fractional / 4294967296.0f;
    dbRetValue = m_Integral + (m_Integral >= 0 ? dbRetValue : -dbRetValue);
    if (m_Exponent != 0) {
        dbRetValue *= FXSYS_pow(10, (FX_FLOAT)m_Exponent);
    }
    return dbRetValue;
}
double CFX_LCNumeric::GetDouble() const
{
    double value = m_Fractional / 4294967296.0;
    value = m_Integral + (m_Integral >= 0 ? value : -value);
    if (m_Exponent != 0) {
        value *= FXSYS_pow(10, (FX_FLOAT)m_Exponent);
    }
    return value;
}
CFX_WideString CFX_LCNumeric::ToString() const
{
    return ToString(8, TRUE);
}
CFX_WideString CFX_LCNumeric::ToString(FX_INT32 nTreading, FX_BOOL bTrimTailZeros) const
{
#ifdef FX_NUM_DOUBLE
    CFX_WideString wsResult;
    if (!m_wsValue.IsEmpty()) {
        const FX_INT32 nIntegralMaxLen = 17;
        FX_INT32 cc = 0;
        FX_BOOL bNegative = FALSE, bExpSign = FALSE;
        FX_LPCWSTR str = (FX_LPCWSTR)m_wsValue;
        FX_INT32 len = m_wsValue.GetLength();
        while (cc < len && FX_IsSpace(str[cc])) {
            cc++;
        }
        if (cc >= len) {
            return wsResult;
        }
        if (str[cc] == '+') {
            cc++;
        } else if (str[cc] == '-') {
            bNegative = TRUE;
            cc++;
        }
        FX_INT32 nIntegralLen = 0;
        while (cc < len) {
            if (str[cc] == '.') {
                break;
            }
            if (!FX_IsDigit(str[cc])) {
                if ((str[cc] == 'E' || str[cc] == 'e')) {
                    break;
                } else {
                    return wsResult;
                }
            }
            if (nIntegralLen < nIntegralMaxLen) {
                *pIntegral = *pIntegral * 10 + str[cc] - '0';
                nIntegralLen++;
            }
            cc ++;
        }
        *pIntegral = bNegative ? -*pIntegral : *pIntegral;
        if (cc < len && str[cc] == '.') {
            int scale = 0;
            double fraction = 0.0;
            cc ++;
            while (cc < len) {
                if (scale >= gs_fraction_count) {
                    while (cc < len) {
                        if (!FX_IsDigit(str[cc])) {
                            break;
                        }
                        cc++;
                    }
                }
                if (!FX_IsDigit(str[cc])) {
                    if ((str[cc] == 'E' || str[cc] == 'e')) {
                        break;
                    } else {
                        return FALSE;
                    }
                }
                fraction += gs_fraction_scales[scale] * (str[cc] - '0');
                scale ++;
                cc ++;
            }
            *pFractional = (FX_DWORD)(fraction * 4294967296.0);
        }
    }
    double dbValeu = GetDouble();
    FX_INT64 iInte = (FX_INT64)dbValeu;
    wsResult.Format((FX_LPCWSTR)L"%l", (FX_INT64)iInte);
    if (m_Fractional) {
        CFX_WideString wsFormat;
        wsFormat.Format((FX_LPCWSTR)L"%%.%dG", nTreading);
        double dblMantissa = (dbValeu > 0) ? (dbValeu - iInte) : (iInte - dbValeu);
        CFX_WideString wsFrac;
        wsFrac.Format((FX_LPCWSTR)wsFormat, dblMantissa);
        wsResult += CFX_WideStringC((FX_LPCWSTR)wsFrac + 1, wsFrac.GetLength() - 1);
        if (bTrimTailZeros && nTreading > 0) {
            wsResult.TrimRight(L"0");
            wsResult.TrimRight(L".");
        }
    }
#endif
    CFX_WideString wsFormat;
    wsFormat.Format((FX_LPCWSTR)L"%%.%df", nTreading);
    CFX_WideString wsResult;
    wsResult.Format(FX_LPCWSTR(wsFormat), GetDouble());
    if (bTrimTailZeros && nTreading > 0) {
        wsResult.TrimRight(L"0");
        wsResult.TrimRight(L".");
    }
    return wsResult;
}
IFX_FormatString* IFX_FormatString::Create(IFX_LocaleMgr* pLocaleMgr, FX_BOOL bUseLCID)
{
    if (!pLocaleMgr) {
        return NULL;
    }
    return FX_NEW CFX_FormatString(pLocaleMgr, bUseLCID);
}
CFX_FormatString::CFX_FormatString(IFX_LocaleMgr* pLocaleMgr, FX_BOOL bUseLCID)
    : m_pLocaleMgr(pLocaleMgr)
    , m_bUseLCID(bUseLCID)
{
}
CFX_FormatString::~CFX_FormatString()
{
}
void CFX_FormatString::SplitFormatString(const CFX_WideString& wsFormatString, CFX_WideStringArray& wsPatterns)
{
    FX_INT32 iStrLen = wsFormatString.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsFormatString;
    FX_LPCWSTR pToken = pStr;
    FX_LPCWSTR pEnd = pStr + iStrLen;
    FX_BOOL iQuote = FALSE;
    while (TRUE) {
        if (pStr >= pEnd) {
            CFX_WideString sub(pToken, pStr - pToken);
            wsPatterns.Add(sub);
            return;
        } else if (*pStr == '\'') {
            iQuote = !iQuote;
        } else if (*pStr == L'|' && !iQuote) {
            CFX_WideString sub(pToken, pStr - pToken);
            wsPatterns.Add(sub);
            pToken = pStr + 1;
        }
        pStr ++;
    }
}
static CFX_WideString FX_GetLiteralText(FX_LPCWSTR pStrPattern, FX_INT32 &iPattern, FX_INT32 iLenPattern)
{
    CFX_WideString wsOutput;
    if (pStrPattern[iPattern] != '\'') {
        return wsOutput;
    }
    iPattern++;
    FX_INT32 iQuote = 1;
    while (iPattern < iLenPattern) {
        if (pStrPattern[iPattern] == '\'') {
            iQuote++;
            if ((iPattern + 1 >= iLenPattern) || ((pStrPattern[iPattern + 1] != '\'') && (iQuote % 2 == 0))) {
                break;
            } else {
                iQuote++;
            }
            iPattern++;
        } else if (pStrPattern[iPattern] == '\\' && (iPattern + 1 < iLenPattern) && pStrPattern[iPattern + 1] == 'u') {
            FX_INT32 iKeyValue = 0;
            iPattern += 2;
            FX_INT32 i = 0;
            while (iPattern < iLenPattern && i++ < 4) {
                FX_WCHAR ch = pStrPattern[iPattern++];
                if ((ch >= '0' && ch <= '9')) {
                    iKeyValue = iKeyValue * 16 + ch - '0';
                } else if ((ch >= 'a' && ch <= 'f')) {
                    iKeyValue = iKeyValue * 16 + ch - 'a' + 10;
                } else if ((ch >= 'A' && ch <= 'F')) {
                    iKeyValue = iKeyValue * 16 + ch - 'A' + 10;
                }
            }
            if (iKeyValue != 0) {
                wsOutput += (FX_WCHAR)(iKeyValue & 0x0000FFFF);
            }
            continue;
        }
        wsOutput += pStrPattern[iPattern++];
    }
    return wsOutput;
}
static CFX_WideString FX_GetLiteralTextReverse(FX_LPCWSTR pStrPattern, FX_INT32 &iPattern)
{
    CFX_WideString wsOutput;
    if (pStrPattern[iPattern] != '\'') {
        return wsOutput;
    }
    iPattern--;
    FX_INT32 iQuote = 1;
    while (iPattern >= 0) {
        if (pStrPattern[iPattern] == '\'') {
            iQuote++;
            if (iPattern - 1 >= 0 || ((pStrPattern[iPattern - 1] != '\'') && (iQuote % 2 == 0))) {
                break;
            } else {
                iQuote++;
            }
            iPattern--;
        } else if (pStrPattern[iPattern] == '\\' && pStrPattern[iPattern + 1] == 'u') {
            iPattern--;
            FX_INT32 iKeyValue = 0;
            FX_INT32 iLen = wsOutput.GetLength();
            FX_INT32 i = 1;
            for (; i < iLen && i < 5; i++) {
                FX_WCHAR ch = wsOutput[i];
                if ((ch >= '0' && ch <= '9')) {
                    iKeyValue = iKeyValue * 16 + ch - '0';
                } else if ((ch >= 'a' && ch <= 'f')) {
                    iKeyValue = iKeyValue * 16 + ch - 'a' + 10;
                } else if ((ch >= 'A' && ch <= 'F')) {
                    iKeyValue = iKeyValue * 16 + ch - 'A' + 10;
                }
            }
            if (iKeyValue != 0) {
                wsOutput.Delete(0, i);
                wsOutput = (FX_WCHAR)(iKeyValue & 0x0000FFFF) + wsOutput;
            }
            continue;
        }
        wsOutput = pStrPattern[iPattern--] + wsOutput;
    }
    return wsOutput;
}
FX_LOCALECATEGORY CFX_FormatString::GetCategory(const CFX_WideString& wsPattern)
{
    FX_LOCALECATEGORY eCategory = FX_LOCALECATEGORY_Unknown;
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsPattern.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsPattern;
    FX_BOOL bBraceOpen = FALSE;
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_GetLiteralText(pStr, ccf, iLenf);
        } else if (!bBraceOpen && FX_Local_Find(gs_wsConstChars, pStr[ccf]) < 0) {
            CFX_WideString wsCategory(pStr[ccf]);
            ccf++;
            while (TRUE) {
                if (ccf == iLenf) {
                    return eCategory;
                }
                if ( pStr[ccf] == '.' || pStr[ccf] == '(') {
                    break;
                }
                if (pStr[ccf] == '{') {
                    bBraceOpen = TRUE;
                    break;
                }
                wsCategory += pStr[ccf];
                ccf++;
            }
            FX_DWORD dwHash = FX_HashCode_String_GetW(wsCategory, wsCategory.GetLength());
            if (dwHash == FX_LOCALECATEGORY_DateHash) {
                if (eCategory == FX_LOCALECATEGORY_Time) {
                    return FX_LOCALECATEGORY_DateTime;
                }
                eCategory = FX_LOCALECATEGORY_Date;
            } else if (dwHash == FX_LOCALECATEGORY_TimeHash) {
                if (eCategory == FX_LOCALECATEGORY_Date) {
                    return FX_LOCALECATEGORY_DateTime;
                }
                eCategory = FX_LOCALECATEGORY_Time;
            } else if (dwHash == FX_LOCALECATEGORY_DateTimeHash) {
                return FX_LOCALECATEGORY_DateTime;
            } else if (dwHash == FX_LOCALECATEGORY_TextHash) {
                return FX_LOCALECATEGORY_Text;
            } else if (dwHash == FX_LOCALECATEGORY_NumHash) {
                return FX_LOCALECATEGORY_Num;
            } else if (dwHash == FX_LOCALECATEGORY_ZeroHash) {
                return FX_LOCALECATEGORY_Zero;
            } else if (dwHash == FX_LOCALECATEGORY_NullHash) {
                return FX_LOCALECATEGORY_Null;
            }
        } else if (pStr[ccf] == '}') {
            bBraceOpen = FALSE;
        }
        ccf++;
    }
    return eCategory;
}
static FX_WORD FX_WStringToLCID(FX_LPCWSTR pstrLCID)
{
    if (!pstrLCID) {
        return 0;
    }
    wchar_t* pEnd;
    return (FX_WORD)wcstol((wchar_t*)pstrLCID, &pEnd, 16);
}
FX_WORD	CFX_FormatString::GetLCID(const CFX_WideString& wsPattern)
{
    return FX_WStringToLCID(GetLocaleName(wsPattern));
}
CFX_WideString CFX_FormatString::GetLocaleName(const CFX_WideString& wsPattern)
{
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsPattern.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsPattern;
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_GetLiteralText(pStr, ccf, iLenf);
        } else if (pStr[ccf] == '(') {
            ccf++;
            CFX_WideString wsLCID;
            while (ccf < iLenf && pStr[ccf] != ')') {
                wsLCID += pStr[ccf++];
            }
            return wsLCID;
        }
        ccf++;
    }
    return CFX_WideString();
}
IFX_Locale* CFX_FormatString::GetTextFormat(const CFX_WideString &wsPattern, FX_WSTR wsCategory, CFX_WideString& wsPurgePattern)
{
    IFX_Locale* pLocale = NULL;
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsPattern.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsPattern;
    FX_BOOL bBrackOpen = FALSE;
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_INT32 iCurChar = ccf;
            FX_GetLiteralText(pStr, ccf, iLenf);
            wsPurgePattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
        } else if (!bBrackOpen && FX_Local_Find(gs_wsConstChars, pStr[ccf]) < 0) {
            CFX_WideString wsSearchCategory(pStr[ccf]);
            ccf++;
            while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' && pStr[ccf] != '(') {
                wsSearchCategory += pStr[ccf];
                ccf++;
            }
            if (wsSearchCategory != wsCategory) {
                continue;
            }
            while (ccf < iLenf) {
                if (pStr[ccf] == '(') {
                    ccf++;
                    CFX_WideString wsLCID;
                    while (ccf < iLenf && pStr[ccf] != ')')	{
                        wsLCID += pStr[ccf++];
                    }
                    pLocale = GetPatternLocale(wsLCID);
                } else if (pStr[ccf] == '{') {
                    bBrackOpen = TRUE;
                    break;
                }
                ccf++;
            }
        } else if (pStr[ccf] != '}') {
            wsPurgePattern += pStr[ccf];
        }
        ccf++;
    }
    if (!bBrackOpen) {
        wsPurgePattern = wsPattern;
    }
    if (!pLocale) {
        pLocale = m_pLocaleMgr->GetDefLocale();
    }
    return pLocale;
}
#define FX_NUMSTYLE_Percent		0x01
#define FX_NUMSTYLE_Exponent	0x02
#define FX_NUMSTYLE_DotVorv		0x04
IFX_Locale* CFX_FormatString::GetNumericFormat(const CFX_WideString& wsPattern, FX_INT32& iDotIndex, FX_DWORD& dwStyle, CFX_WideString& wsPurgePattern)
{
    dwStyle = 0;
    IFX_Locale* pLocale = NULL;
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsPattern.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsPattern;
    FX_BOOL bFindDot = FALSE;
    FX_BOOL bBrackOpen = FALSE;
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_INT32 iCurChar = ccf;
            FX_GetLiteralText(pStr, ccf, iLenf);
            wsPurgePattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
        } else if (!bBrackOpen && FX_Local_Find(gs_wsConstChars, pStr[ccf]) < 0) {
            CFX_WideString wsCategory(pStr[ccf]);
            ccf++;
            while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' && pStr[ccf] != '(') {
                wsCategory += pStr[ccf];
                ccf++;
            }
            if (wsCategory != FX_WSTRC(L"num")) {
                bBrackOpen = TRUE;
                ccf = 0;
                continue;
            }
            while (ccf < iLenf) {
                if (pStr[ccf] == '(') {
                    ccf++;
                    CFX_WideString wsLCID;
                    while (ccf < iLenf && pStr[ccf] != ')')	{
                        wsLCID += pStr[ccf++];
                    }
                    pLocale = GetPatternLocale(wsLCID);
                } else if (pStr[ccf] == '{') {
                    bBrackOpen = TRUE;
                    break;
                } else if (pStr[ccf] == '.') {
                    CFX_WideString wsSubCategory;
                    ccf++;
                    while (ccf < iLenf && pStr[ccf] != '(' && pStr[ccf] != '{')	{
                        wsSubCategory += pStr[ccf++];
                    }
                    FX_DWORD dwSubHash = FX_HashCode_String_GetW(wsSubCategory, wsSubCategory.GetLength());
                    FX_LOCALENUMSUBCATEGORY eSubCategory = FX_LOCALENUMPATTERN_Decimal;
                    for (FX_INT32 i = 0; i < g_iFXLocaleNumSubCatCount; i++) {
                        if (g_FXLocaleNumSubCatData[i].uHash == dwSubHash) {
                            eSubCategory = (FX_LOCALENUMSUBCATEGORY)g_FXLocaleNumSubCatData[i].eSubCategory;
                            break;
                        }
                    }
                    wsSubCategory.Empty();
                    if (!pLocale) {
                        pLocale = m_pLocaleMgr->GetDefLocale();
                    }
                    FXSYS_assert(pLocale != NULL);
                    pLocale->GetNumPattern(eSubCategory, wsSubCategory);
                    iDotIndex = wsSubCategory.Find('.');
                    if (iDotIndex > 0) {
                        iDotIndex += wsPurgePattern.GetLength() ;
                        bFindDot = TRUE;
                        dwStyle |= FX_NUMSTYLE_DotVorv;
                    }
                    wsPurgePattern += wsSubCategory;
                    if (eSubCategory == FX_LOCALENUMPATTERN_Percent) {
                        dwStyle |= FX_NUMSTYLE_Percent;
                    }
                    continue;
                }
                ccf++;
            }
        } else if (pStr[ccf] == 'E') {
            dwStyle |= FX_NUMSTYLE_Exponent;
            wsPurgePattern += pStr[ccf];
        } else if (pStr[ccf] == '%') {
            dwStyle |= FX_NUMSTYLE_Percent;
            wsPurgePattern += pStr[ccf];
        } else if (pStr[ccf] != '}') {
            wsPurgePattern += pStr[ccf];
        }
        if (!bFindDot) {
            if (pStr[ccf] == '.' || pStr[ccf] == 'V' || pStr[ccf] == 'v') {
                bFindDot = TRUE;
                iDotIndex = wsPurgePattern.GetLength() - 1;
                dwStyle |= FX_NUMSTYLE_DotVorv;
            }
        }
        ccf++;
    }
    if (!bFindDot) {
        iDotIndex = wsPurgePattern.GetLength();
    }
    if (!pLocale) {
        pLocale = m_pLocaleMgr->GetDefLocale();
    }
    return pLocale;
}
static FX_BOOL FX_GetNumericDotIndex(const CFX_WideString& wsNum, const CFX_WideString& wsDotSymbol, FX_INT32 &iDotIndex)
{
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsNum.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsNum;
    FX_INT32 iLenDot = wsDotSymbol.GetLength();
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_GetLiteralText(pStr, ccf, iLenf);
        } else if (ccf + iLenDot <= iLenf && !FXSYS_wcsncmp(pStr + ccf, (FX_LPCWSTR)wsDotSymbol, iLenDot)) {
            iDotIndex = ccf;
            return TRUE;
        }
        ccf++;
    }
    iDotIndex = wsNum.Find('.');
    if (iDotIndex < 0) {
        iDotIndex = iLenf;
        return FALSE;
    }
    return TRUE;
}
FX_BOOL	CFX_FormatString::ParseText(const CFX_WideString& wsSrcText, const CFX_WideString& wsPattern, CFX_WideString& wsValue)
{
    wsValue.Empty();
    if (wsSrcText.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"text"), wsTextFormat);
    if (wsTextFormat.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 iText = 0, iPattern = 0;
    FX_LPCWSTR pStrText = (FX_LPCWSTR)wsSrcText;
    FX_INT32 iLenText = wsSrcText.GetLength();
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern && iText < iLenText) {
        switch (pStrPattern[iPattern]) {
            case '\'': {
                    CFX_WideString wsLiteral = FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
                    FX_INT32 iLiteralLen = wsLiteral.GetLength();
                    if (iText + iLiteralLen > iLenText || FXSYS_wcsncmp(pStrText + iText, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                        wsValue = wsSrcText;
                        return FALSE;
                    }
                    iText += iLiteralLen;
                    iPattern++;
                    break;
                }
            case 'A':
                if (FX_IsAlpha(pStrText[iText])) {
                    wsValue += pStrText[iText];
                    iText++;
                }
                iPattern++;
                break;
            case 'X':
                wsValue += pStrText[iText];
                iText++;
                iPattern++;
                break;
            case 'O':
            case '0':
                if (FX_IsDigit(pStrText[iText]) || FX_IsAlpha(pStrText[iText])) {
                    wsValue += pStrText[iText];
                    iText++;
                }
                iPattern++;
                break;
            case '9':
                if (FX_IsDigit(pStrText[iText])) {
                    wsValue += pStrText[iText];
                    iText++;
                }
                iPattern++;
                break;
            default:
                if (pStrPattern[iPattern] != pStrText[iText]) {
                    wsValue = wsSrcText;
                    return FALSE;
                }
                iPattern++;
                iText++;
                break;
        }
    }
    return iPattern == iLenPattern && iText == iLenText;
}
FX_BOOL	CFX_FormatString::ParseNum(const CFX_WideString& wsSrcNum, const CFX_WideString& wsPattern, FX_FLOAT &fValue)
{
    fValue = 0.0f;
    if (wsSrcNum.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 dot_index_f = -1;
    FX_DWORD dwFormatStyle = 0;
    CFX_WideString wsNumFormat;
    IFX_Locale* pLocale = GetNumericFormat(wsPattern, dot_index_f, dwFormatStyle, wsNumFormat);
    if (!pLocale || wsNumFormat.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 iExponent = 0;
    CFX_WideString wsDotSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsDotSymbol);
    FX_INT32 iDotLen = wsDotSymbol.GetLength();
    CFX_WideString wsGroupSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping, wsGroupSymbol);
    FX_INT32 iGroupLen = wsGroupSymbol.GetLength();
    CFX_WideString wsMinus;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinus);
    FX_INT32 iMinusLen = wsMinus.GetLength();
    int cc = 0, ccf = 0;
    FX_LPCWSTR str = (FX_LPCWSTR)wsSrcNum;
    int len = wsSrcNum.GetLength();
    FX_LPCWSTR strf = (FX_LPCWSTR)wsNumFormat;
    int lenf = wsNumFormat.GetLength();
    double dbRetValue = 0;
    double coeff = 1;
    FX_BOOL bHavePercentSymbol = FALSE;
    FX_BOOL bNeg = FALSE;
    FX_BOOL bReverseParse = FALSE;
    FX_INT32 dot_index = 0;
    if (!FX_GetNumericDotIndex(wsSrcNum, wsDotSymbol, dot_index) && (dwFormatStyle & FX_NUMSTYLE_DotVorv)) {
        bReverseParse = TRUE;
    }
    bReverseParse = FALSE;
    if (bReverseParse) {
        ccf = lenf - 1;
        cc = len - 1;
        while (ccf > dot_index_f && cc >= 0) {
            switch (strf[ccf]) {
                case '\'': {
                        CFX_WideString wsLiteral = FX_GetLiteralTextReverse(strf, ccf);
                        FX_INT32 iLiteralLen = wsLiteral.GetLength();
                        cc -= iLiteralLen - 1;
                        if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                            return FALSE;
                        }
                        cc--;
                        ccf--;
                        break;
                    }
                case '9':
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    dbRetValue = dbRetValue * coeff + (str[cc] - '0') * 0.1;
                    coeff *= 0.1;
                    cc--;
                    ccf--;
                    break;
                case 'z':
                    if (cc >= 0) {
                        dbRetValue = dbRetValue * coeff + (str[cc] - '0') * 0.1;
                        coeff *= 0.1;
                        cc--;
                    }
                    ccf--;
                    break;
                case 'Z':
                    if (str[cc] != ' ') {
                        dbRetValue = dbRetValue * coeff + (str[cc] - '0') * 0.1;
                        coeff *= 0.1;
                    }
                    cc--;
                    ccf--;
                    break;
                case 'S':
                    if (str[cc] == '+' || str[cc] == ' ') {
                        cc--;
                    } else {
                        cc -= iMinusLen - 1;
                        if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        cc--;
                        bNeg = TRUE;
                    }
                    ccf--;
                    break;
                case 's':
                    if (str[cc] == '+') {
                        cc--;
                    } else {
                        cc -= iMinusLen - 1;
                        if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        cc--;
                        bNeg = TRUE;
                    }
                    ccf--;
                    break;
                case 'E': {
                        if (cc >= dot_index) {
                            return FALSE;
                        }
                        FX_BOOL bExpSign = FALSE;
                        while (cc >= 0) {
                            if (str[cc] == 'E' || str[cc] == 'e') {
                                break;
                            }
                            if (FX_IsDigit(str[cc])) {
                                iExponent = iExponent + (str[cc] - '0') * 10;
                                cc--;
                                continue;
                            } else if (str[cc] == '+') {
                                cc--;
                                continue;
                            } else if (cc - iMinusLen + 1 > 0 && !FXSYS_wcsncmp(str + (cc - iMinusLen + 1), (FX_LPCWSTR)wsMinus, iMinusLen)) {
                                bExpSign = TRUE;
                                cc -= iMinusLen;
                            } else {
                                return FALSE;
                            }
                        }
                        cc --;
                        iExponent = bExpSign ? -iExponent : iExponent;
                        ccf--;
                    }
                    break;
                case '$': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                        FX_INT32 iSymbolLen = wsSymbol.GetLength();
                        cc -= iSymbolLen - 1;
                        if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSymbolLen)) {
                            return FALSE;
                        }
                        cc--;
                        ccf--;
                    }
                    break;
                case 'r':
                    if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
                        if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                            bNeg = TRUE;
                            cc -= 2;
                        }
                        ccf -= 2;
                    } else {
                        ccf --;
                    }
                    break;
                case 'R':
                    if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                            bNeg = TRUE;
                            cc -= 2;
                        }
                        ccf -= 2;
                    } else {
                        ccf--;
                    }
                    break;
                case 'b':
                    if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
                        if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                            bNeg = TRUE;
                            cc -= 2;
                        }
                        ccf -= 2;
                    } else {
                        ccf --;
                    }
                    break;
                case 'B':
                    if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                            bNeg = TRUE;
                            cc -= 2;
                        }
                        ccf -= 2;
                    } else {
                        ccf--;
                    }
                    break;
                case '.':
                case 'V':
                case 'v':
                    return FALSE;
                case '%': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                        FX_INT32 iSysmbolLen = wsSymbol.GetLength();
                        cc -= iSysmbolLen - 1;
                        if (cc < 0  || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSysmbolLen)) {
                            return FALSE;
                        }
                        cc--;
                        ccf--;
                        bHavePercentSymbol = TRUE;
                    }
                    break;
                case '8':
                    while (ccf < lenf && strf[ccf] == '8') {
                        ccf++;
                    }
                    while (cc < len && FX_IsDigit(str[cc])) {
                        dbRetValue = (str[cc] - '0') * coeff + dbRetValue;
                        coeff *= 0.1;
                        cc++;
                    }
                    break;
                case ',': {
                        if (cc >= 0) {
                            cc -= iGroupLen - 1;
                            if (cc >= 0 && FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsGroupSymbol, iGroupLen) == 0) {
                                cc--;
                            } else {
                                cc += iGroupLen - 1;
                            }
                        }
                        ccf--;
                    }
                    break;
                case '(':
                    if (str[cc] == L'(') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    break;
                case ')':
                    if (str[cc] == L')') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    break;
                default:
                    if (strf[ccf] != str[cc]) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
            }
        }
        dot_index = cc + 1;
    }
    ccf = dot_index_f - 1;
    cc = dot_index - 1;
    coeff = 1;
    while (ccf >= 0 && cc >= 0) {
        switch (strf[ccf]) {
            case '\'': {
                    CFX_WideString wsLiteral = FX_GetLiteralTextReverse(strf, ccf);
                    FX_INT32 iLiteralLen = wsLiteral.GetLength();
                    cc -= iLiteralLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    break;
                }
            case '9':
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                coeff *= 10;
                cc--;
                ccf--;
                break;
            case 'z':
                if (FX_IsDigit(str[cc])) {
                    dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                    coeff *= 10;
                    cc--;
                }
                ccf--;
                break;
            case 'Z':
                if (str[cc] != ' ') {
                    if (FX_IsDigit(str[cc])) {
                        dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                        coeff *= 10;
                        cc--;
                    }
                } else {
                    cc--;
                }
                ccf--;
                break;
            case 'S':
                if (str[cc] == '+' || str[cc] == ' ') {
                    cc--;
                } else {
                    cc -= iMinusLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                        return FALSE;
                    }
                    cc--;
                    bNeg = TRUE;
                }
                ccf--;
                break;
            case 's':
                if (str[cc] == '+') {
                    cc--;
                } else {
                    cc -= iMinusLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                        return FALSE;
                    }
                    cc--;
                    bNeg = TRUE;
                }
                ccf--;
                break;
            case 'E': {
                    if (cc >= dot_index) {
                        return FALSE;
                    }
                    FX_BOOL bExpSign = FALSE;
                    while (cc >= 0) {
                        if (str[cc] == 'E' || str[cc] == 'e') {
                            break;
                        }
                        if (FX_IsDigit(str[cc])) {
                            iExponent = iExponent + (str[cc] - '0') * 10;
                            cc--;
                            continue;
                        } else if (str[cc] == '+') {
                            cc--;
                            continue;
                        } else if (cc - iMinusLen + 1 > 0 && !FXSYS_wcsncmp(str + (cc - iMinusLen + 1), (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            bExpSign = TRUE;
                            cc -= iMinusLen;
                        } else {
                            return FALSE;
                        }
                    }
                    cc --;
                    iExponent = bExpSign ? -iExponent : iExponent;
                    ccf--;
                }
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    FX_INT32 iSymbolLen = wsSymbol.GetLength();
                    cc -= iSymbolLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSymbolLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                }
                break;
            case 'r':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
                    if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf --;
                }
                break;
            case 'R':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
                    if (str[cc] == ' ') {
                        cc++;
                    } else if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf--;
                }
                break;
            case 'b':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
                    if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf --;
                }
                break;
            case 'B':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
                    if (str[cc] == ' ') {
                        cc++;
                    } else if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf--;
                }
                break;
            case '.':
            case 'V':
            case 'v':
                return FALSE;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    FX_INT32 iSysmbolLen = wsSymbol.GetLength();
                    cc -= iSysmbolLen - 1;
                    if (cc < 0  || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSysmbolLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    bHavePercentSymbol = TRUE;
                }
                break;
            case '8':
                return FALSE;
            case ',': {
                    if (cc >= 0) {
                        cc -= iGroupLen - 1;
                        if (cc >= 0 && FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsGroupSymbol, iGroupLen) == 0) {
                            cc--;
                        } else {
                            cc += iGroupLen - 1;
                        }
                    }
                    ccf--;
                }
                break;
            case '(':
                if (str[cc] == L'(') {
                    bNeg = TRUE;
                } else if (str[cc] != L' ') {
                    return FALSE;
                }
                cc--;
                ccf--;
                break;
            case ')':
                if (str[cc] == L')') {
                    bNeg = TRUE;
                } else if (str[cc] != L' ') {
                    return FALSE;
                }
                cc--;
                ccf--;
                break;
            default:
                if (strf[ccf] != str[cc]) {
                    return FALSE;
                }
                cc--;
                ccf--;
        }
    }
    if (cc >= 0) {
        return FALSE;
    }
    if (!bReverseParse) {
        ccf = dot_index_f + 1;
        cc = (dot_index == len) ? len : dot_index + 1;
        coeff = 0.1;
        while (cc < len && ccf < lenf) {
            switch (strf[ccf]) {
                case '\'': {
                        CFX_WideString wsLiteral = FX_GetLiteralText(strf, ccf, lenf);
                        FX_INT32 iLiteralLen = wsLiteral.GetLength();
                        if (cc + iLiteralLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                            return FALSE;
                        }
                        cc += iLiteralLen;
                        ccf++;
                        break;
                    }
                case '9':
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    {
                        dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                        coeff *= 0.1;
                    }
                    cc++;
                    ccf++;
                    break;
                case 'z':
                    if (FX_IsDigit(str[cc])) {
                        dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                        coeff *= 0.1;
                        cc++;
                    }
                    ccf++;
                    break;
                case 'Z':
                    if (str[cc] != ' ') {
                        if (FX_IsDigit(str[cc])) {
                            dbRetValue = dbRetValue + (str[cc] - '0') * coeff;
                            coeff *= 0.1;
                            cc++;
                        }
                    } else {
                        cc++;
                    }
                    ccf++;
                    break;
                case 'S':
                    if (str[cc] == '+' || str[cc] == ' ') {
                        cc++;
                    } else {
                        if (cc + iMinusLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        bNeg = TRUE;
                        cc += iMinusLen;
                    }
                    ccf++;
                    break;
                case 's':
                    if (str[cc] == '+') {
                        cc++;
                    } else {
                        if (cc + iMinusLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        bNeg = TRUE;
                        cc += iMinusLen;
                    }
                    ccf++;
                    break;
                case 'E': {
                        if (cc >= len || (str[cc] != 'E' && str[cc] != 'e')) {
                            return FALSE;
                        }
                        FX_BOOL bExpSign = FALSE;
                        cc ++;
                        if (cc < len) {
                            if (str[cc] == '+') {
                                cc++;
                            } else if (str[cc] == '-') {
                                bExpSign = TRUE;
                                cc++;
                            }
                        }
                        while (cc < len) {
                            if (!FX_IsDigit(str[cc])) {
                                break;
                            }
                            iExponent = iExponent * 10 + str[cc] - '0';
                            cc ++;
                        }
                        iExponent = bExpSign ? -iExponent : iExponent;
                        ccf++;
                    }
                    break;
                case '$': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                        FX_INT32 iSymbolLen = wsSymbol.GetLength();
                        if (cc + iSymbolLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSymbolLen)) {
                            return FALSE;
                        }
                        cc += iSymbolLen;
                        ccf++;
                    }
                    break;
                case 'c':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
                        if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'C':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'd':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
                        if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'D':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case '.':
                case 'V':
                case 'v':
                    return FALSE;
                case '%': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                        FX_INT32 iSysmbolLen = wsSymbol.GetLength();
                        if (cc + iSysmbolLen <= len && !FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSysmbolLen)) {
                            cc += iSysmbolLen;
                        }
                        ccf++;
                        bHavePercentSymbol = TRUE;
                    }
                    break;
                case '8': {
                        while (ccf < lenf && strf[ccf] == '8') {
                            ccf++;
                        }
                        while (cc < len && FX_IsDigit(str[cc])) {
                            dbRetValue = (str[cc] - '0') * coeff + dbRetValue;
                            coeff *= 0.1;
                            cc++;
                        }
                    }
                    break;
                case ',': {
                        if (cc + iGroupLen <= len && FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsGroupSymbol, iGroupLen) == 0) {
                            cc += iGroupLen;
                        }
                        ccf++;
                    }
                    break;
                case '(':
                    if (str[cc] == L'(') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
                    break;
                case ')':
                    if (str[cc] == L')') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
                    break;
                default:
                    if (strf[ccf] != str[cc]) {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
            }
        }
        if (cc != len) {
            return FALSE;
        }
    }
    if (iExponent) {
        dbRetValue *= FXSYS_pow(10, (FX_FLOAT)iExponent);
    }
    if (bHavePercentSymbol) {
        dbRetValue /= 100.0;
    }
    if (bNeg) {
        dbRetValue = -dbRetValue;
    }
    fValue = (FX_FLOAT)dbRetValue;
    return TRUE;
}
void  FX_ParseNumString(const CFX_WideString& wsNum, CFX_WideString& wsResult)
{
    FX_INT32 iCount = wsNum.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsNum;
    FX_LPWSTR pDst = wsResult.GetBuffer(iCount);
    FX_INT32 nIndex = 0;
    FX_BOOL bMinus = FALSE;
    FX_INT32 i = 0;
    for (i = 0; i < iCount; i++) {
        FX_WCHAR wc = pStr[i];
        if (wc == '.') {
            break;
        }
        if ((wc == L'0' || wc == L' ' || wc == '+') && nIndex == 0) {
            continue;
        }
        if (wc == '-') {
            pDst[nIndex++] = wc;
            bMinus = TRUE;
            continue;
        }
        if (wc == L'0' && nIndex == 1 && bMinus) {
            continue;
        }
        pDst[nIndex++] = wc;
    }
    if (bMinus && nIndex == 1) {
        pDst[nIndex++] = '0';
    }
    if (nIndex == 0) {
        wsResult.ReleaseBuffer(0);
        pDst = wsResult.GetBuffer(iCount + 1);
        pDst[nIndex++] = '0';
    }
    FX_INT32 j = 0;
    for (j = iCount - 1; j > i; j--) {
        FX_WCHAR wc = pStr[j];
        if (wc != L'0' && wc != L' ') {
            break;
        }
    }
    if (j > i) {
        pDst[nIndex++] = '.';
        FXSYS_wcsncpy(pDst + nIndex, pStr + i + 1, j - i);
        nIndex += j - i;
    }
    wsResult.ReleaseBuffer(nIndex);
}
FX_BOOL	CFX_FormatString::ParseNum(const CFX_WideString& wsSrcNum, const CFX_WideString& wsPattern, CFX_WideString &wsValue)
{
    wsValue.Empty();
    if (wsSrcNum.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 dot_index_f = -1;
    FX_DWORD dwFormatStyle = 0;
    CFX_WideString wsNumFormat;
    IFX_Locale* pLocale = GetNumericFormat(wsPattern, dot_index_f, dwFormatStyle, wsNumFormat);
    if (!pLocale || wsNumFormat.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 iExponent = 0;
    CFX_WideString wsDotSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsDotSymbol);
    FX_INT32 iDotLen = wsDotSymbol.GetLength();
    CFX_WideString wsGroupSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping, wsGroupSymbol);
    FX_INT32 iGroupLen = wsGroupSymbol.GetLength();
    CFX_WideString wsMinus;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinus);
    FX_INT32 iMinusLen = wsMinus.GetLength();
    int cc = 0, ccf = 0;
    FX_LPCWSTR str = (FX_LPCWSTR)wsSrcNum;
    int len = wsSrcNum.GetLength();
    FX_LPCWSTR strf = (FX_LPCWSTR)wsNumFormat;
    int lenf = wsNumFormat.GetLength();
    FX_BOOL bHavePercentSymbol = FALSE;
    FX_BOOL bNeg = FALSE;
    FX_BOOL bReverseParse = FALSE;
    FX_INT32 dot_index = 0;
    if (!FX_GetNumericDotIndex(wsSrcNum, wsDotSymbol, dot_index) && (dwFormatStyle & FX_NUMSTYLE_DotVorv)) {
        bReverseParse = TRUE;
    }
    bReverseParse = FALSE;
    ccf = dot_index_f - 1;
    cc = dot_index - 1;
    while (ccf >= 0 && cc >= 0) {
        switch (strf[ccf]) {
            case '\'': {
                    CFX_WideString wsLiteral = FX_GetLiteralTextReverse(strf, ccf);
                    FX_INT32 iLiteralLen = wsLiteral.GetLength();
                    cc -= iLiteralLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    break;
                }
            case '9':
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                wsValue = CFX_WideStringC(str[cc]) + wsValue;
                cc--;
                ccf--;
                break;
            case 'z':
                if (FX_IsDigit(str[cc])) {
                    wsValue = CFX_WideStringC(str[cc]) + wsValue;
                    cc--;
                }
                ccf--;
                break;
            case 'Z':
                if (str[cc] != ' ') {
                    if (FX_IsDigit(str[cc])) {
                        wsValue = CFX_WideStringC(str[cc]) + wsValue;
                        cc--;
                    }
                } else {
                    cc--;
                }
                ccf--;
                break;
            case 'S':
                if (str[cc] == '+' || str[cc] == ' ') {
                    cc--;
                } else {
                    cc -= iMinusLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                        return FALSE;
                    }
                    cc--;
                    bNeg = TRUE;
                }
                ccf--;
                break;
            case 's':
                if (str[cc] == '+') {
                    cc--;
                } else {
                    cc -= iMinusLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                        return FALSE;
                    }
                    cc--;
                    bNeg = TRUE;
                }
                ccf--;
                break;
            case 'E': {
                    if (cc >= dot_index) {
                        return FALSE;
                    }
                    FX_BOOL bExpSign = FALSE;
                    while (cc >= 0) {
                        if (str[cc] == 'E' || str[cc] == 'e') {
                            break;
                        }
                        if (FX_IsDigit(str[cc])) {
                            iExponent = iExponent + (str[cc] - '0') * 10;
                            cc--;
                            continue;
                        } else if (str[cc] == '+') {
                            cc--;
                            continue;
                        } else if (cc - iMinusLen + 1 > 0 && !FXSYS_wcsncmp(str + (cc - iMinusLen + 1), (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            bExpSign = TRUE;
                            cc -= iMinusLen;
                        } else {
                            return FALSE;
                        }
                    }
                    cc --;
                    iExponent = bExpSign ? -iExponent : iExponent;
                    ccf--;
                }
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    FX_INT32 iSymbolLen = wsSymbol.GetLength();
                    cc -= iSymbolLen - 1;
                    if (cc < 0 || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSymbolLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                }
                break;
            case 'r':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
                    if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf --;
                }
                break;
            case 'R':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
                    if (str[cc] == ' ') {
                        cc++;
                    } else if (str[cc] == 'R' && cc - 1 >= 0 && str[cc - 1] == 'C') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf--;
                }
                break;
            case 'b':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
                    if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf --;
                }
                break;
            case 'B':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
                    if (str[cc] == ' ') {
                        cc++;
                    } else if (str[cc] == 'B' && cc - 1 >= 0 && str[cc - 1] == 'D') {
                        bNeg = TRUE;
                        cc -= 2;
                    }
                    ccf -= 2;
                } else {
                    ccf--;
                }
                break;
            case '.':
            case 'V':
            case 'v':
                return FALSE;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    FX_INT32 iSysmbolLen = wsSymbol.GetLength();
                    cc -= iSysmbolLen - 1;
                    if (cc < 0  || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSysmbolLen)) {
                        return FALSE;
                    }
                    cc--;
                    ccf--;
                    bHavePercentSymbol = TRUE;
                }
                break;
            case '8':
                return FALSE;
            case ',': {
                    if (cc >= 0) {
                        cc -= iGroupLen - 1;
                        if (cc >= 0 && FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsGroupSymbol, iGroupLen) == 0) {
                            cc--;
                        } else {
                            cc += iGroupLen - 1;
                        }
                    }
                    ccf--;
                }
                break;
            case '(':
                if (str[cc] == L'(') {
                    bNeg = TRUE;
                } else if (str[cc] != L' ') {
                    return FALSE;
                }
                cc--;
                ccf--;
                break;
            case ')':
                if (str[cc] == L')') {
                    bNeg = TRUE;
                } else if (str[cc] != L' ') {
                    return FALSE;
                }
                cc--;
                ccf--;
                break;
            default:
                if (strf[ccf] != str[cc]) {
                    return FALSE;
                }
                cc--;
                ccf--;
        }
    }
    if (cc >= 0) {
        if (str[cc] == '-') {
            bNeg = TRUE;
            cc --;
        }
        if (cc >= 0) {
            return FALSE;
        }
    }
    if (dot_index < len && (dwFormatStyle & FX_NUMSTYLE_DotVorv)) {
        wsValue += '.';
    }
    if (!bReverseParse) {
        ccf = dot_index_f + 1;
        cc = (dot_index == len) ? len : dot_index + 1;
        while (cc < len && ccf < lenf) {
            switch (strf[ccf]) {
                case '\'': {
                        CFX_WideString wsLiteral = FX_GetLiteralText(strf, ccf, lenf);
                        FX_INT32 iLiteralLen = wsLiteral.GetLength();
                        if (cc + iLiteralLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                            return FALSE;
                        }
                        cc += iLiteralLen;
                        ccf++;
                        break;
                    }
                case '9':
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    {
                        wsValue += str[cc];
                    }
                    cc++;
                    ccf++;
                    break;
                case 'z':
                    if (FX_IsDigit(str[cc])) {
                        wsValue += str[cc];
                        cc++;
                    }
                    ccf++;
                    break;
                case 'Z':
                    if (str[cc] != ' ') {
                        if (FX_IsDigit(str[cc])) {
                            wsValue += str[cc];
                            cc++;
                        }
                    } else {
                        cc++;
                    }
                    ccf++;
                    break;
                case 'S':
                    if (str[cc] == '+' || str[cc] == ' ') {
                        cc++;
                    } else {
                        if (cc + iMinusLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        bNeg = TRUE;
                        cc += iMinusLen;
                    }
                    ccf++;
                    break;
                case 's':
                    if (str[cc] == '+') {
                        cc++;
                    } else {
                        if (cc + iMinusLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsMinus, iMinusLen)) {
                            return FALSE;
                        }
                        bNeg = TRUE;
                        cc += iMinusLen;
                    }
                    ccf++;
                    break;
                case 'E': {
                        if (cc >= len || (str[cc] != 'E' && str[cc] != 'e')) {
                            return FALSE;
                        }
                        FX_BOOL bExpSign = FALSE;
                        cc ++;
                        if (cc < len) {
                            if (str[cc] == '+') {
                                cc++;
                            } else if (str[cc] == '-') {
                                bExpSign = TRUE;
                                cc++;
                            }
                        }
                        while (cc < len) {
                            if (!FX_IsDigit(str[cc])) {
                                break;
                            }
                            iExponent = iExponent * 10 + str[cc] - '0';
                            cc ++;
                        }
                        iExponent = bExpSign ? -iExponent : iExponent;
                        ccf++;
                    }
                    break;
                case '$': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                        FX_INT32 iSymbolLen = wsSymbol.GetLength();
                        if (cc + iSymbolLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSymbolLen)) {
                            return FALSE;
                        }
                        cc += iSymbolLen;
                        ccf++;
                    }
                    break;
                case 'c':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
                        if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'C':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'C' && cc + 1 < len && str[cc + 1] == 'R') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'd':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
                        if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case 'D':
                    if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
                        if (str[cc] == ' ') {
                            cc++;
                        } else if (str[cc] == 'D' && cc + 1 < len && str[cc + 1] == 'B') {
                            bNeg = TRUE;
                            cc += 2;
                        }
                        ccf += 2;
                    }
                    break;
                case '.':
                case 'V':
                case 'v':
                    return FALSE;
                case '%': {
                        CFX_WideString wsSymbol;
                        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                        FX_INT32 iSysmbolLen = wsSymbol.GetLength();
                        if (cc + iSysmbolLen <= len && !FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsSymbol, iSysmbolLen)) {
                            cc += iSysmbolLen;
                        }
                        ccf++;
                        bHavePercentSymbol = TRUE;
                    }
                    break;
                case '8': {
                        while (ccf < lenf && strf[ccf] == '8') {
                            ccf++;
                        }
                        while (cc < len && FX_IsDigit(str[cc])) {
                            wsValue += str[cc];
                            cc++;
                        }
                    }
                    break;
                case ',': {
                        if (cc + iGroupLen <= len && FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsGroupSymbol, iGroupLen) == 0) {
                            cc += iGroupLen;
                        }
                        ccf++;
                    }
                    break;
                case '(':
                    if (str[cc] == L'(') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
                    break;
                case ')':
                    if (str[cc] == L')') {
                        bNeg = TRUE;
                    } else if (str[cc] != L' ') {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
                    break;
                default:
                    if (strf[ccf] != str[cc]) {
                        return FALSE;
                    }
                    cc++;
                    ccf++;
            }
        }
        if (cc != len) {
            return FALSE;
        }
    }
    if (iExponent || bHavePercentSymbol) {
        CFX_Decimal decimal = CFX_Decimal(wsValue);
        if (iExponent) {
            decimal = decimal * CFX_Decimal(FXSYS_pow(10, (FX_FLOAT)iExponent));
        }
        if (bHavePercentSymbol) {
            decimal = decimal / CFX_Decimal(100);
        }
        wsValue = decimal;
    }
    if (bNeg) {
        wsValue = CFX_WideStringC('-') + wsValue;
    }
    return TRUE;
}
FX_DATETIMETYPE CFX_FormatString::GetDateTimeFormat(const CFX_WideString& wsPattern, IFX_Locale*& pLocale, CFX_WideString &wsDatePattern, CFX_WideString &wsTimePattern)
{
    pLocale = NULL;
    CFX_WideString wsTempPattern;
    FX_LOCALECATEGORY eCategory = FX_LOCALECATEGORY_Unknown;
    FX_INT32 ccf = 0;
    FX_INT32 iLenf = wsPattern.GetLength();
    FX_LPCWSTR pStr = (FX_LPCWSTR)wsPattern;
    FX_INT32 iFindCategory = 0;
    FX_BOOL bBraceOpen = FALSE;
    while (ccf < iLenf) {
        if (pStr[ccf] == '\'') {
            FX_INT32 iCurChar = ccf;
            FX_GetLiteralText(pStr, ccf, iLenf);
            wsTempPattern += CFX_WideStringC(pStr + iCurChar, ccf - iCurChar + 1);
        } else if (!bBraceOpen && iFindCategory != 3 && FX_Local_Find(gs_wsConstChars, pStr[ccf]) < 0) {
            CFX_WideString wsCategory(pStr[ccf]);
            ccf++;
            while (ccf < iLenf && pStr[ccf] != '{' && pStr[ccf] != '.' && pStr[ccf] != '(') {
                if (pStr[ccf] == 'T') {
                    wsDatePattern = wsPattern.Left(ccf);
                    wsTimePattern = wsPattern.Right(wsPattern.GetLength() - ccf);
                    wsTimePattern.SetAt(0, ' ');
                    if (!pLocale) {
                        pLocale = m_pLocaleMgr->GetDefLocale();
                    }
                    return FX_DATETIMETYPE_DateTime;
                }
                wsCategory += pStr[ccf];
                ccf++;
            }
            if (!(iFindCategory & 1) && wsCategory == FX_WSTRC(L"date")) {
                iFindCategory |= 1;
                eCategory = FX_LOCALECATEGORY_Date;
                if (iFindCategory & 2) {
                    iFindCategory = 4;
                }
            } else if (!(iFindCategory & 2) && wsCategory == FX_WSTRC(L"time")) {
                iFindCategory |= 2;
                eCategory = FX_LOCALECATEGORY_Time;
            } else if (wsCategory == FX_WSTRC(L"datetime")) {
                iFindCategory = 3;
                eCategory = FX_LOCALECATEGORY_DateTime;
            } else {
                continue;
            }
            while (ccf < iLenf) {
                if (pStr[ccf] == '(') {
                    ccf++;
                    CFX_WideString wsLCID;
                    while (ccf < iLenf && pStr[ccf] != ')')	{
                        wsLCID += pStr[ccf++];
                    }
                    pLocale = GetPatternLocale(wsLCID);
                } else if (pStr[ccf] == '{') {
                    bBraceOpen = TRUE;
                    break;
                } else if (pStr[ccf] == '.') {
                    CFX_WideString wsSubCategory;
                    ccf++;
                    while (ccf < iLenf && pStr[ccf] != '(' && pStr[ccf] != '{')	{
                        wsSubCategory += pStr[ccf++];
                    }
                    FX_DWORD dwSubHash = FX_HashCode_String_GetW(wsSubCategory, wsSubCategory.GetLength());
                    FX_LOCALEDATETIMESUBCATEGORY eSubCategory = FX_LOCALEDATETIMESUBCATEGORY_Medium;
                    for (FX_INT32 i = 0; i < g_iFXLocaleDateTimeSubCatCount; i++) {
                        if (g_FXLocaleDateTimeSubCatData[i].uHash == dwSubHash) {
                            eSubCategory = (FX_LOCALEDATETIMESUBCATEGORY)g_FXLocaleDateTimeSubCatData[i].eSubCategory;
                            break;
                        }
                    }
                    if (!pLocale) {
                        pLocale = m_pLocaleMgr->GetDefLocale();
                    }
                    FXSYS_assert(pLocale != NULL);
                    switch (eCategory) {
                        case FX_LOCALECATEGORY_Date:
                            pLocale->GetDatePattern(eSubCategory, wsDatePattern);
                            wsDatePattern = wsTempPattern + wsDatePattern;
                            break;
                        case FX_LOCALECATEGORY_Time:
                            pLocale->GetTimePattern(eSubCategory, wsTimePattern);
                            wsTimePattern = wsTempPattern + wsTimePattern;
                            break;
                        case FX_LOCALECATEGORY_DateTime:
                            pLocale->GetDatePattern(eSubCategory, wsDatePattern);
                            wsDatePattern = wsTempPattern + wsDatePattern;
                            pLocale->GetTimePattern(eSubCategory, wsTimePattern);
                            break;
                        default:
                            break;
                    }
                    wsTempPattern.Empty();
                    continue;
                }
                ccf++;
            }
        } else if (pStr[ccf] == '}') {
            bBraceOpen = FALSE;
            if (!wsTempPattern.IsEmpty()) {
                if (eCategory == FX_LOCALECATEGORY_Time) {
                    wsTimePattern = wsTempPattern;
                } else if (eCategory == FX_LOCALECATEGORY_Date) {
                    wsDatePattern = wsTempPattern;
                }
                wsTempPattern.Empty();
            }
        } else {
            wsTempPattern += pStr[ccf];
        }
        ccf++;
    }
    if (!wsTempPattern.IsEmpty()) {
        if (eCategory == FX_LOCALECATEGORY_Date) {
            wsDatePattern += wsTempPattern;
        } else {
            wsTimePattern += wsTempPattern;
        }
    }
    if (!pLocale) {
        pLocale = m_pLocaleMgr->GetDefLocale();
    }
    if (!iFindCategory) {
        wsTimePattern.Empty();
        wsDatePattern = wsPattern;
    }
    return (FX_DATETIMETYPE)iFindCategory;
}
static FX_BOOL FX_ParseLocaleDate(const CFX_WideString& wsDate, const CFX_WideString& wsDatePattern, IFX_Locale* pLocale, CFX_Unitime &datetime, FX_INT32 &cc)
{
    FX_INT32 year = 1900;
    FX_INT32 month = 1;
    FX_INT32 day = 1;
    FX_INT32 ccf = 0;
    FX_LPCWSTR str = (FX_LPCWSTR)wsDate;
    FX_INT32 len = wsDate.GetLength();
    FX_LPCWSTR strf = (FX_LPCWSTR)wsDatePattern;
    FX_INT32 lenf = wsDatePattern.GetLength();
    while (cc < len && ccf < lenf) {
        if (strf[ccf] == '\'') {
            CFX_WideString wsLiteral = FX_GetLiteralText(strf, ccf, lenf);
            FX_INT32 iLiteralLen = wsLiteral.GetLength();
            if (cc + iLiteralLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                return FALSE;
            }
            cc += iLiteralLen;
            ccf++;
            continue;
        } else if (FX_Local_Find(gs_wsDateSymbols, strf[ccf]) < 0) {
            if (strf[ccf] != str[cc]) {
                return FALSE;
            }
            cc++;
            ccf++;
            continue;
        }
        FX_DWORD dwSymbolNum = 1;
        FX_DWORD dwSymbol = strf[ccf++];
        while (ccf < lenf && strf[ccf] == dwSymbol) {
            ccf++;
            dwSymbolNum++;
        }
        dwSymbol = (dwSymbol << 8) | (dwSymbolNum + '0');
        if (dwSymbol == FXBSTR_ID(0, 0, 'D', '1')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            day = str[cc++] - '0';
            if (cc < len && FX_IsDigit(str[cc])) {
                day = day * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'D', '2')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            day = str[cc++] - '0';
            if (cc < len) {
                day = day * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '1')) {
            int i = 0;
            while (cc < len && i < 3 && FX_IsDigit(str[cc])) {
                cc++;
                i++;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '3')) {
            cc += 3;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            month = str[cc++] - '0';
            if (cc < len && FX_IsDigit(str[cc])) {
                month = month * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            month = str[cc++] - '0';
            if (cc < len) {
                month = month * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '3')) {
            CFX_WideString wsMonthNameAbbr;
            FX_WORD i = 0;
            for (; i < 12; i++) {
                pLocale->GetMonthName(i, wsMonthNameAbbr, TRUE);
                if (wsMonthNameAbbr.IsEmpty()) {
                    continue;
                }
                if (!FXSYS_wcsncmp((FX_LPCWSTR)wsMonthNameAbbr, str + cc, wsMonthNameAbbr.GetLength())) {
                    break;
                }
            }
            if (i < 12) {
                cc += wsMonthNameAbbr.GetLength();
                month = i + 1;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '4')) {
            CFX_WideString wsMonthName;
            FX_WORD i = 0;
            for (; i < 12; i++) {
                pLocale->GetMonthName(i, wsMonthName, FALSE);
                if (wsMonthName.IsEmpty()) {
                    continue;
                }
                if (!FXSYS_wcsncmp((FX_LPCWSTR)wsMonthName, str + cc, wsMonthName.GetLength())) {
                    break;
                }
            }
            if (i < 12) {
                cc += wsMonthName.GetLength();
                month = i + 1;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '1')) {
            cc += 1;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '3')) {
            CFX_WideString wsDayNameAbbr;
            FX_WORD i = 0;
            for (; i < 7; i++) {
                pLocale->GetDayName(i, wsDayNameAbbr, TRUE);
                if (wsDayNameAbbr.IsEmpty()) {
                    continue;
                }
                if (!FXSYS_wcsncmp((FX_LPCWSTR)wsDayNameAbbr, str + cc, wsDayNameAbbr.GetLength())) {
                    break;
                }
            }
            if (i < 12) {
                cc += wsDayNameAbbr.GetLength();
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '4')) {
            CFX_WideString wsDayName;
            FX_INT32 i = 0;
            for (; i < 7; i++) {
                pLocale->GetDayName(i, wsDayName, FALSE);
                if (wsDayName == L"") {
                    continue;
                }
                if (!FXSYS_wcsncmp((FX_LPCWSTR)wsDayName, str + cc, wsDayName.GetLength())) {
                    break;
                }
            }
            if (i < 12) {
                cc += wsDayName.GetLength();
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'e', '1')) {
            cc += 1;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'G', '1')) {
            cc += 2;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '2')) {
            if (cc + 2 > len) {
                return FALSE;
            }
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            year = str[cc++] - '0';
            if (cc >= len || !FX_IsDigit(str[cc])) {
                return FALSE;
            }
            year = year * 10 + str[cc++] - '0';
            if (year <= 29) {
                year += 2000;
            } else {
                year += 1900;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '4')) {
            int i = 0;
            year = 0;
            if (cc + 4 > len) {
                return FALSE;
            }
            while (i < 4) {
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                year = year * 10 + str[cc] - '0';
                cc++;
                i++;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'w', '1')) {
            cc += 1;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'W', '2')) {
            cc += 2;
        }
    }
    CFX_Unitime ut;
    ut.Set(year, month, day);
    datetime = datetime + ut;
    return cc;
}
static void FX_ResolveZone(FX_BYTE& wHour, FX_BYTE& wMinute, FX_TIMEZONE tzDiff, IFX_Locale* pLocale)
{
    FX_INT32 iMinuteDiff = wHour * 60 + wMinute;
    FX_TIMEZONE tzLocale;
    pLocale->GetTimeZone(tzLocale);
    iMinuteDiff += tzLocale.tzHour * 60 + (tzLocale.tzHour < 0 ? -tzLocale.tzMinute : tzLocale.tzMinute);
    iMinuteDiff -= tzDiff.tzHour * 60 + (tzDiff.tzHour < 0 ? -tzDiff.tzMinute : tzDiff.tzMinute);
    while (iMinuteDiff > 1440) {
        iMinuteDiff -= 1440;
    }
    while (iMinuteDiff < 0) {
        iMinuteDiff += 1440;
    }
    wHour = iMinuteDiff / 60;
    wMinute = iMinuteDiff % 60;
}
static FX_BOOL FX_ParseLocaleTime(const CFX_WideString& wsTime, const CFX_WideString& wsTimePattern, IFX_Locale* pLocale, CFX_Unitime &datetime, FX_INT32 &cc)
{
    FX_BYTE hour = 0;
    FX_BYTE minute = 0;
    FX_BYTE second = 0;
    FX_WORD millisecond = 0;
    FX_INT32 ccf = 0;
    FX_LPCWSTR str = (FX_LPCWSTR)wsTime;
    int len = wsTime.GetLength();
    FX_LPCWSTR strf = (FX_LPCWSTR)wsTimePattern;
    int lenf = wsTimePattern.GetLength();
    FX_BOOL bHasA = FALSE;
    FX_BOOL bPM = FALSE;
    while (cc < len && ccf < lenf) {
        if (strf[ccf] == '\'') {
            CFX_WideString wsLiteral = FX_GetLiteralText(strf, ccf, lenf);
            FX_INT32 iLiteralLen = wsLiteral.GetLength();
            if (cc + iLiteralLen > len || FXSYS_wcsncmp(str + cc, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                return FALSE;
            }
            cc += iLiteralLen;
            ccf++;
            continue;
        } else if (FX_Local_Find(gs_wsTimeSymbols, strf[ccf]) == -1) {
            if (strf[ccf] != str[cc]) {
                return FALSE;
            }
            cc++;
            ccf++;
            continue;
        }
        FX_DWORD dwSymbolNum = 1;
        FX_DWORD dwSymbol = strf[ccf++];
        while (ccf < lenf && strf[ccf] == dwSymbol) {
            ccf++;
            dwSymbolNum++;
        }
        dwSymbol = (dwSymbol << 8) | (dwSymbolNum + '0');
        if (dwSymbol == FXBSTR_ID(0, 0, 'k', '1') || dwSymbol == FXBSTR_ID(0, 0, 'H', '1') || dwSymbol == FXBSTR_ID(0, 0, 'h', '1') || dwSymbol == FXBSTR_ID(0, 0, 'K', '1')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            hour = str[cc++] - '0';
            if (cc < len && FX_IsDigit(str[cc])) {
                hour = hour * 10 + str[cc++] - '0';
            }
            if (dwSymbol == FXBSTR_ID(0, 0, 'K', '1') && hour == 24) {
                hour = 0;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '2') || dwSymbol == FXBSTR_ID(0, 0, 'H', '2') || dwSymbol == FXBSTR_ID(0, 0, 'h', '2') || dwSymbol == FXBSTR_ID(0, 0, 'K', '2')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            hour = str[cc++] - '0';
            if (cc >= len) {
                return FALSE;
            }
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            hour = hour * 10 + str[cc++] - '0';
            if (dwSymbol == FXBSTR_ID(0, 0, 'K', '2') && hour == 24) {
                hour = 0;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            minute = str[cc++] - '0';
            if (cc < len && FX_IsDigit(str[cc])) {
                minute = minute * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            minute = str[cc++] - '0';
            if (cc >= len) {
                return FALSE;
            }
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            minute = minute * 10 + str[cc++] - '0';
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '1')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            second = str[cc++] - '0';
            if (cc < len && FX_IsDigit(str[cc])) {
                second = second * 10 + str[cc++] - '0';
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '2')) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            second = str[cc++] - '0';
            if (cc >= len) {
                return FALSE;
            }
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            second = second * 10 + str[cc++] - '0';
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'F', '3')) {
            if (cc + 3 >= len) {
                return FALSE;
            }
            int i = 0;
            while (i < 3) {
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                millisecond = millisecond * 10 + str[cc++] - '0';
                i++;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'A', '1')) {
            CFX_WideString wsAM;
            pLocale->GetMeridiemName(wsAM, TRUE);
            CFX_WideString wsPM;
            pLocale->GetMeridiemName(wsPM, FALSE);
            if ((cc + wsAM.GetLength() <= len) && (CFX_WideStringC(str + cc, wsAM.GetLength()) == wsAM)) {
                cc += wsAM.GetLength();
                bHasA = TRUE;
            } else if ((cc + wsPM.GetLength() <= len) && (CFX_WideStringC(str + cc, wsPM.GetLength()) == wsPM)) {
                cc += wsPM.GetLength();
                bHasA = TRUE;
                bPM = TRUE;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Z', '1')) {
            if (cc + 3 > len) {
                continue;
            }
            FX_DWORD dwHash = str[cc++];
            dwHash = (dwHash << 8) | str[cc++];
            dwHash = (dwHash << 8) | str[cc++];
            if (dwHash == FXBSTR_ID(0, 'G', 'M', 'T')) {
                FX_TIMEZONE tzDiff;
                tzDiff.tzHour = 0;
                tzDiff.tzMinute = 0;
                if (cc < len && (str[cc] == '-' || str[cc] == '+')) {
                    cc += FX_ParseTimeZone(str + cc, len - cc, tzDiff);
                }
                FX_ResolveZone(hour, minute, tzDiff, pLocale);
            } else {
                FX_LPCLOCALETIMEZONEINFO pTimeZoneInfo = NULL;
                FX_INT32 iStart = 0, iEnd = g_iFXLocaleTimeZoneCount - 1;
                do {
                    FX_INT32 iMid = (iStart + iEnd) / 2;
                    FX_LPCLOCALETIMEZONEINFO pInfo = g_FXLocaleTimeZoneData + iMid;
                    if (dwHash == pInfo->uHash) {
                        pTimeZoneInfo = pInfo;
                        break;
                    } else if (dwHash < pInfo->uHash) {
                        iEnd = iMid - 1;
                    } else {
                        iStart = iMid + 1;
                    }
                } while (iStart <= iEnd);
                if (pTimeZoneInfo) {
                    hour += pTimeZoneInfo->iHour;
                    minute += pTimeZoneInfo->iHour > 0 ? pTimeZoneInfo->iMinute : -pTimeZoneInfo->iMinute;
                }
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'z', '1')) {
            if (str[cc] != 'Z') {
                FX_TIMEZONE tzDiff;
                cc += FX_ParseTimeZone(str + cc, len - cc, tzDiff);
                FX_ResolveZone(hour, minute, tzDiff, pLocale);
            } else {
                cc++;
            }
        }
    }
    if (bHasA) {
        if (bPM) {
            hour += 12;
            if (hour == 24) {
                hour = 12;
            }
        } else {
            if (hour == 12) {
                hour = 0;
            }
        }
    }
    CFX_Unitime ut;
    ut.Set(0, 0, 0, hour, minute, second, millisecond);
    datetime = datetime + ut;
    return cc;
}
FX_BOOL	CFX_FormatString::ParseDateTime(const CFX_WideString& wsSrcDateTime, const CFX_WideString& wsPattern, FX_DATETIMETYPE eDateTimeType, CFX_Unitime& dtValue)
{
    dtValue.Set(0);
    if (wsSrcDateTime.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsDatePattern, wsTimePattern;
    IFX_Locale* pLocale = NULL;
    FX_DATETIMETYPE eCategory = GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
    if (!pLocale) {
        return FALSE;
    }
    if (eCategory == FX_DATETIMETYPE_Unknown) {
        eCategory = eDateTimeType;
    }
    if (eCategory == FX_DATETIMETYPE_Unknown) {
        return FALSE;
    }
    if (eCategory == FX_DATETIMETYPE_TimeDate) {
        FX_INT32 iStart = 0;
        if (!FX_ParseLocaleTime(wsSrcDateTime, wsTimePattern, pLocale, dtValue, iStart)) {
            return FALSE;
        }
        if (!FX_ParseLocaleDate(wsSrcDateTime, wsDatePattern, pLocale, dtValue, iStart)) {
            return FALSE;
        }
    } else {
        FX_INT32 iStart = 0;
        if ((eCategory & FX_DATETIMETYPE_Date) && !FX_ParseLocaleDate(wsSrcDateTime, wsDatePattern, pLocale, dtValue, iStart)) {
            return FALSE;
        }
        if ((eCategory & FX_DATETIMETYPE_Time) && !FX_ParseLocaleTime(wsSrcDateTime, wsTimePattern, pLocale, dtValue, iStart)) {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BOOL CFX_FormatString::ParseZero(const CFX_WideString& wsSrcText, const CFX_WideString& wsPattern)
{
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"zero"), wsTextFormat);
    FX_INT32 iText = 0, iPattern = 0;
    FX_LPCWSTR pStrText = (FX_LPCWSTR)wsSrcText;
    FX_INT32 iLenText = wsSrcText.GetLength();
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern && iText < iLenText) {
        if (pStrPattern[iPattern] == '\'') {
            CFX_WideString wsLiteral = FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
            FX_INT32 iLiteralLen = wsLiteral.GetLength();
            if (iText + iLiteralLen > iLenText || FXSYS_wcsncmp(pStrText + iText, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                return FALSE;
            }
            iText += iLiteralLen;
            iPattern++;
            continue;
        } else if (pStrPattern[iPattern] != pStrText[iText]) {
            return FALSE;
        } else {
            iText++;
            iPattern++;
        }
    }
    return iPattern == iLenPattern && iText == iLenText;
}
FX_BOOL CFX_FormatString::ParseNull(const CFX_WideString& wsSrcText, const CFX_WideString& wsPattern)
{
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"null"), wsTextFormat);
    FX_INT32 iText = 0, iPattern = 0;
    FX_LPCWSTR pStrText = (FX_LPCWSTR)wsSrcText;
    FX_INT32 iLenText = wsSrcText.GetLength();
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern && iText < iLenText) {
        if (pStrPattern[iPattern] == '\'') {
            CFX_WideString wsLiteral = FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
            FX_INT32 iLiteralLen = wsLiteral.GetLength();
            if (iText + iLiteralLen > iLenText || FXSYS_wcsncmp(pStrText + iText, (FX_LPCWSTR)wsLiteral, iLiteralLen)) {
                return FALSE;
            }
            iText += iLiteralLen;
            iPattern++;
            continue;
        } else if (pStrPattern[iPattern] != pStrText[iText]) {
            return FALSE;
        } else {
            iText++;
            iPattern++;
        }
    }
    return iPattern == iLenPattern && iText == iLenText;
}
FX_BOOL	CFX_FormatString::FormatText(const CFX_WideString& wsSrcText, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsPattern.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 iLenText = wsSrcText.GetLength();
    if (iLenText == 0) {
        return FALSE;
    }
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"text"), wsTextFormat);
    FX_INT32 iText = 0, iPattern = 0;
    FX_LPCWSTR pStrText = (FX_LPCWSTR)wsSrcText;
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern) {
        switch (pStrPattern[iPattern]) {
            case '\'': {
                    wsOutput += FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
                    iPattern++;
                    break;
                }
            case 'A':
                if (iText >= iLenText || !FX_IsAlpha(pStrText[iText])) {
                    return FALSE;
                }
                wsOutput += pStrText[iText++];
                iPattern++;
                break;
            case 'X':
                if (iText >= iLenText) {
                    return FALSE;
                }
                wsOutput += pStrText[iText++];
                iPattern++;
                break;
            case 'O':
            case '0':
                if (iText >= iLenText || (!FX_IsDigit(pStrText[iText]) && !FX_IsAlpha(pStrText[iText]))) {
                    return FALSE;
                }
                wsOutput += pStrText[iText++];
                iPattern++;
                break;
            case '9':
                if (iText >= iLenText || !FX_IsDigit(pStrText[iText])) {
                    return FALSE;
                }
                wsOutput += pStrText[iText++];
                iPattern++;
                break;
            default:
                wsOutput += pStrPattern[iPattern++];
                break;
        }
    }
    return iText == iLenText;
}
static FX_INT32 FX_GetNumTrailingLimit(const CFX_WideString& wsFormat, int iDotPos, FX_BOOL &bTrimTailZeros)
{
    if (iDotPos < 0) {
        return 0;
    }
    FX_INT32 iCount = wsFormat.GetLength();
    FX_INT32 iTreading = 0;
    for (iDotPos ++; iDotPos < iCount; iDotPos ++) {
        FX_WCHAR wc = wsFormat[iDotPos];
        if (wc == L'z' || wc == L'9' || wc == 'Z') {
            iTreading ++;
            bTrimTailZeros = (wc == L'9' ? FALSE : TRUE);
        }
    }
    return iTreading;
}
FX_BOOL CFX_FormatString::FormatStrNum(FX_WSTR wsInputNum, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsInputNum.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 dot_index_f = -1;
    FX_DWORD dwNumStyle = 0;
    CFX_WideString wsNumFormat;
    IFX_Locale* pLocale = GetNumericFormat(wsPattern, dot_index_f, dwNumStyle, wsNumFormat);
    if (!pLocale || wsNumFormat.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 cc = 0, ccf = 0;
    FX_LPCWSTR strf = (FX_LPCWSTR)wsNumFormat;
    int lenf = wsNumFormat.GetLength();
    CFX_WideString wsSrcNum = wsInputNum;
    wsSrcNum.TrimLeft('0');
    if (wsSrcNum.IsEmpty() || wsSrcNum[0] == '.') {
        wsSrcNum.Insert(0, '0');
    }
    CFX_Decimal decimal = CFX_Decimal(wsSrcNum);
    if (dwNumStyle & FX_NUMSTYLE_Percent) {
        decimal = decimal * CFX_Decimal(100);
        wsSrcNum = decimal;
    }
    FX_INT32 exponent = 0;
    if (dwNumStyle & FX_NUMSTYLE_Exponent) {
        int fixed_count = 0;
        while (ccf < dot_index_f) {
            switch (strf[ccf]) {
                case '\'':
                    FX_GetLiteralText(strf, ccf, dot_index_f);
                    break;
                case '9':
                case 'z':
                case 'Z':
                    fixed_count++;
                    break;
            }
            ccf++;
        }
        int threshold = 1;
        while (fixed_count > 1) {
            threshold *= 10;
            fixed_count--;
        }
        if (decimal != CFX_Decimal(0)) {
            if (decimal < CFX_Decimal(threshold)) {
                decimal = decimal * CFX_Decimal(10);
                exponent = -1;
                while (decimal < CFX_Decimal(threshold)) {
                    decimal = decimal * CFX_Decimal(10);
                    exponent -= 1;
                }
            } else if (decimal > CFX_Decimal(threshold)) {
                threshold *= 10;
                while (decimal > CFX_Decimal(threshold)) {
                    decimal = decimal / CFX_Decimal(10);
                    exponent += 1;
                }
            }
        }
    }
    FX_BOOL bTrimTailZeros = FALSE;
    FX_INT32 iTreading = FX_GetNumTrailingLimit(wsNumFormat, dot_index_f, bTrimTailZeros);
    FX_INT32 scale = decimal.GetScale();
    if (iTreading < scale) {
        decimal.SetScale(iTreading);
        wsSrcNum = decimal;
    }
    if (bTrimTailZeros && scale > 0 && iTreading > 0) {
        wsSrcNum.TrimRight((FX_LPCWSTR)L"0");
        wsSrcNum.TrimRight((FX_LPCWSTR)L".");
    }
    CFX_WideString wsGroupSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping, wsGroupSymbol);
    FX_BOOL bNeg = FALSE;
    if (wsSrcNum[0] == '-') {
        bNeg = TRUE;
        wsSrcNum.Delete(0, 1);
    }
    FX_BOOL bAddNeg = FALSE;
    FX_LPCWSTR str = (FX_LPCWSTR)wsSrcNum;
    int len = wsSrcNum.GetLength();
    int dot_index = wsSrcNum.Find('.');
    if (dot_index == -1) {
        dot_index = len;
    }
    ccf = dot_index_f - 1;
    cc = dot_index - 1;
    while (ccf >= 0) {
        switch (strf[ccf]) {
            case '9':
                if (cc >= 0) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    cc--;
                } else {
                    wsOutput = CFX_WideStringC(L'0') + wsOutput;
                }
                ccf--;
                break;
            case 'z':
                if (cc >= 0) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    if (str[0] != '0') {
                        wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    }
                    cc--;
                }
                ccf--;
                break;
            case 'Z':
                if (cc >= 0) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    if (str[0] == '0') {
                        wsOutput = CFX_WideStringC(L' ') + wsOutput;
                    } else {
                        wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    }
                    cc--;
                } else {
                    wsOutput = CFX_WideStringC(L' ') + wsOutput;
                }
                ccf--;
                break;
            case 'S':
                if (bNeg) {
                    CFX_WideString wsMinusSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusSymbol);
                    wsOutput = wsMinusSymbol + wsOutput;
                    bAddNeg = TRUE;
                } else {
                    wsOutput = CFX_WideStringC(L' ') + wsOutput;
                }
                ccf--;
                break;
            case 's':
                if (bNeg) {
                    CFX_WideString wsMinusSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusSymbol);
                    wsOutput = wsMinusSymbol + wsOutput;
                    bAddNeg = TRUE;
                }
                ccf--;
                break;
            case 'E': {
                    CFX_WideString wsExp;
                    wsExp.Format((FX_LPCWSTR)L"E%+d", exponent);
                    wsOutput = wsExp + wsOutput;
                }
                ccf--;
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    wsOutput = wsSymbol + wsOutput;
                }
                ccf--;
                break;
            case 'r':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
                    if (bNeg) {
                        wsOutput = L"CR" + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'R':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
                    if (bNeg) {
                        wsOutput = L"CR" + wsOutput;
                    } else {
                        wsOutput = L"  " + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'b':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
                    if (bNeg) {
                        wsOutput = L"db" + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'B':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
                    if (bNeg) {
                        wsOutput = L"DB" + wsOutput;
                    } else {
                        wsOutput = L"  " + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    wsOutput = wsSymbol + wsOutput;
                }
                ccf--;
                break;
            case ',':
                if (cc >= 0) {
                    wsOutput = wsGroupSymbol + wsOutput;
                }
                ccf--;
                break;
            case '(':
                if (bNeg) {
                    wsOutput = L"(" + wsOutput;
                } else {
                    wsOutput = L" " + wsOutput;
                }
                bAddNeg = TRUE;
                ccf--;
                break;
            case ')':
                if (bNeg) {
                    wsOutput = L")" + wsOutput;
                } else {
                    wsOutput = L" " + wsOutput;
                }
                ccf--;
                break;
            case '\'':
                wsOutput = FX_GetLiteralTextReverse(strf, ccf) + wsOutput;
                ccf--;
                break;
            default:
                wsOutput = CFX_WideStringC(strf[ccf]) + wsOutput;
                ccf--;
        }
    }
    if (cc >= 0) {
        int nPos = dot_index % 3;
        wsOutput.Empty();
        for (FX_INT32 i = 0; i < dot_index; i++) {
            if (i % 3 == nPos && i != 0) {
                wsOutput += wsGroupSymbol;
            }
            wsOutput += wsSrcNum[i];
        }
        if (dot_index < len) {
            CFX_WideString wsSymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsSymbol);
            wsOutput += wsSymbol;
            wsOutput += wsSrcNum.Right(len - dot_index - 1);
        }
        if (bNeg) {
            CFX_WideString wsMinusymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
            wsOutput = wsMinusymbol + wsOutput;
        }
        return FALSE;
    }
    if (dot_index_f == wsNumFormat.GetLength()) {
        if (!bAddNeg && bNeg) {
            CFX_WideString wsMinusymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
            wsOutput = wsMinusymbol + wsOutput;
        }
        return TRUE;
    }
    CFX_WideString wsDotSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsDotSymbol);
    if (strf[dot_index_f] == 'V') {
        wsOutput += wsDotSymbol;
    } else if (strf[dot_index_f] == '.') {
        if (dot_index < len) {
            wsOutput += wsDotSymbol;
        } else {
            if (strf[dot_index_f + 1] == '9' || strf[dot_index_f + 1] == 'Z') {
                wsOutput += wsDotSymbol;
            }
        }
    }
    ccf = dot_index_f + 1;
    cc = dot_index + 1;
    while (ccf < lenf) {
        switch (strf[ccf]) {
            case '\'':
                wsOutput += FX_GetLiteralText(strf, ccf, lenf);
                ccf++;
                break;
            case '9':
                if (cc < len) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    wsOutput += str[cc];
                    cc++;
                } else {
                    wsOutput += L'0';
                }
                ccf++;
                break;
            case 'z':
                if (cc < len) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    wsOutput += str[cc];
                    cc++;
                }
                ccf++;
                break;
            case 'Z':
                if (cc < len) {
                    if (!FX_IsDigit(str[cc])) {
                        return FALSE;
                    }
                    wsOutput += str[cc];
                    cc++;
                } else {
                    wsOutput += L'0';
                }
                ccf++;
                break;
            case 'E': {
                    CFX_WideString wsExp;
                    wsExp.Format((FX_LPCWSTR)L"E%+d", exponent);
                    wsOutput += wsExp;
                }
                ccf++;
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    wsOutput += wsSymbol;
                }
                ccf++;
                break;
            case 'c':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
                    if (bNeg) {
                        wsOutput += L"CR";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'C':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
                    if (bNeg) {
                        wsOutput += L"CR";
                    } else {
                        wsOutput += L"  ";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'd':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
                    if (bNeg) {
                        wsOutput += L"db";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'D':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
                    if (bNeg) {
                        wsOutput += L"DB";
                    } else {
                        wsOutput += L"  ";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    wsOutput += wsSymbol;
                }
                ccf++;
                break;
            case '8': {
                    while (ccf < lenf && strf[ccf] == '8') {
                        ccf++;
                    }
                    while (cc < len && FX_IsDigit(str[cc])) {
                        wsOutput += str[cc];
                        cc++;
                    }
                }
                break;
            case ',':
                wsOutput += wsGroupSymbol;
                ccf++;
                break;
            case '(':
                if (bNeg) {
                    wsOutput += '(';
                } else {
                    wsOutput += ' ';
                }
                bAddNeg = TRUE;
                ccf++;
                break;
            case ')':
                if (bNeg) {
                    wsOutput += ')';
                } else {
                    wsOutput += ' ';
                }
                ccf++;
                break;
            default:
                ccf++;
        }
    }
    if (!bAddNeg && bNeg) {
        CFX_WideString wsMinusymbol;
        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
        wsOutput = wsMinusymbol + wsOutput[0] + wsOutput.Mid(1, wsOutput.GetLength() - 1);
    }
    return TRUE;
}
FX_BOOL CFX_FormatString::FormatLCNumeric(CFX_LCNumeric& lcNum, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    FX_INT32 dot_index_f = -1;
    FX_DWORD dwNumStyle = 0;
    CFX_WideString wsNumFormat;
    IFX_Locale* pLocale = GetNumericFormat(wsPattern, dot_index_f, dwNumStyle, wsNumFormat);
    if (!pLocale || wsNumFormat.IsEmpty()) {
        return FALSE;
    }
    FX_INT32 cc = 0, ccf = 0;
    FX_LPCWSTR strf = (FX_LPCWSTR)wsNumFormat;
    int lenf = wsNumFormat.GetLength();
    double dbOrgRaw = lcNum.GetDouble();
    double dbRetValue = dbOrgRaw;
    if (dwNumStyle & FX_NUMSTYLE_Percent) {
        dbRetValue *= 100;
    }
    FX_INT32 exponent = 0;
    if (dwNumStyle & FX_NUMSTYLE_Exponent) {
        int fixed_count = 0;
        while (ccf < dot_index_f) {
            switch (strf[ccf]) {
                case '\'':
                    FX_GetLiteralText(strf, ccf, dot_index_f);
                    break;
                case '9':
                case 'z':
                case 'Z':
                    fixed_count++;
                    break;
            }
            ccf++;
        }
        int threshold = 1;
        while (fixed_count > 1) {
            threshold *= 10;
            fixed_count--;
        }
        if (dbRetValue != 0) {
            if (dbRetValue < threshold) {
                dbRetValue *= 10;
                exponent = -1;
                while (dbRetValue < threshold) {
                    dbRetValue *= 10;
                    exponent -= 1;
                }
            } else if (dbRetValue > threshold) {
                threshold *= 10;
                while (dbRetValue > threshold) {
                    dbRetValue /= 10;
                    exponent += 1;
                }
            }
        }
    }
    if (dwNumStyle & (FX_NUMSTYLE_Percent | FX_NUMSTYLE_Exponent)) {
        lcNum = CFX_LCNumeric(dbRetValue);
    }
    FX_BOOL bTrimTailZeros = FALSE;
    FX_INT32 iTreading = FX_GetNumTrailingLimit(wsNumFormat, dot_index_f, bTrimTailZeros);
    CFX_WideString wsNumeric = lcNum.ToString(iTreading, bTrimTailZeros);
    if (wsNumeric.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsGroupSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping, wsGroupSymbol);
    FX_BOOL bNeg = FALSE;
    if (wsNumeric[0] == '-') {
        bNeg = TRUE;
        wsNumeric.Delete(0, 1);
    }
    FX_BOOL bAddNeg = FALSE;
    FX_LPCWSTR str = (FX_LPCWSTR)wsNumeric;
    int len = wsNumeric.GetLength();
    int dot_index = wsNumeric.Find('.');
    if (dot_index == -1) {
        dot_index = len;
    }
    ccf = dot_index_f - 1;
    cc = dot_index - 1;
    while (ccf >= 0) {
        switch (strf[ccf]) {
            case '9':
                if (cc >= 0) {
                    wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    cc--;
                } else {
                    wsOutput = CFX_WideStringC(L'0') + wsOutput;
                }
                ccf--;
                break;
            case 'z':
                if (cc >= 0) {
                    if (lcNum.m_Integral != 0) {
                        wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    }
                    cc--;
                }
                ccf--;
                break;
            case 'Z':
                if (cc >= 0) {
                    if (lcNum.m_Integral == 0) {
                        wsOutput = CFX_WideStringC(L' ') + wsOutput;
                    } else {
                        wsOutput = CFX_WideStringC(str[cc]) + wsOutput;
                    }
                    cc--;
                } else {
                    wsOutput = CFX_WideStringC(L' ') + wsOutput;
                }
                ccf--;
                break;
            case 'S':
                if (bNeg) {
                    CFX_WideString wsMinusSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusSymbol);
                    wsOutput = wsMinusSymbol + wsOutput;
                    bAddNeg = TRUE;
                } else {
                    wsOutput = CFX_WideStringC(L' ') + wsOutput;
                }
                ccf--;
                break;
            case 's':
                if (bNeg) {
                    CFX_WideString wsMinusSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusSymbol);
                    wsOutput = wsMinusSymbol + wsOutput;
                    bAddNeg = TRUE;
                }
                ccf--;
                break;
            case 'E': {
                    CFX_WideString wsExp;
                    wsExp.Format((FX_LPCWSTR)L"E%+d", exponent);
                    wsOutput = wsExp + wsOutput;
                }
                ccf--;
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    wsOutput = wsSymbol + wsOutput;
                }
                ccf--;
                break;
            case 'r':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'c') {
                    if (bNeg) {
                        wsOutput = L"CR" + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'R':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'C') {
                    if (bNeg) {
                        wsOutput = L"CR" + wsOutput;
                    } else {
                        wsOutput = L"  " + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'b':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'd') {
                    if (bNeg) {
                        wsOutput = L"db" + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'B':
                if (ccf - 1 >= 0 && strf[ccf - 1] == 'D') {
                    if (bNeg) {
                        wsOutput = L"DB" + wsOutput;
                    } else {
                        wsOutput = L"  " + wsOutput;
                    }
                    ccf -= 2;
                    bAddNeg = TRUE;
                }
                break;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    wsOutput = wsSymbol + wsOutput;
                }
                ccf--;
                break;
            case ',':
                if (cc >= 0) {
                    wsOutput = wsGroupSymbol + wsOutput;
                }
                ccf--;
                break;
            case '(':
                if (bNeg) {
                    wsOutput = L"(" + wsOutput;
                } else {
                    wsOutput = L" " + wsOutput;
                }
                bAddNeg = TRUE;
                ccf--;
                break;
            case ')':
                if (bNeg) {
                    wsOutput = L")" + wsOutput;
                } else {
                    wsOutput = L" " + wsOutput;
                }
                ccf--;
                break;
            case '\'':
                wsOutput = FX_GetLiteralTextReverse(strf, ccf) + wsOutput;
                ccf--;
                break;
            default:
                wsOutput = CFX_WideStringC(strf[ccf]) + wsOutput;
                ccf--;
        }
    }
    if (cc >= 0) {
        int nPos = dot_index % 3;
        wsOutput.Empty();
        for (FX_INT32 i = 0; i < dot_index; i++) {
            if (i % 3 == nPos && i != 0) {
                wsOutput += wsGroupSymbol;
            }
            wsOutput += wsNumeric[i];
        }
        if (dot_index < len) {
            CFX_WideString wsSymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsSymbol);
            wsOutput += wsSymbol;
            wsOutput += wsNumeric.Right(len - dot_index - 1);
        }
        if (bNeg) {
            CFX_WideString wsMinusymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
            wsOutput = wsMinusymbol + wsOutput;
        }
        return FALSE;
    }
    if (dot_index_f == wsNumFormat.GetLength()) {
        if (!bAddNeg && bNeg) {
            CFX_WideString wsMinusymbol;
            pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
            wsOutput = wsMinusymbol + wsOutput;
        }
        return TRUE;
    }
    CFX_WideString wsDotSymbol;
    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal, wsDotSymbol);
    if (strf[dot_index_f] == 'V') {
        wsOutput += wsDotSymbol;
    } else if (strf[dot_index_f] == '.') {
        if (dot_index < len) {
            wsOutput += wsDotSymbol;
        } else {
            if (strf[dot_index_f + 1] == '9' || strf[dot_index_f + 1] == 'Z') {
                wsOutput += wsDotSymbol;
            }
        }
    }
    ccf = dot_index_f + 1;
    cc = dot_index + 1;
    while (ccf < lenf) {
        switch (strf[ccf]) {
            case '\'':
                wsOutput += FX_GetLiteralText(strf, ccf, lenf);
                ccf++;
                break;
            case '9':
                if (cc < len) {
                    wsOutput += str[cc];
                    cc++;
                } else {
                    wsOutput += L'0';
                }
                ccf++;
                break;
            case 'z':
                if (cc < len) {
                    wsOutput += str[cc];
                    cc++;
                }
                ccf++;
                break;
            case 'Z':
                if (cc < len) {
                    wsOutput += str[cc];
                    cc++;
                } else {
                    wsOutput += L'0';
                }
                ccf++;
                break;
            case 'E': {
                    CFX_WideString wsExp;
                    wsExp.Format((FX_LPCWSTR)L"E%+d", exponent);
                    wsOutput += wsExp;
                }
                ccf++;
                break;
            case '$': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_CurrencySymbol, wsSymbol);
                    wsOutput += wsSymbol;
                }
                ccf++;
                break;
            case 'c':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'r') {
                    if (bNeg) {
                        wsOutput += L"CR";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'C':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'R') {
                    if (bNeg) {
                        wsOutput += L"CR";
                    } else {
                        wsOutput += L"  ";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'd':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'b') {
                    if (bNeg) {
                        wsOutput += L"db";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case 'D':
                if (ccf + 1 < lenf && strf[ccf + 1] == 'B') {
                    if (bNeg) {
                        wsOutput += L"DB";
                    } else {
                        wsOutput += L"  ";
                    }
                    ccf += 2;
                    bAddNeg = TRUE;
                }
                break;
            case '%': {
                    CFX_WideString wsSymbol;
                    pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Percent, wsSymbol);
                    wsOutput += wsSymbol;
                }
                ccf++;
                break;
            case '8': {
                    while (ccf < lenf && strf[ccf] == '8') {
                        ccf++;
                    }
                    while (cc < len && FX_IsDigit(str[cc])) {
                        wsOutput += str[cc];
                        cc++;
                    }
                }
                break;
            case ',':
                wsOutput += wsGroupSymbol;
                ccf++;
                break;
            case '(':
                if (bNeg) {
                    wsOutput += '(';
                } else {
                    wsOutput += ' ';
                }
                bAddNeg = TRUE;
                ccf++;
                break;
            case ')':
                if (bNeg) {
                    wsOutput += ')';
                } else {
                    wsOutput += ' ';
                }
                ccf++;
                break;
            default:
                ccf++;
        }
    }
    if (!bAddNeg && bNeg) {
        CFX_WideString wsMinusymbol;
        pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus, wsMinusymbol);
        wsOutput = wsOutput[0] + wsMinusymbol + wsOutput.Mid(1, wsOutput.GetLength() - 1);
    }
    return TRUE;
}
FX_BOOL	CFX_FormatString::FormatNum(const CFX_WideString& wsSrcNum, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsSrcNum.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    return FormatStrNum(wsSrcNum, wsPattern, wsOutput);
}
FX_BOOL	CFX_FormatString::FormatNum(FX_FLOAT fNum, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_LCNumeric lcNum(fNum);
    return FormatLCNumeric(lcNum, wsPattern, wsOutput);
}
FX_BOOL FX_DateFromCanonical(const CFX_WideString& wsDate, CFX_Unitime& datetime)
{
    FX_INT32 year = 1900;
    FX_INT32 month = 1;
    FX_INT32 day = 1;
    FX_WORD wYear = 0;
    int cc_start = 0, cc = 0;
    FX_LPCWSTR str = (FX_LPCWSTR)wsDate;
    int len = wsDate.GetLength();
    if(len > 10) {
        return FALSE;
    }
    while (cc < len && cc < 4) {
        if (!FX_IsDigit(str[cc])) {
            return FALSE;
        }
        wYear = wYear * 10 + str[cc++] - '0';
    }
    year  = wYear;
    if (cc < 4 || wYear < 1900) {
        return FALSE;
    }
    if(cc < len) {
        if (str[cc] == '-') {
            cc++;
        }
        cc_start = cc;
        FX_BYTE tmpM = 0;
        while (cc < len && cc < cc_start + 2) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            tmpM = tmpM * 10 + str[cc++] - '0';
        }
        month = tmpM;
        if (cc == cc_start + 1 || tmpM > 12 || tmpM < 1) {
            return FALSE;
        }
        if(cc < len) {
            if (str[cc] == '-') {
                cc++;
            }
            FX_BYTE tmpD = 0;
            cc_start = cc;
            while (cc < len && cc < cc_start + 2) {
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                tmpD = tmpD * 10 + str[cc++] - '0';
            }
            day = tmpD;
            if (tmpD < 1) {
                return FALSE;
            }
            if ((tmpM == 1 || tmpM == 3 || tmpM == 5 || tmpM == 7 || tmpM == 8 || tmpM == 10 || tmpM == 12) && tmpD > 31) {
                return FALSE;
            }
            if((tmpM == 4 || tmpM == 6 || tmpM == 9 || tmpM == 11) && tmpD > 30) {
                return FALSE;
            }
            FX_BOOL iLeapYear;
            if((wYear % 4 == 0 && wYear % 100 != 0 ) || wYear % 400 == 0) {
                iLeapYear = TRUE;
            } else {
                iLeapYear = FALSE;
            }
            if ((iLeapYear && tmpM == 2 && tmpD > 29) || (!iLeapYear && tmpM == 2 && tmpD > 28)) {
                return FALSE;
            }
        }
    }
    CFX_Unitime ut;
    ut.Set(year, month, day);
    datetime = datetime + ut;
    return TRUE;
}
FX_BOOL FX_TimeFromCanonical(FX_WSTR wsTime, CFX_Unitime& datetime, IFX_Locale* pLocale)
{
    if (wsTime.GetLength() == 0) {
        return FALSE;
    }
    FX_BYTE hour = 0;
    FX_BYTE minute = 0;
    FX_BYTE second = 0;
    FX_WORD millisecond = 0;
    int cc_start = 0, cc = cc_start;
    FX_LPCWSTR str = (FX_LPCWSTR)wsTime.GetPtr();
    int len = wsTime.GetLength();
    while (cc < len && cc < 2) {
        if (!FX_IsDigit(str[cc])) {
            return FALSE;
        }
        hour = hour * 10 + str[cc++] - '0';
    }
    if (cc < 2 || hour >= 24) {
        return FALSE;
    }
    if(cc < len) {
        if (str[cc] == ':') {
            cc++;
        }
        cc_start = cc;
        while (cc < len && cc < cc_start + 2) {
            if (!FX_IsDigit(str[cc])) {
                return FALSE;
            }
            minute = minute * 10 + str[cc++] - '0';
        }
        if (cc == cc_start + 1 || minute >= 60) {
            return FALSE;
        }
        if(cc < len) {
            if (str[cc] == ':') {
                cc++;
            }
            cc_start = cc;
            while (cc < len && cc < cc_start + 2) {
                if (!FX_IsDigit(str[cc])) {
                    return FALSE;
                }
                second = second * 10 + str[cc++] - '0';
            }
            if (cc == cc_start + 1 || second >= 60) {
                return FALSE;
            }
            if(cc < len) {
                if (str[cc] == '.') {
                    cc++;
                    cc_start = cc;
                    while (cc < len && cc < cc_start + 3) {
                        if (!FX_IsDigit(str[cc])) {
                            return FALSE;
                        }
                        millisecond = millisecond * 10 + str[cc++] - '0';
                    }
                    if (cc < cc_start + 3 || millisecond >= 1000) {
                        return FALSE;
                    }
                }
                if(cc < len) {
                    FX_TIMEZONE tzDiff;
                    tzDiff.tzHour = 0;
                    tzDiff.tzMinute = 0;
                    if (str[cc] != 'Z') {
                        cc += FX_ParseTimeZone(str + cc, len - cc, tzDiff);
                    }
                    FX_ResolveZone(hour, minute, tzDiff, pLocale);
                }
            }
        }
    }
    CFX_Unitime ut;
    ut.Set(0, 0, 0, hour, minute, second, millisecond);
    datetime = datetime + ut;
    return TRUE;
}
static FX_WORD FX_GetSolarMonthDays(FX_WORD year, FX_WORD month)
{
    if (month % 2) {
        return 31;
    } else if (month == 2) {
        return FX_IsLeapYear(year) ? 29 : 28;
    }
    return 30;
}
static FX_WORD FX_GetWeekDay(FX_WORD year, FX_WORD month, FX_WORD day)
{
    FX_WORD g_month_day[] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
    FX_WORD nDays = (year - 1) % 7 + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;
    nDays += g_month_day[month - 1] + day;
    if (FX_IsLeapYear(year) && month > 2) {
        nDays++;
    }
    return nDays % 7;
}
static FX_WORD FX_GetWeekOfMonth(FX_WORD year, FX_WORD month, FX_WORD day)
{
    FX_WORD week_day = FX_GetWeekDay(year, month, 1);
    FX_WORD week_index = 0;
    week_index += day / 7;
    day = day % 7;
    if (week_day + day > 7) {
        week_index++;
    }
    return week_index;
}
static FX_WORD FX_GetWeekOfYear(FX_WORD year, FX_WORD month, FX_WORD day)
{
    FX_WORD nDays = 0;
    for (FX_WORD i = 1; i < month; i++) {
        nDays += FX_GetSolarMonthDays(year, i);
    }
    nDays += day;
    FX_WORD week_day = FX_GetWeekDay(year, 1, 1);
    FX_WORD week_index = 1;
    week_index += nDays / 7;
    nDays = nDays % 7;
    if (week_day + nDays > 7) {
        week_index++;
    }
    return week_index;
}
static FX_BOOL FX_DateFormat(const CFX_WideString& wsDatePattern, IFX_Locale *pLocale, const CFX_Unitime &datetime, CFX_WideString& wsResult)
{
    FX_BOOL bRet = TRUE;
    FX_INT32 year = datetime.GetYear();
    FX_BYTE month = datetime.GetMonth();
    FX_BYTE day = datetime.GetDay();
    FX_INT32 ccf = 0;
    FX_LPCWSTR strf = (FX_LPCWSTR)wsDatePattern;
    FX_INT32 lenf = wsDatePattern.GetLength();
    while (ccf < lenf) {
        if (strf[ccf] == '\'') {
            wsResult += FX_GetLiteralText(strf, ccf, lenf);
            ccf++;
            continue;
        } else if (FX_Local_Find(gs_wsDateSymbols, strf[ccf]) < 0) {
            wsResult += strf[ccf++];
            continue;
        }
        FX_DWORD dwSymbolNum = 1;
        FX_DWORD dwSymbol = strf[ccf++];
        while (ccf < lenf && strf[ccf] == dwSymbol) {
            ccf++;
            dwSymbolNum++;
        }
        dwSymbol = (dwSymbol << 8) | (dwSymbolNum + '0');
        if (dwSymbol == FXBSTR_ID(0, 0, 'D', '1')) {
            CFX_WideString wsDay;
            wsDay.Format((FX_LPCWSTR)L"%d", day);
            wsResult += wsDay;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'D', '2')) {
            CFX_WideString wsDay;
            wsDay.Format((FX_LPCWSTR)L"%02d", day);
            wsResult += wsDay;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '1')) {
            FX_WORD nDays = 0;
            for (int i = 1; i < month; i++) {
                nDays += FX_GetSolarMonthDays(year, i);
            }
            nDays += day;
            CFX_WideString wsDays;
            wsDays.Format((FX_LPCWSTR)L"%d", nDays);
            wsResult += wsDays;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'J', '3')) {
            FX_WORD nDays = 0;
            for (int i = 1; i < month; i++) {
                nDays += FX_GetSolarMonthDays(year, i);
            }
            nDays += day;
            CFX_WideString wsDays;
            wsDays.Format((FX_LPCWSTR)L"%03d", nDays);
            wsResult += wsDays;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
            CFX_WideString wsMonth;
            wsMonth.Format((FX_LPCWSTR)L"%d", month);
            wsResult += wsMonth;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
            CFX_WideString wsMonth;
            wsMonth.Format((FX_LPCWSTR)L"%02d", month);
            wsResult += wsMonth;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '3')) {
            CFX_WideString wsTemp;
            pLocale->GetMonthName(month - 1, wsTemp, TRUE);
            wsResult += wsTemp;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '4')) {
            CFX_WideString wsTemp;
            pLocale->GetMonthName(month - 1, wsTemp, FALSE);
            wsResult += wsTemp;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '1')) {
            FX_WORD wWeekDay = FX_GetWeekDay(year, month, day);
            CFX_WideString wsWeekDay;
            wsWeekDay.Format((FX_LPCWSTR)L"%d", wWeekDay + 1);
            wsResult += wsWeekDay;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '3')) {
            FX_WORD wWeekDay = FX_GetWeekDay(year, month, day);
            CFX_WideString wsTemp;
            pLocale->GetDayName(wWeekDay, wsTemp, TRUE);
            wsResult += wsTemp;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'E', '4')) {
            FX_WORD wWeekDay = FX_GetWeekDay(year, month, day);
            if (pLocale) {
                CFX_WideString wsTemp;
                pLocale->GetDayName(wWeekDay, wsTemp, FALSE);
                wsResult += wsTemp;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'e', '1')) {
            FX_WORD wWeekDay = FX_GetWeekDay(year, month, day);
            CFX_WideString wsWeekDay;
            wsWeekDay.Format((FX_LPCWSTR)L"%d", wWeekDay ? wWeekDay : 7);
            wsResult += wsWeekDay;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'G', '1')) {
            CFX_WideString wsTemp;
            pLocale->GetEraName(wsTemp, year < 0);
            wsResult += wsTemp;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '2')) {
            CFX_WideString wsYear;
            wsYear.Format((FX_LPCWSTR)L"%02d", year % 100);
            wsResult += wsYear;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Y', '4')) {
            CFX_WideString wsYear;
            wsYear.Format((FX_LPCWSTR)L"%d", year);
            wsResult += wsYear;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'w', '1')) {
            FX_WORD week_index = FX_GetWeekOfMonth(year, month, day);
            CFX_WideString wsWeekInMonth;
            wsWeekInMonth.Format((FX_LPCWSTR)L"%d", week_index);
            wsResult += wsWeekInMonth;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'W', '2')) {
            FX_WORD week_index = FX_GetWeekOfYear(year, month, day);
            CFX_WideString wsWeekInYear;
            wsWeekInYear.Format((FX_LPCWSTR)L"%02d", week_index);
            wsResult += wsWeekInYear;
        }
    }
    return bRet;
}
static FX_BOOL FX_TimeFormat(const CFX_WideString& wsTimePattern, IFX_Locale *pLocale, const CFX_Unitime &datetime, CFX_WideString& wsResult)
{
    FX_BOOL bGMT = FALSE;
    FX_BOOL bRet = TRUE;
    FX_BYTE hour = datetime.GetHour();
    FX_BYTE minute = datetime.GetMinute();
    FX_BYTE second = datetime.GetSecond();
    FX_WORD millisecond = datetime.GetMillisecond();
    FX_INT32 ccf = 0;
    FX_LPCWSTR strf = (FX_LPCWSTR)wsTimePattern;
    FX_INT32 lenf = wsTimePattern.GetLength();
    FX_WORD wHour = hour;
    FX_BOOL bPM = FALSE;
    if (wsTimePattern.Find('A') != -1) {
        if (wHour >= 12) {
            bPM = TRUE;
        }
    }
    while (ccf < lenf) {
        if (strf[ccf] == '\'') {
            wsResult += FX_GetLiteralText(strf, ccf, lenf);
            ccf++;
            continue;
        } else if (FX_Local_Find(gs_wsTimeSymbols, strf[ccf]) < 0) {
            wsResult += strf[ccf++];
            continue;
        }
        FX_DWORD dwSymbolNum = 1;
        FX_DWORD dwSymbol = strf[ccf++];
        while (ccf < lenf && strf[ccf] == dwSymbol) {
            ccf++;
            dwSymbolNum++;
        }
        dwSymbol = (dwSymbol << 8) | (dwSymbolNum + '0');
        if (dwSymbol == FXBSTR_ID(0, 0, 'h', '1')) {
            if (wHour > 12) {
                wHour -= 12;
            }
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%d", wHour == 0 ? 12 : wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'h', '2')) {
            if (wHour > 12) {
                wHour -= 12;
            }
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%02d", wHour == 0 ? 12 : wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'K', '1')) {
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%d", wHour == 0 ? 24 : wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'K', '2')) {
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%02d", wHour == 0 ? 24 : wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '1')) {
            if (wHour > 12) {
                wHour -= 12;
            }
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%d", wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'H', '1')) {
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%d", wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'k', '2')) {
            if (wHour > 12) {
                wHour -= 12;
            }
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%02d", wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'H', '2')) {
            CFX_WideString wsHour;
            wsHour.Format((FX_LPCWSTR)L"%02d", wHour);
            wsResult += wsHour;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '1')) {
            CFX_WideString wsMinute;
            wsMinute.Format((FX_LPCWSTR)L"%d", minute);
            wsResult += wsMinute;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'M', '2')) {
            CFX_WideString wsMinute;
            wsMinute.Format((FX_LPCWSTR)L"%02d", minute);
            wsResult += wsMinute;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '1')) {
            CFX_WideString wsSecond;
            wsSecond.Format((FX_LPCWSTR)L"%d", second);
            wsResult += wsSecond;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'S', '2')) {
            CFX_WideString wsSecond;
            wsSecond.Format((FX_LPCWSTR)L"%02d", second);
            wsResult += wsSecond;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'F', '3')) {
            CFX_WideString wsMilliseconds;
            wsMilliseconds.Format((FX_LPCWSTR)L"%03d", millisecond);
            wsResult += wsMilliseconds;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'A', '1')) {
            CFX_WideString wsMeridiem;
            pLocale->GetMeridiemName(wsMeridiem, !bPM);
            wsResult += wsMeridiem;
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'Z', '1')) {
            wsResult += FX_WSTRC(L"GMT");
            FX_TIMEZONE tz;
            pLocale->GetTimeZone(tz);
            if (!bGMT && (tz.tzHour != 0 || tz.tzMinute != 0)) {
                if (tz.tzHour < 0) {
                    wsResult += FX_WSTRC(L"-");
                } else {
                    wsResult += FX_WSTRC(L"+");
                }
                CFX_WideString wsTimezone;
                wsTimezone.Format((FX_LPCWSTR)L"%02d:%02d", FXSYS_abs(tz.tzHour), tz.tzMinute);
                wsResult += wsTimezone;
            }
        } else if (dwSymbol == FXBSTR_ID(0, 0, 'z', '1')) {
            FX_TIMEZONE tz;
            pLocale->GetTimeZone(tz);
            if (!bGMT && tz.tzHour != 0 && tz.tzMinute != 0) {
                if (tz.tzHour < 0) {
                    wsResult += FX_WSTRC(L"-");
                } else {
                    wsResult += FX_WSTRC(L"+");
                }
                CFX_WideString wsTimezone;
                wsTimezone.Format((FX_LPCWSTR)L"%02d:%02d", FXSYS_abs(tz.tzHour), tz.tzMinute);
                wsResult += wsTimezone;
            }
        }
    }
    return bRet;
}
static FX_BOOL FX_FormatDateTime(const CFX_Unitime& dt, const CFX_WideString& wsDatePattern, const CFX_WideString& wsTimePattern,
                                 FX_BOOL bDateFirst, IFX_Locale* pLocale, CFX_WideString& wsOutput)
{
    FX_BOOL bRet = TRUE;
    CFX_WideString wsDateOut, wsTimeOut;
    if (!wsDatePattern.IsEmpty()) {
        bRet &= FX_DateFormat(wsDatePattern, pLocale, dt, wsDateOut);
    }
    if (!wsTimePattern.IsEmpty()) {
        bRet &= FX_TimeFormat(wsTimePattern, pLocale, dt, wsTimeOut);
    }
    wsOutput = bDateFirst ? wsDateOut + wsTimeOut : wsTimeOut + wsDateOut;
    return bRet;
}
FX_BOOL	CFX_FormatString::FormatDateTime(const CFX_WideString& wsSrcDateTime, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsSrcDateTime.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsDatePattern, wsTimePattern;
    IFX_Locale* pLocale = NULL;
    FX_DATETIMETYPE eCategory = GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
    if (pLocale == NULL || eCategory == FX_DATETIMETYPE_Unknown) {
        return FALSE;
    }
    CFX_Unitime dt(0);
    FX_INT32 iT = wsSrcDateTime.Find(L"T");
    if (iT < 0) {
        if (eCategory == FX_DATETIMETYPE_Date) {
            FX_DateFromCanonical(wsSrcDateTime, dt);
        } else if (eCategory == FX_DATETIMETYPE_Time) {
            FX_TimeFromCanonical(wsSrcDateTime, dt, pLocale);
        }
    } else {
        FX_DateFromCanonical(wsSrcDateTime.Left(iT), dt);
        FX_TimeFromCanonical(wsSrcDateTime.Right(wsSrcDateTime.GetLength() - iT - 1), dt, pLocale);
    }
    return FX_FormatDateTime(dt, wsDatePattern, wsTimePattern, eCategory != FX_DATETIMETYPE_TimeDate, pLocale, wsOutput);
}
FX_BOOL	CFX_FormatString::FormatDateTime(const CFX_WideString& wsSrcDateTime, const CFX_WideString& wsPattern, CFX_WideString& wsOutput, FX_DATETIMETYPE eDateTimeType)
{
    if (wsSrcDateTime.IsEmpty() || wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsDatePattern, wsTimePattern;
    IFX_Locale* pLocale = NULL;
    FX_DATETIMETYPE eCategory = GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
    if (!pLocale) {
        return FALSE;
    }
    if (eCategory == FX_DATETIMETYPE_Unknown) {
        if (eDateTimeType == FX_DATETIMETYPE_Time) {
            wsTimePattern = wsDatePattern;
            wsDatePattern.Empty();
        }
        eCategory = eDateTimeType;
    }
    if (eCategory == FX_DATETIMETYPE_Unknown) {
        return FALSE;
    }
    CFX_Unitime dt(0);
    FX_INT32 iT = wsSrcDateTime.Find(L"T");
    if (iT < 0) {
        if (eCategory == FX_DATETIMETYPE_Date && FX_DateFromCanonical(wsSrcDateTime, dt)) {
            return FX_FormatDateTime(dt, wsDatePattern, wsTimePattern, TRUE, pLocale, wsOutput);
        } else if (eCategory == FX_DATETIMETYPE_Time && FX_TimeFromCanonical(wsSrcDateTime, dt, pLocale)) {
            return FX_FormatDateTime(dt, wsDatePattern, wsTimePattern, TRUE, pLocale, wsOutput);
        }
    } else {
        CFX_WideStringC wsSrcDate((FX_LPCWSTR)wsSrcDateTime, iT);
        CFX_WideStringC wsSrcTime((FX_LPCWSTR)wsSrcDateTime + iT + 1, wsSrcDateTime.GetLength() - iT - 1);
        if(wsSrcDate.IsEmpty() || wsSrcTime.IsEmpty()) {
            return FALSE;
        }
        if(FX_DateFromCanonical(wsSrcDate, dt) && FX_TimeFromCanonical(wsSrcTime, dt, pLocale)) {
            return FX_FormatDateTime(dt, wsDatePattern, wsTimePattern, eCategory != FX_DATETIMETYPE_TimeDate, pLocale, wsOutput);
        }
    }
    return FALSE;
}
FX_BOOL	CFX_FormatString::FormatDateTime(const CFX_Unitime& dt, const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsDatePattern, wsTimePattern;
    IFX_Locale* pLocale = NULL;
    FX_DATETIMETYPE eCategory = GetDateTimeFormat(wsPattern, pLocale, wsDatePattern, wsTimePattern);
    if (!pLocale) {
        return FALSE;
    }
    return FX_FormatDateTime(dt, wsPattern, wsTimePattern, eCategory != FX_DATETIMETYPE_TimeDate, pLocale, wsOutput);
}
FX_BOOL CFX_FormatString::FormatZero(const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"zero"), wsTextFormat);
    FX_INT32 iPattern = 0;
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern) {
        if (pStrPattern[iPattern] == '\'') {
            wsOutput += FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
            iPattern++;
            continue;
        } else {
            wsOutput += pStrPattern[iPattern++];
            continue;
        }
    }
    return TRUE;
}
FX_BOOL CFX_FormatString::FormatNull(const CFX_WideString& wsPattern, CFX_WideString& wsOutput)
{
    if (wsPattern.IsEmpty()) {
        return FALSE;
    }
    CFX_WideString wsTextFormat;
    IFX_Locale* pLocale = GetTextFormat(wsPattern, FX_WSTRC(L"null"), wsTextFormat);
    FX_INT32 iPattern = 0;
    FX_LPCWSTR pStrPattern = (FX_LPCWSTR)wsTextFormat;
    FX_INT32 iLenPattern = wsTextFormat.GetLength();
    while (iPattern < iLenPattern) {
        if (pStrPattern[iPattern] == '\'') {
            wsOutput += FX_GetLiteralText(pStrPattern, iPattern, iLenPattern);
            iPattern++;
            continue;
        } else {
            wsOutput += pStrPattern[iPattern++];
            continue;
        }
    }
    return TRUE;
}
IFX_Locale* CFX_FormatString::GetPatternLocale(FX_WSTR wsLocale)
{
    if (m_bUseLCID) {
    }
    return m_pLocaleMgr->GetLocaleByName(wsLocale);
}
#define FXMATH_DECIMAL_SCALELIMIT				0x1c
#define FXMATH_DECIMAL_NEGMASK					(0x80000000L)
#define FXMATH_DECIMAL_FORCEBOOL(x)				(!(!(x)))
#define FXMATH_DECIMAL_MAKEFLAGS(NEG, SCALE)	(((SCALE) << 0x10) | ((NEG) ? FXMATH_DECIMAL_NEGMASK : 0))
#define FXMATH_DECIMAL_FLAGS2NEG(FLAGS)			FXMATH_DECIMAL_FORCEBOOL((FLAGS) & FXMATH_DECIMAL_NEGMASK)
#define FXMATH_DECIMAL_FLAGS2SCALE(FLAGS)		((FX_UINT8)(((FLAGS) & ~FXMATH_DECIMAL_NEGMASK) >> 0x10))
#define FXMATH_DECIMAL_RSHIFT32BIT(x)			((x)>>0x10>>0x10)
#define FXMATH_DECIMAL_LSHIFT32BIT(x)			((x)<<0x10<<0x10)
static inline FX_UINT8 fxmath_decimal_helper_div10(FX_UINT64& phi, FX_UINT64& pmid, FX_UINT64& plo)
{
    FX_UINT8 retVal;
    pmid += FXMATH_DECIMAL_LSHIFT32BIT(phi % 0xA);
    phi  /= 0xA;
    plo += FXMATH_DECIMAL_LSHIFT32BIT(pmid % 0xA);
    pmid /= 0xA;
    retVal = plo % 0xA;
    plo /= 0xA;
    return retVal;
}
static inline FX_UINT8 fxmath_decimal_helper_div10_any(FX_UINT64 nums[], FX_UINT8 numcount)
{
    FX_UINT8 retVal = 0;
    for(int i = numcount - 1; i > 0; i --) {
        nums[i - 1] += FXMATH_DECIMAL_LSHIFT32BIT(nums[i] % 0xA);
        nums[i]   /= 0xA;
    }
    if(numcount) {
        retVal  =  nums[0] % 0xA;
        nums[0] /= 0xA;
    }
    return retVal;
}
static inline void fxmath_decimal_helper_mul10(FX_UINT64& phi, FX_UINT64& pmid, FX_UINT64& plo)
{
    plo  *= 0xA;
    pmid = pmid * 0xA + FXMATH_DECIMAL_RSHIFT32BIT(plo);
    plo  = (FX_UINT32)plo;
    phi  = phi * 0xA + FXMATH_DECIMAL_RSHIFT32BIT(pmid);
    pmid = (FX_UINT32)pmid;
}
static inline void fxmath_decimal_helper_mul10_any(FX_UINT64 nums[], FX_UINT8 numcount)
{
    nums[0] *= 0xA;
    for(int i = 1; i < numcount; i++) {
        nums[i] = nums[i] * 0xA + FXMATH_DECIMAL_RSHIFT32BIT (nums[i - 1]);
        nums[i - 1] = (FX_UINT32)nums[i - 1];
    }
}
static inline void fxmath_decimal_helper_normalize(FX_UINT64& phi, FX_UINT64& pmid, FX_UINT64& plo)
{
    phi  += FXMATH_DECIMAL_RSHIFT32BIT(pmid);
    pmid = (FX_UINT32)pmid;
    pmid += FXMATH_DECIMAL_RSHIFT32BIT(plo);
    plo  = (FX_UINT32)plo;
    phi  += FXMATH_DECIMAL_RSHIFT32BIT(pmid);
    pmid = (FX_UINT32)pmid;
}
static inline void fxmath_decimal_helper_normalize_any(FX_UINT64 nums[], FX_UINT8 len)
{
    {
        for(int i = len - 2; i > 0; i --) {
            nums[i + 1] += FXMATH_DECIMAL_RSHIFT32BIT(nums[i]);
            nums[i] = (FX_UINT32)nums[i];
        }
    }
    {
        for(int i = 0; i < len - 1; i ++) {
            nums[i + 1] += FXMATH_DECIMAL_RSHIFT32BIT(nums[i]);
            nums[i] = (FX_UINT32)nums[i];
        }
    }
}
static inline FX_INT8 fxmath_decimal_helper_raw_compare(FX_UINT32 hi1, FX_UINT32 mid1, FX_UINT32 lo1, FX_UINT32 hi2, FX_UINT32 mid2, FX_UINT32 lo2)
{
    FX_INT8 retVal = 0;
    if (!retVal) {
        retVal += (hi1 > hi2 ? 1 : (hi1 < hi2 ? -1 : 0));
    }
    if (!retVal) {
        retVal += (mid1 > mid2 ? 1 : (mid1 < mid2 ? -1 : 0));
    }
    if (!retVal) {
        retVal += (lo1 > lo2 ? 1 : (lo1 < lo2 ? -1 : 0));
    }
    return retVal;
}
static inline FX_INT8 fxmath_decimal_helper_raw_compare_any(FX_UINT64 a[], FX_UINT8 al, FX_UINT64 b[], FX_UINT8 bl)
{
    FX_INT8 retVal = 0;
    for(int i = FX_MAX(al - 1, bl - 1); i >= 0; i --) {
        FX_UINT64 l = (i >= al ? 0 : a[i]), r = (i >= bl ? 0 : b[i]);
        retVal += (l > r ? 1 : (l < r ? -1 : 0));
        if(retVal) {
            return retVal;
        }
    }
    return retVal;
}
static inline void fxmath_decimal_helper_dec_any(FX_UINT64 a[], FX_UINT8 al)
{
    for(int i = 0; i < al; i ++) {
        if(a[i]--) {
            return;
        }
    }
}
static inline void fxmath_decimal_helper_inc_any(FX_UINT64 a[], FX_UINT8 al)
{
    for(int i = 0; i < al; i ++) {
        a[i]++;
        if((FX_UINT32)a[i] == a[i]) {
            return;
        }
        a[i] = 0;
    }
}
static inline void fxmath_decimal_helper_raw_mul(FX_UINT64 a[], FX_UINT8 al, FX_UINT64 b[], FX_UINT8 bl, FX_UINT64 c[], FX_UINT8 cl)
{
    assert(al + bl <= cl);
    {
        for(int i = 0; i < cl; i++) {
            c[i] = 0;
        }
    }
    {
        for(int i = 0; i < al; i++) {
            for(int j = 0; j < bl; j++) {
                FX_UINT64 m = (FX_UINT64)a[i] * b[j];
                c[i + j] += (FX_UINT32)m;
                c[i + j + 1] += FXMATH_DECIMAL_RSHIFT32BIT(m);
            }
        }
    }
    {
        for(int i = 0; i < cl - 1; i++) {
            c[i + 1] += FXMATH_DECIMAL_RSHIFT32BIT(c[i]);
            c[i] = (FX_UINT32)c[i];
        }
    }
    {
        for(int i = 0; i < cl; i++) {
            c[i] = (FX_UINT32)c[i];
        }
    }
}
static inline void fxmath_decimal_helper_raw_div(FX_UINT64 a[], FX_UINT8 al, FX_UINT64 b[], FX_UINT8 bl, FX_UINT64 c[], FX_UINT8 cl)
{
    int i;
    for(i = 0; i < cl; i++) {
        c[i] = 0;
    }
    FX_UINT64 left[16] = {0}, right[16] = {0};
    left[0] = 0;
    for(i = 0; i < al; i++) {
        right[i] = a[i];
    }
    FX_UINT64 tmp[16];
    while(fxmath_decimal_helper_raw_compare_any(left, al, right, al) <= 0) {
        FX_UINT64 cur[16];
        for(i = 0; i < al; i++) {
            cur[i] = left[i] + right[i];
        }
        for(i = al - 1; i >= 0; i--) {
            if(i) {
                cur[i - 1] += FXMATH_DECIMAL_LSHIFT32BIT(cur[i] % 2);
            }
            cur[i] /= 2;
        }
        fxmath_decimal_helper_raw_mul(cur, al, b, bl, tmp, 16);
        switch(fxmath_decimal_helper_raw_compare_any(tmp, 16, a, al)) {
            case -1:
                for(i = 0; i < 16; i++) {
                    left[i] = cur[i];
                }
                left[0]++;
                fxmath_decimal_helper_normalize_any(left, al);
                break;
            case 1:
                for(i = 0; i < 16; i++) {
                    right[i] = cur[i];
                }
                fxmath_decimal_helper_dec_any(right, al);
                break;
            case 0:
                for(i = 0; i < FX_MIN(al, cl); i++) {
                    c[i] = cur[i];
                }
                return;
        }
    }
    for(i = 0; i < FX_MIN(al, cl); i++) {
        c[i] = left[i];
    }
}
static inline FX_BOOL fxmath_decimal_helper_outofrange(FX_UINT64 a[], FX_UINT8 al, FX_UINT8 goal)
{
    for(int i = goal; i < al; i++) {
        if(a[i]) {
            return TRUE;
        }
    }
    return FALSE;
}
static inline void fxmath_decimal_helper_shrinkintorange(FX_UINT64 a[], FX_UINT8 al, FX_UINT8 goal, FX_UINT8& scale)
{
    FX_BOOL bRoundUp = FALSE;
    while(scale != 0 && (scale > FXMATH_DECIMAL_SCALELIMIT || fxmath_decimal_helper_outofrange(a, al, goal))) {
        bRoundUp = fxmath_decimal_helper_div10_any(a, al) >= 5;
        scale --;
    }
    if(bRoundUp) {
        fxmath_decimal_helper_normalize_any(a, goal);
        fxmath_decimal_helper_inc_any(a, goal);
    }
}
static inline void fxmath_decimal_helper_truncate(FX_UINT64& phi, FX_UINT64& pmid, FX_UINT64& plo, FX_UINT8& scale, FX_UINT8 minscale = 0)
{
    while(scale > minscale) {
        FX_UINT64 thi = phi, tmid = pmid, tlo = plo;
        if(fxmath_decimal_helper_div10(thi, tmid, tlo) != 0) {
            break;
        }
        phi = thi, pmid = tmid, plo = tlo;
        scale--;
    }
}
CFX_Decimal::CFX_Decimal()
{
    m_uLo = m_uMid = m_uHi = m_uFlags = 0;
}
CFX_Decimal::CFX_Decimal(FX_UINT64 val)
{
    m_uLo  = (FX_UINT32)val;
    m_uMid = (FX_UINT32)FXMATH_DECIMAL_RSHIFT32BIT(val);
    m_uHi  = 0;
    m_uFlags = 0;
}
CFX_Decimal::CFX_Decimal(FX_UINT32 val)
{
    m_uLo =  (FX_UINT32)val;
    m_uMid = m_uHi = 0;
    m_uFlags = 0;
}
CFX_Decimal::CFX_Decimal(FX_UINT32 lo, FX_UINT32 mid, FX_UINT32 hi, FX_BOOL neg, FX_UINT8 scale)
{
    scale = (scale > FXMATH_DECIMAL_SCALELIMIT ? 0 : scale);
    m_uLo = lo;
    m_uMid = mid;
    m_uHi = hi;
    m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(neg && IsNotZero(), scale);
}
CFX_Decimal::CFX_Decimal(FX_INT32 val)
{
    if(val >= 0) {
        *this = CFX_Decimal((FX_UINT32)val);
    } else {
        *this = CFX_Decimal((FX_UINT32) - val);
        SetNegate();
    }
}
CFX_Decimal::CFX_Decimal(FX_INT64 val)
{
    if(val >= 0) {
        *this = CFX_Decimal((FX_UINT64)val);
    } else {
        *this = CFX_Decimal((FX_UINT64) - val);
        SetNegate();
    }
}
CFX_Decimal::CFX_Decimal(FX_FLOAT val, FX_UINT8 scale )
{
    FX_FLOAT newval = fabs(val);
    FX_UINT64 phi, pmid, plo;
    plo =  (FX_UINT64)newval;
    pmid = (FX_UINT64)(newval / 1e32);
    phi =  (FX_UINT64)(newval / 1e64);
    newval = FXSYS_fmod(newval, 1.0f);
    for(FX_UINT8 iter = 0; iter < scale; iter++) {
        fxmath_decimal_helper_mul10(phi, pmid, plo);
        newval *= 10;
        plo += (FX_UINT64)newval;
        newval = FXSYS_fmod(newval, 1.0f);
    }
    plo += FXSYS_round(newval);
    fxmath_decimal_helper_normalize(phi, pmid, plo);
    m_uHi = (FX_UINT32)phi;
    m_uMid = (FX_UINT32)pmid;
    m_uLo = (FX_UINT32)plo;
    m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(val < 0 && IsNotZero(), scale);
}
CFX_Decimal::CFX_Decimal(FX_WSTR strObj)
{
    FX_LPCWSTR str = strObj.GetPtr();
    FX_LPCWSTR strBound = str + strObj.GetLength();
    FX_BOOL pointmet = 0;
    FX_BOOL negmet = 0;
    FX_UINT8 scale = 0;
    m_uHi = m_uMid = m_uLo = 0;
    while (str != strBound && *str == ' ') {
        str++;
    }
    if (str != strBound && *str == '-') {
        negmet = 1;
        str++;
    } else if (str != strBound && *str == '+') {
        str++;
    }
    while (str != strBound && ((*str >= '0' && *str <= '9') || *str == '.') && scale < FXMATH_DECIMAL_SCALELIMIT) {
        if (*str == '.') {
            if(pointmet) {
                goto cont;
            }
            pointmet = 1;
        } else {
            m_uHi = m_uHi * 0xA + FXMATH_DECIMAL_RSHIFT32BIT((FX_UINT64)m_uMid * 0xA);
            m_uMid = m_uMid * 0xA + FXMATH_DECIMAL_RSHIFT32BIT((FX_UINT64)m_uLo  * 0xA);
            m_uLo = m_uLo * 0xA + (*str - '0');
            if(pointmet) {
                scale++;
            }
        }
cont:
        str++;
    }
    m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(negmet && IsNotZero(), scale);
}
CFX_Decimal::CFX_Decimal(FX_BSTR strObj)
{
    CFX_WideString wstrObj;
    wstrObj.ConvertFrom(strObj);
    *this = CFX_Decimal(wstrObj);
}
CFX_Decimal::operator CFX_WideString() const
{
    CFX_WideString retString;
    CFX_WideString tmpbuf;
    FX_UINT64 phi = m_uHi, pmid = m_uMid, plo = m_uLo;
    while(phi || pmid || plo) {
        tmpbuf += fxmath_decimal_helper_div10(phi, pmid, plo) + '0';
    }
    FX_UINT8 outputlen = (FX_UINT8)tmpbuf.GetLength();
    FX_UINT8 scale = (FX_UINT8)FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags);
    while (scale >= outputlen) {
        tmpbuf += '0';
        outputlen++;
    }
    if (FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) && IsNotZero()) {
        retString += '-';
    }
    for (FX_UINT8 idx = 0; idx < outputlen; idx++) {
        if (idx == (outputlen - scale) && scale != 0) {
            retString += '.';
        }
        retString += tmpbuf[outputlen - 1 - idx];
    }
    return retString;
}
CFX_Decimal::operator double() const
{
    double pow = (double)(1 << 16) * (1 << 16);
    double base = ((double)m_uHi) * pow * pow + ((double)m_uMid) * pow + ((double)m_uLo);
    FX_INT8 scale = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags);
    FX_BOOL bNeg = FXMATH_DECIMAL_FLAGS2NEG(m_uFlags);
    return (bNeg ? -1 : 1) * base * ::pow(10.0, -scale);
}
void CFX_Decimal::SetScale(FX_UINT8 newscale)
{
    FX_UINT8 oldscale = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags);
    if (newscale > oldscale) {
        FX_UINT64 phi = m_uHi, pmid = m_uMid, plo = m_uLo;
        for (FX_UINT8 iter = 0; iter < newscale - oldscale; iter++) {
            fxmath_decimal_helper_mul10(phi, pmid, plo);
        }
        m_uHi = (FX_UINT32)phi;
        m_uMid = (FX_UINT32)pmid;
        m_uLo = (FX_UINT32)plo;
        m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) && IsNotZero(), newscale);
    } else if (newscale < oldscale) {
        FX_UINT64 phi, pmid, plo;
        phi = 0, pmid = 0, plo = 5;
        {
            for(FX_UINT8 iter = 0; iter < oldscale - newscale - 1; iter++) {
                fxmath_decimal_helper_mul10(phi, pmid, plo);
            }
        }
        phi += m_uHi;
        pmid += m_uMid;
        plo += m_uLo;
        fxmath_decimal_helper_normalize(phi, pmid, plo);
        {
            for(FX_UINT8 iter = 0; iter < oldscale - newscale; iter++) {
                fxmath_decimal_helper_div10(phi, pmid, plo);
            }
        }
        m_uHi = (FX_UINT32)phi;
        m_uMid = (FX_UINT32)pmid;
        m_uLo = (FX_UINT32)plo;
        m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) && IsNotZero(), newscale);
    }
}
FX_UINT8 CFX_Decimal::GetScale()
{
    FX_UINT8 oldscale = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags);
    return oldscale;
}
void CFX_Decimal::SetAbs()
{
    m_uFlags &= ~FXMATH_DECIMAL_NEGMASK;
}
void CFX_Decimal::SetNegate()
{
    if(IsNotZero()) {
        m_uFlags ^= FXMATH_DECIMAL_NEGMASK;
    }
}
void CFX_Decimal::FloorOrCeil(FX_BOOL bFloor)
{
    FX_UINT64 nums[3] = {m_uLo, m_uMid, m_uHi};
    FX_BOOL bDataLoss = FALSE;
    for(int i = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags); i > 0; i --) {
        bDataLoss = fxmath_decimal_helper_div10_any(nums, 3) || bDataLoss;
    }
    if(bDataLoss && (bFloor ? FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) : !FXMATH_DECIMAL_FLAGS2NEG(m_uFlags))) {
        fxmath_decimal_helper_inc_any(nums, 3);
    }
    m_uHi = (FX_UINT32)nums[2];
    m_uMid = (FX_UINT32)nums[1];
    m_uLo = (FX_UINT32)nums[0];
    m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) && IsNotZero(), 0);
}
void CFX_Decimal::SetFloor()
{
    FloorOrCeil(TRUE);
}
void CFX_Decimal::SetCeiling()
{
    FloorOrCeil(FALSE);
}
void CFX_Decimal::SetTruncate()
{
    FloorOrCeil(!FXMATH_DECIMAL_FLAGS2NEG(m_uFlags));
}
void CFX_Decimal::Swap(CFX_Decimal& val)
{
    FX_UINT32 tmp;
    tmp = m_uHi;
    m_uHi = val.m_uHi;
    val.m_uHi = tmp;
    tmp = m_uMid;
    m_uMid = val.m_uMid;
    val.m_uMid = tmp;
    tmp = m_uLo;
    m_uLo = val.m_uLo;
    val.m_uLo = tmp;
    tmp = m_uFlags;
    m_uFlags = val.m_uFlags;
    val.m_uFlags = tmp;
}
FX_INT8 CFX_Decimal::Compare (const CFX_Decimal& val) const
{
    CFX_Decimal lhs = *this, rhs = val;
    FX_INT8 retVal = 0;
    if (FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags) != FXMATH_DECIMAL_FLAGS2SCALE(rhs.m_uFlags)) {
        FX_UINT8 scale = FX_MIN(FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags), FXMATH_DECIMAL_FLAGS2SCALE(rhs.m_uFlags));
        lhs.SetScale(scale);
        rhs.SetScale(scale);
    }
    retVal = - (FXMATH_DECIMAL_FLAGS2NEG(lhs.m_uFlags) - FXMATH_DECIMAL_FLAGS2NEG(rhs.m_uFlags));
    if (retVal) {
        return retVal;
    }
    retVal = fxmath_decimal_helper_raw_compare(lhs.m_uHi, lhs.m_uMid, lhs.m_uLo, rhs.m_uHi, rhs.m_uMid, rhs.m_uLo);
    return (FXMATH_DECIMAL_FLAGS2NEG(lhs.m_uFlags) ? - retVal : retVal);
}
CFX_Decimal CFX_Decimal::AddOrMinus(const CFX_Decimal& val, FX_BOOL isAdding) const
{
    CFX_Decimal lhs = *this, rhs = val;
    if (FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags) != FXMATH_DECIMAL_FLAGS2SCALE(rhs.m_uFlags)) {
        FX_UINT8 scale = FX_MAX(FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags), FXMATH_DECIMAL_FLAGS2SCALE(rhs.m_uFlags));
        lhs.SetScale(scale);
        rhs.SetScale(scale);
    }
    if(!isAdding) {
        rhs.SetNegate();
    }
    FX_BOOL doRawAdd = (FXMATH_DECIMAL_FLAGS2NEG(lhs.m_uFlags) == FXMATH_DECIMAL_FLAGS2NEG(rhs.m_uFlags));
    if (doRawAdd) {
        FX_UINT64 phi = lhs.m_uHi, pmid = lhs.m_uMid, plo = lhs.m_uLo;
        phi += rhs.m_uHi;
        pmid += rhs.m_uMid;
        plo += rhs.m_uLo;
        fxmath_decimal_helper_normalize(phi, pmid, plo);
        if(FXMATH_DECIMAL_RSHIFT32BIT(phi) && FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags) != 0) {
            fxmath_decimal_helper_div10(phi, pmid, plo);
            lhs.m_uFlags = FXMATH_DECIMAL_MAKEFLAGS(FXMATH_DECIMAL_FLAGS2NEG(lhs.m_uFlags), FXMATH_DECIMAL_FLAGS2SCALE(lhs.m_uFlags) - 1);
        }
        lhs.m_uHi = (FX_UINT32)phi;
        lhs.m_uMid = (FX_UINT32)pmid;
        lhs.m_uLo = (FX_UINT32)plo;
        return lhs;
    } else {
        if(fxmath_decimal_helper_raw_compare(lhs.m_uHi, lhs.m_uMid, lhs.m_uLo, rhs.m_uHi, rhs.m_uMid, rhs.m_uLo) < 0) {
            lhs.Swap(rhs);
        }
        lhs.m_uHi -= rhs.m_uHi;
        if(lhs.m_uMid < rhs.m_uMid) {
            lhs.m_uHi--;
        }
        lhs.m_uMid -= rhs.m_uMid;
        if(lhs.m_uLo < rhs.m_uLo) {
            if(!lhs.m_uMid) {
                lhs.m_uHi--;
            }
            lhs.m_uMid--;
        }
        lhs.m_uLo -= rhs.m_uLo;
        return lhs;
    }
}
CFX_Decimal CFX_Decimal::Multiply(const CFX_Decimal& val) const
{
    FX_UINT64 a[3] = {m_uLo, m_uMid, m_uHi}, b[3] = {val.m_uLo, val.m_uMid, val.m_uHi};
    FX_UINT64 c[6];
    fxmath_decimal_helper_raw_mul(a, 3, b, 3, c, 6);
    FX_BOOL neg = FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) ^ FXMATH_DECIMAL_FLAGS2NEG(val.m_uFlags);
    FX_UINT8 scale = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags) + FXMATH_DECIMAL_FLAGS2SCALE(val.m_uFlags);
    fxmath_decimal_helper_shrinkintorange(c, 6, 3, scale);
    return CFX_Decimal((FX_UINT32)c[0], (FX_UINT32)c[1], (FX_UINT32)c[2], neg, scale);
}
CFX_Decimal CFX_Decimal::Divide(const CFX_Decimal& val) const
{
    if(!val.IsNotZero()) {
        return CFX_Decimal();
    }
    FX_BOOL neg = FXMATH_DECIMAL_FLAGS2NEG(m_uFlags) ^ FXMATH_DECIMAL_FLAGS2NEG(val.m_uFlags);
    FX_UINT64 a[7] = {m_uLo, m_uMid, m_uHi}, b[3] = {val.m_uLo, val.m_uMid, val.m_uHi}, c[7] = {0};
    FX_UINT8 scale = 0;
    if(FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags) < FXMATH_DECIMAL_FLAGS2SCALE(val.m_uFlags)) {
        for(int i = FXMATH_DECIMAL_FLAGS2SCALE(val.m_uFlags) - FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags); i > 0; i--) {
            fxmath_decimal_helper_mul10_any(a, 7);
        }
    } else {
        scale = FXMATH_DECIMAL_FLAGS2SCALE(m_uFlags) - FXMATH_DECIMAL_FLAGS2SCALE(val.m_uFlags);
    }
    FX_UINT8 minscale = scale;
    if(!IsNotZero()) {
        return CFX_Decimal(0, 0, 0, 0, minscale);
    }
    while(!a[6]) {
        fxmath_decimal_helper_mul10_any(a, 7);
        scale++;
    }
    fxmath_decimal_helper_div10_any(a, 7);
    scale--;
    fxmath_decimal_helper_raw_div(a, 6, b, 3, c, 7);
    fxmath_decimal_helper_shrinkintorange(c, 6, 3, scale);
    fxmath_decimal_helper_truncate(c[2], c[1], c[0], scale, minscale);
    return CFX_Decimal((FX_UINT32)c[0], (FX_UINT32)c[1], (FX_UINT32)c[2], neg, scale);
}
CFX_Decimal CFX_Decimal::Modulus(const CFX_Decimal& val) const
{
    CFX_Decimal lhs = *this, rhs_abs = val;
    rhs_abs.SetAbs();
    if(!rhs_abs.IsNotZero()) {
        return *this;
    }
    while(TRUE) {
        CFX_Decimal lhs_abs = lhs;
        lhs_abs.SetAbs();
        if(lhs_abs < rhs_abs) {
            break;
        }
        CFX_Decimal quot = lhs / rhs_abs;
        quot.SetTruncate();
        lhs = lhs - quot * rhs_abs;
    }
    return lhs;
}
FX_BOOL CFX_Decimal::operator == (const CFX_Decimal& val) const
{
    return Compare(val) == 0;
}
FX_BOOL CFX_Decimal::operator <= (const CFX_Decimal& val) const
{
    return Compare(val) <= 0;
}
FX_BOOL CFX_Decimal::operator >= (const CFX_Decimal& val) const
{
    return Compare(val) >= 0;
}
FX_BOOL CFX_Decimal::operator != (const CFX_Decimal& val) const
{
    return Compare(val) != 0;
}
FX_BOOL CFX_Decimal::operator <  (const CFX_Decimal& val) const
{
    return Compare(val) < 0;
}
FX_BOOL CFX_Decimal::operator >  (const CFX_Decimal& val) const
{
    return Compare(val) > 0;
}
CFX_Decimal CFX_Decimal::operator + (const CFX_Decimal& val) const
{
    return AddOrMinus(val, TRUE);
}
CFX_Decimal CFX_Decimal::operator - (const CFX_Decimal& val) const
{
    return AddOrMinus(val, FALSE);
}
CFX_Decimal CFX_Decimal::operator * (const CFX_Decimal& val) const
{
    return Multiply(val);
}
CFX_Decimal CFX_Decimal::operator / (const CFX_Decimal& val) const
{
    return Divide(val);
}
CFX_Decimal CFX_Decimal::operator % (const CFX_Decimal& val) const
{
    return Modulus(val);
}
