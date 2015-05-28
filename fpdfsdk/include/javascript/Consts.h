// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_

#include "JS_Define.h"

/* ------------------------------ border ------------------------------ */

class CJS_Border : public CJS_Object
{
public:
	CJS_Border(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Border(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ display ------------------------------ */

class CJS_Display : public CJS_Object
{
public:
	CJS_Display(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Display(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ font ------------------------------ */

class CJS_Font : public CJS_Object
{
public:
	CJS_Font(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Font(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ highlight ------------------------------ */

class CJS_Highlight : public CJS_Object
{
public:
	CJS_Highlight(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Highlight(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ position ------------------------------ */

class CJS_Position : public CJS_Object
{
public:
	CJS_Position(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Position(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ scaleHow ------------------------------ */

class CJS_ScaleHow : public CJS_Object
{
public:
	CJS_ScaleHow(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_ScaleHow(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ scaleWhen ------------------------------ */

class CJS_ScaleWhen : public CJS_Object
{
public:
	CJS_ScaleWhen(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_ScaleWhen(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ style ------------------------------ */

class CJS_Style : public CJS_Object
{
public:
	CJS_Style(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Style(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ zoomtype ------------------------------ */

class CJS_Zoomtype : public CJS_Object
{
public:
	CJS_Zoomtype(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Zoomtype(void){};

	DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ CJS_GlobalConsts ------------------------------ */

class CJS_GlobalConsts : public CJS_Object
{
public:
	static int				Init(IJS_Runtime* pRuntime);
};

/* ------------------------------ CJS_GlobalArrays ------------------------------ */

class CJS_GlobalArrays : public CJS_Object
{
public:
	static int				Init(IJS_Runtime* pRuntime);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_
