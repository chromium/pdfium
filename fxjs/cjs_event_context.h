// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_EVENT_CONTEXT_H_
#define FXJS_CJS_EVENT_CONTEXT_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/ijs_event_context.h"

class CJS_EventRecorder;
class CJS_Field;
class CJS_Runtime;
class CPDFSDK_FormFillEnvironment;

class CJS_EventContext final : public IJS_EventContext {
 public:
  explicit CJS_EventContext(CJS_Runtime* pRuntime);
  ~CJS_EventContext() override;

  // IJS_EventContext
  Optional<IJS_Runtime::JS_Error> RunScript(const WideString& script) override;
  void OnApp_Init() override;
  void OnDoc_Open(const WideString& strTargetName) override;
  void OnDoc_WillPrint() override;
  void OnDoc_DidPrint() override;
  void OnDoc_WillSave() override;
  void OnDoc_DidSave() override;
  void OnDoc_WillClose() override;
  void OnPage_Open() override;
  void OnPage_Close() override;
  void OnPage_InView() override;
  void OnPage_OutView() override;
  void OnField_MouseDown(bool bModifier,
                         bool bShift,
                         CPDF_FormField* pTarget) override;
  void OnField_MouseEnter(bool bModifier,
                          bool bShift,
                          CPDF_FormField* pTarget) override;
  void OnField_MouseExit(bool bModifier,
                         bool bShift,
                         CPDF_FormField* pTarget) override;
  void OnField_MouseUp(bool bModifier,
                       bool bShift,
                       CPDF_FormField* pTarget) override;
  void OnField_Focus(bool bModifier,
                     bool bShift,
                     CPDF_FormField* pTarget,
                     WideString* Value) override;
  void OnField_Blur(bool bModifier,
                    bool bShift,
                    CPDF_FormField* pTarget,
                    WideString* Value) override;
  void OnField_Calculate(CPDF_FormField* pSource,
                         CPDF_FormField* pTarget,
                         WideString* pValue,
                         bool* pRc) override;
  void OnField_Format(CPDF_FormField* pTarget, WideString* Value) override;
  void OnField_Keystroke(WideString* strChange,
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
                         bool* bRc) override;
  void OnField_Validate(WideString* strChange,
                        const WideString& strChangeEx,
                        bool bKeyDown,
                        bool bModifier,
                        bool bShift,
                        CPDF_FormField* pTarget,
                        WideString* Value,
                        bool* bRc) override;
  void OnScreen_Focus(bool bModifier,
                      bool bShift,
                      CPDFSDK_Annot* pScreen) override;
  void OnScreen_Blur(bool bModifier,
                     bool bShift,
                     CPDFSDK_Annot* pScreen) override;
  void OnScreen_Open(bool bModifier,
                     bool bShift,
                     CPDFSDK_Annot* pScreen) override;
  void OnScreen_Close(bool bModifier,
                      bool bShift,
                      CPDFSDK_Annot* pScreen) override;
  void OnScreen_MouseDown(bool bModifier,
                          bool bShift,
                          CPDFSDK_Annot* pScreen) override;
  void OnScreen_MouseUp(bool bModifier,
                        bool bShift,
                        CPDFSDK_Annot* pScreen) override;
  void OnScreen_MouseEnter(bool bModifier,
                           bool bShift,
                           CPDFSDK_Annot* pScreen) override;
  void OnScreen_MouseExit(bool bModifier,
                          bool bShift,
                          CPDFSDK_Annot* pScreen) override;
  void OnScreen_InView(bool bModifier,
                       bool bShift,
                       CPDFSDK_Annot* pScreen) override;
  void OnScreen_OutView(bool bModifier,
                        bool bShift,
                        CPDFSDK_Annot* pScreen) override;
  void OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) override;
  void OnLink_MouseUp() override;
  void OnMenu_Exec(const WideString& strTargetName) override;
  void OnBatch_Exec() override;
  void OnConsole_Exec() override;
  void OnExternal_Exec() override;

  CJS_Runtime* GetJSRuntime() const { return m_pRuntime.Get(); }
  CJS_EventRecorder* GetEventRecorder() const { return m_pEventRecorder.get(); }
  CPDFSDK_FormFillEnvironment* GetFormFillEnv();
  CJS_Field* SourceField();
  CJS_Field* TargetField();

 private:
  UnownedPtr<CJS_Runtime> const m_pRuntime;
  std::unique_ptr<CJS_EventRecorder> const m_pEventRecorder;
  bool m_bBusy = false;
};

#endif  // FXJS_CJS_EVENT_CONTEXT_H_
