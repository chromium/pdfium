// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJSE_RUNTIME_H_
#define FXJSE_RUNTIME_H_

#include <vector>

#include "core/fxcrt/include/fx_basic.h"
#include "v8/include/v8.h"

class CFXJSE_RuntimeList;

class CFXJSE_RuntimeData {
 public:
  static CFXJSE_RuntimeData* Get(v8::Isolate* pIsolate);

  v8::Isolate* m_pIsolate;
  v8::Global<v8::FunctionTemplate> m_hRootContextGlobalTemplate;
  v8::Global<v8::Context> m_hRootContext;

 protected:
  static CFXJSE_RuntimeData* Create(v8::Isolate* pIsolate);
  CFXJSE_RuntimeData(v8::Isolate* pIsolate) : m_pIsolate(pIsolate) {}
  CFXJSE_RuntimeData();
  CFXJSE_RuntimeData(const CFXJSE_RuntimeData&);
  CFXJSE_RuntimeData& operator=(const CFXJSE_RuntimeData&);
};

class CFXJSE_IsolateTracker {
 public:
  typedef void (*DisposeCallback)(v8::Isolate*, bool bOwnedIsolate);
  static CFXJSE_IsolateTracker* g_pInstance;

  void Append(v8::Isolate* pIsolate);
  void Remove(v8::Isolate* pIsolate, DisposeCallback lpfnDisposeCallback);
  void RemoveAll(DisposeCallback lpfnDisposeCallback);

 protected:
  std::vector<v8::Isolate*> m_OwnedIsolates;
};

#endif  // FXJSE_RUNTIME_H_
