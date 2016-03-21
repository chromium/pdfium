// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"

CPDF_PageObject::CPDF_PageObject() {}

CPDF_PageObject::~CPDF_PageObject() {}

void CPDF_PageObject::CopyData(const CPDF_PageObject* pSrc) {
  CopyStates(*pSrc);
  m_Left = pSrc->m_Left;
  m_Right = pSrc->m_Right;
  m_Top = pSrc->m_Top;
  m_Bottom = pSrc->m_Bottom;
}

void CPDF_PageObject::TransformClipPath(CFX_Matrix& matrix) {
  if (m_ClipPath.IsNull()) {
    return;
  }
  m_ClipPath.GetModify();
  m_ClipPath.Transform(matrix);
}

void CPDF_PageObject::TransformGeneralState(CFX_Matrix& matrix) {
  if (m_GeneralState.IsNull()) {
    return;
  }
  CPDF_GeneralStateData* pGS = m_GeneralState.GetModify();
  pGS->m_Matrix.Concat(matrix);
}

FX_RECT CPDF_PageObject::GetBBox(const CFX_Matrix* pMatrix) const {
  CFX_FloatRect rect(m_Left, m_Bottom, m_Right, m_Top);
  if (pMatrix) {
    pMatrix->TransformRect(rect);
  }
  return rect.GetOutterRect();
}
