// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_event_context.h"

#include "core/fxcrt/autorestorer.h"
#include "fxjs/cjs_eventrecorder.h"
#include "fxjs/cjs_field.h"
#include "fxjs/cjs_runtime.h"
#include "fxjs/js_define.h"
#include "fxjs/js_resources.h"
#include "third_party/base/ptr_util.h"

CJS_EventContext::CJS_EventContext(CJS_Runtime* pRuntime)
    : m_pRuntime(pRuntime),
      m_pEventRecorder(pdfium::MakeUnique<CJS_EventRecorder>()) {
  ASSERT(pRuntime);
}

CJS_EventContext::~CJS_EventContext() = default;

CPDFSDK_FormFillEnvironment* CJS_EventContext::GetFormFillEnv() {
  return m_pRuntime->GetFormFillEnv();
}

Optional<IJS_Runtime::JS_Error> CJS_EventContext::RunScript(
    const WideString& script) {
  v8::Isolate::Scope isolate_scope(m_pRuntime->GetIsolate());
  v8::HandleScope handle_scope(m_pRuntime->GetIsolate());
  v8::Local<v8::Context> context = m_pRuntime->GetV8Context();
  v8::Context::Scope context_scope(context);

  if (m_bBusy) {
    return IJS_Runtime::JS_Error(1, 1,
                                 JSGetStringFromID(JSMessage::kBusyError));
  }

  AutoRestorer<bool> restorer(&m_bBusy);
  m_bBusy = true;

  ASSERT(m_pEventRecorder->IsValid());
  CJS_Runtime::FieldEvent event(m_pEventRecorder->TargetName(),
                                m_pEventRecorder->EventType());
  if (!m_pRuntime->AddEventToSet(event)) {
    return IJS_Runtime::JS_Error(
        1, 1, JSGetStringFromID(JSMessage::kDuplicateEventError));
  }

  Optional<IJS_Runtime::JS_Error> err;
  if (script.GetLength() > 0)
    err = m_pRuntime->ExecuteScript(script);

  m_pRuntime->RemoveEventFromSet(event);
  m_pEventRecorder->Destroy();
  return err;
}

CJS_Field* CJS_EventContext::SourceField() {
  v8::Local<v8::Object> pDocObj = m_pRuntime->NewFXJSBoundObject(
      CJS_Document::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pDocObj.IsEmpty())
    return nullptr;

  v8::Local<v8::Object> pFieldObj = m_pRuntime->NewFXJSBoundObject(
      CJS_Field::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pFieldObj.IsEmpty())
    return nullptr;

  auto* pFormFillEnv = m_pEventRecorder->GetFormFillEnvironment();
  if (!pFormFillEnv)
    pFormFillEnv = GetFormFillEnv();

  auto* pJSDocument =
      static_cast<CJS_Document*>(CFXJS_Engine::GetObjectPrivate(pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField =
      static_cast<CJS_Field*>(CFXJS_Engine::GetObjectPrivate(pFieldObj));
  pJSField->AttachField(pJSDocument, m_pEventRecorder->SourceName());
  return pJSField;
}

CJS_Field* CJS_EventContext::TargetField() {
  v8::Local<v8::Object> pDocObj = m_pRuntime->NewFXJSBoundObject(
      CJS_Document::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pDocObj.IsEmpty())
    return nullptr;

  v8::Local<v8::Object> pFieldObj = m_pRuntime->NewFXJSBoundObject(
      CJS_Field::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pFieldObj.IsEmpty())
    return nullptr;

  auto* pFormFillEnv = m_pEventRecorder->GetFormFillEnvironment();
  if (!pFormFillEnv)
    pFormFillEnv = GetFormFillEnv();

  auto* pJSDocument =
      static_cast<CJS_Document*>(CFXJS_Engine::GetObjectPrivate(pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField =
      static_cast<CJS_Field*>(CFXJS_Engine::GetObjectPrivate(pFieldObj));
  pJSField->AttachField(pJSDocument, m_pEventRecorder->TargetName());
  return pJSField;
}

void CJS_EventContext::OnApp_Init() {
  m_pEventRecorder->OnApp_Init();
}

void CJS_EventContext::OnDoc_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                  const WideString& strTargetName) {
  m_pEventRecorder->OnDoc_Open(pFormFillEnv, strTargetName);
}

void CJS_EventContext::OnDoc_WillPrint(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnDoc_WillPrint(pFormFillEnv);
}

void CJS_EventContext::OnDoc_DidPrint(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnDoc_DidPrint(pFormFillEnv);
}

void CJS_EventContext::OnDoc_WillSave(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnDoc_WillSave(pFormFillEnv);
}

void CJS_EventContext::OnDoc_DidSave(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnDoc_DidSave(pFormFillEnv);
}

void CJS_EventContext::OnDoc_WillClose(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnDoc_WillClose(pFormFillEnv);
}

void CJS_EventContext::OnPage_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnPage_Open(pFormFillEnv);
}

void CJS_EventContext::OnPage_Close(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnPage_Close(pFormFillEnv);
}

void CJS_EventContext::OnPage_InView(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnPage_InView(pFormFillEnv);
}

void CJS_EventContext::OnPage_OutView(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnPage_OutView(pFormFillEnv);
}

void CJS_EventContext::OnField_MouseDown(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  m_pEventRecorder->OnField_MouseDown(bModifier, bShift, pTarget);
}

void CJS_EventContext::OnField_MouseEnter(bool bModifier,
                                          bool bShift,
                                          CPDF_FormField* pTarget) {
  m_pEventRecorder->OnField_MouseEnter(bModifier, bShift, pTarget);
}

void CJS_EventContext::OnField_MouseExit(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  m_pEventRecorder->OnField_MouseExit(bModifier, bShift, pTarget);
}

void CJS_EventContext::OnField_MouseUp(bool bModifier,
                                       bool bShift,
                                       CPDF_FormField* pTarget) {
  m_pEventRecorder->OnField_MouseUp(bModifier, bShift, pTarget);
}

void CJS_EventContext::OnField_Focus(bool bModifier,
                                     bool bShift,
                                     CPDF_FormField* pTarget,
                                     WideString* Value) {
  m_pEventRecorder->OnField_Focus(bModifier, bShift, pTarget, Value);
}

void CJS_EventContext::OnField_Blur(bool bModifier,
                                    bool bShift,
                                    CPDF_FormField* pTarget,
                                    WideString* Value) {
  m_pEventRecorder->OnField_Blur(bModifier, bShift, pTarget, Value);
}

void CJS_EventContext::OnField_Calculate(CPDF_FormField* pSource,
                                         CPDF_FormField* pTarget,
                                         WideString* pValue,
                                         bool* pRc) {
  m_pEventRecorder->OnField_Calculate(pSource, pTarget, pValue, pRc);
}

void CJS_EventContext::OnField_Format(CPDF_FormField* pTarget,
                                      WideString* Value) {
  m_pEventRecorder->OnField_Format(pTarget, Value);
}

void CJS_EventContext::OnField_Keystroke(WideString* strChange,
                                         const WideString& strChangeEx,
                                         bool bKeyDown,
                                         bool bModifier,
                                         int* nSelEnd,
                                         int* nSelStart,
                                         bool bShift,
                                         CPDF_FormField* pTarget,
                                         WideString* Value,
                                         bool bWillCommit,
                                         bool bFieldFull,
                                         bool* bRc) {
  m_pEventRecorder->OnField_Keystroke(
      strChange, strChangeEx, bKeyDown, bModifier, nSelEnd, nSelStart, bShift,
      pTarget, Value, bWillCommit, bFieldFull, bRc);
}

void CJS_EventContext::OnField_Validate(WideString* strChange,
                                        const WideString& strChangeEx,
                                        bool bKeyDown,
                                        bool bModifier,
                                        bool bShift,
                                        CPDF_FormField* pTarget,
                                        WideString* Value,
                                        bool* bRc) {
  m_pEventRecorder->OnField_Validate(strChange, strChangeEx, bKeyDown,
                                     bModifier, bShift, pTarget, Value, bRc);
}

void CJS_EventContext::OnScreen_Focus(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_Focus(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_Blur(bool bModifier,
                                     bool bShift,
                                     CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_Blur(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_Open(bool bModifier,
                                     bool bShift,
                                     CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_Open(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_Close(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_Close(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_MouseDown(bool bModifier,
                                          bool bShift,
                                          CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_MouseDown(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_MouseUp(bool bModifier,
                                        bool bShift,
                                        CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_MouseUp(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_MouseEnter(bool bModifier,
                                           bool bShift,
                                           CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_MouseEnter(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_MouseExit(bool bModifier,
                                          bool bShift,
                                          CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_MouseExit(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_InView(bool bModifier,
                                       bool bShift,
                                       CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_InView(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnScreen_OutView(bool bModifier,
                                        bool bShift,
                                        CPDFSDK_Annot* pScreen) {
  m_pEventRecorder->OnScreen_OutView(bModifier, bShift, pScreen);
}

void CJS_EventContext::OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) {
  m_pEventRecorder->OnBookmark_MouseUp(pBookMark);
}

void CJS_EventContext::OnLink_MouseUp(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnLink_MouseUp(pFormFillEnv);
}

void CJS_EventContext::OnConsole_Exec() {
  m_pEventRecorder->OnConsole_Exec();
}

void CJS_EventContext::OnExternal_Exec() {
  m_pEventRecorder->OnExternal_Exec();
}

void CJS_EventContext::OnBatchExec(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pEventRecorder->OnBatchExec(pFormFillEnv);
}

void CJS_EventContext::OnMenu_Exec(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                   const WideString& strTargetName) {
  m_pEventRecorder->OnMenu_Exec(pFormFillEnv, strTargetName);
}
