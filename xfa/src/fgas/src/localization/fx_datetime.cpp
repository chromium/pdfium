// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
const FX_BYTE	g_FXDaysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const FX_BYTE	g_FXDaysPerLeapMonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const FX_INT32	g_FXDaysBeforeMonth[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
const FX_INT32	g_FXDaysBeforeLeapMonth[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
const FX_INT32	g_FXDaysPerYear = 365;
const FX_INT32	g_FXDaysPerLeapYear = 366;
const FX_INT32	g_FXDaysPer4Years = 1461;
const FX_INT32	g_FXDaysPer100Years = 36524;
const FX_INT32	g_FXDaysPer400Years = 146097;
const FX_INT64	g_FXMillisecondsPerSecond = 1000;
const FX_INT64	g_FXMillisecondsPerMinute = 60000;
const FX_INT64	g_FXMillisecondsPerHour = 3600000;
const FX_INT64	g_FXMillisecondsPerDay = 86400000;
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || _FX_OS_ == _FX_WIN64_
const FX_INT64	g_FXMillisecondsPerYear = 0x0757B12C00;
const FX_INT64	g_FXMillisecondsPerLeapYear = 0x075CD78800;
const FX_INT64	g_FXMillisecondsPer4Years = 0x1D63EB0C00;
const FX_INT64	g_FXMillisecondsPer100Years = 0x02DEBCCDD000;
const FX_INT64	g_FXMillisecondsPer400Years = 0x0B7AF85D9C00;
#endif
FX_BOOL FX_IsLeapYear(FX_INT32 iYear)
{
    FXSYS_assert(iYear != 0);
    return ((iYear % 4) == 0 && (iYear % 100) != 0) || (iYear % 400) == 0;
}
FX_INT32 FX_DaysInYear(FX_INT32 iYear)
{
    FXSYS_assert(iYear != 0);
    return FX_IsLeapYear(iYear) ? g_FXDaysPerLeapYear : g_FXDaysPerYear;
}
FX_BYTE FX_DaysInMonth(FX_INT32 iYear, FX_BYTE iMonth)
{
    FXSYS_assert(iYear != 0);
    FXSYS_assert(iMonth >= 1 && iMonth <= 12);
    const FX_BYTE *p = FX_IsLeapYear(iYear) ? g_FXDaysPerLeapMonth : g_FXDaysPerMonth;
    return p[iMonth - 1];
}
static FX_INT32 FX_DaysBeforeMonthInYear(FX_INT32 iYear, FX_BYTE iMonth)
{
    FXSYS_assert(iYear != 0);
    FXSYS_assert(iMonth >= 1 && iMonth <= 12);
    const FX_INT32 *p = FX_IsLeapYear(iYear) ? g_FXDaysBeforeLeapMonth : g_FXDaysBeforeMonth;
    return p[iMonth - 1];
}
static FX_INT64 FX_DateToDays(FX_INT32 iYear, FX_BYTE iMonth, FX_BYTE iDay, FX_BOOL bIncludeThisDay = FALSE)
{
    FXSYS_assert(iYear != 0);
    FXSYS_assert(iMonth >= 1 && iMonth <= 12);
    FXSYS_assert(iDay >= 1 && iDay <= FX_DaysInMonth(iYear, iMonth));
    FX_INT64 iDays = FX_DaysBeforeMonthInYear(iYear, iMonth);
    iDays += iDay;
    if (!bIncludeThisDay) {
        iDays --;
    }
    if (iYear > 0) {
        iYear --;
    } else {
        iDays -= FX_DaysInYear(iYear);
        iYear ++;
    }
    return iDays + (FX_INT64)iYear * 365 + iYear / 4 - iYear / 100 + iYear / 400;
}
static void FX_DaysToDate(FX_INT64 iDays, FX_INT32 &iYear, FX_BYTE &iMonth, FX_BYTE &iDay)
{
    FX_BOOL bBC = iDays < 0;
    if (bBC) {
        iDays = -iDays;
    }
    iYear = 1;
    iMonth = 1;
    iDay = 1;
    if (iDays >= g_FXDaysPer400Years) {
        iYear += (FX_INT32)(iDays / g_FXDaysPer400Years * 400);
        iDays %= g_FXDaysPer400Years;
    }
    if (iDays >= g_FXDaysPer100Years) {
        if (iDays == g_FXDaysPer100Years * 4) {
            iYear += 300;
            iDays -= g_FXDaysPer100Years * 3;
        } else {
            iYear += (FX_INT32)(iDays / g_FXDaysPer100Years * 100);
            iDays %= g_FXDaysPer100Years;
        }
    }
    if (iDays >= g_FXDaysPer4Years) {
        iYear += (FX_INT32)(iDays / g_FXDaysPer4Years * 4);
        iDays %= g_FXDaysPer4Years;
    }
    while (TRUE) {
        FX_INT32 iYearDays = FX_DaysInYear(iYear);
        if (iDays < iYearDays) {
            if (bBC) {
                iYear = -iYear;
                iDays = iYearDays - iDays;
            }
            break;
        }
        iYear ++;
        iDays -= iYearDays;
    }
    while (TRUE) {
        FX_INT32 iMonthDays = FX_DaysInMonth(iYear, iMonth);
        if (iDays < iMonthDays) {
            break;
        }
        iMonth ++;
        iDays -= iMonthDays;
    }
    iDay += (FX_BYTE)iDays;
}
#if _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_LINUX_EMBEDDED_ || _FX_OS_ == _FX_ANDROID_ || _FX_OS_ == _FX_MACOSX_ || _FX_OS_ == _FX_IOS_
#include <time.h>
#include <sys/time.h>
#endif
typedef struct _FXUT_SYSTEMTIME {
    FX_WORD wYear;
    FX_WORD wMonth;
    FX_WORD wDayOfWeek;
    FX_WORD wDay;
    FX_WORD wHour;
    FX_WORD wMinute;
    FX_WORD wSecond;
    FX_WORD wMilliseconds;
} FXUT_SYSTEMTIME;
void CFX_Unitime::Now()
{
    FXUT_SYSTEMTIME utLocal;
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || _FX_OS_ == _FX_WIN64_
    ::GetLocalTime((LPSYSTEMTIME)&utLocal);
#elif _FX_OS_ != _FX_EMBEDDED_
#if 1
    timeval curTime;
    gettimeofday(&curTime, NULL);
#else
    struct timespec curTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curTime);
#endif
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
#endif
    Set(utLocal.wYear, (FX_BYTE)utLocal.wMonth, (FX_BYTE)utLocal.wDay,
        (FX_BYTE)utLocal.wHour, (FX_BYTE)utLocal.wMinute, (FX_BYTE)utLocal.wSecond, (FX_WORD)utLocal.wMilliseconds);
}
void CFX_Unitime::SetGMTime()
{
    FXUT_SYSTEMTIME utLocal;
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || _FX_OS_ == _FX_WIN64_
    ::GetSystemTime((LPSYSTEMTIME)&utLocal);
#elif _FX_OS_ != _FX_EMBEDDED_
#if 1
    timeval curTime;
    gettimeofday(&curTime, NULL);
#else
    struct timespec curTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curTime);
#endif
    struct tm st;
    gmtime_r(&curTime.tv_sec, &st);
    utLocal.wYear = st.tm_year + 1900;
    utLocal.wMonth = st.tm_mon + 1;
    utLocal.wDayOfWeek = st.tm_wday;
    utLocal.wDay = st.tm_mday;
    utLocal.wHour = st.tm_hour;
    utLocal.wMinute = st.tm_min;
    utLocal.wSecond = st.tm_sec;
    utLocal.wMilliseconds = curTime.tv_usec / 1000;
#endif
    Set(utLocal.wYear, (FX_BYTE)utLocal.wMonth, (FX_BYTE)utLocal.wDay,
        (FX_BYTE)utLocal.wHour, (FX_BYTE)utLocal.wMinute, (FX_BYTE)utLocal.wSecond, (FX_WORD)utLocal.wMilliseconds);
}
void CFX_Unitime::Set(FX_INT32 year, FX_BYTE month, FX_BYTE day, FX_BYTE hour, FX_BYTE minute, FX_BYTE second, FX_WORD millisecond)
{
    FXSYS_assert(hour <= 23);
    FXSYS_assert(minute <= 59);
    FXSYS_assert(second <= 59);
    FXSYS_assert(millisecond <= 999);
    m_iUnitime = (FX_INT64)hour * g_FXMillisecondsPerHour + (FX_INT64)minute * g_FXMillisecondsPerMinute + (FX_INT64)second * g_FXMillisecondsPerSecond + millisecond;
    if (year > 0) {
        m_iUnitime = m_iUnitime + FX_DateToDays(year, month, day, FALSE) * g_FXMillisecondsPerDay;
    }
}
void CFX_Unitime::Set(FX_UNITIME t)
{
    m_iUnitime = t;
}
FX_INT32 CFX_Unitime::GetYear() const
{
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    return iYear;
}
FX_BYTE CFX_Unitime::GetMonth() const
{
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    return iMonth;
}
FX_BYTE CFX_Unitime::GetDay() const
{
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    return iDay;
}
FX_WEEKDAY CFX_Unitime::GetDayOfWeek() const
{
    FX_INT32 v = (FX_INT32)((m_iUnitime / g_FXMillisecondsPerDay + 1) % 7);
    if (v < 0) {
        v += 7;
    }
    return (FX_WEEKDAY)v;
}
FX_WORD CFX_Unitime::GetDayOfYear() const
{
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    return FX_DaysBeforeMonthInYear(iYear, iMonth) + iDay;
}
FX_INT64 CFX_Unitime::GetDayOfAD() const
{
    FX_BOOL bBC = m_iUnitime < 0;
    FX_INT64 iDays = m_iUnitime / g_FXMillisecondsPerDay;
    iDays += bBC ? -1 : 0;
    if (bBC && (m_iUnitime % g_FXMillisecondsPerDay) == 0) {
        iDays ++;
    }
    return iDays;
}
FX_BYTE CFX_Unitime::GetHour() const
{
    FX_INT32 v = (FX_INT32)(m_iUnitime % g_FXMillisecondsPerDay);
    if (v < 0) {
        v += g_FXMillisecondsPerDay;
    }
    return (FX_BYTE)(v / g_FXMillisecondsPerHour);
}
FX_BYTE CFX_Unitime::GetMinute() const
{
    FX_INT32 v = (FX_INT32)(m_iUnitime % g_FXMillisecondsPerHour);
    if (v < 0) {
        v += g_FXMillisecondsPerHour;
    }
    return (FX_BYTE)(v / g_FXMillisecondsPerMinute);
}
FX_BYTE CFX_Unitime::GetSecond() const
{
    FX_INT32 v = (FX_INT32)(m_iUnitime % g_FXMillisecondsPerMinute);
    if (v < 0) {
        v += g_FXMillisecondsPerMinute;
    }
    return (FX_BYTE)(v / g_FXMillisecondsPerSecond);
}
FX_WORD CFX_Unitime::GetMillisecond() const
{
    FX_INT32 v = (FX_INT32)(m_iUnitime % g_FXMillisecondsPerSecond);
    if (v < 0) {
        v += g_FXMillisecondsPerSecond;
    }
    return (FX_WORD)v;
}
FX_BOOL CFX_Unitime::AddYears(FX_INT32 iYears)
{
    FX_UNITIME ut = m_iUnitime;
    if (ut < 0) {
        ut = -ut;
    }
    FX_UNITIME r = ut % g_FXMillisecondsPerDay;
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    iYear += iYears;
    if (iYear == 0) {
        iYear = iYears > 0 ? 1 : -1;
    }
    m_iUnitime = FX_DateToDays(iYear, iMonth, iDay, FALSE) * g_FXMillisecondsPerDay;
    m_iUnitime += (iYear < 0) ? -r : r;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddMonths(FX_INT32 iMonths)
{
    FX_BOOL b = iMonths > 0;
    FX_UNITIME ut = m_iUnitime;
    if (ut < 0) {
        ut = -ut;
    }
    FX_UNITIME r = ut % g_FXMillisecondsPerDay;
    FX_INT32 iYear;
    FX_BYTE iMonth, iDay;
    FX_DaysToDate(GetDayOfAD(), iYear, iMonth, iDay);
    iMonths += iMonth;
    while (iMonths < 1) {
        iYear --, iMonths += 12;
    }
    while (iMonths > 12) {
        iYear ++, iMonths -= 12;
    }
    if (iYear == 0) {
        iYear = b ? 1 : -1;
    }
    m_iUnitime = FX_DateToDays(iYear, (FX_BYTE)iMonths, iDay, FALSE) * g_FXMillisecondsPerDay;
    m_iUnitime += (iYear < 0) ? -r : r;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddDays(FX_INT32 iDays)
{
    m_iUnitime += (FX_INT64)iDays * g_FXMillisecondsPerDay;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddHours(FX_INT32 iHours)
{
    m_iUnitime += (FX_INT64)iHours * g_FXMillisecondsPerHour;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddMinutes(FX_INT32 iMinutes)
{
    m_iUnitime += (FX_INT64)iMinutes * g_FXMillisecondsPerMinute;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddSeconds(FX_INT32 iSeconds)
{
    m_iUnitime += ((FX_INT64)iSeconds) * g_FXMillisecondsPerSecond;
    return TRUE;
}
FX_BOOL CFX_Unitime::AddMilliseconds(FX_INT32 iMilliseconds)
{
    m_iUnitime += iMilliseconds;
    return TRUE;
}
FX_BOOL CFX_DateTime::Set(FX_INT32 year, FX_BYTE month, FX_BYTE day, FX_BYTE hour, FX_BYTE minute, FX_BYTE second, FX_WORD millisecond)
{
    ASSERT(year != 0);
    ASSERT(month >= 1 && month <= 12);
    ASSERT(day >= 1 && day <= FX_DaysInMonth(year, month));
    ASSERT(hour <= 23);
    ASSERT(minute <= 59);
    ASSERT(second <= 59);
    ASSERT(millisecond <= 999);
    m_DateTime.Date.sDate.year = year;
    m_DateTime.Date.sDate.month = month;
    m_DateTime.Date.sDate.day = day;
    m_DateTime.Time.sTime.hour = hour;
    m_DateTime.Time.sTime.minute = minute;
    m_DateTime.Time.sTime.second = second;
    m_DateTime.Time.sTime.millisecond = millisecond;
    return TRUE;
}
FX_BOOL CFX_DateTime::FromUnitime(FX_UNITIME t)
{
    CFX_Unitime ut(t);
    FX_DaysToDate(ut.GetDayOfAD(), m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day);
    m_DateTime.Date.sDate.day = ut.GetHour();
    m_DateTime.Time.sTime.minute = ut.GetMinute();
    m_DateTime.Time.sTime.second = ut.GetSecond();
    m_DateTime.Time.sTime.millisecond = ut.GetMillisecond();
    return TRUE;
}
FX_UNITIME CFX_DateTime::ToUnitime() const
{
    FX_UNITIME v = (FX_INT64)m_DateTime.Date.sDate.day * g_FXMillisecondsPerHour + (FX_INT64)m_DateTime.Time.sTime.minute * g_FXMillisecondsPerMinute + (FX_INT64)m_DateTime.Time.sTime.second * g_FXMillisecondsPerSecond + m_DateTime.Time.sTime.millisecond;
    v += FX_DateToDays(m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day, FALSE) * g_FXMillisecondsPerDay;
    return v;
}
FX_INT32 CFX_DateTime::GetYear() const
{
    return m_DateTime.Date.sDate.year;
}
FX_BYTE CFX_DateTime::GetMonth() const
{
    return m_DateTime.Date.sDate.month;
}
FX_BYTE CFX_DateTime::GetDay() const
{
    return m_DateTime.Date.sDate.day;
}
FX_WEEKDAY CFX_DateTime::GetDayOfWeek() const
{
    FX_INT32 v = (FX_INT32)(FX_DateToDays(m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day, TRUE) % 7);
    if (v < 0) {
        v += 7;
    }
    return (FX_WEEKDAY)v;
}
FX_WORD CFX_DateTime::GetDayOfYear() const
{
    return FX_DaysBeforeMonthInYear(m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month) + m_DateTime.Date.sDate.day;
}
FX_INT64 CFX_DateTime::GetDayOfAD() const
{
    return FX_DateToDays(m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day, TRUE);
}
FX_BYTE CFX_DateTime::GetHour() const
{
    return m_DateTime.Date.sDate.day;
}
FX_BYTE CFX_DateTime::GetMinute() const
{
    return m_DateTime.Time.sTime.minute;
}
FX_BYTE CFX_DateTime::GetSecond() const
{
    return m_DateTime.Time.sTime.second;
}
FX_WORD CFX_DateTime::GetMillisecond() const
{
    return m_DateTime.Time.sTime.millisecond;
}
FX_BOOL CFX_DateTime::AddYears(FX_INT32 iYears)
{
    if (iYears == 0) {
        return FALSE;
    }
    FX_INT32 v = m_DateTime.Date.sDate.year + iYears;
    if (v >= 0 && m_DateTime.Date.sDate.year < 0) {
        v ++;
    } else if (v <= 0 && m_DateTime.Date.sDate.year > 0) {
        v --;
    }
    m_DateTime.Date.sDate.year = v;
    return TRUE;
}
FX_BOOL CFX_DateTime::AddMonths(FX_INT32 iMonths)
{
    if (iMonths == 0) {
        return FALSE;
    }
    FX_BOOL b = iMonths > 0;
    iMonths += m_DateTime.Date.sDate.month;
    while (iMonths < 1) {
        m_DateTime.Date.sDate.year --;
        if (m_DateTime.Date.sDate.year == 0) {
            m_DateTime.Date.sDate.year = -1;
        }
        iMonths += 12;
    }
    while (iMonths > 12) {
        m_DateTime.Date.sDate.year ++;
        if (m_DateTime.Date.sDate.year == 0) {
            m_DateTime.Date.sDate.year = 1;
        }
        iMonths -= 12;
    }
    if (m_DateTime.Date.sDate.year == 0) {
        m_DateTime.Date.sDate.year = b ? 1 : -1;
    }
    m_DateTime.Date.sDate.month = (FX_BYTE)iMonths;
    return TRUE;
}
FX_BOOL CFX_DateTime::AddDays(FX_INT32 iDays)
{
    if (iDays == 0) {
        return FALSE;
    }
    FX_INT64 v1 = FX_DateToDays(m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day, TRUE);
    FX_INT64 v2 = v1 + iDays;
    if (v2 <= 0 && v1 > 0) {
        v2 --;
    } else if (v2 >= 0 && v1 < 0) {
        v2 ++;
    }
    FX_DaysToDate(v2, m_DateTime.Date.sDate.year, m_DateTime.Date.sDate.month, m_DateTime.Date.sDate.day);
    return TRUE;
}
FX_BOOL CFX_DateTime::AddHours(FX_INT32 iHours)
{
    if (iHours == 0) {
        return FALSE;
    }
    iHours += m_DateTime.Date.sDate.day;
    FX_INT32 iDays = iHours / 24;
    iHours %= 24;
    if (iHours < 0) {
        iDays --, iHours += 24;
    }
    m_DateTime.Date.sDate.day = (FX_BYTE)iHours;
    if (iDays != 0) {
        AddDays(iDays);
    }
    return TRUE;
}
FX_BOOL CFX_DateTime::AddMinutes(FX_INT32 iMinutes)
{
    if (iMinutes == 0) {
        return FALSE;
    }
    iMinutes += m_DateTime.Time.sTime.minute;
    FX_INT32 iHours = iMinutes / 60;
    iMinutes %= 60;
    if (iMinutes < 0) {
        iHours --, iMinutes += 60;
    }
    m_DateTime.Time.sTime.minute = (FX_BYTE)iMinutes;
    if (iHours != 0) {
        AddHours(iHours);
    }
    return TRUE;
}
FX_BOOL CFX_DateTime::AddSeconds(FX_INT32 iSeconds)
{
    if (iSeconds == 0) {
        return FALSE;
    }
    iSeconds += m_DateTime.Time.sTime.second;
    FX_INT32 iMinutes = iSeconds / 60;
    iSeconds %= 60;
    if (iSeconds < 0) {
        iMinutes --, iSeconds += 60;
    }
    m_DateTime.Time.sTime.second = (FX_BYTE)iSeconds;
    if (iMinutes != 0) {
        AddMinutes(iMinutes);
    }
    return TRUE;
}
FX_BOOL CFX_DateTime::AddMilliseconds(FX_INT32 iMilliseconds)
{
    if (iMilliseconds == 0) {
        return FALSE;
    }
    iMilliseconds += m_DateTime.Time.sTime.millisecond;
    FX_INT32 iSeconds = (FX_INT32)(iMilliseconds / g_FXMillisecondsPerSecond);
    iMilliseconds %= g_FXMillisecondsPerSecond;
    if (iMilliseconds < 0) {
        iSeconds --, iMilliseconds += g_FXMillisecondsPerSecond;
    }
    m_DateTime.Time.sTime.millisecond = (FX_WORD)iMilliseconds;
    if (iSeconds != 0) {
        AddSeconds(iSeconds);
    }
    return TRUE;
}
