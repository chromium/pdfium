// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFRECTANGLE_H_
#define XFA_FXFA_CXFA_FFRECTANGLE_H_

#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFRectangle final : public CXFA_FFWidget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFRectangle() override;

  // CXFA_FFWidget
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;

 private:
  explicit CXFA_FFRectangle(CXFA_Node* pNode);
};

#endif  // XFA_FXFA_CXFA_FFRECTANGLE_H_
