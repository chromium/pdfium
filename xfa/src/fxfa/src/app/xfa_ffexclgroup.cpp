// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffwidget.h"
#include "xfa_ffexclgroup.h"
#include "xfa_ffpageview.h"
#include "xfa_ffapp.h"
#include "xfa_ffdoc.h"
CXFA_FFExclGroup::CXFA_FFExclGroup(CXFA_FFPageView* pPageView,
                                   CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFWidget(pPageView, pDataAcc) {}
CXFA_FFExclGroup::~CXFA_FFExclGroup() {}
void CXFA_FFExclGroup::RenderWidget(CFX_Graphics* pGS,
                                    CFX_Matrix* pMatrix,
                                    FX_DWORD dwStatus,
                                    int32_t iRotate) {
  if (!IsMatchVisibleStatus(dwStatus)) {
    return;
  }
  CFX_Matrix mtRotate;
  GetRotateMatrix(mtRotate);
  if (pMatrix) {
    mtRotate.Concat(*pMatrix);
  }
  CXFA_FFWidget::RenderWidget(pGS, &mtRotate, dwStatus);
}
