// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFPUSHBUTTON_H_
#define XFA_FXFA_CXFA_FFPUSHBUTTON_H_

#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/cxfa_fffield.h"

#define XFA_FWL_PSBSTYLEEXT_HiliteInverted (1L << 0)
#define XFA_FWL_PSBSTYLEEXT_HilitePush (1L << 1)
#define XFA_FWL_PSBSTYLEEXT_HiliteOutLine (1L << 2)

class CXFA_Button;
class CXFA_TextLayout;
class CXFA_TextProvider;

class CXFA_FFPushButton final : public CXFA_FFField {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFPushButton() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_FFField
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool LoadWidget() override;
  bool PerformLayout() override;
  void UpdateWidgetProperty() override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  FormFieldType GetFormFieldType() override;

 private:
  CXFA_FFPushButton(CXFA_Node* pNode, CXFA_Button* button);

  void LoadHighlightCaption();
  void LayoutHighlightCaption();
  void RenderHighlightCaption(CFGAS_GEGraphics* pGS, CFX_Matrix* pMatrix);
  float GetLineWidth();
  FX_ARGB GetLineColor();
  FX_ARGB GetFillColor();

  cppgc::Member<CXFA_TextLayout> m_pRolloverTextLayout;
  cppgc::Member<CXFA_TextLayout> m_pDownTextLayout;
  cppgc::Member<CXFA_TextProvider> m_pRollProvider;
  cppgc::Member<CXFA_TextProvider> m_pDownProvider;
  cppgc::Member<IFWL_WidgetDelegate> m_pOldDelegate;
  cppgc::Member<CXFA_Button> const button_;
};

#endif  // XFA_FXFA_CXFA_FFPUSHBUTTON_H_
