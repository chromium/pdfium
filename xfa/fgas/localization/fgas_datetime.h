// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LOCALIZATION_FGAS_DATETIME_H_
#define XFA_FGAS_LOCALIZATION_FGAS_DATETIME_H_

#include "core/fxcrt/fx_system.h"

class CFX_Unitime;
class CFX_DateTime;

bool FX_IsLeapYear(int32_t iYear);
uint8_t FX_DaysInMonth(int32_t iYear, uint8_t iMonth);

class CFX_Unitime {
 public:
  CFX_Unitime() { m_iUnitime = 0; }
  explicit CFX_Unitime(int64_t iUnitime) { m_iUnitime = iUnitime; }

  int64_t ToInt64() const { return m_iUnitime; }

  void Now();
  void Set(int32_t year,
           uint8_t month,
           uint8_t day,
           uint8_t hour,
           uint8_t minute,
           uint8_t second,
           uint16_t millisecond);
  void Set(int64_t t) { m_iUnitime = t; }

  int32_t GetYear() const;
  uint8_t GetMonth() const;
  uint8_t GetDay() const;
  uint8_t GetHour() const;
  uint8_t GetMinute() const;
  uint8_t GetSecond() const;
  uint16_t GetMillisecond() const;

  CFX_Unitime operator+(const CFX_Unitime& t2) const {
    return CFX_Unitime(m_iUnitime + t2.m_iUnitime);
  }

 private:
  int64_t m_iUnitime;
};

#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
struct FX_TIMEZONE {
  int8_t tzHour;
  uint8_t tzMinute;
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif

class CFX_DateTime {
 public:
  CFX_DateTime() {}
  ~CFX_DateTime() {}

  bool Set(int32_t year, uint8_t month, uint8_t day);
  int32_t GetDayOfWeek() const;

 private:
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
  struct FX_DATE {
    int32_t year;
    uint8_t month;
    uint8_t day;
  };

  struct FX_TIME {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
  };

  struct FX_DATETIME {
    union {
      FX_DATE sDate;
      FX_DATE aDate;
    } Date;
    union {
      FX_TIME sTime;
      FX_TIME aTime;
    } Time;
  };

#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif
  FX_DATETIME m_DateTime;
};

#endif  // XFA_FGAS_LOCALIZATION_FGAS_DATETIME_H_
