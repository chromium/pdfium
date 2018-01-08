// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffrectangle.h"

#include "xfa/fxfa/parser/cxfa_rectangle.h"
#include "xfa/fxfa/parser/cxfa_value.h"

CXFA_FFRectangle::CXFA_FFRectangle(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFDraw(pDataAcc) {}

CXFA_FFRectangle::~CXFA_FFRectangle() {}

void CXFA_FFRectangle::RenderWidget(CXFA_Graphics* pGS,
                                    const CFX_Matrix& matrix,
                                    uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_Value* value = m_pDataAcc->GetNode()->GetFormValue();
  if (!value)
    return;

  CFX_RectF rect = GetRectWithoutRotate();
  CXFA_Margin* margin = m_pDataAcc->GetNode()->GetMargin();
  if (margin)
    XFA_RectWidthoutMargin(rect, margin);

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  DrawBorder(pGS, value->GetRectangle(), rect, mtRotate);
}
