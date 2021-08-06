// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFIMAGEEDIT_H_
#define XFA_FXFA_CXFA_FFIMAGEEDIT_H_

#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/cxfa_fffield.h"

class CXFA_FFImageEdit final : public CXFA_FFField {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFImageEdit() override;

  // CXFA_FFField:
  void PreFinalize() override;
  void Trace(cppgc::Visitor* visitor) const override;
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool LoadWidget() override;
  bool AcceptsFocusOnButtonDown(
      FWL_KeyFlagMask dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;
  bool OnLButtonDown(FWL_KeyFlagMask dwFlags, const CFX_PointF& point) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  FormFieldType GetFormFieldType() override;

 private:
  explicit CXFA_FFImageEdit(CXFA_Node* pNode);

  void SetFWLRect() override;
  bool UpdateFWLData() override;
  bool CommitData() override;

  cppgc::Member<IFWL_WidgetDelegate> m_pOldDelegate;
};

#endif  // XFA_FXFA_CXFA_FFIMAGEEDIT_H_
