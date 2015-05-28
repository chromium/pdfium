// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_

#include "../../../third_party/base/nonstd_unique_ptr.h"
#include "../../../core/include/fxcrt/fx_basic.h"
#include "../jsapi/fxjs_v8.h"
#include "IJavaScript.h"
#include "JS_EventHandler.h"

class CJS_Context;

class CJS_ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
    void* Allocate(size_t length) override;
    void* AllocateUninitialized(size_t length) override;
    void Free(void* data, size_t length) override;
};

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

	v8::Local<v8::Context>							NewJSContext();
protected:
	CFX_ArrayTemplate<CJS_Context*>		m_ContextArray;
	CPDFDoc_Environment*							m_pApp;
	CPDFSDK_Document*						m_pDocument;
	FX_BOOL									m_bBlocking;
	FX_BOOL									m_bRegistered;
	CJS_FieldEvent*							m_pFieldEventPath;

	v8::Isolate* m_isolate;
	nonstd::unique_ptr<CJS_ArrayBufferAllocator> m_pArrayBufferAllocator;
	v8::Global<v8::Context> m_context;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_
