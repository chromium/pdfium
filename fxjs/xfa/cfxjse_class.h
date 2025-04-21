// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_CLASS_H_
#define FXJS_XFA_CFXJSE_CLASS_H_

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/xfa/fxjse.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"

class CFXJSE_Context;
struct FXJSE_CLASS_DESCRIPTOR;

class CFXJSE_Class {
 public:
  static CFXJSE_Class* Create(CFXJSE_Context* pContext,
                              const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor,
                              bool bIsJSGlobal);

  explicit CFXJSE_Class(const CFXJSE_Context* pContext);
  ~CFXJSE_Class();

  bool IsName(ByteStringView name) const { return name == class_name_; }
  const CFXJSE_Context* GetContext() const { return context_; }
  v8::Local<v8::FunctionTemplate> GetTemplate(v8::Isolate* pIsolate);

 protected:
  ByteString class_name_;
  UnownedPtr<const FXJSE_CLASS_DESCRIPTOR> class_descriptor_;
  UnownedPtr<const CFXJSE_Context> const context_;
  v8::Global<v8::FunctionTemplate> func_template_;
};

#endif  // FXJS_XFA_CFXJSE_CLASS_H_
