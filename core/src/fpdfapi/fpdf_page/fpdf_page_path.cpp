// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "pageint.h"

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_pageobj.h"

void CPDF_PathObject::CopyData(const CPDF_PageObject* pSrc) {
  const CPDF_PathObject* pSrcObj = (const CPDF_PathObject*)pSrc;
  m_Path = pSrcObj->m_Path;
  m_FillType = pSrcObj->m_FillType;
  m_bStroke = pSrcObj->m_bStroke;
  m_Matrix = pSrcObj->m_Matrix;
}
void CPDF_PathObject::Transform(const CFX_Matrix& matrix) {
  m_Matrix.Concat(matrix);
  CalcBoundingBox();
}
void CPDF_PathObject::SetGraphState(CPDF_GraphState GraphState) {
  m_GraphState = GraphState;
  CalcBoundingBox();
}
void CPDF_PathObject::CalcBoundingBox() {
  if (m_Path.IsNull()) {
    return;
  }
  CFX_FloatRect rect;
  FX_FLOAT width = m_GraphState.GetObject()->m_LineWidth;
  if (m_bStroke && width != 0) {
    rect = m_Path.GetBoundingBox(width, m_GraphState.GetObject()->m_MiterLimit);
  } else {
    rect = m_Path.GetBoundingBox();
  }
  rect.Transform(&m_Matrix);
  if (width == 0 && m_bStroke) {
    rect.left += -0.5f;
    rect.right += 0.5f;
    rect.bottom += -0.5f;
    rect.top += 0.5f;
  }
  m_Left = rect.left;
  m_Right = rect.right;
  m_Top = rect.top;
  m_Bottom = rect.bottom;
}
