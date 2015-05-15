// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/PublicMethods.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/resource.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/util.h"
#include "../../include/javascript/Field.h"
#include "../../include/javascript/color.h"
#include "../../include/javascript/JS_Runtime.h"

static v8::Isolate* GetIsolate(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	return pRuntime->GetIsolate();
}


/* -------------------------------- CJS_PublicMethods -------------------------------- */

#define DOUBLE_CORRECT	0.000000000000001

BEGIN_JS_STATIC_GLOBAL_FUN(CJS_PublicMethods)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFNumber_Format)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFNumber_Keystroke)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFPercent_Format)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFPercent_Keystroke)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFDate_FormatEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFDate_KeystrokeEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFDate_Format)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFDate_Keystroke)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFTime_FormatEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFTime_KeystrokeEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFTime_Format)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFTime_Keystroke)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFSpecial_Format)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFSpecial_Keystroke)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFSpecial_KeystrokeEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFSimple)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFMakeNumber)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFSimple_Calculate)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFRange_Validate)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFMergeChange)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFParseDateEx)
	JS_STATIC_GLOBAL_FUN_ENTRY(AFExtractNums)
END_JS_STATIC_GLOBAL_FUN()

IMPLEMENT_JS_STATIC_GLOBAL_FUN(CJS_PublicMethods)

struct stru_TbConvert
{
	FX_LPCSTR lpszJSMark;
	FX_LPCSTR lpszCppMark;
};

static const stru_TbConvert fcTable[] = {
	{ "mmmm","%B" },
	{ "mmm", "%b" },
	{ "mm",  "%m" },
	//"m"
	{ "dddd","%A" },
	{ "ddd", "%a" },
	{ "dd",  "%d" },
	//"d",   "%w",
	{ "yyyy","%Y" },
	{ "yy",  "%y" },
	{ "HH",  "%H" },
	//"H"
	{ "hh",  "%I" },
	//"h"
	{ "MM",  "%M" },
	//"M"
	{ "ss",  "%S" },
	//"s
	{ "tt",  "%p" },
	//"t"
};

static FX_LPCWSTR months[] =
{
	L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
};

static FX_LPCWSTR fullmonths[] =
{
	L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December"
};

FX_BOOL CJS_PublicMethods::IsNumber(FX_LPCWSTR string)
{
	CFX_WideString sTrim = StrTrim(string);
	FX_LPCWSTR pTrim = sTrim.c_str();
	FX_LPCWSTR p = pTrim;


	FX_BOOL bDot = FALSE;
	FX_BOOL bKXJS = FALSE;

	wchar_t c;
	while ((c = *p))
	{
		if (c == '.' || c == ',')
		{
			if (bDot) return FALSE;
			bDot = TRUE;
		}
		else if (c == '-' || c == '+')
		{
			if (p != pTrim)
				return FALSE;
		}
		else if (c == 'e' || c == 'E')
		{
			if (bKXJS) return FALSE;

			p++;
			c = *p;
			if (c == '+' || c == '-')
			{
				bKXJS = TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else if (!IsDigit(c))
		{
			return FALSE;
		}
		p++;
	}

	return TRUE;
}

FX_BOOL CJS_PublicMethods::IsDigit(wchar_t ch)
{
	return (ch >= L'0' && ch <= L'9');
}

FX_BOOL CJS_PublicMethods::IsDigit(char ch)
{
	return (ch >= '0' && ch <= '9');
}

FX_BOOL CJS_PublicMethods::IsAlphabetic(wchar_t ch)
{
	return ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z'));
}

FX_BOOL CJS_PublicMethods::IsAlphaNumeric(wchar_t ch)
{
	return (IsDigit(ch) || IsAlphabetic(ch));
}

FX_BOOL CJS_PublicMethods::maskSatisfied(wchar_t c_Change,wchar_t c_Mask)
{
	switch (c_Mask)
	{
	case L'9':
        return IsDigit(c_Change);
    case L'A':
        return IsAlphabetic(c_Change);
    case L'O':
        return IsAlphaNumeric(c_Change);
    case L'X':
        return TRUE;
	default:
        return (c_Change == c_Mask);
	}
}

FX_BOOL CJS_PublicMethods::isReservedMaskChar(wchar_t ch)
{
	return ch == L'9' || ch == L'A' || ch == L'O' || ch == L'X';
}

double CJS_PublicMethods::AF_Simple(FX_LPCWSTR sFuction, double dValue1, double dValue2)
{
	if (FXSYS_wcsicmp(sFuction,L"AVG") == 0 || FXSYS_wcsicmp(sFuction,L"SUM") == 0)
	{
		return dValue1 + dValue2;
	}
	else if (FXSYS_wcsicmp(sFuction, L"PRD") == 0)
	{
		return dValue1 * dValue2;
	}
	else if (FXSYS_wcsicmp(sFuction,L"MIN") == 0)
	{
		return FX_MIN(dValue1, dValue2);
	}
	else if (FXSYS_wcsicmp(sFuction,L"MAX") == 0)
	{
		return FX_MAX(dValue1, dValue2);
	}

	return dValue1;
}

CFX_WideString CJS_PublicMethods::StrLTrim(FX_LPCWSTR pStr)
{
	while (*pStr && *pStr == L' ') pStr++;

	return pStr;
}

CFX_WideString CJS_PublicMethods::StrRTrim(FX_LPCWSTR pStr)
{
	FX_LPCWSTR p = pStr;
	while (*p) p++;
	while (p > pStr && *(p - 1) == L' ') p--;

	return CFX_WideString(pStr, p - pStr);
}

CFX_WideString CJS_PublicMethods::StrTrim(FX_LPCWSTR pStr)
{
	return StrRTrim(StrLTrim(pStr).c_str());
}

CFX_ByteString CJS_PublicMethods::StrLTrim(FX_LPCSTR pStr)
{
	while (*pStr && *pStr == ' ') pStr++;

        return pStr;
}

CFX_ByteString CJS_PublicMethods::StrRTrim(FX_LPCSTR pStr)
{
	FX_LPCSTR p = pStr;
	while (*p) p++;
	while (p > pStr && *(p - 1) == L' ') p--;

	return CFX_ByteString(pStr,p-pStr);
}

CFX_ByteString CJS_PublicMethods::StrTrim(FX_LPCSTR pStr)
{
	return StrRTrim(StrLTrim(pStr));
}

double CJS_PublicMethods::ParseNumber(FX_LPCWSTR swSource, FX_BOOL& bAllDigits, FX_BOOL& bDot, FX_BOOL& bSign, FX_BOOL& bKXJS)
{
	bDot = FALSE;
	bSign = FALSE;
	bKXJS = FALSE;

	FX_BOOL bDigitExist = FALSE;

	FX_LPCWSTR p = swSource;
	wchar_t c;

	FX_LPCWSTR pStart = NULL;
	FX_LPCWSTR pEnd = NULL;

	while ((c = *p))
	{
		if (!pStart && c != L' ')
		{
			pStart = p;
		}

		pEnd = p;
		p++;
	}

	if (!pStart)
	{
		bAllDigits = FALSE;
		return 0;
	}

	while (pEnd != pStart)
	{
		if (*pEnd == L' ')
			pEnd --;
		else
			break;
	}

	double dRet = 0;
	p = pStart;
	bAllDigits = TRUE;
	CFX_WideString swDigits;

	while (p <= pEnd)
	{
		c = *p;

		if (IsDigit(c))
		{
			swDigits += c;
			bDigitExist = TRUE;
		}
		else
		{
			switch (c)
			{
			case L' ':
				bAllDigits = FALSE;
				break;
			case L'.':
			case L',':
				if (!bDot)
				{
					if (bDigitExist)
					{
						swDigits += L'.';
					}
					else
					{
						swDigits += L'0';
						swDigits += L'.';
						bDigitExist = TRUE;
					}

					bDot = TRUE;
					break;
				}
			case 'e':
			case 'E':
				if (!bKXJS)
				{
					p++;
					c = *p;
					if (c == '+' || c == '-')
					{
						bKXJS = TRUE;
						swDigits += 'e';
						swDigits += c;
					}
					break;
				}
			case L'-':
				if (!bDigitExist && !bSign)
				{
					swDigits += c;
					bSign = TRUE;
					break;
				}
			default:
				bAllDigits = FALSE;

				if (p != pStart && !bDot && bDigitExist)
				{
					swDigits += L'.';
					bDot = TRUE;
				}
				else
				{
					bDot = FALSE;
					bDigitExist = FALSE;
					swDigits = L"";
				}
				break;
			}
		}

		p++;
	}

	if (swDigits.GetLength() > 0 && swDigits.GetLength() < 17)
	{
		CFX_ByteString sDigits = swDigits.UTF8Encode();

		if (bKXJS)
		{
			dRet = atof(sDigits);
		}
		else
		{
			if (bDot)
			{
				char* pStopString;
				dRet = ::strtod(sDigits, &pStopString);
			}
			else
			{
				dRet = atol(sDigits);
			}
		}

	}

	return dRet;
}

double CJS_PublicMethods::ParseStringToNumber(FX_LPCWSTR swSource)
{
	FX_BOOL bAllDigits = FALSE;
	FX_BOOL bDot = FALSE;
	FX_BOOL bSign = FALSE;
	FX_BOOL bKXJS = FALSE;

	return ParseNumber(swSource, bAllDigits, bDot, bSign, bKXJS);
}

FX_BOOL	CJS_PublicMethods::ConvertStringToNumber(FX_LPCWSTR swSource, double & dRet, FX_BOOL & bDot)
{
	FX_BOOL bAllDigits = FALSE;
	FX_BOOL bSign = FALSE;
	FX_BOOL bKXJS = FALSE;

	dRet = ParseNumber(swSource, bAllDigits, bDot, bSign, bKXJS);

	return bAllDigits;
}

CJS_Array CJS_PublicMethods::AF_MakeArrayFromList(v8::Isolate* isolate, CJS_Value val)
{
	CJS_Array StrArray(isolate);
	if(val.IsArrayObject())
	{
		val.ConvertToArray(StrArray);
		return StrArray;
	}
	CFX_WideString wsStr = val.ToCFXWideString();
	CFX_ByteString t = CFX_ByteString::FromUnicode(wsStr);
	const char * p = (const char *)t;


	int ch = ',' ;
	int nIndex = 0;

	while (*p)
	{
		const char * pTemp = strchr(p, ch);
		if (pTemp == NULL)
		{
			StrArray.SetElement(nIndex, CJS_Value(isolate, StrTrim(p).c_str()));
			break;
		}
		else
		{
			char * pSub = new char[pTemp - p + 1];
			strncpy(pSub, p, pTemp - p);
			*(pSub + (pTemp - p)) = '\0';

			StrArray.SetElement(nIndex, CJS_Value(isolate, StrTrim(pSub).c_str()));
			delete []pSub;

			nIndex ++;
			p = ++pTemp;
		}

	}
	return StrArray;
}

int CJS_PublicMethods::ParseStringInteger(const CFX_WideString& string,int nStart,int& nSkip, int nMaxStep)
{
	int nRet = 0;
	nSkip = 0;
	for (int i=nStart, sz=string.GetLength(); i < sz; i++)
	{
		if (i-nStart > 10)
			break;

		FX_WCHAR c = string.GetAt(i);
		if (IsDigit((wchar_t)c))
		{
			nRet = nRet * 10 + (c - '0');
			nSkip = i - nStart + 1;
			if (nSkip >= nMaxStep)
				break;
		}
		else
			break;
	}

	return nRet;
}

CFX_WideString CJS_PublicMethods::ParseStringString(const CFX_WideString& string, int nStart, int& nSkip)
{
	CFX_WideString swRet;
	nSkip = 0;
	for (int i=nStart, sz=string.GetLength(); i < sz; i++)
	{
		FX_WCHAR c = string.GetAt(i);
		if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z'))
		{
			swRet += c;
			nSkip = i - nStart + 1;
		}
		else
			break;
	}

	return swRet;
}

double CJS_PublicMethods::ParseNormalDate(const CFX_WideString & value, FX_BOOL& bWrongFormat)
{
	double dt = JS_GetDateTime();

	int nYear = JS_GetYearFromTime(dt);
	int nMonth = JS_GetMonthFromTime(dt) + 1;
	int nDay = JS_GetDayFromTime(dt);
	int nHour = JS_GetHourFromTime(dt);
	int nMin = JS_GetMinFromTime(dt);
	int nSec = JS_GetSecFromTime(dt);

	int number[3];

	int nSkip = 0;
	int nLen = value.GetLength();
	int nIndex = 0;
	int i = 0;
	while (i < nLen)
	{
		if (nIndex > 2) break;

		FX_WCHAR c = value.GetAt(i);
		if (IsDigit((wchar_t)c))
		{
			number[nIndex++] = ParseStringInteger(value, i, nSkip, 4);
			i += nSkip;
		}
		else
		{
			i ++;
		}
	}

	if (nIndex == 2)
	{
		// case2: month/day
		// case3: day/month
		if ((number[0] >= 1 && number[0] <= 12) && (number[1] >= 1 && number[1] <= 31))
		{
			nMonth = number[0];
			nDay = number[1];
		}
		else if ((number[0] >= 1 && number[0] <= 31) && (number[1] >= 1 && number[1] <= 12))
		{
			nDay = number[0];
			nMonth = number[1];
		}

		bWrongFormat = FALSE;
	}
	else if (nIndex == 3)
	{
		// case1: year/month/day
		// case2: month/day/year
		// case3: day/month/year

		if (number[0] > 12 && (number[1] >= 1 && number[1] <= 12) && (number[2] >= 1 && number[2] <= 31))
		{
			nYear = number[0];
			nMonth = number[1];
			nDay = number[2];
		}
		else if ((number[0] >= 1 && number[0] <= 12) && (number[1] >= 1 && number[1] <= 31) && number[2] > 31)
		{
			nMonth = number[0];
			nDay = number[1];
			nYear = number[2];
		}
		else if ((number[0] >= 1 && number[0] <= 31) && (number[1] >= 1 && number[1] <= 12) && number[2] > 31)
		{
			nDay = number[0];
			nMonth = number[1];
			nYear = number[2];
		}

		bWrongFormat = FALSE;
	}
	else
	{
		bWrongFormat = TRUE;
		return dt;
	}

	CFX_WideString swTemp;
	swTemp.Format(L"%d/%d/%d %d:%d:%d",nMonth,nDay,nYear,nHour,nMin,nSec);
	return JS_DateParse(swTemp.c_str());
}

double CJS_PublicMethods::MakeRegularDate(const CFX_WideString & value, const CFX_WideString & format, FX_BOOL& bWrongFormat)
{
	double dt = JS_GetDateTime();

	if (format.IsEmpty() || value.IsEmpty())
		return dt;

	int nYear = JS_GetYearFromTime(dt);
	int nMonth = JS_GetMonthFromTime(dt) + 1;
	int nDay = JS_GetDayFromTime(dt);
	int nHour = JS_GetHourFromTime(dt);
	int nMin = JS_GetMinFromTime(dt);
	int nSec = JS_GetSecFromTime(dt);

	int nYearSub = 99; //nYear - 2000;

	FX_BOOL bPm = FALSE;
	FX_BOOL bExit = FALSE;
	bWrongFormat = FALSE;

	int i=0;
	int j=0;

	while (i < format.GetLength())
	{
		if (bExit) break;

		FX_WCHAR c = format.GetAt(i);
		switch (c)
		{
			case ':':
			case '.':
			case '-':
			case '\\':
			case '/':
				i++;
				j++;
				break;

			case 'y':
			case 'm':
			case 'd':
			case 'H':
			case 'h':
			case 'M':
			case 's':
			case 't':
				{
					int oldj = j;
					int nSkip = 0;
					int remaining = format.GetLength() - i - 1;

					if (remaining == 0 || format.GetAt(i+1) != c)
					{
						switch (c)
						{
							case 'y':
								i++;
								j++;
								break;
							case 'm':
								nMonth = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 'd':
								nDay = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 'H':
								nHour = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 'h':
								nHour = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 'M':
								nMin = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 's':
								nSec = ParseStringInteger(value, j, nSkip, 2);
								i++;
								j += nSkip;
								break;
							case 't':
								bPm = (j < value.GetLength() && value.GetAt(j) == 'p');
								i++;
								j++;
								break;
						}
					}
					else if (remaining == 1 || format.GetAt(i+2) != c)
					{
						switch (c)
						{
							case 'y':
								nYear = ParseStringInteger(value, j, nSkip, 4);
								i += 2;
								j += nSkip;
								break;
							case 'm':
								nMonth = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 'd':
								nDay = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 'H':
								nHour = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 'h':
								nHour = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 'M':
								nMin = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 's':
								nSec = ParseStringInteger(value, j, nSkip, 2);
								i += 2;
								j += nSkip;
								break;
							case 't':
								bPm = (j + 1 < value.GetLength() && value.GetAt(j) == 'p' && value.GetAt(j+1) == 'm');
								i += 2;
								j += 2;
								break;
						}
					}
					else if (remaining == 2 || format.GetAt(i+3) != c)
					{
						switch (c)
						{
							case 'm':
								{
									CFX_WideString sMonth = ParseStringString(value, j, nSkip);
									FX_BOOL bFind = FALSE;
									for (int m = 0; m < 12; m++)
									{
										if (sMonth.CompareNoCase(months[m]) == 0)
										{
											nMonth = m + 1;
											i+=3;
											j+=nSkip;
											bFind = TRUE;
											break;
										}
									}

									if (!bFind)
									{
										nMonth = ParseStringInteger(value, j, nSkip, 3);
										i+=3;
										j += nSkip;
									}
								}
								break;
							case 'y':
								break;
							default:
								i+=3;
								j+=3;
								break;
						}
					}
					else if (remaining == 3 || format.GetAt(i+4) != c)
					{
						switch (c)
						{


							case 'y':
								nYear = ParseStringInteger(value, j, nSkip, 4);
								j += nSkip;
								i += 4;
								break;
							case 'm':
								{
									FX_BOOL bFind = FALSE;

									CFX_WideString sMonth = ParseStringString(value, j, nSkip);
									sMonth.MakeLower();

									for (int m = 0; m < 12; m++)
									{
										CFX_WideString sFullMonths = fullmonths[m];
										sFullMonths.MakeLower();

										if (sFullMonths.Find(sMonth.c_str(), 0) != -1)
										{
											nMonth = m + 1;
											i += 4;
											j += nSkip;
											bFind = TRUE;
											break;
										}
									}

									if (!bFind)
									{
										nMonth = ParseStringInteger(value, j, nSkip, 4);
										i+=4;
										j += nSkip;
									}
								}
								break;
							default:
								i += 4;
								j += 4;
								break;
						}
					}
					else
					{
						if (j >= value.GetLength() || format.GetAt(i) != value.GetAt(j))
						{
							bWrongFormat = TRUE;
							bExit = TRUE;
						}
						i++;
						j++;
					}

					if (oldj == j)
					{
						bWrongFormat = TRUE;
						bExit = TRUE;
					}
				}

				break;
			default:
				if (value.GetLength() <= j)
				{
					bExit = TRUE;
				}
				else if (format.GetAt(i) != value.GetAt(j))
				{
					bWrongFormat = TRUE;
					bExit = TRUE;
				}

				i++;
				j++;
				break;
		}
	}

	if (bPm) nHour += 12;

	if (nYear >= 0 && nYear <= nYearSub)
		nYear += 2000;

	if (nMonth < 1 || nMonth > 12)
		bWrongFormat = TRUE;

	if (nDay < 1 || nDay > 31)
		bWrongFormat = TRUE;

	if (nHour < 0 || nHour > 24)
		bWrongFormat = TRUE;

	if (nMin < 0 || nMin > 60)
		bWrongFormat = TRUE;

	if (nSec < 0 || nSec > 60)
		bWrongFormat = TRUE;

	double dRet = 0;

	if (bWrongFormat)
	{
		dRet = ParseNormalDate(value, bWrongFormat);
	}
	else
	{
		dRet = JS_MakeDate(JS_MakeDay(nYear,nMonth - 1,nDay),JS_MakeTime(nHour, nMin, nSec, 0));

		if (JS_PortIsNan(dRet))
		{
			dRet = JS_DateParse(value.c_str());
		}
	}

	if (JS_PortIsNan(dRet))
	{
		dRet = ParseNormalDate(value, bWrongFormat);
	}

	return dRet;

}

CFX_WideString CJS_PublicMethods::MakeFormatDate(double dDate, const CFX_WideString & format)
{
	CFX_WideString sRet = L"",sPart = L"";

	int nYear = JS_GetYearFromTime(dDate);
	int nMonth = JS_GetMonthFromTime(dDate) + 1;
	int nDay = JS_GetDayFromTime(dDate);
	int nHour = JS_GetHourFromTime(dDate);
	int nMin = JS_GetMinFromTime(dDate);
	int nSec = JS_GetSecFromTime(dDate);

	int i = 0;
	while (i < format.GetLength())
	{
	        FX_WCHAR c = format.GetAt(i);
                int remaining = format.GetLength() - i - 1;
		sPart = L"";
		switch (c)
		{
			case 'y':
			case 'm':
			case 'd':
			case 'H':
			case 'h':
			case 'M':
			case 's':
			case 't':
				if (remaining == 0 || format.GetAt(i+1) != c)
				{
					switch (c)
					{
						case 'y':
							sPart += c;
							break;
						case 'm':
							sPart.Format(L"%d",nMonth);
							break;
						case 'd':
							sPart.Format(L"%d",nDay);
							break;
						case 'H':
							sPart.Format(L"%d",nHour);
							break;
						case 'h':
							sPart.Format(L"%d",nHour>12?nHour - 12:nHour);
							break;
						case 'M':
							sPart.Format(L"%d",nMin);
							break;
						case 's':
							sPart.Format(L"%d",nSec);
							break;
						case 't':
							sPart += nHour>12?'p':'a';
							break;
					}
					i++;
				}
				else if (remaining == 1 || format.GetAt(i+2) != c)
				{
					switch (c)
					{
						case 'y':
							sPart.Format(L"%02d",nYear - (nYear / 100) * 100);
							break;
						case 'm':
							sPart.Format(L"%02d",nMonth);
							break;
						case 'd':
							sPart.Format(L"%02d",nDay);
							break;
						case 'H':
							sPart.Format(L"%02d",nHour);
							break;
						case 'h':
							sPart.Format(L"%02d",nHour>12?nHour - 12:nHour);
							break;
						case 'M':
							sPart.Format(L"%02d",nMin);
							break;
						case 's':
							sPart.Format(L"%02d",nSec);
							break;
						case 't':
							sPart = nHour>12? L"pm": L"am";
							break;
					}
					i+=2;
				}
				else if (remaining == 2 || format.GetAt(i+3) != c)
				{
					switch (c)
					{
						case 'm':
							i+=3;
							if (nMonth > 0&&nMonth <= 12)
								sPart += months[nMonth - 1];
							break;
						default:
							i+=3;
							sPart += c;
							sPart += c;
							sPart += c;
							break;
					}
				}
				else if (remaining == 3 || format.GetAt(i+4) != c)
				{
					switch (c)
					{
						case 'y':
							sPart.Format(L"%04d",nYear);
							i += 4;
							break;
						case 'm':
							i+=4;
							if (nMonth > 0&&nMonth <= 12)
								sPart += fullmonths[nMonth - 1];
							break;
						default:
							i += 4;
							sPart += c;
							sPart += c;
							sPart += c;
							sPart += c;
							break;
					}
				}
				else
				{
					i++;
					sPart += c;
				}
				break;
			default:
				i++;
				sPart += c;
				break;
		}

		sRet += sPart;
	}

	return sRet;
}

/* -------------------------------------------------------------------------- */

//function AFNumber_Format(nDec, sepStyle, negStyle, currStyle, strCurrency, bCurrencyPrepend)
FX_BOOL CJS_PublicMethods::AFNumber_Format(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
#if _FX_OS_ != _FX_ANDROID_
	v8::Isolate* isolate = ::GetIsolate(cc);
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (params.size() != 6)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}
	if(!pEvent->m_pValue)
		return FALSE;
	CFX_WideString& Value = pEvent->Value();
	CFX_ByteString strValue = StrTrim(CFX_ByteString::FromUnicode(Value));

	if (strValue.IsEmpty()) return TRUE;

	int iDec = params[0].ToInt();
	int iSepStyle = params[1].ToInt();
	int iNegStyle = params[2].ToInt();
	// params[3] is iCurrStyle, it's not used.
	std::wstring wstrCurrency(params[4].ToCFXWideString().c_str());
	FX_BOOL bCurrencyPrepend = params[5].ToBool();

	if (iDec < 0) iDec = -iDec;

	if (iSepStyle < 0 || iSepStyle > 3)
		iSepStyle = 0;

	if (iNegStyle < 0 || iNegStyle > 3)
		iNegStyle = 0;


	//////////////////////////////////////////////////////
	//for processing decimal places
	strValue.Replace(",", ".");
	double dValue = atof(strValue);
	if (iDec > 0)
		dValue += DOUBLE_CORRECT;

	int iDec2;
	FX_BOOL bNegative = FALSE;

	strValue = fcvt(dValue,iDec,&iDec2,&bNegative);
	if (strValue.IsEmpty())
	{
		dValue = 0;
		strValue = fcvt(dValue,iDec,&iDec2,&bNegative);
		if (strValue.IsEmpty())
		{
			strValue = "0";
			iDec2 = 1;
		}

	}

	if (iDec2 < 0)
	{
		for (int iNum = 0;iNum < abs(iDec2);iNum++)
		{
			strValue = "0" + strValue;
		}
		iDec2 = 0;

	}
	int iMax = strValue.GetLength();
	if (iDec2 > iMax)
	{
		for (int iNum = 0;iNum <= iDec2 - iMax ;iNum++)
		{
			strValue += "0";
		}
		iMax = iDec2+1;
	}
	///////////////////////////////////////////////////////
    //for processing seperator style
	if (iDec2 < iMax)
	{
		if (iSepStyle == 0 || iSepStyle == 1)
		{
			strValue.Insert(iDec2, '.');
			iMax++;
		}
		else if (iSepStyle == 2 || iSepStyle == 3)
		{
			strValue.Insert(iDec2, ',');
			iMax++;
		}

		if (iDec2 == 0)
			strValue.Insert(iDec2, '0');
	}
	if (iSepStyle == 0 || iSepStyle == 2)
	{
		char cSeperator;
		if (iSepStyle == 0)
			cSeperator = ',';
		else
			cSeperator = '.';

		int iDecPositive;
		iDecPositive = iDec2;

		for (iDecPositive = iDec2 -3; iDecPositive > 0;iDecPositive -= 3)
		{
			strValue.Insert(iDecPositive, cSeperator);
			iMax++;
		}
	}

	//////////////////////////////////////////////////////////////////////
    //for processing currency string

	Value = CFX_WideString::FromLocal(strValue);
	std::wstring strValue2 = Value.c_str();

	if (bCurrencyPrepend)
		strValue2 = wstrCurrency + strValue2;
	else
		strValue2 = strValue2 + wstrCurrency;



	/////////////////////////////////////////////////////////////////////////
	//for processing negative style
	if (bNegative)
	{
		if (iNegStyle == 0)
		{
			strValue2.insert(0,L"-");
		}
		if (iNegStyle == 2 || iNegStyle == 3)
		{
			strValue2.insert(0,L"(");
			strValue2.insert(strValue2.length(),L")");
		}
		if (iNegStyle == 1 || iNegStyle == 3)
		{
			if (Field * fTarget = pEvent->Target_Field())
			{
				CJS_Array arColor(isolate);
				CJS_Value vColElm(isolate);
				vColElm = L"RGB";
				arColor.SetElement(0,vColElm);
				vColElm = 1;
				arColor.SetElement(1,vColElm);
				vColElm = 0;
				arColor.SetElement(2,vColElm);

				arColor.SetElement(3,vColElm);

				CJS_PropValue vProp(isolate);
				vProp.StartGetting();
				vProp<<arColor;
				vProp.StartSetting();
				fTarget->textColor(cc,vProp,sError);// red
			}
		}
	}
	else
	{
		if (iNegStyle == 1 || iNegStyle == 3)
		{
			if (Field *fTarget = pEvent->Target_Field())
			{
				CJS_Array arColor(isolate);
				CJS_Value vColElm(isolate);
				vColElm = L"RGB";
				arColor.SetElement(0,vColElm);
				vColElm = 0;
				arColor.SetElement(1,vColElm);
				arColor.SetElement(2,vColElm);
				arColor.SetElement(3,vColElm);

				CJS_PropValue vProp(isolate);
				vProp.StartGetting();
				fTarget->textColor(cc,vProp,sError);

				CJS_Array aProp(isolate);
				vProp.ConvertToArray(aProp);

				CPWL_Color crProp;
				CPWL_Color crColor;
				color::ConvertArrayToPWLColor(aProp, crProp);
				color::ConvertArrayToPWLColor(arColor, crColor);

				if (crColor != crProp)
				{
					CJS_PropValue vProp2(isolate);
					vProp2.StartGetting();
					vProp2<<arColor;
					vProp2.StartSetting();
     				fTarget->textColor(cc,vProp2,sError);
				}
			}
		}
	}
	Value = strValue2.c_str();
#endif
	return TRUE;
}

//function AFNumber_Keystroke(nDec, sepStyle, negStyle, currStyle, strCurrency, bCurrencyPrepend)
FX_BOOL CJS_PublicMethods::AFNumber_Keystroke(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if(params.size() < 2)
		return FALSE;
	int iSepStyle = params[1].ToInt();

	if (iSepStyle < 0 || iSepStyle > 3)
		iSepStyle = 0;
	if(!pEvent->m_pValue)
		return FALSE;
	CFX_WideString & val = pEvent->Value();
	CFX_WideString & w_strChange = pEvent->Change();
    CFX_WideString w_strValue = val;

	if (pEvent->WillCommit())
	{
		CFX_WideString wstrChange = w_strChange;
		CFX_WideString wstrValue = StrLTrim(w_strValue.c_str());
		if (wstrValue.IsEmpty())
			return TRUE;

		CFX_WideString swTemp = wstrValue;
		swTemp.Replace(L",", L".");
		if (!IsNumber(swTemp.c_str()))
		{
			pEvent->Rc() = FALSE;
			sError = JSGetStringFromID(pContext, IDS_STRING_JSAFNUMBER_KEYSTROKE);
			Alert(pContext, sError.c_str());
			return TRUE;
		}
		return TRUE; // it happens after the last keystroke and before validating,
	}

	std::wstring w_strValue2 = w_strValue.c_str();
	std::wstring w_strChange2 = w_strChange.c_str();
	std::wstring w_strSelected;
	if(-1 != pEvent->SelStart())
		w_strSelected = w_strValue2.substr(pEvent->SelStart(),(pEvent->SelEnd() - pEvent->SelStart()));
	FX_BOOL bHasSign = (w_strValue2.find('-') != -1) && (w_strSelected.find('-') == -1);
	if (bHasSign)
	{
		//can't insert "change" in front to sign postion.
		if (pEvent->SelStart() == 0)
		{
            FX_BOOL &bRc = pEvent->Rc();
			bRc = FALSE;
			return TRUE;
		}
	}

	char cSep = L'.';

	switch (iSepStyle)
	{
	case 0:
	case 1:
		cSep = L'.';
		break;
	case 2:
	case 3:
		cSep = L',';
		break;
	}

	FX_BOOL bHasSep = (w_strValue2.find(cSep) != -1);
	for (std::wstring::iterator it = w_strChange2.begin(); it != w_strChange2.end(); it++)
	{
		if (*it == cSep)
		{
			if (bHasSep)
			{
				FX_BOOL &bRc = pEvent->Rc();
				bRc = FALSE;
				return TRUE;
			}
			else
			{
				bHasSep = TRUE;
				continue;
			}
		}
		if (*it == L'-')
		{
			if (bHasSign)
			{
				FX_BOOL &bRc = pEvent->Rc();
				bRc = FALSE;
				return TRUE;
			}
			else if (it != w_strChange2.begin()) //sign's position is not correct
			{
				FX_BOOL &bRc = pEvent->Rc();
				bRc = FALSE;
				return TRUE;
			}
			else if (pEvent->SelStart() != 0)
			{
				FX_BOOL &bRc = pEvent->Rc();
				bRc = FALSE;
				return TRUE;
			}
			bHasSign = TRUE;
			continue;
		}

		if (!IsDigit(*it))
		{
			FX_BOOL &bRc = pEvent->Rc();
			bRc = FALSE;
			return TRUE;
		}
	}


	std::wstring w_prefix = w_strValue2.substr(0,pEvent->SelStart());
	std::wstring w_postfix;
	if (pEvent->SelEnd()<(int)w_strValue2.length())
		w_postfix  = w_strValue2.substr(pEvent->SelEnd());
	w_strValue2 = w_prefix + w_strChange2 + w_postfix;
	w_strValue = w_strValue2.c_str();
	val = w_strValue;
	return TRUE;

}

//function AFPercent_Format(nDec, sepStyle)
FX_BOOL CJS_PublicMethods::AFPercent_Format(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
#if _FX_OS_ != _FX_ANDROID_
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

    if (params.size() != 2)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}
	if(!pEvent->m_pValue)
		return FALSE;

	CFX_WideString& Value = pEvent->Value();
	CFX_ByteString strValue = StrTrim(CFX_ByteString::FromUnicode(Value));
	if (strValue.IsEmpty())
		return TRUE;

	int iDec = params[0].ToInt();
	if (iDec < 0)
		iDec = -iDec;

	int iSepStyle = params[1].ToInt();
	if (iSepStyle < 0 || iSepStyle > 3)
		iSepStyle = 0;

	//////////////////////////////////////////////////////
	//for processing decimal places
	double dValue = atof(strValue);
	dValue *= 100;
	if (iDec > 0)
		dValue += DOUBLE_CORRECT;//Ð£Õý

	int iDec2;
	FX_BOOL bNegative = FALSE;
	strValue = fcvt(dValue,iDec,&iDec2,&bNegative);
    if (strValue.IsEmpty())
	{
		dValue = 0;
		strValue = fcvt(dValue,iDec,&iDec2,&bNegative);
	}

	if (iDec2 < 0)
	{
		for (int iNum = 0; iNum < abs(iDec2); iNum++)
		{
			strValue = "0" + strValue;
		}
		iDec2 = 0;

	}
	int iMax = strValue.GetLength();
	if (iDec2 > iMax)
	{
		for (int iNum = 0; iNum <= iDec2 - iMax; iNum++)
		{
			strValue += "0";
		}
		iMax = iDec2+1;
	}
	///////////////////////////////////////////////////////
    //for processing seperator style
	if (iDec2 < iMax)
	{
		if (iSepStyle == 0 || iSepStyle == 1)
		{
			strValue.Insert(iDec2, '.');
			iMax++;
		}
		else if (iSepStyle == 2 || iSepStyle == 3)
		{
			strValue.Insert(iDec2, ',');
			iMax++;
		}

		if (iDec2 == 0)
			strValue.Insert(iDec2, '0');
	}
	if (iSepStyle == 0 || iSepStyle == 2)
	{
		char cSeperator;
		if (iSepStyle == 0)
			cSeperator = ',';
		else
			cSeperator = '.';

		int iDecPositive;
		iDecPositive = iDec2;

		for (iDecPositive = iDec2 -3; iDecPositive > 0; iDecPositive -= 3)
		{
			strValue.Insert(iDecPositive,cSeperator);
			iMax++;
		}
	}
	////////////////////////////////////////////////////////////////////
	//negative mark
	if(bNegative)
		strValue = "-" + strValue;
	strValue += "%";
	Value = CFX_WideString::FromLocal(strValue);
#endif
	return TRUE;
}
//AFPercent_Keystroke(nDec, sepStyle)
FX_BOOL CJS_PublicMethods::AFPercent_Keystroke(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return AFNumber_Keystroke(cc,params,vRet,sError);
}

//function AFDate_FormatEx(cFormat)
FX_BOOL CJS_PublicMethods::AFDate_FormatEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}
	if(!pEvent->m_pValue)
		return FALSE;

	CFX_WideString& val = pEvent->Value();
	CFX_WideString strValue = val;
	if (strValue.IsEmpty())
		return TRUE;

	CFX_WideString sFormat = params[0].ToCFXWideString();
	FX_BOOL bWrongFormat = FALSE;
	double dDate = 0.0f;

	if(strValue.Find(L"GMT") != -1)
	{
		//for GMT format time
		//such as "Tue Aug 11 14:24:16 GMT+08002009"
		dDate = MakeInterDate(strValue);
	}
	else
	{
		dDate = MakeRegularDate(strValue,sFormat,bWrongFormat);
	}

	if (JS_PortIsNan(dDate))
	{
		CFX_WideString swMsg;
		swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSPARSEDATE).c_str(), sFormat.c_str());
		Alert(pContext, swMsg.c_str());
		return FALSE;
	}

	val =  MakeFormatDate(dDate,sFormat);
	return TRUE;
}

double CJS_PublicMethods::MakeInterDate(CFX_WideString strValue)
{
	int nHour;
	int nMin;
	int nSec;
	int nYear;
	int nMonth;
	int nDay;

	CFX_WideStringArray wsArray;
	CFX_WideString sMonth = L"";
	CFX_WideString sTemp = L"";
	int nSize = strValue.GetLength();

	for(int i = 0; i < nSize; i++)
	{
		FX_WCHAR c = strValue.GetAt(i);
		if(c == L' ' || c == L':')
		{
			wsArray.Add(sTemp);
			sTemp = L"";
			continue;
		}

		sTemp += c;
	}

	wsArray.Add(sTemp);
	if(wsArray.GetSize() != 8)return 0;

	sTemp = wsArray[1];
	if(sTemp.Compare(L"Jan") == 0) nMonth = 1;
	if(sTemp.Compare(L"Feb") == 0) nMonth = 2;
	if(sTemp.Compare(L"Mar") == 0) nMonth = 3;
	if(sTemp.Compare(L"Apr") == 0) nMonth = 4;
	if(sTemp.Compare(L"May") == 0) nMonth = 5;
	if(sTemp.Compare(L"Jun") == 0) nMonth = 6;
	if(sTemp.Compare(L"Jul") == 0) nMonth = 7;
	if(sTemp.Compare(L"Aug") == 0) nMonth = 8;
	if(sTemp.Compare(L"Sep") == 0) nMonth = 9;
	if(sTemp.Compare(L"Oct") == 0) nMonth = 10;
	if(sTemp.Compare(L"Nov") == 0) nMonth = 11;
	if(sTemp.Compare(L"Dec") == 0) nMonth = 12;

	nDay = (int)ParseStringToNumber(wsArray[2].c_str());
	nHour = (int)ParseStringToNumber(wsArray[3].c_str());
	nMin = (int)ParseStringToNumber(wsArray[4].c_str());
	nSec = (int)ParseStringToNumber(wsArray[5].c_str());
	nYear = (int)ParseStringToNumber(wsArray[7].c_str());

	double dRet = JS_MakeDate(JS_MakeDay(nYear,nMonth - 1,nDay),JS_MakeTime(nHour, nMin, nSec, 0));

	if (JS_PortIsNan(dRet))
	{
		dRet = JS_DateParse(strValue.c_str());
	}

	return dRet;
}

//AFDate_KeystrokeEx(cFormat)
FX_BOOL CJS_PublicMethods::AFDate_KeystrokeEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (params.size() != 1)
	{
		sError = L"AFDate_KeystrokeEx's parameters' size r not correct";
		return FALSE;
	}

	if (pEvent->WillCommit())
	{
		if(!pEvent->m_pValue)
			return FALSE;
		CFX_WideString strValue = pEvent->Value();
		if (strValue.IsEmpty())
			return TRUE;

		CFX_WideString sFormat = params[0].ToCFXWideString();
		FX_BOOL bWrongFormat = FALSE;
		double dRet = MakeRegularDate(strValue,sFormat,bWrongFormat);
		if (bWrongFormat || JS_PortIsNan(dRet))
		{
			CFX_WideString swMsg;
			swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSPARSEDATE).c_str(), sFormat.c_str());
			Alert(pContext, swMsg.c_str());
			pEvent->Rc() = FALSE;
			return TRUE;
		}
	}
	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFDate_Format(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);

	if (params.size() != 1)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);

		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	int iIndex = params[0].ToInt();
	FX_LPCWSTR cFormats[] =  {L"m/d", L"m/d/yy", L"mm/dd/yy", L"mm/yy", L"d-mmm", L"d-mmm-yy", L"dd-mmm-yy",
		L"yy-mm-dd", L"mmm-yy", L"mmmm-yy", L"mmm d, yyyy", L"mmmm d, yyyy",
		L"m/d/yy h:MM tt", L"m/d/yy HH:MM" };

	ASSERT(iIndex < FX_ArraySize(cFormats));

	if (iIndex < 0)
		iIndex = 0;
	if (iIndex >= FX_ArraySize(cFormats))
		iIndex = 0;
	CJS_Parameters newParams;
	CJS_Value val(isolate,cFormats[iIndex]);
	newParams.push_back(val);
	return AFDate_FormatEx(cc,newParams,vRet,sError);
}

//AFDate_KeystrokeEx(cFormat)
FX_BOOL CJS_PublicMethods::AFDate_Keystroke(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);

	if (params.size() != 1)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);

		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	int iIndex = params[0].ToInt();
	FX_LPCWSTR cFormats[] =  {L"m/d", L"m/d/yy", L"mm/dd/yy", L"mm/yy", L"d-mmm", L"d-mmm-yy", L"dd-mmm-yy",
		L"yy-mm-dd", L"mmm-yy", L"mmmm-yy", L"mmm d, yyyy", L"mmmm d, yyyy",
		L"m/d/yy h:MM tt", L"m/d/yy HH:MM" };

	ASSERT(iIndex<FX_ArraySize(cFormats));

	if (iIndex < 0)
		iIndex = 0;
	if (iIndex >= FX_ArraySize(cFormats))
		iIndex = 0;
	CJS_Parameters newParams;
	CJS_Value val(isolate,cFormats[iIndex]);
	newParams.push_back(val);
	return AFDate_KeystrokeEx(cc,newParams,vRet,sError);
}

//function AFTime_Format(ptf)
FX_BOOL CJS_PublicMethods::AFTime_Format(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);

	if (params.size() != 1)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	int iIndex = params[0].ToInt();
	FX_LPCWSTR cFormats[] = {L"HH:MM", L"h:MM tt", L"HH:MM:ss", L"h:MM:ss tt"};

	ASSERT(iIndex<FX_ArraySize(cFormats));

	if (iIndex < 0)
		iIndex = 0;
	if (iIndex >= FX_ArraySize(cFormats))
		iIndex = 0;
	CJS_Parameters newParams;
	CJS_Value val(isolate,cFormats[iIndex]);
	newParams.push_back(val);
	return AFDate_FormatEx(cc,newParams,vRet,sError);
}

FX_BOOL CJS_PublicMethods::AFTime_Keystroke(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);
	if (params.size() != 1)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	int iIndex = params[0].ToInt();
	FX_LPCWSTR cFormats[] = {L"HH:MM", L"h:MM tt", L"HH:MM:ss", L"h:MM:ss tt"};

	ASSERT(iIndex<FX_ArraySize(cFormats));

	if (iIndex < 0)
		iIndex = 0;
	if (iIndex >= FX_ArraySize(cFormats))
		iIndex = 0;
	CJS_Parameters newParams;
	CJS_Value val(isolate,cFormats[iIndex]);
	newParams.push_back(val);
	return AFDate_KeystrokeEx(cc,newParams,vRet,sError);
}

FX_BOOL CJS_PublicMethods::AFTime_FormatEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return AFDate_FormatEx(cc,params,vRet,sError);
}

FX_BOOL CJS_PublicMethods::AFTime_KeystrokeEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return AFDate_KeystrokeEx(cc,params,vRet,sError);
}

//function AFSpecial_Format(psf)
FX_BOOL CJS_PublicMethods::AFSpecial_Format(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	std::string cFormat;
	int iIndex = params[0].ToInt();

	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if(!pEvent->m_pValue)
		return FALSE;
	CFX_WideString& Value = pEvent->Value();
	std::string strSrc = CFX_ByteString::FromUnicode(Value).c_str();

	switch (iIndex)
	{
	case 0:
		cFormat = "99999";
		break;
	case 1:
		cFormat = "99999-9999";
		break;
	case 2:
		{
			std::string NumberStr;
			util::printx("9999999999", strSrc,NumberStr);
			if (NumberStr.length() >= 10 )
				cFormat = "(999) 999-9999";
			else
				cFormat = "999-9999";
			break;
		}
	case 3:
		cFormat = "999-99-9999";
		break;
	}

	std::string strDes;
	util::printx(cFormat,strSrc,strDes);
	Value = CFX_WideString::FromLocal(strDes.c_str());
	return TRUE;
}


//function AFSpecial_KeystrokeEx(mask)
FX_BOOL CJS_PublicMethods::AFSpecial_KeystrokeEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();

	ASSERT(pEvent != NULL);

	if (params.size() < 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	if(!pEvent->m_pValue)
		return FALSE;
	CFX_WideString& valEvent = pEvent->Value();

	CFX_WideString wstrMask = params[0].ToCFXWideString();
	if (wstrMask.IsEmpty())
		return TRUE;

	std::wstring wstrValue = valEvent.c_str();

	if (pEvent->WillCommit())
	{
		if (wstrValue.empty())
			return TRUE;
		int iIndexMask = 0;
		for (std::wstring::iterator it = wstrValue.begin(); it != wstrValue.end(); it++)
		{
			wchar_t w_Value = *it;
			if (!maskSatisfied(w_Value,wstrMask[iIndexMask]))
				break;
			iIndexMask++;
		}

		if (iIndexMask != wstrMask.GetLength() || (iIndexMask != wstrValue.size() && wstrMask.GetLength() != 0))
		{
			Alert(pContext, JSGetStringFromID(pContext, IDS_STRING_JSAFNUMBER_KEYSTROKE).c_str());
			pEvent->Rc() = FALSE;
		}
		return TRUE;
	}

	CFX_WideString &wideChange = pEvent->Change();
	std::wstring wChange = wideChange.c_str();
	if (wChange.empty())
		return TRUE;

	int iIndexMask = pEvent->SelStart();

	if (wstrValue.length() - (pEvent->SelEnd()-pEvent->SelStart()) + wChange.length() > (FX_DWORD)wstrMask.GetLength())
	{
		Alert(pContext, JSGetStringFromID(pContext, IDS_STRING_JSPARAM_TOOLONG).c_str());
		pEvent->Rc() = FALSE;
		return TRUE;
	}

	if (iIndexMask >= wstrMask.GetLength() && (!wChange.empty()))
	{
		Alert(pContext, JSGetStringFromID(pContext, IDS_STRING_JSPARAM_TOOLONG).c_str());
		pEvent->Rc() = FALSE;
		return TRUE;
	}

	for (std::wstring::iterator it = wChange.begin(); it != wChange.end(); it++)
	{
		if (iIndexMask >= wstrMask.GetLength())
		{
			Alert(pContext, JSGetStringFromID(pContext, IDS_STRING_JSPARAM_TOOLONG).c_str());
			pEvent->Rc() = FALSE;
			return TRUE;
		}
		wchar_t w_Mask = wstrMask[iIndexMask];
		if (!isReservedMaskChar(w_Mask))
		{
			*it = w_Mask;
		}
		wchar_t w_Change = *it;
		if (!maskSatisfied(w_Change,w_Mask))
		{
			pEvent->Rc() = FALSE;
			return TRUE;
		}
		iIndexMask++;
	}

	wideChange = wChange.c_str();
	return TRUE;
}


//function AFSpecial_Keystroke(psf)
FX_BOOL CJS_PublicMethods::AFSpecial_Keystroke(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);

	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	std::string cFormat;
	int iIndex = params[0].ToInt();

	if(!pEvent->m_pValue)
		return FALSE;
	//CJS_Value val = pEvent->Value();
	CFX_WideString& val = pEvent->Value();
	std::string strSrc = CFX_ByteString::FromUnicode(val).c_str();
	std::wstring wstrChange = pEvent->Change().c_str();

	switch (iIndex)
	{
	case 0:
		cFormat = "99999";
		break;
	case 1:
		//cFormat = "99999-9999";
		cFormat = "999999999";
		break;
	case 2:
		{
			std::string NumberStr;
			util::printx("9999999999", strSrc,NumberStr);
			if (strSrc.length() + wstrChange.length() > 7 )
				//cFormat = "(999) 999-9999";
				cFormat = "9999999999";
			else
				//cFormat = "999-9999";
				cFormat = "9999999";
			break;
		}
	case 3:
		//cFormat = "999-99-9999";
		cFormat = "999999999";
		break;
	}

	CJS_Parameters params2;
	CJS_Value vMask(isolate, cFormat.c_str());
	params2.push_back(vMask);

    return AFSpecial_KeystrokeEx(cc,params2,vRet,sError);
}

FX_BOOL CJS_PublicMethods::AFMergeChange(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEventHandler = pContext->GetEventHandler();
	ASSERT(pEventHandler != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CFX_WideString swValue;
	if (pEventHandler->m_pValue != NULL)
		swValue = pEventHandler->Value();

	if (pEventHandler->WillCommit())
	{
		vRet = swValue.c_str();
		return TRUE;
	}

	CFX_WideString prefix,postfix;

	if (pEventHandler->SelStart() >= 0)
		prefix = swValue.Mid(0,pEventHandler->SelStart());
	else
		prefix = L"";


	if (pEventHandler->SelEnd() >= 0 && pEventHandler->SelEnd() <= swValue.GetLength())
		postfix = swValue.Mid(pEventHandler->SelEnd(), swValue.GetLength() - pEventHandler->SelEnd());
	else postfix = L"";

	vRet = (prefix + pEventHandler->Change() + postfix).c_str();

	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFParseDateEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	if (params.size() != 2)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CFX_WideString sValue = params[0].ToCFXWideString();
	CFX_WideString sFormat = params[1].ToCFXWideString();

	FX_BOOL bWrongFormat = FALSE;
	double dDate = MakeRegularDate(sValue,sFormat,bWrongFormat);

	if (JS_PortIsNan(dDate))
	{
		CFX_WideString swMsg;
		swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSPARSEDATE).c_str(), sFormat.c_str());
		Alert((CJS_Context *)cc, swMsg.c_str());
		return FALSE;
	}

	vRet = dDate;
	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFSimple(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	if (params.size() != 3)
	{
		CJS_Context* pContext = (CJS_Context *)cc;
		ASSERT(pContext != NULL);

		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	vRet = (double)AF_Simple(params[0].ToCFXWideString().c_str(), params[1].ToDouble(), params[2].ToDouble());
	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFMakeNumber(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	if (params.size() != 1)
	{
		CJS_Context* pContext = (CJS_Context *)cc;
		ASSERT(pContext != NULL);

		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}
	vRet = ParseStringToNumber(params[0].ToCFXWideString().c_str());
	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFSimple_Calculate(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);

	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	if (params.size() != 2)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CJS_Value params1 = params[1];

	if (!params1.IsArrayObject() && params1.GetType() != VT_string)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CPDFSDK_Document* pReaderDoc = pContext->GetReaderDocument();
    ASSERT(pReaderDoc != NULL);

	CPDFSDK_InterForm* pReaderInterForm = pReaderDoc->GetInterForm();
	ASSERT(pReaderInterForm != NULL);

	CPDF_InterForm* pInterForm = pReaderInterForm->GetInterForm();
	ASSERT(pInterForm != NULL);

	double dValue;
	CFX_WideString sFunction = params[0].ToCFXWideString();
	if (wcscmp(sFunction.c_str(), L"PRD") == 0)
    	dValue = 1.0;
	else
		dValue = 0.0;

	CJS_Array FieldNameArray = AF_MakeArrayFromList(isolate,params1);

	int nFieldsCount = 0;

	for (int i=0,isz=FieldNameArray.GetLength(); i<isz; i++)
	{
		CJS_Value jsValue(isolate);
		FieldNameArray.GetElement(i,jsValue);
		CFX_WideString wsFieldName = jsValue.ToCFXWideString();

        for (int j=0,jsz=pInterForm->CountFields(wsFieldName); j<jsz; j++)
		{
			if (CPDF_FormField* pFormField = pInterForm->GetField(j, wsFieldName))
			{
				double dTemp = 0.0;

				switch (pFormField->GetFieldType())
				{
				case FIELDTYPE_TEXTFIELD:
				case FIELDTYPE_COMBOBOX:
					{
						dTemp = ParseStringToNumber(pFormField->GetValue().c_str());
						break;
					}
				case FIELDTYPE_PUSHBUTTON:
					{
						dTemp = 0.0;
						break;
					}
				case FIELDTYPE_CHECKBOX:
				case FIELDTYPE_RADIOBUTTON:
					{
						dTemp = 0.0;
						for (int c=0,csz=pFormField->CountControls(); c<csz; c++)
						{
							if (CPDF_FormControl* pFormCtrl = pFormField->GetControl(c))
							{
								if (pFormCtrl->IsChecked())
								{
									dTemp += ParseStringToNumber(pFormCtrl->GetExportValue().c_str());
									break;
								}
								else
									continue;
							}
						}
						break;
					}
				case FIELDTYPE_LISTBOX:
					{
						dTemp = 0.0;
						if (pFormField->CountSelectedItems() > 1)
							break;
						else
						{
							dTemp = ParseStringToNumber(pFormField->GetValue().c_str());
							break;
						}
					}
				default:
					break;
				}

				if (i == 0 && j == 0 && (wcscmp(sFunction.c_str(), L"MIN") == 0 || wcscmp(sFunction.c_str(), L"MAX") == 0))
					dValue = dTemp;

				dValue = AF_Simple(sFunction.c_str(), dValue, dTemp);

				nFieldsCount++;
			}
		}
	}

	if (wcscmp(sFunction.c_str(), L"AVG") == 0 && nFieldsCount > 0)
		dValue /= nFieldsCount;

	dValue = (double)floor(dValue * FXSYS_pow((double)10,(double)6) + 0.49) / FXSYS_pow((double)10,(double)6);
	CJS_Value jsValue(isolate,dValue);
	if((CJS_EventHandler*)pContext->GetEventHandler()->m_pValue)
		((CJS_EventHandler*)pContext->GetEventHandler())->Value() = jsValue.ToCFXWideString();

	return TRUE;
}

/* This function validates the current event to ensure that its value is
** within the specified range. */

FX_BOOL CJS_PublicMethods::AFRange_Validate(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (params.size() != 4)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	if(!pEvent->m_pValue)
		return FALSE;
	if (pEvent->Value().IsEmpty() )
		return TRUE;
	double dEentValue = atof(CFX_ByteString::FromUnicode(pEvent->Value()));
	FX_BOOL bGreaterThan = params[0].ToBool();
	double dGreaterThan = params[1].ToDouble();
	FX_BOOL bLessThan = params[2].ToBool();
	double dLessThan = params[3].ToDouble();
	CFX_WideString swMsg;

	if (bGreaterThan && bLessThan)
	{
		if (dEentValue < dGreaterThan || dEentValue > dLessThan)
			swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSRANGE1).c_str(),
						 params[1].ToCFXWideString().c_str(),
						 params[3].ToCFXWideString().c_str());
	}
	else if (bGreaterThan)
	{
		if (dEentValue < dGreaterThan)
			swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSRANGE2).c_str(),
						 params[1].ToCFXWideString().c_str());
	}
	else if (bLessThan)
	{
		if (dEentValue > dLessThan)
			swMsg.Format(JSGetStringFromID(pContext, IDS_STRING_JSRANGE3).c_str(),
						 params[3].ToCFXWideString().c_str());
	}

	if (!swMsg.IsEmpty())
	{
		Alert(pContext, swMsg.c_str());
		pEvent->Rc() = FALSE;
	}
	return TRUE;
}

FX_BOOL CJS_PublicMethods::AFExtractNums(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = ::GetIsolate(cc);
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CJS_Array nums(isolate);

	CFX_WideString str = params[0].ToCFXWideString();
	CFX_WideString sPart;

	if (str.GetAt(0) == L'.' || str.GetAt(0) == L',')
		str = L"0" + str;

	int nIndex = 0;
	for (int i=0, sz=str.GetLength(); i<sz; i++)
	{
		FX_WCHAR wc = str.GetAt(i);
		if (IsDigit((wchar_t)wc))
		{
			sPart += wc;
		}
		else
		{
			if (sPart.GetLength() > 0)
			{
				nums.SetElement(nIndex,CJS_Value(isolate,sPart.c_str()));
				sPart = L"";
				nIndex ++;
			}
		}
	}

	if (sPart.GetLength() > 0)
	{
		nums.SetElement(nIndex,CJS_Value(isolate,sPart.c_str()));
	}

	if (nums.GetLength() > 0)
		vRet = nums;
	else
		vRet.SetNull();

	return TRUE;
}
