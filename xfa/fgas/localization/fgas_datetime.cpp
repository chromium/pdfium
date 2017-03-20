// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/localization/fgas_datetime.h"

#if _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_ANDROID_ || \
    _FX_OS_ == _FX_MACOSX_ || _FX_OS_ == _FX_IOS_
#include <sys/time.h>
#include <time.h>
#endif

namespace {

const uint8_t g_FXDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
const uint8_t g_FXDaysPerLeapMonth[12] = {31, 29, 31, 30, 31, 30,
                                          31, 31, 30, 31, 30, 31};
const int32_t g_FXDaysBeforeMonth[12] = {0,   31,  59,  90,  120, 151,
                                         181, 212, 243, 273, 304, 334};
const int32_t g_FXDaysBeforeLeapMonth[12] = {0,   31,  60,  91,  121, 152,
                                             182, 213, 244, 274, 305, 335};
const int32_t g_FXDaysPerYear = 365;
const int32_t g_FXDaysPerLeapYear = 366;
const int32_t g_FXDaysPer4Years = 1461;
const int32_t g_FXDaysPer100Years = 36524;
const int32_t g_FXDaysPer400Years = 146097;
const int64_t g_FXMillisecondsPerSecond = 1000;
const int64_t g_FXMillisecondsPerMinute = 60000;
const int64_t g_FXMillisecondsPerHour = 3600000;
const int64_t g_FXMillisecondsPerDay = 86400000;

int64_t GetDayOfAD(int64_t unitime) {
  bool bBC = unitime < 0;
  int64_t iDays = unitime / g_FXMillisecondsPerDay;
  iDays += bBC ? -1 : 0;
  if (bBC && (unitime % g_FXMillisecondsPerDay) == 0)
    iDays++;
  return iDays;
}

int32_t DaysBeforeMonthInYear(int32_t iYear, uint8_t iMonth) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);

  const int32_t* p =
      FX_IsLeapYear(iYear) ? g_FXDaysBeforeLeapMonth : g_FXDaysBeforeMonth;
  return p[iMonth - 1];
}

int32_t DaysInYear(int32_t iYear) {
  ASSERT(iYear != 0);
  return FX_IsLeapYear(iYear) ? g_FXDaysPerLeapYear : g_FXDaysPerYear;
}

int64_t DateToDays(int32_t iYear,
                   uint8_t iMonth,
                   uint8_t iDay,
                   bool bIncludeThisDay) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);
  ASSERT(iDay >= 1 && iDay <= FX_DaysInMonth(iYear, iMonth));

  int64_t iDays = DaysBeforeMonthInYear(iYear, iMonth);
  iDays += iDay;
  if (!bIncludeThisDay)
    iDays--;

  if (iYear > 0) {
    iYear--;
  } else {
    iDays -= DaysInYear(iYear);
    iYear++;
  }
  return iDays + static_cast<int64_t>(iYear) * 365 + iYear / 4 - iYear / 100 +
         iYear / 400;
}

void DaysToDate(int64_t iDays,
                int32_t* retYear,
                uint8_t* retMonth,
                uint8_t* retDay) {
  bool bBC = iDays < 0;
  if (bBC)
    iDays = -iDays;

  int32_t iYear = 1;
  uint8_t iMonth = 1;
  uint8_t iDay = 1;
  if (iDays >= g_FXDaysPer400Years) {
    iYear += static_cast<int32_t>(iDays / g_FXDaysPer400Years * 400);
    iDays %= g_FXDaysPer400Years;
  }

  if (iDays >= g_FXDaysPer100Years) {
    if (iDays == g_FXDaysPer100Years * 4) {
      iYear += 300;
      iDays -= g_FXDaysPer100Years * 3;
    } else {
      iYear += static_cast<int32_t>(iDays / g_FXDaysPer100Years * 100);
      iDays %= g_FXDaysPer100Years;
    }
  }

  if (iDays >= g_FXDaysPer4Years) {
    iYear += static_cast<int32_t>(iDays / g_FXDaysPer4Years * 4);
    iDays %= g_FXDaysPer4Years;
  }

  while (true) {
    int32_t iYearDays = DaysInYear(iYear);
    if (iDays < iYearDays) {
      if (bBC) {
        iYear = -iYear;
        iDays = iYearDays - iDays;
      }
      break;
    }
    iYear++;
    iDays -= iYearDays;
  }
  while (true) {
    int32_t iMonthDays = FX_DaysInMonth(iYear, iMonth);
    if (iDays < iMonthDays)
      break;

    iMonth++;
    iDays -= iMonthDays;
  }
  iDay += static_cast<uint8_t>(iDays);

  *retYear = iYear;
  *retMonth = iMonth;
  *retDay = iDay;
}

struct FXUT_SYSTEMTIME {
  uint16_t wYear;
  uint16_t wMonth;
  uint16_t wDayOfWeek;
  uint16_t wDay;
  uint16_t wHour;
  uint16_t wMinute;
  uint16_t wSecond;
  uint16_t wMilliseconds;
};

}  // namespace

uint8_t FX_DaysInMonth(int32_t iYear, uint8_t iMonth) {
  ASSERT(iYear != 0);
  ASSERT(iMonth >= 1 && iMonth <= 12);

  const uint8_t* p =
      FX_IsLeapYear(iYear) ? g_FXDaysPerLeapMonth : g_FXDaysPerMonth;
  return p[iMonth - 1];
}

bool FX_IsLeapYear(int32_t iYear) {
  ASSERT(iYear != 0);
  return ((iYear % 4) == 0 && (iYear % 100) != 0) || (iYear % 400) == 0;
}

void CFX_Unitime::Now() {
  FXUT_SYSTEMTIME utLocal;
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
  ::GetLocalTime((LPSYSTEMTIME)&utLocal);
#elif _FX_OS_ != _FX_EMBEDDED_
  timeval curTime;
  gettimeofday(&curTime, nullptr);

  struct tm st;
  localtime_r(&curTime.tv_sec, &st);
  utLocal.wYear = st.tm_year + 1900;
  utLocal.wMonth = st.tm_mon + 1;
  utLocal.wDayOfWeek = st.tm_wday;
  utLocal.wDay = st.tm_mday;
  utLocal.wHour = st.tm_hour;
  utLocal.wMinute = st.tm_min;
  utLocal.wSecond = st.tm_sec;
  utLocal.wMilliseconds = curTime.tv_usec / 1000;
#endif  // _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
        // _FX_OS_ == _FX_WIN64_
  Set(utLocal.wYear, static_cast<uint8_t>(utLocal.wMonth),
      static_cast<uint8_t>(utLocal.wDay), static_cast<uint8_t>(utLocal.wHour),
      static_cast<uint8_t>(utLocal.wMinute),
      static_cast<uint8_t>(utLocal.wSecond),
      static_cast<uint16_t>(utLocal.wMilliseconds));
}

void CFX_Unitime::Set(int32_t year,
                      uint8_t month,
                      uint8_t day,
                      uint8_t hour,
                      uint8_t minute,
                      uint8_t second,
                      uint16_t millisecond) {
  ASSERT(hour <= 23);
  ASSERT(minute <= 59);
  ASSERT(second <= 59);
  ASSERT(millisecond <= 999);

  m_iUnitime = static_cast<int64_t>(hour) * g_FXMillisecondsPerHour +
               static_cast<int64_t>(minute) * g_FXMillisecondsPerMinute +
               static_cast<int64_t>(second) * g_FXMillisecondsPerSecond +
               millisecond;
  if (year <= 0)
    return;

  m_iUnitime += DateToDays(year, month, day, false) * g_FXMillisecondsPerDay;
}

int32_t CFX_Unitime::GetYear() const {
  int32_t iYear;
  uint8_t iMonth, iDay;
  DaysToDate(GetDayOfAD(m_iUnitime), &iYear, &iMonth, &iDay);
  return iYear;
}

uint8_t CFX_Unitime::GetMonth() const {
  int32_t iYear;
  uint8_t iMonth, iDay;
  DaysToDate(GetDayOfAD(m_iUnitime), &iYear, &iMonth, &iDay);
  return iMonth;
}

uint8_t CFX_Unitime::GetDay() const {
  int32_t iYear;
  uint8_t iMonth, iDay;
  DaysToDate(GetDayOfAD(m_iUnitime), &iYear, &iMonth, &iDay);
  return iDay;
}

uint8_t CFX_Unitime::GetHour() const {
  int32_t v = static_cast<int32_t>(m_iUnitime % g_FXMillisecondsPerDay);
  if (v < 0)
    v += g_FXMillisecondsPerDay;
  return static_cast<uint8_t>(v / g_FXMillisecondsPerHour);
}

uint8_t CFX_Unitime::GetMinute() const {
  int32_t v = static_cast<int32_t>(m_iUnitime % g_FXMillisecondsPerHour);
  if (v < 0)
    v += g_FXMillisecondsPerHour;
  return static_cast<uint8_t>(v / g_FXMillisecondsPerMinute);
}

uint8_t CFX_Unitime::GetSecond() const {
  int32_t v = static_cast<int32_t>(m_iUnitime % g_FXMillisecondsPerMinute);
  if (v < 0)
    v += g_FXMillisecondsPerMinute;
  return static_cast<uint8_t>(v / g_FXMillisecondsPerSecond);
}

uint16_t CFX_Unitime::GetMillisecond() const {
  int32_t v = static_cast<int32_t>(m_iUnitime % g_FXMillisecondsPerSecond);
  if (v < 0)
    v += g_FXMillisecondsPerSecond;
  return static_cast<uint16_t>(v);
}

bool CFX_DateTime::Set(int32_t year, uint8_t month, uint8_t day) {
  ASSERT(year != 0);
  ASSERT(month >= 1 && month <= 12);
  ASSERT(day >= 1 && day <= FX_DaysInMonth(year, month));

  m_DateTime.Date.sDate.year = year;
  m_DateTime.Date.sDate.month = month;
  m_DateTime.Date.sDate.day = day;
  m_DateTime.Time.sTime.hour = 0;
  m_DateTime.Time.sTime.minute = 0;
  m_DateTime.Time.sTime.second = 0;
  m_DateTime.Time.sTime.millisecond = 0;
  return true;
}

int32_t CFX_DateTime::GetDayOfWeek() const {
  int32_t v = static_cast<int32_t>(DateToDays(m_DateTime.Date.sDate.year,
                                              m_DateTime.Date.sDate.month,
                                              m_DateTime.Date.sDate.day, true) %
                                   7);
  if (v < 0)
    v += 7;
  return v;
}
