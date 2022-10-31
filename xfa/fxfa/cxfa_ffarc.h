// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFARC_H_
#define XFA_FXFA_CXFA_FFARC_H_

#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFArc final : public CXFA_FFWidget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFArc() override;

  // CXFA_FFWidget
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;

 private:
  explicit CXFA_FFArc(CXFA_Node* pnode);
};

#endif  // XFA_FXFA_CXFA_FFARC_H_
