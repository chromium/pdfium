// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJSE_RUNTIME_H_
#define FXJSE_RUNTIME_H_
class CFXJSE_RuntimeList;
class CFXJSE_RuntimeData {
 protected:
  CFXJSE_RuntimeData(v8::Isolate* pIsolate) : m_pIsolate(pIsolate){};

 public:
  static CFXJSE_RuntimeData* Create(v8::Isolate* pIsolate);
  static CFXJSE_RuntimeData* Get(v8::Isolate* pIsolate);

 public:
  v8::Isolate* m_pIsolate;
  v8::Global<v8::FunctionTemplate> m_hRootContextGlobalTemplate;
  v8::Global<v8::Context> m_hRootContext;

 public:
  static CFXJSE_RuntimeList* g_RuntimeList;

 protected:
  CFXJSE_RuntimeData();
  CFXJSE_RuntimeData(const CFXJSE_RuntimeData&);
  CFXJSE_RuntimeData& operator=(const CFXJSE_RuntimeData&);
};
class CFXJSE_RuntimeList {
 public:
  typedef void (*RuntimeDisposeCallback)(v8::Isolate*);

 public:
  void AppendRuntime(v8::Isolate* pIsolate);
  void RemoveRuntime(v8::Isolate* pIsolate,
                     RuntimeDisposeCallback lpfnDisposeCallback);
  void RemoveAllRuntimes(RuntimeDisposeCallback lpfnDisposeCallback);

 protected:
  CFX_ArrayTemplate<v8::Isolate*> m_RuntimeList;
};
#endif
