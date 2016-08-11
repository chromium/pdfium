// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_datetime.h"

#include "core/fxcrt/include/fx_ext.h"

namespace {

int GetTimeZoneInSeconds(int8_t tzhour, uint8_t tzminute) {
  return (int)tzhour * 3600 + (int)tzminute * (tzhour >= 0 ? 60 : -60);
}

bool IsLeapYear(int16_t year) {
  return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

uint16_t GetYearDays(int16_t year) {
  return (IsLeapYear(year) ? 366 : 365);
}

uint8_t GetMonthDays(int16_t year, uint8_t month) {
  uint8_t mDays;
  switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      mDays = 31;
      break;

    case 4:
    case 6:
    case 9:
    case 11:
      mDays = 30;
      break;

    case 2:
      if (IsLeapYear(year))
        mDays = 29;
      else
        mDays = 28;
      break;

    default:
      mDays = 0;
      break;
  }

  return mDays;
}

}  // namespace

CPDFSDK_DateTime::CPDFSDK_DateTime() {
  ResetDateTime();
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const CFX_ByteString& dtStr) {
  ResetDateTime();

  FromPDFDateTimeString(dtStr);
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const CPDFSDK_DateTime& datetime) {
  FXSYS_memcpy(&dt, &datetime.dt, sizeof(FX_DATETIME));
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const FX_SYSTEMTIME& st) {
  tzset();

  dt.year = static_cast<int16_t>(st.wYear);
  dt.month = static_cast<uint8_t>(st.wMonth);
  dt.day = static_cast<uint8_t>(st.wDay);
  dt.hour = static_cast<uint8_t>(st.wHour);
  dt.minute = static_cast<uint8_t>(st.wMinute);
  dt.second = static_cast<uint8_t>(st.wSecond);
}

void CPDFSDK_DateTime::ResetDateTime() {
  tzset();

  time_t curTime;
  time(&curTime);
  struct tm* newtime = localtime(&curTime);

  dt.year = newtime->tm_year + 1900;
  dt.month = newtime->tm_mon + 1;
  dt.day = newtime->tm_mday;
  dt.hour = newtime->tm_hour;
  dt.minute = newtime->tm_min;
  dt.second = newtime->tm_sec;
}

bool CPDFSDK_DateTime::operator==(const CPDFSDK_DateTime& datetime) const {
  return (FXSYS_memcmp(&dt, &datetime.dt, sizeof(FX_DATETIME)) == 0);
}

bool CPDFSDK_DateTime::operator!=(const CPDFSDK_DateTime& datetime) const {
  return !(*this == datetime);
}

time_t CPDFSDK_DateTime::ToTime_t() const {
  struct tm newtime;

  newtime.tm_year = dt.year - 1900;
  newtime.tm_mon = dt.month - 1;
  newtime.tm_mday = dt.day;
  newtime.tm_hour = dt.hour;
  newtime.tm_min = dt.minute;
  newtime.tm_sec = dt.second;

  return mktime(&newtime);
}

CPDFSDK_DateTime& CPDFSDK_DateTime::FromPDFDateTimeString(
    const CFX_ByteString& dtStr) {
  int strLength = dtStr.GetLength();
  if (strLength <= 0)
    return *this;

  int i = 0;
  while (i < strLength && !std::isdigit(dtStr[i]))
    ++i;

  if (i >= strLength)
    return *this;

  int j = 0;
  int k = 0;
  FX_CHAR ch;
  while (i < strLength && j < 4) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.year = static_cast<int16_t>(k);
  if (i >= strLength || j < 4)
    return *this;

  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.month = static_cast<uint8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.day = static_cast<uint8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.hour = static_cast<uint8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.minute = static_cast<uint8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.second = static_cast<uint8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  ch = dtStr[i++];
  if (ch != '-' && ch != '+')
    return *this;
  if (ch == '-')
    dt.tzHour = -1;
  else
    dt.tzHour = 1;
  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.tzHour *= static_cast<int8_t>(k);
  if (i >= strLength || j < 2)
    return *this;

  if (dtStr[i++] != '\'')
    return *this;
  j = 0;
  k = 0;
  while (i < strLength && j < 2) {
    ch = dtStr[i];
    k = k * 10 + FXSYS_toDecimalDigit(ch);
    j++;
    if (!std::isdigit(ch))
      break;
    i++;
  }
  dt.tzMinute = static_cast<uint8_t>(k);
  return *this;
}

CFX_ByteString CPDFSDK_DateTime::ToCommonDateTimeString() {
  CFX_ByteString str1;
  str1.Format("%04d-%02u-%02u %02u:%02u:%02u ", dt.year, dt.month, dt.day,
              dt.hour, dt.minute, dt.second);
  if (dt.tzHour < 0)
    str1 += "-";
  else
    str1 += "+";
  CFX_ByteString str2;
  str2.Format("%02d:%02u", std::abs(static_cast<int>(dt.tzHour)), dt.tzMinute);
  return str1 + str2;
}

CFX_ByteString CPDFSDK_DateTime::ToPDFDateTimeString() {
  CFX_ByteString dtStr;
  char tempStr[32];
  memset(tempStr, 0, sizeof(tempStr));
  FXSYS_snprintf(tempStr, sizeof(tempStr) - 1, "D:%04d%02u%02u%02u%02u%02u",
                 dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
  dtStr = CFX_ByteString(tempStr);
  if (dt.tzHour < 0)
    dtStr += CFX_ByteString("-");
  else
    dtStr += CFX_ByteString("+");
  memset(tempStr, 0, sizeof(tempStr));
  FXSYS_snprintf(tempStr, sizeof(tempStr) - 1, "%02d'%02u'",
                 std::abs(static_cast<int>(dt.tzHour)), dt.tzMinute);
  dtStr += CFX_ByteString(tempStr);
  return dtStr;
}

void CPDFSDK_DateTime::ToSystemTime(FX_SYSTEMTIME& st) {
  time_t t = this->ToTime_t();
  struct tm* pTime = localtime(&t);

  if (!pTime)
    return;

  st.wYear = static_cast<uint16_t>(pTime->tm_year) + 1900;
  st.wMonth = static_cast<uint16_t>(pTime->tm_mon) + 1;
  st.wDay = static_cast<uint16_t>(pTime->tm_mday);
  st.wDayOfWeek = static_cast<uint16_t>(pTime->tm_wday);
  st.wHour = static_cast<uint16_t>(pTime->tm_hour);
  st.wMinute = static_cast<uint16_t>(pTime->tm_min);
  st.wSecond = static_cast<uint16_t>(pTime->tm_sec);
  st.wMilliseconds = 0;
}

CPDFSDK_DateTime CPDFSDK_DateTime::ToGMT() const {
  CPDFSDK_DateTime new_dt = *this;
  new_dt.AddSeconds(
      -GetTimeZoneInSeconds(new_dt.dt.tzHour, new_dt.dt.tzMinute));
  new_dt.dt.tzHour = 0;
  new_dt.dt.tzMinute = 0;
  return new_dt;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::AddDays(short days) {
  if (days == 0)
    return *this;

  int16_t y = dt.year;
  uint8_t m = dt.month;
  uint8_t d = dt.day;

  int ldays = days;
  if (ldays > 0) {
    int16_t yy = y;
    if ((static_cast<uint16_t>(m) * 100 + d) > 300)
      yy++;
    int ydays = GetYearDays(yy);
    int mdays;
    while (ldays >= ydays) {
      y++;
      ldays -= ydays;
      yy++;
      mdays = GetMonthDays(y, m);
      if (d > mdays) {
        m++;
        d -= mdays;
      }
      ydays = GetYearDays(yy);
    }
    mdays = GetMonthDays(y, m) - d + 1;
    while (ldays >= mdays) {
      ldays -= mdays;
      m++;
      d = 1;
      mdays = GetMonthDays(y, m);
    }
    d += ldays;
  } else {
    ldays *= -1;
    int16_t yy = y;
    if ((static_cast<uint16_t>(m) * 100 + d) < 300)
      yy--;
    int ydays = GetYearDays(yy);
    while (ldays >= ydays) {
      y--;
      ldays -= ydays;
      yy--;
      int mdays = GetMonthDays(y, m);
      if (d > mdays) {
        m++;
        d -= mdays;
      }
      ydays = GetYearDays(yy);
    }
    while (ldays >= d) {
      ldays -= d;
      m--;
      d = GetMonthDays(y, m);
    }
    d -= ldays;
  }

  dt.year = y;
  dt.month = m;
  dt.day = d;

  return *this;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::AddSeconds(int seconds) {
  if (seconds == 0)
    return *this;

  int n;
  int days;

  n = dt.hour * 3600 + dt.minute * 60 + dt.second + seconds;
  if (n < 0) {
    days = (n - 86399) / 86400;
    n -= days * 86400;
  } else {
    days = n / 86400;
    n %= 86400;
  }
  dt.hour = static_cast<uint8_t>(n / 3600);
  dt.hour %= 24;
  n %= 3600;
  dt.minute = static_cast<uint8_t>(n / 60);
  dt.second = static_cast<uint8_t>(n % 60);
  if (days != 0)
    AddDays(days);

  return *this;
}
