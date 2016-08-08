// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/JS_Object.h"

#include "fpdfsdk/include/fsdk_mgr.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/cjs_context.h"

CJS_EmbedObj::CJS_EmbedObj(CJS_Object* pJSObject) : m_pJSObject(pJSObject) {}

CJS_EmbedObj::~CJS_EmbedObj() {
  m_pJSObject = nullptr;
}

int CJS_EmbedObj::MsgBox(CPDFDoc_Environment* pApp,
                         const FX_WCHAR* swMsg,
                         const FX_WCHAR* swTitle,
                         FX_UINT nType,
                         FX_UINT nIcon) {
  if (!pApp)
    return 0;

  if (CPDFSDK_Document* pDoc = pApp->GetSDKDocument())
    pDoc->KillFocusAnnot();

  return pApp->JS_appAlert(swMsg, swTitle, nType, nIcon);
}

void CJS_EmbedObj::Alert(CJS_Context* pContext, const FX_WCHAR* swMsg) {
  CJS_Object::Alert(pContext, swMsg);
}

void FreeObject(const v8::WeakCallbackInfo<CJS_Object>& data) {
  CJS_Object* pJSObj = data.GetParameter();
  pJSObj->ExitInstance();
  delete pJSObj;
  FXJS_FreePrivate(data.GetInternalField(0));
}

void DisposeObject(const v8::WeakCallbackInfo<CJS_Object>& data) {
  CJS_Object* pJSObj = data.GetParameter();
  pJSObj->Dispose();
  data.SetSecondPassCallback(FreeObject);
}

CJS_Object::CJS_Object(v8::Local<v8::Object> pObject) {
  m_pIsolate = pObject->GetIsolate();
  m_pV8Object.Reset(m_pIsolate, pObject);
}

CJS_Object::~CJS_Object() {}

void CJS_Object::MakeWeak() {
  m_pV8Object.SetWeak(this, DisposeObject,
                      v8::WeakCallbackType::kInternalFields);
}

void CJS_Object::Dispose() {
  m_pV8Object.Reset();
}

void CJS_Object::InitInstance(IJS_Runtime* pIRuntime) {}

void CJS_Object::ExitInstance() {}

void CJS_Object::Alert(CJS_Context* pContext, const FX_WCHAR* swMsg) {
  CPDFDoc_Environment* pApp = pContext->GetReaderApp();
  if (pApp)
    pApp->JS_appAlert(swMsg, nullptr, 0, 3);
}
