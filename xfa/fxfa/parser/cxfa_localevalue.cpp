// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_localevalue.h"

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/span.h"
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
      : locale_mgr_(pLocaleMgr),
        new_locale_(pNewLocale),
        orig_locale_(pNewLocale ? locale_mgr_->GetDefLocale() : nullptr) {
    if (new_locale_) {
      locale_mgr_->SetDefLocale(pNewLocale);
    }
  }

  ~ScopedLocale() {
    if (new_locale_) {
      locale_mgr_->SetDefLocale(orig_locale_);
    }
  }

  ScopedLocale(const ScopedLocale& that) = delete;
  ScopedLocale& operator=(const ScopedLocale& that) = delete;

 private:
  UnownedPtr<CXFA_LocaleMgr> const locale_mgr_;    // Ok, stack-only.
  UnownedPtr<GCedLocaleIface> const new_locale_;   // Ok, stack-only.
  UnownedPtr<GCedLocaleIface> const orig_locale_;  // Ok, stack-only.
};

}  // namespace

CXFA_LocaleValue::CXFA_LocaleValue() = default;

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType, CXFA_LocaleMgr* pLocaleMgr)
    : locale_mgr_(pLocaleMgr),
      type_(eType),
      valid_(type_ != ValueType::kNull) {}

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType,
                                   const WideString& wsValue,
                                   CXFA_LocaleMgr* pLocaleMgr)
    : locale_mgr_(pLocaleMgr),
      value_(wsValue),
      type_(eType),
      valid_(ValidateCanonicalValue(wsValue, eType)) {}

CXFA_LocaleValue::CXFA_LocaleValue(ValueType eType,
                                   const WideString& wsValue,
                                   const WideString& wsFormat,
                                   GCedLocaleIface* pLocale,
                                   CXFA_LocaleMgr* pLocaleMgr)
    : locale_mgr_(pLocaleMgr),
      type_(eType),
      valid_(ParsePatternValue(wsValue, wsFormat, pLocale)) {}

CXFA_LocaleValue::CXFA_LocaleValue(const CXFA_LocaleValue& that) = default;

CXFA_LocaleValue& CXFA_LocaleValue::operator=(const CXFA_LocaleValue& that) =
    default;

CXFA_LocaleValue::~CXFA_LocaleValue() = default;

bool CXFA_LocaleValue::ValidateValue(const WideString& wsValue,
                                     const WideString& wsPattern,
                                     GCedLocaleIface* pLocale,
                                     WideString* pMatchFormat) {
  if (!locale_mgr_) {
    return false;
  }

  ScopedLocale scoped_locale(locale_mgr_, pLocale);
  std::vector<WideString> wsPatterns =
      CFGAS_StringFormatter::SplitOnBars(wsPattern);

  WideString wsOutput;
  bool bRet = false;
  size_t i = 0;
  for (; !bRet && i < wsPatterns.size(); i++) {
    const WideString& wsFormat = wsPatterns[i];
    auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
    switch (ValueCategory(pFormat->GetCategory(), type_)) {
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
        bRet = pFormat->ParseNum(locale_mgr_, wsValue, &fNum);
        if (!bRet)
          bRet = pFormat->FormatNum(locale_mgr_, wsValue, &wsOutput);
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
              locale_mgr_, wsValue, CFGAS_StringFormatter::DateTimeType::kDate,
              &dt);
          if (!bRet) {
            bRet = pFormat->FormatDateTime(
                locale_mgr_, wsValue,
                CFGAS_StringFormatter::DateTimeType::kDate, &wsOutput);
          }
        }
        break;
      }
      case CFGAS_StringFormatter::Category::kTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            locale_mgr_, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
            &dt);
        if (!bRet) {
          bRet = pFormat->FormatDateTime(
              locale_mgr_, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
              &wsOutput);
        }
        break;
      }
      case CFGAS_StringFormatter::Category::kDateTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            locale_mgr_, wsValue,
            CFGAS_StringFormatter::DateTimeType::kDateTime, &dt);
        if (!bRet) {
          bRet = pFormat->FormatDateTime(
              locale_mgr_, wsValue,
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
  if (!valid_ || (type_ != CXFA_LocaleValue::ValueType::kBoolean &&
                  type_ != CXFA_LocaleValue::ValueType::kInteger &&
                  type_ != CXFA_LocaleValue::ValueType::kDecimal &&
                  type_ != CXFA_LocaleValue::ValueType::kFloat)) {
    return 0;
  }

  return StringToDouble(value_.AsStringView());
}

CFX_DateTime CXFA_LocaleValue::GetDate() const {
  if (!valid_ || type_ != CXFA_LocaleValue::ValueType::kDate) {
    return CFX_DateTime();
  }

  CFX_DateTime dt;
  FX_DateFromCanonical(value_.span(), &dt);
  return dt;
}

CFX_DateTime CXFA_LocaleValue::GetTime() const {
  if (!valid_ || type_ != CXFA_LocaleValue::ValueType::kTime) {
    return CFX_DateTime();
  }

  CFX_DateTime dt;
  FX_TimeFromCanonical(locale_mgr_->GetDefLocale(), value_.span(), &dt);
  return dt;
}

void CXFA_LocaleValue::SetDate(const CFX_DateTime& d) {
  type_ = CXFA_LocaleValue::ValueType::kDate;
  value_ = WideString::Format(L"%04d-%02d-%02d", d.GetYear(), d.GetMonth(),
                              d.GetDay());
}

void CXFA_LocaleValue::SetTime(const CFX_DateTime& t) {
  type_ = CXFA_LocaleValue::ValueType::kTime;
  value_ = WideString::Format(L"%02d:%02d:%02d", t.GetHour(), t.GetMinute(),
                              t.GetSecond());
  if (t.GetMillisecond() > 0)
    value_ += WideString::Format(L"%:03d", t.GetMillisecond());
}

void CXFA_LocaleValue::SetDateTime(const CFX_DateTime& dt) {
  type_ = CXFA_LocaleValue::ValueType::kDateTime;
  value_ = WideString::Format(L"%04d-%02d-%02dT%02d:%02d:%02d", dt.GetYear(),
                              dt.GetMonth(), dt.GetDay(), dt.GetHour(),
                              dt.GetMinute(), dt.GetSecond());
  if (dt.GetMillisecond() > 0)
    value_ += WideString::Format(L"%:03d", dt.GetMillisecond());
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
  if (!locale_mgr_) {
    return false;
  }

  ScopedLocale scoped_locale(locale_mgr_, pLocale);
  wsResult.clear();

  bool bRet = false;
  auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
  CFGAS_StringFormatter::Category eCategory =
      ValueCategory(pFormat->GetCategory(), type_);
  switch (eCategory) {
    case CFGAS_StringFormatter::Category::kNull:
      if (value_.IsEmpty()) {
        bRet = pFormat->FormatNull(&wsResult);
      }
      break;
    case CFGAS_StringFormatter::Category::kZero:
      if (value_.EqualsASCII("0")) {
        bRet = pFormat->FormatZero(&wsResult);
      }
      break;
    case CFGAS_StringFormatter::Category::kNum:
      bRet = pFormat->FormatNum(locale_mgr_, value_, &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kText:
      bRet = pFormat->FormatText(value_, &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kDate:
      bRet = pFormat->FormatDateTime(locale_mgr_, value_,
                                     CFGAS_StringFormatter::DateTimeType::kDate,
                                     &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kTime:
      bRet = pFormat->FormatDateTime(locale_mgr_, value_,
                                     CFGAS_StringFormatter::DateTimeType::kTime,
                                     &wsResult);
      break;
    case CFGAS_StringFormatter::Category::kDateTime:
      bRet = pFormat->FormatDateTime(
          locale_mgr_, value_, CFGAS_StringFormatter::DateTimeType::kDateTime,
          &wsResult);
      break;
    default:
      wsResult = value_;
      bRet = true;
  }
  if (!bRet && (eCategory != CFGAS_StringFormatter::Category::kNum ||
                eValueType != XFA_ValuePicture::kDisplay)) {
    wsResult = value_;
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
  static const std::array<const uint8_t, 12> LastDay = {
      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
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
  if (!locale_mgr_) {
    return false;
  }

  std::vector<WideString> wsPatterns =
      CFGAS_StringFormatter::SplitOnBars(wsPattern);

  ScopedLocale scoped_locale(locale_mgr_, pLocale);
  bool bRet = false;
  for (size_t i = 0; !bRet && i < wsPatterns.size(); i++) {
    const WideString& wsFormat = wsPatterns[i];
    auto pFormat = std::make_unique<CFGAS_StringFormatter>(wsFormat);
    switch (ValueCategory(pFormat->GetCategory(), type_)) {
      case CFGAS_StringFormatter::Category::kNull:
        bRet = pFormat->ParseNull(wsValue);
        if (bRet)
          value_.clear();
        break;
      case CFGAS_StringFormatter::Category::kZero:
        bRet = pFormat->ParseZero(wsValue);
        if (bRet)
          value_ = L"0";
        break;
      case CFGAS_StringFormatter::Category::kNum: {
        WideString fNum;
        bRet = pFormat->ParseNum(locale_mgr_, wsValue, &fNum);
        if (bRet)
          value_ = std::move(fNum);
        break;
      }
      case CFGAS_StringFormatter::Category::kText:
        bRet = pFormat->ParseText(wsValue, &value_);
        break;
      case CFGAS_StringFormatter::Category::kDate: {
        CFX_DateTime dt;
        bRet = ValidateCanonicalDate(wsValue, &dt);
        if (!bRet) {
          bRet = pFormat->ParseDateTime(
              locale_mgr_, wsValue, CFGAS_StringFormatter::DateTimeType::kDate,
              &dt);
        }
        if (bRet)
          SetDate(dt);
        break;
      }
      case CFGAS_StringFormatter::Category::kTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            locale_mgr_, wsValue, CFGAS_StringFormatter::DateTimeType::kTime,
            &dt);
        if (bRet)
          SetTime(dt);
        break;
      }
      case CFGAS_StringFormatter::Category::kDateTime: {
        CFX_DateTime dt;
        bRet = pFormat->ParseDateTime(
            locale_mgr_, wsValue,
            CFGAS_StringFormatter::DateTimeType::kDateTime, &dt);
        if (bRet)
          SetDateTime(dt);
        break;
      }
      default:
        value_ = wsValue;
        bRet = true;
        break;
    }
  }
  if (!bRet)
    value_ = wsValue;

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
