// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_EVENT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_EVENT_H_

#include "JS_Define.h"

class event : public CJS_EmbedObj
{
public:
	event(CJS_Object * pJSObject);
	virtual ~event(void);

public:
	bool change(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool changeEx(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool commitKey(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool fieldFull(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool keyDown(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool modifier(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool name(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool rc(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool richChange(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool richChangeEx(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool richValue(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool selEnd(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool selStart(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool shift(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool source(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool target(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool targetName(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool type(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool value(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool willCommit(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);

};

class CJS_Event : public CJS_Object
{
public:
	CJS_Event(JSFXObject  pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Event(void){};

	DECLARE_JS_CLASS(CJS_Event);

	JS_STATIC_PROP(change, event);
	JS_STATIC_PROP(changeEx, event);
	JS_STATIC_PROP(commitKey, event);
	JS_STATIC_PROP(fieldFull, event);
	JS_STATIC_PROP(keyDown, event);
	JS_STATIC_PROP(modifier, event);
	JS_STATIC_PROP(name, event);
	JS_STATIC_PROP(rc, event);
	JS_STATIC_PROP(richChange, event);
	JS_STATIC_PROP(richChangeEx, event);
	JS_STATIC_PROP(richValue, event);
	JS_STATIC_PROP(selEnd, event);
	JS_STATIC_PROP(selStart, event);
	JS_STATIC_PROP(shift, event);
	JS_STATIC_PROP(source, event);
	JS_STATIC_PROP(target, event);
	JS_STATIC_PROP(targetName, event);
	JS_STATIC_PROP(type, event);
	JS_STATIC_PROP(value, event);
	JS_STATIC_PROP(willCommit, event);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_EVENT_H_
