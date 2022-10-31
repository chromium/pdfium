// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffrectangle.h"

#include "xfa/fxfa/parser/cxfa_rectangle.h"
#include "xfa/fxfa/parser/cxfa_value.h"

CXFA_FFRectangle::CXFA_FFRectangle(CXFA_Node* pNode) : CXFA_FFWidget(pNode) {}

CXFA_FFRectangle::~CXFA_FFRectangle() = default;

void CXFA_FFRectangle::RenderWidget(CFGAS_GEGraphics* pGS,
                                    const CFX_Matrix& matrix,
                                    HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CXFA_Value* value = m_pNode->GetFormValueIfExists();
  if (!value)
    return;

  CFX_RectF rect = GetRectWithoutRotate();
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  XFA_RectWithoutMargin(&rect, margin);

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);
  DrawBorder(pGS, value->GetRectangleIfExists(), rect, mtRotate);
}
