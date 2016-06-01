// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXJSE_CLASS_H_
#define XFA_FXJSE_CLASS_H_

#include "v8/include/v8.h"
#include "xfa/fxjse/cfxjse_arguments.h"
#include "xfa/fxjse/include/fxjse.h"

class CFXJSE_Context;
class CFXJSE_Value;

class CFXJSE_Class {
 public:
  static CFXJSE_Class* Create(CFXJSE_Context* pContext,
                              const FXJSE_CLASS_DESCRIPTOR* lpClassDefintion,
                              FX_BOOL bIsJSGlobal = FALSE);
  static CFXJSE_Class* GetClassFromContext(CFXJSE_Context* pContext,
                                           const CFX_ByteStringC& szName);
  static void SetUpNamedPropHandler(
      v8::Isolate* pIsolate,
      v8::Local<v8::ObjectTemplate>& hObjectTemplate,
      const FXJSE_CLASS_DESCRIPTOR* lpClassDefinition);

  CFXJSE_Context* GetContext() { return m_pContext; }
  v8::Global<v8::FunctionTemplate>& GetTemplate() { return m_hTemplate; }

 protected:
  explicit CFXJSE_Class(CFXJSE_Context* lpContext)
      : m_lpClassDefinition(nullptr), m_pContext(lpContext) {}

  CFX_ByteString m_szClassName;
  const FXJSE_CLASS_DESCRIPTOR* m_lpClassDefinition;
  CFXJSE_Context* m_pContext;
  v8::Global<v8::FunctionTemplate> m_hTemplate;
  friend class CFXJSE_Context;
  friend class CFXJSE_Value;
};

#endif  // XFA_FXJSE_CLASS_H_
