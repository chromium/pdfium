// Copyright 2017 PDFium Authors. All rights reserved.
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

CJS_EventContext::CJS_EventContext(CJS_Runtime* pRuntime)
    : m_pRuntime(pRuntime), m_pFormFillEnv(pRuntime->GetFormFillEnv()) {}

CJS_EventContext::~CJS_EventContext() = default;

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

  DCHECK(IsValid());
  CJS_Runtime::FieldEvent event(TargetName(), EventType());
  if (!m_pRuntime->AddEventToSet(event)) {
    return IJS_Runtime::JS_Error(
        1, 1, JSGetStringFromID(JSMessage::kDuplicateEventError));
  }

  Optional<IJS_Runtime::JS_Error> err;
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
  if (!pFormFillEnv)
    pFormFillEnv = GetFormFillEnv();

  auto* pJSDocument =
      static_cast<CJS_Document*>(CFXJS_Engine::GetObjectPrivate(pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField =
      static_cast<CJS_Field*>(CFXJS_Engine::GetObjectPrivate(pFieldObj));
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
  if (!pFormFillEnv)
    pFormFillEnv = GetFormFillEnv();

  auto* pJSDocument =
      static_cast<CJS_Document*>(CFXJS_Engine::GetObjectPrivate(pDocObj));
  pJSDocument->SetFormFillEnv(pFormFillEnv);

  auto* pJSField =
      static_cast<CJS_Field*>(CFXJS_Engine::GetObjectPrivate(pFieldObj));
  pJSField->AttachField(pJSDocument, TargetName());
  return pJSField;
}

void CJS_EventContext::OnApp_Init() {
  Initialize(JET_APP_INIT);
}

void CJS_EventContext::OnDoc_Open(const WideString& strTargetName) {
  Initialize(JET_DOC_OPEN);
  m_strTargetName = strTargetName;
}

void CJS_EventContext::OnDoc_WillPrint() {
  Initialize(JET_DOC_WILLPRINT);
}

void CJS_EventContext::OnDoc_DidPrint() {
  Initialize(JET_DOC_DIDPRINT);
}

void CJS_EventContext::OnDoc_WillSave() {
  Initialize(JET_DOC_WILLSAVE);
}

void CJS_EventContext::OnDoc_DidSave() {
  Initialize(JET_DOC_DIDSAVE);
}

void CJS_EventContext::OnDoc_WillClose() {
  Initialize(JET_DOC_WILLCLOSE);
}

void CJS_EventContext::OnPage_Open() {
  Initialize(JET_PAGE_OPEN);
}

void CJS_EventContext::OnPage_Close() {
  Initialize(JET_PAGE_CLOSE);
}

void CJS_EventContext::OnPage_InView() {
  Initialize(JET_PAGE_INVIEW);
}

void CJS_EventContext::OnPage_OutView() {
  Initialize(JET_PAGE_OUTVIEW);
}

void CJS_EventContext::OnField_MouseEnter(bool bModifier,
                                          bool bShift,
                                          CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEENTER);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseExit(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEEXIT);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseDown(bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEDOWN);
  m_eEventType = JET_FIELD_MOUSEDOWN;

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_MouseUp(bool bModifier,
                                       bool bShift,
                                       CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEUP);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventContext::OnField_Focus(bool bModifier,
                                     bool bShift,
                                     CPDF_FormField* pTarget,
                                     WideString* pValue) {
  DCHECK(pValue);
  Initialize(JET_FIELD_FOCUS);

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
  Initialize(JET_FIELD_BLUR);

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

  Initialize(JET_FIELD_KEYSTROKE);

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

  Initialize(JET_FIELD_VALIDATE);

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

  Initialize(JET_FIELD_CALCULATE);

  if (pSource)
    m_strSourceName = pSource->GetFullName();
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_pbRc = pRc;
}

void CJS_EventContext::OnField_Format(CPDF_FormField* pTarget,
                                      WideString* pValue) {
  DCHECK(pValue);
  Initialize(JET_FIELD_FORMAT);

  m_nCommitKey = 0;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_bWillCommit = true;
}

void CJS_EventContext::OnScreen_Focus(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_FOCUS);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_Blur(bool bModifier,
                                     bool bShift,
                                     CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_BLUR);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_Open(bool bModifier,
                                     bool bShift,
                                     CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_OPEN);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_Close(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_CLOSE);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_MouseDown(bool bModifier,
                                          bool bShift,
                                          CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEDOWN);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_MouseUp(bool bModifier,
                                        bool bShift,
                                        CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEUP);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_MouseEnter(bool bModifier,
                                           bool bShift,
                                           CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEENTER);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_MouseExit(bool bModifier,
                                          bool bShift,
                                          CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEEXIT);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_InView(bool bModifier,
                                       bool bShift,
                                       CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_INVIEW);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnScreen_OutView(bool bModifier,
                                        bool bShift,
                                        CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_OUTVIEW);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventContext::OnLink_MouseUp() {
  Initialize(JET_LINK_MOUSEUP);
}

void CJS_EventContext::OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) {
  Initialize(JET_BOOKMARK_MOUSEUP);
  m_pTargetBookMark = pBookMark;
}

void CJS_EventContext::OnMenu_Exec(const WideString& strTargetName) {
  Initialize(JET_MENU_EXEC);
  m_strTargetName = strTargetName;
}

void CJS_EventContext::OnExternal_Exec() {
  Initialize(JET_EXTERNAL_EXEC);
}

void CJS_EventContext::OnBatch_Exec() {
  Initialize(JET_BATCH_EXEC);
}

void CJS_EventContext::OnConsole_Exec() {
  Initialize(JET_CONSOLE_EXEC);
}

void CJS_EventContext::Initialize(JS_EVENT_T type) {
  m_eEventType = type;
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
  m_pTargetBookMark = nullptr;
  m_pTargetAnnot.Reset();
  m_bValid = true;
}

void CJS_EventContext::Destroy() {
  m_bValid = false;
}

bool CJS_EventContext::IsUserGesture() const {
  switch (m_eEventType) {
    case JET_FIELD_MOUSEDOWN:
    case JET_FIELD_MOUSEUP:
    case JET_SCREEN_MOUSEDOWN:
    case JET_SCREEN_MOUSEUP:
    case JET_BOOKMARK_MOUSEUP:
    case JET_LINK_MOUSEUP:
    case JET_FIELD_KEYSTROKE:
      return true;
    default:
      return false;
  }
}

WideString& CJS_EventContext::Change() {
  return m_pWideStrChange ? *m_pWideStrChange : m_WideStrChangeDu;
}

ByteStringView CJS_EventContext::Name() const {
  switch (m_eEventType) {
    case JET_APP_INIT:
      return "Init";
    case JET_BATCH_EXEC:
      return "Exec";
    case JET_BOOKMARK_MOUSEUP:
      return "Mouse Up";
    case JET_CONSOLE_EXEC:
      return "Exec";
    case JET_DOC_DIDPRINT:
      return "DidPrint";
    case JET_DOC_DIDSAVE:
      return "DidSave";
    case JET_DOC_OPEN:
      return "Open";
    case JET_DOC_WILLCLOSE:
      return "WillClose";
    case JET_DOC_WILLPRINT:
      return "WillPrint";
    case JET_DOC_WILLSAVE:
      return "WillSave";
    case JET_EXTERNAL_EXEC:
      return "Exec";
    case JET_FIELD_FOCUS:
    case JET_SCREEN_FOCUS:
      return "Focus";
    case JET_FIELD_BLUR:
    case JET_SCREEN_BLUR:
      return "Blur";
    case JET_FIELD_MOUSEDOWN:
    case JET_SCREEN_MOUSEDOWN:
      return "Mouse Down";
    case JET_FIELD_MOUSEUP:
    case JET_SCREEN_MOUSEUP:
      return "Mouse Up";
    case JET_FIELD_MOUSEENTER:
    case JET_SCREEN_MOUSEENTER:
      return "Mouse Enter";
    case JET_FIELD_MOUSEEXIT:
    case JET_SCREEN_MOUSEEXIT:
      return "Mouse Exit";
    case JET_FIELD_CALCULATE:
      return "Calculate";
    case JET_FIELD_FORMAT:
      return "Format";
    case JET_FIELD_KEYSTROKE:
      return "Keystroke";
    case JET_FIELD_VALIDATE:
      return "Validate";
    case JET_LINK_MOUSEUP:
      return "Mouse Up";
    case JET_MENU_EXEC:
      return "Exec";
    case JET_PAGE_OPEN:
    case JET_SCREEN_OPEN:
      return "Open";
    case JET_PAGE_CLOSE:
    case JET_SCREEN_CLOSE:
      return "Close";
    case JET_SCREEN_INVIEW:
    case JET_PAGE_INVIEW:
      return "InView";
    case JET_PAGE_OUTVIEW:
    case JET_SCREEN_OUTVIEW:
      return "OutView";
    default:
      return "";
  }
}

ByteStringView CJS_EventContext::Type() const {
  switch (m_eEventType) {
    case JET_APP_INIT:
      return "App";
    case JET_BATCH_EXEC:
      return "Batch";
    case JET_BOOKMARK_MOUSEUP:
      return "BookMark";
    case JET_CONSOLE_EXEC:
      return "Console";
    case JET_DOC_DIDPRINT:
    case JET_DOC_DIDSAVE:
    case JET_DOC_OPEN:
    case JET_DOC_WILLCLOSE:
    case JET_DOC_WILLPRINT:
    case JET_DOC_WILLSAVE:
      return "Doc";
    case JET_EXTERNAL_EXEC:
      return "External";
    case JET_FIELD_BLUR:
    case JET_FIELD_FOCUS:
    case JET_FIELD_MOUSEDOWN:
    case JET_FIELD_MOUSEENTER:
    case JET_FIELD_MOUSEEXIT:
    case JET_FIELD_MOUSEUP:
    case JET_FIELD_CALCULATE:
    case JET_FIELD_FORMAT:
    case JET_FIELD_KEYSTROKE:
    case JET_FIELD_VALIDATE:
      return "Field";
    case JET_SCREEN_FOCUS:
    case JET_SCREEN_BLUR:
    case JET_SCREEN_OPEN:
    case JET_SCREEN_CLOSE:
    case JET_SCREEN_MOUSEDOWN:
    case JET_SCREEN_MOUSEUP:
    case JET_SCREEN_MOUSEENTER:
    case JET_SCREEN_MOUSEEXIT:
    case JET_SCREEN_INVIEW:
    case JET_SCREEN_OUTVIEW:
      return "Screen";
    case JET_LINK_MOUSEUP:
      return "Link";
    case JET_MENU_EXEC:
      return "Menu";
    case JET_PAGE_OPEN:
    case JET_PAGE_CLOSE:
    case JET_PAGE_INVIEW:
    case JET_PAGE_OUTVIEW:
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
