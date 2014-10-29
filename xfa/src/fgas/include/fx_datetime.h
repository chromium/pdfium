// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_DATETIME_H_
#define _FX_DATETIME_H_
class CFX_Unitime;
class CFX_DateTime;
typedef FX_INT64	FX_UNITIME;
enum FX_WEEKDAY {
    FX_Sunday			=  0,
    FX_Monday				,
    FX_Tuesday				,
    FX_Wednesday			,
    FX_Thursday				,
    FX_Friday				,
    FX_Saturday				,
};
FX_BOOL		FX_IsLeapYear(FX_INT32 iYear);
FX_INT32	FX_DaysInYear(FX_INT32 iYear);
FX_BYTE		FX_DaysInMonth(FX_INT32 iYear, FX_BYTE iMonth);
class CFX_Unitime : public CFX_Object
{
public:
    CFX_Unitime()
    {
        m_iUnitime = 0;
    }
    CFX_Unitime(FX_UNITIME iUnitime)
    {
        m_iUnitime = iUnitime;
    }
    CFX_Unitime(const CFX_Unitime &unitime)
    {
        m_iUnitime = unitime.m_iUnitime;
    }
    operator FX_UNITIME * ()
    {
        return &m_iUnitime;
    }
    operator FX_UNITIME const * () const
    {
        return &m_iUnitime;
    }
    operator FX_UNITIME& ()
    {
        return m_iUnitime;
    }
    operator const FX_UNITIME& () const
    {
        return m_iUnitime;
    }
    CFX_Unitime&	operator = (const CFX_Unitime &t)
    {
        m_iUnitime = t.m_iUnitime;
        return *this;
    }
    CFX_Unitime&	operator = (FX_UNITIME t)
    {
        m_iUnitime = t;
        return *this;
    }
    CFX_Unitime&	operator += (const CFX_Unitime &t)
    {
        m_iUnitime += t.m_iUnitime;
        return *this;
    }
    CFX_Unitime&	operator += (FX_UNITIME t)
    {
        m_iUnitime += t;
        return *this;
    }
    CFX_Unitime&	operator -= (const CFX_Unitime &t)
    {
        m_iUnitime -= t.m_iUnitime;
        return *this;
    }
    CFX_Unitime&	operator -= (FX_UNITIME t)
    {
        m_iUnitime -= t;
        return *this;
    }
    void			Now();
    void			SetGMTime();
    void			Set(FX_INT32 year, FX_BYTE month, FX_BYTE day, FX_BYTE hour = 0, FX_BYTE minute = 0, FX_BYTE second = 0, FX_WORD millisecond = 0);
    void			Set(FX_UNITIME t);
    FX_INT32		GetYear() const;
    FX_BYTE			GetMonth() const;
    FX_BYTE			GetDay() const;
    FX_WEEKDAY		GetDayOfWeek() const;
    FX_WORD			GetDayOfYear() const;
    FX_INT64		GetDayOfAD() const;
    FX_BYTE			GetHour() const;
    FX_BYTE			GetMinute() const;
    FX_BYTE			GetSecond() const;
    FX_WORD			GetMillisecond() const;
    FX_BOOL			AddYears(FX_INT32 iYears);
    FX_BOOL			AddMonths(FX_INT32 iMonths);
    FX_BOOL			AddDays(FX_INT32 iDays);
    FX_BOOL			AddHours(FX_INT32 iHours);
    FX_BOOL			AddMinutes(FX_INT32 iMinutes);
    FX_BOOL			AddSeconds(FX_INT32 iSeconds);
    FX_BOOL			AddMilliseconds(FX_INT32 iMilliseconds);
    friend CFX_Unitime	operator + (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return CFX_Unitime(t1.m_iUnitime + t2.m_iUnitime);
    }
    friend CFX_Unitime	operator + (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return CFX_Unitime(t1.m_iUnitime + t2);
    }
    friend CFX_Unitime	operator + (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return CFX_Unitime(t1 + t2.m_iUnitime);
    }
    friend CFX_Unitime	operator - (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return CFX_Unitime(t1.m_iUnitime + t2.m_iUnitime);
    }
    friend CFX_Unitime	operator - (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return CFX_Unitime(t1.m_iUnitime + t2);
    }
    friend CFX_Unitime	operator - (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return CFX_Unitime(t1 + t2.m_iUnitime);
    }
    friend FX_BOOL		operator == (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime == t2.m_iUnitime;
    }
    friend FX_BOOL		operator == (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime == t2;
    }
    friend FX_BOOL		operator == (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 == t2.m_iUnitime;
    }
    friend FX_BOOL		operator != (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime != t2.m_iUnitime;
    }
    friend FX_BOOL		operator != (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime != t2;
    }
    friend FX_BOOL		operator != (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 != t2.m_iUnitime;
    }
    friend FX_BOOL		operator > (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime > t2.m_iUnitime;
    }
    friend FX_BOOL		operator > (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime > t2;
    }
    friend FX_BOOL		operator > (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 > t2.m_iUnitime;
    }
    friend FX_BOOL		operator >= (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime >= t2.m_iUnitime;
    }
    friend FX_BOOL		operator >= (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime >= t2;
    }
    friend FX_BOOL		operator >= (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 >= t2.m_iUnitime;
    }
    friend FX_BOOL		operator < (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime < t2.m_iUnitime;
    }
    friend FX_BOOL		operator < (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime < t2;
    }
    friend FX_BOOL		operator < (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 < t2.m_iUnitime;
    }
    friend FX_BOOL		operator <= (const CFX_Unitime &t1, const CFX_Unitime &t2)
    {
        return t1.m_iUnitime <= t2.m_iUnitime;
    }
    friend FX_BOOL		operator <= (const CFX_Unitime &t1, FX_UNITIME t2)
    {
        return t1.m_iUnitime <= t2;
    }
    friend FX_BOOL		operator <= (FX_UNITIME t1, const CFX_Unitime &t2)
    {
        return t1 <= t2.m_iUnitime;
    }
private:
    FX_UNITIME m_iUnitime;
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
typedef struct _FX_DATE {
    FX_INT32	year;
    FX_BYTE		month;
    FX_BYTE		day;
} FX_DATE, * FX_LPDATE;
typedef FX_DATE const * FX_LPCDATE;
typedef struct _FX_TIME {
    FX_BYTE		hour;
    FX_BYTE		minute;
    FX_BYTE		second;
    FX_WORD		millisecond;
} FX_TIME, * FX_LPTIME;
typedef FX_TIME const * FX_LPCTIME;
typedef struct _FX_TIMEZONE {
    FX_INT8		tzHour;
    FX_BYTE		tzMinute;
} FX_TIMEZONE, * FX_LPTIMEZONE;
typedef FX_TIMEZONE const * FX_LPCTIMEZONE;
typedef struct _FX_DATETIME {
    union {
        struct {
            FX_INT32	year;
            FX_BYTE		month;
            FX_BYTE		day;
        } sDate;
        FX_DATE aDate;
    } Date;
    union {
        struct {
            FX_BYTE		hour;
            FX_BYTE		minute;
            FX_BYTE		second;
            FX_WORD		millisecond;
        } sTime;
        FX_TIME aTime;
    } Time;
} FX_DATETIME, * FX_LPDATETIME;
typedef FX_DATETIME const * FX_LPCDATETIME;
typedef struct _FX_DATETIMEZONE {
    union {
        struct {
            union {
                struct {
                    FX_INT32	year;
                    FX_BYTE		month;
                    FX_BYTE		day;
                };
                FX_DATE	date;
            };
            union {
                struct {
                    FX_BYTE		hour;
                    FX_BYTE		minute;
                    FX_BYTE		second;
                    FX_WORD		millisecond;
                };
                FX_TIME time;
            };
        };
        FX_DATETIME dt;
    };
    union {
        struct {
            FX_INT8	tzHour;
            FX_BYTE	tzMinute;
        };
        FX_TIMEZONE tz;
    };
} FX_DATETIMEZONE, * FX_LPDATETIMEZONE;
typedef FX_DATETIMEZONE const * FX_LPCDATETIMEZONE;
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif
class CFX_DateTime : public CFX_Object
{
public:
    CFX_DateTime() {}
    CFX_DateTime(const FX_DATETIME &dt)
    {
        m_DateTime = dt;
    }
    CFX_DateTime(const CFX_DateTime &dt)
    {
        m_DateTime = dt.m_DateTime;
    }
    operator FX_DATETIME * ()
    {
        return &m_DateTime;
    }
    operator FX_DATETIME const * () const
    {
        return &m_DateTime;
    }
    operator FX_DATETIME& ()
    {
        return m_DateTime;
    }
    operator const FX_DATETIME& () const
    {
        return m_DateTime;
    }
    CFX_DateTime&	operator = (const CFX_DateTime &dt)
    {
        m_DateTime = dt.m_DateTime;
        return *this;
    }
    CFX_DateTime&	operator = (const FX_DATETIME &dt)
    {
        m_DateTime = dt;
        return *this;
    }
    CFX_DateTime&	operator += (const CFX_DateTime &dt)
    {
        FromUnitime(ToUnitime() + dt.ToUnitime());
        return *this;
    }
    CFX_DateTime&	operator += (const FX_DATETIME &dt)
    {
        FromUnitime(ToUnitime() + ((const CFX_DateTime&)dt).ToUnitime());
        return *this;
    }
    CFX_DateTime&	operator -= (const CFX_DateTime &dt)
    {
        FromUnitime(ToUnitime() - dt.ToUnitime());
        return *this;
    }
    CFX_DateTime&	operator -= (const FX_DATETIME &dt)
    {
        FromUnitime(ToUnitime() - ((const CFX_DateTime&)dt).ToUnitime());
        return *this;
    }
    virtual FX_BOOL			Set(FX_INT32 year, FX_BYTE month, FX_BYTE day, FX_BYTE hour = 0, FX_BYTE minute = 0, FX_BYTE second = 0, FX_WORD millisecond = 0);
    virtual FX_BOOL			FromUnitime(FX_UNITIME t);
    virtual FX_UNITIME		ToUnitime() const;
    virtual FX_INT32		GetYear() const;
    virtual FX_BYTE			GetMonth() const;
    virtual FX_BYTE			GetDay() const;
    virtual FX_WEEKDAY		GetDayOfWeek() const;
    virtual FX_WORD			GetDayOfYear() const;
    virtual FX_INT64		GetDayOfAD() const;
    virtual FX_BYTE			GetHour() const;
    virtual FX_BYTE			GetMinute() const;
    virtual FX_BYTE			GetSecond() const;
    virtual FX_WORD			GetMillisecond() const;
    virtual FX_BOOL			AddYears(FX_INT32 iYears);
    virtual FX_BOOL			AddMonths(FX_INT32 iMonths);
    virtual FX_BOOL			AddDays(FX_INT32 iDays);
    virtual FX_BOOL			AddHours(FX_INT32 iHours);
    virtual FX_BOOL			AddMinutes(FX_INT32 iMinutes);
    virtual FX_BOOL			AddSeconds(FX_INT32 iSeconds);
    virtual FX_BOOL			AddMilliseconds(FX_INT32 iMilliseconds);
    friend CFX_DateTime	operator + (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(dt1.ToUnitime() + dt2.ToUnitime());
        return dt;
    }
    friend CFX_DateTime	operator + (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(dt1.ToUnitime() + ((const CFX_DateTime&)dt2).ToUnitime());
        return dt;
    }
    friend CFX_DateTime	operator + (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(((const CFX_DateTime&)dt1).ToUnitime() + dt2.ToUnitime());
        return dt;
    }
    friend CFX_DateTime	operator - (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(dt1.ToUnitime() - dt2.ToUnitime());
        return dt;
    }
    friend CFX_DateTime	operator - (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(dt1.ToUnitime() - ((const CFX_DateTime&)dt2).ToUnitime());
        return dt;
    }
    friend CFX_DateTime	operator - (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        CFX_DateTime dt;
        dt.FromUnitime(((const CFX_DateTime&)dt1).ToUnitime() - dt2.ToUnitime());
        return dt;
    }
    friend FX_BOOL			operator == (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return FXSYS_memcmp((FX_LPCDATETIME)dt1, (FX_LPCDATETIME)dt2, sizeof(FX_DATETIME)) == 0;
    }
    friend FX_BOOL			operator == (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return FXSYS_memcmp((FX_LPCDATETIME)dt1, &dt2, sizeof(FX_DATETIME)) == 0;
    }
    friend FX_BOOL			operator == (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return FXSYS_memcmp(&dt1, (FX_LPCDATETIME)dt2, sizeof(FX_DATETIME)) == 0;
    }
    friend FX_BOOL			operator != (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return FXSYS_memcmp((FX_LPCDATETIME)dt1, (FX_LPCDATETIME)dt2, sizeof(FX_DATETIME)) != 0;
    }
    friend FX_BOOL			operator != (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return FXSYS_memcmp((FX_LPCDATETIME)dt1, &dt2, sizeof(FX_DATETIME)) != 0;
    }
    friend FX_BOOL			operator != (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return FXSYS_memcmp(&dt1, (FX_LPCDATETIME)dt2, sizeof(FX_DATETIME)) != 0;
    }
    friend FX_BOOL			operator > (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return dt1.ToUnitime() > dt2.ToUnitime();
    }
    friend FX_BOOL			operator > (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return dt1.ToUnitime() > ((const CFX_DateTime&)dt2).ToUnitime();
    }
    friend FX_BOOL			operator > (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return ((const CFX_DateTime&)dt1).ToUnitime() > dt2.ToUnitime();
    }
    friend FX_BOOL			operator >= (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return dt1.ToUnitime() >= dt2.ToUnitime();
    }
    friend FX_BOOL			operator >= (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return dt1.ToUnitime() >= ((const CFX_DateTime&)dt2).ToUnitime();
    }
    friend FX_BOOL			operator >= (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return ((const CFX_DateTime&)dt1).ToUnitime() >= dt2.ToUnitime();
    }
    friend FX_BOOL			operator < (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return dt1.ToUnitime() < dt2.ToUnitime();
    }
    friend FX_BOOL			operator < (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return dt1.ToUnitime() < ((const CFX_DateTime&)dt2).ToUnitime();
    }
    friend FX_BOOL			operator < (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return ((const CFX_DateTime&)dt1).ToUnitime() < dt2.ToUnitime();
    }
    friend FX_BOOL			operator <= (const CFX_DateTime &dt1, const CFX_DateTime &dt2)
    {
        return dt1.ToUnitime() <= dt2.ToUnitime();
    }
    friend FX_BOOL			operator <= (const CFX_DateTime &dt1, const FX_DATETIME &dt2)
    {
        return dt1.ToUnitime() <= ((const CFX_DateTime&)dt2).ToUnitime();
    }
    friend FX_BOOL			operator <= (const FX_DATETIME &dt1, const CFX_DateTime &dt2)
    {
        return ((const CFX_DateTime&)dt1).ToUnitime() <= dt2.ToUnitime();
    }
private:
    FX_DATETIME m_DateTime;
};
#endif
