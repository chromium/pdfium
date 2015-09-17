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

class CJS_FieldEvent {
 public:
  CFX_WideString sTargetName;
  JS_EVENT_T eEventType;
  CJS_FieldEvent* pNext;
};

class CJS_Runtime : public IFXJS_Runtime {
 public:
  explicit CJS_Runtime(CPDFDoc_Environment* pApp);
  ~CJS_Runtime() override;

  // IFXJS_Runtime
  IFXJS_Context* NewContext() override;
  void ReleaseContext(IFXJS_Context* pContext) override;
  IFXJS_Context* GetCurrentContext() override;
  void SetReaderDocument(CPDFSDK_Document* pReaderDoc) override;
  CPDFSDK_Document* GetReaderDocument() override { return m_pDocument; }

  CPDFDoc_Environment* GetReaderApp() const { return m_pApp; }

  FX_BOOL AddEventToLoop(const CFX_WideString& sTargetName,
                         JS_EVENT_T eEventType);
  void RemoveEventInLoop(const CFX_WideString& sTargetName,
                         JS_EVENT_T eEventType);
  void RemoveEventsInLoop(CJS_FieldEvent* pStart);

  void BeginBlock() { m_bBlocking = TRUE; }
  void EndBlock() { m_bBlocking = FALSE; }
  FX_BOOL IsBlocking() const { return m_bBlocking; }

  v8::Isolate* GetIsolate() const { return m_isolate; }
  v8::Local<v8::Context> NewJSContext();

  virtual FX_BOOL GetHValueByName(const CFX_ByteStringC& utf8Name,
                                  FXJSE_HVALUE hValue);
  virtual FX_BOOL SetHValueByName(const CFX_ByteStringC& utf8Name,
                                  FXJSE_HVALUE hValue);

 private:
  void DefineJSObjects();

  CFX_ArrayTemplate<CJS_Context*> m_ContextArray;
  CPDFDoc_Environment* m_pApp;
  CPDFSDK_Document* m_pDocument;
  FX_BOOL m_bBlocking;
  CJS_FieldEvent* m_pFieldEventPath;
  v8::Isolate* m_isolate;
  bool m_isolateManaged;
  nonstd::unique_ptr<FXJS_ArrayBufferAllocator> m_pArrayBufferAllocator;
  v8::Global<v8::Context> m_context;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_
