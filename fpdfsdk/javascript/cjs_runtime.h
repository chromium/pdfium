// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_CJS_RUNTIME_H_
#define FPDFSDK_JAVASCRIPT_CJS_RUNTIME_H_

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/include/cfx_observable.h"
#include "core/fxcrt/include/fx_basic.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/ijs_runtime.h"
#include "fxjs/include/fxjs_v8.h"

class CJS_Context;

class CJS_Runtime : public IJS_Runtime,
                    public CFXJS_Engine,
                    public CFX_Observable<CJS_Runtime> {
 public:
  using FieldEvent = std::pair<CFX_WideString, JS_EVENT_T>;

  static CJS_Runtime* FromContext(const IJS_Context* cc);
  static CJS_Runtime* CurrentRuntimeFromIsolate(v8::Isolate* pIsolate);

  explicit CJS_Runtime(CPDFSDK_Environment* pApp);
  ~CJS_Runtime() override;

  // IJS_Runtime
  IJS_Context* NewContext() override;
  void ReleaseContext(IJS_Context* pContext) override;
  IJS_Context* GetCurrentContext() override;
  void SetReaderDocument(CPDFSDK_Document* pReaderDoc) override;
  CPDFSDK_Document* GetReaderDocument() override;
  int ExecuteScript(const CFX_WideString& script,
                    CFX_WideString* info) override;

  CPDFSDK_Environment* GetReaderApp() const { return m_pApp; }

  // Returns true if the event isn't already found in the set.
  bool AddEventToSet(const FieldEvent& event);
  void RemoveEventFromSet(const FieldEvent& event);

  void BeginBlock() { m_bBlocking = TRUE; }
  void EndBlock() { m_bBlocking = FALSE; }
  FX_BOOL IsBlocking() const { return m_bBlocking; }

#ifdef PDF_ENABLE_XFA
  FX_BOOL GetValueByName(const CFX_ByteStringC& utf8Name,
                         CFXJSE_Value* pValue) override;
  FX_BOOL SetValueByName(const CFX_ByteStringC& utf8Name,
                         CFXJSE_Value* pValue) override;
#endif  // PDF_ENABLE_XFA

 private:
  void DefineJSObjects();

  std::vector<std::unique_ptr<CJS_Context>> m_ContextArray;
  CPDFSDK_Environment* const m_pApp;
  CPDFSDK_Document* m_pDocument;
  bool m_bBlocking;
  bool m_isolateManaged;
  std::set<FieldEvent> m_FieldEventSet;
};

#endif  // FPDFSDK_JAVASCRIPT_CJS_RUNTIME_H_
