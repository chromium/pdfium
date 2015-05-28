// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_CONSOLE_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_CONSOLE_H_

#include "JS_Define.h"

class console : public CJS_EmbedObj
{
public:
	console(CJS_Object* pJSObject);
	virtual ~console(void);

public:
	FX_BOOL clear(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL hide(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL println(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	FX_BOOL show(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
};

class CJS_Console : public CJS_Object  
{
public:
	CJS_Console(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Console(void){};

	DECLARE_JS_CLASS(CJS_Console);

	JS_STATIC_METHOD(clear, console);
	JS_STATIC_METHOD(hide, console);
	JS_STATIC_METHOD(println, console);
	JS_STATIC_METHOD(show, console);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_CONSOLE_H_
