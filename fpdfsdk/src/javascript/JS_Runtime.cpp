// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/Document.h"
#include "../../include/javascript/app.h"
#include "../../include/javascript/color.h"
#include "../../include/javascript/Consts.h"
#include "../../include/javascript/Document.h"
#include "../../include/javascript/event.h"
#include "../../include/javascript/Field.h"
#include "../../include/javascript/Icon.h"
#include "../../include/javascript/PublicMethods.h"
#include "../../include/javascript/report.h"
#include "../../include/javascript/util.h"
#include "../../include/javascript/JS_GlobalData.h"
#include "../../include/javascript/global.h"
#include "../../include/javascript/console.h"
#include "../../include/fpdfxfa/fpdfxfa_app.h"
#include "../../../xfa/src/fxjse/src/value.h"

/* ------------------------------ CJS_Runtime ------------------------------ */
v8::Global<v8::ObjectTemplate>& _getGlobalObjectTemplate(v8::Isolate* pIsolate);

CJS_Runtime::CJS_Runtime(CPDFDoc_Environment* pApp)
    : m_pApp(pApp),
      m_pDocument(NULL),
      m_bBlocking(FALSE),
      m_pFieldEventPath(NULL),
      m_isolate(NULL),
      m_isolateManaged(false) {
  if (CPDFXFA_App::GetInstance()->GetJSERuntime()) {
    // TODO(tsepez): CPDFXFA_App should also use the embedder provided isolate.
    m_isolate = (v8::Isolate*)CPDFXFA_App::GetInstance()->GetJSERuntime();
  } else if (m_pApp->GetFormFillInfo()->m_pJsPlatform->version >= 2) {
    m_isolate = reinterpret_cast<v8::Isolate*>(
        m_pApp->GetFormFillInfo()->m_pJsPlatform->m_isolate);
  }
  if (!m_isolate) {
    m_pArrayBufferAllocator.reset(new FXJS_ArrayBufferAllocator());

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = m_pArrayBufferAllocator.get();
    m_isolate = v8::Isolate::New(params);
    m_isolateManaged = true;
  }

  v8::Isolate* isolate = m_isolate;
  v8::Isolate::Scope isolate_scope(isolate);
  v8::Locker locker(isolate);
  v8::HandleScope handle_scope(isolate);
  if (CPDFXFA_App::GetInstance()->IsJavaScriptInitialized()) {
    CJS_Context* pContext = (CJS_Context*)NewContext();
    FXJS_InitializeRuntime(GetIsolate(), this, pContext, m_context);
    ReleaseContext(pContext);
    return;
  }

  unsigned int embedderDataSlot = 0;
  if (m_pApp->GetFormFillInfo()->m_pJsPlatform->version >= 2)
    embedderDataSlot = pApp->GetFormFillInfo()->m_pJsPlatform->m_v8EmbedderSlot;
  FXJS_Initialize(embedderDataSlot);
  DefineJSObjects();
  CPDFXFA_App::GetInstance()->SetJavaScriptInitialized(TRUE);

  CJS_Context* pContext = (CJS_Context*)NewContext();
  FXJS_InitializeRuntime(GetIsolate(), this, pContext, m_context);
  ReleaseContext(pContext);
}

CJS_Runtime::~CJS_Runtime() {
  int size = m_ContextArray.GetSize();
  for (int i = 0; i < size; i++)
    delete m_ContextArray.GetAt(i);

  m_ContextArray.RemoveAll();
  RemoveEventsInLoop(m_pFieldEventPath);

  m_pApp = NULL;
  m_pDocument = NULL;
  m_pFieldEventPath = NULL;
  m_context.Reset();

  if (m_isolateManaged)
    m_isolate->Dispose();
  m_isolate = NULL;
}

void CJS_Runtime::DefineJSObjects() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::Locker locker(GetIsolate());
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
    v8::Locker locker(m_isolate);
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

FX_BOOL CJS_Runtime::AddEventToLoop(const CFX_WideString& sTargetName,
                                    JS_EVENT_T eEventType) {
  if (m_pFieldEventPath == NULL) {
    m_pFieldEventPath = new CJS_FieldEvent;
    m_pFieldEventPath->sTargetName = sTargetName;
    m_pFieldEventPath->eEventType = eEventType;
    m_pFieldEventPath->pNext = NULL;

    return TRUE;
  }

  // to search
  CJS_FieldEvent* p = m_pFieldEventPath;
  CJS_FieldEvent* pLast = m_pFieldEventPath;
  while (p) {
    if (p->eEventType == eEventType && p->sTargetName == sTargetName)
      return FALSE;

    pLast = p;
    p = p->pNext;
  }

  // to add
  CJS_FieldEvent* pNew = new CJS_FieldEvent;
  pNew->sTargetName = sTargetName;
  pNew->eEventType = eEventType;
  pNew->pNext = NULL;

  pLast->pNext = pNew;

  return TRUE;
}

void CJS_Runtime::RemoveEventInLoop(const CFX_WideString& sTargetName,
                                    JS_EVENT_T eEventType) {
  FX_BOOL bFind = FALSE;

  CJS_FieldEvent* p = m_pFieldEventPath;
  CJS_FieldEvent* pLast = NULL;
  while (p) {
    if (p->eEventType == eEventType && p->sTargetName == sTargetName) {
      bFind = TRUE;
      break;
    }

    pLast = p;
    p = p->pNext;
  }

  if (bFind) {
    RemoveEventsInLoop(p);

    if (p == m_pFieldEventPath)
      m_pFieldEventPath = NULL;

    if (pLast)
      pLast->pNext = NULL;
  }
}

void CJS_Runtime::RemoveEventsInLoop(CJS_FieldEvent* pStart) {
  CJS_FieldEvent* p = pStart;

  while (p) {
    CJS_FieldEvent* pOld = p;
    p = pOld->pNext;

    delete pOld;
  }
}

v8::Local<v8::Context> CJS_Runtime::NewJSContext() {
  return v8::Local<v8::Context>::New(m_isolate, m_context);
}

CFX_WideString ChangeObjName(const CFX_WideString& str) {
  CFX_WideString sRet = str;
  sRet.Replace(L"_", L".");
  return sRet;
}
FX_BOOL CJS_Runtime::GetHValueByName(const CFX_ByteStringC& utf8Name,
                                     FXJSE_HVALUE hValue) {
  const FX_CHAR* name = utf8Name.GetCStr();

  v8::Locker lock(GetIsolate());
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::Context> context =
      v8::Local<v8::Context>::New(GetIsolate(), m_context);
  v8::Context::Scope context_scope(context);

  // v8::Local<v8::Context> tmpCotext =
  // v8::Local<v8::Context>::New(GetIsolate(), m_context);
  v8::Local<v8::Value> propvalue =
      context->Global()->Get(v8::String::NewFromUtf8(
          GetIsolate(), name, v8::String::kNormalString, utf8Name.GetLength()));

  if (propvalue.IsEmpty()) {
    FXJSE_Value_SetUndefined(hValue);
    return FALSE;
  }
  ((CFXJSE_Value*)hValue)->ForceSetValue(propvalue);

  return TRUE;
}
FX_BOOL CJS_Runtime::SetHValueByName(const CFX_ByteStringC& utf8Name,
                                     FXJSE_HVALUE hValue) {
  if (utf8Name.IsEmpty() || hValue == NULL)
    return FALSE;
  const FX_CHAR* name = utf8Name.GetCStr();
  v8::Isolate* pIsolate = GetIsolate();
  v8::Locker lock(pIsolate);
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);
  v8::Local<v8::Context> context =
      v8::Local<v8::Context>::New(pIsolate, m_context);
  v8::Context::Scope context_scope(context);

  // v8::Local<v8::Context> tmpCotext =
  // v8::Local<v8::Context>::New(GetIsolate(), m_context);
  v8::Local<v8::Value> propvalue = v8::Local<v8::Value>::New(
      GetIsolate(), ((CFXJSE_Value*)hValue)->DirectGetValue());
  context->Global()->Set(
      v8::String::NewFromUtf8(pIsolate, name, v8::String::kNormalString,
                              utf8Name.GetLength()),
      propvalue);

  return TRUE;
}
