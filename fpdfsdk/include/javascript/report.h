// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _REPORT_H_
#define _REPORT_H_

class Report : public CJS_EmbedObj
{
public:
	Report(CJS_Object * pJSObject);
	virtual ~Report();

public:
	FX_BOOL save(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL writeText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
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

#endif //_REPORT_H_

