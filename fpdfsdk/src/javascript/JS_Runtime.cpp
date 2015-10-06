// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JS_Runtime.h"

#include "../../include/fsdk_mgr.h"  // For CPDFDoc_Environment.
#include "../../include/javascript/IJavaScript.h"
#include "Consts.h"
#include "Document.h"
#include "Field.h"
#include "Icon.h"
#include "JS_Context.h"
#include "JS_Define.h"
#include "JS_EventHandler.h"
#include "JS_GlobalData.h"
#include "JS_Object.h"
#include "JS_Value.h"
#include "PublicMethods.h"
#include "app.h"
#include "color.h"
#include "console.h"
#include "event.h"
#include "global.h"
#include "report.h"
#include "util.h"

/* ------------------------------ CJS_Runtime ------------------------------ */

IFXJS_Runtime* IFXJS_Runtime::Create(CPDFDoc_Environment* pEnv) {
  return new CJS_Runtime(pEnv);
}

CJS_Runtime::CJS_Runtime(CPDFDoc_Environment* pApp)
    : m_pApp(pApp),
      m_pDocument(NULL),
      m_bBlocking(FALSE),
      m_isolate(NULL),
      m_isolateManaged(false) {
  IPDF_JSPLATFORM* pPlatform = m_pApp->GetFormFillInfo()->m_pJsPlatform;
  if (pPlatform->version <= 2) {
    unsigned int embedderDataSlot = 0;
    v8::Isolate* pExternalIsolate = nullptr;
    if (pPlatform->version == 2) {
      pExternalIsolate = reinterpret_cast<v8::Isolate*>(pPlatform->m_isolate);
      embedderDataSlot = pPlatform->m_v8EmbedderSlot;
    }
    FXJS_Initialize(embedderDataSlot, pExternalIsolate);
  }
  m_isolateManaged = FXJS_GetIsolate(&m_isolate);
  if (m_isolateManaged || FXJS_GlobalIsolateRefCount() == 0)
    DefineJSObjects();

  CJS_Context* pContext = (CJS_Context*)NewContext();
  FXJS_InitializeRuntime(GetIsolate(), this, pContext, m_context);
  ReleaseContext(pContext);
}

CJS_Runtime::~CJS_Runtime() {
  for (auto* obs : m_observers)
    obs->OnDestroyed();

  for (int i = 0, sz = m_ContextArray.GetSize(); i < sz; i++)
    delete m_ContextArray.GetAt(i);

  m_ContextArray.RemoveAll();
  FXJS_ReleaseRuntime(GetIsolate(), m_context);

  m_pApp = NULL;
  m_pDocument = NULL;
  m_context.Reset();

  if (m_isolateManaged)
    m_isolate->Dispose();
}

void CJS_Runtime::DefineJSObjects() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::Context> context = v8::Context::New(GetIsolate());
  v8::Context::Scope context_scope(context);

  // The call order determines the "ObjDefID" assigned to each class.
  // ObjDefIDs 0 - 2
  CJS_Border::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Display::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Font::DefineJSObjects(GetIsolate(), FXJS_STATIC);

  // ObjDefIDs 3 - 5
  CJS_Highlight::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Position::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_ScaleHow::DefineJSObjects(GetIsolate(), FXJS_STATIC);

  // ObjDefIDs 6 - 8
  CJS_ScaleWhen::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Style::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Zoomtype::DefineJSObjects(GetIsolate(), FXJS_STATIC);

  // ObjDefIDs 9 - 11
  CJS_App::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Color::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Console::DefineJSObjects(GetIsolate(), FXJS_STATIC);

  // ObjDefIDs 12 - 14
  CJS_Document::DefineJSObjects(GetIsolate(), FXJS_DYNAMIC);
  CJS_Event::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Field::DefineJSObjects(GetIsolate(), FXJS_DYNAMIC);

  // ObjDefIDs 15 - 17
  CJS_Global::DefineJSObjects(GetIsolate(), FXJS_STATIC);
  CJS_Icon::DefineJSObjects(GetIsolate(), FXJS_DYNAMIC);
  CJS_Util::DefineJSObjects(GetIsolate(), FXJS_STATIC);

  // ObjDefIDs 18 - 20 (these can't fail, return void).
  CJS_PublicMethods::DefineJSObjects(GetIsolate());
  CJS_GlobalConsts::DefineJSObjects(GetIsolate());
  CJS_GlobalArrays::DefineJSObjects(GetIsolate());

  // ObjDefIDs 21 - 22.
  CJS_TimerObj::DefineJSObjects(GetIsolate(), FXJS_DYNAMIC);
  CJS_PrintParamsObj::DefineJSObjects(GetIsolate(), FXJS_DYNAMIC);
}

IFXJS_Context* CJS_Runtime::NewContext() {
  CJS_Context* p = new CJS_Context(this);
  m_ContextArray.Add(p);
  return p;
}

void CJS_Runtime::ReleaseContext(IFXJS_Context* pContext) {
  CJS_Context* pJSContext = (CJS_Context*)pContext;

  for (int i = 0, sz = m_ContextArray.GetSize(); i < sz; i++) {
    if (pJSContext == m_ContextArray.GetAt(i)) {
      delete pJSContext;
      m_ContextArray.RemoveAt(i);
      break;
    }
  }
}

IFXJS_Context* CJS_Runtime::GetCurrentContext() {
  if (!m_ContextArray.GetSize())
    return NULL;
  return m_ContextArray.GetAt(m_ContextArray.GetSize() - 1);
}

void CJS_Runtime::SetReaderDocument(CPDFSDK_Document* pReaderDoc) {
  if (m_pDocument != pReaderDoc) {
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    v8::Local<v8::Context> context =
        v8::Local<v8::Context>::New(m_isolate, m_context);
    v8::Context::Scope context_scope(context);

    m_pDocument = pReaderDoc;
    if (pReaderDoc) {
      v8::Local<v8::Object> pThis = FXJS_GetThisObj(GetIsolate());
      if (!pThis.IsEmpty()) {
        if (FXJS_GetObjDefnID(pThis) ==
            FXJS_GetObjDefnID(GetIsolate(), L"Document")) {
          if (CJS_Document* pJSDocument =
                  (CJS_Document*)FXJS_GetPrivate(GetIsolate(), pThis)) {
            if (Document* pDocument = (Document*)pJSDocument->GetEmbedObject())
              pDocument->AttachDoc(pReaderDoc);
          }
        }
      }
    }
  }
}

bool CJS_Runtime::AddEventToSet(const FieldEvent& event) {
  return m_FieldEventSet.insert(event).second;
}

void CJS_Runtime::RemoveEventFromSet(const FieldEvent& event) {
  m_FieldEventSet.erase(event);
}

v8::Local<v8::Context> CJS_Runtime::NewJSContext() {
  return v8::Local<v8::Context>::New(m_isolate, m_context);
}

void CJS_Runtime::AddObserver(Observer* observer) {
  ASSERT(m_observers.find(observer) == m_observers.end());
  m_observers.insert(observer);
}

void CJS_Runtime::RemoveObserver(Observer* observer) {
  ASSERT(m_observers.find(observer) != m_observers.end());
  m_observers.erase(observer);
}

CFX_WideString ChangeObjName(const CFX_WideString& str) {
  CFX_WideString sRet = str;
  sRet.Replace(L"_", L".");
  return sRet;
}
