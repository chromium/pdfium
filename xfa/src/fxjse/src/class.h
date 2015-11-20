// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJSE_CLASS_H_
#define FXJSE_CLASS_H_
class CFXJSE_Context;
class CFXJSE_Value;
class CFXJSE_Class {
 protected:
  CFXJSE_Class(CFXJSE_Context* lpContext)
      : m_lpClassDefinition(nullptr), m_pContext(lpContext) {}

 public:
  inline CFXJSE_Context* GetContext() { return m_pContext; }
  inline v8::Global<v8::FunctionTemplate>& GetTemplate() { return m_hTemplate; }

 public:
  static CFXJSE_Class* Create(CFXJSE_Context* pContext,
                              const FXJSE_CLASS* lpClassDefintion,
                              FX_BOOL bIsJSGlobal = FALSE);
  static CFXJSE_Class* GetClassFromContext(CFXJSE_Context* pContext,
                                           const CFX_ByteStringC& szName);
  static void SetUpDynPropHandler(CFXJSE_Context* pContext,
                                  CFXJSE_Value* pValue,
                                  const FXJSE_CLASS* lpClassDefinition);
  static void SetUpNamedPropHandler(
      v8::Isolate* pIsolate,
      v8::Local<v8::ObjectTemplate>& hObjectTemplate,
      const FXJSE_CLASS* lpClassDefinition);

 protected:
  CFX_ByteString m_szClassName;
  const FXJSE_CLASS* m_lpClassDefinition;
  CFXJSE_Context* m_pContext;
  v8::Global<v8::FunctionTemplate> m_hTemplate;
  friend class CFXJSE_Context;
  friend class CFXJSE_Value;
};
struct CFXJSE_ArgumentsImpl {
  const v8::FunctionCallbackInfo<v8::Value>* m_pInfo;
  CFXJSE_Value* m_pRetValue;
};
#endif
