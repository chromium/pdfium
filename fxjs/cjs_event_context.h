// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_EVENT_CONTEXT_H_
#define FXJS_CJS_EVENT_CONTEXT_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fxjs/ijs_event_context.h"

class CJS_Field;
class CJS_Runtime;

class CJS_EventContext final : public IJS_EventContext {
 public:
  enum class Kind : uint8_t {
    kUnknown,
    kDocOpen,
    kDocWillPrint,
    kDocDidPrint,
    kDocWillSave,
    kDocDidSave,
    kDocWillClose,
    kPageOpen,
    kPageClose,
    kPageInView,
    kPageOutView,
    kFieldMouseDown,
    kFieldMouseUp,
    kFieldMouseEnter,
    kFieldMouseExit,
    kFieldFocus,
    kFieldBlur,
    kFieldKeystroke,
    kFieldValidate,
    kFieldCalculate,
    kFieldFormat,
    kExternalExec,
  };

  explicit CJS_EventContext(CJS_Runtime* pRuntime);
  ~CJS_EventContext() override;

  // IJS_EventContext
  absl::optional<IJS_Runtime::JS_Error> RunScript(
      const WideString& script) override;
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
  void OnExternal_Exec() override;

  CJS_Runtime* GetJSRuntime() const { return m_pRuntime; }
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const {
    return m_pFormFillEnv.Get();
  }
  CJS_Field* SourceField();
  CJS_Field* TargetField();

  Kind EventKind() const { return m_eKind; }
  bool IsValid() const { return m_bValid; }
  bool IsUserGesture() const;
  WideString& Change();
  WideString ChangeEx() const { return m_WideStrChangeEx; }
  WideString SourceName() const { return m_strSourceName; }
  WideString TargetName() const { return m_strTargetName; }
  int CommitKey() const { return m_nCommitKey; }
  bool FieldFull() const { return m_bFieldFull; }
  bool KeyDown() const { return m_bKeyDown; }
  bool Modifier() const { return m_bModifier; }
  ByteStringView Name() const;
  ByteStringView Type() const;
  bool& Rc();
  int SelEnd() const;
  int SelStart() const;
  void SetSelEnd(int value);
  void SetSelStart(int value);
  bool Shift() const { return m_bShift; }
  bool HasValue() const { return !!m_pValue; }
  WideString& Value() { return *m_pValue; }
  bool WillCommit() const { return m_bWillCommit; }

  void SetValueForTest(WideString* pStr) { m_pValue = pStr; }
  void SetRCForTest(bool* pRC) { m_pbRc = pRC; }
  void SetStrChangeForTest(WideString* pStrChange) {
    m_pWideStrChange = pStrChange;
  }
  void ResetWillCommitForTest() { m_bWillCommit = false; }

 private:
  void Initialize(Kind kind);
  void Destroy();

  UnownedPtr<CJS_Runtime> const m_pRuntime;
  ObservedPtr<CPDFSDK_FormFillEnvironment> m_pFormFillEnv;
  Kind m_eKind = Kind::kUnknown;
  bool m_bBusy = false;
  bool m_bValid = false;
  UnownedPtr<WideString> m_pValue;
  WideString m_strSourceName;
  WideString m_strTargetName;
  WideString m_WideStrChangeDu;
  WideString m_WideStrChangeEx;
  UnownedPtr<WideString> m_pWideStrChange;
  int m_nCommitKey = -1;
  bool m_bKeyDown = false;
  bool m_bModifier = false;
  bool m_bShift = false;
  int m_nSelEndDu = 0;
  int m_nSelStartDu = 0;
  UnownedPtr<int> m_pISelEnd;
  UnownedPtr<int> m_pISelStart;
  bool m_bWillCommit = false;
  bool m_bFieldFull = false;
  bool m_bRcDu = false;
  UnownedPtr<bool> m_pbRc;
};

#endif  // FXJS_CJS_EVENT_CONTEXT_H_
