// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_REPORT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_REPORT_H_

#include "JS_Define.h"

class Report : public CJS_EmbedObj
{
public:
	Report(CJS_Object * pJSObject);
	virtual ~Report();

public:
	FX_BOOL save(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL writeText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
};

class CJS_Report : public CJS_Object
{
public:
	CJS_Report(JSFXObject  pObject) : CJS_Object(pObject){};
	virtual ~CJS_Report(){};

public:
	DECLARE_JS_CLASS(CJS_Report);

	JS_STATIC_METHOD(save, Report)
	JS_STATIC_METHOD(writeText, Report);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_REPORT_H_
