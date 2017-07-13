// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_ffarc.h"

CXFA_FFArc::CXFA_FFArc(CXFA_WidgetAcc* pDataAcc) : CXFA_FFDraw(pDataAcc) {}

CXFA_FFArc::~CXFA_FFArc() {}

void CXFA_FFArc::RenderWidget(CXFA_Graphics* pGS,
                              CFX_Matrix* pMatrix,
                              uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_Value value = m_pDataAcc->GetFormValue();
  if (!value)
    return;

  CXFA_Arc arcObj = value.GetArc();
  CFX_Matrix mtRotate = GetRotateMatrix();
  if (pMatrix)
    mtRotate.Concat(*pMatrix);

  CFX_RectF rtArc = GetRectWithoutRotate();
  if (CXFA_Margin mgWidget = m_pDataAcc->GetMargin())
    XFA_RectWidthoutMargin(rtArc, mgWidget);

  DrawBorder(pGS, arcObj, rtArc, &mtRotate);
}
