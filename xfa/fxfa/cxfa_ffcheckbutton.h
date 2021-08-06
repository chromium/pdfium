// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFCHECKBUTTON_H_
#define XFA_FXFA_CXFA_FFCHECKBUTTON_H_

#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_CheckButton;

class CXFA_FFCheckButton final : public CXFA_FFField {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFCheckButton() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_FFField
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;

  bool LoadWidget() override;
  bool PerformLayout() override;
  bool UpdateFWLData() override;
  void UpdateWidgetProperty() override;
  bool OnLButtonUp(FWL_KeyFlagMask dwFlags, const CFX_PointF& point) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  FormFieldType GetFormFieldType() override;

  void SetFWLCheckState(XFA_CheckState eCheckState);

 private:
  CXFA_FFCheckButton(CXFA_Node* pNode, CXFA_CheckButton* button);

  bool CommitData() override;
  bool IsDataChanged() override;
  void CapLeftRightPlacement(const CXFA_Margin* captionMargin);
  void AddUIMargin(XFA_AttributeValue iCapPlacement);
  XFA_CheckState FWLState2XFAState();

  cppgc::Member<IFWL_WidgetDelegate> m_pOldDelegate;
  cppgc::Member<CXFA_CheckButton> const button_;
  CFX_RectF m_CheckBoxRect;
};

#endif  // XFA_FXFA_CXFA_FFCHECKBUTTON_H_
