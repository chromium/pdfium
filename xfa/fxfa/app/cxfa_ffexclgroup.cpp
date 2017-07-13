// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_ffexclgroup.h"

#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

CXFA_FFExclGroup::CXFA_FFExclGroup(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFWidget(pDataAcc) {}

CXFA_FFExclGroup::~CXFA_FFExclGroup() {}

void CXFA_FFExclGroup::RenderWidget(CXFA_Graphics* pGS,
                                    CFX_Matrix* pMatrix,
                                    uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  if (pMatrix)
    mtRotate.Concat(*pMatrix);

  CXFA_FFWidget::RenderWidget(pGS, &mtRotate, dwStatus);
}
