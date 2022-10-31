// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFSIGNATURE_H_
#define XFA_FXFA_CXFA_FFSIGNATURE_H_

#include "xfa/fxfa/cxfa_fffield.h"

class CXFA_FFSignature final : public CXFA_FFField {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFSignature() override;

  // CXFA_FFField
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool LoadWidget() override;
  bool AcceptsFocusOnButtonDown(
      Mask<XFA_FWL_KeyFlag> dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;
  bool OnMouseEnter() override;
  bool OnMouseExit() override;
  bool OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnLButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnLButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                       const CFX_PointF& point) override;
  bool OnMouseMove(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnMouseWheel(Mask<XFA_FWL_KeyFlag> dwFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta) override;
  bool OnRButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnRButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                       const CFX_PointF& point) override;
  bool OnKeyDown(XFA_FWL_VKEYCODE dwKeyCode,
                 Mask<XFA_FWL_KeyFlag> dwFlags) override;
  bool OnChar(uint32_t dwChar, Mask<XFA_FWL_KeyFlag> dwFlags) override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  FormFieldType GetFormFieldType() override;

 private:
  explicit CXFA_FFSignature(CXFA_Node* pNode);
};

#endif  // XFA_FXFA_CXFA_FFSIGNATURE_H_
