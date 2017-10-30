// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cjs_event_context_stub.h"
#include "fxjs/ijs_runtime.h"
#include "third_party/base/ptr_util.h"

class CJS_RuntimeStub final : public IJS_Runtime {
 public:
  explicit CJS_RuntimeStub(CPDFSDK_FormFillEnvironment* pFormFillEnv)
      : m_pFormFillEnv(pFormFillEnv) {}
  ~CJS_RuntimeStub() override {}

  IJS_EventContext* NewEventContext() override {
    if (!m_pContext)
      m_pContext = pdfium::MakeUnique<CJS_EventContextStub>();
    return m_pContext.get();
  }

  void ReleaseEventContext(IJS_EventContext* pContext) override {}

  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const override {
    return m_pFormFillEnv.Get();
  }

#ifdef PDF_ENABLE_XFA
  bool GetValueByName(const ByteStringView&, CFXJSE_Value*) override {
    return false;
  }

  bool SetValueByName(const ByteStringView&, CFXJSE_Value*) override {
    return false;
  }
#endif  // PDF_ENABLE_XFA

  int ExecuteScript(const WideString& script, WideString* info) override {
    return 0;
  }

 protected:
  UnownedPtr<CPDFSDK_FormFillEnvironment> const m_pFormFillEnv;
  std::unique_ptr<CJS_EventContextStub> m_pContext;
};

// static
void IJS_Runtime::Initialize(unsigned int slot, void* isolate) {}

// static
void IJS_Runtime::Destroy() {}

// static
IJS_Runtime* IJS_Runtime::Create(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  return new CJS_RuntimeStub(pFormFillEnv);
}
