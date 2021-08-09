// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFTEXTEDIT_H_
#define XFA_FXFA_CXFA_FFTEXTEDIT_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/cxfa_fffield.h"

class CFWL_Event;
class CFWL_EventTextWillChange;
class CFWL_Widget;
class CFX_Matrix;
class CXFA_FFWidget;
class IFWL_WidgetDelegate;

class CXFA_FFTextEdit : public CXFA_FFField {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFTextEdit() override;

  void PreFinalize() override;
  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_FFField
  bool LoadWidget() override;
  void UpdateWidgetProperty() override;
  bool AcceptsFocusOnButtonDown(
      Mask<XFA_FWL_KeyFlag> dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;
  bool OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnSetFocus(CXFA_FFWidget* pOldWidget) override WARN_UNUSED_RESULT;
  bool OnKillFocus(CXFA_FFWidget* pNewWidget) override WARN_UNUSED_RESULT;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void OnTextWillChange(CFWL_Widget* pWidget, CFWL_EventTextWillChange* change);
  void OnTextFull(CFWL_Widget* pWidget);

  // CXFA_FFWidget
  bool CanUndo() override;
  bool CanRedo() override;
  bool CanCopy() override;
  bool CanCut() override;
  bool CanPaste() override;
  bool CanSelectAll() override;
  bool Undo() override;
  bool Redo() override;
  Optional<WideString> Copy() override;
  Optional<WideString> Cut() override;
  bool Paste(const WideString& wsPaste) override;
  void SelectAll() override;
  void Delete() override;
  void DeSelect() override;
  WideString GetText() override;
  FormFieldType GetFormFieldType() override;

 protected:
  explicit CXFA_FFTextEdit(CXFA_Node* pNode);
  uint32_t GetAlignment();

  cppgc::Member<IFWL_WidgetDelegate> m_pOldDelegate;

 private:
  bool CommitData() override;
  bool UpdateFWLData() override;
  bool IsDataChanged() override;
  void ValidateNumberField(const WideString& wsText);
};

#endif  // XFA_FXFA_CXFA_FFTEXTEDIT_H_
