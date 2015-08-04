// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_LOCALE_H_
#define _FX_LOCALE_H_
class CFX_Unitime;
class IFX_Locale;
class IFX_FormatString;
class IFX_LocaleMgr;
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
  static IFX_Locale* Create(CXML_Element* pLocaleData);

  virtual ~IFX_Locale() {}
  virtual void Release() = 0;

  virtual CFX_WideString GetName() = 0;

  virtual void GetNumbericSymbol(FX_LOCALENUMSYMBOL eType,
                                 CFX_WideString& wsNumSymbol) const = 0;
  virtual void GetDateTimeSymbols(CFX_WideString& wsDtSymbol) const = 0;
  virtual void GetMonthName(int32_t nMonth,
                            CFX_WideString& wsMonthName,
                            FX_BOOL bAbbr = TRUE) const = 0;
  virtual void GetDayName(int32_t nWeek,
                          CFX_WideString& wsDayName,
                          FX_BOOL bAbbr = TRUE) const = 0;
  virtual void GetMeridiemName(CFX_WideString& wsMeridiemName,
                               FX_BOOL bAM = TRUE) const = 0;
  virtual void GetTimeZone(FX_TIMEZONE& tz) const = 0;
  virtual void GetEraName(CFX_WideString& wsEraName,
                          FX_BOOL bAD = TRUE) const = 0;
  virtual void GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                              CFX_WideString& wsPattern) const = 0;
  virtual void GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY eType,
                              CFX_WideString& wsPattern) const = 0;
  virtual void GetNumPattern(FX_LOCALENUMSUBCATEGORY eType,
                             CFX_WideString& wsPattern) const = 0;
};

class IFX_LocaleMgr {
 public:
  virtual ~IFX_LocaleMgr() {}
  virtual void Release() = 0;
  virtual FX_WORD GetDefLocaleID() = 0;
  virtual IFX_Locale* GetDefLocale() = 0;
  virtual IFX_Locale* GetLocale(FX_WORD lcid) = 0;
  virtual IFX_Locale* GetLocaleByName(const CFX_WideStringC& wsLocaleName) = 0;
};
IFX_LocaleMgr* FX_LocaleMgr_Create(const FX_WCHAR* pszLocalPath,
                                   FX_WORD wDefaultLCID);
void FX_ParseNumString(const CFX_WideString& wsNum, CFX_WideString& wsResult);
FX_BOOL FX_DateFromCanonical(const CFX_WideString& wsDate,
                             CFX_Unitime& datetime);
FX_BOOL FX_TimeFromCanonical(const CFX_WideStringC& wsTime,
                             CFX_Unitime& datetime,
                             IFX_Locale* pLocale);
class IFX_FormatString {
 public:
  static IFX_FormatString* Create(IFX_LocaleMgr* pLocaleMgr, FX_BOOL bUseLCID);

  virtual ~IFX_FormatString() {}
  virtual void Release() = 0;
  virtual void SplitFormatString(const CFX_WideString& wsFormatString,
                                 CFX_WideStringArray& wsPatterns) = 0;
  virtual FX_LOCALECATEGORY GetCategory(const CFX_WideString& wsPattern) = 0;
  virtual FX_WORD GetLCID(const CFX_WideString& wsPattern) = 0;
  virtual CFX_WideString GetLocaleName(const CFX_WideString& wsPattern) = 0;
  virtual FX_BOOL ParseText(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsValue) = 0;
  virtual FX_BOOL ParseNum(const CFX_WideString& wsSrcNum,
                           const CFX_WideString& wsPattern,
                           FX_FLOAT& fValue) = 0;
  virtual FX_BOOL ParseNum(const CFX_WideString& wsSrcNum,
                           const CFX_WideString& wsPattern,
                           CFX_WideString& wsValue) = 0;
  virtual FX_BOOL ParseDateTime(const CFX_WideString& wsSrcDateTime,
                                const CFX_WideString& wsPattern,
                                FX_DATETIMETYPE eDateTimeType,
                                CFX_Unitime& dtValue) = 0;
  virtual FX_BOOL ParseZero(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern) = 0;
  virtual FX_BOOL ParseNull(const CFX_WideString& wsSrcText,
                            const CFX_WideString& wsPattern) = 0;
  virtual FX_BOOL FormatText(const CFX_WideString& wsSrcText,
                             const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatNum(const CFX_WideString& wsSrcNum,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatNum(FX_FLOAT fNum,
                            const CFX_WideString& wsPattern,
                            CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatDateTime(const CFX_WideString& wsSrcDateTime,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatDateTime(const CFX_WideString& wsSrcDateTime,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput,
                                 FX_DATETIMETYPE eDateTimeType) = 0;
  virtual FX_BOOL FormatDateTime(const CFX_Unitime& dt,
                                 const CFX_WideString& wsPattern,
                                 CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatZero(const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput) = 0;
  virtual FX_BOOL FormatNull(const CFX_WideString& wsPattern,
                             CFX_WideString& wsOutput) = 0;
};
class CFX_Decimal {
 public:
  CFX_Decimal();
  CFX_Decimal(uint32_t val);
  CFX_Decimal(uint64_t val);
  CFX_Decimal(int32_t val);
  CFX_Decimal(int64_t val);
  CFX_Decimal(FX_FLOAT val, uint8_t scale = 3);
  CFX_Decimal(const CFX_WideStringC& str);
  CFX_Decimal(const CFX_ByteStringC& str);
  operator CFX_WideString() const;
  operator double() const;
  FX_BOOL operator==(const CFX_Decimal& val) const;
  FX_BOOL operator<=(const CFX_Decimal& val) const;
  FX_BOOL operator>=(const CFX_Decimal& val) const;
  FX_BOOL operator!=(const CFX_Decimal& val) const;
  FX_BOOL operator<(const CFX_Decimal& val) const;
  FX_BOOL operator>(const CFX_Decimal& val) const;
  CFX_Decimal operator+(const CFX_Decimal& val) const;
  CFX_Decimal operator-(const CFX_Decimal& val) const;
  CFX_Decimal operator*(const CFX_Decimal& val) const;
  CFX_Decimal operator/(const CFX_Decimal& val) const;
  CFX_Decimal operator%(const CFX_Decimal& val) const;
  void SetScale(uint8_t newScale);
  uint8_t GetScale();
  void SetAbs();
  void SetNegate();
  void SetFloor();
  void SetCeiling();
  void SetTruncate();

 protected:
  CFX_Decimal(uint32_t hi,
              uint32_t mid,
              uint32_t lo,
              FX_BOOL neg,
              uint8_t scale);
  inline FX_BOOL IsNotZero() const { return m_uHi || m_uMid || m_uLo; }
  inline int8_t Compare(const CFX_Decimal& val) const;
  inline void Swap(CFX_Decimal& val);
  inline void FloorOrCeil(FX_BOOL bFloor);
  CFX_Decimal AddOrMinus(const CFX_Decimal& val, FX_BOOL isAdding) const;
  CFX_Decimal Multiply(const CFX_Decimal& val) const;
  CFX_Decimal Divide(const CFX_Decimal& val) const;
  CFX_Decimal Modulus(const CFX_Decimal& val) const;
  uint32_t m_uFlags;
  uint32_t m_uHi;
  uint32_t m_uLo;
  uint32_t m_uMid;
};
#endif
