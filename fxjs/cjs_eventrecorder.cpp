// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_eventrecorder.h"

#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_formfield.h"

CJS_EventRecorder::CJS_EventRecorder() = default;

CJS_EventRecorder::~CJS_EventRecorder() = default;

void CJS_EventRecorder::OnApp_Init() {
  Initialize(JET_APP_INIT);
}

void CJS_EventRecorder::OnDoc_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                   const WideString& strTargetName) {
  Initialize(JET_DOC_OPEN);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
  m_strTargetName = strTargetName;
}

void CJS_EventRecorder::OnDoc_WillPrint(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_DOC_WILLPRINT);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnDoc_DidPrint(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_DOC_DIDPRINT);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnDoc_WillSave(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_DOC_WILLSAVE);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnDoc_DidSave(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_DOC_DIDSAVE);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnDoc_WillClose(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_DOC_WILLCLOSE);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnPage_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_PAGE_OPEN);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnPage_Close(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_PAGE_CLOSE);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnPage_InView(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_PAGE_INVIEW);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnPage_OutView(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  Initialize(JET_PAGE_OUTVIEW);
  m_pTargetFormFillEnv.Reset(pFormFillEnv);
}

void CJS_EventRecorder::OnField_MouseEnter(bool bModifier,
                                           bool bShift,
                                           CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEENTER);

  m_bModifier = bModifier;
  m_bShift = bShift;

  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventRecorder::OnField_MouseExit(bool bModifier,
                                          bool bShift,
                                          CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEEXIT);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventRecorder::OnField_MouseDown(bool bModifier,
                                          bool bShift,
                                          CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEDOWN);
  m_eEventType = JET_FIELD_MOUSEDOWN;

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventRecorder::OnField_MouseUp(bool bModifier,
                                        bool bShift,
                                        CPDF_FormField* pTarget) {
  Initialize(JET_FIELD_MOUSEUP);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
}

void CJS_EventRecorder::OnField_Focus(bool bModifier,
                                      bool bShift,
                                      CPDF_FormField* pTarget,
                                      WideString* pValue) {
  ASSERT(pValue);
  Initialize(JET_FIELD_FOCUS);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
}

void CJS_EventRecorder::OnField_Blur(bool bModifier,
                                     bool bShift,
                                     CPDF_FormField* pTarget,
                                     WideString* pValue) {
  ASSERT(pValue);
  Initialize(JET_FIELD_BLUR);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
}

void CJS_EventRecorder::OnField_Keystroke(WideString* strChange,
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
  ASSERT(pValue);
  ASSERT(pbRc);
  ASSERT(pSelStart);
  ASSERT(pSelEnd);

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

void CJS_EventRecorder::OnField_Validate(WideString* strChange,
                                         const WideString& strChangeEx,
                                         bool bKeyDown,
                                         bool bModifier,
                                         bool bShift,
                                         CPDF_FormField* pTarget,
                                         WideString* pValue,
                                         bool* pbRc) {
  ASSERT(pValue);
  ASSERT(pbRc);

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

void CJS_EventRecorder::OnField_Calculate(CPDF_FormField* pSource,
                                          CPDF_FormField* pTarget,
                                          WideString* pValue,
                                          bool* pRc) {
  ASSERT(pValue);
  ASSERT(pRc);

  Initialize(JET_FIELD_CALCULATE);

  if (pSource)
    m_strSourceName = pSource->GetFullName();
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_pbRc = pRc;
}

void CJS_EventRecorder::OnField_Format(CPDF_FormField* pTarget,
                                       WideString* pValue) {
  ASSERT(pValue);
  Initialize(JET_FIELD_FORMAT);

  m_nCommitKey = 0;
  m_strTargetName = pTarget->GetFullName();
  m_pValue = pValue;
  m_bWillCommit = true;
}

void CJS_EventRecorder::OnScreen_Focus(bool bModifier,
                                       bool bShift,
                                       CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_FOCUS);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_Blur(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_BLUR);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_Open(bool bModifier,
                                      bool bShift,
                                      CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_OPEN);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_Close(bool bModifier,
                                       bool bShift,
                                       CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_CLOSE);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_MouseDown(bool bModifier,
                                           bool bShift,
                                           CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEDOWN);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_MouseUp(bool bModifier,
                                         bool bShift,
                                         CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEUP);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_MouseEnter(bool bModifier,
                                            bool bShift,
                                            CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEENTER);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_MouseExit(bool bModifier,
                                           bool bShift,
                                           CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_MOUSEEXIT);

  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_InView(bool bModifier,
                                        bool bShift,
                                        CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_INVIEW);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnScreen_OutView(bool bModifier,
                                         bool bShift,
                                         CPDFSDK_Annot* pScreen) {
  Initialize(JET_SCREEN_OUTVIEW);
  m_bModifier = bModifier;
  m_bShift = bShift;
  m_pTargetAnnot.Reset(pScreen);
}

void CJS_EventRecorder::OnLink_MouseUp(
    CPDFSDK_FormFillEnvironment* pTargetFormFillEnv) {
  Initialize(JET_LINK_MOUSEUP);
  m_pTargetFormFillEnv.Reset(pTargetFormFillEnv);
}

void CJS_EventRecorder::OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) {
  Initialize(JET_BOOKMARK_MOUSEUP);
  m_pTargetBookMark = pBookMark;
}

void CJS_EventRecorder::OnMenu_Exec(
    CPDFSDK_FormFillEnvironment* pTargetFormFillEnv,
    const WideString& strTargetName) {
  Initialize(JET_MENU_EXEC);
  m_pTargetFormFillEnv.Reset(pTargetFormFillEnv);
  m_strTargetName = strTargetName;
}

void CJS_EventRecorder::OnExternal_Exec() {
  Initialize(JET_EXTERNAL_EXEC);
}

void CJS_EventRecorder::OnBatchExec(
    CPDFSDK_FormFillEnvironment* pTargetFormFillEnv) {
  Initialize(JET_BATCH_EXEC);
  m_pTargetFormFillEnv.Reset(pTargetFormFillEnv);
}

void CJS_EventRecorder::OnConsole_Exec() {
  Initialize(JET_CONSOLE_EXEC);
}

void CJS_EventRecorder::Initialize(JS_EVENT_T type) {
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
  m_pTargetFormFillEnv.Reset();
  m_pTargetAnnot.Reset();
  m_bValid = true;
}

void CJS_EventRecorder::Destroy() {
  m_bValid = false;
}

bool CJS_EventRecorder::IsUserGesture() const {
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

WideString& CJS_EventRecorder::Change() {
  return m_pWideStrChange ? *m_pWideStrChange : m_WideStrChangeDu;
}

ByteStringView CJS_EventRecorder::Name() const {
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

ByteStringView CJS_EventRecorder::Type() const {
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

bool& CJS_EventRecorder::Rc() {
  return m_pbRc ? *m_pbRc : m_bRcDu;
}

int CJS_EventRecorder::SelEnd() const {
  return m_pISelEnd ? *m_pISelEnd : m_nSelEndDu;
}

int CJS_EventRecorder::SelStart() const {
  return m_pISelStart ? *m_pISelStart : m_nSelStartDu;
}

void CJS_EventRecorder::SetSelEnd(int value) {
  int& target = m_pISelEnd ? *m_pISelEnd : m_nSelEndDu;
  target = value;
}

void CJS_EventRecorder::SetSelStart(int value) {
  int& target = m_pISelStart ? *m_pISelStart : m_nSelStartDu;
  target = value;
}
