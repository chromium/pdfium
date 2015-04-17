// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"
#include "../include/fsdk_baseannot.h"


//---------------------------------------------------------------------------
//								CPDFSDK_DateTime	
//---------------------------------------------------------------------------
int _gAfxGetTimeZoneInSeconds(FX_CHAR tzhour, FX_BYTE tzminute)
{
	return (int)tzhour * 3600 + (int)tzminute * (tzhour >= 0 ? 60 : -60);
}

FX_BOOL _gAfxIsLeapYear(FX_SHORT year)
{
	return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

FX_WORD _gAfxGetYearDays(FX_SHORT year)
{
	return (_gAfxIsLeapYear(year) == TRUE ? 366 : 365);
}

FX_BYTE _gAfxGetMonthDays(FX_SHORT year, FX_BYTE month)
{
	FX_BYTE	mDays;
	switch (month)
	{
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
		if (_gAfxIsLeapYear(year) == TRUE)
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

CPDFSDK_DateTime::CPDFSDK_DateTime()
{
	ResetDateTime();
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const CFX_ByteString& dtStr)
{
	ResetDateTime();

	FromPDFDateTimeString(dtStr);
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const CPDFSDK_DateTime& datetime)
{
	operator = (datetime);
}

CPDFSDK_DateTime::CPDFSDK_DateTime(const FX_SYSTEMTIME& st)
{
	operator = (st) ;
}


void CPDFSDK_DateTime::ResetDateTime()
{
	tzset();

	time_t	curTime;
	time(&curTime);
	struct tm* newtime;
	//newtime = gmtime(&curTime);
	newtime = localtime(&curTime);

	dt.year = newtime->tm_year + 1900;
	dt.month = newtime->tm_mon + 1;
	dt.day = newtime->tm_mday;
	dt.hour = newtime->tm_hour;
	dt.minute = newtime->tm_min;
	dt.second = newtime->tm_sec;
// 	dt.tzHour = _timezone / 3600 * -1;
// 	dt.tzMinute = (abs(_timezone) % 3600) / 60;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::operator = (const CPDFSDK_DateTime& datetime)
{
	FXSYS_memcpy(&dt, &datetime.dt, sizeof(FX_DATETIME));
	return *this;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::operator = (const FX_SYSTEMTIME& st)
{
	tzset();

	dt.year = (FX_SHORT)st.wYear;
	dt.month = (FX_BYTE)st.wMonth;
	dt.day = (FX_BYTE)st.wDay;
	dt.hour = (FX_BYTE)st.wHour;
	dt.minute = (FX_BYTE)st.wMinute;
	dt.second = (FX_BYTE)st.wSecond;
// 	dt.tzHour = _timezone / 3600 * -1;
// 	dt.tzMinute = (abs(_timezone) % 3600) / 60;
	return *this;
}

FX_BOOL CPDFSDK_DateTime::operator == (CPDFSDK_DateTime& datetime)
{
	return (FXSYS_memcmp(&dt, &datetime.dt, sizeof(FX_DATETIME)) == 0);
}

FX_BOOL CPDFSDK_DateTime::operator != (CPDFSDK_DateTime& datetime)
{
	return (FXSYS_memcmp(&dt, &datetime.dt, sizeof(FX_DATETIME)) != 0);
}

FX_BOOL CPDFSDK_DateTime::operator > (CPDFSDK_DateTime& datetime)
{
	CPDFSDK_DateTime dt1 = ToGMT();
	CPDFSDK_DateTime dt2 = datetime.ToGMT();
	int d1 = (((int)dt1.dt.year) << 16) | (((int)dt1.dt.month) << 8) | (int)dt1.dt.day;
	int d2 = (((int)dt1.dt.hour) << 16) | (((int)dt1.dt.minute) << 8) | (int)dt1.dt.second;
	int d3 = (((int)dt2.dt.year) << 16) | (((int)dt2.dt.month) << 8) | (int)dt2.dt.day;
	int d4 = (((int)dt2.dt.hour) << 16) | (((int)dt2.dt.minute) << 8) | (int)dt2.dt.second;

	if (d1 > d3) return TRUE;
	if (d2 > d4) return TRUE;
	return FALSE;
}

FX_BOOL CPDFSDK_DateTime::operator >= (CPDFSDK_DateTime& datetime)
{
	CPDFSDK_DateTime dt1 = ToGMT();
	CPDFSDK_DateTime dt2 = datetime.ToGMT();
	int d1 = (((int)dt1.dt.year) << 16) | (((int)dt1.dt.month) << 8) | (int)dt1.dt.day;
	int d2 = (((int)dt1.dt.hour) << 16) | (((int)dt1.dt.minute) << 8) | (int)dt1.dt.second;
	int d3 = (((int)dt2.dt.year) << 16) | (((int)dt2.dt.month) << 8) | (int)dt2.dt.day;
	int d4 = (((int)dt2.dt.hour) << 16) | (((int)dt2.dt.minute) << 8) | (int)dt2.dt.second;

	if (d1 >= d3) return TRUE;
	if (d2 >= d4) return TRUE;
	return FALSE;
}

FX_BOOL CPDFSDK_DateTime::operator < (CPDFSDK_DateTime& datetime)
{
	CPDFSDK_DateTime dt1 = ToGMT();
	CPDFSDK_DateTime dt2 = datetime.ToGMT();
	int d1 = (((int)dt1.dt.year) << 16) | (((int)dt1.dt.month) << 8) | (int)dt1.dt.day;
	int d2 = (((int)dt1.dt.hour) << 16) | (((int)dt1.dt.minute) << 8) | (int)dt1.dt.second;
	int d3 = (((int)dt2.dt.year) << 16) | (((int)dt2.dt.month) << 8) | (int)dt2.dt.day;
	int d4 = (((int)dt2.dt.hour) << 16) | (((int)dt2.dt.minute) << 8) | (int)dt2.dt.second;

	if (d1 < d3) return TRUE;
	if (d2 < d4) return TRUE;
	return FALSE;
}

FX_BOOL CPDFSDK_DateTime::operator <= (CPDFSDK_DateTime& datetime)
{
	CPDFSDK_DateTime dt1 = ToGMT();
	CPDFSDK_DateTime dt2 = datetime.ToGMT();
	int d1 = (((int)dt1.dt.year) << 16) | (((int)dt1.dt.month) << 8) | (int)dt1.dt.day;
	int d2 = (((int)dt1.dt.hour) << 16) | (((int)dt1.dt.minute) << 8) | (int)dt1.dt.second;
	int d3 = (((int)dt2.dt.year) << 16) | (((int)dt2.dt.month) << 8) | (int)dt2.dt.day;
	int d4 = (((int)dt2.dt.hour) << 16) | (((int)dt2.dt.minute) << 8) | (int)dt2.dt.second;

	if (d1 <= d3) return TRUE;
	if (d2 <= d4) return TRUE;
	return FALSE;
}

CPDFSDK_DateTime::operator time_t()
{
	struct tm newtime;

	newtime.tm_year = dt.year - 1900;
	newtime.tm_mon = dt.month - 1;
	newtime.tm_mday = dt.day;
	newtime.tm_hour = dt.hour;
	newtime.tm_min = dt.minute;
	newtime.tm_sec = dt.second;

	return mktime(&newtime);
}

CPDFSDK_DateTime& CPDFSDK_DateTime::FromPDFDateTimeString(const CFX_ByteString& dtStr)
{
	int strLength = dtStr.GetLength();
	if (strLength > 0)
	{
		int i = 0;
		int j, k;
		FX_CHAR ch;
		while (i < strLength)
		{
			ch = dtStr[i];
			if (ch >= '0' && ch <= '9') break;
			i ++;
		}
		if (i >= strLength) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 4)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.year = (FX_SHORT)k;
		if (i >= strLength || j < 4) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.month = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.day = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.hour = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.minute = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;

		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.second = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;

		ch = dtStr[i ++];
		if (ch != '-' && ch != '+') return *this;
		if (ch == '-')
			dt.tzHour = -1;
		else
			dt.tzHour = 1;
		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.tzHour *= (FX_CHAR)k;
		if (i >= strLength || j < 2) return *this;

		ch = dtStr[i ++];
		if (ch != '\'') return *this;
		j = 0;
		k = 0;
		while (i < strLength && j < 2)
		{
			ch = dtStr[i];
			k = k * 10 + ch - '0';
			j ++;
			if (ch < '0' || ch > '9') break;
			i ++;
		}
		dt.tzMinute = (FX_BYTE)k;
		if (i >= strLength || j < 2) return *this;
	}

	return  *this;
}

CFX_ByteString CPDFSDK_DateTime::ToCommonDateTimeString()
{
	CFX_ByteString str1;
	str1.Format("%04d-%02d-%02d %02d:%02d:%02d ", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	if (dt.tzHour < 0)
		str1 += "-";
	else
		str1 += "+";
	CFX_ByteString str2;
	str2.Format("%02d:%02d", abs(dt.tzHour), dt.tzMinute);
	return str1 + str2;
}

CFX_ByteString CPDFSDK_DateTime::ToPDFDateTimeString()
{
	CFX_ByteString dtStr;
	char tempStr[32];
	memset(tempStr, 0, sizeof(tempStr));
	FXSYS_snprintf(tempStr, sizeof(tempStr) - 1, "D:%04d%02d%02d%02d%02d%02d",
                   dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	dtStr = CFX_ByteString(tempStr);
	if (dt.tzHour < 0)
		dtStr += CFX_ByteString("-");
	else
		dtStr += CFX_ByteString("+");
	memset(tempStr, 0, sizeof(tempStr));
	FXSYS_snprintf(tempStr, sizeof(tempStr) - 1, "%02d'%02d'", abs(dt.tzHour), dt.tzMinute);
	dtStr += CFX_ByteString(tempStr);
	return dtStr;
}

void CPDFSDK_DateTime::ToSystemTime(FX_SYSTEMTIME& st)
{
	CPDFSDK_DateTime dt = *this;
	time_t t = (time_t)dt;
	struct tm* pTime = localtime(&t);
	if(pTime){ 
		st.wYear = (FX_WORD)pTime->tm_year + 1900;
		st.wMonth = (FX_WORD)pTime->tm_mon + 1;
		st.wDay = (FX_WORD)pTime->tm_mday;
		st.wDayOfWeek = (FX_WORD)pTime->tm_wday;
		st.wHour = (FX_WORD)pTime->tm_hour;
		st.wMinute = (FX_WORD)pTime->tm_min;
		st.wSecond = (FX_WORD)pTime->tm_sec;
		st.wMilliseconds = 0;
	}
}

CPDFSDK_DateTime CPDFSDK_DateTime::ToGMT()
{
	CPDFSDK_DateTime dt = *this;
	dt.AddSeconds(-_gAfxGetTimeZoneInSeconds(dt.dt.tzHour, dt.dt.tzMinute));
	dt.dt.tzHour = 0;
	dt.dt.tzMinute = 0;
	return dt;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::AddDays(short days)
{
	if (days == 0) return *this;

	FX_SHORT	y = dt.year, yy;
	FX_BYTE		m = dt.month;
	FX_BYTE		d = dt.day;
	int			mdays, ydays, ldays;

	ldays = days;
	if (ldays > 0)
	{
		yy = y;
		if (((FX_WORD)m * 100 + d) > 300) yy ++;
		ydays = _gAfxGetYearDays(yy);
		while (ldays >= ydays)
		{
			y ++;
			ldays -= ydays;
			yy ++;
			mdays = _gAfxGetMonthDays(y, m);
			if (d > mdays)
			{
				m ++;
				d -= mdays;
			}
			ydays = _gAfxGetYearDays(yy);
		}
		mdays = _gAfxGetMonthDays(y, m) - d + 1;
		while (ldays >= mdays)
		{
			ldays -= mdays;
			m ++;
			d = 1;
			mdays = _gAfxGetMonthDays(y, m);
		}
		d += ldays;
	}
	else
	{
		ldays *= -1;
		yy = y;
		if (((FX_WORD)m * 100 + d) < 300) yy --;
		ydays = _gAfxGetYearDays(yy);
		while (ldays >= ydays)
		{
			y --;
			ldays -= ydays;
			yy --;
			mdays = _gAfxGetMonthDays(y, m);
			if (d > mdays)
			{
				m ++;
				d -= mdays;
			}
			ydays = _gAfxGetYearDays(yy);
		}
		while (ldays >= d)
		{
			ldays -= d;
			m --;
			mdays = _gAfxGetMonthDays(y, m);
			d = mdays;
		}
		d -= ldays;
	}

	dt.year = y;
	dt.month = m;
	dt.day = d;

	return *this;
}

CPDFSDK_DateTime& CPDFSDK_DateTime::AddSeconds(int seconds)
{
	if (seconds == 0) return *this;

	int	n;
	int	days;

	n = dt.hour * 3600 + dt.minute * 60 + dt.second + seconds;
	if (n < 0)
	{
		days = (n - 86399) / 86400;
		n -= days * 86400;
	}
	else
	{
		days = n / 86400;
		n %= 86400;
	}
	dt.hour = (FX_BYTE)(n / 3600);
	dt.hour %= 24;
	n %= 3600;
	dt.minute = (FX_BYTE)(n / 60);
	dt.second = (FX_BYTE)(n % 60);
	if (days != 0) AddDays(days);

	return *this;
}


//---------------------------------------------------------------------------
//								CPDFSDK_Annot	
//---------------------------------------------------------------------------
CPDFSDK_Annot::CPDFSDK_Annot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPageView) :
m_pAnnot(pAnnot),
m_pPageView(pPageView),
m_bSelected(FALSE),
m_nTabOrder(-1)
{
}

CPDFSDK_Annot::~CPDFSDK_Annot()
{
	m_pAnnot = NULL;
	m_pPageView = NULL;
}

CPDF_Annot*	CPDFSDK_Annot::GetPDFAnnot()
{
	return m_pAnnot;
}

FX_DWORD CPDFSDK_Annot::GetFlags()
{
	ASSERT(m_pAnnot != NULL);
	
	return m_pAnnot->GetFlags();
}

void CPDFSDK_Annot::SetPage(CPDFSDK_PageView* pPageView)
{
	m_pPageView = pPageView;
}

CPDFSDK_PageView* CPDFSDK_Annot::GetPageView()
{
	return m_pPageView;
}

FX_BOOL CPDFSDK_Annot::IsSelected()
{
	return m_bSelected;
}

void CPDFSDK_Annot::SetSelected(FX_BOOL bSelected)
{
	m_bSelected = bSelected;
}

// Tab Order	
int CPDFSDK_Annot::GetTabOrder()
{
	return m_nTabOrder;
}

void CPDFSDK_Annot::SetTabOrder(int iTabOrder)
{
	m_nTabOrder = iTabOrder;
}

CPDF_Dictionary* CPDFSDK_Annot::GetAnnotDict() const
{
	ASSERT(m_pAnnot != NULL);
	
	return m_pAnnot->m_pAnnotDict;
}

void CPDFSDK_Annot::SetRect(const CPDF_Rect& rect)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	ASSERT(rect.right - rect.left >= GetMinWidth());
	ASSERT(rect.top - rect.bottom >= GetMinHeight());
	
	m_pAnnot->m_pAnnotDict->SetAtRect("Rect", rect);
}

CPDF_Rect CPDFSDK_Annot::GetRect() const
{
	ASSERT(m_pAnnot != NULL);
	
	CPDF_Rect rect;
	m_pAnnot->GetRect(rect);
	
	return rect;
}

CFX_ByteString CPDFSDK_Annot::GetType() const
{
	ASSERT(m_pAnnot != NULL);
	
	return m_pAnnot->GetSubType();
}

CFX_ByteString CPDFSDK_Annot::GetSubType() const
{
	return "";
}

void CPDFSDK_Annot::DrawAppearance(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
								   CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions)	
{
	ASSERT(m_pPageView != NULL);
	ASSERT(m_pAnnot != NULL);
	
	m_pAnnot->DrawAppearance(m_pPageView->GetPDFPage(), pDevice, pUser2Device, mode, pOptions);
}

FX_BOOL	CPDFSDK_Annot::IsAppearanceValid()
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	return m_pAnnot->m_pAnnotDict->GetDict("AP") != NULL;
}

FX_BOOL	CPDFSDK_Annot::IsAppearanceValid(CPDF_Annot::AppearanceMode mode)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	CPDF_Dictionary* pAP = m_pAnnot->m_pAnnotDict->GetDict("AP");
	if (pAP == NULL) return FALSE;
	
	// Choose the right sub-ap
	const FX_CHAR* ap_entry = "N";
	if (mode == CPDF_Annot::Down)
		ap_entry = "D";
	else if (mode == CPDF_Annot::Rollover)
		ap_entry = "R";
	if (!pAP->KeyExist(ap_entry))
		ap_entry = "N";
	
	// Get the AP stream or subdirectory
	CPDF_Object* psub = pAP->GetElementValue(ap_entry);
	if (psub == NULL) return FALSE;
	
	return TRUE;
}

void CPDFSDK_Annot::DrawBorder(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
						   const CPDF_RenderOptions* pOptions)
{
	ASSERT(m_pAnnot != NULL);
	m_pAnnot->DrawBorder(pDevice, pUser2Device, pOptions); 
}

void CPDFSDK_Annot::ClearCachedAP()
{
	ASSERT(m_pAnnot != NULL);
	m_pAnnot->ClearCachedAP();
}    

void CPDFSDK_Annot::SetContents(const CFX_WideString& sContents)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	if (sContents.IsEmpty())
		m_pAnnot->m_pAnnotDict->RemoveAt("Contents");
	else
		m_pAnnot->m_pAnnotDict->SetAtString("Contents", PDF_EncodeText(sContents));
}

CFX_WideString CPDFSDK_Annot::GetContents() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	return m_pAnnot->m_pAnnotDict->GetUnicodeText("Contents");
}

void CPDFSDK_Annot::SetAnnotName(const CFX_WideString& sName)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	if (sName.IsEmpty())
		m_pAnnot->m_pAnnotDict->RemoveAt("NM");
	else
		m_pAnnot->m_pAnnotDict->SetAtString("NM", PDF_EncodeText(sName));
}

CFX_WideString CPDFSDK_Annot::GetAnnotName() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	return m_pAnnot->m_pAnnotDict->GetUnicodeText("NM");
}

void CPDFSDK_Annot::SetModifiedDate(const FX_SYSTEMTIME& st)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	CPDFSDK_DateTime dt(st);
	CFX_ByteString str = dt.ToPDFDateTimeString();
	
	if (str.IsEmpty())
		m_pAnnot->m_pAnnotDict->RemoveAt("M");
	else
		m_pAnnot->m_pAnnotDict->SetAtString("M", str);
}

FX_SYSTEMTIME CPDFSDK_Annot::GetModifiedDate() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	FX_SYSTEMTIME systime;	
	CFX_ByteString str = m_pAnnot->m_pAnnotDict->GetString("M");
	
 	CPDFSDK_DateTime dt(str);
 	dt.ToSystemTime(systime);
	
	return systime;
}

void CPDFSDK_Annot::SetFlags(int nFlags)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	m_pAnnot->m_pAnnotDict->SetAtInteger("F", nFlags);
}

int CPDFSDK_Annot::GetFlags() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	return m_pAnnot->m_pAnnotDict->GetInteger("F");
}

void CPDFSDK_Annot::SetAppState(const CFX_ByteString& str)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	if (str.IsEmpty())
		m_pAnnot->m_pAnnotDict->RemoveAt("AS");
	else
		m_pAnnot->m_pAnnotDict->SetAtString("AS", str);
}

CFX_ByteString CPDFSDK_Annot::GetAppState() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	return m_pAnnot->m_pAnnotDict->GetString("AS");
}

void CPDFSDK_Annot::SetStructParent(int key)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	m_pAnnot->m_pAnnotDict->SetAtInteger("StructParent", key);
}

int	CPDFSDK_Annot::GetStructParent() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	return m_pAnnot->m_pAnnotDict->GetInteger("StructParent");
}

//border
void CPDFSDK_Annot::SetBorderWidth(int nWidth)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Array* pBorder = m_pAnnot->m_pAnnotDict->GetArray("Border");

	if (pBorder)
	{
		pBorder->SetAt(2, new CPDF_Number(nWidth));
	}
	else
	{
		CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");

		if (!pBSDict)
		{
			pBSDict = new CPDF_Dictionary;
			m_pAnnot->m_pAnnotDict->SetAt("BS", pBSDict);
		}

		pBSDict->SetAtInteger("W", nWidth);
	}
}

int	CPDFSDK_Annot::GetBorderWidth() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Array* pBorder = m_pAnnot->m_pAnnotDict->GetArray("Border");

	if (pBorder)
	{
		return pBorder->GetInteger(2);
	}
	else
	{
		CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");

		if (pBSDict)
		{
			return pBSDict->GetInteger("W", 1);
		}
	}
	return 1;
}

void CPDFSDK_Annot::SetBorderStyle(int nStyle)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");
	if (!pBSDict)
	{
		pBSDict = new CPDF_Dictionary;
		m_pAnnot->m_pAnnotDict->SetAt("BS", pBSDict);
	}

	switch (nStyle)
	{
	case BBS_SOLID:
		pBSDict->SetAtName("S", "S");
		break;
	case BBS_DASH:
		pBSDict->SetAtName("S", "D");
		break;
	case BBS_BEVELED:
		pBSDict->SetAtName("S", "B");
		break;
	case BBS_INSET:
		pBSDict->SetAtName("S", "I");
		break;
	case BBS_UNDERLINE:
		pBSDict->SetAtName("S", "U");
		break;
	}
}

int	CPDFSDK_Annot::GetBorderStyle() const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");
	if (pBSDict)
	{
		CFX_ByteString sBorderStyle = pBSDict->GetString("S", "S");
		if (sBorderStyle == "S") return BBS_SOLID;
		if (sBorderStyle == "D") return BBS_DASH;
		if (sBorderStyle == "B") return BBS_BEVELED;
		if (sBorderStyle == "I") return BBS_INSET;
		if (sBorderStyle == "U") return BBS_UNDERLINE;
	}

	CPDF_Array* pBorder = m_pAnnot->m_pAnnotDict->GetArray("Border");
	if (pBorder)
	{
		if (pBorder->GetCount() >= 4) 
		{ 
			CPDF_Array *pDP = pBorder->GetArray(3);
			if (pDP && pDP->GetCount() > 0)
				return BBS_DASH;
		}
	}

	return BBS_SOLID;
}

void CPDFSDK_Annot::SetBorderDash(const CFX_IntArray& array)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");
	if (!pBSDict)
	{
		pBSDict = new CPDF_Dictionary;
		m_pAnnot->m_pAnnotDict->SetAt("BS", pBSDict);
	}

	CPDF_Array* pArray = new CPDF_Array;
	for (int i=0,sz=array.GetSize(); i<sz; i++)
	{
		pArray->AddInteger(array[i]);
	}

	pBSDict->SetAt("D", pArray);
}

void CPDFSDK_Annot::GetBorderDash(CFX_IntArray& array) const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Array* pDash = NULL;

	CPDF_Array* pBorder = m_pAnnot->m_pAnnotDict->GetArray("Border");
	if (pBorder)
	{
		pDash = pBorder->GetArray(3);
	}
	else
	{
		CPDF_Dictionary* pBSDict = m_pAnnot->m_pAnnotDict->GetDict("BS");
		if (pBSDict)
		{
			pDash = pBSDict->GetArray("D");
		}
	}

	if (pDash)
	{
		for (int i=0,sz=pDash->GetCount(); i<sz; i++)
		{
			array.Add(pDash->GetInteger(i));
		}
	}
}

void CPDFSDK_Annot::SetColor(FX_COLORREF color)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Array* pArray = new CPDF_Array;
	pArray->AddNumber((FX_FLOAT)FXSYS_GetRValue(color) / 255.0f);
	pArray->AddNumber((FX_FLOAT)FXSYS_GetGValue(color) / 255.0f);
	pArray->AddNumber((FX_FLOAT)FXSYS_GetBValue(color) / 255.0f);
	m_pAnnot->m_pAnnotDict->SetAt("C", pArray);
}

void CPDFSDK_Annot::RemoveColor()
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	m_pAnnot->m_pAnnotDict->RemoveAt("C") ; 
}

FX_BOOL CPDFSDK_Annot::GetColor(FX_COLORREF& color) const
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	if (CPDF_Array* pEntry = m_pAnnot->m_pAnnotDict->GetArray("C"))		
	{
		int nCount = pEntry->GetCount();
		if (nCount == 1)
		{
			FX_FLOAT g = pEntry->GetNumber(0) * 255;

			color = FXSYS_RGB((int)g, (int)g, (int)g);

			return TRUE;
		}
		else if (nCount == 3)
		{
			FX_FLOAT r = pEntry->GetNumber(0) * 255;
			FX_FLOAT g = pEntry->GetNumber(1) * 255;
			FX_FLOAT b = pEntry->GetNumber(2) * 255;

			color = FXSYS_RGB((int)r, (int)g, (int)b);

			return TRUE;
		}
		else if (nCount == 4)
		{
			FX_FLOAT c = pEntry->GetNumber(0);
			FX_FLOAT m = pEntry->GetNumber(1);
			FX_FLOAT y = pEntry->GetNumber(2);
			FX_FLOAT k = pEntry->GetNumber(3);

			FX_FLOAT r = 1.0f - FX_MIN(1.0f, c + k);
			FX_FLOAT g = 1.0f - FX_MIN(1.0f, m + k);
			FX_FLOAT b = 1.0f - FX_MIN(1.0f, y + k);

			color = FXSYS_RGB((int)(r * 255), (int)(g * 255), (int)(b * 255));

			return TRUE;
		}
	}

	return FALSE;
}


void CPDFSDK_Annot::WriteAppearance(const CFX_ByteString& sAPType, const CPDF_Rect& rcBBox, 
								const CPDF_Matrix& matrix, const CFX_ByteString& sContents,
								const CFX_ByteString& sAPState)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	CPDF_Dictionary* pAPDict = m_pAnnot->m_pAnnotDict->GetDict("AP");
	
	if (!pAPDict) 
	{
		pAPDict = new CPDF_Dictionary;
		m_pAnnot->m_pAnnotDict->SetAt("AP", pAPDict);
	}
	
	CPDF_Stream* pStream = NULL;
	CPDF_Dictionary* pParentDict = NULL;
	
	if (sAPState.IsEmpty())
	{
		pParentDict = pAPDict;
		pStream = pAPDict->GetStream(sAPType);
	}
	else
	{
		CPDF_Dictionary* pAPTypeDict = pAPDict->GetDict(sAPType);
		if (!pAPTypeDict)
		{
			pAPTypeDict = new CPDF_Dictionary;
			pAPDict->SetAt(sAPType, pAPTypeDict);
		}
		
		pParentDict = pAPTypeDict;
		pStream = pAPTypeDict->GetStream(sAPState);
	}
	
	if (!pStream)
	{
		CPDF_Document* pDoc = m_pPageView->GetPDFDocument();
		pStream = new CPDF_Stream(NULL, 0, NULL);
		FX_INT32 objnum = pDoc->AddIndirectObject(pStream);
		pParentDict->SetAtReference(sAPType, pDoc, objnum);
	}

	CPDF_Dictionary *pStreamDict = pStream->GetDict();
	if (!pStreamDict)
	{
		pStreamDict = new CPDF_Dictionary;
		pStreamDict->SetAtName("Type", "XObject");
		pStreamDict->SetAtName("Subtype", "Form");
		pStreamDict->SetAtInteger("FormType", 1);
		pStream->InitStream(NULL,0,pStreamDict);
	}
	
	if (pStreamDict)
	{
		pStreamDict->SetAtMatrix("Matrix",matrix);	
		pStreamDict->SetAtRect("BBox", rcBBox);		
	}
	
	pStream->SetData((FX_BYTE*)sContents.c_str(), sContents.GetLength(), FALSE, FALSE);
}

#define BA_ANNOT_MINWIDTH			1
#define BA_ANNOT_MINHEIGHT			1

FX_FLOAT CPDFSDK_Annot::GetMinWidth() const
{
	return BA_ANNOT_MINWIDTH;
}

FX_FLOAT CPDFSDK_Annot::GetMinHeight() const
{
	return BA_ANNOT_MINHEIGHT;
}

FX_BOOL CPDFSDK_Annot::CreateFormFiller()
{
	return TRUE;
}
FX_BOOL	CPDFSDK_Annot::IsVisible() const
{
	int nFlags = GetFlags();
	return !((nFlags & ANNOTFLAG_INVISIBLE) || (nFlags & ANNOTFLAG_HIDDEN) || (nFlags & ANNOTFLAG_NOVIEW));
}

CPDF_Action CPDFSDK_Annot::GetAction() const
{
	return CPDF_Action(m_pAnnot->m_pAnnotDict->GetDict("A"));
}

void CPDFSDK_Annot::SetAction(const CPDF_Action& action)
{
	ASSERT(action);
	if ((CPDF_Action&)action != CPDF_Action(m_pAnnot->m_pAnnotDict->GetDict("A")))
	{
		CPDF_Document* pDoc = m_pPageView->GetPDFDocument();
		CPDF_Dictionary* pDict = action.GetDict();
		if (pDict && pDict->GetObjNum() == 0) {
			pDoc->AddIndirectObject(pDict);
		}
		m_pAnnot->m_pAnnotDict->SetAtReference("A", pDoc, pDict->GetObjNum());
	}
}

void CPDFSDK_Annot::RemoveAction()
{
	m_pAnnot->m_pAnnotDict->RemoveAt("A");
}

CPDF_AAction CPDFSDK_Annot::GetAAction() const
{
	return m_pAnnot->m_pAnnotDict->GetDict("AA");
}

void CPDFSDK_Annot::SetAAction(const CPDF_AAction& aa)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	ASSERT(aa != NULL);
	
	if ((CPDF_AAction&)aa != m_pAnnot->m_pAnnotDict->GetDict("AA"))
		m_pAnnot->m_pAnnotDict->SetAt("AA", (CPDF_AAction&)aa);
}

void CPDFSDK_Annot::RemoveAAction()
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	m_pAnnot->m_pAnnotDict->RemoveAt("AA");
}

CPDF_Action	CPDFSDK_Annot::GetAAction(CPDF_AAction::AActionType eAAT)
{
	CPDF_AAction AAction = GetAAction();

	if (AAction.ActionExist(eAAT))
		return AAction.GetAction(eAAT);

	if (eAAT == CPDF_AAction::ButtonUp)
		return GetAction();

	return CPDF_Action();
}

void  CPDFSDK_Annot::Annot_OnDraw(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, CPDF_RenderOptions* pOptions)
{
	
	m_pAnnot->GetAPForm(m_pPageView->GetPDFPage(), CPDF_Annot::Normal);
	m_pAnnot->DrawAppearance(m_pPageView->GetPDFPage(), pDevice, pUser2Device, CPDF_Annot::Normal, NULL);

	return ;
}

CPDF_Page* CPDFSDK_Annot::GetPDFPage()
{
	if(m_pPageView)
		return m_pPageView->GetPDFPage();
	return NULL;
}

