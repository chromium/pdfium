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
  std::optional<IJS_Runtime::JS_Error> RunScript(
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

  CJS_Runtime* GetJSRuntime() const { return runtime_; }
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const {
    return form_fill_env_.Get();
  }
  CJS_Field* SourceField();
  CJS_Field* TargetField();

  Kind EventKind() const { return kind_; }
  bool IsValid() const { return valid_; }
  bool IsUserGesture() const;
  WideString& Change();
  WideString ChangeEx() const { return change_ex_; }
  WideString SourceName() const { return source_name_; }
  WideString TargetName() const { return target_name_; }
  int CommitKey() const { return commit_key_; }
  bool FieldFull() const { return field_full_; }
  bool KeyDown() const { return key_down_; }
  bool Modifier() const { return modifier_; }
  ByteStringView Name() const;
  ByteStringView Type() const;
  bool& Rc();
  int SelEnd() const;
  int SelStart() const;
  void SetSelEnd(int value);
  void SetSelStart(int value);
  bool Shift() const { return shift_; }
  bool HasValue() const { return !!value_; }
  WideString& Value() { return *value_; }
  bool WillCommit() const { return will_commit_; }

  void SetValueForTest(WideString* pStr) { value_ = pStr; }
  void SetRCForTest(bool* pRC) { pb_rc_ = pRC; }
  void SetStrChangeForTest(WideString* pStrChange) { change_ = pStrChange; }
  void ResetWillCommitForTest() { will_commit_ = false; }

 private:
  void Initialize(Kind kind);
  void Destroy();

  UnownedPtr<CJS_Runtime> const runtime_;
  ObservedPtr<CPDFSDK_FormFillEnvironment> form_fill_env_;
  Kind kind_ = Kind::kUnknown;
  bool busy_ = false;
  bool valid_ = false;
  UnownedPtr<WideString> value_;
  WideString source_name_;
  WideString target_name_;
  WideString change_du_;
  WideString change_ex_;
  UnownedPtr<WideString> change_;
  int commit_key_ = -1;
  bool key_down_ = false;
  bool modifier_ = false;
  bool shift_ = false;
  int sel_end_du_ = 0;
  int sel_start_du_ = 0;
  UnownedPtr<int> sel_end_;
  UnownedPtr<int> sel_start_;
  bool will_commit_ = false;
  bool field_full_ = false;
  bool rc_du_ = false;
  UnownedPtr<bool> pb_rc_;
};

#endif  // FXJS_CJS_EVENT_CONTEXT_H_
