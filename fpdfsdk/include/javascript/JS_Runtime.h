// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

class CJS_FieldEvent
{
public:
	CFX_WideString		sTargetName;
	JS_EVENT_T			eEventType;
	CJS_FieldEvent*		pNext;
};

class CJS_Runtime : public IFXJS_Runtime
{
public:
	CJS_Runtime(CPDFDoc_Environment * pApp);
	virtual ~CJS_Runtime();

	virtual IFXJS_Context *					NewContext();
	virtual void							ReleaseContext(IFXJS_Context * pContext);
	virtual IFXJS_Context*					GetCurrentContext();

	virtual void							SetReaderDocument(CPDFSDK_Document *pReaderDoc);
	virtual CPDFSDK_Document *				GetReaderDocument(){return m_pDocument;}

	CPDFDoc_Environment *							GetReaderApp(){return m_pApp;}

	FX_BOOL									InitJSObjects();

	FX_BOOL									AddEventToLoop(const CFX_WideString& sTargetName, JS_EVENT_T eEventType);
	void									RemoveEventInLoop(const CFX_WideString& sTargetName, JS_EVENT_T eEventType);
	void									RemoveEventsInLoop(CJS_FieldEvent* pStart);

	void									BeginBlock(){m_bBlocking = TRUE;}
	void									EndBlock(){m_bBlocking = FALSE;}
	FX_BOOL									IsBlocking(){return m_bBlocking;}

	operator								IJS_Runtime*() {return (IJS_Runtime*)m_isolate;}
	v8::Isolate*								GetIsolate(){return m_isolate;};
	void									SetIsolate(v8::Isolate* isolate){m_isolate = isolate;}

	v8::Handle<v8::Context>							NewJSContext();
protected:
	CFX_ArrayTemplate<CJS_Context *>		m_ContextArray;
	CPDFDoc_Environment *							m_pApp;
	CPDFSDK_Document *						m_pDocument;
	FX_BOOL									m_bBlocking;
	CJS_FieldEvent*							m_pFieldEventPath;

	v8::Isolate*								m_isolate;
	v8::Persistent<v8::Context>						m_context;
	FX_BOOL									m_bRegistered;
};

#endif //_JS_RUNTIME_H_

