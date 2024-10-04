// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_graphstate.h"

#include <utility>

CFX_GraphState::CFX_GraphState() = default;

CFX_GraphState::CFX_GraphState(const CFX_GraphState& that) = default;

CFX_GraphState::~CFX_GraphState() = default;

void CFX_GraphState::Emplace() {
  m_Ref.Emplace();
}

void CFX_GraphState::SetLineDash(std::vector<float> dashes, float phase) {
  CFX_GraphStateData* pData = m_Ref.GetPrivateCopy();
  pData->set_dash_phase(phase);
  pData->set_dash_array(std::move(dashes));
}

void CFX_GraphState::SetLineDashPhase(float phase) {
  CFX_GraphStateData* pData = m_Ref.GetPrivateCopy();
  pData->set_dash_phase(phase);
}

std::vector<float> CFX_GraphState::GetLineDashArray() const {
  std::vector<float> ret;
  if (m_Ref.GetObject()) {
    ret = m_Ref.GetObject()->dash_array();
  }
  return ret;
}

size_t CFX_GraphState::GetLineDashSize() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->dash_array().size() : 0;
}

float CFX_GraphState::GetLineDashPhase() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->dash_phase() : 1.0f;
}

float CFX_GraphState::GetLineWidth() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->line_width() : 1.0f;
}

void CFX_GraphState::SetLineWidth(float width) {
  m_Ref.GetPrivateCopy()->set_line_width(width);
}

CFX_GraphStateData::LineCap CFX_GraphState::GetLineCap() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->line_cap()
                           : CFX_GraphStateData::LineCap::kButt;
}
void CFX_GraphState::SetLineCap(CFX_GraphStateData::LineCap cap) {
  m_Ref.GetPrivateCopy()->set_line_cap(cap);
}

CFX_GraphStateData::LineJoin CFX_GraphState::GetLineJoin() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->line_join()
                           : CFX_GraphStateData::LineJoin::kMiter;
}

void CFX_GraphState::SetLineJoin(CFX_GraphStateData::LineJoin join) {
  m_Ref.GetPrivateCopy()->set_line_join(join);
}

float CFX_GraphState::GetMiterLimit() const {
  return m_Ref.GetObject() ? m_Ref.GetObject()->miter_limit() : 10.f;
}

void CFX_GraphState::SetMiterLimit(float limit) {
  m_Ref.GetPrivateCopy()->set_miter_limit(limit);
}
