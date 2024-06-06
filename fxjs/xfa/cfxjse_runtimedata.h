// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_RUNTIMEDATA_H_
#define FXJS_XFA_CFXJSE_RUNTIMEDATA_H_

#include <memory>

#include "fxjs/cfxjs_engine.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"

class CFXJSE_RuntimeData final : public CFXJS_PerIsolateData::ExtensionIface {
 public:
  static CFXJSE_RuntimeData* Get(v8::Isolate* pIsolate);

  CFXJSE_RuntimeData(const CFXJSE_RuntimeData&) = delete;
  CFXJSE_RuntimeData& operator=(const CFXJSE_RuntimeData&) = delete;
  ~CFXJSE_RuntimeData() override;

  v8::Local<v8::Context> GetRootContext(v8::Isolate* pIsolate);

 private:
  static std::unique_ptr<CFXJSE_RuntimeData> Create(v8::Isolate* pIsolate);

  CFXJSE_RuntimeData();

  v8::Global<v8::FunctionTemplate> root_context_global_template_;
  v8::Global<v8::Context> root_context_;
};

#endif  // FXJS_XFA_CFXJSE_RUNTIMEDATA_H_
