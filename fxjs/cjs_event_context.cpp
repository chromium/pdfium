// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_event_context.h"

#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fxjs/cjs_field.h"
#include "fxjs/cjs_runtime.h"
#include "fxjs/js_define.h"
#include "fxjs/js_resources.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-isolate.h"

CJS_EventContext::CJS_EventContext(CJS_Runtime* pRuntime)
    : runtime_(pRuntime), form_fill_env_(pRuntime->GetFormFillEnv()) {}

CJS_EventContext::~CJS_EventContext() = default;

std::optional<IJS_Runtime::JS_Error> CJS_EventContext::RunScript(
    const WideString& script) {
  v8::Isolate::Scope isolate_scope(runtime_->GetIsolate());
  v8::HandleScope handle_scope(runtime_->GetIsolate());
  v8::Local<v8::Context> context = runtime_->GetV8Context();
  v8::Context::Scope context_scope(context);

  if (busy_) {
    return IJS_Runtime::JS_Error(1, 1,
                                 JSGetStringFromID(JSMessage::kBusyError));
  }

  AutoRestorer<bool> restorer(&busy_);
  busy_ = true;

  DCHECK(IsValid());
  CJS_Runtime::FieldEvent event(TargetName(), EventKind());
  if (!runtime_->AddEventToSet(event)) {
    return IJS_Runtime::JS_Error(
        1, 1, JSGetStringFromID(JSMessage::kDuplicateEventError));
  }

  std::optional<IJS_Runtime::JS_Error> err;
  if (script.GetLength() > 0) {
    err = runtime_->ExecuteScript(script);
  }

  runtime_->RemoveEventFromSet(event);
  Destroy();
  return err;
}

CJS_Field* CJS_EventContext::SourceField() {
  v8::Local<v8::Object> pDocObj = runtime_->NewFXJSBoundObject(
      CJS_Document::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pDocObj.IsEmpty()) {
    return nullptr;
  }

  v8::Local<v8::Object> pFieldObj = runtime_->NewFXJSBoundObject(
      CJS_Field::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pFieldObj.IsEmpty()) {
    return nullptr;
  }

  auto* pFormFillEnv = GetFormFillEnv();
  auto* pJSDocument = static_cast<CJS_Document*>(
      CFXJS_Engine::GetBinding(runtime_->GetIsolate(), pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField = static_cast<CJS_Field*>(
      CFXJS_Engine::GetBinding(runtime_->GetIsolate(), pFieldObj));
  pJSField->AttachField(pJSDocument, SourceName());
  return pJSField;
}

CJS_Field* CJS_EventContext::TargetField() {
  v8::Local<v8::Object> pDocObj = runtime_->NewFXJSBoundObject(
      CJS_Document::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pDocObj.IsEmpty()) {
    return nullptr;
  }

  v8::Local<v8::Object> pFieldObj = runtime_->NewFXJSBoundObject(
      CJS_Field::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pFieldObj.IsEmpty()) {
    return nullptr;
  }

  auto* pFormFillEnv = GetFormFillEnv();
  auto* pJSDocument = static_cast<CJS_Document*>(
      CFXJS_Engine::GetBinding(runtime_->GetIsolate(), pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField = static_cast<CJS_Field*>(
      CFXJS_Engine::GetBinding(runtime_->GetIsolate(), pFieldObj));
  pJSField->AttachField(pJSDocument, TargetName());
  return pJSField;
}

void CJS_EventContext::OnDoc_Open(const WideString& strTargetName) {
  Initialize(Kind::kDocOpen);
  target_name_ = strTargetName;
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
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseExit(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseExit);
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseDown(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseDown);
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseUp(bool bModifier,
                                       bool bShift,
                                       CPDF_FormField* pTarget) {
  Initialize(Kind::kFieldMouseUp);
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
}

void CJS_EventContext::OnField_Focus(bool bModifier,
                                     bool bShift,
                                     CPDF_FormField* pTarget,
                                     WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldFocus);
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
}

void CJS_EventContext::OnField_Blur(bool bModifier,
                                    bool bShift,
                                    CPDF_FormField* pTarget,
                                    WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldBlur);
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
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
  commit_key_ = 0;
  change_ = strChange;
  change_ex_ = strChangeEx;
  key_down_ = KeyDown;
  modifier_ = bModifier;
  sel_end_ = pSelEnd;
  sel_start_ = pSelStart;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
  will_commit_ = bWillCommit;
  pb_rc_ = pbRc;
  field_full_ = bFieldFull;
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
  change_ = strChange;
  change_ex_ = strChangeEx;
  key_down_ = bKeyDown;
  modifier_ = bModifier;
  shift_ = bShift;
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
  pb_rc_ = pbRc;
}

void CJS_EventContext::OnField_Calculate(CPDF_FormField* pSource,
                                         CPDF_FormField* pTarget,
                                         WideString* pValue,
                                         bool* pRc) {
  DCHECK(pValue);
  DCHECK(pRc);
  Initialize(Kind::kFieldCalculate);
  if (pSource) {
    source_name_ = pSource->GetFullName();
  }
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
  pb_rc_ = pRc;
}

void CJS_EventContext::OnField_Format(CPDF_FormField* pTarget,
                                      WideString* pValue) {
  DCHECK(pValue);
  Initialize(Kind::kFieldFormat);
  commit_key_ = 0;
  target_name_ = pTarget->GetFullName();
  value_ = pValue;
  will_commit_ = true;
}

void CJS_EventContext::OnExternal_Exec() {
  Initialize(Kind::kExternalExec);
}

void CJS_EventContext::Initialize(Kind kind) {
  kind_ = kind;
  target_name_.clear();
  source_name_.clear();
  change_ = nullptr;
  change_du_.clear();
  change_ex_.clear();
  commit_key_ = -1;
  key_down_ = false;
  modifier_ = false;
  shift_ = false;
  sel_end_ = nullptr;
  sel_end_du_ = 0;
  sel_start_ = nullptr;
  sel_start_du_ = 0;
  will_commit_ = false;
  value_ = nullptr;
  field_full_ = false;
  pb_rc_ = nullptr;
  rc_du_ = false;
  valid_ = true;
}

void CJS_EventContext::Destroy() {
  valid_ = false;
}

bool CJS_EventContext::IsUserGesture() const {
  switch (kind_) {
    case Kind::kFieldMouseDown:
    case Kind::kFieldMouseUp:
    case Kind::kFieldKeystroke:
      return true;
    default:
      return false;
  }
}

WideString& CJS_EventContext::Change() {
  return change_ ? *change_ : change_du_;
}

ByteStringView CJS_EventContext::Name() const {
  switch (kind_) {
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
  switch (kind_) {
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
  return pb_rc_ ? *pb_rc_ : rc_du_;
}

int CJS_EventContext::SelEnd() const {
  return sel_end_ ? *sel_end_ : sel_end_du_;
}

int CJS_EventContext::SelStart() const {
  return sel_start_ ? *sel_start_ : sel_start_du_;
}

void CJS_EventContext::SetSelEnd(int value) {
  int& target = sel_end_ ? *sel_end_ : sel_end_du_;
  target = value;
}

void CJS_EventContext::SetSelStart(int value) {
  int& target = sel_start_ ? *sel_start_ : sel_start_du_;
  target = value;
}
