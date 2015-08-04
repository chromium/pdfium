// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJSE_SCOPE_INLINE_H_
#define FXJSE_SCOPE_INLINE_H_
#include "runtime.h"
#include "context.h"
class CFXJSE_ScopeUtil_IsolateHandle {
 protected:
  v8::Isolate* m_isolate;
  v8::Locker m_locker;
  v8::Isolate::Scope m_iscope;
  v8::HandleScope m_hscope;

 public:
  explicit CFXJSE_ScopeUtil_IsolateHandle(v8::Isolate* pIsolate)
      : m_isolate(pIsolate),
        m_locker(pIsolate),
        m_iscope(pIsolate),
        m_hscope(pIsolate) {}
  v8::Isolate* GetIsolate() { return m_isolate; }

 private:
  CFXJSE_ScopeUtil_IsolateHandle(const CFXJSE_ScopeUtil_IsolateHandle&);
  void operator=(const CFXJSE_ScopeUtil_IsolateHandle&);
  void* operator new(size_t size);
  void operator delete(void*, size_t);
};
class CFXJSE_ScopeUtil_IsolateHandleRootContext {
  CFXJSE_ScopeUtil_IsolateHandle m_parent;
  v8::Context::Scope m_cscope;

 public:
  explicit CFXJSE_ScopeUtil_IsolateHandleRootContext(v8::Isolate* pIsolate)
      : m_parent(pIsolate),
        m_cscope(v8::Local<v8::Context>::New(
            pIsolate,
            CFXJSE_RuntimeData::Get(pIsolate)->m_hRootContext)) {}

 private:
  CFXJSE_ScopeUtil_IsolateHandleRootContext(
      const CFXJSE_ScopeUtil_IsolateHandleRootContext&);
  void operator=(const CFXJSE_ScopeUtil_IsolateHandleRootContext&);
  void* operator new(size_t size);
  void operator delete(void*, size_t);
};
class CFXJSE_ScopeUtil_IsolateHandleContext {
  CFXJSE_Context* m_context;
  CFXJSE_ScopeUtil_IsolateHandle m_parent;
  v8::Context::Scope m_cscope;

 public:
  explicit CFXJSE_ScopeUtil_IsolateHandleContext(CFXJSE_Context* pContext)
      : m_context(pContext),
        m_parent(pContext->m_pIsolate),
        m_cscope(v8::Local<v8::Context>::New(pContext->m_pIsolate,
                                             pContext->m_hContext)) {}
  v8::Isolate* GetIsolate() { return m_context->m_pIsolate; }
  v8::Local<v8::Context> GetLocalContext() {
    return v8::Local<v8::Context>::New(m_context->m_pIsolate,
                                       m_context->m_hContext);
  }

 private:
  CFXJSE_ScopeUtil_IsolateHandleContext(
      const CFXJSE_ScopeUtil_IsolateHandleContext&);
  void operator=(const CFXJSE_ScopeUtil_IsolateHandleContext&);
  void* operator new(size_t size);
  void operator delete(void*, size_t);
};
class CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext {
  CFXJSE_Context* m_context;
  CFXJSE_ScopeUtil_IsolateHandle m_parent;
  v8::Context::Scope m_cscope;

 public:
  explicit CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext(
      v8::Isolate* pIsolate,
      CFXJSE_Context* pContext)
      : m_context(pContext),
        m_parent(pIsolate),
        m_cscope(v8::Local<v8::Context>::New(
            pIsolate,
            pContext ? pContext->m_hContext
                     : CFXJSE_RuntimeData::Get(pIsolate)->m_hRootContext)) {}
  v8::Isolate* GetIsolate() { return m_parent.GetIsolate(); }
  v8::Local<v8::Context> GetLocalContext() {
    v8::Isolate* pIsolate = m_parent.GetIsolate();
    return v8::Local<v8::Context>::New(
        pIsolate, m_context
                      ? m_context->m_hContext
                      : CFXJSE_RuntimeData::Get(pIsolate)->m_hRootContext);
  }

 private:
  CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext(
      const CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext&);
  void operator=(const CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext&);
  void* operator new(size_t size);
  void operator delete(void*, size_t);
};
#endif
