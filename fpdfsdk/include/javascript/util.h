// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_UTIL_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_UTIL_H_

#include <string>  // For std::wstring.

#include "JS_Define.h"

class util : public CJS_EmbedObj
{
public:
	util(CJS_Object * pJSObject);
	virtual ~util(void);

public:
	FX_BOOL printd(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL printf(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL printx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL scand(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL byteToChar(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);

public:
	static void		printd(const std::wstring &cFormat,CJS_Date Date,bool bXFAPicture, std::wstring &cPurpose);
	static void		printx(const std::string &cFormat,const std::string &cSource, std::string &cPurpose);
	static int		ParstDataType(std::wstring* sFormat);
};

class CJS_Util : public CJS_Object
{
public:
	CJS_Util(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Util(void){};

	DECLARE_JS_CLASS(CJS_Util);

	JS_STATIC_METHOD(printd, util);
	JS_STATIC_METHOD(printf, util);
	JS_STATIC_METHOD(printx, util);
	JS_STATIC_METHOD(scand, util);
	JS_STATIC_METHOD(byteToChar, util);
};

FX_INT64 FX_atoi64(const char *nptr);

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_UTIL_H_
