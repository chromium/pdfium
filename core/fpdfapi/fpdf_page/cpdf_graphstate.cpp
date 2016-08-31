// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_graphstate.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"

CPDF_GraphState::CPDF_GraphState() {}

CPDF_GraphState::CPDF_GraphState(const CPDF_GraphState& that)
    : m_Ref(that.m_Ref) {}

CPDF_GraphState::~CPDF_GraphState() {}

void CPDF_GraphState::Emplace() {
  m_Ref.Emplace();
}

void CPDF_GraphState::SetLineDash(CPDF_Array* pArray,
                                  FX_FLOAT phase,
                                  FX_FLOAT scale) {
  CFX_GraphStateData* pData = m_Ref.GetPrivateCopy();
  pData->m_DashPhase = phase * scale;
  pData->SetDashCount(static_cast<int>(pArray->GetCount()));
  for (size_t i = 0; i < pArray->GetCount(); i++)
    pData->m_DashArray[i] = pArray->GetNumberAt(i) * scale;
}

FX_FLOAT CPDF_GraphState::GetLineWidth() const {
  return m_Ref.GetObject()->m_LineWidth;
}

void CPDF_GraphState::SetLineWidth(FX_FLOAT width) {
  m_Ref.GetPrivateCopy()->m_LineWidth = width;
}

CFX_GraphStateData::LineCap CPDF_GraphState::GetLineCap() const {
  return m_Ref.GetObject()->m_LineCap;
}
void CPDF_GraphState::SetLineCap(CFX_GraphStateData::LineCap cap) {
  m_Ref.GetPrivateCopy()->m_LineCap = cap;
}

CFX_GraphStateData::LineJoin CPDF_GraphState::GetLineJoin() const {
  return m_Ref.GetObject()->m_LineJoin;
}

void CPDF_GraphState::SetLineJoin(CFX_GraphStateData::LineJoin join) {
  m_Ref.GetPrivateCopy()->m_LineJoin = join;
}

FX_FLOAT CPDF_GraphState::GetMiterLimit() const {
  return m_Ref.GetObject()->m_MiterLimit;
}

void CPDF_GraphState::SetMiterLimit(FX_FLOAT limit) {
  m_Ref.GetPrivateCopy()->m_MiterLimit = limit;
}
