// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/util.h"
#include "../../include/javascript/PublicMethods.h"
#include "../../include/javascript/resource.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_Runtime.h"

#if _FX_OS_  == _FX_ANDROID_
#include <ctype.h>
#endif

static v8::Isolate* GetIsolate(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	return pRuntime->GetIsolate();
}

BEGIN_JS_STATIC_CONST(CJS_Util)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Util)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Util)
	JS_STATIC_METHOD_ENTRY(printd)
	JS_STATIC_METHOD_ENTRY(printf)
	JS_STATIC_METHOD_ENTRY(printx)
	JS_STATIC_METHOD_ENTRY(scand)
	JS_STATIC_METHOD_ENTRY(byteToChar)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Util,util)

util::util(CJS_Object *pJSObject) : CJS_EmbedObj(pJSObject)
{
}

util::~util(void)
{
}


struct stru_TbConvert
{
	FX_LPCWSTR lpszJSMark;
	FX_LPCWSTR lpszCppMark;
};

const stru_TbConvert fcTable[] = {
	{ L"mmmm", L"%B" },
	{ L"mmm", L"%b" },
	{ L"mm",  L"%m" },
	//"m"
	{ L"dddd", L"%A" },
	{ L"ddd", L"%a" },
	{ L"dd",  L"%d" },
	//"d",   "%w",
	{ L"yyyy", L"%Y" },
	{ L"yy",  L"%y" },
	{ L"HH",  L"%H" },
	//"H"
	{ L"hh",  L"%I" },
	//"h"
	{ L"MM",  L"%M" },
	//"M"
	{ L"ss",  L"%S" },
	//"s
	{ L"TT",  L"%p" },
	//"t"
#if defined(_WIN32)
	{ L"tt",  L"%p" },
	{ L"h",  L"%#I" },
#else
	{ L"tt",  L"%P" },
	{ L"h",  L"%l" },
#endif
};

#define UTIL_INT			0
#define UTIL_DOUBLE			1
#define UTIL_STRING			2

int util::ParstDataType(std::wstring* sFormat)
{
        size_t i = 0;
	bool bPercent = FALSE;
	for (i=0; i<sFormat->length(); ++i)
	{
		wchar_t c = (*sFormat)[i];
		if (c == L'%')
		{
			bPercent = true;
			continue;
		}

		if (bPercent)
		{
			if (c == L'c' || c == L'C' || c == L'd' || c == L'i' || c == L'o' || c == L'u' || c == L'x' || c == L'X')
			{
				return UTIL_INT;
			}
			else if (c == L'e' || c == L'E' || c == L'f' || c == L'g' || c == L'G')
			{
				return UTIL_DOUBLE;
			}
			else if (c == L's' || c == L'S')
			{
				// Map s to S since we always deal internally
				// with wchar_t strings.
				(*sFormat)[i] = L'S';
				return UTIL_STRING;
			}
			else if (c == L'.' || c == L'+' || c == L'-' || c == L'#' || c == L' ' || CJS_PublicMethods::IsDigit(c))
			{
				continue;
			}
			else break;
		}
	}

	return -1;
}

FX_BOOL util::printf(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	int iSize = params.size();
	if (iSize < 1)
		return FALSE;
	std::wstring c_ConvChar(params[0].ToCFXWideString().c_str());
	std::vector<std::wstring> c_strConvers;
	int iOffset = 0;
	int iOffend = 0;
	c_ConvChar.insert(c_ConvChar.begin(),L'S');
	while(iOffset != -1)
	{
		iOffend = c_ConvChar.find(L"%",iOffset+1);
		std::wstring strSub;
		if (iOffend == -1)
			strSub = c_ConvChar.substr(iOffset);			
		else
			strSub = c_ConvChar.substr(iOffset ,iOffend - iOffset);
		c_strConvers.push_back(strSub);
		iOffset = iOffend ;
	}

	std::wstring c_strResult;

	//for(int iIndex = 1;iIndex < params.size();iIndex++)
	std::wstring c_strFormat;
	for(int iIndex = 0;iIndex < (int)c_strConvers.size();iIndex++)
	{
		c_strFormat = c_strConvers[iIndex];
		if (iIndex == 0)
		{
			c_strResult = c_strFormat;
			continue;
		}


		CFX_WideString strSegment;
		if (iIndex >= iSize) {
			c_strResult += c_strFormat;
			continue;
		}

		switch (ParstDataType(&c_strFormat))
		{
			case UTIL_INT:
				strSegment.Format(c_strFormat.c_str(), params[iIndex].ToInt());
				break;
			case UTIL_DOUBLE:
				strSegment.Format(c_strFormat.c_str(), params[iIndex].ToDouble());
				break;
			case UTIL_STRING:
				strSegment.Format(c_strFormat.c_str(), params[iIndex].ToCFXWideString().c_str());
				break;
			default:
				strSegment.Format(L"%S", c_strFormat.c_str());
				break;
		}
		c_strResult += strSegment.GetBuffer(strSegment.GetLength()+1);
	}

	c_strResult.erase(c_strResult.begin());
	vRet = c_strResult.c_str();
	return TRUE;
}

FX_BOOL util::printd(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = GetIsolate(cc);

	int iSize = params.size();
	if (iSize < 2)
		return FALSE;

	CJS_Value p1(isolate);
	p1 = params[0];

	CJS_Value p2 = params[1];
	CJS_Date jsDate(isolate);
	if (!p2.ConvertToDate(jsDate))
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPRINT1);
		return FALSE;
	}

	if (!jsDate.IsValidDate())
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPRINT2);
		return FALSE;
	}

	if (p1.GetType() == VT_number)
	{
		int nFormat = p1.ToInt();
		CFX_WideString swResult;

		switch (nFormat)
		{
		case 0:
			swResult.Format(L"D:%04d%02d%02d%02d%02d%02d", 
				jsDate.GetYear(),
				jsDate.GetMonth() + 1,
				jsDate.GetDay(),
				jsDate.GetHours(),
				jsDate.GetMinutes(),
				jsDate.GetSeconds());
			break;
		case 1:
			swResult.Format(L"%04d.%02d.%02d %02d:%02d:%02d", 
				jsDate.GetYear(),
				jsDate.GetMonth() + 1,
				jsDate.GetDay(),
				jsDate.GetHours(),
				jsDate.GetMinutes(),
				jsDate.GetSeconds());
			break;
		case 2:
			swResult.Format(L"%04d/%02d/%02d %02d:%02d:%02d", 
				jsDate.GetYear(),
				jsDate.GetMonth() + 1,
				jsDate.GetDay(),
				jsDate.GetHours(),
				jsDate.GetMinutes(),
				jsDate.GetSeconds());
			break;
		default:
			return FALSE;
		}

		vRet = swResult.c_str();
		return TRUE;
	}
	else if (p1.GetType() == VT_string)
	{
		std::basic_string<wchar_t> cFormat = p1.ToCFXWideString().c_str();

		bool bXFAPicture = false;
		if (iSize > 2)
		{
			bXFAPicture = params[2].ToBool();
		}

		if (bXFAPicture)
		{
			return FALSE; //currently, it doesn't support XFAPicture.
		}

        int iIndex;
		for(iIndex = 0;iIndex<sizeof(fcTable)/sizeof(stru_TbConvert);iIndex++)
		{
			int iStart = 0;
			int iEnd;
			while((iEnd = cFormat.find(fcTable[iIndex].lpszJSMark, iStart)) != -1)
			{
				cFormat.replace(iEnd, FXSYS_wcslen(fcTable[iIndex].lpszJSMark), fcTable[iIndex].lpszCppMark);
				iStart = iEnd;
			}
		}

		int iYear,iMonth,iDay,iHour,iMin,iSec;
		iYear = jsDate.GetYear();
		iMonth = jsDate.GetMonth();
		iDay = jsDate.GetDay();
		iHour = jsDate.GetHours();
		iMin = jsDate.GetMinutes();
		iSec = jsDate.GetSeconds();

		struct tm time = {0};
		time.tm_year = iYear-1900;
		time.tm_mon = iMonth;
		time.tm_mday = iDay;
		time.tm_hour = iHour;
		time.tm_min = iMin;
		time.tm_sec = iSec;
		//COleDateTime cppTm(iYear,iMonth+1,iDay,iHour,iMin,iSec);
		//CString strFormat = cppTm.Format(cFormat.c_str());

		struct stru_TbConvertAd
		{
			FX_LPCWSTR lpszJSMark;
			int     iValue;
		};

		stru_TbConvertAd cTableAd[] ={
			{ L"m", iMonth+1 },
			{ L"d", iDay },
			{ L"H", iHour },
			{ L"h", iHour>12?iHour-12:iHour },
			{ L"M", iMin },
			{ L"s", iSec },
		};

		//cFormat = strFormat.GetBuffer(strFormat.GetLength()+1);
		for(iIndex = 0;iIndex<sizeof(cTableAd)/sizeof(stru_TbConvertAd);iIndex++)
		{
			wchar_t tszValue[10];
			//_itot(cTableAd[iIndex].iValue,tszValue,10);
			CFX_WideString sValue;
			sValue.Format(L"%d",cTableAd[iIndex].iValue);
			memcpy(tszValue, (wchar_t *)sValue.GetBuffer(sValue.GetLength()+1),
                               (sValue.GetLength()+1)*sizeof(wchar_t));

			//strFormat.Replace(cTableAd[iIndex].lpszJSMark,"%d");
			//strFormat.Format(strFormat,cTableAd[iIndex].iValue);
			int iStart = 0;
			int iEnd;
			while((iEnd = cFormat.find(cTableAd[iIndex].lpszJSMark, iStart)) != -1)
			{
				if (iEnd > 0)
				{
					if (cFormat[iEnd-1] == L'%')
					{
						iStart = iEnd+1;
						continue;
					}
				}
				cFormat.replace(iEnd, FXSYS_wcslen(cTableAd[iIndex].lpszJSMark), tszValue);
				iStart = iEnd;
			}
		}

		CFX_WideString strFormat;
//		strFormat.Format(L"%d,%d,%d,%d,%d,%d",iYear, iMonth, iDay, iHour, iMin, iSec);
//		CString strFormat = cppTm.Format(cFormat.c_str());
		wchar_t buf[64] = {0};
		strFormat = wcsftime(buf, 64, cFormat.c_str(), &time);
		cFormat = buf;
		vRet = cFormat.c_str();
		//rtRet = strFormat.GetBuffer(strFormat.GetLength()+1);
		return TRUE;
	}
	return FALSE;
}

void util::printd(const std::wstring &cFormat2, CJS_Date jsDate, bool bXFAPicture, std::wstring &cPurpose)
{
	std::wstring cFormat = cFormat2;
	    
	if (bXFAPicture)
	{
		return ; //currently, it doesn't support XFAPicture.
	}

    int iIndex;
	for(iIndex = 0;iIndex<sizeof(fcTable)/sizeof(stru_TbConvert);iIndex++)
	{
		int iStart = 0;
		int iEnd;
		while((iEnd = cFormat.find(fcTable[iIndex].lpszJSMark, iStart)) != -1)
		{
			cFormat.replace(iEnd,FXSYS_wcslen(fcTable[iIndex].lpszJSMark), fcTable[iIndex].lpszCppMark);
			iStart = iEnd;
		}
	}

	int iYear,iMonth,iDay,iHour,iMin,iSec;
	iYear = jsDate.GetYear();
	iMonth = jsDate.GetMonth();
	iDay = jsDate.GetDay();
	iHour = jsDate.GetHours();
	iMin = jsDate.GetMinutes();
	iSec = jsDate.GetSeconds();

	struct tm time = {0};
	time.tm_year = iYear-1900;
	time.tm_mon = iMonth;
	time.tm_mday = iDay;
	time.tm_hour = iHour;
	time.tm_min = iMin;
	time.tm_sec = iSec;
//	COleDateTime cppTm(iYear,iMonth+1,iDay,iHour,iMin,iSec);
	//CString strFormat = cppTm.Format(cFormat.c_str());

	struct stru_TbConvertAd
	{
		FX_LPCWSTR lpszJSMark;
		int     iValue;
	};

	stru_TbConvertAd cTableAd[] ={
		{ L"m", iMonth+1 },
		{ L"d", iDay },
		{ L"H", iHour },
		{ L"h", iHour>12?iHour-12:iHour },
		{ L"M", iMin },
		{ L"s", iSec },
	};

	//cFormat = strFormat.GetBuffer(strFormat.GetLength()+1);
	for(iIndex = 0;iIndex<sizeof(cTableAd)/sizeof(stru_TbConvertAd);iIndex++)
	{
		wchar_t tszValue[10];
		//_itot(cTableAd[iIndex].iValue,tszValue,10);
		CFX_WideString sValue;
		sValue.Format(L"%d",cTableAd[iIndex].iValue);
		memcpy(tszValue, (wchar_t *)sValue.GetBuffer(sValue.GetLength()+1),sValue.GetLength()*sizeof(wchar_t));


		//strFormat.Replace(cTableAd[iIndex].lpszJSMark,"%d");
		//strFormat.Format(strFormat,cTableAd[iIndex].iValue);
		int iStart = 0;
		int iEnd;
		while((iEnd = cFormat.find(cTableAd[iIndex].lpszJSMark, iStart)) != -1)
		{
			if (iEnd > 0)
			{
				if (cFormat[iEnd-1] == L'%')
				{
					iStart = iEnd+1;
					continue;
				}
			}
			cFormat.replace(iEnd,FXSYS_wcslen(cTableAd[iIndex].lpszJSMark),tszValue);
			iStart = iEnd;
		}
	}

	CFX_WideString strFormat;
	wchar_t buf[64] = {0};
	strFormat = wcsftime(buf, 64, cFormat.c_str(), &time);
	cFormat = buf;
	cPurpose = cFormat;
}

FX_BOOL util::printx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	int iSize = params.size();
	if (iSize<2)
		return FALSE;
	CFX_WideString sFormat = params[0].ToCFXWideString();
	CFX_WideString sSource = params[1].ToCFXWideString();
	std::string cFormat = CFX_ByteString::FromUnicode(sFormat).c_str();
	std::string cSource = CFX_ByteString::FromUnicode(sSource).c_str();
	std::string cDest;
	printx(cFormat,cSource,cDest);
	vRet = cDest.c_str();
	return TRUE;
}

void util::printx(const std::string &cFormat,const std::string &cSource2,std::string &cPurpose)
{
	std::string cSource(cSource2);
	if (!cPurpose.empty())
		//cPurpose.clear();
		cPurpose.erase();
	int itSource = 0;
	int iSize = cSource.size();
	for(int iIndex = 0; iIndex < (int)cFormat.size() && itSource<iSize; iIndex++)
	{
		char letter = cFormat[iIndex];
		switch(letter)
		{
		case '?':
			//cPurpose.push_back(cSource[itSource]);
			cPurpose += cSource[itSource];
			itSource++;
			break;
		case 'X':
			{
				while(itSource < iSize)
				{
					if ((cSource[itSource]>='0'&&cSource[itSource]<='9') || (cSource[itSource]>='a' && cSource[itSource]<='z') || (cSource[itSource]>='A' && cSource[itSource]<='Z'))
					{
						//cPurpose.push_back(cSource[itSource]);
						cPurpose += cSource[itSource];
						itSource++;
						break;
					}
					itSource++;
				}
				break;
			}
			break;
		case 'A':
			{
				while(itSource < iSize)
				{
					if ((cSource[itSource]>='a' && cSource[itSource]<='z') || (cSource[itSource]>='A' && cSource[itSource]<='Z'))
					{
						//cPurpose.push_back(cSource[itSource]);
						cPurpose += cSource[itSource];
						itSource++;
						break;
					}
					itSource++;
				}
				break;
			}
			break;
		case '9':
			{
				while(itSource < iSize)
				{
					if (cSource[itSource]>='0'&&cSource[itSource]<='9')
					{
						//cPurpose.push_back(cSource[itSource]);
						cPurpose += cSource[itSource];
						itSource++;
						break;
					}
					itSource++;
				}
				break;
			}
		case '*':
			{
				cPurpose.append(cSource,itSource,iSize-itSource);
				itSource = iSize-1;
				break;
			}
		case '\\':
			break;
		case '>':
			{
				for(std::string::iterator it = cSource.begin();it != cSource.end(); it++)
				{
					*it = toupper(*it);
				}
				break;
			}
		case '<':
			{
				for(std::string::iterator it = cSource.begin();it != cSource.end(); it++)
				{
					*it = tolower(*it);
				}
				break;
			}
		case '=':
			break;
		default:
			//cPurpose.push_back(letter);
			cPurpose += letter;
			break;
		}
	}
}

FX_BOOL util::scand(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	v8::Isolate* isolate = GetIsolate(cc);
	int iSize = params.size();
	if (iSize < 2)
		return FALSE;

	CFX_WideString sFormat = params[0].ToCFXWideString();
	CFX_WideString sDate = params[1].ToCFXWideString();
	double dDate = JS_GetDateTime();
	if (sDate.GetLength() > 0)
	{
		FX_BOOL bWrongFormat = FALSE;
		dDate = CJS_PublicMethods::MakeRegularDate(sDate,sFormat,bWrongFormat);
	}	
	
	if (!JS_PortIsNan(dDate))
	{
		CJS_Date date(isolate,dDate);
		vRet = date;
	}
	else
	{
		vRet.SetNull();
	}

	return TRUE;
}

FX_INT64 FX_atoi64(const char *nptr)
{
        int c;              /* current char */
        FX_INT64 total;      /* current total */
        int sign;           /* if '-', then negative, otherwise positive */

        /* skip whitespace */
        while ( isspace((int)(unsigned char)*nptr) )
            ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;           /* save sign indication */
        if (c == '-' || c == '+')
            c = (int)(unsigned char)*nptr++;    /* skip sign */

        total = 0;

        while (isdigit(c)) {
            total = 10 * total + (c - '0');     /* accumulate digit */
            c = (int)(unsigned char)*nptr++;    /* get next char */
        }

        if (sign == '-')
            return -total;
        else
            return total;   /* return result, negated if necessary */
}

FX_BOOL util::byteToChar(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	int iSize = params.size();
	if (iSize == 0)
		return FALSE;
	int nByte = params[0].ToInt();
	unsigned char cByte = (unsigned char)nByte;
	CFX_WideString csValue;
	csValue.Format(L"%c", cByte);
	vRet = csValue.c_str();
	return TRUE;
}
