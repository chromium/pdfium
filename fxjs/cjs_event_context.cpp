// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_event_context.h"

#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcrt/autorestorer.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fxjs/cjs_field.h"
#include "fxjs/cjs_runtime.h"
#include "fxjs/js_define.h"
#include "fxjs/js_resources.h"
#include "third_party/base/check.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-isolate.h"

CJS_EventContext::CJS_EventContext(CJS_Runtime* pRuntime)
    : m_pRuntime(pRuntime), m_pFormFillEnv(pRuntime->GetFormFillEnv()) {}

CJS_EventContext::~CJS_EventContext() = default;

absl::optional<IJS_Runtime::JS_Error> CJS_EventContext::RunScript(
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

  DCHECK(IsValid());
  CJS_Runtime::FieldEvent event(TargetName(), EventKind());
  if (!m_pRuntime->AddEventToSet(event)) {
    return IJS_Runtime::JS_Error(
        1, 1, JSGetStringFromID(JSMessage::kDuplicateEventError));
  }

  absl::optional<IJS_Runtime::JS_Error> err;
  if (script.GetLength() > 0)
    err = m_pRuntime->ExecuteScript(script);

  m_pRuntime->RemoveEventFromSet(event);
  Destroy();
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

  auto* pFormFillEnv = GetFormFillEnv();
  auto* pJSDocument = static_cast<CJS_Document*>(
      CFXJS_Engine::GetObjectPrivate(m_pRuntime->GetIsolate(), pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField = static_cast<CJS_Field*>(
      CFXJS_Engine::GetObjectPrivate(m_pRuntime->GetIsolate(), pFieldObj));
  pJSField->AttachField(pJSDocument, SourceName());
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

  auto* pFormFillEnv = GetFormFillEnv();
  auto* pJSDocument = static_cast<CJS_Document*>(
      CFXJS_Engine::GetObjectPrivate(m_pRuntime->GetIsolate(), pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField = static_cast<CJS_Field*>(
      CFXJS_Engine::GetObjectPrivate(m_pRuntime->GetIsolate(), pFieldObj));
  pJSField->AttachField(pJSDocument, TargetName());
  return pJSField;
}

void CJS_EventContext::OnDoc_Open(const WideString& strTargetName) {
  Initialize(Kind::kDocOpen);
  m_strTargetName = strTargetName;
}

void CJS_EventContext::OnDoc_WillPrint() {
  Initialize(Kind::kDocWillPrint);
}

void CJS_EventContext::OnDoc_DidPrint() {
  Initialize(Kind::kDocDidPrint);
}

void CJS_EventContext::OnDoc_WillSave() {
  Initialize(Kind::kDocWillSave);
}

void CJS_EventContext::OnDoc_DidSave() {
  Initialize(Kind::kDocDidSave);
}

void CJS_EventContext::OnDoc_WillClose() {
  Initialize(Kind::kDocWillClose);
}

void CJS_EventContext::OnPage_Open() {
  Initialize(Kind::kPageOpen);
}

void CJS_EventContext::OnPage_Close() {
  Initialize(Kind::kPageClose);
}

void CJS_EventContext::OnPage_InView() {
  Initialize(Kind::kPageInView);
}

void CJS_EventContext::OnPage_OutView() {
  Initialize(Kind::kPageOutView);
}

void CJS_EventContext::OnField_MouseEnter(bool bModifier,
                                          bool bShift,
                                          CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseEnter);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseExit(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseExit);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseDown(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseDown);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseUp(bool bModifier,
                                       bool bShift,
                                       CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseUp);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_Focus(bool bModifier,
                                     bool bShift,
                                     CPDF_FormField* pTarget,
                                     WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldFocus);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
}

void CJS_EventContext::OnField_Blur(bool bModifier,
                                    bool bShift,
                                    CPDF_FormField* pTarget,
                                    WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldBlur);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
}

void CJS_EventContext::OnField_Keystroke(WideString* strChange,
                                         const WideString& strChangeEx,
                                         bool KeyDown,
                                         bool bModifier,
                                         int* pSelEnd,
                                         int* pSelStart,
                                         bool bShift,
                                         CPDF_FormField* pTarget,
                                         WideString* pValue,
                                         bool bWillCommit,
                                         bool bFieldFull,
                                         bool* pbRc) {
  DCHECK(pValue);
  DCHECK(pbRc);
  DCHECK(pSelStart);
  DCHECK(pSelEnd);

  Initialize(Kind::kFieldKeystroke);
  m_nCommitKey = 0;
  m_pWideStrChange = strChange;
  m_WideStrChangeEx = strChangeEx;
  m_bKeyDown = KeyDown;
  m_bModifier = bModifier;
  m_pISelEnd = pSelEnd;
  m_pISelStart = pSelStart;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_bWillCommit = bWillCommit;
  m_pbRc = pbRc;
  m_bFieldFull = bFieldFull;
}

void CJS_EventContext::OnField_Validate(WideString* strChange,
                                        const WideString& strChangeEx,
                                        bool bKeyDown,
                                        bool bModifier,
                                        bool bShift,
                                        CPDF_FormField* pTarget,
                                        WideString* pValue,
                                        bool* pbRc) {
  DCHECK(pValue);
  DCHECK(pbRc);
  Initialize(Kind::kFieldValidate);
  m_pWideStrChange = strChange;
  m_WideStrChangeEx = strChangeEx;
  m_bKeyDown = bKeyDown;
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_pbRc = pbRc;
}

void CJS_EventContext::OnField_Calculate(CPDF_FormField* pSource,
                                         CPDF_FormField* pTarget,
                                         WideString* pValue,
                                         bool* pRc) {
  DCHECK(pValue);
  DCHECK(pRc);
  Initialize(Kind::kFieldCalculate);
  if (pSource)
    m_strSourceName = pSource->GetFullName();
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_pbRc = pRc;
}

void CJS_EventContext::OnField_Format(CPDF_FormField* pTarget,
                                      WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldFormat);
  m_nCommitKey = 0;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_bWillCommit = true;
}

void CJS_EventContext::OnExternal_Exec() {
  Initialize(Kind::kExternalExec);
}

void CJS_EventContext::Initialize(Kind kind) {
  m_eKind = kind;
  m_strTargetName.clear();
  m_strSourceName.clear();
  m_pWideStrChange = nullptr;
  m_WideStrChangeDu.clear();
  m_WideStrChangeEx.clear();
  m_nCommitKey = -1;
  m_bKeyDown = false;
  m_bModifier = false;
  m_bShift = false;
  m_pISelEnd = nullptr;
  m_nSelEndDu = 0;
  m_pISelStart = nullptr;
  m_nSelStartDu = 0;
  m_bWillCommit = false;
  m_pValue = nullptr;
  m_bFieldFull = false;
  m_pbRc = nullptr;
  m_bRcDu = false;
  m_bValid = true;
}

void CJS_EventContext::Destroy() {
  m_bValid = false;
}

bool CJS_EventContext::IsUserGesture() const {
  switch (m_eKind) {
    case Kind::kFieldMouseDown:
    case Kind::kFieldMouseUp:
    case Kind::kFieldKeystroke:
      return true;
    default:
      return false;
  }
}

WideString& CJS_EventContext::Change() {
  return m_pWideStrChange ? *m_pWideStrChange : m_WideStrChangeDu;
}

ByteStringView CJS_EventContext::Name() const {
  switch (m_eKind) {
    case Kind::kDocDidPrint:
      return "DidPrint";
    case Kind::kDocDidSave:
      return "DidSave";
    case Kind::kDocOpen:
      return "Open";
    case Kind::kDocWillClose:
      return "WillClose";
    case Kind::kDocWillPrint:
      return "WillPrint";
    case Kind::kDocWillSave:
      return "WillSave";
    case Kind::kExternalExec:
      return "Exec";
    case Kind::kFieldFocus:
      return "Focus";
    case Kind::kFieldBlur:
      return "Blur";
    case Kind::kFieldMouseDown:
      return "Mouse Down";
    case Kind::kFieldMouseUp:
      return "Mouse Up";
    case Kind::kFieldMouseEnter:
      return "Mouse Enter";
    case Kind::kFieldMouseExit:
      return "Mouse Exit";
    case Kind::kFieldCalculate:
      return "Calculate";
    case Kind::kFieldFormat:
      return "Format";
    case Kind::kFieldKeystroke:
      return "Keystroke";
    case Kind::kFieldValidate:
      return "Validate";
    case Kind::kPageOpen:
      return "Open";
    case Kind::kPageClose:
      return "Close";
    case Kind::kPageInView:
      return "InView";
    case Kind::kPageOutView:
      return "OutView";
    default:
      return "";
  }
}

ByteStringView CJS_EventContext::Type() const {
  switch (m_eKind) {
    case Kind::kDocDidPrint:
    case Kind::kDocDidSave:
    case Kind::kDocOpen:
    case Kind::kDocWillClose:
    case Kind::kDocWillPrint:
    case Kind::kDocWillSave:
      return "Doc";
    case Kind::kExternalExec:
      return "External";
    case Kind::kFieldBlur:
    case Kind::kFieldFocus:
    case Kind::kFieldMouseDown:
    case Kind::kFieldMouseUp:
    case Kind::kFieldMouseEnter:
    case Kind::kFieldMouseExit:
    case Kind::kFieldCalculate:
    case Kind::kFieldFormat:
    case Kind::kFieldKeystroke:
    case Kind::kFieldValidate:
      return "Field";
    case Kind::kPageOpen:
    case Kind::kPageClose:
    case Kind::kPageInView:
    case Kind::kPageOutView:
      return "Page";
    default:
      return "";
  }
}

bool& CJS_EventContext::Rc() {
  return m_pbRc ? *m_pbRc : m_bRcDu;
}

int CJS_EventContext::SelEnd() const {
  return m_pISelEnd ? *m_pISelEnd : m_nSelEndDu;
}

int CJS_EventContext::SelStart() const {
  return m_pISelStart ? *m_pISelStart : m_nSelStartDu;
}

void CJS_EventContext::SetSelEnd(int value) {
  int& target = m_pISelEnd ? *m_pISelEnd : m_nSelEndDu;
  target = value;
}

void CJS_EventContext::SetSelStart(int value) {
  int& target = m_pISelStart ? *m_pISelStart : m_nSelStartDu;
  target = value;
}
