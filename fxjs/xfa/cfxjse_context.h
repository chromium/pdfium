// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_CONTEXT_H_
#define FXJS_XFA_CFXJSE_CONTEXT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/unowned_ptr.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"

class CFXJSE_Class;
class CFXJSE_HostObject;
class CFXJSE_Value;
class CXFA_ThisProxy;
struct FXJSE_CLASS_DESCRIPTOR;

class CFXJSE_Context {
 public:
  static std::unique_ptr<CFXJSE_Context> Create(
      v8::Isolate* pIsolate,
      const FXJSE_CLASS_DESCRIPTOR* pGlobalClass,
      CFXJSE_HostObject* pGlobalObject,
      CXFA_ThisProxy* pProxy);

  ~CFXJSE_Context();

  v8::Isolate* GetIsolate() const { return m_pIsolate; }
  v8::Local<v8::Context> GetContext();
  v8::Local<v8::Object> GetGlobalObject();

  void AddClass(std::unique_ptr<CFXJSE_Class> pClass);
  CFXJSE_Class* GetClassByName(ByteStringView szName) const;
  void EnableCompatibleMode();

  // Note: `pNewThisObject` may be empty.
  bool ExecuteScript(ByteStringView bsScript,
                     CFXJSE_Value* pRetValue,
                     v8::Local<v8::Object> pNewThisObject);

 private:
  CFXJSE_Context(v8::Isolate* pIsolate, CXFA_ThisProxy* pProxy);
  CFXJSE_Context(const CFXJSE_Context&) = delete;
  CFXJSE_Context& operator=(const CFXJSE_Context&) = delete;

  v8::Global<v8::Context> m_hContext;
  UnownedPtr<v8::Isolate> m_pIsolate;
  std::vector<std::unique_ptr<CFXJSE_Class>> m_rgClasses;
  cppgc::Persistent<CXFA_ThisProxy> m_pProxy;
};

void FXJSE_UpdateObjectBinding(v8::Local<v8::Object> hObject,
                               CFXJSE_HostObject* pNewBinding);

void FXJSE_ClearObjectBinding(v8::Local<v8::Object> hJSObject);
CFXJSE_HostObject* FXJSE_RetrieveObjectBinding(v8::Local<v8::Value> hValue);

#endif  // FXJS_XFA_CFXJSE_CONTEXT_H_
