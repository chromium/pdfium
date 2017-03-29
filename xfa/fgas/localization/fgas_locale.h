// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LOCALIZATION_FGAS_LOCALE_H_
#define XFA_FGAS_LOCALIZATION_FGAS_LOCALE_H_

#include <memory>

#include "core/fxcrt/fx_xml.h"
#include "xfa/fgas/localization/fgas_datetime.h"

class CFX_Unitime;

enum FX_LOCALENUMSYMBOL {
  FX_LOCALENUMSYMBOL_Decimal,
  FX_LOCALENUMSYMBOL_Grouping,
  FX_LOCALENUMSYMBOL_Percent,
  FX_LOCALENUMSYMBOL_Minus,
  FX_LOCALENUMSYMBOL_Zero,
  FX_LOCALENUMSYMBOL_CurrencySymbol,
  FX_LOCALENUMSYMBOL_CurrencyName,
};
enum FX_LOCALEDATETIMESUBCATEGORY {
  FX_LOCALEDATETIMESUBCATEGORY_Default,
  FX_LOCALEDATETIMESUBCATEGORY_Short,
  FX_LOCALEDATETIMESUBCATEGORY_Medium,
  FX_LOCALEDATETIMESUBCATEGORY_Full,
  FX_LOCALEDATETIMESUBCATEGORY_Long,
};
enum FX_LOCALENUMSUBCATEGORY {
  FX_LOCALENUMPATTERN_Percent,
  FX_LOCALENUMPATTERN_Currency,
  FX_LOCALENUMPATTERN_Decimal,
  FX_LOCALENUMPATTERN_Integer,
};
enum FX_LOCALECATEGORY {
  FX_LOCALECATEGORY_Unknown,
  FX_LOCALECATEGORY_Date,
  FX_LOCALECATEGORY_Time,
  FX_LOCALECATEGORY_DateTime,
  FX_LOCALECATEGORY_Num,
  FX_LOCALECATEGORY_Text,
  FX_LOCALECATEGORY_Zero,
  FX_LOCALECATEGORY_Null,
};
enum FX_DATETIMETYPE {
  FX_DATETIMETYPE_Unknown,
  FX_DATETIMETYPE_Date,
  FX_DATETIMETYPE_Time,
  FX_DATETIMETYPE_DateTime,
  FX_DATETIMETYPE_TimeDate,
};

class IFX_Locale {
 public:
  virtual ~IFX_Locale() {}

  virtual CFX_WideString GetName() const = 0;
  virtual CFX_WideString GetNumbericSymbol(FX_LOCALENUMSYMBOL eType) const = 0;
  virtual CFX_WideString GetDateTimeSymbols() const = 0;
  virtual CFX_WideString GetMonthName(int32_t nMonth, bool bAbbr) const = 0;
  virtual CFX_WideString GetDayName(int32_t nWeek, bool bAbbr) const = 0;
  virtual CFX_WideString GetMeridiemName(bool bAM) const = 0;
  virtual FX_TIMEZONE GetTimeZone() const = 0;
  virtual CFX_WideString GetEraName(bool bAD) const = 0;
  virtual CFX_WideString GetDatePattern(
      FX_LOCALEDATETIMESUBCATEGORY eType) const = 0;
  virtual CFX_WideString GetTimePattern(
      FX_LOCALEDATETIMESUBCATEGORY eType) const = 0;
  virtual CFX_WideString GetNumPattern(FX_LOCALENUMSUBCATEGORY eType) const = 0;
};

bool FX_DateFromCanonical(const CFX_WideString& wsDate, CFX_Unitime* datetime);
bool FX_TimeFromCanonical(const CFX_WideStringC& wsTime,
                          CFX_Unitime* datetime,
                          IFX_Locale* pLocale);
class CFX_Decimal {
 public:
  CFX_Decimal();
  explicit CFX_Decimal(uint32_t val);
  explicit CFX_Decimal(uint64_t val);
  explicit CFX_Decimal(int32_t val);
  explicit CFX_Decimal(float val, uint8_t scale);
  explicit CFX_Decimal(const CFX_WideStringC& str);

  operator CFX_WideString() const;
  operator double() const;

  CFX_Decimal operator*(const CFX_Decimal& val) const;
  CFX_Decimal operator/(const CFX_Decimal& val) const;

  void SetScale(uint8_t newScale);
  uint8_t GetScale();
  void SetNegate();

 private:
  CFX_Decimal(uint32_t hi, uint32_t mid, uint32_t lo, bool neg, uint8_t scale);
  bool IsNotZero() const { return m_uHi || m_uMid || m_uLo; }
  void Swap(CFX_Decimal& val);

  uint32_t m_uHi;
  uint32_t m_uLo;
  uint32_t m_uMid;
  uint32_t m_uFlags;
};

#endif  // XFA_FGAS_LOCALIZATION_FGAS_LOCALE_H_
