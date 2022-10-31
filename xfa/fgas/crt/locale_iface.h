// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_LOCALE_IFACE_H_
#define XFA_FGAS_CRT_LOCALE_IFACE_H_

#include "core/fxcrt/widestring.h"

class LocaleIface {
 public:
  enum class DateTimeSubcategory {
    kDefault,
    kShort,
    kMedium,
    kFull,
    kLong,
  };

  enum class NumSubcategory {
    kPercent,
    kCurrency,
    kDecimal,
    kInteger,
  };

  virtual ~LocaleIface() = default;

  virtual WideString GetName() const = 0;
  virtual WideString GetDecimalSymbol() const = 0;
  virtual WideString GetGroupingSymbol() const = 0;
  virtual WideString GetPercentSymbol() const = 0;
  virtual WideString GetMinusSymbol() const = 0;
  virtual WideString GetCurrencySymbol() const = 0;
  virtual WideString GetDateTimeSymbols() const = 0;
  virtual WideString GetMonthName(int32_t nMonth, bool bAbbr) const = 0;
  virtual WideString GetDayName(int32_t nWeek, bool bAbbr) const = 0;
  virtual WideString GetMeridiemName(bool bAM) const = 0;
  virtual int GetTimeZoneInMinutes() const = 0;
  virtual WideString GetEraName(bool bAD) const = 0;
  virtual WideString GetDatePattern(DateTimeSubcategory eType) const = 0;
  virtual WideString GetTimePattern(DateTimeSubcategory eType) const = 0;
  virtual WideString GetNumPattern(NumSubcategory eType) const = 0;
};

#endif  // XFA_FGAS_CRT_LOCALE_IFACE_H_
