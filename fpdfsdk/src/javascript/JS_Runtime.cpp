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

// static
void IJS_Runtime::Initialize(unsigned int slot, void* isolate) {
  FXJS_Initialize(slot, reinterpret_cast<v8::Isolate*>(isolate));
}

// static
IJS_Runtime* IJS_Runtime::Create(CPDFDoc_Environment* pEnv) {
  return new CJS_Runtime(pEnv);
}

// static
CJS_Runtime* CJS_Runtime::FromContext(const IJS_Context* cc) {
  const CJS_Context* pContext = static_cast<const CJS_Context*>(cc);
  return pContext->GetJSRuntime();
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
  FXJS_InitializeRuntime(GetIsolate(), this, m_context);
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
  CJS_Border::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Display::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Font::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);

  // ObjDefIDs 3 - 5
  CJS_Highlight::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Position::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_ScaleHow::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);

  // ObjDefIDs 6 - 8
  CJS_ScaleWhen::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Style::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Zoomtype::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);

  // ObjDefIDs 9 - 11
  CJS_App::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Color::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Console::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);

  // ObjDefIDs 12 - 14
  CJS_Document::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_GLOBAL);
  CJS_Event::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Field::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_DYNAMIC);

  // ObjDefIDs 15 - 17
  CJS_Global::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);
  CJS_Icon::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_DYNAMIC);
  CJS_Util::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_STATIC);

  // ObjDefIDs 18 - 20 (these can't fail, return void).
  CJS_PublicMethods::DefineJSObjects(GetIsolate());
  CJS_GlobalConsts::DefineJSObjects(this);
  CJS_GlobalArrays::DefineJSObjects(this);

  // ObjDefIDs 21 - 22.
  CJS_TimerObj::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_DYNAMIC);
  CJS_PrintParamsObj::DefineJSObjects(GetIsolate(), FXJSOBJTYPE_DYNAMIC);
}

IJS_Context* CJS_Runtime::NewContext() {
  CJS_Context* p = new CJS_Context(this);
  m_ContextArray.Add(p);
  return p;
}

void CJS_Runtime::ReleaseContext(IJS_Context* pContext) {
  CJS_Context* pJSContext = (CJS_Context*)pContext;

  for (int i = 0, sz = m_ContextArray.GetSize(); i < sz; i++) {
    if (pJSContext == m_ContextArray.GetAt(i)) {
      delete pJSContext;
      m_ContextArray.RemoveAt(i);
      break;
    }
  }
}

IJS_Context* CJS_Runtime::GetCurrentContext() {
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
        if (FXJS_GetObjDefnID(pThis) == CJS_Document::g_nObjDefnID) {
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

int CJS_Runtime::Execute(IJS_Context* cc,
                         const wchar_t* script,
                         CFX_WideString* info) {
  FXJSErr error = {};
  int nRet = FXJS_Execute(m_isolate, cc, script, &error);
  if (nRet < 0) {
    info->Format(L"[ Line: %05d { %s } ] : %s", error.linnum - 1, error.srcline,
                 error.message);
  }
  return nRet;
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
