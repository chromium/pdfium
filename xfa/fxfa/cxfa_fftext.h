// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFTEXT_H_
#define XFA_FXFA_CXFA_FFTEXT_H_

#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFText final : public CXFA_FFWidget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFText() override;

  // CXFA_FFWidget
  bool AcceptsFocusOnButtonDown(
      uint32_t dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;
  bool OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) override;
  bool OnLButtonUp(uint32_t dwFlags, const CFX_PointF& point) override;
  bool OnMouseMove(uint32_t dwFlags, const CFX_PointF& point) override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool IsLoaded() override;
  bool PerformLayout() override;

 private:
  explicit CXFA_FFText(CXFA_Node* pNode);

  // Returns empty string when no link is present.
  WideString GetLinkURLAtPoint(const CFX_PointF& point);
};

#endif  // XFA_FXFA_CXFA_FFTEXT_H_
