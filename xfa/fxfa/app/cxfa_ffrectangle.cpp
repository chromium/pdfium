// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_ffrectangle.h"

CXFA_FFRectangle::CXFA_FFRectangle(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFDraw(pDataAcc) {}

CXFA_FFRectangle::~CXFA_FFRectangle() {}

void CXFA_FFRectangle::RenderWidget(CXFA_Graphics* pGS,
                                    CFX_Matrix* pMatrix,
                                    uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_Value value = m_pDataAcc->GetFormValue();
  if (!value)
    return;

  CXFA_Rectangle rtObj = value.GetRectangle();
  CFX_RectF rect = GetRectWithoutRotate();
  if (CXFA_Margin mgWidget = m_pDataAcc->GetMargin())
    XFA_RectWidthoutMargin(rect, mgWidget);

  CFX_Matrix mtRotate = GetRotateMatrix();
  if (pMatrix)
    mtRotate.Concat(*pMatrix);

  DrawBorder(pGS, rtObj, rect, &mtRotate);
}
