// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_

#include <set>
#include <utility>

#include "../../../third_party/base/nonstd_unique_ptr.h"
#include "../../../core/include/fxcrt/fx_basic.h"
#include "../jsapi/fxjs_v8.h"
#include "IJavaScript.h"
#include "JS_EventHandler.h"

class CJS_Context;

class CJS_Runtime : public IFXJS_Runtime {
 public:
  using FieldEvent = std::pair<CFX_WideString, JS_EVENT_T>;

  explicit CJS_Runtime(CPDFDoc_Environment* pApp);
  ~CJS_Runtime() override;

  // IFXJS_Runtime
  IFXJS_Context* NewContext() override;
  void ReleaseContext(IFXJS_Context* pContext) override;
  IFXJS_Context* GetCurrentContext() override;
  void SetReaderDocument(CPDFSDK_Document* pReaderDoc) override;
  CPDFSDK_Document* GetReaderDocument() override { return m_pDocument; }

  CPDFDoc_Environment* GetReaderApp() const { return m_pApp; }

  // Returns true if the event isn't already found in the set.
  bool AddEventToSet(const FieldEvent& event);
  void RemoveEventFromSet(const FieldEvent& event);

  void BeginBlock() { m_bBlocking = TRUE; }
  void EndBlock() { m_bBlocking = FALSE; }
  FX_BOOL IsBlocking() const { return m_bBlocking; }

  v8::Isolate* GetIsolate() const { return m_isolate; }
  v8::Local<v8::Context> NewJSContext();

 private:
  void DefineJSObjects();

  CFX_ArrayTemplate<CJS_Context*> m_ContextArray;
  CPDFDoc_Environment* m_pApp;
  CPDFSDK_Document* m_pDocument;
  FX_BOOL m_bBlocking;
  std::set<FieldEvent> m_FieldEventSet;
  v8::Isolate* m_isolate;
  bool m_isolateManaged;
  nonstd::unique_ptr<FXJS_ArrayBufferAllocator> m_pArrayBufferAllocator;
  v8::Global<v8::Context> m_context;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_RUNTIME_H_
