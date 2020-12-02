// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_ISOLATETRACKER_H_
#define FXJS_XFA_CFXJSE_ISOLATETRACKER_H_

#include "v8/include/v8.h"

class CFXJSE_Context;

class CFXJSE_ScopeUtil_IsolateHandle {
 public:
  explicit CFXJSE_ScopeUtil_IsolateHandle(v8::Isolate* pIsolate);
  CFXJSE_ScopeUtil_IsolateHandle(const CFXJSE_ScopeUtil_IsolateHandle&) =
      delete;
  CFXJSE_ScopeUtil_IsolateHandle& operator=(
      const CFXJSE_ScopeUtil_IsolateHandle&) = delete;
  ~CFXJSE_ScopeUtil_IsolateHandle();

 private:
  void* operator new(size_t size) = delete;
  void operator delete(void*, size_t) = delete;

  v8::Isolate::Scope isolate_scope_;
  v8::HandleScope handle_scope_;
};

class CFXJSE_ScopeUtil_Context {
 public:
  explicit CFXJSE_ScopeUtil_Context(CFXJSE_Context* pContext);
  CFXJSE_ScopeUtil_Context(const CFXJSE_ScopeUtil_Context&) = delete;
  CFXJSE_ScopeUtil_Context& operator=(const CFXJSE_ScopeUtil_Context&) = delete;
  ~CFXJSE_ScopeUtil_Context();

 private:
  void* operator new(size_t size) = delete;
  void operator delete(void*, size_t) = delete;

  v8::Context::Scope context_scope_;
};

class CFXJSE_ScopeUtil_IsolateHandleContext {
 public:
  explicit CFXJSE_ScopeUtil_IsolateHandleContext(CFXJSE_Context* pContext);
  CFXJSE_ScopeUtil_IsolateHandleContext(
      const CFXJSE_ScopeUtil_IsolateHandleContext&) = delete;
  CFXJSE_ScopeUtil_IsolateHandleContext& operator=(
      const CFXJSE_ScopeUtil_IsolateHandleContext&) = delete;
  ~CFXJSE_ScopeUtil_IsolateHandleContext();

 private:
  void* operator new(size_t size) = delete;
  void operator delete(void*, size_t) = delete;

  CFXJSE_ScopeUtil_IsolateHandle isolate_handle_;
  CFXJSE_ScopeUtil_Context context_;
};

class CFXJSE_ScopeUtil_RootContext {
 public:
  explicit CFXJSE_ScopeUtil_RootContext(v8::Isolate* pIsolate);
  CFXJSE_ScopeUtil_RootContext(const CFXJSE_ScopeUtil_RootContext&) = delete;
  CFXJSE_ScopeUtil_RootContext& operator=(const CFXJSE_ScopeUtil_RootContext&) =
      delete;
  ~CFXJSE_ScopeUtil_RootContext();

 private:
  void* operator new(size_t size) = delete;
  void operator delete(void*, size_t) = delete;

  v8::Context::Scope context_scope_;
};

class CFXJSE_ScopeUtil_IsolateHandleRootContext {
 public:
  explicit CFXJSE_ScopeUtil_IsolateHandleRootContext(v8::Isolate* pIsolate);
  CFXJSE_ScopeUtil_IsolateHandleRootContext(
      const CFXJSE_ScopeUtil_IsolateHandleRootContext&) = delete;
  CFXJSE_ScopeUtil_IsolateHandleRootContext& operator=(
      const CFXJSE_ScopeUtil_IsolateHandleRootContext&) = delete;
  ~CFXJSE_ScopeUtil_IsolateHandleRootContext();

 private:
  void* operator new(size_t size) = delete;
  void operator delete(void*, size_t) = delete;

  CFXJSE_ScopeUtil_IsolateHandle isolate_handle_;
  CFXJSE_ScopeUtil_RootContext root_context_;
};

#endif  // FXJS_XFA_CFXJSE_ISOLATETRACKER_H_
