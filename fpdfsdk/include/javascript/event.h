// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _EVENT_H_
#define _EVENT_H_

class event : public CJS_EmbedObj
{
public:
	event(CJS_Object * pJSObject);
	virtual ~event(void);

public:
	FX_BOOL change(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL changeEx(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL commitKey(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL fieldFull(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL keyDown(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL modifier(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL name(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL rc(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL richChange(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL richChangeEx(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL richValue(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL selEnd(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL selStart(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL shift(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL source(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL target(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL targetName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL type(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL value(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL willCommit(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);

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

#endif //_EVENT_H_
