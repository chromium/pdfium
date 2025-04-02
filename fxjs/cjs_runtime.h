// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_RUNTIME_H_
#define FXJS_CJS_RUNTIME_H_

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/observed_ptr.h"
#include "fxjs/cfxjs_engine.h"
#include "fxjs/cjs_event_context.h"
#include "fxjs/ijs_runtime.h"

class CPDFSDK_FormFillEnvironment;

class CJS_Runtime final : public IJS_Runtime,
                          public CFXJS_Engine,
                          public Observable {
 public:
  using FieldEvent = std::pair<WideString, CJS_EventContext::Kind>;

  explicit CJS_Runtime(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  ~CJS_Runtime() override;

  // IJS_Runtime:
  CJS_Runtime* AsCJSRuntime() override;
  IJS_EventContext* NewEventContext() override;
  void ReleaseEventContext(IJS_EventContext* pContext) override;
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const override;
  std::optional<IJS_Runtime::JS_Error> ExecuteScript(
      const WideString& script) override;

  CJS_EventContext* GetCurrentEventContext() const;
  CFX_Timer::HandlerIface* GetTimerHandler() const;

  // Returns true if the event isn't already found in the set.
  bool AddEventToSet(const FieldEvent& event);
  void RemoveEventFromSet(const FieldEvent& event);

  void BeginBlock() { blocking_ = true; }
  void EndBlock() { blocking_ = false; }
  bool IsBlocking() const { return blocking_; }

  // Attempt to convert the |value| into a number. If successful the number
  // value will be returned, otherwise |value| is returned.
  v8::Local<v8::Value> MaybeCoerceToNumber(v8::Local<v8::Value> value);

  v8::Local<v8::Value> GetValueByNameFromGlobalObject(ByteStringView utf8Name);
  bool SetValueByNameInGlobalObject(ByteStringView utf8Name,
                                    v8::Local<v8::Value> pValue);

 private:
  void DefineJSObjects();
  void SetFormFillEnvToDocument();

  std::vector<std::unique_ptr<CJS_EventContext>> event_context_array_;
  ObservedPtr<CPDFSDK_FormFillEnvironment> form_fill_env_;
  bool blocking_ = false;
  bool isolate_managed_ = false;
  std::set<FieldEvent> field_event_set_;
};

#endif  // FXJS_CJS_RUNTIME_H_
