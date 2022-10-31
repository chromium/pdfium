// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFLINE_H_
#define XFA_FXFA_CXFA_FFLINE_H_

#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFLine final : public CXFA_FFWidget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFLine() override;

  // CXFA_FFWidget
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;

 private:
  explicit CXFA_FFLine(CXFA_Node* pNode);

  void GetRectFromHand(CFX_RectF& rect,
                       XFA_AttributeValue iHand,
                       float fLineWidth);
};

#endif  // XFA_FXFA_CXFA_FFLINE_H_
