// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_localevalue.h"

#include <wchar.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/span.h"
#include "xfa/fgas/crt/cfgas_stringformatter.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

CFGAS_StringFormatter::Category ValueCategory(
    CFGAS_StringFormatter::Category eCategory,
    CXFA_LocaleValue::ValueType eValueType) {
  if (eCategory != CFGAS_StringFormatter::Category::kUnknown)
    return eCategory;

  switch (eValueType) {
    case CXFA_LocaleValue::ValueType::kBoolean:
    case CXFA_LocaleValue::ValueType::kInteger:
    case CXFA_LocaleValue::ValueType::kDecimal:
    case CXFA_LocaleValue::ValueType::kFloat:
      return CFGAS_StringFormatter::Category::kNum;
    case CXFA_LocaleValue::ValueType::kText:
      return CFGAS_StringFormatter::Category::kText;
    case CXFA_LocaleValue::ValueType::kDate:
      return CFGAS_StringFormatter::Category::kDate;
    case CXFA_LocaleValue::ValueType::kTime:
      return CFGAS_StringFormatter::Category::kTime;
    case CXFA_LocaleValue::ValueType::kDateTime:
      return CFGAS_StringFormatter::Category::kDateTime;
    default:
      return CFGAS_StringFormatter::Category::kUnknown;
  }
}

bool ValueSplitDateTime(const WideString& wsDateTime,
                        WideString& wsDate,
                        WideString& wsTime) {
  wsDate.clear();
  wsTime.clear();
  if (wsDateTime.IsEmpty())
    return false;

  auto nSplitIndex = wsDateTime.Find('T');
  if (!nSplitIndex.has_value())
    nSplitIndex = wsDateTime.Find(' ');
  if (!nSplitIndex.has_value())
    return false;

  wsDate = wsDateTime.First(nSplitIndex.value());
  wsTime = wsDateTime.Last(wsDateTime.GetLength() - nSplitIndex.value() - 1);
  return true;
}

class ScopedLocale {
  CPPGC_STACK_ALLOCATED();  // Raw/Unowned pointers allowed.

 public:
  ScopedLocale(CXFA_LocaleMgr* pLocaleMgr, GCedLocaleIface* pNewLocale)
      : m_pLocaleMgr(pLocaleMgr),
        m_pNewLocale(pNewLocale),
        m_pOrigLocale(pNewLocale ? m_pLocaleMgr->GetDefLocale() : nullptr) {
    if (m_pNewLocale)
      m_pLocaleMgr->SetDefLocale(pNewLocale);
  }

  ~ScopedLocale() {
    if (m_pNewLocale)
      m_pLocaleMgr->SetDefLocale(m_pOrigLocale);
  }

  ScopedLocale(const ScopedLocale& that) = delete;
  ScopedLocale& operator=(const ScopedLocale& that) = delete;

 private:
  UnownedPtr<CXFA_LocaleMgr> const m_pLocaleMgr;    // Ok, stack-only.
  UnownedPtr<GCedLocaleIface> const m_pNewLocale;   // Ok, stack-only.
  UnownedPtr<GCedLocaleIface> const m_pOrigLocale;  // Ok, stack-only.
};

}  // namespace

CXFA_LocaleValue::CXFA_LocaleValue() = default;

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType, CXFA_LocaleMgr* pLocaleMgr)
    : m_pLocaleMgr(pLocaleMgr),
      m_eType(eType),
      m_bValid(m_eType != ValueType::kNull) {}

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType,
                                   const WideString& wsValue,
                                   CXFA_LocaleMgr* pLocaleMgr)
    : m_pLocaleMgr(pLocaleMgr),
      m_wsValue(wsValue),
      m_eType(eType),
      m_bValid(ValidateCanonicalValue(wsValue, eType)) {}

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType,
                                   const WideString& wsValue,
                                   const WideString& wsFormat,
                                   GCedLocaleIface* pLocale,
                                   CXFA_LocaleMgr* pLocaleMgr)
    : m_pLocaleMgr(pLocaleMgr),
      m_eType(eType),
      m_bValid(ParsePatternValue(wsValue, wsFormat, pLocale)) {}

CXFA_LocaleValue::CXFA_LocaleValue(const CXFA_LocaleValue& that) = default;

CXFA_LocaleValue& CXFA_LocaleValue::operator=(const CXFA_LocaleValue& that) =
    default;

CXFA_LocaleValue::~CXFA_LocaleValue() = default;

bool CXFA_LocaleValue::ValidateValue(const WideString& wsValue,
                                     const WideString& wsPattern,
                                     GCedLocaleIface* pLocale,
                                     WideString* pMatchFormat) {
  if (!m_pLocaleMgr)
    return false;

  ScopedLocale scoped_locale(m_pLocaleMgr, pLocale);
  std::vector<WideString> wsPatterns =
      CFGAS_StringFormatter::SplitOnBars(wsPattern);

  WideString wsOutput;
  bool bRet = false;
  size_t i = 0;
  for (; !bRet && i < wsPatterns.size(); i++) {
    const WideString& wsFormat = wsPatterns[i];
    auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
    switch (ValueCategory(pFormat->GetCategory(), m_eType)) {
      case CFGAS_StringFormatter::Category::kNull:
        bRet = pFormat->ParseNull(wsValue);
        if (!bRet)
          bRet = wsValue.IsEmpty();
        break;
      case CFGAS_StringFormatter::Category::kZero:
        bRet = pFormat->ParseZero(wsValue);
        if (!bRet)
          bRet = wsValue.EqualsASCII("0");
        break;
      case CFGAS_StringFormatter::Category::kNum: {
        WideString fNum;
        bRet = pFormat->ParseNum(m_pLocaleMgr, wsValue, &fNum);
        if (!bRet)
          bRet = pFormat->FormatNum(m_pLocaleMgr, wsValue, &wsOutput);
        break;
      }
      case CFGAS_StringFormatter::Category::kText:
        bRet = pFormat->ParseText(wsValue, &wsOutput);
        wsOutput.clear();
        if (!bRet)
          bRet = pFormat->FormatText(wsValue, &wsOutput);
        break;
      case CFGAS_StringFormatter::Category::kDate: {
        CFX_DateTime dt;
        bRet = ValidateCanonicalDate(wsValue, &dt);
        if (!bRet) {
          bRet = pFormat->ParseDateTime(
              m_pLocaleMgr, wsValue, CFGAS_StringFormatter::DateTimeType::kDate,
              &dt);
          if (!bRet) {
            bRet = pFormat->FormatDateTime(
                m_pLocaleMgr, wsValue,
                CFGAS_StringFormatter::DateTimeType::kDate, &wsOutput);
          }
        }
        break;
      }
      case CFGAS_StringFormatter::Category::kTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            m_pLocaleMgr, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
            &dt);
        if (!bRet) {
          bRet = pFormat->FormatDateTime(
              m_pLocaleMgr, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
              &wsOutput);
        }
        break;
      }
      case CFGAS_StringFormatter::Category::kDateTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            m_pLocaleMgr, wsValue,
            CFGAS_StringFormatter::DateTimeType::kDateTime, &dt);
        if (!bRet) {
          bRet = pFormat->FormatDateTime(
              m_pLocaleMgr, wsValue,
              CFGAS_StringFormatter::DateTimeType::kDateTime, &wsOutput);
        }
        break;
      }
      default:
        bRet = false;
        break;
    }
  }
  if (bRet && pMatchFormat)
    *pMatchFormat = wsPatterns[i - 1];
  return bRet;
}

double CXFA_LocaleValue::GetDoubleNum() const {
  if (!m_bValid || (m_eType != CXFA_LocaleValue::ValueType::kBoolean &&
                    m_eType != CXFA_LocaleValue::ValueType::kInteger &&
                    m_eType != CXFA_LocaleValue::ValueType::kDecimal &&
                    m_eType != CXFA_LocaleValue::ValueType::kFloat)) {
    return 0;
  }

  return wcstod(m_wsValue.c_str(), nullptr);
}

CFX_DateTime CXFA_LocaleValue::GetDate() const {
  if (!m_bValid || m_eType != CXFA_LocaleValue::ValueType::kDate)
    return CFX_DateTime();

  CFX_DateTime dt;
  FX_DateFromCanonical(m_wsValue.span(), &dt);
  return dt;
}

CFX_DateTime CXFA_LocaleValue::GetTime() const {
  if (!m_bValid || m_eType != CXFA_LocaleValue::ValueType::kTime)
    return CFX_DateTime();

  CFX_DateTime dt;
  FX_TimeFromCanonical(m_pLocaleMgr->GetDefLocale(), m_wsValue.span(), &dt);
  return dt;
}

bool CXFA_LocaleValue::SetDate(const CFX_DateTime& d) {
  m_eType = CXFA_LocaleValue::ValueType::kDate;
  m_wsValue = WideString::Format(L"%04d-%02d-%02d", d.GetYear(), d.GetMonth(),
                                 d.GetDay());
  return true;
}

bool CXFA_LocaleValue::SetTime(const CFX_DateTime& t) {
  m_eType = CXFA_LocaleValue::ValueType::kTime;
  m_wsValue = WideString::Format(L"%02d:%02d:%02d", t.GetHour(), t.GetMinute(),
                                 t.GetSecond());
  if (t.GetMillisecond() > 0)
    m_wsValue += WideString::Format(L"%:03d", t.GetMillisecond());
  return true;
}

bool CXFA_LocaleValue::SetDateTime(const CFX_DateTime& dt) {
  m_eType = CXFA_LocaleValue::ValueType::kDateTime;
  m_wsValue = WideString::Format(L"%04d-%02d-%02dT%02d:%02d:%02d", dt.GetYear(),
                                 dt.GetMonth(), dt.GetDay(), dt.GetHour(),
                                 dt.GetMinute(), dt.GetSecond());
  if (dt.GetMillisecond() > 0)
    m_wsValue += WideString::Format(L"%:03d", dt.GetMillisecond());
  return true;
}

bool CXFA_LocaleValue::FormatPatterns(WideString& wsResult,
                                      const WideString& wsFormat,
                                      GCedLocaleIface* pLocale,
                                      XFA_ValuePicture eValueType) const {
  wsResult.clear();
  for (const auto& pattern : CFGAS_StringFormatter::SplitOnBars(wsFormat)) {
    if (FormatSinglePattern(wsResult, pattern, pLocale, eValueType))
      return true;
  }
  return false;
}

bool CXFA_LocaleValue::FormatSinglePattern(WideString& wsResult,
                                           const WideString& wsFormat,
                                           GCedLocaleIface* pLocale,
                                           XFA_ValuePicture eValueType) const {
  if (!m_pLocaleMgr)
    return false;

  ScopedLocale scoped_locale(m_pLocaleMgr, pLocale);
  wsResult.clear();

  bool bRet = false;
  auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
  CFGAS_StringFormatter::Category eCategory =
      ValueCategory(pFormat->GetCategory(), m_eType);
  switch (eCategory) {
    case CFGAS_StringFormatter::Category::kNull:
      if (m_wsValue.IsEmpty())
        bRet = pFormat->FormatNull(&wsResult);
      break;
    case CFGAS_StringFormatter::Category::kZero:
      if (m_wsValue.EqualsASCII("0"))
        bRet = pFormat->FormatZero(&wsResult);
      break;
    case CFGAS_StringFormatter::Category::kNum:
      bRet = pFormat->FormatNum(m_pLocaleMgr, m_wsValue, &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kText:
      bRet = pFormat->FormatText(m_wsValue, &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kDate:
      bRet = pFormat->FormatDateTime(m_pLocaleMgr, m_wsValue,
                                     CFGAS_StringFormatter::DateTimeType::kDate,
                                     &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kTime:
      bRet = pFormat->FormatDateTime(m_pLocaleMgr, m_wsValue,
                                     CFGAS_StringFormatter::DateTimeType::kTime,
                                     &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kDateTime:
      bRet = pFormat->FormatDateTime(
          m_pLocaleMgr, m_wsValue,
          CFGAS_StringFormatter::DateTimeType::kDateTime, &wsResult);
      break;
    default:
      wsResult = m_wsValue;
      bRet = true;
  }
  if (!bRet && (eCategory != CFGAS_StringFormatter::Category::kNum ||
                eValueType != XFA_ValuePicture::kDisplay)) {
    wsResult = m_wsValue;
  }

  return bRet;
}

bool CXFA_LocaleValue::ValidateCanonicalValue(const WideString& wsValue,
                                              ValueType eType) {
  if (wsValue.IsEmpty())
    return true;

  CFX_DateTime dt;
  switch (eType) {
    case ValueType::kDate: {
      if (ValidateCanonicalDate(wsValue, &dt))
        return true;

      WideString wsDate;
      WideString wsTime;
      if (ValueSplitDateTime(wsValue, wsDate, wsTime) &&
          ValidateCanonicalDate(wsDate, &dt)) {
        return true;
      }
      return false;
    }
    case ValueType::kTime: {
      if (ValidateCanonicalTime(wsValue))
        return true;

      WideString wsDate;
      WideString wsTime;
      if (ValueSplitDateTime(wsValue, wsDate, wsTime) &&
          ValidateCanonicalTime(wsTime)) {
        return true;
      }
      return false;
    }
    case ValueType::kDateTime: {
      WideString wsDate, wsTime;
      if (ValueSplitDateTime(wsValue, wsDate, wsTime) &&
          ValidateCanonicalDate(wsDate, &dt) && ValidateCanonicalTime(wsTime)) {
        return true;
      }
    } break;
    default: {
      break;
    }
  }
  return true;
}

bool CXFA_LocaleValue::ValidateCanonicalDate(const WideString& wsDate,
                                             CFX_DateTime* unDate) {
  static const uint8_t LastDay[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
  static const uint16_t wCountY = 4;
  static const uint16_t wCountM = 2;
  static const uint16_t wCountD = 2;
  pdfium::span<const wchar_t> spDate = wsDate.span();
  if (spDate.size() < wCountY ||
      spDate.size() > wCountY + wCountM + wCountD + 2) {
    return false;
  }
  const bool bSymbol = wsDate.Contains(0x2D);
  uint16_t wYear = 0;
  uint16_t wMonth = 0;
  uint16_t wDay = 0;
  size_t nIndex = 0;
  size_t nStart = 0;
  while (nIndex < wCountY && spDate[nIndex] != '\0') {
    if (!FXSYS_IsDecimalDigit(spDate[nIndex]))
      return false;

    wYear = (spDate[nIndex] - '0') + wYear * 10;
    nIndex++;
  }
  if (bSymbol) {
    if (nIndex >= spDate.size() || spDate[nIndex] != 0x2D)
      return false;
    nIndex++;
  }

  nStart = nIndex;
  while (nIndex < spDate.size() && spDate[nIndex] != '\0' &&
         nIndex - nStart < wCountM) {
    if (!FXSYS_IsDecimalDigit(spDate[nIndex]))
      return false;

    wMonth = (spDate[nIndex] - '0') + wMonth * 10;
    nIndex++;
  }
  if (bSymbol) {
    if (nIndex >= spDate.size() || spDate[nIndex] != 0x2D)
      return false;
    nIndex++;
  }

  nStart = nIndex;
  while (nIndex < spDate.size() && spDate[nIndex] != '\0' &&
         nIndex - nStart < wCountD) {
    if (!FXSYS_IsDecimalDigit(spDate[nIndex]))
      return false;

    wDay = (spDate[nIndex] - '0') + wDay * 10;
    nIndex++;
  }
  if (nIndex != spDate.size())
    return false;
  if (wYear < 1900 || wYear > 2029)
    return false;
  if (wMonth < 1 || wMonth > 12)
    return wMonth == 0 && spDate.size() == wCountY;
  if (wDay < 1)
    return wDay == 0 && spDate.size() == wCountY + wCountM;
  if (wMonth == 2) {
    if (wYear % 400 == 0 || (wYear % 100 != 0 && wYear % 4 == 0)) {
      if (wDay > 29)
        return false;
    } else if (wDay > 28) {
      return false;
    }
  } else if (wDay > LastDay[wMonth - 1]) {
    return false;
  }

  unDate->SetDate(wYear, static_cast<uint8_t>(wMonth),
                  static_cast<uint8_t>(wDay));
  return true;
}

bool CXFA_LocaleValue::ValidateCanonicalTime(const WideString& wsTime) {
  pdfium::span<const wchar_t> spTime = wsTime.span();
  if (spTime.size() < 2)
    return false;

  const uint16_t wCountH = 2;
  const uint16_t wCountM = 2;
  const uint16_t wCountS = 2;
  const uint16_t wCountF = 3;
  const bool bSymbol = wsTime.Contains(':');
  uint16_t wHour = 0;
  uint16_t wMinute = 0;
  uint16_t wSecond = 0;
  uint16_t wFraction = 0;
  size_t nIndex = 0;
  size_t nStart = 0;
  while (nIndex - nStart < wCountH && spTime[nIndex]) {
    if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
      return false;
    wHour = spTime[nIndex] - '0' + wHour * 10;
    nIndex++;
  }
  if (bSymbol) {
    if (nIndex < spTime.size() && spTime[nIndex] != ':')
      return false;
    nIndex++;
  }

  nStart = nIndex;
  while (nIndex < spTime.size() && spTime[nIndex] != '\0' &&
         nIndex - nStart < wCountM) {
    if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
      return false;
    wMinute = spTime[nIndex] - '0' + wMinute * 10;
    nIndex++;
  }
  if (bSymbol) {
    if (nIndex >= spTime.size() || spTime[nIndex] != ':')
      return false;
    nIndex++;
  }
  nStart = nIndex;
  while (nIndex < spTime.size() && spTime[nIndex] != '\0' &&
         nIndex - nStart < wCountS) {
    if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
      return false;
    wSecond = spTime[nIndex] - '0' + wSecond * 10;
    nIndex++;
  }
  auto pos = wsTime.Find('.');
  if (pos.has_value() && pos.value() != 0) {
    if (nIndex >= spTime.size() || spTime[nIndex] != '.')
      return false;
    nIndex++;
    nStart = nIndex;
    while (nIndex < spTime.size() && spTime[nIndex] != '\0' &&
           nIndex - nStart < wCountF) {
      if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
        return false;
      wFraction = spTime[nIndex] - '0' + wFraction * 10;
      nIndex++;
    }
  }
  if (nIndex < spTime.size()) {
    if (spTime[nIndex] == 'Z') {
      nIndex++;
    } else if (spTime[nIndex] == '-' || spTime[nIndex] == '+') {
      int16_t nOffsetH = 0;
      int16_t nOffsetM = 0;
      nIndex++;
      nStart = nIndex;
      while (nIndex < spTime.size() && spTime[nIndex] != '\0' &&
             nIndex - nStart < wCountH) {
        if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
          return false;
        nOffsetH = spTime[nIndex] - '0' + nOffsetH * 10;
        nIndex++;
      }
      if (bSymbol) {
        if (nIndex >= spTime.size() || spTime[nIndex] != ':')
          return false;
        nIndex++;
      }
      nStart = nIndex;
      while (nIndex < spTime.size() && spTime[nIndex] != '\0' &&
             nIndex - nStart < wCountM) {
        if (!FXSYS_IsDecimalDigit(spTime[nIndex]))
          return false;
        nOffsetM = spTime[nIndex] - '0' + nOffsetM * 10;
        nIndex++;
      }
      if (nOffsetH > 12 || nOffsetM >= 60)
        return false;
    }
  }
  return nIndex == spTime.size() && wHour < 24 && wMinute < 60 &&
         wSecond < 60 && wFraction <= 999;
}

bool CXFA_LocaleValue::ParsePatternValue(const WideString& wsValue,
                                         const WideString& wsPattern,
                                         GCedLocaleIface* pLocale) {
  if (!m_pLocaleMgr)
    return false;

  std::vector<WideString> wsPatterns =
      CFGAS_StringFormatter::SplitOnBars(wsPattern);

  ScopedLocale scoped_locale(m_pLocaleMgr, pLocale);
  bool bRet = false;
  for (size_t i = 0; !bRet && i < wsPatterns.size(); i++) {
    const WideString& wsFormat = wsPatterns[i];
    auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
    switch (ValueCategory(pFormat->GetCategory(), m_eType)) {
      case CFGAS_StringFormatter::Category::kNull:
        bRet = pFormat->ParseNull(wsValue);
        if (bRet)
          m_wsValue.clear();
        break;
      case CFGAS_StringFormatter::Category::kZero:
        bRet = pFormat->ParseZero(wsValue);
        if (bRet)
          m_wsValue = L"0";
        break;
      case CFGAS_StringFormatter::Category::kNum: {
        WideString fNum;
        bRet = pFormat->ParseNum(m_pLocaleMgr, wsValue, &fNum);
        if (bRet)
          m_wsValue = std::move(fNum);
        break;
      }
      case CFGAS_StringFormatter::Category::kText:
        bRet = pFormat->ParseText(wsValue, &m_wsValue);
        break;
      case CFGAS_StringFormatter::Category::kDate: {
        CFX_DateTime dt;
        bRet = ValidateCanonicalDate(wsValue, &dt);
        if (!bRet) {
          bRet = pFormat->ParseDateTime(
              m_pLocaleMgr, wsValue, CFGAS_StringFormatter::DateTimeType::kDate,
              &dt);
        }
        if (bRet)
          SetDate(dt);
        break;
      }
      case CFGAS_StringFormatter::Category::kTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            m_pLocaleMgr, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
            &dt);
        if (bRet)
          SetTime(dt);
        break;
      }
      case CFGAS_StringFormatter::Category::kDateTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            m_pLocaleMgr, wsValue,
            CFGAS_StringFormatter::DateTimeType::kDateTime, &dt);
        if (bRet)
          SetDateTime(dt);
        break;
      }
      default:
        m_wsValue = wsValue;
        bRet = true;
        break;
    }
  }
  if (!bRet)
    m_wsValue = wsValue;

  return bRet;
}

void CXFA_LocaleValue::GetNumericFormat(WideString& wsFormat,
                                        int32_t nIntLen,
                                        int32_t nDecLen) {
  DCHECK(wsFormat.IsEmpty());
  DCHECK(nIntLen >= -1);
  DCHECK(nDecLen >= -1);

  int32_t nTotalLen = (nIntLen >= 0 ? nIntLen : 2) + 1 +
                      (nDecLen >= 0 ? nDecLen : 2) + (nDecLen == 0 ? 0 : 1);
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<wchar_t> lpBuf = wsFormat.GetBuffer(nTotalLen);
    int32_t nPos = 0;
    lpBuf[nPos++] = L's';

    if (nIntLen == -1) {
      lpBuf[nPos++] = L'z';
      lpBuf[nPos++] = L'*';
    } else {
      while (nIntLen) {
        lpBuf[nPos++] = L'z';
        nIntLen--;
      }
    }
    if (nDecLen != 0) {
      lpBuf[nPos++] = L'.';
    }
    if (nDecLen == -1) {
      lpBuf[nPos++] = L'z';
      lpBuf[nPos++] = L'*';
    } else {
      while (nDecLen) {
        lpBuf[nPos++] = L'z';
        nDecLen--;
      }
    }
  }
  wsFormat.ReleaseBuffer(nTotalLen);
}

bool CXFA_LocaleValue::ValidateNumericTemp(const WideString& wsNumeric,
                                           const WideString& wsFormat,
                                           GCedLocaleIface* pLocale) {
  if (wsFormat.IsEmpty() || wsNumeric.IsEmpty())
    return true;

  pdfium::span<const wchar_t> spNum = wsNumeric.span();
  pdfium::span<const wchar_t> spFmt = wsFormat.span();
  size_t n = 0;
  size_t nf = 0;
  wchar_t c = spNum[n];
  wchar_t cf = spFmt[nf];
  if (cf == L's') {
    if (c == L'-' || c == L'+')
      ++n;
    ++nf;
  }

  bool bLimit = true;
  size_t nCount = wsNumeric.GetLength();
  size_t nCountFmt = wsFormat.GetLength();
  while (n < nCount && (!bLimit || nf < nCountFmt) &&
         FXSYS_IsDecimalDigit(c = spNum[n])) {
    if (bLimit) {
      if ((cf = spFmt[nf]) == L'*')
        bLimit = false;
      else if (cf == L'z')
        nf++;
      else
        return false;
    }
    n++;
  }
  if (n == nCount)
    return true;
  if (nf == nCountFmt)
    return false;

  while (nf < nCountFmt && (cf = spFmt[nf]) != L'.') {
    DCHECK(cf == L'z' || cf == L'*');
    ++nf;
  }

  WideString wsDecimalSymbol;
  if (pLocale)
    wsDecimalSymbol = pLocale->GetDecimalSymbol();
  else
    wsDecimalSymbol = WideString(L'.');

  if (spFmt[nf] != L'.')
    return false;
  if (wsDecimalSymbol != WideStringView(c) && c != L'.')
    return false;

  ++nf;
  ++n;
  bLimit = true;
  while (n < nCount && (!bLimit || nf < nCountFmt) &&
         FXSYS_IsDecimalDigit(spNum[n])) {
    if (bLimit) {
      if ((cf = spFmt[nf]) == L'*')
        bLimit = false;
      else if (cf == L'z')
        nf++;
      else
        return false;
    }
    n++;
  }
  return n == nCount;
}
