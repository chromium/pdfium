// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_FFLINE_H_
#define XFA_FXFA_APP_CXFA_FFLINE_H_

#include "xfa/fxfa/app/cxfa_ffdraw.h"

class CXFA_FFLine : public CXFA_FFDraw {
 public:
  explicit CXFA_FFLine(CXFA_WidgetAcc* pDataAcc);
  ~CXFA_FFLine() override;

  // CXFA_FFWidget
  void RenderWidget(CXFA_Graphics* pGS,
                    CFX_Matrix* pMatrix,
                    uint32_t dwStatus) override;

 private:
  void GetRectFromHand(CFX_RectF& rect, int32_t iHand, float fLineWidth);
};

#endif  // XFA_FXFA_APP_CXFA_FFLINE_H_
