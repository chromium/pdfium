// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

class console : public CJS_EmbedObj
{
public:
	console(CJS_Object* pJSObject);
	virtual ~console(void);

public:
	FX_BOOL clear(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL hide(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL println(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL show(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
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

#endif //_CONSOLE_H_

